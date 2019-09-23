/**
 * @file pa_fwupdate_qmi.c
 *
 * QMI implementation of @ref c_pa_fwupdate API.
 *
 * This PA supports writing and reading the SWI FOTA partition that is accessible from the modem.
 *
 * Write/Download Design Overview:
 *
 * The main functionality provided is to download CWE images to the modem.  This consists of
 * splitting up the image file into blocks (up to 32KB), and writing each block using the QMI
 * FDT_WRITE message.
 *
 * If an error occurs during writing, then the modem sends an FDT_IND message, giving the reason,
 * and the download process is stopped.
 *
 * Once the complete image is written to the modem, the process will close the FDT partition, and
 * wait for a final FDT IND, which is used to indicate that the download was successfully completed.
 * After this is completed (and the AVMS Selection Accept is sent), the modem will reset in order
 * to apply the updated image.
 *
 * There are two threads involved in the download process. A Legato thread which is the thread
 * that calls the API functions, i.e. pa_fwupdate_Download(), and a QMI thread which receives the
 * FDT_IND messages. Thus, a semaphore and some shared data is used to communicate between these
 * two threads.
 *
 * The current implementation does not support resuming downloads.  This will be supported later.
 *
 * Read Design Overview:
 *
 *
 *
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

// Need to use linux semaphores
#include <semaphore.h>

#include "legato.h"
#include "pa_fwupdate.h"
#include "pa_fwupdate_qmi_common.h"
#include "swiQmi.h"
#include "le_fs.h"
#include "watchdogChain.h"
#include "fwupdate_local.h"

// for print macros
#include "le_print.h"

// For htonl
#include <arpa/inet.h>

// FW update status result
#define FWUPDATE_STATUS_OK             0x00000001
#define FWUPDATE_STATUS_UNKNOWN        0xFFFFFFFF
#define FWUPDATE_STATUS_DFLTS_ERROR    0x08000000
#define FWUPDATE_STATUS_FILE_ERROR     0x10000000
#define FWUPDATE_STATUS_NVUP_ERROR     0x20000000
#define FWUPDATE_STATUS_UA_ERROR       0x40000000
#define FWUPDATE_STATUS_BL_ERROR       0x80000000
#define FWUPDATE_STATUS_ERROR_MASK     0x00000FFF

// Default timeout
#define DEFAULT_TIMEOUT_MS     900000

//--------------------------------------------------------------------------------------------------
/**
 * max events managed by epoll
 */
//--------------------------------------------------------------------------------------------------
#define MAX_EVENTS 10

//--------------------------------------------------------------------------------------------------
/**
 * Image size offset
 */
//--------------------------------------------------------------------------------------------------
#define CWE_IMAGE_SIZE_OFST     0x114

//--------------------------------------------------------------------------------------------------
/**
 * CWE image header size
 */
//--------------------------------------------------------------------------------------------------
#define CWE_HEADER_SIZE         400

//--------------------------------------------------------------------------------------------------
/**
 * File name where the image length is stored
 */
//--------------------------------------------------------------------------------------------------
#define FULL_IMAGE_LENGTH_PATH  "/avc/fw/FullImageLength"

//--------------------------------------------------------------------------------------------------
/**
 * Handle for the open swi-fota partition.
 */
//--------------------------------------------------------------------------------------------------
static uint32_t CurrentFdtHandle = 0;


//--------------------------------------------------------------------------------------------------
/**
 * The DMS (Device Management Service) client.  Must be obtained by calling
 * swiQmi_GetClientHandle() before it can be used.
 */
//--------------------------------------------------------------------------------------------------
static qmi_client_type DmsClient = NULL;

//--------------------------------------------------------------------------------------------------
/**
 * The MGS (M2M General Service) client.  Must be obtained by calling
 * swiQmi_GetClientHandle() before it can be used.
 */
//--------------------------------------------------------------------------------------------------
static qmi_client_type MgsClient;

//--------------------------------------------------------------------------------------------------
/**
 * Reason for closing FDT
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    REASON_NORMAL,                  ///< indicate FOTA update; triggers reset
    REASON_OMADM_FUMO,              ///< legacy OMADM update; not used by LWM2M
    REASON_FLASH_WRITE_ERROR,       ///< error while writing to swi-fota
    REASON_INVAID_SIGNATURE,        ///< signature of downloaded package is invalid
    REASON_FLASH_READ_ERROR,        ///< error while reading from swi-fota
    REASON_NORMAL_SOTA,             ///< firmware will not process swi-fota image;
                                    ///< doesn't trigger reset
    REASON_INVALID,                 ///< invalid
}
FDTCloseReason_t;

//--------------------------------------------------------------------------------------------------
/**
 * Data associated with an FDT indication.
 *
 * Since there is only one field, it is probably not necessary to have a struct, but it may be
 * useful for future expansion, since additional functionality is still to added for this service.
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    FDTCloseReason_t closeReason;      ///< The reason the FDT session is closed
}
FDTIndicationData_t;

//--------------------------------------------------------------------------------------------------
/**
 * Global data for the FDT indication.  This data needs to be shared between a QMI thread and a
 * Legato thread.  One global instance is sufficient, since FW update is not thread safe.
 */
//--------------------------------------------------------------------------------------------------
static FDTIndicationData_t FDTIndicationData;


//--------------------------------------------------------------------------------------------------
/**
 * Semaphore used to communicate/synchronize between QMI thread and Legato thread.
 */
//--------------------------------------------------------------------------------------------------
static le_sem_Ref_t Semaphore = NULL;


//--------------------------------------------------------------------------------------------------
/**
 * Total # of bytes read from SWI FOTA partition and written to pipe for one app package read.
 *
 * Accessed by pa_fwupdate_Read() and FDTReadHandler()
 */
//--------------------------------------------------------------------------------------------------
size_t TotalBytes = 0;


//--------------------------------------------------------------------------------------------------
/**
 * Size of the app package to read from the SWI FOTA partition
 *
 * Accessed by pa_fwupdate_Read() and FDTReadHandler()
 */
//--------------------------------------------------------------------------------------------------
size_t PackageSizeBytes = 0;

//--------------------------------------------------------------------------------------------------
/**
 * Fota resume offset, initialized by pa_fwupdate_GetResumePosition
 */
//--------------------------------------------------------------------------------------------------
static size_t ResumeOffset = 0;

//--------------------------------------------------------------------------------------------------
/**
 * Size of the image
 */
//--------------------------------------------------------------------------------------------------
static ssize_t  FullImageLength = 0;

//--------------------------------------------------------------------------------------------------
/**
 * Handler for QMI_SWI_M2M_FDT_IND_V01.
 */
//--------------------------------------------------------------------------------------------------
static void FDTIndicationHandler
(
    void*           indBufPtr,   ///< [IN] The indication message data.
    unsigned int    indBufLen,   ///< [IN] The indication message data length in bytes.
    void*           contextPtr   ///< [IN] Context value that was passed to
                                 ///       swiQmi_AddIndicationHandler().
)
{
    qmi_client_error_type rc;
    SWIQMI_DECLARE_MESSAGE(swi_m2m_fdt_ind_msg_v01, fdtInd);

    rc = qmi_client_message_decode(MgsClient,
                                   QMI_IDL_INDICATION,
                                   QMI_SWI_M2M_FDT_IND_V01,
                                   indBufPtr, indBufLen,
                                   &fdtInd, sizeof(fdtInd));
    if (QMI_NO_ERR != rc)
    {
        LE_ERROR("Error in decoding QMI_SWI_M2M_FDT_IND_V01, rc=%i", rc);
        return;
    }

    LE_DEBUG("Got FDT indication message");
    LE_PRINT_VALUE("%i", fdtInd.ind_context_handle.fdt_context_handle);
    LE_PRINT_VALUE("%i", fdtInd.ind_context_handle.reason);

    FDTIndicationData.closeReason = fdtInd.ind_context_handle.reason;

    switch (FDTIndicationData.closeReason)
    {
        case REASON_NORMAL:
            LE_DEBUG("Close reason normal");
            break;

        case REASON_OMADM_FUMO:
            LE_ERROR("OMA DM/AVMS pre-empts for FUMO/FOTA writing");
            break;

        case REASON_FLASH_WRITE_ERROR:
            LE_ERROR("Flash writing problem");
            break;

        case REASON_INVAID_SIGNATURE:
            LE_ERROR("invalid image signature");
            break;

        case REASON_FLASH_READ_ERROR:
            LE_ERROR("Flash read error");
            break;

        case REASON_NORMAL_SOTA:
            LE_DEBUG("Close normal SOTA");
            break;

        default:
            LE_DEBUG("Close reason invalid");
            break;
    }

    if (Semaphore)
    {
        // Indicate to the Legato thread that the indication was received.
        le_sem_Post(Semaphore);
        LE_DEBUG("After posting semaphore");
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Wait EPOLLIN before read
 *
 * @return
 *      - LE_OK             On success
 *      - LE_TIMEOUT        After DEFAULT_TIMEOUT without data received
 *      - LE_CLOSED         The file descriptor has been closed
 *      - LE_FAULT          On failure
 */
//--------------------------------------------------------------------------------------------------
static le_result_t EpollinRead
(
    int fd,             ///< [IN] file descriptor
    int efd,            ///< [IN] event file descriptor
    void* bufferPtr,    ///< [OUT] pointer where to store data
    ssize_t *lengthPtr  ///< [INOUT] input: max length to read,
                        ///<         output: read length (if LE_OK is returned)
)
{
    int n;
    struct epoll_event events[MAX_EVENTS];

    while (1)
    {
        if (-1 == efd)
        {
            // fd is a regular file, not compliant with epoll, simulate it
            n = 1;
            events[0].events = EPOLLIN;
            events[0].data.fd = fd;
        }
        else
        {
            n = epoll_wait(efd, events, sizeof(events), DEFAULT_TIMEOUT_MS);
            LE_DEBUG("n=%d", n);
        }
        switch (n)
        {
            case -1:
                LE_ERROR("epoll_wait error %m");
                return LE_FAULT;
            case 0:
                LE_DEBUG("Timeout");
                return LE_TIMEOUT;
            default:
                for(;n--;)
                {
                    LE_DEBUG("events[%d] .data.fd=%d .events=0x%x",
                             n, events[n].data.fd, events[n].events);
                    if (events[n].data.fd == fd)
                    {
                        uint32_t evts = events[n].events;

                        if (evts & EPOLLERR)
                        {
                            return LE_FAULT;
                        }
                        else if (evts & EPOLLIN)
                        {
                            do
                            {
                                *lengthPtr = read(fd, bufferPtr, *lengthPtr);
                            }
                            while ((-1 == *lengthPtr) && (EINTR == errno));

                            LE_DEBUG("read %d bytes", *lengthPtr);
                            if (*lengthPtr == 0)
                            {
                                return LE_CLOSED;
                            }
                            return LE_OK;
                        }
                        else if ((evts & EPOLLRDHUP ) || (evts & EPOLLHUP))
                        {
                            // file descriptor has been closed
                            LE_INFO("file descriptor %d has been closed", fd);
                            return LE_CLOSED;
                        }
                        else
                        {
                            LE_WARN("unexpected event received 0x%x",
                                    evts & ~(EPOLLRDHUP|EPOLLHUP|EPOLLERR|EPOLLIN));
                        }
                    }
                }
                break;
        }
    }

    return LE_FAULT;
}


//--------------------------------------------------------------------------------------------------
/**
 * Do synchronous read on a NON-BLOCK file descriptor
 *
 * @return
 *      - LE_OK             On success
 *      - LE_TIMEOUT        After DEFAULT_TIMEOUT without data received
 *      - LE_CLOSED         The file descriptor has been closed
 *      - LE_FAULT          On failure
 */
//--------------------------------------------------------------------------------------------------
static le_result_t ReadSync
(
    int fd,             ///< [IN] file descriptor
    int efd,            ///< [IN] event file descriptor
    void* bufferPtr,    ///< [OUT] pointer where to store data
    ssize_t *lengthPtr  ///< [INOUT] input: max length to read,
                        ///<         output: read length (if LE_OK),
                        ///<                 if -1 then check errno (see read(2))
)
{
    ssize_t size;

    do
    {
        size = read(fd, bufferPtr, *lengthPtr);
    }
    while ((-1 == size) && (EINTR == errno));

    if (((-1 == size) && (EAGAIN == errno)) || (0 == size))
    {
        return EpollinRead(fd, efd, bufferPtr, lengthPtr);
    }

    *lengthPtr = size;
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Configure a file descriptor as NON-BLOCK
 *
 * @return
 *      - LE_OK             On success
 *      - LE_FAULT          On failure
 */
//--------------------------------------------------------------------------------------------------
static le_result_t MakeFdNonBlocking
(
    int fd      ///< [IN] file descriptor
)
{
    int flags;

    flags = fcntl(fd, F_GETFL);
    if (-1 == flags)
    {
        LE_ERROR("Fails to GETFL fd %d: %m", fd);
        return LE_FAULT;
    }
    else
    {
        if (-1 == fcntl(fd, F_SETFL, flags | O_NONBLOCK))
        {
            LE_ERROR("Fails to SETFL fd %d: %m", fd);
            return LE_FAULT;
        }
    }
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Create and configure an event notification
 *
 * @return
 *      - LE_OK             On success
 *      - LE_FAULT          On failure
 */
//--------------------------------------------------------------------------------------------------
static le_result_t CreateAndConfEpoll
(
    int  fd,            ///< [IN] file descriptor
    int* efdPtr         ///< [OUT] event file descriptor
)
{
    struct epoll_event event;
    int efd = epoll_create1(0);
    if (efd == -1)
    {
        return LE_FAULT;
    }

    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLRDHUP | EPOLLET;
    if (-1 == epoll_ctl(efd, EPOLL_CTL_ADD, fd, &event))
    {
        LE_ERROR("epoll_ctl error %m");
        return LE_FAULT;
    }

    *efdPtr = efd;

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Open FDT partition for writing
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
static le_result_t OpenFDT
(
    uint32_t* fdtHandlePtr,    ///< [OUT] Handle for open FDT partition
    bool isRead,               ///< [IN] True if open for read
    size_t* fdtSizePtr         ///< [OUT] Size of file in bytes for read, or
                               ///        size of partition for write.
)
{
    qmi_client_error_type rc;
    SWIQMI_DECLARE_MESSAGE(swi_m2m_fdt_open_req_msg_v01, openReq);
    SWIQMI_DECLARE_MESSAGE(swi_m2m_fdt_open_resp_msg_v01, openResp);

    // Always send TLV, even though default is write if not sent.
    openReq.fdt_access_flag_valid = true;
    if (isRead)
    {
        openReq.fdt_access_flag = true;
    }
    else
    {
        openReq.fdt_access_flag = false;
    }

    rc = qmi_client_send_msg_sync(MgsClient,
                                  QMI_SWI_M2M_FDT_OPEN_REQ_V01,
                                  &openReq, sizeof(openReq),
                                  &openResp, sizeof(openResp),
                                  COMM_LONG_PLATFORM_TIMEOUT);

    if (LE_OK != swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_SWI_M2M_FDT_OPEN_REQ_V01),
                                      rc,
                                      openResp.resp.result,
                                      openResp.resp.error))
    {
        return LE_FAULT;
    }

    *fdtHandlePtr = openResp.fdt_context_handle;
    *fdtSizePtr = openResp.fdt_pkg_size;
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Close FDT partition for writing
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
static le_result_t CloseFDT
(
    uint32_t fdtHandle,             ///< [IN] Handle for open FDT partition
    FDTCloseReason_t closeReason    ///< [IN] Reason for FDT closure
)
{
    qmi_client_error_type rc;
    SWIQMI_DECLARE_MESSAGE(swi_m2m_fdt_close_req_msg_v01, closeReq);
    SWIQMI_DECLARE_MESSAGE(swi_m2m_fdt_close_resp_msg_v01, closeResp);

    closeReq.close_context_handle.fdt_context_handle = fdtHandle;
    // 0 means normal close, anything else means error
    closeReq.close_context_handle.reason = closeReason;

    rc = qmi_client_send_msg_sync(MgsClient,
                                  QMI_SWI_M2M_FDT_CLOSE_REQ_V01,
                                  &closeReq, sizeof(closeReq),
                                  &closeResp, sizeof(closeResp),
                                  COMM_LONG_PLATFORM_TIMEOUT);

    return swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_SWI_M2M_FDT_CLOSE_REQ_V01),
                                rc,
                                closeResp.resp.result,
                                closeResp.resp.error);
}

//--------------------------------------------------------------------------------------------------
/**
 * Write one block to FDT partition
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
static le_result_t WriteFDT
(
    uint32_t fdtHandle,        ///< [IN] Handle for open FDT partition
    uint8_t* bufPtr,           ///< [IN] Buffer to write
    size_t bufSize             ///< [IN] Size of buffer to write
)
{
    qmi_client_error_type rc;
    SWIQMI_DECLARE_MESSAGE(swi_m2m_fdt_write_req_msg_v01, writeReq);
    SWIQMI_DECLARE_MESSAGE(swi_m2m_fdt_write_resp_msg_v01, writeResp);

    writeReq.write_context_handle.fdt_context_handle = fdtHandle;
    writeReq.write_context_handle.binary_data_len = bufSize;
    memcpy(writeReq.write_context_handle.binary_data, bufPtr, bufSize);

    rc = qmi_client_send_msg_sync(MgsClient,
                                  QMI_SWI_M2M_FDT_WRITE_REQ_V01,
                                  &writeReq, sizeof(writeReq),
                                  &writeResp, sizeof(writeResp),
                                  COMM_LONG_PLATFORM_TIMEOUT);

    return swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_SWI_M2M_FDT_WRITE_REQ_V01),
                                rc,
                                writeResp.resp.result,
                                writeResp.resp.error);
}

//--------------------------------------------------------------------------------------------------
/**
 * Read one block from FDT partition
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
static le_result_t ReadFDT
(
    uint32_t fdtHandle,         ///< [IN] Handle for open FDT partition
    uint8_t* bufPtr,            ///< [OUT] Buffer to put block read
    size_t bufSize,             ///< [IN] Size of buffer
    size_t* readBytePtr         ///< [IN] # of bytes actually read
)
{
    qmi_client_error_type rc;
    SWIQMI_DECLARE_MESSAGE(swi_m2m_fdt_read_req_msg_v01, readReq);
    SWIQMI_DECLARE_MESSAGE(swi_m2m_fdt_read_resp_msg_v01, readResp);

    // We want this to fail, if the QMI spec changed the size
    LE_ASSERT(bufSize == sizeof(readResp.read_context_handle.binary_data));

    readReq.fdt_context_handle = fdtHandle;

    rc = qmi_client_send_msg_sync(MgsClient,
                                  QMI_SWI_M2M_FDT_READ_REQ_V01,
                                  &readReq, sizeof(readReq),
                                  &readResp, sizeof(readResp),
                                  COMM_LONG_PLATFORM_TIMEOUT);

    if (LE_OK != swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_SWI_M2M_FDT_READ_REQ_V01),
                                      rc,
                                      readResp.resp.result,
                                      readResp.resp.error))
    {
        return LE_FAULT;
    }

    LE_PRINT_VALUE("%i", readResp.read_context_handle.binary_data_len);
    *readBytePtr = readResp.read_context_handle.binary_data_len;
    memcpy(bufPtr, readResp.read_context_handle.binary_data, *readBytePtr);

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Seek to the specified offset in the open FDT partition
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
static le_result_t SeekFDT
(
    uint32_t fdtHandle,        ///< [IN] Handle for open FDT partition
    uint32_t offset            ///< [IN] Offset to start FDT write for partial download
)
{
    qmi_client_error_type rc;
    SWIQMI_DECLARE_MESSAGE(swi_m2m_fdt_seek_req_msg_v01, seekReq);
    SWIQMI_DECLARE_MESSAGE(swi_m2m_fdt_seek_resp_msg_v01, seekResp);

    seekReq.fdt_seek.fdt_context_handle = fdtHandle;
    seekReq.fdt_seek.fdt_write_offset = offset;

    LE_PRINT_VALUE("%x", offset);

    rc = qmi_client_send_msg_sync(MgsClient,
                                  QMI_SWI_M2M_FDT_SEEK_REQ_V01,
                                  &seekReq, sizeof(seekReq),
                                  &seekResp, sizeof(seekResp),
                                  COMM_LONG_PLATFORM_TIMEOUT);

    return swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_SWI_M2M_FDT_SEEK_REQ_V01),
                                rc,
                                seekResp.resp.result,
                                seekResp.resp.error);
}

//--------------------------------------------------------------------------------------------------
/**
 * Read one block of data from the SWI FOTA partition and write to the given fd.
 *
 * @return
 *      - LE_OK on success
 *      - LE_CLOSED if other side closes the pipe
 *      - LE_FAULT on failure
 *
 * TODO: Should this function take another parameter which indicates how many bytes are left to
 *       be read.  This may be useful when we get to end of the file, if the read actually returns
 *       data past the end of the file.  Need to confirm what FDT_READ will do, before deciding
 *       whether or not this additional parameter is necessary.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t FDTProcessOneRead
(
    uint32_t fdtHandle,     ///< [IN] The SWI FOTA handle for reading
    int writefd,            ///< [IN] The pipe fd for writing
    size_t* numBytesPtr     ///< [OUT] Number of bytes read/written
)
{
    le_result_t result;
    size_t readCount;
    ssize_t writeCount;
    uint8_t buffer[MAX_HEX_DATA_LEN_V01];   // max size supported by QMI FDT_READ message

    LE_INFO("About to read a packet from SWI FOTA");

    // Read a block and write it to the pipe
    result = ReadFDT(fdtHandle, buffer, sizeof(buffer), &readCount);
    if ( result != LE_OK )
    {
        // An error occurred, so close the FDT partition and pipe.
        CloseFDT(fdtHandle, REASON_FLASH_READ_ERROR);
        close(writefd);
        LE_ERROR("Read failed");
        return LE_FAULT;
    }

    LE_PRINT_VALUE("%i", readCount);

    writeCount =  write(writefd, buffer, readCount);
    LE_ERROR_IF( writeCount != readCount, "Did not write all the data");

    if ( writeCount == -1 )
    {
        // Did the other side close the pipe; if so we are done
        if ( errno == EPIPE )
        {
            LE_INFO("Other side of pipe closed");
            CloseFDT(fdtHandle, REASON_NORMAL_SOTA);
            close(writefd);

            // Normal case, but no bytes written
            // TODO: Confirm that this is really a normal case.
            *numBytesPtr = 0;
            return LE_CLOSED;
        }

        // TODO: handle other errors here
        return LE_FAULT;
    }

    // Success, so return number of bytes written.
    *numBytesPtr = writeCount;
    return LE_OK;
}


//--------------------------------------------------------------------------------------------------
/**
 * Read from the SWI FOTA partition and write to the pipe created in pa_fwupdate_Read()
 */
//--------------------------------------------------------------------------------------------------
static void FDTReadHandler
(
    int fd,         ///< The pipe fd for writing
    short events    ///< The event bit map.
)
{

    size_t writeCount;

    // TODO: Need to do this in a better way
    uint32_t fdtHandle = (uint32_t)le_fdMonitor_GetContextPtr();

    // POLLOUT event must be fired to invoke this routine. POLLHUP is mutually exclusive with
    // POLLOUT i.e. this routine should be called when POLLOUT or POLLOUT|POLLERR event fire.
    LE_ASSERT((events == POLLOUT) || (events == (POLLOUT | POLLERR)));

    if ( events & POLLERR )
    {
        // Error occurred in the pipe, so deallocate resources.
        LE_ERROR("Error in pipe");
        // First delete the fdMonitor, then close the fd.
        le_fdMonitor_Ref_t monitorRef = le_fdMonitor_GetMonitor();
        le_fdMonitor_Disable( monitorRef, POLLOUT );
        le_fdMonitor_Delete( monitorRef );
        CloseFDT(fdtHandle, REASON_FLASH_READ_ERROR);
        close(fd);

        return;
    }

    if ( FDTProcessOneRead(fdtHandle, fd, &writeCount) != LE_OK )
    {
        LE_ERROR("Can't process one read");
        return;
    }

    // Update totals
    TotalBytes += writeCount;

    // Once we have read/written enough, then stop the monitor.
    if ( TotalBytes >= PackageSizeBytes )
   {
        LE_DEBUG("Delete fdMonitor and close the file.");

        // First delete the fdMonitor, then close the fd.
        le_fdMonitor_Ref_t monitorRef = le_fdMonitor_GetMonitor();
        le_fdMonitor_Disable( monitorRef, POLLOUT );
        le_fdMonitor_Delete( monitorRef );
        CloseFDT(fdtHandle, REASON_NORMAL_SOTA);
        close(fd);
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Read an integer of the given size and in network byte order from the buffer
 */
//--------------------------------------------------------------------------------------------------
static void ReadUint(uint8_t* dataPtr, uint32_t* valuePtr)
{
    uint32_t networkValue=0;
    uint8_t* networkValuePtr = ((uint8_t*)&networkValue);

    memcpy(networkValuePtr, dataPtr, sizeof(uint32_t));

    *valuePtr = ntohl(networkValue);
}


//==================================================================================================
//  MODULE/COMPONENT FUNCTIONS
//==================================================================================================


//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to initialize the QMI FW UPDATE module.
 */
//--------------------------------------------------------------------------------------------------
COMPONENT_INIT
{
    // Block the sigpipe signal
    le_sig_Block(SIGPIPE);

    // Initialize the required services
    if (LE_OK != swiQmi_InitServices(QMI_SERVICE_DMS))
    {
        LE_ERROR ("Could not initialize QMI Device Management Service.");
        return;
    }

    DmsClient = swiQmi_GetClientHandle(QMI_SERVICE_DMS);
    if (!DmsClient)
    {
        LE_ERROR ("Error opening QMI Device Management Service.");
        return;
    }

    if (LE_OK != swiQmi_InitServices(QMI_SERVICE_MGS))
    {
        LE_ERROR("Could not initialize QMI M2M General Service.");
        return;
    }

    MgsClient = swiQmi_GetClientHandle(QMI_SERVICE_MGS);
    if (!MgsClient)
    {
        // This should not happen, since we just did the init above, but log an error anyways.
        LE_ERROR("Could not get service handle for QMI M2M General Service.");
        return;
    }

    // Register MGS handler for FDT indications
    swiQmi_AddIndicationHandler(FDTIndicationHandler,
                                QMI_SERVICE_MGS,
                                QMI_SWI_M2M_FDT_IND_V01,
                                NULL);

}


//==================================================================================================
//  PUBLIC API FUNCTIONS
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/**
 * Initialize the resume context
 *
 * @return
 *      - LE_OK             on success
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_fwupdate_InitDownload
(
    void
)
{
    // Force resume offset to 0
    ResumeOffset = 0;
    FullImageLength = 0;

    // Reinit fdt handle
    CurrentFdtHandle = 0;

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Prepare the file descriptor to be used for download
 *
 * @return
 *      - LE_OK             On success
 *      - LE_FAULT          On failure
 */
//--------------------------------------------------------------------------------------------------
static le_result_t PrepareFd
(
    int  fd,            ///< [IN] file descriptor
    bool isRegularFile, ///< [IN] flag to indicate if the file descriptor is related to a regular
                        ///<      file or not
    int* efdPtr         ///< [OUT] event file descriptor
)
{
    /* Like we use epoll(2), force the O_NONBLOCK flags in fd */
    if (LE_OK != MakeFdNonBlocking(fd))
    {
        return LE_FAULT;
    }

    if (!isRegularFile)
    {
        if (LE_OK != CreateAndConfEpoll(fd, efdPtr))
        {
            return LE_FAULT;
        }
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Check the file descriptor type
 *
 * @return
 *      - LE_OK             If fd is socket, pipe or regular file
 *      - LE_FAULT          On other file descriptor type
 */
//--------------------------------------------------------------------------------------------------
static le_result_t CheckFdType
(
    int fd,                     ///< [IN] file descriptor to test
    bool *isRegularFilePtr      ///< [OUT] true if fd is a regular file
)
{
    struct stat buf;

    if (-1 == fstat(fd, &buf))
    {
        LE_ERROR("fstat error %m");
        return LE_FAULT;
    }

    switch (buf.st_mode & S_IFMT)
    {
        case 0:       // unknown type
        case S_IFDIR: // directory
        case S_IFLNK: // link
            LE_ERROR("Bad file descriptor type 0x%x", buf.st_mode & S_IFMT);
            return LE_FAULT;

        case S_IFIFO:  // fifo or pipe
        case S_IFSOCK: // socket
            LE_DEBUG("Socket, fifo or pipe");
            *isRegularFilePtr = false;
            break;

        default:
            LE_DEBUG("Regular file");
            *isRegularFilePtr = true;
            break;
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Download the firmware image file to the modem.
 *
 * @return
 *      - LE_OK              On success
 *      - LE_BAD_PARAMETER   If an input parameter is not valid
 *      - LE_TIMEOUT         After 900 seconds without data received
 *      - LE_CLOSED          File descriptor has been closed before all data have been received
 *      - LE_FAULT           On failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_fwupdate_Download
(
    int fd   ///< [IN] File descriptor of the image, opened to the start of the image.
)
{
    le_result_t result;
    le_result_t semResult;
    size_t fdtSize;
    int efd = -1;
    uint8_t* offsetPtr;
    le_fs_FileRef_t fileRef;
    uint32_t imageSize;
    uint32_t cweImageSizeOffset = CWE_IMAGE_SIZE_OFST;

    size_t totalCount = 0;
    ssize_t readCount;
    uint8_t buffer[MAX_HEX_DATA_LEN_V01];   // max size supported by QMI FDT_WRITE message

    if (fd < 0)
    {
        LE_ERROR("Bad file descriptor: %d", fd);
        return LE_BAD_PARAMETER;
    }

    bool isRegularFile;

    LE_DEBUG ("fd %d", fd);
    if ((fd < 0) || (LE_OK != CheckFdType(fd, &isRegularFile)))
    {
        LE_ERROR("Bad file descriptor: %d", fd);
        return LE_BAD_PARAMETER;
    }

    le_clk_Time_t startTime = le_clk_GetAbsoluteTime();

    /* Like we use epoll(2), force the O_NONBLOCK flags in fd */
    result = PrepareFd(fd, isRegularFile, &efd);
    if (result != LE_OK)
    {
        LE_ERROR("Fail to prepare fd: %d", fd);
        // fd passed here should be closed as it is duped and sent here by messaging api.
        close(fd);
        return LE_FAULT;
    }

    // Init the semaphore used between the QMI thread and Legato Thread.
    // It is shared between threads, and has an initial value of zero.
    Semaphore = le_sem_Create("PaFwUpdateSem", 0);

    // Need to reset the indication data each time we try a download.  At this point, before
    // FDT OPEN, this data will not be written by the QMI thread.
    FDTIndicationData.closeReason = REASON_INVALID;

    // First need to get access to the FDT partition
    if (0 == CurrentFdtHandle)
    {
        LE_DEBUG("Open swi-fota");
        result = OpenFDT(&CurrentFdtHandle, false, &fdtSize);
        if (result != LE_OK)
        {
            LE_ERROR("Failed to open swi-fota");
            CurrentFdtHandle = 0;
            result = LE_FAULT;
            goto error;
        }
    }
    else
    {
        totalCount = ResumeOffset;
    }

    // Seek to resume offset
    LE_DEBUG("Seek swi-fota to offset %d", ResumeOffset);
    result = SeekFDT(CurrentFdtHandle, ResumeOffset);
    if (result != LE_OK)
    {
        LE_ERROR("Seek to offset %d failed", ResumeOffset);
        goto error;
    }

    // Read a block at a time from the fd, and send to the modem
    while (true)
    {
        // Check if we received in FDT indication, which means we should stop trying to write
        // to the FDT partition.
        //  - if le_sem_TryWait() returns LE_OK, then we got the indication;
        //  - otherwise if it returns LE_WOULD_BLOCK, then continue reading/write;
        //  - otherwise, this is a serious error, catch by le_sem_TryWait.
        semResult = le_sem_TryWait(Semaphore);
        if (LE_OK == semResult)
        {
            result = LE_FAULT;
            break;
        }

        // We received no FDT indication, so go ahead and read a block and try writing to the modem.
        readCount = sizeof(buffer);
        result = ReadSync(fd, efd, buffer, &readCount);

        if (LE_TIMEOUT == result)
        {
            LE_DEBUG("Download timed out.");
            break;
        }
        else if (LE_FAULT == result)
        {
            LE_DEBUG("Download failed.");
            break;
        }
        else if (LE_CLOSED == result)
        {
            // Read remaining bytes.
            LE_DEBUG("Fd closed; reading remaining bytes");

            while (1)
            {
                // Check if we received in FDT indication, which means we should stop trying
                // to write to the FDT partition.
                //  - if le_sem_TryWait() returns LE_OK, then we got the indication;
                //  - otherwise if it returns LE_WOULD_BLOCK, then continue reading/write;
                //  - otherwise, this is a serious error, catch by le_sem_TryWait.
                semResult = le_sem_TryWait(Semaphore);
                if (LE_OK == semResult)
                {
                    result = LE_FAULT;
                    break;
                }

                // We received no FDT indication, so go ahead and read a block.
                do
                {
                    readCount = read(fd, buffer, sizeof(buffer));
                }
                while ((-1 == readCount) && (EINTR == errno));

                // Write the bytes read to swi-fota partition.
                if (readCount > 0)
                {
                    totalCount += readCount;
                    result = WriteFDT(CurrentFdtHandle, buffer, readCount);
                    if (LE_OK != result)
                    {
                        LE_ERROR("Write to swi-fota failed");
                        result = LE_FAULT;
                        break;
                    }
                }
                else if (readCount < 0)
                {
                    // No need to check EAGAIN as we are reading the remaining data of a closed pipe
                    LE_ERROR("Error while reading fd=%i: %m", fd);
                    result = LE_FAULT;
                    break;
                }
                else
                {
                    LE_DEBUG("All remaining data are now read.");
                    result = LE_CLOSED;
                    break;
                }
            }
            break;
        }
        else
        {
            // Write the bytes read to swi-fota partition.
            if (readCount > 0)
            {
                totalCount += readCount;

                // If this is not a resume operation and image length is not read yet,
                // parse the incoming bytes to get the image length.
                if ((0 == ResumeOffset) && (0 == FullImageLength))
                {
                    // get application image size
                    if (totalCount >= CWE_IMAGE_SIZE_OFST + 4)
                    {
                        offsetPtr = buffer + cweImageSizeOffset;
                        ReadUint(offsetPtr, &imageSize);
                        LE_DEBUG("imageSize %d, 0x%x", imageSize, imageSize);

                        // Full length of the CWE image is provided inside the
                        // first CWE header
                        FullImageLength = imageSize + CWE_HEADER_SIZE;
                        LE_DEBUG("FullImageLength = %u", FullImageLength);

                        // Save full image length in workspace
                        char *pathPtr = FULL_IMAGE_LENGTH_PATH;
                        result = le_fs_Open(pathPtr, LE_FS_WRONLY | LE_FS_CREAT, &fileRef);
                        if (LE_OK != result)
                        {
                            LE_ERROR("failed to open %s: %s", pathPtr, LE_RESULT_TXT(result));
                            result = LE_FAULT;
                            break;
                        }

                        result = le_fs_Write(fileRef,
                                             (uint8_t*)&FullImageLength,
                                             sizeof(FullImageLength));

                        if (LE_OK != result)
                        {
                            LE_ERROR("failed to write %s: %s", pathPtr, LE_RESULT_TXT(result));
                            if (LE_OK != le_fs_Close(fileRef))
                            {
                                LE_ERROR("failed to close %s", pathPtr);
                            }
                            result = LE_FAULT;
                            break;
                        }

                        result = le_fs_Close(fileRef);
                        if (LE_OK != result)
                        {
                            LE_ERROR("failed to close %s: %s", pathPtr, LE_RESULT_TXT(result));
                            result = LE_FAULT;
                            break;
                        }
                    }
                    else
                    {
                        cweImageSizeOffset = CWE_IMAGE_SIZE_OFST - totalCount;
                    }
                }

                result = WriteFDT(CurrentFdtHandle, buffer, readCount);
                if (LE_OK != result)
                {
                    LE_ERROR("Write to swi-fota failed");
                    result = LE_FAULT;
                    break;
                }
            }
            else if (readCount < 0)
            {
                LE_ERROR("Error while reading fd=%i : %m", fd);
                result = LE_FAULT;
                break;
            }
            else
            {
                LE_DEBUG("End of file reached.");
                result = LE_OK;
                break;
            }

            if (totalCount == FullImageLength)
            {
                LE_INFO("Download completed");
                break;
            }
            else if (totalCount > FullImageLength)
            {
                LE_ERROR("Download Failed");
                result = LE_FAULT;
                break;
            }
            else
            {
                //Reset Watchdog if it isn't done for certain time interval
                le_clk_Time_t curTime = le_clk_GetAbsoluteTime();
                le_clk_Time_t diffTime = le_clk_Sub(curTime, startTime);
                if (diffTime.sec >= FWUPDATE_WDOG_KICK_INTERVAL)
                {
                    LE_DEBUG("Kicking watchdog");
                    startTime = curTime;
                    le_wdogChain_Kick(FWUPDATE_WDOG_TIMER);
                }
            }
        }
    }

    LE_DEBUG("%d bytes written to swi-fota partition", totalCount);

    // Close FDT to flush the bytes to disk. We cannot resume once swi-fota is closed.
    // We are working around this issue by closing the partition only when the entire image
    // is written.
    if (totalCount == FullImageLength)
    {
        le_result_t closeResult = CloseFDT(CurrentFdtHandle, REASON_NORMAL_SOTA);

        if (LE_OK == closeResult)
        {
            CurrentFdtHandle = 0;

            // Wait for the close reason indication
            le_sem_Wait(Semaphore);

            // Got the indication data, so make sure the download was successful
            if (REASON_NORMAL_SOTA == FDTIndicationData.closeReason)
            {
                LE_INFO("Download Complete");
            }
            else
            {
                result = LE_FAULT;
            }
        }
        else
        {
            result = LE_FAULT;
        }
    }
    else
    {
        LE_DEBUG("Download suspended: bytes written = %d", totalCount);
    }

error:
    // Done with the file, so close the opened file descriptors.

    // fd passed here should be closed as it is duped and sent here by messaging api.
    close(fd);
    if (efd > -1)
    {
        close(efd);
    }

    // Need to destroy the semaphore, so that we start fresh the next time
    // this function is called.
    le_sem_Delete(Semaphore);
    Semaphore = NULL;

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Return the last stored offset from swifota
 *
 * @return
 *      - LE_OK            on success
 *      - LE_BAD_PARAMETER bad parameter
 *      - LE_FAULT         on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_fwupdate_GetResumePosition
(
    size_t *positionPtr     ///< [OUT] Update package read position
)
{
    qmi_client_error_type rc;
    le_result_t result;
    size_t      fdtSize;
    le_fs_FileRef_t fileRef;
    const char* pathPtr;
    size_t bytesRead;

    SWIQMI_DECLARE_MESSAGE(swi_m2m_fdt_read_download_marker_req_msg_v01, markerReq);
    SWIQMI_DECLARE_MESSAGE(swi_m2m_fdt_read_download_marker_resp_msg_v01, markerResp);

    // Check the parameter
    if (NULL == positionPtr)
    {
        LE_ERROR("Invalid parameter.");
        return LE_BAD_PARAMETER;
    }

    // Initialize offset
    *positionPtr = 0;
    FullImageLength = 0;

    // First need to get access to the FDT partition
    if (0 == CurrentFdtHandle)
    {
        result = OpenFDT(&CurrentFdtHandle, false, &fdtSize);

        if (result != LE_OK)
        {
            LE_ERROR("failed to open swi-fota");
            return LE_FAULT;
        }
    }

    markerReq.fdt_context_handle = CurrentFdtHandle;

    rc = qmi_client_send_msg_sync(MgsClient,
                                  QMI_SWI_M2M_FDT_READ_DOWNLOAD_MARKER_REQ_V01,
                                  &markerReq, sizeof(markerReq),
                                  &markerResp, sizeof(markerResp),
                                  COMM_DEFAULT_PLATFORM_TIMEOUT);

    if(LE_OK != swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_SWI_M2M_FDT_READ_DOWNLOAD_MARKER_REQ_V01),
                                    rc,
                                    markerResp.resp.result,
                                    markerResp.resp.error))
    {
        LE_ERROR("Failed to read swi-fota resume position");
        return LE_FAULT;
    }

    // Read the image length from workspace
    pathPtr = FULL_IMAGE_LENGTH_PATH;

    result = le_fs_Open(pathPtr, LE_FS_RDONLY, &fileRef);
    if (LE_OK != result)
    {
        LE_ERROR("failed to open %s: %s", pathPtr, LE_RESULT_TXT(result));
        return result;
    }

    bytesRead = sizeof(FullImageLength);

    result = le_fs_Read(fileRef, (uint8_t *)&FullImageLength, &bytesRead);
    if (LE_OK != result)
    {
        LE_ERROR("failed to read %s: %s", pathPtr, LE_RESULT_TXT(result));
        if (LE_OK != le_fs_Close(fileRef))
        {
            LE_ERROR("failed to close %s", pathPtr);
        }
        return result;
    }

    result = le_fs_Close(fileRef);
    if (LE_OK != result)
    {
        LE_ERROR("failed to close %s: %s", pathPtr, LE_RESULT_TXT(result));
        return result;
    }

    // Get the download marker. When the CRC is -1, the marker is invalid.
    if (markerResp.fdt_read_download_marker.crc == -1)
    {
        *positionPtr = 0;
    }
    else
    {
        *positionPtr = (size_t)markerResp.fdt_read_download_marker.fdt_write_offset;
    }

    // Set resume position to be used by pa_fwupdate_Download later
    ResumeOffset = *positionPtr;

    LE_INFO("FullImageLength %d, Resume offset %d", FullImageLength, *positionPtr);

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Initialize the QMI Services.
 *
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT for failure
 */
//--------------------------------------------------------------------------------------------------
static le_result_t InitQmiServices
(
    void
)
{
    // Initialize the required service
    if (swiQmi_InitServices(QMI_SERVICE_DMS) != LE_OK)
    {
        LE_ERROR("Could not initialize QMI Device Management Service.\n");
        return LE_FAULT;
    }

    if ( (DmsClient = swiQmi_GetClientHandle(QMI_SERVICE_DMS)) == NULL)
    {
        printf("Error opening QMI Device Management Service.\n");
        return LE_FAULT;
    }

    return LE_OK;
}


//--------------------------------------------------------------------------------------------------
/**
 * Get the device firmware and bootloader version strings.
 *
 * If only one value is desired, then pass NULL as the buffer for the other value.
 *
 * @return
 *      - LE_OK on success
 *      - LE_OVERFLOW if version string to big to fit in provided buffer
 *      - LE_NOT_FOUND if bootloader version is not available; firmware version is always available.
 *      - LE_FAULT for any other errors
 */
//--------------------------------------------------------------------------------------------------
static le_result_t GetDeviceRevision
(
    char* fwVersionPtr,
    size_t fwVersionSize,
    char* blVersionPtr,
    size_t blVersionSize
)
{
    qmi_client_error_type rc;
    le_result_t result;

    if (DmsClient == NULL)
    {
        if (InitQmiServices() == LE_FAULT)
        {
            printf("Failed to initialize QMI Service.\n");
            return LE_FAULT;
        }
    }

    // There is no request message, only a response message.
    SWIQMI_DECLARE_MESSAGE(dms_get_device_rev_id_resp_msg_v01, deviceResp);

    LE_PRINT_VALUE("%p", DmsClient);
    rc = qmi_client_send_msg_sync(DmsClient,
                                  QMI_DMS_GET_DEVICE_REV_ID_REQ_V01,
                                  NULL, 0,
                                  &deviceResp, sizeof(deviceResp),
                                  COMM_DEFAULT_PLATFORM_TIMEOUT);

    result = swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_DMS_GET_DEVICE_REV_ID_REQ_V01),
                                  rc,
                                  deviceResp.resp.result,
                                  deviceResp.resp.error);
    if (LE_OK != result)
    {
        return result;
    }

    if ( fwVersionPtr != NULL )
    {
        result = le_utf8_Copy(fwVersionPtr, deviceResp.device_rev_id, fwVersionSize, NULL);
    }

    // Result could only be LE_OVERFLOW here if returned by le_utf8_Copy() immediately above
    // Just log the error and return right away, without trying to get the other version.
    if ( result == LE_OVERFLOW )
    {
        LE_ERROR("firmware version buffer overflow: '%s'", deviceResp.device_rev_id);
        result = LE_OVERFLOW;
    }
    else
    {
        if ( blVersionPtr != NULL )
        {
            if ( deviceResp.boot_code_rev_valid )
            {
                result = le_utf8_Copy(blVersionPtr, deviceResp.boot_code_rev, blVersionSize, NULL);
                if ( result == LE_OVERFLOW )
                {
                    LE_ERROR("bootloader version buffer overflow: '%s'", deviceResp.boot_code_rev);
                    result = LE_OVERFLOW;
                }
            }
            else
            {
                result = LE_NOT_FOUND;
            }
        }
    }
    return result;
}


//--------------------------------------------------------------------------------------------------
/**
 * Get the firmware version string
 *
 * @return
 *      - LE_OK on success
 *      - LE_NOT_FOUND if the version string is not available
 *      - LE_OVERFLOW if version string to big to fit in provided buffer
 *      - LE_FAULT for any other errors
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_fwupdate_GetFirmwareVersion
(
    char* versionPtr,        ///< [OUT] Firmware version string
    size_t versionSize       ///< [IN] Size of version buffer
)
{
    return GetDeviceRevision(versionPtr, versionSize, NULL, 0);
}


//--------------------------------------------------------------------------------------------------
/**
 * Get the bootloader version string
 *
 * @return
 *      - LE_OK on success
 *      - LE_NOT_FOUND if the version string is not available
 *      - LE_OVERFLOW if version string to big to fit in provided buffer
 *      - LE_FAULT for any other errors
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_fwupdate_GetBootloaderVersion
(
    char* versionPtr,        ///< [OUT] Firmware version string
    size_t versionSize       ///< [IN] Size of version buffer
)
{
    return GetDeviceRevision(NULL, 0, versionPtr, versionSize);
}


//--------------------------------------------------------------------------------------------------
/**
 * Get the app bootloader version string
 *
 * @return
 *      - LE_OK on success
 *      - LE_NOT_FOUND if the version string is not available
 *      - LE_OVERFLOW if version string to big to fit in provided buffer
 *      - LE_BAD_PARAMETER bad parameter
 *      - LE_FAULT for any other errors
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_fwupdate_GetAppBootloaderVersion
(
    char* versionPtr,        ///< [OUT] Version string
    size_t versionSize       ///< [IN] Size of version buffer
)
{
    return (GetAppBootloaderVersion(versionPtr, versionSize));
}


//--------------------------------------------------------------------------------------------------
/**
 * Read the image file from the modem.
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_fwupdate_Read
(
    int* fdPtr         ///< [OUT] File descriptor for the image, ready for reading.
)
{
    le_result_t result;
    uint32_t    fdtHandle;
    size_t      pkgSize;
    int         pipefd[2];

    LE_INFO("Reading from SWI FOTA ...");

    // Open a pipe for sending data back to the caller.
    if ( pipe(pipefd) != 0 )
    {
        LE_FATAL("pipe() failed with errno = %d (%m)", errno);
        return LE_FAULT;
    }
    int outfd = pipefd[1];

    LE_INFO("About to open SWI FOTA");

    result = OpenFDT(&fdtHandle, true, &pkgSize);
    if ( result != LE_OK )
    {
        return result;
    }

    // Keep track of the size of the app package to read/write.
    // TODO: numBytes parameter is now ignored, and should be removed.
    PackageSizeBytes = pkgSize;
    LE_PRINT_VALUE("%zu", pkgSize);
    LE_PRINT_VALUE("%zu", PackageSizeBytes);

    // Go to beginning of file
    SeekFDT(fdtHandle, 0);

    // TODO:
    // There is an issue with UpdateDaemon reading the manifest from the fd before we're ready.
    // The work-around is to read the first block and write to the pipe, and then do any additional
    // reads by using an fdMonitor.

#if 0
    // TODO: The symbols F_GETPIPE_SZ and F_SETPIPE_SZ are not recognised on AR7 builds, although
    //       they are defined in the header files.  Need to sort this out, but in the meantime,
    //       all the documentation I found says that the default buffer size is 64KB, which should
    //       be more than enough for the first block.

    // Ensure that the pipe write buffer is large enough for this first block.
    // Adjust the pipe write buffer size, if necessary.
    int pipeBufSize = fcntl(outfd, F_GETPIPE_SZ);
    LE_PRINT_VALUE("0x%X", pipeBufSize);
    if ( pipeBufSize < MAX_HEX_DATA_LEN_V01 )
    {
        LE_INFO("Trying to set pipe write buffer size to %i", MAX_HEX_DATA_LEN_V01);
        pipeBufSize = fcntl(outfd, F_SETPIPE_SZ, MAX_HEX_DATA_LEN_V01);
        LE_INFO("Set pipe write buffer size to %i", pipeBufSize);
    }
#endif

    size_t writeCount;

    if ( FDTProcessOneRead(fdtHandle, outfd, &writeCount) != LE_OK )
    {
        LE_ERROR("Can't process one read");
        return LE_FAULT;
    }

    // Init totals, since this is the first block read/written
    TotalBytes = writeCount;

    // Are we done?
    if ( TotalBytes >= PackageSizeBytes )
    {
        // Done, so close the write end of the pipe, and the FDT partition.
        CloseFDT(fdtHandle, REASON_NORMAL_SOTA);
        close(outfd);
    }
    else
    {
        // Not done, so set up an fdMonitor to write to the pipe when ready.
        le_fdMonitor_Ref_t monitorRef;
        monitorRef = le_fdMonitor_Create("SWIFOTA Read", outfd, FDTReadHandler, POLLOUT);
        le_fdMonitor_SetContextPtr(monitorRef, (void*)fdtHandle);
    }

    // Return the read end of the pipe
    *fdPtr = pipefd[0];

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Install the package from FOTA partition.
 *
 * @note: isMarkGoodReq is ignored as the synchronization mechanism is not applicable for
 *        single system.
 *
 * @return
 *      - LE_OK            on success
 *      - LE_UNSUPPORTED   the feature is not supported
 *      - LE_FAULT         on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_fwupdate_Install
(
    bool isMarkGoodReq      ///< [IN] Indicate if a mark good operation is required after install
)
{
    le_result_t result;
    size_t pkgSize;

    // Init the semaphore used between the QMI thread and Legato Thread.
    // It is shared between threads, and has an initial value of zero.
    Semaphore = le_sem_Create("PaFwUpdateSem", 0);

    // Need to reset the indication data each time we try a download.  At this point, before
    // FDT OPEN, this data will not be written by the QMI thread.
    FDTIndicationData.closeReason = REASON_INVALID;

    // Do a normal close, and wait for the final FDT indication.
    result = OpenFDT(&CurrentFdtHandle, true, &pkgSize);
    if (LE_OK != result)
    {
        goto error;
    }

    // Close FDT with REASON_NORMAL will trigger a modem reset
    result = CloseFDT(CurrentFdtHandle, REASON_NORMAL);
    if (LE_OK == result)
    {
        // Wait for the close reason indication
        le_sem_Wait(Semaphore);

        // Got the indication data, so make sure the download was successful
        if (REASON_NORMAL == FDTIndicationData.closeReason)
        {
            LE_INFO("Install command successful. System resetting.");
        }
        else
        {
            LE_ERROR("Install command failure %d", FDTIndicationData.closeReason);
            result = LE_FAULT;
        }
    }

error:
    le_sem_Delete(Semaphore);
    Semaphore = NULL;
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the firmware update status.
 *
 * If only one value is desired, then pass NULL as the buffer for the other value.
 *
 * @return
 *      - LE_OK on success
 *      - LE_OVERFLOW if version string to big to fit in provided buffer
 *      - LE_FAULT for any other errors
 */
//--------------------------------------------------------------------------------------------------
static le_result_t GetFwUpdateStatus
(
    uint32_t *updateResultPtr,
    uint8_t *imageTypePtr,
    uint32_t *refDataPtr,
    size_t refStringLength,
    char *refStringPtr
)
{
    qmi_client_error_type rc;
    le_result_t result;

    // There is no request message, only a response message.
    SWIQMI_DECLARE_MESSAGE(dms_swi_get_firmware_status_resp_msg_v01, fwResp);

    rc = qmi_client_send_msg_sync(DmsClient,
                                  QMI_DMS_SWI_GET_FWUPDATE_STATUS_REQ_V01,
                                  NULL, 0,
                                  &fwResp, sizeof(fwResp),
                                  COMM_DEFAULT_PLATFORM_TIMEOUT);

    result = swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_DMS_SWI_GET_FWUPDATE_STATUS_REQ_V01),
                                  rc,
                                  fwResp.resp.result,
                                  fwResp.resp.error);
    if (result != LE_OK)
    {
        return result;
    }

    if (NULL != updateResultPtr)
    {
        if (true == fwResp.FW_update_result_valid)
        {
            *updateResultPtr = fwResp.FW_update_result;
            LE_INFO("FW update result: 0x%x", *updateResultPtr);
        }
        else
        {
            *updateResultPtr = 0;
            LE_INFO("FW update result: None");
        }
    }

    if (NULL != imageTypePtr)
    {
        if (true == fwResp.image_type_valid)
        {
            *imageTypePtr = fwResp.image_type;
            LE_INFO("Image type: 0x%x", *imageTypePtr);
        }
        else
        {
            *imageTypePtr = 0xFF;
            LE_INFO("Image type: None");
        }
    }

    if (NULL != refDataPtr)
    {
        if (true == fwResp.ref_data_valid)
        {
            *refDataPtr = fwResp.ref_data;
            LE_INFO("Reference data: 0x%x", *refDataPtr);
        }
        else
        {
            *refDataPtr = 0xFFFFFFFF;
            LE_INFO("Reference data: None");
        }
    }

    if (NULL != refStringPtr)
    {
        result = le_utf8_Copy(refStringPtr, fwResp.refstring, refStringLength, NULL);
    }

    // Result could only be LE_OVERFLOW here if returned by le_utf8_Copy() immediately above
    if (result == LE_OVERFLOW)
    {
        LE_ERROR("Reference id buffer overflow: %s", fwResp.refstring);
    }
    else
    {
        LE_INFO("Reference id: %s", refStringPtr);
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the string for the fwupdate error code.
 *
 * @return
 *      - Firmware update result string.
 */
//--------------------------------------------------------------------------------------------------
const char *GetStatusLabel
(
    uint32_t fwUpdateResult        ///< [IN] firmware update result code
)
{
    const char* statusLabelPtr;;

    switch (fwUpdateResult)
    {
        case FWUPDATE_STATUS_OK:
            statusLabelPtr = "Success";
            break;
        case FWUPDATE_STATUS_NVUP_ERROR:
            statusLabelPtr = "NVUP partition error";
            break;
        case FWUPDATE_STATUS_FILE_ERROR:
            statusLabelPtr = "FILE Partition error";
            break;
        case FWUPDATE_STATUS_DFLTS_ERROR:
            statusLabelPtr = "Failed to set default configuration";
            break;
        case FWUPDATE_STATUS_UA_ERROR:
            statusLabelPtr = "Update agent failed";
            break;
        case FWUPDATE_STATUS_BL_ERROR:
            statusLabelPtr = "Bootloader error";
            break;
        case FWUPDATE_STATUS_UNKNOWN:
            statusLabelPtr = "Unknown";
            break;
        default:
            statusLabelPtr = "Invalid";
            break;
    }

    return statusLabelPtr;
}


//--------------------------------------------------------------------------------------------------
/**
 * Return the last update status.
 *
 * @return
 *      - LE_OK on success
 *      - LE_UNSUPPORTED   the feature is not supported
 *      - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_fwupdate_GetUpdateStatus
(
    pa_fwupdate_UpdateStatus_t *statusPtr, ///< [OUT] Returned update status
    char *statusLabelPtr,                  ///< [OUT] String matching the status
    size_t statusLabelLength               ///< [IN] Maximum length of the status description
)
{
    uint32_t                       fwUpdateResult;
    uint8_t                        imageType;
    uint32_t                       refData;
    uint32_t                       refStringLength = QMI_DMS_FWUPDATERESTRLEN_V01;
    char                           refString[QMI_DMS_FWUPDATERESTRLEN_V01];
    le_result_t                    result = LE_OK;
    const char                     *resultStringPtr;

    if (LE_OK == GetFwUpdateStatus(
         &fwUpdateResult, &imageType, &refData, refStringLength, refString))
    {

        uint32_t updateResult;

        if ((fwUpdateResult == FWUPDATE_STATUS_OK) || (fwUpdateResult == FWUPDATE_STATUS_UNKNOWN))
        {
            updateResult = fwUpdateResult;
        }
        else
        {
            // Incurred error on last firmware update. Mask out individual error codes and get the
            // error group.
            updateResult = fwUpdateResult & ~FWUPDATE_STATUS_ERROR_MASK;
        }

        switch (updateResult)
        {
            case FWUPDATE_STATUS_OK:
                *statusPtr = PA_FWUPDATE_UPDATE_STATUS_OK;
                break;
            case FWUPDATE_STATUS_NVUP_ERROR:
                // There was an Error when processing update component of type ‘NVUP’.
                *statusPtr = PA_FWUPDATE_UPDATE_STATUS_PARTITION_ERROR;
                break;
            case FWUPDATE_STATUS_FILE_ERROR:
                // There was an Error when processing update component of type ‘FILE’.
                *statusPtr = PA_FWUPDATE_UPDATE_STATUS_PARTITION_ERROR;
                break;
            case FWUPDATE_STATUS_DFLTS_ERROR:
                // There was an error when setting modem to its Default Configuration
                *statusPtr = PA_FWUPDATE_UPDATE_STATUS_UNKNOWN;
                break;
            case FWUPDATE_STATUS_UA_ERROR:
                // Update agent failure
                *statusPtr = PA_FWUPDATE_UPDATE_STATUS_UNKNOWN;
                break;
            case FWUPDATE_STATUS_BL_ERROR:
                // Boot loader failure
                *statusPtr = PA_FWUPDATE_UPDATE_STATUS_UNKNOWN;
                break;
            case FWUPDATE_STATUS_UNKNOWN:
                *statusPtr = PA_FWUPDATE_UPDATE_STATUS_UNKNOWN;
                break;
            default:
                *statusLabelPtr = '\0';
                result = LE_FAULT;
                break;
        }

        // Set status Label
        resultStringPtr = GetStatusLabel(updateResult);

        if (statusLabelLength > strlen(resultStringPtr))
        {
            strcpy(statusLabelPtr, resultStringPtr);
            // Print the complete FW update result, not the masked one.
            LE_INFO("FW update status: %s, FW update result: 0x%x", statusLabelPtr, fwUpdateResult);
        }
        else
        {
            *statusLabelPtr = '\0';
        }
    }
    else
    {
        LE_ERROR("Error reading update status");
        result = LE_FAULT;
    }

    return result;
}


//--------------------------------------------------------------------------------------------------
/**
 * Mark the current system as good.
 *
 * @return
 *      - LE_OK             on success
 *      - LE_UNSUPPORTED    the feature is not supported
 *      - LE_FAULT          on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_fwupdate_MarkGood
(
    void
)
{
    LE_ERROR("Unsupported function called");
    return LE_UNSUPPORTED;
}

//--------------------------------------------------------------------------------------------------
/**
 * Retrieve status of the current system.
 *
 * @return
 *      - LE_OK            on success
 *      - LE_UNSUPPORTED   the feature is not supported
 *      - LE_FAULT         else
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_fwupdate_GetSystemState
(
    bool *isSystemGoodPtr ///< Indicates if the system is marked good
)
{
    LE_ERROR("Unsupported function called");
    return LE_UNSUPPORTED;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function which indicates if a Sync operation is needed (swap & sync operation)
 *
 * @return
 *      - LE_OK            on success
 *      - LE_UNSUPPORTED   the feature is not supported
 *      - LE_FAULT         else
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_fwupdate_DualSysCheckSync
(
    bool *isSyncReq ///< Indicates if synchronization is requested
)
{
    LE_ERROR("Unsupported function called");
    return LE_UNSUPPORTED;
}

//--------------------------------------------------------------------------------------------------
/**
 * This API is to be called to set the SW update state in SSDATA
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_fwupdate_SetState
(
    pa_fwupdate_state_t state   ///< [IN] state to set
)
{
    LE_ERROR("Unsupported function called");
    return LE_UNSUPPORTED;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function which issue a system reset
 */
//--------------------------------------------------------------------------------------------------
void pa_fwupdate_Reset
(
    void
)
{
    LE_ERROR("Unsupported function called");
}

//--------------------------------------------------------------------------------------------------
/**
 * request the modem to apply the NVUP files in UD system
 *
 * @return
 *      - LE_OK             on success
 *      - LE_UNSUPPORTED    the feature is not supported
 *      - LE_FAULT          on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_fwupdate_NvupApply
(
    void
)
{
    LE_ERROR("Unsupported function called");
    return LE_UNSUPPORTED;
}

