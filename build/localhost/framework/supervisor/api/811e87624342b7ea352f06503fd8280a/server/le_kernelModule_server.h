

/*
 * ====================== WARNING ======================
 *
 * THE CONTENTS OF THIS FILE HAVE BEEN AUTO-GENERATED.
 * DO NOT MODIFY IN ANY WAY.
 *
 * ====================== WARNING ======================
 */


#ifndef LE_KERNELMODULE_INTERFACE_H_INCLUDE_GUARD
#define LE_KERNELMODULE_INTERFACE_H_INCLUDE_GUARD


#include "legato.h"


//--------------------------------------------------------------------------------------------------
/**
 * Get the server service reference
 */
//--------------------------------------------------------------------------------------------------
le_msg_ServiceRef_t le_kernelModule_GetServiceRef
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * Get the client session reference for the current message
 */
//--------------------------------------------------------------------------------------------------
le_msg_SessionRef_t le_kernelModule_GetClientSessionRef
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * Initialize the server and advertise the service.
 */
//--------------------------------------------------------------------------------------------------
void le_kernelModule_AdvertiseService
(
    void
);


//--------------------------------------------------------------------------------------------------
/**
 * @file le_kernelModule_interface.h
 *
 * Legato @ref c_kernelModule include file.
 *
 * Copyright (C) Sierra Wireless Inc.
 */
//--------------------------------------------------------------------------------------------------
#define LE_KERNELMODULE_NAME_LEN 60

//--------------------------------------------------------------------------------------------------
/**
 */
//--------------------------------------------------------------------------------------------------
#define LE_KERNELMODULE_NAME_LEN_BYTES 61


//--------------------------------------------------------------------------------------------------
/**
 * Load the specified kernel module that was bundled with a Legato system.
 *
 * @return
 *   - LE_OK if the module has been successfully loaded into the kernel.
 *   - LE_NOT_FOUND if the named module was not found in the system.
 *   - LE_FAULT if errors were encountered when loading the module, or one of the module's
 *     dependencies.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_kernelModule_Load
(
    const char* LE_NONNULL moduleName
        ///< [IN] Name of the module to load.
);



//--------------------------------------------------------------------------------------------------
/**
 * Unload the specified module.  The module to be unloaded must be one that was bundled with the
 * system.
 *
 * @return
 *   - LE_OK if the module has been successfully unloaded from the kernel.
 *   - LE_NOT_FOUND if the named module was not found in the system.
 *   - LE_FAULT if errors were encountered during the module, or one of the module's dependencies
 *     unloading.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_kernelModule_Unload
(
    const char* LE_NONNULL moduleName
        ///< [IN] Name of the module to unload.
);


#endif // LE_KERNELMODULE_INTERFACE_H_INCLUDE_GUARD