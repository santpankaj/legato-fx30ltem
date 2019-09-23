/**
 * @file utils.c
 *
 * <HR>
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#include <legato.h>
#include <string.h>
#include <qmi_atcop_srvc.h>
#include "utils.h"

//--------------------------------------------------------------------------------------------------
/**
 * Check if a string ends with substring
 */
//--------------------------------------------------------------------------------------------------
int utils_EndsWith
(
    const char* strPtr,
    const char* subStrPtr
)
{
    size_t strLen, subStrLen;

    strLen = strlen(strPtr);
    subStrLen = strlen(subStrPtr);

    if (subStrLen <= strLen)
    {
       return (0 == memcmp(subStrPtr, strPtr + (strLen - subStrLen), subStrLen));
    }

    return 0;
}

//--------------------------------------------------------------------------------------------------
/**
 * Check if we're done receiving a response or if the response is an unsolicited
 */
//--------------------------------------------------------------------------------------------------
int utils_CheckResponse
(
    char* respPtr
)
{
    if (utils_EndsWith(respPtr, "\r\nOK\r\n") ||
        utils_EndsWith(respPtr, "\r\nERROR\r\n") ||
        strcasestr(respPtr, "+CME ERROR"))
    {
        return 0;
    }

    return -1;
}

//--------------------------------------------------------------------------------------------------
/**
 * Build forwarded AT command
 *
 * Example:
 *      dataPtr->at_name = +wdss
 *      CMD_ACTION:
 *          cmdPtr = at+wdss
 *      CMD_READ:
 *          cmdPtr = at+wdss?
 *      CMD_TEST:
 *          cmdPtr = at+wdss=?
 *      CMD_PARAM:
 *          cmdPtr = at+wdss=1,1
 */
//--------------------------------------------------------------------------------------------------
void utils_BuildAtCommand
(
    qmi_atcop_at_cmd_fwd_ind_type*  dataPtr,    ///< [IN] Forwarded AT command
    char*                           cmdPtr      ///< [OUT] Built AT command
)
{
    int i, size = 0;
    char cmd[QMI_CMD_LEN] = {0};

    switch (dataPtr->op_code)
    {
        case CMD_ACTION:
            snprintf(cmd, QMI_CMD_LEN, "at%s\r\n", dataPtr->at_name);
            break;
        case CMD_READ:
            snprintf(cmd, QMI_CMD_LEN, "at%s?\r\n", dataPtr->at_name);
            break;
        case CMD_TEST:
            snprintf(cmd, QMI_CMD_LEN, "at%s=?\r\n", dataPtr->at_name);
            break;
        case CMD_PARAM:
            size += snprintf(cmd, QMI_CMD_LEN, "at%s=", dataPtr->at_name);
            for (i=0; i< dataPtr->num_tokens; i++)
            {
                size += snprintf(cmd+size, QMI_CMD_LEN-size, "%s,", dataPtr->tokens[i]);
            }
            snprintf(cmd+size-1, QMI_CMD_LEN-size+1, "\r\n");
            break;
        default:
            LE_ERROR("unknown command type: %ld", dataPtr->op_code);
            snprintf(cmd, QMI_CMD_LEN, "invalid");
            break;
    }

    memset(cmdPtr, 0, QMI_CMD_LEN);
    memcpy(cmdPtr, cmd, strlen(cmd));
}
