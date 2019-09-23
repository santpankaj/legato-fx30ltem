

/*
 * ====================== WARNING ======================
 *
 * THE CONTENTS OF THIS FILE HAVE BEEN AUTO-GENERATED.
 * DO NOT MODIFY IN ANY WAY.
 *
 * ====================== WARNING ======================
 */

/**
 * @page c_appProc Application Process Execution API
 *
 * @ref le_appProc_interface.h "API Reference"
 *
 * API for creating and running processes within an application that the app would not normally
 * start on its own.
 *
 * This API can be used for testing and debugging purposes and should only be made available to
 * trusted applications/tools.
 *
 * <HR>
 *
 * Copyright (C) Sierra Wireless Inc.
 */
/**
 * @file le_appProc_interface.h
 *
 * Legato @ref c_appProc include file.
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#ifndef LE_APPPROC_INTERFACE_H_INCLUDE_GUARD
#define LE_APPPROC_INTERFACE_H_INCLUDE_GUARD


#include "legato.h"

// Interface specific includes
#include "le_limit_interface.h"


//--------------------------------------------------------------------------------------------------
/**
 * Type for handler called when a server disconnects.
 */
//--------------------------------------------------------------------------------------------------
typedef void (*le_appProc_DisconnectHandler_t)(void *);

//--------------------------------------------------------------------------------------------------
/**
 *
 * Connect the current client thread to the service providing this API. Block until the service is
 * available.
 *
 * For each thread that wants to use this API, either ConnectService or TryConnectService must be
 * called before any other functions in this API.  Normally, ConnectService is automatically called
 * for the main thread, but not for any other thread. For details, see @ref apiFilesC_client.
 *
 * This function is created automatically.
 */
//--------------------------------------------------------------------------------------------------
void le_appProc_ConnectService
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 *
 * Try to connect the current client thread to the service providing this API. Return with an error
 * if the service is not available.
 *
 * For each thread that wants to use this API, either ConnectService or TryConnectService must be
 * called before any other functions in this API.  Normally, ConnectService is automatically called
 * for the main thread, but not for any other thread. For details, see @ref apiFilesC_client.
 *
 * This function is created automatically.
 *
 * @return
 *  - LE_OK if the client connected successfully to the service.
 *  - LE_UNAVAILABLE if the server is not currently offering the service to which the client is
 *    bound.
 *  - LE_NOT_PERMITTED if the client interface is not bound to any service (doesn't have a binding).
 *  - LE_COMM_ERROR if the Service Directory cannot be reached.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_appProc_TryConnectService
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * Set handler called when server disconnection is detected.
 *
 * When a server connection is lost, call this handler then exit with LE_FATAL.  If a program wants
 * to continue without exiting, it should call longjmp() from inside the handler.
 */
//--------------------------------------------------------------------------------------------------
void le_appProc_SetServerDisconnectHandler
(
    le_appProc_DisconnectHandler_t disconnectHandler,
    void *contextPtr
);

//--------------------------------------------------------------------------------------------------
/**
 *
 * Disconnect the current client thread from the service providing this API.
 *
 * Normally, this function doesn't need to be called. After this function is called, there's no
 * longer a connection to the service, and the functions in this API can't be used. For details, see
 * @ref apiFilesC_client.
 *
 * This function is created automatically.
 */
//--------------------------------------------------------------------------------------------------
void le_appProc_DisconnectService
(
    void
);


//--------------------------------------------------------------------------------------------------
/**
 * Reference to application process objects.
 */
//--------------------------------------------------------------------------------------------------
typedef struct le_appProc_Ref* le_appProc_RefRef_t;


//--------------------------------------------------------------------------------------------------
/**
 * Fault action for a process.
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LE_APPPROC_FAULT_ACTION_IGNORE = 0,
        ///< A fault occured but no further action is required.
    LE_APPPROC_FAULT_ACTION_RESTART_PROC = 1,
        ///< The process should be restarted.
    LE_APPPROC_FAULT_ACTION_RESTART_APP = 2,
        ///< The application should be restarted.
    LE_APPPROC_FAULT_ACTION_STOP_APP = 3,
        ///< The application should be terminated.
    LE_APPPROC_FAULT_ACTION_REBOOT = 4
        ///< The system should be rebooted.
}
le_appProc_FaultAction_t;


//--------------------------------------------------------------------------------------------------
/**
 * Reference type used by Add/Remove functions for EVENT 'le_appProc_Stop'
 */
//--------------------------------------------------------------------------------------------------
typedef struct le_appProc_StopHandler* le_appProc_StopHandlerRef_t;


//--------------------------------------------------------------------------------------------------
/**
 * A handler that is called when the application process exits.
 */
//--------------------------------------------------------------------------------------------------
typedef void (*le_appProc_StopHandlerFunc_t)
(
    int32_t exitCode,
        ///< Exit code of the process.
    void* contextPtr
        ///<
);

//--------------------------------------------------------------------------------------------------
/**
 * Creates a process in an app.  This function can be used to create and subsequently start a
 * process in an application that the application normally would not start on its own.  This
 * function does not actually start the process, use Start() to start the process.
 *
 * If the specified process name matches a name in the app's list of configured processes then
 * runtime parameters such as environment variables, priority, etc. will be taken from the
 * configuration database.  Otherwise default parameters will be used.
 *
 * Parameters can be overridden by the other functions in this API such as AddArg(), SetPriority(),
 * etc.
 *
 * If the executable path is empty and the process name matches a configured process then the
 * configured executable is used.  Otherwise the specified executable path is used.
 *
 * Either the process name or the executable path may be empty but not both.
 *
 * It is an error to call this function on a configured process that is already running.
 *
 * @return
 *      Reference to the application process object if successful.
 *      NULL if there was an error.
 *
 * @note If the application or process names pointers are null or if their strings are empty or of
 *       bad format it is a fatal error, the function will not return.
 */
//--------------------------------------------------------------------------------------------------
le_appProc_RefRef_t le_appProc_Create
(
    const char* LE_NONNULL appName,
        ///< [IN] Name of the app.
    const char* LE_NONNULL procName,
        ///< [IN] Name of the process.
    const char* LE_NONNULL execPath
        ///< [IN] Path to the executable file.
);

//--------------------------------------------------------------------------------------------------
/**
 * Sets the file descriptor that the application process's standard in should be attached to.
 *
 * By default the standard in is directed to /dev/null.
 *
 * If there is an error this function will kill the calling process
 */
//--------------------------------------------------------------------------------------------------
void le_appProc_SetStdIn
(
    le_appProc_RefRef_t appProcRef,
        ///< [IN] Application process to start.
    int stdInFd
        ///< [IN] File descriptor to use as the app proc's standard in.
        ///< -1 means direct to /dev/null.
);

//--------------------------------------------------------------------------------------------------
/**
 * Sets the file descriptor that the application process's standard out should be attached to.
 *
 * By default the standard out is directed to the logs.
 *
 * If there is an error this function will kill the calling process
 */
//--------------------------------------------------------------------------------------------------
void le_appProc_SetStdOut
(
    le_appProc_RefRef_t appProcRef,
        ///< [IN] Application process to start.
    int stdOutFd
        ///< [IN] File descriptor to use as the app proc's standard out.
        ///<  -1 means direct to logs.
);

//--------------------------------------------------------------------------------------------------
/**
 * Sets the file descriptor that the application process's standard err should be attached to.
 *
 * By default the standard err is directed to the logs.
 *
 * If there is an error this function will kill the calling process
 */
//--------------------------------------------------------------------------------------------------
void le_appProc_SetStdErr
(
    le_appProc_RefRef_t appProcRef,
        ///< [IN] Application process to start.
    int stdErrFd
        ///< [IN] File descriptor to use as the app proc's standard error.
        ///< -1 means direct to logs.
);

//--------------------------------------------------------------------------------------------------
/**
 * Add handler function for EVENT 'le_appProc_Stop'
 *
 * Process stopped event.
 */
//--------------------------------------------------------------------------------------------------
le_appProc_StopHandlerRef_t le_appProc_AddStopHandler
(
    le_appProc_RefRef_t appProcRef,
        ///< [IN] Application process to start.
    le_appProc_StopHandlerFunc_t handlerPtr,
        ///< [IN] Handler for the event.
    void* contextPtr
        ///< [IN]
);

//--------------------------------------------------------------------------------------------------
/**
 * Remove handler function for EVENT 'le_appProc_Stop'
 */
//--------------------------------------------------------------------------------------------------
void le_appProc_RemoveStopHandler
(
    le_appProc_StopHandlerRef_t handlerRef
        ///< [IN]
);

//--------------------------------------------------------------------------------------------------
/**
 * Adds a command line argument to the application process.
 *
 * If the application process is a configured process adding any argument means no arguments from
 * the configuration database will be used.
 *
 * Adding an empty argument validates the argument list but does not acutally add an argument. This
 * is useful for overriding the configured arguments list with an empty argument list.
 *
 * If there is an error this function will kill the calling client.
 */
//--------------------------------------------------------------------------------------------------
void le_appProc_AddArg
(
    le_appProc_RefRef_t appProcRef,
        ///< [IN] Application process to start.
    const char* LE_NONNULL arg
        ///< [IN] Argument to add.
);

//--------------------------------------------------------------------------------------------------
/**
 * Deletes and invalidates the cmd-line arguments to a process and use the configured arguments if
 * available.
 *
 * @note If the caller is passing an invalid reference to the application process, it is a fatal
 *       error, the function will not return.
 */
//--------------------------------------------------------------------------------------------------
void le_appProc_ClearArgs
(
    le_appProc_RefRef_t appProcRef
        ///< [IN] Application process to start.
);

//--------------------------------------------------------------------------------------------------
/**
 * Sets the application process's priority.
 *
 * The priority string must be either 'idle','low', 'medium', 'high', 'rt1', 'rt2'...'rt32'.
 *
 * If there is an error this function will kill the client.
 */
//--------------------------------------------------------------------------------------------------
void le_appProc_SetPriority
(
    le_appProc_RefRef_t appProcRef,
        ///< [IN] Application process to start.
    const char* LE_NONNULL priority
        ///< [IN] Priority for the application process.
);

//--------------------------------------------------------------------------------------------------
/**
 * Clears the application process's priority and use either the configured priority or the default.
 *
 * @note If the caller is passing an invalid reference to the application process, it is a fatal
 *       error, the function will not return.
 */
//--------------------------------------------------------------------------------------------------
void le_appProc_ClearPriority
(
    le_appProc_RefRef_t appProcRef
        ///< [IN] Application process to start.
);

//--------------------------------------------------------------------------------------------------
/**
 * Sets the application process's fault action.
 *
 * @note If the caller is passing an invalid reference to the application process, it is a fatal
 *       error, the function will not return.
 */
//--------------------------------------------------------------------------------------------------
void le_appProc_SetFaultAction
(
    le_appProc_RefRef_t appProcRef,
        ///< [IN] Application process to start.
    le_appProc_FaultAction_t action
        ///< [IN] Fault action for the application process.
);

//--------------------------------------------------------------------------------------------------
/**
 * Clears the application process's fault action and use either the configured fault action or the
 * default.
 *
 * @note If the caller is passing an invalid reference to the application process, it is a fatal
 *       error, the function will not return.
 */
//--------------------------------------------------------------------------------------------------
void le_appProc_ClearFaultAction
(
    le_appProc_RefRef_t appProcRef
        ///< [IN] Application process to start.
);

//--------------------------------------------------------------------------------------------------
/**
 * Starts the application process.  If the application was not running this function will start it
 * first.
 *
 * @return
 *      LE_OK if successful.
 *      LE_FAULT if there was some other error.
 *
 * @note If the caller is passing an invalid reference to the application process, it is a fatal
 *       error, the function will not return.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_appProc_Start
(
    le_appProc_RefRef_t appProcRef
        ///< [IN] Application process to start.
);

//--------------------------------------------------------------------------------------------------
/**
 * Deletes the application process object.
 *
 * @note If the caller is passing an invalid reference to the application process, it is a fatal
 *       error, the function will not return.
 */
//--------------------------------------------------------------------------------------------------
void le_appProc_Delete
(
    le_appProc_RefRef_t appProcRef
        ///< [IN] Application process to start.
);

#endif // LE_APPPROC_INTERFACE_H_INCLUDE_GUARD