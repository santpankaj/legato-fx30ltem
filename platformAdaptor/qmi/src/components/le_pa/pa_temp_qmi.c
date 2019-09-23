/**
 * @file pa_temp_qmi.c
 *
 * QMI implementation of Temperature API.
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#include "legato.h"
#include "interfaces.h"
#include "pa_temp.h"
#include "swiQmi.h"



//--------------------------------------------------------------------------------------------------
// Symbol and Enum definitions.
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Define the sensor names.
 */
//--------------------------------------------------------------------------------------------------
#define PC_SENSOR                 "POWER_CONTROLLER"
#define PA_SENSOR                 "POWER_AMPLIFIER"

//--------------------------------------------------------------------------------------------------
/**
 * Define the threshold names.
 */
//--------------------------------------------------------------------------------------------------
#define HI_CRITICAL_THRESHOLD     "HI_CRITICAL_THRESHOLD"
#define HI_NORMAL_THRESHOLD       "HI_NORMAL_THRESHOLD"
#define LO_NORMAL_THRESHOLD       "LO_NORMAL_THRESHOLD"
#define LO_CRITICAL_THRESHOLD     "LO_CRITICAL_THRESHOLD"

//--------------------------------------------------------------------------------------------------
/**
 * Define the event names.
 */
//--------------------------------------------------------------------------------------------------
#define HI_CRITICAL_EVENT         "HI_CRITICAL_EVENT"
#define HI_WARNING_EVENT          "HI_WARNING_EVENT"
#define NORMAL_EVENT              "NORMAL_EVENT"
#define LO_WARNING_EVENT          "LO_WARNING_EVENT"
#define LO_CRITICAL_EVENT         "LO_CRITICAL_EVENT"

//--------------------------------------------------------------------------------------------------
/**
 * Define an identifier for the sensors.
 */
//--------------------------------------------------------------------------------------------------
typedef enum {
    PC_SENSOR_ID,
    PA_SENSOR_ID,
    SENSOR_ID_MAX
} SensorId_t;

//--------------------------------------------------------------------------------------------------
// Data structures.
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Temperature threshold setting structure.
 */
//--------------------------------------------------------------------------------------------------
typedef struct {
    int16_t temperature;    ///<  Temperature in degrees Celsius.
    bool hiCriticalValid;   ///<  true if High Critical threshold is set.
    int16_t hiCritical;     ///<  High Critical threshold.
    bool hiNormalValid;     ///<  true if High Normal threshold is set.
    int16_t hiNormal;       ///<  High Normal threshold.
    bool loNormalValid;     ///<  true if Low Normal threshold is set.
    int16_t loNormal;       ///<  Low Normal threshold.
    bool loCriticalValid;   ///<  true if Low Critical threshold is set.
    int16_t loCritical;     ///<  Low Critical threshold.
} SensorEnvironment_t;

//--------------------------------------------------------------------------------------------------
/**
 * Temperature Environement structure.
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    SensorEnvironment_t    pwrCtrl;
    SensorEnvironment_t    pwrAmp;
} TemperatureEnvironement_t;

//--------------------------------------------------------------------------------------------------
/**
 * Sensor context structure.
 */
//--------------------------------------------------------------------------------------------------
typedef struct {
    le_temp_Handle_t     leHandle;
    char                 name[LE_TEMP_SENSOR_NAME_MAX_BYTES];
    char                 event[LE_TEMP_THRESHOLD_NAME_MAX_BYTES];
    SensorEnvironment_t* envPtr;
} SensorContext_t;

//--------------------------------------------------------------------------------------------------
/**
 * Temperature threshold report structure.
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    le_temp_Handle_t    leHandle;
    char                event[LE_TEMP_THRESHOLD_NAME_MAX_BYTES];
} ThresholdEventReport_t;


//--------------------------------------------------------------------------------------------------
// Public declarations
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Temperature environment.
 */
//--------------------------------------------------------------------------------------------------
static TemperatureEnvironement_t TempEnv;

//--------------------------------------------------------------------------------------------------
/**
 * The QMI clients. Must be obtained by calling swiQmi_GetClientHandle() before it can be used.
 */
//--------------------------------------------------------------------------------------------------
static qmi_client_type DmsClient;

//--------------------------------------------------------------------------------------------------
/**
 * Temperature status threshold Event Id
 */
//--------------------------------------------------------------------------------------------------
static le_event_Id_t TemperatureThresholdEventId;

//--------------------------------------------------------------------------------------------------
/**
 * Pool for Temperature threshold Event reporting.
 */
//--------------------------------------------------------------------------------------------------
static le_mem_PoolRef_t ThresholdReportPool;

//--------------------------------------------------------------------------------------------------
/**
 * The 2 sensor contexts
 */
//--------------------------------------------------------------------------------------------------
static SensorContext_t SensorCtx[SENSOR_ID_MAX];

//--------------------------------------------------------------------------------------------------
/**
 * Get Current temperature and Thresholds setting.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t GetPlatformEnvironement
(
    bool withThresholds
)
{
    dms_swi_get_environment_resp_msg_v01 snResp = { {0} };

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync( DmsClient,
                    QMI_DMS_SWI_GET_ENVIRONMENT_REQ_V01,
                    NULL, 0,
                    &snResp, sizeof(snResp),
                    COMM_DEFAULT_PLATFORM_TIMEOUT);

    if ( swiQmi_CheckResponseCode(STRINGIZE_EXPAND(QMI_DMS_SWI_GET_ENVIRONMENT_REQ_V01),
                                  clientMsgErr,
                                  snResp.resp ) == LE_OK )
    {
        if ( snResp.DMS_SWI_ENVIRONMENT(get, temperature_environment_valid) &&
             snResp.DMS_SWI_ENVIRONMENT(get, patemperature_environment_valid) )
        {
            TempEnv.pwrAmp.temperature =
                snResp.DMS_SWI_ENVIRONMENT(get, patemperature_environment).temperature;
            TempEnv.pwrCtrl.temperature =
                snResp.DMS_SWI_ENVIRONMENT(get, temperature_environment).temperature;

            LE_DEBUG("Get Power Amp temp.%d", TempEnv.pwrAmp.temperature);
            LE_DEBUG("Get Power Ctrl temp.%d", TempEnv.pwrCtrl.temperature);

            if (withThresholds)
            {
                // I update only unset user values

                // Power Controller temperature
                if(!TempEnv.pwrAmp.hiCriticalValid)
                {
                    TempEnv.pwrAmp.hiCritical =
                        snResp.DMS_SWI_ENVIRONMENT(get, patemperature_environment).hi_critical;
                }
                if(!TempEnv.pwrAmp.hiNormalValid)
                {
                    TempEnv.pwrAmp.hiNormal =
                        snResp.DMS_SWI_ENVIRONMENT(get, patemperature_environment).hi_normal;
                }

                // Platform temperature
                if(!TempEnv.pwrCtrl.hiCriticalValid)
                {
                    TempEnv.pwrCtrl.hiCritical =
                        snResp.DMS_SWI_ENVIRONMENT(get, temperature_environment).hi_critical;
                }
                if(!TempEnv.pwrCtrl.hiNormalValid)
                {
                    TempEnv.pwrCtrl.hiNormal =
                        snResp.DMS_SWI_ENVIRONMENT(get, temperature_environment).hi_normal;
                }
                if(!TempEnv.pwrCtrl.loNormalValid)
                {
                    TempEnv.pwrCtrl.loNormal =
                        snResp.DMS_SWI_ENVIRONMENT(get, temperature_environment).lo_normal;
                }
                if(!TempEnv.pwrCtrl.loCriticalValid)
                {
                    TempEnv.pwrCtrl.loCritical =
                        snResp.DMS_SWI_ENVIRONMENT(get, temperature_environment).lo_critical;
                }

                LE_DEBUG("Get Power Amp hi_nor.%d, hi_war.%d, hi_crit.%d",
                        TempEnv.pwrAmp.hiNormal,
                        snResp.DMS_SWI_ENVIRONMENT(get, patemperature_environment).hi_warning,
                        TempEnv.pwrAmp.hiCritical);

                LE_DEBUG("Get Power Ctrl lo_crit.%d, lo_nor.%d, hi_nor.%d, hi_war.%d, hi_crit.%d",
                        TempEnv.pwrCtrl.loCritical,
                        TempEnv.pwrCtrl.loNormal,
                        TempEnv.pwrCtrl.hiNormal,
                        snResp.DMS_SWI_ENVIRONMENT(get, temperature_environment).hi_warning,
                        TempEnv.pwrCtrl.hiCritical);
            }

            return LE_OK;
        }
        else
        {
            LE_ERROR("Failed to Get Temperature (valid.'%c')",
                (snResp.DMS_SWI_ENVIRONMENT(get, patemperature_environment_valid) ? 'Y' : 'N'));
            return LE_FAULT;
        }
    }
    else
    {
        LE_ERROR("Failed to Get Temperature!");
        return LE_FAULT;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * The first-layer temperature Handler.
 *
 */
//--------------------------------------------------------------------------------------------------
static void FirstLayerTemperatureHandler
(
    void* reportPtr,
    void* secondLayerHandlerFunc
)
{
    ThresholdEventReport_t*         tempReportPtr = reportPtr;
    pa_temp_ThresholdHandlerFunc_t  clientHandlerFunc = secondLayerHandlerFunc;

    clientHandlerFunc((le_temp_Handle_t)tempReportPtr->leHandle,
                      tempReportPtr->event,
                      le_event_GetContextPtr());

    // The reportPtr is a reference counted object, so I must release it
    le_mem_Release(reportPtr);
}

//--------------------------------------------------------------------------------------------------
/**
 * Temperature handler function called by the DMS QMI service indications.
 */
//--------------------------------------------------------------------------------------------------
static void TemperatureHandler
(
    void*           indBufPtr,  // [IN] The indication message data.
    unsigned int    indBufLen,  // [IN] The indication message data length in bytes.
    void*           contextPtr  // [IN] Context value that was passed to
                                //      swiQmi_AddIndicationHandler().
)
{
    LE_DEBUG("Temperature Threshold event");

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

    if (msgEventInd.patemperature_status_valid)
    {
        ThresholdEventReport_t* tempEventPtr = le_mem_ForceAlloc(ThresholdReportPool);

        // Value defined in the DMS header file.
        // 0 unknown
        // 1 normal
        // 2 high (warning)
        // 3 high (critical)

        LE_DEBUG("Power Amp Temp status event %d", msgEventInd.patemperature_status.state);

        tempEventPtr->leHandle = SensorCtx[PA_SENSOR_ID].leHandle;

        switch(msgEventInd.patemperature_status.state)
        {
            case 1: // normal
            {
                strncpy(tempEventPtr->event, NORMAL_EVENT, LE_TEMP_THRESHOLD_NAME_MAX_LEN);
            }
            break;

            case 2: // high (warning)
            {
                strncpy(tempEventPtr->event, HI_WARNING_EVENT, LE_TEMP_THRESHOLD_NAME_MAX_LEN);
            }
            break;

            case 3: // high (critical)
            {
                strncpy(tempEventPtr->event, HI_CRITICAL_EVENT, LE_TEMP_THRESHOLD_NAME_MAX_LEN);
            }
            break;

            default:
            {
                LE_ERROR("Unknow Power Amplifier event!!");
            }
            break;
        }
        le_event_ReportWithRefCounting(TemperatureThresholdEventId, tempEventPtr);
    }

    if (msgEventInd.temperature_status_valid)
    {
        ThresholdEventReport_t* tempEventPtr = le_mem_ForceAlloc(ThresholdReportPool);

        // Value defined in the DMS header file.
        // 0 unknown
        // 1 normal
        // 2 high (warning)
        // 3 high (critical)
        // 4 low (warning)
        // 5 low (critical)

        LE_DEBUG("Power Ctrl Temp status event %d", msgEventInd.temperature_status.state);

        tempEventPtr->leHandle = SensorCtx[PC_SENSOR_ID].leHandle;

        switch(msgEventInd.temperature_status.state)
        {
            case 5: // low (critical)
            {
                strncpy(tempEventPtr->event, LO_CRITICAL_EVENT, LE_TEMP_THRESHOLD_NAME_MAX_LEN);
            }
            break;

            case 4: // low (warning)
            {
                strncpy(tempEventPtr->event, LO_WARNING_EVENT, LE_TEMP_THRESHOLD_NAME_MAX_LEN);
            }
            break;

            case 1: // normal
            {
                strncpy(tempEventPtr->event, NORMAL_EVENT, LE_TEMP_THRESHOLD_NAME_MAX_LEN);
            }
            break;

            case 2: // high (warning)
            {
                strncpy(tempEventPtr->event, HI_WARNING_EVENT, LE_TEMP_THRESHOLD_NAME_MAX_LEN);
            }
            break;

            case 3: // high (critical)
            {
                strncpy(tempEventPtr->event, HI_CRITICAL_EVENT, LE_TEMP_THRESHOLD_NAME_MAX_LEN);
            }
            break;

            case 0:
            default:
            {
                LE_ERROR("Unknown Power Controller event!!");
            }
            break;
        }
        le_event_ReportWithRefCounting(TemperatureThresholdEventId, tempEventPtr);
    }
}


//--------------------------------------------------------------------------------------------------
// Public declarations
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Request a new handle for a temperature sensor
 *
 * @return
 *      - LE_OK            The function succeeded.
 *      - LE_FAULT         The function failed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_temp_Request
(
    const char*         sensorPtr,  ///< [IN] Name of the temperature sensor.
    le_temp_Handle_t    leHandle,   ///< [IN] Sensor context in LE side, opaque type in PA side
    pa_temp_Handle_t*   paHandlePtr ///< [OUT] Sensor context in PA side, opaque type in LE side
)
{
    if (strncmp(PC_SENSOR, sensorPtr, strlen(PC_SENSOR)) == 0)
    {
        strncpy(SensorCtx[PC_SENSOR_ID].name, sensorPtr, LE_TEMP_SENSOR_NAME_MAX_BYTES);
        SensorCtx[PC_SENSOR_ID].envPtr = &TempEnv.pwrCtrl;
        SensorCtx[PC_SENSOR_ID].leHandle = leHandle;
        *paHandlePtr = (pa_temp_Handle_t)&SensorCtx[PC_SENSOR_ID];
        return LE_OK;
    }
    else if (strncmp(PA_SENSOR, sensorPtr, strlen(PA_SENSOR)) == 0)
    {
        strncpy(SensorCtx[PA_SENSOR_ID].name, sensorPtr, LE_TEMP_SENSOR_NAME_MAX_BYTES);
        SensorCtx[PA_SENSOR_ID].envPtr = &TempEnv.pwrAmp;
        SensorCtx[PA_SENSOR_ID].leHandle = leHandle;
        *paHandlePtr = (pa_temp_Handle_t)&SensorCtx[PA_SENSOR_ID];
        return LE_OK;
    }
    else
    {
        LE_ERROR("This sensor (%s) is not supported!", sensorPtr);
        return LE_FAULT;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the handle of a temperature sensor
 *
 * @return
 *      - LE_OK            The function succeeded.
 *      - LE_FAULT         The function failed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_temp_GetHandle
(
    const char*         sensorPtr,  ///< [IN] Name of the temperature sensor.
    le_temp_Handle_t*   leHandlePtr ///< [OUT] Sensor context in LE side, opaque type in PA side
)
{
    if (strncmp(PC_SENSOR, sensorPtr, strlen(PC_SENSOR)) == 0)
    {
        *leHandlePtr=SensorCtx[PC_SENSOR_ID].leHandle;
        return LE_OK;
    }
    else if (strncmp(PA_SENSOR, sensorPtr, strlen(PA_SENSOR)) == 0)
    {
        *leHandlePtr=SensorCtx[PA_SENSOR_ID].leHandle;
        return LE_OK;
    }
    else
    {
        LE_ERROR("This sensor (%s) is not supported!", sensorPtr);
        return LE_FAULT;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Retrieve the temperature sensor's name from its handle.
 *
 * @return
 *      - LE_OK            The function succeeded.
 *      - LE_OVERFLOW      The name length exceed the maximum length.
 *      - LE_FAULT         The function failed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_temp_GetSensorName
(
    pa_temp_Handle_t    paHandle, ///< [IN] Handle of the temperature sensor.
    char*               namePtr,  ///< [OUT] Name of the temperature sensor.
    size_t              len       ///< [IN] The maximum length of the sensor name.
)
{
    SensorContext_t* sensorCtxPtr = (SensorContext_t*)paHandle;

    if (namePtr == NULL)
    {
        LE_ERROR("namePtr is NULL !");
        return LE_FAULT;
    }

    if (strlen(sensorCtxPtr->name) > (len - 1))
    {
        return LE_OVERFLOW;
    }
    else
    {
        strncpy(namePtr, sensorCtxPtr->name, len);
        return LE_OK;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the temperature in degree Celsius.
 *
 * @return
 *      - LE_OK            The function succeeded.
 *      - LE_FAULT         The function failed to get the temperature.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_temp_GetTemperature
(
    pa_temp_Handle_t    paHandle,      ///< [IN] Handle of the temperature sensor.
    int32_t*            temperaturePtr ///< [OUT] Temperature in degree Celsius.
)
{
    SensorContext_t* sensorCtxPtr = (SensorContext_t*)paHandle;

    if (GetPlatformEnvironement(false) == LE_OK)
    {
        *temperaturePtr = sensorCtxPtr->envPtr->temperature;
        return LE_OK;
    }
    else
    {
        LE_ERROR("Failed to get the temperature!");
        return LE_FAULT;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the Power Controller warning and critical temperature thresholds in degree Celsius.
 *  When thresholds temperature are reached, a temperature event is triggered.
 *
 * @return
 *      - LE_OK            The function succeeded.
 *      - LE_FAULT         The function failed to set the thresholds.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_temp_SetThreshold
(
    pa_temp_Handle_t    paHandle,      ///< [IN] Handle of the temperature sensor.
    const char*         thresholdPtr,  ///< [IN] Name of the threshold.
    int32_t             temperature    ///< [IN] Temperature threshold in degree Celsius.
)
{
    SensorContext_t* sensorCtxPtr = (SensorContext_t*)paHandle;
    le_result_t res = LE_OK;

    if (strncmp(HI_CRITICAL_THRESHOLD, thresholdPtr, strlen(HI_CRITICAL_THRESHOLD)) == 0)
    {
        sensorCtxPtr->envPtr->hiCriticalValid = true;
        sensorCtxPtr->envPtr->hiCritical = temperature;
    }
    else if (strncmp(HI_NORMAL_THRESHOLD, thresholdPtr, strlen(HI_NORMAL_THRESHOLD)) == 0)
    {
        sensorCtxPtr->envPtr->hiNormalValid = true;
        sensorCtxPtr->envPtr->hiNormal = temperature;
    }
    else if (strncmp(LO_NORMAL_THRESHOLD, thresholdPtr, strlen(LO_NORMAL_THRESHOLD)) == 0)
    {
        sensorCtxPtr->envPtr->loNormalValid = true;
        sensorCtxPtr->envPtr->loNormal = temperature;
    }
    else if (strncmp(LO_CRITICAL_THRESHOLD, thresholdPtr, strlen(LO_CRITICAL_THRESHOLD)) == 0)
    {
        sensorCtxPtr->envPtr->loCriticalValid = true;
        sensorCtxPtr->envPtr->loCritical = temperature;
    }
    else
    {
        LE_ERROR("%s threshold is not found for %s sensor!", thresholdPtr, sensorCtxPtr->name);
        res = LE_NOT_FOUND;
    }

    return res;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the Power Controller warning and critical temperature thresholds in degree Celsius.
 *
 * @return
 *      - LE_OK            The function succeeded.
 *      - LE_FAULT         The function failed to get the thresholds.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_temp_GetThreshold
(
    pa_temp_Handle_t    paHandle,      ///< [IN] Handle of the temperature sensor.
    const char*         thresholdPtr,  ///< [IN] Name of the threshold.
    int32_t*            temperaturePtr ///< [OUT] Temperature in degree Celsius.
)
{
    SensorContext_t* sensorCtxPtr = (SensorContext_t*)paHandle;
    le_result_t        res = LE_OK;

    if (GetPlatformEnvironement(true) == LE_OK)
    {
        if (strncmp(HI_CRITICAL_THRESHOLD, thresholdPtr, strlen(HI_CRITICAL_THRESHOLD)) == 0)
        {
            *temperaturePtr = sensorCtxPtr->envPtr->hiCritical;
        }
        else if (strncmp(HI_NORMAL_THRESHOLD, thresholdPtr, strlen(HI_NORMAL_THRESHOLD)) == 0)
        {
            *temperaturePtr = sensorCtxPtr->envPtr->hiNormal;
        }
        else if (strncmp(LO_NORMAL_THRESHOLD, thresholdPtr, strlen(LO_NORMAL_THRESHOLD)) == 0)
        {
            *temperaturePtr = sensorCtxPtr->envPtr->loNormal;
        }
        else if (strncmp(LO_CRITICAL_THRESHOLD, thresholdPtr, strlen(LO_CRITICAL_THRESHOLD)) == 0)
        {
            *temperaturePtr = sensorCtxPtr->envPtr->loCritical;
        }
        else
        {
            LE_ERROR("%s threshold is not found for %s sensor!", thresholdPtr, sensorCtxPtr->name);
            res = LE_NOT_FOUND;
        }
    }
    else
    {
        LE_ERROR("Failed to get the thresholds!");
        res = LE_FAULT;
    }

    return res;
}

//--------------------------------------------------------------------------------------------------
/**
 * Start the temperature monitoring with the temperature thresholds configured by
 * le_temp_SetThreshold() function.
 *
 * @return
 *      - LE_OK            The function succeeded.
 *      - LE_FAULT         The function failed to apply the thresholds.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_temp_StartMonitoring
(
    void
)
{
    qmi_client_error_type rc;

    if (GetPlatformEnvironement(true) == LE_OK)
    {
        dms_swi_set_environment_req_msg_v01  sndReq = { 0 };
        dms_swi_set_environment_resp_msg_v01 rcvResp = { {0} };

        sndReq.DMS_SWI_ENVIRONMENT(set, temperature_environment_valid) = true;
        sndReq.DMS_SWI_ENVIRONMENT(set, temperature_environment).hi_critical =
                                                                TempEnv.pwrCtrl.hiCritical;
        sndReq.DMS_SWI_ENVIRONMENT(set, temperature_environment).hi_warning =
                                                                TempEnv.pwrCtrl.hiCritical-1;
        sndReq.DMS_SWI_ENVIRONMENT(set, temperature_environment).hi_normal =
                                                                TempEnv.pwrCtrl.hiNormal;
        sndReq.DMS_SWI_ENVIRONMENT(set, temperature_environment).lo_normal =
                                                                TempEnv.pwrCtrl.loNormal;
        sndReq.DMS_SWI_ENVIRONMENT(set, temperature_environment).lo_critical =
                                                                TempEnv.pwrCtrl.loCritical;

        LE_DEBUG("Set Power Ctrl Temp thresholds lo_cri %d, lo_nor %d,"
                 "hi_nor %d, hi_war %d, hi_crit %d",
        sndReq.DMS_SWI_ENVIRONMENT(set, temperature_environment).lo_critical,
        sndReq.DMS_SWI_ENVIRONMENT(set, temperature_environment).lo_normal,
        sndReq.DMS_SWI_ENVIRONMENT(set, temperature_environment).hi_normal,
        sndReq.DMS_SWI_ENVIRONMENT(set, temperature_environment).hi_warning,
        sndReq.DMS_SWI_ENVIRONMENT(set, temperature_environment).hi_critical);

        sndReq.DMS_SWI_ENVIRONMENT(set, patemperature_environment_valid) = true;
        sndReq.DMS_SWI_ENVIRONMENT(set, patemperature_environment).hi_critical =
                                                                  TempEnv.pwrAmp.hiCritical;
        sndReq.DMS_SWI_ENVIRONMENT(set, patemperature_environment).hi_warning =
                                                                  TempEnv.pwrAmp.hiCritical-1;
        sndReq.DMS_SWI_ENVIRONMENT(set, patemperature_environment).hi_normal =
                                                                  TempEnv.pwrAmp.hiNormal;

        LE_DEBUG("Set Power Amp Temp thresholds hi_nor %d, hi_war %d, hi_crit %d",
                    sndReq.DMS_SWI_ENVIRONMENT(set, patemperature_environment).hi_normal,
                    sndReq.DMS_SWI_ENVIRONMENT(set, patemperature_environment).hi_warning,
                    sndReq.DMS_SWI_ENVIRONMENT(set, patemperature_environment).hi_critical);

        rc = qmi_client_send_msg_sync(DmsClient,
                                      QMI_DMS_SWI_SET_ENVIRONMENT_REQ_V01,
                                      &sndReq, sizeof(sndReq),
                                      &rcvResp, sizeof(rcvResp),
                                      COMM_DEFAULT_PLATFORM_TIMEOUT);

        if (swiQmi_CheckResponseCode(
                        STRINGIZE_EXPAND(QMI_DMS_SWI_SET_ENVIRONMENT_REQ_V01),
                        rc,
                        rcvResp.resp ) != LE_OK)
        {
            LE_ERROR("Failed to Set Temperature thresholds, rc %d, resp %d",
                     rc,
                     rcvResp.resp.error);
            return LE_FAULT;
        }
        else
        {
            dms_swi_set_event_report_req_msg_v01 req = {0};
            dms_swi_set_event_report_resp_msg_v01 resp = { {0} };

            req.patemp_report_valid = true;
            req.patemp_report = true;
            req.temperature_valid = true;
            req.temperature = true;

            rc = qmi_client_send_msg_sync(DmsClient,
                                          QMI_DMS_SWI_SET_EVENT_REPORT_REQ_V01,
                                          &req, sizeof(req),
                                          &resp, sizeof(resp),
                                          COMM_DEFAULT_PLATFORM_TIMEOUT);

            if (swiQmi_CheckResponseCode(STRINGIZE_EXPAND(QMI_DMS_SWI_SET_EVENT_REPORT_REQ_V01),
                                         rc,
                                         resp.resp ) != LE_OK)
            {
                LE_ERROR("Failed to Set Temperature Threshold Event report (err.%d)",
                         rc);
                return LE_FAULT;
            }
            else
            {
                return LE_OK;
            }
        }
    }
    else
    {
        LE_ERROR("Failed to Get Temperature thresholds !");
        return LE_FAULT;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * This function is used to add a temperature status notification handler
 *
 * @return A handler reference, which is only needed for later removal of the handler.
 */
//--------------------------------------------------------------------------------------------------
le_event_HandlerRef_t* pa_temp_AddTempEventHandler
(
    pa_temp_ThresholdHandlerFunc_t  handlerFunc, ///< [IN] The handler function.
    void*                           contextPtr   ///< [IN] The context to be given to the handler.
)
{
    le_event_HandlerRef_t  handlerRef = NULL;

    if (handlerFunc != NULL)
    {
        handlerRef = le_event_AddLayeredHandler("ThresholdStatushandler",
                                                TemperatureThresholdEventId,
                                                FirstLayerTemperatureHandler,
                                                (le_event_HandlerFunc_t)handlerFunc);
        le_event_SetContextPtr (handlerRef, contextPtr);
    }
    else
    {
        LE_ERROR("Null handler given in parameter");
    }

    return (le_event_HandlerRef_t*) handlerRef;
}

//--------------------------------------------------------------------------------------------------
/**
 * Reset the temperature sensor handle.
 *
 * @return
 *      - LE_OK            The function succeeded.
 *      - LE_FAULT         The function failed.
 *      - LE_UNSUPPORTED   The function does not support this operation.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_temp_ResetHandle
(
    const char* sensorPtr  ///< [IN] Name of the temperature sensor.
)
{
    if (0 == strncmp(PC_SENSOR, sensorPtr, strlen(PC_SENSOR)))
    {
        SensorCtx[PC_SENSOR_ID].leHandle = NULL;
        return LE_OK;
    }
    if (0 == strncmp(PA_SENSOR, sensorPtr, strlen(PA_SENSOR)))
    {
        SensorCtx[PA_SENSOR_ID].leHandle = NULL;
        return LE_OK;
    }

    LE_ERROR("This sensor (%s) is not supported!", sensorPtr);

    return LE_FAULT;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function is used to initialize the PA Temperature
 *
 * @return
 * - LE_OK if successful.
 * - LE_FAULT if unsuccessful.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_temp_Init
(
    void
)
{
    // Create the event for signaling user handlers.
    TemperatureThresholdEventId = le_event_CreateIdWithRefCounting("TempThresholdsEvent");

    ThresholdReportPool = le_mem_CreatePool("ThresholdReportPool",
                                            sizeof(ThresholdEventReport_t));

    TempEnv.pwrAmp.hiCriticalValid = false;
    TempEnv.pwrAmp.hiNormalValid = false;

    TempEnv.pwrCtrl.hiCriticalValid = false;
    TempEnv.pwrCtrl.hiNormalValid = false;
    TempEnv.pwrCtrl.loNormalValid = false;
    TempEnv.pwrCtrl.loCriticalValid = false;

    if (swiQmi_InitServices(QMI_SERVICE_DMS) != LE_OK)
    {
        LE_CRIT("QMI_SERVICE_DMS cannot be initialized.");
        return LE_FAULT;
    }

    // Get the qmi client service handle for later use.
    if ((DmsClient = swiQmi_GetClientHandle(QMI_SERVICE_DMS)) == NULL)
    {
        return LE_FAULT;
    }

    // Register our own handler with the QMI DMS service.
    swiQmi_AddIndicationHandler(TemperatureHandler,
                                QMI_SERVICE_DMS,
                                QMI_DMS_SWI_EVENT_REPORT_IND_V01,
                                NULL);

    return LE_OK;
}
