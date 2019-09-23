
/*
 * ====================== WARNING ======================
 *
 * THE CONTENTS OF THIS FILE HAVE BEEN AUTO-GENERATED.
 * DO NOT MODIFY IN ANY WAY.
 *
 * ====================== WARNING ======================
 */


#ifndef LE_KERNELMODULE_MESSAGES_H_INCLUDE_GUARD
#define LE_KERNELMODULE_MESSAGES_H_INCLUDE_GUARD


#include "legato.h"

#define PROTOCOL_ID_STR "b9a021649e1b6d35ccdd0293014c964b"

#ifdef MK_TOOLS_BUILD
    extern const char** le_kernelModule_ServiceInstanceNamePtr;
    #define SERVICE_INSTANCE_NAME (*le_kernelModule_ServiceInstanceNamePtr)
#else
    #define SERVICE_INSTANCE_NAME "le_kernelModule"
#endif


#define _MAX_MSG_SIZE 76

// Define the message type for communicating between client and server
typedef struct __attribute__((packed))
{
    uint32_t id;
    uint8_t buffer[_MAX_MSG_SIZE];
}
_Message_t;

#define _MSGID_le_kernelModule_Load 0
#define _MSGID_le_kernelModule_Unload 1


#endif // LE_KERNELMODULE_MESSAGES_H_INCLUDE_GUARD