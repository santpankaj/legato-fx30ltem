/**
 * @file pa_ecall_qmi.c
 *
 * QMI implementation of @ref c_pa_ecall API.
 *
 * Copyright (C) Sierra Wireless Inc.
 */


#include "legato.h"
#include "pa_ecall.h"
#include "swiQmi.h"
#include "mcc_qmi.h"

//--------------------------------------------------------------------------------------------------
// Symbol and Enum definitions.
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Macro: Convert the PA_ECALL_START_* enumeration to QMI eCall Start type
 */
//--------------------------------------------------------------------------------------------------
#define CONVERT_PA_ECALL_START_TO_QMI_SWI_M2M_ECALL_START_TYPE(_type_) \
                ((_type_ == PA_ECALL_START_MANUAL)   ? 2: \
                (_type_ == PA_ECALL_START_AUTO)     ? 3: \
                (_type_ == PA_ECALL_START_TEST)     ? 0: \
                0)

//--------------------------------------------------------------------------------------------------
// Static declarations.
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Store the legato thread, since we need to queue a function to this thread from the QMI thread.
 */
//--------------------------------------------------------------------------------------------------
static le_thread_Ref_t LegatoThread;
// Pool used for the LegatoThread
static le_mem_PoolRef_t ECallSessionInfoTypePool;

//--------------------------------------------------------------------------------------------------
/**
 * The MGS (M2M General Service) client.  Must be obtained by calling
 * swiQmi_GetClientHandle() before it can be used.
 */
//--------------------------------------------------------------------------------------------------
static qmi_client_type MgsClient;

//--------------------------------------------------------------------------------------------------
/**
 * Call event ID used to report ecall events to the registered event handlers.
 */
//--------------------------------------------------------------------------------------------------
static le_event_Id_t ECallEvent;

//--------------------------------------------------------------------------------------------------
/**
 * eCall configuration
 */
//--------------------------------------------------------------------------------------------------
static swi_m2m_ecall_config_req_msg_v01 ECallConfiguration = { {0} };

//--------------------------------------------------------------------------------------------------
/**
 * eCall msd
 */
//--------------------------------------------------------------------------------------------------
static swi_m2m_ecall_send_msd_blob_req_msg_v01 ECallMsd = {0};

//--------------------------------------------------------------------------------------------------
/**
 * Ecall system standard.
 */
//--------------------------------------------------------------------------------------------------
static pa_ecall_SysStd_t SystemStandard;

//--------------------------------------------------------------------------------------------------
// APIs.
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Check the QMI OEM response codes for any errors
 *
 * This provides common handling for checking the response codes of qmi_client_send_msg_sync() and
 * the associated QMI response message.

 * @return
 *      - LE_OK on success
 *      - LE_FAULT on error
 *      - LE_UNSUPPORTED Not supported on this platform
 */
//--------------------------------------------------------------------------------------------------
static le_result_t OEMCheckResponseCode
(
    const char* msgIdStr,           ///< [IN] The string for the QMI message
    qmi_client_error_type rc,       ///< [IN] Response from the send function
#if (QAPI_SWI_COMMON_V01_IDL_MINOR_VERS <= 1)
    oem_response_type_v01 resp      ///< [IN] Response value in the QMI response message
#else
    qmi_response_type_v01 resp      ///< [IN] Response value in the QMI response message
#endif
)
{
    // todo: Should we also check if resp.error == QMI_ERR_NONE_V01. I don't think its necessary.
    if ( (rc==QMI_NO_ERR) && (resp.result==QMI_RESULT_SUCCESS_V01) )
    {
        LE_DEBUG("%s sent to Modem", msgIdStr);
        return LE_OK;
    }
    else
    {
        LE_ERROR("Sending %s failed: rc=%i, resp.result=%i.[0x%02x], resp.error=%i.[0x%02x]",
                    msgIdStr, rc, resp.result, resp.result, resp.error, resp.error);
        LE_ERROR_IF(((rc == QMI_IDL_LIB_MESSAGE_ID_NOT_FOUND)         ||
                     (rc == QMI_IDL_LIB_UNRECOGNIZED_SERVICE_VERSION) ||
                     (rc == QMI_IDL_LIB_INCOMPATIBLE_SERVICE_VERSION)),
                    "Modem is running an incompatible version of firmware!");
#if defined(QMI_SERVICE_ERR_NOT_SUPPORTED)
        if (resp.error == QMI_SERVICE_ERR_NOT_SUPPORTED)
        {
            LE_ERROR("Platform does not support this feature!");
            return LE_UNSUPPORTED;
        }
#endif
        return LE_FAULT;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to get the eCall configuration variable.
 *
 * @return
 *      LE_OK if successful.
 *      LE_FAULT if unsuccessful.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t GetEcallConfiguration
(
    swi_m2m_ecall_config_req_msg_v01 *configurationPtr ///< [OUT] qmi configuration
)
{
    qmi_client_error_type rc;

    swi_m2m_ecall_read_config_resp_msg_v01 ecallConfig = { {0} };

    rc = qmi_client_send_msg_sync(MgsClient,
                                  QMI_SWI_M2M_ECALL_READ_CONFIG_REQ_V01,
                                  NULL, 0,
                                  &ecallConfig, sizeof(ecallConfig),
                                  COMM_DEFAULT_PLATFORM_TIMEOUT);

    if ( OEMCheckResponseCode(STRINGIZE_EXPAND(QMI_SWI_M2M_ECALL_READ_CONFIG_REQ_V01),
                                               rc, ecallConfig.resp) != LE_OK )
    {
        return LE_FAULT;
    }

    memcpy(&(configurationPtr->configure_eCall_session),
           &(ecallConfig.configure_eCall_session),
           sizeof(configurationPtr->configure_eCall_session));

    configurationPtr->modem_msd_type = ecallConfig.modem_msd_type;

    configurationPtr->num_valid = ecallConfig.num_valid;
    configurationPtr->num_len = ecallConfig.num_len;
    memcpy(configurationPtr->num,ecallConfig.num,sizeof(configurationPtr->num));

    configurationPtr->max_redial_attempt_valid = ecallConfig.max_redial_attempt_valid;
    configurationPtr->max_redial_attempt = ecallConfig.max_redial_attempt;

    configurationPtr->gnss_update_time_valid = ecallConfig.gnss_update_time_valid;
    configurationPtr->gnss_update_time = ecallConfig.gnss_update_time;

    configurationPtr->nad_deregistration_time_valid = ecallConfig.nad_deregistration_time_valid;
    configurationPtr->nad_deregistration_time = ecallConfig.nad_deregistration_time;

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to set the eCall configuration variable.
 *
 * @return
 *      LE_OK if successful.
 *      LE_FAULT if unsuccessful.
 *      LE_UNSUPPORTED Not supported on this platform.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t SetEcallConfiguration
(
    swi_m2m_ecall_config_req_msg_v01* configurationPtr ///< [IN] qmi configuration
)
{
    qmi_client_error_type rc;

    swi_m2m_ecall_config_resp_msg_v01 ecallConfigResp = { {0} };

    rc = qmi_client_send_msg_sync(MgsClient,
                                  QMI_SWI_M2M_ECALL_CONFIG_REQ_V01,
                                  configurationPtr, sizeof(*configurationPtr),
                                  &ecallConfigResp, sizeof(ecallConfigResp),
                                  COMM_DEFAULT_PLATFORM_TIMEOUT);

    return OEMCheckResponseCode(STRINGIZE_EXPAND(QMI_SWI_M2M_ECALL_CONFIG_REQ_V01),
                             rc, ecallConfigResp.resp);
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to get the EraGlonass setting.
 *
 * @return
 *      LE_OK if successful.
 *      LE_FAULT if unsuccessful.
 */
//--------------------------------------------------------------------------------------------------
#ifdef QMI_SWI_M2M_GET_EGLONASS_CFG_REQ_V01
static le_result_t GetEraGlonassConfiguration
(
   swi_m2m_get_eglonass_cfg_resp_msg_v01* configurationPtr ///< [OUT] qmi configuration
)
{
   qmi_client_error_type rc;

   SWIQMI_DECLARE_MESSAGE(swi_m2m_get_eglonass_cfg_resp_msg_v01, eglonassConfig);

   rc = qmi_client_send_msg_sync(MgsClient,
                                 QMI_SWI_M2M_GET_EGLONASS_CFG_REQ_V01,
                                 NULL, 0,
                                 &eglonassConfig, sizeof(eglonassConfig),
                                 COMM_DEFAULT_PLATFORM_TIMEOUT);

   if (LE_OK != OEMCheckResponseCode(STRINGIZE_EXPAND(QMI_SWI_M2M_GET_EGLONASS_CFG_RESP_V01),
                                              rc, eglonassConfig.resp))
   {
       return LE_FAULT;
   }

   memcpy(configurationPtr, &eglonassConfig, sizeof(swi_m2m_get_eglonass_cfg_resp_msg_v01));

   return LE_OK;
}
#endif

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to set the EraGlonass configuration
 *
 * @return
 *      LE_OK if successful.
 *      LE_FAULT if unsuccessful.
 *      LE_UNSUPPORTED Not supported on this platform.
 */
//--------------------------------------------------------------------------------------------------
#ifdef QMI_SWI_M2M_SET_EGLONASS_CFG_REQ_V01
static le_result_t SetEraGlonassConfiguration
(
   swi_m2m_set_eglonass_cfg_req_msg_v01 *configurationPtr ///< [IN] qmi configuration
)
{
   qmi_client_error_type rc;

   SWIQMI_DECLARE_MESSAGE(swi_m2m_set_eglonass_cfg_resp_msg_v01, eglonassConfigResp);

   rc = qmi_client_send_msg_sync(MgsClient,
                                 QMI_SWI_M2M_SET_EGLONASS_CFG_REQ_V01,
                                 configurationPtr, sizeof(*configurationPtr),
                                 &eglonassConfigResp, sizeof(eglonassConfigResp),
                                 COMM_DEFAULT_PLATFORM_TIMEOUT);

   return OEMCheckResponseCode(STRINGIZE_EXPAND(QMI_SWI_M2M_SET_EGLONASS_CFG_RESP_V01),
                            rc, eglonassConfigResp.resp);
}
#endif

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to start the eCall.
 *
 * @return
 *      LE_OK if successful.
 *      LE_FAULT if unsuccessful.
 *      LE_UNSUPPORTED Not supported on this platform.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_ecall_Start
(
    pa_ecall_StartType_t callType
)
{
    qmi_client_error_type rc;
    le_result_t result;

    swi_m2m_ecall_start_req_msg_v01 ecallStartReq = {0};
    swi_m2m_ecall_start_resp_msg_v01 ecallStartResp = { {0} };

    // eCall session type:
    //   0: test call
    //   1: reconfiguration call
    //   2: manually initiated voice call
    //   3: automatically initiated voice call
    ecallStartReq.call_type = CONVERT_PA_ECALL_START_TO_QMI_SWI_M2M_ECALL_START_TYPE(callType);

    rc = qmi_client_send_msg_sync(MgsClient,
                                  QMI_SWI_M2M_ECALL_START_REQ_V01,
                                  &ecallStartReq, sizeof(ecallStartReq),
                                  &ecallStartResp, sizeof(ecallStartResp),
                                  COMM_DEFAULT_PLATFORM_TIMEOUT);

    result = OEMCheckResponseCode(STRINGIZE_EXPAND(QMI_SWI_M2M_ECALL_START_REQ_V01),
                                  rc, ecallStartResp.resp);
    if (LE_OK == result)
    {
        return LE_OK;
    }

    LE_ERROR("Could not Start the ecall session type %d", callType);
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Parse the eCall event to report it or not.
 *
 */
//--------------------------------------------------------------------------------------------------
static void ProcessECallState
(
    void* param1Ptr,
    void* param2Ptr
)
{
    swi_m2m_ecall_status_ind_msg_v01* eCallIndPtr = (swi_m2m_ecall_status_ind_msg_v01*)param1Ptr;
    bool             reportEvent = true;
    le_ecall_State_t newECallState = LE_ECALL_STATE_UNKNOWN;

    if (eCallIndPtr==NULL)
    {
        LE_CRIT("param1Ptr is NULL");
        return;
    }

    LE_DEBUG("eCall Event: %d", eCallIndPtr->indication);

    switch (eCallIndPtr->indication)
    {
        case 0: /* eCall session started */
        {
            newECallState = LE_ECALL_STATE_STARTED;
            break;
        }
        case 5: /* MO call Disconnected */
        case 7: /* MT call Disconnected */
        {
            newECallState = LE_ECALL_STATE_DISCONNECTED;
            break;
        }
        case 4: /* MO call connected */
        case 6: /* MT call connected */
        {
            newECallState = LE_ECALL_STATE_CONNECTED;
            break;
        }
        case 8: /* Waiting for PSAP START indication */
        {
             newECallState = LE_ECALL_STATE_WAITING_PSAP_START_IND;
             break;
        }
        case 9: /* PSAP START received but no MSD available */
        {
             newECallState = LE_ECALL_STATE_MSD_TX_FAILED;
             break;
        }
        case 10: /* PSAP START received and MSD available */
        {
            newECallState = LE_ECALL_STATE_MSD_TX_STARTED;
            break;
        }
        case 14: /* LL nack received */
        {
             newECallState = LE_ECALL_STATE_LLNACK_RECEIVED;
             break;
        }
        case 12: /* LL ack received */
        case 13: /* 2LL acks received */
        {
             newECallState = LE_ECALL_STATE_LLACK_RECEIVED;
             break;
        }
        case 15: /* HL ack received */
        case 17: /* 2AL acks received */
        {
             newECallState = LE_ECALL_STATE_ALACK_RECEIVED_POSITIVE;
             break;
        }
        case 19: /* eCall clear-down received */
        {
             newECallState = LE_ECALL_STATE_ALACK_RECEIVED_CLEAR_DOWN;
             mcc_qmi_EcallStop();
             break;
        }
        case 16: /* IVS Transmission completed */
        {
            newECallState = LE_ECALL_STATE_MSD_TX_COMPLETED;
            break;
        }
        case 18: /* eCall session completed */
        {
            newECallState = LE_ECALL_STATE_COMPLETED;
            break;
        }
        case 20: /* eCall session reset */
        {
            newECallState = LE_ECALL_STATE_RESET;
            break;
        }
        case 21: /* eCall session failure */
        {
            newECallState = LE_ECALL_STATE_FAILED;
            break;
        }
        case 23: /* eCall session stop */
        {
            newECallState = LE_ECALL_STATE_STOPPED;
            break;
        }
        case 22: /* MSD update request available */
        {
            newECallState = LE_ECALL_STATE_PSAP_START_IND_RECEIVED;
            break;
        }
        case 1: /* Get GPS Fix */
        case 2: /* GPS Fix Received */
        case 3: /* GPS Fix Timeout */
        case 11: /* PSAP START received and MSD sent */
        case 24: /* eCall operating mode is eCall and normal call mode */
        case 25: /* eCall operating mode is eCall only mode */
        case 26: /* eCall transmission mode is PUSH mode */
        case 27: /* eCall transmission mode is PULL mode */
        {
            // Do not report the event.
            LE_DEBUG("eCall %d not reported", eCallIndPtr->indication);
            reportEvent = false;
            break;
        }
        break;
        case 28: /* ecall timeouts */
        {
            if (eCallIndPtr->timer_id_valid)
            {
                switch (eCallIndPtr->timer_id)
                {
                    case 2:
                    newECallState = LE_ECALL_STATE_TIMEOUT_T2;
                    break;
                    case 3:
                    newECallState = LE_ECALL_STATE_TIMEOUT_T3;
                    break;
                    case 5:
                    newECallState = LE_ECALL_STATE_TIMEOUT_T5;
                    break;
                    case 6:
                    newECallState = LE_ECALL_STATE_TIMEOUT_T6;
                    break;
                    case 7:
                    newECallState = LE_ECALL_STATE_TIMEOUT_T7;
                    break;
                    case 9:
                    newECallState = LE_ECALL_STATE_TIMEOUT_T9;
                    break;
                    case 10:
                    newECallState = LE_ECALL_STATE_TIMEOUT_T10;
                    break;
                    default:
                        LE_ERROR("Unknown timer id !!");
                        reportEvent = false;
                    break;
                }
            }
            else
            {
                LE_ERROR("Timeout notification without timer id valid");
                reportEvent = false;
            }
        }
        break;
        default:
        {
            LE_ERROR("Unknown eCall indication %d", eCallIndPtr->indication);
            break;
        }
    }

    if (reportEvent)
    {
        LE_DEBUG("eCallState %d reported with %d event", eCallIndPtr->indication, newECallState);
        le_event_Report(ECallEvent, &newECallState, sizeof(newECallState));
    }

    le_mem_Release(eCallIndPtr);
}


//--------------------------------------------------------------------------------------------------
/**
 * Handler function called when there is an eCall Status indication.  This handler reports a
 * eCall event to the user.
 */
//--------------------------------------------------------------------------------------------------
static void ECallHandler
(
    void*           indBufPtr,   // [IN] The indication message data.
    unsigned int    indBufLen,   // [IN] The indication message data length in bytes.
    void*           contextPtr   // [IN] Context value that was passed to
                                 //      swiQmi_AddIndicationHandler().
)
{
    uint32_t* eCallIndPtr = (uint32_t*)0xdeadbeef;
    if (indBufPtr == NULL)
    {
        LE_ERROR("Indication message empty.");
        return;
    }

    swi_m2m_ecall_status_ind_msg_v01 eCallInd;

    qmi_client_error_type clientMsgErr = qmi_client_message_decode(
        MgsClient, QMI_IDL_INDICATION, QMI_SWI_M2M_ECALL_STATUS_IND_V01,
        indBufPtr, indBufLen,
        &eCallInd, sizeof(eCallInd));

    if (clientMsgErr != QMI_NO_ERR)
    {
        LE_ERROR("Error in decoding QMI_SWI_M2M_ECALL_STATUS_IND_V01, error = %i", clientMsgErr);
        return;
    }

    LE_DEBUG("eCall Event: %d", eCallInd.indication);

    eCallIndPtr = le_mem_ForceAlloc(ECallSessionInfoTypePool);
    memcpy(eCallIndPtr, &eCallInd, sizeof(swi_m2m_ecall_status_ind_msg_v01) );

    // Got valid session info, so queue the function to respond.
    le_event_QueueFunctionToThread(LegatoThread, ProcessECallState, eCallIndPtr, NULL);
}

//--------------------------------------------------------------------------------------------------
/**
 * Enable/Disable ERA-GLONASS configuration mode
 *
 * @return
 *      - LE_OK           On success.
 *      - LE_FAULT        For unexpected error.
 *      - LE_UNSUPPORTED  Not supported on this platform.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t EnableEraGlonassConfig
(
    bool  enable  ///< [IN] enable or disable Era-GLONASS mode
)
{
#ifdef QMI_SWI_M2M_SET_GOSTMODE_REQ_V01
    qmi_client_error_type rc;

    SWIQMI_DECLARE_MESSAGE(swi_m2m_set_gostmode_req_msg_v01, setEraModeReq);
    SWIQMI_DECLARE_MESSAGE(swi_m2m_set_gostmode_resp_msg_v01, setEraModeResp);
    setEraModeReq.gost_mode = enable;

    rc = qmi_client_send_msg_sync(MgsClient,
                                  QMI_SWI_M2M_SET_GOSTMODE_REQ_V01,
                                  &setEraModeReq, sizeof(setEraModeReq),
                                  &setEraModeResp, sizeof(setEraModeResp),
                                  COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (LE_OK != swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_SWI_M2M_SET_GOSTMODE_RESP_V01),
                                          rc, setEraModeResp.resp.result,
                                          setEraModeResp.resp.error))
    {
        LE_ERROR("Unable to set ERA-GLONASS configuration mode");
        return LE_FAULT;
    }

#else
    return LE_UNSUPPORTED;
#endif

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Update current system standard
 *
 * @return
 *  - LE_OK    on success
 *  - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_ecall_UpdateSystemStandard
(
    pa_ecall_SysStd_t sysStandard  ///< [IN] The system standard
)
{
    if (PA_ECALL_PAN_EUROPEAN == sysStandard)
    {
        EnableEraGlonassConfig(false);
    }
    else if (PA_ECALL_ERA_GLONASS == sysStandard)
    {
        EnableEraGlonassConfig(true);
    }
    else
    {
        LE_ERROR("Unknow system standard (%d)",sysStandard);
        return LE_FAULT;
    }

    SystemStandard = sysStandard;
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function initializes the platform adapter layer for eCall services.
 *
 * @return LE_OK if successful.
 * @return LE_FAULT if unsuccessful.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_ecall_Init
(
    pa_ecall_SysStd_t sysStd ///< [IN] Choosen system (PA_ECALL_PAN_EUROPEAN or PA_ECALL_ERA_GLONASS)
)
{
    LE_INFO("Start the eCall QMI PA initialization with %s standard.",
            ((sysStd == PA_ECALL_PAN_EUROPEAN) ? "PAN-EUROPEAN" : "ERA-GLONASS") );

    pa_ecall_UpdateSystemStandard(sysStd);

    // Create the event for signaling user handlers.
    ECallEvent = le_event_CreateId("ECallEvent", sizeof(le_ecall_State_t));

    ECallSessionInfoTypePool = le_mem_CreatePool("ECallSessionInfoTypePool",
                                                 sizeof(swi_m2m_ecall_status_ind_msg_v01));

    // Get the current legato thread, since we need it for later.
    LegatoThread = le_thread_GetCurrent();

    if (swiQmi_InitServices(QMI_SERVICE_MGS) != LE_OK)
    {
        LE_CRIT("QMI_SERVICE_MGS cannot be initialized.");
        return LE_FAULT;
    }

    // This must be called first.
    if (swiQmi_InitServices(QMI_SERVICE_MGS) != LE_OK)
    {
        LE_CRIT("Could not initialize the QMI Modem Services.");
        return LE_FAULT;
    }

    // Get the qmi client service handle for later use.
    if ( (MgsClient = swiQmi_GetClientHandle(QMI_SERVICE_MGS)) == NULL)
    {
        LE_ERROR("Could not get the QMI service");
        return LE_FAULT;
    }

    // Register our own handler with the QMI ecall service.
    swiQmi_AddIndicationHandler(ECallHandler, QMI_SERVICE_MGS, QMI_SWI_M2M_ECALL_STATUS_IND_V01,
                                NULL);

    // End the previous eCall session if any
    pa_ecall_End();

    LE_INFO("Initialized the eCall QMI PA.");
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to set the eCall operation mode.
 *
 * @return LE_FAULT          The function failed.
 * @return LE_OK             The function succeed.
 * @return LE_UNSUPPORTED    Not supported on this platform.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_ecall_SetOperationMode
(
    le_ecall_OpMode_t mode ///< [IN] Operation mode
)
{
    qmi_client_error_type rc;
    swi_m2m_ecall_set_op_mode_req_msg_v01 req = {0};
    swi_m2m_ecall_set_op_mode_resp_msg_v01 resp = {{0}};
    le_ecall_OpMode_t currentMode;

    //Check requested operation mode is same as current mode
    if (pa_ecall_GetOperationMode(&currentMode) == LE_OK)
    {
        if (mode == currentMode)
        {
            return LE_OK;
        }
    }
    if (mode == LE_ECALL_NORMAL_MODE)
    {
        req.op_mode = 0;
    }
    else if (mode == LE_ECALL_ONLY_MODE)
    {
        req.op_mode = 1;
    }
    else if (mode == LE_ECALL_FORCED_PERSISTENT_ONLY_MODE)
    {
        req.op_mode = 2;
    }

    rc = qmi_client_send_msg_sync(MgsClient,
                                  QMI_SWI_M2M_ECALL_SET_OP_MODE_REQ_V01,
                                  &req, sizeof(req),
                                  &resp, sizeof(resp),
                                  COMM_DEFAULT_PLATFORM_TIMEOUT);

    return OEMCheckResponseCode(STRINGIZE_EXPAND(QMI_SWI_M2M_ECALL_SET_OP_MODE_REQ_V01),
                                rc, resp.resp);
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to retrieve the configured eCall operation mode.
 *
 * @return LE_FAULT       The function failed.
 * @return LE_OK          The function succeed.
 * @return LE_UNSUPPORTED Not supported on this platform.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_ecall_GetOperationMode
(
    le_ecall_OpMode_t* mode ///< [OUT] Operation mode
)
{
    qmi_client_error_type rc;
    le_result_t result;

    SWIQMI_DECLARE_MESSAGE(swi_m2m_ecall_get_op_mode_resp_msg_v01, resp);

    rc = qmi_client_send_msg_sync(MgsClient,
                                  QMI_SWI_M2M_ECALL_GET_OP_MODE_REQ_V01,
                                  NULL, 0,
                                  &resp, sizeof(resp),
                                  COMM_DEFAULT_PLATFORM_TIMEOUT);

    result = OEMCheckResponseCode(STRINGIZE_EXPAND(QMI_SWI_M2M_ECALL_GET_OP_MODE_REQ_V01),
                                  rc, resp.resp);
    if (LE_OK == result)
    {
        if (resp.op_mode == 0)
        {
            *mode = LE_ECALL_NORMAL_MODE;
        }
        else if (resp.op_mode == 1)
        {
            *mode = LE_ECALL_ONLY_MODE;
        }
        else if (resp.op_mode == 2)
        {
            *mode = LE_ECALL_FORCED_PERSISTENT_ONLY_MODE;
        }
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to register a handler for eCall event notifications.
 *
 * @return A handler reference, which is only needed for later removal of the handler.
 *
 * @note Doesn't return on failure, so there's no need to check the return value for errors.
 */
//--------------------------------------------------------------------------------------------------
le_event_HandlerRef_t pa_ecall_AddEventHandler
(
    pa_ecall_EventHandlerFunc_t   handlerFuncPtr ///< [IN] The event handler function.
)
{
    LE_DEBUG("Set new eCall Event handler.");

    LE_FATAL_IF(handlerFuncPtr == NULL, "The new eCall Event handler is NULL.");

    return le_event_AddHandler("ECallEventHandler",
                               ECallEvent,
                               (le_event_HandlerFunc_t)handlerFuncPtr);

}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to unregister the handler for eCalls handling.
 *
 * @note Doesn't return on failure, so there's no need to check the return value for errors.
 */
//--------------------------------------------------------------------------------------------------
void pa_ecall_RemoveEventHandler
(
    le_event_HandlerRef_t handlerRef
)
{
    LE_DEBUG("Clear eCall Event handler %p",handlerRef);
    le_event_RemoveHandler(handlerRef);
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the Public Safely Answering Point telephone number.
 *
 * @note Important! This function doesn't modify the U/SIM content.
 *
 * @return LE_OK           The function succeed.
 * @return LE_FAULT        The function failed.
 * @return LE_UNSUPPORTED  Not supported on this platform.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_ecall_SetPsapNumber
(
    char psap[LE_MDMDEFS_PHONE_NUM_MAX_BYTES] ///< [IN] Public Safely Answering Point number
)
{
    if (psap == NULL)
    {
        LE_ERROR("Argument is NULL");
        return LE_FAULT;
    }

    size_t length = strlen(psap);

    if (SystemStandard == PA_ECALL_PAN_EUROPEAN)
    {
        if (length >= MAX_DIAL_DIGITS_V01)
        {
            LE_ERROR("QMI buffer for PSAP number is too small");
            return LE_FAULT;
        }

        if (GetEcallConfiguration(&ECallConfiguration) != LE_OK)
        {
            LE_ERROR("Could not retrieve eCall configuration in PAN-EUROPEAN standard");
            return LE_FAULT;
        }

        // By setting 1 to dial_type field, the eCall modem will dial the number specified in the
        // optional num field.
        ECallConfiguration.configure_eCall_session.dial_type = 1;

        memset(ECallConfiguration.num, 0, sizeof(ECallConfiguration.num));
        memcpy(ECallConfiguration.num, psap, length);

        ECallConfiguration.num[length] = '\0';
        ECallConfiguration.num_len = length+1;
        ECallConfiguration.num_valid = true;

        if (SetEcallConfiguration(&ECallConfiguration) != LE_OK)
        {
            LE_ERROR("Could not configure PSAP in PAN-EUROPEAN standard");
            return LE_FAULT;
        }

        LE_DEBUG("PSAP number %s has been configured in PAN-EUROPEAN standard", psap);
    }
    else if (SystemStandard == PA_ECALL_ERA_GLONASS)
    {
#ifdef QMI_SWI_M2M_SET_EGLONASS_CFG_REQ_V01
        if ( length > QMI_SWI_M2M_NUMBER_MAX_V01)
        {
            LE_ERROR("QMI buffer for PSAP number is too small");
            return LE_FAULT;
        }

        SWIQMI_DECLARE_MESSAGE(swi_m2m_set_eglonass_cfg_req_msg_v01, config);

        memset(config.Number, 0, sizeof(config.Number));
        memcpy(config.Number, psap, length);

        config.Number[length] = '\0';
        config.Number_valid = true;

        if (LE_OK != SetEraGlonassConfiguration(&config))
        {
            LE_ERROR("Could not configure PSAP in ERA-GLONASS standard");
            return LE_FAULT;
        }

        LE_DEBUG("PSAP number %s has been configured in ERA-GLONASS standard", psap);
#else
        return LE_UNSUPPORTED;
#endif
    }
    else
    {
        LE_ERROR("System standard is not valid");
        return LE_FAULT;
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the Public Safely Answering Point telephone number.
 *
 * @return LE_OK           The function succeed.
 * @return LE_FAULT        The function failed.
 * @return LE_OVERFLOW     Retrieved PSAP number is too long for the out parameter.
 * @return LE_UNSUPPORTED  Not supported on this platform.
 *
 * @note Important! This function doesn't read the U/SIM content.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_ecall_GetPsapNumber
(
    char*    psapPtr, ///< [OUT] Public Safely Answering Point number
    size_t   len      ///< [IN] The length of SMSC string.
)
{
    if (psapPtr == NULL)
    {
        LE_ERROR("psapPtr is NULL !");
        return LE_FAULT;
    }

    if (SystemStandard == PA_ECALL_PAN_EUROPEAN)
    {
        if (GetEcallConfiguration(&ECallConfiguration) != LE_OK)
        {
            LE_ERROR("Could not retrieve eCall configuration in PAN-EUROPEAN standard");
            return LE_FAULT;
        }

        if (ECallConfiguration.num_valid)
        {
            // Retrieved string is '"PSAP_NUMBER"\0'
            LE_DEBUG("num_len.%d  num.%s", ECallConfiguration.num_len, &ECallConfiguration.num[0]);

            if (ECallConfiguration.num_len > len)
            {
                LE_ERROR("Retrieved PSAP number is too long for the out parameter");
                return LE_OVERFLOW;
            }

            memcpy(psapPtr, &ECallConfiguration.num[0], ECallConfiguration.num_len);
            psapPtr[ECallConfiguration.num_len] = '\0';
        }
        else
        {
            LE_ERROR("Cannot retrieve PSAP number from eCall configuration");
            return LE_FAULT;
        }

        LE_DEBUG("Retrieved PSAP in PAN-EUROPEAN standard: %s", psapPtr);
    }
    else if (SystemStandard == PA_ECALL_ERA_GLONASS)
    {
#ifdef QMI_SWI_M2M_GET_EGLONASS_CFG_REQ_V01
        size_t length = 0;

        SWIQMI_DECLARE_MESSAGE(swi_m2m_get_eglonass_cfg_resp_msg_v01, config);

        if (LE_OK != GetEraGlonassConfiguration(&config))
        {
            LE_ERROR("Could not get PSAP in ERA-GLONASS standard");
            return LE_FAULT;
        }

        length = strlen(config.Number);
        // Retrieved string is '"PSAP_NUMBER"\0'
        LE_DEBUG("Number length.%d  num.%s", length, &config.Number[0]);

        if (length+1 > len)
        {
            LE_ERROR("Retrieved PSAP number is too long for the out parameter");
            return LE_OVERFLOW;
        }

        memcpy(psapPtr, &config.Number[0], length);
        psapPtr[length] = '\0';

        LE_DEBUG("Retrieved PSAP in ERA-GLONASS standard: %s", psapPtr);
#else
        return LE_UNSUPPORTED;
#endif
    }
    else
    {
        LE_ERROR("System standard is not valid");
        return LE_FAULT;
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function can be recalled to indicate the modem to read the number to dial from the FDN/SDN
 * of the U/SIM, depending upon the eCall operating mode.
 *
 * @return
 *  - LE_OK on success
 *  - LE_FAULT for other failures
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_ecall_UseUSimNumbers
(
    void
)
{
    if ( GetEcallConfiguration(&ECallConfiguration) != LE_OK )
    {
        LE_ERROR("Could not retrieve eCall Configuration");
        return LE_FAULT;
    }

    // 0: Read the number to dial from the FDN/SDN, depending upon the eCall operating mode.
    ECallConfiguration.configure_eCall_session.dial_type = 0;

    if ( SetEcallConfiguration(&ECallConfiguration) != LE_OK )
    {
        LE_ERROR("Could not configure the eCall");
        return LE_FAULT;
    }
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to set push/pull transmission mode.
 *
 * @return LE_FAULT        The function failed.
 * @return LE_OK           The function succeed.
 * @return LE_UNSUPPORTED  Not supported on this platform.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_ecall_SetMsdTxMode
(
    le_ecall_MsdTxMode_t mode
)
{
    qmi_client_error_type rc;

    swi_m2m_ecall_set_tx_mode_req_msg_v01 msdTxModeReq = {0};
    swi_m2m_ecall_set_tx_mode_resp_msg_v01 msdTxModeResp = { {0} };

    if ( mode == LE_ECALL_TX_MODE_PULL )
    {
        /* 0: Pull mode (modem/host waits for MSD request from PSAP to send MSD) */
        msdTxModeReq.tx_mode = 0;
    }
    else if ( mode == LE_ECALL_TX_MODE_PUSH )
    {
        /* 1: Push mode (modem/host sends MSD to PSAP right after eCall is connected).
         * This is the default mode. */
        msdTxModeReq.tx_mode = 1;
    }
    else
    {
        return LE_FAULT;
    }

    rc = qmi_client_send_msg_sync(MgsClient,
                                  QMI_SWI_M2M_ECALL_SET_TX_MODE_REQ_V01,
                                  &msdTxModeReq, sizeof(msdTxModeReq),
                                  &msdTxModeResp, sizeof(msdTxModeResp),
                                  COMM_DEFAULT_PLATFORM_TIMEOUT);

    return OEMCheckResponseCode(STRINGIZE_EXPAND(QMI_SWI_M2M_ECALL_SET_TX_MODE_REQ_V01),
                             rc, msdTxModeResp.resp);
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to get push/pull transmission mode.
 *
 * @return LE_FAULT        The function failed.
 * @return LE_OK           The function succeed.
 * @return LE_UNSUPPORTED  Not supported on this platform.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_ecall_GetMsdTxMode
(
    le_ecall_MsdTxMode_t* modePtr
)
{
    qmi_client_error_type rc;
    le_result_t result;

    SWIQMI_DECLARE_MESSAGE(swi_m2m_ecall_get_tx_mode_resp_msg_v01, msdTxModeResp);

    if (modePtr == NULL)
    {
        LE_ERROR("modePtr is NULL !");
        return LE_FAULT;
    }

    rc = qmi_client_send_msg_sync(MgsClient,
                                  QMI_SWI_M2M_ECALL_GET_TX_MODE_REQ_V01,
                                  NULL, 0,
                                  &msdTxModeResp, sizeof(msdTxModeResp),
                                  COMM_DEFAULT_PLATFORM_TIMEOUT);

    result = OEMCheckResponseCode(STRINGIZE_EXPAND(QMI_SWI_M2M_ECALL_GET_TX_MODE_REQ_V01),
                                  rc, msdTxModeResp.resp);
    if (LE_OK == result)
    {
        LE_DEBUG("MSD tx_mode.%d", msdTxModeResp.tx_mode);
        if (msdTxModeResp.tx_mode == 0)
        {
            *modePtr = LE_ECALL_TX_MODE_PULL;
            return LE_OK;
        }
        else if (msdTxModeResp.tx_mode == 1)
        {
            *modePtr = LE_ECALL_TX_MODE_PUSH;
            return LE_OK;
        }

        return LE_FAULT;
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to send the Minimum Set of Data for the eCall.
 *
 * @return LE_FAULT        The function failed.
 * @return LE_OK           The function succeed.
 * @return LE_UNSUPPORTED  Not supported on this platform.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_ecall_SendMsd
(
    uint8_t  *msdPtr,   ///< [IN] Encoded Msd
    size_t    msdSize   ///< [IN] msd buffer size
)
{
    qmi_client_error_type rc;
    swi_m2m_ecall_send_msd_blob_resp_msg_v01 sendMsdBlobResp = { {0} };

    if (msdPtr == NULL)
    {
        LE_ERROR("msdPtr is NULL !");
        return LE_FAULT;
    }

    if ( msdSize > sizeof(ECallMsd.msd_blob) )
    {
        LE_ERROR("The MSD provided is too big (%d)", msdSize);
        return LE_FAULT;
    }

    ECallMsd.msd_blob_len = msdSize;
    memcpy(ECallMsd.msd_blob, msdPtr, msdSize);

    LE_DEBUG("Send MSD is: 0x%X 0x%X 0x%x 0x%X... with length.%d",
             ECallMsd.msd_blob[0],
             ECallMsd.msd_blob[1],
             ECallMsd.msd_blob[2],
             ECallMsd.msd_blob[3],
             ECallMsd.msd_blob_len);

    rc = qmi_client_send_msg_sync(MgsClient,
                                  QMI_SWI_M2M_ECALL_SEND_MSD_BLOB_REQ_V01,
                                  &ECallMsd, sizeof(ECallMsd),
                                  &sendMsdBlobResp, sizeof(sendMsdBlobResp),
                                  COMM_DEFAULT_PLATFORM_TIMEOUT);

    return OEMCheckResponseCode(STRINGIZE_EXPAND(QMI_SWI_M2M_ECALL_SEND_MSD_BLOB_REQ_V01),
                                rc, sendMsdBlobResp.resp);
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to end a eCall.
 *
 * @return LE_FAULT        The function failed.
 * @return LE_OK           The function succeed.
 * @return LE_UNSUPPORTED  Not supported on this platform.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_ecall_End
(
    void
)
{
    le_result_t result;
    qmi_client_error_type rc;

    swi_m2m_ecall_stop_resp_msg_v01 ecallStopResp = { {0} };

    rc = qmi_client_send_msg_sync(MgsClient,
                                  QMI_SWI_M2M_ECALL_STOP_REQ_V01,
                                  NULL, 0,
                                  &ecallStopResp, sizeof(ecallStopResp),
                                  COMM_DEFAULT_PLATFORM_TIMEOUT);

    result = OEMCheckResponseCode(STRINGIZE_EXPAND(QMI_SWI_M2M_ECALL_STOP_REQ_V01),
                                  rc, ecallStopResp.resp);
    if (LE_OK == result)
    {
        memset(ECallMsd.msd_blob,0,sizeof(ECallMsd.msd_blob));
        ECallMsd.msd_blob_len = 0;
        return LE_OK;
    }

    LE_ERROR("Could not End the eCall");
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the 'NAD Deregistration Time' value in minutes.
 *
 * @return
 *  - LE_OK on success
 *  - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_ecall_SetNadDeregistrationTime
(
    uint16_t    deregTime  ///< [IN] the 'NAD Deregistration Time' value in minutes.
)
{
    if (PA_ECALL_PAN_EUROPEAN == SystemStandard)
    {
        if (0 == deregTime)
        {
            LE_ERROR("NAD Deregistration time cannot be set to zero!");
            return LE_FAULT;
        }

        if (LE_OK != GetEcallConfiguration(&ECallConfiguration))
        {
            LE_ERROR("Could not retrieve eCall Configuration");
            return LE_FAULT;
        }

        // Must be set to true to allow the nad_deregistration_time value change
        ECallConfiguration.nad_deregistration_time_valid = true;

        // nad_deregistration_time is set in hours on QMI platform. The conversion is:
        // from 1 to 60 minutes -> 1 hour, from 61 to 120 minutes -> 2hours, etc...
        ECallConfiguration.nad_deregistration_time = (deregTime+59)/60;
        LE_INFO("Set NAD Deregistration Time to %d minutes",
                ECallConfiguration.nad_deregistration_time * 60);

        if (LE_OK != SetEcallConfiguration(&ECallConfiguration))
        {
            LE_ERROR("Could not configure the eCall");
            return LE_FAULT;
        }
    }
    else if (PA_ECALL_ERA_GLONASS == SystemStandard)
    {
#ifdef QMI_SWI_M2M_SET_EGLONASS_CFG_REQ_V01
        SWIQMI_DECLARE_MESSAGE(swi_m2m_set_eglonass_cfg_req_msg_v01, config);

        // Must be set to true to allow the nad_deregistration_time value change
        config.NAD_Deregistration_Time_valid = true;
        config.NAD_Deregistration_Time = deregTime;

        if (LE_OK != SetEraGlonassConfiguration(&config))
        {
            LE_ERROR("Could not set ERA-GLONASS configuration");
            return LE_FAULT;
        }
        return LE_OK;
#else
        return LE_UNSUPPORTED;
#endif
    }
    else
    {
        LE_ERROR("System standard is not valid!");
        return LE_FAULT;
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the 'NAD Deregistration Time' value in minutes.
 *
 * @return
 *  - LE_OK on success
 *  - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_ecall_GetNadDeregistrationTime
(
    uint16_t*    deregTimePtr  ///< [OUT] the 'NAD Deregistration Time' value in minutes.
)
{
    if (deregTimePtr == NULL)
    {
        LE_ERROR("deregTimePtr is NULL !");
        return LE_FAULT;
    }

    if (PA_ECALL_PAN_EUROPEAN == SystemStandard)
    {
        if (LE_OK != GetEcallConfiguration(&ECallConfiguration))
        {
            LE_ERROR("Could not retrieve eCall Configuration");
            return LE_FAULT;
        }

        if (ECallConfiguration.nad_deregistration_time_valid)
        {
            *deregTimePtr = ECallConfiguration.nad_deregistration_time * 60;
            return LE_OK;
        }
        else
        {
            LE_ERROR("NAD Deregistration time is not received from QMI!");
            return LE_FAULT;
        }
    }
    else if (PA_ECALL_ERA_GLONASS == SystemStandard)
    {
#ifdef QMI_SWI_M2M_GET_EGLONASS_CFG_REQ_V01
        SWIQMI_DECLARE_MESSAGE(swi_m2m_get_eglonass_cfg_resp_msg_v01, config);

        if (LE_OK != GetEraGlonassConfiguration(&config))
        {
            LE_ERROR("Could not get ERA-GLONASS configuration");
            return LE_FAULT;
        }

        *deregTimePtr = config.NAD_Deregistration_Time;
        return LE_OK;
#else
        return LE_UNSUPPORTED;
#endif
    }
    else
    {
        LE_ERROR("System standard is not valid!");
        return LE_FAULT;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the 'ECALL_CCFT' value in minutes.
 *
 * @return
 *  - LE_OK on success
 *  - LE_FAULT on failure
 *  - LE_UNSUPPORTED if the function is not supported by the target
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_ecall_SetEraGlonassFallbackTime
(
    uint16_t    duration  ///< [IN] the ECALL_CCFT time value (in minutes)
)
{
#ifdef QMI_SWI_M2M_SET_EGLONASS_CFG_REQ_V01
   SWIQMI_DECLARE_MESSAGE(swi_m2m_set_eglonass_cfg_req_msg_v01, config);

   config.Test_Only_Cleardown_Timer_valid = 1;
   config.Test_Only_Cleardown_Timer = duration;

   if (LE_OK != SetEraGlonassConfiguration(&config))
   {
       LE_ERROR("Could not set ERA-GLONASS configuration");
       return LE_FAULT;
   }
   return LE_OK;
#else
    return LE_UNSUPPORTED;
#endif
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the 'ECALL_CCFT' value in minutes.
 *
 * @return
 *  - LE_OK on success
 *  - LE_FAULT on failure
 *  - LE_UNSUPPORTED if the function is not supported by the target
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_ecall_GetEraGlonassFallbackTime
(
    uint16_t*    durationPtr  ///< [OUT] the ECALL_CCFT time value (in minutes).
)
{
#ifdef QMI_SWI_M2M_GET_EGLONASS_CFG_REQ_V01
   SWIQMI_DECLARE_MESSAGE(swi_m2m_get_eglonass_cfg_resp_msg_v01, config);

   if (LE_OK != GetEraGlonassConfiguration(&config))
   {
       LE_ERROR("Could not get ERA-GLONASS configuration");
       return LE_FAULT;
   }

   *durationPtr = config.Test_Only_Cleardown_Timer;
    return LE_OK;
#else
    return LE_UNSUPPORTED;
#endif
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the 'ECALL_AUTO_ANSWER_TIME' time value in minutes.
 *
 * @return
 *  - LE_OK on success
 *  - LE_FAULT on failure
 *  - LE_UNSUPPORTED if the function is not supported by the target
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_ecall_SetEraGlonassAutoAnswerTime
(
    uint16_t autoAnswerTime  ///< [IN] The ECALL_AUTO_ANSWER_TIME time value in minutes.
)
{
#ifdef QMI_SWI_M2M_SET_EGLONASS_CFG_REQ_V01
    SWIQMI_DECLARE_MESSAGE(swi_m2m_set_eglonass_cfg_req_msg_v01, config);

    config.ECall_Auto_Answer_Time_valid = true;
    config.ECall_Auto_Answer_Time = autoAnswerTime;

    if (LE_OK != SetEraGlonassConfiguration(&config))
    {
        LE_ERROR("Could not set ERA-GLONASS configuration");
        return LE_FAULT;
    }
    return LE_OK;
#else
    return LE_UNSUPPORTED;
#endif
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the 'ECALL_AUTO_ANSWER_TIME' time value in minutes.
 *
 * @return
 *  - LE_OK on success
 *  - LE_FAULT on failure
 *  - LE_UNSUPPORTED if the function is not supported by the target
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_ecall_GetEraGlonassAutoAnswerTime
(
    uint16_t* autoAnswerTimePtr  ///< [OUT] The ECALL_AUTO_ANSWER_TIME time value in minutes.
)
{
#ifdef QMI_SWI_M2M_GET_EGLONASS_CFG_REQ_V01
    SWIQMI_DECLARE_MESSAGE(swi_m2m_get_eglonass_cfg_resp_msg_v01, config);

    if (NULL == autoAnswerTimePtr)
    {
        LE_ERROR("autoAnswerTimePtr is NULL!");
        return LE_FAULT;
    }

    if (LE_OK != GetEraGlonassConfiguration(&config))
    {
        LE_ERROR("Could not get ERA-GLONASS configuration");
        return LE_FAULT;
    }

    *autoAnswerTimePtr = config.ECall_Auto_Answer_Time;
    return LE_OK;
#else
    return LE_UNSUPPORTED;
#endif
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the 'ECALL_MSD_MAX_TRANSMISSION_TIME' time value in seconds.
 *
 * @return
 *  - LE_OK on success
 *  - LE_FAULT on failure
 *  - LE_UNSUPPORTED if the function is not supported by the target
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_ecall_SetEraGlonassMSDMaxTransmissionTime
(
    uint16_t msdMaxTransTime  ///< [IN] The ECALL_MSD_MAX_TRANSMISSION_TIME time value in seconds.
)
{
#ifdef QMI_SWI_M2M_SET_EGLONASS_CFG_REQ_V01
    SWIQMI_DECLARE_MESSAGE(swi_m2m_set_eglonass_cfg_req_msg_v01, config);

    config.MSD_Max_Transmission_Time_valid = true;
    config.MSD_Max_Transmission_Time = msdMaxTransTime;

    if (LE_OK != SetEraGlonassConfiguration(&config))
    {
        LE_ERROR("Could not set ERA-GLONASS configuration");
        return LE_FAULT;
    }
    return LE_OK;
#else
    return LE_UNSUPPORTED;
#endif
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the 'ECALL_MSD_MAX_TRANSMISSION_TIME' time value in seconds.
 *
 * @return
 *  - LE_OK on success
 *  - LE_FAULT on failure
 *  - LE_UNSUPPORTED if the function is not supported by the target
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_ecall_GetEraGlonassMSDMaxTransmissionTime
(
    uint16_t* msdMaxTransTimePtr  ///< [OUT] The ECALL_MSD_MAX_TRANSMISSION_TIME time in seconds.
)
{
#ifdef QMI_SWI_M2M_GET_EGLONASS_CFG_REQ_V01
    SWIQMI_DECLARE_MESSAGE(swi_m2m_get_eglonass_cfg_resp_msg_v01, config);

    if (NULL == msdMaxTransTimePtr)
    {
        LE_ERROR("msdMaxTransTimePtr is NULL!");
        return LE_FAULT;
    }

    if (LE_OK != GetEraGlonassConfiguration(&config))
    {
        LE_ERROR("Could not get ERA-GLONASS configuration");
        return LE_FAULT;
    }

    *msdMaxTransTimePtr = config.MSD_Max_Transmission_Time;
    return LE_OK;
#else
    return LE_UNSUPPORTED;
#endif
}

#if defined(QMI_SWI_M2M_SET_EGLONASS_CFG_REQ_V01) && defined(SIERRA_MDM9X28)
//--------------------------------------------------------------------------------------------------
/**
 * Set the 'ECALL_POST_TEST_REGISTRATION_TIME' time value in seconds.
 *
 * @return
 *  - LE_OK on success
 *  - LE_FAULT on failure
 *  - LE_UNSUPPORTED if the function is not supported by the target
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_ecall_SetEraGlonassPostTestRegistrationTime
(
    uint16_t postTestRegTime  ///< [IN] ECALL_POST_TEST_REGISTRATION_TIME time value (in seconds).
)
{
    SWIQMI_DECLARE_MESSAGE(swi_m2m_set_eglonass_cfg_req_msg_v01, config);

    config.Post_Test_Registration_Time_valid = true;
    config.Post_Test_Registration_Time = postTestRegTime;

    if (LE_OK != SetEraGlonassConfiguration(&config))
    {
        LE_ERROR("Could not set ERA-GLONASS configuration");
        return LE_FAULT;
    }
    return LE_OK;
}
#endif

#if defined(QMI_SWI_M2M_GET_EGLONASS_CFG_REQ_V01) && defined(SIERRA_MDM9X28)
//--------------------------------------------------------------------------------------------------
/**
 * Get the 'ECALL_POST_TEST_REGISTRATION_TIME' time value in seconds.
 *
 * @return
 *  - LE_OK on success
 *  - LE_FAULT on failure
 *  - LE_UNSUPPORTED if the function is not supported by the target
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_ecall_GetEraGlonassPostTestRegistrationTime
(
    uint16_t* postTestRegTimePtr  ///< [OUT] Post test registration time value (in seconds).
)
{
    SWIQMI_DECLARE_MESSAGE(swi_m2m_get_eglonass_cfg_resp_msg_v01, config);

    if (NULL == postTestRegTimePtr)
    {
        LE_ERROR("postTestRegTimePtr is NULL!");
        return LE_FAULT;
    }

    if (LE_OK != GetEraGlonassConfiguration(&config))
    {
        LE_ERROR("Could not get ERA-GLONASS configuration");
        return LE_FAULT;
    }

    *postTestRegTimePtr = config.Post_Test_Registration_Time;
    return LE_OK;
}
#endif