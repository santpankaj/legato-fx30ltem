/**
 * @file atLinker.h
 *
 * <HR>
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#ifndef _LINKER_H
#define _LINKER_H

#include <pthread.h>

//--------------------------------------------------------------------------------------------------
/**
 * Max response length
 */
//--------------------------------------------------------------------------------------------------
#define MAX_RESP_LEN    1024

//--------------------------------------------------------------------------------------------------
/**
 * Max command length
 */
//--------------------------------------------------------------------------------------------------
#define MAX_CMD_LEN     1024

//--------------------------------------------------------------------------------------------------
/**
 * AtEvent_t struct definition
 */
//--------------------------------------------------------------------------------------------------
typedef struct {
    pthread_mutex_t mutex;  ///< event's mutex
    pthread_cond_t cond;    ///< event's condition
    char cmd[MAX_CMD_LEN];  ///< At command
}
AtEvent_t;

//--------------------------------------------------------------------------------------------------
/**
 * AtLinker_t struct definition
 */
//--------------------------------------------------------------------------------------------------
typedef struct {
    int (*init)(AtEvent_t *atEventPtr);                         ///< Initialize forwarding client
    int (*registerCmds)(char **cmdsPtr, unsigned int count);    ///< Register commands
    int (*release)(void);                                       ///< Release forwarding client
    int (*sendResponse)(char *bufPtr, size_t len);              ///< Respond to the forwarded cmd
}
AtLinker_t;

//--------------------------------------------------------------------------------------------------
/**
 * Initialize atLinker
 */
//--------------------------------------------------------------------------------------------------
void atLinker_Init
(
    AtLinker_t *atLinkerPtr ///< pointer to atLinker struct
);

#endif /* _ATQMILINKER_H */
