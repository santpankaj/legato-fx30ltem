/**
 * @file pa_mdc_qmi.c
 *
 * QMI implementation of @ref c_pa_mdc API.
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 * @todo:
 *      - add more error/state checking
 */

#include <arpa/inet.h>

#include "legato.h"
#include "interfaces.h"
#include "pa_mdc.h"
#include "pa_mrc.h"
#include "pa_info.h"
#include "swiQmi.h"
#include "mdc_qmi.h"
#include "watchdogChain.h"
#include "le_ms_local.h"
#include "thread.h"

#if defined(QMI_WDS_SWI_SET_MTPDP_RING_REPORT_IND_V01)
#define SIERRA_MTPDP
#endif

#if defined(SIERRA_MDM9X40) || defined(SIERRA_MDM9X28)
#define SIERRA_BINDMUX
#endif

// Include macros for printing out values
#include "le_print.h"
#include <time.h>
#include <semaphore.h>


//--------------------------------------------------------------------------------------------------
/**
 * The maximal number of WDS (Wireless Data Service) clients.
 */
//--------------------------------------------------------------------------------------------------
#if defined (PDP_MAX_MULTIPDP)
#define MAX_WDS_CLIENTS PDP_MAX_MULTIPDP
#else
#define MAX_WDS_CLIENTS 4
#endif

//--------------------------------------------------------------------------------------------------
/**
 * Structure used to save the handle identifying the call instance providing packet service (coming
 * from QMI). IPv4, IPv6, or IPv4v6 handle can be saved in this structure
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    bool               enable;         // indicate if callRef parameter exists
    uint32_t           callRef;        // QMI handle
    le_mdc_ConState_t  state;          // data connection state
}
CallIpContext_t;

//--------------------------------------------------------------------------------------------------
/**
 * Structure used to save the call context
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    CallIpContext_t ipv4, ipv6;
    pa_mdc_ConnectionFailureCode_t failureCode;
}
CallContext_t;

//--------------------------------------------------------------------------------------------------
/**
 * Structure used to save the WDS client information
 *
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    swiQmi_ClientInfo_t  wdsClientInfo;
    uint32_t             rmnetNumber;
    uint32_t             profileIndex;
}
WdsClientInfo_t;

//--------------------------------------------------------------------------------------------------
/**
 * Structure used to save profile context
 *
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    WdsClientInfo_t         *clientInfoPtr[LE_MDMDEFS_IPMAX];
    uint32_t                profileIndex;
    CallContext_t           callContext;
    uint8_t                 mappedRmnetId;
    le_dls_Link_t           link;
    le_mdc_Pdp_t            pdpType;
}
ProfileCtx_t;

//--------------------------------------------------------------------------------------------------
/**
 * Structure for data statistics
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    bool                   saving;      ///< Is saving activated?
    pa_mdc_PktStatistics_t saved;       ///< Latest saved data statistics
    pa_mdc_PktStatistics_t reset;       ///< Saved data statistics when reset required
}
DataStatistics_t;

//--------------------------------------------------------------------------------------------------
/**
 * Semaphore used to synchronize StartSession and StopSession functions.
 */
//--------------------------------------------------------------------------------------------------
# define SESSION_TIMEOUT_MS 6000
static bool IsStartSession=false;
static bool IsStopSession=false;
static le_sem_Ref_t SessionSemRef;

//--------------------------------------------------------------------------------------------------
/**
 * Save the PA MDC main thread reference because QMINewSessionStateHandler is not a Legato thread.
 * Legato Semaphore API need to be called from a Legato thread and the swiQmi_ReleaseService() API
 * need to be called of another thread than QMINewSessionStateHandler.
 * (due to QMI).
 */
//--------------------------------------------------------------------------------------------------
static le_thread_Ref_t CurrentThreadRef;

//--------------------------------------------------------------------------------------------------
/**
 * The WDS (Wireless Data Service) clients.
 */
//--------------------------------------------------------------------------------------------------
static WdsClientInfo_t WdsClientInfo[MAX_WDS_CLIENTS][LE_MDMDEFS_IPMAX];

#if defined(SIERRA_MTPDP)

//--------------------------------------------------------------------------------------------------
/**
 * The WDS (Wireless Data Service) clients for MT-PDP incoming notification.
 */
//--------------------------------------------------------------------------------------------------
qmi_client_type MtPdpWdsClient;

#endif

//--------------------------------------------------------------------------------------------------
/**
 * QMI WDS service parameters (for IPv4 and IPv6 connectivities).
 */
//--------------------------------------------------------------------------------------------------
swiQmi_Param_t SwiQmiParam[LE_MDMDEFS_IPMAX];

//--------------------------------------------------------------------------------------------------
/**
 * This event is reported when a data session state change is received from the modem.  The
 * report data is allocated from the associated pool.   Only one event handler is allowed to be
 * registered at a time, so its reference is stored, in case it needs to be removed later.
 */
//--------------------------------------------------------------------------------------------------
static le_event_Id_t NewSessionStateEvent;

//--------------------------------------------------------------------------------------------------
/**
 * The memory pool for the session state
 */
//--------------------------------------------------------------------------------------------------
static le_mem_PoolRef_t NewSessionStatePool;

//--------------------------------------------------------------------------------------------------
/**
 * The memory pool for the profile context
 */
//--------------------------------------------------------------------------------------------------
static le_mem_PoolRef_t ProfileCtxPool;

//--------------------------------------------------------------------------------------------------
/**
 * List of profile
 */
//--------------------------------------------------------------------------------------------------
static le_dls_List_t ProfileCtxList;

//--------------------------------------------------------------------------------------------------
/**
 * Data flow statistics.
 */
//--------------------------------------------------------------------------------------------------
static DataStatistics_t DataStatistics;

//--------------------------------------------------------------------------------------------------
/**
 * Mutex used to protect access to WdsClientInfo.  This is required because the variable
 * can be accessed by both API functions, and the QMI WDS handler.
 */
//--------------------------------------------------------------------------------------------------
static le_mutex_Ref_t WdsMutexRef = NULL;

//--------------------------------------------------------------------------------------------------
/**
 * Trace reference used for controlling tracing in this module.
 */
//--------------------------------------------------------------------------------------------------
static le_log_TraceRef_t TraceRef;

/// Macro used to generate trace output in this module.
/// Takes the same parameters as LE_DEBUG() et. al.
#define TRACE(...) LE_TRACE(TraceRef, ##__VA_ARGS__)

/// Macro used to query current trace state in this module
#define IS_TRACE_ENABLED LE_IS_TRACE_ENABLED(TraceRef)

//--------------------------------------------------------------------------------------------------
/**
 * Translate a wds_ip_family_enum_v01 variable into a le_mdmdefs_ipversion_t variable
 */
//--------------------------------------------------------------------------------------------------
#define WDS_IP_FAMILY_ENUM_2_LE_MDMDEFS_IPVERSION(X) \
                            (X == WDS_IP_FAMILY_IPV4_V01) ? LE_MDMDEFS_IPV4 : LE_MDMDEFS_IPV6

//--------------------------------------------------------------------------------------------------
/**
 * Uninitialize state
 */
//--------------------------------------------------------------------------------------------------
#define WDS_STATE_UNDEF -1

//--------------------------------------------------------------------------------------------------
/**
 * Define prefix name of data interfaces.
 *
 * On 9x15, the QMI_PLATFORM_RMNET_PREFIX is not defined, and it happens to be the platform where
 * "rmnet" should be used, while there is no define referring to "rmnet".
 */
//--------------------------------------------------------------------------------------------------
#if defined(QMI_PLATFORM_RMNET_PREFIX) && defined(QMI_PLATFORM_RMNET_DATA_PREFIX)
    #define DATA_INTERFACE_PREFIX    QMI_PLATFORM_RMNET_DATA_PREFIX
#else
    #define DATA_INTERFACE_PREFIX    "rmnet"
#endif

// =============================================
//  PRIVATE FUNCTIONS
// =============================================
static le_result_t CreateEmptyProfile
(
    uint32_t   testIndex    ///< [IN] The index of the desired profile
);


//--------------------------------------------------------------------------------------------------
/**
 * Kick the mdc watchdog chain
 */
//--------------------------------------------------------------------------------------------------
static void KickWatchdog
(
    void
)
{
    char threadName[MAX_THREAD_NAME_SIZE];

    le_thread_GetName(le_thread_GetCurrent(), threadName, MAX_THREAD_NAME_SIZE);

    if (strncmp(threadName, WDOG_THREAD_NAME_MDC_COMMAND_EVENT,
                strlen(WDOG_THREAD_NAME_MDC_COMMAND_EVENT)) == 0)
    {
        le_wdogChain_Kick(MS_WDOG_MDC_LOOP);
    }
    else
    {
        le_wdogChain_Kick(MS_WDOG_MAIN_LOOP);
    }
}


//--------------------------------------------------------------------------------------------------
/**
 * Dump the data flow statistics if the traces are enabled
 */
//--------------------------------------------------------------------------------------------------
static void DumpDataFlowStatistics
(
    void
)
{
    if (IS_TRACE_ENABLED)
    {
        LE_DEBUG("Saving statistics: %c", (DataStatistics.saving?'Y':'N'));
        LE_DEBUG("Saved statistics: rx=%"PRIu64", tx=%"PRIu64,
                 DataStatistics.saved.receivedBytesCount,
                 DataStatistics.saved.transmittedBytesCount);
        LE_DEBUG("Reset statistics: rx=%"PRIu64", tx=%"PRIu64,
                 DataStatistics.reset.receivedBytesCount,
                 DataStatistics.reset.transmittedBytesCount);
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Test if the given profile index is a 3GPP2 profile
 *
 * @return
 *      The index value
 */
//--------------------------------------------------------------------------------------------------
static bool Is3Gpp2Profile
(
    uint32_t profileIndex                     ///< [IN] The profile to test
)
{
    if ( ( ( profileIndex >= PA_MDC_MIN_INDEX_3GPP2_PROFILE ) &&
         ( profileIndex <= PA_MDC_MAX_INDEX_3GPP2_PROFILE ) ) ||
         ( profileIndex == 0 ) )
    {
        return true;
    }
    else
    {
        return false;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the profile context from the profile dls list (create if doesn't exist).
 */
//--------------------------------------------------------------------------------------------------
static ProfileCtx_t* GetProfileCtxFromList
(
    uint32_t index ///< index of the search profile.
)
{
    le_dls_Link_t* profileCtxLinkPtr = le_dls_Peek(&ProfileCtxList);
    ProfileCtx_t* profileCtxPtr = NULL;

    while(profileCtxLinkPtr)
    {
        ProfileCtx_t* profileCtxPtr = CONTAINER_OF(profileCtxLinkPtr, ProfileCtx_t, link);

        if ( profileCtxPtr->profileIndex == index )
        {
            return profileCtxPtr;
        }
        else
        {
            profileCtxLinkPtr = le_dls_PeekNext(&ProfileCtxList,profileCtxLinkPtr);
        }
    }

    LE_DEBUG("No profile found, create a new one");
    profileCtxPtr = le_mem_ForceAlloc(ProfileCtxPool);
    memset(profileCtxPtr,0,sizeof(ProfileCtx_t));
    profileCtxPtr->profileIndex = index;
    profileCtxPtr->mappedRmnetId = 0xFF;
    profileCtxPtr->callContext.ipv4.state = WDS_STATE_UNDEF;
    profileCtxPtr->callContext.ipv6.state = WDS_STATE_UNDEF;
    profileCtxPtr->link = LE_DLS_LINK_INIT;
    le_dls_Queue(&ProfileCtxList, &profileCtxPtr->link);

    return profileCtxPtr;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the rmnet interface number of a profile index
 *
 * @return
 *      The index value
 */
//--------------------------------------------------------------------------------------------------
static int GetRmnetInterfaceNumber
(
    uint32_t profileIndex
)
{
    ProfileCtx_t *profileCtxPtr = GetProfileCtxFromList(profileIndex);

    if (profileCtxPtr)
    {
        if ((profileCtxPtr->clientInfoPtr[LE_MDMDEFS_IPV4] != NULL) &&
            (profileCtxPtr->clientInfoPtr[LE_MDMDEFS_IPV4]->profileIndex == profileIndex))
        {
            return profileCtxPtr->clientInfoPtr[LE_MDMDEFS_IPV4]->rmnetNumber;
        }
        else if ((profileCtxPtr->clientInfoPtr[LE_MDMDEFS_IPV6] != NULL) &&
              (profileCtxPtr->clientInfoPtr[LE_MDMDEFS_IPV6]->profileIndex == profileIndex))
        {
            return profileCtxPtr->clientInfoPtr[LE_MDMDEFS_IPV6]->rmnetNumber;
        }
    }

    return 0;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the QMI WDS client used with a specific profile index
 * The wdsClient is released when the QMI request is over.
 * When a connection is started, the wdsclient is still active until the session is stopped
 * (the wdsclient is locked with the profile thanks to the lock parameter), or a disconnection is
 * received from the network.
 *
 * @return
 *      The QMI WDS client handle
 */
//--------------------------------------------------------------------------------------------------
static qmi_client_type GetWdsClient
(
    uint32_t profileIndex,
    le_mdmDefs_IpVersion_t ipType,
    bool createIfDontExist,
    bool lock
)
{
    int index;
    qmi_client_type wdsClient = NULL;

    if (ipType >= LE_MDMDEFS_IPMAX)
    {
        LE_ERROR("Bad input parameter");
        return NULL;
    }

    ProfileCtx_t *profileCtxPtr = GetProfileCtxFromList(profileIndex);

    if (!profileCtxPtr)
    {
        return NULL;
    }

    le_mutex_Lock(WdsMutexRef);

    // If a wdsClient is not linked with the profile, allocate a new wdsClient
    if ( createIfDontExist &&
         ( (profileCtxPtr->clientInfoPtr[ipType] == NULL) ||
           (profileCtxPtr->clientInfoPtr[ipType]->profileIndex != profileIndex) ) )
    {
        // Search an empty channel
        for (index=0; index < MAX_WDS_CLIENTS; index++)
        {
            if (WdsClientInfo[index][ipType].profileIndex == 0)
            {
                LE_DEBUG("index %d, ipType %d", index, ipType);

                if ( WdsClientInfo[index][ipType].wdsClientInfo.serviceHandle == NULL )
                {
                    // If rmnet is not mapped, allocate automatically a RMNET interface number
                    if (profileCtxPtr->mappedRmnetId == 0xFF)
                    {
                        int i;

                        LE_DEBUG("usedInfoIndex = 0x%x", SwiQmiParam[ipType].usedInfoIndex);
                        // Search a free info index
                        for (i = 0; i < 8*sizeof(SwiQmiParam[ipType].usedInfoIndex); i++)
                        {
                            if ((SwiQmiParam[ipType].usedInfoIndex & (1<<i)) == 0)
                            {
                                SwiQmiParam[ipType].infoIndex = i;
                                SwiQmiParam[ipType].usedInfoIndex |= (1<<i);
                                break;
                            }
                        }

                        if (i == 8*sizeof(SwiQmiParam[ipType].usedInfoIndex))
                        {
                            // All bit are set to 1 => error
                            wdsClient = NULL;
                            goto end;
                        }
                    }
                    else
                    {
                        if (1<<profileCtxPtr->mappedRmnetId >
                                                          sizeof(SwiQmiParam[ipType].usedInfoIndex))
                        {
                            LE_ERROR("Bad mapping mappedRmnetId=%d", profileCtxPtr->mappedRmnetId);
                            wdsClient = NULL;
                            goto end;
                        }
                        // Check if the rmnet is currently in used
                        if (SwiQmiParam[ipType].usedInfoIndex & (1<<profileCtxPtr->mappedRmnetId))
                        {
                            LE_ERROR("RMNET %d already in use", profileCtxPtr->mappedRmnetId);
                            wdsClient = NULL;
                            goto end;
                        }

                        // Use the specifed RMNET
                        SwiQmiParam[ipType].usedInfoIndex |= (1<<profileCtxPtr->mappedRmnetId);
                        SwiQmiParam[ipType].infoIndex = profileCtxPtr->mappedRmnetId;
                    }

                    LE_DEBUG(" infoIndex %d", SwiQmiParam[ipType].infoIndex);

                    le_result_t res = swiQmi_InitQmiService(QMI_SERVICE_WDS,
                                            &(WdsClientInfo[index][ipType].wdsClientInfo),
                                            &SwiQmiParam[ipType]);

                    if ((res != LE_OK) ||
                        (WdsClientInfo[index][ipType].wdsClientInfo.serviceHandle == NULL))
                    {
                        LE_ERROR("No QMI client res %d", res);
                        WdsClientInfo[index][ipType].wdsClientInfo.serviceHandle = NULL;
                        wdsClient = NULL;
                        goto end;
                    }

                    // rmnet identifier is the index of the "info" area
                    WdsClientInfo[index][ipType].rmnetNumber = SwiQmiParam[ipType].infoIndex;
                }

                LE_DEBUG("qmiClient %p", WdsClientInfo[index][ipType].wdsClientInfo.serviceHandle);

                // The lock parameter is used to link a profile to a wdsClient
                if (lock)
                {
                    profileCtxPtr->clientInfoPtr[ipType] = &WdsClientInfo[index][ipType];

                    WdsClientInfo[index][ipType].profileIndex = profileIndex;
                }

                wdsClient = WdsClientInfo[index][ipType].wdsClientInfo.serviceHandle;
                goto end;
            }
        }
    }
    // wdsclient in used with a profile => reuse the same
    else
    {
        if (profileCtxPtr->clientInfoPtr[ipType] != NULL)
        {
            if (profileCtxPtr->clientInfoPtr[ipType]->profileIndex == profileIndex)
            {
                LE_DEBUG("wdsclient %p",
                                profileCtxPtr->clientInfoPtr[ipType]->wdsClientInfo.serviceHandle);
                wdsClient = profileCtxPtr->clientInfoPtr[ipType]->wdsClientInfo.serviceHandle;
                goto end;
            }
            else
            {
                LE_ERROR("Bad allocation: profileIndex %d, clientInfoPtr->profileIndex %d",
                            profileIndex, profileCtxPtr->clientInfoPtr[ipType]->profileIndex);
                wdsClient = NULL;
                goto end;
            }
        }
        else
        {
            LE_DEBUG("Profile not mapped (i.e. not started)");
            wdsClient = NULL;
            goto end;
        }
    }

end:
    le_mutex_Unlock(WdsMutexRef);
    return wdsClient;
}

//--------------------------------------------------------------------------------------------------
/**
 * Unassign the profile index of a QMI WDS client
 *
 */
//--------------------------------------------------------------------------------------------------
static void UnassignedWdsClient
(
    uint32_t profileIndex,
    le_mdmDefs_IpVersion_t ipType
)
{
    ProfileCtx_t *profileCtxPtr = GetProfileCtxFromList(profileIndex);

    le_mutex_Lock(WdsMutexRef);
    if (profileCtxPtr && profileCtxPtr->clientInfoPtr[ipType])
    {
        profileCtxPtr->clientInfoPtr[ipType]->profileIndex = 0;
        profileCtxPtr->clientInfoPtr[ipType] = NULL;
    }
    le_mutex_Unlock(WdsMutexRef);
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the profile index assigned to a QMI WDS client
 *
 * @return
 *      The QMI WDS client handle
 */
//--------------------------------------------------------------------------------------------------
static uint32_t GetProfileIndexFromWdsClient
(
    qmi_client_type wdsClient
)
{
    uint32_t index;
    le_mdmDefs_IpVersion_t ipType;

    for (ipType=0; ipType < LE_MDMDEFS_IPMAX; ipType++)
    {
        for (index=0; index < MAX_WDS_CLIENTS; index++)
        {
            if ((WdsClientInfo[index][ipType].wdsClientInfo.serviceHandle == wdsClient) &&
                (WdsClientInfo[index][ipType].profileIndex != 0))
            {
                return WdsClientInfo[index][ipType].profileIndex;
            }
        }
    }

    return 0;
}

//--------------------------------------------------------------------------------------------------
/**
 * Convert the IPv4 address from numeric to string format
 *
 * @return
 *      - LE_OK on success
 *      - LE_OVERFLOW if size of ipStr is too small
 *      - LE_FAULT on failure
 *
 * @note
 *      Appropriate error messages will be logged on error
 */
//--------------------------------------------------------------------------------------------------
static le_result_t ConvertIPv4NumToStr
(
    uint32_t ipNum,             ///< [IN] IP address as a number
    char* ipStr,                ///< [OUT] Buffer for IP address as a string
    size_t ipStrSize            ///< [IN] Size in bytes of ipStr buffer
)
{
    struct in_addr ipAddrStruct;

    if ( ipStrSize < INET_ADDRSTRLEN )
    {
        LE_ERROR("ipStr size is %d; must be at least %i", (uint32_t) ipStrSize, INET_ADDRSTRLEN);
        return LE_OVERFLOW;
    }

    ipAddrStruct.s_addr = htonl(ipNum);

    if ( inet_ntop(AF_INET, &ipAddrStruct, ipStr, ipStrSize) == NULL )
    {
        LE_ERROR("Error converting %X to IP string", ipNum);
        return LE_FAULT;
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Convert the IPv6 address from numeric to string format
 *
 * @return
 *      - LE_OK on success
 *      - LE_OVERFLOW if size of ipStr is too small
 *      - LE_FAULT on failure
 *
 * @note
 *      Appropriate error messages will be logged on error
 */
//--------------------------------------------------------------------------------------------------
static le_result_t ConvertIPv6NumToStr
(
    uint8_t ipNum[16],          ///< [IN] IP address as a number
    char* ipStr,                ///< [OUT] Buffer for IP address as a string
    size_t ipStrSize            ///< [IN] Size in bytes of ipStr buffer
)
{
    int i;
    struct in6_addr ipAddrStruct;

    if ( ipStrSize < INET6_ADDRSTRLEN )
    {
        LE_ERROR("ipStr size is %d; must be at least %i", (uint32_t) ipStrSize, INET6_ADDRSTRLEN);
        return LE_OVERFLOW;
    }

    for (i=0;i<16;i++) {
        ipAddrStruct.s6_addr[i] = ipNum[i];
    }

    if ( inet_ntop(AF_INET6, &ipAddrStruct, ipStr, ipStrSize) == NULL )
    {
        char ipStr[33];
        le_hex_BinaryToString(ipNum,16,ipStr,33);
        LE_ERROR("Error converting %s to IP string", ipStr);
        return LE_FAULT;
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Translate the data session state from a QMI enum to Legato enum
 *
 * @return
 *      session state as a Legato enum
 */
//--------------------------------------------------------------------------------------------------
static le_mdc_ConState_t TranslateSessionState
(
    wds_connection_status_enum_v01 qmiSessionState     ///< [IN] session state as a QMI enum
)
{
    le_mdc_ConState_t sessionState;

    switch (qmiSessionState)
    {
        case WDS_CONNECTION_STATUS_CONNECTED_V01:
            sessionState = LE_MDC_CONNECTED;
        break;
        case WDS_CONNECTION_STATUS_SUSPENDED_V01:
            sessionState = LE_MDC_SUSPENDING;
        break;
        case WDS_CONNECTION_STATUS_AUTHENTICATING_V01:
            sessionState = LE_MDC_AUTHENTICATING;
        break;
        case WDS_CONNECTION_STATUS_DISCONNECTED_V01:
        default:
            sessionState = LE_MDC_DISCONNECTED;
        break;
    }

    return sessionState;
}

//--------------------------------------------------------------------------------------------------
/**
 * Remove Wds Client handler
 * This function is called in the main MDC thread from the QMI handler thread
 *
 */
//--------------------------------------------------------------------------------------------------
static void RemoveWdsClientHandler
(
    void* param1Ptr,
    void* param2Ptr
)
{
    qmi_client_type wdsClient = (qmi_client_type) param1Ptr;
    le_mdmDefs_IpVersion_t ipVersion = (le_mdmDefs_IpVersion_t) param2Ptr;
    uint32_t profileIndex = GetProfileIndexFromWdsClient(wdsClient);

    UnassignedWdsClient(profileIndex, (le_mdmDefs_IpVersion_t) ipVersion);
    mdc_qmi_ReleaseWdsClient(wdsClient);
}


//--------------------------------------------------------------------------------------------------
/**
 * Translate the disconnection reason from a QMI enum to Legato enum
 *
 * @return
 *      Disconnection reason
 */
//--------------------------------------------------------------------------------------------------
static le_mdc_DisconnectionReason_t TranslateDisconnectionReason
(
    wds_call_end_reason_enum_v01 call_end_reason
)
{
    switch (call_end_reason)
    {
        // Technology-agnostic call end reasons
        case WDS_CER_UNSPECIFIED_V01:
            return LE_MDC_DISC_UNDEFINED;

        case WDS_CER_CLIENT_END_V01:
            return LE_MDC_DISC_REGULAR_DEACTIVATION;

        case WDS_CER_NO_SRV_V01:
            return LE_MDC_DISC_NO_SERVICE;

        case WDS_CER_FADE_V01:
            return LE_MDC_DISC_PLATFORM_SPECIFIC;

        case WDS_CER_REL_NORMAL_V01:
            return LE_MDC_DISC_REGULAR_DEACTIVATION;

        case WDS_CER_ACC_IN_PROG_V01:
            return LE_MDC_DISC_PLATFORM_SPECIFIC;

        case WDS_CER_ACC_FAIL_V01:
            return LE_MDC_DISC_PLATFORM_SPECIFIC;

        case WDS_CER_REDIR_OR_HANDOFF_V01:
            return LE_MDC_DISC_PLATFORM_SPECIFIC;

        case WDS_CER_CLOSE_IN_PROGRESS_V01:
            return LE_MDC_DISC_PLATFORM_SPECIFIC;

        case WDS_CER_AUTH_FAILED_V01:
            return LE_MDC_DISC_USER_AUTHENTICATION_FAILURE;

        case WDS_CER_INTERNAL_CALL_END_V01:
            return LE_MDC_DISC_PLATFORM_SPECIFIC;

        // CDMA call end reasons
        case WDS_CER_CDMA_LOCK_V01:
            return LE_MDC_DISC_PLATFORM_SPECIFIC;

        case WDS_CER_INTERCEPT_V01:
            return LE_MDC_DISC_PLATFORM_SPECIFIC;

        case WDS_CER_REORDER_V01:
            return LE_MDC_DISC_PLATFORM_SPECIFIC;

        case WDS_CER_REL_SO_REJ_V01:
            return LE_MDC_DISC_PLATFORM_SPECIFIC;

        case WDS_CER_INCOM_CALL_V01:
            return LE_MDC_DISC_PLATFORM_SPECIFIC;

        case WDS_CER_ALERT_STOP_V01:
            return LE_MDC_DISC_PLATFORM_SPECIFIC;

        case WDS_CER_ACTIVATION_V01:
            return LE_MDC_DISC_PLATFORM_SPECIFIC;

        case WDS_CER_MAX_ACCESS_PROBE_V01:
            return LE_MDC_DISC_PLATFORM_SPECIFIC;

        case WDS_CER_CCS_NOT_SUPP_BY_BS_V01:
            return LE_MDC_DISC_PLATFORM_SPECIFIC;

        case WDS_CER_NO_RESPONSE_FROM_BS_V01:
            return LE_MDC_DISC_PLATFORM_SPECIFIC;

        case WDS_CER_REJECTED_BY_BS_V01:
            return LE_MDC_DISC_PLATFORM_SPECIFIC;

        case WDS_CER_INCOMPATIBLE_V01:
            return LE_MDC_DISC_PLATFORM_SPECIFIC;

        case WDS_CER_ALREADY_IN_TC_V01:
            return LE_MDC_DISC_PLATFORM_SPECIFIC;

        case WDS_CER_USER_CALL_ORIG_DURING_GPS_V01:
            return LE_MDC_DISC_PLATFORM_SPECIFIC;

        case WDS_CER_USER_CALL_ORIG_DURING_SMS_V01:
            return LE_MDC_DISC_PLATFORM_SPECIFIC;

        case WDS_CER_NO_CDMA_SRV_V01:
            return LE_MDC_DISC_NO_SERVICE;

        // WCDMA/GSM disconnection reason
        case WDS_CER_CONF_FAILED_V01:
            return LE_MDC_DISC_ACTIVATION_REJECTED_UNSPECIFIED;

        case WDS_CER_INCOM_REJ_V01:
            return LE_MDC_DISC_PLATFORM_SPECIFIC;

        case WDS_CER_NO_GW_SRV_V01:
            return LE_MDC_DISC_NO_SERVICE;

        case WDS_CER_NETWORK_END_V01:
            return LE_MDC_DISC_NETWORK_FAILURE;

        case WDS_CER_LLC_SNDCP_FAILURE_V01:
            return LE_MDC_DISC_LLC_SNDCP_FAILURE;

        case WDS_CER_INSUFFICIENT_RESOURCES_V01:
            return LE_MDC_DISC_INSUFFICIENT_RESOURCES;

        case WDS_CER_OPTION_TEMP_OOO_V01:
            return LE_MDC_DISC_REQUESTED_SERVICE_OPTION_OUT_OF_ORDER;

        case WDS_CER_NSAPI_ALREADY_USED_V01:
            return LE_MDC_DISC_NSAPI_ALREADY_USED;

        case WDS_CER_REGULAR_DEACTIVATION_V01:
            return LE_MDC_DISC_REGULAR_DEACTIVATION;

        case WDS_CER_NETWORK_FAILURE_V01:
            return LE_MDC_DISC_NETWORK_FAILURE;

        case WDS_CER_UMTS_REATTACH_REQ_V01:
            return LE_MDC_DISC_REACTIVATION_REQUESTED;

        case WDS_CER_PROTOCOL_ERROR_V01:
            return LE_MDC_DISC_PROTOCOL_ERROR_UNSPECIFIED;

        case WDS_CER_OPERATOR_DETERMINED_BARRING_V01:
            return LE_MDC_DISC_OPERATOR_DETERMINED_BARRING;

        case WDS_CER_UNKNOWN_APN_V01:
            return LE_MDC_DISC_MISSING_OR_UNKNOWN_APN;

        case WDS_CER_UNKNOWN_PDP_V01:
            return LE_MDC_DISC_UNKNOWN_PDP_CONTEXT;

        case WDS_CER_GGSN_REJECT_V01:
            return LE_MDC_DISC_ACTIVATION_REJECTED_BY_GGSN_OR_GW;

        case WDS_CER_ACTIVATION_REJECT_V01:
            return LE_MDC_DISC_ACTIVATION_REJECTED_UNSPECIFIED;

        case WDS_CER_OPTION_NOT_SUPP_V01:
            return LE_MDC_DISC_SERVICE_OPTION_NOT_SUPPORTED;

        case WDS_CER_OPTION_UNSUBSCRIBED_V01:
            return LE_MDC_DISC_REQUESTED_SERVICE_OPTION_UNSUBSCRIBED;

        case WDS_CER_QOS_NOT_ACCEPTED_V01:
            return LE_MDC_DISC_QOS_NOT_ACCEPTED;

        case WDS_CER_TFT_SEMANTIC_ERROR_V01:
            return LE_MDC_DISC_TFT_SEMANTIC_ERROR;

        case WDS_CER_TFT_SYNTAX_ERROR_V01:
            return LE_MDC_DISC_TFT_SYNTACTICAL_ERROR;

        case WDS_CER_UNKNOWN_PDP_CONTEXT_V01:
            return LE_MDC_DISC_UNKNOWN_PDP_CONTEXT;

        case WDS_CER_FILTER_SEMANTIC_ERROR_V01:
            return LE_MDC_DISC_PACKET_FILTER_SEMANTIC_ERROR;

        case WDS_CER_FILTER_SYNTAX_ERROR_V01:
            return LE_MDC_DISC_PACKET_FILTER_SYNTACTICAL_ERROR;

        case WDS_CER_PDP_WITHOUT_ACTIVE_TFT_V01:
            return LE_MDC_DISC_PDP_CONTEXT_WITHOUT_ACTIVE_TFT;

        case WDS_CER_INVALID_TRANSACTION_ID_V01:
            return LE_MDC_DISC_INVALID_TRANSACTION_ID;

        case WDS_CER_MESSAGE_INCORRECT_SEMANTIC_V01:
            return LE_MDC_DISC_MESSAGE_INCORRECT_SEMANTIC;

        case WDS_CER_INVALID_MANDATORY_INFO_V01:
            return LE_MDC_DISC_INVALID_MANDATORY_INFORMATION;

        case WDS_CER_MESSAGE_TYPE_UNSUPPORTED_V01:
            return LE_MDC_DISC_UNSUPPORTED_MESSAGE_TYPE;

        case WDS_CER_MSG_TYPE_NONCOMPATIBLE_STATE_V01:
            return LE_MDC_DISC_MESSAGE_AND_STATE_UNCOMPATIBLE;

        case WDS_CER_UNKNOWN_INFO_ELEMENT_V01:
            return LE_MDC_DISC_UNKNOWN_INFORMATION_ELEMENT;

        case WDS_CER_CONDITIONAL_IE_ERROR_V01:
            return LE_MDC_DISC_CONDITIONAL_IE_ERROR;

        case WDS_CER_MSG_AND_PROTOCOL_STATE_UNCOMPATIBLE_V01:
            return LE_MDC_DISC_MESSAGE_AND_PROTOCOL_STATE_UNCOMPATIBLE;

        case WDS_CER_APN_TYPE_CONFLICT_V01:
            return LE_MDC_DISC_INCOMPATIBLE_APN;

        case WDS_CER_NO_GPRS_CONTEXT_V01:
            return LE_MDC_DISC_PLATFORM_SPECIFIC;

        case WDS_CER_FEATURE_NOT_SUPPORTED_V01:
            return LE_MDC_DISC_FEATURE_NOT_SUPPORTED;

        // xEV-DO call end reasons
        case WDS_CER_CD_GEN_OR_BUSY_V01:
            return LE_MDC_DISC_PLATFORM_SPECIFIC;

        case WDS_CER_CD_BILL_OR_AUTH_V01:
            return LE_MDC_DISC_PLATFORM_SPECIFIC;

        case WDS_CER_CHG_HDR_V01:
            return LE_MDC_DISC_PLATFORM_SPECIFIC;

        case WDS_CER_EXIT_HDR_V01:
            return LE_MDC_DISC_PLATFORM_SPECIFIC;

        case WDS_CER_HDR_NO_SESSION_V01:
            return LE_MDC_DISC_PLATFORM_SPECIFIC;

        case WDS_CER_HDR_ORIG_DURING_GPS_FIX_V01:
            return LE_MDC_DISC_PLATFORM_SPECIFIC;

        case WDS_CER_HDR_CS_TIMEOUT_V01:
            return LE_MDC_DISC_PLATFORM_SPECIFIC;

        case WDS_CER_HDR_RELEASED_BY_CM_V01:
            return LE_MDC_DISC_PLATFORM_SPECIFIC;

        default:
            return LE_MDC_DISC_PLATFORM_SPECIFIC;
    }

    return LE_MDC_DISC_PLATFORM_SPECIFIC;
}

//--------------------------------------------------------------------------------------------------
/**
 * Handler for QMI_WDS_PKT_SRVC_STATUS_IND_V01.  Generates new session state events.
 */
//--------------------------------------------------------------------------------------------------
static void QMINewSessionStateHandler
(
    void*          indBufPtr, ///< [IN] The indication message data.
    unsigned int   indBufLen, ///< [IN] The indication message data length in bytes.
    void*          contextPtr ///< [IN] Context value that was passed to swiQmi_AddIndicationHandler
)
{
    qmi_client_error_type rc;

    wds_pkt_srvc_status_ind_msg_v01 wdsPktSvcInd;
    pa_mdc_SessionStateData_t* sessionStateDataPtr;
    wds_ip_family_enum_v01 ipFamily = WDS_IP_FAMILY_IPV4_V01;
    qmi_client_type wdsClient = (qmi_client_type) contextPtr;
    uint32_t profileIndex = GetProfileIndexFromWdsClient(wdsClient);

    if ( profileIndex == 0 )
    {
        LE_DEBUG("No profile index found for wdsClient %p", wdsClient);
        return;
    }

    LE_DEBUG("wdsClient: %p, profile index: %d", wdsClient, profileIndex);

    rc = qmi_client_message_decode(
        wdsClient, QMI_IDL_INDICATION, QMI_WDS_PKT_SRVC_STATUS_IND_V01,
        indBufPtr, indBufLen,
        &wdsPktSvcInd, sizeof(wdsPktSvcInd));

    if (rc != QMI_NO_ERR)
    {
        LE_ERROR("Error in decoding QMI_WDS_PKT_SRVC_STATUS_IND_V01, rc=%i", rc);
        return;
    }

    if ( IS_TRACE_ENABLED )
    {
        LE_PRINT_VALUE("%i", wdsPktSvcInd.status.connection_status);
        LE_PRINT_VALUE("%i", wdsPktSvcInd.status.reconfiguration_required);
        LE_PRINT_VALUE_IF(wdsPktSvcInd.call_end_reason_valid, "%i", wdsPktSvcInd.call_end_reason);
        LE_PRINT_VALUE_IF(wdsPktSvcInd.ip_family_valid, "%i", wdsPktSvcInd.ip_family);
        LE_PRINT_VALUE_IF(wdsPktSvcInd.tech_name_valid, "%i", wdsPktSvcInd.tech_name);
        LE_PRINT_VALUE_IF(wdsPktSvcInd.verbose_call_end_reason_valid,
            "%i", wdsPktSvcInd.verbose_call_end_reason.call_end_reason_type);
        LE_PRINT_VALUE_IF(wdsPktSvcInd.verbose_call_end_reason_valid,
            "%i", wdsPktSvcInd.verbose_call_end_reason.call_end_reason);
    }

    // Init the data for the event report
    sessionStateDataPtr = le_mem_ForceAlloc(NewSessionStatePool);
    sessionStateDataPtr->profileIndex = profileIndex;
    sessionStateDataPtr->newState = TranslateSessionState(wdsPktSvcInd.status.connection_status);

    if(wdsPktSvcInd.ip_family_valid)
    {
        ipFamily = wdsPktSvcInd.ip_family;

        switch(wdsPktSvcInd.ip_family)
        {
            case WDS_IP_FAMILY_IPV4_V01:
                sessionStateDataPtr->pdp = LE_MDC_PDP_IPV4;
                break;
            case WDS_IP_FAMILY_IPV6_V01:
                sessionStateDataPtr->pdp = LE_MDC_PDP_IPV6;
                break;
            default:
                sessionStateDataPtr->pdp = LE_MDC_PDP_UNKNOWN;
                break;
        }
    }
    else
    {
        sessionStateDataPtr->pdp = LE_MDC_PDP_UNKNOWN;
    }

    ProfileCtx_t* profileCtxPtr = GetProfileCtxFromList(profileIndex);
    le_mdmDefs_IpVersion_t ipVersion = WDS_IP_FAMILY_ENUM_2_LE_MDMDEFS_IPVERSION(ipFamily);
    CallIpContext_t* ipCtxPtr = NULL;

    if (profileCtxPtr)
    {
        if (ipVersion == LE_MDMDEFS_IPV4)
        {
            ipCtxPtr = &profileCtxPtr->callContext.ipv4;
        }
        else
        {
            ipCtxPtr = &profileCtxPtr->callContext.ipv6;
        }
    }

    if ( sessionStateDataPtr->newState == LE_MDC_CONNECTED )
    {
        if (IsStartSession)
        {
            IsStartSession = false;
            le_sem_Post(SessionSemRef);
        }
    }
    else if ( sessionStateDataPtr->newState == LE_MDC_DISCONNECTED )
    {
        if (ipCtxPtr)
        {
            memset(ipCtxPtr,0,sizeof(CallIpContext_t));
        }

        if (wdsPktSvcInd.call_end_reason_valid)
        {
            sessionStateDataPtr->disc = TranslateDisconnectionReason(wdsPktSvcInd.call_end_reason);
            sessionStateDataPtr->discCode = wdsPktSvcInd.call_end_reason;
        }
        else
        {
            sessionStateDataPtr->disc = LE_MDC_DISC_UNDEFINED;
            sessionStateDataPtr->discCode = -1;
        }

        if (IsStopSession)
        {
            IsStopSession = false;
            le_sem_Post(SessionSemRef);
        }
        else
        {
            le_event_QueueFunctionToThread(CurrentThreadRef,
                RemoveWdsClientHandler,
                wdsClient,
                (void *) ipVersion);
        }
    }

    // set the state
    if (ipCtxPtr)
    {
        ipCtxPtr->state = sessionStateDataPtr->newState;
    }

    if ( profileCtxPtr && (LE_MDC_CONNECTED == sessionStateDataPtr->newState)
         && (LE_MDC_PDP_IPV4V6 == profileCtxPtr->pdpType ))
    {
        // LE_MDC_CONNECTED event is sent by pa_mdc_StartSessionIPV4V6() for IPV4V6 PDP type
        LE_DEBUG("Connect status Ipv4 %c, Ipv6 %c",
                ((LE_MDC_CONNECTED == profileCtxPtr->callContext.ipv4.state) ? 'Y':'N'),
                ((LE_MDC_CONNECTED == profileCtxPtr->callContext.ipv6.state) ? 'Y':'N') );
        le_mem_Release(sessionStateDataPtr);
    }
    else
    {
        le_event_ReportWithRefCounting(NewSessionStateEvent, sessionStateDataPtr);
    }


}

#if defined(SIERRA_MTPDP)

//--------------------------------------------------------------------------------------------------
/**
 * Handler for QMI_WDS_SWI_SET_MTPDP_RING_REPORT_IND_V01.  Generates MT-PDP ring report events.
 */
//--------------------------------------------------------------------------------------------------
static void QMImtPdpRingReportHandler
(
    void*          indBufPtr, ///< [IN] The indication message data.
    unsigned int   indBufLen, ///< [IN] The indication message data length in bytes.
    void*          contextPtr ///< [IN] Context value that was passed to swiQmi_AddIndicationHandler
)
{
    qmi_client_error_type rc;
    wds_swi_set_mtpdp_ring_report_ind_msg_v01 mtPdpRingReportInd = {0};
    pa_mdc_SessionStateData_t* sessionStateDataPtr;
    qmi_client_type wdsClient = (qmi_client_type) contextPtr;

    LE_DEBUG("MT-PDP wdsClient %p", wdsClient);

    rc = qmi_client_message_decode(
        wdsClient, QMI_IDL_INDICATION, QMI_WDS_SWI_SET_MTPDP_RING_REPORT_IND_V01,
        indBufPtr, indBufLen,
        &mtPdpRingReportInd, sizeof(mtPdpRingReportInd));

    if (rc != QMI_NO_ERR)
    {
        LE_ERROR("Error in decoding QMI_WDS_SWI_SET_MTPDP_RING_REPORT_IND_V01, rc=%i", rc);
        return;
    }

    if ( IS_TRACE_ENABLED )
    {
        LE_PRINT_VALUE("%i", mtPdpRingReportInd.profile_id);
        LE_PRINT_VALUE("%i", mtPdpRingReportInd.pdp_type);
        LE_PRINT_VALUE("%s", mtPdpRingReportInd.apn.apn_name);
        LE_PRINT_VALUE_IF(mtPdpRingReportInd.pdp_addr.valid,
                          "%s", mtPdpRingReportInd.pdp_addr.address);
        LE_PRINT_VALUE("%i", mtPdpRingReportInd.pdp_addr.pdp_type_num);
        LE_PRINT_VALUE("%i", mtPdpRingReportInd.pdp_addr.pdp_type_org);
    }

    // Profile Index from the notification
    LE_DEBUG("MT-PDP profile index: %d", mtPdpRingReportInd.profile_id);

    // MT-PDP Profile management
    MtPdpWdsClient = wdsClient;

    /*  pdp_type */
    switch(mtPdpRingReportInd.pdp_type)
    {
        case 0: // DS_UMTS_PDP_IP=0x0,      /**< PDP type IP. */
        case 1: // DS_UMTS_PDP_IPV4=0x0,    /**< PDP type IPv4. */
            LE_DEBUG("MT-PDP ipv4");
        break;
        case 2: // DS_UMTS_PDP_IPV6=0x2,/**< PDP type IPv6. */
            LE_DEBUG("MT-PDP ipv6");
        break;
        case 3: // DS_UMTS_PDP_IPV4V6=0x3, /**< PDP type IPv4v6. */
            LE_DEBUG("MT-PDP ipv4v6");
        break;
        default: // DS_UMTS_PDP_PPP=0x1, /**< PDP type PPP. */
            LE_ERROR("MT-PDP ip state unknown %d", mtPdpRingReportInd.pdp_type);
        break;
    }

    // Init the data for the event report
    sessionStateDataPtr = le_mem_ForceAlloc(NewSessionStatePool);
    sessionStateDataPtr->profileIndex = mtPdpRingReportInd.profile_id;
    sessionStateDataPtr->newState = LE_MDC_INCOMING;

    le_event_ReportWithRefCounting(NewSessionStateEvent, sessionStateDataPtr);

}

//--------------------------------------------------------------------------------------------------
/**
 * Set the ring indicator notification for MT-PDP.
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
static le_result_t SetMtPdpRingIndicator
(
    bool   ringIndicator    ///< [IN] The state of MT-PDP ring indicator
)
{
    qmi_client_error_type rc;
    le_result_t result = LE_OK;

    wds_swi_set_mtpdp_ring_report_req_msg_v01  mtPdpRingReportReq = {0};
    wds_swi_set_mtpdp_ring_report_resp_msg_v01 mtPdpRingReportRsp = { {0} };
    qmi_client_type wdsClient = mdc_qmi_GetDefaultWdsClient();

    if (wdsClient == NULL)
    {
        LE_ERROR("Invalid wdsclient");
        return LE_FAULT;
    }

    LE_DEBUG("MT-PDP Ring indicator to %d wdsClient %p", ringIndicator, wdsClient);

    // QMI message configuration
    if(ringIndicator == false)
    {
        mtPdpRingReportReq.ring_indicator = 0;
    }
    else
    {
        mtPdpRingReportReq.ring_indicator = 1;
    }

    // Send QMI message
    rc = qmi_client_send_msg_sync(wdsClient,
                                  QMI_WDS_SWI_SET_MTPDP_RING_REPORT_REQ_V01,
                                  &mtPdpRingReportReq, sizeof(mtPdpRingReportReq),
                                  &mtPdpRingReportRsp, sizeof(mtPdpRingReportRsp),
                                  COMM_NETWORK_TIMEOUT);

    result = swiQmi_CheckResponseCode( STRINGIZE_EXPAND(QMI_WDS_SWI_SET_MTPDP_RING_REPORT_RESP_V01),
                                       rc, mtPdpRingReportRsp.resp);

    if (result != LE_OK)
    {
        LE_ERROR("Unable to set MT-PDP Ring indicator to %d", ringIndicator);
        result = LE_FAULT;
    }

    mdc_qmi_ReleaseWdsClient(wdsClient);
    return result;
}

#endif

//--------------------------------------------------------------------------------------------------
/**
 * Check if the profile for the specified profile index is defined
 *
 * @return
 *      - true if the profile is defined
 *      - false otherwise
 */
//--------------------------------------------------------------------------------------------------
static bool IsProfileDefined
(
    uint32_t profileIndex      ///< [IN] The profile to test
)
{
    qmi_client_error_type rc;
    bool profileDefined = false;

    wds_get_profile_settings_req_msg_v01 wdsReadReq = { {0} };
    wds_get_profile_settings_resp_msg_v01 wdsReadResp = { {0} };
    qmi_client_type wdsClient = mdc_qmi_GetDefaultWdsClient();

    if (wdsClient == NULL)
    {
        LE_ERROR("Invalid wdsclient");
        return NULL;
    }

    if ( Is3Gpp2Profile ( profileIndex ) )
    {
        wdsReadReq.profile.profile_type = WDS_PROFILE_TYPE_3GPP2_V01;
    }
    else
    {
        wdsReadReq.profile.profile_type = WDS_PROFILE_TYPE_3GPP_V01;
    }

    wdsReadReq.profile.profile_index = profileIndex;

    rc = qmi_client_send_msg_sync(wdsClient,
            QMI_WDS_GET_PROFILE_SETTINGS_REQ_V01,
            &wdsReadReq, sizeof(wdsReadReq),
            &wdsReadResp, sizeof(wdsReadResp),
            COMM_LONG_PLATFORM_TIMEOUT);

    le_result_t result = swiQmi_CheckResponseCode(
                                STRINGIZE_EXPAND(QMI_WDS_GET_PROFILE_SETTINGS_REQ_V01),
                                rc, wdsReadResp.resp);

    if ( result != LE_OK )
    {
        // Technically, we should check if the extended_error_code indicates an invalid profile
        // index, but in reality, any error is sufficient to indicate invalid profile index.
        if ( IS_TRACE_ENABLED )
        {
            LE_PRINT_VALUE_IF(wdsReadResp.extended_error_code_valid,
                              "%X", wdsReadResp.extended_error_code);
        }
        profileDefined = false;
    }
    else
    {
        profileDefined = true;
    }

    mdc_qmi_ReleaseWdsClient(wdsClient);
    return profileDefined;
}


//--------------------------------------------------------------------------------------------------
/**
 * Create a new empty profile.
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
static le_result_t CreateEmptyProfile
(
    uint32_t   testIndex    ///< [IN] The index of the desired profile
)
{
    qmi_client_error_type rc;

    wds_swi_create_profile_req_msg_v01  wdsCreateReq = { {0} };
    wds_swi_create_profile_resp_msg_v01 wdsCreateResp = { {0} };
    qmi_client_type wdsClient = mdc_qmi_GetDefaultWdsClient();

    if (wdsClient == NULL)
    {
        LE_ERROR("Invalid wdsclient");
        return LE_FAULT;
    }

    if ( Is3Gpp2Profile(testIndex) )
    {
        wdsCreateReq.profile.Profile_type = WDS_TECH_TYEP_3GPP2_V01;
        wdsCreateReq.profile.Profile_index = testIndex;
    }
    else
    {
        wdsCreateReq.profile.Profile_type = WDS_TECH_TYPE_3GPP_V01;
        wdsCreateReq.profile.Profile_index = testIndex;
    }

    rc = qmi_client_send_msg_sync(wdsClient,
                                  QMI_WDS_SWI_CREATE_PROFILE_REQ_V01,
                                  &wdsCreateReq, sizeof(wdsCreateReq),
                                  &wdsCreateResp, sizeof(wdsCreateResp),
                                  COMM_LONG_PLATFORM_TIMEOUT);

    le_result_t result = swiQmi_CheckResponseCode(
                                STRINGIZE_EXPAND(QMI_WDS_SWI_CREATE_PROFILE_REQ_V01),
                                rc, wdsCreateResp.resp);

    if (result == LE_OK)
    {
        if (wdsCreateResp.profile.Profile_index != testIndex)
        {
            LE_ERROR("Created profile.%d is different form desired profile.%d!",
                     wdsCreateResp.profile.Profile_index,
                     testIndex);
            result = LE_FAULT;
        }
    }

    mdc_qmi_ReleaseWdsClient(wdsClient);
    return result;
}


//--------------------------------------------------------------------------------------------------
/**
 * Request the runtime settings specified in the mask, and return the response
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
static le_result_t GetRuntimeSettings
(
    uint32_t profileIndex,                                      ///< [IN] The profile to use
    le_mdmDefs_IpVersion_t ipVersion,                           ///< [IN] IP Version
    wds_req_settings_mask_v01 requestedSettingsMask,            ///< [IN] The requested settings
    wds_get_runtime_settings_resp_msg_v01* wdsSettingsRespPtr   ///< [OUT] QMI response message
)
{
    qmi_client_error_type rc;

    wds_get_runtime_settings_req_msg_v01 wdsSettingsReq = {0};
    qmi_client_type wdsClient = GetWdsClient(profileIndex, ipVersion, false, false);

    if (wdsClient == NULL)
    {
        LE_ERROR("Bad wdsClient");
        return LE_FAULT;
    }

    // zero the response structure
    memset(wdsSettingsRespPtr, 0, sizeof(*wdsSettingsRespPtr) );

    wdsSettingsReq.requested_settings_valid = true;
    wdsSettingsReq.requested_settings = requestedSettingsMask;

    LE_DEBUG("GetRunTimeSetting 0x%08"PRIX32,requestedSettingsMask);

    rc = qmi_client_send_msg_sync(wdsClient,
            QMI_WDS_GET_RUNTIME_SETTINGS_REQ_V01,
            &wdsSettingsReq, sizeof(wdsSettingsReq),
            wdsSettingsRespPtr, sizeof(*wdsSettingsRespPtr),
            COMM_LONG_PLATFORM_TIMEOUT);

    le_result_t result = swiQmi_CheckResponseCode(
                                STRINGIZE_EXPAND(QMI_WDS_GET_RUNTIME_SETTINGS_REQ_V01),
                                rc, wdsSettingsRespPtr->resp);
    if ( result != LE_OK )
    {
        return result;
    }

    if ( IS_TRACE_ENABLED )
    {
        LE_PRINT_VALUE_IF(wdsSettingsRespPtr->profile_valid,
                          "%i", wdsSettingsRespPtr->profile.profile_index);
        LE_PRINT_VALUE_IF(wdsSettingsRespPtr->apn_name_valid,
                          "%s", wdsSettingsRespPtr->apn_name);
        LE_PRINT_VALUE_IF(wdsSettingsRespPtr->ip_family_valid,
                          "%d", wdsSettingsRespPtr->ip_family);
        LE_PRINT_VALUE_IF(wdsSettingsRespPtr->ipv4_address_preference_valid,
                          "%X", wdsSettingsRespPtr->ipv4_address_preference);
        LE_PRINT_VALUE_IF(wdsSettingsRespPtr->ipv4_gateway_addr_valid,
                          "%X", wdsSettingsRespPtr->ipv4_gateway_addr);
        LE_PRINT_VALUE_IF(wdsSettingsRespPtr->ipv4_subnet_mask_valid,
                          "%X", wdsSettingsRespPtr->ipv4_subnet_mask);
        LE_PRINT_VALUE_IF(wdsSettingsRespPtr->primary_DNS_IPv4_address_preference_valid,
                          "%X", wdsSettingsRespPtr->primary_DNS_IPv4_address_preference);
        LE_PRINT_VALUE_IF(wdsSettingsRespPtr->secondary_DNS_IPv4_address_preference_valid,
                          "%X", wdsSettingsRespPtr->secondary_DNS_IPv4_address_preference);
        if (wdsSettingsRespPtr->ipv6_addr_valid)
        {
            char ipStr[33];
            le_hex_BinaryToString(wdsSettingsRespPtr->ipv6_addr.ipv6_addr,16,ipStr,33);
            LE_PRINT_VALUE("%s",ipStr);
        }
        if (wdsSettingsRespPtr->ipv6_gateway_addr_valid)
        {
            char ipStr[33];
            le_hex_BinaryToString(wdsSettingsRespPtr->ipv6_gateway_addr.ipv6_addr,16,ipStr,33);
            LE_PRINT_VALUE("%s",ipStr);
        }
        if (wdsSettingsRespPtr->primary_dns_IPv6_address_valid)
        {
            char ipStr[33];
            le_hex_BinaryToString(wdsSettingsRespPtr->primary_dns_IPv6_address,16,ipStr,33);
            LE_PRINT_VALUE("%s",ipStr);
        }
        if (wdsSettingsRespPtr->secondary_dns_IPv6_address_valid)
        {
            char ipStr[33];
            le_hex_BinaryToString(wdsSettingsRespPtr->secondary_dns_IPv6_address,16,ipStr,33);
            LE_PRINT_VALUE("%s",ipStr);
        }
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the data flow statistics.
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT for all other errors
 */
//--------------------------------------------------------------------------------------------------
static le_result_t GetDataFlowStatistics
(
    qmi_client_type         wdsClient,              ///< [IN]  WDS client to use
    bool*                   sessionInProgressPtr,   ///< [OUT] Indicates if a session in progress
    pa_mdc_PktStatistics_t* dataStatisticsPtr       ///< [OUT] Statistics data
)
{
    qmi_client_error_type rc;

    SWIQMI_DECLARE_MESSAGE(wds_get_pkt_statistics_req_msg_v01, wdsPktStatsReq);
    SWIQMI_DECLARE_MESSAGE(wds_get_pkt_statistics_resp_msg_v01, wdsPktStatsResp);

    // Indicate requested information
    wdsPktStatsReq.stats_mask = 0;
    wdsPktStatsReq.stats_mask |= QMI_WDS_MASK_STATS_TX_BYTES_OK_V01;
    wdsPktStatsReq.stats_mask |= QMI_WDS_MASK_STATS_RX_BYTES_OK_V01;

    rc = qmi_client_send_msg_sync(wdsClient,
                                  QMI_WDS_GET_PKT_STATISTICS_REQ_V01,
                                  &wdsPktStatsReq, sizeof(wdsPktStatsReq),
                                  &wdsPktStatsResp, sizeof(wdsPktStatsResp),
                                  COMM_LONG_PLATFORM_TIMEOUT);

    if (LE_OK != swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_WDS_GET_PKT_STATISTICS_REQ_V01),
                                      rc,
                                      wdsPktStatsResp.resp.result,
                                      wdsPktStatsResp.resp.error))
    {
        return LE_FAULT;
    }

    // There is a session in progress, get the current data statistics
    if (QMI_RESULT_SUCCESS_V01 == wdsPktStatsResp.resp.result)
    {
        if (IS_TRACE_ENABLED)
        {
            LE_DEBUG("Trace DataFlowStatistics request");
            LE_PRINT_VALUE_IF(wdsPktStatsResp.rx_ok_bytes_count_valid,
                              "%"PRIu64, wdsPktStatsResp.rx_ok_bytes_count);
            LE_PRINT_VALUE_IF(wdsPktStatsResp.tx_ok_bytes_count_valid,
                              "%"PRIu64, wdsPktStatsResp.tx_ok_bytes_count);
        }
        if (wdsPktStatsResp.rx_ok_bytes_count_valid)
        {
            dataStatisticsPtr->receivedBytesCount = wdsPktStatsResp.rx_ok_bytes_count;
        }
        if (wdsPktStatsResp.tx_ok_bytes_count_valid)
        {
            dataStatisticsPtr->transmittedBytesCount = wdsPktStatsResp.tx_ok_bytes_count;
        }
        *sessionInProgressPtr = true;
        return LE_OK;
    }

    // There is no session in progress, get the last data statistics
    if (   (QMI_RESULT_FAILURE_V01 == wdsPktStatsResp.resp.result)
        && (QMI_ERR_OUT_OF_CALL_V01 == wdsPktStatsResp.resp.error))
    {
        if (IS_TRACE_ENABLED)
        {
            LE_DEBUG("Trace DataFlowStatistics request");
            LE_PRINT_VALUE_IF(wdsPktStatsResp.last_call_rx_ok_bytes_count_valid,
                              "%"PRIu64, wdsPktStatsResp.last_call_rx_ok_bytes_count);
            LE_PRINT_VALUE_IF(wdsPktStatsResp.last_call_tx_ok_bytes_count_valid,
                              "%"PRIu64, wdsPktStatsResp.last_call_tx_ok_bytes_count);
        }
        if (wdsPktStatsResp.last_call_rx_ok_bytes_count_valid)
        {
            dataStatisticsPtr->receivedBytesCount = wdsPktStatsResp.last_call_rx_ok_bytes_count;
        }
        if (wdsPktStatsResp.last_call_tx_ok_bytes_count_valid)
        {
            dataStatisticsPtr->transmittedBytesCount = wdsPktStatsResp.last_call_tx_ok_bytes_count;
        }
        *sessionInProgressPtr = false;
        return LE_OK;
    }

    return LE_FAULT;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the IP family of the QMI client.
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT for all other errors
 */
//--------------------------------------------------------------------------------------------------
static le_result_t SetIpFamily
(
    qmi_client_type wdsClient,
    wds_ip_family_enum_v01 ipFamily
)
{
    qmi_client_error_type rc;
    wds_set_client_ip_family_pref_req_msg_v01 wdsSetIpFamilyReq = {0};
    wds_set_client_ip_family_pref_resp_msg_v01 wdsSetIpFamilyResp = { {0} };

    wdsSetIpFamilyReq.ip_preference = ipFamily;

    rc = qmi_client_send_msg_sync(wdsClient,
                QMI_WDS_SET_CLIENT_IP_FAMILY_PREF_REQ_V01,
                &wdsSetIpFamilyReq, sizeof(wdsSetIpFamilyReq),
                &wdsSetIpFamilyResp, sizeof(wdsSetIpFamilyResp),
                COMM_LONG_PLATFORM_TIMEOUT);

    le_result_t result = swiQmi_CheckResponseCode(
                                STRINGIZE_EXPAND(QMI_WDS_SET_CLIENT_IP_FAMILY_PREF_REQ_V01),
                                rc, wdsSetIpFamilyResp.resp);

    if (result != LE_OK)
    {
        LE_ERROR("Unable to set IP family to %0X", ipFamily);
        return LE_FAULT;
    }

    return LE_OK;
}

#if defined(SIERRA_BINDMUX)

//--------------------------------------------------------------------------------------------------
/**
 * Bind the MUX data port
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT for all other errors
 */
//--------------------------------------------------------------------------------------------------
static le_result_t BindMux(qmi_client_type  user_handle, const char  *dev_str)
{
    int ep_type = -1;
    int epid = -1;
    int mux_id = -1;
    qmi_connection_id_type conn_id;
    qmi_client_error_type rc;
    wds_bind_mux_data_port_req_msg_v01 req;
    wds_bind_mux_data_port_resp_msg_v01 resp;
    le_result_t result = LE_FAULT;

    conn_id = QMI_PLATFORM_DEV_NAME_TO_CONN_ID_EX(dev_str,
                       &ep_type,
                       &epid,
                       &mux_id);

    /* Get EPID, MUX-ID information */
    if (QMI_CONN_ID_INVALID == conn_id)
    {
        LE_ERROR("Dev to conn_id translation failed");
        return LE_FAULT;
    }

    LE_DEBUG("conn_id %d ep_type %d epid %d mux_id %d", conn_id, ep_type, epid, mux_id);

    // The QMI timer out below is too long, kick watchdog here to avoid watchdog expiry.
    KickWatchdog();

    /* Bind to a epid/mux-id if mux-id-binding is supported */
    if (mux_id > 0)
    {
        memset (&req, 0, sizeof(req));
        memset (&resp, 0, sizeof(resp));

        req.ep_id_valid = (epid != -1);
        req.ep_id.ep_type = (data_ep_type_enum_v01) ep_type;
        req.ep_id.iface_id = (uint32_t) epid;

        req.mux_id_valid = (mux_id != -1);
        req.mux_id = (uint8_t) mux_id;

        /* Send bind_req */
        rc = qmi_client_send_msg_sync(user_handle,
                                      QMI_WDS_BIND_MUX_DATA_PORT_REQ_V01,
                                      (void *)&req,
                                      sizeof(wds_bind_mux_data_port_req_msg_v01),
                                      (void*)&resp,
                                      sizeof(wds_bind_mux_data_port_resp_msg_v01),
                                      90000);

       result = swiQmi_CheckResponseCode(
                                STRINGIZE_EXPAND(QMI_WDS_BIND_MUX_DATA_PORT_REQ_V01),
                                rc, resp.resp);
    }

    return result;
}

#endif

//--------------------------------------------------------------------------------------------------
/**
 * Start a data session with the given profile
 *
 * @return
 *      - LE_OK on success
 *      - LE_DUPLICATE if the data session is already connected
 *      - LE_FAULT for other failures
 */
//--------------------------------------------------------------------------------------------------
static le_result_t StartSession
(
    uint32_t profileIndex,           ///< [IN] The profile to use
    wds_ip_family_enum_v01 ipFamily, ///< [IN] IP Family
    uint32_t* callRefPtr             ///< [OUT] Reference used for stopping the data session
)
{
    qmi_client_error_type rc;

    qmi_client_type wdsClient = GetWdsClient(profileIndex,
                                            WDS_IP_FAMILY_ENUM_2_LE_MDMDEFS_IPVERSION(ipFamily),
                                            true, true);

    if (wdsClient == NULL)
    {
        LE_ERROR("Bad wdsClient");
        return LE_FAULT;
    }

    ProfileCtx_t *profileCtxPtr = GetProfileCtxFromList(profileIndex);
    if(!profileCtxPtr)
    {
        LE_ERROR("bad profileIndex");
        return LE_FAULT;
    }

    profileCtxPtr->callContext.failureCode.callEndFailureCode = 0;
    profileCtxPtr->callContext.failureCode.callEndFailure = LE_MDC_DISC_UNDEFINED;
    profileCtxPtr->callContext.failureCode.callConnectionFailureCode = 0;
    profileCtxPtr->callContext.failureCode.callConnectionFailureType = 0;

#if defined(SIERRA_BINDMUX)
    le_result_t res;

    char rmnetString[LE_MDC_INTERFACE_NAME_MAX_BYTES] = {0};
    snprintf(rmnetString, LE_MDC_INTERFACE_NAME_MAX_BYTES, "%s%d", DATA_INTERFACE_PREFIX,
             GetRmnetInterfaceNumber(profileIndex));

    res = BindMux(wdsClient, rmnetString);
    if (res != LE_OK)
    {
        LE_ERROR("Error in binding mux port");
        return LE_FAULT;
    }
#endif

    LE_DEBUG("Start session profile %d IP %d wdsClient %p", profileIndex, ipFamily, wdsClient);

    wds_start_network_interface_req_msg_v01 wdsStartReq = {0};
    SWIQMI_DECLARE_MESSAGE(wds_start_network_interface_resp_msg_v01, wdsStartResp);

    if ( SetIpFamily(wdsClient, ipFamily) != LE_OK )
    {
        UnassignedWdsClient(profileIndex, WDS_IP_FAMILY_ENUM_2_LE_MDMDEFS_IPVERSION(ipFamily));
        mdc_qmi_ReleaseWdsClient(wdsClient);
        return LE_FAULT;
    }

    wdsStartReq.ip_family_preference_valid = true;
    wdsStartReq.ip_family_preference = (wds_ip_family_preference_enum_v01)ipFamily;

    wdsStartReq.technology_preference_valid = true;

    /* 3gpp2 profile */
    if ( Is3Gpp2Profile ( profileIndex ) )
    {
        wdsStartReq.technology_preference = 2;
        wdsStartReq.profile_index_3gpp2_valid = true;
        wdsStartReq.profile_index_3gpp2 = profileIndex;
    }
    else
    {
        wdsStartReq.technology_preference = 1;
        wdsStartReq.profile_index_valid = true;
        wdsStartReq.profile_index = profileIndex;
    }

    wdsStartReq.call_type_valid = true;
    wdsStartReq.call_type = WDS_CALL_TYPE_EMBEDDED_CALL_V01;

    IsStartSession = true;

    LE_DEBUG("Starting session for IPv%d profileIndex[%u]", ipFamily, profileIndex);

    // The QMI timer out below is too long, Kick watchdog here to avoid watchdog expiry.
    KickWatchdog();

    // Init the semaphore to synchronize StartSession and StopSession functions
    SessionSemRef = le_sem_Create("SessionSem", 0);

    // Following the 3GPP specification when the network doesn't respond to the connection request
    // message the firmware retries after T3380 expires (30s) for up to 5 attempts total.
    // Ensure the timeout is really long, such as 150s, because there is no easy way to recover if
    // a timeout occurs.
    // todo: Consider using the QMI async functions to allow more control, although there are
    //       potential race conditions with that solution.
    rc = qmi_client_send_msg_sync(wdsClient,
            QMI_WDS_START_NETWORK_INTERFACE_REQ_V01,
            &wdsStartReq, sizeof(wdsStartReq),
            &wdsStartResp, sizeof(wdsStartResp),
            COMM_NETWORK_TIMEOUT * 5);

    le_result_t result = swiQmi_CheckResponseCode(
                                STRINGIZE_EXPAND(QMI_WDS_START_NETWORK_INTERFACE_REQ_V01),
                                rc, wdsStartResp.resp);

    if (result == LE_OK)
    {
        le_clk_Time_t timeOut;

        timeOut.sec = (SESSION_TIMEOUT_MS/1000);
        timeOut.usec = (SESSION_TIMEOUT_MS%1000)*1000;

        // Make the function synchronous
        if (le_sem_WaitWithTimeOut(SessionSemRef, timeOut) == LE_TIMEOUT)
        {
            le_sem_Delete(SessionSemRef);
            UnassignedWdsClient(profileIndex
                    , WDS_IP_FAMILY_ENUM_2_LE_MDMDEFS_IPVERSION(ipFamily));
            mdc_qmi_ReleaseWdsClient(wdsClient);
            IsStartSession = false;
            return LE_FAULT;
        }

        // Return the call reference; to be used when stopping the data session
        *callRefPtr = wdsStartResp.pkt_data_handle;
        if ( IS_TRACE_ENABLED )
        {
            LE_PRINT_VALUE("%X", *callRefPtr);
        }
    }
    else
    {
        // If the data session is already started, the LE_DUPLICATE error is returned
        // and the WDS client is not unassigned.
        if ((wdsStartResp.resp.result!=QMI_NO_ERR)
             &&(wdsStartResp.resp.error == QMI_ERR_NO_EFFECT_V01))
        {
            result = LE_DUPLICATE;
        }
        else
        {
            UnassignedWdsClient(profileIndex
                                , WDS_IP_FAMILY_ENUM_2_LE_MDMDEFS_IPVERSION(ipFamily));
            mdc_qmi_ReleaseWdsClient(wdsClient);
            result = LE_FAULT;
        }

        // Get the specific QMi error code if data session failed.
        if ((wdsStartResp.resp.result == QMI_RESULT_FAILURE_V01)
             && (wdsStartResp.resp.error == QMI_ERR_CALL_FAILED_V01))
        {
            if (wdsStartResp.call_end_reason_valid)
            {
                profileCtxPtr->callContext.failureCode.callEndFailureCode =
                                wdsStartResp.call_end_reason;
                profileCtxPtr->callContext.failureCode.callEndFailure =
                                TranslateDisconnectionReason(wdsStartResp.call_end_reason);
                LE_ERROR("Data connection failure Call End provided %d, Code %d",
                                profileCtxPtr->callContext.failureCode.callEndFailure,
                                profileCtxPtr->callContext.failureCode.callEndFailureCode);
            }

            if (wdsStartResp.verbose_call_end_reason_valid)
            {
                profileCtxPtr->callContext.failureCode.callConnectionFailureType =
                                wdsStartResp.verbose_call_end_reason.call_end_reason_type;
                profileCtxPtr->callContext.failureCode.callConnectionFailureCode =
                                wdsStartResp.verbose_call_end_reason.call_end_reason;
                LE_ERROR("Data connection failure Verbose Call End provided Type %d, Verbose %d",
                    profileCtxPtr->callContext.failureCode.callConnectionFailureType,
                    profileCtxPtr->callContext.failureCode.callConnectionFailureCode );
            }

        }
        else
        {
            LE_WARN("Data connection failure reason not available");
        }
    }

    le_sem_Delete(SessionSemRef);

    IsStartSession = false;
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Stop a data session with the given session reference
 *
 * @return
 *      - LE_OK on success
 *      - LE_BAD_PARAMETER if the input parameter is not valid
 *      - LE_FAULT for other failures
 */
//--------------------------------------------------------------------------------------------------
static le_result_t StopSession
(
    uint32_t callRef,               ///< [IN] The data connection reference
    qmi_client_type wdsClient,      ///< [IN] QMI client to be used to close the session
    wds_ip_family_enum_v01 ipFamily ///< [IN] IP Family to stop.
)
{
    qmi_client_error_type rc;

    wds_stop_network_interface_req_msg_v01 wdsStopReq = {0};
    wds_stop_network_interface_resp_msg_v01 wdsStopResp = { {0} };

    LE_DEBUG("Stopping session for IPv%d (callRef %X) wdsClient %p", ipFamily, callRef, wdsClient);

    // Note that the profile index should not be set to an invalid value here.  Instead, it is
    // reset in QMINewSessionStateHandler(), since the index is still needed to generate the
    // session state event for the disconnect.

    wdsStopReq.pkt_data_handle = callRef;

    IsStopSession = true;

    // Init the semaphore to synchronize StartSession and StopSession functions
    SessionSemRef = le_sem_Create("SessionSem", 0);

    rc = qmi_client_send_msg_sync(wdsClient,
            QMI_WDS_STOP_NETWORK_INTERFACE_REQ_V01,
            &wdsStopReq, sizeof(wdsStopReq),
            &wdsStopResp, sizeof(wdsStopResp),
            COMM_NETWORK_TIMEOUT * 2);


    le_result_t result = swiQmi_CheckResponseCode(
                                STRINGIZE_EXPAND(QMI_WDS_STOP_NETWORK_INTERFACE_REQ_V01),
                                rc, wdsStopResp.resp);

    if(result == LE_OK)
    {
        le_clk_Time_t timeOut;

        timeOut.sec = (SESSION_TIMEOUT_MS/1000);
        timeOut.usec = (SESSION_TIMEOUT_MS%1000)*1000;

        // Make the function synchronous
        if (le_sem_WaitWithTimeOut(SessionSemRef, timeOut) == LE_TIMEOUT)
        {
            LE_ERROR("Semaphore timeout");
            IsStartSession = false;
            result = LE_FAULT;
        }
    }

    LE_DEBUG("sem = %d", le_sem_GetValue(SessionSemRef));

    le_sem_Delete(SessionSemRef);
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Save data statistics.
 *
 */
//--------------------------------------------------------------------------------------------------
static void SaveDataFlowStatistics
(
    qmi_client_type wdsClient
)
{
    bool sessionInProgress;
    pa_mdc_PktStatistics_t currentDataStatistics = {0};

    if (!wdsClient)
    {
        return;
    }

    // Check if the data statistics saving is enabled
    if (!DataStatistics.saving)
    {
        LE_DEBUG("Not saving data statistics");
        return;
    }

    if (LE_OK == GetDataFlowStatistics(wdsClient, &sessionInProgress, &currentDataStatistics))
    {
        DataStatistics.saved.receivedBytesCount += currentDataStatistics.receivedBytesCount;
        DataStatistics.saved.transmittedBytesCount += currentDataStatistics.transmittedBytesCount;
    }
    else
    {
        LE_WARN("Could not save Data flow statistics when closing the session");
    }

    DumpDataFlowStatistics();
}

//--------------------------------------------------------------------------------------------------
/**
 * Get data connection status for an IP family .
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT for other failures
 */
//--------------------------------------------------------------------------------------------------
static le_result_t GetSessionStateForIpVersion
(
    uint32_t profileIndex,                  ///< [IN] The profile to use
    wds_ip_family_enum_v01 ipFamily,        ///< [IN] IP Family
    le_mdc_ConState_t* sessionStatePtr      ///< [OUT] The data session state
)
{
    ProfileCtx_t* profileCtxPtr = GetProfileCtxFromList(profileIndex);
    CallIpContext_t* ipCtxPtr=NULL;
    qmi_client_type wdsClient=NULL;
    le_result_t result = LE_OK;

    if (profileCtxPtr)
    {
        if (ipFamily == WDS_IP_FAMILY_IPV4_V01)
        {
            ipCtxPtr = &profileCtxPtr->callContext.ipv4;
        }
        else
        {
            ipCtxPtr = &profileCtxPtr->callContext.ipv6;
        }
    }

    if (ipCtxPtr && (ipCtxPtr->state != WDS_STATE_UNDEF))
    {
        *sessionStatePtr = ipCtxPtr->state;
        LE_DEBUG("cid %d session state %d", profileIndex, *sessionStatePtr);
    }
    else
    {
        // There is no request message, only a response message.
        wds_get_runtime_settings_resp_msg_v01 wdsSettingsResp = { {0} };

        wdsClient = GetWdsClient(profileIndex,
                                 WDS_IP_FAMILY_ENUM_2_LE_MDMDEFS_IPVERSION(ipFamily),
                                 false,
                                 false);

        if (wdsClient == NULL)
        {
            // No connected
            *sessionStatePtr = LE_MDC_DISCONNECTED;
            return LE_OK;
        }

        result = GetRuntimeSettings(profileIndex,
                                    WDS_IP_FAMILY_ENUM_2_LE_MDMDEFS_IPVERSION(ipFamily),
                                    QMI_WDS_MASK_REQ_SETTINGS_IP_FAMILY_V01,
                                    &wdsSettingsResp);

        if ( result != LE_OK )
        {
            if ( wdsSettingsResp.resp.error == QMI_ERR_OUT_OF_CALL_V01 )
            {
                LE_DEBUG("0x%X QMI error is not considered as an error case,"
                         "it indicates a disconnected state.",
                         QMI_ERR_OUT_OF_CALL_V01);
                // Not connected
                *sessionStatePtr = LE_MDC_DISCONNECTED;
                result = LE_OK;
            }
        }
        else
        {
            // ip_family should be valid
            if (wdsSettingsResp.ip_family_valid == true)
            {
                if (wdsSettingsResp.ip_family == ipFamily)
                {
                    *sessionStatePtr = LE_MDC_CONNECTED;
                }
                else
                {
                    *sessionStatePtr = LE_MDC_DISCONNECTED;
                }
            }
            else
            {
                result = LE_FAULT;
            }
        }
    }

    LE_DEBUG("IP%d session state %d", ipFamily, *sessionStatePtr);

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * The first-layer New Session State Change Handler.
 */
//--------------------------------------------------------------------------------------------------
static void FirstLayerSessionStateHandler
(
    void*   reportPtr,
    void*   secondLayerFunc
)
{
    pa_mdc_SessionStateData_t* sessionStatePtr = (pa_mdc_SessionStateData_t*) reportPtr;
    pa_mdc_SessionStateHandler_t handlerFunc = (pa_mdc_SessionStateHandler_t) secondLayerFunc;

    handlerFunc(sessionStatePtr);
}


// =============================================
//  MODULE/COMPONENT FUNCTIONS
// =============================================


//--------------------------------------------------------------------------------------------------
/**
 * Initialize the QMI MDC module.
 *
 * @return
 *      LE_OK if successful.
 *      LE_FAULT if unsuccessful.
 */
//--------------------------------------------------------------------------------------------------
le_result_t mdc_qmi_Init(void)
{
    // Get a reference to the trace keyword that is used to control tracing in this module.
    TraceRef = le_log_GetTraceRef("mdc");

    // Save the PA MDC main thread reference
    CurrentThreadRef = le_thread_GetCurrent();

    // Mutex for accessing WdsClientInfo.
    WdsMutexRef = le_mutex_CreateNonRecursive("WdsMutexRef");

    memset(SwiQmiParam,0,LE_MDMDEFS_IPMAX*sizeof(swiQmi_Param_t));
    memset(WdsClientInfo, 0, sizeof(WdsClientInfo_t) * MAX_WDS_CLIENTS * LE_MDMDEFS_IPMAX);

    NewSessionStateEvent = le_event_CreateIdWithRefCounting("NewSessionStateEvent");
    NewSessionStatePool = le_mem_CreatePool("NewSessionStatePool",
                                            sizeof(pa_mdc_SessionStateData_t));

    // Allocate the profile pool, and set the max number of objects, since it is already known.
    ProfileCtxPool = le_mem_CreatePool("ProfileCtxPool", sizeof(ProfileCtx_t));

    ProfileCtxList = LE_DLS_LIST_INIT;

    // Register our own handler with the QMI WDS service.
    swiQmi_AddIndicationHandler(QMINewSessionStateHandler,
                                QMI_SERVICE_WDS, QMI_WDS_PKT_SRVC_STATUS_IND_V01, NULL);

#if defined(SIERRA_MTPDP)
    // Register our own handler with the QMI WDS service.
    swiQmi_AddIndicationHandler(QMImtPdpRingReportHandler,
                                QMI_SERVICE_WDS, QMI_WDS_SWI_SET_MTPDP_RING_REPORT_IND_V01, NULL);

    // Active the MT-PDP ring report notification
    SetMtPdpRingIndicator(true);
#endif

    LE_INFO("Max Profiles %d, Max PDPs %d", PA_MDC_MAX_PROFILE, MAX_WDS_CLIENTS);

    memset(&DataStatistics, 0, sizeof(DataStatistics));
    DataStatistics.saving = true;

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the index of the default profile (link to the platform)
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mdc_GetDefaultProfileIndex
(
    uint32_t* profileIndexPtr   ///< [OUT] index of the profile.
)
{
    le_result_t           res = LE_OK;
    le_mrc_Rat_t          rat;
    qmi_client_error_type rc;
    qmi_client_type       wdsClient = mdc_qmi_GetDefaultWdsClient();

    if (wdsClient == NULL)
    {
        LE_ERROR("Invalid wdsclient");
        return LE_FAULT;
    }

    wds_get_default_profile_num_req_msg_v01 defaultProfileReq;
    wds_get_default_profile_num_resp_msg_v01 defaultProfileRsp;

    memset(&defaultProfileReq, 0, sizeof(wds_get_default_profile_num_req_msg_v01));
    memset(&defaultProfileRsp, 0, sizeof(wds_get_default_profile_num_resp_msg_v01));

    if (pa_mrc_GetRadioAccessTechInUse(&rat) != LE_OK)
    {
        // If unable to get the rat, use GSM by default
        rat = LE_MRC_RAT_GSM;
    }

    if (rat != LE_MRC_RAT_CDMA)
    {
        defaultProfileReq.profile.profile_type = WDS_PROFILE_TYPE_3GPP_V01;
    }
    else
    {
        defaultProfileReq.profile.profile_type = WDS_PROFILE_TYPE_3GPP2_V01;
    }

    defaultProfileReq.profile.profile_family = WDS_PROFILE_FAMILY_EMBEDDED_V01;

    rc = qmi_client_send_msg_sync(wdsClient,
                                  QMI_WDS_GET_DEFAULT_PROFILE_NUM_REQ_V01,
                                  &defaultProfileReq, sizeof(defaultProfileReq),
                                  &defaultProfileRsp, sizeof(defaultProfileRsp),
                                  COMM_LONG_PLATFORM_TIMEOUT);

    res = swiQmi_CheckResponseCode(
                  STRINGIZE_EXPAND(QMI_WDS_GET_DEFAULT_PROFILE_NUM_REQ_V01),
                  rc, defaultProfileRsp.resp);
    if ( res != LE_OK )
    {
        LE_PRINT_VALUE_IF(defaultProfileRsp.extended_error_code_valid,
                          "%X",
                          defaultProfileRsp.extended_error_code);

        res = LE_FAULT;
    }
    else
    {
        LE_DEBUG("Default profile: %d", defaultProfileRsp.profile_index);

        *profileIndexPtr = defaultProfileRsp.profile_index;
    }

    mdc_qmi_ReleaseWdsClient(wdsClient);
    return res;
}

#if defined(QMI_WDS_SWI_GET_DEFAULT_PROFILE_NUM_REQ_V01)

//--------------------------------------------------------------------------------------------------
/**
 * Get the index of the default profile for Bearer Independent Protocol (link to the platform)
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mdc_GetBipDefaultProfileIndex
(
    uint32_t* profileIndexPtr   ///< [OUT] index of the profile.
)
{
    le_result_t           res = LE_FAULT;
    le_mrc_Rat_t          rat;
    qmi_client_error_type rc;
    qmi_client_type       wdsClient = mdc_qmi_GetDefaultWdsClient();

    if (wdsClient == NULL)
    {
        LE_ERROR("Invalid wdsclient");
        return LE_FAULT;
    }

    wds_swi_get_default_profile_num_req_msg_v01 defaultProfileReq;
    wds_swi_get_default_profile_num_resp_msg_v01 defaultProfileRsp;

    memset(&defaultProfileReq, 0, sizeof(wds_swi_get_default_profile_num_req_msg_v01));
    memset(&defaultProfileRsp, 0, sizeof(wds_swi_get_default_profile_num_resp_msg_v01));

    if (pa_mrc_GetRadioAccessTechInUse(&rat) != LE_OK)
    {
        // If unable to get the rat, use GSM by default
        rat = LE_MRC_RAT_GSM;
    }

    if (rat != LE_MRC_RAT_CDMA)
    {
        defaultProfileReq.default_profile_req.profile_type = WDS_PROFILE_TYPE_3GPP_V01;
    }
    else
    {
        defaultProfileReq.default_profile_req.profile_type = WDS_PROFILE_TYPE_3GPP2_V01;
    }

    defaultProfileReq.default_profile_req.profile_family = 0x00;

    rc = qmi_client_send_msg_sync(wdsClient,
                                  QMI_WDS_SWI_GET_DEFAULT_PROFILE_NUM_REQ_V01,
                                  &defaultProfileReq, sizeof(defaultProfileReq),
                                  &defaultProfileRsp, sizeof(defaultProfileRsp),
                                  COMM_LONG_PLATFORM_TIMEOUT);

    res = swiQmi_CheckResponseCode(
                  STRINGIZE_EXPAND(QMI_WDS_SWI_GET_DEFAULT_PROFILE_NUM_REQ_V01),
                  rc, defaultProfileRsp.resp);

    if ( res != LE_OK )
    {
        LE_PRINT_VALUE_IF(defaultProfileRsp.extended_error_code_valid,
                          "%X",
                          defaultProfileRsp.extended_error_code);

        res = LE_FAULT;
    }
    else
    {
        LE_DEBUG("Default profile: %d", defaultProfileRsp.default_profile_resp);

        *profileIndexPtr = defaultProfileRsp.default_profile_resp;
    }

    mdc_qmi_ReleaseWdsClient(wdsClient);
    return res;
}

#endif

//--------------------------------------------------------------------------------------------------
/**
 * Read the profile data for the given profile
 *
 * @return
 *      - LE_OK on success
 *      - LE_OVERFLOW if a string (apn name, user, password) is too long to fit in profile data
 *      - LE_UNSUPPORTED if authentication type not supported
 *      - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mdc_ReadProfile
(
    uint32_t profileIndex,                  ///< [IN] The profile to read
    pa_mdc_ProfileData_t* profileDataPtr    ///< [OUT] The profile data
)
{
    qmi_client_error_type rc;

    wds_get_profile_settings_req_msg_v01 wdsReadReq = { {0} };
    wds_get_profile_settings_resp_msg_v01 wdsReadResp = { {0} };
    qmi_client_type wdsClient = mdc_qmi_GetDefaultWdsClient();

    if (wdsClient == NULL)
    {
        LE_ERROR("Invalid wdsclient");
        return LE_FAULT;
    }

    if ( Is3Gpp2Profile ( profileIndex ) )
    {
        wdsReadReq.profile.profile_type = WDS_PROFILE_TYPE_3GPP2_V01;
    }
    else
    {
        wdsReadReq.profile.profile_type = WDS_PROFILE_TYPE_3GPP_V01;
    }

    LE_DEBUG("Read profile %d type %d", profileIndex, wdsReadReq.profile.profile_type);

    wdsReadReq.profile.profile_index = profileIndex;

    rc = qmi_client_send_msg_sync(wdsClient,
            QMI_WDS_GET_PROFILE_SETTINGS_REQ_V01,
            &wdsReadReq, sizeof(wdsReadReq),
            &wdsReadResp, sizeof(wdsReadResp),
            COMM_LONG_PLATFORM_TIMEOUT);

    le_result_t result = swiQmi_CheckResponseCode(
                                STRINGIZE_EXPAND(QMI_WDS_GET_PROFILE_SETTINGS_REQ_V01),
                                rc, wdsReadResp.resp);
    if ( result != LE_OK )
    {
        LE_PRINT_VALUE_IF(wdsReadResp.extended_error_code_valid,
                          "%X", wdsReadResp.extended_error_code);

        if ( (wdsReadResp.extended_error_code_valid) &&
             (wdsReadResp.extended_error_code ==
             WDS_EEC_DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_NUM_V01) )
        {
            result = LE_NOT_FOUND;
            goto end;
        }
        else
        {
            goto end;
        }
    }

    LE_PRINT_VALUE_IF(wdsReadResp.profile_name_valid, "%s", wdsReadResp.profile_name);
    LE_PRINT_VALUE_IF(wdsReadResp.pdp_type_valid, "%i", wdsReadResp.pdp_type);
    LE_PRINT_VALUE_IF(wdsReadResp.apn_name_valid, ">>%s<<", wdsReadResp.apn_name);
    LE_PRINT_VALUE_IF(wdsReadResp.authentication_preference_valid, "%i",
                      wdsReadResp.authentication_preference);
    LE_PRINT_VALUE_IF(wdsReadResp.username_valid, "%s", wdsReadResp.username);
    LE_PRINT_VALUE_IF(wdsReadResp.password_valid, "%s", wdsReadResp.password);

    // 3GPP profile only
    if ( !Is3Gpp2Profile ( profileIndex ) )
    {
        if ( wdsReadResp.apn_name_valid )
        {
            if ( le_utf8_Copy(profileDataPtr->apn,
                            wdsReadResp.apn_name,
                            sizeof(profileDataPtr->apn), NULL) == LE_OVERFLOW )
           {
               LE_ERROR("APN '%s' is too long", wdsReadResp.apn_name);
               result = LE_OVERFLOW;
               goto end;
           }
        }

        if ( wdsReadResp.pdp_type_valid )
        {
            switch (wdsReadResp.pdp_type)
            {
                case WDS_PDP_TYPE_PDP_IPV4_V01:
                {
                    profileDataPtr->pdp = LE_MDC_PDP_IPV4;
                }
                break;
                case WDS_PDP_TYPE_PDP_IPV6_V01:
                {
                    profileDataPtr->pdp = LE_MDC_PDP_IPV6;
                }
                break;
                case WDS_PDP_TYPE_PDP_IPV4V6_V01:
                {
                    profileDataPtr->pdp = LE_MDC_PDP_IPV4V6;
                }
                break;
                default:
                {
                    profileDataPtr->pdp = LE_MDC_PDP_UNKNOWN;
                }
                break;
            }
        }
        else
        {
            profileDataPtr->pdp = LE_MDC_PDP_UNKNOWN;
        }

        profileDataPtr->authentication.type = 0;
        if ( wdsReadResp.authentication_preference_valid )
        {
            if ( wdsReadResp.authentication_preference & QMI_WDS_MASK_AUTH_PREF_PAP_V01 )
            {
                profileDataPtr->authentication.type = LE_MDC_AUTH_PAP;
            }

            if ( wdsReadResp.authentication_preference & QMI_WDS_MASK_AUTH_PREF_CHAP_V01 )
            {
                profileDataPtr->authentication.type |= LE_MDC_AUTH_CHAP;
            }
        }
        if ( !profileDataPtr->authentication.type )
        {
            profileDataPtr->authentication.type = LE_MDC_AUTH_NONE;
        }

        if ( wdsReadResp.username_valid )
        {
            if ( le_utf8_Copy(profileDataPtr->authentication.userName,
                              wdsReadResp.username,
                              sizeof(profileDataPtr->authentication.userName),
                              NULL) == LE_OVERFLOW )
            {
                LE_ERROR("UserName '%s' is too long", wdsReadResp.username);
                result = LE_OVERFLOW;
                goto end;
            }
        }

        if ( wdsReadResp.password_valid )
        {
            if ( le_utf8_Copy(profileDataPtr->authentication.password,
                              wdsReadResp.password,
                              sizeof(profileDataPtr->authentication.password),
                              NULL) == LE_OVERFLOW )
            {
                LE_ERROR("Password '%s' is too long", wdsReadResp.password);
                result = LE_OVERFLOW;
                goto end;
            }
        }
    }
    // 3GPP2 profile only
    else
    {
        if ( wdsReadResp.apn_string_valid )
        {
            if ( le_utf8_Copy(profileDataPtr->apn,
                            wdsReadResp.apn_string,
                            sizeof(profileDataPtr->apn), NULL) == LE_OVERFLOW )
           {
               LE_ERROR("APN '%s' is too long", wdsReadResp.apn_string);
               result = LE_OVERFLOW;
               goto end;
           }
        }

        if ( wdsReadResp.pdn_type_valid )
        {
            switch (wdsReadResp.pdn_type)
            {
                case WDS_PROFILE_PDN_TYPE_IPV4_V01:
                {
                    profileDataPtr->pdp = LE_MDC_PDP_IPV4;
                }
                break;
                case WDS_PROFILE_PDN_TYPE_IPV6_V01:
                {
                    profileDataPtr->pdp = LE_MDC_PDP_IPV6;
                }
                break;
                case WDS_PROFILE_PDN_TYPE_IPV4_IPV6_V01:
                {
                    profileDataPtr->pdp = LE_MDC_PDP_IPV4V6;
                }
                break;
                default:
                {
                    profileDataPtr->pdp = LE_MDC_PDP_UNKNOWN;
                }
                break;
            }
        }
        else
        {
            profileDataPtr->pdp = LE_MDC_PDP_UNKNOWN;
        }

        profileDataPtr->authentication.type = 0;
        if ( wdsReadResp.pdn_level_auth_protocol_valid )
        {
            if (wdsReadResp.pdn_level_auth_protocol & WDS_PROFILE_PDN_LEVEL_AUTH_PROTOCOL_PAP_V01)
            {
                profileDataPtr->authentication.type = LE_MDC_AUTH_PAP;
            }

            if (wdsReadResp.pdn_level_auth_protocol & WDS_PROFILE_PDN_LEVEL_AUTH_PROTOCOL_CHAP_V01)
            {
                profileDataPtr->authentication.type |= LE_MDC_AUTH_CHAP;
            }
        }
        if ( !profileDataPtr->authentication.type )
        {
            profileDataPtr->authentication.type = LE_MDC_AUTH_NONE;
        }
    }


end:
    mdc_qmi_ReleaseWdsClient(wdsClient);
    return result;
}


// =============================================
//  PUBLIC API FUNCTIONS
// =============================================

//--------------------------------------------------------------------------------------------------
/**
 * Get the default QMI WDS client handle
 *
 * @return
 *      The QMI WDS client handle
 */
//--------------------------------------------------------------------------------------------------
qmi_client_type mdc_qmi_GetDefaultWdsClient
(
    void
)
{
    le_mdmDefs_IpVersion_t ipType;
    int index;

    le_mutex_Lock(WdsMutexRef);
    // By default, use a profile currently in use
    for (index=0; index < MAX_WDS_CLIENTS; index++)
    {
        for (ipType = 0; ipType < LE_MDMDEFS_IPMAX; ipType++)
        {
            if (WdsClientInfo[index][ipType].wdsClientInfo.serviceHandle)
            {
                LE_DEBUG("Found a WdsClient %p",
                         WdsClientInfo[index][ipType].wdsClientInfo.serviceHandle);
                le_mutex_Unlock(WdsMutexRef);
                return WdsClientInfo[index][ipType].wdsClientInfo.serviceHandle;
            }
        }
    }
    le_mutex_Unlock(WdsMutexRef);

    // wdsclient not find, use a default one
    return GetWdsClient(1, LE_MDMDEFS_IPV4, true, false);
}

//--------------------------------------------------------------------------------------------------
/**
 * Release a wdsClient
 *
 */
//--------------------------------------------------------------------------------------------------
void mdc_qmi_ReleaseWdsClient
(
    qmi_client_type wdsClient
)
{
    bool removeWdsClient = true;

    le_mutex_Lock(WdsMutexRef);

    le_dls_Link_t* profileCtxLinkPtr = le_dls_Peek(&ProfileCtxList);

    // Check if wdsClient is in used by a profile
    while(profileCtxLinkPtr)
    {
        ProfileCtx_t* profileCtxPtr = CONTAINER_OF(profileCtxLinkPtr, ProfileCtx_t, link);

        le_mdmDefs_IpVersion_t ip = 0;

        for (; ip < LE_MDMDEFS_IPMAX; ip++)
        {
            if ( profileCtxPtr && profileCtxPtr->clientInfoPtr[ip] &&
                 (profileCtxPtr->clientInfoPtr[ip]->wdsClientInfo.serviceHandle == wdsClient) )
            {
                LE_DEBUG("WDSClient in use");
                removeWdsClient = false;
            }
        }

        profileCtxLinkPtr = le_dls_PeekNext(&ProfileCtxList,profileCtxLinkPtr);
    }

    if (removeWdsClient)
    {
        le_mdmDefs_IpVersion_t ipType;
        int index;

        for (index=0; index < MAX_WDS_CLIENTS; index++)
        {
            for (ipType = 0; ipType < LE_MDMDEFS_IPMAX; ipType++)
            {
                if (WdsClientInfo[index][ipType].wdsClientInfo.serviceHandle == wdsClient)
                {
                    LE_DEBUG("Remove WdsClient [%d][%d] %p", index, ipType, wdsClient);

                    swiQmi_ReleaseService(wdsClient);

                    SwiQmiParam[ipType].usedInfoIndex &=
                                    ~(1<<WdsClientInfo[index][ipType].rmnetNumber);
                    memset(&WdsClientInfo[index][ipType], 0, sizeof(WdsClientInfo_t));
                }
            }
        }
    }

    le_mutex_Unlock(WdsMutexRef);
}

//--------------------------------------------------------------------------------------------------
/**
 * Check whether the profile already exists on the modem ; if not, ask to the modem to create a new
 * profile.
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mdc_InitializeProfile
(
    uint32_t   profileIndex     ///< [IN] The profile to write
)
{
    if ( IsProfileDefined(profileIndex) )
    {
        LE_DEBUG("Profile %i is defined", profileIndex);
    }
    else
    {
        if ( CreateEmptyProfile(profileIndex) != LE_OK )
        {
            LE_ERROR("Error creating empty profile %i", profileIndex);
            return LE_FAULT;
        }
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Write the profile data for the given profile
 *
 * @return
 *      - LE_OK on success
 *      - LE_OVERFLOW if a string (apn name, user, password) is too long to fit in profile data
 *      - LE_UNSUPPORTED if authentication type not supported
 *      - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mdc_WriteProfile
(
    uint32_t profileIndex,                  ///< [IN] The profile to write
    pa_mdc_ProfileData_t* profileDataPtr    ///< [IN] The profile data
)
{
    qmi_client_error_type rc;

    wds_modify_profile_settings_req_msg_v01 wdsWriteReq = { {0} };
    wds_modify_profile_settings_resp_msg_v01 wdsWriteResp = { {0} };

    le_result_t res = LE_OK;

    pa_mdc_InitializeProfile(profileIndex);

    qmi_client_type wdsClient = mdc_qmi_GetDefaultWdsClient();

    if (wdsClient == NULL)
    {
        LE_ERROR("Invalid wdsclient");
        return LE_FAULT;
    }

    if ( Is3Gpp2Profile ( profileIndex ) )
    {
        // 3GPP2 Profile

        wdsWriteReq.profile.profile_type = WDS_PROFILE_TYPE_3GPP2_V01;

        if ( le_utf8_Copy(wdsWriteReq.apn_string,
                        profileDataPtr->apn, sizeof(wdsWriteReq.apn_string), NULL) == LE_OVERFLOW )
        {
            LE_ERROR("APN '%s' is too long", profileDataPtr->apn);
            res = LE_OVERFLOW;
            goto end;
        }
        wdsWriteReq.apn_string_valid = true;

        switch (profileDataPtr->pdp)
        {
            case LE_MDC_PDP_IPV4:
            {
                wdsWriteReq.pdn_type = WDS_PROFILE_PDN_TYPE_IPV4_V01;
            }
            break;
            case LE_MDC_PDP_IPV6:
            {
                wdsWriteReq.pdn_type = WDS_PROFILE_PDN_TYPE_IPV6_V01;
            }
            break;
            case LE_MDC_PDP_IPV4V6:
            {
                wdsWriteReq.pdn_type = WDS_PROFILE_PDN_TYPE_IPV4_IPV6_V01;
            }
            break;
            // Default is set to IPV4
            default:
            {
                wdsWriteReq.pdn_type = WDS_PROFILE_PDN_TYPE_IPV4_V01;
            }
            break;
        }
        wdsWriteReq.pdn_type_valid = true;

        wdsWriteReq.pdn_level_auth_protocol = 0;
        if (profileDataPtr->authentication.type & LE_MDC_AUTH_PAP )
        {
            wdsWriteReq.pdn_level_auth_protocol = WDS_PROFILE_PDN_LEVEL_AUTH_PROTOCOL_PAP_V01;
        }

        if ( profileDataPtr->authentication.type & LE_MDC_AUTH_CHAP )
        {
            wdsWriteReq.pdn_level_auth_protocol |= WDS_PROFILE_PDN_LEVEL_AUTH_PROTOCOL_CHAP_V01;
        }

        wdsWriteReq.pdn_level_auth_protocol_valid = true;
    }
    else
    {
        // 3GPP Profile

        wdsWriteReq.profile.profile_type = WDS_PROFILE_TYPE_3GPP_V01;

        if ( le_utf8_Copy(wdsWriteReq.apn_name,
                        profileDataPtr->apn, sizeof(wdsWriteReq.apn_name), NULL) == LE_OVERFLOW )
        {
            LE_ERROR("APN '%s' is too long", profileDataPtr->apn);
            res = LE_OVERFLOW;
            goto end;
        }
        wdsWriteReq.apn_name_valid = true;

        switch (profileDataPtr->pdp)
        {
            case LE_MDC_PDP_IPV4:
            {
                wdsWriteReq.pdp_type = WDS_PDP_TYPE_PDP_IPV4_V01;
            }
            break;
            case LE_MDC_PDP_IPV6:
            {
                wdsWriteReq.pdp_type = WDS_PDP_TYPE_PDP_IPV6_V01;
            }
            break;
            case LE_MDC_PDP_IPV4V6:
            {
                wdsWriteReq.pdp_type = WDS_PDP_TYPE_PDP_IPV4V6_V01;
            }
            break;
            // Default is set to IPV4
            default:
            {
                wdsWriteReq.pdp_type = WDS_PDP_TYPE_PDP_IPV4_V01;
            }
            break;
        }
        wdsWriteReq.pdp_type_valid = true;

        wdsWriteReq.authentication_preference = 0;
        if (profileDataPtr->authentication.type & LE_MDC_AUTH_PAP )
        {
            wdsWriteReq.authentication_preference = QMI_WDS_MASK_AUTH_PREF_PAP_V01;
        }

        if ( profileDataPtr->authentication.type & LE_MDC_AUTH_CHAP )
        {
            wdsWriteReq.authentication_preference |= QMI_WDS_MASK_AUTH_PREF_CHAP_V01;
        }

        wdsWriteReq.authentication_preference_valid = true;

        if ( le_utf8_Copy(wdsWriteReq.username,
                profileDataPtr->authentication.userName,
                sizeof(wdsWriteReq.username),
                NULL) == LE_OVERFLOW )
        {
            LE_ERROR("UserName '%s' is too long", profileDataPtr->authentication.userName);
            res = LE_OVERFLOW;
            goto end;
        }
        wdsWriteReq.username_valid = true;

        if ( le_utf8_Copy(wdsWriteReq.password,
                    profileDataPtr->authentication.password,
                    sizeof(wdsWriteReq.password),
                    NULL) == LE_OVERFLOW )
        {
            LE_ERROR("Password '%s' is too long", profileDataPtr->authentication.password);
            res = LE_OVERFLOW;
            goto end;
        }
        wdsWriteReq.password_valid = true;
    }
    wdsWriteReq.profile.profile_index = profileIndex;

    rc = qmi_client_send_msg_sync(wdsClient,
            QMI_WDS_MODIFY_PROFILE_SETTINGS_REQ_V01,
            &wdsWriteReq, sizeof(wdsWriteReq),
            &wdsWriteResp, sizeof(wdsWriteResp),
            COMM_LONG_PLATFORM_TIMEOUT);

    res = swiQmi_CheckResponseCode(
                                STRINGIZE_EXPAND(QMI_WDS_MODIFY_PROFILE_SETTINGS_REQ_V01),
                                rc, wdsWriteResp.resp);
    if ( res != LE_OK )
    {
        LE_PRINT_VALUE_IF(wdsWriteResp.extended_error_code_valid,
                          "%X", wdsWriteResp.extended_error_code);
    }

    LE_DEBUG("Profile '%d' saved on modem", profileIndex);

end:
    mdc_qmi_ReleaseWdsClient(wdsClient);
    return res;
}


//--------------------------------------------------------------------------------------------------
/**
 * Start a data session with the given profile using IPV4
 *
 * @return
 *      - LE_OK on success
 *      - LE_DUPLICATE if the data session is already connected
 *      - LE_FAULT for other failures
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mdc_StartSessionIPV4
(
    uint32_t profileIndex        ///< [IN] The profile to use
)
{
    uint32_t ipv4Handle = 0;
    le_result_t result;
    ProfileCtx_t *profileCtxPtr = GetProfileCtxFromList(profileIndex);

    if (!profileCtxPtr)
    {
        LE_ERROR("Bad profile context");
        return LE_FAULT;
    }

    if(profileCtxPtr->callContext.ipv4.enable)
    {
        LE_ERROR("Data session is already connected");
        return LE_DUPLICATE;
    }

    profileCtxPtr->pdpType = LE_MDC_PDP_IPV4;

    result = StartSession(profileIndex, WDS_IP_FAMILY_IPV4_V01, &ipv4Handle);

    if ( result == LE_OK )
    {
        profileCtxPtr->callContext.ipv4.enable = true;
        profileCtxPtr->callContext.ipv4.callRef = ipv4Handle;
        profileCtxPtr->callContext.ipv6.enable = false;
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the connection failure reason
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_mdc_GetConnectionFailureReason
(
    uint32_t profileIndex,                           ///< [IN] The profile to use
    pa_mdc_ConnectionFailureCode_t* failureCodesPtr  ///< [OUT] The specific Failure Reason codes
)
{
    if (failureCodesPtr)
    {
        ProfileCtx_t *profileCtxPtr = GetProfileCtxFromList(profileIndex);
        if (profileCtxPtr)
        {
            memcpy(failureCodesPtr, &profileCtxPtr->callContext.failureCode,
                sizeof(pa_mdc_ConnectionFailureCode_t));
        }
        else
        {
            memset(failureCodesPtr, 0, sizeof(pa_mdc_ConnectionFailureCode_t));
        }
    }
}

//--------------------------------------------------------------------------------------------------
 /**
 * Start a data session with the given profile using IPV6
 *
 * @return
 *      - LE_OK on success
 *      - LE_DUPLICATE if the data session is already connected
 *      - LE_FAULT for other failures
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mdc_StartSessionIPV6
(
    uint32_t profileIndex        ///< [IN] The profile to use
)
{
    uint32_t ipv6Handle = 0;
    le_result_t result;
    ProfileCtx_t *profileCtxPtr = GetProfileCtxFromList(profileIndex);

    if (!profileCtxPtr)
    {
        return LE_FAULT;
    }

    if(profileCtxPtr->callContext.ipv6.enable)
    {
        LE_ERROR("Data session is already connected");
        return LE_DUPLICATE;
    }

    profileCtxPtr->pdpType = LE_MDC_PDP_IPV6;

    result = StartSession(profileIndex, WDS_IP_FAMILY_IPV6_V01, &ipv6Handle);

    if ( result == LE_OK )
    {
        profileCtxPtr->callContext.ipv4.enable = false;
        profileCtxPtr->callContext.ipv6.enable = true;
        profileCtxPtr->callContext.ipv6.callRef = ipv6Handle;
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Start a data session with the given profile using IPV4-V6
 *
 * @return
 *      - LE_OK on success
 *      - LE_DUPLICATE if the data session is already connected
 *      - LE_FAULT for other failures
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mdc_StartSessionIPV4V6
(
    uint32_t profileIndex        ///< [IN] The profile to use
)
{
    uint32_t ipHandle;
    le_result_t res;
    ProfileCtx_t *profileCtxPtr = GetProfileCtxFromList(profileIndex);

    if (!profileCtxPtr)
    {
        return LE_FAULT;
    }

    if(profileCtxPtr->callContext.ipv4.enable || profileCtxPtr->callContext.ipv6.enable)
    {
        LE_ERROR("Data session is already connected");
        return LE_DUPLICATE;
    }

    profileCtxPtr->pdpType = LE_MDC_PDP_IPV4V6;

    res = StartSession(profileIndex, WDS_IP_FAMILY_IPV4_V01, &ipHandle);
    if (res != LE_OK)
    {
        LE_DEBUG("Unable to start IPv4 profile");
        profileCtxPtr->callContext.ipv4.enable = false;

        // TODO remove this sleep, it is here to add a delay between both session start
        sleep(2);
    }
    else
    {
        profileCtxPtr->callContext.ipv4.enable = true;
        profileCtxPtr->callContext.ipv4.callRef = ipHandle;
    }

    res = StartSession(profileIndex, WDS_IP_FAMILY_IPV6_V01, &ipHandle);
    if (res != LE_OK)
    {
        LE_DEBUG("Unable to start IPv6 session");
        profileCtxPtr->callContext.ipv6.enable = false;
    }
    else
    {
        profileCtxPtr->callContext.ipv6.enable = true;
        profileCtxPtr->callContext.ipv6.callRef = ipHandle;
    }

    if ( profileCtxPtr->callContext.ipv4.enable || profileCtxPtr->callContext.ipv6.enable )
    {
        // Send LE_MDC_CONNECTED event if at least one session is activated for IPV4V6 PDP type
        pa_mdc_SessionStateData_t* sessionStateDataPtr = le_mem_ForceAlloc(NewSessionStatePool);
        memset(sessionStateDataPtr, 0, sizeof(pa_mdc_SessionStateData_t));

        sessionStateDataPtr->profileIndex = profileIndex;
        sessionStateDataPtr->newState = LE_MDC_CONNECTED;
        sessionStateDataPtr->pdp = LE_MDC_PDP_IPV4V6;

        le_event_ReportWithRefCounting(NewSessionStateEvent, sessionStateDataPtr);

        return LE_OK;
    }

    return LE_FAULT;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get session type for the given profile ( IP V4 or V6 )
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT for other failures
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mdc_GetSessionType
(
    uint32_t profileIndex,              ///< [IN] The profile to use
    pa_mdc_SessionType_t* sessionIpPtr  ///< [OUT] IP family session
)
{
    le_result_t result;
    le_mdc_ConState_t stateIpV4, stateIpV6;

    result = GetSessionStateForIpVersion(profileIndex, WDS_IP_FAMILY_IPV4_V01 , &stateIpV4);
    if(result != LE_OK)
    {
        return result;
    }

    result = GetSessionStateForIpVersion(profileIndex, WDS_IP_FAMILY_IPV6_V01, &stateIpV6);
    if(result != LE_OK)
    {
        return result;
    }

    LE_DEBUG("GetSessionType IP4 %d IP6 %d", stateIpV4, stateIpV6);


    if( (stateIpV4 == LE_MDC_CONNECTED) && (stateIpV6 == LE_MDC_CONNECTED) )
    {
        *sessionIpPtr = PA_MDC_SESSION_IPV4V6;
    }
    else if(stateIpV4 == LE_MDC_CONNECTED)
    {
        *sessionIpPtr = PA_MDC_SESSION_IPV4;
    }
    else if(stateIpV6 == LE_MDC_CONNECTED)
    {
        *sessionIpPtr = PA_MDC_SESSION_IPV6;
    }
    else
    {
        return LE_FAULT;
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Stop a data session for the given profile
 *
 * @return
 *      - LE_OK on success
 *      - LE_BAD_PARAMETER if the input parameter is not valid
 *      - LE_FAULT for other failures
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mdc_StopSession
(
    uint32_t profileIndex   ///< [IN] The profile to use
)
{
    le_result_t result;
    ProfileCtx_t *profileCtxPtr = GetProfileCtxFromList(profileIndex);
    qmi_client_type wdsClient = NULL;

    /* sanity check */
    if ( ( !profileCtxPtr ) ||
         (!(profileCtxPtr->callContext.ipv4.enable) && !(profileCtxPtr->callContext.ipv6.enable)) ||
         ( profileCtxPtr->callContext.ipv4.enable && !(profileCtxPtr->callContext.ipv4.callRef) ) ||
         ( profileCtxPtr->callContext.ipv6.enable && !(profileCtxPtr->callContext.ipv6.callRef) )
       )
    {
        LE_ERROR("Bad input parameter");
        return LE_BAD_PARAMETER;
    }

    /* Case of IPv4 session */
    if (profileCtxPtr->callContext.ipv4.enable)
    {
        wdsClient = GetWdsClient(profileIndex, LE_MDMDEFS_IPV4, false, false);

        if (wdsClient == NULL)
        {
            LE_ERROR("Bad wdsClient");
            return LE_FAULT;
        }

        SaveDataFlowStatistics(wdsClient);

        result = StopSession(profileCtxPtr->callContext.ipv4.callRef,
                             wdsClient,
                             WDS_IP_FAMILY_IPV4_V01);

        if (result != LE_OK)
        {
            LE_ERROR("Unable to stop session for IPv4");
            return LE_FAULT;
        }

        /* reset the ipv4 context: needed for ipv4v6 case if the ipv6 stop session failed */
        profileCtxPtr->callContext.ipv4.callRef = 0;
        profileCtxPtr->callContext.ipv4.enable = false;

        // If the session has become disconnected, unassigned the wds client
        UnassignedWdsClient(profileIndex, LE_MDMDEFS_IPV4);
        mdc_qmi_ReleaseWdsClient(wdsClient);
    }

    /* Case of IPv6 session, or IPv4v6 */
    if (profileCtxPtr->callContext.ipv6.enable)
    {
        wdsClient = GetWdsClient(profileIndex, LE_MDMDEFS_IPV6, false, false);

        if (wdsClient == NULL)
        {
            LE_ERROR("Bad wdsClient");
            return LE_FAULT;
        }

        SaveDataFlowStatistics(wdsClient);

        result = StopSession(profileCtxPtr->callContext.ipv6.callRef,
                             wdsClient,
                             WDS_IP_FAMILY_IPV6_V01);

        if (result != LE_OK)
        {
            LE_ERROR("Unable to stop session for IPv6");
            return LE_FAULT;
        }

        // If the session has become disconnected, unassigned the wds client
        UnassignedWdsClient(profileIndex, LE_MDMDEFS_IPV6);
        mdc_qmi_ReleaseWdsClient(wdsClient);
    }

    profileCtxPtr->callContext.ipv6.callRef = 0;
    profileCtxPtr->callContext.ipv6.enable = false;

    IsStopSession = false;
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Reject a MT-PDP data session for the given profile
 *
 * @return
 *      - LE_OK on success
 *      - LE_UNSUPPORTED if not supported by the target
 *      - LE_FAULT for other failures
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mdc_RejectMtPdpSession
(
    uint32_t profileIndex      ///< [IN] The profile to use
)
{
    le_result_t result = LE_UNSUPPORTED;

#if defined(SIERRA_MTPDP)
    qmi_client_error_type rc;
    // wds_swi_reject_mtpdp_call_req_msg_v01: There is no request message, only a response message.
    wds_swi_reject_mtpdp_call_resp_msg_v01 wdsRejectResp = { {0} };

    LE_DEBUG("Reject MT-PDP request for profile %d wdsClient %p", profileIndex, MtPdpWdsClient);

    rc = qmi_client_send_msg_sync(MtPdpWdsClient,
            QMI_WDS_SWI_REJECT_MTPDP_CALL_REQ_V01,
            NULL, 0,
            &wdsRejectResp, sizeof(wdsRejectResp),
            COMM_NETWORK_TIMEOUT * 2);

    result = swiQmi_CheckResponseCode(
                                STRINGIZE_EXPAND(QMI_WDS_SWI_REJECT_MTPDP_CALL_RESP_V01),
                                rc, wdsRejectResp.resp);
#endif

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the session state for the given profile
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on error
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mdc_GetSessionState
(
    uint32_t profileIndex,                  ///< [IN] The profile to use
    le_mdc_ConState_t* sessionStatePtr      ///< [OUT] The data session state
)
{
    le_result_t res;

    /* Check IPv4 */
    res = GetSessionStateForIpVersion(profileIndex, WDS_IP_FAMILY_IPV4_V01, sessionStatePtr);
    if(res != LE_OK)
    {
        return res;
    }

    if(*sessionStatePtr == LE_MDC_CONNECTED)
    {
        return LE_OK;
    }

    /* Check IPv6 */
    res = GetSessionStateForIpVersion(profileIndex, WDS_IP_FAMILY_IPV6_V01, sessionStatePtr);
    if(res != LE_OK)
    {
        return res;
    }

    return LE_OK;
}


//--------------------------------------------------------------------------------------------------
/**
 * Register a handler for session state notifications.
 *
 * If the handler is NULL, then the previous handler will be removed.
 *
 * @note
 *      The process exits on failure
 */
//--------------------------------------------------------------------------------------------------
le_event_HandlerRef_t pa_mdc_AddSessionStateHandler
(
    pa_mdc_SessionStateHandler_t handlerRef, ///< [IN] The session state handler function.
    void*                        contextPtr  ///< [IN] The context to be given to the handler.
)
{
    // Check if new handler is being added
    if ( handlerRef != NULL )
    {
        le_event_HandlerRef_t sessionStateHandlerRef = le_event_AddLayeredHandler(
                                                                "NewSessionStateHandler",
                                                                NewSessionStateEvent,
                                                                FirstLayerSessionStateHandler,
                                                                (le_event_HandlerFunc_t)handlerRef);

        le_event_SetContextPtr (sessionStateHandlerRef, contextPtr);

        return sessionStateHandlerRef;
    }
    else
    {
        return NULL;
    }
}


//--------------------------------------------------------------------------------------------------
/**
 * Get the name of the network interface for the given profile, if the data session is connected.
 *
 * @return
 *      - LE_OK on success
 *      - LE_OVERFLOW if the interface name would not fit in interfaceNameStr
 *      - LE_FAULT for all other errors
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mdc_GetInterfaceName
(
    uint32_t profileIndex,                  ///< [IN] The profile to use
    char*  interfaceNameStr,                ///< [OUT] The name of the network interface
    size_t interfaceNameStrSize             ///< [IN] The size in bytes of the name buffer
)
{
    char rmnetInterfaceNameStr[LE_MDC_INTERFACE_NAME_MAX_BYTES];

    snprintf(rmnetInterfaceNameStr, LE_MDC_INTERFACE_NAME_MAX_BYTES,
             "%s%d", DATA_INTERFACE_PREFIX, GetRmnetInterfaceNumber(profileIndex));

    le_mdc_ConState_t sessionState;
    le_result_t result;

    result = pa_mdc_GetSessionState(profileIndex, &sessionState);
    if ( (result != LE_OK) || (sessionState != LE_MDC_CONNECTED) )
    {
        return LE_FAULT;
    }

    if (le_utf8_Copy(interfaceNameStr,
                        rmnetInterfaceNameStr, interfaceNameStrSize, NULL) == LE_OVERFLOW )
    {
        LE_ERROR("Interface name '%s' is too long", rmnetInterfaceNameStr);
        return LE_OVERFLOW;
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the IP address for the given profile, if the data session is connected.
 *
 * @return
 *      - LE_OK on success
 *      - LE_OVERFLOW if the IP address would not fit in gatewayAddrStr
 *      - LE_UNSUPPORTED if the IP version is unsupported
 *      - LE_FAULT for all other errors
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mdc_GetIPAddress
(
    uint32_t profileIndex,             ///< [IN] The profile to use
    le_mdmDefs_IpVersion_t ipVersion,  ///< [IN] IP Version
    char*  ipAddrStr,                  ///< [OUT] The IP address in dotted format
    size_t ipAddrStrSize               ///< [IN] The size in bytes of the address buffer
)
{
    le_mdc_ConState_t sessionState;
    le_result_t result;
    wds_get_runtime_settings_resp_msg_v01 wdsSettingsResp;

    result = pa_mdc_GetSessionState(profileIndex, &sessionState);
    if ( (result != LE_OK) || (sessionState != LE_MDC_CONNECTED) )
    {
        return LE_FAULT;
    }

    result = GetRuntimeSettings(profileIndex,
                                ipVersion,
                                QMI_WDS_MASK_REQ_SETTINGS_IP_ADDR_V01,
                                &wdsSettingsResp);

    if ( result != LE_OK )
    {
        return LE_FAULT;
    }

    switch(ipVersion)
    {
        case LE_MDMDEFS_IPV4:
            if ( !wdsSettingsResp.ipv4_address_preference_valid )
            {
                return LE_FAULT;
            }

            result = ConvertIPv4NumToStr(wdsSettingsResp.ipv4_address_preference, ipAddrStr,
                                         ipAddrStrSize);
            if ( result != LE_OK )
            {
                return result;
            }
            LE_DEBUG("%X -> %s", wdsSettingsResp.ipv4_address_preference, ipAddrStr);
            break;

        case LE_MDMDEFS_IPV6:
            if ( !wdsSettingsResp.ipv6_addr_valid )
            {
                return LE_FAULT;
            }

            result = ConvertIPv6NumToStr(wdsSettingsResp.ipv6_addr.ipv6_addr, ipAddrStr,
                                         ipAddrStrSize);
            if ( result != LE_OK )
            {
                return result;
            }
            {
                char ipStr[33];
                le_hex_BinaryToString(wdsSettingsResp.ipv6_addr.ipv6_addr,16,ipStr,33);
                LE_DEBUG("%s -> %s",ipStr,ipAddrStr);
            }
            break;

        default:
        {
            LE_WARN("'%d' is not supported",ipVersion);
            return LE_UNSUPPORTED;
        }
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the gateway IP address for the given profile, if the data session is connected.
 *
 * @return
 *      - LE_OK on success
 *      - LE_OVERFLOW if the IP address would not fit in gatewayAddrStr
 *      - LE_UNSUPPORTED if the IP version is unsupported
 *      - LE_FAULT for all other errors
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mdc_GetGatewayAddress
(
    uint32_t profileIndex,                  ///< [IN] The profile to use
    le_mdmDefs_IpVersion_t ipVersion,       ///< [IN] IP Version
    char*  gatewayAddrStr,                  ///< [OUT] The gateway IP address in dotted format
    size_t gatewayAddrStrSize               ///< [IN] The size in bytes of the address buffer
)
{
    le_mdc_ConState_t sessionState;
    le_result_t result;
    wds_req_settings_mask_v01 mask = 0;
    wds_get_runtime_settings_resp_msg_v01 wdsSettingsResp;

    result = pa_mdc_GetSessionState(profileIndex, &sessionState);
    if ( (result != LE_OK) || (sessionState != LE_MDC_CONNECTED) )
    {
        return LE_FAULT;
    }

    mask |= QMI_WDS_MASK_REQ_SETTINGS_GATEWAY_INFO_V01;
    mask |= QMI_WDS_MASK_REQ_SETTINGS_IP_FAMILY_V01;

    result = GetRuntimeSettings(profileIndex, ipVersion, mask, &wdsSettingsResp);
    if ( result != LE_OK)
    {
        return LE_FAULT;
    }

    if (!wdsSettingsResp.ip_family_valid)
    {
        return LE_FAULT;
    }

    switch (ipVersion)
    {
        case LE_MDMDEFS_IPV4:
        {
            if (!wdsSettingsResp.ipv4_gateway_addr_valid)
            {
                return LE_FAULT;
            }

            result = ConvertIPv4NumToStr(wdsSettingsResp.ipv4_gateway_addr,
                                         gatewayAddrStr,
                                         gatewayAddrStrSize);
            if ( result != LE_OK )
            {
                return result;
            }
            LE_DEBUG("%X -> %s", wdsSettingsResp.ipv4_gateway_addr, gatewayAddrStr);
            break;
        }
        case LE_MDMDEFS_IPV6:
        {
            if (!wdsSettingsResp.ipv6_gateway_addr_valid)
            {
                return LE_FAULT;
            }

            result = ConvertIPv6NumToStr(wdsSettingsResp.ipv6_gateway_addr.ipv6_addr,
                                         gatewayAddrStr,
                                         gatewayAddrStrSize);
            if ( result != LE_OK )
            {
                return result;
            }
            {
                char ipStr[33];
                le_hex_BinaryToString(wdsSettingsResp.ipv6_gateway_addr.ipv6_addr,16,ipStr,33);
                LE_DEBUG("%s -> %s",ipStr,gatewayAddrStr);
            }

            break;
        }
        default:
        {
            LE_WARN("'%d' is not supported",ipVersion);
            return LE_UNSUPPORTED;
        }
    }

    return LE_OK;
}


//--------------------------------------------------------------------------------------------------
/**
 * Get the primary/secondary DNS addresses for the given profile, if the data session is connected.
 *
 * @return
 *      - LE_OK on success
 *      - LE_OVERFLOW if the IP address would not fit in buffer
 *      - LE_UNSUPPORTED if the IP version is unsupported
 *      - LE_FAULT for all other errors
 *
 * @note
 *      If only one DNS address is available, then it will be returned, and an empty string will
 *      be returned for the unavailable address
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mdc_GetDNSAddresses
(
    uint32_t profileIndex,                  ///< [IN] The profile to use
    le_mdmDefs_IpVersion_t ipVersion,       ///< [IN] IP Version
    char*  dns1AddrStr,                     ///< [OUT] The primary DNS IP address in dotted format
    size_t dns1AddrStrSize,                 ///< [IN] The size in bytes of the dns1AddrStr buffer
    char*  dns2AddrStr,                     ///< [OUT] The secondary DNS IP address in dotted format
    size_t dns2AddrStrSize                  ///< [IN] The size in bytes of the dns2AddrStr buffer
)
{
    le_mdc_ConState_t sessionState;
    le_result_t result;
    wds_req_settings_mask_v01 mask = 0;
    wds_get_runtime_settings_resp_msg_v01 wdsSettingsResp;

    result = pa_mdc_GetSessionState(profileIndex, &sessionState);
    if ( (result != LE_OK) || (sessionState != LE_MDC_CONNECTED) )
    {
        return LE_FAULT;
    }

    mask |= QMI_WDS_MASK_REQ_SETTINGS_DNS_ADDR_V01;
    mask |= QMI_WDS_MASK_REQ_SETTINGS_IP_FAMILY_V01;

    result = GetRuntimeSettings(profileIndex, ipVersion, mask, &wdsSettingsResp);
    if (result != LE_OK)
    {
        return LE_FAULT;
    }

    // default to empty string if the value is not present
    dns1AddrStr[0] = '\0';
    dns2AddrStr[0] = '\0';

    if (!wdsSettingsResp.ip_family_valid)
    {
        return LE_FAULT;
    }

    switch (ipVersion)
    {
        case LE_MDMDEFS_IPV4:
        {
            if (wdsSettingsResp.primary_DNS_IPv4_address_preference_valid)
            {
                if (dns1AddrStrSize < INET_ADDRSTRLEN)
                {
                    LE_ERROR("ipv4Str size is %d; must be at least %i", (uint32_t) dns1AddrStrSize,
                             INET_ADDRSTRLEN);
                    return LE_OVERFLOW;
                }
                result = ConvertIPv4NumToStr(wdsSettingsResp.primary_DNS_IPv4_address_preference,
                                             dns1AddrStr,
                                             dns1AddrStrSize);
                if (result != LE_OK)
                {
                    // Could not be converted correctly, so set back to empty string
                    dns1AddrStr[0] = '\0';
                }
            }

            if (wdsSettingsResp.secondary_DNS_IPv4_address_preference_valid)
            {
                 if (dns2AddrStrSize < INET_ADDRSTRLEN)
                 {
                     LE_ERROR("ipv4Str size is %d; must be at least %i", (uint32_t) dns2AddrStrSize,
                              INET_ADDRSTRLEN);
                     return LE_OVERFLOW;
                 }
                 result = ConvertIPv4NumToStr(wdsSettingsResp.secondary_DNS_IPv4_address_preference,
                                             dns2AddrStr,
                                             dns2AddrStrSize);
                if (result != LE_OK)
                {
                    // Could not be converted correctly, so set back to empty string
                    dns2AddrStr[0] = '\0';
                }
            }
            LE_DEBUG("%X -> %s", wdsSettingsResp.primary_DNS_IPv4_address_preference, dns1AddrStr);
            LE_DEBUG("%X -> %s", wdsSettingsResp.secondary_DNS_IPv4_address_preference,
                     dns2AddrStr);
            break;
        }
        case LE_MDMDEFS_IPV6:
        {
            if (wdsSettingsResp.primary_dns_IPv6_address_valid)
            {
                if (dns1AddrStrSize < INET6_ADDRSTRLEN)
                {
                   LE_ERROR("ipv6Str size is %d; must be at least %i", (uint32_t) dns1AddrStrSize,
                            INET6_ADDRSTRLEN);
                   return LE_OVERFLOW;
                }
                result = ConvertIPv6NumToStr(wdsSettingsResp.primary_dns_IPv6_address,
                                             dns1AddrStr,
                                             dns1AddrStrSize);
                if (result != LE_OK)
                {
                    dns1AddrStr[0] = '\0';
                }
            }

            if (wdsSettingsResp.secondary_dns_IPv6_address_valid)
            {
                 if ( dns2AddrStrSize < INET6_ADDRSTRLEN)
                 {
                      LE_ERROR("ipv6Str size is %d; must be at least %i",
                               (uint32_t) dns2AddrStrSize, INET6_ADDRSTRLEN);
                      return LE_OVERFLOW;
                 }

                result = ConvertIPv6NumToStr(wdsSettingsResp.secondary_dns_IPv6_address,
                                             dns2AddrStr,
                                             dns2AddrStrSize);
                if ( result != LE_OK )
                {
                    // Could not be converted correctly, so set back to empty string
                    dns2AddrStr[0] = '\0';
                }
            }
            {
                char dns1IpStr[33];
                char dns2IpStr[33];
                le_hex_BinaryToString(wdsSettingsResp.primary_dns_IPv6_address,16,dns1IpStr,33);
                le_hex_BinaryToString(wdsSettingsResp.secondary_dns_IPv6_address,16,dns2IpStr,33);
                LE_DEBUG("%s -> %s",dns1IpStr, dns1AddrStr);
                LE_DEBUG("%s -> %s",dns2IpStr, dns2AddrStr);
            }
            break;
        }
        default:
        {
            LE_WARN("'%d' is not supported",wdsSettingsResp.ip_family);
            return LE_UNSUPPORTED;
        }
    }
    // Only return an error if both DNS addresses are empty
    if ( (dns1AddrStr[0] == '\0') && (dns2AddrStr[0] == '\0') )
    {
        return LE_FAULT;
    }

    // At least one DNS address is present
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the Access Point Name for the given profile, if the data session is connected.
 *
 * @return
 *      - LE_OK on success
 *      - LE_OVERFLOW if the Access Point Name would not fit in apnNameStr
 *      - LE_FAULT for all other errors
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mdc_GetAccessPointName
(
    uint32_t profileIndex,             ///< [IN] The profile to use
    char*  apnNameStr,                 ///< [OUT] The Access Point Name
    size_t apnNameStrSize              ///< [IN] The size in bytes of the address buffer
)
{
    le_mdc_ConState_t sessionState;
    le_result_t result;
    wds_get_runtime_settings_resp_msg_v01 wdsSettingsResp;

    result = pa_mdc_GetSessionState(profileIndex, &sessionState);
    if ( (result != LE_OK) || (sessionState != LE_MDC_CONNECTED) )
    {
        return LE_FAULT;
    }

    result = GetRuntimeSettings(profileIndex,
                                WDS_IP_FAMILY_ENUM_2_LE_MDMDEFS_IPVERSION(WDS_IP_FAMILY_IPV4_V01),
                                QMI_WDS_MASK_REQ_SETTINGS_APN_NAME_V01,
                                &wdsSettingsResp);

    if ( (result != LE_OK) || (!wdsSettingsResp.apn_name_valid) )
    {
        result = GetRuntimeSettings(profileIndex,
                                WDS_IP_FAMILY_ENUM_2_LE_MDMDEFS_IPVERSION(WDS_IP_FAMILY_IPV6_V01),
                                QMI_WDS_MASK_REQ_SETTINGS_APN_NAME_V01,
                                &wdsSettingsResp);

        if ( (result != LE_OK) || (!wdsSettingsResp.apn_name_valid) )
        {
            return LE_FAULT;
        }
    }

    if (le_utf8_Copy(apnNameStr,
                     wdsSettingsResp.apn_name, apnNameStrSize, NULL) == LE_OVERFLOW )
    {
        LE_ERROR("Interface name '%s' is too long", wdsSettingsResp.apn_name);
        return LE_OVERFLOW;
    }

    return LE_OK;
}


//--------------------------------------------------------------------------------------------------
/**
 * Get the Data Bearer Technology for the given profile, if the data session is connected.
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT for all other errors
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mdc_GetDataBearerTechnology
(
    uint32_t                       profileIndex,              ///< [IN] The profile to use
    le_mdc_DataBearerTechnology_t* downlinkDataBearerTechPtr, ///< [OUT] downlink data bearer
                                                              /// technology
    le_mdc_DataBearerTechnology_t* uplinkDataBearerTechPtr    ///< [OUT] uplink data bearer
                                                              /// technology
)
{
    le_mdc_ConState_t sessionState;
    le_result_t result;

    ProfileCtx_t* profileCtxPtr = GetProfileCtxFromList(profileIndex);
    qmi_client_type wdsClient = NULL;

    if (profileCtxPtr->callContext.ipv4.enable)
    {
        wdsClient = GetWdsClient(profileIndex, LE_MDMDEFS_IPV4, false, false);
    }

    if (profileCtxPtr->callContext.ipv6.enable)
    {
        wdsClient = GetWdsClient(profileIndex, LE_MDMDEFS_IPV6, false, false);
    }

    if (wdsClient == NULL)
    {
        LE_ERROR("Bad wdsClient");
        return LE_FAULT;
    }

    result = pa_mdc_GetSessionState(profileIndex, &sessionState);
    if ( (result != LE_OK) || (sessionState != LE_MDC_CONNECTED) )
    {
        return LE_FAULT;
    }

    qmi_client_error_type rc;

    // There is no request message, only a response message.
    SWIQMI_DECLARE_MESSAGE(wds_get_data_bearer_technology_ex_resp_msg_v01,
                           wdsdataBearerTechnologyExResp);

    rc = qmi_client_send_msg_sync(wdsClient,
            QMI_WDS_GET_DATA_BEARER_TECHNOLOGY_EX_REQ_V01,
            NULL, 0,
            &wdsdataBearerTechnologyExResp, sizeof(wdsdataBearerTechnologyExResp),
            COMM_LONG_PLATFORM_TIMEOUT);

    if (LE_OK !=
        swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_WDS_GET_DATA_BEARER_TECHNOLOGY_EX_REQ_V01),
                             rc,
                             wdsdataBearerTechnologyExResp.resp.result,
                             wdsdataBearerTechnologyExResp.resp.error))
    {
        return LE_FAULT;
    }

    wds_bearer_tech_so_mask_v01 so_mask = wdsdataBearerTechnologyExResp.bearer_tech.so_mask;

    // Set the default value to uplink and downlink.
    *downlinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_UNKNOWN;
    *uplinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_UNKNOWN;

    if (wdsdataBearerTechnologyExResp.bearer_tech_valid)
    {
        // Set the return value
        switch (wdsdataBearerTechnologyExResp.bearer_tech.rat_value)
        {
            case WDS_BEARER_TECH_RAT_EX_3GPP_WCDMA_V01:
                // The so_mask could contains multiple masks.
                #if defined(QMI_WDS_3GPP_SO_MASK_WCDMA_V01)
                if (so_mask & QMI_WDS_3GPP_SO_MASK_WCDMA_V01)
                {
                    *downlinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_WCDMA;
                    *uplinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_WCDMA;
                }
                #endif
                #if defined(QMI_WDS_3GPP_SO_MASK_HSPA_V01)
                if (so_mask & QMI_WDS_3GPP_SO_MASK_HSPA_V01)
                {
                    *downlinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_HSPA;
                    *uplinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_HSPA;
                }
                #endif
                #if defined(QMI_WDS_3GPP_SO_MASK_HSDPA_V01)
                if (so_mask & QMI_WDS_3GPP_SO_MASK_HSDPA_V01)
                {
                    *downlinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_HSDPA;
                }
                #endif
                #if defined(QMI_WDS_3GPP_SO_MASK_HSUPA_V01)
                if (so_mask & QMI_WDS_3GPP_SO_MASK_HSUPA_V01)
                {
                    *uplinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_HSUPA;
                }
                #endif
                #if defined(QMI_WDS_3GPP_SO_MASK_HSDPAPLUS_V01)
                if (so_mask & QMI_WDS_3GPP_SO_MASK_HSDPAPLUS_V01)
                {
                    *downlinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_HSPA_PLUS;
                }
                #endif
                #if defined(QMI_WDS_3GPP_SO_MASK_DC_HSDPAPLUS_V01)
                if (so_mask & QMI_WDS_3GPP_SO_MASK_DC_HSDPAPLUS_V01)
                {
                    *downlinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_DC_HSPA_PLUS;
                }
                #endif
                #if defined(QMI_WDS_3GPP_SO_MASK_DC_HSUPA_V01)
                if (so_mask & QMI_WDS_3GPP_SO_MASK_DC_HSUPA_V01)
                {
                    *uplinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_DC_HSPA;
                }
                #endif
                break;

            case WDS_BEARER_TECH_RAT_EX_3GPP_GERAN_V01:
                #if defined(QMI_WDS_3GPP_SO_MASK_GSM_V01)
                if (so_mask & QMI_WDS_3GPP_SO_MASK_GSM_V01)
                {
                    *downlinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_GSM;
                    *uplinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_GSM;
                }
                #endif
                #if defined(QMI_WDS_3GPP_SO_MASK_GPRS_V01)
                if (so_mask & QMI_WDS_3GPP_SO_MASK_GPRS_V01)
                {
                    *downlinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_GPRS;
                    *uplinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_GPRS;
                }
                #endif
                #if defined(QMI_WDS_3GPP_SO_MASK_EDGE_V01)
                if (so_mask & QMI_WDS_3GPP_SO_MASK_EDGE_V01)
                {
                    *downlinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_EGPRS;
                    *uplinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_EGPRS;
                }
                #endif
                break;

            case WDS_BEARER_TECH_RAT_EX_3GPP_LTE_V01:
                #if defined(QMI_WDS_3GPP_SO_MASK_LTE_LIMITED_SRVC_V01)
                if (so_mask & QMI_WDS_3GPP_SO_MASK_LTE_LIMITED_SRVC_V01)
                {
                    *downlinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_LTE;
                    *uplinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_LTE;
                }
                #endif
                #if defined(QMI_WDS_3GPP_SO_MASK_LTE_FDD_V01)
                if (so_mask & QMI_WDS_3GPP_SO_MASK_LTE_FDD_V01)
                {
                    *downlinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_LTE_FDD;
                    *uplinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_LTE_FDD;
                }
                #endif
                #if defined(QMI_WDS_3GPP_SO_MASK_LTE_TDD_V01)
                if (so_mask & QMI_WDS_3GPP_SO_MASK_LTE_TDD_V01)
                {
                    *downlinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_LTE_TDD;
                    *uplinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_LTE_TDD;
                }
                #endif
                #if defined(QMI_WDS_3GPP_SO_MASK_LTE_CA_DL_V01)
                if (so_mask & QMI_WDS_3GPP_SO_MASK_LTE_CA_DL_V01)
                {
                    *downlinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_LTE_CA_DL;
                }
                #endif
                #if defined(QMI_WDS_3GPP_SO_MASK_LTE_CA_UL_V01)
                if (so_mask & QMI_WDS_3GPP_SO_MASK_LTE_CA_UL_V01)
                {
                    *uplinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_LTE_CA_UL;
                }
                #endif
                break;

            case WDS_BEARER_TECH_RAT_EX_3GPP_TDSCDMA_V01:
                #if defined(QMI_WDS_3GPP_SO_MASK_TDSCDMA_V01)
                if (so_mask & QMI_WDS_3GPP_SO_MASK_TDSCDMA_V01)
                {
                    *downlinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_TD_SCDMA;
                    *uplinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_TD_SCDMA;
                }
                #endif
                #if defined(QMI_WDS_3GPP_SO_MASK_HSPA_V01)
                if (so_mask & QMI_WDS_3GPP_SO_MASK_HSPA_V01)
                {
                    *downlinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_HSPA;
                    *uplinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_HSPA;
                }
                #endif
                #if defined(QMI_WDS_3GPP_SO_MASK_HSDPA_V01)
                if (so_mask & QMI_WDS_3GPP_SO_MASK_HSDPA_V01)
                {
                    *downlinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_HSDPA;
                }
                #endif
                #if defined(QMI_WDS_3GPP_SO_MASK_HSUPA_V01)
                if (so_mask & QMI_WDS_3GPP_SO_MASK_HSUPA_V01)
                {
                    *uplinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_HSUPA;
                }
                #endif
                #if defined(QMI_WDS_3GPP_SO_MASK_HSDPAPLUS_V01)
                if (so_mask & QMI_WDS_3GPP_SO_MASK_HSDPAPLUS_V01)
                {
                    *downlinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_HSPA_PLUS;
                }
                #endif
                #if defined(QMI_WDS_3GPP_SO_MASK_DC_HSDPAPLUS_V01)
                if (so_mask & QMI_WDS_3GPP_SO_MASK_DC_HSDPAPLUS_V01)
                {
                    *downlinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_DC_HSPA_PLUS;
                }
                #endif
                #if defined(QMI_WDS_3GPP_SO_MASK_DC_HSUPA_V01)
                if (so_mask & QMI_WDS_3GPP_SO_MASK_DC_HSUPA_V01)
                {
                    *uplinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_DC_HSPA;
                }
                #endif
                break;

            case WDS_BEARER_TECH_RAT_EX_3GPP2_1X_V01:
                #if defined(QMI_WDS_3GPP2_SO_MASK_1X_IS95_V01)
                if (so_mask & QMI_WDS_3GPP2_SO_MASK_1X_IS95_V01)
                {
                    *downlinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_IS95_1X;
                    *uplinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_IS95_1X;
                }
                #endif
                #if defined(QMI_WDS_3GPP2_SO_MASK_1X_IS2000_V01)
                if (so_mask & QMI_WDS_3GPP2_SO_MASK_1X_IS2000_V01)
                {
                    *downlinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_CDMA2000_1X;
                    *uplinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_CDMA2000_1X;
                }
                #endif
                #if defined(QMI_WDS_3GPP2_SO_MASK_1X_IS2000_REL_A_V01)
                if (so_mask & QMI_WDS_3GPP2_SO_MASK_1X_IS2000_REL_A_V01)
                {
                    *downlinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_CDMA2000_EVDO_REVA;
                    *uplinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_CDMA2000_EVDO_REVA;
                }
                #endif
                break;

            case WDS_BEARER_TECH_RAT_EX_3GPP2_HRPD_V01:
                #if defined(QMI_WDS_3GPP2_SO_MASK_HDR_REV0_DPA_V01)
                if (so_mask & QMI_WDS_3GPP2_SO_MASK_HDR_REV0_DPA_V01)
                {
                    *downlinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_HDR_REV0_DPA;
                    *uplinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_HDR_REV0_DPA;
                }
                #endif
                #if defined(QMI_WDS_3GPP2_SO_MASK_HDR_REVA_DPA_V01)
                if (so_mask & QMI_WDS_3GPP2_SO_MASK_HDR_REVA_DPA_V01)
                {
                    *downlinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_HDR_REVA_DPA;
                    *uplinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_HDR_REVA_DPA;
                }
                #endif
                #if defined(QMI_WDS_3GPP2_SO_MASK_HDR_REVB_DPA_V01)
                if (so_mask & QMI_WDS_3GPP2_SO_MASK_HDR_REVB_DPA_V01)
                {
                    *downlinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_HDR_REVB_DPA;
                    *uplinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_HDR_REVB_DPA;
                }
                #endif
                #if defined(QMI_WDS_3GPP2_SO_MASK_HDR_REVA_MPA_V01)
                if (so_mask & QMI_WDS_3GPP2_SO_MASK_HDR_REVA_MPA_V01)
                {
                    *downlinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_HDR_REVA_MPA;
                    *uplinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_HDR_REVA_MPA;
                }
                #endif
                #if defined(QMI_WDS_3GPP2_SO_MASK_HDR_REVB_MPA_V01)
                if (so_mask & QMI_WDS_3GPP2_SO_MASK_HDR_REVB_MPA_V01)
                {
                    *downlinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_HDR_REVB_MPA;
                    *uplinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_HDR_REVB_MPA;
                }
                #endif
                #if defined(QMI_WDS_3GPP2_SO_MASK_HDR_REVA_EMPA_V01)
                if (so_mask & QMI_WDS_3GPP2_SO_MASK_HDR_REVA_EMPA_V01)
                {
                    *downlinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_HDR_REVA_EMPA;
                    *uplinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_HDR_REVA_EMPA;
                }
                #endif
                #if defined(QMI_WDS_3GPP2_SO_MASK_HDR_REVB_EMPA_V01)
                if (so_mask & QMI_WDS_3GPP2_SO_MASK_HDR_REVB_EMPA_V01)
                {
                    *downlinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_HDR_REVB_EMPA;
                    *uplinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_HDR_REVB_EMPA;
                }
                #endif
                #if defined(QMI_WDS_3GPP2_SO_MASK_HDR_REVB_MMPA_V01)
                if (so_mask & QMI_WDS_3GPP2_SO_MASK_HDR_REVB_MMPA_V01)
                {
                    *downlinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_HDR_REVB_MMPA;
                    *uplinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_HDR_REVB_MMPA;
                }
                #endif
                #if defined(QMI_WDS_3GPP2_SO_MASK_HDR_EVDO_FMC_V01)
                if (so_mask & QMI_WDS_3GPP2_SO_MASK_HDR_EVDO_FMC_V01)
                {
                    *downlinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_HDR_EVDO_FMC;
                    *uplinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_HDR_EVDO_FMC;
                }
                #endif
                break;

            case WDS_BEARER_TECH_RAT_EX_3GPP_WLAN_V01:
                #if defined(QMI_WDS_3GPP_SO_MASK_S2B_V01)
                if (so_mask & QMI_WDS_3GPP_SO_MASK_S2B_V01)
                {
                    *downlinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_S2B;
                    *uplinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_S2B;
                }
                #endif
                break;

            default:
                *downlinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_UNKNOWN;
                *uplinkDataBearerTechPtr = LE_MDC_DATA_BEARER_TECHNOLOGY_UNKNOWN;
        }

        LE_DEBUG("downlink: %d, uplink: %d QMI Bearer: 0x%016"PRIX64,
                 *downlinkDataBearerTechPtr,
                 *uplinkDataBearerTechPtr,
                 wdsdataBearerTechnologyExResp.bearer_tech.so_mask);
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get data flow statistics since the last reset.
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT for all other errors
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mdc_GetDataFlowStatistics
(
    pa_mdc_PktStatistics_t* dataStatisticsPtr   ///< [OUT] Statistics data
)
{
    le_result_t result = LE_OK;
    bool sessionInProgress;
    pa_mdc_PktStatistics_t currentDataStatistics = {0};
    qmi_client_type wdsClient = mdc_qmi_GetDefaultWdsClient();

    if (!wdsClient)
    {
        LE_ERROR("Invalid WDS client");
        return LE_FAULT;
    }

    DumpDataFlowStatistics();

    memset(dataStatisticsPtr, 0, sizeof(*dataStatisticsPtr));

    dataStatisticsPtr->receivedBytesCount = DataStatistics.saved.receivedBytesCount
                                            - DataStatistics.reset.receivedBytesCount;
    dataStatisticsPtr->transmittedBytesCount = DataStatistics.saved.transmittedBytesCount
                                               - DataStatistics.reset.transmittedBytesCount;

    // Check if the data statistics saving is enabled
    if (DataStatistics.saving)
    {
        result = GetDataFlowStatistics(wdsClient, &sessionInProgress, &currentDataStatistics);
        if ((LE_OK == result) && (sessionInProgress))
        {
            dataStatisticsPtr->receivedBytesCount += currentDataStatistics.receivedBytesCount;
            dataStatisticsPtr->transmittedBytesCount += currentDataStatistics.transmittedBytesCount;
        }
    }

    mdc_qmi_ReleaseWdsClient(wdsClient);

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Reset data flow statistics
 *
 * * @return
 *      - LE_OK on success
 *      - LE_FAULT for all other errors
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mdc_ResetDataFlowStatistics
(
    void
)
{
    le_result_t result;
    bool sessionInProgress;
    pa_mdc_PktStatistics_t currentDataStatistics = {0};
    qmi_client_type wdsClient = mdc_qmi_GetDefaultWdsClient();

    if (!wdsClient)
    {
        LE_ERROR("Invalid WDS client");
        return LE_FAULT;
    }

    result = GetDataFlowStatistics(wdsClient, &sessionInProgress, &currentDataStatistics);
    if (LE_OK == result)
    {
        if (sessionInProgress)
        {
            DataStatistics.reset.receivedBytesCount=currentDataStatistics.receivedBytesCount;
            DataStatistics.reset.transmittedBytesCount=currentDataStatistics.transmittedBytesCount;
        }
        else
        {
            DataStatistics.reset.receivedBytesCount = 0;
            DataStatistics.reset.transmittedBytesCount = 0;
        }

        DataStatistics.saved.receivedBytesCount = 0;
        DataStatistics.saved.transmittedBytesCount = 0;
    }

    mdc_qmi_ReleaseWdsClient(wdsClient);

    DumpDataFlowStatistics();

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Stop collecting data flow statistics
 *
 * * @return
 *      - LE_OK on success
 *      - LE_FAULT for all other errors
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mdc_StopDataFlowStatistics
(
    void
)
{
    // Save the current data statistics
    qmi_client_type wdsClient = mdc_qmi_GetDefaultWdsClient();
    if (!wdsClient)
    {
        LE_ERROR("Invalid WDS client");
        return LE_FAULT;
    }
    SaveDataFlowStatistics(wdsClient);
    mdc_qmi_ReleaseWdsClient(wdsClient);

    // Stop saving data statistics
    DataStatistics.saving = false;

    DumpDataFlowStatistics();

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Start collecting data flow statistics
 *
 * * @return
 *      - LE_OK on success
 *      - LE_FAULT for all other errors
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mdc_StartDataFlowStatistics
(
    void
)
{
    DataStatistics.saving = true;
    DumpDataFlowStatistics();
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Map a profile on a network interface
 *
 * * @return
 *      - LE_OK on success
 *      - LE_UNSUPPORTED if not supported by the target
 *      - LE_FAULT for all other errors
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mdc_MapProfileOnNetworkInterface
(
    uint32_t         profileIndex,         ///< [IN] The profile to use
    const char*      interfaceNamePtr      ///< [IN] Network interface name
)
{
#if defined(SIERRA_MDM9X40) || defined(SIERRA_MDM9X28)
    ProfileCtx_t* profileCtxPtr = GetProfileCtxFromList(profileIndex);

    if (!profileCtxPtr)
    {
        return LE_FAULT;
    }

    if (strncmp(interfaceNamePtr, DATA_INTERFACE_PREFIX, strlen(DATA_INTERFACE_PREFIX)) != 0)
    {
        LE_ERROR("Unknown rmnet interface %s %s %d", interfaceNamePtr, DATA_INTERFACE_PREFIX,
            strlen(DATA_INTERFACE_PREFIX));
        return LE_FAULT;
    }

    errno = 0;

    profileCtxPtr->mappedRmnetId = strtol(interfaceNamePtr+strlen(DATA_INTERFACE_PREFIX), NULL, 10);

    if (errno != 0)
    {
        LE_ERROR("Bad rmnet %d %m", errno);
        return LE_FAULT;
    }

    LE_DEBUG("mapped RmnetId: %d", profileCtxPtr->mappedRmnetId);

    return LE_OK;
#else
    return LE_UNSUPPORTED;
#endif
}
