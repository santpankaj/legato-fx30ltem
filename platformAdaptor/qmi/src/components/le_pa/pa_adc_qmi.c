/**
 * @file pa_adc_qmi.c
 *
 * QMI implementation of ADC API.
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#include "legato.h"
#include "interfaces.h"
#include "pa_adc.h"
#include "swiQmi.h"

//--------------------------------------------------------------------------------------------------
/**
 * The QMI clients. Must be obtained by calling swiQmi_GetClientHandle() before it can be used.
 */
//--------------------------------------------------------------------------------------------------
static qmi_client_type QmiClient;

//--------------------------------------------------------------------------------------------------
/**
 * The QMI service provided by variable QmiClient.
 */
//--------------------------------------------------------------------------------------------------
static swiQmi_Service_t QmiService = QMI_SERVICE_COUNT;

//--------------------------------------------------------------------------------------------------
/**
 * Convert the ADC string name into the ADC index.
 *
 * @return
 * - LE_OK            The function succeeded.
 * - LE_FAULT         The function failed.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t GetExtAdcValue
(
    const char* adcNamePtr,
    uint8_t*   indexPtr
)
{
    // check adc name
    const char adcCommonName[]="EXT_ADC";
    if (strncmp(adcNamePtr, adcCommonName, strlen(adcCommonName)) != 0)
    {
        LE_ERROR("Bad name");
        return LE_FAULT;
    }

    adcNamePtr += strlen(adcCommonName);

    if ((*adcNamePtr < '0') || (*adcNamePtr > '9'))
    {
        LE_ERROR("Not a digit");
        return LE_FAULT;
    }

    *indexPtr = atoi(adcNamePtr);

    LE_DEBUG("read ADC %d", *indexPtr);

    return LE_OK;

}


//--------------------------------------------------------------------------------------------------
/**
 * Read the value of a given ADC channel in units appropriate to that channel.
 *
 * @return
 * - LE_OK            The function succeeded.
 * - LE_FAULT         The function failed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_adc_ReadValue
(
    const char* adcNamePtr,
        ///< [IN]
        ///< Name of the ADC to read.

    int32_t* adcValuePtr
        ///< [OUT]
        ///< The adc value
)
{
    uint8_t   adcChannel=0;

    if (GetExtAdcValue(adcNamePtr, &adcChannel) != LE_OK)
    {
        return LE_FAULT;
    }

    if ((adcChannel < ADC_BEGIN_NUM) || (adcChannel >= (ADC_BEGIN_NUM + ADC_COUNT)))
    {
        LE_CRIT("EXT_ADC%d is not supported on this platform", adcChannel);
        return LE_FAULT;
    }

#if defined(QMI_SWI_M2M_ADC_READ_REQ_V01)

    // Mapping of external ADC index to QMI input
    uint8_t                         adcInput[]={0xA, 0x5, 0x6, 0xB};
    swi_m2m_adc_read_req_msg_v01    req={ 0 };
    SWIQMI_DECLARE_MESSAGE(swi_m2m_adc_read_resp_msg_v01, resp);

    if ( adcChannel >= NUM_ARRAY_MEMBERS(adcInput) )
    {
        LE_CRIT("Maximum %d ADCs can be supported on this platform",
                NUM_ARRAY_MEMBERS(adcInput));
        return LE_FAULT;
    }

    if (QmiService != QMI_SERVICE_REALTIME)
    {
        LE_CRIT("QMI service for ADC not as expected (%d)", QmiService);
        return LE_FAULT;
    }

    req.adc_input = adcInput[adcChannel];

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(QmiClient,
                                            QMI_SWI_M2M_ADC_READ_REQ_V01,
                                            &req, sizeof(req),
                                            &resp, sizeof(resp),
                                            COMM_DEFAULT_PLATFORM_TIMEOUT);

    le_result_t respResult = swiQmi_OEMCheckResponseCode(
                                STRINGIZE_EXPAND(QMI_SWI_M2M_ADC_READ_RESP_V01),
                                clientMsgErr,
                                resp.resp);

    if (respResult == LE_OK)
    {
        if (1 == resp.adc_value_valid)
        {
            LE_DEBUG("Value read %d for ADC.%d (channel.%d)",
                     resp.adc_value,
                     adcChannel,
                     req.adc_input);
             *adcValuePtr = resp.adc_value;
        }
        else
        {
            LE_ERROR("resp.adc_value_valid not set");
        }
    }
    else
    {
        LE_ERROR("Failed to read ADC.%d (channel.%d)", adcChannel, req.adc_input);
    }

#elif defined(QMI_SWI_BSP_ADC_READ_REQ_V01)

    // Mapping of external ADC index to QMI input
    uint8_t                         adcInput[]={10, 5, 6, 11, 12};
    swi_bsp_adc_read_req_msg_v01    req = { 0 };
    SWIQMI_DECLARE_MESSAGE(swi_bsp_adc_read_resp_msg_v01, resp);


    if ( adcChannel >= NUM_ARRAY_MEMBERS(adcInput) )
    {
        LE_CRIT("Maximum %d ADCs can be supported on this platform",
                NUM_ARRAY_MEMBERS(adcInput));
        return LE_FAULT;
    }

    if (QmiService != QMI_SERVICE_SWI_BSP)
    {
        LE_CRIT("QMI service for ADC not as expected (%d)", QmiService);
        return LE_FAULT;
    }

    req.adc_input = adcInput[adcChannel];

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(QmiClient,
                                            QMI_SWI_BSP_ADC_READ_REQ_V01,
                                            &req, sizeof(req),
                                            &resp, sizeof(resp),
                                            COMM_DEFAULT_PLATFORM_TIMEOUT);

    le_result_t respResult = swiQmi_CheckResponse(
                                STRINGIZE_EXPAND(QMI_SWI_BSP_ADC_READ_REQ_V01),
                                clientMsgErr,
                                resp.resp.result,
                                resp.resp.error);

    if ((respResult == LE_OK) && resp.adc_value_valid)
    {
        *adcValuePtr = resp.adc_value;
    }
    else
    {
        LE_ERROR("Failed to read ADC.%d (channel.%d)", adcChannel, req.adc_input);
    }

#else

    LE_ERROR("No available QMI service to read ADC!");

#endif

    return respResult;
}


//--------------------------------------------------------------------------------------------------
/**
 * When we are initialized we should probably get our properties - i.e. at least the
 * names of the channels we support adc_get_input_properties_req_msg_v01; AdcInputPropertiesType_v01
 *
 **/
//--------------------------------------------------------------------------------------------------

le_result_t pa_adc_Init
(
    void
)
{
#if defined(QMI_SWI_M2M_ADC_READ_REQ_V01)
    QmiService = QMI_SERVICE_REALTIME;
#elif defined(QMI_SWI_BSP_ADC_READ_REQ_V01)
    QmiService = QMI_SERVICE_SWI_BSP;
#else
    LE_ERROR("Cannot Init ADC service!");
    return LE_FAULT;
#endif

    LE_DEBUG("pa_adc_qmi init");

    if (swiQmi_InitServices(QmiService) != LE_OK)
    {
        LE_ERROR("Cannot Init QMI client!");
        return LE_FAULT;
    }

    if ((QmiClient = swiQmi_GetClientHandle(QmiService)) == NULL)
    {
        LE_ERROR("Cannot get QMI client!");
        return LE_FAULT;
    }

    return LE_OK;
}

