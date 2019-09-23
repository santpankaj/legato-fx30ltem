#ifndef QAPI_COMMON_SERVICE_01_H
#define QAPI_COMMON_SERVICE_01_H
/**
  @file qapi_common_v01.h
  
  @brief This is the public header file which defines the qapi_common service Data structures.

  This header file defines the types and structures that were defined in 
  qapi_common. It contains the constant values defined, enums, structures,
  messages, and service message IDs (in that order) Structures that were 
  defined in the IDL as messages contain mandatory elements, optional 
  elements, a combination of mandatory and optional elements (mandatory 
  always come before optionals in the structure), or nothing (null message)
   
  An optional element in a message is preceded by a uint8_t value that must be
  set to true if the element is going to be included. When decoding a received
  message, the uint8_t values will be set to true or false by the decode
  routine, and should be checked before accessing the values that they
  correspond to. 
   
  Variable sized arrays are defined as static sized arrays with an unsigned
  integer (32 bit) preceding it that must be set to the number of elements
  in the array that are valid. For Example:
   
  uint32_t test_opaque_len;
  uint8_t test_opaque[16];
   
  If only 4 elements are added to test_opaque[] then test_opaque_len must be
  set to 4 before sending the message.  When decoding, the _len value is set 
  by the decode routine and should be checked so that the correct number of 
  elements in the array will be accessed. 

*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  

  
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.2 
   It was generated on: Thu Jun 22 2017 (Spin 1)
   From IDL File: qapi_common_v01.idl */

/** @defgroup qapi_common_qmi_consts Constant values defined in the IDL */
/** @defgroup qapi_common_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup qapi_common_qmi_enums Enumerated types used in QMI messages */
/** @defgroup qapi_common_qmi_messages Structures sent as QMI messages */
/** @defgroup qapi_common_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup qapi_common_qmi_accessor Accessor for QMI service object */
/** @defgroup qapi_common_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "common_v01.h"

/* NOTE: imorrison 2014:03:18 - these need to be on the public API so
 * they are copied here.  This set is the union of supported messages
 * in the apps and modem specific .h files, which are not public
 */
#define QAPI_PING_REQ_V01 0x0001
#define QAPI_PING_RESP_V01 0x0001
#define QAPI_PING_IND_V01 0x0001
#define QAPI_RESET_REQ_V01 0x0002
#define QAPI_RESET_RESP_V01 0x0002
#define QAPI_NV_OP_REQ_V01 0x0003
#define QAPI_NV_OP_RESP_V01 0x0003
#define QAPI_NV_OP_IND_V01 0x0003
#define QAPI_UBIST_OP_REQ_V01 0x0004
#define QAPI_UBIST_OP_RESP_V01 0x0004
#define QAPI_UBIST_OP_IND_V01 0x0004
#define QAPI_UD_OP_REQ_V01 0x0005
#define QAPI_UD_OP_RESP_V01 0x0005
#define QAPI_QFPROM_OP_REQ_V01 0x0006
#define QAPI_QFPROM_OP_RESP_V01 0x0006
#define QAPI_PC_OP_REQ_V01 0x0007
#define QAPI_PC_OP_RESP_V01 0x0007
#define QAPI_DATA_LINUX_OP_REQ_V01 0x0008
#define QAPI_DATA_LINUX_OP_RESP_V01 0x0008
#define QAPI_PCIE_TEST_OP_REQ_V01 0x0009
#define QAPI_PCIE_TEST_OP_RESP_V01 0x0009
#define QAPI_SDIO_TEST_OP_REQ_V01 0x000A
#define QAPI_SDIO_TEST_OP_RESP_V01 0x000A
#define QAPI_DBI_OP_REQ_V01 0x000B
#define QAPI_DBI_OP_RESP_V01 0x000B
#define QAPI_LE_STATUS_OP_REQ_V01 0x000C
#define QAPI_LE_STATUS_OP_RESP_V01 0x000C
#define QAPI_LE_APP_STATUS_OP_REQ_V01 0x000D
#define QAPI_LE_APP_STATUS_OP_RESP_V01 0x000D
#define QAPI_FM_OP_REQ_V01 0x000E
#define QAPI_FM_OP_RESP_V01 0x000E
#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup qapi_common_qmi_version 
    @{ 
  */ 
/** Major Version Number of the IDL used to generate this file */
#define QAPI_COMMON_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define QAPI_COMMON_V01_IDL_MINOR_VERS 0x02
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define QAPI_COMMON_V01_IDL_TOOL_VERS 0x06

/** 
    @} 
  */


/** @addtogroup qapi_common_qmi_consts 
    @{ 
  */

/**  Maximum size of NV data */
#define QAPI_NV_DATA_SIZE_MAX_V01 4096

/**  Maximum number of NV tag to delete */
#define QAPI_NV_DEL_TAG_MAX_V01 255

/**  Maximum number of longs for UBIST data */
#define UBISTOP_MAX_DATA_SIZE_V01 22
#define QMI_PCIE_DEVICES_SIZE_V01 1024
#define QMI_SDIO_DEVICES_SIZE_V01 1024

/**  Maximum size of DBI ARGS */
#define QAPI_DBI_ARGS_SIZE_MAX_V01 10
#define QAPI_LE_STATUS_DATA_SIZE_V01 1024
#define QAPI_LE_APP_REQ_DATA_SIZE_V01 50
#define QAPI_LE_APP_STATUS_DATA_SIZE_V01 512

/**  Maximum size of FM data */
#define QAPI_FM_DATA_SIZE_MAX_V01 2000
#define QAPI_FM_PATH_SIZE_MAX_V01 256
/**
    @}
  */

/** @addtogroup qapi_common_qmi_aggregates
    @{
  */
typedef struct {

  /*  ID of device containing file */
  uint32_t st_dev;

  /*  inode number */
  uint32_t st_ino;

  /*  protection */
  uint16_t st_mode;

  /*  number of hard links */
  uint8_t st_nlink;

  /*  user ID of owner */
  uint16_t st_uid;

  /*  group ID of owner */
  uint16_t st_gid;

  /*  device ID (if special file) */
  uint32_t st_rdev;

  /*  total size, in bytes */
  int32_t st_size;

  /*  blocksize for file system I/O */
  uint32_t st_blksize;

  /*  number of 512B blocks allocated */
  uint32_t st_blocks;

  /*  time of last access */
  uint32_t st_atme;

  /*  time of last modification */
  uint32_t st_mtme;

  /*  time of last status change */
  uint32_t st_ctme;
}qapi_fm_stat_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qapi_common_qmi_messages
    @{
  */
/** Request Message; This message tests basic message passing between
  client and service */
typedef struct {

  /* Mandatory */
  /*  Ping */
  char ping[5];
  /**<   Simple 'ping' request */
}qapi_ping_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_common_qmi_messages
    @{
  */
/** Response Message; This message tests basic message passing between
  client and service */
typedef struct {

  /* Mandatory */
  /*  Status of the request */
  qmi_response_type_v01 status;

  /* Mandatory */
  /*  Pong */
  char pong[5];
  /**<   Simple 'pong' response */
}qapi_ping_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_common_qmi_messages
    @{
  */
/** Indication Message; This message tests basic message passing between
  client and service */
typedef struct {

  /* Mandatory */
  /*  Status of the indication */
  qmi_response_type_v01 status;

  /* Mandatory */
  /*  SWI_TBD imorrison 2015:04:27 - temporary workaround to avoid
 duplicate indications.
 !!! Private data, not to be used by clients or services !!! */
  uint32_t ind_count;

  /* Mandatory */
  /*  Hello indication */
  char hello[6];
  /**<   Simple 'hello' indication */
}qapi_ping_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_common_qmi_messages
    @{
  */
/** Request Message; This message sends an reset command to apps proc */
typedef struct {

  /* Mandatory */
  /*  Reset Type
 0 - reset
 1 - power down
 2 - boot & hold
 0xFF00 and above - crash test */
  uint16_t type;
}qapi_reset_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_common_qmi_messages
    @{
  */
/** Response Message; This message sends an reset command to apps proc */
typedef struct {

  /* Mandatory */
  /*  Status of the request */
  qmi_response_type_v01 status;

  /* Mandatory */
  /*  echo of the requested reset Type
 0 - reset
 1 - power down
 2 - boot & hold */
  uint16_t type;
}qapi_reset_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_common_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t delallflag;

  uint8_t arrayidx;

  uint32_t nvdeltag;
}qapi_nv_del_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qapi_common_qmi_messages
    @{
  */
/** Request Message; This message sends an NV request message */
typedef struct {

  /* Mandatory */
  /*  NV Operation Type */
  uint16_t operation;

  /* Optional */
  /*  NV Item Tag */
  uint8_t nvtag_valid;  /**< Must be set to true if nvtag is being passed */
  uint32_t nvtag;

  /* Optional */
  /*  NV Item Data */
  uint8_t nvdata_valid;  /**< Must be set to true if nvdata is being passed */
  uint32_t nvdata_len;  /**< Must be set to # of elements in nvdata */
  uint8_t nvdata[QAPI_NV_DATA_SIZE_MAX_V01];

  /* Optional */
  /*  NV delete tag */
  uint8_t nvdelinfo_valid;  /**< Must be set to true if nvdelinfo is being passed */
  uint32_t nvdelinfo_len;  /**< Must be set to # of elements in nvdelinfo */
  qapi_nv_del_info_type_v01 nvdelinfo[QAPI_NV_DEL_TAG_MAX_V01];
}qapi_nv_op_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_common_qmi_messages
    @{
  */
/** Response Message; This message sends an NV request message */
typedef struct {

  /* Mandatory */
  /*  Status of the request */
  qmi_response_type_v01 status;

  /* Mandatory */
  /*  NV Operation Type */
  uint16_t operation;

  /* Optional */
  /*  NV Item Tag */
  uint8_t nvtag_valid;  /**< Must be set to true if nvtag is being passed */
  uint32_t nvtag;

  /* Optional */
  /*  NV Item Data */
  uint8_t nvdata_valid;  /**< Must be set to true if nvdata is being passed */
  uint32_t nvdata_len;  /**< Must be set to # of elements in nvdata */
  uint8_t nvdata[QAPI_NV_DATA_SIZE_MAX_V01];
}qapi_nv_op_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_common_qmi_messages
    @{
  */
/** Indication Message; This message sends an NV request message */
typedef struct {

  /* Mandatory */
  /*  Status of the indication */
  qmi_response_type_v01 status;

  /* Mandatory */
  /*  SWI_TBD imorrison 2015:04:27 - temporary workaround to avoid
 duplicate indications.
 !!! Private data, not to be used by clients or services !!! */
  uint32_t ind_count;

  /* Mandatory */
  /*  NV Operation Type */
  uint16_t operation;

  /* Optional */
  /*  NV Item Tag */
  uint8_t nvtag_valid;  /**< Must be set to true if nvtag is being passed */
  uint32_t nvtag;

  /* Optional */
  /*  NV Item Data */
  uint8_t nvdata_valid;  /**< Must be set to true if nvdata is being passed */
  uint32_t nvdata_len;  /**< Must be set to # of elements in nvdata */
  uint8_t nvdata[QAPI_NV_DATA_SIZE_MAX_V01];
}qapi_nv_op_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_common_qmi_messages
    @{
  */
/** Request Message; This message sends an UBIST operation request message */
typedef struct {

  /* Mandatory */
  /*  UBIST Operation Type */
  uint16_t optype;

  /* Optional */
  /*  UBIST function parameter */
  uint8_t data_valid;  /**< Must be set to true if data is being passed */
  uint32_t data_len;  /**< Must be set to # of elements in data */
  uint32_t data[UBISTOP_MAX_DATA_SIZE_V01];
}qapi_ubist_op_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_common_qmi_messages
    @{
  */
/** Response Message; This message sends an UBIST operation request message */
typedef struct {

  /* Mandatory */
  /*  Status of the request */
  qmi_response_type_v01 status;

  /* Mandatory */
  /*  UBIST Operation Type */
  uint16_t optype;

  /* Optional */
  /*  UBIST function parameter */
  uint8_t data_valid;  /**< Must be set to true if data is being passed */
  uint32_t data_len;  /**< Must be set to # of elements in data */
  uint32_t data[UBISTOP_MAX_DATA_SIZE_V01];
}qapi_ubist_op_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_common_qmi_messages
    @{
  */
/** Indication Message; This message sends an UBIST operation request message */
typedef struct {

  /* Mandatory */
  /*  Status of the indication */
  qmi_response_type_v01 status;

  /* Mandatory */
  /*  SWI_TBD imorrison 2015:04:27 - temporary workaround to avoid
 duplicate indications.
 !!! Private data, not to be used by clients or services !!! */
  uint32_t ind_count;

  /* Mandatory */
  /*  UBIST indication */
  uint32_t data_len;  /**< Must be set to # of elements in data */
  uint32_t data[UBISTOP_MAX_DATA_SIZE_V01];
}qapi_ubist_op_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_common_qmi_messages
    @{
  */
/** Request Message; This message sends an UD operation request message */
typedef struct {

  /* Mandatory */
  /*  UD host_sleep_state */
  uint8_t host_sleep_state;
}qapi_ud_op_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_common_qmi_messages
    @{
  */
/** Response Message; This message sends an UD operation request message */
typedef struct {

  /* Mandatory */
  /*  Status of the request */
  qmi_response_type_v01 status;
}qapi_ud_op_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_common_qmi_messages
    @{
  */
/** Request Message; This message sends an QFPROM request message */
typedef struct {

  /* Mandatory */
  /*  Operation Type */
  uint16_t operation;

  /* Mandatory */
  /*  Row address */
  uint32_t row_address;

  /* Optional */
  /*  Fuse data */
  uint8_t fuse_data_valid;  /**< Must be set to true if fuse_data is being passed */
  uint64_t fuse_data;

  /* Optional */
  /*  Fuse data bitmask */
  uint8_t bitmask_valid;  /**< Must be set to true if bitmask is being passed */
  uint64_t bitmask;
}qapi_qfprom_op_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_common_qmi_messages
    @{
  */
/** Response Message; This message sends an QFPROM request message */
typedef struct {

  /* Mandatory */
  /*  Status of the request */
  qmi_response_type_v01 status;

  /* Mandatory */
  /*  Operation Type */
  uint16_t operation;

  /* Mandatory */
  /*  Row address */
  uint32_t row_address;

  /* Optional */
  /*  Fuse Data */
  uint8_t fuse_data_valid;  /**< Must be set to true if fuse_data is being passed */
  uint64_t fuse_data;
}qapi_qfprom_op_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_common_qmi_messages
    @{
  */
/** Request Message; This message sends an PC operation request message */
typedef struct {

  /* Mandatory */
  /*  PC Operation Type */
  uint16_t optype;

  /* Optional */
  /*  PC Client type */
  uint8_t pc_client_valid;  /**< Must be set to true if pc_client is being passed */
  uint16_t pc_client;

  /* Optional */
  /*  PC mode request data */
  uint8_t pc_mode_valid;  /**< Must be set to true if pc_mode is being passed */
  uint16_t pc_mode;
}qapi_pc_op_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_common_qmi_messages
    @{
  */
/** Response Message; This message sends an PC operation request message */
typedef struct {

  /* Mandatory */
  /*  Status of the request */
  qmi_response_type_v01 status;

  /* Mandatory */
  /*  PC Operation Type */
  uint16_t optype;
}qapi_pc_op_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_common_qmi_messages
    @{
  */
/** Request Message; This message sends an Data Linux operation request message */
typedef struct {

  /* Mandatory */
  /*  Data Linux Operation Type */
  uint32_t optype;

  /* Optional */
  /*  Data Call Profile ID */
  uint8_t profile_id_valid;  /**< Must be set to true if profile_id is being passed */
  uint32_t profile_id;

  /* Optional */
  /*  IP Forward Flag */
  uint8_t ip_version_valid;  /**< Must be set to true if ip_version is being passed */
  uint32_t ip_version;
}qapi_data_linux_op_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_common_qmi_messages
    @{
  */
/** Response Message; This message sends an Data Linux operation request message */
typedef struct {

  /* Mandatory */
  /*  Status of the request */
  qmi_response_type_v01 status;
}qapi_data_linux_op_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_common_qmi_messages
    @{
  */
/** Request Message; This message sends an PCIE test operation request message */
typedef struct {

  /* Mandatory */
  /*  PCIE Test Operation Type */
  uint32_t optype;
}qapi_pcie_test_op_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_common_qmi_messages
    @{
  */
/** Response Message; This message sends an PCIE test operation request message */
typedef struct {

  /* Mandatory */
  /*  Status of the request */
  qmi_response_type_v01 status;

  /* Optional */
  /*  PCIE Devices Info */
  uint8_t devices_valid;  /**< Must be set to true if devices is being passed */
  char devices[QMI_PCIE_DEVICES_SIZE_V01 + 1];
}qapi_pcie_test_op_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_common_qmi_messages
    @{
  */
/** Request Message; This message sends an SDIO test operation request message */
typedef struct {

  /* Mandatory */
  /*  SDIO Test Operation Type */
  uint32_t optype;
}qapi_sdio_test_op_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_common_qmi_messages
    @{
  */
/** Response Message; This message sends an SDIO test operation request message */
typedef struct {

  /* Mandatory */
  /*  Status of the request */
  qmi_response_type_v01 status;

  /* Optional */
  /*  SDIO Devices Info */
  uint8_t devices_valid;  /**< Must be set to true if devices is being passed */
  char devices[QMI_SDIO_DEVICES_SIZE_V01 + 1];
}qapi_sdio_test_op_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_common_qmi_messages
    @{
  */
/** Request Message; This message sends an DBI request message */
typedef struct {

  /* Mandatory */
  /*  DBI Request Command Type */
  uint32_t req_type;

  /* Mandatory */
  /*  DBI Request Command */
  uint32_t cmd;

  /* Mandatory */
  /*  DBI Request Band Index */
  uint32_t band_idx;

  /* Mandatory */
  /*  DBI ARGS */
  int32_t args[QAPI_DBI_ARGS_SIZE_MAX_V01];
}qapi_dbi_op_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_common_qmi_messages
    @{
  */
/** Response Message; This message sends an DBI request message */
typedef struct {

  /* Mandatory */
  /*  Status of the request */
  qmi_response_type_v01 status;

  /* Mandatory */
  /*  DBI Response Type */
  uint32_t resp_type;

  /* Mandatory */
  /*  DBI Response Value */
  uint32_t value;

  /* Mandatory */
  /*  DBI Rsponse ARGS */
  int32_t args[QAPI_DBI_ARGS_SIZE_MAX_V01];
}qapi_dbi_op_resp_msg_v01;  /* Message */
/**
    @}
  */



/** @addtogroup qapi_common_qmi_messages
    @{
  */
/** Request Message; This message sends a FM operation request */
typedef struct {

  /* Mandatory */
  /*  FM Operation Type */
  uint16_t operation;

  /* Optional */
  /*  FM Handle */
  uint8_t fmhandle_valid;  /**< Must be set to true if fmhandle is being passed */
  int32_t fmhandle;

  /* Optional */
  /*  FM Path */
  uint8_t fmpath_valid;  /**< Must be set to true if fmpath is being passed */
  uint32_t fmpath_len;  /**< Must be set to # of elements in fmpath */
  uint8_t fmpath[QAPI_FM_PATH_SIZE_MAX_V01];

  /* Optional */
  /*  FM Oflag */
  uint8_t fmoflag_valid;  /**< Must be set to true if fmoflag is being passed */
  int32_t fmoflag;

  /* Optional */
  /*  FM Mode */
  uint8_t fmmode_valid;  /**< Must be set to true if fmmode is being passed */
  uint32_t fmmode;

  /* Optional */
  /*  FM Data */
  uint8_t fmdata_valid;  /**< Must be set to true if fmdata is being passed */
  uint32_t fmdata_len;  /**< Must be set to # of elements in fmdata */
  uint8_t fmdata[QAPI_FM_DATA_SIZE_MAX_V01];
}qapi_fm_op_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_common_qmi_messages
    @{
  */
/** Response Message; This message sends a FM operation request */
typedef struct {

  /* Mandatory */
  /*  Status of the request */
  qmi_response_type_v01 status;

  /* Mandatory */
  /*  FM Operation Type */
  uint16_t operation;

  /* Optional */
  /*  FM Handle */
  uint8_t fmhandle_valid;  /**< Must be set to true if fmhandle is being passed */
  int32_t fmhandle;

  /* Optional */
  /*  FM Stat */
  uint8_t fmstat_valid;  /**< Must be set to true if fmstat is being passed */
  qapi_fm_stat_type_v01 fmstat;

  /* Optional */
  /*  FM Path */
  uint8_t fmpath_valid;  /**< Must be set to true if fmpath is being passed */
  uint32_t fmpath_len;  /**< Must be set to # of elements in fmpath */
  uint8_t fmpath[QAPI_FM_PATH_SIZE_MAX_V01];

  /* Optional */
  /*  FM Data */
  uint8_t fmdata_valid;  /**< Must be set to true if fmdata is being passed */
  uint32_t fmdata_len;  /**< Must be set to # of elements in fmdata */
  uint8_t fmdata[QAPI_FM_DATA_SIZE_MAX_V01];
}qapi_fm_op_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_common_qmi_messages
    @{
  */
/** Request Message; This message sends an get LE status operation request message */
typedef struct {

  /* Mandatory */
  /*  LE STATUS Operation Type */
  uint32_t optype;
}qapi_le_status_op_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_common_qmi_messages
    @{
  */
/** Response Message; This message sends an get LE status operation request message */
typedef struct {

  /* Mandatory */
  /*  Status of the request */
  qmi_response_type_v01 status;

  /* Optional */
  /*  LEGATO Devices Info */
  uint8_t lestatus_valid;  /**< Must be set to true if lestatus is being passed */
  char lestatus[QAPI_LE_STATUS_DATA_SIZE_V01 + 1];
}qapi_le_status_op_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_common_qmi_messages
    @{
  */
/** Request Message; This message sends an get LE status operation request message */
typedef struct {

  /* Mandatory */
  /*  APP Test Operation Type */
  uint32_t optype;

  /* Optional */
  /*  LE APP Req Data */
  uint8_t appstatus_valid;  /**< Must be set to true if appstatus is being passed */
  char appstatus[QAPI_LE_APP_REQ_DATA_SIZE_V01 + 1];
}qapi_le_app_status_op_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_common_qmi_messages
    @{
  */
/** Response Message; This message sends an get LE status operation request message */
typedef struct {

  /* Mandatory */
  /*  Status of the request */
  qmi_response_type_v01 status;

  /* Optional */
  /*  LEAPP Info */
  uint8_t appstatus_valid;  /**< Must be set to true if appstatus is being passed */
  char appstatus[QAPI_LE_APP_STATUS_DATA_SIZE_V01 + 1];
}qapi_le_app_status_op_resp_msg_v01;  /* Message */
/**
    @}
  */
/*Extern Definition of Type Table Object*/
/*THIS IS AN INTERNAL OBJECT AND SHOULD ONLY*/
/*BE ACCESSED BY AUTOGENERATED FILES*/
extern const qmi_idl_type_table_object qapi_common_qmi_idl_type_table_object_v01;


#ifdef __cplusplus
}
#endif
#endif

