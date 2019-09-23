/**
 * @file pa_secStore_qmi.c
 *
 * QMI implementation of @ref c_pa_secStore API.
 *
 * This API allows the caller to store its own data in specific paths.
 *
 * This Platform Adapter implementation of the Secure Storage uses the qualcomm SFS in the modem.
 * Access to the SFS is over QMI.
 *
 * The SFS has a few limitations that affect the design of this implementation:
 *
 *      1) Directories cannot contain both directories and files.
 *      2) Directories cannot listed or iterated over.
 *      3) Directories cannot be deleted.
 *
 * Due to the above limitations we use the following design:
 *
 * The user's path is mapped to a sfs file name.  All files are stored in the same directory.  A
 * meta data file stored in a separate location in sfs is used to maintain the list of sfs files and
 * their corresponding paths.  The sfs file names are generated in such a way as to ensure they are
 * unique.
 *
 * Before performing any access on the sfs, the meta data is imported into our own virtual memory
 * and stored in a hash map.  When an access occurs that causes a change in the meta data the meta
 * data is exported back to the meta data file.
 *
 * The meta data is only ever stored in plaintext in our virtual memory not in persistent memory,
 * which is safer from a security perspective.
 *
 * Some paths, such as /global/avms, are mapped directly to sfs.
 *
 * @section Mapping
 *
 * The translation from PA to QMI SFS is primarily done using a meta file
 * (stored at /sys/meta, on default path_type 0x0) that indexes the content of the /user folder.
 *
 * It looks like this:
 * |  Size      | Top (=PA) layer   | Bottom (=QMI SFS) layer                         |
 * |------------|-------------------|-------------------------------------------------|
 * |  10 bytes  | /my/file          | (path=/user/0, path_type=0x0)                   |
 * | 100 bytes  | /global/test      | (path=/user/1, path_type=0x0)                   |
 * |   0 byte   | /empty/file       | (no actual file, only referenced in /sys/meta)  |
 *
 * It is possible to skip this layer by declaring an entry in DirectAccessPaths, in which case it
 * simply maps using the definition.
 * For instance, if AVMS path_type is available:
 * |  10 bytes  | /global/avms/test | (path=test, path_type=0x2)                      |
 * Otherwise:
 * |  10 bytes  | /global/avms/test | (path=/user/1, path_type=0x0)                   |
 *
 * In case the path goes through direct access, if the file already exists it can be read, updated
 * or deleted.
 * If it doesn't already exist, it will be instead created and later managed as if it was not going
 * through direct access.
 * That way SFS limitations does not apply even for a path that is accessible through direct
 * access.
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#include "legato.h"
#include "pa_secStore.h"
#include "swiQmi.h"
#include "interfaces.h"


//--------------------------------------------------------------------------------------------------
/**
 * The SFS client handle.  Must be obtained by calling swiQmi_GetClientHandle() before it can be
 * used.
 */
//--------------------------------------------------------------------------------------------------
static qmi_client_type SfsClientHandle;


//--------------------------------------------------------------------------------------------------
/**
 * QMI request timeout in milliseconds.
 */
//--------------------------------------------------------------------------------------------------
#define QMI_REQ_TIMEOUT             10000


//--------------------------------------------------------------------------------------------------
/**
 * Root directory of all system files.
 */
//--------------------------------------------------------------------------------------------------
#define SYS_ROOT                    "/sys"


//--------------------------------------------------------------------------------------------------
/**
 * Location of meta data file.  This meta data file is used to store a mapping between sfs file
 * names and user specified paths.
 */
//--------------------------------------------------------------------------------------------------
#define META_FILE                   SYS_ROOT "/meta"


//--------------------------------------------------------------------------------------------------
/**
 * Root directory of all sfs user files.
 */
//--------------------------------------------------------------------------------------------------
#define USER_ROOT                   "/user"


//--------------------------------------------------------------------------------------------------
/**
 * Temporary file, created in our local tmpfs, used to help with importing the sfs meta data file.
 */
//--------------------------------------------------------------------------------------------------
#define TEMP_FILE                   "/tmp/meta"


//--------------------------------------------------------------------------------------------------
/**
 * SFS Path types.
 */
//--------------------------------------------------------------------------------------------------
typedef enum {
    SFS_PATH_TYPE_WITH_BACKUP = 0x0,        ///< Content on this path type is backed-up.
    SFS_PATH_TYPE_NO_BACKUP = 0x1,          ///< Content on this path type is not backed-up.
    SFS_PATH_TYPE_AVMS_CREDENTIALS = 0x2,   ///< Path type points to AVMS credentials.
}
SfsPathType_t;


//--------------------------------------------------------------------------------------------------
/**
 * Default SFS path type.
 */
//--------------------------------------------------------------------------------------------------
#define SFS_PATH_TYPE_DEFAULT SFS_PATH_TYPE_WITH_BACKUP

//--------------------------------------------------------------------------------------------------
/**
 * Memory pool of user specified paths.
 */
//--------------------------------------------------------------------------------------------------
static le_mem_PoolRef_t PathPool = NULL;


//--------------------------------------------------------------------------------------------------
/**
 * Memory pool of sfs file names.
 */
//--------------------------------------------------------------------------------------------------
static le_mem_PoolRef_t SfsNamePool = NULL;


//--------------------------------------------------------------------------------------------------
/**
 * Hash map of meta data.
 */
//--------------------------------------------------------------------------------------------------
static le_hashmap_Ref_t MetaMap = NULL;


//--------------------------------------------------------------------------------------------------
/**
 * Estimated maximum number of files in the sfs.
 */
//--------------------------------------------------------------------------------------------------
#define ESTIMATED_MAX_SFS_FILES             97


//--------------------------------------------------------------------------------------------------
/**
 * Flag to indicate if the sfs is ready for user access.
 */
//--------------------------------------------------------------------------------------------------
static bool SfsReady = false;


//--------------------------------------------------------------------------------------------------
/**
 * Timer for sfs backup.
 */
//--------------------------------------------------------------------------------------------------
static le_timer_Ref_t SfsBackupTimer = NULL;


//--------------------------------------------------------------------------------------------------
/**
 * Expiry interval for the Sfs Backup Timer. The unit is sec.
 */
//--------------------------------------------------------------------------------------------------
#define SFS_BACKUP_INTERVAL 300


//--------------------------------------------------------------------------------------------------
 /**
 *  Define of separator
 */
//--------------------------------------------------------------------------------------------------
#define SEPARATOR "/"


//--------------------------------------------------------------------------------------------------
 /**
 *  Define of wildcard
 */
//--------------------------------------------------------------------------------------------------
#define WILDCARD "*"


//--------------------------------------------------------------------------------------------------
/**
 * Sfs name object.
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    char name[LE_SECSTORE_MAX_NAME_BYTES];  ///< Name of the sfs file.
    bool deleteFlag;                        ///< true if the sfs file needs to be deleted.
}
SfsName_t;

//--------------------------------------------------------------------------------------------------
/**
 * Sfs copy context
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    const char* srcPathPtr;                 ///< Sfs source path.
    const char* destPathPtr;                ///< Sfs destination path.
}
SfsCopyCtxt_t;

//--------------------------------------------------------------------------------------------------
/**
 * Structure to describe a path that is made directly accessible.
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    const char virtualBasePath[SECSTOREADMIN_MAX_PATH_BYTES]; ///< Path made available by the PA IOs
    const char sfsBasePath[SECSTOREADMIN_MAX_PATH_BYTES];     ///< Base path used for SFS operations
    const SfsPathType_t sfsPathType;                          ///< Associated SFS path type
    bool isInUse;                                             ///< Is this direct path enabled?
}
SfsDirectAccessPath_t;

//--------------------------------------------------------------------------------------------------
/**
 * List of paths made directly accessible, without the meta-file mapping.
 */
//--------------------------------------------------------------------------------------------------
static SfsDirectAccessPath_t DirectAccessPaths[] = {
    {
        .virtualBasePath = "/sys/*/apps/avcService/avms/",
        .sfsBasePath = "",
        .sfsPathType = SFS_PATH_TYPE_AVMS_CREDENTIALS,
        .isInUse = false
    },
};

//--------------------------------------------------------------------------------------------------
/**
 * Context used while copying sfs data from one system to other.
 */
//--------------------------------------------------------------------------------------------------
static SfsCopyCtxt_t SfsCopyCtxt;

//--------------------------------------------------------------------------------------------------
/**
 * Gets the next string in the monotonic order.
 *
 * Calling this function multiple times will produce an ordered set of strings in monotonic order.
 * Each string is guaranteed to be unique.
 *
 * If an empty string is given as the currentStr then the first string in this order is returned.
 * Otherwise, only strings generated by this function should be used as the currentStr in this
 * function.
 *
 * The strings generated by this function can be used as identifiers where the meaning of the
 * strings is unimportant but the strings must be unique.  All generated strings are NULL-terminated
 * ASCII strings.
 *
 * @return
 *      LE_OK if successful.
 *      LE_OVERFLOW if the next string will not fit in the buffer.
 *      LE_FAULT if the currentStr and/or buffer is invalid.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t GetMonotonicString
(
    const char* currentStr,                 ///< [IN] The current string.
    char* bufPtr,                           ///< [OUT] Buffer to store the next string.
    size_t bufSize                          ///< [IN] Buffer size.
)
{
    // Check parameters.
    if ( (bufPtr == NULL) || (bufSize < 2) || (currentStr == NULL) )
    {
        return LE_FAULT;
    }

    // Handle the initial string.
    if (currentStr[0] == '\0')
    {
        bufPtr[0] = '0';
        bufPtr[1] = '\0';

        return LE_OK;
    }

    // Copy the current string into the user buffer
    size_t currLen;
    if (le_utf8_Copy(bufPtr, currentStr, bufSize, &currLen) != LE_OK)
    {
        return LE_OVERFLOW;
    }

    // Get the next string.
    size_t i = 0;

    for (i = 0; i < currLen; i++)
    {
        // We only deal with alpha numeric characters.
        if (isalnum(bufPtr[i]) == 0)
        {
            return LE_FAULT;
        }

        if ( ((int)bufPtr[i] < (int)'9') ||
             ( ((int)bufPtr[i] >= (int)'A') && ((int)bufPtr[i] < (int)'Z') ) ||
             ( ((int)bufPtr[i] >= (int)'a') && ((int)bufPtr[i] < (int)'z') ) )
        {
            // Increment the current character.
            bufPtr[i] = (int)bufPtr[i] + 1;
            return LE_OK;
        }

        if ((int)bufPtr[i] == (int)'9')
        {
            // Increment the current character.
            bufPtr[i] = 'A';
            return LE_OK;
        }

        if ((int)bufPtr[i] == (int)'Z')
        {
            // Increment the current character.
            bufPtr[i] = 'a';
            return LE_OK;
        }

        // The current character must be 'z'.  Reset the character to '0' and go on to the next char.
        bufPtr[i] = '0';
    }

    // We've reached the end of the current string.  Try to add another character.
    if (bufSize > currLen + 1)
    {
        bufPtr[currLen] = '0';
        bufPtr[currLen + 1] = '\0';

        return LE_OK;
    }

    return LE_OVERFLOW;
}

//--------------------------------------------------------------------------------------------------
/**
 * Open an sfs file.
 *
 * @return
 *      LE_OK if successful.
 *      LE_NOT_FOUND if the item does not exist.
 *      LE_UNAVAILABLE if the sfs is currently unavailable.
 *      LE_FAULT if there was some other error.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t OpenSfs
(
    const char* sfsPathPtr,             ///< [IN] Sfs file path.
    int32_t sfsPathType,                ///< [IN] Path type associated with the given path.
    int32_t acceMode,                   ///< [IN] Access mode for the file.
    int32_t* fileHandlePtr              ///< [OUT] Handle to the sfs file if successful.
)
{
    SWIQMI_DECLARE_MESSAGE(swi_sfs_open_req_msg_v01, sfsReq);
    SWIQMI_DECLARE_MESSAGE(swi_sfs_open_resp_msg_v01, sfsResp);

    LE_DEBUG("Open path[%s] path_type[%u] mode[%d]", sfsPathPtr, sfsPathType, acceMode);

    sfsReq.path_type = (uint32_t)sfsPathType;

    // Fill in the request message.
    if (le_utf8_Copy(sfsReq.path, sfsPathPtr, sizeof(sfsReq.path), NULL) == LE_OVERFLOW)
    {
        return LE_NOT_FOUND;
    }

    sfsReq.mode = acceMode;

    // Send the message and handle the response
    qmi_client_error_type rc = qmi_client_send_msg_sync(SfsClientHandle,
                                                        QMI_SWI_SFS_OPEN_REQ_V01,
                                                        &sfsReq, sizeof(sfsReq),
                                                        &sfsResp, sizeof(sfsResp),
                                                        QMI_REQ_TIMEOUT);

    le_result_t result = swiQmi_OEMCheckResponseCode(STRINGIZE_EXPAND(QMI_SWI_SFS_OPEN_REQ_V01),
                                                     rc,
                                                     sfsResp.resp);

    if (LE_OK != result)
    {
        if (sfsResp.resp.error == QMI_ERR_INVALID_ARG_V01)
        {
            return LE_NOT_FOUND;
        }

        LE_ERROR("Unexpected QMI response %d (%s)", result, LE_RESULT_TXT(result));
        return LE_FAULT;
    }

    if (sfsResp.fp == 0)
    {
        // TODO: Translate error codes to return value.
        // NOTE: Return LE_NOT_FOUND here because that is the most likely error.
        return LE_NOT_FOUND;
    }

    *fileHandlePtr = sfsResp.fp;
    return LE_OK;
}


//--------------------------------------------------------------------------------------------------
/**
 * Close an sfs file.
 */
//--------------------------------------------------------------------------------------------------
static void CloseSfsFile
(
    int32_t fileHandle                      ///< [IN] Handle to the sfs file.
)
{
    SWIQMI_DECLARE_MESSAGE(swi_sfs_close_req_msg_v01, sfsReq);
    SWIQMI_DECLARE_MESSAGE(swi_sfs_close_resp_msg_v01, sfsResp);

    // Fill in the request message.
    sfsReq.fp = fileHandle;

    // Send the message and handle the response
    qmi_client_error_type rc = qmi_client_send_msg_sync(SfsClientHandle,
                                                        QMI_SWI_SFS_CLOSE_REQ_V01,
                                                        &sfsReq, sizeof(sfsReq),
                                                        &sfsResp, sizeof(sfsResp),
                                                        QMI_REQ_TIMEOUT);

    // Can't do anything about the errors here so we don't check the return code.
    le_result_t result = swiQmi_OEMCheckResponseCode(STRINGIZE_EXPAND(QMI_SWI_SFS_CLOSE_REQ_V01),
                                                     rc,
                                                     sfsResp.resp);

    if (LE_OK != result)
    {
        LE_ERROR("Unexpected QMI response %d (%s)", result, LE_RESULT_TXT(result));
    }
}


//--------------------------------------------------------------------------------------------------
/**
 * Unbuffered-write an sfs file.
 *
 * @return
 *      LE_OK if successful.
 *      LE_UNAVAILABLE if the sfs is currently unavailable.
 *      LE_NO_MEMORY if the sfs is out of space.
 *      LE_FAULT if there was some other error.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t WriteSfs
(
    int32_t fileHandle,                     ///< [IN] Handle to the sfs file.
    const uint8_t* bufPtr,                  ///< [IN] Buffer to write to the sfs file.
    size_t bufSize                          ///< [IN] Size of the buffer.
)
{
    SWIQMI_DECLARE_MESSAGE(swi_sfs_write_req_msg_v01, sfsReq);
    SWIQMI_DECLARE_MESSAGE(swi_sfs_write_resp_msg_v01, sfsResp);

    // Fill in the request message.
    sfsReq.fp = fileHandle;

    // Write the data a chunk at a time to the sfs.
    size_t bytesWritten = 0;

    do
    {
        // Calculate the number of bytes to write.
        sfsReq.data_len = bufSize - bytesWritten;

        if (sfsReq.data_len > MAX_DATA_SIZE_V01)
        {
            sfsReq.data_len = MAX_DATA_SIZE_V01;
        }

        // Copy the data to the qmi buffer.
        memcpy(sfsReq.data, &(bufPtr[bytesWritten]), sfsReq.data_len);

        // Send the message and handle the response
        qmi_client_error_type rc = qmi_client_send_msg_sync(SfsClientHandle,
                                                            QMI_SWI_SFS_WRITE_REQ_V01,
                                                            &sfsReq, sizeof(sfsReq),
                                                            &sfsResp, sizeof(sfsResp),
                                                            QMI_REQ_TIMEOUT);

        le_result_t result = swiQmi_OEMCheckResponseCode(STRINGIZE_EXPAND(QMI_SWI_SFS_WRITE_REQ_V01),
                                                         rc,
                                                         sfsResp.resp);

        if (LE_OK != result)
        {
            LE_ERROR("Unexpected QMI response %d (%s)", result, LE_RESULT_TXT(result));
            return LE_FAULT;
        }

        if (-1 == sfsResp.bytes_written)
        {
            return LE_FAULT;
        }

        if (0 == sfsResp.bytes_written)
        {
            // No byte were written. So we assume that the EFS is full
            LE_ERROR("No byte written. Only %u bytes written. The EFS may be full", bytesWritten);
            return LE_NO_MEMORY;
        }

        // Update the number of bytes written.
        bytesWritten += sfsResp.bytes_written;
    }
    while (bytesWritten < bufSize);

    return LE_OK;
}


//--------------------------------------------------------------------------------------------------
/**
 * Buffered-write to an sfs file. If the data to be written to the sfs causes the write buffer to
 * overflow, then data are written to the sfs until the write buffer is no longer full. The
 * remaining data in the write buffer can be flushed out to the sfs by specifying bufSize of 0.
 *
 * @return
 *      LE_OK if successful.
 *      LE_UNAVAILABLE if the sfs is currently unavailable.
 *      LE_FAULT if there was some other error.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t WriteSfsBuffered
(
    int32_t fileHandle,                     ///< [IN] Handle to the sfs file.
    const uint8_t* bufPtr,                  ///< [IN] Buffer to write to the sfs file.
    size_t bufSize                          ///< [IN] Size of the buffer.
)
{
    le_result_t result = LE_FAULT;
    static uint8_t writeBuffer[MAX_DATA_SIZE_V01] = {0};
    static size_t writeBufferIdx = 0;
    // index for the incoming buffer (bufPtr)
    size_t bufPtrIdx = 0;

    // remaining space on the writeBuffer
    size_t remainingSpace = MAX_DATA_SIZE_V01 - writeBufferIdx;

    // flush out the write buffer if the incoming buffer has size 0.
    if (bufSize == 0)
    {
        // if writeBuffer is empty, we must not send the write request otherwise it's regarded
        // as an error.
        if (writeBufferIdx == 0)
        {
            return LE_OK;
        }

        result = WriteSfs(fileHandle, writeBuffer, writeBufferIdx);

        if (result != LE_OK)
        {
            return result;
        }

        // reset writeBuffer
        writeBufferIdx = 0;
        memset(writeBuffer, 0, MAX_DATA_SIZE_V01);

        return LE_OK;
    }

    while (bufPtrIdx < bufSize)
    {
        size_t copysize = bufSize - bufPtrIdx;

        // Write to sfs right away if the writeBuffer is filled up.
        if (copysize >= remainingSpace)
        {
            // Copy from the incoming buffer to the writeBuffer, as much as we can.
            memcpy(writeBuffer + writeBufferIdx, bufPtr + bufPtrIdx, remainingSpace);

            // Update index of the incoming buffer.
            bufPtrIdx += remainingSpace;

            // Write the data a chunk at a time to the sfs.
            result = WriteSfs(fileHandle, writeBuffer, MAX_DATA_SIZE_V01);

            if (result != LE_OK)
            {
                return result;
            }

            // reset writeBuffer
            writeBufferIdx = 0;
            memset(writeBuffer, 0, MAX_DATA_SIZE_V01);

            // update remainingSpace
            remainingSpace = MAX_DATA_SIZE_V01;
        }
        else
        {
            // stuff the writeBuffer and return
            memcpy(writeBuffer + writeBufferIdx, bufPtr + bufPtrIdx, copysize);
            writeBufferIdx += copysize;

            return LE_OK;
        }
    }

    // we arrive here when the last attempted write to sfs exactly fits the remaining space in the
    // writeBuffer.
    return LE_OK;
}


//--------------------------------------------------------------------------------------------------
/**
 * Reads a buffer from an sfs file.
 *
 * @return
 *      LE_OK if successful.
 *      LE_OVERFLOW if the user buffer is too small to hold the contents of the sfs file.
 *      LE_UNAVAILABLE if the sfs is currently unavailable.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t ReadSfs
(
    int32_t fileHandle,                     ///< [IN] Handle to the sfs file.
    uint8_t* bufPtr,                        ///< [OUT] Buffer to store the sfs file data.
    size_t* bufSizePtr,                     ///< [IN/OUT] Number of bytes to read when this function
                                            ///           is called.  The number of bytes read when
                                            ///           this function returns.
    bool checkOverflow                      ///  [IN] Is overflow checking required?
)
{
    SWIQMI_DECLARE_MESSAGE(swi_sfs_read_req_msg_v01, sfsReq);
    SWIQMI_DECLARE_MESSAGE(swi_sfs_read_resp_msg_v01, sfsResp);

    // Fill in the request message.
    sfsReq.fp = fileHandle;

    // Read the file in a loop until we've read the entire file or we run out of buffer space.
    size_t bytesRead = 0;

    while (1)
    {
        // Calculate the number of bytes to read.
        sfsReq.data_len = *bufSizePtr - bytesRead;

        if (sfsReq.data_len > MAX_DATA_SIZE_V01)
        {
            sfsReq.data_len = MAX_DATA_SIZE_V01;
        }
        else if (sfsReq.data_len == 0)
        {
            if (checkOverflow)
            {
                // The user buffer is filled but we still need to check if there is an overflow.
                // But the sfs cannot read 0 bytes so we must read at least one byte.
                sfsReq.data_len = 1;
            }
            else
            {
                // The user buffer is filled and we don't need to check overflow.
                return LE_OK;
            }
        }

        // Send the message and handle the response
        qmi_client_error_type rc = qmi_client_send_msg_sync(SfsClientHandle,
                                                            QMI_SWI_SFS_READ_REQ_V01,
                                                            &sfsReq, sizeof(sfsReq),
                                                            &sfsResp, sizeof(sfsResp),
                                                            QMI_REQ_TIMEOUT);

        le_result_t result = swiQmi_OEMCheckResponseCode(STRINGIZE_EXPAND(QMI_SWI_SFS_READ_REQ_V01),
                                                         rc,
                                                         sfsResp.resp);
        if (LE_OK != result)
        {
            LE_ERROR("Unexpected QMI response %d (%s)", result, LE_RESULT_TXT(result));
            return LE_FAULT;
        }

        // Check if we've reached the end of the file.
        if (sfsResp.data_len == 0)
        {
            *bufSizePtr = bytesRead;
            return LE_OK;
        }

        if (bytesRead >= *bufSizePtr)
        {
            // The user buffer is already full but there is still data in the sfs.
            return LE_OVERFLOW;
        }

        // Copy the read value to the caller's buffer.
        memcpy(&(bufPtr[bytesRead]), sfsResp.data, sfsResp.data_len);

        bytesRead += sfsResp.data_len;
    }
}


//--------------------------------------------------------------------------------------------------
/**
 * Delete an sfs file.
 *
 * @return
 *      LE_OK if successful.
 *      LE_UNAVAILABLE if the sfs is currently unavailable.
 *      LE_FAULT if there was some other error.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t DeleteSfsFile
(
    const char* sfsPathPtr,                 ///< [IN] Sfs file path.
    SfsPathType_t sfsPathType               ///< [IN] Sfs path type.
)
{
    SWIQMI_DECLARE_MESSAGE(swi_sfs_rm_req_msg_v01, sfsReq);
    SWIQMI_DECLARE_MESSAGE(swi_sfs_rm_resp_msg_v01, sfsResp);

    LE_DEBUG("Delete path[%s] path_type[%u]", sfsPathPtr, sfsPathType);

    sfsReq.path_type = (uint32_t)sfsPathType;

    // Fill in the request message.
    if (le_utf8_Copy(sfsReq.path, sfsPathPtr, sizeof(sfsReq.path), NULL) == LE_OVERFLOW)
    {
        return LE_NOT_FOUND;
    }

    // Send the message and handle the response
    qmi_client_error_type rc = qmi_client_send_msg_sync(SfsClientHandle,
                                                        QMI_SWI_SFS_RM_REQ_V01,
                                                        &sfsReq, sizeof(sfsReq),
                                                        &sfsResp, sizeof(sfsResp),
                                                        QMI_REQ_TIMEOUT);

    le_result_t result = swiQmi_OEMCheckResponseCode(STRINGIZE_EXPAND(QMI_SWI_SFS_RM_REQ_V01),
                                                     rc,
                                                     sfsResp.resp);
    if (LE_OK != result)
    {
        LE_ERROR("Unexpected QMI response %d (%s)", result, LE_RESULT_TXT(result));
        return LE_FAULT;
    }

    if (sfsResp.result != 0)
    {
        // TODO: Translate error codes to return value.
        LE_ERROR("Could not delete '%s'.  Result code %d.", sfsPathPtr, sfsResp.result);

        return LE_FAULT;
    }

    return LE_OK;
}


//--------------------------------------------------------------------------------------------------
/**
 * Gets the size, in bytes, of an sfs file.
 *
 * @return
 *      LE_OK if successful.
 *      LE_UNAVAILABLE if the sfs is currently unavailable.
 *      LE_FAULT if there was some other error.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t GetSfsSize
(
    int32_t fileHandle,                 ///< [IN] Handle to the sfs file.
    size_t* size                        ///< [OUT] The size of the file.
)
{
    SWIQMI_DECLARE_MESSAGE(swi_sfs_getsize_req_msg_v01, sfsReq);
    SWIQMI_DECLARE_MESSAGE(swi_sfs_getsize_resp_msg_v01, sfsResp);

    // Fill in the request message.
    sfsReq.fp = fileHandle;

    // Send the message and handle the response
    qmi_client_error_type rc = qmi_client_send_msg_sync(SfsClientHandle,
                                                        QMI_SWI_SFS_GETSIZE_REQ_V01,
                                                        &sfsReq, sizeof(sfsReq),
                                                        &sfsResp, sizeof(sfsResp),
                                                        QMI_REQ_TIMEOUT);

    le_result_t result = swiQmi_OEMCheckResponseCode(STRINGIZE_EXPAND(QMI_SWI_SFS_GETSIZE_REQ_V01),
                                                     rc,
                                                     sfsResp.resp);

    if (LE_OK != result)
    {
        LE_ERROR("Unexpected QMI response %d (%s)", result, LE_RESULT_TXT(result));
        return LE_FAULT;
    }

    if (sfsResp.result != 0)
    {
        // TODO: Translate error codes to return value.
        LE_ERROR("Could not get size of file handle %d.  Result code %d.",
                 fileHandle, sfsResp.result);

        return LE_FAULT;
    }

    *size = sfsResp.size;

    return LE_OK;
}


//--------------------------------------------------------------------------------------------------
/**
 * Creates a directory in the sfs.
 *
 * @return
 *      LE_OK if successful.
 *      LE_UNAVAILABLE if the sfs is currently unavailable.
 *      LE_FAULT if there was some other error.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t MkSfsDir
(
    const char* sfsPathPtr,                 ///< [IN] Sfs directory path.
    SfsPathType_t sfsPathType               ///< [IN] Sfs path type.

)
{
    SWIQMI_DECLARE_MESSAGE(swi_sfs_mkdir_req_msg_v01, sfsReq);
    SWIQMI_DECLARE_MESSAGE(swi_sfs_mkdir_resp_msg_v01, sfsResp);

    LE_DEBUG("MkDir path[%s] path_type[%u]", sfsPathPtr, sfsPathType);

    sfsReq.path_type = (uint32_t)sfsPathType;

    // Fill in the request message.
    if (le_utf8_Copy(sfsReq.path, sfsPathPtr, sizeof(sfsReq.path), NULL) == LE_OVERFLOW)
    {
        return LE_FAULT;
    }

    // Send the message and handle the response
    qmi_client_error_type rc = qmi_client_send_msg_sync(SfsClientHandle,
                                                        QMI_SWI_SFS_MKDIR_REQ_V01,
                                                        &sfsReq, sizeof(sfsReq),
                                                        &sfsResp, sizeof(sfsResp),
                                                        QMI_REQ_TIMEOUT);

    le_result_t result = swiQmi_OEMCheckResponseCode(STRINGIZE_EXPAND(QMI_SWI_SFS_MKDIR_REQ_V01),
                                                     rc,
                                                     sfsResp.resp);
    if (LE_OK != result)
    {
        LE_ERROR("Unexpected QMI response %d (%s)", result, LE_RESULT_TXT(result));
        return LE_FAULT;
    }

    if (sfsResp.result != 0)
    {
        // TODO: Translate error codes to return value.
        LE_ERROR("Could not make directory '%s'.  Result code %d.", sfsPathPtr, sfsResp.result);
        return LE_FAULT;
    }

    return LE_OK;
}


//--------------------------------------------------------------------------------------------------
/**
 * Closes a stream.
 *
 * @return
 *      LE_OK if successful.
 *      LE_NOT_FOUND if the sfs file does not exist.
 *      LE_UNAVAILABLE if the sfs is currently unavailable.
 *      LE_FAULT if there was some other error.
 */
//--------------------------------------------------------------------------------------------------
static void CloseStream
(
    FILE* streamPtr,            ///< [IN] Stream to close.
    const char* streamNamePtr   ///< [IN] Name of the stream.
)
{
    int r;
    do
    {
        r = fclose(streamPtr);
    }
    while ( (r != 0) && (errno == EINTR) );

    LE_ERROR_IF(r != 0, "Could not close %s.  %m.", streamNamePtr);
}

//--------------------------------------------------------------------------------------------------
/**
 * Copies an sfs file to a temp file.
 *
 * @warning
 *      This API is limited to SFS_PATH_TYPE_WITH_BACKUP.
 *
 * @return
 *      LE_OK if successful.
 *      LE_NOT_FOUND if the sfs file does not exist.
 *      LE_UNAVAILABLE if the sfs is currently unavailable.
 *      LE_FAULT if there was some other error.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t CopySfsFileToTemp
(
    const char* sfsPathPtr,         ///< [IN] Path to the sfs file.
    const char* tmpPathPtr          ///< [IN] Path to the temp file.
)
{
    // Open the sfs file.
    int32_t sfsFile;

    le_result_t result = OpenSfs(sfsPathPtr, SFS_PATH_TYPE_WITH_BACKUP, SFS_READONLY_V01, &sfsFile);

    if (result != LE_OK)
    {
        return result;
    }

    // Create the temp file.
    FILE* tmpFilePtr;

    do
    {
        tmpFilePtr = fopen(tmpPathPtr, "w");
    }
    while ( (tmpFilePtr == NULL) && (errno == EINTR) );

    if (tmpFilePtr == NULL)
    {
        LE_ERROR("Could not create temp file %s.  %m.", tmpPathPtr);
        CloseSfsFile(sfsFile);
        return LE_FAULT;
    }

    // Copy the contents of the sfs file to the temp file.
    while (1)
    {
        // Read a buffer of data from the sfs file.
        char buf[MAX_DATA_SIZE_V01];
        size_t numBytes = MAX_DATA_SIZE_V01;

        // Read MAX_DATA_SIZE_V01 chunks until there is no more bytes to read.
        result = ReadSfs(sfsFile, (uint8_t*)buf, &numBytes, false);

        if ( (result == LE_FAULT) || (result == LE_UNAVAILABLE) )
        {
            LE_ERROR("Could not read meta data file.");
            goto out;
        }

        if (numBytes == 0)
        {
            // Nothing more to read.
            break;
        }

        // Write the buffer to the temp file.
        if (fwrite(buf, 1, numBytes, tmpFilePtr) != numBytes)
        {
            LE_ERROR("Could not write to temp file %s.", tmpPathPtr);
            result = LE_FAULT;
            goto out;
        }
    }

    result = LE_OK;

out:
    CloseSfsFile(sfsFile);
    CloseStream(tmpFilePtr, tmpPathPtr);
    return result;
};


//--------------------------------------------------------------------------------------------------
/**
 * Reads a line of text from the stream up to the first newline or eof.  The output buffer will
 * always be NULL-terminated and will not include the newline.
 *
 * @return
 *      LE_OK if successful.
 *      LE_OVERFLOW if the buffer is too small.  As much of the line as possible will be copied to
 *                  buf.
 *      LE_OUT_OF_RANGE if there is nothing else to read from the file.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t ReadLine
(
    FILE* streamPtr,            ///< [IN] Stream to read from.
    char* bufPtr,               ///< [OUT] Buffer to store the line in.
    size_t bufSize              ///< [IN] Buffer size.
)
{
    char localBuf[bufSize + 1];

    if (fgets(localBuf, sizeof(localBuf), streamPtr) == NULL)
    {
        return LE_OUT_OF_RANGE;
    }

    // Remove the trailing newline char.
    size_t len = strlen(localBuf);

    if (localBuf[len - 1] == '\n')
    {
        localBuf[len - 1] = '\0';
    }

    // Copy the buffer into the user buffer.
    return le_utf8_Copy(bufPtr, localBuf, bufSize, NULL);
}


//--------------------------------------------------------------------------------------------------
/**
 * Imports meta data from a file into a hash map.
 *
 * @return
 *      LE_OK if successful.
 *      LE_UNAVAILABLE if the sfs is currently unavailable.
 *      LE_FAULT if there was some other error.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t ImportMeta
(
    le_hashmap_Ref_t map,           ///< [IN] The hash map to import the meta data to.
    const char* metaFilePtr         ///< [IN] Path to the sfs meta data file.
)
{
    // Copy the meta data file into a temp file in RAM.  This will make it easier to read and parse.
    le_result_t result = CopySfsFileToTemp(metaFilePtr, TEMP_FILE);

    if (result == LE_NOT_FOUND)
    {
        // The meta data file does not exist.  This is the first time the sfs is used.
        return LE_OK;
    }

    if (result != LE_OK)
    {
        return result;
    }

    // Read the temp file to get the meta data.
    FILE* tmpFilePtr;

    do
    {
        tmpFilePtr = fopen(TEMP_FILE, "r");
    }
    while ( (tmpFilePtr == NULL) && (errno == EINTR) );

    if (tmpFilePtr == NULL)
    {
        LE_ERROR("Could not open temp file %s.  %m.", TEMP_FILE);

        // Attempt to delete the temp file.
        LE_ERROR_IF(unlink(TEMP_FILE) != 0, "Could not delete %s.  %m.", TEMP_FILE);

        return LE_FAULT;
    }

    // Read the path and sfs names from the meta file.
    while (1)
    {
        // Read the path.
        char* pathPtr = le_mem_ForceAlloc(PathPool);

        result = ReadLine(tmpFilePtr, pathPtr, SECSTOREADMIN_MAX_PATH_BYTES);

        if (result == LE_OUT_OF_RANGE)
        {
            // Nothing more to read.
            le_mem_Release(pathPtr);
            break;
        }

        if (result == LE_OVERFLOW)
        {
            LE_CRIT("Item path '%s...' is too long.", pathPtr);
            le_mem_Release(pathPtr);
            goto handleMetaCorruption;
        }

        // Read the sfs name.
        SfsName_t* sfsPtr = le_mem_ForceAlloc(SfsNamePool);
        sfsPtr->deleteFlag = false;

        result = ReadLine(tmpFilePtr, sfsPtr->name, LE_SECSTORE_MAX_NAME_BYTES);

        if (result != LE_OK)
        {
            if (result == LE_OVERFLOW)
            {
                LE_CRIT("sfs name '%s...' is too long.", sfsPtr->name);
            }
            else
            {
                LE_CRIT("Unexpected end of meta file.");
            }

            le_mem_Release(pathPtr);
            le_mem_Release(sfsPtr);

            goto handleMetaCorruption;
        }

        // Store the path and sfs name in our hash map.
        if (le_hashmap_Put(map, pathPtr, sfsPtr) != NULL)
        {
            LE_CRIT("Duplicate paths in meta file.");

            le_mem_Release(pathPtr);
            le_mem_Release(sfsPtr);

            goto handleMetaCorruption;
        }
    }


    CloseStream(tmpFilePtr, TEMP_FILE);

    // Delete the temp file.
    LE_ERROR_IF(unlink(TEMP_FILE) != 0, "Could not delete %s.  %m.", TEMP_FILE);

    return LE_OK;

handleMetaCorruption:
    // Close and delete the temp file.
    CloseStream(tmpFilePtr, TEMP_FILE);
    LE_ERROR_IF(unlink(TEMP_FILE) != 0, "Could not delete %s.  %m.", TEMP_FILE);

    // Delete the meta data in the sfs.
    LE_CRIT("Deleting SFS meta file to try to recover the sfs.");
    LE_CRIT_IF(DeleteSfsFile(metaFilePtr, SFS_PATH_TYPE_WITH_BACKUP) != LE_OK,
               "Could not delete sfs meta data.");

    return LE_FAULT;
};


//--------------------------------------------------------------------------------------------------
/**
 * Start the sfs backup timer, if it's not already running.
 */
//--------------------------------------------------------------------------------------------------
static void StartSfsBackupTimer
(
    void
)
{
    if (!le_timer_IsRunning(SfsBackupTimer))
    {
        le_timer_Start(SfsBackupTimer);
    }
}


//--------------------------------------------------------------------------------------------------
/**
 * Exports the meta data in the hash map to the meta data file.
 *
 * @return
 *      LE_OK if successful.
 *      LE_UNAVAILABLE if the sfs is currently unavailable.
 *      LE_FAULT if there was some other error.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t ExportMeta
(
    le_hashmap_Ref_t map,           ///< [IN] The hash map containing the meta data.
    const char* metaFilePtr         ///< [IN] Path to the sfs file to store the meta data.
)
{
    // Open the sfs file.
    int32_t sfsFile;

    le_result_t result = OpenSfs(metaFilePtr, SFS_PATH_TYPE_WITH_BACKUP, SFS_CREATE_V01, &sfsFile);

    LE_FATAL_IF(result == LE_NOT_FOUND, "Could not create meta data file %s.", metaFilePtr);

    if (result != LE_OK)
    {
        return result;
    }

    // Iterate over the hash map and write the path and sfs names into the file.
    le_hashmap_It_Ref_t iter = le_hashmap_GetIterator(map);

    while(le_hashmap_NextNode(iter) == LE_OK)
    {
        const char* pathPtr = le_hashmap_GetKey(iter);
        LE_ASSERT(NULL != pathPtr);

        const SfsName_t* sfsNamePtr = le_hashmap_GetValue(iter);

        if (!(sfsNamePtr->deleteFlag))
        {
            // Write the path to the file.
            result = WriteSfsBuffered(sfsFile, (uint8_t*)pathPtr, strlen(pathPtr));

            if (result != LE_OK)
            {
                return result;
            }

            result = WriteSfsBuffered(sfsFile, (uint8_t*)"\n", 1);

            if (result != LE_OK)
            {
                return result;
            }

            // Write the sfs name to the file.
            result = WriteSfsBuffered(sfsFile,
                                      (uint8_t*)(sfsNamePtr->name),
                                      strlen(sfsNamePtr->name));

            if (result != LE_OK)
            {
                return result;
            }

            result = WriteSfsBuffered(sfsFile, (uint8_t*)"\n", 1);

            if (result != LE_OK)
            {
                return result;
            }
        }
    }

    // flush the write buffer
    result = WriteSfsBuffered(sfsFile, NULL, 0);

    if (result != LE_OK)
    {
        return result;
    }

    CloseSfsFile(sfsFile);

    /* Perform sfs backup. */
    // At this point, any sort of modification to the sfs has already succeeded. So it's a good
    // time to perform backup.
    StartSfsBackupTimer();

    return LE_OK;
};


//--------------------------------------------------------------------------------------------------
/**
 * Clears the hash map.  Removes all items from the hash map and releases the memory for the items.
 */
//--------------------------------------------------------------------------------------------------
static void ClearHashmap
(
    le_hashmap_Ref_t map                ///< [IN] The map to clear.
)
{
    // Iterate over the hash map.
    le_hashmap_It_Ref_t iter = le_hashmap_GetIterator(map);

    while (le_hashmap_NextNode(iter) == LE_OK)
    {
        void* key = (void*)le_hashmap_GetKey(iter);
        void* val = (void*)le_hashmap_GetValue(iter);

        le_hashmap_Remove(map, key);

        le_mem_Release(key);
        le_mem_Release(val);
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Generate a unique sfs name that will be used to identify an entry in modem.
 */
//--------------------------------------------------------------------------------------------------
static void GenerateSfsName
(
    char* bufPtr,                   ///< [OUT] Buffer to store the sfs file name.
    size_t bufSize                  ///< [IN] Size of the buffer
)
{
    // The path does not exist yet.  Find an unused sfs name.
    char tmpBuf[LE_SECSTORE_MAX_NAME_BYTES] = "";
    char* currPtr = bufPtr;
    char* prevPtr = tmpBuf;

    while (1)
    {
        // Get a potential sfs name.
        le_result_t result = GetMonotonicString(prevPtr, currPtr, bufSize);

        LE_FATAL_IF(result == LE_FAULT,
                "Error generating sfs name. Previous name %s is corrupt.", prevPtr);

        LE_FATAL_IF(result == LE_OVERFLOW,
                "Error generating sfs name.  Buffer size too small for '%s'.", currPtr);

        // Check if the potential sfs name is already being used.
        bool nameUsed = false;
        le_hashmap_It_Ref_t iter = le_hashmap_GetIterator(MetaMap);

        while (le_hashmap_NextNode(iter) == LE_OK)
        {
            const SfsName_t* sfsNamePtr = le_hashmap_GetValue(iter);

            if (strcmp(currPtr, sfsNamePtr->name) == 0)
            {
                nameUsed = true;
                break;
            }
        }

        if (!nameUsed)
        {
            break;
        }
        else
        {
            // Make the current name the previous name and try again.
            char* tmpPtr = prevPtr;
            prevPtr = currPtr;
            currPtr = tmpPtr;
        }
    }

    // Copy the sfs name into the caller's buffer.
    if (currPtr != bufPtr)
    {
        LE_ASSERT(le_utf8_Copy(bufPtr, currPtr, bufSize, NULL) == LE_OK);
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Initialize the sfs if is not already initialized.
 *
 * @note
 *      This should be called each time the sfs is accessed to ensure that the sfs is ready for use.
 *
 * @return
 *      LE_OK if successful.
 *      LE_UNAVAILABLE if the sfs is currently unavailable.
 *      LE_FAULT if there was some other error.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t InitSfs
(
    void
)
{
    if (!SfsReady)
    {

        // Create the sfs directories in case they don't exist.
        le_result_t result;

        result = MkSfsDir(USER_ROOT, SFS_PATH_TYPE_WITH_BACKUP);
        if (result != LE_OK)
        {
            return result;
        }

        result = MkSfsDir(SYS_ROOT, SFS_PATH_TYPE_WITH_BACKUP);
        if (result != LE_OK)
        {
            return result;
        }

        // Clears previously created meta data maps.
        ClearHashmap(MetaMap);

        // Load the meta data file if it is not already loaded.
        result = ImportMeta(MetaMap, META_FILE);

        if (result != LE_OK)
        {
            return result;
        }

        SfsReady = true;
    }

    return LE_OK;
}


//--------------------------------------------------------------------------------------------------
/**
 * Gets the sfs file name to use for the given user path.  If the user path already exists then the
 * sfs name will be returned for that path.  If the user path does not exists then a sfs name is
 * generated that can be used for the path.
 *
 * @return
 *      LE_OK if the path already exists.
 *      LE_NOT_FOUND if the path does not already exist.  A new sfs name that can be used for this
 *                   path is returned.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t GetSfsName
(
    const char* pathPtr,            ///< [IN] User data path.
    char* bufPtr,                   ///< [OUT] Buffer to store the sfs file name.
    size_t bufSize                  ///< [IN] Size of the buffer.
)
{
    // Check to see if the file exists.
    SfsName_t* sfsNamePtr = le_hashmap_Get(MetaMap, pathPtr);

    if (sfsNamePtr != NULL)
    {
        // Copy the name into the user buffer.
        LE_FATAL_IF(le_utf8_Copy(bufPtr, sfsNamePtr->name, bufSize, NULL) == LE_OVERFLOW,
                    "Buffer too small to store sfs name for %s.", pathPtr);

        return LE_OK;
    }

    // Generate a unique sfs name.
    GenerateSfsName(bufPtr, bufSize);

    return LE_NOT_FOUND;
}


//--------------------------------------------------------------------------------------------------
/**
 * Checks if the specified user path is a directory.  Searchs the meta data to see if the specified
 * path is a user directory.
 *
 * @return
 *      true if the path is a directory.
 *      false if the path is not a directory.
 */
//--------------------------------------------------------------------------------------------------
static bool IsUserDirectory
(
    const char* pathPtr             ///< [IN] User path.
)
{
    // Check to see if the file exists first.
    if (le_hashmap_Get(MetaMap, pathPtr) != NULL)
    {
        // The path is a file not a directory.
        return false;
    }

    // Iterate over the hash map.
    le_hashmap_It_Ref_t iter = le_hashmap_GetIterator(MetaMap);

    while (le_hashmap_NextNode(iter) == LE_OK)
    {
        // Get the current path.
        const char* currPathPtr = le_hashmap_GetKey(iter);
        LE_ASSERT(currPathPtr != NULL);

        // Check if the specified path is already an existing directory.
        if (le_path_IsSubpath(currPathPtr, pathPtr, "/"))
        {
            return true;
        }

        // Check if the specified path is under an existing file.
        if (le_path_IsSubpath(pathPtr, currPathPtr, "/"))
        {
            return true;
        }
    }

    // Path not found.
    return false;
};


//--------------------------------------------------------------------------------------------------
/**
 * Updates the meta data for the specified path and sfs name.
 *
 * @return
 *      LE_OK if successful.
 *      LE_UNAVAILABLE if the sfs is currently unavailable.
 *      LE_FAULT if there was some other error.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t UpdateMetaData
(
    const char* pathPtr,            ///< [IN] Path.
    const char* sfsNamePtr          ///< [IN] SFS file name.
)
{
    // Check to see if the path exists.
    SfsName_t* namePtr = le_hashmap_Get(MetaMap, pathPtr);

    if (namePtr != NULL)
    {
        // Sanity check the sfs name.
        LE_FATAL_IF(strcmp(namePtr->name, sfsNamePtr) != 0,
                    "Sfs name for %s should be %s but %s is provided.",
                    pathPtr, namePtr->name, sfsNamePtr);

        /* Perform sfs backup. */
        // This code path is the case of writing to an existing sfs file, which doesn't prompt an
        // update to the meta file, where sfs backup is performed. Therefore we also need to
        // perform backup here.
        StartSfsBackupTimer();

        return LE_OK;
    }

    // Add the path to the hash map.
    char* newPathPtr = le_mem_ForceAlloc(PathPool);
    LE_ASSERT(le_utf8_Copy(newPathPtr, pathPtr, SECSTOREADMIN_MAX_PATH_BYTES, NULL) == LE_OK);

    SfsName_t* newSfsNamePtr = le_mem_ForceAlloc(SfsNamePool);
    LE_ASSERT(le_utf8_Copy(newSfsNamePtr->name,
                           sfsNamePtr,
                           LE_SECSTORE_MAX_NAME_BYTES, NULL) == LE_OK);

    newSfsNamePtr->deleteFlag = false;

    LE_ASSERT(le_hashmap_Put(MetaMap, newPathPtr, newSfsNamePtr) == NULL);

    // Our hash map has changed so we must update the meta file.
    return ExportMeta(MetaMap, META_FILE);
}


//--------------------------------------------------------------------------------------------------
/**
 * Backup sfs.
 *
 * @return
 *      LE_OK if successful.
 *      LE_FAULT if there was some other error.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t BackupSfs
(
    void
)
{
    SWIQMI_DECLARE_MESSAGE(swi_sfs_backup_resp_msg_v01, sfsResp);

    // Send the message and handle the response
    qmi_client_error_type rc = qmi_client_send_msg_sync(SfsClientHandle,
                                                        QMI_SWI_SFS_BACKUP_REQ_V01,
                                                        NULL, 0,
                                                        &sfsResp, sizeof(sfsResp),
                                                        QMI_REQ_TIMEOUT);

    le_result_t result = swiQmi_OEMCheckResponseCode(STRINGIZE_EXPAND(QMI_SWI_SFS_BACKUP_REQ_V01),
                                                     rc,
                                                     sfsResp.resp);
    if (LE_OK != result)
    {
        LE_ERROR("Unexpected QMI response %d (%s)", result, LE_RESULT_TXT(result));
        return LE_FAULT;
    }

    if (sfsResp.result == 0)
    {
        // TODO: Translate error codes to return value.
        LE_ERROR("Could not backup sfs.  Result code %d.", sfsResp.result);

        return LE_FAULT;
    }

    return LE_OK;
}


//--------------------------------------------------------------------------------------------------
/**
 * Handler for the Sfs Backup Timer.
 */
//--------------------------------------------------------------------------------------------------
static void SfsBackupTimerHandler
(
    le_timer_Ref_t timerRef
)
{
    le_result_t result = BackupSfs();

    if (result == LE_OK)
    {
        LE_DEBUG("Successfully performed sfs backup.");
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Determine if a SFS path type is available.
 */
//--------------------------------------------------------------------------------------------------
static bool IsPathTypeAvailable
(
    SfsPathType_t sfsPathType    ///< [IN] Sfs path type that needs to be checked for availability.
)
{
    bool isAvailable = false;

    // Open the sfs file.
    int32_t sfsFile = 0;
    le_result_t result = OpenSfs("test", sfsPathType, SFS_READONLY_V01, &sfsFile);
    if (result == LE_OK)
    {
        // File already exists -> partition available
        isAvailable = true;
        CloseSfsFile(sfsFile);
    }
    else if (result == LE_NOT_FOUND)
    {
        // File doesn't exist but call didn't fault -> partition available
        isAvailable = true;
    }
    else
    {
        isAvailable = false;
        LE_WARN("%u path_type not available, %d %s", sfsPathType, result, LE_RESULT_TXT(result));
    }

    LE_DEBUG_IF(isAvailable, "%u path_type available", sfsPathType);

    return isAvailable;
}


//--------------------------------------------------------------------------------------------------
/**
 * Get direct access path for SFS.
 *
 * @return
 *      LE_OK if it was possible to determine the path.
 *      LE_BAD_PARAMETER if at least one parameter is invalid.
 *      LE_FAULT if the direct patch can not be retrieved.
 *      LE_OVERFLOW if the virtualPathPtr buffer is too big.
 *      LE_NOT_FOUND if the virtualPathPtr does not belong to the direct path.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t GetDirectAccessSfsPath
(
    const SfsDirectAccessPath_t* infoPtr,   ///< [IN] Info about the direct path being accessed.
    const char* virtualPathPtr,             ///< [IN] Path accessed.
    char* sfsPathPtr,                       ///< [OUT] Buffer to store the path that should be
                                            ///<       actually used for QMI sfs operations.
    size_t sfsPathSize                      ///< [IN] Size of the sfsPathPtr buffer.
)
{
    LE_ASSERT(infoPtr != NULL);

    char* savePtr;
    char* save2Ptr;
    char copyVirtualPathPtr[SECSTOREADMIN_MAX_PATH_BYTES] = {0};
    char pathPtr[SECSTOREADMIN_MAX_PATH_BYTES] = {0};
    char* tmpBufferPathPtr;
    char* tmpBufferVirtualPtr;

    if ((!virtualPathPtr) || (!sfsPathPtr))
    {
        return LE_BAD_PARAMETER;
    }

    if (SECSTOREADMIN_MAX_PATH_SIZE < strlen(virtualPathPtr))
    {
        return LE_OVERFLOW;
    }

    // This function will compare all directories, one by one, present in
    // infoPtr->virtualBasePath and in virtualBasePath
    // Wild card (*) is managed in infoPtr->virtualBasePath

    strncpy(pathPtr, virtualPathPtr, strlen(virtualPathPtr) + 1);
    strncpy(copyVirtualPathPtr, infoPtr->virtualBasePath, strlen(infoPtr->virtualBasePath) + 1);

    tmpBufferPathPtr = strtok_r(pathPtr, SEPARATOR, &savePtr);
    tmpBufferVirtualPtr = strtok_r(copyVirtualPathPtr, SEPARATOR, &save2Ptr);
    if ((!tmpBufferPathPtr) || (!tmpBufferVirtualPtr))
    {
        return LE_BAD_PARAMETER;
    }


    while (tmpBufferPathPtr && tmpBufferVirtualPtr)
    {
        tmpBufferPathPtr = strtok_r(NULL, SEPARATOR, &savePtr);
        tmpBufferVirtualPtr = strtok_r(NULL, SEPARATOR, &save2Ptr);

        if (tmpBufferPathPtr && (!tmpBufferVirtualPtr))
        {
            return le_path_Concat("/",
                                  sfsPathPtr,
                                  sfsPathSize,
                                  infoPtr->sfsBasePath,
                                  tmpBufferPathPtr,
                                  NULL);
        }
        else if (tmpBufferPathPtr && tmpBufferVirtualPtr)
        {
            if( strncmp(tmpBufferVirtualPtr, WILDCARD, strlen(WILDCARD)) &&
                strncmp(tmpBufferPathPtr, tmpBufferVirtualPtr, strlen(tmpBufferVirtualPtr)))
            {
                return LE_NOT_FOUND;
            }
        }
        else
        {
            return LE_NOT_FOUND;
        }
    }

    return LE_NOT_FOUND;
}


//--------------------------------------------------------------------------------------------------
/**
 * Reads data from the specified path in secure storage.
 *
 * @return
 *      A structure containing info about the direct-path, and null otherwise.
 */
//--------------------------------------------------------------------------------------------------
static const SfsDirectAccessPath_t* GetDirectAccessPathInfo
(
    const char* pathPtr,            ///< [IN] Path on which operations needs to be performed
    SfsPathType_t* sfsPathType,     ///< [OUT] Path type (in case direct access is used).
    char* sfsPathPtr,               ///< [OUT] Buffer to store the path that should be
                                    ///<       actually used for QMI sfs operations (in case direct
                                    ///<       access is used).
    size_t sfsPathSize,             ///< [IN] Size of the sfsPathPtr buffer.
    int32_t* sfsFilePtr             ///< [OUT] If not null and direct access is used, then return
                                    ///<       a read-only file descriptor.
)
{
    int index;
    const SfsDirectAccessPath_t *directAccessInfoPtr;
    le_result_t res;

    // Go through all direct-access entries
    for (index = 0; index < NUM_ARRAY_MEMBERS(DirectAccessPaths); index++)
    {
        directAccessInfoPtr = &DirectAccessPaths[index];

        // Not in use?
        if (!directAccessInfoPtr->isInUse)
        {
            continue;
        }

        // Get the direct access path in SFS
        res = GetDirectAccessSfsPath(directAccessInfoPtr, pathPtr, sfsPathPtr, sfsPathSize);
        if (LE_OK != res)
        {
            if (LE_NOT_FOUND != res)
            {
                LE_ERROR("Unable to determine direct path to access %s", pathPtr);
            }
            continue;
        }

        // It is an existing file?
        int32_t sfsFile;
        if (LE_OK != OpenSfs(sfsPathPtr, directAccessInfoPtr->sfsPathType,
                             SFS_READONLY_V01, &sfsFile))
        {
            continue;
        }

        // In case a sfsFilePtr has been provided, return the file descriptor
        // as to avoid a re-opening of the file if the type of access required
        // is read-only.
        if (sfsFilePtr == NULL)
        {
            CloseSfsFile(sfsFile);
        }
        else
        {
            *sfsFilePtr = sfsFile;
        }

        // Return path_type and path info
        *sfsPathType = directAccessInfoPtr->sfsPathType;
        return &(DirectAccessPaths[index]);
    }
    return NULL;
}


//--------------------------------------------------------------------------------------------------
/**
 * Writes the data in the buffer to the specified path in secure storage replacing any previously
 * written data at the same path.  Specifying 0 for buffer size means emptying an existing file or
 * creating a 0-byte file.
 *
 * @return
 *      LE_OK if successful.
 *      LE_NO_MEMORY if there is not enough memory to store the data.
 *      LE_UNAVAILABLE if the secure storage is currently unavailable.
 *      LE_BAD_PARAMETER if the path cannot be written to because it is a directory or ot would
 *                       result in an invalid path.
 *      LE_FAULT if there was some other error.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_secStore_Write
(
    const char* pathPtr,            ///< [IN] Path to write to.
    const uint8_t* bufPtr,          ///< [IN] Buffer containing the data to write.
    size_t bufSize                  ///< [IN] Size of the buffer.
)
{
    // Init the sfs.
    le_result_t result = InitSfs();
    if (result != LE_OK)
    {
        return result;
    }

    LE_DEBUG("Write to %s", pathPtr);

    char sfsPath[SECSTOREADMIN_MAX_PATH_BYTES] = {0};
    char sfsName[LE_SECSTORE_MAX_NAME_BYTES] = {0};
    bool updateMeta = true;
    SfsPathType_t sfsPathType = SFS_PATH_TYPE_DEFAULT;

    const SfsDirectAccessPath_t *directAccessInfoPtr = GetDirectAccessPathInfo(pathPtr,
                                                                               &sfsPathType,
                                                                               sfsPath,
                                                                               sizeof(sfsPath),
                                                                               NULL);
    if (NULL != directAccessInfoPtr)
    {
        LE_DEBUG("Direct access for %s", pathPtr);

        // Do not maintain meta file for direct access
        updateMeta = false;
    }
    else
    {
        // Search the list of files to make sure that the caller's specified path is not
        // already a dir.
        if (IsUserDirectory(pathPtr))
        {
            LE_ERROR("Cannot write to directory %s.", pathPtr);
            return LE_BAD_PARAMETER;
        }

        // Get the sfs filename to use.  Don't need to check the return code because we will use the
        // sfs name regardless.
        GetSfsName(pathPtr, sfsName, sizeof(sfsName));

        // Prepend the root directory to the sfs name.
        sfsPath[0] = '\0';
        LE_ASSERT(le_path_Concat("/", sfsPath, sizeof(sfsPath), USER_ROOT, sfsName, NULL) == LE_OK);

        // In the special case of 0-sized buffer, we create a 0-byte file by deleting the sfs file
        // and create/maintain an entry in the meta file.
        if (bufSize == 0)
        {
            // Delete the sfs file.
            result = DeleteSfsFile(sfsPath, sfsPathType);

            if (result != LE_OK)
            {
                return result;
            }

            return UpdateMetaData(pathPtr, sfsName);
        }
    }

    // Open the sfs file.
    int32_t sfsFile;
    result = OpenSfs(sfsPath, sfsPathType, SFS_CREATE_V01, &sfsFile);

    if (result != LE_OK)
    {
        return result;
    }

    // Write the data to the sfs file.
    result = WriteSfs(sfsFile, bufPtr, bufSize);

    CloseSfsFile(sfsFile);

    // Update the meta data.
    if ((LE_OK == result) && updateMeta)
    {
        result = UpdateMetaData(pathPtr, sfsName);
    }

    return result;
}


//--------------------------------------------------------------------------------------------------
/**
 * Reads data from the specified path in secure storage.
 *
 * @return
 *      LE_OK if successful.
 *      LE_OVERFLOW if the buffer is too small to hold all the data.  No data will be written to the
 *                  buffer in this case.
 *      LE_NOT_FOUND if the path is empty.
 *      LE_UNAVAILABLE if the secure storage is currently unavailable.
 *      LE_FAULT if there was some other error.
 */
//--------------------------------------------------------------------------------------------------
LE_SHARED le_result_t pa_secStore_Read
(
    const char* pathPtr,            ///< [IN] Path to read from.
    uint8_t* bufPtr,                ///< [OUT] Buffer to store the data in.
    size_t* bufSizePtr              ///< [IN/OUT] Size of buffer when this function is called.
                                    ///          Number of bytes read when this function returns.
)
{
    // Init the sfs.
    le_result_t result = InitSfs();
    if (result != LE_OK)
    {
        return result;
    }

    LE_DEBUG("Read from %s", pathPtr);

    int32_t sfsFile;
    char sfsPath[SECSTOREADMIN_MAX_PATH_BYTES] = {0};
    SfsPathType_t sfsPathType = SFS_PATH_TYPE_DEFAULT;

    const SfsDirectAccessPath_t *directAccessInfoPtr = GetDirectAccessPathInfo(pathPtr,
                                                                               &sfsPathType,
                                                                               sfsPath,
                                                                               sizeof(sfsPath),
                                                                               &sfsFile);
    if (NULL != directAccessInfoPtr)
    {
        LE_DEBUG("Direct access for %s", pathPtr);
    }
    else
    {
        // Get the sfs name.
        char sfsName[LE_SECSTORE_MAX_NAME_BYTES];
        if (GetSfsName(pathPtr, sfsName, sizeof(sfsName)) == LE_NOT_FOUND)
        {
            return LE_NOT_FOUND;
        }

        // Prepend the root directory to the sfs name.
        sfsPath[0] = '\0';
        LE_ASSERT(le_path_Concat("/", sfsPath, sizeof(sfsPath), USER_ROOT, sfsName, NULL) == LE_OK);

        // Open the file for reading.
        result = OpenSfs(sfsPath, sfsPathType, SFS_READONLY_V01, &sfsFile);

        // If an sfs item has entry in the Meta file but doesn't have an actual sfs file,
        // that means this sfs item is a 0-byte file. Therefore we zero-out the read buffer
        // and return LE_OK.
        if (result == LE_NOT_FOUND)
        {
            memset(bufPtr, 0, *bufSizePtr);
            *bufSizePtr = 0;
            return LE_OK;
        }
        else if (result != LE_OK)
        {
            return result;
        }
    }

    // Read the entire file. One extra byte will be read from modem to check if EOF is reached.
    result = ReadSfs(sfsFile, bufPtr, bufSizePtr, true);
    CloseSfsFile(sfsFile);
    return result;
}


//--------------------------------------------------------------------------------------------------
/**
 * Copy the meta file to the specified path.
 *
 * @return
 *      LE_OK if successful.
 *      LE_NOT_FOUND if the meta file does not exist.
 *      LE_UNAVAILABLE if the sfs is currently unavailable.
 *      LE_FAULT if there was some other error.
 */
//--------------------------------------------------------------------------------------------------
LE_SHARED le_result_t pa_secStore_CopyMetaTo
(
    const char* pathPtr             ///< [IN] Destination path of meta file copy.
)
{
    return CopySfsFileToTemp(META_FILE, pathPtr);
}


//--------------------------------------------------------------------------------------------------
/**
 * Deletes the specified path and everything under it.
 *
 * @return
 *      LE_OK if successful.
 *      LE_NOT_FOUND if the path does not exist.
 *      LE_UNAVAILABLE if the secure storage is currently unavailable.
 *      LE_FAULT if there was some other error.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_secStore_Delete
(
    const char* pathPtr             ///< [IN] Path to delete.
)
{
    // Init the sfs.
    le_result_t result = InitSfs();
    if (result != LE_OK)
    {
        return result;
    }

    char sfsPath[SECSTOREADMIN_MAX_PATH_BYTES] = {0};
    SfsPathType_t sfsPathType = SFS_PATH_TYPE_DEFAULT;

    // If the file is directly accessible, remove directly without meta file management.
    const SfsDirectAccessPath_t *directAccessInfoPtr = GetDirectAccessPathInfo(pathPtr,
                                                                               &sfsPathType,
                                                                               sfsPath,
                                                                               sizeof(sfsPath),
                                                                               NULL);
    if (NULL != directAccessInfoPtr)
    {
        LE_DEBUG("Direct access for %s", pathPtr);

        return DeleteSfsFile(sfsPath, sfsPathType);
    }

    // Iterate over the hash map to find the items to delete.
    bool updateMeta = false;
    le_hashmap_It_Ref_t iter = le_hashmap_GetIterator(MetaMap);

    while (le_hashmap_NextNode(iter) == LE_OK)
    {
        // Get the current path.
        char* currPathPtr = (char*)le_hashmap_GetKey(iter);
        LE_ASSERT(currPathPtr != NULL);

        // See if the current path is in the path to delete.
        if ( le_path_IsEquivalent(pathPtr, currPathPtr, "/") ||
             le_path_IsSubpath(pathPtr, currPathPtr, "/") )
        {
            // Get the sfs file name.
            SfsName_t* sfsNamePtr = (SfsName_t*)le_hashmap_GetValue(iter);
            LE_ASSERT(sfsNamePtr != NULL);

            // Mark this sfs file for deletion.
            sfsNamePtr->deleteFlag = true;

            updateMeta = true;
        }
    }

    if (updateMeta)
    {
        // Update the meta file first.
        result = ExportMeta(MetaMap, META_FILE);

        if (result != LE_OK)
        {
            return result;
        }

        // Then delete the actual sfs files.
        iter = le_hashmap_GetIterator(MetaMap);

        while (le_hashmap_NextNode(iter) == LE_OK)
        {
            // Get the current path.
            char* currPathPtr = (char*)le_hashmap_GetKey(iter);
            LE_ASSERT(currPathPtr != NULL);

            // Get the sfs file name.
            SfsName_t* sfsNamePtr = (SfsName_t*)le_hashmap_GetValue(iter);
            LE_ASSERT(sfsNamePtr != NULL);

            if (sfsNamePtr->deleteFlag)
            {
                // Prepend the root directory to the sfs name.
                sfsPath[0] = '\0';
                LE_ASSERT(le_path_Concat("/", sfsPath, sizeof(sfsPath), USER_ROOT,
                                                                        sfsNamePtr->name,
                                                                        NULL) == LE_OK);

                // Attempt to delete the sfs file and disregard the result. At this point the meta
                // file has been successfully updated, so to the user of this interface, the sfs
                // file can no longer be accessed. Therefore we can safely proceed even if the deletion
                // fails, in which case there's just some wasted space.
                DeleteSfsFile(sfsPath, sfsPathType);

                // Delete the item from the hashmap.
                sfsNamePtr = le_hashmap_Remove(MetaMap, currPathPtr);
                LE_ASSERT(sfsNamePtr != NULL);

                // Release the memory for the path and the sfs name.
                le_mem_Release(currPathPtr);
                le_mem_Release(sfsNamePtr);
            }
        }

        return LE_OK;
    }

    return LE_NOT_FOUND;
}


//--------------------------------------------------------------------------------------------------
/**
 * Gets the size, in bytes, of the data at the specified path and everything under it.
 *
 * @return
 *      LE_OK if successful.
 *      LE_NOT_FOUND if the path does not exist.
 *      LE_UNAVAILABLE if the secure storage is currently unavailable.
 *      LE_FAULT if there was some other error.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_secStore_GetSize
(
    const char* pathPtr,            ///< [IN] Path.
    size_t* sizePtr                 ///< [OUT] Size in bytes of all items in the path.
)
{
    // Init the sfs.
    le_result_t result = InitSfs();
    if (result != LE_OK)
    {
        return result;
    }

    char sfsPath[SECSTOREADMIN_MAX_PATH_BYTES] = {0};
    SfsPathType_t sfsPathType = SFS_PATH_TYPE_DEFAULT;
    int32_t sfsFile;

    // If the file is directly accessible, get size directly without meta file management.
    // This only works for fildes and not directories.
    const SfsDirectAccessPath_t *directAccessInfoPtr = GetDirectAccessPathInfo(pathPtr,
                                                                               &sfsPathType,
                                                                               sfsPath,
                                                                               sizeof(sfsPath),
                                                                               &sfsFile);
    if (NULL != directAccessInfoPtr)
    {
        LE_DEBUG("Direct access for %s", pathPtr);

        result = GetSfsSize(sfsFile, sizePtr);

        CloseSfsFile(sfsFile);
        return result;
    }

    // Iterate over the hash map.
    bool foundEntry = false;
    *sizePtr = 0;

    le_hashmap_It_Ref_t iter = le_hashmap_GetIterator(MetaMap);

    while (le_hashmap_NextNode(iter) == LE_OK)
    {
        // Get the current path.
        const char* currPathPtr = le_hashmap_GetKey(iter);
        LE_ASSERT(currPathPtr != NULL);

        // See if the current path is in the path.
        if ( le_path_IsEquivalent(pathPtr, currPathPtr, "/") ||
             le_path_IsSubpath(pathPtr, currPathPtr, "/") )
        {
            foundEntry = true;

            // Get the sfs file name.
            const SfsName_t* sfsNamePtr = le_hashmap_GetValue(iter);
            LE_ASSERT(sfsNamePtr != NULL);

            // Prepend the root directory to the sfs name.
            sfsPath[0] = '\0';
            LE_ASSERT(le_path_Concat("/", sfsPath, sizeof(sfsPath), USER_ROOT,
                                                                    sfsNamePtr->name,
                                                                    NULL) == LE_OK);

            // Open the file for reading.
            int32_t sfsFile;
            le_result_t result = OpenSfs(sfsPath, sfsPathType, SFS_READONLY_V01,
                                         &sfsFile);

            // If an sfs item has entry in the Meta file but doesn't have an actual sfs file, that means
            // this sfs item is a 0-byte file. Therefore we skip to examine the next node.
            if (result == LE_NOT_FOUND)
            {
                continue;
            }
            else if (result != LE_OK)
            {
                return result;
            }

            // Get the sfs file size.
            size_t currSize;
            result = GetSfsSize(sfsFile, &currSize);

            if (result != LE_OK)
            {
                CloseSfsFile(sfsFile);
                return result;
            }

            CloseSfsFile(sfsFile);

            *sizePtr += currSize;
        }
    }

    if (foundEntry)
    {
        return LE_OK;
    }

    return LE_NOT_FOUND;
}


//--------------------------------------------------------------------------------------------------
/**
 * Iterates over all entries under the specified path (non-recursive), calling the supplied callback
 * with each entry name.
 *
 * Doesn't work for direct-access paths since it is not possible to list entries with SFS, so this
 * implementation enterily relies on the meta file.
 *
 * @return
 *      LE_OK if successful.
 *      LE_UNAVAILABLE if the secure storage is currently unavailable.
 *      LE_FAULT if there was some other error.
 */
//--------------------------------------------------------------------------------------------------
LE_SHARED le_result_t pa_secStore_GetEntries
(
    const char* pathPtr,                    ///< [IN] Path.
    pa_secStore_GetEntry_t getEntryFunc,    ///< [IN] Callback function to call with each entry.
    void* contextPtr                        ///< [IN] Context to be supplied to the callback.
)
{
    // Init the sfs.
    le_result_t result = InitSfs();
    if (result != LE_OK)
    {
        return result;
    }

    // Iterate over the meta hash map to find entries.
    size_t pathLen = strlen(pathPtr);

    le_hashmap_It_Ref_t iter = le_hashmap_GetIterator(MetaMap);

    while (le_hashmap_NextNode(iter) == LE_OK)
    {
        const char* itemPathPtr = le_hashmap_GetKey(iter);
        LE_ASSERT(itemPathPtr != NULL);

        if (le_path_IsSubpath(pathPtr, itemPathPtr, "/"))
        {
            // Find the entry string.
            char* entryPtr;

            if (pathPtr[pathLen-1] == '/')
            {
                entryPtr = (char*)&(itemPathPtr[pathLen]);
            }
            else if (itemPathPtr[pathLen] == '/')
            {
                entryPtr = (char*)&(itemPathPtr[pathLen + 1]);
            }
            else
            {
                // There is no entry string, move on.
                continue;
            }

            // Check if the entry is a directory.
            bool isDir = (strchr(entryPtr, '/') != NULL);

            // Copy the entry to a local buffer.
            char buf[SECSTOREADMIN_MAX_PATH_BYTES];

            LE_FATAL_IF(le_utf8_CopyUpToSubStr(buf, entryPtr, "/", sizeof(buf), NULL) != LE_OK,
                        "Buffer too small to hold entry name '%s...'.", buf);

            // Call the callback function.
            if (getEntryFunc != NULL)
            {
                getEntryFunc(buf, isDir, contextPtr);
            }
        }
    }

    return LE_OK;
}


//--------------------------------------------------------------------------------------------------
/**
 * Gets the total space and the available free space in secure storage.
 *
 * @return
 *      LE_OK if successful.
 *      LE_UNAVAILABLE if the secure storage is currently unavailable.
 *      LE_FAULT if there was some other error.
 */
//--------------------------------------------------------------------------------------------------
LE_SHARED le_result_t pa_secStore_GetTotalSpace
(
    size_t* totalSpacePtr,                  ///< [OUT] Total size, in bytes, of secure storage.
    size_t* freeSizePtr                     ///< [OUT] Free space, in bytes, in secure storage.
)
{
    // Init the sfs.
    le_result_t result = InitSfs();
    if (result != LE_OK)
    {
        return result;
    }

    SWIQMI_DECLARE_MESSAGE(swi_sfs_get_space_info_resp_msg_v01, sfsResp);

    // Send the message and handle the response
    qmi_client_error_type rc = qmi_client_send_msg_sync(SfsClientHandle,
                                                        QMI_SWI_SFS_GET_SPACE_INFO_REQ_V01,
                                                        NULL, 0,
                                                        &sfsResp, sizeof(sfsResp),
                                                        QMI_REQ_TIMEOUT);

    result = swiQmi_OEMCheckResponseCode(STRINGIZE_EXPAND(QMI_SWI_SFS_GET_SPACE_INFO_REQ_V01),
                                         rc,
                                         sfsResp.resp);
    if (LE_OK != result)
    {
        LE_ERROR("Unexpected QMI response %d (%s)", result, LE_RESULT_TXT(result));
        return LE_FAULT;
    }

    *totalSpacePtr = sfsResp.remaining_mem + sfsResp.used_mem;
    *freeSizePtr = sfsResp.remaining_mem;

    return LE_OK;
}


//--------------------------------------------------------------------------------------------------
/**
 * Copies an sfs file to another location in the sfs.
 *
 * @warning
 *      This API is limited to SFS_PATH_TYPE_WITH_BACKUP.
 *
 * @return
 *      LE_OK if successful.
 *      LE_NOT_FOUND if the source sfs file does not exist.
 *      LE_UNAVAILABLE if the sfs is currently unavailable.
 *      LE_FAULT if there was some other error.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t CopySfsFile
(
    const char* srcSfsPathPtr,          ///< [IN] Path to the source sfs file.
    const char* destSfsPathPtr          ///< [IN] Path to the destination sfs file.
)
{
    // TODO: Handly copy between SFS path_type
    SfsPathType_t sfsPathType = SFS_PATH_TYPE_WITH_BACKUP;

    // Open the source file for reading.
    int32_t srcSfsFile;
    le_result_t result = OpenSfs(srcSfsPathPtr, sfsPathType, SFS_READONLY_V01, &srcSfsFile);

    if (result != LE_OK)
    {
        LE_ERROR("Could not open sfs file %s for reading", srcSfsPathPtr);
        return result;
    }

    // Open the destination file for writing.
    int32_t destSfsFile;
    result = OpenSfs(destSfsPathPtr, sfsPathType, SFS_CREATE_V01, &destSfsFile);

    if (result != LE_OK)
    {
        LE_ERROR("Could not create sfs file %s", destSfsPathPtr);

        CloseSfsFile(srcSfsFile);
        return result;
    }

    // Copy the contents of the sfs source file to the destination.
    while (1)
    {
        // Read a buffer of data from the sfs source file.
        uint8_t buf[MAX_DATA_SIZE_V01];
        size_t numBytes = MAX_DATA_SIZE_V01;

        // Read MAX_DATA_SIZE_V01 chunks until there is no more bytes to read.
        result = ReadSfs(srcSfsFile, buf, &numBytes, false);

        if ( (result == LE_FAULT) || (result == LE_UNAVAILABLE) )
        {
            LE_ERROR("Could not read source file.");
            goto out;
        }

        if (numBytes == 0)
        {
            // Nothing more to read.
            break;
        }

        // Write the buffer to the dest file.
        result = WriteSfs(destSfsFile, buf, numBytes);

        if (result != LE_OK)
        {
            LE_ERROR("Could not write to destination file.");
            goto out;
        }
    }

    result = LE_OK;

out:
    CloseSfsFile(srcSfsFile);
    CloseSfsFile(destSfsFile);
    return result;
};


//--------------------------------------------------------------------------------------------------
/**
 * Callback handler for each sfs entry. This function is used in pa_secStore_Copy(), for iterating
 * through the sfs entries.
 *
 * @return
 *      true if successful.
 *      false otherwise
 */
//--------------------------------------------------------------------------------------------------
static bool CopyToCurrentSys
(
    const void* keyPtr,     // Pointer to the map entry's key
    const void* valuePtr,   // Pointer to the map entry's value
    void* contextPtr        // Pointer to an abritrary context for the callback function
)
{
    le_result_t result;
    SfsCopyCtxt_t* ctxt = (SfsCopyCtxt_t*)contextPtr;

    // Generate new sfs file name to use.
    char destSfsName[LE_SECSTORE_MAX_NAME_BYTES];
    GenerateSfsName(destSfsName, sizeof(destSfsName));

    char* currFilePtr = (char*)keyPtr;
    LE_ASSERT(currFilePtr != NULL);

    SfsName_t* srcSfsNamePtr = (SfsName_t*)valuePtr;
    LE_ASSERT(srcSfsNamePtr != NULL);

    // See if the current path is in the src path.
    if ( le_path_IsEquivalent(ctxt->srcPathPtr, currFilePtr, "/") ||
         le_path_IsSubpath(ctxt->srcPathPtr, currFilePtr, "/") )
    {
        // Create the dest path to copy to.
        char* destFilePtr = le_mem_ForceAlloc(PathPool);
        destFilePtr[0] = '\0';

        LE_FATAL_IF(le_path_Concat("/",
                                   destFilePtr,
                                   SECSTOREADMIN_MAX_PATH_BYTES,
                                   ctxt->destPathPtr,
                                   &(currFilePtr[strlen(ctxt->srcPathPtr)]),
                                   NULL) != LE_OK,
                    "Dest path '%s' is too long.", destFilePtr);

        // Copy the sfs file.
        char srcSfsFile[SECSTOREADMIN_MAX_PATH_BYTES] = USER_ROOT;
        LE_ASSERT(le_path_Concat("/", srcSfsFile, sizeof(srcSfsFile), srcSfsNamePtr->name, NULL) == LE_OK);

        char destSfsFile[SECSTOREADMIN_MAX_PATH_BYTES] = USER_ROOT;
        LE_ASSERT(le_path_Concat("/", destSfsFile, sizeof(destSfsFile), destSfsName, NULL) == LE_OK);

        result = CopySfsFile(srcSfsFile, destSfsFile);

        if (result == LE_NOT_FOUND)
        {
            LE_ERROR("Copy failed.  Source file %s does not exist", currFilePtr);
            return false;
        }

        if (result != LE_OK)
        {
            LE_ERROR("Copy failed. Error in copying %s", currFilePtr);
            return false;
        }

        // Add the destination path and name to the hash table.
        SfsName_t* destSfsNamePtr = le_mem_ForceAlloc(SfsNamePool);
        destSfsNamePtr->deleteFlag = false;

        LE_ASSERT(le_utf8_Copy(destSfsNamePtr->name, destSfsName, LE_SECSTORE_MAX_NAME_BYTES, NULL) == LE_OK);

        LE_ASSERT(le_hashmap_Put(MetaMap, destFilePtr, destSfsNamePtr) == NULL);
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/**
 * Copies all the data from source path to destination path.  The destination path must be empty.
 *
 * @warning
 *      In this implementation the destination path should be a non-existant or empty path.
 *      This API is limited to SFS_PATH_TYPE_WITH_BACKUP.
 *
 * @return
 *      LE_OK if successful.
 *      LE_UNAVAILABLE if the secure storage is currently unavailable.
 *      LE_FAULT if there was some other error.
 */
//--------------------------------------------------------------------------------------------------
LE_SHARED le_result_t pa_secStore_Copy
(
    const char* destPathPtr,                ///< [IN] Destination path.
    const char* srcPathPtr                  ///< [IN] Source path.
)
{
    bool updateMeta = false;

    // Make sure the source and dest paths don't overlap.
    if ( le_path_IsEquivalent(srcPathPtr, destPathPtr, "/") ||
         le_path_IsSubpath(srcPathPtr, destPathPtr, "/") ||
         le_path_IsSubpath(destPathPtr, srcPathPtr, "/") )
    {
        LE_ERROR("Cannot copy %s into itself.", srcPathPtr);
        return LE_FAULT;
    }

    // Init the sfs.
    le_result_t result = InitSfs();
    if (result != LE_OK)
    {
        return result;
    }

    // Set context.
    SfsCopyCtxt.srcPathPtr = srcPathPtr;
    SfsCopyCtxt.destPathPtr = destPathPtr;

    // Iterate through all entries in the hashmap and copy them over to current system.
    updateMeta = le_hashmap_ForEach(MetaMap, CopyToCurrentSys, &SfsCopyCtxt);

    // Update the meta file.
    if (updateMeta)
    {
        // Our hash map has changed so we must update the meta file.
        LE_DEBUG("Update meta file");
        return ExportMeta(MetaMap, META_FILE);
    }

    return LE_OK;
}


//--------------------------------------------------------------------------------------------------
/**
 * Moves all the data from source path to destination path.  The destination path must be empty.
 *
 * @warning
 *      In this implementation, this API is limited to SFS_PATH_TYPE_WITH_BACKUP.
 *
 * @return
 *      LE_OK if successful.
 *      LE_UNAVAILABLE if the secure storage is currently unavailable.
 *      LE_FAULT if there was some other error.
 */
//--------------------------------------------------------------------------------------------------
LE_SHARED le_result_t pa_secStore_Move
(
    const char* destPathPtr,                ///< [IN] Destination path.
    const char* srcPathPtr                  ///< [IN] Source path.
)
{
    // Make sure the source and dest paths don't overlap.
    if ( le_path_IsEquivalent(srcPathPtr, destPathPtr, "/") ||
         le_path_IsSubpath(srcPathPtr, destPathPtr, "/") ||
         le_path_IsSubpath(destPathPtr, srcPathPtr, "/") )
    {
        LE_ERROR("Cannot move %s into itself.", srcPathPtr);
        return LE_FAULT;
    }

    // Init the sfs.
    le_result_t result = InitSfs();
    if (result != LE_OK)
    {
        return result;
    }

    // Iterate over the hash map to find the items to move.
    bool updateMeta = false;
    le_hashmap_It_Ref_t iter = le_hashmap_GetIterator(MetaMap);

    while (le_hashmap_NextNode(iter) == LE_OK)
    {
        // Get the current file path.
        char* currFilePtr = (char*)le_hashmap_GetKey(iter);
        LE_ASSERT(currFilePtr != NULL);

        // See if the current path is in the src path.
        if ( le_path_IsEquivalent(srcPathPtr, currFilePtr, "/") ||
             le_path_IsSubpath(srcPathPtr, currFilePtr, "/") )
        {
            // Create the new file path.
            char destFile[SECSTOREADMIN_MAX_PATH_BYTES] = "";

            LE_FATAL_IF(le_path_Concat("/",
                                       destFile,
                                       SECSTOREADMIN_MAX_PATH_BYTES,
                                       destPathPtr,
                                       &(currFilePtr[strlen(srcPathPtr)]),
                                       NULL) != LE_OK,
                        "Dest path '%s' is too long.", destFile);

            // temporarily store the value ptr.
            const SfsName_t* currSfsNamePtr = le_hashmap_GetValue(iter);

            // remove the entry from the hashmap.
            LE_ASSERT(le_hashmap_Remove(MetaMap, currFilePtr) != NULL);

            // Update the file path to the key ptr.
            LE_ASSERT(le_utf8_Copy(currFilePtr, destFile, SECSTOREADMIN_MAX_PATH_BYTES, NULL) == LE_OK);

            // Put the updated key and original value ptrs back to the hashmap.
            LE_ASSERT(le_hashmap_Put(MetaMap, currFilePtr, currSfsNamePtr) == NULL);

            updateMeta = true;
        }
    }

    // Update the meta file.
    if (updateMeta)
    {
        // Our hash map has changed so we must update the meta file.
        return ExportMeta(MetaMap, META_FILE);
    }

    return LE_OK;
}


//--------------------------------------------------------------------------------------------------
/**
 * Init this component
 */
//--------------------------------------------------------------------------------------------------
COMPONENT_INIT
{
    // Create the memory pools needed to store the path names and sfs file names.
    PathPool = le_mem_CreatePool("SecStorePaths", SECSTOREADMIN_MAX_PATH_BYTES);
    SfsNamePool = le_mem_CreatePool("SfsFiles", sizeof(SfsName_t));

    // Create the meta data hash map.
    MetaMap = le_hashmap_Create("SfsFiles",
                                ESTIMATED_MAX_SFS_FILES,
                                le_hashmap_HashString,
                                le_hashmap_EqualsString);

    // Initialize the required service
    if (swiQmi_InitServices(QMI_SERVICE_SFS) != LE_OK)
    {
        LE_ERROR("Could not initialize QMI SFS Service.");
        return;
    }

    // Get the qmi client service handle for later use.
    if ((SfsClientHandle = swiQmi_GetClientHandle(QMI_SERVICE_SFS)) == NULL)
    {
        // This should not happen, since we just did the init above, but log an error anyways.
        LE_ERROR("Could not get service handle for QMI SFS Service.");
        return;
    }

    // Create and initialize the one-shot timer for sfs backup.
    SfsBackupTimer = le_timer_Create("SfsBackupTimer");
    le_timer_SetHandler(SfsBackupTimer, SfsBackupTimerHandler);
    le_timer_SetInterval(SfsBackupTimer, (le_clk_Time_t){SFS_BACKUP_INTERVAL, 0});

    // Determine AVMS partition availability
    int index;
    for (index = 0; index < NUM_ARRAY_MEMBERS(DirectAccessPaths); index++)
    {
        if (SFS_PATH_TYPE_AVMS_CREDENTIALS == DirectAccessPaths[index].sfsPathType)
        {
            DirectAccessPaths[index].isInUse = IsPathTypeAvailable(SFS_PATH_TYPE_AVMS_CREDENTIALS);
        }
    }

    // Ignore the error.  If there is an error we can try again later.
    InitSfs();
}
