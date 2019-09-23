/*
 * AUTO-GENERATED _componentMain.c for the wifi component.

 * Don't bother hand-editing this file.
 */

#include "legato.h"
#include "../liblegato/eventLoop.h"
#include "../liblegato/log.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char* _wifi_le_wifiClient_ServiceInstanceName;
const char** le_wifiClient_ServiceInstanceNamePtr = &_wifi_le_wifiClient_ServiceInstanceName;
void le_wifiClient_ConnectService(void);
extern const char* _wifi_le_wifiAp_ServiceInstanceName;
const char** le_wifiAp_ServiceInstanceNamePtr = &_wifi_le_wifiAp_ServiceInstanceName;
void le_wifiAp_ConnectService(void);
// Component log session variables.
le_log_SessionRef_t wifi_LogSession;
le_log_Level_t* wifi_LogLevelFilterPtr;

// Component initialization function (COMPONENT_INIT).
void _wifi_COMPONENT_INIT(void);

// Library initialization function.
// Will be called by the dynamic linker loader when the library is loaded.
__attribute__((constructor)) void _wifi_Init(void)
{
    LE_DEBUG("Initializing wifi component library.");

    // Connect client-side IPC interfaces.
    le_wifiClient_ConnectService();
    le_wifiAp_ConnectService();

    // Register the component with the Log Daemon.
    wifi_LogSession = log_RegComponent("wifi", &wifi_LogLevelFilterPtr);

    //Queue the COMPONENT_INIT function to be called by the event loop
    event_QueueComponentInit(_wifi_COMPONENT_INIT);
}


#ifdef __cplusplus
}
#endif
