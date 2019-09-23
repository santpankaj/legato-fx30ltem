

/*
 * ====================== WARNING ======================
 *
 * THE CONTENTS OF THIS FILE HAVE BEEN AUTO-GENERATED.
 * DO NOT MODIFY IN ANY WAY.
 *
 * ====================== WARNING ======================
 */


#ifndef SUPERVISORWDOG_INTERFACE_H_INCLUDE_GUARD
#define SUPERVISORWDOG_INTERFACE_H_INCLUDE_GUARD


#include "legato.h"


//--------------------------------------------------------------------------------------------------
/**
 * Get the server service reference
 */
//--------------------------------------------------------------------------------------------------
le_msg_ServiceRef_t supervisorWdog_GetServiceRef
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * Get the client session reference for the current message
 */
//--------------------------------------------------------------------------------------------------
le_msg_SessionRef_t supervisorWdog_GetClientSessionRef
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * Initialize the server and advertise the service.
 */
//--------------------------------------------------------------------------------------------------
void supervisorWdog_AdvertiseService
(
    void
);


//--------------------------------------------------------------------------------------------------
/**
 * Reference type used by Add/Remove functions for EVENT 'supervisorWdog_KickEvent'
 */
//--------------------------------------------------------------------------------------------------
typedef struct supervisorWdog_KickEventHandler* supervisorWdog_KickEventHandlerRef_t;


//--------------------------------------------------------------------------------------------------
/**
 * Handler for framework daemon kick.  Called periodically by a framework daemon to notify
 * the watchdogDaemon it's still alive.
 */
//--------------------------------------------------------------------------------------------------
typedef void (*supervisorWdog_KickHandlerFunc_t)
(
    void* contextPtr
        ///<
);


//--------------------------------------------------------------------------------------------------
/**
 * Add handler function for EVENT 'supervisorWdog_KickEvent'
 *
 * This event is fired by a framework daemon periodically in its event loop to notify
 * the watchdogDaemon it's still alive.
 */
//--------------------------------------------------------------------------------------------------
supervisorWdog_KickEventHandlerRef_t supervisorWdog_AddKickEventHandler
(
    uint32_t interval,
        ///< [IN] [IN] Interval to kick the timer, in ms.
    supervisorWdog_KickHandlerFunc_t handlerPtr,
        ///< [IN] [IN] Handler for watchdog kick.
    void* contextPtr
        ///< [IN]
);



//--------------------------------------------------------------------------------------------------
/**
 * Remove handler function for EVENT 'supervisorWdog_KickEvent'
 */
//--------------------------------------------------------------------------------------------------
void supervisorWdog_RemoveKickEventHandler
(
    supervisorWdog_KickEventHandlerRef_t handlerRef
        ///< [IN]
);


#endif // SUPERVISORWDOG_INTERFACE_H_INCLUDE_GUARD