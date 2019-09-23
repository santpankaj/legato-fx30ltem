/**
 * @file atAirVantage.c: Implements AT commands for AVMS LWM2M client
 *
 * <HR>
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#include <legato.h>
#include <interfaces.h>

//--------------------------------------------------------------------------------------------------
/**
 * Strtoul base 10
 */
//--------------------------------------------------------------------------------------------------
#define BASE10                      10

//--------------------------------------------------------------------------------------------------
/**
 * Level: no indication
 */
//--------------------------------------------------------------------------------------------------
#define LEVEL_NO_INDICATIONS        (0<<0)

//--------------------------------------------------------------------------------------------------
/**
 * Level: activate the initialization end indication
 */
//--------------------------------------------------------------------------------------------------
#define LEVEL_INIT                  (1<<0)

//--------------------------------------------------------------------------------------------------
/**
 * Level: activate the server request for a user agreement indication
 */
//--------------------------------------------------------------------------------------------------
#define LEVEL_USER_AGREEMENT        (1<<1)

//--------------------------------------------------------------------------------------------------
/**
 * Level: activate the authentication indications
 */
//--------------------------------------------------------------------------------------------------
#define LEVEL_AUTHENTICATION        (1<<2)

//--------------------------------------------------------------------------------------------------
/**
 * Level: activate the session indication
 */
//--------------------------------------------------------------------------------------------------
#define LEVEL_SESSION               (1<<3)

//--------------------------------------------------------------------------------------------------
/**
 * Level: activate the package download indications
 */
//--------------------------------------------------------------------------------------------------
#define LEVEL_DOWNLOAD              (1<<4)

//--------------------------------------------------------------------------------------------------
/**
 * Level: activate the certified downloaded package indication
 */
//--------------------------------------------------------------------------------------------------
#define LEVEL_CERTIFIED_DOWNLOAD    (1<<5)

//--------------------------------------------------------------------------------------------------
/**
 * Level: activate the update indications
 */
//--------------------------------------------------------------------------------------------------
#define LEVEL_UPDATE                (1<<6)

//--------------------------------------------------------------------------------------------------
/**
 * Level: activate the fallback indication
 */
//--------------------------------------------------------------------------------------------------
#define LEVEL_FALLBACK              (1<<7)

//--------------------------------------------------------------------------------------------------
/**
 * Level: activate download progress indication
 */
//--------------------------------------------------------------------------------------------------
#define LEVEL_PROGRESS              (1<<8)

//--------------------------------------------------------------------------------------------------
/**
 * Level: Activate Bootstrap event indication
 */
//--------------------------------------------------------------------------------------------------
#define LEVEL_BOOTSTRAP             (1<<12)

//--------------------------------------------------------------------------------------------------
/**
 * Supported indications levels mask
 */
//--------------------------------------------------------------------------------------------------
#define SUPPORTED_IND_MASK          (LEVEL_INIT | LEVEL_USER_AGREEMENT | \
                                     LEVEL_AUTHENTICATION | LEVEL_SESSION | \
                                     LEVEL_DOWNLOAD | LEVEL_CERTIFIED_DOWNLOAD | \
                                     LEVEL_UPDATE | LEVEL_PROGRESS | LEVEL_BOOTSTRAP)

//--------------------------------------------------------------------------------------------------
/**
 * Config: Config is disabled
 */
//--------------------------------------------------------------------------------------------------
#define CONFIG_DISABLED             0

//--------------------------------------------------------------------------------------------------
/**
 * Config: Config is enabled
 */
//--------------------------------------------------------------------------------------------------
#define CONFIG_ENABLED              1

//--------------------------------------------------------------------------------------------------
/**
 * Timer: Polling mode timer max value in minutes
 */
//--------------------------------------------------------------------------------------------------
#define POLL_TIMER_MAX              525600

//--------------------------------------------------------------------------------------------------
/**
 * Timer: Retry mode timer max value in minutes
 */
//--------------------------------------------------------------------------------------------------
#define RETRY_TIMER_MAX             20160

//--------------------------------------------------------------------------------------------------
/**
 * Minimum number of retry timers to configure
 */
//--------------------------------------------------------------------------------------------------
#define MIN_RETRY_TIMERS_COUNT          1

//--------------------------------------------------------------------------------------------------
/**
 * Device services activation status indication
 */
//--------------------------------------------------------------------------------------------------
#define DEVICE_SERVICES             0

//--------------------------------------------------------------------------------------------------
/**
 * Session and package indication
 */
//--------------------------------------------------------------------------------------------------
#define SESSION_PKG                 1

//--------------------------------------------------------------------------------------------------
/**
 * Max replies
 */
//--------------------------------------------------------------------------------------------------
#define REPLY_MAX                   9

//--------------------------------------------------------------------------------------------------
/**
 * reply timer default value
 */
//--------------------------------------------------------------------------------------------------
#define REPLY_TIMER_DEFAULT         30

//--------------------------------------------------------------------------------------------------
/**
 * reply timer max value
 */
//--------------------------------------------------------------------------------------------------
#define REPLY_TIMER_MAX             1440

//--------------------------------------------------------------------------------------------------
/**
 * Config command max parameters number for setting-up retry timer.
 */
//--------------------------------------------------------------------------------------------------
#define WDSC_RETRY_PARAMS_COUNT   LE_AVC_NUM_RETRY_TIMERS+1

//--------------------------------------------------------------------------------------------------
/**
 * Config command regular parameters number(except setting the retry timer).
 */
//--------------------------------------------------------------------------------------------------
#define WDSC_REG_PARAMS_COUNT   2

//--------------------------------------------------------------------------------------------------
/**
 * Reply command min parameters number
 */
//--------------------------------------------------------------------------------------------------
#define WDSR_MIN_PARAMS_COUNT   1

//--------------------------------------------------------------------------------------------------
/**
 * Reply command max parameters number where optional timer value can be specified
 * (i.e. DeferDownload, DeferInstall etc)
 */
//--------------------------------------------------------------------------------------------------
#define WDSR_MAX_PARAMS_COUNT_WITH_TIMER   2

//--------------------------------------------------------------------------------------------------
/**
 * Reply command max parameters number where no timer value should be specified
 * (i.e. AcceptDownload, AcceptInstall etc)
 */
//--------------------------------------------------------------------------------------------------
#define WDSR_MAX_PARAMS_COUNT_NO_TIMER   1

//--------------------------------------------------------------------------------------------------
/**
 * CID setup command max parameters number
 */
//--------------------------------------------------------------------------------------------------
#define WDSS_CID_SET_PARAMS_COUNT   2

//--------------------------------------------------------------------------------------------------
/**
 * Session turn on/off command parameters number
 */
//--------------------------------------------------------------------------------------------------
#define WDSS_SESSION_ON_OFF_PARAMS_COUNT   2

//--------------------------------------------------------------------------------------------------
/**
 * Indications command max parameters number
 */
//--------------------------------------------------------------------------------------------------
#define WDSI_MAX_PARAMS_COUNT   1

//--------------------------------------------------------------------------------------------------
/**
 * Indications command max parameters number
 */
//--------------------------------------------------------------------------------------------------
#define WDSW_MAX_PARAMS_COUNT   3

//--------------------------------------------------------------------------------------------------
/**
 * APN max len
 */
//--------------------------------------------------------------------------------------------------
#define APN_MAX_LEN             50

//--------------------------------------------------------------------------------------------------
/**
 * APN max bytes
 */
//--------------------------------------------------------------------------------------------------
#define APN_MAX_BYTES           APN_MAX_LEN+1

//--------------------------------------------------------------------------------------------------
/**
 * User name max len
 */
//--------------------------------------------------------------------------------------------------
#define USRNAME_MAX_LEN         30

//--------------------------------------------------------------------------------------------------
/**
 * User name max bytes
 */
//--------------------------------------------------------------------------------------------------
#define USRNAME_MAX_BYTES       USRNAME_MAX_LEN+1

//--------------------------------------------------------------------------------------------------
/**
 * Password max len
 */
//--------------------------------------------------------------------------------------------------
#define PASSWD_MAX_LEN          30

//--------------------------------------------------------------------------------------------------
/**
 * Password max bytes
 */
//--------------------------------------------------------------------------------------------------
#define PASSWD_MAX_BYTES        PASSWD_MAX_LEN+1

//--------------------------------------------------------------------------------------------------
/**
 * Retry timers max string length
 */
//--------------------------------------------------------------------------------------------------
#define RETRY_TIMERS_LEN        64

//--------------------------------------------------------------------------------------------------
/**
 * Verify the number of parameters
 */
//--------------------------------------------------------------------------------------------------
#define VERIFY_PARAMETERS_COUNT(COUNT, EXPECTED_COUNT) (COUNT <= EXPECTED_COUNT)

//--------------------------------------------------------------------------------------------------
/**
 * Indications save file
 *
 */
//--------------------------------------------------------------------------------------------------
#define INDICATIONS_FILE       "/atAirVantage/indications"

//--------------------------------------------------------------------------------------------------
/**
 * Indications events
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    EVENT_INIT = 0,                 ///< Device services are initialized and can be used.
                                    ///< Device services are initialized when the SIM PIN code
                                    ///< is entered and a dedicated NAP is configured.
    EVENT_CONN_REQUEST = 1,         ///< The Device Services server requests the device to make
                                    ///< a connection.
    EVENT_DWL_REQUEST = 2,          ///< The Device Services server requests the device to make
                                    ///< a package download.
    EVENT_DWL_SUCCESS = 3,          ///< The device has downloaded a package.
    EVENT_AUTH = 4,                 ///< The embedded module starts authentication with the server.
    EVENT_AUTH_FAILURE = 5,         ///< Authentication with the server failed.
    EVENT_SESSION_SUCCESS = 6,      ///< Authentication has succeeded and session with the server
                                    ///< started.
    EVENT_SESSION_FAILURE = 7,      ///< Session with the server failed.
    EVENT_SESSION_END = 8,          ///< Session with the server is finished.
    EVENT_DWL_READY = 9,            ///< A package is available on the server and can be downloaded
                                    ///< by the embedded module.
    EVENT_STORE_SUCCESS = 10,       ///< A package was successfully downloaded and stored in flash
    EVENT_DWL_ERROR = 11,           ///< An issue happened during the package download. If the
                                    ///< download has not started (event 9 was not returned), this
                                    ///< indicates that there is not enough space in the device to
                                    ///< download the update package. If the download has started
                                    ///< (event 9 was returned), a flash problem implies that the
                                    ///< package has not been saved in the device
    EVENT_DWL_CERTIFIED = 12,       ///< Downloaded package is certified to be sent by the AirPrime
                                    ///< Management Services server
    EVENT_DWL_NOT_CERTIFIED = 13,   ///< Downloaded package is not certified to be sent by the
                                    ///< Management Services server
    EVENT_UPDATE_START = 14,        ///< Update will be launched
    EVENT_OTA_FAILURE = 15,         ///< OTA failed
    EVENT_OTA_SUCCESS = 16,         ///< OTA finished successfully
    EVENT_DWL_PROGRESS = 18,        ///< Download progress
    EVENT_SESSION_TYPE = 23,        ///< Session type
    EVENT_REBOOT_REQUEST = 24,      ///< Reboot request
    EVENT_UNINSTALL_REQUEST = 25,   ///< Uninstall request
}
IndicationsEvent_t;

//--------------------------------------------------------------------------------------------------
/**
 * User configuration
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    CONFIG_CONNECT,      ///< User agreement for connecting to server
    CONFIG_DOWNLOAD,     ///< User agreement for package download
    CONFIG_INSTALL,      ///< User agreement for package install
    CONFIG_POLL,         ///< The embedded module will initiate a connection to the Device
                         ///< Services server according to the defined timer.
    CONFIG_RETRY,        ///< If an error occurs during a connection to the Device Services server
                         ///< (WWAN DATA establishment failed, http error code received),
                         ///< the embedded module will initiate a new connection according to the
                         ///< defined timers. This mechanism is persistent to the reset
    CONFIG_REBOOT,       ///< User agreement for device reboot
    CONFIG_UNINSTALL     ///< User agreement for application uninstall
}
UserConfig_t;

//--------------------------------------------------------------------------------------------------
/**
 * Retry default timers
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    RETRY_TIMER_DEFAULT1 = 15,      ///< Retry mode timer 1st default value in minutes
    RETRY_TIMER_DEFAULT2 = 60,      ///< Retry mode timer 2nd default value in minutes
    RETRY_TIMER_DEFAULT3 = 240,     ///< Retry mode timer 3rd default value in minutes
    RETRY_TIMER_DEFAULT4 = 960,     ///< Retry mode timer 4th default value in minutes
    RETRY_TIMER_DEFAULT5 = 2880,    ///< Retry mode timer 5th default value in minutes
    RETRY_TIMER_DEFAULT6 = 10080,   ///< Retry mode timer 6th default value in minutes
    RETRY_TIMER_DEFAULT7 = 10080,   ///< Retry mode timer 7th default value in minutes
}
RetryTimer_t;

//--------------------------------------------------------------------------------------------------
/**
 * Service status
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    SERVICE_STATUS_PROHIBITED,  ///< Device services are prohibited. Services will never be
                                ///< activated.
    SERVICE_STATUS_DEACTIVATED, ///< Device services are deactivated. Connection parameters to a
                                ///< device Services server have to be provisioned.
    SERVICE_STATUS_BOOTSTRAP,   ///< Device services have to be provisioned. A bootstrap session is
                                ///< required.
    SERVICE_STATUS_ACTIVATED,   ///< Device services are activated.
                                ///< For LWM2M protocol, this indication means that a bootstrap
                                ///< session was already made and the device is able to initiate a
                                ///< device Management session with the AirVantage server.
}
ServiceStatus_t;

//--------------------------------------------------------------------------------------------------
/**
 * Session status
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    SESSION_STATUS_DOWN,            ///< No session or package.
    SESSION_STATUS_UP,              ///< A session is under treatment.
    SESSION_STATUS_PKG_AVAILABLE,   ///< A package is available on the server.
    SESSION_STATUS_PKG_DOWNLOADED,  ///< A package was downloaded and ready to install.
}
SessionStatus_t;


//--------------------------------------------------------------------------------------------------
/**
 * Session type
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    SESSION_BS,            ///< Bootstrap server session
    SESSION_DM,            ///< DM server session
}
SessionType_t;

//--------------------------------------------------------------------------------------------------
/**
 * Reply to a user agreement request
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    REPLY_DELAY_CONNECT,                ///< Accept connect
    REPLY_ACCEPT_CONNECT,               ///< Delay connect
    REPLY_DELAY_DOWNLOAD,               ///< Delay download
    REPLY_ACCEPT_DOWNLOAD,              ///< Accept download
    REPLY_ACCEPT_INSTALL,               ///< Accept install
    REPLY_DELAY_INSTALL,                ///< Delay install
    REPLY_ACCEPT_REBOOT,                ///< Accept reboot
    REPLY_DELAY_REBOOT,                 ///< Delay reboot
    REPLY_ACCEPT_UNINSTALL,             ///< Accept uninstall
    REPLY_DELAY_UNINSTALL               ///< Delay uninstall
}
UserReply_t;

//--------------------------------------------------------------------------------------------------
/**
 * Download failure reason as defined in +WDSI:11
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    FAIL_NO_SUFFICIENT_MEMORY,  ///< Insufficient memory in device to save package (not supported)
    FAIL_HTTP_ERROR,            ///< HTTP/HTTPS error occurred
    FAIL_CORRUPTED_PKG          ///< Corrupted firmware update package
}
DownloadFailureReason_t;

//--------------------------------------------------------------------------------------------------
/**
 * Access Point Name configuration struct definition
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    char            apn[APN_MAX_BYTES];         ///< APN name
    le_mdc_Auth_t   auth;                       ///< Authentication type
    char            usr[USRNAME_MAX_BYTES];     ///< User name
    char            passwd[PASSWD_MAX_BYTES];   ///< Password
}
ApnConf_t;

//--------------------------------------------------------------------------------------------------
/**
 * Status_t struct definition
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    ServiceStatus_t service; ///< Service indication
    SessionStatus_t session; ///< Session indication
}
Status_t;

//--------------------------------------------------------------------------------------------------
/**
 * Indications_t struct definition
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    le_event_Id_t           eventId;    ///< Event id
    le_event_HandlerRef_t   eventRef;   ///< Event reference
    const char*             namePtr;    ///< Command name
    uint16_t                level;      ///< Indication level
    int                     event;      ///< Event type
    int                     data;       ///< Event data
}
Indications_t;

//--------------------------------------------------------------------------------------------------
/**
 * Session_t struct definition
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    le_avc_StatusEventHandlerRef_t  avcEventRef;    ///< AVC event reference
    int                             action;         ///< Start or stop a session
}
Session_t;

//--------------------------------------------------------------------------------------------------
/**
 * Reply_t struct definition
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    UserReply_t reply; ///< User reply
    int         timer; ///< Reply timer
}
Reply_t;

//--------------------------------------------------------------------------------------------------
/**
 * Handler_t struct definition
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    Status_t        status;         ///< General status
    Session_t       session;        ///< Session info
    Indications_t   indications;    ///< Indications
    Reply_t         reply;          ///< Reply
}
Handler_t;

//--------------------------------------------------------------------------------------------------
/**
 * AtCmd_t struct definition
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    const char* atCmdPtr;                           ///< At command pointer
    le_atServer_CmdRef_t cmdRef;                    ///< Command reference
    le_atServer_CommandHandlerFunc_t handlerPtr;    ///< Handler func pointer
}
atCmd_t;

//--------------------------------------------------------------------------------------------------
/**
 * Handler struct initialization
 */
//--------------------------------------------------------------------------------------------------
static Handler_t Handler = {
    .status = {
        .service = SERVICE_STATUS_ACTIVATED,
        .session = SESSION_STATUS_DOWN,
    },
    .session = {
        .avcEventRef = NULL,
        .action = 0,
    },
    .indications = {
        .eventId= NULL,
        .eventRef = NULL,
        .namePtr = "+WDSI",
        .event = -1,
        .data = -1,
    },
    .reply = {
        .reply = 0,
        .timer = -1,
    },
};

//--------------------------------------------------------------------------------------------------
/**
 * Check that device services are initialized by verifying that the platform is registered on
 * network and APN configured
 */
//--------------------------------------------------------------------------------------------------
static void CheckDeviceServices
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * Read from file using Legato le_fs API
 *
 */
//--------------------------------------------------------------------------------------------------
static le_result_t ReadFs
(
    const char* pathPtr,
    uint8_t*    bufPtr,
    size_t      size
)
{
    le_fs_FileRef_t fileRef;
    le_result_t result;

    result = le_fs_Open(pathPtr, LE_FS_RDONLY, &fileRef);
    if (LE_OK != result)
    {
        LE_ERROR("failed to open %s: %s", pathPtr, LE_RESULT_TXT(result));
        return result;
    }

    result = le_fs_Read(fileRef, bufPtr, &size);
    if (LE_OK != result)
    {
        LE_ERROR("failed to read %s: %s", pathPtr, LE_RESULT_TXT(result));
        if (LE_OK != le_fs_Close(fileRef))
        {
            LE_ERROR("failed to close %s", pathPtr);
        }
        return result;
    }

    result = le_fs_Close(fileRef);
    if (LE_OK != result)
    {
        LE_ERROR("failed to close %s: %s", pathPtr, LE_RESULT_TXT(result));
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Write to file using Legato le_fs API
 *
 */
//--------------------------------------------------------------------------------------------------
static le_result_t WriteFs
(
    const char* pathPtr,
    uint8_t*    bufPtr,
    size_t      size
)
{
    le_fs_FileRef_t fileRef;
    le_result_t result;

    result = le_fs_Open(pathPtr, LE_FS_WRONLY | LE_FS_CREAT, &fileRef);
    if (LE_OK != result)
    {
        LE_ERROR("failed to open %s: %s", pathPtr, LE_RESULT_TXT(result));
        return result;
    }

    result = le_fs_Write(fileRef, bufPtr, size);
    if (LE_OK != result)
    {
        LE_ERROR("failed to write %s: %s", pathPtr, LE_RESULT_TXT(result));
        if (LE_OK != le_fs_Close(fileRef))
        {
            LE_ERROR("failed to close %s", pathPtr);
        }
        return result;
    }

    result = le_fs_Close(fileRef);
    if (LE_OK != result)
    {
        LE_ERROR("failed to close %s: %s", pathPtr, LE_RESULT_TXT(result));
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Convert string to integer
 *
 * @return
 *    - LE_BAD_PARAMETER invalid input parameter
 *    - LE_UNDERFLOW value is less than INT_MIN or strtol failed
 *    - LE_OVERFLOW value is greater than INT_MAX or strtol failed
 *    - LE_FORMAT_ERROR value contains non digit characters
 *    - LE_OK success
 */
//--------------------------------------------------------------------------------------------------
static le_result_t GetInt
(
    char*   strPtr,
    int*    valPtr
)
{
    long rc;
    char* endPtr = NULL;

    if (!strPtr || !valPtr)
    {
        return LE_BAD_PARAMETER;
    }

    // Consider empty string as bad parameter. This is needed to discard invalid command
    // (e.g. at+wdsc=0,).
    if ('\0' == *strPtr)
    {
        return LE_BAD_PARAMETER;
    }

    rc = strtol(strPtr, &endPtr, BASE10);

    if ((LONG_MIN == rc) && (ERANGE == errno))
    {
        return LE_UNDERFLOW;
    }

    if ((LONG_MAX == rc) && (ERANGE == errno))
    {
        return LE_OVERFLOW;
    }

    if (*endPtr != '\0')
    {
        return LE_FORMAT_ERROR;
    }

    if (INT_MIN > rc)
    {
        return LE_UNDERFLOW;
    }

    if (INT_MAX < rc)
    {
        return LE_OVERFLOW;
    }

    *valPtr = (int)rc;

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Report handler function
 */
//--------------------------------------------------------------------------------------------------
static void ReportStatus
(
    void* reportPtr
)
{
    char rsp[LE_ATDEFS_RESPONSE_MAX_BYTES] = { 0 };
    Indications_t *indPtr;

    if (!reportPtr)
    {
        LE_ERROR("reportPtr cannot be null");
        return;
    }

    indPtr = (Indications_t *)reportPtr;

    LE_DEBUG("Name: %s, Event: %d, Data: %d", indPtr->namePtr, indPtr->event, indPtr->data);

    if (0 <= indPtr->event)
    {
        if (0 <= indPtr->data)
        {
            snprintf(rsp, LE_ATDEFS_RESPONSE_MAX_BYTES, "%s: %d,%d",
                indPtr->namePtr, indPtr->event, indPtr->data);
        }
        else
        {
            snprintf(rsp, LE_ATDEFS_RESPONSE_MAX_BYTES, "%s: %d",
                indPtr->namePtr, indPtr->event);
        }

        le_atServer_SendUnsolicitedResponse(rsp, LE_ATSERVER_ALL_DEVICES, NULL);
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Report an event
 */
//--------------------------------------------------------------------------------------------------
static void ReportEvent
(
    IndicationsEvent_t event,
    int data
)
{
    Indications_t *indPtr;
    size_t size;
    int reported = 0;

    indPtr = &Handler.indications;
    size = sizeof(Indications_t);

    if (!indPtr->eventId)
    {
        return;
    }

    switch (event)
    {
        case EVENT_INIT:
            if (LEVEL_INIT & indPtr->level)
            {
                indPtr->event = event;
                le_event_Report(indPtr->eventId, indPtr, size);
                reported = 1;
            }
            break;
        case EVENT_CONN_REQUEST:
        case EVENT_DWL_REQUEST:
        case EVENT_DWL_SUCCESS:
        case EVENT_REBOOT_REQUEST:
        case EVENT_UNINSTALL_REQUEST:
            if (LEVEL_USER_AGREEMENT & indPtr->level)
            {
                indPtr->event = event;
                le_event_Report(indPtr->eventId, indPtr, size);
                reported = 1;
            }
            break;
        case EVENT_AUTH:
        case EVENT_AUTH_FAILURE:
            if (LEVEL_AUTHENTICATION & indPtr->level)
            {
                indPtr->event = event;
                le_event_Report(indPtr->eventId, indPtr, size);
                reported = 1;
            }
            break;
        case EVENT_SESSION_SUCCESS:
        case EVENT_SESSION_FAILURE:
        case EVENT_SESSION_END:
            if (LEVEL_SESSION & indPtr->level)
            {
                indPtr->event = event;
                le_event_Report(indPtr->eventId, indPtr, size);
                reported = 1;
            }
            break;
        case EVENT_DWL_READY:
        case EVENT_STORE_SUCCESS:
        case EVENT_DWL_ERROR:
            if (LEVEL_DOWNLOAD & indPtr->level)
            {
                indPtr->event = event;
                indPtr->data = data;
                le_event_Report(indPtr->eventId, indPtr, size);
                reported = 1;
            }
            break;
        case EVENT_DWL_CERTIFIED:
        case EVENT_DWL_NOT_CERTIFIED:
            if (LEVEL_CERTIFIED_DOWNLOAD & indPtr->level)
            {
                indPtr->event = event;
                le_event_Report(indPtr->eventId, indPtr, size);
                reported = 1;
            }
            break;
        case EVENT_UPDATE_START:
        case EVENT_OTA_FAILURE:
        case EVENT_OTA_SUCCESS:
            if (LEVEL_UPDATE & indPtr->level)
            {
                indPtr->event = event;
                le_event_Report(indPtr->eventId, indPtr, size);
                reported = 1;
            }
            break;
        case EVENT_DWL_PROGRESS:
            if (LEVEL_PROGRESS & indPtr->level)
            {
                indPtr->event = event;
                if (data)
                {
                    indPtr->data = data;
                }
                le_event_Report(indPtr->eventId, indPtr, size);
                reported = 1;
            }
            break;
        case EVENT_SESSION_TYPE:
            if (LEVEL_BOOTSTRAP & indPtr->level)
            {
                indPtr->event = event;
                indPtr->data = data;
                le_event_Report(indPtr->eventId, indPtr, size);
                reported = 1;
            }
            break;
        default:
            break;
    }

    LE_DEBUG("Event: %d, Data: %d, Reported: %s", event, data, reported?"yes":"No");
    indPtr->event = indPtr->data = -1;
}

//--------------------------------------------------------------------------------------------------
/**
 * Setup indications
 */
//--------------------------------------------------------------------------------------------------
static int SetIndications
(
    uint16_t level
)
{
    Indications_t *indPtr;

    indPtr = &Handler.indications;

    if (~SUPPORTED_IND_MASK & level)
    {
        LE_ERROR("Unsupported indications levels cannot be set");
        return -1;
    }

    if (LEVEL_NO_INDICATIONS == level)
    {
        if (LE_OK != WriteFs(INDICATIONS_FILE, (uint8_t *)&level, sizeof(level)))
        {
            LE_ERROR("Failed to store indications level");
            return -1;
        }

        if (indPtr->eventId && indPtr->eventRef)
        {
            le_event_RemoveHandler(indPtr->eventRef);
            indPtr->eventId = NULL;
            indPtr->eventRef = NULL;
        }

        indPtr->level = level;
    }
    else
    {
        if (!indPtr->eventId)
        {
            indPtr->eventId = le_event_CreateId("unsolicited-response", sizeof(Indications_t));
        }

        if (!indPtr->eventRef)
        {
            indPtr->eventRef = le_event_AddHandler("unsolicited-reponse-handler",
                                                    indPtr->eventId, ReportStatus);
        }

        level &= (uint16_t)SUPPORTED_IND_MASK;

        if (level != indPtr->level)
        {
            if (LE_OK != WriteFs(INDICATIONS_FILE, (uint8_t *)&level, sizeof(level)))
            {
                LE_ERROR("Failed to store indications level");
                return -1;
            }

            indPtr->level = level;
        }
    }

    return 0;
}

//--------------------------------------------------------------------------------------------------
/**
 * Check if HTTP status code represents a success
 */
//--------------------------------------------------------------------------------------------------
static bool IsSuccessHttpCode
(
    int statusCode
)
{
    // According to RFC 2616, HTTP success codes are included in [200, 299]
    return ((statusCode >= 200) && (statusCode <= 299));
}

//--------------------------------------------------------------------------------------------------
/**
 * Format last download error code to WDSI: 11 error code
 */
//--------------------------------------------------------------------------------------------------
static DownloadFailureReason_t ConvertLastDownloadError
(
    void
)
{
    int httpStatus;
    int avcError;

    httpStatus = le_avc_GetHttpStatus();
    if ((LE_AVC_HTTP_STATUS_INVALID != httpStatus) && (!IsSuccessHttpCode(httpStatus)))
    {
        return FAIL_HTTP_ERROR;
    }

    avcError = le_avc_GetErrorCode();
    if (LE_AVC_ERR_NONE != avcError)
    {
        return FAIL_CORRUPTED_PKG;
    }

    return -1;
}

//--------------------------------------------------------------------------------------------------
/**
 * Avc status handler
 */
//--------------------------------------------------------------------------------------------------
static void AvcStatus
(
    le_avc_Status_t updateStatus,
    int32_t totalNumBytes,
    int32_t progress,
    void* contextPtr
)
{
    Status_t *statusPtr;
    Session_t *sessionPtr;
    bool isUserAgreementEnabled;
    le_result_t result;

    statusPtr = &Handler.status;
    sessionPtr = &Handler.session;

    switch (updateStatus)
    {
        case LE_AVC_DOWNLOAD_PENDING:
            statusPtr->session = SESSION_STATUS_PKG_AVAILABLE;
            ReportEvent(EVENT_DWL_READY, (int)totalNumBytes);

            result = le_avc_GetUserAgreement(LE_AVC_USER_AGREEMENT_DOWNLOAD,
                                             &isUserAgreementEnabled);
            if ((LE_OK == result) && (true == isUserAgreementEnabled))
            {
                LE_DEBUG("Request user agreement for download");
                ReportEvent(EVENT_DWL_REQUEST, -1);
            }
            break;
        case LE_AVC_DOWNLOAD_IN_PROGRESS:
            result = le_avc_GetUserAgreement(LE_AVC_USER_AGREEMENT_DOWNLOAD,
                                             &isUserAgreementEnabled);

            // Zero download progress means that the event is actually notified by the download
            // start event from LWM2M package downloader
            if ((LE_OK == result) && (false == isUserAgreementEnabled) && (0 == progress))
            {
                 ReportEvent(EVENT_DWL_READY, (int)totalNumBytes);
            }
            ReportEvent(EVENT_DWL_PROGRESS, (int)progress);
            break;
        case LE_AVC_DOWNLOAD_COMPLETE:
            statusPtr->session = SESSION_STATUS_PKG_DOWNLOADED;
            ReportEvent(EVENT_STORE_SUCCESS, -1);
            break;
        case LE_AVC_DOWNLOAD_FAILED:
            ReportEvent(EVENT_DWL_ERROR, ConvertLastDownloadError());
            break;
        case LE_AVC_INSTALL_PENDING:
            result = le_avc_GetUserAgreement(LE_AVC_USER_AGREEMENT_INSTALL,
                                             &isUserAgreementEnabled);
            if ((LE_OK == result) && (true == isUserAgreementEnabled))
            {
                LE_DEBUG("Request user agreement for install");
                ReportEvent(EVENT_DWL_SUCCESS, -1);
            }
            break;
        case LE_AVC_UNINSTALL_COMPLETE:
        case LE_AVC_INSTALL_COMPLETE:
            ReportEvent(EVENT_OTA_SUCCESS, -1);
            break;
        case LE_AVC_UNINSTALL_PENDING:
            result = le_avc_GetUserAgreement(LE_AVC_USER_AGREEMENT_UNINSTALL,
                                             &isUserAgreementEnabled);
            if ((LE_OK == result) && (true == isUserAgreementEnabled))
            {
                LE_DEBUG("Request user agreement for uninstall");
                ReportEvent(EVENT_UNINSTALL_REQUEST, -1);
            }
            break;
        case LE_AVC_INSTALL_FAILED:
        case LE_AVC_UNINSTALL_FAILED:
            ReportEvent(EVENT_OTA_FAILURE, -1);
            break;
        case LE_AVC_SESSION_BS_STARTED:
            ReportEvent(EVENT_SESSION_SUCCESS, -1);
            ReportEvent(EVENT_SESSION_TYPE, SESSION_BS);
            break;
        case LE_AVC_SESSION_STARTED:
            statusPtr->session = SESSION_STATUS_UP;
            sessionPtr->action = 1;
            ReportEvent(EVENT_SESSION_SUCCESS, -1);
            ReportEvent(EVENT_SESSION_TYPE, SESSION_DM);
            break;
        case LE_AVC_SESSION_STOPPED:
            statusPtr->session = SESSION_STATUS_DOWN;
            ReportEvent(EVENT_SESSION_END, -1);
            sessionPtr->action = 0;
            break;
        case LE_AVC_SESSION_FAILED:
            statusPtr->session = SESSION_STATUS_DOWN;
            ReportEvent(EVENT_SESSION_FAILURE, -1);
            sessionPtr->action = 0;
            break;
        case LE_AVC_AUTH_STARTED:
            ReportEvent(EVENT_AUTH, -1);
            break;
        case LE_AVC_AUTH_FAILED:
            ReportEvent(EVENT_AUTH_FAILURE, -1);
            break;
        case LE_AVC_CONNECTION_PENDING:
            result = le_avc_GetUserAgreement(LE_AVC_USER_AGREEMENT_CONNECTION,
                                             &isUserAgreementEnabled);
            if ((LE_OK == result) && (true == isUserAgreementEnabled))
            {
                LE_DEBUG("Request user agreement for connection");
                ReportEvent(EVENT_CONN_REQUEST, -1);
            }
            break;
        case LE_AVC_REBOOT_PENDING:
            result = le_avc_GetUserAgreement(LE_AVC_USER_AGREEMENT_REBOOT,
                                             &isUserAgreementEnabled);
            if ((LE_OK == result) && (true == isUserAgreementEnabled))
            {
                LE_DEBUG("Request user agreement for reboot");
                ReportEvent(EVENT_REBOOT_REQUEST, -1);
            }
            break;
        case LE_AVC_INSTALL_IN_PROGRESS:
            // Reports only when install started. Suppress all other.
            if (0 == progress)
            {
                ReportEvent(EVENT_UPDATE_START, -1);
            }
            break;
        case LE_AVC_CERTIFICATION_OK:
            ReportEvent(EVENT_DWL_CERTIFIED, -1);
            break;
        case LE_AVC_CERTIFICATION_KO:
            ReportEvent(EVENT_DWL_NOT_CERTIFIED, -1);
            break;
        case LE_AVC_NO_UPDATE:
        case LE_AVC_UNINSTALL_IN_PROGRESS:
        default:
            break;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Convert avc credential status to wds credential status
 */
//--------------------------------------------------------------------------------------------------
static ServiceStatus_t ConvertToWdsStatus
(
    le_avc_CredentialStatus_t status             ///< [IN] avc credential status
)
{
    ServiceStatus_t serviceStatus;

    switch (status)
    {
        case LE_AVC_DM_CREDENTIAL_PROVISIONED:
            serviceStatus = SERVICE_STATUS_ACTIVATED;
            break;
        case LE_AVC_BS_CREDENTIAL_PROVISIONED:
            serviceStatus = SERVICE_STATUS_BOOTSTRAP;
            break;
        case LE_AVC_NO_CREDENTIAL_PROVISIONED:
            serviceStatus = SERVICE_STATUS_DEACTIVATED;
            break;
        default:
            serviceStatus = SERVICE_STATUS_PROHIBITED;
            break;
    }

    return serviceStatus;
}

//--------------------------------------------------------------------------------------------------
/**
 * Start or stop an avc session
 */
//--------------------------------------------------------------------------------------------------
static le_result_t HandleAvcSession
(
    le_atServer_CmdRef_t cmdRef
)
{
    char param[LE_ATDEFS_PARAMETER_MAX_BYTES] = { 0 };
    le_result_t result;

    result = le_atServer_GetParameter(cmdRef, 1, param, LE_ATDEFS_PARAMETER_MAX_BYTES);
    if (LE_OK != result)
    {
        LE_ERROR("failed to get parameter");
        return LE_FAULT;
    }

    int action = 0;
    result = GetInt(param, &action);
    if (LE_OK != result)
    {
        LE_ERROR("Failed to get session action: %s", LE_RESULT_TXT(result));
        return result;
    }

    switch (action)
    {
        case 0:
            result = le_avc_StopSession();
            if (LE_DUPLICATE == result)
            {
                LE_DEBUG("avc session already stopped");
                result = LE_OK;
            }
            else if (LE_OK != result)
            {
                LE_ERROR("failed to stop avc: %s", LE_RESULT_TXT(result));
            }
            break;
        case 1:
            result = le_avc_StartSession();
            if (LE_DUPLICATE == result)
            {
                LE_DEBUG("avc session already started");
                result = LE_OK;
            }
            else if (LE_OK != result)
            {
                LE_ERROR("failed to start avc: %s", LE_RESULT_TXT(result));
            }
            break;
        default:
            result = LE_FAULT;
            break;
    }

    if (LE_OK == result)
    {
        // Store when it is successful
        Handler.session.action = action;
    }
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the cellular profile index used by the data connection service when the cellular technology
 * is active.
 *
 * @return
 *      - LE_OK             on success
 *      - LE_FAULT          internal error
 *      - LE_BAD_PARAMETER  if the profile index is invalid
 *      - LE_BUSY           the cellular connection is in use
 */
//--------------------------------------------------------------------------------------------------
static le_result_t SetCellularProfileIndex
(
    le_atServer_CmdRef_t    cmdRef,  ///< Command reference
    uint32_t count                   ///< Number of arguments in the command
)
{
    char param[LE_ATDEFS_PARAMETER_MAX_BYTES] = {0};
    int32_t index = LE_MDC_DEFAULT_PROFILE;
    le_result_t result;

    // Check if a CID parameter is specified in the command arguments and extract it. Otherwise,
    // fallback to default CID profile.
    if (WDSS_CID_SET_PARAMS_COUNT == count)
    {
        result = le_atServer_GetParameter(cmdRef, 1, param, LE_ATDEFS_PARAMETER_MAX_BYTES);
        if (LE_OK != result)
        {
            LE_ERROR("CID argument not specified");
            return LE_FAULT;
        }

        result = GetInt(param, (int *)&index);
        if (LE_OK != result)
        {
            LE_ERROR("Failed to get CID: %s", LE_RESULT_TXT(result));
            return result;
        }

        if (index < 1)
        {
            LE_ERROR("Invalid profile index");
            return LE_FAULT;
        }
    }

    result = le_data_SetCellularProfileIndex(index);
    if (LE_OK != result)
    {
        LE_ERROR("Failed to set CID: %s", LE_RESULT_TXT(result));
        return result;
    }

    CheckDeviceServices();
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get apn config
 */
//--------------------------------------------------------------------------------------------------
static le_result_t GetApnConfig
(
    le_atServer_CmdRef_t    cmdRef,
    ApnConf_t*              apnConfPtr
)
{
    int32_t index;
    le_result_t result;
    le_mdc_ProfileRef_t pRef;

    memset(apnConfPtr->apn, 0, APN_MAX_BYTES);
    memset(apnConfPtr->usr, 0, USRNAME_MAX_BYTES);
    memset(apnConfPtr->passwd, 0, PASSWD_MAX_BYTES);

    index = le_data_GetCellularProfileIndex();
    pRef = le_mdc_GetProfile((uint32_t) index);
    if (!pRef)
    {
        LE_ERROR("failed to get profile reference");
        return LE_NOT_FOUND;
    }

    result = le_mdc_GetAPN(pRef, apnConfPtr->apn, APN_MAX_BYTES);
    if (LE_OK != result)
    {
        LE_ERROR("failed to get apn: %s", LE_RESULT_TXT(result));
        return result;
    }

    result = le_mdc_GetAuthentication(pRef, &apnConfPtr->auth, apnConfPtr->usr, USRNAME_MAX_BYTES,
                                      apnConfPtr->passwd, PASSWD_MAX_BYTES);
    if (LE_OK != result)
    {
        LE_ERROR("failed to get authentication: %s", LE_RESULT_TXT(result));
        return result;
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set retry timers
 */
//--------------------------------------------------------------------------------------------------
static le_result_t SetRetryTimers
(
    le_atServer_CmdRef_t    cmdRef,
    uint32_t                count
)
{
    int i;
    char param[LE_ATDEFS_PARAMETER_MAX_BYTES] = {0};
    uint16_t timers[LE_AVC_NUM_RETRY_TIMERS] = {0};
    int timerVal;
    le_result_t result;

    if ((count > LE_AVC_NUM_RETRY_TIMERS) || (count < MIN_RETRY_TIMERS_COUNT))
    {
        LE_ERROR("Unexpected number of retryTimers to configure: %d. Allowed range: %d to %d",
                 count,
                 MIN_RETRY_TIMERS_COUNT,
                 LE_AVC_NUM_RETRY_TIMERS);
        return LE_FAULT;
    }

    size_t numTimers = LE_AVC_NUM_RETRY_TIMERS;
    result = le_avc_GetRetryTimers(timers, &numTimers);

    // Read the current configuration
    if ((LE_OK != result) || (LE_AVC_NUM_RETRY_TIMERS != numTimers))
    {
        LE_ERROR("Failed to get retry timers");
        return LE_FAULT;
    }

    // Set only number of retryTimers that are supplied. Others keep intact.
    for (i=0; i<count; i++)
    {
        memset(param, 0, LE_ATDEFS_PARAMETER_MAX_BYTES);
        result = le_atServer_GetParameter(cmdRef, i+1, param, LE_ATDEFS_PARAMETER_MAX_BYTES);
        if (LE_OK != result)
        {
            LE_ERROR("failed to get retry timer %d", i);
            return LE_FAULT;
        }
        result = GetInt(param, &timerVal);
        if (LE_OK != result)
        {
            LE_ERROR("Failed to get timer value: %s", LE_RESULT_TXT(result));
            return result;
        }
        if (RETRY_TIMER_MAX < timerVal)
        {
            LE_ERROR("timer value cannot exceed %d", RETRY_TIMER_MAX);
            return LE_FAULT;
        }

        timers[i] = (uint16_t)timerVal;
    }

    return le_avc_SetRetryTimers(timers, numTimers);
}

//--------------------------------------------------------------------------------------------------
/**
 * Get retry timers configuration from AVC
 */
//--------------------------------------------------------------------------------------------------
static char *GetRetryTimers
(
    void
)
{
    uint16_t timers[LE_AVC_NUM_RETRY_TIMERS];
    size_t numTimers = LE_AVC_NUM_RETRY_TIMERS;
    static char resp[RETRY_TIMERS_LEN] = {0};
    int size = 0, i;

    if (LE_OK != le_avc_GetRetryTimers(timers, &numTimers))
    {
        LE_ERROR("Failed to get retry timers");
        return "0";
    }

    for (i=0; i<numTimers; i++)
    {
        size += snprintf(resp+size, RETRY_TIMERS_LEN-size, "%u,", timers[i]);
    }

    resp[size-1] = '\0';

    return resp;
}

//--------------------------------------------------------------------------------------------------
/**
 * Handle user agreement
 */
//--------------------------------------------------------------------------------------------------
static le_result_t HandleUserAgreement
(
    int config,
    int userAgreement
)
{
    le_result_t result = LE_OK;

    if ((0 > userAgreement) || (1 < userAgreement))
    {
        LE_ERROR("Invalid user agreement value %d", userAgreement);
        return LE_FAULT;
    }

    switch (config)
    {
        case CONFIG_CONNECT:
            result = le_avc_SetUserAgreement(LE_AVC_USER_AGREEMENT_CONNECTION,
                                             (bool)userAgreement);
            break;
        case CONFIG_DOWNLOAD:
            result = le_avc_SetUserAgreement(LE_AVC_USER_AGREEMENT_DOWNLOAD,
                                             (bool)userAgreement);
            break;
        case CONFIG_INSTALL:
            result = le_avc_SetUserAgreement(LE_AVC_USER_AGREEMENT_INSTALL,
                                             (bool)userAgreement);
            break;
        case CONFIG_UNINSTALL:
            result = le_avc_SetUserAgreement(LE_AVC_USER_AGREEMENT_UNINSTALL,
                                             (bool)userAgreement);
            break;
        case CONFIG_REBOOT:
            result = le_avc_SetUserAgreement(LE_AVC_USER_AGREEMENT_REBOOT,
                                             (bool)userAgreement);
            break;
        default:
            LE_ERROR("Invalid user agreement configuration %d", config);
            result = LE_FAULT;
            break;
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * handle configuration modes
 */
//--------------------------------------------------------------------------------------------------
static le_result_t HandleConfigModes
(
    le_atServer_CmdRef_t cmdRef,
    uint32_t count
)
{
    char param[LE_ATDEFS_PARAMETER_MAX_BYTES]  = { 0 };
    le_result_t result;
    int mode;
    int timer;
    int userAgreement;

    result = le_atServer_GetParameter(cmdRef, 0, param, LE_ATDEFS_PARAMETER_MAX_BYTES);
    if (LE_OK != result)
    {
        LE_ERROR("failed to get parameter");
        return LE_FAULT;
    }

    result = GetInt(param, &mode);
    if (LE_OK != result)
    {
        LE_ERROR("Failed to get config mode: %s", LE_RESULT_TXT(result));
        return result;
    }

    switch (mode)
    {
        case CONFIG_POLL:
            if (!VERIFY_PARAMETERS_COUNT(count, WDSC_REG_PARAMS_COUNT))
            {
                LE_ERROR("Too many parameter. Provided: %d, Max allowed: %d",
                         count,
                         WDSC_REG_PARAMS_COUNT);
                return LE_FAULT;
            }
            memset(param, 0, LE_ATDEFS_PARAMETER_MAX_BYTES);
            result = le_atServer_GetParameter(cmdRef, 1, param, LE_ATDEFS_PARAMETER_MAX_BYTES);
            if (LE_OK != result)
            {
                LE_ERROR("failed to get polling timer");
                break;
            }
            result = GetInt(param, &timer);
            if (LE_OK != result)
            {
                LE_ERROR("Failed to get polling timer value: %s", LE_RESULT_TXT(result));
                break;
            }
            result = le_avc_SetPollingTimer((uint32_t)timer);
            break;
        case CONFIG_RETRY:
            if (!VERIFY_PARAMETERS_COUNT(count, WDSC_RETRY_PARAMS_COUNT))
            {
                LE_ERROR("Too many parameter. Provided: %d, Max allowed: %d",
                         count,
                         WDSC_RETRY_PARAMS_COUNT);
                return LE_FAULT;
            }
            result = SetRetryTimers(cmdRef, (count-1));
            break;
        case CONFIG_CONNECT:
        case CONFIG_DOWNLOAD:
        case CONFIG_INSTALL:
        case CONFIG_REBOOT:
        case CONFIG_UNINSTALL:
            if (!VERIFY_PARAMETERS_COUNT(count, WDSC_REG_PARAMS_COUNT))
            {
                LE_ERROR("Too many parameter. Provided: %d, Max allowed: %d",
                         count,
                         WDSC_REG_PARAMS_COUNT);
                return LE_FAULT;
            }
            memset(param, 0, LE_ATDEFS_PARAMETER_MAX_BYTES);
            result = le_atServer_GetParameter(cmdRef, 1, param, LE_ATDEFS_PARAMETER_MAX_BYTES);
            if (LE_OK != result)
            {
                LE_ERROR("failed to set user agreement");
                break;
            }
            result = GetInt(param, &userAgreement);
            if (LE_OK != result)
            {
                LE_ERROR("Failed to get user agreement: %s", LE_RESULT_TXT(result));
                break;
            }
            result = HandleUserAgreement(mode, userAgreement);
        break;
        default:
            result = LE_FAULT;
            break;
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * handle session modes
 */
//--------------------------------------------------------------------------------------------------
static le_result_t HandleSessionModes
(
    le_atServer_CmdRef_t cmdRef,
    uint32_t count
)
{
    char param[LE_ATDEFS_PARAMETER_MAX_BYTES]  = { 0 };
    le_result_t result;
    int mode;

    result = le_atServer_GetParameter(cmdRef, 0, param, LE_ATDEFS_PARAMETER_MAX_BYTES);
    if (LE_OK != result)
    {
        LE_ERROR("failed to get parameter");
        return LE_FAULT;
    }

    result = GetInt(param, &mode);
    if (LE_OK != result)
    {
        LE_ERROR("Failed to get session mode: %s", LE_RESULT_TXT(result));
        return result;
    }

    switch (mode)
    {
        case 1:
            if (!VERIFY_PARAMETERS_COUNT(count, WDSS_SESSION_ON_OFF_PARAMS_COUNT))
            {
                LE_ERROR("Too many parameter. Provided: %d, Max allowed: %d",
                         count,
                         WDSS_SESSION_ON_OFF_PARAMS_COUNT);
                return LE_FAULT;
            }
            result = HandleAvcSession(cmdRef);
            break;

        case 2:
            if (!VERIFY_PARAMETERS_COUNT(count, WDSS_CID_SET_PARAMS_COUNT))
            {
                LE_ERROR("Too many parameter. Provided: %d, Max allowed: %d",
                         count,
                         WDSS_CID_SET_PARAMS_COUNT);
                return LE_FAULT;
            }
            result = SetCellularProfileIndex(cmdRef, count);
            break;

        default:
            result = LE_FAULT;
            break;
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * handle LwM2M objects
 */
//--------------------------------------------------------------------------------------------------
static le_result_t HandleLwm2mObjects
(
    le_atServer_CmdRef_t cmdRef,
    uint32_t count
)
{
    char param[LE_ATDEFS_PARAMETER_MAX_BYTES]  = { 0 };
    char rsp[LE_ATDEFS_RESPONSE_MAX_BYTES] = { 0 };
    le_result_t result;
    int operation;
    int action;
    int objectId = 0;
    int objectInstanceId = 0;
    int resourceId = 0;
    int resourceInstanceId = 0;
    char* dataPtr;
    char* relativeDataPtr;

    // Read the operation
    result = le_atServer_GetParameter(cmdRef, 0, param, LE_ATDEFS_PARAMETER_MAX_BYTES);
    if (LE_OK != result)
    {
        LE_ERROR("failed to get parameter");
        return LE_FAULT;
    }
    result = GetInt(param, &operation);
    if (LE_OK != result)
    {
        LE_ERROR("Failed to operation: %s", LE_RESULT_TXT(result));
        return result;
    }

    // Read the action
    result = le_atServer_GetParameter(cmdRef, 1, param, LE_ATDEFS_PARAMETER_MAX_BYTES);
    if (LE_OK != result)
    {
        LE_ERROR("failed to get parameter");
        return LE_FAULT;
    }
    result = GetInt(param, &action);
    if (LE_OK != result)
    {
        LE_ERROR("Failed to operation: %s", LE_RESULT_TXT(result));
        return result;
    }

    // Read the object path
    result = le_atServer_GetParameter(cmdRef, 2, param, LE_ATDEFS_PARAMETER_MAX_BYTES);
    if (LE_OK != result)
    {
        LE_ERROR("failed to get parameter");
        return LE_FAULT;
    }

    // Decode the object path
    relativeDataPtr = strtok_r(param, "/", &dataPtr);
    result = GetInt(relativeDataPtr, &objectId);
    if (LE_OK != result)
    {
        LE_ERROR("Failed to operation: %s", LE_RESULT_TXT(result));
        return result;
    }

    relativeDataPtr = strtok_r(NULL, "/", &dataPtr);
    result = GetInt(relativeDataPtr, &objectInstanceId);
    if (LE_OK != result)
    {
        LE_ERROR("Failed to operation: %s", LE_RESULT_TXT(result));
        return result;
    }

    relativeDataPtr = strtok_r(NULL, "/", &dataPtr);
    result = GetInt(relativeDataPtr, &resourceId);
    if (LE_OK != result)
    {
        LE_ERROR("Failed to operation: %s", LE_RESULT_TXT(result));
        return result;
    }

    relativeDataPtr = strtok_r(NULL, "/", &dataPtr);
    result = GetInt(relativeDataPtr, &resourceInstanceId);
    if (LE_OK != result)
    {
        LE_DEBUG("Ressource Instance ID not provided");
        resourceInstanceId = 0xFFFF;
    }

    LE_INFO("oid=%d oiid=%d rid=%d riid=%d", objectId, objectInstanceId, resourceId,
            resourceInstanceId);

    // Only read action on single ressource instances is allowed
    if ((3 == operation) && (0 == action))
    {
        memset(rsp, 0, LE_ATDEFS_RESPONSE_MAX_BYTES);
        result = le_avc_ReadLwm2mResource(objectId, objectInstanceId, resourceId,
                                          resourceInstanceId, rsp, LE_ATDEFS_RESPONSE_MAX_BYTES);
        if (LE_OK != result)
        {
            return result;
        }

        result = le_atServer_SendIntermediateResponse(cmdRef, rsp);
        if (LE_OK != result)
        {
            LE_ERROR("failed to send intermediate response");
            return LE_FAULT;
        }
        return LE_OK;
    }

    return LE_FAULT;
}

//--------------------------------------------------------------------------------------------------
/**
 * User reply
 */
//--------------------------------------------------------------------------------------------------
static le_result_t HandleUserReply
(
    le_atServer_CmdRef_t cmdRef,
    uint32_t count
)
{
    le_result_t result;
    char param[LE_ATDEFS_PARAMETER_MAX_BYTES] = { 0 };
    Reply_t *replyPtr;

    replyPtr = &Handler.reply;

    result = le_atServer_GetParameter(cmdRef, 0, param, LE_ATDEFS_PARAMETER_MAX_BYTES);
    if (LE_OK != result)
    {
        LE_ERROR("Failed to get reply type %s", LE_RESULT_TXT(result));
        return LE_FAULT;
    }
    result = GetInt(param, (int *)&replyPtr->reply);
    if (LE_OK != result)
    {
        LE_ERROR("Failed to get user reply: %s", LE_RESULT_TXT(result));
        return LE_FAULT;
    }

    switch (replyPtr->reply)
    {
        case REPLY_ACCEPT_CONNECT:
        case REPLY_ACCEPT_DOWNLOAD:
        case REPLY_ACCEPT_INSTALL:
        case REPLY_ACCEPT_UNINSTALL:
        case REPLY_ACCEPT_REBOOT:
            // Check if more parameter is given. If yes return error.
            if (count > WDSR_MAX_PARAMS_COUNT_NO_TIMER)
            {
                LE_ERROR("Too many params. Max allowed: %d, Provided: %d",
                         WDSR_MAX_PARAMS_COUNT_NO_TIMER,
                         count);
                return LE_FAULT;
            }
            break;
        case REPLY_DELAY_CONNECT:
        case REPLY_DELAY_DOWNLOAD:
        case REPLY_DELAY_INSTALL:
        case REPLY_DELAY_UNINSTALL:
        case REPLY_DELAY_REBOOT:
            // No need to check parameter number. It is already done in called function.
            // Check whether any timer value is specified or not.
            result = le_atServer_GetParameter(cmdRef, 1, param, LE_ATDEFS_PARAMETER_MAX_BYTES);
            if (LE_OK == result)
            {
                // Timer value is specified
                result = GetInt(param, &replyPtr->timer);
                if (LE_OK != result)
                {
                    LE_ERROR("Failed to get defer timer: %s", LE_RESULT_TXT(result));
                    return LE_FAULT;
                }
            }
            else
            {
                replyPtr->timer = REPLY_TIMER_DEFAULT;
            }

            if (!replyPtr->timer)
            {
                LE_ERROR("Not possible to refuse user agreement");
                return LE_FAULT;
            }

            if ((replyPtr->timer < 0) || (replyPtr->timer > REPLY_TIMER_MAX))
            {
                LE_ERROR("Defer timer value out of range. Provided: %d, Allowed: 1 - %d",
                         replyPtr->timer,
                         REPLY_TIMER_MAX);
                return LE_FAULT;
            }
            break;
        default:
            LE_ERROR("Provided unknown reply option: %d", replyPtr->reply);
            return LE_FAULT;

    }

    LE_DEBUG("user reply %d", replyPtr->reply);
    LE_DEBUG("reply timer %d", replyPtr->timer);

    switch (replyPtr->reply)
    {
       case REPLY_ACCEPT_CONNECT:
            result = le_avc_StartSession();
            break;
        case REPLY_DELAY_CONNECT:
            result = le_avc_DeferConnect((uint32_t)replyPtr->timer);
            break;
        case REPLY_ACCEPT_DOWNLOAD:
            result = le_avc_AcceptDownload();
            break;
        case REPLY_DELAY_DOWNLOAD:
            result = le_avc_DeferDownload((uint32_t)replyPtr->timer);
            break;
        case REPLY_ACCEPT_INSTALL:
            result = le_avc_AcceptInstall();
            break;
        case REPLY_DELAY_INSTALL:
            result = le_avc_DeferInstall((uint32_t)replyPtr->timer);
            break;
        case REPLY_ACCEPT_UNINSTALL:
            result = le_avc_AcceptUninstall();
            break;
        case REPLY_DELAY_UNINSTALL:
            result = le_avc_DeferUninstall((uint32_t)replyPtr->timer);
            break;
        case REPLY_ACCEPT_REBOOT:
            result = le_avc_AcceptReboot();
            break;
        case REPLY_DELAY_REBOOT:
            result = le_avc_DeferReboot((uint32_t)replyPtr->timer);
            break;
        default:
            result = LE_FAULT;
            break;
    }

    replyPtr->timer = -1;

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * AT+WDSC - Device Services Configuration
 *
 * Allows a user to configure:
 *     - User agreement for connection, package download and package install.
 *     - Polling mode to make a connection to the Device Services server.
 *     - Retry mode to attempt a new connection to the server when the WWAN DATA
 *      service is temporarily out of order or when an http/coap error occurs
 */
//--------------------------------------------------------------------------------------------------
static void WdscCmdHandler
(
    le_atServer_CmdRef_t cmdRef,
    le_atServer_Type_t type,
    uint32_t count,
    void* ctxPtr
)
{
    char name[LE_ATDEFS_COMMAND_MAX_BYTES] = { 0 };
    char rsp[LE_ATDEFS_RESPONSE_MAX_BYTES] = { 0 };
    uint32_t pollingTimer = CONFIG_DISABLED;
    le_result_t result;
    bool userAgreementConfig;

    result = le_atServer_GetCommandName(cmdRef, name, LE_ATDEFS_COMMAND_MAX_BYTES);
    if (LE_OK != result)
    {
        LE_ERROR("failed to get command name");
        goto at_err;
    }

    switch (type)
    {
        case LE_ATSERVER_TYPE_PARA:
            // No need to check parameter count here, it will be checked inside HandleConfigModes()
            // function.
            result = HandleConfigModes(cmdRef, count);
            if (LE_OK != result)
            {
                LE_ERROR("failed with %s", LE_RESULT_TXT(result));
                goto at_err;
            }
            break;
        case LE_ATSERVER_TYPE_READ:
            result = le_avc_GetUserAgreement(LE_AVC_USER_AGREEMENT_CONNECTION, &userAgreementConfig);
            if (LE_OK != result)
            {
                LE_ERROR("failed to get user agreement configuration");
                goto at_err;
            }
            memset(rsp, 0, LE_ATDEFS_RESPONSE_MAX_BYTES);
            snprintf(rsp, LE_ATDEFS_RESPONSE_MAX_BYTES, "%s: %d,%d", name+2,
                CONFIG_CONNECT, userAgreementConfig);
            result = le_atServer_SendIntermediateResponse(cmdRef, rsp);
            if (LE_OK != result)
            {
                LE_ERROR("failed to send intermediate response");
                goto at_err;
            }
            result = le_avc_GetUserAgreement(LE_AVC_USER_AGREEMENT_DOWNLOAD, &userAgreementConfig);
            if (LE_OK != result)
            {
                LE_ERROR("failed to get user agreement configuration");
                goto at_err;
            }
            memset(rsp, 0, LE_ATDEFS_RESPONSE_MAX_BYTES);
            snprintf(rsp, LE_ATDEFS_RESPONSE_MAX_BYTES, "%s: %d,%d", name+2,
                CONFIG_DOWNLOAD, userAgreementConfig);
            result = le_atServer_SendIntermediateResponse(cmdRef, rsp);
            if (LE_OK != result)
            {
                LE_ERROR("failed to send intermediate response");
                goto at_err;
            }
            result = le_avc_GetUserAgreement(LE_AVC_USER_AGREEMENT_INSTALL, &userAgreementConfig);
            if (LE_OK != result)
            {
                LE_ERROR("failed to get user agreement configuration");
                goto at_err;
            }
            memset(rsp, 0, LE_ATDEFS_RESPONSE_MAX_BYTES);
            snprintf(rsp, LE_ATDEFS_RESPONSE_MAX_BYTES, "%s: %d,%d", name+2,
                CONFIG_INSTALL, userAgreementConfig);
            result = le_atServer_SendIntermediateResponse(cmdRef, rsp);
            if (LE_OK != result)
            {
                LE_ERROR("failed to send intermediate response");
                goto at_err;
            }
            if (LE_OK != le_avc_GetPollingTimer(&pollingTimer))
            {
                pollingTimer = CONFIG_DISABLED;
            }
            memset(rsp, 0, LE_ATDEFS_RESPONSE_MAX_BYTES);
            snprintf(rsp, LE_ATDEFS_RESPONSE_MAX_BYTES, "%s: %d,%u", name+2,
                CONFIG_POLL, pollingTimer);
            result = le_atServer_SendIntermediateResponse(cmdRef, rsp);
            if (LE_OK != result)
            {
                LE_ERROR("failed to send intermediate response");
                goto at_err;
            }
            memset(rsp, 0, LE_ATDEFS_RESPONSE_MAX_BYTES);
            snprintf(rsp, LE_ATDEFS_RESPONSE_MAX_BYTES, "%s: %d,%s", name+2,
                CONFIG_RETRY, GetRetryTimers());
            result = le_atServer_SendIntermediateResponse(cmdRef, rsp);
            if (LE_OK != result)
            {
                LE_ERROR("failed to send intermediate response");
                goto at_err;
            }
            result = le_avc_GetUserAgreement(LE_AVC_USER_AGREEMENT_REBOOT, &userAgreementConfig);
            if (LE_OK != result)
            {
                LE_ERROR("failed to get user agreement configuration");
                goto at_err;
            }
            memset(rsp, 0, LE_ATDEFS_RESPONSE_MAX_BYTES);
            snprintf(rsp, LE_ATDEFS_RESPONSE_MAX_BYTES, "%s: %d,%d", name+2,
                CONFIG_REBOOT, userAgreementConfig);
            result = le_atServer_SendIntermediateResponse(cmdRef, rsp);
            if (LE_OK != result)
            {
                LE_ERROR("failed to send intermediate response");
                goto at_err;
            }
            result = le_avc_GetUserAgreement(LE_AVC_USER_AGREEMENT_UNINSTALL, &userAgreementConfig);
            if (LE_OK != result)
            {
                LE_ERROR("failed to get user agreement configuration");
                goto at_err;
            }
            memset(rsp, 0, LE_ATDEFS_RESPONSE_MAX_BYTES);
            snprintf(rsp, LE_ATDEFS_RESPONSE_MAX_BYTES, "%s: %d,%d", name+2,
                CONFIG_UNINSTALL, userAgreementConfig);
            result = le_atServer_SendIntermediateResponse(cmdRef, rsp);
            if (LE_OK != result)
            {
                LE_ERROR("failed to send intermediate response");
                goto at_err;
            }
            break;
        case LE_ATSERVER_TYPE_TEST:
            snprintf(rsp, LE_ATDEFS_RESPONSE_MAX_BYTES, "%s: (0-2,5,6),(0,1)", name+2);
            result = le_atServer_SendIntermediateResponse(cmdRef, rsp);
            if (LE_OK != result)
            {
                LE_ERROR("failed to send intermediate response");
                goto at_err;
            }
            memset(rsp, 0, LE_ATDEFS_RESPONSE_MAX_BYTES);
            snprintf(rsp, LE_ATDEFS_RESPONSE_MAX_BYTES, "%s: 3,(0,%d)", name+2,
                POLL_TIMER_MAX);
            result = le_atServer_SendIntermediateResponse(cmdRef, rsp);
            if (LE_OK != result)
            {
                LE_ERROR("failed to send intermediate response");
                goto at_err;
            }
            memset(rsp, 0, LE_ATDEFS_RESPONSE_MAX_BYTES);
            snprintf(rsp, LE_ATDEFS_RESPONSE_MAX_BYTES, "%1$s: 4,(0,%2$d),(1, %2$d),"
                "(1, %2$d),(1,%2$d),(1,%2$d),(1,%2$d),(1,%2$d),(1,%2$d)",
                name+2, RETRY_TIMER_MAX);
            result = le_atServer_SendIntermediateResponse(cmdRef, rsp);
            if (LE_OK != result)
            {
                LE_ERROR("failed to send intermediate response");
                goto at_err;
            }
            break;
        default:
            goto at_err;
    }

    result = le_atServer_SendFinalResultCode(cmdRef, LE_ATSERVER_OK, "", 0);
    if (LE_OK != result)
    {
        LE_ERROR("failed to send final response");
    }
    return;

at_err:
    result = le_atServer_SendFinalResultCode(cmdRef, LE_ATSERVER_ERROR, LE_ATDEFS_CME_ERROR, 3);
    if (LE_OK != result)
    {
        LE_ERROR("failed to send final response");
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * AT+WDSE - Device Services Error
 *
 * Allows a user to know the last HTTP response received by the device for the
 * package download.
 */
//--------------------------------------------------------------------------------------------------
static void WdseCmdHandler
(
    le_atServer_CmdRef_t cmdRef,
    le_atServer_Type_t type,
    uint32_t count,
    void* ctxPtr
)
{
    char name[LE_ATDEFS_COMMAND_MAX_BYTES] = { 0 };
    char rsp[LE_ATDEFS_RESPONSE_MAX_BYTES] = { 0 };
    uint16_t httpStatus;
    le_result_t result;

    Status_t *statusPtr;
    statusPtr = &Handler.status;

    result = le_atServer_GetCommandName(cmdRef, name, LE_ATDEFS_COMMAND_MAX_BYTES);
    if (LE_OK != result)
    {
        LE_ERROR("failed to get command name");
        goto at_err;
    }

    switch (type)
    {
        case LE_ATSERVER_TYPE_ACT:
            // CME error if the data service is not activated
            if (statusPtr->service != SERVICE_STATUS_ACTIVATED)
            {
                LE_ERROR("Data service not activated");
                goto at_err;
            }

            // OK when session is down
            if (statusPtr->session == SESSION_STATUS_DOWN)
            {
                LE_ERROR("Session not started");
                break;
            }

            // Get the http response code
            httpStatus = le_avc_GetHttpStatus();

            // If http status is invalid that means a download was not started. Skip
            // sending http response code in this case.
            if (httpStatus != LE_AVC_HTTP_STATUS_INVALID)
            {
                // Send http response code.
                snprintf(rsp, LE_ATDEFS_RESPONSE_MAX_BYTES, "%s: %d", name+2, httpStatus);

                result = le_atServer_SendIntermediateResponse(cmdRef, rsp);
                if (LE_OK != result)
                {
                    LE_ERROR("failed to send intermediate response");
                    goto at_err;
                }
            }
        break;
        default:
            goto at_err;
    }

    result = le_atServer_SendFinalResultCode(cmdRef, LE_ATSERVER_OK, "", 0);
    if (LE_OK != result)
    {
        LE_ERROR("failed to send final response");
    }

    return;

at_err:
    result = le_atServer_SendFinalResultCode(cmdRef, LE_ATSERVER_ERROR, LE_ATDEFS_CME_ERROR, 3);
    if (LE_OK != result)
    {
        LE_ERROR("failed to send final response");
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * AT+WDSG - Device Services General status
 *
 * Returns some general status about Device Services.
 */
//--------------------------------------------------------------------------------------------------
static void WdsgCmdHandler
(
    le_atServer_CmdRef_t cmdRef,
    le_atServer_Type_t type,
    uint32_t count,
    void* ctxPtr
)
{
    char name[LE_ATDEFS_COMMAND_MAX_BYTES] = { 0 };
    char rsp[LE_ATDEFS_RESPONSE_MAX_BYTES] = { 0 };
    le_result_t result;
    Status_t *statusPtr;

    statusPtr = &Handler.status;

    // Get the credential status
    le_avc_CredentialStatus_t status = le_avc_GetCredentialStatus();
    statusPtr->service = ConvertToWdsStatus(status);

    result = le_atServer_GetCommandName(cmdRef, name, LE_ATDEFS_COMMAND_MAX_BYTES);
    if (LE_OK != result)
    {
        LE_ERROR("failed to get command name");
        goto at_err;
    }

    switch (type)
    {
        case LE_ATSERVER_TYPE_ACT:
            snprintf(rsp, LE_ATDEFS_RESPONSE_MAX_BYTES, "%s: %d,%d", name+2,
                DEVICE_SERVICES, statusPtr->service);
            result = le_atServer_SendIntermediateResponse(cmdRef, rsp);
            if (LE_OK != result)
            {
                LE_ERROR("failed to send intermediate response");
                goto at_err;
            }
            snprintf(rsp, LE_ATDEFS_RESPONSE_MAX_BYTES, "%s: %d,%d", name+2,
                SESSION_PKG, statusPtr->session);
            result = le_atServer_SendIntermediateResponse(cmdRef, rsp);
            if (LE_OK != result)
            {
                LE_ERROR("failed to send intermediate response");
                goto at_err;
            }
            break;
        case LE_ATSERVER_TYPE_TEST:
            break;
        default:
            goto at_err;
    }

    result = le_atServer_SendFinalResultCode(cmdRef, LE_ATSERVER_OK, "", 0);
    if (LE_OK != result)
    {
        LE_ERROR("failed to send final response");
        goto at_err;
    }

    return;

at_err:
    result = le_atServer_SendFinalResultCode(cmdRef, LE_ATSERVER_ERROR, LE_ATDEFS_CME_ERROR, 3);
    if (LE_OK != result)
    {
        LE_ERROR("failed to send final response");
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * AT+WDSR - Device Services Reply
 *
 * Allows a user to respond to the Device Services server request when user
 * agreement is requested for connection, download and/or package install.
 */
//--------------------------------------------------------------------------------------------------
static void WdsrCmdHandler
(
    le_atServer_CmdRef_t cmdRef,
    le_atServer_Type_t type,
    uint32_t count,
    void* contextPtr
)
{
    char name[LE_ATDEFS_COMMAND_MAX_BYTES] = { 0 };
    char rsp[LE_ATDEFS_RESPONSE_MAX_BYTES] = { 0 };
    le_result_t result;

    result = le_atServer_GetCommandName(cmdRef, name, LE_ATDEFS_COMMAND_MAX_BYTES);
    if (LE_OK != result)
    {
        LE_ERROR("failed to get command name");
        goto at_err;
    }

    switch (type)
    {
        case LE_ATSERVER_TYPE_PARA:
            if ((count > WDSR_MAX_PARAMS_COUNT_WITH_TIMER) || (count < WDSR_MIN_PARAMS_COUNT))
            {
                LE_ERROR("Unexpected number of params: %d, "
                         "Number of allowed params (min-max): %d - %d",
                          count,
                          WDSR_MIN_PARAMS_COUNT,
                          WDSR_MAX_PARAMS_COUNT_WITH_TIMER);
                goto at_err;
            }
            if (LE_OK != HandleUserReply(cmdRef, count))
            {
                goto at_err;
            }
            break;
        case LE_ATSERVER_TYPE_TEST:
            memset(rsp, 0, LE_ATDEFS_RESPONSE_MAX_BYTES);
            snprintf(rsp, LE_ATDEFS_RESPONSE_MAX_BYTES, "%s: (0-%d),(0-%d)",
                name+2, REPLY_MAX, REPLY_TIMER_MAX);
            result = le_atServer_SendIntermediateResponse(cmdRef, rsp);
            if (LE_OK != result)
            {
                LE_ERROR("failed to send intermediate response");
                goto at_err;
            }
            break;
        default:
            goto at_err;
    }

    result = le_atServer_SendFinalResultCode(cmdRef, LE_ATSERVER_OK, "", 0);
    if (LE_OK != result)
    {
        LE_ERROR("failed to send final response");
    }

    return;

at_err:
    result = le_atServer_SendFinalResultCode(cmdRef, LE_ATSERVER_ERROR, LE_ATDEFS_CME_ERROR, 3);
    if (LE_OK != result)
    {
        LE_ERROR("failed to send final response");
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * AT+WDSS - Device Services Session
 *
 * Allows a user to configure a dedicated NAP and to initiate a connection to
 * Device Services server.
 * This command is also used to activate an automatic registration to the
 * AirVantage server.
 */
//--------------------------------------------------------------------------------------------------
static void WdssCmdHandler
(
    le_atServer_CmdRef_t cmdRef,
    le_atServer_Type_t type,
    uint32_t count,
    void* ctxPtr
)
{
    char rsp[LE_ATDEFS_RESPONSE_MAX_BYTES] = { 0 };
    char name[LE_ATDEFS_COMMAND_MAX_BYTES] = { 0 };
    Session_t *sessionPtr;
    le_result_t result;
    int32_t index;

    sessionPtr = &Handler.session;

    result = le_atServer_GetCommandName(cmdRef, name, LE_ATDEFS_COMMAND_MAX_BYTES);
    if (LE_OK != result)
    {
        LE_ERROR("failed to get command name");
        goto at_err;
    }

    switch (type)
    {
        case LE_ATSERVER_TYPE_PARA:
            // No need to check parameter count here, it will be checked inside HandleSessionModes()
            // function.
            result = HandleSessionModes(cmdRef, count);
            if (LE_OK != result)
            {
                LE_ERROR("failed with %s", LE_RESULT_TXT(result));
                goto at_err;
            }
            break;
        case LE_ATSERVER_TYPE_TEST:
            memset(rsp, 0, LE_ATDEFS_RESPONSE_MAX_BYTES);
            snprintf(rsp, LE_ATDEFS_RESPONSE_MAX_BYTES, "%s: 1,(0-1)", name+2);
            result = le_atServer_SendIntermediateResponse(cmdRef, rsp);
            if (LE_OK != result)
            {
                LE_ERROR("failed to send intermediate response");
                goto at_err;
            }
            memset(rsp, 0, LE_ATDEFS_RESPONSE_MAX_BYTES);
            snprintf(rsp, LE_ATDEFS_RESPONSE_MAX_BYTES, "%s: 2,(1-%d)",
                     name+2, le_mdc_NumProfiles());
            result = le_atServer_SendIntermediateResponse(cmdRef, rsp);
            if (LE_OK != result)
            {
                LE_ERROR("failed to send intermediate response");
                goto at_err;
            }

            break;
        case LE_ATSERVER_TYPE_READ:
            memset(rsp, 0, LE_ATDEFS_RESPONSE_MAX_BYTES);
            snprintf(rsp, LE_ATDEFS_RESPONSE_MAX_BYTES, "%s: 1,%d",
                     name+2, sessionPtr->action);
            result = le_atServer_SendIntermediateResponse(cmdRef, rsp);
            if (LE_OK != result)
            {
                LE_ERROR("failed to send intermediate response");
                goto at_err;
            }

            index = le_data_GetCellularProfileIndex();
            memset(rsp, 0, LE_ATDEFS_RESPONSE_MAX_BYTES);
            snprintf(rsp, LE_ATDEFS_RESPONSE_MAX_BYTES, "%s: 2,%d", name+2, index);
            result = le_atServer_SendIntermediateResponse(cmdRef, rsp);
            if (LE_OK != result)
            {
                LE_ERROR("failed to send intermediate response");
                goto at_err;
            }

            break;
        default:
            goto at_err;
    }

    result = le_atServer_SendFinalResultCode(cmdRef, LE_ATSERVER_OK, "", 0);
    if (LE_OK != result)
    {
        LE_ERROR("failed to send final response");
    }

    return;

at_err:
    result = le_atServer_SendFinalResultCode(cmdRef, LE_ATSERVER_ERROR, LE_ATDEFS_CME_ERROR, 3);
    if (LE_OK != result)
    {
        LE_ERROR("failed to send final response");
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * AT+WDSI - Device Services Indications
 *
 * Allows user to request some unsolicited indication for Device Services.
 */
//--------------------------------------------------------------------------------------------------
static void WdsiCmdHandler
(
    le_atServer_CmdRef_t cmdRef,
    le_atServer_Type_t type,
    uint32_t count,
    void* ctxPtr
)
{
    char name[LE_ATDEFS_COMMAND_MAX_BYTES] = { 0 };
    char param[LE_ATDEFS_PARAMETER_MAX_BYTES] = { 0 };
    char rsp[LE_ATDEFS_RESPONSE_MAX_BYTES] = { 0 };
    le_result_t result;
    Indications_t *indPtr;
    int level;

    indPtr = &Handler.indications;

    result = le_atServer_GetCommandName(cmdRef, name, LE_ATDEFS_COMMAND_MAX_BYTES);
    if (LE_OK != result)
    {
        LE_ERROR("failed to get command name");
        goto at_err;
    }

    switch (type)
    {
        case LE_ATSERVER_TYPE_PARA:
            if (!VERIFY_PARAMETERS_COUNT(count, WDSI_MAX_PARAMS_COUNT))
            {
                goto at_err;
            }
            result = le_atServer_GetParameter(cmdRef, 0, param, LE_ATDEFS_PARAMETER_MAX_BYTES);
            if (LE_OK != result)
            {
                LE_ERROR("failed to get parameter");
                goto at_err;
            }
            result = GetInt(param, &level);
            if (LE_OK != result)
            {
                LE_ERROR("Failed to get indications level: %s", LE_RESULT_TXT(result));
                goto at_err;
            }
            if (-1 == SetIndications((uint16_t)level))
            {
                goto at_err;
            }
            break;
        case LE_ATSERVER_TYPE_TEST:
            snprintf(rsp, LE_ATDEFS_RESPONSE_MAX_BYTES, "%s: (0-%d)",
                name+2, SUPPORTED_IND_MASK);
            result = le_atServer_SendIntermediateResponse(cmdRef, rsp);
            if (LE_OK != result)
            {
                LE_ERROR("failed to send intermediate response");
                goto at_err;
            }
            break;
        case LE_ATSERVER_TYPE_READ:
            snprintf(rsp, LE_ATDEFS_RESPONSE_MAX_BYTES, "%s: %d", name+2, indPtr->level);
            result = le_atServer_SendIntermediateResponse(cmdRef, rsp);
            if (LE_OK != result)
            {
                LE_ERROR("failed to send intermediate response");
                goto at_err;
            }
            break;
        default:
            goto at_err;
    }

    result = le_atServer_SendFinalResultCode(cmdRef, LE_ATSERVER_OK, "", 0);
    if (LE_OK != result)
    {
        LE_ERROR("failed to send final response");
    }

    return;

at_err:
    result = le_atServer_SendFinalResultCode(cmdRef, LE_ATSERVER_ERROR, LE_ATDEFS_CME_ERROR, 3);
    if (LE_OK != result)
    {
        LE_ERROR("failed to send final response");
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * AT+WDSW - Allows user to configure LwM2M AirVantage objects.
 *
 * @note Only read operation is currently supported
 */
//--------------------------------------------------------------------------------------------------
static void WdswCmdHandler
(
    le_atServer_CmdRef_t cmdRef,
    le_atServer_Type_t type,
    uint32_t count,
    void* ctxPtr
)
{
    char name[LE_ATDEFS_COMMAND_MAX_BYTES] = { 0 };
    le_result_t result;

    result = le_atServer_GetCommandName(cmdRef, name, LE_ATDEFS_COMMAND_MAX_BYTES);
    if (LE_OK != result)
    {
        LE_ERROR("failed to get command name");
        goto at_err;
    }

    switch (type)
    {
        case LE_ATSERVER_TYPE_PARA:
            if (!VERIFY_PARAMETERS_COUNT(count, WDSW_MAX_PARAMS_COUNT))
            {
                goto at_err;
            }
            result = HandleLwm2mObjects(cmdRef, count);
            if (LE_OK != result)
            {
                LE_ERROR("failed with %s", LE_RESULT_TXT(result));
                goto at_err;
            }
            break;

        case LE_ATSERVER_TYPE_TEST:
        case LE_ATSERVER_TYPE_READ:
        default:
            goto at_err;
    }

    result = le_atServer_SendFinalResultCode(cmdRef, LE_ATSERVER_OK, "", 0);
    if (LE_OK != result)
    {
        LE_ERROR("failed to send final response");
    }

    return;

at_err:
    result = le_atServer_SendFinalResultCode(cmdRef, LE_ATSERVER_ERROR, LE_ATDEFS_CME_ERROR, 3);
    if (LE_OK != result)
    {
        LE_ERROR("failed to send final response");
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Check that device services are initialized by verifying that the platform is registered on
 * network and APN configured
 */
//--------------------------------------------------------------------------------------------------
static void CheckDeviceServices
(
    void
)
{
    le_mrc_NetRegState_t regState;
    ApnConf_t apnConf;

    // Check that the platform is registered on network
    if (LE_OK != le_mrc_GetNetRegState(&regState))
    {
        LE_ERROR("Unable to retrieve network registration state");
        return;
    }

    LE_DEBUG("Registration state: %d", regState);

    if ((LE_MRC_REG_HOME != regState) && (LE_MRC_REG_ROAMING != regState))
    {
        return;
    }

    // Check that an APN is correctly filled and configured
    if (LE_OK != GetApnConfig(NULL, &apnConf))
    {
        LE_ERROR("Unable to retrieve APN");
        return;
    }

    LE_DEBUG("Current APN: %s", apnConf.apn);

    if (strnlen(apnConf.apn, sizeof(apnConf.apn)))
    {
        ReportEvent(EVENT_INIT, -1);
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Network registration status event handler
 */
//--------------------------------------------------------------------------------------------------
static void NetRegHandler
(
    le_mrc_NetRegState_t state,
    void* contextPtr
)
{
    if ((LE_MRC_REG_HOME == state) || (LE_MRC_REG_ROAMING == state))
    {
        CheckDeviceServices();
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Initialize atAirVantage app
 */
//--------------------------------------------------------------------------------------------------
COMPONENT_INIT
{
    int i = 0;
    Indications_t *indPtr;
    atCmd_t AtCmds[] = {
        {
            .atCmdPtr = "AT+WDSC",
            .handlerPtr = WdscCmdHandler,
        },
        {
            .atCmdPtr = "AT+WDSE",
            .handlerPtr = WdseCmdHandler,
        },
        {
            .atCmdPtr = "AT+WDSG",
            .handlerPtr = WdsgCmdHandler,
        },
        {
            .atCmdPtr = "AT+WDSR",
            .handlerPtr = WdsrCmdHandler,
        },
        {
            .atCmdPtr = "AT+WDSS",
            .handlerPtr = WdssCmdHandler,
        },
        {
            .atCmdPtr = "AT+WDSI",
            .handlerPtr = WdsiCmdHandler,
        },
        {
            .atCmdPtr = "AT+WDSW",
            .handlerPtr = WdswCmdHandler,
        }
    };

    indPtr = &Handler.indications;

    if (LE_OK != ReadFs(INDICATIONS_FILE, (uint8_t *)&indPtr->level, sizeof(indPtr->level)))
    {
        LE_ERROR("Failed to read indications level");
        indPtr->level = LEVEL_NO_INDICATIONS;
    }

    SetIndications(indPtr->level);

    while (i < NUM_ARRAY_MEMBERS(AtCmds))
    {
        AtCmds[i].cmdRef = le_atServer_Create(AtCmds[i].atCmdPtr);
        if (!AtCmds[i].cmdRef)
        {
            LE_ERROR("failed to create command %s", AtCmds[i].atCmdPtr);
        }

        le_atServer_AddCommandHandler(AtCmds[i].cmdRef, AtCmds[i].handlerPtr, NULL);
        i++;
    }

    le_avc_AddStatusEventHandler(AvcStatus, NULL);

    le_mrc_AddNetRegStateEventHandler(NetRegHandler, NULL);

    CheckDeviceServices();
}
