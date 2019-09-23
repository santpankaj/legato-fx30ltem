/*
 * AUTO-GENERATED _componentMain.c for the wifiWebApComponent component.

 * Don't bother hand-editing this file.
 */

#include "legato.h"
#include "../liblegato/eventLoop.h"
#include "../liblegato/log.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char* _wifiWebApComponent_le_wifiAp_ServiceInstanceName;
const char** le_wifiAp_ServiceInstanceNamePtr = &_wifiWebApComponent_le_wifiAp_ServiceInstanceName;
void le_wifiAp_ConnectService(void);
// Component log session variables.
le_log_SessionRef_t wifiWebApComponent_LogSession;
le_log_Level_t* wifiWebApComponent_LogLevelFilterPtr;

// Component initialization function (COMPONENT_INIT).
void _wifiWebApComponent_COMPONENT_INIT(void);

// Library initialization function.
// Will be called by the dynamic linker loader when the library is loaded.
__attribute__((constructor)) void _wifiWebApComponent_Init(void)
{
    LE_DEBUG("Initializing wifiWebApComponent component library.");

    // Connect client-side IPC interfaces.
    le_wifiAp_ConnectService();

    // Register the component with the Log Daemon.
    wifiWebApComponent_LogSession = log_RegComponent("wifiWebApComponent", &wifiWebApComponent_LogLevelFilterPtr);

    //Queue the COMPONENT_INIT function to be called by the event loop
    event_QueueComponentInit(_wifiWebApComponent_COMPONENT_INIT);
}


#ifdef __cplusplus
}
#endif
