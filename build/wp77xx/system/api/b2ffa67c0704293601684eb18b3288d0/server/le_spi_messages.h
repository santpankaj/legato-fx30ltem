
/*
 * ====================== WARNING ======================
 *
 * THE CONTENTS OF THIS FILE HAVE BEEN AUTO-GENERATED.
 * DO NOT MODIFY IN ANY WAY.
 *
 * ====================== WARNING ======================
 */


#ifndef LE_SPI_MESSAGES_H_INCLUDE_GUARD
#define LE_SPI_MESSAGES_H_INCLUDE_GUARD


#include "legato.h"

#define PROTOCOL_ID_STR "740a490bba2182b36d04362adb42f669"

#ifdef MK_TOOLS_BUILD
    extern const char** le_spi_ServiceInstanceNamePtr;
    #define SERVICE_INSTANCE_NAME (*le_spi_ServiceInstanceNamePtr)
#else
    #define SERVICE_INSTANCE_NAME "le_spi"
#endif


#define _MAX_MSG_SIZE 2072

// Define the message type for communicating between client and server
typedef struct __attribute__((packed))
{
    uint32_t id;
    uint8_t buffer[_MAX_MSG_SIZE];
}
_Message_t;

#define _MSGID_le_spi_Open 0
#define _MSGID_le_spi_Close 1
#define _MSGID_le_spi_Configure 2
#define _MSGID_le_spi_WriteReadHD 3
#define _MSGID_le_spi_WriteHD 4
#define _MSGID_le_spi_ReadHD 5
#define _MSGID_le_spi_WriteReadFD 6


#endif // LE_SPI_MESSAGES_H_INCLUDE_GUARD