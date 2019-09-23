/**
 * @file pa_simu.c
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#include "legato.h"

#include "interfaces.h"
#include "pa_simu.h"
#include "simuConfig.h"

#include "pa_sim_simu.h"
#include "pa_mdc_simu.h"
#include "pa_mcc_simu.h"
#include "pa_mrc_simu.h"
#include "pa_ips_simu.h"
#include "pa_sms_simu.h"
#include "pa_info_simu.h"
#include "pa_temp.h"
#include "pa_antenna.h"
#include "pa_lpt_simu.h"

//--------------------------------------------------------------------------------------------------
/**
 * Component initializer automatically called by the application framework when the process starts.
 *
 **/
//--------------------------------------------------------------------------------------------------
COMPONENT_INIT
{
    le_result_t res;

    LE_INFO("PA Init");

    /* Init sub-PAs */
    res = mrc_simu_Init();
    LE_FATAL_IF(res != LE_OK, "PA MRC Init Failed");

    res = mcc_simu_Init();
    LE_FATAL_IF(res != LE_OK, "PA MCC Init Failed");

    res = pa_simSimu_Init();
    LE_FATAL_IF(res != LE_OK, "PA SIM Init Failed");

    res = sms_simu_Init();
    LE_FATAL_IF(res != LE_OK, "PA SMS Init Failed");

    res = pa_infoSimu_Init();
    LE_FATAL_IF(res != LE_OK, "PA Info Init Failed");

    res = pa_mdcSimu_Init();
    LE_FATAL_IF(res != LE_OK, "PA MDC Init Failed");

    res = pa_temp_Init();
    LE_FATAL_IF(res != LE_OK, "PA Temperature Failed");

    res = pa_ipsSimu_Init();
    LE_FATAL_IF(res != LE_OK, "PA Input Power Supply Failed");

    res = pa_antenna_Init();
    LE_FATAL_IF(res != LE_OK, "PA Antenna Failed");

    res = pa_lptSimu_Init();
    LE_FATAL_IF(res != LE_OK, "PA LPT Failed");
}
