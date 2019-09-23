/*
 * AUTO-GENERATED _componentMain.c for the atQmiLinker component.

 * Don't bother hand-editing this file.
 */

#include "legato.h"
#include "../liblegato/eventLoop.h"
#include "../liblegato/log.h"

#ifdef __cplusplus
extern "C" {
#endif

// Component log session variables.
le_log_SessionRef_t atQmiLinker_LogSession;
le_log_Level_t* atQmiLinker_LogLevelFilterPtr;

// Component initialization function (COMPONENT_INIT).
void _atQmiLinker_COMPONENT_INIT(void);

// Library initialization function.
// Will be called by the dynamic linker loader when the library is loaded.
__attribute__((constructor)) void _atQmiLinker_Init(void)
{
    LE_DEBUG("Initializing atQmiLinker component library.");

    // Register the component with the Log Daemon.
    atQmiLinker_LogSession = log_RegComponent("atQmiLinker", &atQmiLinker_LogLevelFilterPtr);

    //Queue the COMPONENT_INIT function to be called by the event loop
    event_QueueComponentInit(_atQmiLinker_COMPONENT_INIT);
}


#ifdef __cplusplus
}
#endif
