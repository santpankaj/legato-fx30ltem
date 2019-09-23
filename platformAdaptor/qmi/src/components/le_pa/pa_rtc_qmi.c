/**
 * @file pa_rtc_qmi.c
 *
 * QMI implementation of RTC API.
 *
 * Copyright (C) Sierra Wireless Inc.
 */



#include "legato.h"
#include "interfaces.h"
#include "pa_rtc.h"
#include "swiQmi.h"

//--------------------------------------------------------------------------------------------------
/**
 * The QMI clients. Must be obtained by calling swiQmi_GetClientHandle() before it can be used.
 */
//--------------------------------------------------------------------------------------------------
static qmi_client_type DmsClient;


//--------------------------------------------------------------------------------------------------
/**
 * This function sets the user time.
 *
 * @return
 * - LE_FAULT         The function failed to get the value.
 * - LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_rtc_SetUserTime
(
    uint64_t  millisecondsPastGpsEpoch  ///< [IN]
)
{
    dms_set_time_req_msg_v01 req={ 0 };
    dms_set_time_resp_msg_v01 resp = { {0} };

    LE_DEBUG("Epoch time: %llu", (unsigned long long int) millisecondsPastGpsEpoch);

    req.time_in_ms = millisecondsPastGpsEpoch;
    req.time_reference_type_valid = 0;

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(DmsClient,
                                            QMI_DMS_SET_TIME_REQ_V01,
                                            &req, sizeof(req),
                                            &resp, sizeof(resp),
                                            COMM_DEFAULT_PLATFORM_TIMEOUT);

    le_result_t respResult = swiQmi_CheckResponseCode(
                                STRINGIZE_EXPAND(QMI_DMS_SET_TIME_RESP_V01),
                                clientMsgErr,
                                resp.resp);

    if (respResult != LE_OK)
    {
        LE_ERROR("Failed to write user time to rtc");
    }

    return respResult;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function sets the user time.
 *
 * @return
 * - LE_FAULT         The function failed to get the value.
 * - LE_UNAVAILABLE   No valid user time was returned.
 * - LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_rtc_GetUserTime
(
    uint64_t*  millisecondsPastGpsEpochPtr  ///< [OUT]
)
{
    dms_get_time_req_msg_v01 req={ 0 };
    dms_get_time_resp_msg_v01 resp; // = { {0} };

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(DmsClient,
                                            QMI_DMS_GET_TIME_REQ_V01,
                                            &req, sizeof(req),
                                            &resp, sizeof(resp),
                                            COMM_DEFAULT_PLATFORM_TIMEOUT);

    le_result_t respResult = swiQmi_CheckResponseCode(
                                STRINGIZE_EXPAND(QMI_DMS_GET_TIME_RESP_V01),
                                clientMsgErr,
                                resp.resp);

    *millisecondsPastGpsEpochPtr = 0;

    if (respResult == LE_OK)
    {
        if (resp.user_time_in_ms_valid)
        {
            *millisecondsPastGpsEpochPtr = resp.user_time_in_ms;
        }
        else
        {
            respResult = LE_UNAVAILABLE;
        }
    }
    else
    {
        LE_ERROR("Failed to read user time from rtc");
    }


    return respResult;
}

//--------------------------------------------------------------------------------------------------
/**
 *  Initialize the service
 **/
//--------------------------------------------------------------------------------------------------

le_result_t pa_rtc_Init
(
    void
)
{
    LE_DEBUG("pa_rtc_qmi init");

    if (swiQmi_InitServices(QMI_SERVICE_DMS) != LE_OK)
    {
        LE_CRIT("QMI_SERVICE_DMS cannot be initialized.");
        return LE_FAULT;
    }

    // Get the qmi client service handle for later use.
    if ((DmsClient = swiQmi_GetClientHandle(QMI_SERVICE_DMS)) == NULL)
    {
        LE_ERROR("Cannot get QMI_SERVICE_DMS client!");
        return LE_FAULT;
    }
    return LE_OK;
}
