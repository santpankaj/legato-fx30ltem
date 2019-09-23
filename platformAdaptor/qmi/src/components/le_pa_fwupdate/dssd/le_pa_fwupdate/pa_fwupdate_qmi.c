/**
 * @file pa_fwupdate_qmi.c
 *
 * QMI implementation of @ref c_pa_fwupdate API.
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#include "legato.h"
#include "pa_fwupdate.h"
#include "pa_fwupdate_dualsys.h"
#include "pa_fwupdate_qmi_common.h"
#include "partition_local.h"
#include "swiQmi.h"

// for print macros
#include "le_print.h"

// FW update status result
#define FWUPDATE_STATUS_OK             0x00000001
#define FWUPDATE_STATUS_UNKNOWN        0xFFFFFFFF
#define FWUPDATE_STATUS_FILE_ERROR     0x10000000
#define FWUPDATE_STATUS_NVUP_ERROR     0x20000000
#define FWUPDATE_STATUS_FOTA_ERROR     0x40000000
#define FWUPDATE_STATUS_FDT_SSDP_ERROR 0x80000000
#define FWUPDATE_STATUS_ERROR_MASK     0x00000FFF

//--------------------------------------------------------------------------------------------------
/**
 * If we need to clear specify bad image flags, we should contain this flags in high bit.
 */
//--------------------------------------------------------------------------------------------------
#define DS_IMAGE_FLAG_UP               0x8000000000000000

// Out of sync flags
#define OUT_OF_SYNC_CLEARED 0x00000000 // out of sync flag cleared
#define OUT_OF_SYNC_ON      0x4F6F5300 // dual system is out of sync
#define OUT_OF_SYNC_OFF     0x73796E63 // dual system is sync

// EFS status
#define EFS_STATUS_OK        0x00000000 // EFS is healthy.
#define EFS_STATUS_CORRUPTED 0x45465343 // EFS is corrupted and needs a recovery.

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
static qmi_client_type MgsClient = NULL;

//--------------------------------------------------------------------------------------------------
/**
 * SSDATA bad_image bit field is updated during fwupdate process: store its value for later use.
 */
//--------------------------------------------------------------------------------------------------
static uint64_t SsdataBadImage = 0x0;

//--------------------------------------------------------------------------------------------------
/**
 * Open bad image handlers counter
 */
//--------------------------------------------------------------------------------------------------
static int BadImageHandlerCounter = 0;


//--------------------------------------------------------------------------------------------------
/**
 * Flag to know if an update is ongoing
 */
//--------------------------------------------------------------------------------------------------
static bool IsUpdateOngoing;


//==================================================================================================
//  MODULE/COMPONENT FUNCTIONS
//==================================================================================================

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
static le_result_t pa_fwupdate_Init
(
    void
)
{
    /* Initialize the required service */
    if (swiQmi_InitServices(QMI_SERVICE_DMS) != LE_OK)
    {
        LE_ERROR ("Could not initialize QMI Device Management Service.");
        return LE_FAULT;
    }

    if ( (DmsClient = swiQmi_GetClientHandle(QMI_SERVICE_DMS)) == NULL)
    {
        LE_ERROR ("Error opening QMI Device Management Service.");
        return LE_FAULT;
    }

    if (swiQmi_InitServices(QMI_SERVICE_MGS) != LE_OK)
    {
        LE_ERROR ("Could not initialize QMI M2M General Service.");
        return LE_FAULT;
    }

    if ( (MgsClient = swiQmi_GetClientHandle(QMI_SERVICE_MGS)) == NULL)
    {
        LE_ERROR ("Error opening QMI M2M General Service.");
        return LE_FAULT;
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to initialize the QMI FW UPDATE module.
 */
//--------------------------------------------------------------------------------------------------
COMPONENT_INIT
{
    LE_INFO ("Start the QMI PA FW update initialization.");
    LE_ERROR_IF ((pa_fwupdate_Init() != LE_OK), "Error on PA FWUPDATE initialization.");
    LE_INFO ("QMI PA FW update initialization ends.");
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
        LE_ERROR("Could not initialize QMI Device Management Service.");
        return LE_FAULT;
    }

    if ( (DmsClient = swiQmi_GetClientHandle(QMI_SERVICE_DMS)) == NULL)
    {
        LE_ERROR("Error opening QMI Device Management Service.");
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

    if (DmsClient == NULL)
    {
        if (InitQmiServices() == LE_FAULT)
        {
            LE_ERROR("Failed to initialize QMI Service.");
            return LE_FAULT;
        }
    }

    // There is no request message, only a response message.
    dms_get_device_rev_id_resp_msg_v01 deviceResp = { {0} };

    LE_PRINT_VALUE("%p", DmsClient);
    rc = qmi_client_send_msg_sync(
            DmsClient,
            QMI_DMS_GET_DEVICE_REV_ID_REQ_V01,
            NULL, 0,
            &deviceResp, sizeof(deviceResp),
            COMM_DEFAULT_PLATFORM_TIMEOUT);

    le_result_t result = swiQmi_CheckResponseCode(
                                STRINGIZE_EXPAND(QMI_DMS_GET_DEVICE_REV_ID_REQ_V01),
                                rc, deviceResp.resp);
    if ( result != LE_OK )
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
 * Send a SSDATA read request: QMI message
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
static le_result_t ReadSsdata
(
    dms_swi_dssd_read_resp_msg_v01* dataPtr    ///< [OUT] SSDATA structure
)
{
    le_result_t result = LE_FAULT;

    /* Check that the data size is the correct one */
    if ((dataPtr != NULL) && (sizeof (*dataPtr) == sizeof (dms_swi_dssd_read_resp_msg_v01)))
    {
        SWIQMI_DECLARE_MESSAGE (dms_swi_dssd_read_req_msg_v01, dmsReq);
        memset (dataPtr, 0, sizeof(dms_swi_dssd_read_resp_msg_v01));

        /* Read the SSDATA */
        qmi_client_error_type rc = qmi_client_send_msg_sync (DmsClient,
                                                             QMI_DMS_SWI_DSSD_READ_REQ_V01,
                                                             &dmsReq,
                                                             sizeof(dms_swi_dssd_read_req_msg_v01),
                                                             dataPtr,
                                                             sizeof(dms_swi_dssd_read_resp_msg_v01),
                                                             COMM_DEFAULT_PLATFORM_TIMEOUT);
        LE_DEBUG ("rc %d, dmsResp.resp.result %d, dmsResp.resp.error %d",
                    rc, dataPtr->resp.result, dataPtr->resp.error);
        /* Check the response */
        if ( swiQmi_CheckResponse (STRINGIZE_EXPAND(QMI_DMS_SWI_DSSD_READ_REQ_V01),
                                    rc,
                                    dataPtr->resp.result,
                                    dataPtr->resp.error) != LE_OK)
        {
            LE_ERROR ("Error on reading SSDATA: rc %d, resp result %d, resp error %d",
                        rc, dataPtr->resp.result, dataPtr->resp.error);
        }
        else
        {
            LE_DEBUG ("ssid_modem_idx 0x%x", dataPtr->ssid_modem_idx);
            LE_DEBUG ("ssid_lk_idx 0x%x", dataPtr->ssid_lk_idx);
            LE_DEBUG ("ssid_linux_idx 0x%x", dataPtr->ssid_linux_idx);
            LE_DEBUG ("swap_reason 0x%x", dataPtr->swap_reason);
            LE_DEBUG ("out_of_sync 0x%x", dataPtr->out_of_sync);
            LE_DEBUG ("sw_update_state 0x%x", dataPtr->sw_update_state);
            result = LE_OK;
        }
    }
    LE_DEBUG ("return %d", result);
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Send a SSDATA read request: QMI message
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
static le_result_t WriteSsdata
(
    const dms_swi_dssd_write_req_msg_v01* dmsReqPtr   ///< [IN] SSDATA structure to be written
)
{
    le_result_t result = LE_FAULT;

    if (sizeof (*dmsReqPtr) == sizeof (dms_swi_dssd_write_req_msg_v01) )
    {
        SWIQMI_DECLARE_MESSAGE (dms_swi_dssd_write_resp_msg_v01, dmsResp);
        qmi_client_error_type rc;

        /* Send request to FW */
        rc = qmi_client_send_msg_sync (DmsClient,
                                        QMI_DMS_SWI_DSSD_WRITE_REQ_V01,
                                        (void*)dmsReqPtr, sizeof(*dmsReqPtr),
                                        &dmsResp, sizeof(dmsResp),
                                        COMM_PLATFORM_TIMEOUT_60_SECONDS);

        LE_DEBUG ("rc %d, resp result %d, resp error %d",
                    rc, dmsResp.resp.result, dmsResp.resp.error);
        result = swiQmi_CheckResponse (STRINGIZE_EXPAND (QMI_DMS_SWI_DSSD_WRITE_REQ_V01),
                                        rc,
                                        dmsResp.resp.result,
                                        dmsResp.resp.error);

        if (result != LE_OK)
        {
            LE_ERROR ("Error on writting SSDATA: rc %d, resp result %d, resp error %d",
                        rc, dmsResp.resp.result, dmsResp.resp.error);
        }
    }

    LE_DEBUG ("return %d", result);
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Update some variables in SSDATA to indicate that systems are synchronized
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
static le_result_t SetOutOfSyncState
(
    bool isSync ///< [IN] Set to true if systems are synchronized
)
{
    le_result_t result = LE_FAULT;
    SWIQMI_DECLARE_MESSAGE (dms_swi_dssd_write_req_msg_v01, dmsReq);

    dmsReq.out_of_sync_valid = true;
    if(isSync)
    {
        /* Set the "Out of Sync" field in SSDATA to "sync" */
        dmsReq.out_of_sync = OUT_OF_SYNC_OFF;

        /* Set the sw update state to normal */
        dmsReq.sw_update_state = 1;
        dmsReq.sw_update_state_valid = true;
    }
    else
    {
        /* Set the "Out of Sync" field in SSDATA to "OoS" */
        dmsReq.out_of_sync = OUT_OF_SYNC_ON;
    }
    dmsReq.out_of_sync_valid = true;

    result = WriteSsdata (&dmsReq);
    if (result != LE_OK)
    {
        LE_ERROR ("Error on setting out_of_sync state");
    }

    LE_DEBUG ("result %d", result);
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * request the modem to program a NVUP file in UD system
 *
 * @return
 *      - LE_OK             on success
 *      - LE_BAD_PARAMETER  bad parameter
 *      - LE_FAULT          on failure
 */
//--------------------------------------------------------------------------------------------------
static le_result_t NvupProgram
(
    size_t *lengthPtr,                  ///< [INOUT] data length, will be decreased by the
                                        ///< amount of data sent
    const uint8_t* dataPtr,             ///< [IN] input data
    bool isMore                         ///< [IN] flag to indicate that more data are following
)
{
    le_result_t result;
    qmi_client_error_type rc;
    size_t txSize;

    if ((dataPtr == NULL) || (lengthPtr == NULL))
    {
        LE_ERROR("bad parameter");
        return LE_BAD_PARAMETER;
    }

    SWIQMI_DECLARE_MESSAGE (swi_m2m_nvup_program_req_msg_v01, mgsReq);
    SWIQMI_DECLARE_MESSAGE (swi_m2m_nvup_program_resp_msg_v01, mgsResp);

    txSize = (*lengthPtr >= QMI_SWI_DATA_SIZE_V01) ? QMI_SWI_DATA_SIZE_V01 : *lengthPtr;
    memcpy(mgsReq.data, dataPtr, txSize);
    mgsReq.data_len = txSize;
    mgsReq.is_more = isMore;

    LE_DEBUG("txSize=%d isMore=%d", txSize, isMore);

    /* Send request to FW */
    rc = qmi_client_send_msg_sync (MgsClient,
                                   QMI_SWI_M2M_NVUP_PROGRAM_REQ_V01,
                                   &mgsReq, sizeof(mgsReq),
                                   &mgsResp, sizeof(mgsResp),
                                   COMM_LONG_PLATFORM_TIMEOUT);

    LE_DEBUG ("rc %d, resp result %d, resp error %d", rc, mgsResp.resp.result, mgsResp.resp.error);

    result = swiQmi_CheckResponse (STRINGIZE_EXPAND (QMI_SWI_M2M_NVUP_PROGRAM_REQ_V01),
                                   rc,
                                   mgsResp.resp.result,
                                   mgsResp.resp.error);

    if (result != LE_OK)
    {
        LE_ERROR ("Error on programming NVUP files: rc %d, resp result %d, resp error %d op_result %d",
                  rc, mgsResp.resp.result, mgsResp.resp.error, mgsResp.op_result);
    }
    else
    {
        *lengthPtr -= txSize;
    }

    LE_DEBUG ("return %d", result);
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
 *      - LE_NOT_FOUND if bootloader version is not available; firmware version is always available.
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

    // There is no request message, only a response message.
    dms_swi_get_firmware_status_resp_msg_v01 fwResp = { {0} };

    rc = qmi_client_send_msg_sync(
            DmsClient,
            QMI_DMS_SWI_GET_FWUPDATE_STATUS_REQ_V01,
            NULL, 0,
            &fwResp, sizeof(fwResp),
            COMM_DEFAULT_PLATFORM_TIMEOUT);

    le_result_t result = swiQmi_CheckResponseCode(
                                STRINGIZE_EXPAND(QMI_DMS_SWI_GET_FWUPDATE_STATUS_REQ_V01),
                                rc, fwResp.resp);
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
 * Handler function called when there is a bad image indication.
 */
//--------------------------------------------------------------------------------------------------
static void BadImageHandler
(
    void*           indBufPtr,  ///< [IN] The indication message data.
    unsigned int    indBufLen , ///< [IN] The indication message data length in bytes.
    void*           contextPtr  ///< [IN] Context value that was passed to
                                ///       swiQmi_AddIndicationHandler().
)
{
    static uint64_t BadImage = 0;

    if (IsUpdateOngoing)
    {
        LE_DEBUG("Update is ongoing, do not report bad image");
        return;
    }

    LE_DEBUG("notification");

    dms_swi_bad_image_ind_msg_v01 dmsReport;

    qmi_client_error_type clientMsgErr = qmi_client_message_decode(
        DmsClient, QMI_IDL_INDICATION, QMI_DMS_SWI_BAD_IMAGE_IND_V01,
        indBufPtr, indBufLen,
        &dmsReport, sizeof(dmsReport));

    if (clientMsgErr != QMI_NO_ERR)
    {
        LE_ERROR("Error in decoding QMI_DMS_SWI_BAD_IMAGE_IND_V01, error = %i", clientMsgErr);
        return;
    }

    uint64_t newBadImage = (dmsReport.bad_image ^ BadImage) & ~BadImage;
    char const *badImageNamePtr;
    int idx = 1; // start to one to be compatible with pa_fwupdate_GetUpdateStatusLabel()
    BadImage = dmsReport.bad_image;
    // Report all new bad images
    while (newBadImage)
    {
        if (newBadImage & 1)
        {
            // Get the corrupted image name
            badImageNamePtr = pa_fwupdate_GetUpdateStatusLabel(idx);
            LE_DEBUG("report bad image %s, string size %d",badImageNamePtr,
                     strlen(badImageNamePtr));
            le_event_Report((le_event_Id_t)contextPtr,(void*)badImageNamePtr,
                            strlen(badImageNamePtr));
        }
        newBadImage >>= 1;
        idx++;
    }
}


//==================================================================================================
//  PUBLIC API FUNCTIONS
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/**
 * Get the firmware version string
 *
 * @return
 *      - LE_OK            on success
 *      - LE_NOT_FOUND     if the version string is not available
 *      - LE_OVERFLOW      if version string to big to fit in provided buffer
 *      - LE_BAD_PARAMETER bad parameter
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
 *      - LE_OK            on success
 *      - LE_NOT_FOUND     if the version string is not available
 *      - LE_OVERFLOW      if version string to big to fit in provided buffer
 *      - LE_BAD_PARAMETER bad parameter
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
 *      - LE_OK            on success
 *      - LE_NOT_FOUND     if the version string is not available
 *      - LE_OVERFLOW      if version string to big to fit in provided buffer
 *      - LE_BAD_PARAMETER bad parameter
 *      - LE_FAULT for any other errors
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_fwupdate_GetAppBootloaderVersion
(
    char* versionPtr,        ///< [OUT] Firmware version string
    size_t versionSize       ///< [IN] Size of version buffer
)
{
    return (GetAppBootloaderVersion(versionPtr, versionSize));
}

//--------------------------------------------------------------------------------------------------
/**
 * Function which indicates if Active and Update systems are synchronized
 *
 * @return
 *      - LE_OK            on success
 *      - LE_UNSUPPORTED   the feature is not supported
 *      - LE_BAD_PARAMETER bad parameter
 *      - LE_FAULT         else
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_fwupdate_GetSystemState
(
    bool *isSyncPtr ///< Indicates if both systems are synchronized
)
{
    le_result_t result;

    if (isSyncPtr == NULL)
    {
        LE_ERROR("Bad parameter");
        return LE_BAD_PARAMETER;
    }

    *isSyncPtr = false;

    SWIQMI_DECLARE_MESSAGE (dms_swi_dssd_read_resp_msg_v01, dmsResp);
    result = ReadSsdata (&dmsResp);
    if (result == LE_OK)
    {
        LE_DEBUG ("QMI SSDATA READ");
        LE_DEBUG ("ssid_modem_idx 0x%x", dmsResp.ssid_modem_idx);
        LE_DEBUG ("ssid_lk_idx 0x%x", dmsResp.ssid_lk_idx);
        LE_DEBUG ("ssid_linux_idx 0x%x", dmsResp.ssid_linux_idx);
        LE_DEBUG ("swap_reason %d", dmsResp.swap_reason);
        LE_DEBUG ("out_of_sync 0x%x", dmsResp.out_of_sync);
        LE_DEBUG ("sw_update_state %d", dmsResp.sw_update_state);
        LE_DEBUG ("bad_image 0x%llx", dmsResp.bad_image);

        /* bad_image value will be updated later */
        SsdataBadImage = dmsResp.bad_image;

        if (OUT_OF_SYNC_OFF == dmsResp.out_of_sync)
        {
            /* Means dual system is sync */
            *isSyncPtr = true;
        }
        else if (OUT_OF_SYNC_ON == dmsResp.out_of_sync)
        {
            /* Means dual system is out of sync */
            *isSyncPtr = false;
        }
        else
        {
            /* Not a known value: we assume the OoS state */
            LE_ERROR ("Bad Out of Sync value 0x%x - Assuming Out-of-Sync", dmsResp.out_of_sync);
            *isSyncPtr = false;
        }
    }
    else
    {
        LE_ERROR ("Error on QMI response %d", result);
    }

    LE_DEBUG ("result %d, isSync %d", result, *isSyncPtr);
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Update some variables in SSDATA to indicate that systems are not synchronized
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_fwupdate_SetUnsyncState
(
    void
)
{
    le_result_t result = SetOutOfSyncState(false);

    LE_DEBUG ("result %d", result);
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Update some variables in SSDATA to indicate that systems are synchronized
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_fwupdate_SetSyncState
(
    void
)
{
    le_result_t result = SetOutOfSyncState(true);

    LE_DEBUG ("result %d", result);
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Program the partitions to become active and update systems
 *
 * @return
 *      - LE_OK             on success
 *      - LE_UNSUPPORTED    the feature is not supported
 *      - LE_FAULT          on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_fwupdate_SetActiveSystem
(
    pa_fwupdate_System_t systemArray[PA_FWUPDATE_SUBSYSID_MAX],
                         ///< [IN] System array for "modem/lk/linux" partition groups
    bool isSyncReq       ///< [IN] Indicate if a synchronization is requested after the swap
)
{
    le_result_t result;
    SWIQMI_DECLARE_MESSAGE(dms_swi_dssd_write_req_msg_v01, writeReq);
    int iSSid;

    for (iSSid = PA_FWUPDATE_SUBSYSID_MODEM; iSSid < PA_FWUPDATE_SUBSYSID_MAX; iSSid++)
    {
        switch (systemArray[iSSid])
        {
            case PA_FWUPDATE_SYSTEM_1:
            case PA_FWUPDATE_SYSTEM_2:
                    break;
            default:
                    LE_ERROR("Unknown SSId value: 0%02hhX, field %d", systemArray[iSSid], iSSid);
                    result = LE_FAULT;
                    goto end;
        }
    }

    writeReq.ssid_modem_idx = systemArray[PA_FWUPDATE_SUBSYSID_MODEM];
    writeReq.ssid_lk_idx = systemArray[PA_FWUPDATE_SUBSYSID_LK];
    writeReq.ssid_linux_idx = systemArray[PA_FWUPDATE_SUBSYSID_LINUX];

    LE_INFO("New SSId %d,%d,%d",
            writeReq.ssid_modem_idx, writeReq.ssid_lk_idx, writeReq.ssid_linux_idx);

    /* write the new boot_system */
    writeReq.ssid_modem_idx_valid = true;
    writeReq.ssid_lk_idx_valid = true;
    writeReq.ssid_linux_idx_valid = true;

    /* Set the swap reason: Swap triggered by Legato API */
    writeReq.swap_reason = 4;
    writeReq.swap_reason_valid = true;

    /* Check if the request is a simple SWAP or a SWAP&Sync */
    if (isSyncReq == true)
    {
        /* SWAP&SYNC: Set the sw update state: SYNC */
        writeReq.sw_update_state = 2;
        writeReq.sw_update_state_valid = true;
    }

    result = WriteSsdata (&writeReq);
    if (result != LE_OK)
    {
        LE_ERROR("Cannot write SSDATA");
        result = LE_FAULT;
        goto end;
    }

    result = LE_OK;
end:
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function which indicates if a Sync operation is needed (swap & sync operation)
 *
 * @return
 *      - LE_OK            On success
 *      - LE_UNSUPPORTED   The feature is not supported
 *      - LE_BAD_PARAMETER bad parameter
 *      - LE_FAULT         Else
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_fwupdate_DualSysCheckSync
(
    bool *isSyncReqPtr ///< [OUT] Indicates if synchronization is requested
)
{
    le_result_t result;

    if (isSyncReqPtr == NULL)
    {
        LE_ERROR("Bad parameter");
        return LE_BAD_PARAMETER;
    }

    *isSyncReqPtr = false;

    SWIQMI_DECLARE_MESSAGE (dms_swi_dssd_read_resp_msg_v01, dmsResp);
    result = ReadSsdata (&dmsResp);
    if (result == LE_OK)
    {
        LE_DEBUG ("QMI SSDATA READ");
        LE_DEBUG ("ssid_modem_idx 0x%x", dmsResp.ssid_modem_idx);
        LE_DEBUG ("ssid_lk_idx 0x%x", dmsResp.ssid_lk_idx);
        LE_DEBUG ("ssid_linux_idx 0x%x", dmsResp.ssid_linux_idx);
        LE_DEBUG ("swap_reason %d", dmsResp.swap_reason);
        LE_DEBUG ("out_of_sync 0x%x", dmsResp.out_of_sync);
        LE_DEBUG ("sw_update_state %d", dmsResp.sw_update_state);
        LE_DEBUG ("bad_image 0x%llx", dmsResp.bad_image);

        /* Check from SSDATA if the SW update state is equal to "SYNC" value (2)
         * In this case a SYNC operation needs to be performed
         */
        if (dmsResp.sw_update_state == 2)
        {
            /* Means Sync and swap state from active system to inactive system */
            *isSyncReqPtr = true;
        }
        else
        {
            *isSyncReqPtr = false;
        }

        /* Check the swap reason: if it's 4 (Legato API),
         * this needs to be set to 0 (No swap since power up)
         */
        if (dmsResp.swap_reason == 4)
        {
            /* Swap reason needs to be updated to 0 (No swap since power up) */
            SWIQMI_DECLARE_MESSAGE (dms_swi_dssd_write_req_msg_v01, dmsReq);

            dmsReq.swap_reason_valid = true;
            dmsReq.swap_reason = 0;
            result = WriteSsdata (&dmsReq);
            if (result != LE_OK)
            {
                LE_ERROR ("error on writing SSDATA for swap reason");
            }
        }
    }
    else
    {
        LE_ERROR ("Error on QMI response %d", result);
    }

    LE_DEBUG ("result %d, isSync %d", result, *isSyncReqPtr);
    return result;
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
    le_result_t result;

    if ((state == 0) || (state >= PA_FWUPDATE_STATE_INVALID))
    {
        LE_ERROR("Bad state value (%d)", state);
        result = LE_FAULT;
        goto end;
    }

    SWIQMI_DECLARE_MESSAGE (dms_swi_dssd_write_req_msg_v01, dmsReq);

    /* Set SW update state to Sync: 2 */
    dmsReq.sw_update_state = state;
    dmsReq.sw_update_state_valid = true;

    result = WriteSsdata (&dmsReq);
    if (result != LE_OK)
    {
        LE_ERROR ("Error on setting sw_update_state");
    }

end:
    LE_DEBUG ("result %d", result);
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * request the modem to delete the NVUP files in UD system
 *
 * @return
 *      - LE_OK             on success
 *      - LE_UNSUPPORTED    the feature is not supported
 *      - LE_FAULT          on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_fwupdate_NvupDelete
(
    void
)
{
    le_result_t result;
    qmi_client_error_type rc;

    SWIQMI_DECLARE_MESSAGE (swi_m2m_nvup_delete_resp_msg_v01, mgsResp);

    /* Send request to FW */
    rc = qmi_client_send_msg_sync (MgsClient,
                                   QMI_SWI_M2M_NVUP_DELETE_REQ_V01,
                                   NULL, 0,
                                   &mgsResp, sizeof(mgsResp),
                                   COMM_LONG_PLATFORM_TIMEOUT);

    LE_DEBUG ("rc %d, resp result %d, resp error %d", rc, mgsResp.resp.result, mgsResp.resp.error);

    result = swiQmi_CheckResponse (STRINGIZE_EXPAND (QMI_SWI_M2M_NVUP_DELETE_REQ_V01),
                                   rc,
                                   mgsResp.resp.result,
                                   mgsResp.resp.error);

    if (result != LE_OK)
    {
        LE_ERROR ("Error on deleting NVUP files: rc %d, resp result %d, resp error %d",
                  rc, mgsResp.resp.result, mgsResp.resp.error);
    }

    LE_DEBUG ("return %d", result);
    return result;

}

//--------------------------------------------------------------------------------------------------
/**
 * Write a NVUP file in UD system
 *
 * @return
 *      - LE_OK             on success
 *      - LE_UNSUPPORTED    the feature is not supported
 *      - LE_FAULT          on failure
 *      - others            Depending of the underlying operations
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_fwupdate_NvupWrite
(
    size_t length,                      ///< [IN] data length
    const uint8_t* dataPtr,             ///< [IN] input data
    bool isEnd                          ///< [IN] flag to indicate the end of the file
)
{
    le_result_t result;
    size_t prevLength;

    do
    {
        bool isMore = true;

        if (isEnd && (length <= QMI_SWI_DATA_SIZE_V01))
        {
            isMore = false;
        }
        LE_DEBUG("length=%d data=0x%x isMore=%d", length, (unsigned int)dataPtr, isMore);
        prevLength = length;
        result = NvupProgram(&length, dataPtr, isMore);
        if (result == LE_OK)
        {
            dataPtr += (prevLength - length);
        }
    }
    while ((length > 0) && (result == LE_OK));

    return result;
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
    le_result_t result;
    qmi_client_error_type rc;

    SWIQMI_DECLARE_MESSAGE (swi_m2m_nvup_apply_resp_msg_v01, mgsResp);

    /* Send request to FW */
    rc = qmi_client_send_msg_sync (MgsClient,
                                   QMI_SWI_M2M_NVUP_APPLY_REQ_V01,
                                   NULL, 0,
                                   &mgsResp, sizeof(mgsResp),
                                   COMM_LONG_PLATFORM_TIMEOUT);

    LE_DEBUG ("rc %d, resp result %d, resp error %d", rc, mgsResp.resp.result, mgsResp.resp.error);

    result = swiQmi_CheckResponse (STRINGIZE_EXPAND (QMI_SWI_M2M_NVUP_APPLY_REQ_V01),
                                   rc,
                                   mgsResp.resp.result,
                                   mgsResp.resp.error);

    if (result != LE_OK)
    {
        LE_ERROR ("Error on applying NVUP files: rc %d, resp result %d, resp error %d op_result %d",
                  rc, mgsResp.resp.result, mgsResp.resp.error, mgsResp.op_result);
    }

    LE_DEBUG ("return %d", result);
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the firmware update status label
 *
 * @return
 *      - The address of the FW update status description matching the given status.
 *      - NULL if the given status is invalid.
 */
//--------------------------------------------------------------------------------------------------
const char *pa_fwupdate_GetUpdateStatusLabel
(
    pa_fwupdate_InternalStatus_t status    ///< [IN] Firmware update status
)
{
    const char *FwUpdateStatusLabel[] =
    {
        "No bad image found",               // PA_FWUPDATE_INTERNAL_STATUS_OK
        "sbl",                              // PA_FWUPDATE_INTERNAL_STATUS_SBL
        "mibib",                            // PA_FWUPDATE_INTERNAL_STATUS_MIBIB
        "Reserved1",                        // PA_FWUPDATE_INTERNAL_STATUS_RESERVED1
        "sedb"     ,                        // PA_FWUPDATE_INTERNAL_STATUS_SEDB
        "Reserved2",                        // PA_FWUPDATE_INTERNAL_STATUS_RESERVED2
        "tz_1",                             // PA_FWUPDATE_INTERNAL_STATUS_TZ1
        "tz_2",                             // PA_FWUPDATE_INTERNAL_STATUS_TZ2
        "rpm_1",                            // PA_FWUPDATE_INTERNAL_STATUS_RPM1
        "rpm_2",                            // PA_FWUPDATE_INTERNAL_STATUS_RPM2
        "modem_1",                          // PA_FWUPDATE_INTERNAL_STATUS_MODEM1
        "modem_2",                          // PA_FWUPDATE_INTERNAL_STATUS_MODEM2
        "aboot_1",                          // PA_FWUPDATE_INTERNAL_STATUS_LK1
        "aboot_2",                          // PA_FWUPDATE_INTERNAL_STATUS_LK2
        "boot_1",                           // PA_FWUPDATE_INTERNAL_STATUS_KERNEL1
        "boot_2",                           // PA_FWUPDATE_INTERNAL_STATUS_KERNEL2
        "system_1",                         // PA_FWUPDATE_INTERNAL_STATUS_ROOT_FS1
        "system_2",                         // PA_FWUPDATE_INTERNAL_STATUS_ROOT_FS2
        "lefwkro_1",                        // PA_FWUPDATE_INTERNAL_STATUS_USER_DATA1
        "lefwkro_2",                        // PA_FWUPDATE_INTERNAL_STATUS_USER_DATA2
        "customer0",                        // PA_FWUPDATE_INTERNAL_STATUS_CUST_APP1
        "customer1",                        // PA_FWUPDATE_INTERNAL_STATUS_CUST_APP2
        "Download in progress",             // PA_FWUPDATE_INTERNAL_STATUS_DWL_ONGOING
        "Download failed",                  // PA_FWUPDATE_INTERNAL_STATUS_DWL_FAILED
        "Download timeout",                 // PA_FWUPDATE_INTERNAL_STATUS_DWL_TIMEOUT
        "Unknown status"                    // PA_FWUPDATE_INTERNAL_STATUS_UNKNOWN
    };

    // Check parameters
    if (status > PA_FWUPDATE_INTERNAL_STATUS_UNKNOWN)
    {
        LE_ERROR("Invalid status parameter (%d)!", (int)status);
        // Always return a status label.
        status = PA_FWUPDATE_INTERNAL_STATUS_UNKNOWN;
    }

    // Point to the matching label.
    return FwUpdateStatusLabel[status];
}

//--------------------------------------------------------------------------------------------------
/**
 * Return the last internal update status.
 *
 * @return
 *      - LE_OK on success
 *      - LE_BAD_PARAMETER Invalid parameter
 *      - LE_FAULT on failure
 *      - LE_UNSUPPORTED not supported
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_fwupdate_GetInternalUpdateStatus
(
    pa_fwupdate_InternalStatus_t *statusPtr, ///< [OUT] Returned update status
    char *statusLabelPtr,                    ///< [OUT] String matching the status
    size_t statusLabelLength                 ///< [IN] Maximum length of the status description
)
{
    uint32_t                       fwUpdateResult;
    uint8_t                        imageType;
    uint32_t                       refData;
    uint32_t                       refStringLength = QMI_DMS_FWUPDATERESTRLEN_V01;
    char                           refString[QMI_DMS_FWUPDATERESTRLEN_V01];
    le_result_t                    result          = LE_BAD_PARAMETER;
    dms_swi_dssd_read_resp_msg_v01 ssdata;

    const char *labelPtr = NULL;

    const char *SwapReasonLabel[] =
    {
        "No swap since power up",
        "Swap due to bad image",
        "Swap due to normal SW update",
        "Swap triggered by AT command",
        "Swap triggered by Legato API"
    };

    const char *SwUpdateStateLabel[] =
    {
        "Normal state",
        "Sync state between system 1 and system 2",
        "Sync and swap state from active system to inactive system",
        "SW recovery phase 1",
        "SW recovery phase 2"
    };

    const char *EdbSwUpdateLabel[] =
    {
        "EDB flag is cleared",
        "EDB in system 1",
        "EDB in system 2"
    };

    // Check the parameter
    if (NULL == statusPtr)
    {
        LE_ERROR("Invalid parameter.");
        return LE_BAD_PARAMETER;
    }

    result = ReadSsdata(&ssdata);
    if (LE_OK != result)
    {
        LE_ERROR("Unable to access SSDATA!");
        if (NULL != statusLabelPtr)
        {
            if (statusLabelLength > 0)
            {
                // Update the status label
                strncpy(statusLabelPtr,
                    pa_fwupdate_GetUpdateStatusLabel(PA_FWUPDATE_INTERNAL_STATUS_UNKNOWN),
                    statusLabelLength);
            }
            else
            {
                // At least, reset the label
                *statusLabelPtr = '\0';
            }
        }
        return result;
    }

    /* swap_reason */
    if (NUM_ARRAY_MEMBERS(SwapReasonLabel) > ssdata.swap_reason)
    {
        LE_INFO("%s", SwapReasonLabel[ssdata.swap_reason]);
    }
    else
    {
        LE_ERROR("Invalid swap reason!");
    }

    /* sw_update_state */
    if (NUM_ARRAY_MEMBERS(SwUpdateStateLabel) > ssdata.sw_update_state)
    {
        LE_INFO("%s", SwUpdateStateLabel[ssdata.sw_update_state]);
    }
    else
    {
        LE_ERROR("Invalid SW update state!");
    }

    /* out_of_sync */
    switch (ssdata.out_of_sync)
    {
        case OUT_OF_SYNC_CLEARED:
            LE_INFO("Out of sync is cleared.");
            break;
        case OUT_OF_SYNC_OFF:
            LE_INFO("Dual system is synchronized.");
            break;
        case OUT_OF_SYNC_ON:
            LE_INFO("Dual system is out of synchronization.");
            break;
        default:
            LE_ERROR("Invalid out of sync value!");
            break;
    }

    /* efs_corruption_in_sw_update */
    switch (ssdata.efs_corruption_in_sw_update)
    {
        case EFS_STATUS_OK:
            LE_INFO("EFS is healthy.");
            break;
        case EFS_STATUS_CORRUPTED:
            LE_WARN("EFS is corrupted!");
            break;
        default:
            LE_ERROR("Invalid EFS status!");
            break;
    }

    /* edb_in_sw_update */
    if (ssdata.edb_in_sw_update < NUM_ARRAY_MEMBERS(EdbSwUpdateLabel))
    {
        LE_INFO("%s", EdbSwUpdateLabel[ssdata.edb_in_sw_update]);
    }
    else
    {
        LE_ERROR("Invalid EDB value!");
    }

    // Everything is fine
    *statusPtr = PA_FWUPDATE_INTERNAL_STATUS_OK;
    result = LE_OK;

    /* Check whether a bad image is present */
    if (1 == ssdata.swap_reason)
    {
        if (0 != ssdata.bad_image)
        {
            // Determine the biggest mask possible
            uint64_t bad_mask  = (1 << (PA_FWUPDATE_INTERNAL_STATUS_CUST_APP2 + 1));
            uint64_t bad_image = ssdata.bad_image;

            LE_INFO("Bad image bit mask            = 0x%016jX", bad_image);
            LE_INFO("Supported image type bit mask = 0x%016jX", bad_mask - 1);

            // Check the validity of the mask
            if (bad_image >= bad_mask)
            {
                LE_ERROR("Unknown bad image type!");
                result = LE_FAULT;
            }
            else
            {
                pa_fwupdate_InternalStatus_t status = *statusPtr;

                // Looking for the first corrupted image
                while (0 == ((bad_image >> status) & 1))
                {
                    status++;
                }
                // Adjust the index to match the enum range of values
                status++;
                // Get the corrupted image id
                labelPtr = pa_fwupdate_GetUpdateStatusLabel(status);
                LE_INFO("Found bad image (%s).", labelPtr);
                // Update the status
                *statusPtr = status;
            }
        }
        else
        {
            LE_INFO("Inconsistency between swap reason (%X) and bad image (%jX)!",
                ssdata.swap_reason, ssdata.bad_image);
        }
    }
    else
    {
        // No bad image declared
        // Still need to crosscheck the consistency
        if (0 != ssdata.bad_image)
        {
            LE_INFO("Inconsistency between swap reason (%X) and bad image (%jX)!",
                ssdata.swap_reason, ssdata.bad_image);
        }
        else
        {
            // Just to be sure
            if (*statusPtr != PA_FWUPDATE_INTERNAL_STATUS_OK)
            {
                LE_CRIT("Update status should be OK!");
                if (*statusPtr > PA_FWUPDATE_INTERNAL_STATUS_UNKNOWN)
                {
                    LE_CRIT("Update status (%d) seems to be corrupted!", *statusPtr);
                }
                else
                {
                    LE_CRIT("Update status (%s) is inconsistent!",
                            pa_fwupdate_GetUpdateStatusLabel(*statusPtr));
                }
                // Reset to unknown
                *statusPtr = PA_FWUPDATE_INTERNAL_STATUS_UNKNOWN;
                // Update the label accordingly
                labelPtr = pa_fwupdate_GetUpdateStatusLabel(*statusPtr);
                LE_CRIT("Update status reset: %s", labelPtr);
            }
        }
    }

    if (LE_OK == GetFwUpdateStatus(
         &fwUpdateResult, &imageType, &refData, refStringLength, refString))
    {
        const char *errorIdPtr[] =
            {
                "UNKWOWN (reset or power off)", "FILE", "NVUP", "FOTA", "FDT/SSDP"
            };
        uint32_t errorIndex = 0;
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
                LE_INFO("FW update status: OK");
                break;
            case FWUPDATE_STATUS_FDT_SSDP_ERROR:
                errorIndex++;
                // No break
            case FWUPDATE_STATUS_FOTA_ERROR:
                errorIndex++;
                // No break
            case FWUPDATE_STATUS_NVUP_ERROR:
                errorIndex++;
                // No break
            case FWUPDATE_STATUS_FILE_ERROR:
                errorIndex++;
                // No break
            case FWUPDATE_STATUS_UNKNOWN:
                LE_INFO("FW update status: %s update error (0x%08X)",
                    errorIdPtr[errorIndex], fwUpdateResult);
                LE_INFO("FW update image type: 0x%02X", imageType);
                LE_INFO("FW update reference data: 0x%08X", refData);
                if (0 < refStringLength)
                {
                    LE_INFO("FW update reference id: %s", refString);
                }
                else
                {
                    LE_INFO("FW update reference id: undefined");
                }
                break;
            default:
                LE_INFO("FW update status: INVALID");
                break;
        }
    }

    if (LE_OK == result)
    {
        // Update status label accordingly
        if (NULL != statusLabelPtr)
        {
            if ((statusLabelLength > 0) && (NULL != labelPtr))
            {
                // Update the status label
                strncpy(statusLabelPtr, labelPtr, statusLabelLength);
            }
            else
            {
                // At least, reset the label
                *statusLabelPtr = '\0';
            }
        }
        LE_INFO("Status Label : %s", labelPtr ? labelPtr : "<NULL>");
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Request the flash access for a SW update
 *
 * @return
 *      - LE_OK          on success
 *      - LE_UNAVAILABLE the flash access is not granted for SW update
 *      - LE_FAULT       on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_fwupdate_RequestUpdate
(
    void
)
{
    le_result_t result;
    qmi_client_error_type rc;

    SWIQMI_DECLARE_MESSAGE (swi_m2m_sw_update_request_resp_msg_v01, mgsResp);

    /* Send request to FW */
    rc = qmi_client_send_msg_sync (MgsClient,
                                   QMI_SWI_M2M_SW_UPDATE_REQUEST_REQ_V01,
                                   NULL, 0,
                                   &mgsResp, sizeof(mgsResp),
                                   COMM_LONG_PLATFORM_TIMEOUT);

    LE_DEBUG ("rc %d, resp result %d, resp error %d", rc, mgsResp.resp.result, mgsResp.resp.error);

    result = swiQmi_CheckResponse (STRINGIZE_EXPAND (QMI_SWI_M2M_SW_UPDATE_REQUEST_RESP_V01),
                                   rc,
                                   mgsResp.resp.result,
                                   mgsResp.resp.error);

    if (result != LE_OK)
    {
        LE_ERROR ("Error on SW update request response: rc %d, resp result %d, resp error %d",
                  rc, mgsResp.resp.result, mgsResp.resp.error);
    }
    else
    {
        LE_INFO("SW update request response -> SW update allow: %hhd", mgsResp.sw_update_allow);
        result = (mgsResp.sw_update_allow ? LE_OK : LE_UNAVAILABLE);
    }

    if (LE_OK == result)
    {
        IsUpdateOngoing = true;
    }

    LE_DEBUG ("return %d", result);
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Release the flash access after a SW update
 *
 * @return
 *      - LE_OK    on success
 *      - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_fwupdate_CompleteUpdate
(
    void
)
{
    le_result_t result;
    qmi_client_error_type rc;

    SWIQMI_DECLARE_MESSAGE (swi_m2m_sw_update_complete_resp_msg_v01, mgsResp);

    /* Send request to FW */
    rc = qmi_client_send_msg_sync (MgsClient,
                                   QMI_SWI_M2M_SW_UPDATE_COMPLETE_REQ_V01,
                                   NULL, 0,
                                   &mgsResp, sizeof(mgsResp),
                                   COMM_LONG_PLATFORM_TIMEOUT);

    LE_DEBUG ("rc %d, resp result %d, resp error %d", rc, mgsResp.resp.result, mgsResp.resp.error);

    result = swiQmi_CheckResponse (STRINGIZE_EXPAND (QMI_SWI_M2M_SW_UPDATE_COMPLETE_RESP_V01),
                                   rc,
                                   mgsResp.resp.result,
                                   mgsResp.resp.error);

    if (result != LE_OK)
    {
        LE_ERROR ("Error on SW update complete response: rc %d, resp result %d, resp error %d",
                  rc, mgsResp.resp.result, mgsResp.resp.error);
    }

    IsUpdateOngoing = false;

    LE_DEBUG ("return %d", result);
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set bad image flag preventing concurrent partition access
 *
 * @return
 *      - LE_OK             on success
 *      - LE_FAULT          on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_fwupdate_SetBadImage
(
    uint64_t badImageMask,  ///< [IN] image to be written according to bitmask
    bool isBad              ///< [IN] true to set bad image flag, false to clear it
)
{
    le_result_t result = LE_FAULT;
    SWIQMI_DECLARE_MESSAGE(dms_swi_dssd_write_req_msg_v01, dmsReq);

    dmsReq.bad_image = SsdataBadImage;
    dmsReq.bad_image_valid = true;

    dmsReq.bad_image = isBad ? (dmsReq.bad_image | badImageMask)
                             : (DS_IMAGE_FLAG_UP | badImageMask);

    result = WriteSsdata(&dmsReq);
    if (LE_OK != result)
    {
        LE_ERROR("Failed to %s bad image 0x%llx", isBad ? "set":"clear", badImageMask);
    }
    else
    {
        SsdataBadImage = isBad ? dmsReq.bad_image
                               : (SsdataBadImage & (~badImageMask));
        LE_DEBUG("%s bad image 0x%llx SsdataBadImage 0x%llx", isBad ? "Set":"Cleared", badImageMask,
                 SsdataBadImage);
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Start the bad image indication
 *
 * @return
 *      - LE_OK             on success
 *      - LE_FAULT          on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_fwupdate_StartBadImageIndication
(
    le_event_Id_t eventId       ///< the event Id to use to report the bad image
)
{
    le_result_t result;

    LE_DEBUG("start");

    if (BadImageHandlerCounter)
    {
        LE_DEBUG("already started");
        BadImageHandlerCounter++;
        LE_DEBUG("BadImageHandlerCounter = %d", BadImageHandlerCounter);
        return LE_OK;
    }

    result = swiQmi_AddIndicationHandler(BadImageHandler,
                                         QMI_SERVICE_DMS,
                                         QMI_DMS_SWI_BAD_IMAGE_IND_V01,
                                         (void*)eventId);

    if (result != LE_OK)
    {
        LE_ERROR("Failed to add bad image handler");
    }
    else
    {
        LE_DEBUG("BadImageHandlerCounter = 1");
        BadImageHandlerCounter = 1;
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Stop the bad image indication
 *
 * @return
 *      - LE_OK             on success
 *      - LE_FAULT          on failure
 */
//--------------------------------------------------------------------------------------------------
void pa_fwupdate_StopBadImageIndication
(
    void
)
{
    LE_DEBUG("stop");

    if (BadImageHandlerCounter > 1)
    {
        LE_DEBUG("some handlers are always opened, nothing to do");
        BadImageHandlerCounter--;
        LE_DEBUG("BadImageHandlerCounter = %d", BadImageHandlerCounter);
        return;
    }

    swiQmi_RemoveIndicationHandler(BadImageHandler,
                                   QMI_SERVICE_DMS,
                                   QMI_DMS_SWI_BAD_IMAGE_IND_V01);

    LE_DEBUG("BadImageHandlerCounter = 0");
    BadImageHandlerCounter = 0;
}
