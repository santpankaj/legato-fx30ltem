/*
 * AUTO-GENERATED _componentMain.c for the le_pa_secStore component.

 * Don't bother hand-editing this file.
 */

#include "legato.h"
#include "../liblegato/eventLoop.h"
#include "../liblegato/log.h"

#ifdef __cplusplus
extern "C" {
#endif

// Component log session variables.
le_log_SessionRef_t le_pa_secStore_LogSession;
le_log_Level_t* le_pa_secStore_LogLevelFilterPtr;

// Component initialization function (COMPONENT_INIT).
void _le_pa_secStore_COMPONENT_INIT(void);

// Library initialization function.
// Will be called by the dynamic linker loader when the library is loaded.
__attribute__((constructor)) void _le_pa_secStore_Init(void)
{
    LE_DEBUG("Initializing le_pa_secStore component library.");

    // Register the component with the Log Daemon.
    le_pa_secStore_LogSession = log_RegComponent("le_pa_secStore", &le_pa_secStore_LogLevelFilterPtr);

    //Queue the COMPONENT_INIT function to be called by the event loop
    event_QueueComponentInit(_le_pa_secStore_COMPONENT_INIT);
}


#ifdef __cplusplus
}
#endif
