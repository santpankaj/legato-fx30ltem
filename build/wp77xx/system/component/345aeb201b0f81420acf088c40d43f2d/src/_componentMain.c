/*
 * AUTO-GENERATED _componentMain.c for the wifiClientTestComponent component.

 * Don't bother hand-editing this file.
 */

#include "legato.h"
#include "../liblegato/eventLoop.h"
#include "../liblegato/log.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char* _wifiClientTestComponent_le_wifiClient_ServiceInstanceName;
const char** le_wifiClient_ServiceInstanceNamePtr = &_wifiClientTestComponent_le_wifiClient_ServiceInstanceName;
void le_wifiClient_ConnectService(void);
// Component log session variables.
le_log_SessionRef_t wifiClientTestComponent_LogSession;
le_log_Level_t* wifiClientTestComponent_LogLevelFilterPtr;

// Component initialization function (COMPONENT_INIT).
void _wifiClientTestComponent_COMPONENT_INIT(void);

// Library initialization function.
// Will be called by the dynamic linker loader when the library is loaded.
__attribute__((constructor)) void _wifiClientTestComponent_Init(void)
{
    LE_DEBUG("Initializing wifiClientTestComponent component library.");

    // Connect client-side IPC interfaces.
    le_wifiClient_ConnectService();

    // Register the component with the Log Daemon.
    wifiClientTestComponent_LogSession = log_RegComponent("wifiClientTestComponent", &wifiClientTestComponent_LogLevelFilterPtr);

    //Queue the COMPONENT_INIT function to be called by the event loop
    event_QueueComponentInit(_wifiClientTestComponent_COMPONENT_INIT);
}


#ifdef __cplusplus
}
#endif
