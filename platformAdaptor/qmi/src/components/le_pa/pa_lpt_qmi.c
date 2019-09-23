/**
 * @file pa_lpt_qmi.c
 *
 * QMI implementation of @ref c_pa_lpt API.
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#include "legato.h"
#include "le_print.h"
#include "lpt_qmi.h"
#include "pa_lpt.h"
#include "swiQmi.h"


//--------------------------------------------------------------------------------------------------
// Symbol and Enum definitions.
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Invalid value for eDRX cycle length, used for initialization.
 */
//--------------------------------------------------------------------------------------------------
#define INVALID_EDRX_VALUE  -1


//--------------------------------------------------------------------------------------------------
// Static declarations.
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * The NAS (Network Access Service) client.
 * Must be obtained by calling swiQmi_GetClientHandle() before it can be used.
 */
//--------------------------------------------------------------------------------------------------
static qmi_client_type NasClient;

//--------------------------------------------------------------------------------------------------
/**
 * This event is reported when an eDRX parameters change indication is received from the modem.
 */
//--------------------------------------------------------------------------------------------------
static le_event_Id_t EDrxParamsChangeEventId;

//--------------------------------------------------------------------------------------------------
/**
 * Pool for eDRX parameters change indication reporting.
 */
//--------------------------------------------------------------------------------------------------
static le_mem_PoolRef_t EDrxParamsChangeIndPool;

//--------------------------------------------------------------------------------------------------
/**
 * Requested eDRX cycle length values for the different Radio Access Technologies.
 */
//--------------------------------------------------------------------------------------------------
static int8_t RequestedEDrxValue[LE_LPT_EDRX_RAT_MAX] =
{
    INVALID_EDRX_VALUE,     ///< LE_LPT_EDRX_RAT_UNKNOWN
    INVALID_EDRX_VALUE,     ///< LE_LPT_EDRX_RAT_EC_GSM_IOT
    INVALID_EDRX_VALUE,     ///< LE_LPT_EDRX_RAT_GSM
    INVALID_EDRX_VALUE,     ///< LE_LPT_EDRX_RAT_UTRAN
    INVALID_EDRX_VALUE,     ///< LE_LPT_EDRX_RAT_LTE_M1
    INVALID_EDRX_VALUE,     ///< LE_LPT_EDRX_RAT_LTE_NB1
};

#if (defined(QMI_NAS_SET_EDRX_PARAMS_REQ_MSG_V01) || defined(QMI_NAS_GET_EDRX_PARAMS_REQ_MSG_V01))
//--------------------------------------------------------------------------------------------------
/**
 * Convert Legato eDRX Radio Access Technology to QMI eDRX Radio Access Technology.
 *
 * @return
 *  - LE_OK             The function succeeded.
 *  - LE_BAD_PARAMETER  A parameter is invalid.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t ConvertEDrxRatFromLegatoToQmi
(
    le_lpt_EDrxRat_t                            eDrxRat,         ///< [IN]  Legato RAT.
    nas_radio_if_enum_v01*                      ratTypePtr,      ///< [OUT] Radio Access Technology.
    uint8_t*                                    lteModevalidPtr, ///< [OUT] CIOT LTE mode validity.
    nas_camped_ciot_lte_op_mode_enum_type_v01*  lteModePtr       ///< [OUT] CIOT LTE mode.
)
{
    switch (eDrxRat)
    {
        case LE_LPT_EDRX_RAT_EC_GSM_IOT:
        case LE_LPT_EDRX_RAT_GSM:
            *ratTypePtr = NAS_RADIO_IF_GSM_V01;
            *lteModevalidPtr = 0;
            break;

        case LE_LPT_EDRX_RAT_UTRAN:
            *ratTypePtr = NAS_RADIO_IF_UMTS_V01;
            *lteModevalidPtr = 0;
            break;

        case LE_LPT_EDRX_RAT_LTE_M1:
            *ratTypePtr = NAS_RADIO_IF_LTE_V01;
            *lteModevalidPtr = 1;
            *lteModePtr = NAS_CIOT_SYS_MODE_LTE_M1_V01;
            break;

        case LE_LPT_EDRX_RAT_LTE_NB1:
            *ratTypePtr = NAS_RADIO_IF_LTE_V01;
            *lteModevalidPtr = 1;
            *lteModePtr = NAS_CIOT_SYS_MODE_LTE_NB1_V01;
            break;

        default:
            LE_ERROR("Invalid Radio Access Technology: %d", eDrxRat);
            return LE_BAD_PARAMETER;
    }

    return LE_OK;
}
#endif // QMI_NAS_SET_EDRX_PARAMS_REQ_MSG_V01 || QMI_NAS_GET_EDRX_PARAMS_REQ_MSG_V01

#if defined(QMI_NAS_EDRX_CHANGE_INFO_IND_V01)
//--------------------------------------------------------------------------------------------------
/**
 * Convert QMI eDRX Radio Access Technology to Legato eDRX Radio Access Technology.
 *
 * @return Converted eDRX RAT.
 */
//--------------------------------------------------------------------------------------------------
static le_lpt_EDrxRat_t ConvertEDrxRatFromQmiToLegato
(
    nas_radio_if_enum_v01                     ratType,      ///< [IN] Radio Access Technology.
    uint8_t                                   lteModevalid, ///< [IN] CIOT LTE mode validity.
    nas_camped_ciot_lte_op_mode_enum_type_v01 lteMode       ///< [IN] CIOT LTE mode.
)
{
    switch (ratType)
    {
        case NAS_RADIO_IF_GSM_V01:
            return LE_LPT_EDRX_RAT_GSM;

        case NAS_RADIO_IF_UMTS_V01:
        case NAS_RADIO_IF_TDSCDMA_V01:
            return LE_LPT_EDRX_RAT_UTRAN;

        case NAS_RADIO_IF_LTE_V01:
            if (!lteModevalid)
            {
                return LE_LPT_EDRX_RAT_UNKNOWN;
            }
            switch (lteMode)
            {
                case NAS_CIOT_SYS_MODE_LTE_WB_V01:
                case NAS_CIOT_SYS_MODE_LTE_M1_V01:
                    return LE_LPT_EDRX_RAT_LTE_M1;

                case NAS_CIOT_SYS_MODE_LTE_NB1_V01:
                    return LE_LPT_EDRX_RAT_LTE_NB1;

                default:
                    LE_ERROR("Unsupported CIOT LTE mode: %d", lteMode);
                    return LE_LPT_EDRX_RAT_UNKNOWN;
            }
            break;

        default:
            LE_ERROR("Unsupported eDRX RAT: %d", ratType);
            return LE_LPT_EDRX_RAT_UNKNOWN;
    }
}
#endif // QMI_NAS_EDRX_CHANGE_INFO_IND_V01

#if defined(QMI_NAS_EDRX_CHANGE_INFO_IND_V01)
//--------------------------------------------------------------------------------------------------
/**
 * eDRX parameters change indication handler called by the QMI NAS indications.
 */
//--------------------------------------------------------------------------------------------------
static void EDrxParamsChangeIndHandler
(
    void*           indBufPtr,  ///< [IN] The indication message data.
    unsigned int    indBufLen,  ///< [IN] The indication message data length in bytes.
    void*           contextPtr  ///< [IN] Context value that was passed to
                                ///<      swiQmi_AddIndicationHandler().
)
{
    nas_edrx_change_info_ind_v01 nasInd;
    pa_lpt_EDrxParamsIndication_t* eDrxParamsChangeIndPtr;
    qmi_client_error_type clientMsgErr;

    clientMsgErr = qmi_client_message_decode(NasClient,
                                             QMI_IDL_INDICATION,
                                             QMI_NAS_EDRX_CHANGE_INFO_IND_V01,
                                             indBufPtr, indBufLen,
                                             &nasInd, sizeof(nasInd));
    if (QMI_NO_ERR != clientMsgErr)
    {
        LE_ERROR("Error in decoding QMI_NAS_EDRX_CHANGE_INFO_IND_V01, error = %d", clientMsgErr);
        return;
    }

    LE_DEBUG("New eDRX parameters validity: enabled=%c, eDRX value=%c, PTW=%c, RAT=%c, LTE Mode=%c",
             (nasInd.edrx_enabled_valid ? 'Y' : 'N'),
             (nasInd.edrx_cycle_length_valid ? 'Y' : 'N'),
             (nasInd.edrx_ptw_valid ? 'Y' : 'N'),
             (nasInd.edrx_rat_type_valid ? 'Y' : 'N'),
             (nasInd.edrx_ciot_lte_mode_valid ? 'Y' : 'N'));
    LE_PRINT_VALUE_IF(nasInd.edrx_enabled_valid, "%d", nasInd.edrx_enabled);
    LE_PRINT_VALUE_IF(nasInd.edrx_cycle_length_valid, "%d", nasInd.edrx_cycle_length);
    LE_PRINT_VALUE_IF(nasInd.edrx_ptw_valid, "%d", nasInd.edrx_ptw);
    LE_PRINT_VALUE_IF(nasInd.edrx_rat_type_valid, "%d", nasInd.edrx_rat_type);
    LE_PRINT_VALUE_IF(nasInd.edrx_ciot_lte_mode_valid, "%d", nasInd.edrx_ciot_lte_mode);

    eDrxParamsChangeIndPtr = le_mem_ForceAlloc(EDrxParamsChangeIndPool);
    memset(eDrxParamsChangeIndPtr, 0, sizeof(pa_lpt_EDrxParamsIndication_t));

    if (nasInd.edrx_rat_type_valid)
    {
        eDrxParamsChangeIndPtr->rat = ConvertEDrxRatFromQmiToLegato(nasInd.edrx_rat_type,
                                                                    nasInd.edrx_ciot_lte_mode_valid,
                                                                    nasInd.edrx_ciot_lte_mode);
    }
    if (nasInd.edrx_enabled_valid)
    {
        eDrxParamsChangeIndPtr->activation = (nasInd.edrx_enabled ? LE_ON : LE_OFF);
    }
    if (nasInd.edrx_cycle_length_valid)
    {
        eDrxParamsChangeIndPtr->eDrxValue = nasInd.edrx_cycle_length;
    }
    if (nasInd.edrx_ptw_valid)
    {
        eDrxParamsChangeIndPtr->pagingTimeWindow = nasInd.edrx_ptw;
    }

    le_event_ReportWithRefCounting(EDrxParamsChangeEventId, eDrxParamsChangeIndPtr);
}
#endif // QMI_NAS_EDRX_CHANGE_INFO_IND_V01


//--------------------------------------------------------------------------------------------------
// APIs.
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to register a handler for eDRX parameters change indication.
 *
 * @return A handler reference, which is only needed for later removal of the handler.
 *
 * @note Doesn't return on failure, so there's no need to check the return value for errors.
 */
//--------------------------------------------------------------------------------------------------
le_event_HandlerRef_t pa_lpt_AddEDrxParamsChangeHandler
(
    pa_lpt_EDrxParamsChangeIndHandlerFunc_t handlerFuncPtr  ///< [IN] The handler function.
)
{
    LE_ASSERT(handlerFuncPtr);

    return le_event_AddHandler("EDrxParamsChange",
                               EDrxParamsChangeEventId,
                               (le_event_HandlerFunc_t) handlerFuncPtr);
}

#if defined(QMI_NAS_SET_EDRX_PARAMS_REQ_MSG_V01)
//--------------------------------------------------------------------------------------------------
/**
 * Set the eDRX activation state for the given Radio Access Technology.
 *
 * @return
 *  - LE_OK             The function succeeded.
 *  - LE_BAD_PARAMETER  A parameter is invalid.
 *  - LE_UNSUPPORTED    eDRX is not supported by the platform.
 *  - LE_FAULT          The function failed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_lpt_SetEDrxState
(
    le_lpt_EDrxRat_t    eDrxRat,    ///< [IN] Radio Access Technology.
    le_onoff_t          activation  ///< [IN] eDRX activation state.
)
{
    le_result_t result;
    qmi_client_error_type clientMsgErr;
    SWIQMI_DECLARE_MESSAGE(nas_set_edrx_params_req_msg_v01, setParamsReq);
    SWIQMI_DECLARE_MESSAGE(nas_set_edrx_params_resp_msg_v01, setParamsResp);

    setParamsReq.edrx_rat_type_valid = 1;
    result = ConvertEDrxRatFromLegatoToQmi(eDrxRat,
                                           &setParamsReq.edrx_rat_type,
                                           &setParamsReq.edrx_ciot_lte_mode_valid,
                                           &setParamsReq.edrx_ciot_lte_mode);
    if (LE_OK != result)
    {
        return result;
    }

    switch (activation)
    {
        case LE_ON:
            setParamsReq.edrx_enabled_valid = true;
            setParamsReq.edrx_enabled = 1;
            break;

        case LE_OFF:
            setParamsReq.edrx_enabled_valid = false;
            setParamsReq.edrx_enabled = 0;
            break;

        default:
            LE_ERROR("Invalid activation state %d", activation);
            return LE_BAD_PARAMETER;
    }

    LE_DEBUG("%s eDRX for RAT type %d with LTE mode: %c (%d)",
             (setParamsReq.edrx_enabled_valid ? "Enable" : "Disable"),
             setParamsReq.edrx_rat_type,
             (setParamsReq.edrx_ciot_lte_mode_valid ? 'Y' : 'N'),
             setParamsReq.edrx_ciot_lte_mode);

    clientMsgErr = qmi_client_send_msg_sync(NasClient,
                                            QMI_NAS_SET_EDRX_PARAMS_REQ_MSG_V01,
                                            &setParamsReq, sizeof(setParamsReq),
                                            &setParamsResp, sizeof(setParamsResp),
                                            COMM_LONG_PLATFORM_TIMEOUT);
    if (LE_OK != swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_NAS_SET_EDRX_PARAMS_REQ_MSG_V01),
                                      clientMsgErr,
                                      setParamsResp.resp.result,
                                      setParamsResp.resp.error))
    {
        return LE_FAULT;
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the requested eDRX cycle value for the given Radio Access Technology.
 *
 * @return
 *  - LE_OK             The function succeeded.
 *  - LE_BAD_PARAMETER  A parameter is invalid.
 *  - LE_UNSUPPORTED    eDRX is not supported by the platform.
 *  - LE_FAULT          The function failed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_lpt_SetRequestedEDrxValue
(
    le_lpt_EDrxRat_t    eDrxRat,    ///< [IN] Radio Access Technology.
    uint8_t             eDrxValue   ///< [IN] Requested eDRX cycle value.
)
{
    le_result_t result;
    qmi_client_error_type clientMsgErr;
    SWIQMI_DECLARE_MESSAGE(nas_set_edrx_params_req_msg_v01, setParamsReq);
    SWIQMI_DECLARE_MESSAGE(nas_set_edrx_params_resp_msg_v01, setParamsResp);

    setParamsReq.edrx_rat_type_valid = 1;
    result = ConvertEDrxRatFromLegatoToQmi(eDrxRat,
                                           &setParamsReq.edrx_rat_type,
                                           &setParamsReq.edrx_ciot_lte_mode_valid,
                                           &setParamsReq.edrx_ciot_lte_mode);
    if (LE_OK != result)
    {
        return result;
    }

    setParamsReq.edrx_cycle_length_valid = 1;
    setParamsReq.edrx_cycle_length = eDrxValue;

    LE_DEBUG("Set eDRX value %d for RAT type %d with LTE mode: %c (%d)",
             setParamsReq.edrx_cycle_length,
             setParamsReq.edrx_rat_type,
             (setParamsReq.edrx_ciot_lte_mode_valid ? 'Y' : 'N'),
             setParamsReq.edrx_ciot_lte_mode);

    clientMsgErr = qmi_client_send_msg_sync(NasClient,
                                            QMI_NAS_SET_EDRX_PARAMS_REQ_MSG_V01,
                                            &setParamsReq, sizeof(setParamsReq),
                                            &setParamsResp, sizeof(setParamsResp),
                                            COMM_LONG_PLATFORM_TIMEOUT);
    if (LE_OK != swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_NAS_SET_EDRX_PARAMS_REQ_MSG_V01),
                                      clientMsgErr,
                                      setParamsResp.resp.result,
                                      setParamsResp.resp.error))
    {
        return LE_FAULT;
    }

    RequestedEDrxValue[eDrxRat] = eDrxValue;
    return LE_OK;
}
#endif // QMI_NAS_SET_EDRX_PARAMS_REQ_MSG_V01

//--------------------------------------------------------------------------------------------------
/**
 * Get the requested eDRX cycle value for the given Radio Access Technology.
 *
 * @return
 *  - LE_OK             The function succeeded.
 *  - LE_BAD_PARAMETER  A parameter is invalid.
 *  - LE_UNSUPPORTED    eDRX is not supported by the platform.
 *  - LE_FAULT          The function failed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_lpt_GetRequestedEDrxValue
(
    le_lpt_EDrxRat_t    eDrxRat,        ///< [IN] Radio Access Technology.
    uint8_t*            eDrxValuePtr    ///< [OUT] Requested eDRX cycle value
)
{
    if ((LE_LPT_EDRX_RAT_UNKNOWN == eDrxRat) || (eDrxRat >= LE_LPT_EDRX_RAT_MAX))
    {
        LE_ERROR("Invalid Radio Access Technology: %d", eDrxRat);
        return LE_BAD_PARAMETER;
    }

    if (!eDrxValuePtr)
    {
        LE_ERROR("Invalid parameter");
        return LE_BAD_PARAMETER;
    }

    if (INVALID_EDRX_VALUE == RequestedEDrxValue[eDrxRat])
    {
        *eDrxValuePtr = 0;
        return LE_UNAVAILABLE;
    }

    *eDrxValuePtr = RequestedEDrxValue[eDrxRat];
    return LE_OK;
}

#if defined(QMI_NAS_GET_EDRX_PARAMS_REQ_MSG_V01)
//--------------------------------------------------------------------------------------------------
/**
 * Get the network-provided eDRX cycle value for the given Radio Access Technology.
 *
 * @return
 *  - LE_OK             The function succeeded.
 *  - LE_BAD_PARAMETER  A parameter is invalid.
 *  - LE_UNSUPPORTED    eDRX is not supported by the platform.
 *  - LE_UNAVAILABLE    No network-provided eDRX cycle value.
 *  - LE_FAULT          The function failed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_lpt_GetNetworkProvidedEDrxValue
(
    le_lpt_EDrxRat_t    eDrxRat,        ///< [IN]  Radio Access Technology.
    uint8_t*            eDrxValuePtr    ///< [OUT] Network-provided eDRX cycle value.
)
{
    le_result_t result;
    qmi_client_error_type clientMsgErr;
    SWIQMI_DECLARE_MESSAGE(nas_get_edrx_params_req_msg_v01, getParamsReq);
    SWIQMI_DECLARE_MESSAGE(nas_get_edrx_params_resp_msg_v01, getParamsResp);

    if (!eDrxValuePtr)
    {
        LE_ERROR("Invalid parameter");
        return LE_BAD_PARAMETER;
    }

    getParamsReq.edrx_rat_type_valid = 1;
    result = ConvertEDrxRatFromLegatoToQmi(eDrxRat,
                                           &getParamsReq.edrx_rat_type,
                                           &getParamsReq.edrx_ciot_lte_mode_valid,
                                           &getParamsReq.edrx_ciot_lte_mode);
    if (LE_OK != result)
    {
        return result;
    }

    LE_DEBUG("Retrieve eDRX cycle value for RAT type %d with LTE mode: %c (%d)",
             getParamsReq.edrx_rat_type,
             (getParamsReq.edrx_ciot_lte_mode_valid ? 'Y' : 'N'),
             getParamsReq.edrx_ciot_lte_mode);

    clientMsgErr = qmi_client_send_msg_sync(NasClient,
                                            QMI_NAS_GET_EDRX_PARAMS_REQ_MSG_V01,
                                            &getParamsReq, sizeof(getParamsReq),
                                            &getParamsResp, sizeof(getParamsResp),
                                            COMM_LONG_PLATFORM_TIMEOUT);
    if (LE_OK != swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_NAS_GET_EDRX_PARAMS_REQ_MSG_V01),
                                      clientMsgErr,
                                      getParamsResp.resp.result,
                                      getParamsResp.resp.error))
    {
        return LE_FAULT;
    }

    LE_DEBUG("eDRX value: %c (%d)",
             (getParamsResp.edrx_cycle_length_valid ? 'Y' : 'N'),
             getParamsResp.edrx_cycle_length);

    if (!getParamsResp.edrx_cycle_length_valid)
    {
        *eDrxValuePtr = 0;
        return LE_UNAVAILABLE;
    }

    *eDrxValuePtr = getParamsResp.edrx_cycle_length;
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the network-provided Paging Time Window for the given Radio Access Technology.
 *
 * @return
 *  - LE_OK             The function succeeded.
 *  - LE_BAD_PARAMETER  A parameter is invalid.
 *  - LE_UNSUPPORTED    eDRX is not supported by the platform.
 *  - LE_UNAVAILABLE    No defined Paging Time Window.
 *  - LE_FAULT          The function failed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_lpt_GetNetworkProvidedPagingTimeWindow
(
    le_lpt_EDrxRat_t    eDrxRat,            ///< [IN]  Radio Access Technology.
    uint8_t*            pagingTimeWindowPtr ///< [OUT] Network-provided Paging Time Window.
)
{
    le_result_t result;
    qmi_client_error_type clientMsgErr;
    SWIQMI_DECLARE_MESSAGE(nas_get_edrx_params_req_msg_v01, getParamsReq);
    SWIQMI_DECLARE_MESSAGE(nas_get_edrx_params_resp_msg_v01, getParamsResp);

    if (!pagingTimeWindowPtr)
    {
        LE_ERROR("Invalid parameter");
        return LE_BAD_PARAMETER;
    }

    getParamsReq.edrx_rat_type_valid = 1;
    result = ConvertEDrxRatFromLegatoToQmi(eDrxRat,
                                           &getParamsReq.edrx_rat_type,
                                           &getParamsReq.edrx_ciot_lte_mode_valid,
                                           &getParamsReq.edrx_ciot_lte_mode);
    if (LE_OK != result)
    {
        return result;
    }

    LE_DEBUG("Retrieve eDRX parameters for RAT type %d with LTE mode: %c (%d)",
             getParamsReq.edrx_rat_type,
             (getParamsReq.edrx_ciot_lte_mode_valid ? 'Y' : 'N'),
             getParamsReq.edrx_ciot_lte_mode);

    clientMsgErr = qmi_client_send_msg_sync(NasClient,
                                            QMI_NAS_GET_EDRX_PARAMS_REQ_MSG_V01,
                                            &getParamsReq, sizeof(getParamsReq),
                                            &getParamsResp, sizeof(getParamsResp),
                                            COMM_LONG_PLATFORM_TIMEOUT);
    if (LE_OK != swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_NAS_GET_EDRX_PARAMS_REQ_MSG_V01),
                                      clientMsgErr,
                                      getParamsResp.resp.result,
                                      getParamsResp.resp.error))
    {
        return LE_FAULT;
    }

    LE_DEBUG("Paging Time Window: %c (%d)",
             (getParamsResp.edrx_ptw_valid ? 'Y' : 'N'),
             getParamsResp.edrx_ptw);

    if (!getParamsResp.edrx_ptw_valid)
    {
        *pagingTimeWindowPtr = 0;
        return LE_UNAVAILABLE;
    }

    *pagingTimeWindowPtr = getParamsResp.edrx_ptw;
    return LE_OK;
}
#endif // QMI_NAS_GET_EDRX_PARAMS_REQ_MSG_V01

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to initialize the QMI LPT module.
 *
 * @return
 *      LE_OK if successful.
 *      LE_FAULT if unsuccessful.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_lpt_Init
(
    void
)
{
    // Create the event for eDRX parameters change indication.
    EDrxParamsChangeEventId = le_event_CreateIdWithRefCounting("EDrxParamsChangeEvent");

    // Create the associated memory pool.
    EDrxParamsChangeIndPool = le_mem_CreatePool("EDrxParamsChangeIndPool",
                                                sizeof(pa_lpt_EDrxParamsIndication_t));

    if (LE_OK != swiQmi_InitServices(QMI_SERVICE_NAS))
    {
        LE_CRIT("QMI NAS service cannot be initialized");
        return LE_FAULT;
    }

    // Get the QMI client service handle for later use.
    NasClient = swiQmi_GetClientHandle(QMI_SERVICE_NAS);
    if (!NasClient)
    {
        LE_CRIT("Unable to get QMI client handle");
        return LE_FAULT;
    }

#if defined(QMI_NAS_EDRX_CHANGE_INFO_IND_V01)
    // Register for eDRX parameters change notifications
    SWIQMI_DECLARE_MESSAGE(nas_indication_register_req_msg_v01, indRegReq);
    SWIQMI_DECLARE_MESSAGE(nas_indication_register_resp_msg_v01, indRegResp);
    qmi_client_error_type clientMsgErr;

    indRegReq.reg_edrx_change_info_ind_valid = true;
    indRegReq.reg_edrx_change_info_ind = 0x01;

    clientMsgErr = qmi_client_send_msg_sync(NasClient,
                                            QMI_NAS_INDICATION_REGISTER_REQ_MSG_V01,
                                            &indRegReq, sizeof(indRegReq),
                                            &indRegResp, sizeof(indRegResp),
                                            COMM_DEFAULT_PLATFORM_TIMEOUT);
    if (LE_OK != swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_NAS_INDICATION_REGISTER_REQ_MSG_V01),
                                      clientMsgErr,
                                      indRegResp.resp.result,
                                      indRegResp.resp.error))
    {
        LE_WARN("Cannot register for eDRX parameters change indication notifications!");
    }

    // Register our own handler with the QMI NAS service.
    swiQmi_AddIndicationHandler(EDrxParamsChangeIndHandler,
                                QMI_SERVICE_NAS,
                                QMI_NAS_EDRX_CHANGE_INFO_IND_V01,
                                NULL);
#endif // QMI_NAS_EDRX_CHANGE_INFO_IND_V01

    return LE_OK;
}
