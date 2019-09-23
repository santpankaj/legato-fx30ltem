/*
 * AUTO-GENERATED _componentMain.c for the wifiApTestComponent component.

 * Don't bother hand-editing this file.
 */

#include "legato.h"
#include "../liblegato/eventLoop.h"
#include "../liblegato/log.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char* _wifiApTestComponent_le_wifiAp_ServiceInstanceName;
const char** le_wifiAp_ServiceInstanceNamePtr = &_wifiApTestComponent_le_wifiAp_ServiceInstanceName;
void le_wifiAp_ConnectService(void);
// Component log session variables.
le_log_SessionRef_t wifiApTestComponent_LogSession;
le_log_Level_t* wifiApTestComponent_LogLevelFilterPtr;

// Component initialization function (COMPONENT_INIT).
void _wifiApTestComponent_COMPONENT_INIT(void);

// Library initialization function.
// Will be called by the dynamic linker loader when the library is loaded.
__attribute__((constructor)) void _wifiApTestComponent_Init(void)
{
    LE_DEBUG("Initializing wifiApTestComponent component library.");

    // Connect client-side IPC interfaces.
    le_wifiAp_ConnectService();

    // Register the component with the Log Daemon.
    wifiApTestComponent_LogSession = log_RegComponent("wifiApTestComponent", &wifiApTestComponent_LogLevelFilterPtr);

    //Queue the COMPONENT_INIT function to be called by the event loop
    event_QueueComponentInit(_wifiApTestComponent_COMPONENT_INIT);
}


#ifdef __cplusplus
}
#endif
