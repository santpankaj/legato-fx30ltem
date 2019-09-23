/**
 * @file pa_fwupdate_qmi_common.c
 *
 * Contains code that can be shared between different QMI implementations
 * (e.g. fdt, dssd)
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#include "legato.h"
#include "le_print.h"

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
)
{
    // Check input parameters
    if (versionPtr == NULL)
    {
        LE_KILL_CLIENT("versionPtr is NULL");
        return LE_BAD_PARAMETER;
    }

    if (bufferSize == 0)
    {
        LE_ERROR("Size is zero");
        return LE_BAD_PARAMETER;
    }

    // Read the file containing kernel boot parameters, separated by spaces
    char fileBuffer[512];
    const char fileName[] = "/proc/cmdline";
    FILE *fpPtr;

    if ((fpPtr = fopen(fileName, "r")) == NULL)
    {
        LE_ERROR("File %s not found", fileName);
        return LE_NOT_FOUND;
    }
    if (fgets(fileBuffer, sizeof(fileBuffer), fpPtr) == NULL)
    {
        LE_ERROR("Error reading file %s", fileName);
        fclose(fpPtr);
        return LE_NOT_FOUND;
    }
    if ((strlen(fileBuffer) >= sizeof(fileBuffer) - 1) && (getc(fpPtr) != EOF))
    {
        fclose(fpPtr);
        LE_ERROR("Contents of %s exceeds %d", fileName, sizeof(fileBuffer) - 1);
        return LE_OVERFLOW;
    }
    fclose(fpPtr);

    // Tokenize the file and find the pattern
    char *savePtr;
    const char delimiter[] = " ";
    const char pattern[] = "lkversion=";

    char *tokenPtr = strtok_r(fileBuffer, delimiter, &savePtr);
    while (tokenPtr)
    {
        if (strncmp(tokenPtr, pattern, strlen(pattern)) == 0)
        {
            tokenPtr += strlen(pattern);
            return (le_utf8_Copy(versionPtr, tokenPtr, bufferSize, NULL));
        }
        tokenPtr = strtok_r(NULL, delimiter, &savePtr);
    }
    LE_ERROR("pattern %s not found in file %s", pattern, fileName);
    return LE_NOT_FOUND;
}
