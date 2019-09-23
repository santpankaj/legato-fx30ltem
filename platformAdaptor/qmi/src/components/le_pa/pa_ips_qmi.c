/**
 * @file pa_psi_qmi.c
 *
 * QMI implementation of pa Input Power Supply API.
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#include "legato.h"
#include "interfaces.h"
#include "pa_ips.h"
#include "swiQmi.h"

//--------------------------------------------------------------------------------------------------
/**
 * The QMI clients. Must be obtained by calling swiQmi_GetClientHandle() before it can be used.
 */
//--------------------------------------------------------------------------------------------------
static qmi_client_type DmsClient;

//--------------------------------------------------------------------------------------------------
/**
 * Input Voltage status threshold Event Id
 */
//--------------------------------------------------------------------------------------------------
static le_event_Id_t VoltageThresholdEventId;

//--------------------------------------------------------------------------------------------------
/**
 * Pool for Input Voltage threshold Event reporting.
 */
//--------------------------------------------------------------------------------------------------
static le_mem_PoolRef_t VoltageThresholdEventPool;

//--------------------------------------------------------------------------------------------------
/*
 * Voltage environment structure.
 */
//--------------------------------------------------------------------------------------------------
typedef struct {
        uint16_t voltage;        ///<  Current voltage in mV
        uint16_t hi_critical;    ///<  High Critical threshold.
        uint16_t hi_normal;      ///<  High Normal threshold.
        uint16_t lo_normal;      ///<  Low Normal (Normal) threshold.
        uint16_t lo_warning;     ///<  Low Warning (warning) threshold.
        uint16_t lo_critical;    ///<  Low Critical threshold.
} VoltageEnvironment_t;


//--------------------------------------------------------------------------------------------------
/**
 * Get Current voltage environment
 */
//--------------------------------------------------------------------------------------------------
static le_result_t GetVoltageEnvironement
(
    VoltageEnvironment_t * voltageEnvPtr
)
{
    le_result_t result;
    dms_swi_get_environment_resp_msg_v01 snResp = { {0} };

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync( DmsClient,
                    QMI_DMS_SWI_GET_ENVIRONMENT_REQ_V01,
                    NULL, 0,
                    &snResp, sizeof(snResp),
                    COMM_DEFAULT_PLATFORM_TIMEOUT);

    result = swiQmi_CheckResponseCode(
                    STRINGIZE_EXPAND(QMI_DMS_SWI_GET_ENVIRONMENT_REQ_V01),
                    clientMsgErr,
                    snResp.resp );

    if ( result == LE_OK )
    {
        if ( snResp.DMS_SWI_ENVIRONMENT(get, voltage_environment_valid) )
        {
            // Input voltage
            voltageEnvPtr->voltage =
                            snResp.DMS_SWI_ENVIRONMENT(get, voltage_environment).voltage;
            voltageEnvPtr->hi_critical =
                            snResp.DMS_SWI_ENVIRONMENT(get, voltage_environment).hi_critical;
            voltageEnvPtr->hi_normal =
                            snResp.DMS_SWI_ENVIRONMENT(get, voltage_environment).hi_normal;
            voltageEnvPtr->lo_normal =
                            snResp.DMS_SWI_ENVIRONMENT(get, voltage_environment).lo_normal;
            voltageEnvPtr->lo_warning =
                            snResp.DMS_SWI_ENVIRONMENT(get, voltage_environment).lo_warning;
            voltageEnvPtr->lo_critical =
                            snResp.DMS_SWI_ENVIRONMENT(get, voltage_environment).lo_critical;

            LE_DEBUG("Get Platform input voltage %u, lo_crit %u, lo_warning %u,"
                " lo_nor %u, hi_nor %u, hi_crit %u",
                voltageEnvPtr->voltage,
                voltageEnvPtr->lo_critical,
                voltageEnvPtr->lo_warning,
                voltageEnvPtr->lo_normal,
                voltageEnvPtr->hi_normal,
                voltageEnvPtr->hi_critical);

            return LE_OK;
        }
        else
        {
            result = LE_FAULT;
        }
    }

    LE_ERROR("Failed to get Platform input voltage Valid '%c'",
        ( snResp.DMS_SWI_ENVIRONMENT(get, voltage_environment_valid) ? 'Y' : 'N') );

    return result;
}


//--------------------------------------------------------------------------------------------------
/**
 * Set Current voltage environment
 */
//--------------------------------------------------------------------------------------------------
static le_result_t SetVoltageEnvironement
(
    VoltageEnvironment_t * voltageEnvPtr
)
{
    le_result_t result = LE_OK;

    dms_swi_set_environment_req_msg_v01 snReq = { 0 };
    dms_swi_set_environment_resp_msg_v01 snResp = { {0} };

    snReq.set_voltage_environment_valid = true;
    snReq.DMS_SWI_ENVIRONMENT(set, voltage_environment).hi_critical = voltageEnvPtr->hi_critical;
    // hi_normal parameter have no effect
    snReq.DMS_SWI_ENVIRONMENT(set, voltage_environment).hi_normal = voltageEnvPtr->hi_normal;
    snReq.DMS_SWI_ENVIRONMENT(set, voltage_environment).lo_normal = voltageEnvPtr->lo_normal;
    snReq.DMS_SWI_ENVIRONMENT(set, voltage_environment).lo_warning = voltageEnvPtr->lo_warning;
    snReq.DMS_SWI_ENVIRONMENT(set, voltage_environment).lo_critical = voltageEnvPtr->lo_critical;

    LE_DEBUG("Set input voltage Thresholds LoCrit %u, LoWarn %u, LoNorm %u, HiNorm %u, HiCri %u",
        voltageEnvPtr->lo_critical,
        voltageEnvPtr->lo_warning,
        voltageEnvPtr->lo_normal,
        voltageEnvPtr->hi_normal,
        voltageEnvPtr->hi_critical  );

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync( DmsClient,
        QMI_DMS_SWI_SET_ENVIRONMENT_REQ_V01,
        &snReq, sizeof(snReq),
        &snResp, sizeof(snResp),
        COMM_DEFAULT_PLATFORM_TIMEOUT);

    result = swiQmi_CheckResponseCode(
        STRINGIZE_EXPAND(QMI_DMS_SWI_SET_ENVIRONMENT_REQ_V01),
        clientMsgErr,
        snResp.resp );

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * New message handler function called by the DMS QMI service indications.
 */
//--------------------------------------------------------------------------------------------------
static void NewMsgIPSHandler
(
    void*           indBufPtr,  // [IN] The indication message data.
    unsigned int    indBufLen,  // [IN] The indication message data length in bytes.
    void*           contextPtr  // [IN] Context value that was passed to
                                //      swiQmi_AddIndicationHandler().
)
{
    if (indBufPtr == NULL)
    {
        LE_ERROR("Indication message empty.");
        return;
    }

    dms_swi_event_report_ind_msg_v01 msgEventInd = { 0 };

    qmi_client_error_type clientMsgErr = qmi_client_message_decode(
        DmsClient, QMI_IDL_INDICATION, QMI_DMS_SWI_EVENT_REPORT_IND_V01,
        indBufPtr, indBufLen,
        &msgEventInd, sizeof(msgEventInd));

    if (clientMsgErr != QMI_NO_ERR)
    {
        LE_ERROR("Error in decoding QMI_DMS_SWI_EVENT_REPORT_IND_V01, error = %i", clientMsgErr);
        return;
    }

    if (msgEventInd.voltage_status_valid)
    {
        LE_DEBUG("Input Voltage status event %d", msgEventInd.voltage_status.state);
        le_ips_ThresholdStatus_t * VoltageEventPtr =
                        le_mem_ForceAlloc(VoltageThresholdEventPool);

        switch(msgEventInd.voltage_status.state)
        {
            // normal voltage
            case 1:
            {
                *VoltageEventPtr = LE_IPS_VOLTAGE_NORMAL;
            }
            break;

            // low voltage warning
            case 2:
            {
                *VoltageEventPtr = LE_IPS_VOLTAGE_WARNING;
            }
            break;

            // low critical voltage
            case 3:
            {
                *VoltageEventPtr = LE_IPS_VOLTAGE_CRITICAL;
            }
            break;

            // hi critical voltage
            case 4:
            {
                *VoltageEventPtr = LE_IPS_VOLTAGE_HI_CRITICAL;
            }
            break;

            // Unmanaged event
            default:
            {
                LE_ERROR("Unknown Input Voltage event!!");
                le_mem_Release(VoltageEventPtr);
                return;
            }
        }

        le_event_ReportWithRefCounting(VoltageThresholdEventId, VoltageEventPtr);
    }
}


//--------------------------------------------------------------------------------------------------
//                                       Public declarations
//--------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------
/**
 * Get the Platform input voltage in [mV].
 *
 * @return
 *      - LE_OK            The function succeeded.
 *      - LE_FAULT         The function failed to get the value.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_ips_GetInputVoltage
(
    uint32_t* inputVoltagePtr
        ///< [OUT]
        ///< [OUT] The input voltage in [mV]
)
{
    le_result_t result;
    VoltageEnvironment_t voltageEnv = { 0 };

    result = GetVoltageEnvironement(&voltageEnv);

    if (result == LE_OK)
    {
        *inputVoltagePtr = voltageEnv.voltage;
        return LE_OK;
    }

    return LE_FAULT;
}


//--------------------------------------------------------------------------------------------------
/**
 *
 * This function is used to add an input voltage status notification handler
 *
 * @return A handler reference, which is only needed for later removal of the handler.
 */
//--------------------------------------------------------------------------------------------------
le_event_HandlerRef_t* pa_ips_AddVoltageEventHandler
(
    pa_ips_ThresholdInd_HandlerFunc_t   msgHandler
)
{
    le_event_HandlerRef_t  handlerRef = NULL;

    if ( msgHandler != NULL )
    {
        handlerRef = le_event_AddHandler( "VoltThresholdStatushandler",
                             VoltageThresholdEventId,
                            (le_event_HandlerFunc_t) msgHandler);
    }
    else
    {
        LE_ERROR("Null handler given in parameter");
    }

    return (le_event_HandlerRef_t*) handlerRef;
}


//--------------------------------------------------------------------------------------------------
/**
 * Set the Platform warning and critical input voltage thresholds in [mV].
 *  When thresholds input voltage are reached, a input voltage event is triggered.
 *
 * @return
 *      - LE_OK            The function succeeded.
 *      - LE_FAULT         The function failed to set the thresholds.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_SetVoltageThresholds
(
    uint16_t criticalVolt,
        ///< [IN]
        ///< [IN] The critical input voltage threshold in [mV].

    uint16_t warningVolt,
        ///< [IN]
        ///< [IN] The warning input voltage threshold in [mV].

    uint16_t normalVolt,
        ///< [IN]
        ///< [IN] The normal input voltage threshold in [mV].

    uint16_t hiCriticalVolt
        ///< [IN]
        ///< [IN] The high critical input voltage threshold in [mV].
)
{
    VoltageEnvironment_t voltageEnv = { 0 };

    voltageEnv.hi_critical = hiCriticalVolt;
    voltageEnv.hi_normal = hiCriticalVolt-1;
    voltageEnv.lo_normal = normalVolt;
    voltageEnv.lo_warning = warningVolt;
    voltageEnv.lo_critical = criticalVolt;
    return SetVoltageEnvironement(&voltageEnv);
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the Platform warning and critical input voltage thresholds in [mV].
 *
 * @return
 *      - LE_OK            The function succeeded.
 *      - LE_FAULT         The function failed to get the thresholds.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_GetVoltageThresholds
(
    uint16_t* criticalVoltPtr,
        ///< [OUT]
        ///< [OUT] The critical input voltage threshold in [mV].

    uint16_t* warningVoltPtr,
        ///< [OUT]
        ///< [OUT] The warning input voltage threshold in [mV].

    uint16_t* normalVoltPtr,
        ///< [OUT]
        ///< [OUT] The normal input voltage threshold in [mV].

    uint16_t* hiCriticalVoltPtr
        ///< [OUT]
        ///< [IN] The high critical input voltage threshold in [mV].
)
{
    le_result_t result;
    VoltageEnvironment_t voltageEnv = { 0 };

    result = GetVoltageEnvironement(&voltageEnv);

    if (result == LE_OK)
    {
        *hiCriticalVoltPtr = voltageEnv.hi_critical;
        *normalVoltPtr = voltageEnv.lo_normal;
        *warningVoltPtr = voltageEnv.lo_warning;
        *criticalVoltPtr = voltageEnv.lo_critical;
    }
    else
    {
        result = LE_FAULT;
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the Platform power source.
 *
 * @return
 *      - LE_OK             The function succeeded.
 *      - LE_BAD_PARAMETER  Null pointer provided as a parameter.
 *      - LE_FAULT          The function failed to get the value.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_ips_GetPowerSource
(
    le_ips_PowerSource_t* powerSourcePtr    ///< [OUT] The power source.
)
{
    qmi_client_error_type clientMsgErr;

    if (!powerSourcePtr)
    {
        LE_ERROR("powerSourcePtr is NULL!");
        return LE_BAD_PARAMETER;
    }

    SWIQMI_DECLARE_MESSAGE(dms_get_power_state_req_msg_v01, powerStateReq);
    SWIQMI_DECLARE_MESSAGE(dms_get_power_state_resp_msg_v01, powerStateResp);

    clientMsgErr = qmi_client_send_msg_sync(DmsClient,
                                            QMI_DMS_GET_POWER_STATE_REQ_V01,
                                            &powerStateReq, sizeof(powerStateReq),
                                            &powerStateResp, sizeof(powerStateResp),
                                            COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (LE_OK != swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_DMS_GET_POWER_STATE_REQ_V01),
                                      clientMsgErr,
                                      powerStateResp.resp.result,
                                      powerStateResp.resp.error))
    {
        return LE_FAULT;
    }

    LE_DEBUG("Power state response: power_status=%d", powerStateResp.power_state.power_status);

    // Bit 0, Power source: 0 = Powered by battery, 1 = Powered by external source
    if (powerStateResp.power_state.power_status & QMI_DMS_MASK_POWER_SOURCE_V01)
    {
        *powerSourcePtr = LE_IPS_POWER_SOURCE_EXTERNAL;
    }
    else
    {
        *powerSourcePtr = LE_IPS_POWER_SOURCE_BATTERY;
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the Platform battery level in percent.
 *
 * @return
 *      - LE_OK             The function succeeded.
 *      - LE_BAD_PARAMETER  Null pointer provided as a parameter.
 *      - LE_FAULT          The function failed to get the value.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_ips_GetBatteryLevel
(
    uint8_t* batteryLevelPtr    ///< [OUT] The battery level in percent.
)
{
    qmi_client_error_type clientMsgErr;

    if (!batteryLevelPtr)
    {
        LE_ERROR("batteryLevelPtr is NULL!");
        return LE_BAD_PARAMETER;
    }

    SWIQMI_DECLARE_MESSAGE(dms_get_power_state_req_msg_v01, powerStateReq);
    SWIQMI_DECLARE_MESSAGE(dms_get_power_state_resp_msg_v01, powerStateResp);

    clientMsgErr = qmi_client_send_msg_sync(DmsClient,
                                            QMI_DMS_GET_POWER_STATE_REQ_V01,
                                            &powerStateReq, sizeof(powerStateReq),
                                            &powerStateResp, sizeof(powerStateResp),
                                            COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (LE_OK != swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_DMS_GET_POWER_STATE_REQ_V01),
                                      clientMsgErr,
                                      powerStateResp.resp.result,
                                      powerStateResp.resp.error))
    {
        return LE_FAULT;
    }

    LE_DEBUG("Power state response: battery_lvl=%d", powerStateResp.power_state.battery_lvl);

    *batteryLevelPtr = powerStateResp.power_state.battery_lvl;
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to initialize the PA Input Power Supply module.
 *
 * @return
 *   - LE_FAULT         The function failed to initialize the PA Input Power Supply module.
 *   - LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_ips_Init
(
    void
)
{
    // Create the event for signaling user handlers.
    VoltageThresholdEventId = le_event_CreateIdWithRefCounting("VoltageStatusEvent");

    VoltageThresholdEventPool = le_mem_CreatePool("VoltageThresholdEventPool",
        sizeof(le_ips_ThresholdStatus_t));

    if (swiQmi_InitServices(QMI_SERVICE_DMS) != LE_OK)
    {
        LE_CRIT("QMI_SERVICE_DMS cannot be initialized.");
        return LE_FAULT;
    }

    // Get the QMI client service handle for later use.
    if ( (DmsClient = swiQmi_GetClientHandle(QMI_SERVICE_DMS)) == NULL)
    {
        LE_ERROR("pa_ips_qmi Failed");
        return LE_FAULT;
    }

    // Register our own handler with the QMI DMS service.
    swiQmi_AddIndicationHandler(NewMsgIPSHandler, QMI_SERVICE_DMS,
        QMI_DMS_SWI_EVENT_REPORT_IND_V01, NULL);

    dms_swi_set_event_report_req_msg_v01 snReq = {0};
    dms_swi_set_event_report_resp_msg_v01 snResp = { {0} };

    snReq.voltage_valid = true;
    snReq.voltage = true;

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync( DmsClient,
        QMI_DMS_SWI_SET_EVENT_REPORT_REQ_V01,
        &snReq, sizeof(snReq),
        &snResp, sizeof(snResp),
        COMM_DEFAULT_PLATFORM_TIMEOUT);

    le_result_t result = swiQmi_CheckResponseCode(
        STRINGIZE_EXPAND(QMI_DMS_SWI_SET_EVENT_REPORT_REQ_V01),
        clientMsgErr,
        snResp.resp );

    if ( result != LE_OK)
    {
        LE_ERROR("Failed to Set Input Voltage Event report err %d",
            clientMsgErr);
        return LE_FAULT;
    }

    LE_INFO("pa_ips_qmi init done");
    return LE_OK;
}
