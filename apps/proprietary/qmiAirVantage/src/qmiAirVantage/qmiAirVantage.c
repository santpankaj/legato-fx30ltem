/**
 * @file qmiAirVantage.c: Implements QMI commands for AVMS LWM2M client
 *
 * <HR>
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#include <legato.h>
#include <interfaces.h>
#include "qmi_idl_lib.h"
#include "qmi_csi.h"
#include "qmi_sap.h"
#include "qmi.h"
#include "swi_legato_service_v01.h"

//--------------------------------------------------------------------------------------------------
/**
 * Default defer time in minutes for le_avc_Defer* actions
 */
//--------------------------------------------------------------------------------------------------
#define DEFAULT_DEFER_IN_MIN (5)

//--------------------------------------------------------------------------------------------------
/**
 * QMI request dispatch
 */
//--------------------------------------------------------------------------------------------------
enum{
    START,
    STOP,
    SELECT,
    GET_SETTINGS,
    SET_SETTINGS
};

//--------------------------------------------------------------------------------------------------
/**
 * FOTA's Binary Type
 */
//--------------------------------------------------------------------------------------------------
typedef enum{
    UNDEFINED_BINARY_TYPE,
    FIRMWARE_BINARY_TYPE,
    APP_BINARY_TYPE,
    LEGATO_BINARY_TYPE,
}
BinaryType_t;

//--------------------------------------------------------------------------------------------------
/**
 * QMI to AVC User Response struct definition
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    uint8_t         userInput;
    uint8_t         withDeferTime;
    uint32_t        deferTime;
}
Qmi2AvcSelect_t;

//--------------------------------------------------------------------------------------------------
/**
 * QMI to AVC API dispatch struct definition
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    uint8_t         action;
    qmi_req_handle  reqHandle;
    int             msgId;
    union
    {
        Qmi2AvcSelect_t  selectPara;
        swi_legato_avms_set_setting_req_msg_v01 settingPara;
    };
}
Qmi2Avc_t;

//--------------------------------------------------------------------------------------------------
/**
 * Firmware update state defined in swi_leagto_service_v01.h
 */
//--------------------------------------------------------------------------------------------------
enum {
    NO_FW_AVAIL = 1,
    QUERY_FW_DOWNLOAD,
    DOWNLOADING_FW,
    FW_DOWNLOADED,
    QUERY_FW_UPDATE,
    FW_UPDATING,
    FW_UPDATED
};

//--------------------------------------------------------------------------------------------------
/**
 * Represent when user input is required
 */
//--------------------------------------------------------------------------------------------------
enum{
    NOT_EXPECTING_USER_INPUT,
    EXPECTING_USER_INPUT
};

//--------------------------------------------------------------------------------------------------
/**
 * QMI indication event
 */
//--------------------------------------------------------------------------------------------------
enum{
    START_SESSION = 0x14,
    AUTH_SESSION,
    STOP_SESSION
};

//--------------------------------------------------------------------------------------------------
/**
 * User replace to prompt
 */
//--------------------------------------------------------------------------------------------------
enum{
    ACCEPT = 1,
    REJECT,
    DEFER
};

//--------------------------------------------------------------------------------------------------
/**
 * Session start response
 */
//--------------------------------------------------------------------------------------------------
enum{
    AVALABLE = 1,
    NOT_AVAILABLE,
    CHECK_TIMED_OUT
};

//--------------------------------------------------------------------------------------------------
/**
 * QMI client info type
 */
//--------------------------------------------------------------------------------------------------
typedef struct {
    qmi_client_handle clnt;
}
ClientInfoType_t;

//--------------------------------------------------------------------------------------------------
/**
 * QMI service context type
 */
//--------------------------------------------------------------------------------------------------
typedef struct {
    volatile int            cleanup;
    qmi_csi_service_handle  service_handle;
    qmi_sap_client_handle   qsap_handle;
    int                     num_requests;
}
ServiceContextType_t;

//--------------------------------------------------------------------------------------------------
/**
 * Pool for ClientInfoType_t
 *
 */
//--------------------------------------------------------------------------------------------------
static  le_mem_PoolRef_t                     ClntPool;

static  ServiceContextType_t                 ServiceCookie;
static  ClientInfoType_t*                    QmiClientPtr;
static  le_event_Id_t                        Qmi2AvcEvent;
static  le_thread_Ref_t                      QmiThreadRef;
static  le_data_ConnectionStateHandlerRef_t  DataHandler;
static  le_avc_Status_t                      LastStatus;
static  swi_legato_avms_event_ind_msg_v01    Indication;
static  swi_legato_avms_event_ind_msg_v01*   IndPtr = &Indication;
static  int                                  SessionOnGoing;

//--------------------------------------------------------------------------------------------------
/**
 * send event which translates user reply to AVC APIs
 */
//--------------------------------------------------------------------------------------------------
static void AvcDispatchSelect
(
    uint8_t     action,
    uint8_t     userInput,
    uint8_t     withDeferTime,
    uint32_t    deferTime
)
{
    Qmi2Avc_t trigger;
    trigger.action = action;
    trigger.selectPara.userInput = userInput;
    trigger.selectPara.withDeferTime = withDeferTime;
    trigger.selectPara.deferTime = deferTime;
    le_event_Report(Qmi2AvcEvent, &trigger, sizeof(trigger));
}

//--------------------------------------------------------------------------------------------------
/**
 * send event with QMI context (request handler & message id)
 */
//--------------------------------------------------------------------------------------------------
static void AvcDispatchWithQmiCtx
(
    uint8_t         action,
    qmi_req_handle  reqHandle,
    int             msgId
)
{
    Qmi2Avc_t trigger;
    trigger.action = action;
    trigger.reqHandle = reqHandle;
    trigger.msgId = msgId;
    le_event_Report(Qmi2AvcEvent, &trigger, sizeof(trigger));
}

//--------------------------------------------------------------------------------------------------
/**
 * AirVantage session stop
 * @return
 *  - QMI_CSI_CB_NO_ERR on success
 */
//--------------------------------------------------------------------------------------------------
static qmi_csi_cb_error SessionStop
(
    ClientInfoType_t* pClntInfo,
    qmi_req_handle    reqHandle,
    int               msgId,
    void*             pReq,
    int               reqLen,
    void*             serviceCookie
)
{
    AvcDispatchWithQmiCtx(STOP, reqHandle, msgId);
    return QMI_CSI_CB_NO_ERR;
}

//--------------------------------------------------------------------------------------------------
/**
 * AirVantage session start
 * @return
 *  - QMI_CSI_CB_NO_ERR on success
 */
//--------------------------------------------------------------------------------------------------
static qmi_csi_cb_error SessionStart
(
    ClientInfoType_t* pClntInfo,
    qmi_req_handle    reqHandle,
    int               msgId,
    void*             pReq,
    int               reqLen,
    void*             serviceCookie
)
{
    AvcDispatchWithQmiCtx(START, reqHandle, msgId);
    return QMI_CSI_CB_NO_ERR;
}

//--------------------------------------------------------------------------------------------------
/**
 * Subscribe to QMI indication
 * @return
 *  - QMI_CSI_CB_NO_ERR on success
 *  - QMI_CSI_CB_INTERNAL_ERR on failure
 */
//--------------------------------------------------------------------------------------------------
static qmi_csi_cb_error RegisterIndication
(
    ClientInfoType_t* pClntInfo,        ///< [IN] QMI client info
    qmi_req_handle    reqHandle,        ///< [IN] Opaque handle provided by the infrastructure
                                        ///       to specify a particular transaction and
                                        ///       message
    int               msgId,            ///< [IN] message id
    void*             pReq,             ///< [IN] Decoded request message structure
    int               reqLen,           ///< [IN] Decoded request message structure length
    void*             serviceCookie     ///< [IN] service context pointer
)
{
    qmi_csi_cb_error rc = QMI_CSI_CB_INTERNAL_ERR;

    QmiClientPtr = pClntInfo;

    qmi_csi_error resp_err;

    swi_legato_avms_set_event_report_resp_msg_v01 resp;
    memset(&resp, 0, sizeof(resp));

    resp_err = qmi_csi_send_resp( reqHandle, msgId, &resp, sizeof(resp) );
    if (QMI_CSI_NO_ERR != resp_err)
    {
        LE_ERROR("qmi_csi_send_resp returned error: %d\n", resp_err);
    }
    else
    {
        rc = QMI_CSI_CB_NO_ERR;
    }

    return rc;
}

//--------------------------------------------------------------------------------------------------
/**
 * User Reply via QMI
 * @return
 *  - QMI_CSI_CB_NO_ERR on success
 *  - QMI_CSI_CB_INTERNAL_ERR on failure
 */
//--------------------------------------------------------------------------------------------------
static qmi_csi_cb_error Select
(
    ClientInfoType_t* pClntInfo,
    qmi_req_handle    reqHandle,
    int               msgId,
    void*             pReq,
    int               reqLen,
    void*             serviceCookie
)
{
    qmi_csi_cb_error                        rc = QMI_CSI_CB_INTERNAL_ERR;
    qmi_csi_error                           resp_err;
    swi_legato_avms_selection_resp_msg_v01  resp;
    swi_legato_avms_selection_req_msg_v01*  reqPtr = (swi_legato_avms_selection_req_msg_v01*)pReq;

    LE_INFO("User Input: %d\n", reqPtr->user_input);
    AvcDispatchSelect(SELECT, reqPtr->user_input, reqPtr->defer_time_valid, reqPtr->defer_time);

    memset(&resp, 0, sizeof(resp));
    resp_err = qmi_csi_send_resp( reqHandle, msgId, &resp, sizeof(resp) );
    if (QMI_CSI_NO_ERR != resp_err)
    {
        LE_ERROR("qmi_csi_send_resp returned error: %d\n",resp_err);
    }
    else
    {
        rc = QMI_CSI_CB_NO_ERR;
    }

    return rc;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get AVC settings via QMI
 */
//--------------------------------------------------------------------------------------------------
static qmi_csi_cb_error GetSettings
(
    ClientInfoType_t* pClntInfo,
    qmi_req_handle    reqHandle,
    int               msgId,
    void*             pReq,
    int               reqLen,
    void*             serviceCookie
)
{
    AvcDispatchWithQmiCtx(GET_SETTINGS, reqHandle, msgId);
    return QMI_CSI_CB_NO_ERR;
}

//--------------------------------------------------------------------------------------------------
/**
 * Helper function sending event to call AVC APIs
 */
//--------------------------------------------------------------------------------------------------
static void AvcDispatchSetSettings
(
    swi_legato_avms_set_setting_req_msg_v01 settingPara,
    qmi_req_handle  reqHandle,
    int             msgId
)
{
    Qmi2Avc_t trigger;
    trigger.action = SET_SETTINGS;
    trigger.reqHandle = reqHandle;
    trigger.msgId = msgId;
    trigger.settingPara = settingPara;
    le_event_Report(Qmi2AvcEvent, &trigger, sizeof(trigger));
}

//--------------------------------------------------------------------------------------------------
/**
 * Set AVC settings via QMI
 * @return
 *  - QMI_CSI_CB_NO_ERR on success
 */
//--------------------------------------------------------------------------------------------------
static qmi_csi_cb_error SetSettings
(
    ClientInfoType_t* pClntInfo,
    qmi_req_handle    reqHandle,
    int               msgId,
    void*             pReq,
    int               reqLen,
    void*             serviceCookie
)
{
    swi_legato_avms_set_setting_req_msg_v01*    reqPtr
        = (swi_legato_avms_set_setting_req_msg_v01*) pReq;

    AvcDispatchSetSettings(*reqPtr, reqHandle, msgId);

    return QMI_CSI_CB_NO_ERR;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get Info from last indication cached
 * @return
 *  - QMI_CSI_CB_NO_ERR on success
 *  - QMI_CSI_CB_INTERNAL_ERR on failure
 */
//--------------------------------------------------------------------------------------------------
static qmi_csi_cb_error GetInfo
(
    ClientInfoType_t* pClntInfo,        ///< [IN] QMI client info
    qmi_req_handle    reqHandle,        ///< [IN] Opaque handle provided by the infrastructure
                                        ///       to specify a particular transaction and
                                        ///       message
    int               msgId,            ///< [IN] message id
    void*             pReq,             ///< [IN] Decoded request message structure
    int               reqLen,           ///< [IN] Decoded request message structure length
    void*             serviceCookie     ///< [IN] service context pointer
)
{
    qmi_csi_cb_error rc = QMI_CSI_CB_INTERNAL_ERR;
    QmiClientPtr = pClntInfo;
    qmi_csi_error resp_err;
    swi_legato_avms_session_getinfo_resp_msg_v01 resp;

    memset(&resp, 0, sizeof(resp));
    resp.avms_session_info_valid = Indication.avms_session_info_valid;
    resp.avms_config_info_valid = Indication.avms_config_info_valid;
    resp.avms_notifications_valid = Indication.avms_notifications_valid;

    if (resp.avms_session_info_valid)
    {
        resp.avms_session_info = Indication.avms_session_info;
    }
    if (resp.avms_config_info_valid)
    {
        resp.avms_config_info = Indication.avms_config_info;
    }
    if (resp.avms_notifications_valid)
    {
        resp.avms_notifications = Indication.avms_notifications;
    }

    resp_err = qmi_csi_send_resp( reqHandle, msgId, &resp, sizeof(resp) );
    if (QMI_CSI_NO_ERR != resp_err)
    {
        LE_ERROR("qmi_csi_send_resp returned error: %d\n",resp_err);
    }
    else
    {
        rc = QMI_CSI_CB_NO_ERR;
    }

    return rc;
}


//--------------------------------------------------------------------------------------------------
/**
 * Define a jump table to handle the dispatch of request handler functions
 */
//--------------------------------------------------------------------------------------------------
static qmi_csi_cb_error (* const reqHandle_table[])
(
    ClientInfoType_t* pClntInfo,
    qmi_req_handle    reqHandle,
    int               msgId,
    void*             pReq,
    int               reqLen,
    void*             serviceCookie
) =
{
    SessionStart,
    SessionStop,
    GetInfo,
    NULL,
    Select,
    SetSettings,
    GetSettings,
    RegisterIndication,
};

//--------------------------------------------------------------------------------------------------
/**
 * QMI connect callback
 * @return
 *  - QMI_CSI_NO_ERR on Success
 *  - QMI_CSI_CONN_REFUSED when limit on MAX number of
 *                         clients a service can support is reached.
 */
//--------------------------------------------------------------------------------------------------
static qmi_csi_cb_error ConnectCb
(
    qmi_client_handle  client_handle,
    void*              serviceCookie,
    void**             connection_handle
)
{
    ClientInfoType_t *pClntInfo;
    pClntInfo = le_mem_ForceAlloc(ClntPool); // Freed in DisconnectCb
    if (!pClntInfo)
    {
        return QMI_CSI_CB_CONN_REFUSED;
    }
    pClntInfo->clnt = client_handle;
    *connection_handle = pClntInfo;
    return QMI_CSI_CB_NO_ERR;
}

//--------------------------------------------------------------------------------------------------
/**
 * QMI disconnect callback
 */
//--------------------------------------------------------------------------------------------------
static void DisconnectCb
(
    void* connection_handle,
    void* serviceCookie
)
{
    // Free up memory for the client
    if (connection_handle)
    {
        le_mem_Release(connection_handle); // Malloc in ConnectCb
    }

    // Stop sending indication when client disconnect
    QmiClientPtr = NULL;
}

//--------------------------------------------------------------------------------------------------
/**
 * QMI requset handling
 * @return
 *  - QMI_CSI_NO_ERR on Success
 *  - QMI_IDL_... on Error, see error codes defined in qmi.h
 */
//--------------------------------------------------------------------------------------------------
static qmi_csi_cb_error HandleReqCb
(
    void*           connection_handle,
    qmi_req_handle  reqHandle,
    int             msgId,
    void*           pReq,
    int             reqLen,
    void*           serviceCookie
)
{
    int avmsMsgId;
    qmi_csi_cb_error rc = QMI_CSI_CB_INTERNAL_ERR;
    ClientInfoType_t *pClntInfo = (ClientInfoType_t*)connection_handle;

    avmsMsgId = msgId - 0xA0; // where AVMS message id start from

    if ((avmsMsgId < NUM_ARRAY_MEMBERS(reqHandle_table))
        && (avmsMsgId >= 0) )
    {
        if (reqHandle_table[avmsMsgId])
        {
            ((ServiceContextType_t*)serviceCookie)->num_requests++;
            rc = reqHandle_table[avmsMsgId] (pClntInfo, reqHandle, msgId,\
                    pReq, reqLen, serviceCookie);
        }
        else
        {
            LE_INFO("No function defined to handle request for message ID: %d\n",msgId);
        }
    }
    else
    {
        LE_INFO("Message ID: %d out of range\n",msgId);
    }

    return rc;
}

//--------------------------------------------------------------------------------------------------
/**
 * Register QMI Serivce
 * @return
 *  - service handle pointer on success
 *  - NULL on failure
 */
//--------------------------------------------------------------------------------------------------
static void* QmiRegisterService
(
        qmi_csi_os_params* osParams
)
{
    qmi_idl_service_object_type serviceObject = swi_legato_get_service_object_v01();
    qmi_csi_error rc = QMI_CSI_INTERNAL_ERR;
    qmi_client_os_params qsapOsParams;

    rc = qmi_csi_register(serviceObject, ConnectCb,
                          DisconnectCb, (qmi_csi_process_req)HandleReqCb,
                          &ServiceCookie, osParams, &ServiceCookie.service_handle);

    if (QMI_NO_ERR != rc)
    {
        LE_ERROR("qmi_csi_register returns %d", rc );
        return NULL;
    }

    rc = qmi_sap_register(serviceObject, &qsapOsParams,
                          &ServiceCookie.qsap_handle);
    if (QMI_NO_ERR != rc)
    {
        LE_ERROR("qmi_sap_register returns %d", rc);
        LE_ERROR("likely due to missing QTI9x07-529");
        return NULL;
    }

    return ServiceCookie.service_handle;
}

//--------------------------------------------------------------------------------------------------
/**
 * Send QMI indication
 */
//--------------------------------------------------------------------------------------------------
static void Indicate
(
    swi_legato_avms_event_ind_msg_v01* ind
)
{
    // Indication requires registration
    if (NULL != QmiClientPtr)
    {
        qmi_csi_error resp_err;
        resp_err = qmi_csi_send_ind(QmiClientPtr->clnt,
                                    QMI_SWI_LEGATO_AVMS_EVENT_IND_V01, ind,
                                    sizeof(swi_legato_avms_event_ind_msg_v01));
        if (QMI_CSI_NO_ERR != resp_err)
        {
            LE_INFO("qmi_csi_send_ind returned error: %d\n",resp_err);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Send QMI indication for state change
 */
//--------------------------------------------------------------------------------------------------
static void IndicateState
(
    uint8_t state
)
{
    memset(IndPtr, 0, sizeof(swi_legato_avms_event_ind_msg_v01));
    IndPtr->avms_notifications_valid = 1;
    IndPtr->avms_notifications.notification = state;
    Indicate(IndPtr);
}

//--------------------------------------------------------------------------------------------------
/**
 * Convert Update Type to QMI Spec defined enums
 */
//--------------------------------------------------------------------------------------------------
static void ConvertUpdateType
(
    uint8_t* binaryTypePtr
)
{
    le_avc_UpdateType_t type;
    if (LE_OK == le_avc_GetUpdateType(&type))
    {
        //Conversion to align with QMI spec, FH #4111626
        switch (type)
        {
            case LE_AVC_FRAMEWORK_UPDATE:
                *binaryTypePtr = LEGATO_BINARY_TYPE;
                break;
            case LE_AVC_APPLICATION_UPDATE:
                *binaryTypePtr = APP_BINARY_TYPE;
                break;
            default:
                *binaryTypePtr = type;
                break;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Send QMI indication for fota change
 */
//--------------------------------------------------------------------------------------------------
static void IndicateFota
(
        uint8_t status,
        uint8_t input
)
{
    memset(IndPtr, 0, sizeof(swi_legato_avms_event_ind_msg_v01));
    IndPtr->avms_session_info_valid = 1;
    IndPtr->avms_session_info.status = status;
    IndPtr->avms_session_info.user_input_req = input;
    ConvertUpdateType(&IndPtr->avms_session_info.binary_type);
    Indicate(IndPtr);
}

//--------------------------------------------------------------------------------------------------
/**
 * Send QMI indication for fota progress
 */
//--------------------------------------------------------------------------------------------------
static void IndicateFotaWithSize
(
    uint8_t  status,
    uint8_t  input,
    int      totalNumBytes,
    int      downloadProgress
)
{
    memset(IndPtr, 0, sizeof(swi_legato_avms_event_ind_msg_v01));
    IndPtr->avms_session_info_valid = 1;
    IndPtr->avms_session_info.status = status;
    IndPtr->avms_session_info.user_input_req = input;
    IndPtr->avms_session_info.fw_dload_size = totalNumBytes;
    ConvertUpdateType(&IndPtr->avms_session_info.binary_type);
    // Rough conversion
    IndPtr->avms_session_info.fw_dload_complete = totalNumBytes * downloadProgress/ 100;
    Indicate(IndPtr);
}

//--------------------------------------------------------------------------------------------------
/**
 * QMI service thread
 * @return NULL when thread exits
 */
//--------------------------------------------------------------------------------------------------
static void* QmiServiceThread
(
    void* context
)
{
    int result;
    qmi_csi_os_params os_params,os_params_in;
    fd_set fds;
    void *sp;

    QmiClientPtr = NULL;

    sp = QmiRegisterService(&os_params);

    if (!sp)
    {
        LE_ERROR("qmi service reg returns %p", sp);
        return NULL;
    }

    // FIXME dead loop
    while(1)
    {
        fds = os_params.fds;
        result = select(os_params.max_fd+1, &fds, NULL, NULL, NULL);
        if (result < 0)
        {
            if (EINTR != errno)
            {
                LE_ERROR("socket select errno: %d", errno);
            }
        }
        else
        {
            os_params_in.fds = fds;
            // Read from file descriptor and process event
            qmi_csi_handle_event(sp, &os_params_in);
        }
    }
    qmi_csi_unregister(sp);
    return NULL;
}

//--------------------------------------------------------------------------------------------------
/**
 * Avc status handler
 */
//--------------------------------------------------------------------------------------------------
static void AvcStatus
(
    le_avc_Status_t  updateStatus,
    int32_t          totalNumBytes,
    int32_t          downloadProgress,
    void*            contextPtr
)
{
    le_result_t result;
    bool isUserAgreementEnabled;
    LastStatus = updateStatus;
    LE_DEBUG("updateStatus: %d", updateStatus);
    switch (updateStatus)
    {
        case LE_AVC_DOWNLOAD_PENDING:
            result = le_avc_GetUserAgreement(LE_AVC_USER_AGREEMENT_DOWNLOAD,
                                             &isUserAgreementEnabled);
            if ((LE_OK == result) && (true == isUserAgreementEnabled))
            {
                LE_DEBUG("Request user agreement for download");
                IndicateFota(DOWNLOADING_FW, EXPECTING_USER_INPUT);
            }
            else
            {
                // FIXME shall we auto download
                IndicateFota(DOWNLOADING_FW, NOT_EXPECTING_USER_INPUT);
            }

            break;
        case LE_AVC_DOWNLOAD_IN_PROGRESS:
            IndicateFotaWithSize(DOWNLOADING_FW, NOT_EXPECTING_USER_INPUT,\
                                 totalNumBytes, downloadProgress);
            break;
        case LE_AVC_DOWNLOAD_COMPLETE:
            IndicateFota(FW_DOWNLOADED, NOT_EXPECTING_USER_INPUT);
            break;
        case LE_AVC_DOWNLOAD_FAILED:
            LE_ERROR("LE_AVC_DOWNLOAD_FAILED");
            break;
        case LE_AVC_INSTALL_PENDING:
            IndicateFota(QUERY_FW_UPDATE, EXPECTING_USER_INPUT);
            break;
        case LE_AVC_UNINSTALL_COMPLETE:
        case LE_AVC_INSTALL_COMPLETE:
            IndicateFota(FW_UPDATED, NOT_EXPECTING_USER_INPUT);
            break;
        case LE_AVC_UNINSTALL_PENDING:
            break;
        case LE_AVC_INSTALL_FAILED:
        case LE_AVC_UNINSTALL_FAILED:
            break;
        case LE_AVC_SESSION_STARTED:
            IndicateState(START_SESSION);

            //2nd indication just to follow WPx5xx, consider combining
            memset(IndPtr, 0, sizeof(swi_legato_avms_event_ind_msg_v01));
            IndPtr->session_type_valid = 1;
            IndPtr->session_type = le_avc_GetSessionType();
            Indicate(IndPtr);
            break;
        case LE_AVC_SESSION_STOPPED:
            IndicateState(STOP_SESSION);
            break;
        case LE_AVC_AUTH_STARTED:
            IndicateState(AUTH_SESSION);
            break;
        case LE_AVC_AUTH_FAILED:
            IndicateState(updateStatus);
            break;
        case LE_AVC_CONNECTION_PENDING:
            break;
        case LE_AVC_REBOOT_PENDING:
            break;
        case LE_AVC_INSTALL_IN_PROGRESS:
            IndicateFota(FW_UPDATING, NOT_EXPECTING_USER_INPUT);
            break;
        case LE_AVC_CERTIFICATION_OK:
            break;
        case LE_AVC_CERTIFICATION_KO:
            LE_ERROR("LE_AVC_CERTIFICATION_KO");
            break;
        case LE_AVC_NO_UPDATE:
            IndicateFota(NO_FW_AVAIL, NOT_EXPECTING_USER_INPUT);
            break;
        case LE_AVC_UNINSTALL_IN_PROGRESS:
        default:
            break;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * AirVantage Session Start
 */
//--------------------------------------------------------------------------------------------------
static void AvcSessionStart
(
    qmi_req_handle  reqHandle,
    int             msgId
)
{
    le_result_t result = 0;
    qmi_csi_error resp_err;
    swi_legato_avms_session_start_resp_msg_v01 resp;
    memset(&resp, 0, sizeof(resp));
    resp.start_session_rsp = NOT_AVAILABLE;

    if (0 == SessionOnGoing)
    {
        result = le_avc_StartSession();
        if (LE_OK != result)
        {
            LE_ERROR("le_avc_StartSession failed %d", result);
            resp.resp.result = QMI_RESULT_FAILURE;
            if (LE_DUPLICATE == result)
            {
                resp.resp.error = QMI_SERVICE_ERR_NO_EFFECT;
            }
            else
            {
                resp.resp.error = QMI_SERVICE_ERR_INTERNAL;
            }
        }
        else
        {
            SessionOnGoing = 1;
        }
    }
    else
    {
        resp.resp.result = QMI_RESULT_FAILURE;
        resp.resp.error = QMI_SERVICE_ERR_NO_EFFECT;
    }

    resp_err = qmi_csi_send_resp( reqHandle, msgId, &resp, sizeof(resp) );
    if (QMI_CSI_NO_ERR != resp_err)
    {
        LE_ERROR("qmi_csi_send_resp returned error: %d\n",resp_err);
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * AirVantage Session Stop
 */
//--------------------------------------------------------------------------------------------------
static void AvcSessionStop
(
    qmi_req_handle  reqHandle,
    int             msgId
)
{
    le_result_t result = 0;
    qmi_csi_error resp_err;
    swi_legato_avms_session_stop_resp_msg_v01 resp;
    memset( &resp, 0, sizeof(resp) );

    if (SessionOnGoing == 1)
    {
        SessionOnGoing = 0;

        resp.session_type_valid = 1;
        resp.session_type = le_avc_GetSessionType();

        result = le_avc_StopSession();
        if (LE_OK != result)
        {
            LE_ERROR("le_avc_StopSession failed %d", result);
            resp.resp.result = QMI_RESULT_FAILURE;
            resp.resp.error = QMI_SERVICE_ERR_INTERNAL;
        }
    }
    else
    {
        resp.resp.result = QMI_RESULT_FAILURE;
        resp.resp.error = QMI_SERVICE_ERR_NO_EFFECT;
    }

    resp_err = qmi_csi_send_resp( reqHandle, msgId, &resp, sizeof(resp) );
    if (QMI_CSI_NO_ERR != resp_err)
    {
        LE_ERROR("qmi_csi_send_resp returned error: %d\n",resp_err);
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * AirVantage User Response
 */
//--------------------------------------------------------------------------------------------------
static void AvcUserReply
(
    Qmi2AvcSelect_t* ctxPtr
)
{
    uint32_t defer = DEFAULT_DEFER_IN_MIN; // TODO get from store
    le_result_t result = 0;

    if (ctxPtr->withDeferTime)
    {
        defer = ctxPtr->deferTime;
    }
    LE_INFO("Last Status: %d, user input: %d", LastStatus, ctxPtr->userInput);

    switch(LastStatus)
    {
        case LE_AVC_DOWNLOAD_PENDING:
            if (ACCEPT == ctxPtr->userInput)
                result = le_avc_AcceptDownload();
            else if (DEFER == ctxPtr->userInput)
                result = le_avc_DeferDownload(defer);
            break;

        case LE_AVC_INSTALL_PENDING:
            if (ACCEPT == ctxPtr->userInput)
                result = le_avc_AcceptInstall();
            else if (DEFER == ctxPtr->userInput)
                result = le_avc_DeferInstall(defer);
            break;

        case LE_AVC_UNINSTALL_PENDING:
            if (ACCEPT == ctxPtr->userInput)
                result = le_avc_AcceptUninstall();
            else if (DEFER == ctxPtr->userInput)
                result = le_avc_DeferUninstall(defer);
            break;

        case LE_AVC_REBOOT_PENDING:
            if (ACCEPT == ctxPtr->userInput)
                result = le_avc_AcceptReboot();
            else if (DEFER == ctxPtr->userInput)
                result = le_avc_DeferReboot(defer);
            break;

        case LE_AVC_CONNECTION_PENDING:
            if (ACCEPT == ctxPtr->userInput)
                result = le_avc_StartSession();
            else if (DEFER == ctxPtr->userInput)
                result = le_avc_DeferConnect(defer);
            break;

        default:
            LE_ERROR("no corresponding action for select");
    }

    LE_INFO("le_avc_ API returns %d", result);
}

//--------------------------------------------------------------------------------------------------
/**
 * AirVantage Retrieve Settings
 */
//--------------------------------------------------------------------------------------------------
static void AvcGetSettings
(
    qmi_req_handle           reqHandle,
    int                      msgId
)
{

    bool bPromptUser = false;
    bool failed = false;
    le_result_t result = 0;
    qmi_csi_error resp_err;
    swi_legato_avms_get_setting_resp_msg_v01 resp;
    size_t numTimers = sizeof(resp.retry_timer)/sizeof(uint16_t);

    memset( &resp, 0, sizeof(resp) );

    if ((result = le_avc_GetUserAgreement(LE_AVC_USER_AGREEMENT_CONNECTION, &bPromptUser)))
    {
        LE_ERROR("le_avc_GetUserAgreement LE_AVC_USER_AGREEMENT_CONNECTION failed %d", result);
        failed = true;
    }
    else
    {
        resp.autoconnect = bPromptUser;
    }

    if ((result = le_avc_GetUserAgreement(LE_AVC_USER_AGREEMENT_DOWNLOAD, &bPromptUser)))
    {
        LE_ERROR("le_avc_GetUserAgreement LE_AVC_USER_AGREEMENT_DOWNLOAD failed %d", result);
        failed = true;
    }
    else
    {
        resp.fw_autodload = bPromptUser;
    }

    //FIXME: autoupdate same as install and uninstall agreement?
    if ((result = le_avc_GetUserAgreement(LE_AVC_USER_AGREEMENT_INSTALL, &bPromptUser)))
    {
        LE_ERROR("le_avc_GetUserAgreement LE_AVC_USER_AGREEMENT_INSTALL failed %d", result);
        failed = true;
    }
    else
    {
        resp.fw_autoupdate = bPromptUser;
    }

    if (LE_OK == le_avc_GetPollingTimer(&resp.polling_timer))
    {
        LE_INFO("le_avc_GetPollingTimer returns %d", result);
        resp.polling_timer_valid = 1;
    }

    if (LE_OK == le_avc_GetRetryTimers(resp.retry_timer, &numTimers))
    {
        LE_INFO("le_avc_GetRetryTimers returns %d", result);
        resp.retry_timer_valid = 1;
    }

    if (failed)
    {
        resp.resp.result = QMI_RESULT_FAILURE;
        resp.resp.error = QMI_SERVICE_ERR_INTERNAL;
    }

    resp_err = qmi_csi_send_resp( reqHandle, msgId, &resp, sizeof(resp) );
    if (QMI_CSI_NO_ERR != resp_err)
    {
        LE_ERROR("qmi_csi_send_resp returned error: %d\n",resp_err);
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * AirVantage Apply Settings
 */
//--------------------------------------------------------------------------------------------------
static void AvcSetSettings
(
    qmi_req_handle           reqHandle,
    int                      msgId,
    swi_legato_avms_set_setting_req_msg_v01 settingPara
)
{
    qmi_csi_error                               resp_err;
    swi_legato_avms_set_setting_resp_msg_v01    resp;
    le_result_t result = 0;
    bool failed = false;

    memset( &resp, 0, sizeof(resp) );

    if ((result = le_avc_SetUserAgreement( LE_AVC_USER_AGREEMENT_CONNECTION,
                                           settingPara.autoconnect)))
    {
        LE_ERROR("le_avc_SetUserAgreement LE_AVC_USER_AGREEMENT_CONNECTION failed %d", result);
        failed = true;
    }
    if ((result = le_avc_SetUserAgreement( LE_AVC_USER_AGREEMENT_DOWNLOAD,
                                           settingPara.fw_autodload)))
    {
        LE_ERROR("le_avc_SetUserAgreement LE_AVC_USER_AGREEMENT_DOWNLOAD failed %d", result);
        failed = true;
    }
    if ((result = le_avc_SetUserAgreement( LE_AVC_USER_AGREEMENT_INSTALL,
                                           settingPara.fw_autoupdate)))
    {
        LE_ERROR("le_avc_SetUserAgreement LE_AVC_USER_AGREEMENT_INSTALL failed %d", result);
        failed = true;
    }

    if (settingPara.polling_timer_valid)
    {
        LE_INFO("setting polling timer");
        if ((result = le_avc_SetPollingTimer(settingPara.polling_timer)))
        {
            LE_ERROR("le_avc_SetPollingTimer failed %d", result);
            failed = true;
        }
    }

    if (settingPara.retry_timer_valid)
    {
        LE_INFO("setting retry timers");
        if ((result = le_avc_SetRetryTimers(settingPara.retry_timer,
                                            sizeof(settingPara.retry_timer)/sizeof(uint16_t))))
        {
            LE_ERROR("le_avc_SetRetryTimers failed %d", result);
            failed = true;
        }
    }

    //FIXME: where is reboot? LE_AVC_USER_AGREEMENT_REBOOT

    if (failed)
    {
        resp.resp.result = QMI_RESULT_FAILURE;
        resp.resp.error = QMI_SERVICE_ERR_INTERNAL;
    }

    resp_err = qmi_csi_send_resp( reqHandle, msgId, &resp, sizeof(resp) );
    if (QMI_CSI_NO_ERR != resp_err)
    {
        LE_ERROR("qmi_csi_send_resp returned error: %d\n",resp_err);
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Process QMI to AVC APIs
 */
//--------------------------------------------------------------------------------------------------
static void ProcessQmi
(
    void* contextPtr
)
{
    Qmi2Avc_t* data = (Qmi2Avc_t*)contextPtr;
    switch (data->action)
    {
        case START:
            AvcSessionStart(data->reqHandle, data->msgId);
            break;
        case STOP:
            AvcSessionStop(data->reqHandle, data->msgId);
            break;
        case SELECT:
            AvcUserReply(&data->selectPara);
            break;
        case GET_SETTINGS:
            AvcGetSettings(data->reqHandle, data->msgId);
            break;
        case SET_SETTINGS:
            AvcSetSettings(data->reqHandle, data->msgId, data->settingPara);
            break;
        default:
            LE_ERROR("Unhandled action %d", data->action);
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Connection State Handler
 */
//--------------------------------------------------------------------------------------------------
static void ConnectionStateHandler
(
    const char* intfNamePtr,    ///< [IN] Interface name
    bool        connected,      ///< [IN] connection state (true = connected, else false)
    void*       contextPtr      ///< [IN] User data
)
{
    LE_DEBUG("connected %d", connected);

    memset( IndPtr, 0, sizeof(swi_legato_avms_event_ind_msg_v01) );
    IndPtr->avms_data_session_valid = 1;
    IndPtr->avms_data_session.notif_type = connected;
    Indicate(IndPtr);
}
//--------------------------------------------------------------------------------------------------
/**
 * Initialize qmiAirVantage app
 */
//--------------------------------------------------------------------------------------------------
COMPONENT_INIT
{
    ClntPool = le_mem_CreatePool("ClntPool",sizeof(void*));
    le_avc_AddStatusEventHandler(AvcStatus, NULL);

    // FIXME le_avc API must be called on component thread, below event enables this
    Qmi2AvcEvent = le_event_CreateId("Qmi Event", sizeof(Qmi2Avc_t));
    le_event_AddHandler("Qmi Event Handler", Qmi2AvcEvent, ProcessQmi);

    QmiThreadRef = le_thread_Create("QmiThread", QmiServiceThread, NULL);
    le_thread_Start(QmiThreadRef);

    DataHandler = le_data_AddConnectionStateHandler(ConnectionStateHandler, NULL);

    SessionOnGoing = 0;
}
