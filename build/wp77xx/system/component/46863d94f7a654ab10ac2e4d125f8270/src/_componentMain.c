/*
 * AUTO-GENERATED _componentMain.c for the daemon component.

 * Don't bother hand-editing this file.
 */

#include "legato.h"
#include "../liblegato/eventLoop.h"
#include "../liblegato/log.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char* _daemon_le_wifiClient_ServiceInstanceName;
const char** le_wifiClient_ServiceInstanceNamePtr = &_daemon_le_wifiClient_ServiceInstanceName;
void le_wifiClient_AdvertiseService(void);
extern const char* _daemon_le_wifiAp_ServiceInstanceName;
const char** le_wifiAp_ServiceInstanceNamePtr = &_daemon_le_wifiAp_ServiceInstanceName;
void le_wifiAp_AdvertiseService(void);
// Component log session variables.
le_log_SessionRef_t daemon_LogSession;
le_log_Level_t* daemon_LogLevelFilterPtr;

// Component initialization function (COMPONENT_INIT).
void _daemon_COMPONENT_INIT(void);

// Library initialization function.
// Will be called by the dynamic linker loader when the library is loaded.
__attribute__((constructor)) void _daemon_Init(void)
{
    LE_DEBUG("Initializing daemon component library.");

    // Advertise server-side IPC interfaces.
    le_wifiClient_AdvertiseService();
    le_wifiAp_AdvertiseService();

    // Register the component with the Log Daemon.
    daemon_LogSession = log_RegComponent("daemon", &daemon_LogLevelFilterPtr);

    //Queue the COMPONENT_INIT function to be called by the event loop
    event_QueueComponentInit(_daemon_COMPONENT_INIT);
}


#ifdef __cplusplus
}
#endif
