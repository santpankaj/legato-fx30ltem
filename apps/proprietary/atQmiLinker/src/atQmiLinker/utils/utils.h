/**
 * @file utils.h
 *
 * <HR>
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#ifndef _UTILS_H
#define _UTILS_H

#include <qmi_atcop_srvc.h>

//--------------------------------------------------------------------------------------------------
/**
 * Forworded AT command max size
 */
//--------------------------------------------------------------------------------------------------
#define QMI_CMD_LEN             (QMI_ATCOP_AT_CMD_NAME_MAX_LEN + \
                                (QMI_ATCOP_MAX_TOKEN_SIZE*QMI_ATCOP_MAX_TOKENS))

//--------------------------------------------------------------------------------------------------
/**
 * Command type action
 */
//--------------------------------------------------------------------------------------------------
#define CMD_ACTION              QMI_ATCOP_NA

//--------------------------------------------------------------------------------------------------
/**
 * Command type read
 */
//--------------------------------------------------------------------------------------------------
#define CMD_READ                (QMI_ATCOP_QU | QMI_ATCOP_NA)

//--------------------------------------------------------------------------------------------------
/**
 * Command type test
 */
//--------------------------------------------------------------------------------------------------
#define CMD_TEST                (QMI_ATCOP_QU | QMI_ATCOP_EQ | QMI_ATCOP_NA)

//--------------------------------------------------------------------------------------------------
/**
 * Command type parameter
 */
//--------------------------------------------------------------------------------------------------
#define CMD_PARAM               (QMI_ATCOP_AR | QMI_ATCOP_EQ | QMI_ATCOP_NA)

//--------------------------------------------------------------------------------------------------
/**
 * Check if we're done receiving a response or if the response is an unsolicited
 */
//--------------------------------------------------------------------------------------------------
int utils_CheckResponse
(
    char* respPtr
);

//--------------------------------------------------------------------------------------------------
/**
 * Check if a string ends with substring
 */
//--------------------------------------------------------------------------------------------------
int utils_EndsWith
(
    const char* strPtr,
    const char* subStrPtr
);

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
);

#endif /* _UTILS_H */
