/*************
 *
 * $Id$
 *
 * Filename:  sdudefs
 *
 * Purpose:   user definitions for sd
 *
 * Copyright: (c) 2015 Sierra Wireless, Inc.
 *            All rights reserved
 *
 **************/

#ifndef SDUDEFS_H
#define SDUDEFS_H

// General messages types
#define SYNC_MASK  0xF0000
#define SYNC_MNGT  0x10000
#define SYNC_AUDIO 0x20000

#define SOCKET_NAME_LEN 50

// Audio messages identifier enumerate
typedef enum
{
    AUDIO_VOC,
    AUDIO_LOCAL_PLAY,
    AUDIO_MAX
} sd_audio_e;

// Socket management messages identifier
typedef enum
{
    MNGT_SOCKET,
    MNGT_MAX
} sd_mngt_e;

// Messages identifier
#define SD_SYNC_MNGT_SOCKET (SYNC_MNGT | MNGT_SOCKET)

#define SD_SYNC_AUDIO_VOC (SYNC_AUDIO | AUDIO_VOC)
#define SD_SYNC_AUDIO_LOCAL_PLAY (SYNC_AUDIO | AUDIO_LOCAL_PLAY)

// Audio operation enumerate
typedef enum
{
    SYNC_AUDIO_OP_ACTIVATED,
    SYNC_AUDIO_OP_DEACTIVATED

} sd_audio_op_e;

// Audio message structure
typedef struct
{
  sd_audio_op_e operation;     // connection operation
  int           param;         // operation parameter
} sd_audio_type;

// Socket management structure
typedef struct
{
  char sockname[SOCKET_NAME_LEN];     // connection operation
} sd_mngt_type;

typedef struct
{
  int msgId;   // message identifier
  int pid;     // pid of the sender
  int len;     // length of the message payload
} sdheader_type;

typedef struct
{
  sdheader_type header;     // message data
  union                     // message payload
  {
    sd_mngt_type   mngt;    // socket management payload
    sd_audio_type  audio;   // audio payload
  } u;
} sdmsg_type;

// Prototypes
typedef void (*msg_handler_type)(sdmsg_type* msg);
int sdclient_receive_rsp(void* buffer, int len);
int sdclient_send_msg(sdmsg_type* msg);
void sdclient_set_handler( msg_handler_type msg_handler );
void sdclient_init( char* sockName );

#endif //SDUDEFS_H


