// -------------------------------------------------------------------------------------------------
/**
 *  WiFi Client Platform Adapter
 *
 *  Copyright (C) Sierra Wireless Inc.
 *
 */
// -------------------------------------------------------------------------------------------------
#include <sys/types.h>
#include <sys/wait.h>

#include "legato.h"

#include "interfaces.h"

#include "pa_wifi.h"
#include "stdio.h"

//--------------------------------------------------------------------------------------------------
/**
 * WiFi platform adaptor shell script
 */
//--------------------------------------------------------------------------------------------------
#define WIFI_SCRIPT_PATH "/legato/systems/current/apps/wifiService/read-only/pa_wifi.sh "

// Set of commands to drive the WiFi features.
#define COMMAND_WIFI_HW_START "wlan0 WIFI_START"
#define COMMAND_WIFI_HW_STOP "wlan0 WIFI_STOP" /* not sure that this works */
#define COMMAND_WIFI_WLAN_UP "wlan0 WIFI_WLAN_UP"
#define COMMAND_WIFI_SET_EVENT "wlan0 WIFI_SET_EVENT"
#define COMMAND_WIFICLIENT_START_SCAN "wlan0 WIFICLIENT_START_SCAN"
#define COMMAND_WIFICLIENT_DISCONNECT "wlan0 WIFICLIENT_DISCONNECT"
#define COMMAND_WIFICLIENT_CONNECT_SECURITY_NONE
    "wlan0 WIFICLIENT_CONNECT_SECURITY_NONE \"%.*s\" \"%d\""
#define COMMAND_WIFICLIENT_CONNECT_SECURITY_WEP \
    "wlan0 WIFICLIENT_CONNECT_SECURITY_WEP \"%.*s\" \"%d\" \"%s\""
#define COMMAND_WIFICLIENT_CONNECT_SECURITY_WPA_PSK_PERSONAL \
    "wlan0 WIFICLIENT_CONNECT_SECURITY_WPA_PSK_PERSONAL \"%.*s\" \"%s\" \"%s\""
#define COMMAND_WIFICLIENT_CONNECT_SECURITY_WPA2_PSK_PERSONAL \
    "wlan0 WIFICLIENT_CONNECT_SECURITY_WPA2_PSK_PERSONAL \"%.*s\" \"%s\" \"%s\""
#define COMMAND_WIFICLIENT_CONNECT_SECURITY_WPA_EAP_PEAP0_ENTERPRISE \
    "wlan0 WIFICLIENT_CONNECT_SECURITY_WPA_EAP_PEAP0_ENTERPRISE \"%.*s\" \"%d\" \"%s\" \"%s\""
#define COMMAND_WIFICLIENT_CONNECT_SECURITY_WPA2_EAP_PEAP0_ENTERPRISE \
    "wlan0 WIFICLIENT_CONNECT_SECURITY_WPA2_EAP_PEAP0_ENTERPRISE \"%.*s\" \"%d\" \"%s\" \"%s\""
#define COMMAND_WIFICLIENT_CONNECT_WPA_PASSPHRASE \
    "wlan0 WIFICLIENT_CONNECT_WPA_PASSPHRASE \"%.*s\" %s"

//--------------------------------------------------------------------------------------------------
/**
 * The current security protocol.
 */
//--------------------------------------------------------------------------------------------------
static le_wifiClient_SecurityProtocol_t SavedSecurityProtocol;
//--------------------------------------------------------------------------------------------------
/**
 * WEP key used for authentication. Used only with WEP protocol.
 */
//--------------------------------------------------------------------------------------------------
static char SavedWepKey[LE_WIFIDEFS_MAX_WEPKEY_BYTES];
//--------------------------------------------------------------------------------------------------
/**
 * Passphrase used for authentication. Used only with WPA/WPA2 protocol.
 */
//--------------------------------------------------------------------------------------------------
static char SavedPassphrase[LE_WIFIDEFS_MAX_PASSPHRASE_BYTES];
//--------------------------------------------------------------------------------------------------
/**
 * Pre-Shared-Key used for authentication. Used only with WPA/WPA2 protocol.
 */
//--------------------------------------------------------------------------------------------------
static char SavedPreSharedKey[LE_WIFIDEFS_MAX_PSK_BYTES];
//--------------------------------------------------------------------------------------------------
/**
 * Username used for authentication. Used only with WPA/WPA2 protocol with RADIUS server.
 */
//--------------------------------------------------------------------------------------------------
static char SavedUsername[LE_WIFIDEFS_MAX_USERNAME_BYTES];
//--------------------------------------------------------------------------------------------------
/**
 * Password used for authentication. Used only with WPA/WPA2 protocol with RADIUS server.
 */
//--------------------------------------------------------------------------------------------------
static char SavedPassword[LE_WIFIDEFS_MAX_PASSWORD_BYTES];

//--------------------------------------------------------------------------------------------------
/**
 * Indicates if the Access Point SSID is hidden from scan or not
 */
//--------------------------------------------------------------------------------------------------
static bool HiddenAccessPoint = false;

//--------------------------------------------------------------------------------------------------
/**
 * The handle of the input pipe used to be notified of the WiFi events during the scan.
 */
//--------------------------------------------------------------------------------------------------
static FILE *IwScanPipePtr    = NULL;
//--------------------------------------------------------------------------------------------------
/**
 * The handle of the input pipe used to be notified of the WiFi (dis)connections.
 */
//--------------------------------------------------------------------------------------------------
static FILE *IwConnectPipePtr = NULL;
//--------------------------------------------------------------------------------------------------
/**
 * The handle of the input pipe used to be notified of the WiFi events.
 */
//--------------------------------------------------------------------------------------------------
static FILE *IwThreadPipePtr  = NULL;
//--------------------------------------------------------------------------------------------------
/**
 * Flag set when a WiFi scan is in progress.
 */
//--------------------------------------------------------------------------------------------------
static bool  IsScanRunning    = false;


//--------------------------------------------------------------------------------------------------
/**
 * The main thread running the WiFi platform adaptor.
 */
//--------------------------------------------------------------------------------------------------
static void *WifiClientPaThreadMain(void *contextPtr);
//--------------------------------------------------------------------------------------------------
/**
 * The handle of the WiFi platform adaptor thread.
 */
//--------------------------------------------------------------------------------------------------
static le_thread_Ref_t WifiClientPaThread = NULL;


//--------------------------------------------------------------------------------------------------
/**
 * WifiClient state event ID used to report WifiClient state events to the registered event handlers.
 */
//--------------------------------------------------------------------------------------------------
static le_event_Id_t WifiClientPaEvent;

//--------------------------------------------------------------------------------------------------
/**
 * Thread destructor
 */
//--------------------------------------------------------------------------------------------------
static void ThreadDestructor
(
    void *contextPtr
)
{
    if (IwThreadPipePtr)
    {
        // And close FP used in created thread
        pclose(IwThreadPipePtr);
        IwThreadPipePtr = NULL;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * The first-layer WiFi Client Event Handler.
 *
 */
//--------------------------------------------------------------------------------------------------
static void FirstLayerWifiClientEventHandler
(
    void *reportPtr,
    void *secondLayerHandlerFuncPtr
)
{
    pa_wifiClient_NewEventHandlerFunc_t  clientHandlerFunc = secondLayerHandlerFuncPtr;
    le_wifiClient_Event_t               *wifiEventPtr      = (le_wifiClient_Event_t *)reportPtr;

    if (NULL != wifiEventPtr)
    {
        LE_INFO("Event: %d", *wifiEventPtr);
        clientHandlerFunc(*wifiEventPtr, le_event_GetContextPtr());
    }
    else
    {
        LE_ERROR("Event is NULL");
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * WiFi Client PA Thread
 *
 */
//--------------------------------------------------------------------------------------------------
static void *WifiClientPaThreadMain
(
    void *contextPtr
)
{
    char tmpString[] = (WIFI_SCRIPT_PATH COMMAND_WIFI_SET_EVENT);
    char path[1024];

    LE_INFO("Started!");

    // Open the command "iw events" for reading.
    IwThreadPipePtr = popen(tmpString, "r");

    if (NULL == IwThreadPipePtr)
    {
        LE_ERROR("Failed to run command:\"%s\" errno:%d %s",
            (tmpString),
            errno,
            strerror(errno));
        return NULL;
    }

    // Read the output one line at a time - output it.
    while (NULL != fgets(path, sizeof(path) - 1, IwThreadPipePtr))
    {
        LE_INFO("PARSING:%s: len:%d", path, (int) strnlen(path, sizeof(path) - 1));
        if (NULL != strstr(path, "connected to"))
        {
            LE_INFO("FOUND connected");
            // Report event: LE_WIFICLIENT_EVENT_CONNECTED
            le_wifiClient_Event_t event = LE_WIFICLIENT_EVENT_CONNECTED;
            LE_INFO("InternalWifiClientStateEvent event: %d ", event);
            le_event_Report(WifiClientPaEvent, (void *)&event, sizeof(le_wifiClient_Event_t));
        }
        else if (NULL != strstr(path, "disconnected"))
        {
            LE_INFO("FOUND disconnected");
            // Report event: LE_WIFICLIENT_EVENT_DISCONNECTED
            le_wifiClient_Event_t event = LE_WIFICLIENT_EVENT_DISCONNECTED;
            LE_INFO("InternalWifiClientStateEvent event: %d ", event);
            le_event_Report(WifiClientPaEvent, (void *)&event, sizeof(le_wifiClient_Event_t));
        }
    }
    // Run the event loop
    le_event_RunLoop();
    return NULL;
}

//--------------------------------------------------------------------------------------------------
// Public declarations
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to initialize the PA WiFi Module.
 *
 * @return LE_OK     The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_wifiClient_Init
(
    void
)
{
    LE_INFO("Init called");
    // Create the event for signaling user handlers.
    WifiClientPaEvent = le_event_CreateId("WifiClientPaEvent", sizeof(le_wifiClient_Event_t));

    // TODO: Temporary fix
    system("chmod 755 /legato/systems/current/apps/wifiService/read-only/pa_wifi.sh");

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to release the PA WiFi Module.
 *
 * @return LE_OK     The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_wifiClient_Release
(
    void
)
{
    LE_INFO("Release called");
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Start WiFi Client PA
 *
 * @return LE_FAULT  The function failed.
 * @return LE_OK     The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_wifiClient_Start
(
    void
)
{
    le_result_t result       = LE_FAULT;
    int         systemResult;

    LE_INFO("Start called");

    // Create WiFi Client PA Thread
    WifiClientPaThread = le_thread_Create("WifiClientPaThread", WifiClientPaThreadMain, NULL);
    le_thread_SetJoinable(WifiClientPaThread);
    le_thread_AddDestructor(ThreadDestructor, NULL);
    le_thread_Start(WifiClientPaThread);

    systemResult = system(WIFI_SCRIPT_PATH COMMAND_WIFI_HW_START);
    /**
     * Return value of -1 means that the fork() has failed (see man system).
     * The script /etc/init.d/tiwifi returns 0 if the kernel modules are loaded correctly
     * and the wlan0 interface is seen,
     * 127 if modules not loaded or interface not seen,
     * 1 if the option passed if unknown (start stop and restart).
     */
    if (0 == WEXITSTATUS(systemResult))
    {
        LE_INFO("WiFi Client Command OK:" COMMAND_WIFI_HW_START);
        result = LE_OK;
    }
    else
    {
        LE_ERROR("WiFi Client Command Failed: (%d)" COMMAND_WIFI_HW_START, systemResult);
        result = LE_FAULT;
    }
    if (LE_OK == result)
    {
        systemResult = system(WIFI_SCRIPT_PATH COMMAND_WIFI_WLAN_UP);
        if (0 == WEXITSTATUS(systemResult))
        {
            LE_INFO("WiFi Client Command OK:" COMMAND_WIFI_WLAN_UP);
            result = LE_OK;
        }
        else
        {
            LE_ERROR("WiFi Client Command Failed: (%d)" COMMAND_WIFI_WLAN_UP, systemResult);
            result = LE_FAULT;
        }
    }
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Stop WiFi Client PA
 *
 * @return LE_FAULT  The function failed.
 * @return LE_OK     The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_wifiClient_Stop
(
    void
)
{
    le_result_t result       = LE_FAULT;
    int         systemResult = system(WIFI_SCRIPT_PATH COMMAND_WIFI_HW_STOP);

    /**
     * Return value of -1 means that the fork() has failed (see man system).
     * The script /etc/init.d/tiwifi returns 0 if the kernel modules are loaded correctly
     * and the wlan0 interface is seen,
     * 127 if modules not loaded or interface not seen,
     * 1 if the option passed if unknown (start stop and restart).
    */
    if (0 == WEXITSTATUS(systemResult))
    {
        LE_INFO("WiFi Client Command OK:" COMMAND_WIFI_HW_STOP);
        result = LE_OK;
    }
    else
    {
        LE_ERROR("WiFi Client Command Failed: (%d)" COMMAND_WIFI_HW_STOP, systemResult);
        result = LE_FAULT;
    }

    if (LE_OK == result)
    {
        // Must terminate created thread
        le_thread_Cancel(WifiClientPaThread);
        if (LE_OK == le_thread_Join(WifiClientPaThread, NULL))
        {
            // And close FP used in created thread
            pclose(IwThreadPipePtr);
            IwThreadPipePtr = NULL;
            result = LE_OK;
        }
        else
        {
            result= LE_FAULT;
        }
    }
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function will start a scan and returns when it is done.
 * It should NOT return until the scan is done.
 * Results are read via pa_wifiClient_GetScanResult.
 * When the reading is done pa_wifiClient_ScanDone MUST be called.
 *
 * @return LE_FAULT  The function failed.
 * @return LE_BUSY   The function is already ongoing.
 * @return LE_OK     The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_wifiClient_Scan
(
    void
)
{
    le_result_t result = LE_OK;

    LE_INFO("Scanning");
    if (IsScanRunning)
    {
        LE_ERROR("Scan is already running");
        return LE_BUSY;
    }

    if (NULL != IwScanPipePtr)
    {
        return LE_BUSY;
    }

    IsScanRunning = true;
    /* Open the command for reading. */
    IwScanPipePtr = popen(WIFI_SCRIPT_PATH COMMAND_WIFICLIENT_START_SCAN, "r");

    if (NULL == IwScanPipePtr)
    {
        LE_ERROR("Failed to run command: errno:%d: \"%s\" Cmd:"
            COMMAND_WIFICLIENT_START_SCAN,
            errno,
            strerror(errno));
        result = LE_FAULT;
    }

    IsScanRunning = false;
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function is used to find out if a scan is currently running.
 *
 * @return TRUE  Scan is running.
 * @return FALSE Scan is not running
 */
//--------------------------------------------------------------------------------------------------
bool pa_wifiClient_IsScanRunning
(
)
{
    return IsScanRunning;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function can be called after pa_wifi_Scan.
 * When the reading is done, it no longer returns LE_OK,
 * pa_wifiClient_ScanDone MUST be called.
 *
 * @return LE_NOT_FOUND  There is no more AP:s found.
 * @return LE_OK     The function succeeded.
 * @return LE_FAULT  The function failed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_wifiClient_GetScanResult
(
    pa_wifiClient_AccessPoint_t *accessPointPtr
    ///< [IN][OUT]
    ///< Structure provided by calling function.
    ///< Results filled out if result was LE_OK.
)
{
    char path[1024];

    LE_INFO("Scan results");

    if (NULL == IwScanPipePtr)
    {
       LE_ERROR("ERROR must call pa_wifi_Scan first");
       return LE_FAULT;
    }
    if (NULL == accessPointPtr)
    {
       LE_ERROR("ERROR : accessPoint == NULL");
       return LE_BAD_PARAMETER;
    }

    /* Default values */
    accessPointPtr->signalStrength = 0xffff;
    accessPointPtr->ssidLength = 0;

    /* Read the output a line at a time - output it. */
    while (NULL != fgets(path, sizeof(path) - 1, IwScanPipePtr))
    {
        LE_INFO("PARSING:%s: len:%zd", path, strnlen(path, sizeof(path) - 1));

        if (0 == strncmp("\tSSID: ", path, 7))
        {
            accessPointPtr->ssidLength = strnlen(path, LE_WIFIDEFS_MAX_SSID_BYTES) - 7 - 1;
            LE_INFO("FOUND SSID:%s  %c%c.. ", path, path[7], path[8]);
            memset(&accessPointPtr->ssidBytes, 0, LE_WIFIDEFS_MAX_SSID_BYTES);
            memcpy (&accessPointPtr->ssidBytes, &path[7], accessPointPtr->ssidLength);
            LE_INFO("FOUND SSID: Parsed:\"%s\"", &accessPointPtr->ssidBytes[0]);
            return LE_OK;
        }
        else if (0 == strncmp("\tsignal: ", path, 9))
        {
            LE_INFO("FOUND SIGNAL STRENGTH:%s  %c %c ", path, path[10], path[11]);
            accessPointPtr->signalStrength = atoi(&path[9]);
            LE_INFO("FOUND SIGNAL STRENGTH: signalStrength:%d ",
            accessPointPtr->signalStrength);
        }
    }

    return LE_NOT_FOUND;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called after the pa_wifiClient_Scan() has been done.
 * It signals that the scan results are no longer needed and frees some internal resources.
 *
 * @return LE_OK     The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_wifiClient_ScanDone
(
    void
)
{
    if (NULL != IwScanPipePtr)
    {
        pclose(IwScanPipePtr);
        IwScanPipePtr = NULL;
        IsScanRunning = false;
    }
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the security protocol for communication.
 *
 * @return LE_BAD_PARAMETER  The function failed due to an invalid parameter.
 * @return LE_OK             The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_wifiClient_SetSecurityProtocol
(
    const le_wifiClient_SecurityProtocol_t securityProtocol
        ///< [IN]
        ///< Security protocol used for communication.
)
{
    le_result_t result = LE_OK;

    LE_INFO("Security protocol: %d", securityProtocol);
    switch (securityProtocol)
    {
        case LE_WIFICLIENT_SECURITY_NONE:
        case LE_WIFICLIENT_SECURITY_WEP:
        case LE_WIFICLIENT_SECURITY_WPA_PSK_PERSONAL:
        case LE_WIFICLIENT_SECURITY_WPA2_PSK_PERSONAL:
        case LE_WIFICLIENT_SECURITY_WPA_EAP_PEAP0_ENTERPRISE:
        case LE_WIFICLIENT_SECURITY_WPA2_EAP_PEAP0_ENTERPRISE:
            SavedSecurityProtocol = securityProtocol;
            result = LE_OK;
            break;

        default:
            result = LE_BAD_PARAMETER;
            break;
    }
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function connects a wifiClient.
 *
 * @return LE_FAULT  The function failed.
 * @return LE_OK     The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_wifiClient_Connect
(
    uint8_t ssidBytes[LE_WIFIDEFS_MAX_SSID_BYTES],
        ///< [IN]
        ///< Contains ssidLength number of bytes
    uint8_t ssidLength
        ///< [IN]
        ///< The number of Bytes in the ssidBytes
)
{
    int         systemResult;
    char        tmpString[1024];
    char        path[4 *1024];
    le_result_t result          = LE_OK;

    LE_INFO("SSID length %d | SSID: \"%.*s\"", ssidLength, ssidLength, (char *)ssidBytes);

    if (0 == ssidLength)
    {
        LE_ERROR("No valid SSID");
        return LE_FAULT;
    }

    // Which type of WiFi client connection is requested?
    switch (SavedSecurityProtocol)
    {
        case LE_WIFICLIENT_SECURITY_NONE:
            // Opened WiFi hotspot (SSID with no password)
            LE_INFO("SwitchCase: LE_WIFICLIENT_SECURITY_NONE");
            // 1. Now WPA_CLI for LE_WIFICLIENT_SECURITY_NONE
            LE_INFO("Step 2: SH script");
            snprintf(tmpString,
                sizeof(tmpString),
                (WIFI_SCRIPT_PATH COMMAND_WIFICLIENT_CONNECT_SECURITY_NONE),
                ssidLength,
                (char *)ssidBytes,
                HiddenAccessPoint);

            systemResult = system(tmpString);
            // Return value of -1 means that the fork() has failed (see man system).
            if (0 == WEXITSTATUS(systemResult))
            {
                LE_INFO("WiFi Client Command OK: %s", tmpString);
                result = LE_OK;
            }
            else
            {
                LE_ERROR("WiFi Client Command %s Failed: (%d)", tmpString, systemResult);
                result = LE_FAULT;
            }
            break;

        case LE_WIFICLIENT_SECURITY_WEP:
            // Connect to secured network - WEP
            LE_INFO("SwitchCase: LE_WIFICLIENT_SECURITY_WEP");
            if (0 == SavedWepKey[0])
            {
                LE_ERROR("No valid WEP key");
                return LE_FAULT;
            }
            // 1. WPA_CLI for LE_WIFICLIENT_SECURITY_WEP
            if (LE_OK == result)
            {
                LE_INFO("Step 2: SH script");
                snprintf(tmpString,
                    sizeof(tmpString),
                    (WIFI_SCRIPT_PATH COMMAND_WIFICLIENT_CONNECT_SECURITY_WEP),
                    ssidLength,
                    (char *)ssidBytes,
                    HiddenAccessPoint,
                    SavedWepKey);

                systemResult = system(tmpString);
                // Return value of -1 means that the fork() has failed (see man system).
                if (0 == WEXITSTATUS(systemResult))
                {
                    LE_INFO("WiFi Client Command OK: %s", tmpString);
                    result = LE_OK;
                }
                else
                {
                    LE_ERROR("WiFi Client Command %s Failed: (%d)", tmpString, systemResult);
                    result = LE_FAULT;
                }
            }
            break;

        case LE_WIFICLIENT_SECURITY_WPA_PSK_PERSONAL:
            // Connect to secured network - WPA
            LE_INFO("SwitchCase: LE_WIFICLIENT_SECURITY_WPA_PSK_PERSONAL");
            if ((0 == SavedPassphrase[0]) && (0 == SavedPreSharedKey[0]))
            {
                LE_ERROR("No valid PassPhrase & PreSharedKey");
                return LE_FAULT;
            }
            if (0 != SavedPassphrase[0])
            {
                // 1. Passphrase/PSK. With PassPhrase generate PSK and read it for next command
                LE_INFO("Step 1: Generate Passphrase/PSK");
                snprintf(tmpString,
                    sizeof(tmpString),
                    (WIFI_SCRIPT_PATH COMMAND_WIFICLIENT_CONNECT_WPA_PASSPHRASE),
                    ssidLength,
                    (char *)ssidBytes,
                    SavedPassphrase);

                LE_INFO("Cmd: %s", tmpString);

                // Open the command for reading.
                IwConnectPipePtr = popen(tmpString, "r");

                if (NULL == IwConnectPipePtr)
                {
                    LE_ERROR("Failed to run command:\"%s\" errno:%d %s",
                        (tmpString),
                        errno,
                        strerror(errno));
                    result = LE_FAULT;
                }
                else
                {
                    LE_INFO("Cmd successful: %s", tmpString);
                    // Read the output a line at a time - output it.
                    while (NULL != fgets(path, sizeof(path)-1, IwConnectPipePtr))
                    {
                        LE_INFO("PARSING:%s: len:%d", path, (int) strnlen(path, sizeof(path) - 1));
                        if (0 == strncmp("\tpsk=", path, strlen("\tpsk=")))
                        {
                            char     *p1Ptr = strchr(&path[6], '\n');
                            char     *p2Ptr = &path[6];
                            uint32_t  n  = p1Ptr - p2Ptr;

                            LE_INFO("FOUND :%s", path);
                            memset(SavedPreSharedKey, 0, LE_WIFIDEFS_MAX_PSK_BYTES);
                            if (n < LE_WIFIDEFS_MAX_PSK_BYTES)
                            {
                                strncpy(SavedPreSharedKey, &path[6], n);
                                LE_INFO("PSK=%s length: %d", SavedPreSharedKey, n);
                                result = LE_OK;
                            }
                            else
                            {
                                result = LE_FAULT;
                            }
                        }
                    }
                }
                pclose(IwConnectPipePtr);
            }
            // 2. Now WPA_CLI for LE_WIFICLIENT_SECURITY_WPA_PSK_PERSONAL
            if (LE_OK == result)
            {
                LE_INFO("Step 2: SH script");
                snprintf(tmpString,
                    sizeof(tmpString),
                    (WIFI_SCRIPT_PATH COMMAND_WIFICLIENT_CONNECT_SECURITY_WPA_PSK_PERSONAL),
                    ssidLength,
                    (char *)ssidBytes,
                    SavedPreSharedKey,
                    SavedPassphrase);

                systemResult = system(tmpString);
                // Return value of -1 means that the fork() has failed (see man system).
                if (0 == WEXITSTATUS(systemResult))
                {
                    LE_INFO("WiFi Client Command OK: %s", tmpString);
                    result = LE_OK;
                }
                else
                {
                    LE_ERROR("WiFi Client Command %s Failed: (%d)", tmpString, systemResult);
                    result = LE_FAULT;
                }
            }
            break;

        case LE_WIFICLIENT_SECURITY_WPA2_PSK_PERSONAL:
            // Connect to secured network - WPA2
            LE_INFO("SwitchCase: LE_WIFICLIENT_SECURITY_WPA2_PSK_PERSONAL");
            if ((0 == SavedPassphrase[0]) && (0 == SavedPreSharedKey[0]))
            {
                LE_ERROR("pa_wifi_Connect: No valid PassPhrase & PreSharedKey");
                return LE_FAULT;
            }
            if (0 != SavedPassphrase[0])
            {
                // 1. Passphrase/PSK. With PassPhrase generate PSK and read it for next command
                LE_INFO("Step 1: Generate Passphrase/PSK");
                snprintf(tmpString,
                    sizeof(tmpString) ,
                    (WIFI_SCRIPT_PATH COMMAND_WIFICLIENT_CONNECT_WPA_PASSPHRASE),
                    ssidLength,
                    (char *)ssidBytes,
                    SavedPassphrase);

                LE_INFO("Cmd: %s", tmpString);

                // Open the command for reading.
                IwConnectPipePtr = popen(tmpString, "r");

                if (NULL == IwConnectPipePtr)
                {
                    LE_ERROR("pa_wifi_Connect: Failed to run command:\"%s\" errno:%d %s",
                        (tmpString),
                        errno,
                        strerror(errno));
                    result = LE_FAULT;
                } else {
                    LE_INFO("Cmd successful: %s", tmpString);
                    // Read the output a line at a time - output it.
                    while (NULL != fgets(path, sizeof(path)-1, IwConnectPipePtr))
                    {
                        LE_INFO("PARSING:%s: len:%d", path, (int) strnlen(path, sizeof(path)-1));
                        if (0 == strncmp("\tpsk=", path, strlen("\tpsk=")))
                        {
                            char *p1Ptr = strchr(&path[6], '\n');
                            char *p2Ptr = &path[6];
                            int   n  = p1Ptr - p2Ptr;

                            LE_INFO("FOUND :%s", path);
                            memset(SavedPreSharedKey, 0, LE_WIFIDEFS_MAX_PSK_BYTES);
                            if (n < LE_WIFIDEFS_MAX_PSK_BYTES)
                            {
                                strncpy(SavedPreSharedKey, &path[6], n);
                                LE_INFO("PSK=%s length: %d", SavedPreSharedKey, n);
                                result = LE_OK;
                            }
                            else
                            {
                                result = LE_FAULT;
                            }
                        }
                    }
                }
                pclose(IwConnectPipePtr);
            }
            // 2. Now WPA_CLI for LE_WIFICLIENT_SECURITY_WPA2_PSK_PERSONAL
            if (LE_OK == result)
            {
                LE_INFO("Step 2: SH script");
                snprintf(tmpString,
                    sizeof(tmpString),
                    (WIFI_SCRIPT_PATH COMMAND_WIFICLIENT_CONNECT_SECURITY_WPA2_PSK_PERSONAL),
                    ssidLength,
                    (char *)ssidBytes,
                    SavedPreSharedKey,
                    SavedPassphrase);

                systemResult = system(tmpString);
                // Return value of -1 means that the fork() has failed (see man system).
                if (0 == WEXITSTATUS(systemResult))
                {
                    LE_INFO("WiFi Client Command OK: %s", tmpString);
                    result = LE_OK;
                }
                else
                {
                    LE_ERROR("WiFi Client Command %s Failed: (%d)", tmpString, systemResult);
                    result = LE_FAULT;
                }
            }
            break;

        case LE_WIFICLIENT_SECURITY_WPA_EAP_PEAP0_ENTERPRISE:
            // Enterprise environment: connect to WPA EAP PEAP0
            LE_INFO("SwitchCase: ..._SECURITY_WPA2_EAP_PEAP0_ENTERPRISE");
            if ((0 == SavedUsername[0]) && (0 == SavedPassword[0]))
            {
                LE_ERROR("pa_wifi_Connect: No valid Username & Password");
                return LE_FAULT;
            }
            // 2. Now WPA_CLI for LE_WIFICLIENT_SECURITY_WPA_EAP_PEAP0_ENTERPRISE
            if (LE_OK == result)
            {
                LE_INFO("Step 2: SH script");
                snprintf(tmpString,
                    sizeof(tmpString),
                    (WIFI_SCRIPT_PATH COMMAND_WIFICLIENT_CONNECT_SECURITY_WPA_EAP_PEAP0_ENTERPRISE),
                    ssidLength,
                    (char *)ssidBytes,
                    HiddenAccessPoint,
                    SavedUsername,
                    SavedPassword);

                systemResult = system(tmpString);
                // Return value of -1 means that the fork() has failed (see man system).
                if (0 == WEXITSTATUS(systemResult))
                {
                    LE_INFO("WiFi Client Command OK: %s", tmpString);
                    result = LE_OK;
                }
                else
                {
                    LE_ERROR("WiFi Client Command %s Failed: (%d)", tmpString, systemResult);
                    result = LE_FAULT;
                }
            }
            break;

        case LE_WIFICLIENT_SECURITY_WPA2_EAP_PEAP0_ENTERPRISE:
            // Enterprise environment: connect to WPA2 EAP PEAP0
            LE_INFO("SwitchCase: ..._SECURITY_WPA2_EAP_PEAP0_ENTERPRISE");
            if ((0 == SavedUsername[0]) && (0 == SavedPassword[0]))
            {
                LE_ERROR("pa_wifi_Connect: No valid Username & Password");
                return LE_FAULT;
            }
            // 2. Now WPA_CLI for LE_WIFICLIENT_SECURITY_WPA2_EAP_PEAP0_ENTERPRISE
            if (LE_OK == result)
            {
                LE_INFO("Step 2: SH script");
                snprintf(tmpString,
                    sizeof(tmpString),
                    (WIFI_SCRIPT_PATH COMMAND_WIFICLIENT_CONNECT_SECURITY_WPA2_EAP_PEAP0_ENTERPRISE),
                    ssidLength,
                    (char *)ssidBytes,
                    HiddenAccessPoint,
                    SavedUsername,
                    SavedPassword);

                systemResult = system(tmpString);
                // Return value of -1 means that the fork() has failed (see man system).
                if (0 == WEXITSTATUS(systemResult))
                {
                    LE_INFO("WiFi Client Command OK: %s", tmpString);
                    result = LE_OK;
                }
                else
                {
                    LE_ERROR("WiFi Client Command %s Failed: (%d)", tmpString, systemResult);
                    result = LE_FAULT;
                }
            }
            break;

        default:
            result = LE_FAULT;
            break;
    }
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function disconnects a wifiClient.
 *
 * @return LE_FAULT  The function failed.
 * @return LE_OK     The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_wifiClient_Disconnect
(
    void
)
{
    int         systemResult;
    le_result_t result       = LE_OK;

    // Terminate connection
    systemResult = system(WIFI_SCRIPT_PATH COMMAND_WIFICLIENT_DISCONNECT);
    if (0 == WEXITSTATUS(systemResult))
    {
        LE_INFO("WiFi Client Command OK:" COMMAND_WIFICLIENT_DISCONNECT);
        result = LE_OK;
    }
    else
    {
        LE_ERROR("WiFi Client Command Failed: (%d)" COMMAND_WIFICLIENT_DISCONNECT, systemResult);
        result = LE_FAULT;
    }
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Clears all username, password, PreShared Key, passphrase settings previously made by
 *
 * @see  pa_wifiClient_SetPassPhrase
 * @see  pa_wifiClient_SetPreSharedKey
 * @see  pa_wifiClient_SetUserCredentials
 *
 * @return LE_OK     The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_wifiClient_ClearAllCredentials
(
    void
)
{
    memset(SavedWepKey, '\0', LE_WIFIDEFS_MAX_WEPKEY_BYTES);
    memset(SavedPassphrase, '\0', LE_WIFIDEFS_MAX_PASSPHRASE_BYTES);
    memset(SavedPreSharedKey, '\0', LE_WIFIDEFS_MAX_PSK_BYTES);
    memset(SavedUsername, '\0', LE_WIFIDEFS_MAX_USERNAME_BYTES);
    memset(SavedPassword, '\0', LE_WIFIDEFS_MAX_PASSWORD_BYTES);
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the WEP key (Wired Equivalent Privacy)
 *
 * @note WEP is unsecure and has been deprecated by the WiFi alliance. Still, it can be used in
 * user mode.
 *
 * @return LE_BAD_PARAMETER  The function failed due to an invalid parameter.
 * @return LE_OK             The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_wifiClient_SetWepKey
(
    const char *wepKeyPtr
        ///< [IN]
        ///< Wired Equivalent Privacy (WEP) key used for authentication
)
{
    le_result_t result = LE_BAD_PARAMETER;

    LE_INFO("Set WEP key");
    if (NULL != wepKeyPtr)
    {
       strncpy(&SavedWepKey[0], &wepKeyPtr[0], LE_WIFIDEFS_MAX_WEPKEY_LENGTH);
       // Make sure there is a null termination
       SavedWepKey[LE_WIFIDEFS_MAX_WEPKEY_LENGTH] = '\0';
       result = LE_OK;
    }
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the WiFi pre-shared key (WPA-Personal)
 *
 * @see  pa_wifiClient_SetPassPhrase
 *
 * @return LE_BAD_PARAMETER  The function failed due to an invalid parameter.
 * @return LE_OK             The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_wifiClient_SetPreSharedKey
(
    const char *preSharedKeyPtr
        ///< [IN]
        ///< Pre-shared key (PSK) used for authentication
)
{
    le_result_t result = LE_BAD_PARAMETER;

    LE_INFO("Set PSK");
    if (NULL != preSharedKeyPtr)
    {
       strncpy(&SavedPreSharedKey[0], &preSharedKeyPtr[0], LE_WIFIDEFS_MAX_PSK_LENGTH);
       // Make sure there is a null termination
       SavedPreSharedKey[LE_WIFIDEFS_MAX_PSK_LENGTH] = '\0';
       result = LE_OK;
    }
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function specifies whether the target Access Point is hiding its presence from clients or
 * not. When an Access Point is hidden, it cannot be discovered by a scan process.
 *
 * @note By default, this attribute is not set which means that the client is unable to connect to
 * a hidden access point. When enabled, the client will be able to connect to the access point
 * whether it is hidden or not.
 */
//--------------------------------------------------------------------------------------------------
void pa_wifiClient_SetHiddenNetworkAttribute
(
    bool hidden
        ///< [IN]
        ///< If TRUE, the WIFI client will be able to connect to a hidden access point.
)
{
    LE_INFO("Set whether Access Point is hidden or not: %d", hidden);
    (void)hidden;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the PassPhrase used to create PSK (WPA-Personal).
 *
 * @see  pa_wifiClient_SetPreSharedKey
 *
 * @return LE_BAD_PARAMETER  The function failed due to an invalid parameter.
 * @return LE_OK             The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_wifiClient_SetPassphrase
(
    const char *passphrasePtr
        ///< [IN]
        ///< Passphrase used for authentication
)
{
    // Store Passphrase to be used later during connection procedure
    le_result_t result = LE_BAD_PARAMETER;

    LE_INFO("Set passphrase");
    if (NULL != passphrasePtr)
    {
       strncpy(&SavedPassphrase[0], &passphrasePtr[0], LE_WIFIDEFS_MAX_PASSPHRASE_LENGTH);
       // Make sure there is a null termination
       SavedPassphrase[LE_WIFIDEFS_MAX_PASSPHRASE_LENGTH] = '\0';
       result = LE_OK;
    }
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the username and password (WPA-Entreprise).
 *
 * @return LE_BAD_PARAMETER  The function failed due to an invalid parameter.
 * @return LE_OK             The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_wifiClient_SetUserCredentials
(
    const char *usernamePtr,
        ///< [IN]
        ///< Username used for authentication
    const char *passwordPtr
        ///< [IN]
        ///< Password used for authentication
)
{
    // Store User Credentials to be used later during connection procedure
    le_result_t result = LE_BAD_PARAMETER;

    LE_INFO("Set user credentials");
    if (NULL != usernamePtr)
    {
       strncpy(&SavedUsername[0], &usernamePtr[0], LE_WIFIDEFS_MAX_USERNAME_LENGTH);
       // Make sure there is a null termination
       SavedUsername[LE_WIFIDEFS_MAX_USERNAME_LENGTH] = '\0';
       result = LE_OK;
    }
    else
    {
        return LE_BAD_PARAMETER;
    }

    if (NULL != passwordPtr)
    {
       strncpy(&SavedPassword[0], &passwordPtr[0], LE_WIFIDEFS_MAX_PASSWORD_LENGTH);
       // Make sure there is a null termination
       SavedPassword[LE_WIFIDEFS_MAX_PASSWORD_LENGTH] = '\0';
       result = LE_OK;
    }
    else
    {
        return LE_BAD_PARAMETER;
    }
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Add handler function for PA EVENT 'le_wifiClient_Event_t'
 *
 * This event provide information on PA WiFi Client event changes.
 *
 * @return LE_BAD_PARAMETER  The function failed due to an invalid parameter.
 * @return LE_OK             The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_wifiClient_AddEventHandler
(
    pa_wifiClient_NewEventHandlerFunc_t handlerPtr,
        ///< [IN]
        ///< Event handler function pointer.

    void *contextPtr
        ///< [IN]
        ///< Associated event context.
)
{
    le_event_HandlerRef_t handlerRef;

    handlerRef = le_event_AddLayeredHandler("WifiClientPaHandler",
        WifiClientPaEvent,
        FirstLayerWifiClientEventHandler,
        (le_event_HandlerFunc_t)handlerPtr);
    if (NULL != handlerRef)
    {
        LE_INFO("ERROR: le_event_AddLayeredHandler returned NULL");
        return LE_BAD_PARAMETER;
    }
    le_event_SetContextPtr(handlerRef, contextPtr);
    return LE_OK;
}

