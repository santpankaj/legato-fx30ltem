/**
 * @file pa_avc_qmi.c
 *
 * QMI implementation of @ref c_pa_avc API.
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#include "legato.h"
#include "pa_avc.h"
#include "swiQmi.h"
#include "interfaces.h"

// for print macros
#include "le_print.h"

// For fwupdate PA
#include "pa_fwupdate.h"



//--------------------------------------------------------------------------------------------------
// Definitions
//--------------------------------------------------------------------------------------------------
#define FW_DOWNLOAD_COMPLETE    200


//--------------------------------------------------------------------------------------------------
/**
 * Limit the payload to 1024 bytes if firmware doesn't support block transfer mode.
 */
//--------------------------------------------------------------------------------------------------
#define PAYLOAD_MAX_LEN_NO_BLOCKMODE    1024


//--------------------------------------------------------------------------------------------------
/**
 * Maximum size of the COAP Token field. This field is sent to Legato during an Observe operation
 * only. For other lwm2m operations token is managed by firmware.
 */
//--------------------------------------------------------------------------------------------------
#define TOKEN_MAX_LENGTH        8


//--------------------------------------------------------------------------------------------------
/**
 * Session type to stop active session
 */
//--------------------------------------------------------------------------------------------------
#define DISABLE_SESSION 0x01


//--------------------------------------------------------------------------------------------------
/**
 * Session type to disable LWM2M Client
 */
//--------------------------------------------------------------------------------------------------
#define DISABLE_LWM2M_CLIENT 0x02


//--------------------------------------------------------------------------------------------------
/**
 * Data associated with a LWM2M operation indication
 */
//--------------------------------------------------------------------------------------------------
typedef struct pa_avc_LWM2MOperationData
{
    pa_avc_OpType_t opType;                     ///< LWM2M operation type
    char objPrefix[PREFIX_MAX_LEN_V01];         ///< Object prefix string
    int objId;                                  ///< Object i , or -1 if set to 65535 (USHRT_MAX)
    int objInstId;                              ///< Object instance id, or -1 if not specified
    int resourceId;                             ///< Resource id, or -1 if not specified
    uint8_t payload[PAYLOAD_MAX_LEN_V01];       ///< Payload
    uint8_t tokenLength;                        ///< Token length
    uint8_t token[TOKEN_MAX_LENGTH];            ///< Token i.e., request ID
    uint16_t contentType;                       ///< Content Type
    size_t payloadNumBytes;                     ///< Payload size in bytes, or 0 if no payload
    bool blockmodeValid;                        ///< Is this a block transfer mode?
    uint32_t blockOffset;                       ///< Block Offset in bytes
    uint16_t blockSize;                         ///< Block size in bytes
}
LWM2MOperationData_t;


//--------------------------------------------------------------------------------------------------
/**
 * Data associated with a LWM2M operation indication
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    le_avc_Status_t status;             ///< Status of the current update
    le_avc_UpdateType_t type;           ///< Type of the current update
    int32_t totalNumBytes;              ///< # of bytes to download
    int32_t dloadProgress;              ///< Percentage of download completed
    le_avc_ErrorCode_t errorCode;       ///< Incurred error during download
}
UpdateStatusData_t;


//--------------------------------------------------------------------------------------------------
// Local Data
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * The M2m general client.  Must be obtained by calling
 * swiQmi_GetClientHandle() before it can be used.
 */
//--------------------------------------------------------------------------------------------------
static qmi_client_type QapiM2mGeneralClient;

//--------------------------------------------------------------------------------------------------
/**
 * The LWM2M client.  Must be obtained by calling
 * swiQmi_GetClientHandle() before it can be used.
 */
//--------------------------------------------------------------------------------------------------
static qmi_client_type QapiLWM2MClient;

//--------------------------------------------------------------------------------------------------
/**
 * Store the Legato thread, since we need to queue a function to this thread from the QMI thread,
 * for calling the three indication handlers given below.  It is assumed that all three handlers
 * are registered from the same Legato thread, and in fact, that all the pa_avc functions are
 * called from this thread.
 */
//--------------------------------------------------------------------------------------------------
static le_thread_Ref_t LegatoThread=NULL;

//--------------------------------------------------------------------------------------------------
/**
 * Pool used to pass update status notification to the LegatoThread
 */
//--------------------------------------------------------------------------------------------------
static le_mem_PoolRef_t AVMSUpdateStatusPool;

//--------------------------------------------------------------------------------------------------
/**
 * Pool used to pass LWM2M operation inidication to the LegatoThread
 */
//--------------------------------------------------------------------------------------------------
static le_mem_PoolRef_t LWM2MOperationPool;

//--------------------------------------------------------------------------------------------------
/*
 * Handler passed to pa_avc_StartURIDownload().  When the download completes, or fails on error
 * then this function will be called.  It will be NULL when there is no active download.
 */
//--------------------------------------------------------------------------------------------------
static pa_avc_URIDownloadHandlerFunc_t URIDownloadHandlerRef = NULL;

//--------------------------------------------------------------------------------------------------
/*
 * Only one event handler is allowed to be registered at a time.
 */
//--------------------------------------------------------------------------------------------------
static pa_avc_AVMSMessageHandlerFunc_t AVMSMessageHandlerRef = NULL;

//--------------------------------------------------------------------------------------------------
/*
 * Only one event handler is allowed to be registered at a time.
 */
//--------------------------------------------------------------------------------------------------
static pa_avc_LWM2MOperationHandlerFunc_t LWM2MOperationHandlerRef = NULL;

//--------------------------------------------------------------------------------------------------
/*
 * Only one event handler is allowed to be registered at a time.
 */
//--------------------------------------------------------------------------------------------------
static pa_avc_LWM2MUpdateRequiredHandlerFunc_t UpdateRequiredHandlerRef = NULL;

//--------------------------------------------------------------------------------------------------
/**
 * Used for reporting LE_AVC_NO_UPDATE if there has not been any activity from the modem for a
 * specific amount of time, after a session has been started.
 */
//--------------------------------------------------------------------------------------------------
static le_timer_Ref_t ModemActivityTimerRef;


//--------------------------------------------------------------------------------------------------
/**
 * Current session type reported by the modem through avms indication.
 */
//--------------------------------------------------------------------------------------------------
static le_avc_SessionType_t CurrentSessionType = LE_AVC_SESSION_INVALID;


//--------------------------------------------------------------------------------------------------
/**
 * Current http status reported by the modem through avms indication.
 */
//--------------------------------------------------------------------------------------------------
static uint16_t CurrentHttpStatus = LE_AVC_HTTP_STATUS_INVALID;


//--------------------------------------------------------------------------------------------------
// Local functions
//--------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------
/**
 * Handler function for ModemActivityTimerRef expiry
 */
//--------------------------------------------------------------------------------------------------
static void ModemActivityTimerHandler
(
    le_timer_Ref_t timerRef    ///< This timer has expired
)
{
    LE_DEBUG("Modem Activity timer expired; reporting LE_AVC_NO_UPDATE");

    // This is based on similar functionality in ReportAvmsState()
    if ( AVMSMessageHandlerRef != NULL )
    {
        // reporting -1 for total number of bytes and progress as a download is not active at this stage
        AVMSMessageHandlerRef(LE_AVC_NO_UPDATE,
                              LE_AVC_UNKNOWN_UPDATE,
                              -1,
                              -1,
                              LE_AVC_ERR_NONE);
    }
    else
    {
        LE_ERROR("No registered handler to report LE_AVC_NO_UPDATE");
    }
}


//--------------------------------------------------------------------------------------------------
/**
 * Function queued onto LegatoThread to restart the modem activity timer.
 *
 * This is used by the QMI threads to restart the timer, because the timer can only be accessed
 * by the thread that it is running on, i.e. the LegatoThread.  Although the QMI threads do queue
 * other functions onto the LegatoThread, but depending on the inputs, that is not always done.
 * This function provides a consistent mechanism that can be used by all QMI threads.
 */
//--------------------------------------------------------------------------------------------------
static void RestartModemActivityTimerHandler
(
    void* param1Ptr,
    void* param2Ptr
)
{
    // Only restart if it is currently running
    if ( le_timer_IsRunning(ModemActivityTimerRef) )
    {
        LE_DEBUG("Restarting Modem Activity timer");
        le_timer_Restart(ModemActivityTimerRef);
    }
}


//--------------------------------------------------------------------------------------------------
/**
 * Translate QMI status into readable information, for debugging purpose.
 */
//--------------------------------------------------------------------------------------------------
static const char* TranslateAvmsStatus
(
    uint8_t status
)
{
    switch (status)
    {
        case 0x01: return "No Firmware available";
        case 0x02: return "Query Firmware Download";
        case 0x03: return "Firmware Downloading";
        case 0x04: return "Firmware downloaded";
        case 0x05: return "Query Firmware Update";
        case 0x06: return "Firmware updating";
        case 0x07: return "Firmware updated";
        default: return "Undefined status";
    }
}


//--------------------------------------------------------------------------------------------------
/**
 * Report the AVMS state to the registered handler
 */
//--------------------------------------------------------------------------------------------------
static void ReportAvmsState
(
    void* param1Ptr,
    void* param2Ptr
)
{
    UpdateStatusData_t* updateStatusPtr = param1Ptr;

    if ( updateStatusPtr == NULL )
    {
        LE_CRIT("param1Ptr is NULL");
        return;
    }

    // A valid status or start/stop session is being reported. Ensure the timer is stopped, in
    // all cases except for start session.
    if ( updateStatusPtr->status != LE_AVC_SESSION_STARTED )
    {
        le_timer_Stop(ModemActivityTimerRef);
    }

    if ( AVMSMessageHandlerRef != NULL )
    {
        AVMSMessageHandlerRef( updateStatusPtr->status,
                               updateStatusPtr->type,
                               updateStatusPtr->totalNumBytes,
                               updateStatusPtr->dloadProgress,
                               updateStatusPtr->errorCode);
    }
    else
    {
        LE_ERROR("No registered handler to report update status = %d, type = %d",
                 updateStatusPtr->status,
                 updateStatusPtr->type);
    }

    // If there is a download in progress, report the status. If the download is complete or failed
    // report the status and clear the handler.
    // TODO: Should only do this for App or Framework updates, once that information is
    //       correctly reported by the modem.
    if ( URIDownloadHandlerRef != NULL )
    {
        if ( ( updateStatusPtr->status == LE_AVC_DOWNLOAD_COMPLETE ) ||
             ( updateStatusPtr->status == LE_AVC_DOWNLOAD_FAILED ) )
        {
            URIDownloadHandlerRef( updateStatusPtr->status);

            // Status was reported, so reset handlerRef, since we only call it once
            URIDownloadHandlerRef = NULL;
        }
        else if ( updateStatusPtr->status == LE_AVC_DOWNLOAD_IN_PROGRESS )
        {
            URIDownloadHandlerRef( updateStatusPtr->status);
        }
    }

    le_mem_Release(updateStatusPtr);
}


//--------------------------------------------------------------------------------------------------
/**
 * Parse the AMVS event and return the corresponding update status and type values.
 *
 * @return
 *      - LE_OK if returned updateStatus is valid
 *      - LE_FAULT if returned updateStatus is NOT valid
 */
//--------------------------------------------------------------------------------------------------
static le_result_t ProcessAvmsState
(
    avms_session_info_type_v01 avmsInfo,  ///< [IN]
    le_avc_Status_t* updateStatusPtr,     ///< [OUT]
    le_avc_UpdateType_t* updateTypePtr,   ///< [OUT]
    le_avc_ErrorCode_t* errorCodePtr      ///< [OUT]
)
{
    le_result_t result = LE_FAULT;
    uint16_t updateCompleteStatus;

    switch ( avmsInfo.binary_type )
    {
        case 0x1:
            LE_INFO("Received Firmware update");
            *updateTypePtr = LE_AVC_FIRMWARE_UPDATE;
            break;

        case 0x2:
            LE_INFO("Received User App update");
            *updateTypePtr = LE_AVC_APPLICATION_UPDATE;
            break;

        case 0x3:
            LE_INFO("Received Legato Framework update");
            *updateTypePtr = LE_AVC_FRAMEWORK_UPDATE;
            break;

        default:
            LE_WARN("Received unknown binary type = %i", avmsInfo.binary_type);
            *updateTypePtr = LE_AVC_UNKNOWN_UPDATE;
    }

    LE_DEBUG("Process Avms session indication '%s'", TranslateAvmsStatus(avmsInfo.status));
    *errorCodePtr = LE_AVC_ERR_NONE;

    switch ( avmsInfo.status )
    {
        case 0x01: /* No Firmware available */
            *updateStatusPtr = LE_AVC_NO_UPDATE;
            result = LE_OK;
            break;

        case 0x02: /* Query Firmware Download */
            *updateStatusPtr = LE_AVC_DOWNLOAD_PENDING;
            result = LE_OK;
            break;

        case 0x03: /* Firmware downloading */
            *updateStatusPtr = LE_AVC_DOWNLOAD_IN_PROGRESS;
            result = LE_OK;
            break;

        case 0x04: /* Firmware downloaded */
            updateCompleteStatus = avmsInfo.update_complete_status;

            if ( (updateCompleteStatus == 200) ||
                 ( (updateCompleteStatus >= 250) && (updateCompleteStatus <= 299) ) )
            {
                *updateStatusPtr = LE_AVC_DOWNLOAD_COMPLETE;
                result = LE_OK;
            }
            else if ( (updateCompleteStatus >= 400) && (updateCompleteStatus <= 599) )
            {
                *errorCodePtr = ( (updateCompleteStatus >= 402) && (updateCompleteStatus <= 405) ) ?
                                LE_AVC_ERR_BAD_PACKAGE :
                                LE_AVC_ERR_INTERNAL;
                LE_ERROR("Download failed. Avms update_complete_status %d.", updateCompleteStatus);
                *updateStatusPtr = LE_AVC_DOWNLOAD_FAILED;
                result = LE_OK;
            }
            else
            {
                LE_ERROR("Avms update_complete_status %x not recognized.", updateCompleteStatus);
                *updateStatusPtr = LE_AVC_DOWNLOAD_FAILED;
                result = LE_OK;
            }
            break;

        case 0x05: /* Query Firmware Update */
            *updateStatusPtr = LE_AVC_INSTALL_PENDING;
            result = LE_OK;
            break;

        case 0x06: /* Firmware updating */
            *updateStatusPtr = LE_AVC_INSTALL_IN_PROGRESS;
            result = LE_OK;
            break;

        case 0x07: /* Firmware updated */
            updateCompleteStatus = avmsInfo.update_complete_status;

            if ( (updateCompleteStatus == 200) ||
                 ( (updateCompleteStatus >= 250) && (updateCompleteStatus <= 299) ) )
            {
                *updateStatusPtr = LE_AVC_INSTALL_COMPLETE;
                result = LE_OK;
            }
            else if ( (updateCompleteStatus >= 400) && (updateCompleteStatus <= 599) )
            {
                *updateStatusPtr = LE_AVC_INSTALL_FAILED;
                result = LE_OK;
            }
            else
            {
                LE_ERROR("Avms update_complete_status %x not recognized.", updateCompleteStatus);
                *updateStatusPtr = LE_AVC_INSTALL_FAILED;
                result = LE_OK;
            }
            break;

        default:
            LE_ERROR("Avms status %x not recognized.", avmsInfo.status);
    }

    return result;
}


//--------------------------------------------------------------------------------------------------
/**
 * New AVMS indication handler called by the QMI M2m general indications.
 */
//--------------------------------------------------------------------------------------------------
static void AvmsIndicationHandler
(
    void*           indBufPtr,  // [IN] The indication message data.
    unsigned int    indBufLen,  // [IN] The indication message data length in bytes.
    void*           contextPtr  // [IN] Context value that was passed to
                                //      swiQmi_AddIndicationHandler().
)
{
    // We got activity from the modem, so restart the timer.
    if ( LegatoThread != NULL )
    {
        le_event_QueueFunctionToThread(LegatoThread, RestartModemActivityTimerHandler, NULL, NULL);
    }

    swi_m2m_avms_event_ind_msg_v01 avmsInd;

    qmi_client_error_type clientMsgErr = qmi_client_message_decode(
        QapiM2mGeneralClient,
        QMI_IDL_INDICATION,
        QMI_SWI_M2M_AVMS_EVENT_IND_V01,
        indBufPtr, indBufLen,
        &avmsInd, sizeof(avmsInd));

    if (clientMsgErr != QMI_NO_ERR)
    {
        LE_ERROR("Error in decoding QMI_SWI_M2M_AVMS_EVENT_IND_V01, error = %i", clientMsgErr);
        return;
    }

    // Process the valid message response
    // Provide some default values here so the compiler doesn't complain about
    // potentially uninitialized variables.
    le_avc_Status_t updateStatus = LE_AVC_NO_UPDATE;
    le_avc_UpdateType_t updateType = LE_AVC_UNKNOWN_UPDATE;
    le_avc_ErrorCode_t errorCode = LE_AVC_ERR_INTERNAL;
    bool isStatusValid = false;

    LE_DEBUG("Got AVMS message");

    if (avmsInd.avms_session_info_valid)
    {
        LE_PRINT_VALUE("%i", avmsInd.avms_session_info.binary_type);
        LE_PRINT_VALUE("%i", avmsInd.avms_session_info.status);
        LE_PRINT_VALUE("%i", avmsInd.avms_session_info.user_input_req);
        LE_PRINT_VALUE("%i", avmsInd.avms_session_info.user_input_timeout);
        LE_PRINT_VALUE("%i", avmsInd.avms_session_info.fw_dload_size);
        LE_PRINT_VALUE("%i", avmsInd.avms_session_info.fw_dload_complete);
        LE_PRINT_VALUE("%i", avmsInd.avms_session_info.update_complete_status);
        LE_PRINT_VALUE("%i", avmsInd.avms_session_info.severity);
        LE_PRINT_VALUE("%i", avmsInd.avms_session_info.version_name_len);
        LE_PRINT_VALUE("%s", avmsInd.avms_session_info.version_name);
        LE_PRINT_VALUE("%i", avmsInd.avms_session_info.package_name_len);
        LE_PRINT_VALUE("%s", avmsInd.avms_session_info.package_name);
        LE_PRINT_VALUE("%i", avmsInd.avms_session_info.package_description_len);
        LE_PRINT_VALUE("%s", avmsInd.avms_session_info.package_description);

        le_result_t result = ProcessAvmsState(avmsInd.avms_session_info,
                                              &updateStatus,
                                              &updateType,
                                              &errorCode);

        // Is the value stored in updateStatus valid?
        isStatusValid = ( result == LE_OK );
    }
    else
    {
        LE_DEBUG("No session info available");
    }

    // Log config info, if available, mainly for testing ...
    if ( avmsInd.avms_config_info_valid )
    {
        LE_PRINT_VALUE("%i", avmsInd.avms_config_info.state);
        LE_PRINT_VALUE("%i", avmsInd.avms_config_info.user_input_req);
    }
    else
        LE_DEBUG("No config info available");

    // These notifications seem to indicate when a session with the server starts and stops,
    // however, the values do not seem to match the QMI docs.
    // todo: In the process of sorting out this mismatch.
    if ( avmsInd.avms_notifications_valid )
    {
        LE_PRINT_VALUE("%i", avmsInd.avms_notifications.notification);
        LE_PRINT_VALUE("%i", avmsInd.avms_notifications.session_status);

        if ( avmsInd.avms_notifications.notification == 0x15 )
        {
            updateStatus = LE_AVC_SESSION_STARTED;
            isStatusValid = true;
        }
        else if ( avmsInd.avms_notifications.notification == 0x16 )
        {
            updateStatus = LE_AVC_SESSION_STOPPED;
            isStatusValid = true;
        }
    }
    else
        LE_DEBUG("No notifications info available");

    if ( avmsInd.avms_connection_request_valid )
    {
        LE_PRINT_VALUE("%i", avmsInd.avms_connection_request.user_input_req);
        LE_PRINT_VALUE("%i", avmsInd.avms_connection_request.user_input_timeout);
    }
    else
        LE_DEBUG("No connection_request available");

    if ( avmsInd.reg_status_valid )
    {
        LE_PRINT_VALUE("%i", avmsInd.reg_status);
    }
    else
        LE_DEBUG("No reg_status available");

    if ( avmsInd.avms_data_session_valid )
    {
        LE_PRINT_VALUE("%i", avmsInd.avms_data_session.notif_type);
        LE_PRINT_VALUE("%i", avmsInd.avms_data_session.err_code);
    }
    else
        LE_DEBUG("No data_session available");

    if (avmsInd.session_type_valid)
    {
        LE_PRINT_VALUE("%d", avmsInd.session_type);
        CurrentSessionType = avmsInd.session_type;
    }
    else
        LE_DEBUG("No session_type available.");

#ifdef LEGATO_FEATURE_AVMS_CONFIG
    if (avmsInd.http_status_valid)
    {
        LE_PRINT_VALUE("%d", avmsInd.http_status);
        CurrentHttpStatus = avmsInd.http_status;
    }
    else
        LE_DEBUG("No http status available.");
#endif

    if ( isStatusValid )
    {
        // Got valid status info, so queue the function to respond, but only if there is a
        // registered client; otherwise, just drop the message.
        if ( LegatoThread != NULL )
        {
            UpdateStatusData_t* updateStatusPtr = le_mem_ForceAlloc(AVMSUpdateStatusPool);
            updateStatusPtr->status = updateStatus;
            updateStatusPtr->type = updateType;
            updateStatusPtr->errorCode = errorCode;

            // Only valid when downloading or downloaded, but always fill it in.
            updateStatusPtr->totalNumBytes = avmsInd.avms_session_info.fw_dload_size;

            if (avmsInd.avms_session_info.fw_dload_size != 0 )
            {
                // Calculate the percentage of downloaded data; for progress indication to user
                // Casting to uint64_t should prevent overflow if size > ~40MB
                updateStatusPtr->dloadProgress =
                        ((uint64_t)avmsInd.avms_session_info.fw_dload_complete * 100) /
                        avmsInd.avms_session_info.fw_dload_size;

                // Following is a workaround for a FW issue
                // When download is finished, the fw_dload_complete number is smaller than fw_dload_size,
                // and so we get less than 100%. This issue happens because after download complete firmware
                // reports the final size without the dwl header
                if ( avmsInd.avms_session_info.update_complete_status == FW_DOWNLOAD_COMPLETE )
                {
                    updateStatusPtr->dloadProgress = 100;
                }
            }
            else
            {
                LE_DEBUG("Download progress unknown; Firmware Download size unavailable");
                updateStatusPtr->dloadProgress = -1;
                updateStatusPtr->totalNumBytes = -1;
            }

            // We should never get a progress >100%
            // cap the download progress to 100% and warn the user
            if (updateStatusPtr->dloadProgress > 100  )
            {
                LE_WARN("Download progress capped to 100");
                updateStatusPtr->dloadProgress = 100;
            }

            LE_PRINT_VALUE("%i", updateStatusPtr->totalNumBytes);
            LE_PRINT_VALUE("%i%%", updateStatusPtr->dloadProgress);

            le_event_QueueFunctionToThread(LegatoThread, ReportAvmsState, updateStatusPtr, NULL);
        }
    }
}


//--------------------------------------------------------------------------------------------------
/**
 * Report the LWM2M operation to the registered handler; runs in the LegatoThread
 */
//--------------------------------------------------------------------------------------------------
static void ReportOperation
(
    void* param1Ptr,
    void* param2Ptr
)
{
    LWM2MOperationData_t* operationDataPtr = param1Ptr;

    if ( operationDataPtr == NULL )
    {
        LE_CRIT("param1Ptr is NULL");
        return;
    }

    if ( LWM2MOperationHandlerRef != NULL )
    {
        LWM2MOperationHandlerRef(operationDataPtr);
    }
    else
    {
        LE_ERROR("No registered handler to report LWM2M operation");
    }

    le_mem_Release(operationDataPtr);
}


//--------------------------------------------------------------------------------------------------
/**
 * LWM2M Operation indication handler called by the QMI LWM2M Service.
 */
//--------------------------------------------------------------------------------------------------
static void LWM2MOperationIndicationHandler
(
    void*           indBufPtr,  // [IN] The indication message data.
    unsigned int    indBufLen,  // [IN] The indication message data length in bytes.
    void*           contextPtr  // [IN] Context value that was passed to
                                //      swiQmi_AddIndicationHandler().
)
{
    // We got activity from the modem, so restart the timer.
    if ( LegatoThread != NULL )
    {
        le_event_QueueFunctionToThread(LegatoThread, RestartModemActivityTimerHandler, NULL, NULL);
    }

    lwm2m_operation_ind_msg_v01 operationInd;

    qmi_client_error_type clientMsgErr = qmi_client_message_decode(
        QapiLWM2MClient,
        QMI_IDL_INDICATION,
        QMI_LWM2M_OPERATION_IND_V01,
        indBufPtr, indBufLen,
        &operationInd, sizeof(operationInd));

    if (clientMsgErr != QMI_NO_ERR)
    {
        LE_ERROR("Error in decoding QMI_LWM2M_OPERATION_IND_V01, error = %i", clientMsgErr);
        return;
    }

    // Process the valid indication message
    LE_DEBUG("Got Operation indication message");

    // Log the message contents, for debugging/testing
    LE_PRINT_VALUE("%i", operationInd.op_type);
    LE_PRINT_VALUE("%s", operationInd.obj_prefix);
    LE_PRINT_VALUE("%i", operationInd.obj_id);
    LE_PRINT_VALUE_IF(operationInd.obj_iid_valid, "%i", operationInd.obj_iid);
    LE_PRINT_VALUE_IF(operationInd.res_id_valid, "%i", operationInd.res_id);

    LE_PRINT_VALUE("%i", operationInd.blockmode_valid);
    if (operationInd.blockmode_valid)
    {
        LE_PRINT_VALUE("%i", operationInd.blockmode.block_offset);
        LE_PRINT_VALUE("%i", operationInd.blockmode.block_size);
    }

    if ( operationInd.payload_valid )
    {
        LE_DUMP((uint8_t*)operationInd.payload, operationInd.payload_len);
        LE_PRINT_VALUE("%i", operationInd.payload_len);
    }

    // Queue the function to respond, but only if there is a registered client;
    // otherwise, just drop the message.
    if ( LegatoThread != NULL )
    {
        // Allocate and fill in report data block
        LWM2MOperationData_t* operationDataPtr = le_mem_ForceAlloc(LWM2MOperationPool);
        operationDataPtr->opType = operationInd.op_type;

        // It's an error if the complete prefix can't be copied.
        if ( le_utf8_Copy( operationDataPtr->objPrefix,
                           operationInd.obj_prefix,
                           sizeof(operationDataPtr->objPrefix),
                           NULL ) != LE_OK )
        {
            LE_ERROR("object prefix too large: length=%i", strlen(operationInd.obj_prefix) );
            return;
        }

        if (operationInd.blockmode_valid)
        {
            operationDataPtr->blockmodeValid = true;
            operationDataPtr->blockOffset = operationInd.blockmode.block_offset;
            operationDataPtr->blockSize = operationInd.blockmode.block_size;
        }
        else
        {
            operationDataPtr->blockmodeValid = false;
        }
        operationDataPtr->objId = (operationInd.obj_id != USHRT_MAX) ? operationInd.obj_id : -1;
        operationDataPtr->objInstId = operationInd.obj_iid_valid ? operationInd.obj_iid : -1;
        operationDataPtr->resourceId = operationInd.res_id_valid ? operationInd.res_id : -1;

#ifdef LEGATO_FEATURE_OBSERVE
        operationDataPtr->contentType = operationInd.content_type_valid ?
                                                            operationInd.content_type : 0;

        if (operationInd.observe_token_valid
            && operationInd.observe_token_len > 0
            && operationInd.observe_token_len <= TOKEN_MAX_LENGTH)
        {
            memcpy(operationDataPtr->token, operationInd.observe_token, operationInd.observe_token_len);
            operationDataPtr->tokenLength = operationInd.observe_token_len;
        }
        else
        {
            operationDataPtr->tokenLength = 0;
        }
#endif
        if ( operationInd.payload_valid )
        {
            // Even if the payload TLV is valid, make sure it will fit in operationData.  If it
            // doesn't then something is wrong, and just discard the whole message.
            if ( operationInd.payload_len <= sizeof(operationDataPtr->payload) )
            {
                memcpy( operationDataPtr->payload, operationInd.payload, operationInd.payload_len);
                operationDataPtr->payloadNumBytes = operationInd.payload_len;
            }
            else
            {
                LE_ERROR("payload too large: length=%i", operationInd.payload_len );
                return;
            }
        }

        le_event_QueueFunctionToThread(LegatoThread, ReportOperation, operationDataPtr, NULL);
    }
}


//--------------------------------------------------------------------------------------------------
/**
 * Report the LWM2M update required to the registered handler; runs in the LegatoThread
 */
//--------------------------------------------------------------------------------------------------
static void ReportUpdateRequired
(
    void* param1Ptr,
    void* param2Ptr
)
{
    if ( UpdateRequiredHandlerRef != NULL )
    {
        UpdateRequiredHandlerRef(ASSET_DATA_SESSION_STATUS_CHECK);
    }
    else
    {
        LE_ERROR("No registered handler to report update required");
    }
}


//--------------------------------------------------------------------------------------------------
/**
 * LWM2M Update required indication handler called by the QMI LWM2M Service.
 */
//--------------------------------------------------------------------------------------------------
static void LWM2MUpdateRequiredIndicationHandler
(
    void*           indBufPtr,  // [IN] The indication message data.
    unsigned int    indBufLen,  // [IN] The indication message data length in bytes.
    void*           contextPtr  // [IN] Context value that was passed to
                                //      swiQmi_AddIndicationHandler().
)
{
    // We got activity from the modem, so restart the timer.
    if ( LegatoThread != NULL )
    {
        le_event_QueueFunctionToThread(LegatoThread, RestartModemActivityTimerHandler, NULL, NULL);
    }

    // This indication has no TLVs, so just note that we got the message.
    LE_DEBUG("Got Update required indication message");

    // Queue the function to respond, but only if there is a registered client;
    // otherwise, just drop the message.
    if ( LegatoThread != NULL )
    {
        le_event_QueueFunctionToThread(LegatoThread, ReportUpdateRequired, NULL, NULL);
    }
}


#if 0
// todo: This isn't used at the moment, but keep it for now, as it may be useful for debuggging
//--------------------------------------------------------------------------------------------------
/**
 * Get info about the current or most recent session, and print it to the log
 */
//--------------------------------------------------------------------------------------------------
static void GetSessionInfo
(
    void
)
{
    qmi_client_error_type rc;
    swi_m2m_avms_session_getinfo_resp_msg_v01 avmsResp = { {0} };

    // Send the message and handle the response
    rc = qmi_client_send_msg_sync(QapiM2mGeneralClient,
                                  QMI_SWI_M2M_AVMS_SESSION_GETINFO_REQ_V01,
                                  NULL, 0,
                                  &avmsResp, sizeof(avmsResp),
                                  COMM_LONG_PLATFORM_TIMEOUT);

    le_result_t result = swiQmi_OEMCheckResponseCode(
                                           STRINGIZE_EXPAND(QMI_SWI_M2M_AVMS_SESSION_GETINFO_REQ_V01),
                                           rc,
                                           avmsResp.resp);

    if (result != LE_OK)
    {
        // Nothing to log if we get an error
        return;
    }

    if ( avmsResp.avms_session_info_valid )
    {
        LE_PRINT_VALUE("%i", avmsResp.avms_session_info.binary_type);
        LE_PRINT_VALUE("%i", avmsResp.avms_session_info.status);
        LE_PRINT_VALUE("%i", avmsResp.avms_session_info.user_input_req);
        LE_PRINT_VALUE("%i", avmsResp.avms_session_info.fw_dload_size);
        LE_PRINT_VALUE("%i", avmsResp.avms_session_info.fw_dload_complete);
    }

    if ( avmsResp.avms_config_info_valid )
    {
        LE_PRINT_VALUE("%i", avmsResp.avms_config_info.state);
        LE_PRINT_VALUE("%i", avmsResp.avms_config_info.user_input_req);
    }

    if ( avmsResp.avms_notifications_valid )
    {
        LE_PRINT_VALUE("%i", avmsResp.avms_notifications.notification);
        LE_PRINT_VALUE("%i", avmsResp.avms_notifications.session_status);
    }


}
#endif

//--------------------------------------------------------------------------------------------------
// API functions
//--------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------
/**
 * Start a timer to watch the activity from the modem.
 */
//--------------------------------------------------------------------------------------------------
void  pa_avc_StartModemActivityTimer
(
    void
)
{
    // Keep track of activity from modem.
    le_timer_Start(ModemActivityTimerRef);
}


//--------------------------------------------------------------------------------------------------
/**
 * Start a session with the AirVantage server
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on error
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_avc_StartSession
(
    void
)
{
    qmi_client_error_type rc;

    SWIQMI_DECLARE_MESSAGE(swi_m2m_avms_session_start_req_msg_v01, avmsReq);
    SWIQMI_DECLARE_MESSAGE(swi_m2m_avms_session_start_resp_msg_v01, avmsResp);

    // Fill in the request message
    avmsReq.session_type = 0x01;

    // Send the message and handle the response
    rc = qmi_client_send_msg_sync(QapiM2mGeneralClient,
                                  QMI_SWI_M2M_AVMS_SESSION_START_REQ_V01,
                                  &avmsReq, sizeof(avmsReq),
                                  &avmsResp, sizeof(avmsResp),
                                  COMM_LONG_PLATFORM_TIMEOUT);

    le_result_t result = swiQmi_OEMCheckResponseCode(
                                           STRINGIZE_EXPAND(QMI_SWI_M2M_AVMS_SESSION_START_REQ_V01),
                                           rc,
                                           avmsResp.resp);

    if (result != LE_OK)
    {
        // If we got the NO_EFFECT error code, then this is actually ok.
        if ( (rc==QMI_NO_ERR)
             && (avmsResp.resp.result==QMI_RESULT_FAILURE_V01)
             && (avmsResp.resp.error==QMI_ERR_NO_EFFECT_V01) )
        {
            LE_INFO("Session already active");

            // Generate a fake start session notification, because the caller expects to receive
            // the notification after successfully calling this function.
            //
            // The following code is based on similar functionality in ReportAvmsState()
            if ( AVMSMessageHandlerRef != NULL )
            {
                // reporting -1 for total number of bytes and progress as a download is not active at this stage
                AVMSMessageHandlerRef(LE_AVC_SESSION_STARTED,
                                      LE_AVC_UNKNOWN_UPDATE,
                                      -1,
                                      -1,
                                      LE_AVC_ERR_NONE);
            }
            else
            {
                LE_ERROR("No registered handler to report LE_AVC_SESSION_STARTED");
            }
        }
        else
        {
            return result;
        }
    }

    return LE_OK;
}


//--------------------------------------------------------------------------------------------------
/**
 * Enable user agreement for download and install
 *
 * @return
 *     void
 */
//--------------------------------------------------------------------------------------------------
void pa_avc_EnableUserAgreement
(
    void
)
{
    le_result_t rc;

    SWIQMI_DECLARE_MESSAGE(swi_m2m_avms_set_setting_req_msg_v01, avmsSettingSetReq);
    SWIQMI_DECLARE_MESSAGE(swi_m2m_avms_set_setting_resp_msg_v01, avmsSettingSetResp);
    SWIQMI_DECLARE_MESSAGE(swi_m2m_avms_get_setting_resp_msg_v01, avmsSettingGetResp);

    // Read AVMS settings.
    rc = qmi_client_send_msg_sync(QapiM2mGeneralClient,
                                  QMI_SWI_M2M_AVMS_GET_SETTINGS_REQ_V01,
                                  NULL, 0,
                                  &avmsSettingGetResp, sizeof(avmsSettingGetResp),
                                  COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (swiQmi_OEMCheckResponseCode( STRINGIZE_EXPAND(QMI_SWI_M2M_AVMS_GET_SETTINGS_REQ_V01),
                                     rc,
                                     avmsSettingGetResp.resp) != LE_OK )
    {
        LE_ERROR("Cannot read AVMS settings");
    }

    // Make sure we have user agreement set for download and install.
    if (   (avmsSettingGetResp.fw_autodload != 0x01)
        || (avmsSettingGetResp.fw_autoupdate != 0x01))
    {
        // Enable user agreement.
        avmsSettingSetReq.fw_autodload = 0x1;   // Set user agreement for package download; at+wdsc=1,1
        avmsSettingSetReq.fw_autoupdate = 0x1;  // Set user agreement for package install; at+wdsc=2,1

        // Send request to fw.
        rc = qmi_client_send_msg_sync(QapiM2mGeneralClient,
                                      QMI_SWI_M2M_AVMS_SET_SETTINGS_REQ_V01,
                                      &avmsSettingSetReq, sizeof(avmsSettingSetReq),
                                      &avmsSettingSetResp, sizeof(avmsSettingSetResp),
                                      COMM_DEFAULT_PLATFORM_TIMEOUT);

        if (swiQmi_OEMCheckResponseCode( STRINGIZE_EXPAND(QMI_SWI_M2M_AVMS_SET_SETTINGS_REQ_V01),
                                         rc,
                                         avmsSettingSetResp.resp) != LE_OK )
        {
            LE_ERROR("User Agreement not enabled");
        }
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Stop a session with the AirVantage server
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on error
 */
//--------------------------------------------------------------------------------------------------
static le_result_t SendStopSession
(
    uint8_t stopType
)
{
    // Stop the timer, regardless of whether or not the session is successfully stopped.
    if (ModemActivityTimerRef != NULL)
    {
        le_timer_Stop(ModemActivityTimerRef);
    }

    qmi_client_error_type rc;

    SWIQMI_DECLARE_MESSAGE(swi_m2m_avms_session_stop_req_msg_v01, avmsReq);
    SWIQMI_DECLARE_MESSAGE(swi_m2m_avms_session_stop_resp_msg_v01, avmsResp);

    // Fill in the request message -- only stop a FOTA session
    avmsReq.session_type = stopType;

    // Send the message and handle the response
    rc = qmi_client_send_msg_sync(QapiM2mGeneralClient,
                                  QMI_SWI_M2M_AVMS_SESSION_STOP_REQ_V01,
                                  &avmsReq, sizeof(avmsReq),
                                  &avmsResp, sizeof(avmsResp),
                                  COMM_LONG_PLATFORM_TIMEOUT);

    le_result_t result = swiQmi_OEMCheckResponseCode(
                                           STRINGIZE_EXPAND(QMI_SWI_M2M_AVMS_SESSION_STOP_REQ_V01),
                                           rc,
                                           avmsResp.resp);

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Stop a session with the AirVantage server
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on error
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_avc_StopSession
(
    void
)
{
    return SendStopSession(DISABLE_SESSION);
}

//--------------------------------------------------------------------------------------------------
/**
 * Disable the AirVantage agent.
 *
 * @return
 *      - LE_OK on success
 *      - LE_BUSY if the agent cannot be interrupted at the moment
 *      - LE_FAULT on error
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_avc_Disable
(
    void
)
{
    return SendStopSession(DISABLE_LWM2M_CLIENT);
}


//--------------------------------------------------------------------------------------------------
/**
 * Fill in the data structure required for lwm2m notify operation.
 *
 * @return
 *      - Reference to lwm2m operation
 */
//--------------------------------------------------------------------------------------------------
pa_avc_LWM2MOperationDataRef_t pa_avc_CreateOpData
(
    char* prefixPtr,
    int objId,
    int objInstId,
    int resourceId,
    pa_avc_OpType_t opType,
    uint16_t contentType,
    uint8_t* tokenPtr,
    uint8_t tokenLength
)
{
    // Allocate and fill in lwm2m operation data.
    pa_avc_LWM2MOperationDataRef_t notifyOpRef  = le_mem_ForceAlloc(LWM2MOperationPool);


    // Asset data doesn't have the "le_" prefix, but the server uses this prefix while referencing
    // an app. For lwm2m notification we need to append "le_" prefix if it is an app or the
    // "legato" prefix for legato object. If we are dealing with a standard lwm2m object we don't
    // need to send any prefix.
    if (strcmp(prefixPtr, "lwm2m") == 0)
    {
        notifyOpRef->objPrefix[0] = '\0';
    }
    else if (strcmp(prefixPtr, "legato") == 0)
    {
        LE_FATAL_IF(le_utf8_Copy(notifyOpRef->objPrefix, "legato", sizeof(notifyOpRef->objPrefix),
                                 NULL) != LE_OK, "Object prefix is too long.");
    }
    else if (strncmp(prefixPtr, "le_", 3) != 0)
    {
        char newPrefixPtr[30] = "le_";
        strncat(newPrefixPtr, prefixPtr, (sizeof(newPrefixPtr) - strlen(newPrefixPtr) - 1));
        LE_FATAL_IF(le_utf8_Copy(notifyOpRef->objPrefix, newPrefixPtr,
                                 sizeof(notifyOpRef->objPrefix),
                                 NULL) != LE_OK, "Object prefix is too long.");
    }

    // Set path and operation type.
    notifyOpRef->objId = objId;
    notifyOpRef->objInstId = objInstId;
    notifyOpRef->resourceId = resourceId;
    notifyOpRef->opType = opType;
    notifyOpRef->contentType = contentType;

    // lwm2m notify reports cannot be split in block mode.
    notifyOpRef->blockmodeValid = false;

    // Set token if specified (token is applicable only for observe and notify messages).
    if ( (tokenPtr != NULL) && (tokenLength > 0) && (tokenLength <= TOKEN_MAX_LENGTH) )
    {
        notifyOpRef->tokenLength = tokenLength;
        memcpy(notifyOpRef->token, tokenPtr, tokenLength);
    }
    else
    {
        notifyOpRef->tokenLength = 0;
        memset(notifyOpRef->token, 0, TOKEN_MAX_LENGTH);
    }

    return notifyOpRef;
}


//--------------------------------------------------------------------------------------------------
/**
 * Notify the server when the asset value changes.
 */
//--------------------------------------------------------------------------------------------------
void pa_avc_NotifyChange
(
    pa_avc_LWM2MOperationDataRef_t notifyOpRef, ///< [IN] Reference to LWM2M operation
    uint8_t* respPayloadPtr,                    ///< [IN] Payload, or NULL if no payload
    size_t respPayloadNumBytes                  ///< [IN] Payload size in bytes, or 0 if no payload.
                                                ///       If payload is a string, this is strlen()
)
{
    LE_DEBUG("Payload of notify message.");
    LE_DUMP((uint8_t*)respPayloadPtr, respPayloadNumBytes);

    // Send the valid response
    pa_avc_OperationReportSuccess(notifyOpRef,
                                  (uint8_t*)respPayloadPtr,
                                  respPayloadNumBytes);

    // Free lwm2m notify operation data.
    le_mem_Release(notifyOpRef);
}


//--------------------------------------------------------------------------------------------------
/**
 * Respond to the read call back operation.
 */
//--------------------------------------------------------------------------------------------------
void pa_avc_ReadCallBackReport
(
    pa_avc_LWM2MOperationDataRef_t opRef,       ///< [IN] Reference to LWM2M operation
    uint8_t* respPayloadPtr,                    ///< [IN] Payload, or NULL if no payload
    size_t respPayloadNumBytes                  ///< [IN] Payload size in bytes, or 0 if no payload.
                                                ///       If payload is a string, this is strlen()
)
{
    if (opRef != NULL)
    {
        // Send the valid response
        pa_avc_OperationReportSuccess(opRef,
                                      (uint8_t*)respPayloadPtr,
                                      respPayloadNumBytes);

        // Free lwm2m read call back operation reference.
        le_mem_Release(opRef);
    }
    else
    {
        LE_ERROR("Read call back operation reference is NULL.");
    }
}


//--------------------------------------------------------------------------------------------------
/**
 * Send the selection for the current pending update
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on error
 *
 * TODO: Should this take an additional parameter to specify who is doing the download/install
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_avc_SendSelection
(
    pa_avc_Selection_t selection,   ///< [IN] Action to take for pending download or install
    uint32_t deferTime              ///< [IN] If action is DEFER, then defer time in minutes
)
{
    qmi_client_error_type rc;

    SWIQMI_DECLARE_MESSAGE(swi_m2m_avms_selection_req_msg_v01, avmsReq);
    SWIQMI_DECLARE_MESSAGE(swi_m2m_avms_selection_resp_msg_v01, avmsResp);

    // Fill in the request message
    switch ( selection )
    {
        case PA_AVC_ACCEPT:
            avmsReq.user_input = 0x01;
            break;

        case PA_AVC_DEFER:
            avmsReq.user_input = 0x03;
            avmsReq.defer_time_valid = true;
            avmsReq.defer_time = deferTime;
            break;

        default:
            LE_ERROR("Invalid selection %i", selection);
            return LE_FAULT;
    }

    // Send the message and handle the response
    rc = qmi_client_send_msg_sync(QapiM2mGeneralClient,
                                  QMI_SWI_M2M_AVMS_SELECTION_REQ_V01,
                                  &avmsReq, sizeof(avmsReq),
                                  &avmsResp, sizeof(avmsResp),
                                  COMM_DEFAULT_PLATFORM_TIMEOUT);

    le_result_t result = swiQmi_OEMCheckResponseCode(
                                           STRINGIZE_EXPAND(QMI_SWI_M2M_AVMS_SELECTION_REQ_V01),
                                           rc,
                                           avmsResp.resp);

    return result;
}


//--------------------------------------------------------------------------------------------------
/**
 * Get the operation type for the give LWM2M Operation
 *
 * @return opType
 */
//--------------------------------------------------------------------------------------------------
pa_avc_OpType_t pa_avc_GetOpType
(
    pa_avc_LWM2MOperationDataRef_t opRef    ///< [IN] Reference to LWM2M operation
)
{
    return opRef->opType;
}


//--------------------------------------------------------------------------------------------------
/**
 * Get the operation address for the give LWM2M Operation
 */
//--------------------------------------------------------------------------------------------------
void pa_avc_GetOpAddress
(
    pa_avc_LWM2MOperationDataRef_t opRef,   ///< [IN] Reference to LWM2M operation
    const char** objPrefixPtrPtr,           ///< [OUT] Pointer to object prefix string
    int* objIdPtr,                          ///< [OUT] Object id
    int* objInstIdPtr,                      ///< [OUT] Object instance id, or -1 if not available
    int* resourceIdPtr                      ///< [OUT] Resource id, or -1 if not available
)
{
    *objPrefixPtrPtr = opRef->objPrefix;
    *objIdPtr = opRef->objId;
    *objInstIdPtr = opRef->objInstId;
    *resourceIdPtr = opRef->resourceId;
}


//--------------------------------------------------------------------------------------------------
/**
 * Get the operation payload for the give LWM2M Operation
 */
//--------------------------------------------------------------------------------------------------
void pa_avc_GetOpPayload
(
    pa_avc_LWM2MOperationDataRef_t opRef,   ///< [IN] Reference to LWM2M operation
    const uint8_t** payloadPtrPtr,          ///< [OUT] Pointer to payload, or NULL if no payload
    size_t* payloadNumBytesPtr              ///< [OUT] Payload size in bytes, or 0 if no payload.
                                            ///        If payload is a string, this is strlen()
)
{
    // payload pointer returned should be NULL, if there is no payload.
    if ( opRef->payloadNumBytes == 0 )
        *payloadPtrPtr = NULL;
    else
        *payloadPtrPtr = opRef->payload;

    *payloadNumBytesPtr = opRef->payloadNumBytes;
}



//--------------------------------------------------------------------------------------------------
/**
 * Get the token for the given LWM2M Operation
 */
//--------------------------------------------------------------------------------------------------
void pa_avc_GetOpToken
(
    pa_avc_LWM2MOperationDataRef_t opRef,   ///< [IN] Reference to LWM2M operation
    const uint8_t** tokenPtrPtr,            ///< [OUT] Pointer to token, or NULL if no token
    uint8_t* tokenLengthPtr                 ///< [OUT] Token Length bytes, or 0 if no token
)
{
    if ( opRef->tokenLength == 0 )
        *tokenPtrPtr = NULL;
    else
        *tokenPtrPtr = opRef->token;

    *tokenLengthPtr = opRef->tokenLength;
}


//--------------------------------------------------------------------------------------------------
/**
 * Is this a request for reading the first block?
 */
//--------------------------------------------------------------------------------------------------
bool pa_avc_IsFirstBlock
(
    pa_avc_LWM2MOperationDataRef_t opRef   ///< [IN] Reference to LWM2M operation
)
{
    // If block mode is not valid, we consider this as the first block.
    if ( opRef->blockmodeValid && (opRef->blockOffset != 0) )
        return false;
    else
        return true;
}


//--------------------------------------------------------------------------------------------------
/**
 * Respond to the previous LWM2M Operation indication
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on error
 */
//--------------------------------------------------------------------------------------------------
static le_result_t OperationReport
(
    pa_avc_LWM2MOperationDataRef_t opRef,   ///< [IN] Reference to LWM2M operation
    const uint8_t* respPayloadPtr,          ///< [IN] Payload, or NULL if no payload
    size_t respPayloadNumBytes,             ///< [IN] Payload size in bytes, or 0 if no payload.
                                            ///       If payload is a string, this is strlen()
    pa_avc_OpErr_t opError                  ///< [IN] Operation error
)
{
    qmi_client_error_type rc;
    size_t remainingPayloadBytes;

    SWIQMI_DECLARE_MESSAGE(lwm2m_operation_report_req_msg_v01, reportReq);
    SWIQMI_DECLARE_MESSAGE(lwm2m_operation_report_resp_msg_v01, reportResp);

    LE_PRINT_VALUE("%i", opRef->opType);
    LE_PRINT_VALUE("%s", opRef->objPrefix);
    LE_PRINT_VALUE("%i", opRef->objId);
    LE_PRINT_VALUE("%i", opRef->objInstId);
    LE_PRINT_VALUE("%i", opRef->resourceId);

    LE_PRINT_VALUE("%i", respPayloadNumBytes);
    LE_PRINT_VALUE("%i", opError);

    // Fill in the request message
    reportReq.op_type = opRef->opType;

    // TODO: This check for "lwm2m" used to be a work-around for a modem issue.  It seems that
    //       this is no longer needed, but log a message for now, just in case.  Further testing
    //       is needed before this is removed completely.  As long as we never get the "lwm2m"
    //       prefix from the modem, this should not be a problem.
    if (strcmp(opRef->objPrefix, "lwm2m") == 0)
    {
        LE_WARN("lwm2m prefix being sent to modem");
        //opReq.obj_prefix_len = 0;
    }
    else
    {
        // It's an error if the complete prefix can't be copied.
        if ( le_utf8_Copy( reportReq.obj_prefix,
                           opRef->objPrefix,
                           sizeof(reportReq.obj_prefix),
                           NULL ) != LE_OK )
        {
            LE_ERROR("object prefix too large: length=%i", strlen(opRef->objPrefix) );
            return LE_FAULT;
        }
        reportReq.obj_prefix_len = strlen(opRef->objPrefix);
    }

    reportReq.obj_id = opRef->objId;

    if ( opRef->objInstId != -1 )
    {
        reportReq.obj_iid_valid = true;
        reportReq.obj_iid = opRef->objInstId;
    }

    if ( opRef->resourceId != -1 )
    {
        reportReq.res_id_valid = true;
        reportReq.res_id = opRef->resourceId;
    }

    LE_PRINT_VALUE("%i", opRef->blockmodeValid);
    LE_PRINT_VALUE("%i", opRef->blockOffset);
    LE_PRINT_VALUE("%i", opRef->blockSize);

    if ( (respPayloadPtr != NULL) && (respPayloadNumBytes > 0) )
    {
        // If the firmware has set the optional tlv (blockmode_valid), it supports block transfer
        if ( opRef->blockmodeValid )
        {
            remainingPayloadBytes = respPayloadNumBytes - opRef->blockOffset;
            uint8_t* payloadStartAddress = (uint8_t*)respPayloadPtr + opRef->blockOffset;

            if ( remainingPayloadBytes < opRef->blockSize )
            {
                LE_DEBUG("Block transfer mode - last block.");
                reportReq.payload_valid = true;
                reportReq.payload_len = remainingPayloadBytes;
                reportReq.blockmode.is_more = false;

                memcpy(reportReq.payload, payloadStartAddress, remainingPayloadBytes );
                // LE_DUMP((uint8_t *)reportReq.payload, remainingPayloadBytes);
            }
            else
            {
                LE_DEBUG("Block transfer mode.");
                reportReq.payload_valid = true;
                reportReq.payload_len = opRef->blockSize;
                reportReq.blockmode.is_more = true;

                memcpy(reportReq.payload, payloadStartAddress, opRef->blockSize );
                // LE_DUMP((uint8_t *)reportReq.payload, opRef->blockSize);
            }
        }
        else
        {
            if( respPayloadNumBytes > PAYLOAD_MAX_LEN_NO_BLOCKMODE )
            {
                // If the block mode flag is not set by firmware, limit the payload
                // size to 1024 bytes.
                LE_ERROR("Overflow error.");
                opError = PA_AVC_OPERR_OVERFLOW;
            }
            else
            {
                // We can fit in the payload within the max allowed payload length.
                // Send the payload to server.
                reportReq.payload_valid = true;
                reportReq.payload_len = respPayloadNumBytes;
                if (respPayloadNumBytes > strlen((const char *)respPayloadPtr))
                {
                    LE_ERROR("respPayloadNumBytes is more than respPayloadPtr");
                    return LE_FAULT;
                }

                memcpy(reportReq.payload, respPayloadPtr, respPayloadNumBytes);
                // LE_DUMP((uint8_t *)reportReq.payload, respPayloadNumBytes);
            }
        }
    }

#ifdef LEGATO_FEATURE_OBSERVE
    if ( (opRef->token != NULL) && (opRef->tokenLength > 0) && (opRef->tokenLength <= TOKEN_MAX_LENGTH))
    {
        reportReq.observe_token_valid = true;
        reportReq.observe_token_len = opRef->tokenLength;
        memcpy(reportReq.observe_token, opRef->token, opRef->tokenLength);

        LE_PRINT_VALUE("%i", opRef->tokenLength);
        LE_DUMP((uint8_t*)opRef->token, opRef->tokenLength);

        reportReq.content_type_valid = true;
        reportReq.content_type = opRef->contentType;

        LE_PRINT_VALUE("%i", reportReq.content_type);
    }
#endif

    if ( opError != PA_AVC_OPERR_NO_ERROR )
    {
        reportReq.op_err_valid = true;
        reportReq.op_err = opError;
    }

    // The TLV encoded flag is always set.
    // The convention used by Legato is
    // - Any single field read will be sent out as an ASCII string and
    // For example a read of lwm2m/$objectID/$instanceID/$name will give "appName" as string
    // and lwm2m/$objectID/$instanceID/$update state will return 0x30 (ascii for '0')
    // - Read on multiple fields (/lwm2m/9 for instance) will be sent out in TLV encoded format

    reportReq.blockmode_valid = opRef->blockmodeValid;
    reportReq.blockmode.is_tlv_encoded = true;

    LE_PRINT_VALUE("%i", reportReq.blockmode_valid);
    LE_PRINT_VALUE("%i", reportReq.blockmode.is_more);
    LE_PRINT_VALUE("%i", reportReq.blockmode.is_tlv_encoded);

    LE_PRINT_VALUE("%i", reportReq.op_err_valid);
    LE_PRINT_VALUE("%i", reportReq.op_err);;

    LE_PRINT_VALUE("%i", reportReq.payload_valid);
    LE_PRINT_VALUE("%i", reportReq.payload_len);

    // Send the message and handle the response
    rc = qmi_client_send_msg_sync(QapiLWM2MClient,
                                  QMI_LWM2M_OPERATION_REPORT_REQ_V01,
                                  &reportReq, sizeof(reportReq),
                                  &reportResp, sizeof(reportResp),
                                  COMM_LONG_PLATFORM_TIMEOUT);

    le_result_t result = swiQmi_OEMCheckResponseCode(
                                           STRINGIZE_EXPAND(QMI_LWM2M_OPERATION_REPORT_REQ_V01),
                                           rc,
                                           reportResp.resp);

    return result;
}


//--------------------------------------------------------------------------------------------------
/**
 * Respond to the previous LWM2M Operation indication with success
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on error
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_avc_OperationReportSuccess
(
    pa_avc_LWM2MOperationDataRef_t opRef,   ///< [IN] Reference to LWM2M operation
    const uint8_t* respPayloadPtr,          ///< [IN] Payload, or NULL if no payload
    size_t respPayloadNumBytes              ///< [IN] Payload size in bytes, or 0 if no payload.
                                            ///       If payload is a string, this is strlen()
)
{
    return OperationReport(opRef, respPayloadPtr, respPayloadNumBytes, PA_AVC_OPERR_NO_ERROR);
}


//--------------------------------------------------------------------------------------------------
/**
 * Respond to the previous LWM2M Operation indication with error
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on error
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_avc_OperationReportError
(
    pa_avc_LWM2MOperationDataRef_t opRef,   ///< [IN] Reference to LWM2M operation
    pa_avc_OpErr_t opError                  ///< [IN] Operation error
)
{
    return OperationReport(opRef, NULL, 0, opError);
}


//--------------------------------------------------------------------------------------------------
/**
 * Send updated list of assets and asset instances
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on error
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_avc_RegistrationUpdate
(
    const char* updatePtr,          ///< [IN] List formatted for QMI_LWM2M_REG_UPDATE_REQ
    size_t updateNumBytes,          ///< [IN] Size of the update list
    size_t updateCount              ///< [IN] Count of assets + asset instances
)
{
    qmi_client_error_type rc;

    SWIQMI_DECLARE_MESSAGE(lwm2m_reg_update_req_msg_v01, updateReq);
    SWIQMI_DECLARE_MESSAGE(lwm2m_reg_update_resp_msg_v01, updateResp);

    // This could fail, if the QMI spec changed the size to something smaller.
    if ( updateNumBytes > sizeof(updateReq.update.obj_path) )
    {
        LE_ERROR("updateNumBytes (%zu) > sizeof(updateReq.update.obj_path) (%zu)",
                 updateNumBytes,
                 sizeof(updateReq.update.obj_path));
        return LE_FAULT;
    }

    // Fill in the request message
    updateReq.update.update_supported_obj = true;  // TODO: hard-code to true; maybe change later
    updateReq.update.num_supported_obj = updateCount;
    updateReq.update.obj_path_len = updateNumBytes;
    memcpy(updateReq.update.obj_path, updatePtr, updateNumBytes);

    // Send the message and handle the response
    rc = qmi_client_send_msg_sync(QapiLWM2MClient,
                                  QMI_LWM2M_REG_UPDATE_REQ_V01,
                                  &updateReq, sizeof(updateReq),
                                  &updateResp, sizeof(updateResp),
                                  COMM_LONG_PLATFORM_TIMEOUT);

    le_result_t result = swiQmi_OEMCheckResponseCode(
                                           STRINGIZE_EXPAND(QMI_LWM2M_REG_UPDATE_REQ_V01),
                                           rc,
                                           updateResp.resp);

    return result;
}


//--------------------------------------------------------------------------------------------------
/**
 * Start a download from the specified URI
 *
 * The status of the download will passed to handlerRef:
 *  - LE_AVC_DOWNLOAD_IN_PROGRESS, if the download is in progress
 *  - LE_AVC_DOWNLOAD_COMPLETE, if the download completed successfully
 *  - LE_AVC_DOWNLOAD_FAILED, if there was an error, and the download was stopped
 * Note that handlerRef will be cleared after download complete or failed.
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on error
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_avc_StartURIDownload
(
    const char* uriPtr,                         ///< [IN] URI giving location of file to download
    pa_avc_URIDownloadHandlerFunc_t handlerRef  ///< [IN] Handler to receive download status,
)
{
    qmi_client_error_type rc;

    SWIQMI_DECLARE_MESSAGE(lwm2m_download_req_msg_v01, downloadReq);
    SWIQMI_DECLARE_MESSAGE(lwm2m_download_resp_msg_v01, downloadResp);

    // Add one to string length, to account for the null character
    int uriNumBytes = strlen(uriPtr);

    // Fill in the request message
    downloadReq.action = 0x01;   // Download request
    downloadReq.pkg_uri_valid = true;
    downloadReq.pkg_uri.uri_len = uriNumBytes;
    memcpy(downloadReq.pkg_uri.uri, uriPtr, uriNumBytes);

    // Send the message and handle the response
    rc = qmi_client_send_msg_sync(QapiLWM2MClient,
                                  QMI_LWM2M_DOWNLOAD_REQ_V01,
                                  &downloadReq, sizeof(downloadReq),
                                  &downloadResp, sizeof(downloadResp),
                                  COMM_LONG_PLATFORM_TIMEOUT);

    le_result_t result = swiQmi_OEMCheckResponseCode(
                                           STRINGIZE_EXPAND(QMI_LWM2M_DOWNLOAD_REQ_V01),
                                           rc,
                                           downloadResp.resp);

    if ( result == LE_OK )
    {
        // A download was successfully started; handlerRef will be used to return the result
        URIDownloadHandlerRef = handlerRef;
    }

    return result;
}


//--------------------------------------------------------------------------------------------------
/**
 * Add the URIDownload Handler Ref
 *
 * The status of the download will passed to handlerRef:
 *  - LE_AVC_DOWNLOAD_IN_PROGRESS, if the download is in progress
 *  - LE_AVC_DOWNLOAD_COMPLETE, if the download completed successfully
 *  - LE_AVC_DOWNLOAD_FAILED, if there was an error, and the download was stopped
 * Note that handlerRef will be cleared after download complete or failed.
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on error
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_avc_AddURIDownloadStatusHandler
(
    pa_avc_URIDownloadHandlerFunc_t handlerRef  ///< [IN] Handler to receive download status,
)
{
    // Check if there is already a download in progress
    if ( URIDownloadHandlerRef != NULL )
    {
        return LE_FAULT;
    }
    else
    {
        URIDownloadHandlerRef = handlerRef;
    }
    return LE_OK;
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
le_result_t pa_avc_ReadImage
(
    int* fdPtr         ///< [OUT] File descriptor for the image, ready for reading.
)
{
    // This is necessary to make building easier, rather than having the caller directly
    // call pa_fwupdate_Read(). That way, the caller doesn't need to directly use the modem PA.
    return pa_fwupdate_Read(fdPtr);
}


//--------------------------------------------------------------------------------------------------
/**
 * This function registers a handler for AVMS update status
 *
 * - If the handler is NULL, then the previous handler will be removed.
 * - If this is the first time a handler is registered, then the QMI indication will be enabled.
 */
//--------------------------------------------------------------------------------------------------
void pa_avc_SetAVMSMessageHandler
(
    pa_avc_AVMSMessageHandlerFunc_t handlerRef          ///< [IN] Handler for AVMS update status
)
{
    // We only need to enable QMI indications once, the first time a handler is registered.
    static bool IsIndicationEnabled = false;

    // Check if the old handler is replaced or deleted.
    if ( handlerRef == NULL )
    {
        LE_INFO("Clearing old handler");
        AVMSMessageHandlerRef = NULL;
    }
    else
    {
        // New handler is being added
        AVMSMessageHandlerRef = handlerRef;

        // Get the current legato thread, since we need it for later.  The registered handler
        // will be called from this thread.  Assume that all three "Set" functions will be
        // called from the same Legato thread.
        LegatoThread = le_thread_GetCurrent();

        if ( ! IsIndicationEnabled )
        {
            // Register indication handler for QMI_SWI_M2M_AVMS_EVENT_IND_V01
            swiQmi_AddIndicationHandler(AvmsIndicationHandler,
                                        QMI_SERVICE_MGS,
                                        QMI_SWI_M2M_AVMS_EVENT_IND_V01,
                                        NULL);

            // Enable the AVMS event report (QMI_SWI_M2M_AVMS_EVENT_IND_V01)
            qmi_client_error_type rc;

            // There is no request message, only a response message.
            swi_m2m_avms_set_event_report_resp_msg_v01 setResp = { {0} };

            rc = qmi_client_send_msg_sync(QapiM2mGeneralClient,
                                          QMI_SWI_M2M_AVMS_SET_EVENT_REPORT_REQ_V01,
                                          NULL, 0,
                                          &setResp, sizeof(setResp),
                                          COMM_LONG_PLATFORM_TIMEOUT);

            le_result_t result = swiQmi_OEMCheckResponseCode(
                                        STRINGIZE_EXPAND(QMI_SWI_M2M_AVMS_SET_EVENT_REPORT_REQ_V01),
                                        rc, setResp.resp);
            if ( result != LE_OK )
            {
                LE_ERROR("Could not enable QMI M2M AVMS event reports");
                return;
            }

            IsIndicationEnabled = true;
        }
    }
}


//--------------------------------------------------------------------------------------------------
/**
 * This function registers a handler for LWM2M Operation
 *
 * If the handler is NULL, then the previous handler will be removed.
 */
//--------------------------------------------------------------------------------------------------
void pa_avc_SetLWM2MOperationHandler
(
    pa_avc_LWM2MOperationHandlerFunc_t handlerRef       ///< [IN] Handler for LWM2M Operation
)
{
    // Check if the old handler is replaced or deleted.
    if ( handlerRef == NULL )
    {
        LE_INFO("Clearing old handler");
        LWM2MOperationHandlerRef = NULL;
    }
    else
    {
        // New handler is being added
        LWM2MOperationHandlerRef = handlerRef;

        // Get the current legato thread, since we need it for later.  The registered handler
        // will be called from this thread.  Assume that all three "Set" functions will be
        // called from the same Legato thread.
        LegatoThread = le_thread_GetCurrent();
    }
}


//--------------------------------------------------------------------------------------------------
/**
 * This function registers a handler for LWM2M Update Required
 *
 * If the handler is NULL, then the previous handler will be removed.
 */
//--------------------------------------------------------------------------------------------------
void pa_avc_SetLWM2MUpdateRequiredHandler
(
    pa_avc_LWM2MUpdateRequiredHandlerFunc_t handlerRef  ///< [IN] Handler for LWM2M Update Required
)
{
    // Check if the old handler is replaced or deleted.
    if ( handlerRef == NULL )
    {
        LE_INFO("Clearing old handler");
        UpdateRequiredHandlerRef = NULL;
    }
    else
    {
        // New handler is being added
        UpdateRequiredHandlerRef = handlerRef;

        // Get the current legato thread, since we need it for later.  The registered handler
        // will be called from this thread.  Assume that all three "Set" functions will be
        // called from the same Legato thread.
        LegatoThread = le_thread_GetCurrent();
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * This function sets up the modem activity timer. The timeout will default to 20 seconds if
 * user defined value doesn't exist or if the defined value is less than 0.
 */
//--------------------------------------------------------------------------------------------------
void pa_avc_SetModemActivityTimeout
(
    int timeout     ///< [IN] Timeout
)
{
    // After a session is started, if there has been no activity from the modem within the timer
    // interval, then report LE_AVC_NO_UPDATE.  The value of 20s is chosen to match the value used
    // by the modem FW for similar functionality, but this can be overridden by setting a integer
    // value at /apps/avcService/modemInactivityTimeout
    le_clk_Time_t timerInterval = { .sec=20, .usec=0 };

    if (timeout > 0)
    {
        timerInterval.sec = timeout;
    }

    LE_DEBUG("Modem activity timeout set to %d seconds.", (int)timerInterval.sec);

    ModemActivityTimerRef = le_timer_Create("Activity timer");
    le_timer_SetInterval(ModemActivityTimerRef, timerInterval);
    le_timer_SetHandler(ModemActivityTimerRef, ModemActivityTimerHandler);
}



//--------------------------------------------------------------------------------------------------
/**
 * Function to read the last http status.
 *
 * @return
 *      - HttpStatus as defined in RFC 7231, Section 6.
 */
//--------------------------------------------------------------------------------------------------
uint16_t pa_avc_GetHttpStatus
(
    void
)
{
    return CurrentHttpStatus;
}


//--------------------------------------------------------------------------------------------------
/**
 * Function to read the session type.
 *
 * @return
 *      - Session type
 */
//--------------------------------------------------------------------------------------------------
le_avc_SessionType_t pa_avc_GetSessionType
(
    void
)
{
    return CurrentSessionType;
}


//--------------------------------------------------------------------------------------------------
/**
 * Function to read APN configuration.
 *
 * @return
 *      - LE_OK on success.
 *      - LE_FAULT if there is any error while reading.
 *      - LE_OVERFLOW if the buffer provided is too small.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_avc_GetApnConfig
(
    char* apnName,                      ///< [OUT] APN name string
    size_t apnNameNumElements,          ///< [IN]  Buffer size
    char* userName,                     ///< [OUT] User name string
    size_t uNameNumElements,            ///< [IN]  Buffer size
    char* userPwd,                      ///< [OUT] Password string
    size_t userPwdNumElements           ///< [IN]  Buffer size
)
{

#ifdef LEGATO_FEATURE_AVMS_CONFIG

    le_result_t rc;

    SWIQMI_DECLARE_MESSAGE(swi_m2m_avms_get_setting_resp_msg_v01, avmsSettingGetResp);

    // Read AVMS settings.
    rc = qmi_client_send_msg_sync(QapiM2mGeneralClient,
                                  QMI_SWI_M2M_AVMS_GET_SETTINGS_REQ_V01,
                                  NULL, 0,
                                  &avmsSettingGetResp, sizeof(avmsSettingGetResp),
                                  COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (swiQmi_OEMCheckResponseCode(STRINGIZE_EXPAND(QMI_SWI_M2M_AVMS_GET_SETTINGS_REQ_V01),
                                    rc,
                                    avmsSettingGetResp.resp) != LE_OK )
    {
        LE_ERROR("Failed to read AVMS settings.");
        return LE_FAULT;
    }

    if (avmsSettingGetResp.apn_config_valid)
    {
        if (le_utf8_Copy(apnName,
                         avmsSettingGetResp.apn_config.apn,
                         apnNameNumElements,
                         NULL) != LE_OK)
        {
            LE_ERROR("Buffer overflow while reading APN name.");
            return LE_OVERFLOW;
        }
        if (le_utf8_Copy(userName,
                         avmsSettingGetResp.apn_config.uname,
                         uNameNumElements,
                         NULL) != LE_OK)
        {
            LE_ERROR("Buffer overflow while reading APN user name.");
            return LE_OVERFLOW;
        }
        if (le_utf8_Copy(userPwd,
                         avmsSettingGetResp.apn_config.pwd,
                         userPwdNumElements,
                         NULL) != LE_OK)
        {
            LE_ERROR("Buffer overflow while reading APN password.");
            return LE_OVERFLOW;
        }
    }
    else
    {
        LE_ERROR("Failed to read APN config.");
        return LE_FAULT;
    }

    return LE_OK;

#else
    return LE_FAULT;
#endif
}



//--------------------------------------------------------------------------------------------------
/**
 * Function to write APN configuration.
 *
 * @return
 *      - LE_OK on success.
 *      - LE_FAULT if not able to write the APN configuration.
 *      - LE_OVERFLOW if one of the input strings is too long.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_avc_SetApnConfig
(
    const char* apnName,                ///< [IN] APN name string
    const char* userName,               ///< [IN] User name string
    const char* userPwd                 ///< [IN] Password string
)
{

#ifdef LEGATO_FEATURE_AVMS_CONFIG

    le_result_t rc;

    SWIQMI_DECLARE_MESSAGE(swi_m2m_avms_set_setting_req_msg_v01, avmsSettingSetReq);
    SWIQMI_DECLARE_MESSAGE(swi_m2m_avms_set_setting_resp_msg_v01, avmsSettingSetResp);

    avmsSettingSetReq.apn_config_valid = true;

    if (le_utf8_Copy(avmsSettingSetReq.apn_config.apn,
                     apnName,
                     sizeof(avmsSettingSetReq.apn_config.apn),
                     NULL) != LE_OK)
    {
        LE_ERROR("APN name too long.");
        return LE_OVERFLOW;
    }

    if (le_utf8_Copy(avmsSettingSetReq.apn_config.uname,
                     userName,
                     sizeof(avmsSettingSetReq.apn_config.uname),
                     NULL) != LE_OK)
    {
        LE_ERROR("User name too long.");
        return LE_OVERFLOW;
    }

    if (le_utf8_Copy(avmsSettingSetReq.apn_config.pwd,
                     userPwd,
                     sizeof(avmsSettingSetReq.apn_config.pwd),
                     NULL) != LE_OK)
    {
        LE_ERROR("Password too long.");
        return LE_OVERFLOW;
    }

    avmsSettingSetReq.apn_config.apn_len = strlen(apnName);
    avmsSettingSetReq.apn_config.uname_len = strlen(userName);
    avmsSettingSetReq.apn_config.pwd_len = strlen(userPwd);

    // Send request to fw.
    rc = qmi_client_send_msg_sync(QapiM2mGeneralClient,
                                  QMI_SWI_M2M_AVMS_SET_SETTINGS_REQ_V01,
                                  &avmsSettingSetReq, sizeof(avmsSettingSetReq),
                                  &avmsSettingSetResp, sizeof(avmsSettingSetResp),
                                  COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (swiQmi_OEMCheckResponseCode( STRINGIZE_EXPAND(QMI_SWI_M2M_AVMS_SET_SETTINGS_REQ_V01),
                                     rc,
                                     avmsSettingSetResp.resp) != LE_OK )
    {
        LE_ERROR("Failed to set APN config.");
        return LE_FAULT;
    }

    return LE_OK;
#else
    return LE_FAULT;
#endif
}



//--------------------------------------------------------------------------------------------------
/**
 * Function to read the retry timers.
 *
 * @return
 *      - LE_OK on success.
 *      - LE_FAULT if not able to read the timers.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_avc_GetRetryTimers
(
    uint16_t* timerValuePtr,                ///< [OUT] Array of 8 retry timers in minutes
    size_t* numTimers                       ///< [OUT] Number of retry timers
)
{
#ifdef LEGATO_FEATURE_AVMS_CONFIG

    le_result_t rc;
    int index;

    SWIQMI_DECLARE_MESSAGE(swi_m2m_avms_get_setting_resp_msg_v01, avmsSettingGetResp);

    // Read AVMS settings.
    rc = qmi_client_send_msg_sync(QapiM2mGeneralClient,
                                  QMI_SWI_M2M_AVMS_GET_SETTINGS_REQ_V01,
                                  NULL, 0,
                                  &avmsSettingGetResp, sizeof(avmsSettingGetResp),
                                  COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (swiQmi_OEMCheckResponseCode( STRINGIZE_EXPAND(QMI_SWI_M2M_AVMS_GET_SETTINGS_REQ_V01),
                                    rc,
                                    avmsSettingGetResp.resp) != LE_OK )
    {
        LE_ERROR("Failed to read AVMS settings.");
        return LE_FAULT;
    }

    if (NUM_ARRAY_MEMBERS(avmsSettingGetResp.retry_timer) != LE_AVC_NUM_RETRY_TIMERS)
    {
        LE_ERROR("Retry timers count doesn't match.");
        return LE_FAULT;
    }

    if (avmsSettingGetResp.retry_timer_valid)
    {
        for (index = 0; index < LE_AVC_NUM_RETRY_TIMERS; index++)
        {
            timerValuePtr[index] = avmsSettingGetResp.retry_timer[index];
        }
    }
    else
    {
        LE_ERROR("Failed to read retry timer.");
        return LE_FAULT;
    }

    *numTimers = LE_AVC_NUM_RETRY_TIMERS;
    return LE_OK;
#else
    return LE_FAULT;
#endif
}


//--------------------------------------------------------------------------------------------------
/**
 * Function to set the retry timers.
 *
 * @return
 *      - LE_OK on success.
 *      - LE_FAULT if not able to write the timers.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_avc_SetRetryTimers
(
    const uint16_t* timerValuePtr,                  ///< [IN] Array of 8 retry timers in minutes
    size_t numTimers                                ///< [IN] Number of retry timers
)
{
#ifdef LEGATO_FEATURE_AVMS_CONFIG

    le_result_t rc;
    int index;

    SWIQMI_DECLARE_MESSAGE(swi_m2m_avms_set_setting_req_msg_v01, avmsSettingSetReq);
    SWIQMI_DECLARE_MESSAGE(swi_m2m_avms_set_setting_resp_msg_v01, avmsSettingSetResp);

    if (numTimers != LE_AVC_NUM_RETRY_TIMERS)
    {
        LE_ERROR("All the retry timers must be initialized together.");
        return LE_FAULT;
    }

    for (index= 0; index < LE_AVC_NUM_RETRY_TIMERS; index++)
    {
        avmsSettingSetReq.retry_timer[index] = timerValuePtr[index];
    }

    avmsSettingSetReq.retry_timer_valid = true;

    // Send request to fw.
    rc = qmi_client_send_msg_sync(QapiM2mGeneralClient,
                                  QMI_SWI_M2M_AVMS_SET_SETTINGS_REQ_V01,
                                  &avmsSettingSetReq, sizeof(avmsSettingSetReq),
                                  &avmsSettingSetResp, sizeof(avmsSettingSetResp),
                                  COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (swiQmi_OEMCheckResponseCode( STRINGIZE_EXPAND(QMI_SWI_M2M_AVMS_SET_SETTINGS_REQ_V01),
                                     rc,
                                     avmsSettingSetResp.resp) != LE_OK )
    {
        LE_ERROR("Failed to set retry timers.");
        return LE_FAULT;
    }

    return LE_OK;
#else
    return LE_FAULT;
#endif
}


//--------------------------------------------------------------------------------------------------
/**
 * Function to read the polling timer.
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT if not available
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_avc_GetPollingTimer
(
    uint32_t* pollingTimerPtr                       ///< [OUT] polling timer value in minutes
)
{
#ifdef LEGATO_FEATURE_AVMS_CONFIG

    le_result_t rc;

    SWIQMI_DECLARE_MESSAGE(swi_m2m_avms_get_setting_resp_msg_v01, avmsSettingGetResp);

    // Read AVMS settings.
    rc = qmi_client_send_msg_sync(QapiM2mGeneralClient,
                                  QMI_SWI_M2M_AVMS_GET_SETTINGS_REQ_V01,
                                  NULL, 0,
                                  &avmsSettingGetResp, sizeof(avmsSettingGetResp),
                                  COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (swiQmi_OEMCheckResponseCode( STRINGIZE_EXPAND(QMI_SWI_M2M_AVMS_GET_SETTINGS_REQ_V01),
                                     rc,
                                     avmsSettingGetResp.resp) != LE_OK )
    {
        LE_ERROR("Failed to read AVMS settings.");
        return LE_FAULT;
    }

    if (avmsSettingGetResp.polling_timer_valid)
    {
        *pollingTimerPtr = avmsSettingGetResp.polling_timer;
    }
    else
    {
        LE_ERROR("Failed to read polling timer.");
        return LE_FAULT;
    }

    return LE_OK;
#else
    return LE_FAULT;
#endif
}


//--------------------------------------------------------------------------------------------------
/**
 * Function to set the polling timer.
 *
 * @return
 *      - LE_OK on success.
 *      - LE_FAULT if not able to write the timers.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_avc_SetPollingTimer
(
    uint32_t pollingTimer                           ///< [IN] polling timer value in minutes
)
{
#ifdef LEGATO_FEATURE_AVMS_CONFIG

    le_result_t rc;

    SWIQMI_DECLARE_MESSAGE(swi_m2m_avms_set_setting_req_msg_v01, avmsSettingSetReq);
    SWIQMI_DECLARE_MESSAGE(swi_m2m_avms_set_setting_resp_msg_v01, avmsSettingSetResp);

    avmsSettingSetReq.polling_timer = pollingTimer;
    avmsSettingSetReq.polling_timer_valid = true;

    // Send request to fw.
    rc = qmi_client_send_msg_sync(QapiM2mGeneralClient,
                                  QMI_SWI_M2M_AVMS_SET_SETTINGS_REQ_V01,
                                  &avmsSettingSetReq, sizeof(avmsSettingSetReq),
                                  &avmsSettingSetResp, sizeof(avmsSettingSetResp),
                                  COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (swiQmi_OEMCheckResponseCode( STRINGIZE_EXPAND(QMI_SWI_M2M_AVMS_SET_SETTINGS_REQ_V01),
                                     rc,
                                     avmsSettingSetResp.resp) != LE_OK )
    {
        LE_ERROR("Failed to set polling timer.");
        return LE_FAULT;
    }

    return LE_OK;
#else
    return LE_FAULT;
#endif
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to read the user agreement status
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT if not available
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_avc_GetUserAgreement
(
    pa_avc_UserAgreement_t* configPtr
)
{
    le_result_t rc;

    SWIQMI_DECLARE_MESSAGE(swi_m2m_avms_get_setting_resp_msg_v01, avmsSettingGetResp);

    // Read AVMS settings.
    rc = qmi_client_send_msg_sync(QapiM2mGeneralClient,
                                  QMI_SWI_M2M_AVMS_GET_SETTINGS_REQ_V01,
                                  NULL, 0,
                                  &avmsSettingGetResp, sizeof(avmsSettingGetResp),
                                  COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (swiQmi_OEMCheckResponseCode( STRINGIZE_EXPAND(QMI_SWI_M2M_AVMS_GET_SETTINGS_REQ_V01),
                                     rc,
                                     avmsSettingGetResp.resp) != LE_OK )
    {
        LE_ERROR("Failed to read AVMS settings.");
        return LE_FAULT;
    }

    LE_DEBUG("auto connect = %d", avmsSettingGetResp.fw_autodload);
    LE_DEBUG("auto download = %d", avmsSettingGetResp.fw_autoupdate);
    LE_DEBUG("auto update = %d", avmsSettingGetResp.autoconnect);

    // Copy configuration
    configPtr->isAutoDownload = avmsSettingGetResp.fw_autodload;
    configPtr->isAutoUpdate = avmsSettingGetResp.fw_autoupdate;
    configPtr->isAutoConnect = avmsSettingGetResp.autoconnect;

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Component Init
 */
//--------------------------------------------------------------------------------------------------
COMPONENT_INIT
{
    // Initialize the required services
    if (swiQmi_InitServices(QMI_SERVICE_MGS) != LE_OK)
    {
        LE_ERROR("Could not initialize QMI M2M General Service.");
        return;
    }

    if (swiQmi_InitServices(QMI_SERVICE_LWM2M) != LE_OK)
    {
        LE_ERROR("Could not initialize QMI LWM2M Service.");
        return;
    }

    // Get the qmi client service handles for later use.
    if ((QapiM2mGeneralClient = swiQmi_GetClientHandle(QMI_SERVICE_MGS)) == NULL)
    {
        // This should not happen, since we just did the init above, but log an error anyways.
        LE_ERROR("Could not get service handle for QMI M2M General Service.");
        return;
    }

    if ((QapiLWM2MClient = swiQmi_GetClientHandle(QMI_SERVICE_LWM2M)) == NULL)
    {
        // This should not happen, since we just did the init above, but log an error anyways.
        LE_ERROR("Could not get service handle for QMI LWM2M Service.");
        return;
    }

    // Create pools for reporting info the handlers
    AVMSUpdateStatusPool = le_mem_CreatePool("AVMSUpdateStatusPool",
                                             sizeof(UpdateStatusData_t));

    LWM2MOperationPool = le_mem_CreatePool("LWM2MOperationPool",
                                           sizeof(LWM2MOperationData_t));

    // Register indication handler for QMI_LWM2M_OPERATION_IND_V01.
    swiQmi_AddIndicationHandler(LWM2MOperationIndicationHandler,
                                QMI_SERVICE_LWM2M,
                                QMI_LWM2M_OPERATION_IND_V01,
                                NULL);

    // Register indication handler for QMI_LWM2M_UPDATE_REQUIRED_IND_V01.
    swiQmi_AddIndicationHandler(LWM2MUpdateRequiredIndicationHandler,
                                QMI_SERVICE_LWM2M,
                                QMI_LWM2M_UPDATE_REQUIRED_IND_V01,
                                NULL);
}

