/**
 * @file pa_mrc_qmi.c
 *
 * QMI implementation of @ref c_pa_mrc API.
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#include "legato.h"
#include "pa_qmi_local.h"
#include "pa_mrc.h"
#include "swiQmi.h"
#include "mrc_qmi.h"

// Include macros for printing out values
#include "le_print.h"

#include <semaphore.h>
#include "watchdogChain.h"
#include "le_ms_local.h"
#include "thread.h"

//--------------------------------------------------------------------------------------------------
/**
 * Values for Circuit Switched and Packet Switched service state
 */
//--------------------------------------------------------------------------------------------------
#define QMI_CS_ATTACHED 0x01
#define QMI_CS_DETACHED 0x02
#define QMI_PS_ATTACHED 0x01
#define QMI_PS_DETACHED 0x02

//--------------------------------------------------------------------------------------------------
/**
 * Round signal delta for RAT TD-SCDMA.
 */
//--------------------------------------------------------------------------------------------------
#define ROUND_SIGNAL_DELTA    5

//--------------------------------------------------------------------------------------------------
/**
 * Minimum signal delta for RAT TD-SCDMA.
 */
//--------------------------------------------------------------------------------------------------
#define MIN_SIGNAL_DELTA_FOR_TDSCDMA  10

//--------------------------------------------------------------------------------------------------
// Data structures.
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
/**
 * Registration Structure for synchronization
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    bool                             inProgress; ///< Command pa_mrc_RegisterNetwork is in progress
    uint16_t                         mcc;        ///< Mobile Country Code
    uint16_t                         mnc;        ///< Mobile Network Code
    nas_registration_state_enum_v01  state;      ///< Registration State
    sem_t                            semSynchro; ///< Semaphore for synchronization
}
RegisterNetwork_t;


//--------------------------------------------------------------------------------------------------
// Static declarations.
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
/**
 * The NAS (Network Access Service) client.  Must be obtained by calling
 * swiQmi_GetClientHandle() before it can be used.
 */
//--------------------------------------------------------------------------------------------------
static qmi_client_type NasClient;

//--------------------------------------------------------------------------------------------------
/**
 * The DMS (Device Management Services) client.  Must be obtained by calling
 * swiQmi_GetClientHandle() before it can be used.
 */
//--------------------------------------------------------------------------------------------------
static qmi_client_type DmsClient;

//--------------------------------------------------------------------------------------------------
/**
 * The MGS (M2M General Service) client.  Must be obtained by calling
 * swiQmi_GetClientHandle() before it can be used.
 */
//--------------------------------------------------------------------------------------------------
static qmi_client_type MgsClient;

//--------------------------------------------------------------------------------------------------
/**
 * The SWI SAR (Specific Absorption Rate) client.  Must be obtained by calling
 * swiQmi_GetClientHandle() before it can be used.
 */
//--------------------------------------------------------------------------------------------------
static qmi_client_type SwiSarClient;

//--------------------------------------------------------------------------------------------------
/**
 * This event is reported when a Radio Access Technology change indication is received from the
 * modem. The report data is allocated from the associated pool.
 */
//--------------------------------------------------------------------------------------------------
static le_event_Id_t RatChangeEventId;

//--------------------------------------------------------------------------------------------------
/**
 * Pool for Radio Access Technology change indication reporting.
 */
//--------------------------------------------------------------------------------------------------
static le_mem_PoolRef_t RatChangePool;

//--------------------------------------------------------------------------------------------------
/**
 * This event is reported when a Packet Switched change indication is received from the
 * modem. The report data is allocated from the associated pool.
 */
//--------------------------------------------------------------------------------------------------
static le_event_Id_t PSChangeEventId;

//--------------------------------------------------------------------------------------------------
/**
 * Pool for Packet Switched change indication reporting.
 */
//--------------------------------------------------------------------------------------------------
static le_mem_PoolRef_t PSChangePool;

//--------------------------------------------------------------------------------------------------
/**
 * This event is reported when a registration state indication is received from the modem. The
 * report data is allocated from the associated pool.
 */
//--------------------------------------------------------------------------------------------------
static le_event_Id_t NewRegStateEventId;

//--------------------------------------------------------------------------------------------------
/**
 * Pool for registration state indication reporting.
 */
//--------------------------------------------------------------------------------------------------
static le_mem_PoolRef_t NewRegStatePool;

//--------------------------------------------------------------------------------------------------
/**
 * This event is reported when a Signal Strength change is received from the modem.
 */
//--------------------------------------------------------------------------------------------------
static le_event_Id_t SignalStrengthIndEventId;

//--------------------------------------------------------------------------------------------------
/**
 * Pool for ignal Strength indication reporting.
 */
//--------------------------------------------------------------------------------------------------
static le_mem_PoolRef_t SignalStrengthIndPool;

//--------------------------------------------------------------------------------------------------
/**
 * Pool for cells information.
 */
//--------------------------------------------------------------------------------------------------
static le_mem_PoolRef_t CellInfoPool;

//--------------------------------------------------------------------------------------------------
/**
 * Last reported roaming indication
 */
//--------------------------------------------------------------------------------------------------
static nas_roaming_indicator_enum_v01 LastRoamingIndicator = NAS_ROAMING_IND_OFF_V01;

//--------------------------------------------------------------------------------------------------
/**
 * Last reported registration state.
 *
 * Used to avoid reporting the same state multiple times.
 */
//--------------------------------------------------------------------------------------------------
static le_mrc_NetRegState_t LastRegState = LE_MRC_REG_UNKNOWN;

//--------------------------------------------------------------------------------------------------
/**
 * Last reported registration error.
 *
 */
//--------------------------------------------------------------------------------------------------
static int32_t LastRegError = 0;

//--------------------------------------------------------------------------------------------------
/**
 * Last reported Radio Access Technology change.
 *
 * Used to avoid reporting the same state multiple times.
 */
//--------------------------------------------------------------------------------------------------
static le_mrc_Rat_t LastRat = LE_MRC_RAT_UNKNOWN;

//--------------------------------------------------------------------------------------------------
/**
 * Last reported Packet Switched change.
 *
 * Used to avoid reporting the same state multiple times.
 */
//--------------------------------------------------------------------------------------------------
static le_mrc_NetRegState_t LastPSState = LE_MRC_REG_UNKNOWN;

//--------------------------------------------------------------------------------------------------
/**
 * The pa_mrc_ScanInformation_t pool
 */
//--------------------------------------------------------------------------------------------------
static le_mem_PoolRef_t ScanInformationPool;

//--------------------------------------------------------------------------------------------------
/**
 * Trace reference used for controlling tracing in this module.
 */
//--------------------------------------------------------------------------------------------------
static le_log_TraceRef_t TraceRef;

//--------------------------------------------------------------------------------------------------
/**
 * The variable that contains the Registering Network information.
 */
//--------------------------------------------------------------------------------------------------
static RegisterNetwork_t RegisteringNetwork;

//--------------------------------------------------------------------------------------------------
/**
 * Mutex to prevent race condition with RegisteringNetwork.
 */
//--------------------------------------------------------------------------------------------------
static pthread_mutex_t RegisteringNetworkMutex = PTHREAD_MUTEX_INITIALIZER;

//--------------------------------------------------------------------------------------------------
/**
 * Last identified MCC/MNC
 */
//--------------------------------------------------------------------------------------------------
static uint16_t     LastMcc = 0;
static uint16_t     LastMnc = 0;

//--------------------------------------------------------------------------------------------------
/**
 * Platform RAT capabilities
 */
//--------------------------------------------------------------------------------------------------
static mode_pref_mask_type_v01 RatCapabilities;

//--------------------------------------------------------------------------------------------------
/**
 * This event is reported when a network reject indication is received from the modem.
 */
//--------------------------------------------------------------------------------------------------
static le_event_Id_t NetworkRejectIndEventId;

//--------------------------------------------------------------------------------------------------
/**
 * Pool for network reject indication reporting.
 */
//--------------------------------------------------------------------------------------------------
static le_mem_PoolRef_t NetworkRejectIndPool;

//--------------------------------------------------------------------------------------------------
/**
 * This event is reported when a jamming detection indication is received from the modem.
 */
//--------------------------------------------------------------------------------------------------
static le_event_Id_t JammingDetectionIndEventId;

//--------------------------------------------------------------------------------------------------
/**
 * Pool for jamming detection indication reporting.
 */
//--------------------------------------------------------------------------------------------------
static le_mem_PoolRef_t JammingDetectionIndPool;

//--------------------------------------------------------------------------------------------------
/**
 * Boolean to know if QMI resquest was sent to activate jamming detection.
 */
//--------------------------------------------------------------------------------------------------
#if defined(QMI_SWI_M2M_JAMMING_SET_EVENT_REPORT_REQ_V01)
static bool JammingDetectionStarted = false;
#endif

//--------------------------------------------------------------------------------------------------
/**
 * Macro used to query current trace state in this module.
 */
//--------------------------------------------------------------------------------------------------
#define IS_TRACE_ENABLED LE_IS_TRACE_ENABLED(TraceRef)

//--------------------------------------------------------------------------------------------------
/**
 * Macro used to prevent race condition with RegisteringNetwork.
 */
//--------------------------------------------------------------------------------------------------
#define LOCK()    LE_FATAL_IF((pthread_mutex_lock(&RegisteringNetworkMutex)!=0),"Could not lock the mutex")
#define UNLOCK()  LE_FATAL_IF((pthread_mutex_unlock(&RegisteringNetworkMutex)!=0),"Could not unlock the mutex")

// =============================================
//  PRIVATE FUNCTIONS
// =============================================

//--------------------------------------------------------------------------------------------------
/**
 * Kick the mrc watchdog chain
 */
//--------------------------------------------------------------------------------------------------
static void KickWatchdog
(
    void
)
{
    char threadName[MAX_THREAD_NAME_SIZE];

    le_thread_GetName(le_thread_GetCurrent(), threadName, MAX_THREAD_NAME_SIZE);

    if (strncmp(threadName, WDOG_THREAD_NAME_MRC_COMMAND_PROCESS,
                strlen(WDOG_THREAD_NAME_MRC_COMMAND_PROCESS)) == 0)
    {
        le_wdogChain_Kick(MS_WDOG_MRC_LOOP);
    }
    else
    {
        le_wdogChain_Kick(MS_WDOG_MAIN_LOOP);
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Get QMI-RAT bit mask used to perform a Network scan.
 *
 * @return LE_FAULT         The function failed.
 * @return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t GetScanRatBitMask
(
    le_mrc_RatBitMask_t             ratMask,        ///< [IN] The legato RAT bit mask
    nas_network_type_mask_type_v01* qmiNetworkPtr   ///< [OUT] The corresponding QMI-RAT bit mask
)
{
    le_result_t res = LE_OK;

    *qmiNetworkPtr = 0;

    if (ratMask == LE_MRC_BITMASK_RAT_ALL)
    {
        if (RatCapabilities & QMI_NAS_RAT_MODE_PREF_GSM_V01)
        {
            *qmiNetworkPtr |= NAS_NETWORK_TYPE_GSM_ONLY_V01;
        }
        if (RatCapabilities & QMI_NAS_RAT_MODE_PREF_UMTS_V01)
        {
            *qmiNetworkPtr |= NAS_NETWORK_TYPE_WCDMA_ONLY_V01;
        }
        if (RatCapabilities & QMI_NAS_RAT_MODE_PREF_TDSCDMA_V01)
        {
            *qmiNetworkPtr |= NAS_NETWORK_TYPE_TDSCDMA_ONLY_V01;
        }
        if (RatCapabilities & QMI_NAS_RAT_MODE_PREF_LTE_V01)
        {
            *qmiNetworkPtr |= NAS_NETWORK_TYPE_LTE_ONLY_V01;
        }
    }
    else
    {
        if (ratMask & LE_MRC_BITMASK_RAT_GSM)
        {
            if (RatCapabilities & QMI_NAS_RAT_MODE_PREF_GSM_V01)
            {
                *qmiNetworkPtr |= NAS_NETWORK_TYPE_GSM_ONLY_V01;
            }
            else
            {
                res = LE_FAULT;
            }
        }

        if (ratMask & LE_MRC_BITMASK_RAT_UMTS)
        {
            if (RatCapabilities & QMI_NAS_RAT_MODE_PREF_UMTS_V01)
            {
                *qmiNetworkPtr |= NAS_NETWORK_TYPE_WCDMA_ONLY_V01;
            }
            else
            {
                res = LE_FAULT;
            }
        }

        if (ratMask & LE_MRC_BITMASK_RAT_TDSCDMA)
        {
            if (RatCapabilities & QMI_NAS_RAT_MODE_PREF_TDSCDMA_V01)
            {
                *qmiNetworkPtr |= NAS_NETWORK_TYPE_TDSCDMA_ONLY_V01;
            }
            else
            {
                res = LE_FAULT;
            }
        }

        if (ratMask & LE_MRC_BITMASK_RAT_LTE)
        {
            if (RatCapabilities & QMI_NAS_RAT_MODE_PREF_LTE_V01)
            {
                *qmiNetworkPtr |= NAS_NETWORK_TYPE_LTE_ONLY_V01;
            }
            else
            {
                res = LE_FAULT;
            }
        }

        if (ratMask & LE_MRC_BITMASK_RAT_CDMA)
        {
            // LE_MRC_BITMASK_RAT_CDMA not available
            res = LE_FAULT;
        }
    }

    return res;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get LE_MRC RAT bit mask get from operator preferences.
 *
 * @return LE_MRC RAT bit mask
 */
//--------------------------------------------------------------------------------------------------
static uint16_t GetPrefQmiRatBitMask
(
    uint16_t qmiRatMask
)
{
    uint16_t ratMask = 0;

    if (qmiRatMask)
    {
        if (qmiRatMask & QMI_NAS_RAT_UMTS_BIT_V01)
        {
            ratMask |= LE_MRC_BITMASK_RAT_UMTS;
        }
        if (qmiRatMask & QMI_NAS_RAT_LTE_BIT_V01)
        {
            ratMask |= LE_MRC_BITMASK_RAT_LTE;
        }
        if (qmiRatMask & QMI_NAS_RAT_GSM_BIT_V01)
        {
            ratMask |= LE_MRC_BITMASK_RAT_GSM;
        }
        if (qmiRatMask & QMI_NAS_RAT_GSM_COMPACT_BIT_V01)
        {
            ratMask |= LE_MRC_BITMASK_RAT_GSM;
        }
    }
    else
    {
        ratMask = LE_MRC_BITMASK_RAT_ALL;
    }

    return ratMask;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get QMI-RAT bit mask used to set the operator preferences.
 *
 * @return QMI-RAT bit mask
 */
//--------------------------------------------------------------------------------------------------
static uint16_t GetPrefRatBitMask
(
    uint8_t ratMask
)
{
    uint16_t qmiRat = 0;

    if (ratMask&LE_MRC_BITMASK_RAT_GSM)
    {
        //    - Bit 7 -- GSM
        qmiRat |= 0x0001 << 7;
        //    - Bit 6 -- GSM compact
        qmiRat |= 0x0001 << 6;
    }
    if (ratMask&LE_MRC_BITMASK_RAT_LTE)
    {
        //    - Bit 14 -- LTE
        qmiRat |= 0x0001 << 14;
    }
    if (ratMask&LE_MRC_BITMASK_RAT_UMTS)
    {
        //    - Bit 15 -- UMTS
        qmiRat |= 0x0001 << 15;
    }

    return qmiRat;
}

//--------------------------------------------------------------------------------------------------
/**
 * Translate the Scan type from Legato enum to a QMI enum.
 *
 * @return scan state as a QMI enum
 */
//--------------------------------------------------------------------------------------------------
static nas_nw_scan_type_enum_v01 TranslateScanType
(
    pa_mrc_ScanType_t   scanType    ///< [IN] Scan Type
)
{
    switch (scanType)
    {
        case PA_MRC_SCAN_PLMN:
            return NAS_SCAN_TYPE_PLMN_V01;
        case PA_MRC_SCAN_CSG:
            return NAS_SCAN_TYPE_CSG_V01;
    }
    LE_WARN("Translation failed, use default");
    return NAS_SCAN_TYPE_PLMN_V01;
}

//--------------------------------------------------------------------------------------------------
/**
 * Initialize a pa_mrc_ScanInformation_t
 *
 */
//--------------------------------------------------------------------------------------------------
static void InitializeScanInformation
(
    pa_mrc_ScanInformation_t* scanInformationPtr    ///< [IN] the Scan information to initialize.
)
{
    memset(scanInformationPtr,0,sizeof(*scanInformationPtr));
    scanInformationPtr->link = LE_DLS_LINK_INIT;
}

//--------------------------------------------------------------------------------------------------
/**
 * Find a Scan information given the mcc and mnc.
 *
 * @return
 *      - pointer on pa_mrc_ScanInformation_t
 *      - NULL if not found
 */
//--------------------------------------------------------------------------------------------------
static pa_mrc_ScanInformation_t* FindScanInformation
(
    le_dls_List_t *scanInformationListPtr,
    char mcc[LE_MRC_MCC_BYTES],
    char mnc[LE_MRC_MNC_BYTES],
    le_mrc_Rat_t rat
)
{
    pa_mrc_ScanInformation_t* nodePtr;
    le_dls_Link_t *linkPtr;

    linkPtr = le_dls_Peek(scanInformationListPtr);

    do
    {
        if (linkPtr == NULL)
        {
            break;
        }
        // Get the node from MsgList
        nodePtr = CONTAINER_OF(linkPtr, pa_mrc_ScanInformation_t, link);

        if (    (memcmp(nodePtr->mobileCode.mcc,mcc,LE_MRC_MCC_BYTES)==0)
             && (memcmp(nodePtr->mobileCode.mnc,mnc,LE_MRC_MNC_BYTES)==0)
             && (nodePtr->rat == rat)
           )
        {
            LE_DEBUG("Found scan information for [%s,%s]",mcc,mnc);
            return nodePtr;
        }

        // Get Next element
        linkPtr = le_dls_PeekNext(scanInformationListPtr,linkPtr);

    } while (linkPtr != NULL);

    LE_DEBUG("cannot find scan information for [%s,%s]",mcc,mnc);
    return NULL;
}

//--------------------------------------------------------------------------------------------------
/**
 * Translate the network registration state from a QMI enum to Legato enum
 *
 * @return reg state as a Legato enum
 *
 * @warning qmiRoamingInd is only valid when qmiRegState is NAS_REGISTERED_V01, otherwise it should
 *          not be used.
 */
//--------------------------------------------------------------------------------------------------
static le_mrc_NetRegState_t TranslateRegState
(
    nas_registration_state_enum_v01 qmiRegState,     ///< [IN] reg state as a QMI enum
    nas_roaming_indicator_enum_v01  qmiRoamingInd    ///< [IN] roaming indication
)
{
    le_mrc_NetRegState_t regState;

    LE_DEBUG("Translate qmiRegState %d and qmiRoamingInd %d", qmiRegState,qmiRoamingInd);

    switch (qmiRegState)
    {
        case NAS_NOT_REGISTERED_V01:
            regState = LE_MRC_REG_NONE;
            break;

        case NAS_REGISTERED_V01:
            if ((NAS_ROAMING_IND_ON_V01 == qmiRoamingInd) ||
                (NAS_ROAMING_IND_FLASHING_V01 == qmiRoamingInd))
            {
                regState = LE_MRC_REG_ROAMING;
            }
            else
            {
                regState = LE_MRC_REG_HOME;
            }
            break;

        case NAS_NOT_REGISTERED_SEARCHING_V01:
            regState = LE_MRC_REG_SEARCHING;
            break;

        case NAS_REGISTRATION_DENIED_V01:
            regState = LE_MRC_REG_DENIED;
            break;

        case NAS_REGISTRATION_UNKNOWN_V01:
        default:
            regState = LE_MRC_REG_UNKNOWN;
            break;
    }

    return regState;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to convert QMI service state to Legato service state enum value
 *
 */
//--------------------------------------------------------------------------------------------------
static void ConvertPSState
(
    le_mrc_NetRegState_t  *psStatePtr,            ///< [OUT] PS service state
    nas_ps_attach_state_enum_v01 ps_attach_state  ///< [IN] QMI PS service state value
)
{
    LE_DEBUG("PS %d, LastRegState %d", ps_attach_state, LastRegState);
    switch(ps_attach_state)
    {
        case NAS_PS_ATTACHED_V01:
        {
            if(LE_MRC_REG_ROAMING == LastRegState)
            {
                *psStatePtr = LE_MRC_REG_ROAMING;
            }
            else
            {
                *psStatePtr = LE_MRC_REG_HOME;
            }

        }
        break;

        case NAS_PS_DETACHED_V01:
        {
            *psStatePtr = LE_MRC_REG_NONE;
        }
        break;

        // Unknown or not applicable
        default:
        {
            LE_WARN("PS state unknown");
            *psStatePtr = LE_MRC_REG_UNKNOWN;
        }
        break;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * This function gets the Circuit Switched and the Packet Switched Services state
 *
 * @return LE_FAULT The function failed.
 * @return LE_OK    The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t GetPSState
(
    le_mrc_NetRegState_t  * psStatePtr
)
{
    qmi_client_error_type rc;
    nas_get_serving_system_resp_msg_v01 nas_resp = { {0} };

    rc = qmi_client_send_msg_sync(NasClient,
        QMI_NAS_GET_SERVING_SYSTEM_REQ_MSG_V01,
        NULL, 0,
        &nas_resp, sizeof(nas_resp),
        COMM_LONG_PLATFORM_TIMEOUT);

    if ( swiQmi_CheckResponse(
        STRINGIZE_EXPAND(QMI_NAS_GET_SERVING_SYSTEM_REQ_MSG_V01),
        rc,
        nas_resp.resp.result,
        nas_resp.resp.error) == LE_OK )
    {
        ConvertPSState(psStatePtr, nas_resp.serving_system.ps_attach_state);
        return LE_OK;
    }

    return LE_FAULT;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function gets the Network registration state and the Roaming state.
 *
 * @return LE_FAULT The function failed.
 * @return LE_OK    The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t GetRegistrationState
(
    nas_registration_state_enum_v01* regStatePtr,  ///< [OUT]
    nas_roaming_indicator_enum_v01*  roamingIndPtr ///< [OUT]
)
{
    qmi_client_error_type rc;
    nas_get_serving_system_resp_msg_v01 nas_resp = { {0} };

    // todo: Although QMI_NAS_GET_SERVING_SYSTEM_REQ_MSG_V01 is now deprecated, it is easier to use,
    // so keep using it for the time being.
    rc = qmi_client_send_msg_sync(NasClient,
            QMI_NAS_GET_SERVING_SYSTEM_REQ_MSG_V01,
            NULL, 0,
            &nas_resp, sizeof(nas_resp),
            COMM_LONG_PLATFORM_TIMEOUT);

    if ( swiQmi_CheckResponseCode(
                STRINGIZE_EXPAND(QMI_NAS_GET_SERVING_SYSTEM_REQ_MSG_V01),
                rc,
                nas_resp.resp) != LE_OK )
    {
        return LE_FAULT;
    }

    if ( IS_TRACE_ENABLED )
    {
        LE_PRINT_VALUE("%i", nas_resp.serving_system.registration_state);

        LE_PRINT_ARRAY("%i", nas_resp.serving_system.radio_if_len, nas_resp.serving_system.radio_if);

        LE_PRINT_VALUE_IF(nas_resp.current_plmn_valid,
                          "%i", nas_resp.current_plmn.mobile_country_code);
        LE_PRINT_VALUE_IF(nas_resp.current_plmn_valid,
                          "%i", nas_resp.current_plmn.mobile_network_code);

        LE_PRINT_VALUE_IF(nas_resp.cell_id_valid, "%X", nas_resp.cell_id);
        LE_PRINT_VALUE_IF(nas_resp.lac_valid, "%X", nas_resp.lac);

        LE_PRINT_VALUE_IF(nas_resp.roaming_indicator_valid, "%X", nas_resp.roaming_indicator);
        if ( nas_resp.roaming_indicator_list_valid)
        {
            int i;
            for (i=0;i<nas_resp.roaming_indicator_list_len;i++)
            {
                LE_PRINT_VALUE("%i", nas_resp.roaming_indicator_list[i].radio_if);
                LE_PRINT_VALUE("%i", nas_resp.roaming_indicator_list[i].roaming_indicator);
            }
        }
    }

    *regStatePtr = nas_resp.serving_system.registration_state;
    if (nas_resp.roaming_indicator_valid)
    {
        *roamingIndPtr = nas_resp.roaming_indicator;
    }
    else
    {
        *roamingIndPtr = NAS_ROAMING_IND_OFF_V01;
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Convert {mobileCountryCode,mobileNetworkCode} into readable operator name
 *
 * @return
 *      - LE_OK on success
 *      - LE_OVERFLOW if the Home Network Name can't fit in nameStr
 *      - LE_FAULT on any other failure
 */
//--------------------------------------------------------------------------------------------------
static le_result_t ConvertHomeNetworkName
(
    uint16_t mobileCountryCode,  ///< [IN] Mobile country code
    uint16_t mobileNetworkCode,  ///< [IN] Mobile Network code
    char    *nameStr,            ///< [OUT] the home network Name
    size_t   nameStrSize         ///< [IN]  the nameStr size
)
{
    SWIQMI_DECLARE_MESSAGE(nas_get_plmn_name_req_msg_v01, getPlmnNameReq);
    SWIQMI_DECLARE_MESSAGE(nas_get_plmn_name_resp_msg_v01, getPlmnNameResp);

    getPlmnNameReq.plmn.mcc = mobileCountryCode;
    getPlmnNameReq.plmn.mnc = mobileNetworkCode;

    getPlmnNameReq.always_send_plmn_name_valid = 1;
    getPlmnNameReq.always_send_plmn_name = 1;

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(NasClient,
                                            QMI_NAS_GET_PLMN_NAME_REQ_MSG_V01,
                                            &getPlmnNameReq, sizeof(getPlmnNameReq),
                                            &getPlmnNameResp, sizeof(getPlmnNameResp),
                                            COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (LE_OK != swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_NAS_GET_PLMN_NAME_REQ_MSG_V01),
                                                       clientMsgErr,
                                                       getPlmnNameResp.resp.result,
                                                       getPlmnNameResp.resp.error))
    {
        LE_ERROR("Failed to convert home network name");
        return LE_FAULT;
    }

    LE_DEBUG("name valid %d, bit info valid %d, home_network valid %d",
             getPlmnNameResp.eons_plmn_name_3gpp_valid,
             getPlmnNameResp.eons_display_bit_info_valid,
             getPlmnNameResp.is_home_network_valid);

    LE_DEBUG("plmn names valid %d, info valid %d, name source valid %d",
             getPlmnNameResp.lang_plmn_names_valid,
             getPlmnNameResp.addl_info_valid,
             getPlmnNameResp.nw_name_source_valid);

    LE_DEBUG("Retrieved name for {%d;%d} : short(%s) , long(%s)",
                mobileCountryCode,
                mobileNetworkCode,
                getPlmnNameResp.eons_plmn_name_3gpp.plmn_short_name,
                getPlmnNameResp.eons_plmn_name_3gpp.plmn_long_name);

    if (!getPlmnNameResp.eons_plmn_name_3gpp_valid)
    {
        LE_ERROR("eons_plmn_name_3gpp is not valid");
        return LE_FAULT;
    }

    if (LE_OK != le_utf8_Copy(nameStr,
                              getPlmnNameResp.eons_plmn_name_3gpp.plmn_long_name,
                              nameStrSize,
                              NULL))
    {
        LE_ERROR("Home Network Name '%s' is too long",
                 getPlmnNameResp.eons_plmn_name_3gpp.plmn_long_name);
        return LE_OVERFLOW;
    }

   return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Convert {mobileCountryCode,mobileNetworkCode} into number format
 *
 * @return
 *      - LE_OK on success
 */
//--------------------------------------------------------------------------------------------------
static le_result_t ConvertMccMncIntoNumberFormat
(
    const char *mccPtr,                     ///< [IN] the mcc encoded
    const char *mncPtr,                     ///< [IN] the mnc encoded
    uint16_t   *mccNumPtr,                  ///< [OUT] the mcc to encode
    uint16_t   *mncNumPtr,                  ///< [OUT] the mnc to encode
    uint8_t    *mncIncludesPcsDigitPtr      ///< [OUT] the Include Pcs Digit
)
{
    *mccNumPtr = atoi(mccPtr);

    if (strlen(mncPtr) > 2)
    {
        *mncIncludesPcsDigitPtr = true;
    }
    else
    {
        *mncIncludesPcsDigitPtr = false;
    }
    *mncNumPtr = atoi(mncPtr);

    return LE_OK;
}



//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to know the radio access technology
 *
 * @return the rat conversion
 */
//--------------------------------------------------------------------------------------------------
static le_mrc_Rat_t ConvertRat
(
    uint8_t rat     ///< qmi radio access technology
)
{
    switch (rat)
    {
        case 0x01:  //    - 0x01 -- CDMA2000_1X
        case 0x02:  //    - 0x02 -- CDMA2000_HRPD
            return LE_MRC_RAT_CDMA;
        case 0x04:  //    - 0x04 -- GERAN
            return LE_MRC_RAT_GSM;
        case 0x05:  //    - 0x05 -- UMTS
            return LE_MRC_RAT_UMTS;
        case 0x09:  //    - 0x09 -- TD-SCDMA
            return LE_MRC_RAT_TDSCDMA;
        case 0x08:  //    - 0x08 -- LTE
            return LE_MRC_RAT_LTE;
        default:
            return LE_MRC_RAT_UNKNOWN;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to know if networkStatus is in use
 *
 * @return true if network is currently in use, false otherwise
 */
//--------------------------------------------------------------------------------------------------
static bool ConvertNetworkIsInUse
(
    uint8_t networkStatus   ///< Network Status
)
{
    //    Bits 0-1 -- QMI_NAS_NETWORK_IN_USE_ STATUS_BITS    -- In-use status       \n
    //    - 0 -- QMI_NAS_NETWORK_IN_USE_STATUS_ UNKNOWN          -- Unknown         \n
    //    - 1 -- QMI_NAS_NETWORK_IN_USE_STATUS_ CURRENT_SERVING  -- Current serving \n
    //    - 2 -- QMI_NAS_NETWORK_IN_USE_STATUS_ AVAILABLE        -- Available

    if (networkStatus&0x01)
    {
        return true;
    }
    return false;
}


//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to know if networkStatus is available
 *
 * @return true if network is currently available for connection, false otherwise.
 * @note If the network is in use, this is not available for connection.
 */
//--------------------------------------------------------------------------------------------------
static bool ConvertNetworkIsAvailable
(
    uint8_t networkStatus   ///< Network Status
)
{
    //    Bits 0-1 -- QMI_NAS_NETWORK_IN_USE_ STATUS_BITS    -- In-use status       \n
    //    - 0 -- QMI_NAS_NETWORK_IN_USE_STATUS_ UNKNOWN          -- Unknown         \n
    //    - 1 -- QMI_NAS_NETWORK_IN_USE_STATUS_ CURRENT_SERVING  -- Current serving \n
    //    - 2 -- QMI_NAS_NETWORK_IN_USE_STATUS_ AVAILABLE        -- Available

    if (networkStatus&0x02)
    {
        return true;
    }
    return false;
}


//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to know the home status
 *
 * @return true if network is in home status
 */
//--------------------------------------------------------------------------------------------------
static bool ConvertNetworkIsHome
(
    uint8_t networkStatus   ///< Network Status
)
{
    //    Bits 2-3 -- QMI_NAS_NETWORK_ROAMING_ STATUS_BITS   -- Roaming status      \n
    //    - 0 -- QMI_NAS_NETWORK_ROAMING_ STATUS_UNKNOWN         -- Unknown         \n
    //    - 1 -- QMI_NAS_NETWORK_ROAMING_ STATUS_HOME            -- Home            \n
    //    - 2 -- QMI_NAS_NETWORK_ROAMING_ STATUS_ROAM            -- Roam

    if (networkStatus&0x04)
    {
        return true;
    }
    return false;
}


//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to know the forbidden status
 *
 * @return true if network is forbiddend
 */
//--------------------------------------------------------------------------------------------------
static bool ConvertNetworkIsForbidden
(
    uint8_t networkStatus   ///< Network Status
)
{
    //    Bits 4-5 -- QMI_NAS_NETWORK_FORBIDDEN_ STATUS_BITS -- Forbidden status    \n
    //    - 0 -- QMI_NAS_NETWORK_FORBIDDEN_ STATUS_UNKNOWN       -- Unknown         \n
    //    - 1 -- QMI_NAS_NETWORK_FORBIDDEN_ STATUS_FORBIDDEN     -- Forbidden       \n
    //    - 2 -- QMI_NAS_NETWORK_FORBIDDEN_ STATUS_NOT_FORBIDDEN -- Not forbidden

    if (networkStatus&0x10)
    {
        return true;
    }

    return false;
}


//--------------------------------------------------------------------------------------------------
/**
 * Network Access Service (NAS) change handler called by the QMI NAS indications.
 */
//--------------------------------------------------------------------------------------------------
static void NasChangeHandler
(
    void*           indBufPtr,  ///< [IN] The indication message data.
    unsigned int    indBufLen,  ///< [IN] The indication message data length in bytes.
    void*           contextPtr  ///< [IN] Context value that was passed to
                                ///< swiQmi_AddIndicationHandler().
)
{
    SWIQMI_DECLARE_MESSAGE(nas_serving_system_ind_msg_v01, nasServ);
    le_mrc_NetRegState_t           netRegState = LE_MRC_REG_UNKNOWN;
    le_mrc_Rat_t                   rat = LE_MRC_RAT_UNKNOWN;
    uint32_t                       i;
    le_mrc_NetRegState_t           psState = LE_MRC_REG_UNKNOWN;

    qmi_client_error_type clientMsgErr = qmi_client_message_decode(
        NasClient, QMI_IDL_INDICATION, QMI_NAS_SERVING_SYSTEM_IND_MSG_V01,
        indBufPtr, indBufLen, &nasServ, sizeof(nasServ));

    if (clientMsgErr != QMI_NO_ERR)
    {
        LE_ERROR("Error in decoding QMI_NAS_SERVING_SYSTEM_IND_MSG_V01, error = %i", clientMsgErr);
        return;
    }

    if (nasServ.mnc_includes_pcs_digit_valid)
    {
        LE_DEBUG("Update mcc/mnc [%d,%d] to [%d,%d]",
                    LastMcc,LastMnc,
                    nasServ.mnc_includes_pcs_digit.mcc,nasServ.mnc_includes_pcs_digit.mnc);
        LastMcc = nasServ.mnc_includes_pcs_digit.mcc;
        LastMnc = nasServ.mnc_includes_pcs_digit.mnc;
    }
    else if (nasServ.current_plmn_valid)
    {
        LE_DEBUG("Update mcc/mnc [%d,%d] to [%d,%d]", LastMcc,LastMnc,
                            nasServ.current_plmn.mobile_country_code,
                            nasServ.current_plmn.mobile_network_code);
                LastMcc = nasServ.current_plmn.mobile_country_code;
                LastMnc = nasServ.current_plmn.mobile_network_code;
    }

    if(nasServ.roaming_indicator_valid)
    {
        LastRoamingIndicator = nasServ.roaming_indicator;
    }

    ConvertPSState(&psState, nasServ.serving_system.ps_attach_state);

    if(psState != LastPSState)
    {
        // Init the data for the event report
        le_mrc_NetRegState_t* newPacketSwitchedStatePtr = le_mem_ForceAlloc(PSChangePool);
        LE_DEBUG("PS %d(old %d)", psState, LastPSState);

        *newPacketSwitchedStatePtr = psState;
        LastPSState = psState;

        le_event_ReportWithRefCounting(PSChangeEventId, newPacketSwitchedStatePtr);
    }

    if(nasServ.roaming_indicator_list_valid)
    {
        int idx;

        for(idx = 0; idx < nasServ.roaming_indicator_list_len; idx++)
        {
            LE_DEBUG("Roaming Interface %d: %s",
                    nasServ.roaming_indicator_list[idx].radio_if,
                    (nasServ.roaming_indicator_list[idx].roaming_indicator
                                    == NAS_ROAMING_IND_ON_V01) ? "on" : "off" );

            /* If one interface is on roaming, set as ROAMING */
            if(nasServ.roaming_indicator_list[idx].roaming_indicator == NAS_ROAMING_IND_ON_V01)
            {
                LastRoamingIndicator = NAS_ROAMING_IND_ON_V01;
            }
        }
    }

    netRegState = TranslateRegState(nasServ.serving_system.registration_state, LastRoamingIndicator);

    LOCK();
    if (RegisteringNetwork.inProgress)
    {
        LE_DEBUG("Registration in progress for [%d,%d]",
                 RegisteringNetwork.mcc,RegisteringNetwork.mnc);
    }

    if (RegisteringNetwork.inProgress &&
        LastMcc == RegisteringNetwork.mcc &&
        LastMnc == RegisteringNetwork.mnc)
    {
        if (   (nasServ.serving_system.registration_state == NAS_REGISTERED_V01)
            || (nasServ.serving_system.registration_state == NAS_REGISTRATION_DENIED_V01) )
        {
            RegisteringNetwork.state = nasServ.serving_system.registration_state;
            UNLOCK();
            sem_post(&RegisteringNetwork.semSynchro);
            LOCK();
        }
        else
        {
            LE_DEBUG("Skipping %d in [%d,%d] registration synchro",
                netRegState,RegisteringNetwork.mcc,RegisteringNetwork.mnc);
        }
    }
    UNLOCK();

    // If network registration state is new, report it
    if(netRegState != LastRegState)
    {
        // Init the data for the event report
        le_mrc_NetRegState_t* newRegStatePtr = le_mem_ForceAlloc(NewRegStatePool);
        *newRegStatePtr = netRegState;
        LastRegState = netRegState;

        le_event_ReportWithRefCounting(NewRegStateEventId, newRegStatePtr);
    }

    // Verify RAT change
    for (i=0 ; i<nasServ.serving_system.radio_if_len ; i++)
    {
        // Get and return the first available RAT.
        rat = ConvertRat(nasServ.serving_system.radio_if[i]);
    }

    // If RAT is new, report it
    if ((rat != LastRat) && (rat != LE_MRC_RAT_UNKNOWN))
    {
        // Init the data for the event report
        le_mrc_Rat_t* ratPtr = le_mem_ForceAlloc(RatChangePool);
        *ratPtr = rat;
        LastRat = rat;

        le_event_ReportWithRefCounting(RatChangeEventId, ratPtr);
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Signal Strength change handler called by the QMI NAS indications.
 */
//--------------------------------------------------------------------------------------------------
static void SignalStrengthChangeHandler
(
    void*           indBufPtr,  // [IN] The indication message data.
    unsigned int    indBufLen,  // [IN] The indication message data length in bytes.
    void*           contextPtr  // [IN] Context value that was passed to
                                //      swiQmi_AddIndicationHandler().
)
{
    nas_sig_info_ind_msg_v01 nasInd;
    pa_mrc_SignalStrengthIndication_t* ssIndPtr;

    qmi_client_error_type clientMsgErr = qmi_client_message_decode(
        NasClient, QMI_IDL_INDICATION, QMI_NAS_SIG_INFO_IND_MSG_V01,
        indBufPtr, indBufLen,
        &nasInd, sizeof(nasInd));

    if (clientMsgErr != QMI_NO_ERR)
    {
        LE_ERROR("Error in decoding QMI_NAS_SIG_INFO_IND_MSG_V01, error = %i", clientMsgErr);
        return;
    }

    if ((nasInd.cdma_sig_info_valid) || (nasInd.hdr_sig_info_valid))
    {
        ssIndPtr = le_mem_ForceAlloc(SignalStrengthIndPool);
        ssIndPtr->rat = LE_MRC_RAT_CDMA;
        if (nasInd.cdma_sig_info_valid)
        {
            ssIndPtr->ss = nasInd.cdma_sig_info.rssi;
        }
        else
        {
            ssIndPtr->ss = nasInd.hdr_sig_info.common_sig_str.rssi;
        }
        LE_DEBUG("SignalStrengthChangeHandler report rat.%d and ss.%d",
                                           ssIndPtr->rat, ssIndPtr->ss);
        le_event_ReportWithRefCounting(SignalStrengthIndEventId, ssIndPtr);
    }

    if (nasInd.gsm_sig_info_valid)
    {
        ssIndPtr = le_mem_ForceAlloc(SignalStrengthIndPool);
        ssIndPtr->rat = LE_MRC_RAT_GSM;
        ssIndPtr->ss = nasInd.gsm_sig_info;
        LE_DEBUG("SignalStrengthChangeHandler report rat.%d and ss.%d",
                                           ssIndPtr->rat, ssIndPtr->ss);
        le_event_ReportWithRefCounting(SignalStrengthIndEventId, ssIndPtr);
    }

    if (nasInd.wcdma_sig_info_valid)
    {
        ssIndPtr = le_mem_ForceAlloc(SignalStrengthIndPool);
        ssIndPtr->rat = LE_MRC_RAT_UMTS;
        ssIndPtr->ss = nasInd.wcdma_sig_info.rssi;
        LE_DEBUG("SignalStrengthChangeHandler report rat.%d and ss.%d",
                                           ssIndPtr->rat, ssIndPtr->ss);
        le_event_ReportWithRefCounting(SignalStrengthIndEventId, ssIndPtr);
    }

    if (nasInd.tdscdma_sig_info_valid)
    {
        ssIndPtr = le_mem_ForceAlloc(SignalStrengthIndPool);
        ssIndPtr->rat = LE_MRC_RAT_TDSCDMA;
        ssIndPtr->ss = nasInd.tdscdma_sig_info.rssi;
        LE_DEBUG("SignalStrengthChangeHandler report rat.%d and ss.%d",
                                           ssIndPtr->rat, ssIndPtr->ss);
        le_event_ReportWithRefCounting(SignalStrengthIndEventId, ssIndPtr);
    }

    if (nasInd.lte_sig_info_valid)
    {
        ssIndPtr = le_mem_ForceAlloc(SignalStrengthIndPool);
        ssIndPtr->rat = LE_MRC_RAT_LTE;
        ssIndPtr->ss = nasInd.lte_sig_info.rssi;
        LE_DEBUG("SignalStrengthChangeHandler report rat.%d and ss.%d",
                                           ssIndPtr->rat, ssIndPtr->ss);
        le_event_ReportWithRefCounting(SignalStrengthIndEventId, ssIndPtr);
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Network reject indication handler called by the QMI NAS indications.
 */
//--------------------------------------------------------------------------------------------------
#if defined(QMI_NAS_NETWORK_REJECT_IND_V01)
static void NetworkRejectIndHandler
(
    void*           indBufPtr,  // [IN] The indication message data.
    unsigned int    indBufLen,  // [IN] The indication message data length in bytes.
    void*           contextPtr  // [IN] Context value that was passed to
                                //      swiQmi_AddIndicationHandler().
)
{
    SWIQMI_DECLARE_MESSAGE(nas_network_reject_ind_msg_v01, nasInd);
    pa_mrc_NetworkRejectIndication_t* networkRejectIndPtr;

    qmi_client_error_type clientMsgErr = qmi_client_message_decode(
        NasClient, QMI_IDL_INDICATION, QMI_NAS_NETWORK_REJECT_IND_V01,
        indBufPtr, indBufLen,
        &nasInd, sizeof(nasInd));

    if (clientMsgErr != QMI_NO_ERR)
    {
        LE_ERROR("Error in decoding QMI_NAS_NETWORK_REJECT_IND_V01, error = %i", clientMsgErr);
        return;
    }

    networkRejectIndPtr = le_mem_ForceAlloc(NetworkRejectIndPool);
    memset(networkRejectIndPtr, 0, sizeof(pa_mrc_NetworkRejectIndication_t));

    networkRejectIndPtr->rat = nasInd.radio_if;
    if (nasInd.plmn_id_valid)
    {
        ConvertMccMncIntoStringFormat(
                nasInd.plmn_id.mcc,
                nasInd.plmn_id.mnc,
                true,
                networkRejectIndPtr->mcc,
                networkRejectIndPtr->mnc);

        LE_DEBUG("NetworkRejectIndHandler report rat.%d mcc.%s mnc.%s",
                  networkRejectIndPtr->rat, networkRejectIndPtr->mcc, networkRejectIndPtr->mnc);
    }

    le_event_ReportWithRefCounting(NetworkRejectIndEventId, networkRejectIndPtr);
}
#endif

//--------------------------------------------------------------------------------------------------
/**
 * Nas Event report handler called by the QMI NAS indications.
 */
//--------------------------------------------------------------------------------------------------
static void EventReportHandler
(
    void*           indBufPtr,  // [IN] The indication message data.
    unsigned int    indBufLen,  // [IN] The indication message data length in bytes.
    void*           contextPtr  // [IN] Context value that was passed to
                                //      swiQmi_AddIndicationHandler().
)
{
    nas_event_report_ind_msg_v01 nasInd;

    qmi_client_error_type clientMsgErr = qmi_client_message_decode(
        NasClient, QMI_IDL_INDICATION, QMI_NAS_EVENT_REPORT_IND_MSG_V01,
        indBufPtr, indBufLen,
        &nasInd, sizeof(nasInd));

    if(clientMsgErr != QMI_NO_ERR)
    {
        LE_ERROR("Error in decoding QMI_NAS_EVENT_REPORT_IND_MSG_V01, error = %i", clientMsgErr);
        return;
    }

    if(nasInd.registration_reject_reason_valid)
    {
        LastRegError = nasInd.registration_reject_reason.reject_cause;
        LE_ERROR("Network Registration failed error code %d", LastRegError);
    }
}


//--------------------------------------------------------------------------------------------------
/**
 * The first-layer Signal Strength Change Handler.
 */
//--------------------------------------------------------------------------------------------------
static void FirstLayerSignalStrengthIndHandler
(
    void*   reportPtr,
    void*   secondLayerFunc
)
{
    pa_mrc_SignalStrengthIndication_t* ssIndPtr = (pa_mrc_SignalStrengthIndication_t*) reportPtr;
    pa_mrc_SignalStrengthIndHdlrFunc_t handlerFunc =
                                            (pa_mrc_SignalStrengthIndHdlrFunc_t) secondLayerFunc;

    handlerFunc(ssIndPtr);
}

//--------------------------------------------------------------------------------------------------
/**
 * Get Radio Access Technology capabilities of the platform
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on any other failure
 */
//--------------------------------------------------------------------------------------------------
static le_result_t GetRatCapabilities
(
    mode_pref_mask_type_v01*   ratMaskPtr   ///< [OUT] the home network Name
)
{

    SWIQMI_DECLARE_MESSAGE(dms_get_device_cap_resp_msg_v01, getResp);

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(DmsClient,
                                            QMI_DMS_GET_DEVICE_CAP_REQ_V01,
                                            NULL, 0,
                                            &getResp, sizeof(getResp),
                                            COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (LE_OK != swiQmi_CheckResponse(
                    STRINGIZE_EXPAND(QMI_DMS_GET_DEVICE_CAP_REQ_V01),
                    clientMsgErr,
                    getResp.resp.result,
                    getResp.resp.error))
    {
        *ratMaskPtr = 0xFFFF;
        LE_ERROR("Failed to get the RAT capabilities");
        return LE_FAULT;
    }

    *ratMaskPtr = 0;
    if (getResp.device_capabilities.radio_if_list_len)
    {
        int i;
        for (i=0; i<getResp.device_capabilities.radio_if_list_len; i++)
        {
            switch(getResp.device_capabilities.radio_if_list[i])
            {
                case DMS_RADIO_IF_1X_V01: // CDMA2000 1X
                    *ratMaskPtr |= QMI_NAS_RAT_MODE_PREF_CDMA2000_1X_V01;
                    LE_DEBUG("CDMA2000 1X is supported");
                    break;

                case DMS_RADIO_IF_1X_EVDO_V01: // CDMA2000 HRPD (1xEV-DO)
                    *ratMaskPtr |= QMI_NAS_RAT_MODE_PREF_CDMA2000_HRPD_V01;
                    LE_DEBUG("CDMA2000 HRPD is supported");
                    break;

                case DMS_RADIO_IF_GSM_V01: // GSM
                    *ratMaskPtr |= QMI_NAS_RAT_MODE_PREF_GSM_V01;
                    LE_DEBUG("GSM is supported");
                    break;

                case DMS_RADIO_IF_UMTS_V01: // UMTS
                    *ratMaskPtr |= QMI_NAS_RAT_MODE_PREF_UMTS_V01;
                    LE_DEBUG("UMTS is supported");
                    break;

                case DMS_RADIO_IF_LTE_V01: // LTE
                    *ratMaskPtr |= QMI_NAS_RAT_MODE_PREF_LTE_V01;
                    LE_DEBUG("LTE is supported");
                    break;

                case DMS_RADIO_IF_TDS_V01: // TDS
                    *ratMaskPtr |= QMI_NAS_RAT_MODE_PREF_TDSCDMA_V01;
                    LE_DEBUG("TD-SCDMA is supported");
                    break;

                default:
                    break;
            }
        }
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * The first-layer jamming detection indication handler.
 */
//--------------------------------------------------------------------------------------------------
#if defined(QMI_SWI_M2M_JAMMING_IND_V01)
static void FirstLayerJammingDetectionIndHandler
(
    void*   reportPtr,
    void*   secondLayerFunc
)
{
    if (NULL == reportPtr)
    {
        LE_ERROR("reportPtr is NULL");
        return;
    }

    if (NULL == secondLayerFunc)
    {
        LE_ERROR("secondLayerFunc is NULL");
        return;
    }

    pa_mrc_JammingDetectionIndication_t* jammingDetectionIndPtr =
                                      (pa_mrc_JammingDetectionIndication_t*) reportPtr;

    pa_mrc_JammingDetectionHandlerFunc_t handlerFunc =
                                      (pa_mrc_JammingDetectionHandlerFunc_t) secondLayerFunc;

    handlerFunc(jammingDetectionIndPtr);
}
#endif

//--------------------------------------------------------------------------------------------------
/**
 * Jamming detection event report handler called by the QMI DMS indications.
 */
//--------------------------------------------------------------------------------------------------
#if defined(QMI_SWI_M2M_JAMMING_IND_V01)
static void JammingEventReportHandler
(
    void*           indBufPtr,  ///< [IN] The indication message data.
    unsigned int    indBufLen,  ///< [IN] The indication message data length in bytes.
    void*           contextPtr  ///< [IN] Context value that was passed to
                                ///<      swiQmi_AddIndicationHandler().
)
{
    swi_m2m_jamming_ind_msg_v01 jammingInd;
    pa_mrc_JammingDetectionIndication_t* JammingDetectionIndPtr;

    qmi_client_error_type clientMsgErr = qmi_client_message_decode(MgsClient,
                                                                   QMI_IDL_INDICATION,
                                                                   QMI_SWI_M2M_JAMMING_IND_V01,
                                                                   indBufPtr,
                                                                   indBufLen,
                                                                   &jammingInd,
                                                                   sizeof(jammingInd));

    if(QMI_NO_ERR != clientMsgErr)
    {
        LE_ERROR("Error in decoding QMI_SWI_M2M_JAMMING_IND_V01, error = %i", clientMsgErr);
        return;
    }

    JammingDetectionIndPtr = le_mem_ForceAlloc(JammingDetectionIndPool);
    JammingDetectionIndPtr->report = jammingInd.jamming_response_type;
    JammingDetectionIndPtr->status = jammingInd.jamming_status;

    le_event_ReportWithRefCounting(JammingDetectionIndEventId, JammingDetectionIndPtr);
}
#endif

// =============================================
//  MODULE/COMPONENT FUNCTIONS
// =============================================

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to initialize the QMI MRC module.
 *
 * @return
 *      LE_OK if successful.
 *      LE_FAULT if unsuccessful.
 */
//--------------------------------------------------------------------------------------------------
le_result_t mrc_qmi_Init
(
    void
)
{
    // Get a reference to the trace keyword that is used to control tracing in this module.
    TraceRef = le_log_GetTraceRef("mrc");

    RegisteringNetwork.inProgress = false;
    RegisteringNetwork.state = LE_MRC_REG_UNKNOWN;
    sem_init(&RegisteringNetwork.semSynchro,0,0);

    // Create the event for signaling user handlers.
    NewRegStateEventId = le_event_CreateIdWithRefCounting("NewRegStateEvent");
    NewRegStatePool = le_mem_CreatePool("NewRegStatePool", sizeof(le_mrc_NetRegState_t));
    RatChangeEventId = le_event_CreateIdWithRefCounting("RatChangeEvent");
    RatChangePool = le_mem_CreatePool("RatChangePool", sizeof(le_mrc_Rat_t));
    SignalStrengthIndEventId = le_event_CreateIdWithRefCounting("SignalStrengthIndEvent");
    SignalStrengthIndPool = le_mem_CreatePool("SignalStrengthIndPool",
                                              sizeof(pa_mrc_SignalStrengthIndication_t));

    ScanInformationPool=le_mem_CreatePool("ScanInformationPool", sizeof(pa_mrc_ScanInformation_t));

    PSChangeEventId = le_event_CreateIdWithRefCounting("PSChangeEvent");
    PSChangePool = le_mem_CreatePool("PSChangePool", sizeof(le_mrc_NetRegState_t));

    // Create the pool for cells information.
    CellInfoPool = le_mem_CreatePool("CellInfoPool", sizeof(pa_mrc_CellInfo_t));

    if ( (swiQmi_InitServices(QMI_SERVICE_NAS) != LE_OK) ||
         (swiQmi_InitServices(QMI_SERVICE_DMS) != LE_OK) ||
         (swiQmi_InitServices(QMI_SERVICE_SWI_SAR) != LE_OK) ||
         (swiQmi_InitServices(QMI_SERVICE_MGS) != LE_OK))
    {
        LE_CRIT("Some QMI Services cannot be initialized.");
        return LE_FAULT;
    }

    // Get the qmi client service handle for later use.
    if ( ((NasClient = swiQmi_GetClientHandle(QMI_SERVICE_NAS)) == NULL) ||
         ((DmsClient = swiQmi_GetClientHandle(QMI_SERVICE_DMS)) == NULL) ||
         ((SwiSarClient = swiQmi_GetClientHandle(QMI_SERVICE_SWI_SAR)) == NULL) ||
         ((MgsClient = swiQmi_GetClientHandle(QMI_SERVICE_MGS)) == NULL))
    {
        return LE_FAULT;
    }

    // Register for signal strength and network reject notifications
    nas_indication_register_req_msg_v01 indRegReq = {0};
    nas_indication_register_resp_msg_v01 indRegResp = { {0} };

    indRegReq.sig_info_valid = true;
    indRegReq.sig_info = 0x01;

    indRegReq.network_reject_valid = true;
    indRegReq.network_reject.reg_network_reject = 0x01;
    indRegReq.network_reject.suppress_sys_info = 0x01;

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(NasClient,
                                            QMI_NAS_INDICATION_REGISTER_REQ_MSG_V01,
                                            &indRegReq, sizeof(indRegReq),
                                            &indRegResp, sizeof(indRegResp),
                                            COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (swiQmi_CheckResponseCode(
                                STRINGIZE_EXPAND(QMI_NAS_INDICATION_REGISTER_REQ_MSG_V01),
                                clientMsgErr,
                                indRegResp.resp) != LE_OK)
    {
        LE_WARN("Cannot register for registration error notifications!");
    }

    // Register for NAS event report notifications
    nas_set_event_report_req_msg_v01 eventRegReq = {0};
    nas_set_event_report_resp_msg_v01 eventRegResp = { {0} };

    eventRegReq.report_reg_reject_valid = true;
    eventRegReq.report_reg_reject = 0x01;

    clientMsgErr = qmi_client_send_msg_sync(NasClient,
        QMI_NAS_SET_EVENT_REPORT_REQ_MSG_V01,
        &eventRegReq, sizeof(eventRegReq),
        &eventRegResp, sizeof(eventRegResp),
        COMM_DEFAULT_PLATFORM_TIMEOUT);

    if( swiQmi_CheckResponseCode(
        STRINGIZE_EXPAND(QMI_NAS_SET_EVENT_REPORT_REQ_MSG_V01),
        clientMsgErr,
        eventRegResp.resp) != LE_OK)
    {
        LE_WARN("Cannot register for Signal Strength notifications!");
    }

    // Register our own handler with the QMI NAS service.
    swiQmi_AddIndicationHandler(NasChangeHandler,
                                QMI_SERVICE_NAS,
                                QMI_NAS_SERVING_SYSTEM_IND_MSG_V01,
                                NULL);

    swiQmi_AddIndicationHandler(SignalStrengthChangeHandler,
                                QMI_SERVICE_NAS,
                                QMI_NAS_SIG_INFO_IND_MSG_V01,
                                NULL);

    swiQmi_AddIndicationHandler(EventReportHandler,
                                QMI_SERVICE_NAS,
                                QMI_NAS_EVENT_REPORT_IND_MSG_V01,
                                NULL);

#if defined(QMI_NAS_NETWORK_REJECT_IND_V01)
    swiQmi_AddIndicationHandler(NetworkRejectIndHandler,
                                QMI_SERVICE_NAS,
                                QMI_NAS_NETWORK_REJECT_IND_V01,
                                NULL);
#endif

    NetworkRejectIndEventId = le_event_CreateIdWithRefCounting("NetworkRejectIndEvent");
    NetworkRejectIndPool = le_mem_CreatePool("NetworkRejectIndPool",
                                              sizeof(pa_mrc_NetworkRejectIndication_t));

    GetRatCapabilities(&RatCapabilities);

    GetPSState(&LastPSState);

    // Register our own handler with the QMI MGS service.
#if defined(QMI_SWI_M2M_JAMMING_IND_V01)
    swiQmi_AddIndicationHandler(JammingEventReportHandler,
                                QMI_SERVICE_MGS,
                                QMI_SWI_M2M_JAMMING_IND_V01,
                                NULL);
#endif

    JammingDetectionIndEventId = le_event_CreateIdWithRefCounting("JammingDetectionIndEvent");

    JammingDetectionIndPool = le_mem_CreatePool("JammingDetectionIndPool",
                                                sizeof(pa_mrc_JammingDetectionIndication_t));

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to get the Home Network MCC and MNC.
 *
 * @return
 *      - LE_OK        On success
 *      - LE_NOT_FOUND If Home Network has not been provisioned
 *      - LE_FAULT     Unexpected error
 */
//--------------------------------------------------------------------------------------------------
le_result_t mrc_qmi_GetHomeNetworkMccMnc
(
    uint16_t* mccPtr,             ///< the home network MCC
    uint16_t* mncPtr,             ///< the home network MNC
    bool*     isMncIncPcsDigitPtr ///< true if MNC is a three-digit value,
                                  ///  false if MNC is a two-digit value
)
{
   SWIQMI_DECLARE_MESSAGE(nas_get_home_network_resp_msg_v01, getHomeNetworkResp);

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(NasClient,
                                            QMI_NAS_GET_HOME_NETWORK_REQ_MSG_V01,
                                            NULL, 0,
                                            &getHomeNetworkResp, sizeof(getHomeNetworkResp),
                                            COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (LE_OK != swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_NAS_GET_HOME_NETWORK_REQ_MSG_V01),
                                      clientMsgErr,
                                      getHomeNetworkResp.resp.result,
                                      getHomeNetworkResp.resp.error))
    {
        LE_ERROR("Failed to get MCC and MNC");

        if (QMI_ERR_NOT_PROVISIONED_V01 == getHomeNetworkResp.resp.error)
        {
            LE_ERROR("Home Network has not been provisioned");
            return LE_NOT_FOUND;
        }
        return LE_FAULT;
    }

    LE_DEBUG("home valid %d, home_ext valid %d, digit valid %d, source valid %d",
             getHomeNetworkResp.home_system_id_valid,
             getHomeNetworkResp.nas_3gpp2_home_network_ext_valid,
             getHomeNetworkResp.nas_3gpp_mcs_include_digit_valid,
             getHomeNetworkResp.nas_3gpp_nw_name_source_valid);

    if (getHomeNetworkResp.home_system_id_valid)
    {
        LE_DEBUG("sid %d, nid %d",
                 getHomeNetworkResp.home_system_id.sid,
                 getHomeNetworkResp.home_system_id.nid);
    }

    if (getHomeNetworkResp.nas_3gpp2_home_network_ext_valid)
    {
        LE_DEBUG("mcc %d, mnc %d",
                 getHomeNetworkResp.nas_3gpp2_home_network_ext.mcc_mnc.mcc,
                 getHomeNetworkResp.nas_3gpp2_home_network_ext.mcc_mnc.mnc);
    }

    if (getHomeNetworkResp.nas_3gpp_nw_name_source_valid)
    {
        LE_DEBUG("source %d", getHomeNetworkResp.nas_3gpp_nw_name_source);
    }

    if ( getHomeNetworkResp.nas_3gpp_mcs_include_digit_valid)
    {
        *isMncIncPcsDigitPtr = getHomeNetworkResp.nas_3gpp_mcs_include_digit.mnc_includes_pcs_digit;
    }

    LE_DEBUG("network name or description %s mcc %d, mnc %d",
             getHomeNetworkResp.home_network.network_description,
             getHomeNetworkResp.home_network.mobile_country_code,
             getHomeNetworkResp.home_network.mobile_network_code);

    *mccPtr = getHomeNetworkResp.home_network.mobile_country_code;
    *mncPtr = getHomeNetworkResp.home_network.mobile_network_code;
    return LE_OK;
}


//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to get the Home Network Name information.
 *
 * @return
 *      - LE_OK on success
 *      - LE_OVERFLOW if the Home Network Name can't fit in nameStr
 *      - LE_FAULT on any other failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t mrc_qmi_GetHomeNetworkName
(
    char       *nameStr,               ///< [OUT] the home network Name
    size_t      nameStrSize            ///< [IN]  the nameStr size
)
{
    uint16_t    mcc;
    uint16_t    mnc;
    bool        isMncIncPcsDigit; // not used here
    le_result_t result;

    result = mrc_qmi_GetHomeNetworkMccMnc(&mcc, &mnc , &isMncIncPcsDigit);
    if (LE_OK == result)
    {
        return (ConvertHomeNetworkName(mcc, mnc, nameStr, nameStrSize));
    }

    if (LE_NOT_FOUND == result)
    {
        LE_ERROR("Home Network Name has not been provisioned");
        le_utf8_Copy(nameStr,"None",nameStrSize, NULL);
    }
    return LE_FAULT;
}

// =============================================
//  PUBLIC API FUNCTIONS
// =============================================


//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to set the power of the Radio Module.
 *
 * @return LE_FAULT  The function failed.
 * @return LE_OK     The function succeed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_SetRadioPower
(
    le_onoff_t    power   ///< [IN] The power state.
)
{
    dms_set_operating_mode_req_msg_v01 opModeReq = {0};
    dms_set_operating_mode_resp_msg_v01 opModeResp = { {0} };

    if (power == LE_ON)
    {
        opModeReq.operating_mode = DMS_OP_MODE_ONLINE_V01;
    }
    else if (power == LE_OFF)
    {
        opModeReq.operating_mode = DMS_OP_MODE_PERSISTENT_LOW_POWER_V01;
    }
    else
    {
        return LE_FAULT;
    }


    // note: Changing from online to low power seems to take some time even after the
    //       qmi_client_send_msg_sync() function returns.
    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(DmsClient,
                                            QMI_DMS_SET_OPERATING_MODE_REQ_V01,
                                            &opModeReq, sizeof(opModeReq),
                                            &opModeResp, sizeof(opModeResp),
                                            COMM_LONG_PLATFORM_TIMEOUT);

    return swiQmi_CheckResponseCode(
                                STRINGIZE_EXPAND(QMI_DMS_SET_OPERATING_MODE_REQ_V01),
                                clientMsgErr,
                                opModeResp.resp);
}


//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to get the Radio Module power state.
 *
 * @return LE_FAULT  The function failed.
 * @return LE_OK     The function succeed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_GetRadioPower
(
     le_onoff_t*    power   ///< [OUT] The power state.
)
{
    SWIQMI_DECLARE_MESSAGE(dms_get_operating_mode_resp_msg_v01, opModeResp);

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(DmsClient,
                                            QMI_DMS_GET_OPERATING_MODE_REQ_V01,
                                            NULL, 0,
                                            &opModeResp, sizeof(opModeResp),
                                            COMM_DEFAULT_PLATFORM_TIMEOUT);

    le_result_t respResult = swiQmi_CheckResponseCode(
                                STRINGIZE_EXPAND(QMI_DMS_GET_OPERATING_MODE_REQ_V01),
                                clientMsgErr,
                                opModeResp.resp);

    if (respResult == LE_OK)
    {
        if (opModeResp.operating_mode == DMS_OP_MODE_ONLINE_V01)
        {
            *power = LE_ON;
        }
        else
        {
            *power = LE_OFF;
        }

        return LE_OK;
    }

    return LE_FAULT;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to register a handler for Radio Access Technology change handling.
 *
 * @return A handler reference, which is only needed for later removal of the handler.
 *
 * @note Doesn't return on failure, so there's no need to check the return value for errors.
 */
//--------------------------------------------------------------------------------------------------
le_event_HandlerRef_t pa_mrc_SetRatChangeHandler
(
    pa_mrc_RatChangeHdlrFunc_t handlerFuncPtr ///< [IN] The handler function.
)
{
    LE_ASSERT(handlerFuncPtr != NULL);

    return le_event_AddHandler("RatChangeHandler",
                               RatChangeEventId,
                               (le_event_HandlerFunc_t) handlerFuncPtr);
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to unregister the handler for Radio Access Technology change
 * handling.
 *
 * @note Doesn't return on failure, so there's no need to check the return value for errors.
 */
//--------------------------------------------------------------------------------------------------
void pa_mrc_RemoveRatChangeHandler
(
    le_event_HandlerRef_t handlerRef
)
{
    le_event_RemoveHandler(handlerRef);
}


//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to register a handler for Packet Switched change handling.
 *
 * @return A handler reference, which is only needed for later removal of the handler.
 *
 * @note Doesn't return on failure, so there's no need to check the return value for errors.
 */
//--------------------------------------------------------------------------------------------------
le_event_HandlerRef_t pa_mrc_SetPSChangeHandler
(
    pa_mrc_ServiceChangeHdlrFunc_t handlerFuncPtr ///< [IN] The handler function.
)
{
    LE_ASSERT(handlerFuncPtr != NULL);

    return le_event_AddHandler("PSChangeHandler",
                               PSChangeEventId,
                               (le_event_HandlerFunc_t) handlerFuncPtr);
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to unregister the handler for Packet Switched change
 * handling.
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_mrc_RemovePSChangeHandler
(
    le_event_HandlerRef_t handlerRef
)
{
    le_event_RemoveHandler(handlerRef);
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to register a handler for Network registration state handling.
 *
 * @return A handler reference, which is only needed for later removal of the handler.
 *
 * @note Doesn't return on failure, so there's no need to check the return value for errors.
 */
//--------------------------------------------------------------------------------------------------
le_event_HandlerRef_t pa_mrc_AddNetworkRegHandler
(
    pa_mrc_NetworkRegHdlrFunc_t regStateHandler ///< [IN] The handler function to handle the
                                                ///        Network registration state.
)
{
    LE_ASSERT(regStateHandler != NULL);

    return le_event_AddHandler(
                "NewRegStateHandler",
                NewRegStateEventId,
                (le_event_HandlerFunc_t) regStateHandler);
}


//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to unregister the handler for Network registration state handling.
 *
 * @note Doesn't return on failure, so there's no need to check the return value for errors.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_RemoveNetworkRegHandler
(
    le_event_HandlerRef_t handlerRef
)
{
    le_event_RemoveHandler(handlerRef);

    // todo: This function does not need a return value !!
    return LE_OK;
}


//--------------------------------------------------------------------------------------------------
/**
 * This function configures the Network registration setting.
 *
 * @return LE_FAULT         The function failed.
 * @return LE_TIMEOUT       No response was received.
 * @return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_ConfigureNetworkReg
(
    pa_mrc_NetworkRegSetting_t  setting ///< [IN] The selected Network registration setting.
)
{
    // todo: implement

    return LE_OK;
}


//--------------------------------------------------------------------------------------------------
/**
 * This function gets the Network registration setting.
 *
 *
 * @return
 *      \return LE_FAULT        The function failed to get the Network registration setting.
 *      \return LE_TIMEOUT      No response was received from the Modem.
 *      \return LE_OK           The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_GetNetworkRegConfig
(
    pa_mrc_NetworkRegSetting_t*  setting   ///< [OUT] The selected Network registration setting.
)
{
    // todo: implement

    *setting = PA_MRC_ENABLE_REG_LOC_NOTIFICATION;
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the platform specific network registration error code.
 *
 * @return the platform specific registration error code
 *
 */
//--------------------------------------------------------------------------------------------------
int32_t pa_mrc_GetPlatformSpecificRegistrationErrorCode
(
    void
)
{
    return LastRegError;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function gets the Network registration state.
 *
 *
 * @return
 *      \return LE_FAULT        The function failed to get the Network registration state.
 *      \return LE_TIMEOUT      No response was received from the Modem.
 *      \return LE_OK           The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_GetNetworkRegState
(
    le_mrc_NetRegState_t*  regStatePtr    ///< [OUT] The network registration state.
)
{
    le_result_t result;
    nas_registration_state_enum_v01 qmiRegState;
    nas_roaming_indicator_enum_v01  qmiRoamingInd;

    LE_INFO("called");

    result = GetRegistrationState(&qmiRegState,&qmiRoamingInd);
    if (result != LE_OK)
        return result;

    *regStatePtr = TranslateRegState(qmiRegState,qmiRoamingInd);

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function measures the Signal metrics.
 *
 * @return LE_FAULT         The function failed.
 * @return LE_OK            The function succeeded.
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_MeasureSignalMetrics
(
    pa_mrc_SignalMetrics_t* metricsPtr    ///< [OUT] The signal metrics.
)
{
    qmi_client_error_type clientMsgErr;
    SWIQMI_DECLARE_MESSAGE(nas_get_sig_info_resp_msg_v01, getSigResp);

    clientMsgErr = qmi_client_send_msg_sync(NasClient,
                                            QMI_NAS_GET_SIG_INFO_REQ_MSG_V01,
                                            NULL, 0,
                                            &getSigResp, sizeof(getSigResp),
                                            COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (LE_OK != swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_NAS_GET_SIG_INFO_REQ_MSG_V01),
                                      clientMsgErr,
                                      getSigResp.resp.result,
                                      getSigResp.resp.error))
    {
        return LE_FAULT;
    }

    LE_DEBUG("Valid info: cdma.%d, hdr.%d, gsm.%d, wcdma.%d, lte.%d, rscp.%d, tdscdma.%d",
             getSigResp.cdma_sig_info_valid, getSigResp.hdr_sig_info_valid,
             getSigResp.gsm_sig_info_valid, getSigResp.wcdma_sig_info_valid,
             getSigResp.lte_sig_info_valid, getSigResp.rscp_valid,
             getSigResp.tdscdma_sig_info_valid);

    if (getSigResp.gsm_sig_info_valid)
    {
        metricsPtr->rat = LE_MRC_RAT_GSM;
        metricsPtr->ss = getSigResp.gsm_sig_info;
    }
    else if (getSigResp.wcdma_sig_info_valid)
    {
        SWIQMI_DECLARE_MESSAGE(nas_get_cell_location_info_resp_msg_v01, getLocResp);

        metricsPtr->rat = LE_MRC_RAT_UMTS;
        metricsPtr->ss = getSigResp.wcdma_sig_info.rssi;
        metricsPtr->umtsMetrics.ecio = -((getSigResp.wcdma_sig_info.ecio*10)/2);

        clientMsgErr = qmi_client_send_msg_sync(NasClient,
                                                QMI_NAS_GET_CELL_LOCATION_INFO_REQ_MSG_V01,
                                                NULL, 0,
                                                &getLocResp, sizeof(getLocResp),
                                                COMM_NETWORK_TIMEOUT);

        if (LE_OK == swiQmi_CheckResponse(
                                    STRINGIZE_EXPAND(QMI_NAS_GET_CELL_LOCATION_INFO_REQ_MSG_V01),
                                    clientMsgErr,
                                    getLocResp.resp.result,
                                    getLocResp.resp.error))
        {
            LE_DEBUG("Valid info: umts_ext.%d", getLocResp.umts_ext_info_valid);

            if (getLocResp.umts_ext_info_valid)
            {
                metricsPtr->umtsMetrics.rscp = getLocResp.umts_ext_info.rscp;
            }
            else
            {
                metricsPtr->umtsMetrics.rscp = INT32_MAX;
            }
        }
        else
        {
            metricsPtr->umtsMetrics.rscp = INT32_MAX;
        }
    }
    else if (getSigResp.tdscdma_sig_info_valid)
    {
        metricsPtr->rat = LE_MRC_RAT_TDSCDMA;
        metricsPtr->ss = getSigResp.tdscdma_sig_info.rssi;
        metricsPtr->tdscdmaMetrics.ecio = getSigResp.tdscdma_sig_info.ecio*10;
        metricsPtr->tdscdmaMetrics.rscp = getSigResp.tdscdma_sig_info.rscp;
        metricsPtr->tdscdmaMetrics.sinr = getSigResp.tdscdma_sig_info.sinr;
    }
    else if (getSigResp.lte_sig_info_valid)
    {
        metricsPtr->rat = LE_MRC_RAT_LTE;

        metricsPtr->ss = getSigResp.lte_sig_info.rssi;
        metricsPtr->lteMetrics.rsrq = getSigResp.lte_sig_info.rsrq*10;
        metricsPtr->lteMetrics.rsrp = getSigResp.lte_sig_info.rsrp*10;
        metricsPtr->lteMetrics.snr = getSigResp.lte_sig_info.snr;
    }
    else if (getSigResp.cdma_sig_info_valid)
    {
        metricsPtr->rat = LE_MRC_RAT_CDMA;
        metricsPtr->ss = getSigResp.cdma_sig_info.rssi;
        metricsPtr->cdmaMetrics.ecio = -((getSigResp.cdma_sig_info.ecio*10)/2);
        metricsPtr->cdmaMetrics.sinr = INT32_MAX;   // (only applicable for 1xEV-DO)
        metricsPtr->cdmaMetrics.io = INT32_MAX;     // (only applicable for 1xEV-DO)
    }
    else if (getSigResp.hdr_sig_info_valid)
    {
        metricsPtr->rat = LE_MRC_RAT_CDMA;
        metricsPtr->ss = getSigResp.hdr_sig_info.common_sig_str.rssi;
        metricsPtr->cdmaMetrics.ecio = -((getSigResp.hdr_sig_info.common_sig_str.ecio*10)/2);
        switch(getSigResp.hdr_sig_info.sinr)    // (only applicable for 1xEV-DO)
        {
            case NAS_SINR_LEVEL_0_V01:
                metricsPtr->cdmaMetrics.sinr = -90;
                break;
            case NAS_SINR_LEVEL_1_V01:
                metricsPtr->cdmaMetrics.sinr = -60;
                break;
            case NAS_SINR_LEVEL_2_V01:
                metricsPtr->cdmaMetrics.sinr = -45;
                break;
            case NAS_SINR_LEVEL_3_V01:
                metricsPtr->cdmaMetrics.sinr = -30;
                break;
            case NAS_SINR_LEVEL_4_V01:
                metricsPtr->cdmaMetrics.sinr = -20;
                break;
            case NAS_SINR_LEVEL_5_V01:
                metricsPtr->cdmaMetrics.sinr = 10;
                break;
            case NAS_SINR_LEVEL_6_V01:
                metricsPtr->cdmaMetrics.sinr = 30;
                break;
            case NAS_SINR_LEVEL_7_V01:
                metricsPtr->cdmaMetrics.sinr = 60;
                break;
            case NAS_SINR_LEVEL_8_V01:
                metricsPtr->cdmaMetrics.sinr = 90;
                break;
            default:
                metricsPtr->cdmaMetrics.sinr = INT32_MAX;
                break;
        }
        metricsPtr->cdmaMetrics.io = getSigResp.hdr_sig_info.io;    // (only applicable for 1xEV-DO)
    }
    else
    {
        metricsPtr->rat = LE_MRC_RAT_UNKNOWN;   // No info.
    }

    // Block error rate is not provided by QMI in the LTE RAT
    if (LE_MRC_RAT_LTE != metricsPtr->rat)
    {
        // Measure Error Rate
        SWIQMI_DECLARE_MESSAGE(nas_get_err_rate_resp_msg_v01, getErResp);

        clientMsgErr = qmi_client_send_msg_sync(NasClient,
                                                QMI_NAS_GET_ERR_RATE_REQ_MSG_V01,
                                                NULL, 0,
                                                &getErResp, sizeof(getErResp),
                                                COMM_DEFAULT_PLATFORM_TIMEOUT);

        if (LE_OK == swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_NAS_GET_ERR_RATE_REQ_MSG_V01),
                                          clientMsgErr,
                                          getErResp.resp.result,
                                          getErResp.resp.error))
        {
            LE_DEBUG("Err info: gsm.%d, wcdma.%d, tdscdma %d, cdma.%d, hdr.%d",
                     getErResp.gsm_bit_err_rate_valid, getErResp.wcdma_block_err_rate_valid,
                     getErResp.tdscdma_block_err_rate_valid, getErResp.cdma_frame_err_rate_valid,
                     getErResp.hdr_packet_err_rate_valid );

            if (getErResp.gsm_bit_err_rate_valid)
            {
                if (0xFF == getErResp.gsm_bit_err_rate)
                {
                    metricsPtr->er = UINT32_MAX;
                }
                else
                {
                    metricsPtr->er = getErResp.gsm_bit_err_rate;
                }
            }
            else if (getErResp.wcdma_block_err_rate_valid)
            {
                if (0xFF == getErResp.wcdma_block_err_rate)
                {
                    metricsPtr->er = UINT32_MAX;
                }
                else
                {
                    metricsPtr->er = getErResp.wcdma_block_err_rate;
                }
            }
            else if(getErResp.tdscdma_block_err_rate_valid)
            {
                if (0xFF == getErResp.tdscdma_block_err_rate)
                {
                    metricsPtr->er = UINT32_MAX;
                }
                else
                {
                    metricsPtr->er = getErResp.tdscdma_block_err_rate;
                }
            }
            else if(getErResp.cdma_frame_err_rate_valid)
            {
                if (0xFFFF == getErResp.cdma_frame_err_rate)
                {
                    metricsPtr->er = UINT32_MAX;
                }
                else
                {
                    metricsPtr->er = getErResp.cdma_frame_err_rate;
                }
            }
            else if(getErResp.hdr_packet_err_rate_valid)
            {
                if (0xFFFF == getErResp.hdr_packet_err_rate)
                {
                    metricsPtr->er = UINT32_MAX;
                }
                else
                {
                    metricsPtr->er = getErResp.hdr_packet_err_rate;
                }
            }
            else
            {
                metricsPtr->er = UINT32_MAX;
            }
        }
        else
        {
            // Failed to retrieve block error rate
            metricsPtr->er = UINT32_MAX;
        }
    }
    else
    {
        // Block error rate is not provided by QMI in the LTE RAT
        metricsPtr->er = UINT32_MAX;
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function gets the Signal Strength information.
 *
 * @return LE_FAULT         The function failed.
 * @return LE_OK            The function succeeded.
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_GetSignalStrength
(
    int32_t*          rssiPtr    ///< [OUT] The received signal strength (in dBm).
)
{
    nas_get_sig_info_resp_msg_v01 getSigResp = { {0} };

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(NasClient,
                                            QMI_NAS_GET_SIG_INFO_REQ_MSG_V01,
                                            NULL, 0,
                                            &getSigResp, sizeof(getSigResp),
                                            COMM_DEFAULT_PLATFORM_TIMEOUT);

    le_result_t respResult = swiQmi_CheckResponseCode(
                                STRINGIZE_EXPAND(QMI_NAS_GET_SIG_INFO_REQ_MSG_V01),
                                clientMsgErr,
                                getSigResp.resp);

    if (respResult == LE_OK)
    {
        // Get and return the first available signal strength.
        // todo: CDMA not currently supported.  May need to query the technology used instead of
        //       just waiting for the first available technology.
        if (getSigResp.gsm_sig_info_valid)
        {
            *rssiPtr = getSigResp.gsm_sig_info;
        }
        else if (getSigResp.wcdma_sig_info_valid)
        {
            *rssiPtr = getSigResp.wcdma_sig_info.rssi;
        }
        else if (getSigResp.lte_sig_info_valid)
        {
            *rssiPtr = getSigResp.lte_sig_info.rssi;
        }
        else if(getSigResp.tdscdma_sig_info_valid)
        {
            *rssiPtr = getSigResp.tdscdma_sig_info.rssi;
        }
        else if(getSigResp.cdma_sig_info_valid)
        {
            *rssiPtr = getSigResp.cdma_sig_info.rssi;
        }
        else if(getSigResp.hdr_sig_info_valid)
        {
            *rssiPtr = getSigResp.hdr_sig_info.common_sig_str.rssi;
        }
        else
        {
            *rssiPtr = INT32_MIN;  // No signal.
        }
    }
    else if (respResult == LE_FAULT)
    {
        *rssiPtr = INT32_MIN;  // No signal.
        respResult = LE_OK;
    }

    return respResult;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to get the current network information.
 *
 * @return
 *      - LE_OK on success
 *      - LE_OVERFLOW if the current network name can't fit in nameStr
 *      - LE_FAULT on any other failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_GetCurrentNetwork
(
    char       *nameStr,               ///< [OUT] the home network Name
    size_t      nameStrSize,           ///< [IN]  the nameStr size
    char       *mccStr,                ///< [OUT] the mobile country code
    size_t      mccStrNumElements,     ///< [IN]  the mccStr size
    char       *mncStr,                ///< [OUT] the mobile network code
    size_t      mncStrNumElements      ///< [IN]  the mncStr size
)
{
    uint8_t                       i;
    char                          mcc[NAS_MCC_MNC_MAX_V01+1] = {0};
    char                          mnc[NAS_MCC_MNC_MAX_V01+1] = {0};
    char*                         mccPtr = NULL;
    char*                         mncPtr = NULL;
    nas_get_sys_info_resp_msg_v01 getResp = { {0} };

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(NasClient,
                                                                  QMI_NAS_GET_SYS_INFO_REQ_MSG_V01,
                                                                  NULL, 0,
                                                                  &getResp, sizeof(getResp),
                                                                  COMM_NETWORK_TIMEOUT);

    le_result_t respResult = swiQmi_CheckResponseCode(
                                                STRINGIZE_EXPAND(QMI_NAS_GET_SYS_INFO_REQ_MSG_V01),
                                                clientMsgErr,
                                                getResp.resp);

    if (respResult == LE_OK)
    {
        if (getResp.gsm_sys_info_valid)
        {
            if (getResp.gsm_sys_info.threegpp_specific_sys_info.network_id_valid)
            {
                LE_DEBUG("GSM information is valid.");
                mccPtr = getResp.gsm_sys_info.threegpp_specific_sys_info.network_id.mcc;
                mncPtr = getResp.gsm_sys_info.threegpp_specific_sys_info.network_id.mnc;
            }
        }
        else if (getResp.wcdma_sys_info_valid)
        {
            if (getResp.wcdma_sys_info.threegpp_specific_sys_info.network_id_valid)
            {
                LE_DEBUG("UMTS (WCDMA) information is valid.");
                mccPtr = getResp.wcdma_sys_info.threegpp_specific_sys_info.network_id.mcc;
                mncPtr = getResp.wcdma_sys_info.threegpp_specific_sys_info.network_id.mnc;
            }
        }
        else if (getResp.tdscdma_sys_info_valid)
        {
            if (getResp.tdscdma_sys_info.threegpp_specific_sys_info.network_id_valid)
            {
                LE_DEBUG("TDSCDMA information is valid.");
                mccPtr = getResp.tdscdma_sys_info.threegpp_specific_sys_info.network_id.mcc;
                mncPtr = getResp.tdscdma_sys_info.threegpp_specific_sys_info.network_id.mnc;
            }
        }

        else if (getResp.lte_sys_info_valid)
        {
            if (getResp.lte_sys_info.threegpp_specific_sys_info.network_id_valid)
            {
                LE_DEBUG("LTE information is valid.");
                mccPtr = getResp.lte_sys_info.threegpp_specific_sys_info.network_id.mcc;
                mncPtr = getResp.lte_sys_info.threegpp_specific_sys_info.network_id.mnc;
            }
        }
        else if (getResp.cdma_sys_info_valid)
        {
            if (getResp.cdma_sys_info.cdma_specific_sys_info.network_id_valid)
            {
                // For CDMA, the MCC wildcard value is returned as \{`3', 0xFF, 0xFF\}.
                // For CDMA, the MNC wildcard value is returned as \{`7', 0xFF, 0xFF\}.
                LE_DEBUG("CDMA information is valid.");
                mccPtr = getResp.cdma_sys_info.cdma_specific_sys_info.network_id.mcc;
                mncPtr = getResp.cdma_sys_info.cdma_specific_sys_info.network_id.mnc;
            }
        }

        if ((mccPtr) && (mncPtr))
        {
            for(i=0 ; i<NAS_MCC_MNC_MAX_V01 ; i++)
            {
                if (mccPtr[i] != 0xFF)
                {
                    mcc[i] = mccPtr[i];
                }
                else
                {
                    break;
                }
            }

            for(i=0 ; i<NAS_MCC_MNC_MAX_V01 ; i++)
            {
                if (mncPtr[i] != 0xFF)
                {
                    mnc[i] = mncPtr[i];
                }
                else
                {
                    break;
                }
            }

            LE_DEBUG("MCC.%s, MNC.%s", mcc, mnc);

            if (mccStr && mncStr)
            {
                le_utf8_Copy(mccStr, mcc, mccStrNumElements, NULL);
                le_utf8_Copy(mncStr, mnc, mncStrNumElements, NULL);
            }

            if (nameStr)
            {
                return ConvertHomeNetworkName(atoi(mcc), atoi(mnc), nameStr, nameStrSize);
            }

            if (mccStr && mncStr)
            {
                return LE_OK;
            }
        }
    }

    LE_ERROR("Cannot retrieve current Network information.");
    return LE_FAULT;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to delete the list of Scan Information
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_mrc_DeleteScanInformation
(
    le_dls_List_t *scanInformationListPtr ///< [IN] list of pa_mrc_ScanInformation_t
)
{
    pa_mrc_ScanInformation_t* nodePtr;
    le_dls_Link_t *linkPtr;

    while ((linkPtr=le_dls_Pop(scanInformationListPtr)) != NULL)
    {
        nodePtr = CONTAINER_OF(linkPtr, pa_mrc_ScanInformation_t, link);
        le_mem_Release(nodePtr);
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to perform a network scan.
 *
 * @return LE_FAULT         The function failed.
 * @return LE_COMM_ERROR    Radio link failure occurred.
 * @return LE_OK            The function succeeded.
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_PerformNetworkScan
(
    le_mrc_RatBitMask_t ratMask,               ///< [IN] The network mask
    pa_mrc_ScanType_t   scanType,              ///< [IN] the scan type
    le_dls_List_t      *scanInformationListPtr ///< [OUT] list of pa_mrc_ScanInformation_t
)
{
    le_result_t result = LE_OK;

    nas_perform_network_scan_req_msg_v01 performNetworkScanReq = {0};
    nas_perform_network_scan_resp_msg_v01 performNetworkScanResp = { {0} };

    performNetworkScanReq.network_type_valid = true;
    if (GetScanRatBitMask(ratMask, &performNetworkScanReq.network_type) != LE_OK)
    {
        LE_ERROR("The platform does not support scan on the specified RAT (0x%X)",
                 ratMask);
        return LE_FAULT;
    }

    performNetworkScanReq.scan_type_valid = true;
    performNetworkScanReq.scan_type = TranslateScanType(scanType);

    LE_DEBUG("Perform scan for 0x%X, type(0x%X)",
             performNetworkScanReq.network_type,
             performNetworkScanReq.scan_type);

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(NasClient,
                                            QMI_NAS_PERFORM_NETWORK_SCAN_REQ_MSG_V01,
                                            &performNetworkScanReq, sizeof(performNetworkScanReq),
                                            &performNetworkScanResp, sizeof(performNetworkScanResp),
                                            COMM_NETWORK_SCAN_TIMEOUT);

    result = swiQmi_CheckResponseCode(
                                STRINGIZE_EXPAND(QMI_NAS_PERFORM_NETWORK_SCAN_REQ_MSG_V01),
                                clientMsgErr,
                                performNetworkScanResp.resp);


    // The QMI timer out above is too long, kick watchdog here to avoid watchdog expiry.
    KickWatchdog();

    if (result!=LE_OK)
    {
        return LE_FAULT;
    }

    if (!performNetworkScanResp.scan_result_valid )
    {
        return LE_FAULT;
    }
    else
    {
        if (performNetworkScanResp.scan_result==NAS_SCAN_REJ_IN_RLF_V01)
        {
            return LE_COMM_ERROR;
        } else if (performNetworkScanResp.scan_result==NAS_SCAN_AS_ABORT_V01)
        {
            return LE_FAULT;
        }
    }

    if ( IS_TRACE_ENABLED )
    {

        LE_PRINT_VALUE_IF(performNetworkScanResp.scan_result_valid,
                          "%x",
                          performNetworkScanResp.scan_result);

        if ( performNetworkScanResp.nas_3gpp_network_info_valid)
        {
            int i;
            for (i=0;i<performNetworkScanResp.nas_3gpp_network_info_len;i++)
            {
                LE_DEBUG("3gpp_net_info[%i].mobile_country_code = %i",i,
                         performNetworkScanResp.nas_3gpp_network_info[i].mobile_country_code);
                LE_DEBUG("3gpp_net_info[%i].mobile_network_code = %i",i,
                         performNetworkScanResp.nas_3gpp_network_info[i].mobile_network_code);
                LE_DEBUG("3gpp_net_info[%i].network_description = %s",i,
                         performNetworkScanResp.nas_3gpp_network_info[i].network_description);
                LE_DEBUG("3gpp_net_info[%i].network_status = %x",i,
                         performNetworkScanResp.nas_3gpp_network_info[i].network_status);
            }
        }

        if ( performNetworkScanResp.nas_network_radio_access_technology_valid)
        {
            int i;
            for (i=0;i<performNetworkScanResp.nas_network_radio_access_technology_len;i++)
            {
                LE_DEBUG("net_radio[%i].mcc = %i",i,
                         performNetworkScanResp.nas_network_radio_access_technology[i].mcc);
                LE_DEBUG("net_radio[%i].mnc = %i",i,
                         performNetworkScanResp.nas_network_radio_access_technology[i].mnc);
                LE_DEBUG("net_radio[%i].rat = %i",i,
                         performNetworkScanResp.nas_network_radio_access_technology[i].rat);
            }
        }

        if ( performNetworkScanResp.mnc_includes_pcs_digit_valid)
        {
            int i;
            for (i=0;i<performNetworkScanResp.mnc_includes_pcs_digit_len;i++)
            {
                LE_DEBUG("pcs_digit[%i].mcc = %i",i,
                         performNetworkScanResp.mnc_includes_pcs_digit[i].mcc);
                LE_DEBUG("pcs_digit[%i].mnc = %i",i,
                         performNetworkScanResp.mnc_includes_pcs_digit[i].mnc);
                LE_DEBUG("pcs_digit[%i].mnc_includes_pcs_digit = %i",i,
                         performNetworkScanResp.mnc_includes_pcs_digit[i].mnc_includes_pcs_digit);
            }
        }

        if ( performNetworkScanResp.csg_info_valid)
        {
            int i;
            for (i=0;i<performNetworkScanResp.csg_info_len;i++)
            {
                char csgName[49]={0};
                LE_DEBUG("cgs[%i].mcc = %i",i, performNetworkScanResp.csg_info[i].mcc);
                LE_DEBUG("cgs[%i].mnc = %i",i, performNetworkScanResp.csg_info[i].mnc);
                LE_DEBUG("cgs[%i].csg_list_cat = %i",i,
                         performNetworkScanResp.csg_info[i].csg_list_cat);
                LE_DEBUG("cgs[%i].csg_info = %i",i, performNetworkScanResp.csg_info[i].csg_info.id);
                memcpy(csgName,
                    performNetworkScanResp.csg_info[i].csg_info.name,
                    performNetworkScanResp.csg_info[i].csg_info.name_len);
                LE_DEBUG("cgs[%i].name = %s",i, csgName);
            }
        }
    }

    if ( performNetworkScanResp.nas_network_radio_access_technology_valid)
    {
        pa_mrc_ScanInformation_t* newScanInformation = NULL;
        int i;
        for (i=0;i<performNetworkScanResp.nas_network_radio_access_technology_len;i++)
        {
            char mccStr[LE_MRC_MCC_BYTES];
            char mncStr[LE_MRC_MNC_BYTES];

            le_mrc_Rat_t rat = ConvertRat(performNetworkScanResp.nas_network_radio_access_technology[i].rat);
            uint32_t net = performNetworkScanResp.nas_3gpp_network_info[i].network_status;

            ConvertMccMncIntoStringFormat(
                performNetworkScanResp.nas_network_radio_access_technology[i].mcc,
                performNetworkScanResp.nas_network_radio_access_technology[i].mnc,
                performNetworkScanResp.mnc_includes_pcs_digit[i].mnc_includes_pcs_digit,
                mccStr,
                mncStr);

            newScanInformation = FindScanInformation(scanInformationListPtr,mccStr,mncStr,rat);

            if (newScanInformation == NULL)
            {
                newScanInformation = le_mem_ForceAlloc(ScanInformationPool);
                InitializeScanInformation(newScanInformation);
                le_dls_Queue(scanInformationListPtr,&(newScanInformation->link));

                memcpy(newScanInformation->mobileCode.mcc,mccStr,LE_MRC_MCC_BYTES);
                memcpy(newScanInformation->mobileCode.mnc,mncStr,LE_MRC_MNC_BYTES);
                newScanInformation->rat = rat;
                newScanInformation->isInUse = ConvertNetworkIsInUse(net);
                newScanInformation->isAvailable = ConvertNetworkIsAvailable(net);
                newScanInformation->isHome = ConvertNetworkIsHome(net);
                newScanInformation->isForbidden = ConvertNetworkIsForbidden(net);
            }
        }
    }

    return result;
}


//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to get the scanInformation operator name
 *
 * @return
 *      - LE_OK on success
 *      - LE_OVERFLOW if the operator name would not fit in buffer
 *      - LE_FAULT for all other errors
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_GetScanInformationName
(
    pa_mrc_ScanInformation_t *scanInformationPtr, ///< [IN] The scan information
    char *namePtr,                                ///< [OUT] Name of operator
    size_t nameSize                               ///< [IN] The size in bytes of the namePtr buffer
)
{
    uint16_t mcc,mnc;
    if ((!scanInformationPtr) || (!namePtr))
    {
        return LE_FAULT;
    }

    mcc = atoi(scanInformationPtr->mobileCode.mcc);
    mnc = atoi(scanInformationPtr->mobileCode.mnc);

    return ConvertHomeNetworkName(mcc,mnc,namePtr,nameSize);
}


//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to get the number of preferred operators present in the list.
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_CountPreferredOperators
(
    bool      plmnStatic,   ///< [IN] Include Static preferred Operators.
    bool      plmnUser,     ///< [IN] Include Users preferred Operators.
    int32_t*  nbItemPtr     ///< [OUT] number of Preferred operator found if success.
)
{
    int32_t total = 0;
    nas_get_preferred_networks_resp_msg_v01 getPrefNetworkResp = { {0} };

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(
        NasClient,
        QMI_NAS_GET_PREFERRED_NETWORKS_REQ_MSG_V01,
        NULL, 0,
        &getPrefNetworkResp, sizeof(getPrefNetworkResp),
        COMM_UICC_TIMEOUT);

    le_result_t respResult = swiQmi_CheckResponseCode(
        STRINGIZE_EXPAND(QMI_NAS_GET_PREFERRED_NETWORKS_REQ_MSG_V01),
        clientMsgErr,
        getPrefNetworkResp.resp);

    if (respResult == LE_OK)
    {
        LE_DEBUG("Preferred PLMN User: %c pcs %c, Operator: %c pcs %c",
            (getPrefNetworkResp.nas_3gpp_preferred_networks_valid ? 'Y' :'N'),
            (getPrefNetworkResp.nas_3gpp_mnc_includes_pcs_digit_valid ? 'Y' :'N'),
            (getPrefNetworkResp.static_3gpp_preferred_networks_valid ? 'Y' :'N'),
            (getPrefNetworkResp.static_3gpp_mnc_includes_pcs_digit_valid ? 'Y' :'N'));

        /* User preferred operators list */
        if (getPrefNetworkResp.nas_3gpp_preferred_networks_valid && plmnUser)
        {
            LE_DEBUG("3GPP Preferred network information are valid with %d Operators,"
                " PcsFile Present '%c' ",
                getPrefNetworkResp.nas_3gpp_preferred_networks_len,
                (getPrefNetworkResp.nas_3gpp_mnc_includes_pcs_digit_valid ? 'Y' :'N'));
            total = getPrefNetworkResp.nas_3gpp_preferred_networks_len;
        }
        else
        {
            if (plmnUser)
            {
                LE_ERROR("Failed to retrieve User Preferred Operators List Information!");
                return LE_FAULT;
            }
        }

        /* Static preferred operators list */
        if (getPrefNetworkResp.static_3gpp_preferred_networks_valid && plmnStatic )
        {
            LE_DEBUG("STATIC 3GPP Preferred network information are valid with %d Operators,"
                " PcsFile Present '%c' ",
                getPrefNetworkResp.static_3gpp_preferred_networks_len,
                (getPrefNetworkResp.static_3gpp_mnc_includes_pcs_digit_valid ? 'Y' :'N'));
            total += getPrefNetworkResp.static_3gpp_preferred_networks_len;
        }
        else
        {
            if (plmnStatic)
            {
                LE_ERROR("Failed to retrieve Static Preferred Operators List Information!");
                return LE_FAULT;
            }
        }

        LE_DEBUG("Total return %d",total);
        *nbItemPtr = total;
        return LE_OK;
    }

    LE_DEBUG("Failed to retrieve Preferred Operators List Information!");
    return LE_FAULT;
}



//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to get the current preferred operators.
 *
 * @return
 *      - LE_OK on success
 *      - LE_NOT_FOUND if Preferred operator list is not available
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_GetPreferredOperators
(
    pa_mrc_PreferredNetworkOperator_t*   preferredOperatorPtr,
                       ///< [IN/OUT] The preferred operators pointer.
    bool  plmnStatic,  ///< [IN] Include Static preferred Operators.
    bool  plmnUser,    ///< [IN] Include Users preferred Operators.
    int32_t* nbItemPtr ///< [IN/OUT] number of Preferred operator to find (in) and written (out).
)
{
    uint32_t total = 0, i = 0;
    uint16_t qmiRat = 0;
    nas_get_preferred_networks_resp_msg_v01 getPrefNetworkResp = { {0} };

    if (preferredOperatorPtr == NULL)
    {
        LE_FATAL("preferredOperatorPtr is NULL !");
    }

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(
                    NasClient,
                    QMI_NAS_GET_PREFERRED_NETWORKS_REQ_MSG_V01,
                    NULL, 0,
                    &getPrefNetworkResp, sizeof(getPrefNetworkResp),
                    COMM_UICC_TIMEOUT);

    le_result_t respResult = swiQmi_CheckResponseCode(
                    STRINGIZE_EXPAND(QMI_NAS_GET_PREFERRED_NETWORKS_REQ_MSG_V01),
                    clientMsgErr,
                    getPrefNetworkResp.resp);

    if (respResult == LE_OK)
    {
        LE_DEBUG("Preferred PLMN User: %c pcs %c, Operator: %c pcs %c",
            (getPrefNetworkResp.nas_3gpp_preferred_networks_valid ? 'Y' :'N'),
            (getPrefNetworkResp.nas_3gpp_mnc_includes_pcs_digit_valid ? 'Y' :'N'),
            (getPrefNetworkResp.static_3gpp_preferred_networks_valid ? 'Y' :'N'),
            (getPrefNetworkResp.static_3gpp_mnc_includes_pcs_digit_valid ? 'Y' :'N'));

        /* User preferred operators list */
        if (getPrefNetworkResp.nas_3gpp_preferred_networks_valid && plmnUser)
        {
            LE_DEBUG("3GPP Preferred network information are valid with %d Operators,"
                     " PcsFile Present '%c' ",
                     getPrefNetworkResp.nas_3gpp_preferred_networks_len,
                     (getPrefNetworkResp.nas_3gpp_mnc_includes_pcs_digit_valid ? 'Y' :'N'));

            for (i=0; (i < getPrefNetworkResp.nas_3gpp_preferred_networks_len)
                      && (i < *nbItemPtr); i++)
            {
                qmiRat = getPrefNetworkResp.nas_3gpp_preferred_networks[i].radio_access_technology;

                bool isMncIncPcsDigit = false;
                uint16_t* mncPtr = &(getPrefNetworkResp.nas_3gpp_preferred_networks[i].mobile_network_code);

                if ( getPrefNetworkResp.nas_3gpp_mnc_includes_pcs_digit_valid
                    && getPrefNetworkResp.nas_3gpp_mnc_includes_pcs_digit[i].mnc_includes_pcs_digit)
                {
                    isMncIncPcsDigit = true;
                    mncPtr = &(getPrefNetworkResp.nas_3gpp_mnc_includes_pcs_digit[i].mnc);
                }

                ConvertMccMncIntoStringFormat(
                    getPrefNetworkResp.nas_3gpp_preferred_networks[i].mobile_country_code,
                    *mncPtr,
                    isMncIncPcsDigit,
                    preferredOperatorPtr[i].mobileCode.mcc,
                    preferredOperatorPtr[i].mobileCode.mnc);

                LE_DEBUG("3GPP [%d] MCC.%s MNC.%s QmiRat %04X", i+1,
                    preferredOperatorPtr[i].mobileCode.mcc,
                    preferredOperatorPtr[i].mobileCode.mnc,
                    qmiRat );

                preferredOperatorPtr[i].ratMask = GetPrefQmiRatBitMask(qmiRat);
                preferredOperatorPtr[i].index = i;
            }
            total += i;
        }
        else
        {
            if (plmnUser)
            {
                LE_ERROR("Failed to retrieve User Preferred Operators List Information!");
                return LE_NOT_FOUND;
            }
        }

        /* Static preferred operators list */
        if (getPrefNetworkResp.static_3gpp_preferred_networks_valid && plmnStatic )
        {
            LE_DEBUG("STATIC 3GPP Preferred network information are valid with %d Operators,"
                     " PcsFile Present '%c' ",
                     getPrefNetworkResp.static_3gpp_preferred_networks_len,
                     (getPrefNetworkResp.static_3gpp_mnc_includes_pcs_digit_valid ? 'Y' :'N'));

            for (i=0 ; (i < getPrefNetworkResp.static_3gpp_preferred_networks_len)
                       && ((total + i) < *nbItemPtr); i++)
            {
                qmiRat =
                    getPrefNetworkResp.static_3gpp_preferred_networks[i].radio_access_technology;

                bool isMncIncPcsDigit = false;
                uint16_t* mncPtr = &(getPrefNetworkResp.static_3gpp_preferred_networks[i].mobile_network_code);

                if ( getPrefNetworkResp.static_3gpp_mnc_includes_pcs_digit_valid
                    && getPrefNetworkResp.static_3gpp_mnc_includes_pcs_digit[i].mnc_includes_pcs_digit)
                {
                    isMncIncPcsDigit = true;
                    mncPtr = &(getPrefNetworkResp.static_3gpp_mnc_includes_pcs_digit[i].mnc);
                }

                ConvertMccMncIntoStringFormat(
                    getPrefNetworkResp.static_3gpp_preferred_networks[i].mobile_country_code,
                    *mncPtr,
                    isMncIncPcsDigit,
                    preferredOperatorPtr[total+i].mobileCode.mcc,
                    preferredOperatorPtr[total+i].mobileCode.mnc);

                LE_DEBUG("3GPP [%d] MCC.%s MNC.%s QmiRat %04X", i+1,
                    preferredOperatorPtr[total+i].mobileCode.mcc,
                    preferredOperatorPtr[total+i].mobileCode.mnc,
                    qmiRat );

                preferredOperatorPtr[total+i].index = total+i;
            }
            total += i;
        }
        else
        {
            if (plmnStatic)
            {
                LE_ERROR("Failed to retrieve Static Preferred Operators List Information!");
                return LE_NOT_FOUND;
            }
        }

        LE_DEBUG("Total return %d",total);
        *nbItemPtr = total;
        return LE_OK;
    }

    LE_DEBUG("Failed to retrieve Preferred Operators List Information!");
    return LE_NOT_FOUND;
}


//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to apply the preferred list into the modem
 *
 * @return
 *      - LE_OK             on success
 *      - LE_FAULT          for all other errors
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_SavePreferredOperators
(
    le_dls_List_t      *preferredOperatorsListPtr ///< [IN] List of preferred network operator
)
{
    nas_set_preferred_networks_req_msg_v01 preferredNetworkReq = {0};
    nas_set_preferred_networks_resp_msg_v01 preferredNetworkResp = { {0} };

    size_t nbElem = le_dls_NumLinks(preferredOperatorsListPtr);

    preferredNetworkReq.nas_3gpp_preferred_networks_valid = true;
    preferredNetworkReq.nas_3gpp_mnc_includes_pcs_digit_valid = true;

    // This is to reset previous network list saved
    preferredNetworkReq.clear_prev_preferred_networks = true;
    preferredNetworkReq.clear_prev_preferred_networks_valid = true;

    if ( nbElem > NAS_3GPP_PREFERRED_NETWORKS_LIST_MAX_V01 )
    {
        LE_WARN("Number of preferred element (%d) exceeded modem capacity of %d",
                        nbElem,
                        NAS_3GPP_PREFERRED_NETWORKS_LIST_MAX_V01 );

        preferredNetworkReq.nas_3gpp_preferred_networks_len =
                        NAS_3GPP_PREFERRED_NETWORKS_LIST_MAX_V01;
    }
    else
    {
        preferredNetworkReq.nas_3gpp_preferred_networks_len = nbElem;
    }

    LE_INFO("Preferred list: len[%d]", preferredNetworkReq.nas_3gpp_preferred_networks_len);

    preferredNetworkReq.nas_3gpp_mnc_includes_pcs_digit_len =
                    preferredNetworkReq.nas_3gpp_preferred_networks_len;

    if (preferredNetworkReq.nas_3gpp_preferred_networks_len)
    {
        pa_mrc_PreferredNetworkOperator_t* nodePtr;
        le_dls_Link_t *linkPtr;

        int i;
        for(i = 0, linkPtr = le_dls_Peek(preferredOperatorsListPtr);
            (i < preferredNetworkReq.nas_3gpp_preferred_networks_len ) && ( linkPtr!=NULL );
            ++i, linkPtr = le_dls_PeekNext(preferredOperatorsListPtr,linkPtr))
        {
            // Get the node from MsgList
            nodePtr = CONTAINER_OF(linkPtr, pa_mrc_PreferredNetworkOperator_t, link);

            preferredNetworkReq.nas_3gpp_preferred_networks[i].mobile_country_code =
                                                                atoi(nodePtr->mobileCode.mcc);
            preferredNetworkReq.nas_3gpp_preferred_networks[i].mobile_network_code =
                                                                atoi(nodePtr->mobileCode.mnc);

            preferredNetworkReq.nas_3gpp_preferred_networks[i].radio_access_technology =
                                                                GetPrefRatBitMask(nodePtr->ratMask);

            preferredNetworkReq.nas_3gpp_mnc_includes_pcs_digit[i].mcc =
                                                                atoi(nodePtr->mobileCode.mcc);
            preferredNetworkReq.nas_3gpp_mnc_includes_pcs_digit[i].mnc =
                                                                atoi(nodePtr->mobileCode.mnc);
            if(strlen(nodePtr->mobileCode.mnc) > 2)
            {
                LE_DEBUG("mnc 3 digit %s", nodePtr->mobileCode.mnc);
                preferredNetworkReq.nas_3gpp_mnc_includes_pcs_digit[i].mnc_includes_pcs_digit =
                                true;
                LE_DEBUG("Save [%d,%03d] rat 0x%.04"PRIX16,
                    preferredNetworkReq.nas_3gpp_preferred_networks[i].mobile_country_code,
                    preferredNetworkReq.nas_3gpp_preferred_networks[i].mobile_network_code,
                    preferredNetworkReq.nas_3gpp_preferred_networks[i].radio_access_technology);
            }
            else
            {
                preferredNetworkReq.nas_3gpp_mnc_includes_pcs_digit[i].mnc_includes_pcs_digit =
                                false;
                LE_DEBUG("Save [%d,%d] rat 0x%.04"PRIX16,
                    preferredNetworkReq.nas_3gpp_preferred_networks[i].mobile_country_code,
                    preferredNetworkReq.nas_3gpp_preferred_networks[i].mobile_network_code,
                    preferredNetworkReq.nas_3gpp_preferred_networks[i].radio_access_technology);
            }
        }
    }

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(NasClient,
                                            QMI_NAS_SET_PREFERRED_NETWORKS_REQ_MSG_V01,
                                            &preferredNetworkReq, sizeof(preferredNetworkReq),
                                            &preferredNetworkResp, sizeof(preferredNetworkResp),
                                            COMM_UICC_TIMEOUT);

    return swiQmi_CheckResponseCode(
                                STRINGIZE_EXPAND(QMI_NAS_SET_PREFERRED_NETWORKS_REQ_MSG_V01),
                                clientMsgErr,
                                preferredNetworkResp.resp);
}


//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to register on a mobile network [mcc;mnc]
 *
 * @return LE_FAULT         The function failed to register.
 * @return LE_DUPLICATE     There is already a registration in progress for [mcc;mnc].
 * @return LE_TIMEOUT       Registration attempt timed out.
 * @return LE_OK            The function succeeded to register.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_RegisterNetwork
(
    const char *mccPtr,   ///< [IN] Mobile Country Code
    const char *mncPtr    ///< [IN] Mobile Network Code
)
{
    uint8_t pcsInclude;
    le_result_t result;
    nas_set_system_selection_preference_req_msg_v01 systemSelectionPreferenceReq = {0};
    nas_set_system_selection_preference_resp_msg_v01 systemSelectionPreferenceResp = { {0} };

    LastRegError = 0;

    systemSelectionPreferenceReq.net_sel_pref.net_sel_pref = NAS_NET_SEL_PREF_MANUAL_V01;
    ConvertMccMncIntoNumberFormat(mccPtr,
                                  mncPtr,
                                  &systemSelectionPreferenceReq.net_sel_pref.mcc,
                                  &systemSelectionPreferenceReq.net_sel_pref.mnc,
                                  &systemSelectionPreferenceReq.mnc_includes_pcs_digit);

    systemSelectionPreferenceReq.mnc_includes_pcs_digit_valid = true;
    systemSelectionPreferenceReq.net_sel_pref_valid = true;

    LE_DEBUG("Select [%s,%s] (%d-%d-%d)",mccPtr,mncPtr,
                                             systemSelectionPreferenceReq.net_sel_pref.mcc,
                                             systemSelectionPreferenceReq.net_sel_pref.mnc,
                                             systemSelectionPreferenceReq.mnc_includes_pcs_digit);
    if (RegisteringNetwork.inProgress)
    {
        LE_ERROR("There is already a registration in progress for [%d,%d]",
                 RegisteringNetwork.mcc,RegisteringNetwork.mnc);
        return LE_DUPLICATE;
    }

    LOCK();
    if (ConvertMccMncIntoNumberFormat(mccPtr,mncPtr,
                                      &RegisteringNetwork.mcc,
                                      &RegisteringNetwork.mnc,
                                      &pcsInclude) != LE_OK)
    {
        UNLOCK();
        return LE_FAULT;
    }
    UNLOCK();

    qmi_txn_handle txn_handle;
    qmi_client_error_type clientMsgErr =  qmi_client_send_msg_async(NasClient,
                    QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01,
                    &systemSelectionPreferenceReq,
                    sizeof(systemSelectionPreferenceReq),
                    &systemSelectionPreferenceResp,
                    sizeof(systemSelectionPreferenceResp),
                    NULL,
                    NULL,
                    &txn_handle );

    result = swiQmi_CheckResponseCode(
                    STRINGIZE_EXPAND(QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01),
                    clientMsgErr,
                    systemSelectionPreferenceResp.resp);

    if (result != LE_OK)
    {
        LE_ERROR("Could not select [%s,%s] error %d",
            mccPtr,mncPtr,systemSelectionPreferenceResp.resp.error);
        return LE_FAULT;
    }

    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
        LE_ERROR("Cannot get current time");
        return LE_FAULT;
    }

    ts.tv_sec += 20; // Twenty seconds registration progress
    ts.tv_nsec += 0;

    LOCK();
    RegisteringNetwork.inProgress = true;
    UNLOCK();
    // Make the API synchronous

    if ( sem_timedwait(&RegisteringNetwork.semSynchro,&ts) == -1 )
    {
        LE_WARN("errno %d", errno);
        if ( errno == ETIMEDOUT)
        {
            LE_WARN("Timeout when registering");
            LOCK();
            RegisteringNetwork.inProgress = false;
            UNLOCK();
            return LE_TIMEOUT;
        }
    }

    LOCK();
    LE_DEBUG("Registration to [%s,%s] is %d",mccPtr,mncPtr,RegisteringNetwork.state);
    RegisteringNetwork.inProgress = false;
    if ( RegisteringNetwork.state == NAS_REGISTERED_V01 )
    {
        UNLOCK();
        return LE_OK;
    }

    UNLOCK();
    return LE_FAULT;
}


//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to register automatically on network
 *
 * @return
 *      - LE_OK             on success
 *      - LE_FAULT          for all other errors
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_SetAutomaticNetworkRegistration
(
    void
)
{
    nas_set_system_selection_preference_req_msg_v01 systemSelectionPreferenceReq = {0};
    nas_set_system_selection_preference_resp_msg_v01 systemSelectionPreferenceResp = { {0} };

    LE_DEBUG("Set automatic mode");
    LastRegError = 0;

    systemSelectionPreferenceReq.net_sel_pref.net_sel_pref = NAS_NET_SEL_PREF_AUTOMATIC_V01;
    systemSelectionPreferenceReq.net_sel_pref_valid = true;

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(NasClient,
                            QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01,
                            &systemSelectionPreferenceReq, sizeof(systemSelectionPreferenceReq),
                            &systemSelectionPreferenceResp, sizeof(systemSelectionPreferenceResp),
                            COMM_LONG_PLATFORM_TIMEOUT);

    if ( swiQmi_CheckResponseCode(
                                STRINGIZE_EXPAND(QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01),
                                clientMsgErr,
                                systemSelectionPreferenceResp.resp) != LE_OK)
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
 * This function gets the Radio Access Technology currently in use.
 *
 * @return
 * - LE_OK              On success
 * - LE_FAULT           On failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_GetRadioAccessTechInUse
(
    le_mrc_Rat_t*   ratPtr    ///< [OUT] The Radio Access Technology.
)
{
    uint32_t i;
    nas_get_rf_band_info_resp_msg_v01 getBandResp =
#if (NAS_V01_IDL_MAJOR_VERS >= 0x01) && (NAS_V01_IDL_MINOR_VERS >= 0x76)
        {{QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01}};
#else
        {0};
#endif

    qmi_client_error_type clientMsgErr =
                        qmi_client_send_msg_sync(NasClient,
                                                 QMI_NAS_GET_RF_BAND_INFO_REQ_MSG_V01,
                                                 NULL, 0,
                                                 &getBandResp, sizeof(getBandResp),
                                                 COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (LE_OK != swiQmi_CheckResponse(
                    STRINGIZE_EXPAND(QMI_NAS_GET_RF_BAND_INFO_REQ_MSG_V01),
                    clientMsgErr,
                    getBandResp.resp.result,
                    getBandResp.resp.error))
    {
        if (QMI_ERR_INFO_UNAVAILABLE_V01 == getBandResp.resp.error)
        {
            LE_ERROR("The device is not registered on the network");
        }

        LE_ERROR("Failed to retrieve the RAT");
        *ratPtr = LE_MRC_RAT_UNKNOWN;
        return LE_FAULT;
    }

    for (i=0; i<getBandResp.rf_band_info_list_len; i++)
    {
        LE_DEBUG("active_channel[%d].%d, radio_if[%d].%d",
                 i,
                 getBandResp.rf_band_info_list[i].active_channel,
                 i,
                 getBandResp.rf_band_info_list[i].radio_if);

        // Get and return the first available RAT.
        if (getBandResp.rf_band_info_list[i].active_channel)
        {
            *ratPtr = ConvertRat(getBandResp.rf_band_info_list[i].radio_if);
            return LE_OK;
        }
    }

    *ratPtr = LE_MRC_RAT_UNKNOWN;
    return LE_FAULT;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the Radio Access Technology Preferences
 *
 * @return
 * - LE_OK              On success
 * - LE_FAULT           On failure
 * - LE_UNSUPPORTED     Not supported by platfom
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_SetRatPreferences
(
    le_mrc_RatBitMask_t ratMask ///< [IN] A bit mask to set the Radio Access Technology preferences.
)
{
    le_result_t res = LE_OK;
    nas_set_system_selection_preference_req_msg_v01 selPrefReq = {0};
    nas_set_system_selection_preference_resp_msg_v01 selPrefResp = {{0}};
    mode_pref_mask_type_v01 bitMask = 0x00;

    if (ratMask & LE_MRC_BITMASK_RAT_CDMA)
    {
        if ( !(RatCapabilities & QMI_NAS_RAT_MODE_PREF_CDMA2000_1X_V01) &&
             !(RatCapabilities & QMI_NAS_RAT_MODE_PREF_CDMA2000_HRPD_V01) )
        {
            res = LE_UNSUPPORTED;
        }
        else
        {
            if (RatCapabilities & QMI_NAS_RAT_MODE_PREF_CDMA2000_1X_V01)
            {
                bitMask |= QMI_NAS_RAT_MODE_PREF_CDMA2000_1X_V01;
            }
            if (RatCapabilities & QMI_NAS_RAT_MODE_PREF_CDMA2000_HRPD_V01)
            {
                bitMask |= QMI_NAS_RAT_MODE_PREF_CDMA2000_HRPD_V01;
            }
        }
    }
    if (ratMask & LE_MRC_BITMASK_RAT_GSM)
    {
        if (RatCapabilities & QMI_NAS_RAT_MODE_PREF_GSM_V01)
        {
            bitMask |= QMI_NAS_RAT_MODE_PREF_GSM_V01;
        }
        else
        {
            res = LE_UNSUPPORTED;
        }
    }
    if (ratMask & LE_MRC_BITMASK_RAT_TDSCDMA)
    {
        if (RatCapabilities & QMI_NAS_RAT_MODE_PREF_TDSCDMA_V01)
        {
            bitMask |= QMI_NAS_RAT_MODE_PREF_TDSCDMA_V01;
        }
        else
        {
            res = LE_UNSUPPORTED;
        }
    }
    if (ratMask & LE_MRC_BITMASK_RAT_UMTS)
    {
        if (RatCapabilities & QMI_NAS_RAT_MODE_PREF_UMTS_V01)
        {
            bitMask |= QMI_NAS_RAT_MODE_PREF_UMTS_V01;
        }
        else
        {
            res = LE_UNSUPPORTED;
        }
    }
    if (ratMask & LE_MRC_BITMASK_RAT_LTE)
    {
        if (RatCapabilities & QMI_NAS_RAT_MODE_PREF_LTE_V01)
        {
            bitMask |= QMI_NAS_RAT_MODE_PREF_LTE_V01;
        }
        else
        {
            res = LE_UNSUPPORTED;
        }
    }

    if (res == LE_UNSUPPORTED)
    {
        LE_ERROR("The platform does not support the specified RAT (0x%02X)",
                 ratMask);
    }
    else
    {
        LE_DEBUG("Set RAT Preferences QMI bit mask: 0x%02X", bitMask);

        selPrefReq.mode_pref_valid = true;
        selPrefReq.mode_pref = bitMask;

        qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(
            NasClient,
            QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01,
            &selPrefReq, sizeof(selPrefReq),
            &selPrefResp, sizeof(selPrefResp),
            COMM_LONG_PLATFORM_TIMEOUT);

        res = swiQmi_CheckResponseCode(
                            STRINGIZE_EXPAND(QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01),
                            clientMsgErr, selPrefResp.resp);
        if (LE_OK != res)
        {
            if (QMI_ERR_OP_DEVICE_UNSUPPORTED_V01 == selPrefResp.resp.error)
            {
                LE_ERROR("RAT is not supported on this platform");
                res = LE_UNSUPPORTED;
            }
            else
            {
                res = LE_FAULT;
            }
        }
    }

    return res;
}


//--------------------------------------------------------------------------------------------------
/**
 * Set the Rat Automatic Radio Access Technology Preference
 *
 * @return
 * - LE_OK              on success
 * - LE_FAULT           on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_SetAutomaticRatPreference
(
    void
)
{
    nas_set_technology_preference_req_msg_v01 selRatReq = {{0}};
    nas_set_technology_preference_resp_msg_v01 selRatResp = {{0}};

   LE_DEBUG("Set RAT Automatic Preference");

   selRatReq.technology_pref.technology_pref = 0;
   selRatReq.technology_pref.duration = NAS_ACTIVE_TECHNOLOGY_DURATION_PERMANENT_V01;

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(
        NasClient,
        QMI_NAS_SET_TECHNOLOGY_PREFERENCE_REQ_V01,
        &selRatReq, sizeof(selRatReq),
        &selRatResp, sizeof(selRatResp),
        COMM_NETWORK_TIMEOUT);

    le_result_t result = swiQmi_CheckResponseCode(
        STRINGIZE_EXPAND(QMI_NAS_SET_TECHNOLOGY_PREFERENCE_REQ_V01),
        clientMsgErr, selRatResp.resp);

    if (result == LE_OK)
    {
        return result;
    }
    else
    {
        return LE_FAULT;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the Radio Access Technology Preferences
 *
 * @return
 * - LE_OK              on success
 * - LE_FAULT           on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_GetRatPreferences
(
    le_mrc_RatBitMask_t* ratMaskPtr ///< [OUT] A bit mask to set the Radio Access Technology
                                    ///<  preferences.
)
{
    SWIQMI_DECLARE_MESSAGE(nas_get_system_selection_preference_resp_msg_v01, getSysSelecPrefResp);
    SWIQMI_DECLARE_MESSAGE(nas_get_technology_preference_resp_type_v01, selRatResp);
    mode_pref_mask_type_v01 ratMode=0;

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(
        NasClient,
        QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01,
        NULL, 0,
        &getSysSelecPrefResp, sizeof(getSysSelecPrefResp),
        COMM_NETWORK_TIMEOUT);

    if (LE_OK != swiQmi_CheckResponse(
                       STRINGIZE_EXPAND(QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01),
                       clientMsgErr,
                       getSysSelecPrefResp.resp.result,
                       getSysSelecPrefResp.resp.error))
    {
        LE_ERROR("Unable to get system selection preferences");
        return LE_FAULT;
    }

    if (getSysSelecPrefResp.mode_pref_valid)
    {
        ratMode = getSysSelecPrefResp.mode_pref;
        LE_DEBUG("Get RAT Preferences QMI bit mask: 0x%02X", ratMode);

        // Normally, "selRatResp.active_technology_pref.technology_pref" is used in
        // QMI_NAS_GET_TECHNOLOGY_PREFERENCE_REQ_V01 QMI API to determine if the RAT is set manually
        // or automatically.
        // However, QMI_NAS_GET_TECHNOLOGY_PREFERENCE_REQ_V01 QMI API does not cover the case when
        // the RAT is set to TDSCDMA mode only.
        // For this case, QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01 is used first.

        if (QMI_NAS_RAT_MODE_PREF_TDSCDMA_V01 == ratMode)
        {
            *ratMaskPtr = LE_MRC_BITMASK_RAT_TDSCDMA;
            return LE_OK;
        }
    }
    else
    {
        LE_WARN("No valid selection preferences");
    }

    clientMsgErr = qmi_client_send_msg_sync(
        NasClient,
        QMI_NAS_GET_TECHNOLOGY_PREFERENCE_REQ_V01,
        NULL, 0,
        &selRatResp, sizeof(selRatResp),
        COMM_NETWORK_TIMEOUT);

    if (LE_OK != swiQmi_CheckResponse(
                       STRINGIZE_EXPAND(QMI_NAS_GET_TECHNOLOGY_PREFERENCE_REQ_V01),
                       clientMsgErr,
                       selRatResp.resp.result,
                       selRatResp.resp.error))
    {
        LE_ERROR("Unable to get technology preferences");
        return LE_FAULT;
    }

    LE_DEBUG("Get technology preferences QMI bit mask: 0x%02X",
             selRatResp.active_technology_pref.technology_pref);

    if (0 == selRatResp.active_technology_pref.technology_pref)
    {
        *ratMaskPtr = LE_MRC_BITMASK_RAT_ALL;
        return LE_OK;
    }

    *ratMaskPtr = 0;
    if (ratMode  & (QMI_NAS_RAT_MODE_PREF_CDMA2000_1X_V01 |
                        QMI_NAS_RAT_MODE_PREF_CDMA2000_HRPD_V01))
    {
        *ratMaskPtr |= LE_MRC_BITMASK_RAT_CDMA;
    }

    if (ratMode & QMI_NAS_RAT_MODE_PREF_GSM_V01)
    {
        *ratMaskPtr |= LE_MRC_BITMASK_RAT_GSM;
    }

    if (ratMode & QMI_NAS_RAT_MODE_PREF_UMTS_V01)
    {
        *ratMaskPtr |= LE_MRC_BITMASK_RAT_UMTS;
    }

    if (ratMode & QMI_NAS_RAT_MODE_PREF_TDSCDMA_V01)
    {
        *ratMaskPtr |= LE_MRC_BITMASK_RAT_TDSCDMA;
    }

    if (ratMode & QMI_NAS_RAT_MODE_PREF_LTE_V01)
    {
        *ratMaskPtr |= LE_MRC_BITMASK_RAT_LTE;
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the Band Preferences
 *
 * @return
 * - LE_OK              on success
 * - LE_FAULT           on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_SetBandPreferences
(
    le_mrc_BandBitMask_t bands ///< [IN] A bit mask to set the Band preferences.
)
{
    nas_set_system_selection_preference_req_msg_v01 selPrefReq = {0};
    nas_get_system_selection_preference_resp_msg_v01 selPrefResp = {{0}};
    nas_band_pref_mask_type_v01 bitMask = 0;

    if (bands & LE_MRC_BITMASK_BAND_CLASS_0_A_SYSTEM)
    {
        bitMask |= QMI_NAS_BAND_CLASS_0_A_SYSTEM_V01;
    }
    if (bands & LE_MRC_BITMASK_BAND_CLASS_0_B_SYSTEM)
    {
        bitMask |= QMI_NAS_BAND_CLASS_0_B_AB_GSM850_V01;
    }
    if (bands & LE_MRC_BITMASK_BAND_CLASS_1_ALL_BLOCKS)
    {
        bitMask |= QMI_NAS_BAND_CLASS_1_ALL_BLOCKS_V01;
    }
    if (bands & LE_MRC_BITMASK_BAND_CLASS_2_PLACEHOLDER)
    {
        bitMask |= QMI_NAS_BAND_CLASS_2_PLACEHOLDER_V01;
    }
    if (bands & LE_MRC_BITMASK_BAND_CLASS_3_A_SYSTEM)
    {
        bitMask |= QMI_NAS_BAND_CLASS_3_A_SYSTEM_V01;
    }
    if (bands & LE_MRC_BITMASK_BAND_CLASS_4_ALL_BLOCKS)
    {
        bitMask |= QMI_NAS_BAND_CLASS_4_ALL_BLOCKS_V01;
    }
    if (bands & LE_MRC_BITMASK_BAND_CLASS_5_ALL_BLOCKS)
    {
        bitMask |= QMI_NAS_BAND_CLASS_5_ALL_BLOCKS_V01;
    }
    if (bands & LE_MRC_BITMASK_BAND_GSM_DCS_1800)
    {
        bitMask |= QMI_NAS_GSM_DCS_1800_BAND_V01;
    }
    if (bands & LE_MRC_BITMASK_BAND_EGSM_900)
    {
        bitMask |= QMI_NAS_E_GSM_900_BAND_V01;
    }
    if (bands & LE_MRC_BITMASK_BAND_PRI_GSM_900)
    {
        bitMask |= QMI_NAS_P_GSM_900_BAND_V01;
    }
    if (bands & LE_MRC_BITMASK_BAND_CLASS_6)
    {
        bitMask |= QMI_NAS_BAND_CLASS_6_V01;
    }
    if (bands & LE_MRC_BITMASK_BAND_CLASS_7)
    {
        bitMask |= QMI_NAS_BAND_CLASS_7_V01;
    }
    if (bands & LE_MRC_BITMASK_BAND_CLASS_8)
    {
        bitMask |= QMI_NAS_BAND_CLASS_8_V01;
    }
    if (bands & LE_MRC_BITMASK_BAND_CLASS_9)
    {
        bitMask |= QMI_NAS_BAND_CLASS_9_V01;
    }
    if (bands & LE_MRC_BITMASK_BAND_CLASS_10)
    {
        bitMask |= QMI_NAS_BAND_CLASS_10_V01;
    }
    if (bands & LE_MRC_BITMASK_BAND_CLASS_11)
    {
        bitMask |= QMI_NAS_BAND_CLASS_11_V01;
    }
    if (bands & LE_MRC_BITMASK_BAND_GSM_450)
    {
        bitMask |= QMI_NAS_GSM_BAND_450_V01;
    }
    if (bands & LE_MRC_BITMASK_BAND_GSM_480)
    {
        bitMask |= QMI_NAS_GSM_BAND_480_V01;
    }
    if (bands & LE_MRC_BITMASK_BAND_GSM_750)
    {
        bitMask |= QMI_NAS_GSM_BAND_750_V01;
    }
    if (bands & LE_MRC_BITMASK_BAND_GSM_850)
    {
        bitMask |= QMI_NAS_GSM_BAND_850_V01;
    }
    if (bands & LE_MRC_BITMASK_BAND_GSMR_900)
    {
        bitMask |= QMI_NAS_GSM_BAND_RAILWAYS_900_BAND_V01;
    }
    if (bands & LE_MRC_BITMASK_BAND_GSM_PCS_1900)
    {
        bitMask |= QMI_NAS_GSM_BAND_PCS_1900_BAND_V01;
    }
    if (bands & LE_MRC_BITMASK_BAND_WCDMA_EU_J_CH_IMT_2100)
    {
        bitMask |= QMI_NAS_WCDMA_EU_J_CH_IMT_2100_BAND_V01;
    }
    if (bands & LE_MRC_BITMASK_BAND_WCDMA_US_PCS_1900)
    {
        bitMask |= QMI_NAS_WCDMA_US_PCS_1900_BAND_V01;
    }
    if (bands & LE_MRC_BITMASK_BAND_WCDMA_EU_CH_DCS_1800)
    {
        bitMask |= QMI_NAS_EU_CH_DCS_1800_BAND_V01;
    }
    if (bands & LE_MRC_BITMASK_BAND_WCDMA_US_1700)
    {
        bitMask |= QMI_NAS_WCDMA_US_1700_BAND_V01;
    }
    if (bands & LE_MRC_BITMASK_BAND_WCDMA_US_850)
    {
        bitMask |= QMI_NAS_WCDMA_US_850_BAND_V01;
    }
    if (bands & LE_MRC_BITMASK_BAND_WCDMA_J_800)
    {
        bitMask |= QMI_NAS_WCDMA_JAPAN_800_BAND_V01;
    }
    if (bands & LE_MRC_BITMASK_BAND_CLASS_12)
    {
        bitMask |= QMI_NAS_BAND_CLASS_12_V01;
    }
    if (bands & LE_MRC_BITMASK_BAND_CLASS_14)
    {
        bitMask |= QMI_NAS_BAND_CLASS_14_V01;
    }
    if (bands & LE_MRC_BITMASK_BAND_CLASS_15)
    {
        bitMask |= QMI_NAS_BAND_CLASS_15_V01;
    }
    if (bands & LE_MRC_BITMASK_BAND_WCDMA_EU_2600)
    {
        bitMask |= QMI_NAS_WCDMA_EU_2600_BAND_V01;
    }
    if (bands & LE_MRC_BITMASK_BAND_WCDMA_EU_J_900)
    {
        bitMask |= QMI_NAS_WCDMA_EU_J_900_BAND_V01;
    }
    if (bands & LE_MRC_BITMASK_BAND_WCDMA_J_1700)
    {
        bitMask |= QMI_NAS_WCDMA_J_1700_BAND_V01;
    }
    if (bands & LE_MRC_BITMASK_BAND_CLASS_16)
    {
        bitMask |= QMI_NAS_BAND_CLASS_16_V01;
    }
    if (bands & LE_MRC_BITMASK_BAND_CLASS_17)
    {
        bitMask |= QMI_NAS_BAND_CLASS_17_V01;
    }
    if (bands & LE_MRC_BITMASK_BAND_CLASS_18)
    {
        bitMask |= QMI_NAS_BAND_CLASS_18_V01;
    }
    if (bands & LE_MRC_BITMASK_BAND_CLASS_19)
    {
        bitMask |= QMI_NAS_BAND_CLASS_19_V01;
    }
    LE_DEBUG("Set Band Preferences QMI bit mask: 0x%016"PRIX64, bitMask);

    selPrefReq.band_pref_valid = true;
    selPrefReq.band_pref = bitMask;

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(
        NasClient,
        QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01,
        &selPrefReq, sizeof(selPrefReq),
        &selPrefResp, sizeof(selPrefResp),
        COMM_LONG_PLATFORM_TIMEOUT);

    le_result_t result = swiQmi_CheckResponseCode(
        STRINGIZE_EXPAND(QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01),
        clientMsgErr, selPrefResp.resp);

    if (result == LE_OK)
    {
        return result;
    }
    else
    {
        return LE_FAULT;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the Band Preferences
 *
 * @return
 * - LE_OK              on success
 * - LE_FAULT           on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_GetBandPreferences
(
    le_mrc_BandBitMask_t* bandsPtr ///< [OUT] A bit mask to get the Band preferences.
)
{
    nas_get_system_selection_preference_resp_msg_v01 getSysSelecPrefResp = { {0} };
    nas_band_pref_mask_type_v01 bitMask = 0;
    le_mrc_BandBitMask_t bands = 0;

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(
                    NasClient,
                    QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01,
                    NULL, 0,
                    &getSysSelecPrefResp, sizeof(getSysSelecPrefResp),
                    COMM_LONG_PLATFORM_TIMEOUT);

    le_result_t respResult = swiQmi_CheckResponseCode(
                    STRINGIZE_EXPAND(QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01),
                    clientMsgErr,
                    getSysSelecPrefResp.resp);

    if (respResult != LE_OK)
    {
        return LE_FAULT;
    }

    if(getSysSelecPrefResp.band_pref_valid == false)
    {
        LE_ERROR("GSM Bands preferences not present !!");
        return LE_FAULT;
    }

    bitMask = getSysSelecPrefResp.band_pref;
    LE_DEBUG("Get Band Preferences QMI bit mask: 0x%016"PRIX64, bitMask);

    if ( bitMask & QMI_NAS_BAND_CLASS_0_A_SYSTEM_V01)
    {
        bands |= LE_MRC_BITMASK_BAND_CLASS_0_A_SYSTEM;
    }

    if ( bitMask & QMI_NAS_BAND_CLASS_0_B_AB_GSM850_V01)
    {
        bands |= LE_MRC_BITMASK_BAND_CLASS_0_B_SYSTEM;
    }

    if ( bitMask & QMI_NAS_BAND_CLASS_1_ALL_BLOCKS_V01)
    {
        bands |= LE_MRC_BITMASK_BAND_CLASS_1_ALL_BLOCKS;
    }

    if ( bitMask & QMI_NAS_BAND_CLASS_2_PLACEHOLDER_V01)
    {
        bands |= LE_MRC_BITMASK_BAND_CLASS_2_PLACEHOLDER;
    }

    if ( bitMask & QMI_NAS_BAND_CLASS_3_A_SYSTEM_V01)
    {
        bands |= LE_MRC_BITMASK_BAND_CLASS_3_A_SYSTEM;
    }

    if ( bitMask & QMI_NAS_BAND_CLASS_4_ALL_BLOCKS_V01)
    {
        bands |= LE_MRC_BITMASK_BAND_CLASS_4_ALL_BLOCKS;
    }

    if ( bitMask & QMI_NAS_BAND_CLASS_5_ALL_BLOCKS_V01)
    {
        bands |= LE_MRC_BITMASK_BAND_CLASS_5_ALL_BLOCKS;
    }

    if ( bitMask & QMI_NAS_GSM_DCS_1800_BAND_V01)
    {
        bands |= LE_MRC_BITMASK_BAND_GSM_DCS_1800;
    }

    if ( bitMask & QMI_NAS_BAND_CLASS_5_ALL_BLOCKS_V01)
    {
        bands |= LE_MRC_BITMASK_BAND_CLASS_5_ALL_BLOCKS;
    }

    if ( bitMask & QMI_NAS_GSM_DCS_1800_BAND_V01)
    {
        bands |= LE_MRC_BITMASK_BAND_GSM_DCS_1800;
    }

    if ( bitMask & QMI_NAS_E_GSM_900_BAND_V01)
    {
        bands |= LE_MRC_BITMASK_BAND_EGSM_900;
    }

    if ( bitMask & QMI_NAS_P_GSM_900_BAND_V01)
    {
        bands |= LE_MRC_BITMASK_BAND_PRI_GSM_900;
    }

    if ( bitMask & QMI_NAS_E_GSM_900_BAND_V01)
    {
        bands |= LE_MRC_BITMASK_BAND_EGSM_900;
    }

    if ( bitMask & QMI_NAS_P_GSM_900_BAND_V01)
    {
        bands |= LE_MRC_BITMASK_BAND_PRI_GSM_900;
    }

    if ( bitMask & QMI_NAS_BAND_CLASS_6_V01)
    {
        bands |= LE_MRC_BITMASK_BAND_CLASS_6;
    }

    if ( bitMask & QMI_NAS_BAND_CLASS_7_V01)
    {
        bands |= LE_MRC_BITMASK_BAND_CLASS_7;
    }

    if ( bitMask & QMI_NAS_BAND_CLASS_8_V01)
    {
        bands |= LE_MRC_BITMASK_BAND_CLASS_8;
    }

    if ( bitMask & QMI_NAS_BAND_CLASS_9_V01)
    {
        bands |= LE_MRC_BITMASK_BAND_CLASS_9;
    }

    if ( bitMask & QMI_NAS_BAND_CLASS_10_V01)
    {
        bands |= LE_MRC_BITMASK_BAND_CLASS_10;
    }

    if ( bitMask & QMI_NAS_BAND_CLASS_11_V01)
    {
        bands |= LE_MRC_BITMASK_BAND_CLASS_11;
    }

    if ( bitMask & QMI_NAS_GSM_BAND_450_V01)
    {
        bands |= LE_MRC_BITMASK_BAND_GSM_450;
    }

    if ( bitMask & QMI_NAS_GSM_BAND_480_V01)
    {
        bands |= LE_MRC_BITMASK_BAND_GSM_480;
    }

    if ( bitMask & QMI_NAS_GSM_BAND_750_V01)
    {
        bands |= LE_MRC_BITMASK_BAND_GSM_750;
    }

    if ( bitMask & QMI_NAS_GSM_BAND_850_V01)
    {
        bands |= LE_MRC_BITMASK_BAND_GSM_850;
    }

    if ( bitMask & QMI_NAS_GSM_BAND_RAILWAYS_900_BAND_V01)
    {
        bands |= LE_MRC_BITMASK_BAND_GSMR_900;
    }

    if ( bitMask & QMI_NAS_GSM_BAND_PCS_1900_BAND_V01)
    {
        bands |= LE_MRC_BITMASK_BAND_GSM_PCS_1900;
    }

    if ( bitMask & QMI_NAS_WCDMA_EU_J_CH_IMT_2100_BAND_V01)
    {
        bands |= LE_MRC_BITMASK_BAND_WCDMA_EU_J_CH_IMT_2100;
    }

    if ( bitMask & QMI_NAS_WCDMA_US_PCS_1900_BAND_V01)
    {
        bands |= LE_MRC_BITMASK_BAND_WCDMA_US_PCS_1900;
    }

    if ( bitMask & QMI_NAS_EU_CH_DCS_1800_BAND_V01)
    {
        bands |= LE_MRC_BITMASK_BAND_WCDMA_EU_CH_DCS_1800;
    }

    if ( bitMask & QMI_NAS_WCDMA_US_1700_BAND_V01)
    {
        bands |= LE_MRC_BITMASK_BAND_WCDMA_US_1700;
    }

    if ( bitMask & QMI_NAS_WCDMA_US_850_BAND_V01)
    {
        bands |= LE_MRC_BITMASK_BAND_WCDMA_US_850;
    }

    if ( bitMask & QMI_NAS_WCDMA_JAPAN_800_BAND_V01)
    {
        bands |= LE_MRC_BITMASK_BAND_WCDMA_J_800;
    }

    if ( bitMask & QMI_NAS_BAND_CLASS_12_V01)
    {
        bands |= LE_MRC_BITMASK_BAND_CLASS_12;
    }

    if ( bitMask & QMI_NAS_BAND_CLASS_14_V01)
    {
        bands |= LE_MRC_BITMASK_BAND_CLASS_14;
    }

    if ( bitMask & QMI_NAS_BAND_CLASS_15_V01)
    {
        bands |= LE_MRC_BITMASK_BAND_CLASS_15;
    }

    if ( bitMask & QMI_NAS_WCDMA_EU_2600_BAND_V01)
    {
        bands |= LE_MRC_BITMASK_BAND_WCDMA_EU_2600;
    }

    if ( bitMask & QMI_NAS_WCDMA_EU_J_900_BAND_V01)
    {
        bands |= LE_MRC_BITMASK_BAND_WCDMA_EU_J_900;
    }

    if ( bitMask & QMI_NAS_WCDMA_J_1700_BAND_V01)
    {
        bands |= LE_MRC_BITMASK_BAND_WCDMA_J_1700;
    }

    if ( bitMask & QMI_NAS_BAND_CLASS_16_V01)
    {
        bands |= LE_MRC_BITMASK_BAND_CLASS_16;
    }

    if ( bitMask & QMI_NAS_BAND_CLASS_17_V01)
    {
        bands |= LE_MRC_BITMASK_BAND_CLASS_17;
    }
    if ( bitMask & QMI_NAS_BAND_CLASS_18_V01)
    {
        bands |= LE_MRC_BITMASK_BAND_CLASS_18;
    }
    if ( bitMask & QMI_NAS_BAND_CLASS_19_V01)
    {
        bands |= LE_MRC_BITMASK_BAND_CLASS_19;
    }

    *bandsPtr = bands;

    LE_DEBUG("Get Band Preferences MRC mask: 0x%016"PRIX64, bands);

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the LTE Band Preferences
 *
 * @return
 * - LE_OK              on success
 * - LE_FAULT           on failure
 * - LE_UNSUPPORTED     the platform doesn't support setting LTE Band preferences.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_SetLteBandPreferences
(
    le_mrc_LteBandBitMask_t bands ///< [IN] A bit mask to set the LTE Band preferences.
)
{
    nas_set_system_selection_preference_req_msg_v01 selPrefReq = {0};
    nas_get_system_selection_preference_resp_msg_v01 selPrefResp = {{0}};
    lte_band_pref_mask_type_v01 bitMask = 0;

    if (!(RatCapabilities & QMI_NAS_RAT_MODE_PREF_LTE_V01))
    {
        LE_ERROR("LE_MRC_BITMASK_RAT_LTE is not supported");
        return LE_UNSUPPORTED;
    }

    if (bands & LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_1)
    {
        bitMask |= E_UTRA_OPERATING_BAND_1_V01;
    }
    if (bands & LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_2)
    {
        bitMask |= E_UTRA_OPERATING_BAND_2_V01;
    }
    if (bands & LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_3)
    {
        bitMask |= E_UTRA_OPERATING_BAND_3_V01;
    }
    if (bands & LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_4)
    {
        bitMask |= E_UTRA_OPERATING_BAND_4_V01;
    }
    if (bands & LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_5)
    {
        bitMask |= E_UTRA_OPERATING_BAND_5_V01;
    }
    if (bands & LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_6)
    {
        bitMask |= E_UTRA_OPERATING_BAND_6_V01;
    }
    if (bands & LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_7)
    {
        bitMask |= E_UTRA_OPERATING_BAND_7_V01;
    }
    if (bands & LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_8)
    {
        bitMask |= E_UTRA_OPERATING_BAND_8_V01;
    }
    if (bands & LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_9)
    {
        bitMask |= E_UTRA_OPERATING_BAND_9_V01;
    }
    if (bands & LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_10)
    {
        bitMask |= E_UTRA_OPERATING_BAND_10_V01;
    }
    if (bands & LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_11)
    {
        bitMask |= E_UTRA_OPERATING_BAND_11_V01;
    }
    if (bands & LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_12)
    {
        bitMask |= E_UTRA_OPERATING_BAND_12_V01;
    }
    if (bands & LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_13)
    {
        bitMask |= E_UTRA_OPERATING_BAND_13_V01;
    }
    if (bands & LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_14)
    {
        bitMask |= E_UTRA_OPERATING_BAND_14_V01;
    }
    if (bands & LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_17)
    {
        bitMask |= E_UTRA_OPERATING_BAND_17_V01;
    }
    if (bands & LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_18)
    {
        bitMask |= E_UTRA_OPERATING_BAND_18_V01;
    }
    if (bands & LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_19)
    {
        bitMask |= E_UTRA_OPERATING_BAND_19_V01;
    }
    if (bands & LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_20)
    {
        bitMask |= E_UTRA_OPERATING_BAND_20_V01;
    }
    if (bands & LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_21)
    {
        bitMask |= E_UTRA_OPERATING_BAND_21_V01;
    }
    if (bands & LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_24)
    {
        bitMask |= E_UTRA_OPERATING_BAND_24_V01;
    }
    if (bands & LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_25)
    {
        bitMask |= E_UTRA_OPERATING_BAND_25_V01;
    }
    if (bands & LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_33)
    {
        bitMask |= E_UTRA_OPERATING_BAND_33_V01;
    }
    if (bands & LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_34)
    {
        bitMask |= E_UTRA_OPERATING_BAND_34_V01;
    }
    if (bands & LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_35)
    {
        bitMask |= E_UTRA_OPERATING_BAND_35_V01;
    }
    if (bands & LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_36)
    {
        bitMask |= E_UTRA_OPERATING_BAND_36_V01;
    }
    if (bands & LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_37)
    {
        bitMask |= E_UTRA_OPERATING_BAND_37_V01;
    }
    if (bands & LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_38)
    {
        bitMask |= E_UTRA_OPERATING_BAND_38_V01;
    }
    if (bands & LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_39)
    {
        bitMask |= E_UTRA_OPERATING_BAND_39_V01;
    }
    if (bands & LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_40)
    {
        bitMask |= E_UTRA_OPERATING_BAND_40_V01;
    }
    if (bands & LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_41)
    {
        bitMask |= E_UTRA_OPERATING_BAND_41_V01;
    }
    if (bands & LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_42)
    {
        bitMask |= E_UTRA_OPERATING_BAND_42_V01;
    }
    if (bands & LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_43)
    {
        bitMask |= E_UTRA_OPERATING_BAND_43_V01;
    }

    LE_DEBUG("Set LTE Band Preferences QMI bit mask: 0x%016"PRIX64, bitMask);

    selPrefReq.lte_band_pref_valid = true;
    selPrefReq.lte_band_pref = bitMask;

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(
        NasClient,
        QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01,
        &selPrefReq, sizeof(selPrefReq),
        &selPrefResp, sizeof(selPrefResp),
        COMM_LONG_PLATFORM_TIMEOUT);

    le_result_t result = swiQmi_CheckResponseCode(
        STRINGIZE_EXPAND(QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01),
        clientMsgErr, selPrefResp.resp);

    if (result == LE_OK)
    {
        return result;
    }
    else
    {
        return LE_FAULT;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the LTE Band Preferences
 *
 * @return
 * - LE_OK              on success
 * - LE_FAULT           on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_GetLteBandPreferences
(
    le_mrc_LteBandBitMask_t* bandsPtr ///< [OUT] A bit mask to get the LTE Band preferences.
)
{
    nas_get_system_selection_preference_resp_msg_v01 getSysSelecPrefResp = { {0} };
    uint64_t bitMask = 0;
    le_mrc_LteBandBitMask_t bands = 0;

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(
                    NasClient,
                    QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01,
                    NULL, 0,
                    &getSysSelecPrefResp, sizeof(getSysSelecPrefResp),
                    COMM_LONG_PLATFORM_TIMEOUT);

    le_result_t respResult = swiQmi_CheckResponseCode(
                    STRINGIZE_EXPAND(QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01),
                    clientMsgErr,
                    getSysSelecPrefResp.resp);

    if (respResult != LE_OK)
    {
        return LE_FAULT;
    }

    if(getSysSelecPrefResp.band_pref_ext_valid == false)
    {
        LE_ERROR("LTE Bands preferences not present !!");
        return LE_FAULT;
    }

    bitMask = getSysSelecPrefResp.band_pref_ext;
    LE_DEBUG("Get LTE Band Preferences QMI bit mask: 0x%016"PRIX64, bitMask);


    if (bitMask & E_UTRA_OPERATING_BAND_1_V01)
    {
        bands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_1;
    }
    if (bitMask & E_UTRA_OPERATING_BAND_2_V01)
    {
        bands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_2;
    }
    if (bitMask & E_UTRA_OPERATING_BAND_3_V01)
    {
        bands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_3;
    }
    if (bitMask & E_UTRA_OPERATING_BAND_4_V01)
    {
        bands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_4;
    }

    if (bitMask & E_UTRA_OPERATING_BAND_5_V01)
    {
        bands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_5;
    }
    if (bitMask & E_UTRA_OPERATING_BAND_6_V01)
    {
        bands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_6;
    }
    if (bitMask & E_UTRA_OPERATING_BAND_7_V01)
    {
        bands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_7;
    }
    if (bitMask & E_UTRA_OPERATING_BAND_8_V01)
    {
        bands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_8;
    }
    if (bitMask & E_UTRA_OPERATING_BAND_9_V01)
    {
        bands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_9;
    }

    if (bitMask & E_UTRA_OPERATING_BAND_10_V01)
    {
        bands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_10;
    }
    if (bitMask & E_UTRA_OPERATING_BAND_11_V01)
    {
        bands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_11;
    }
    if (bitMask & E_UTRA_OPERATING_BAND_12_V01)
    {
        bands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_12;
    }
    if (bitMask & E_UTRA_OPERATING_BAND_13_V01)
    {
        bands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_13;
    }
    if (bitMask & E_UTRA_OPERATING_BAND_14_V01)
    {
        bands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_14;
    }

    if (bitMask & E_UTRA_OPERATING_BAND_17_V01)
    {
        bands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_17;
    }
    if (bitMask & E_UTRA_OPERATING_BAND_18_V01)
    {
        bands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_18;
    }
    if (bitMask & E_UTRA_OPERATING_BAND_19_V01)
    {
        bands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_19;
    }

    if (bitMask & E_UTRA_OPERATING_BAND_20_V01)
    {
        bands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_20;
    }
    if (bitMask & E_UTRA_OPERATING_BAND_21_V01)
    {
        bands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_21;
    }
    if (bitMask & E_UTRA_OPERATING_BAND_24_V01)
    {
        bands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_24;
    }
    if (bitMask & E_UTRA_OPERATING_BAND_25_V01)
    {
        bands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_25;
    }

    if (bitMask & E_UTRA_OPERATING_BAND_33_V01)
    {
        bands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_33;
    }
    if (bitMask & E_UTRA_OPERATING_BAND_34_V01)
    {
        bands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_34;
    }
    if (bitMask & E_UTRA_OPERATING_BAND_35_V01)
    {
        bands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_35;
    }
    if (bitMask & E_UTRA_OPERATING_BAND_36_V01)
    {
        bands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_36;
    }
    if (bitMask & E_UTRA_OPERATING_BAND_37_V01)
    {
        bands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_37;
    }
    if (bitMask & E_UTRA_OPERATING_BAND_38_V01)
    {
        bands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_38;
    }
    if (bitMask & E_UTRA_OPERATING_BAND_39_V01)
    {
        bands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_39;
    }

    if (bitMask & E_UTRA_OPERATING_BAND_40_V01)
    {
        bands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_40;
    }
    if (bitMask & E_UTRA_OPERATING_BAND_41_V01)
    {
        bands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_41;
    }
    if (bitMask & E_UTRA_OPERATING_BAND_42_V01)
    {
        bands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_42;
    }
    if (bitMask & E_UTRA_OPERATING_BAND_43_V01)
    {
        bands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_43;
    }

    *bandsPtr = bands;

    LE_DEBUG("Get Band Preferences MRC mask: 0x%016"PRIX64, (uint64_t) bands);

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the TD-SCDMA Band Preferences
 *
 * @return
 * - LE_OK           On success
 * - LE_FAULT        On failure
 * - LE_UNSUPPORTED  The platform doesn't support setting TD-SCDMA Band preferences.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_SetTdScdmaBandPreferences
(
    le_mrc_TdScdmaBandBitMask_t bands ///< [IN] A bit mask to set the TD-SCDMA Band Preferences.
)
{
    SWIQMI_DECLARE_MESSAGE(nas_set_system_selection_preference_req_msg_v01, selPrefReq);
    SWIQMI_DECLARE_MESSAGE(nas_get_system_selection_preference_resp_msg_v01, selPrefResp);

    if (!(RatCapabilities & QMI_NAS_RAT_MODE_PREF_TDSCDMA_V01))
    {
        LE_ERROR("LE_MRC_BITMASK_RAT_TDSCDMA is not supported");
        return LE_UNSUPPORTED;
    }

    nas_tdscdma_band_pref_mask_type_v01 bitMask = 0x00;
    if (bands & LE_MRC_BITMASK_TDSCDMA_BAND_A)
    {
        bitMask |= NAS_TDSCDMA_BAND_A_V01;
    }
    if (bands & LE_MRC_BITMASK_TDSCDMA_BAND_B)
    {
        bitMask |= NAS_TDSCDMA_BAND_B_V01;
    }
    if (bands & LE_MRC_BITMASK_TDSCDMA_BAND_C)
    {
        bitMask |= NAS_TDSCDMA_BAND_C_V01;
    }
    if (bands & LE_MRC_BITMASK_TDSCDMA_BAND_D)
    {
        bitMask |= NAS_TDSCDMA_BAND_D_V01;
    }
    if (bands & LE_MRC_BITMASK_TDSCDMA_BAND_E)
    {
        bitMask |= NAS_TDSCDMA_BAND_E_V01;
    }
    if (bands & LE_MRC_BITMASK_TDSCDMA_BAND_F)
    {
        bitMask |= NAS_TDSCDMA_BAND_F_V01;
    }
    LE_DEBUG("Set TD-SCDMA Band Preferences QMI bit mask: 0x%"PRIX64, bitMask);

    selPrefReq.tdscdma_band_pref_valid = true;
    selPrefReq.tdscdma_band_pref = bitMask;

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(
        NasClient,
        QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01,
        &selPrefReq, sizeof(selPrefReq),
        &selPrefResp, sizeof(selPrefResp),
        COMM_LONG_PLATFORM_TIMEOUT);

    if (LE_OK != swiQmi_CheckResponse(
                             STRINGIZE_EXPAND(QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01),
                             clientMsgErr,
                             selPrefResp.resp.result,
                             selPrefResp.resp.error))
    {
        LE_ERROR("Unable to set the TD-SCDMA Band preferences.");
        return LE_FAULT;
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the TD-SCDMA Band Preferences
 *
 * @return
 * - LE_OK           On success
 * - LE_FAULT        On failure
 * - LE_UNSUPPORTED  The platform doesn't support getting TD-SCDMA Band preferences.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_GetTdScdmaBandPreferences
(
    le_mrc_TdScdmaBandBitMask_t* bandsPtr ///< [OUT] A bit mask to get the TD-SCDMA Band preference.
)
{
    SWIQMI_DECLARE_MESSAGE(nas_get_system_selection_preference_resp_msg_v01, getSysSelecPrefResp);
    uint64_t bitMask = 0;
    le_mrc_TdScdmaBandBitMask_t bands = 0;

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(
                    NasClient,
                    QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01,
                    NULL, 0,
                    &getSysSelecPrefResp, sizeof(getSysSelecPrefResp),
                    COMM_NETWORK_TIMEOUT);

    if (LE_OK != swiQmi_CheckResponse(
                        STRINGIZE_EXPAND(QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01),
                        clientMsgErr,
                        getSysSelecPrefResp.resp.result,
                        getSysSelecPrefResp.resp.error))
    {
        LE_ERROR("Unable to get TD-SCDMA Band preferences.");
        return LE_FAULT;
    }

    if (false == getSysSelecPrefResp.tdscdma_band_pref_valid)
    {
        LE_ERROR("TD-SCDMA Band preferences is not present !!");
        return LE_UNSUPPORTED;
    }

    bitMask = getSysSelecPrefResp.tdscdma_band_pref;
    LE_DEBUG("Get TD-SCDMA Band Preferences QMI bit mask: 0x%016"PRIX64, bitMask);

    if (bitMask & NAS_TDSCDMA_BAND_A_V01)
    {
        bands |= LE_MRC_BITMASK_TDSCDMA_BAND_A;
    }
    if (bitMask & NAS_TDSCDMA_BAND_B_V01)
    {
        bands |= LE_MRC_BITMASK_TDSCDMA_BAND_B;
    }
    if (bitMask & NAS_TDSCDMA_BAND_C_V01)
    {
        bands |= LE_MRC_BITMASK_TDSCDMA_BAND_C;
    }
    if (bitMask & NAS_TDSCDMA_BAND_D_V01)
    {
        bands |= LE_MRC_BITMASK_TDSCDMA_BAND_D;
    }
    if (bitMask & NAS_TDSCDMA_BAND_E_V01)
    {
        bands |= LE_MRC_BITMASK_TDSCDMA_BAND_E;
    }
    if (bitMask & NAS_TDSCDMA_BAND_F_V01)
    {
        bands |= LE_MRC_BITMASK_TDSCDMA_BAND_F;
    }

    if (0 == bands)
    {
        LE_ERROR("TD-SCDMA Band preferences is not supported");
        return LE_UNSUPPORTED;
    }

    *bandsPtr = bands;

    LE_DEBUG("Get TD-SCDMA Band Preferences MRC mask: 0x%016"PRIX64, (uint64_t) bands);

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function retrieves the Neighboring Cells information.
 * Each cell information is queued in the list specified with the IN/OUT parameter.
 * Neither add nor remove of elements in the list can be done outside this function.
 *
 * @return LE_FAULT          The function failed to retrieve the Neighboring Cells information.
 * @return a positive value  The function succeeded. The number of cells which the information have
 *                           been retrieved.
 */
//--------------------------------------------------------------------------------------------------
int32_t pa_mrc_GetNeighborCellsInfo
(
    le_dls_List_t*   cellInfoListPtr    ///< [IN/OUT] The Neighboring Cells information.
)
{
    uint32_t total=0, savTotal=0, i=0, j=0;

    SWIQMI_DECLARE_MESSAGE(nas_get_cell_location_info_resp_msg_v01, getLocResp);

    if (NULL == cellInfoListPtr)
    {
        LE_FATAL("cellInfoListPtr is NULL !");
    }

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(
                    NasClient,
                    QMI_NAS_GET_CELL_LOCATION_INFO_REQ_MSG_V01,
                    NULL, 0,
                    &getLocResp, sizeof(getLocResp),
                    COMM_NETWORK_TIMEOUT);

    if (LE_OK != swiQmi_CheckResponse(
                    STRINGIZE_EXPAND(QMI_NAS_GET_CELL_LOCATION_INFO_REQ_MSG_V01),
                    clientMsgErr,
                    getLocResp.resp.result,
                    getLocResp.resp.error))
    {
        LE_DEBUG("Failed to retrieve Neighboring Cells Information!");
        return LE_FAULT;
    }

    LE_DEBUG("geran_info_valid.%d, umts_info_valid.%d, cdma_info_valid.%d, "
             "lte_intra_valid.%d, lte_inter_valid.%d",
             getLocResp.geran_info_valid,
             getLocResp.umts_info_valid,
             getLocResp.cdma_info_valid,
             getLocResp.lte_intra_valid,
             getLocResp.lte_inter_valid);
    LE_DEBUG("lte_gsm_valid.%d, lte_wcdma_valid.%d, umts_cell_id_valid.%d, "
             "wcdma_lte_valid.%d",
             getLocResp.lte_gsm_valid,
             getLocResp.lte_wcdma_valid,
             getLocResp.umts_cell_id_valid,
             getLocResp.wcdma_lte_valid);

    if (getLocResp.geran_info_valid)
    {
        // Main cell in GERAN, get GERAN neighboring cells.
        LE_INFO("Main cell in GERAN, get %d GERAN neighboring cells",
                getLocResp.geran_info.nmr_cell_info_len);

        for (i=0; i<getLocResp.geran_info.nmr_cell_info_len; i++)
        {
            pa_mrc_CellInfo_t* cellInfoPtr =
                                (pa_mrc_CellInfo_t*)le_mem_ForceAlloc(CellInfoPool);

            LE_DEBUG("neighbors GERAN [%d] nmr_cell_id.%d nmr_lac.%d nmr_arfcn.%d"
                     " nmr_bsic.%d nmr_rx_lev.%d",
                     i,
                     getLocResp.geran_info.nmr_cell_info[i].nmr_cell_id,
                     getLocResp.geran_info.nmr_cell_info[i].nmr_lac,
                     getLocResp.geran_info.nmr_cell_info[i].nmr_arfcn,
                     getLocResp.geran_info.nmr_cell_info[i].nmr_bsic,
                     getLocResp.geran_info.nmr_cell_info[i].nmr_rx_lev-111);

            cellInfoPtr->index = i;
            cellInfoPtr->id = getLocResp.geran_info.nmr_cell_info[i].nmr_cell_id;
            if (UINT32_MAX == cellInfoPtr->id)
            {
                cellInfoPtr->lac = UINT16_MAX;
            }
            else
            {
                cellInfoPtr->lac = getLocResp.geran_info.nmr_cell_info[i].nmr_lac;
            }
            cellInfoPtr->rxLevel = getLocResp.geran_info.nmr_cell_info[i].nmr_rx_lev-111;
            cellInfoPtr->rat = LE_MRC_RAT_GSM;
            cellInfoPtr->link = LE_DLS_LINK_INIT;

            le_dls_Queue(cellInfoListPtr, &(cellInfoPtr->link));
        }
    }
    total += i;

    if (getLocResp.lte_gsm_valid)
    {
        // Main cell in LTE, get GERAN neighboring cells.
        for (i=0, savTotal=total; i<getLocResp.lte_gsm.freqs_len; i++)
        {
            for (j=0; j<getLocResp.lte_gsm.freqs[i].cells_len; j++)
            {
                pa_mrc_CellInfo_t* cellInfoPtr =
                                (pa_mrc_CellInfo_t*) le_mem_ForceAlloc(CellInfoPool);
                LE_DEBUG("neighbors GSM Cell [%d] ARFCN.%d Bsic.%d Rssi.%d",
                         i,
                         getLocResp.lte_gsm.freqs[i].cells[j].arfcn,
                         getLocResp.lte_gsm.freqs[i].cells[j].bsic_id,
                         getLocResp.lte_gsm.freqs[i].cells[j].rssi/10);

                cellInfoPtr->index = total + i + j;

                // No cell id available.
                cellInfoPtr->id = UINT32_MAX;
                // No LAC available.
                cellInfoPtr->lac = UINT16_MAX;
                cellInfoPtr->rxLevel = getLocResp.lte_gsm.freqs[i].cells[j].rssi/10;
                cellInfoPtr->rat = LE_MRC_RAT_GSM;
                cellInfoPtr->link = LE_DLS_LINK_INIT;

                le_dls_Queue(cellInfoListPtr, &(cellInfoPtr->link));
            }
            total += j;
        }
        LE_INFO("Main cell in LTE, get %d GERAN neighboring cells", total - savTotal);
    }

    if (getLocResp.lte_wcdma_valid)
    {
        // Main cell in LTE, get UMTS neighboring cells.
        for (i=0, savTotal=total; i<getLocResp.lte_wcdma.freqs_len; i++)
        {
            LE_DEBUG("[LTE] UMTS Cells %d uarfcn.%d information"
                     " are present with %d Neighboring Cells",
                     i,
                     getLocResp.lte_wcdma.freqs[i].uarfcn,
                     getLocResp.lte_wcdma.freqs[i].cells_len);

            for (j=0; j<getLocResp.lte_wcdma.freqs[i].cells_len; j++)
            {
                pa_mrc_CellInfo_t* cellInfoPtr =
                                (pa_mrc_CellInfo_t*)le_mem_ForceAlloc(CellInfoPool);

                LE_DEBUG("[LTE] UMTS cell [%d], cell [%d] psc.%d rssi.%d",
                         i, j,
                         getLocResp.lte_wcdma.freqs[i].cells[j].psc,
                         getLocResp.lte_wcdma.freqs[i].cells[j].cpich_rscp/10 );

                cellInfoPtr->index = total + i + j;
                // No Cell ID.
                cellInfoPtr->id = UINT32_MAX;
                // No LAC available.
                cellInfoPtr->lac = UINT16_MAX;
                cellInfoPtr->rxLevel = getLocResp.lte_wcdma.freqs[i].cells[j].cpich_rscp/10;
                // No Ec/Io available (Ec/No).
                cellInfoPtr->umtsEcIo = INT32_MAX;
                cellInfoPtr->rat = LE_MRC_RAT_UMTS;
                cellInfoPtr->link = LE_DLS_LINK_INIT;

                le_dls_Queue(cellInfoListPtr, &(cellInfoPtr->link));
            }
            total += j;
        }

        LE_INFO("Main cell in LTE, get %d UMTS neighboring cells", total - savTotal);
    }

    if (getLocResp.lte_intra_valid)
    {
        // Main cell in LTE, get LTE intra frequencies neighboring cells.
        LE_INFO("Main cell in LTE, get %d LTE intra frequencies neighboring cells",
                getLocResp.lte_intra.cells_len);

        for (i=0; i<getLocResp.lte_intra.cells_len; i++)
        {
            pa_mrc_CellInfo_t* cellInfoPtr =
                            (pa_mrc_CellInfo_t*)le_mem_ForceAlloc(CellInfoPool);

            LE_DEBUG("[LTE] intra Cell [%d] EARFCN.%d Pci.%d Rssi.%d",
                     i,
                     getLocResp.lte_intra.earfcn,
                     getLocResp.lte_intra.cells[i].pci,
                     getLocResp.lte_intra.cells[i].rssi/10);

            cellInfoPtr->index = total + i;
            // LTE physical cell id.
            cellInfoPtr->id = getLocResp.lte_intra.cells[i].pci;
            // No LAC available.
            cellInfoPtr->lac = UINT16_MAX;
            cellInfoPtr->rxLevel = getLocResp.lte_intra.cells[i].rssi/10;
            cellInfoPtr->lteIntraRsrp = getLocResp.lte_intra.cells[i].rsrp;
            cellInfoPtr->lteIntraRsrq = getLocResp.lte_intra.cells[i].rsrq;
            cellInfoPtr->rat = LE_MRC_RAT_LTE;
            cellInfoPtr->link = LE_DLS_LINK_INIT;

            le_dls_Queue(cellInfoListPtr, &(cellInfoPtr->link));
        }
        total += i;
    }

    if (getLocResp.lte_inter_valid)
    {
        // Main cell in LTE, get LTE inter frequencies neighboring cells.
        for (i=0, savTotal=total; i<getLocResp.lte_inter.freqs_len; i++)
        {
            for (j=0; j<getLocResp.lte_inter.freqs[i].cells_len; j++)
            {
                pa_mrc_CellInfo_t* cellInfoPtr =
                                (pa_mrc_CellInfo_t*)le_mem_ForceAlloc(CellInfoPool);

                LE_DEBUG("[LTE] freq [%d], EARFCN.%d cells [%d] pci.%d rssi.%d",
                         i,
                         getLocResp.lte_inter.freqs[i].earfcn,
                         j,
                         getLocResp.lte_inter.freqs[i].cells[j].pci,
                         getLocResp.lte_inter.freqs[i].cells[j].rssi/10);

                cellInfoPtr->index = total + i + j;
                // LTE physical cell id.
                cellInfoPtr->id = getLocResp.lte_inter.freqs[i].cells[j].pci;
                // No LAC available.
                cellInfoPtr->lac = UINT16_MAX;
                cellInfoPtr->rxLevel = getLocResp.lte_inter.freqs[i].cells[j].rssi/10;
                cellInfoPtr->lteInterRsrp = getLocResp.lte_inter.freqs[i].cells[j].rsrp;
                cellInfoPtr->lteInterRsrq = getLocResp.lte_inter.freqs[i].cells[j].rsrq;
                cellInfoPtr->rat = LE_MRC_RAT_LTE;
                cellInfoPtr->link = LE_DLS_LINK_INIT;

                le_dls_Queue(cellInfoListPtr, &(cellInfoPtr->link));
            }
            total += j;
        }
        LE_INFO("Main cell in LTE, get %d LTE inter frequencies neighboring cells",
                total - savTotal);
    }

    if (getLocResp.wcdma_lte_valid)
    {
        // Main cell in UMTS, get LTE neighboring cells.
        LE_INFO("Main cell in UMTS, get %d LTE neighboring cells",
                getLocResp.wcdma_lte.umts_lte_nbr_cell_len);

        for (i=0; i<getLocResp.wcdma_lte.umts_lte_nbr_cell_len; i++)
        {
            pa_mrc_CellInfo_t* cellInfoPtr =
                            (pa_mrc_CellInfo_t*)le_mem_ForceAlloc(CellInfoPool);
            LE_DEBUG("[UMTS] LTE Cell [%d] EARFCN.%d pci.%d rscp.%d",
                     i,
                     getLocResp.wcdma_lte.umts_lte_nbr_cell[i].earfcn,
                     getLocResp.wcdma_lte.umts_lte_nbr_cell[i].pci,
                     (int) (getLocResp.wcdma_lte.umts_lte_nbr_cell[i].rsrp) );

            cellInfoPtr->index = total + i;
            // LTE physical cell id.
            cellInfoPtr->id = getLocResp.wcdma_lte.umts_lte_nbr_cell[i].pci;
            // No LAC available.
            cellInfoPtr->lac = UINT16_MAX;
            cellInfoPtr->rxLevel = getLocResp.wcdma_lte.umts_lte_nbr_cell[i].rsrp;
            cellInfoPtr->lteInterRsrp = getLocResp.wcdma_lte.umts_lte_nbr_cell[i].rsrp*10;
            cellInfoPtr->lteInterRsrq = getLocResp.wcdma_lte.umts_lte_nbr_cell[i].rsrq*10;
            cellInfoPtr->lteIntraRsrp = getLocResp.wcdma_lte.umts_lte_nbr_cell[i].rsrp*10;
            cellInfoPtr->lteIntraRsrq = getLocResp.wcdma_lte.umts_lte_nbr_cell[i].rsrq*10;
            cellInfoPtr->rat = LE_MRC_RAT_LTE;
            cellInfoPtr->link = LE_DLS_LINK_INIT;

            le_dls_Queue(cellInfoListPtr, &(cellInfoPtr->link));
        }
        total += i;
    }

    if (getLocResp.umts_info_valid)
    {
        // Main cell in UMTS, get GERAN neighboring cells.
        LE_INFO("Main cell in UMTS, get %d GERAN neighboring cells",
                getLocResp.umts_info.umts_geran_nbr_cell_len);

        for (i=0; i<getLocResp.umts_info.umts_geran_nbr_cell_len; i++)
        {
            pa_mrc_CellInfo_t* cellInfoPtr =
                    (pa_mrc_CellInfo_t*)le_mem_ForceAlloc(CellInfoPool);

            LE_DEBUG("[UMTS] Cell [%d] UARFCN.%d umts_rscp.%d",
                     i,
                     getLocResp.umts_info.umts_geran_nbr_cell[i].geran_arfcn,
                     getLocResp.umts_info.umts_geran_nbr_cell[i].geran_rssi);

            cellInfoPtr->index = total + i;
            // No id available.
            cellInfoPtr->id = UINT32_MAX;
            // No LAC available.
            cellInfoPtr->lac = UINT16_MAX;
            cellInfoPtr->rxLevel = getLocResp.umts_info.umts_geran_nbr_cell[i].geran_rssi;
            cellInfoPtr->rat = LE_MRC_RAT_GSM;
            cellInfoPtr->link = LE_DLS_LINK_INIT;

            le_dls_Queue(cellInfoListPtr, &(cellInfoPtr->link));
        }
        total += i;

        // Main cell in UMTS, get UMTS monitored cells.
        LE_INFO("Main cell in UMTS, get %d UMTS monitored cells",
                 getLocResp.umts_info.umts_monitored_cell_len);

        for (i=0; i<getLocResp.umts_info.umts_monitored_cell_len; i++)
        {
            pa_mrc_CellInfo_t* cellInfoPtr =
                            (pa_mrc_CellInfo_t*)le_mem_ForceAlloc(CellInfoPool);

            LE_DEBUG("[UMTS] Cell monitored [%d] UARFCN.%d umts_rscp.%d",
                     i,
                     getLocResp.umts_info.umts_monitored_cell[i].umts_uarfcn,
                     getLocResp.umts_info.umts_monitored_cell[i].umts_rscp);

            cellInfoPtr->index = total + i;
            // No id available.
            cellInfoPtr->id = UINT32_MAX;
            // No LAC available.
            cellInfoPtr->lac = UINT16_MAX;
            cellInfoPtr->rxLevel = getLocResp.umts_info.umts_monitored_cell[i].umts_rscp;
            cellInfoPtr->umtsEcIo = getLocResp.umts_info.umts_monitored_cell[i].umts_ecio*10;
            cellInfoPtr->rat = LE_MRC_RAT_UMTS;
            cellInfoPtr->link = LE_DLS_LINK_INIT;

            le_dls_Queue(cellInfoListPtr, &(cellInfoPtr->link));
        }
        total += i;
    }

    return total;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to delete the list of neighboring cells information.
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_mrc_DeleteNeighborCellsInfo
(
    le_dls_List_t *cellInfoListPtr ///< [IN] list of pa_mrc_CellInfo_t
)
{
    pa_mrc_CellInfo_t* nodePtr;
    le_dls_Link_t *linkPtr;

    while ((linkPtr=le_dls_Pop(cellInfoListPtr)) != NULL)
    {
        nodePtr = CONTAINER_OF(linkPtr, pa_mrc_CellInfo_t, link);
        le_mem_Release(nodePtr);
    }
}


//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to get current registration mode
 *
 * @return
 *  - LE_FAULT  Function failed.
 *  - LE_OK     Function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_GetNetworkRegistrationMode
(
    bool*   isManualPtr,  ///< [OUT] true if the scan mode is manual, false if it is automatic.
    char*   mccPtr,       ///< [OUT] Mobile Country Code
    size_t  mccPtrSize,   ///< [IN] mccPtr buffer size
    char*   mncPtr,       ///< [OUT] Mobile Network Code
    size_t  mncPtrSize    ///< [IN] mncPtr buffer size
)
{
    le_result_t res = LE_FAULT;
    nas_get_system_selection_preference_resp_msg_v01 getSysSelecPrefResp = { {0} };

    if (isManualPtr == NULL)
    {
        LE_FATAL("isManualPtr is NULL !");
    }

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(
                    NasClient,
                    QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01,
                    NULL, 0,
                    &getSysSelecPrefResp, sizeof(getSysSelecPrefResp),
                    COMM_NETWORK_TIMEOUT);

    le_result_t respResult = swiQmi_CheckResponseCode(
                    STRINGIZE_EXPAND(QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01),
                    clientMsgErr,
                    getSysSelecPrefResp.resp);

    if (respResult != LE_OK)
    {
        return res;
    }

    switch (getSysSelecPrefResp.net_sel_pref)
    {
        case NAS_NET_SEL_PREF_MANUAL_V01:
        {
            /*  Manual Selection    */
            *isManualPtr = true;
            if (mccPtr == NULL)
            {
                LE_FATAL("mccPtr is NULL !");
            }
            if (mncPtr == NULL)
            {
                LE_FATAL("mncPtr is NULL !");
            }

            if (getSysSelecPrefResp.manual_net_sel_plmn_valid)
            {
                snprintf(mccPtr,mccPtrSize,"%03d",getSysSelecPrefResp.manual_net_sel_plmn.mcc);
                if (getSysSelecPrefResp.manual_net_sel_plmn.mnc_includes_pcs_digit != false)
                {
                    snprintf(mncPtr,mncPtrSize,"%03d",getSysSelecPrefResp.manual_net_sel_plmn.mnc);
                }
                else
                {
                    snprintf(mncPtr,mncPtrSize,"%02d",getSysSelecPrefResp.manual_net_sel_plmn.mnc);
                }
                LE_DEBUG("Plmn valid mcc.%s, mnc.%s", mccPtr, mncPtr);
            }
            else
            {
                LE_WARN("Manual PLMN not valid!");
                mccPtr[0] = 0;
                mncPtr[0] = 0;
            }
            res = LE_OK;
        }
        break;

        default:
        case NAS_NET_SEL_PREF_AUTOMATIC_V01:
        {
            /*  Automatic Selection  */
            res = pa_mrc_GetCurrentNetwork(NULL, 0, mccPtr, mccPtrSize, mncPtr, mncPtrSize);
            if(res == LE_OK)
            {
                *isManualPtr = false;
            }
            else
            {
                res = LE_FAULT;
            }
        }
        break;
    }

    return res;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to register a handler for Signal Strength change handling.
 *
 * @return A handler reference, which is only needed for later removal of the handler.
 *
 * @note Doesn't return on failure, so there's no need to check the return value for errors.
 */
//--------------------------------------------------------------------------------------------------
le_event_HandlerRef_t pa_mrc_AddSignalStrengthIndHandler
(
    pa_mrc_SignalStrengthIndHdlrFunc_t ssIndHandler, ///< [IN] The handler function to handle the
                                                     ///        Signal Strength change indication.
    void*                              contextPtr    ///< [IN] The context to be given to the handler.
)
{
    // Check if new handler is being added
    if ( ssIndHandler != NULL )
    {
        le_event_HandlerRef_t handlerRef = le_event_AddLayeredHandler(
                                                                "SignalStrengthIndHandler",
                                                                SignalStrengthIndEventId,
                                                                FirstLayerSignalStrengthIndHandler,
                                                                (le_event_HandlerFunc_t)ssIndHandler);


        le_event_SetContextPtr (handlerRef, contextPtr);

        return handlerRef;
    }
    else
    {
        return NULL;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to unregister the handler for Signal Strength change handling.
 *
 * @note Doesn't return on failure, so there's no need to check the return value for errors.
 */
//--------------------------------------------------------------------------------------------------
void pa_mrc_RemoveSignalStrengthIndHandler
(
    le_event_HandlerRef_t handlerRef
)
{
    le_event_RemoveHandler(handlerRef);
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to set and activate the signal strength thresholds for signal
 * strength indications.
 *
 * @return
 *  - LE_FAULT  Function failed.
 *  - LE_OK     Function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_SetSignalStrengthIndThresholds
(
    le_mrc_Rat_t rat,                 ///< Radio Access Technology
    int32_t      lowerRangeThreshold, ///< [IN] lower-range threshold in dBm
    int32_t      upperRangeThreshold  ///< [IN] upper-range strength threshold in dBm
)
{
    nas_config_sig_info2_req_msg_v01 nasReq = {0};
    nas_config_sig_info2_resp_msg_v01 nasResp = {{0}};
    int16_t minVal, maxVal;

    // lowerRangeThreshold and upperRangeThreshold value range.
    minVal = ((int16_t) 0x8000) / 10;
    maxVal = ((int16_t) 0x7FFF) / 10;

    if ((lowerRangeThreshold < minVal) || (lowerRangeThreshold > maxVal))
    {
        LE_ERROR("lowerRangeThreshold value out of range %d", lowerRangeThreshold);
        LE_DEBUG("minVal %d, maxVal %d", minVal, maxVal);
        return LE_FAULT;
    }

    if ((upperRangeThreshold < minVal) || (upperRangeThreshold > maxVal))
    {
        LE_ERROR("upperRangeThreshold value out of range %d", upperRangeThreshold);
        LE_DEBUG("minVal %d, maxVal %d", minVal, maxVal);
        return LE_FAULT;
    }

    switch(rat)
    {
        case LE_MRC_RAT_GSM:
            nasReq.gsm_rssi_threshold_list_valid = true;
            nasReq.gsm_rssi_threshold_list_len = 2;
            nasReq.gsm_rssi_threshold_list[0] = lowerRangeThreshold*10;
            nasReq.gsm_rssi_threshold_list[1] = upperRangeThreshold*10;
            break;

        case LE_MRC_RAT_UMTS:
            nasReq.wcdma_rssi_threshold_list_valid = true;
            nasReq.wcdma_rssi_threshold_list_len = 2;
            nasReq.wcdma_rssi_threshold_list[0] = lowerRangeThreshold*10;
            nasReq.wcdma_rssi_threshold_list[1] = upperRangeThreshold*10;
            break;

        case LE_MRC_RAT_TDSCDMA:
            nasReq.tds_rssi_threshold_list_valid = true;
            nasReq.tds_rssi_threshold_list_len = 2;
            nasReq.tds_rssi_threshold_list[0] = (float)lowerRangeThreshold;
            nasReq.tds_rssi_threshold_list[1] = (float)upperRangeThreshold;
            break;

        case LE_MRC_RAT_LTE:
            nasReq.lte_rssi_threshold_list_valid = true;
            nasReq.lte_rssi_threshold_list_len = 2;
            nasReq.lte_rssi_threshold_list[0] = lowerRangeThreshold*10;
            nasReq.lte_rssi_threshold_list[1] = upperRangeThreshold*10;
            break;

        case LE_MRC_RAT_CDMA:
            nasReq.cdma_rssi_threshold_list_valid = true;
            nasReq.cdma_rssi_threshold_list_len = 2;
            nasReq.cdma_rssi_threshold_list[0] = lowerRangeThreshold*10;
            nasReq.cdma_rssi_threshold_list[1] = upperRangeThreshold*10;
            nasReq.hdr_rssi_threshold_list_valid = true;
            nasReq.hdr_rssi_threshold_list_len = 2;
            nasReq.hdr_rssi_threshold_list[0] = lowerRangeThreshold*10;
            nasReq.hdr_rssi_threshold_list[1] = upperRangeThreshold*10;
            break;

        case LE_MRC_RAT_UNKNOWN:
        default:
            LE_ERROR("Bad parameter!");
            return LE_FAULT;
    }

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(
        NasClient,
        QMI_NAS_CONFIG_SIG_INFO2_REQ_MSG_V01,
        &nasReq, sizeof(nasReq),
        &nasResp, sizeof(nasResp),
        COMM_DEFAULT_PLATFORM_TIMEOUT);

    le_result_t result = swiQmi_CheckResponseCode(
        STRINGIZE_EXPAND(QMI_NAS_CONFIG_SIG_INFO2_REQ_MSG_V01),
        clientMsgErr, nasResp.resp);

    if (result == LE_OK)
    {
        LE_DEBUG("Signal Strength Thresholds are set.");
        return LE_OK;
    }
    else
    {
        return LE_FAULT;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to set and activate the delta for signal strength indications.
 *
 * @return
 *  - LE_FAULT  Function failed.
 *  - LE_OK     Function succeeded.
 *  - LE_BAD_PARAMETER  Bad parameters.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_SetSignalStrengthIndDelta
(
    le_mrc_Rat_t rat,    ///< [IN] Radio Access Technology
    uint16_t     delta   ///< [IN] Signal delta in units of 0.1 dBm
)
{
    SWIQMI_DECLARE_MESSAGE(nas_config_sig_info2_req_msg_v01, nasReq);
    SWIQMI_DECLARE_MESSAGE(nas_config_sig_info2_resp_msg_v01, nasResp);

    if (!delta)
    {
        LE_ERROR("Null delta provided");
        return LE_BAD_PARAMETER;
    }

    if ((LE_MRC_RAT_TDSCDMA == rat) && (delta < MIN_SIGNAL_DELTA_FOR_TDSCDMA))
    {
        LE_ERROR("Delta less than 1 dBm provided for TD-SCDMA RAT");
        return LE_BAD_PARAMETER;
    }

    switch(rat)
    {
        case LE_MRC_RAT_GSM:
            nasReq.gsm_rssi_delta_valid = true;
            nasReq.gsm_rssi_delta = delta;
            break;

        case LE_MRC_RAT_UMTS:
            nasReq.wcdma_rssi_delta_valid = true;
            nasReq.wcdma_rssi_delta = delta;
            break;

        case LE_MRC_RAT_TDSCDMA:
            {
                // round intermediate values
                int unitNumber = delta%10;
                if (unitNumber > ROUND_SIGNAL_DELTA)
                {
                    delta += 10;
                }
                delta -= unitNumber;

                nasReq.tdscdma_rssi_delta_valid = true;
                nasReq.tdscdma_rssi_delta =(float)delta/10.0;
                LE_DEBUG("TD-SCDMA rssi delta %f", nasReq.tdscdma_rssi_delta);
            }
            break;

        case LE_MRC_RAT_LTE:
            nasReq.lte_rssi_delta_valid = true;
            nasReq.lte_rssi_delta = delta;
            break;

        case LE_MRC_RAT_CDMA:
            nasReq.cdma_rssi_delta_valid = true;
            nasReq.cdma_rssi_delta = delta;
            break;

        case LE_MRC_RAT_UNKNOWN:
        default:
            LE_ERROR("Unknown RAT !");
            return LE_BAD_PARAMETER;
    }

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(
        NasClient,
        QMI_NAS_CONFIG_SIG_INFO2_REQ_MSG_V01,
        &nasReq, sizeof(nasReq),
        &nasResp, sizeof(nasResp),
        COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (LE_OK == swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_NAS_CONFIG_SIG_INFO2_REQ_MSG_V01),
                                      clientMsgErr,
                                      nasResp.resp.result,
                                      nasResp.resp.error))
    {
        LE_DEBUG("Signal strength delta is set.");
        return LE_OK;
    }

    return LE_FAULT;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to get the serving cell Identifier.
 *
 * @return
 *  - LE_FAULT  Function failed.
 *  - LE_OK     Function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_GetServingCellId
(
    uint32_t* cellIdPtr ///< [OUT] main Cell Identifier.
)
{
    nas_get_cell_location_info_resp_msg_v01 getLocResp = { {0} };

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(
                    NasClient,
                    QMI_NAS_GET_CELL_LOCATION_INFO_REQ_MSG_V01,
                    NULL, 0,
                    &getLocResp, sizeof(getLocResp),
                    COMM_NETWORK_TIMEOUT);

    le_result_t respResult = swiQmi_CheckResponseCode(
                    STRINGIZE_EXPAND(QMI_NAS_GET_CELL_LOCATION_INFO_REQ_MSG_V01),
                    clientMsgErr,
                    getLocResp.resp);

    if (respResult == LE_OK)
    {
        if (getLocResp.geran_info_valid)
        {
            // 2G
            *cellIdPtr = getLocResp.geran_info.cell_id;
            return LE_OK;
        }
        else if (getLocResp.umts_cell_id_valid)
        {
            // 3G
            *cellIdPtr = getLocResp.umts_cell_id;
            return LE_OK;
        }
        else if (getLocResp.lte_intra_valid)
        {
            // LTE
            *cellIdPtr = getLocResp.lte_intra.global_cell_id;
            return LE_OK;
        }
        else
        {
            LE_ERROR("Main cell information is void!");
            return LE_FAULT;
        }
    }
    else
    {
        LE_ERROR("Cannot retrieve cell information!");
        return LE_FAULT;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to get the Tracking Area Code of the serving cell.
 *
 * @return
 *  - LE_FAULT  Function failed.
 *  - LE_OK     Function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_GetServingCellLteTracAreaCode
(
    uint16_t* tacPtr ///< [OUT] Tracking Area Code of the serving cell.
)
{
    nas_get_cell_location_info_resp_msg_v01 getLocResp = { {0} };

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(
                    NasClient,
                    QMI_NAS_GET_CELL_LOCATION_INFO_REQ_MSG_V01,
                    NULL, 0,
                    &getLocResp, sizeof(getLocResp),
                    COMM_NETWORK_TIMEOUT);

    le_result_t respResult = swiQmi_CheckResponseCode(
                    STRINGIZE_EXPAND(QMI_NAS_GET_CELL_LOCATION_INFO_REQ_MSG_V01),
                    clientMsgErr,
                    getLocResp.resp);

    if (respResult == LE_OK)
    {
        if (getLocResp.lte_intra_valid)
        {
            *tacPtr = getLocResp.lte_intra.tac;
            return LE_OK;
        }
        else
        {
            LE_ERROR("Main cell information is void!");
            return LE_FAULT;
        }
    }
    else
    {
        LE_ERROR("Cannot retrieve cell information!");
        return LE_FAULT;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to get the Location Area Code of the serving cell.
 *
 * @return
 *  - LE_FAULT  Function failed.
 *  - LE_OK     Function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_GetServingCellLocAreaCode
(
    uint32_t* lacPtr ///< [OUT] Location Area Code of the serving cell.
)
{
    nas_get_cell_location_info_resp_msg_v01 getLocResp = { {0} };

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(
                    NasClient,
                    QMI_NAS_GET_CELL_LOCATION_INFO_REQ_MSG_V01,
                    NULL, 0,
                    &getLocResp, sizeof(getLocResp),
                    COMM_NETWORK_TIMEOUT);

    le_result_t respResult = swiQmi_CheckResponseCode(
                    STRINGIZE_EXPAND(QMI_NAS_GET_CELL_LOCATION_INFO_REQ_MSG_V01),
                    clientMsgErr,
                    getLocResp.resp);

    if (respResult == LE_OK)
    {
        if (getLocResp.geran_info_valid)
        {
            *lacPtr = getLocResp.geran_info.lac;
            return LE_OK;
        }
        else if (getLocResp.umts_info_valid)
        {
            *lacPtr = getLocResp.umts_info.lac;
            return LE_OK;
        }
        else
        {
            LE_ERROR("Main cell information is void!");
            return LE_FAULT;
        }
    }
    else
    {
        LE_ERROR("Cannot retrieve cell information!");
        return LE_FAULT;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the 2G/3G Band capabilities
 *
 * @return
 *  - LE_OK              on success
 *  - LE_FAULT           on failure
 *  - LE_UNSUPPORTED     The platform does not support this operation.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_GetBandCapabilities
(
    le_mrc_BandBitMask_t* bandsPtr ///< [OUT] A bit mask to get the Band capabilities.
)
{
#if defined(QMI_DMS_SWI_GET_SUPPORT_BANDS_REQ_V01)
    SWIQMI_DECLARE_MESSAGE(dms_swi_get_support_bands_resp_msg_v01, getResp);
    uint64_t bitMask = 0;
    le_mrc_BandBitMask_t bands = 0;

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(
                    DmsClient,
                    QMI_DMS_SWI_GET_SUPPORT_BANDS_REQ_V01,
                    NULL, 0,
                    &getResp, sizeof(getResp),
                    COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (LE_OK != swiQmi_CheckResponse(
                    STRINGIZE_EXPAND(QMI_DMS_SWI_GET_SUPPORT_BANDS_REQ_V01),
                    clientMsgErr,
                    getResp.resp.result,
                    getResp.resp.error))
    {
        LE_ERROR("Failed to get the band capabilities");
        return LE_FAULT;
    }

    // 2G/3G Band Capabilities
    if (bandsPtr)
    {
        if (!getResp.gw_mask_valid)
        {
            LE_ERROR("Unable to get the 2G/3G Band capabilities on this platform");
            return LE_UNSUPPORTED;
        }

        bitMask = getResp.gw_mask;
        LE_DEBUG("Get 2G/3G Band Capabilities QMI bit mask: 0x%016"PRIX64, bitMask);

        if (bitMask & QMI_NAS_BAND_CLASS_0_A_SYSTEM_V01)
        {
            bands |= LE_MRC_BITMASK_BAND_CLASS_0_A_SYSTEM;
        }
        if (bitMask & QMI_NAS_BAND_CLASS_0_B_AB_GSM850_V01)
        {
            bands |= LE_MRC_BITMASK_BAND_CLASS_0_B_SYSTEM;
        }
        if (bitMask & QMI_NAS_BAND_CLASS_1_ALL_BLOCKS_V01)
        {
            bands |= LE_MRC_BITMASK_BAND_CLASS_1_ALL_BLOCKS;
        }
        if (bitMask & QMI_NAS_BAND_CLASS_2_PLACEHOLDER_V01)
        {
            bands |= LE_MRC_BITMASK_BAND_CLASS_2_PLACEHOLDER;
        }
        if (bitMask & QMI_NAS_BAND_CLASS_3_A_SYSTEM_V01)
        {
            bands |= LE_MRC_BITMASK_BAND_CLASS_3_A_SYSTEM;
        }
        if (bitMask & QMI_NAS_BAND_CLASS_4_ALL_BLOCKS_V01)
        {
            bands |= LE_MRC_BITMASK_BAND_CLASS_4_ALL_BLOCKS;
        }
        if (bitMask & QMI_NAS_BAND_CLASS_5_ALL_BLOCKS_V01)
        {
            bands |= LE_MRC_BITMASK_BAND_CLASS_5_ALL_BLOCKS;
        }
        if (bitMask & QMI_NAS_GSM_DCS_1800_BAND_V01)
        {
            bands |= LE_MRC_BITMASK_BAND_GSM_DCS_1800;
        }
        if (bitMask & QMI_NAS_BAND_CLASS_5_ALL_BLOCKS_V01)
        {
            bands |= LE_MRC_BITMASK_BAND_CLASS_5_ALL_BLOCKS;
        }
        if (bitMask & QMI_NAS_GSM_DCS_1800_BAND_V01)
        {
            bands |= LE_MRC_BITMASK_BAND_GSM_DCS_1800;
        }
        if (bitMask & QMI_NAS_E_GSM_900_BAND_V01)
        {
            bands |= LE_MRC_BITMASK_BAND_EGSM_900;
        }
        if (bitMask & QMI_NAS_P_GSM_900_BAND_V01)
        {
            bands |= LE_MRC_BITMASK_BAND_PRI_GSM_900;
        }
        if (bitMask & QMI_NAS_E_GSM_900_BAND_V01)
        {
            bands |= LE_MRC_BITMASK_BAND_EGSM_900;
        }
        if (bitMask & QMI_NAS_P_GSM_900_BAND_V01)
        {
            bands |= LE_MRC_BITMASK_BAND_PRI_GSM_900;
        }
        if (bitMask & QMI_NAS_BAND_CLASS_6_V01)
        {
            bands |= LE_MRC_BITMASK_BAND_CLASS_6;
        }
        if (bitMask & QMI_NAS_BAND_CLASS_7_V01)
        {
            bands |= LE_MRC_BITMASK_BAND_CLASS_7;
        }
        if (bitMask & QMI_NAS_BAND_CLASS_8_V01)
        {
            bands |= LE_MRC_BITMASK_BAND_CLASS_8;
        }
        if (bitMask & QMI_NAS_BAND_CLASS_9_V01)
        {
            bands |= LE_MRC_BITMASK_BAND_CLASS_9;
        }
        if (bitMask & QMI_NAS_BAND_CLASS_10_V01)
        {
            bands |= LE_MRC_BITMASK_BAND_CLASS_10;
        }
        if (bitMask & QMI_NAS_BAND_CLASS_11_V01)
        {
            bands |= LE_MRC_BITMASK_BAND_CLASS_11;
        }
        if (bitMask & QMI_NAS_GSM_BAND_450_V01)
        {
            bands |= LE_MRC_BITMASK_BAND_GSM_450;
        }
        if (bitMask & QMI_NAS_GSM_BAND_480_V01)
        {
            bands |= LE_MRC_BITMASK_BAND_GSM_480;
        }
        if (bitMask & QMI_NAS_GSM_BAND_750_V01)
        {
            bands |= LE_MRC_BITMASK_BAND_GSM_750;
        }
        if (bitMask & QMI_NAS_GSM_BAND_850_V01)
        {
            bands |= LE_MRC_BITMASK_BAND_GSM_850;
        }
        if (bitMask & QMI_NAS_GSM_BAND_RAILWAYS_900_BAND_V01)
        {
            bands |= LE_MRC_BITMASK_BAND_GSMR_900;
        }
        if (bitMask & QMI_NAS_GSM_BAND_PCS_1900_BAND_V01)
        {
            bands |= LE_MRC_BITMASK_BAND_GSM_PCS_1900;
        }
        if (bitMask & QMI_NAS_WCDMA_EU_J_CH_IMT_2100_BAND_V01)
        {
            bands |= LE_MRC_BITMASK_BAND_WCDMA_EU_J_CH_IMT_2100;
        }
        if (bitMask & QMI_NAS_WCDMA_US_PCS_1900_BAND_V01)
        {
            bands |= LE_MRC_BITMASK_BAND_WCDMA_US_PCS_1900;
        }
        if (bitMask & QMI_NAS_EU_CH_DCS_1800_BAND_V01)
        {
            bands |= LE_MRC_BITMASK_BAND_WCDMA_EU_CH_DCS_1800;
        }
        if (bitMask & QMI_NAS_WCDMA_US_1700_BAND_V01)
        {
            bands |= LE_MRC_BITMASK_BAND_WCDMA_US_1700;
        }
        if (bitMask & QMI_NAS_WCDMA_US_850_BAND_V01)
        {
            bands |= LE_MRC_BITMASK_BAND_WCDMA_US_850;
        }
        if (bitMask & QMI_NAS_WCDMA_JAPAN_800_BAND_V01)
        {
            bands |= LE_MRC_BITMASK_BAND_WCDMA_J_800;
        }
        if (bitMask & QMI_NAS_BAND_CLASS_12_V01)
        {
            bands |= LE_MRC_BITMASK_BAND_CLASS_12;
        }
        if (bitMask & QMI_NAS_BAND_CLASS_14_V01)
        {
            bands |= LE_MRC_BITMASK_BAND_CLASS_14;
        }
        if (bitMask & QMI_NAS_BAND_CLASS_15_V01)
        {
            bands |= LE_MRC_BITMASK_BAND_CLASS_15;
        }
        if (bitMask & QMI_NAS_WCDMA_EU_2600_BAND_V01)
        {
            bands |= LE_MRC_BITMASK_BAND_WCDMA_EU_2600;
        }
        if (bitMask & QMI_NAS_WCDMA_EU_J_900_BAND_V01)
        {
            bands |= LE_MRC_BITMASK_BAND_WCDMA_EU_J_900;
        }
        if (bitMask & QMI_NAS_WCDMA_J_1700_BAND_V01)
        {
            bands |= LE_MRC_BITMASK_BAND_WCDMA_J_1700;
        }
        if (bitMask & QMI_NAS_BAND_CLASS_16_V01)
        {
            bands |= LE_MRC_BITMASK_BAND_CLASS_16;
        }
        if (bitMask & QMI_NAS_BAND_CLASS_17_V01)
        {
            bands |= LE_MRC_BITMASK_BAND_CLASS_17;
        }
        if (bitMask & QMI_NAS_BAND_CLASS_18_V01)
        {
            bands |= LE_MRC_BITMASK_BAND_CLASS_18;
        }
        if (bitMask & QMI_NAS_BAND_CLASS_19_V01)
        {
            bands |= LE_MRC_BITMASK_BAND_CLASS_19;
        }

        if (0 == bands)
        {
            LE_ERROR("Unable to get the 2G/3G Band capabilities on this platform");
            return LE_UNSUPPORTED;
        }
        *bandsPtr = bands;
        LE_DEBUG("Get 2G/3G Band Capabilities bit mask: 0x%016"PRIX64, (uint64_t)bands);
        return LE_OK;
    }

    return LE_FAULT;
#else
    LE_ERROR("Unable to get Band Capabilities on this platform");
    return LE_UNSUPPORTED;
#endif
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the LTE Band capabilities
 *
 * @return
 *  - LE_OK              on success
 *  - LE_FAULT           on failure
 *  - LE_UNSUPPORTED     The platform does not support this operation.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_GetLteBandCapabilities
(
    le_mrc_LteBandBitMask_t* bandsPtr ///< [OUT] Bit mask to get the LTE Band capabilities.
)
{
#if defined(QMI_DMS_SWI_GET_SUPPORT_BANDS_REQ_V01)
    SWIQMI_DECLARE_MESSAGE(dms_swi_get_support_bands_resp_msg_v01, getResp);
    uint64_t                                bitMask = 0;
    le_mrc_LteBandBitMask_t                 lteBands = 0;

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(
                    DmsClient,
                    QMI_DMS_SWI_GET_SUPPORT_BANDS_REQ_V01,
                    NULL, 0,
                    &getResp, sizeof(getResp),
                    COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (LE_OK != swiQmi_CheckResponse(
                    STRINGIZE_EXPAND(QMI_DMS_SWI_GET_SUPPORT_BANDS_REQ_V01),
                    clientMsgErr,
                    getResp.resp.result,
                    getResp.resp.error))
    {
        LE_ERROR("Failed to get the band capabilities");
        return LE_FAULT;
    }

    if (bandsPtr)
    {
        uint32_t i;
        if (!getResp.l_mask_valid)
        {
            LE_ERROR("Unable to get the LTE Band capabilities on this platform");
            return LE_UNSUPPORTED;
        }

        for(i=0; i<QMI_DMS_L_MASK_SIZE_V01; i++)
        {
            bitMask = getResp.l_mask[i];
            LE_DEBUG("Get LTE Band Capabilities QMI bit mask.%d: 0x%016"PRIX64, i, bitMask);

            if (bitMask & E_UTRA_OPERATING_BAND_1_V01)
            {
                lteBands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_1;
            }
            if (bitMask & E_UTRA_OPERATING_BAND_2_V01)
            {
                lteBands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_2;
            }
            if (bitMask & E_UTRA_OPERATING_BAND_3_V01)
            {
                lteBands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_3;
            }
            if (bitMask & E_UTRA_OPERATING_BAND_4_V01)
            {
                lteBands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_4;
            }
            if (bitMask & E_UTRA_OPERATING_BAND_5_V01)
            {
                lteBands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_5;
            }
            if (bitMask & E_UTRA_OPERATING_BAND_6_V01)
            {
                lteBands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_6;
            }
            if (bitMask & E_UTRA_OPERATING_BAND_7_V01)
            {
                lteBands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_7;
            }
            if (bitMask & E_UTRA_OPERATING_BAND_8_V01)
            {
                lteBands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_8;
            }
            if (bitMask & E_UTRA_OPERATING_BAND_9_V01)
            {
                lteBands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_9;
            }
            if (bitMask & E_UTRA_OPERATING_BAND_10_V01)
            {
                lteBands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_10;
            }
            if (bitMask & E_UTRA_OPERATING_BAND_11_V01)
            {
                lteBands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_11;
            }
            if (bitMask & E_UTRA_OPERATING_BAND_12_V01)
            {
                lteBands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_12;
            }
            if (bitMask & E_UTRA_OPERATING_BAND_13_V01)
            {
                lteBands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_13;
            }
            if (bitMask & E_UTRA_OPERATING_BAND_14_V01)
            {
                lteBands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_14;
            }
            if (bitMask & E_UTRA_OPERATING_BAND_17_V01)
            {
                lteBands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_17;
            }
            if (bitMask & E_UTRA_OPERATING_BAND_18_V01)
            {
                lteBands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_18;
            }
            if (bitMask & E_UTRA_OPERATING_BAND_19_V01)
            {
                lteBands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_19;
            }
            if (bitMask & E_UTRA_OPERATING_BAND_20_V01)
            {
                lteBands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_20;
            }
            if (bitMask & E_UTRA_OPERATING_BAND_21_V01)
            {
                lteBands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_21;
            }
            if (bitMask & E_UTRA_OPERATING_BAND_24_V01)
            {
                 lteBands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_24;
            }
            if (bitMask & E_UTRA_OPERATING_BAND_25_V01)
            {
                lteBands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_25;
            }
            if (bitMask & E_UTRA_OPERATING_BAND_33_V01)
            {
                lteBands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_33;
            }
            if (bitMask & E_UTRA_OPERATING_BAND_34_V01)
            {
                lteBands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_34;
            }
            if (bitMask & E_UTRA_OPERATING_BAND_35_V01)
            {
                lteBands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_35;
            }
            if (bitMask & E_UTRA_OPERATING_BAND_36_V01)
            {
                lteBands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_36;
            }
            if (bitMask & E_UTRA_OPERATING_BAND_37_V01)
            {
                lteBands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_37;
            }
            if (bitMask & E_UTRA_OPERATING_BAND_38_V01)
            {
                lteBands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_38;
            }
            if (bitMask & E_UTRA_OPERATING_BAND_39_V01)
            {
                lteBands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_39;
            }
            if (bitMask & E_UTRA_OPERATING_BAND_40_V01)
            {
                lteBands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_40;
            }
            if (bitMask & E_UTRA_OPERATING_BAND_41_V01)
            {
                lteBands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_41;
            }
            if (bitMask & E_UTRA_OPERATING_BAND_42_V01)
            {
                lteBands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_42;
            }
            if (bitMask & E_UTRA_OPERATING_BAND_43_V01)
            {
                lteBands |= LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_43;
            }
        }

        if (!lteBands)
        {
            LE_ERROR("Unable to get the LTE Band capabilities on this platform");
            return LE_UNSUPPORTED;
        }

        *bandsPtr = lteBands;
        LE_DEBUG("Get LTE Band Capabilities bit mask: 0x%016"PRIX64, (uint64_t)lteBands);
        return LE_OK;
    }

    return LE_FAULT;
#else
    LE_ERROR("Unable to get the LTE Band Capabilities on this platform");
    return LE_UNSUPPORTED;
#endif
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the TD-SCDMA Band capabilities
 *
 * @return
 *  - LE_OK              on success
 *  - LE_FAULT           on failure
 *  - LE_UNSUPPORTED     The platform does not support this operation.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_GetTdScdmaBandCapabilities
(
    le_mrc_TdScdmaBandBitMask_t* bandsPtr ///< [OUT] Bit mask to get the TD-SCDMA Band capabilities.
)
{
#if defined(QMI_DMS_SWI_GET_SUPPORT_BANDS_REQ_V01)
    SWIQMI_DECLARE_MESSAGE(dms_swi_get_support_bands_resp_msg_v01, getResp);
    uint64_t bitMask = 0;
    le_mrc_TdScdmaBandBitMask_t tdScdmaBands = 0;

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(
                    DmsClient,
                    QMI_DMS_SWI_GET_SUPPORT_BANDS_REQ_V01,
                    NULL, 0,
                    &getResp, sizeof(getResp),
                    COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (LE_OK != swiQmi_CheckResponse(
                    STRINGIZE_EXPAND(QMI_DMS_SWI_GET_SUPPORT_BANDS_REQ_V01),
                    clientMsgErr,
                    getResp.resp.result,
                    getResp.resp.error))
    {
        LE_ERROR("Failed to get the band capabilities");
        return LE_FAULT;
    }

    // TD-SCDMA Band Capabilities
    if (bandsPtr)
    {
        if (!getResp.tds_mask_valid)
        {
            LE_ERROR("Unable to get the TD-SCDMA Band capabilities on this platform");
            return LE_UNSUPPORTED;
        }

        bitMask = getResp.tds_mask;
        LE_DEBUG("Get TD-SCDMA Band Capabilities QMI bit mask: 0x%016"PRIX64, bitMask);

        if (bitMask & NAS_TDSCDMA_BAND_A_V01)
        {
            tdScdmaBands |= LE_MRC_BITMASK_TDSCDMA_BAND_A;
        }
        if (bitMask & NAS_TDSCDMA_BAND_B_V01)
        {
            tdScdmaBands |= LE_MRC_BITMASK_TDSCDMA_BAND_B;
        }
        if (bitMask & NAS_TDSCDMA_BAND_C_V01)
        {
            tdScdmaBands |= LE_MRC_BITMASK_TDSCDMA_BAND_C;
        }
        if (bitMask & NAS_TDSCDMA_BAND_D_V01)
        {
            tdScdmaBands |= LE_MRC_BITMASK_TDSCDMA_BAND_D;
        }
        if (bitMask & NAS_TDSCDMA_BAND_E_V01)
        {
            tdScdmaBands |= LE_MRC_BITMASK_TDSCDMA_BAND_E;
        }
        if (bitMask & NAS_TDSCDMA_BAND_F_V01)
        {
            tdScdmaBands |= LE_MRC_BITMASK_TDSCDMA_BAND_F;
        }

        if (!tdScdmaBands)
        {
            LE_ERROR("Unable to get the TD-SCDMA Band capabilities on this platform");
            return LE_UNSUPPORTED;
        }

        *bandsPtr = tdScdmaBands;
        LE_DEBUG("Get TD-SCDMA Band Capabilities bit mask: 0x%016"PRIX64, (uint64_t)tdScdmaBands);
        return LE_OK;
    }
    return LE_FAULT;
#else
    LE_ERROR("Unable to get Band Capabilities on this platform");
    return LE_UNSUPPORTED;
#endif
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the Packet Switched state.
 *
 * @return
 *  - LE_FAULT  Function failed.
 *  - LE_OK     Function succeeded.
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_GetPacketSwitchedState
(
    le_mrc_NetRegState_t* statePtr  ///< [OUT] The current Packet switched state.
)
{
    if (statePtr)
    {
        *statePtr = LastPSState;
        return LE_OK;
    }
    return LE_FAULT;
}

//--------------------------------------------------------------------------------------------------
/**
 * The first-layer network reject indication handler.
 */
//--------------------------------------------------------------------------------------------------
static void FirstLayerNetworkRejectIndHandler
(
    void*   reportPtr,
    void*   secondLayerFunc
)
{
    if (NULL == reportPtr)
    {
        LE_ERROR("reportPtr is NULL");
        return;
    }

    if (NULL == secondLayerFunc)
    {
        LE_ERROR("secondLayerFunc is NULL");
        return;
    }

    pa_mrc_NetworkRejectIndication_t* networkRejectIndPtr =
                                      (pa_mrc_NetworkRejectIndication_t*) reportPtr;

    pa_mrc_NetworkRejectIndHdlrFunc_t handlerFunc =
                                      (pa_mrc_NetworkRejectIndHdlrFunc_t) secondLayerFunc;

    handlerFunc(networkRejectIndPtr);
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to register a handler to report network reject code.
 *
 * @return A handler reference, which is only needed for later removal of the handler.
 *
 * @note Doesn't return on failure, so there's no need to check the return value for errors.
 */
//--------------------------------------------------------------------------------------------------
le_event_HandlerRef_t pa_mrc_AddNetworkRejectIndHandler
(
    pa_mrc_NetworkRejectIndHdlrFunc_t networkRejectIndHandler, ///< [IN] The handler function to
                                                               ///< report network reject
                                                               ///< indication.
    void*                             contextPtr               ///< [IN] The context to be given to
                                                               ///< the handler.
)
{
    // Check if new handler is being added
    if (NULL != networkRejectIndHandler)
    {
        le_event_HandlerRef_t handlerRef = le_event_AddLayeredHandler(
                                           "NetworkRejectIndHandler",
                                           NetworkRejectIndEventId,
                                           FirstLayerNetworkRejectIndHandler,
                                           (le_event_HandlerFunc_t)networkRejectIndHandler);

        le_event_SetContextPtr(handlerRef, contextPtr);

        return handlerRef;
    }
    else
    {
        return NULL;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to unregister the handler for network reject indication handling.
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_mrc_RemoveNetworkRejectIndHandler
(
    le_event_HandlerRef_t handlerRef
)
{
    le_event_RemoveHandler(handlerRef);
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to register a handler to report jamming detection notification.
 *
 * @return A handler reference, which is only needed for later removal of the handler.
 *
 * @note Doesn't return on failure, so there's no need to check the return value for errors.
 */
//--------------------------------------------------------------------------------------------------
le_event_HandlerRef_t pa_mrc_AddJammingDetectionIndHandler
(
    pa_mrc_JammingDetectionHandlerFunc_t jammingDetectionIndHandler, ///< [IN] The handler function
                                                                     ///  to handle jamming
                                                                     ///  detection indication.
    void*                               contextPtr                   ///< [IN] The context to be
                                                                     ///  given to the handler.
)
{
#if defined(QMI_SWI_M2M_JAMMING_IND_V01)
    // Check if new handler is being added
    if (NULL != jammingDetectionIndHandler)
    {
        le_event_HandlerRef_t handlerRef = le_event_AddLayeredHandler(
                                           "JammingDetectionIndHandler",
                                           JammingDetectionIndEventId,
                                           FirstLayerJammingDetectionIndHandler,
                                           (le_event_HandlerFunc_t)jammingDetectionIndHandler);

        le_event_SetContextPtr(handlerRef, contextPtr);

        return handlerRef;
    }
    else
    {
        return NULL;
    }
#else
    LE_ERROR("Jamming detection is not supported on this platform");
    return NULL;
#endif
}

#if defined(QMI_SWI_M2M_JAMMING_SET_EVENT_REPORT_REQ_V01)
//--------------------------------------------------------------------------------------------------
/**
 * This function activate or disable jamming detection notification.
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on failure
 *      - LE_DUPLICATE if jamming detection is already activated and an activation is requested
 *      - LE_UNSUPPORTED if jamming detection is not supported
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_SetJammingDetection
(
    bool activation     ///< [IN] Notification activation request
)
{
    qmi_client_error_type rc;
    SWIQMI_DECLARE_MESSAGE(swi_m2m_jamming_set_event_report_req_msg_v01, jammingActivationReq);
    SWIQMI_DECLARE_MESSAGE(swi_m2m_jamming_set_event_report_resp_msg_v01, jammingActivationResp);
    jammingActivationReq.enable = 0;

    if (activation)
    {
        if (JammingDetectionStarted)
        {
            return LE_DUPLICATE;
        }
        jammingActivationReq.enable = 1;
    }

    // Send request to fw.
    rc = qmi_client_send_msg_sync(MgsClient,
                                  QMI_SWI_M2M_JAMMING_SET_EVENT_REPORT_REQ_V01,
                                  &jammingActivationReq, sizeof(jammingActivationReq),
                                  &jammingActivationResp, sizeof(jammingActivationResp),
                                  COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (LE_OK != swiQmi_OEMCheckResponseCode(
                                STRINGIZE_EXPAND(QMI_SWI_M2M_JAMMING_SET_EVENT_REPORT_REQ_V01),
                                rc,
                                jammingActivationResp.resp))
    {
        LE_ERROR("Jamming detection report activation failure");
        return LE_FAULT;
    }

    if (activation)
    {
        JammingDetectionStarted = true;
    }
    else
    {
        JammingDetectionStarted = false;
    }

    LE_DEBUG("Jamming detection activation %d request: %d", activation, rc);
    return LE_OK;
}
#endif

#if defined(QMI_SWI_M2M_JAMMING_GET_EVENT_REPORT_REQ_V01)
//--------------------------------------------------------------------------------------------------
/**
 * This function returns the jamming detection notification status.
 *
 * * @return
 *      - LE_OK on success
 *      - LE_BAD_PARAMETER if the parameter is invalid
 *      - LE_FAULT on failure
 *      - LE_UNSUPPORTED if jamming detection is not supported or if this request is not supported
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_GetJammingDetection
(
    bool* activationPtr     ///< [IN] Notification activation request
)
{
    qmi_client_error_type rc;
    SWIQMI_DECLARE_MESSAGE(swi_m2m_jamming_get_event_report_req_msg_v01, jammingActivationReq);
    SWIQMI_DECLARE_MESSAGE(swi_m2m_jamming_get_event_report_resp_msg_v01, jammingActivationResp);

    if (!activationPtr)
    {
        return LE_BAD_PARAMETER;
    }

    // Send request to fw.
    rc = qmi_client_send_msg_sync(MgsClient,
                                  QMI_SWI_M2M_JAMMING_GET_EVENT_REPORT_REQ_V01,
                                  &jammingActivationReq, sizeof(jammingActivationReq),
                                  &jammingActivationResp, sizeof(jammingActivationResp),
                                  COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (LE_OK != swiQmi_OEMCheckResponseCode(
                                STRINGIZE_EXPAND(QMI_SWI_M2M_JAMMING_GET_EVENT_REPORT_REQ_V01),
                                rc,
                                jammingActivationResp.resp))
    {
        LE_ERROR("Jamming detection report activation failure");
        return LE_FAULT;
    }

    *activationPtr = jammingActivationResp.event_report_status;

    LE_DEBUG("Jamming detection activation %d request: %d", *activationPtr, rc);
    return LE_OK;
}
#endif

//--------------------------------------------------------------------------------------------------
/**
 * Set the SAR backoff state
 *
 * @return
 *  - LE_OK             The function succeeded.
 *  - LE_FAULT          The function failed.
 *  - LE_UNSUPPORTED    The feature is not supported.
 *  - LE_OUT_OF_RANGE   The provided index is out of range.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_SetSarBackoffState
(
    uint8_t state
)
{
    qmi_client_error_type rc;
    SWIQMI_DECLARE_MESSAGE(sar_rf_set_state_req_msg_v01, backoffStateReq);
    SWIQMI_DECLARE_MESSAGE(sar_rf_set_state_resp_msg_v01, backoffStateResp);

    backoffStateReq.sar_rf_state = state;

    // Send request to fw.
    rc = qmi_client_send_msg_sync(SwiSarClient,
                                  QMI_SAR_RF_SET_STATE_REQ_MSG_V01,
                                  &backoffStateReq, sizeof(backoffStateReq),
                                  &backoffStateResp, sizeof(backoffStateResp),
                                  COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (LE_OK != swiQmi_CheckResponseCode(
                                STRINGIZE_EXPAND(QMI_SAR_RF_SET_STATE_RESP_MSG_V01),
                                rc,
                                backoffStateResp.resp))
    {
        LE_ERROR("Failed to set SAR backoff state %d. Error: %d", state, rc);

        if ((QMI_RESULT_FAILURE_V01 == backoffStateResp.resp.result) &&
            (QMI_ERR_INVALID_ARG_V01 == backoffStateResp.resp.error))
        {
            return LE_OUT_OF_RANGE;
        }

        return LE_FAULT;
    }

    LE_DEBUG("SAR backoff state %d request success: %d", state, rc);
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the SAR backoff state
 *
 * @return
 *  - LE_OK             The function succeeded.
 *  - LE_FAULT          The function failed.
 *  - LE_UNSUPPORTED    The feature is not supported.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_GetSarBackoffState
(
    uint8_t* statePtr
)
{
    qmi_client_error_type rc;
    SWIQMI_DECLARE_MESSAGE(sar_rf_get_state_resp_msg_v01, backoffStateResp);

    if (!statePtr)
    {
        LE_ERROR("Null pointer provided");
        return LE_FAULT;
    }

    // Send request to fw.
    rc = qmi_client_send_msg_sync(SwiSarClient,
                                  QMI_SAR_RF_GET_STATE_REQ_MSG_V01,
                                  NULL, 0,
                                  &backoffStateResp, sizeof(backoffStateResp),
                                  COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (LE_OK != swiQmi_CheckResponseCode(
                                STRINGIZE_EXPAND(QMI_SAR_RF_GET_STATE_RESP_MSG_V01),
                                rc,
                                backoffStateResp.resp))
    {
        LE_ERROR("Failed to get SAR backoff");
        return LE_FAULT;
    }

    if (!backoffStateResp.sar_rf_state_valid)
    {
        LE_ERROR("SAR backoff state not passed in QMI message response");
        return LE_FAULT;
    }
    *statePtr = backoffStateResp.sar_rf_state;

    LE_DEBUG("SAR backoff state: %d", *statePtr);
    return LE_OK;
}
