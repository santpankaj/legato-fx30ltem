/**
 * @file pa_qmi.c
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#include "legato.h"
#include "swiQmi.h"
#include "sim_qmi.h"
#include "mrc_qmi.h"
#include "mdc_qmi.h"
#include "sms_qmi.h"
#include "mcc_qmi.h"
#include "info_qmi.h"
#include "pa_ips.h"
#include "pa_temp.h"
#include "pa_antenna.h"
#include "pa_riPin.h"
#include "pa_adc_local.h"
#include "pa_rtc_local.h"
#include "pa_mdmCfg.h"
#include "lpt_qmi.h"

//--------------------------------------------------------------------------------------------------
/**
 * Convert {mobileCountryCode,mobileNetworkCode} into string.
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on any other failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t ConvertMccMncIntoStringFormat
(
    uint16_t    mcc,                        ///< [IN] the mcc to encode
    uint16_t    mnc,                        ///< [IN] the mnc to encode
    bool        isMncIncPcsDigit,           ///< [IN] true if MNC is a three-digit value,
                                            ///       false if MNC is a two-digit value
    char        mccPtr[LE_MRC_MCC_BYTES],   ///< [OUT] the mcc encoded
    char        mncPtr[LE_MRC_MNC_BYTES]    ///< [OUT] the mnc encoded
)
{
    snprintf(mccPtr, LE_MRC_MCC_BYTES, "%03d", mcc % 1000);

    if (isMncIncPcsDigit)
    {
        snprintf(mncPtr, LE_MRC_MNC_BYTES, "%03d", mnc % 1000);
    }
    else
    {
        snprintf(mncPtr, LE_MRC_MNC_BYTES, "%02d", mnc % 1000);
    }

    mccPtr[LE_MRC_MCC_LEN] = '\0';
    mncPtr[LE_MRC_MNC_LEN] = '\0';

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Component initializer automatically called by the application framework when the process starts.
 *
 **/
//--------------------------------------------------------------------------------------------------
COMPONENT_INIT
{
    LE_INFO("Start the QMI PA initialization.");

    // Info module needs to be initialized first as it might be used by other modules
    LE_ERROR_IF((info_qmi_Init() != LE_OK), "Error on PA INFO initialization.");
    LE_ERROR_IF((sim_qmi_Init() != LE_OK), "Error on PA SIM initialization.");
    LE_ERROR_IF((mrc_qmi_Init() != LE_OK), "Error on PA MRC initialization.");
    LE_ERROR_IF((mdc_qmi_Init() != LE_OK), "Error on PA MDC initialization.");
    LE_ERROR_IF((sms_qmi_Init() != LE_OK), "Error on PA SMS initialization.");
    LE_ERROR_IF((mcc_qmi_Init() != LE_OK), "Error on PA MCC initialization.");
    LE_ERROR_IF((pa_ips_Init() != LE_OK), "Error on PA IPS initialization.");
    LE_ERROR_IF((pa_temp_Init() != LE_OK), "Error on PA TEMP initialization.");
    LE_ERROR_IF((pa_antenna_Init() != LE_OK), "Error on PA ANTENNA initialization.");
    LE_ERROR_IF((pa_adc_Init() != LE_OK), "Error on PA ADC initialization.");
    LE_ERROR_IF((pa_rtc_Init() != LE_OK), "Error on PA RTC initialization.");
    LE_ERROR_IF((pa_riPin_Init() != LE_OK), "Error on PA RING PIN initialization.");
    LE_ERROR_IF((pa_mdmCfg_Init() != LE_OK), "Error on PA modem configuration initialization.");
    LE_ERROR_IF((pa_lpt_Init() != LE_OK), "Error on PA LPT initialization.");

    LE_INFO("QMI PA initialization ends.");
}
