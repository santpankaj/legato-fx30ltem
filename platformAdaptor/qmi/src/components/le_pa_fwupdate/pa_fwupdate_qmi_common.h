/** @file pa_fwupdate_qmi_common.h
 *
 * Legato @ref c_pa_fwupdate_qmi_common include file.
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#ifndef LEGATO_PA_FWUPDATE_QMI_COMMON_INCLUDE_GUARD
#define LEGATO_PA_FWUPDATE_QMI_COMMON_INCLUDE_GUARD

#include "legato.h"

//--------------------------------------------------------------------------------------------------
/**
 * Get the app bootloader version string
 *
 * @return
 *      - LE_OK on success
 *      - LE_NOT_FOUND if the version string is not available
 *      - LE_OVERFLOW if version string to big to fit in provided buffer
 *      - LE_BAD_PARAMETER bad parameter
 *      - LE_UNSUPPORTED not supported
 *      - LE_FAULT for any other errors
 */
//--------------------------------------------------------------------------------------------------
le_result_t GetAppBootloaderVersion
(
    char* versionPtr,        ///< [OUT] Version string
    size_t bufferSize        ///< [IN] Size of version buffer
);

#endif // LEGATO_PA_FWUPDATE_QMI_COMMON_INCLUDE_GUARD
