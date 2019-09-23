/**
 * @file pa_info_qmi.c
 *
 * QMI implementation of @ref c_pa_info API.
 *
 * Copyright (C) Sierra Wireless Inc.
 */


#include "legato.h"
#include "pa_info.h"
#include "swiQmi.h"
#include "mdc_qmi.h"

// Include macros for printing out values
#include "le_print.h"

//--------------------------------------------------------------------------------------------------
/**
 * Maximum reset sources
 */
//--------------------------------------------------------------------------------------------------
#define RESET_SOURCES_MAX   12

//--------------------------------------------------------------------------------------------------
/**
 * Maximum reset types
 */
//--------------------------------------------------------------------------------------------------
#define RESET_TYPES_MAX     5

//--------------------------------------------------------------------------------------------------
/**
 * Reset info struct definition
 */
//--------------------------------------------------------------------------------------------------
typedef struct {
    le_info_Reset_t code;   ///< Reset info code
    const char*     str;    ///< Reset info string
}
ResetInfo_t;

//--------------------------------------------------------------------------------------------------
/**
 * The DMS (Device Management Service) client.  Must be obtained by calling
 * swiQmi_GetClientHandle() before it can be used.
 */
//--------------------------------------------------------------------------------------------------
static qmi_client_type DmsClient;

//--------------------------------------------------------------------------------------------------
/**
 * The NAS (Network Access Service) client.  Must be obtained by calling
 * swiQmi_GetClientHandle() before it can be used.
 */
//--------------------------------------------------------------------------------------------------
static qmi_client_type NasClient;


#define PRI_REV_EMPTY_PATTERN  "0.0"

//==================================================================================================
//  PRIVATE FUNCTIONS
//==================================================================================================


//--------------------------------------------------------------------------------------------------
/**
 * Get the device firmware and bootloader version strings.
 *
 * If only one value is desired, then pass NULL as the buffer for the other value.
 *
 * @return
 *      - LE_OK on success
 *      - LE_OVERFLOW if version string to big to fit in provided buffer
 *      - LE_NOT_FOUND if bootloader version is not available; firmware version is always available.
 *      - LE_FAULT for any other errors
 */
//--------------------------------------------------------------------------------------------------
static le_result_t GetDeviceRevision
(
    char* fwVersionPtr,
    size_t fwVersionSize,
    char* blVersionPtr,
    size_t blVersionSize
)
{
    qmi_client_error_type rc;

    // There is no request message, only a response message.
    SWIQMI_DECLARE_MESSAGE(dms_get_device_rev_id_resp_msg_v01, deviceResp);

    LE_PRINT_VALUE("%p", DmsClient);
    rc = qmi_client_send_msg_sync(
            DmsClient,
            QMI_DMS_GET_DEVICE_REV_ID_REQ_V01,
            NULL, 0,
            &deviceResp, sizeof(deviceResp),
            COMM_DEFAULT_PLATFORM_TIMEOUT);

    le_result_t result = swiQmi_CheckResponseCode(
                                STRINGIZE_EXPAND(QMI_DMS_GET_DEVICE_REV_ID_REQ_V01),
                                rc, deviceResp.resp);
    if ( result != LE_OK )
    {
        return result;
    }

    if ( fwVersionPtr != NULL )
    {
        result = le_utf8_Copy(fwVersionPtr, deviceResp.device_rev_id, fwVersionSize, NULL);
    }

    // Result could only be LE_OVERFLOW here if returned by le_utf8_Copy() immediately above
    // Just log the error and return right away, without trying to get the other version.
    if ( result == LE_OVERFLOW )
    {
        LE_ERROR("firmware version buffer overflow: '%s'", deviceResp.device_rev_id);
        result = LE_OVERFLOW;
    }
    else
    {
        if ( blVersionPtr != NULL )
        {
            if ( deviceResp.boot_code_rev_valid )
            {
                result = le_utf8_Copy(blVersionPtr, deviceResp.boot_code_rev, blVersionSize, NULL);
                if ( result == LE_OVERFLOW )
                {
                    LE_ERROR("bootloader version buffer overflow: '%s'", deviceResp.boot_code_rev);
                    result = LE_OVERFLOW;
                }
            }
            else
            {
                result = LE_NOT_FOUND;
            }
        }
    }
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get Preferred Roaming List (PRL) information.
 *
 * @return
 *      - LE_OK            The function succeeded.
 *      - LE_FAULT         The function failed to get the value.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t GetPrlInfo
(
    dms_get_current_prl_info_resp_msg_v01* snRespPtr
        ///< [OUT]
        ///< The Preferred Roaming List (PRL) reponse.
)
{
    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(DmsClient,
                    QMI_DMS_GET_CURRENT_PRL_INFO_REQ_V01,
                    NULL, 0,
                    snRespPtr, sizeof(*snRespPtr),
                    COMM_DEFAULT_PLATFORM_TIMEOUT);

    le_result_t respResult = swiQmi_CheckResponse(
                    STRINGIZE_EXPAND(QMI_DMS_GET_CURRENT_PRL_INFO_REQ_V01),
                    clientMsgErr,
                    snRespPtr->resp.result,
                    snRespPtr->resp.error);

    return respResult;
}


//==================================================================================================
//  PUBLIC API FUNCTIONS
//==================================================================================================

//--------------------------------------------------------------------------------------------------
// APIs.
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to initialize the QMI INFO module.
 *
 * @return
 * - LE_OK if successful.
 * - LE_FAULT if unsuccessful.
 */
//--------------------------------------------------------------------------------------------------
le_result_t info_qmi_Init
(
    void
)
{
    if ( (swiQmi_InitServices(QMI_SERVICE_NAS) != LE_OK) ||
         (swiQmi_InitServices(QMI_SERVICE_DMS) != LE_OK) )
    {
        LE_CRIT("Some QMI Services cannot be initialized.");
        return LE_FAULT;
    }

    // Get the qmi client service handle for later use.
    if ( (DmsClient = swiQmi_GetClientHandle(QMI_SERVICE_DMS)) == NULL)
    {
        return LE_FAULT;
    }

    if ( (NasClient = swiQmi_GetClientHandle(QMI_SERVICE_NAS)) == NULL)
    {
        return LE_FAULT;
    }
    else
    {
        return LE_OK;
    }
}


//--------------------------------------------------------------------------------------------------
/**
 * Get the firmware version string
 *
 * @return
 *      - LE_OK on success
 *      - LE_NOT_FOUND if the version string is not available
 *      - LE_OVERFLOW if version string to big to fit in provided buffer
 *      - LE_FAULT for any other errors
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_info_GetFirmwareVersion
(
    char* versionPtr,        ///< [OUT] Firmware version string
    size_t versionSize       ///< [IN] Size of version buffer
)
{
    return GetDeviceRevision(versionPtr, versionSize, NULL, 0);
}

#ifdef QMI_DMS_SWI_GET_RESET_INFO_REQ_V01
//--------------------------------------------------------------------------------------------------
/**
 * Get the last reset information reason
 *
 * @return
 *      - LE_OK          on success
 *      - LE_UNSUPPORTED if it is not supported by the platform
 *        LE_OVERFLOW    specific reset information length exceeds the maximum length.
 *      - LE_FAULT       for any other errors
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_info_GetResetInformation
(
    le_info_Reset_t* resetPtr,              ///< [OUT] Reset information
    char* resetSpecificInfoStr,             ///< [OUT] Reset specific information
    size_t resetSpecificInfoNumElements     ///< [IN] The length of specific information string.
)
{
    uint8_t type, source;
    static const ResetInfo_t info[RESET_SOURCES_MAX][RESET_TYPES_MAX] = {
        // 0 - unknown
        {
            {LE_INFO_RESET_UNKNOWN, "Unknown"},
            {LE_INFO_RESET_UNKNOWN, "Unknown"},
            {LE_INFO_POWER_DOWN, "Power Down"}, // wp76xx
            {LE_INFO_RESET_CRASH, "Crash"}, // ar75xx, wp76xx
            {LE_INFO_POWER_DOWN, "Power Down"}, // ar75xx
        },
        // 1 - user requested
        {
            {LE_INFO_RESET_UNKNOWN, "Unknown"},
            {LE_INFO_RESET_USER, "Reset, User Requested"},  // ar75xx, wp76xx
            {LE_INFO_RESET_UNKNOWN, "Unknown"},
            {LE_INFO_RESET_UNKNOWN, "Unknown"},
            {LE_INFO_POWER_DOWN, "Power Down, User requested"}, // wp76xx
        },
        // 2 - hardware switch
        {
            {LE_INFO_RESET_UNKNOWN, "Unknown"},
            {LE_INFO_RESET_UNKNOWN, "Unknown"},
            {LE_INFO_RESET_HARD, "Reset, Hardware Switch"}, // ar75xx
            {LE_INFO_RESET_UNKNOWN, "Unknown"},
            {LE_INFO_RESET_HARD, "Reset, Hardware Switch"}, // wp76xx
        },
        // 3 - temperature critical
        {
            {LE_INFO_RESET_UNKNOWN, "Unknown"},
            {LE_INFO_RESET_UNKNOWN, "Unknown"},
            {LE_INFO_RESET_UNKNOWN, "Unknown"},
            {LE_INFO_RESET_UNKNOWN, "Unknown"},
            {LE_INFO_TEMP_CRIT, "Power Down, Critical Temperature"}, // ar75xx, wp76xx
        },
        // 4 - voltage critical
        {
            {LE_INFO_RESET_UNKNOWN, "Unknown"},
            {LE_INFO_RESET_UNKNOWN, "Unknown"},
            {LE_INFO_RESET_UNKNOWN, "Unknown"},
            {LE_INFO_RESET_UNKNOWN, "Unknown"},
            {LE_INFO_VOLT_CRIT, "Power Down, Critical Voltage"}, // ar75xx
        },
        // 5 - configuration update
        {
            {LE_INFO_RESET_UNKNOWN, "Unknown"},
            {LE_INFO_RESET_UPDATE, "Reset, Configuration Update"}, // ar75xx, wp76xx
            {LE_INFO_RESET_UNKNOWN, "Unknown"},
            {LE_INFO_RESET_UNKNOWN, "Unknown"},
            {LE_INFO_RESET_UNKNOWN, "Unknown"},
        },
        // 6 - LWM2M
        {
            {LE_INFO_RESET_UNKNOWN, "Unknown"},
            {LE_INFO_RESET_UPDATE, "Reset, LWM2M Update"}, // ar75xx, wp76xx
            {LE_INFO_RESET_UNKNOWN, "Unknown"},
            {LE_INFO_RESET_UNKNOWN, "Unknown"},
            {LE_INFO_RESET_UNKNOWN, "Unknown"},
        },
        // 7 - OMA-DM
        {
            {LE_INFO_RESET_UNKNOWN, "Unknown"},
            {LE_INFO_RESET_UPDATE, "Reset, OMA-DM Update"}, // ar75xx, wp76xx
            {LE_INFO_RESET_UNKNOWN, "Unknown"},
            {LE_INFO_RESET_UNKNOWN, "Unknown"},
            {LE_INFO_RESET_UNKNOWN, "Unknown"},
        },
        // 8 - FOTA
        {
            {LE_INFO_RESET_UNKNOWN, "Unknown"},
            {LE_INFO_RESET_UPDATE, "Reset, FOTA Update"}, // ar75xx, wp76xx
            {LE_INFO_RESET_UNKNOWN, "Unknown"},
            {LE_INFO_RESET_UNKNOWN, "Unknown"},
            {LE_INFO_RESET_UNKNOWN, "Unknown"},
        },
        // 9 - SW update
        {
            {LE_INFO_RESET_UNKNOWN, "Unknown"},
            {LE_INFO_RESET_UPDATE, "Reset, Software Update"}, // ar75xx
            {LE_INFO_RESET_UNKNOWN, "Unknown"},
            {LE_INFO_RESET_UNKNOWN, "Unknown"},
            {LE_INFO_RESET_UNKNOWN, "Unknown"},
        },
        // 10 - SWAP
        {
            {LE_INFO_RESET_UNKNOWN, "Unknown"},
            {LE_INFO_RESET_UPDATE, "Reset, Swap"}, // ar75xx
            {LE_INFO_RESET_UNKNOWN, "Unknown"},
            {LE_INFO_RESET_UNKNOWN, "Unknown"},
            {LE_INFO_RESET_UNKNOWN, "Unknown"},
        },
        // 11 - SWAP_SYNC
        {
            {LE_INFO_RESET_UNKNOWN, "Unknown"},
            {LE_INFO_RESET_UPDATE, "Reset, Swap Sync"}, // ar75xx
            {LE_INFO_RESET_UNKNOWN, "Unknown"},
            {LE_INFO_RESET_UNKNOWN, "Unknown"},
            {LE_INFO_RESET_UNKNOWN, "Unknown"},
        },
    };

    SWIQMI_DECLARE_MESSAGE(dms_swi_get_reset_info_resp_msg_v01, resetInfoResp);

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(DmsClient,
                                             QMI_DMS_SWI_GET_RESET_INFO_REQ_V01,
                                             NULL, 0,
                                             &resetInfoResp, sizeof(resetInfoResp),
                                             COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (LE_OK != swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_DMS_GET_DEVICE_REV_ID_REQ_V01),
                                      clientMsgErr,
                                      resetInfoResp.resp.result,
                                      resetInfoResp.resp.error))
    {
        return LE_FAULT;
    }

    type = resetInfoResp.reset_info.type;
    source = resetInfoResp.reset_info.source;

    LE_DEBUG("Info type: %d, source: %d", type, source);

    if ( (type >= RESET_TYPES_MAX) || (source >= RESET_SOURCES_MAX) )
    {
        LE_ERROR("Invalid type: %u or source: %u", type, source);
        return LE_FAULT;
    }

    *resetPtr = info[source][type].code;

    return le_utf8_Copy(resetSpecificInfoStr, info[source][type].str,
                        resetSpecificInfoNumElements, NULL);
}
#endif

//--------------------------------------------------------------------------------------------------
/**
 * Get the bootloader version string
 *
 * @return
 *      - LE_OK on success
 *      - LE_NOT_FOUND if the version string is not available
 *      - LE_OVERFLOW if version string to big to fit in provided buffer
 *      - LE_FAULT for any other errors
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_info_GetBootloaderVersion
(
    char* versionPtr,        ///< [OUT] Firmware version string
    size_t versionSize       ///< [IN] Size of version buffer
)
{
    return GetDeviceRevision(NULL, 0, versionPtr, versionSize);
}


//--------------------------------------------------------------------------------------------------
/**
 * This function get the International Mobile Equipment Identity (IMEI).
 *
 * @return
 * - LE_FAULT         The function failed to get the value.
 * - LE_TIMEOUT       No response was received from the Modem.
 * - LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_info_GetImei
(
    pa_info_Imei_t imei   ///< [OUT] IMEI value
)
{
    le_result_t res = LE_FAULT;

    if (imei == NULL)
    {
        LE_ERROR("imei is NULL.");
        return res;
    }

    SWIQMI_DECLARE_MESSAGE(dms_get_device_serial_numbers_resp_msg_v01, snResp);

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(DmsClient,
                                            QMI_DMS_GET_DEVICE_SERIAL_NUMBERS_REQ_V01,
                                            NULL, 0,
                                            &snResp, sizeof(snResp),
                                            COMM_DEFAULT_PLATFORM_TIMEOUT);

    le_result_t respResult = swiQmi_CheckResponseCode(
                                STRINGIZE_EXPAND(QMI_DMS_GET_DEVICE_SERIAL_NUMBERS_REQ_V01),
                                clientMsgErr,
                                snResp.resp);

    if (respResult == LE_OK)
    {
        if (0 != snResp.imei_valid)
        {
            LE_DEBUG("imei %s", snResp.imei);
            // Copy the imei to the user's buffer.
            res = le_utf8_Copy(imei, snResp.imei, PA_INFO_IMEI_MAX_BYTES, NULL);
        }
        else
        {
            LE_WARN("IMEI field option not present or not valid!!");
        }

    }
    return res;


}

//--------------------------------------------------------------------------------------------------
/**
 * This function get the International Mobile Equipment Identity software version number (IMEISV).
 *
 * @return
 * - LE_FAULT         The function failed to get the value.
 * - LE_TIMEOUT       No response was received from the Modem.
 * - LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_info_GetImeiSv
(
    pa_info_ImeiSv_t imeiSv   ///< [OUT] IMEISV value
)
{
    le_result_t res = LE_FAULT;

    if (imeiSv == NULL)
    {
        LE_ERROR("imeiSv is NULL.");
        return res;
    }

    SWIQMI_DECLARE_MESSAGE(dms_get_device_serial_numbers_resp_msg_v01, snResp);

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(DmsClient,
                                            QMI_DMS_GET_DEVICE_SERIAL_NUMBERS_REQ_V01,
                                            NULL, 0,
                                            &snResp, sizeof(snResp),
                                            COMM_DEFAULT_PLATFORM_TIMEOUT);

    le_result_t respResult = swiQmi_CheckResponseCode(
                                STRINGIZE_EXPAND(QMI_DMS_GET_DEVICE_SERIAL_NUMBERS_REQ_V01),
                                clientMsgErr,
                                snResp.resp);

    if (respResult == LE_OK)
    {
        if (0 != snResp.imeisv_svn_valid)
        {
            LE_DEBUG("imeisv %s", snResp.imeisv_svn);
            // Copy the imeiSv to the user's buffer.
            res = le_utf8_Copy(imeiSv, snResp.imeisv_svn, PA_INFO_IMEISV_MAX_BYTES, NULL);
        }
        else
        {
            LE_WARN("IMEISV field option not present or not valid!!");
        }

    }
    return res;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function gets the device model identity.
 *
 * @return
 * - LE_FAULT         The function failed to get the value.
 * - LE_OVERFLOW      The device model identity length exceed the maximum length.
 * - LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_info_GetDeviceModel
(
    pa_info_DeviceModel_t model   ///< [OUT] Model string (null-terminated).
)
{
    SWIQMI_DECLARE_MESSAGE(dms_get_device_model_id_resp_msg_v01, snResp);

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(DmsClient,
                                            QMI_DMS_GET_DEVICE_MODEL_ID_REQ_V01,
                                            NULL, 0,
                                            &snResp, sizeof(snResp),
                                            COMM_DEFAULT_PLATFORM_TIMEOUT);

    le_result_t respResult = swiQmi_CheckResponseCode(
                                STRINGIZE_EXPAND(QMI_DMS_GET_DEVICE_MODEL_ID_REQ_V01),
                                clientMsgErr,
                                snResp.resp);

    if (respResult == LE_OK)
    {
        // Copy the model ID to the user's buffer.
        LE_ASSERT(LE_OK == le_utf8_Copy(model,
                                        snResp.device_model_id,
                                        PA_INFO_MODEL_MAX_BYTES,
                                        NULL));
    }

    return respResult;
}


//--------------------------------------------------------------------------------------------------
/**
 * Get the CDMA device Mobile Equipment Identifier (MEID).
 *
 * @return
 *      - LE_OK            The function succeeded.
 *      - LE_FAULT         The function failed to get the value.
 *      - LE_OVERFLOW      The device Mobile Equipment identifier length exceed the maximum length.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_info_GetMeid
(
    char* meidStr,           ///< [OUT] Firmware version string
    size_t meidStrSize       ///< [IN] Size of version buffer
)
{
    le_result_t res = LE_FAULT;
    SWIQMI_DECLARE_MESSAGE(dms_get_device_serial_numbers_resp_msg_v01, snResp);

    if (meidStr == NULL)
    {
        LE_ERROR("meidPtr is NULL.");
        return res;
    }

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(DmsClient,
                    QMI_DMS_GET_DEVICE_SERIAL_NUMBERS_REQ_V01,
                    NULL, 0,
                    &snResp, sizeof(snResp),
                    COMM_DEFAULT_PLATFORM_TIMEOUT);

    le_result_t respResult = swiQmi_CheckResponseCode(
                    STRINGIZE_EXPAND(QMI_DMS_GET_DEVICE_SERIAL_NUMBERS_REQ_V01),
                    clientMsgErr,
                    snResp.resp);

    if (respResult == LE_OK)
    {
        if (snResp.meid_valid)
        {
            // Copy the MEID to the user's buffer.
            res = le_utf8_Copy(meidStr, snResp.meid, meidStrSize, NULL);
        }
        else
        {
            LE_WARN("MEID field option not present or not valid!!");
        }
    }

    return res;
}


//--------------------------------------------------------------------------------------------------
/**
 * Get the CDMA Electronic Serial Number (ESN) of the device.
 *
 * @return
 *      - LE_OK            The function succeeded.
 *      - LE_FAULT         The function failed to get the value.
 *      - LE_OVERFLOW      The Electric SerialNumbe length exceed the maximum length.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_info_GetEsn
(
    char* esnStr,
        ///< [OUT]
        ///< The Electronic Serial Number (ESN) of the device
        ///<  string (null-terminated).

    size_t esnStrNumElements
        ///< [IN]
)
{
    le_result_t res = LE_FAULT;
    SWIQMI_DECLARE_MESSAGE(dms_get_device_serial_numbers_resp_msg_v01, snResp);

    if (esnStr == NULL)
    {
        LE_ERROR("esnPtr is NULL.");
        return res;
    }

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(DmsClient,
                    QMI_DMS_GET_DEVICE_SERIAL_NUMBERS_REQ_V01,
                    NULL, 0,
                    &snResp, sizeof(snResp),
                    COMM_DEFAULT_PLATFORM_TIMEOUT);

    le_result_t respResult = swiQmi_CheckResponseCode(
                    STRINGIZE_EXPAND(QMI_DMS_GET_DEVICE_SERIAL_NUMBERS_REQ_V01),
                    clientMsgErr,
                    snResp.resp);

    if (respResult == LE_OK)
    {
        if(snResp.esn_valid)
        {
            // Copy the ESN to the user's buffer.
            res = le_utf8_Copy(esnStr, snResp.esn, esnStrNumElements, NULL);
        }
        else
        {
            LE_WARN("ESN field option not present or not valid!!");
        }
    }

    return res;
}


//--------------------------------------------------------------------------------------------------
/**
 * Get the CDMA Mobile Identification Number (MIN).
 *
 * @return
 *      - LE_OK            The function succeeded.
 *      - LE_FAULT         The function failed to get the value.
 *      - LE_OVERFLOW      The CDMA Mobile Identification Number length exceed the maximum length.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_info_GetMin
(
    char        *minStr,    ///< [OUT] The phone Number
    size_t       minStrSize ///< [IN]  Size of phoneNumberStr
)
{
    SWIQMI_DECLARE_MESSAGE(dms_get_msisdn_resp_msg_v01, getMinResp);
    le_result_t res = LE_FAULT;

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(DmsClient,
                    QMI_DMS_GET_MSISDN_REQ_V01,
                    NULL, 0,
                    &getMinResp, sizeof(getMinResp),
                    COMM_DEFAULT_PLATFORM_TIMEOUT);

    le_result_t respResult = swiQmi_CheckResponseCode(
                    STRINGIZE_EXPAND(QMI_DMS_GET_MSISDN_REQ_V01),
                    clientMsgErr,
                    getMinResp.resp);

    if (respResult == LE_OK)
    {
        if (getMinResp.mobile_id_number_valid)
        {
            // Copy the mobile ID number (MIN) to the user's buffer.
            res = le_utf8_Copy(minStr, getMinResp.mobile_id_number, minStrSize, NULL);
        }
        else
        {
            LE_WARN("MIN field option not present or not valid!!");
        }
    }

    return res;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the version of Preferred Roaming List (PRL).
 *
 * @return
 *      - LE_OK            The function succeeded.
 *      - LE_NOT_FOUND     The information is not available.
 *      - LE_FAULT         The function failed to get the value.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_info_GetPrlVersion
(
    uint16_t* prlVersionPtr
        ///< [OUT]
        ///< The Preferred Roaming List (PRL) version.
)
{
    SWIQMI_DECLARE_MESSAGE(dms_get_current_prl_info_resp_msg_v01, snResp);
    le_result_t res = LE_FAULT;

    if (prlVersionPtr == NULL)
    {
        LE_ERROR("prlVersionPtr is NULL.");
        return res;
    }

    le_result_t respResult = GetPrlInfo(&snResp);

    if (respResult == LE_OK)
    {
        if (snResp.prl_version_valid)
        {
            // Copy PRL Version to the user.
            *prlVersionPtr = snResp.prl_version;
            res = LE_OK;
        }
        else
        {
            res = LE_NOT_FOUND;
            LE_WARN("PRL-Version not valid!!");
        }
    }

    return res;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the CDMA Preferred Roaming List (PRL) only preferences status.
 *
 * @return
 *      - LE_OK            The function succeeded.
 *      - LE_NOT_FOUND     The information is not available.
 *      - LE_FAULT         The function failed to get the value.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_info_GetPrlOnlyPreference
(
    bool* prlOnlyPreferencePtr      ///< The Cdma PRL only preferences status.
)
{
    SWIQMI_DECLARE_MESSAGE(dms_get_current_prl_info_resp_msg_v01, snResp);
    le_result_t res = LE_FAULT;

    if (prlOnlyPreferencePtr == NULL)
    {
        LE_ERROR("prlOnlyPreferencePtr is NULL.");
        return res;
    }

    le_result_t respResult = GetPrlInfo(&snResp);

    if (respResult == LE_OK)
    {
        if (snResp.prl_only_valid)
        {
            // Copy PRL status to the user.
            *prlOnlyPreferencePtr = snResp.prl_only;
            res = LE_OK;
        }
        else
        {
            res = LE_NOT_FOUND;
            LE_WARN("PRL-Only Preference not valid!!");
        }
    }

    return res;
}


//--------------------------------------------------------------------------------------------------
/**
 * Get the CDMA Network Access Identifier (NAI) string in ASCII text.
 *
 * @return
 *      - LE_OK            The function succeeded.
 *      - LE_FAULT         The function failed to get the value.
 *      - LE_OVERFLOW      The Network Access Identifier (NAI) length exceed the maximum length.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_info_GetNai
(
    char* naiStr,
        ///< [OUT]
        ///< The Network Access Identifier (NAI)
        ///<  string (null-terminated).

    size_t naiStrNumElements
        ///< [IN]
)
{
    qmi_client_type wdsClient = mdc_qmi_GetDefaultWdsClient();

    if (wdsClient == NULL)
    {
        LE_ERROR("bad wdsclient");
        return LE_FAULT;
    }

    wds_read_mip_profile_req_msg_v01 snReq  = {0};
    SWIQMI_DECLARE_MESSAGE(wds_read_mip_profile_resp_msg_v01, snResp);
    le_result_t res = LE_FAULT;

    SWIQMI_DECLARE_MESSAGE(wds_get_active_mip_profile_resp_msg_v01, getMipProfileResp);

    if (naiStr == NULL)
    {
        LE_ERROR("naiStr is NULL.");
        goto end;
    }

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(wdsClient,
                    QMI_WDS_GET_ACTIVE_MIP_PROFILE_REQ_V01,
                    NULL, 0,
                    &getMipProfileResp, sizeof(getMipProfileResp),
                    COMM_DEFAULT_PLATFORM_TIMEOUT);

    le_result_t respResult = swiQmi_CheckResponseCode(
                    STRINGIZE_EXPAND(QMI_WDS_GET_ACTIVE_MIP_PROFILE_REQ_V01),
                    clientMsgErr,
                    getMipProfileResp.resp);

    if (respResult != LE_OK)
    {
        LE_ERROR("MIP profile Index getting Failed");
        goto end;
    }

    snReq.profile_index = getMipProfileResp.profile_index;
    LE_DEBUG("MIP profile index %d", snReq.profile_index);

    clientMsgErr = qmi_client_send_msg_sync(wdsClient,
                    QMI_WDS_READ_MIP_PROFILE_REQ_V01,
                    &snReq, sizeof(snReq),
                    &snResp, sizeof(snResp),
                    COMM_DEFAULT_PLATFORM_TIMEOUT);

    respResult = swiQmi_CheckResponseCode(
                    STRINGIZE_EXPAND(QMI_WDS_READ_MIP_PROFILE_REQ_V01),
                    clientMsgErr,
                    snResp.resp);

    if (respResult == LE_OK)
    {
        if (snResp.nai_valid)
        {
            // Copy the NAI to the user's buffer.
            res = le_utf8_Copy(naiStr, snResp.nai, naiStrNumElements, NULL);
        }
        else
        {
            LE_WARN("NAI field option is not present or not valid for index %d!!",
                            snReq.profile_index);
        }
    }

end:
    mdc_qmi_ReleaseWdsClient(wdsClient);
    return res;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the Manufacturer Name string in ASCII text.
 *
 * @return
 *      - LE_OK            The function succeeded.
 *      - LE_FAULT         The function failed to get the value.
 *      - LE_OVERFLOW      The Manufacturer Name length exceed the maximum length.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_info_GetManufacturerName
(
    char* mfrNameStr,
        ///< [OUT]
        ///< The Manufacturer Name string (null-terminated).

    size_t mfrNameStrNumElements
        ///< [IN]
)
{
    le_result_t res = LE_FAULT;
    SWIQMI_DECLARE_MESSAGE(dms_get_device_mfr_resp_msg_v01, resp);

    if (mfrNameStr == NULL)
    {
        LE_ERROR("mfrNameStr is NULL.");
    }
    else
    {
        qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(DmsClient,
                        QMI_DMS_GET_DEVICE_MFR_REQ_V01,
                        NULL, 0,
                        &resp, sizeof(resp),
                        COMM_DEFAULT_PLATFORM_TIMEOUT);

        le_result_t respResult = swiQmi_CheckResponseCode(
                        STRINGIZE_EXPAND(QMI_DMS_GET_DEVICE_MFR_REQ_V01),
                        clientMsgErr,
                        resp.resp);

        if (respResult == LE_OK)
        {
            // Copy the Manufacturer Name to the user's buffer.
            res = le_utf8_Copy(mfrNameStr,
                                resp.device_manufacturer,
                                mfrNameStrNumElements,
                                NULL);
        }
    }

    return res;
}


//--------------------------------------------------------------------------------------------------
/**
 * Get the Product Requirement Information Part Number and the Revision Number strings in ASCII text
 *
 * @return
 *      - LE_OK            The function succeeded.
 *      - LE_FAULT         The function failed to get the value.
 *      - LE_OVERFLOW      The PRI Part Number or the Revision Number strings length exceed the
 *                         maximum length provided for copy.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_info_GetPriId
(
    char* priIdPnStr,
        ///< [OUT]
        ///< The Product Requirement Information Identifier
        ///<  (PRI ID) Part Number string (null-terminated).

    size_t priIdPnStrNumElements,
        ///< [IN]

    char* priIdRevStr,
        ///< [OUT]
        ///< The Product Requirement Information Identifier
        ///<  (PRI ID) Revision Number string (null-terminated).

    size_t priIdRevStrNumElements
        ///< [IN]
)
{
    SWIQMI_DECLARE_MESSAGE(dms_get_device_rev_id_resp_msg_v01, snResp);

    if ((NULL == priIdPnStr) || (NULL == priIdRevStr))
    {
        LE_ERROR("priIdPnStr or priIdRevStr is NULL.");
        return LE_FAULT;
    }

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(DmsClient,
                        QMI_DMS_GET_DEVICE_REV_ID_REQ_V01,
                        NULL, 0,
                        &snResp, sizeof(snResp),
                        COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (LE_OK != swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_DMS_GET_DEVICE_REV_ID_REQ_V01),
                                      clientMsgErr,
                                      snResp.resp.result,
                                      snResp.resp.error))
    {
        return LE_FAULT;
    }

    if (!snResp.pri_rev_valid)
    {
        LE_ERROR("PRI and Revision Number string not available");
        return LE_FAULT;
    }

    LE_DEBUG("pri_rev len (%d) = %s", strlen(snResp.pri_rev), snResp.pri_rev);

    // test empty PRI and Revision
    if (0 == strncmp(snResp.pri_rev, PRI_REV_EMPTY_PATTERN, sizeof(PRI_REV_EMPTY_PATTERN)))
    {
        LE_ERROR("Empty PRI and Revision Number");
        priIdRevStr[0] = '\0';
        priIdPnStr[0] = '\0';
        return LE_FAULT;
    }

    // test space in "PRI + RV" string
    char *c = strchr(snResp.pri_rev,' ');
    if (!c)
    {
        LE_ERROR("PRI and Revision Number string error");
        return LE_FAULT;
    }

    // test revision length
    c++;
    int stringSize = strlen(c);

    if (priIdRevStrNumElements <= stringSize)
    {
        LE_ERROR("Revision Number string length too big %d", stringSize);
        return LE_OVERFLOW;
    }

    // test PRI length
    stringSize = strlen(snResp.pri_rev)-strlen(c)-1;

    if (priIdPnStrNumElements <= stringSize)
    {
        LE_ERROR("PRI string length too big %d", stringSize);
        return LE_OVERFLOW;
    }

    // copy the revision
    if (0 > snprintf(priIdRevStr, priIdRevStrNumElements, "%s", c))
    {
        LE_ERROR("Failed to copy the Revision Number");
        return LE_FAULT;
    }
    // copy the PRI
    --c;
    *c = '\0';
    if (0 > snprintf(priIdPnStr, priIdPnStrNumElements, "%s", snResp.pri_rev))
    {
        LE_ERROR("Failed to copy the PRI");
        return LE_FAULT;
    }
    return LE_OK;
}

#ifdef QMI_DMS_CARRIER_PRI_NAME_SIZE_V01

//--------------------------------------------------------------------------------------------------
/**
 * Get the Carrier PRI Name and Revision Number strings in ASCII text.
 *
 * @return
 *      - LE_OK            The function succeeded.
 *      - LE_FAULT         The function failed to get the value.
 *      - LE_OVERFLOW      The Name or the Revision Number strings length exceed the maximum length.
 *      - LE_UNSUPPORTED   The function is not supported on the platform.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_info_GetCarrierPri
(
    char* capriNameStr,
        ///< [OUT]
        ///< The Carrier Product Requirement Information Name
        ///< Carrier PRI Name string (null-terminated).

    size_t capriNameStrNumElements,
        ///< [IN]

    char* capriIdRevStr,
        ///< [OUT]
        ///< The Carrier Product Requirement Information Identifier
        ///< Carrier PRI Revision Number string (null-terminated).

    size_t capriIdRevStrNumElements
        ///< [IN]
)
{
    le_result_t res = LE_OK;
    int length;

    if ( (capriNameStr == NULL) || (capriIdRevStr == NULL))
    {
        LE_ERROR("capriNameStr or capriIdRevStr is NULL.");
        res =  LE_FAULT;
    }

    if (capriNameStrNumElements < LE_INFO_MAX_CAPRI_NAME_BYTES)
    {
        LE_ERROR("capriNameStrNumElements length (%d) too small < %d",
                        (int) capriNameStrNumElements, LE_INFO_MAX_CAPRI_NAME_BYTES);
        res = LE_OVERFLOW;
    }

    if (capriIdRevStrNumElements < LE_INFO_MAX_CAPRI_REV_BYTES)
    {
        LE_ERROR("capriIdRevStrNumElements length (%d) too small < %d",
                        (int) capriIdRevStrNumElements, LE_INFO_MAX_CAPRI_REV_BYTES);
        res = LE_OVERFLOW;
    }

    if (res == LE_OK)
    {
        res = LE_FAULT;

        SWIQMI_DECLARE_MESSAGE(dms_swi_get_cwe_pkgs_info_resp_msg_v01, pkgsResp);

        qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(DmsClient,
                        QMI_DMS_SWI_GET_CWE_PKGS_INFO_REQ_V01,
                        NULL, 0,
                        &pkgsResp, sizeof(pkgsResp),
                        COMM_DEFAULT_PLATFORM_TIMEOUT);

        res = swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_DMS_SWI_GET_CWE_PKGS_INFO_REQ_V01),
                                    clientMsgErr,
                                    pkgsResp.resp.result,
                                    pkgsResp.resp.error);

        if (res == LE_OK)
        {
            //read carrier pri name
            if (pkgsResp.carrier_pri_name_valid)
            {
                length = strlen(pkgsResp.carrier_pri_name);

                if (length <= LE_INFO_MAX_CAPRI_NAME_LEN)
                {
                    le_utf8_Copy(capriNameStr,
                                 pkgsResp.carrier_pri_name,
                                 LE_INFO_MAX_CAPRI_NAME_BYTES,
                                 NULL);
                    res = LE_OK;
                }
                else
                {
                    LE_ERROR("Carrier PRI name buffer overflow");
                    return LE_FAULT;
                }
            }
            else
            {
                LE_ERROR("Carrier PRI name field option is not valid!!");
                return LE_FAULT;
            }

            // read carrier pri rev
            if (pkgsResp.carrier_pri_rev_valid)
            {
                length = strlen(pkgsResp.carrier_pri_rev);

                if (length <= LE_INFO_MAX_CAPRI_REV_LEN)
                {
                    le_utf8_Copy(capriIdRevStr,
                                 pkgsResp.carrier_pri_rev,
                                 LE_INFO_MAX_CAPRI_REV_BYTES,
                                 NULL);
                    res = LE_OK;
                }
                else
                {
                    LE_ERROR("Carrier PRI rev buffer overflow");
                    return LE_FAULT;
                }
            }
            else
            {
                LE_ERROR("Carrier PRI rev field option is not valid!!");
                return LE_FAULT;
            }
        }
        else
        {
            LE_ERROR("PRI ID field option is not valid!!");
            res = LE_FAULT;
        }
    }

    return res;
}

#endif

//--------------------------------------------------------------------------------------------------
/**
 * Get the product stock keeping unit number (SKU) number string in ASCII text.
 *
 * @return
 *      - LE_OK            The function succeeded.
 *      - LE_FAULT         The function failed to get the value.
 *      - LE_OVERFLOW      The SKU number string length exceeds the maximum length.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_info_GetSku
(
    char* skuIdStr,
        ///< [OUT] Product SKU ID string.

    size_t skuIdStrNumElements
        ///< [IN]
)
{
    le_result_t res = LE_FAULT;

    if (skuIdStr == NULL)
    {
        LE_ERROR("skuIdStr is NULL.");
    }

    SWIQMI_DECLARE_MESSAGE(dms_swi_get_cwe_pkgs_info_resp_msg_v01, pkgsResp);

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(DmsClient,
                    QMI_DMS_SWI_GET_CWE_PKGS_INFO_REQ_V01,
                    NULL, 0,
                    &pkgsResp, sizeof(pkgsResp),
                    COMM_DEFAULT_PLATFORM_TIMEOUT);

    res = swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_DMS_SWI_GET_CWE_PKGS_INFO_REQ_V01),
                                clientMsgErr,
                                pkgsResp.resp.result,
                                pkgsResp.resp.error);

    if (res == LE_OK)
    {
        if (pkgsResp.sku_id_valid)
        {
            // Copy the product SKU ID to the user's buffer.
            res = le_utf8_Copy(skuIdStr, pkgsResp.sku_id, skuIdStrNumElements, NULL);
        }
        else
        {
            LE_WARN("SKU field option is not present or not valid!!");
            res = LE_FAULT;
        }
    }
    else
    {
        LE_ERROR("CheckResponseCode is not valid ! res.%d", res);
    }

    return res;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the Platform Serial Number (PSN) string.
 *
 * @return
 *      - LE_OK on success
 *      - LE_OVERFLOW if Platform Serial Number to big to fit in provided buffer
 *      - LE_FAULT for any other errors
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_info_GetPlatformSerialNumber
(
    char* platformSerialNumberStr,
        ///< [OUT]
        ///< Platform Serial Number string.

    size_t platformSerialNumberStrNumElements
        ///< [IN]
)
{
    le_result_t res = LE_FAULT;

    dms_swi_get_fsn_resp_msg_v01 snResp  = { {0} };

    if (platformSerialNumberStr == NULL)
    {
        LE_ERROR("platformSerialNumberStr is NULL.");
        return LE_FAULT;
    }

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(DmsClient,
                    QMI_DMS_SWI_GET_FSN_REQ_V01,
                    NULL, 0,
                    &snResp, sizeof(snResp),
                    COMM_DEFAULT_PLATFORM_TIMEOUT);

    if ( swiQmi_CheckResponseCode(
                    STRINGIZE_EXPAND(QMI_DMS_SWI_GET_FSN_REQ_V01),
                    clientMsgErr,
                    snResp.resp) == LE_OK)
    {
        if (platformSerialNumberStrNumElements >= LE_INFO_MAX_PSN_BYTES)
        {
            /*
             * Copy the Platform Serial Number to the user's buffer.
             * The FSN batch number part (last 2 digits) shall not be returned
             */
            le_utf8_Copy(platformSerialNumberStr, snResp.fsn,
                            LE_INFO_MAX_PSN_BYTES,
                            NULL);
            res = LE_OK;
        }
        else
        {
            LE_ERROR("Buffer size overflow !!");
            res = LE_OVERFLOW;
        }
    }
    else
    {
        LE_ERROR("Failed to get the Platform Serial Number string !!");
        res = LE_FAULT;
    }

    return res;
}

#if defined(QMI_DMS_SWI_RF_STATUS_READ_REQ_V01)
//--------------------------------------------------------------------------------------------------
/**
 * Get the RF devices working status (i.e. working or broken) of modem's RF devices such as
 * power amplifier, antenna switch and transceiver. That status is updated every time the module
 * power on.
 *
 * @return
 *      - LE_OK on success.
 *      - LE_UNSUPPORTED request not supported
 *      - LE_FAULT function failed to get the RF devices working status
 *      - LE_OVERFLOW the number of statuses exceeds the maximum size
 *      - LE_BAD_PARAMETER Null pointers provided
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_info_GetRfDeviceStatus
(
    uint16_t* manufacturedIdPtr,
        ///< [OUT] Manufactured identifier (MID)

    size_t* manufacturedIdNumElementsPtr,
        ///< [INOUT]

    uint8_t* productIdPtr,
        ///< [OUT] Product identifier (PID)

    size_t* productIdNumElementsPtr,
        ///< [INOUT]

    bool* statusPtr,
        ///< [OUT] Status of the specified device (MID,PID):
        ///<       0 means something wrong in the RF device,
        ///<       1 means no error found in the RF device

    size_t* statusNumElementsPtr
        ///< [INOUT]
)
{
    // Check input pointers
    if ((manufacturedIdPtr == NULL) || (manufacturedIdNumElementsPtr == NULL)||
        (productIdPtr == NULL)|| (productIdNumElementsPtr == NULL) ||
        (statusPtr == NULL)|| (statusNumElementsPtr == NULL))
    {
        LE_ERROR("NULL pointers!");
        return LE_BAD_PARAMETER;
    }

    int i = 0;
    SWIQMI_DECLARE_MESSAGE(dms_swi_rf_status_read_resp_msg_v01, readResp);

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(DmsClient,
                    QMI_DMS_SWI_RF_STATUS_READ_REQ_V01,
                    NULL, 0,
                    &readResp, sizeof(readResp),
                    COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (LE_OK != swiQmi_CheckResponse(
                    STRINGIZE_EXPAND(QMI_DMS_SWI_RF_STATUS_READ_RESP_V01),
                    clientMsgErr,
                    readResp.resp.result,
                    readResp.resp.error))
    {
        LE_ERROR("Failed to get the RF devices working status !!");
        return LE_FAULT;
    }

    LE_DEBUG("Status length %d", readResp.RF_status_len);

    // Update returned length
    if ((readResp.RF_status_len > *manufacturedIdNumElementsPtr) ||
        (readResp.RF_status_len > *productIdNumElementsPtr) ||
        (readResp.RF_status_len > *statusNumElementsPtr))
    {
        LE_ERROR("Status lenght overflow !! %d", readResp.RF_status_len);
        *manufacturedIdNumElementsPtr = 0;
        *productIdNumElementsPtr = 0;
        *statusNumElementsPtr = 0;
        return LE_OVERFLOW;
    }

    *manufacturedIdNumElementsPtr = readResp.RF_status_len;
    *productIdNumElementsPtr = readResp.RF_status_len;
    *statusNumElementsPtr = readResp.RF_status_len;

    // Get the status fields
    for (i=0; i < readResp.RF_status_len; i++)
    {
        LE_DEBUG("Status %d for MID %d, PID %d",
                 readResp.RF_status[i].Device_status,
                 readResp.RF_status[i].Manufactured_ID,
                 readResp.RF_status[i].Product_ID);

        manufacturedIdPtr[i] = readResp.RF_status[i].Manufactured_ID;
        productIdPtr[i] = readResp.RF_status[i].Product_ID;
        statusPtr[i] = readResp.RF_status[i].Device_status;
    }
    return LE_OK;
}
#endif
