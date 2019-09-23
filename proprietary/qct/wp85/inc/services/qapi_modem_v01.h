#ifndef SIERRA_MODEM_API_SERVICE_H
#define SIERRA_MODEM_API_SERVICE_H
/**
  @file qapi_modem_v01.h
  
  @brief This is the public header file which defines the sierra_modem_api service Data structures.

  This header file defines the types and structures that were defined in 
  sierra_modem_api. It contains the constant values defined, enums, structures,
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

/* This file was generated with Tool version 5.1 
   It was generated on: Mon Jan 19 2015
   From IDL File: qapi_modem_v01.idl */

/** @defgroup sierra_modem_api_qmi_consts Constant values defined in the IDL */
/** @defgroup sierra_modem_api_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup sierra_modem_api_qmi_enums Enumerated types used in QMI messages */
/** @defgroup sierra_modem_api_qmi_messages Structures sent as QMI messages */
/** @defgroup sierra_modem_api_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup sierra_modem_api_qmi_accessor Accessor for QMI service object */
/** @defgroup sierra_modem_api_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "qapi_swi_common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup sierra_modem_api_qmi_version 
    @{ 
  */ 
/** Major Version Number of the IDL used to generate this file */
#define SIERRA_MODEM_API_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define SIERRA_MODEM_API_V01_IDL_MINOR_VERS 0x01
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define SIERRA_MODEM_API_V01_IDL_TOOL_VERS 0x05
/** Maximum Defined Message ID */
#define SIERRA_MODEM_API_V01_MAX_MESSAGE_ID 0x0009;
/** 
    @} 
  */


/** @addtogroup sierra_modem_api_qmi_consts 
    @{ 
  */

/**  Maximum size for a nv operation name */
#define NV_OPERATION_MAX_SIZE_V01 4

/**  Maxium size for a nv item name */
#define NV_ITEM_NAME_MAX_SIZE_V01 64

/**  Maximum size for the NV data in NV operation */
#define NVOP_MAX_DATA_SIZE_V01 4096

/**  Maximum size for a dx item name */
#define DX_OPERATION_MAX_SIZE_V01 4

/**  Maximum size for a dx item name */
#define DX_ITEM_NAME_MAX_SIZE_V01 64

/**  Maximum size for a dx record operation field */
#define DX_REC_OP_MAX_SIZE_V01 16

/**  Maximum size for the count field */
#define DX_COUNT_MAX_SIZE_V01 4

/**  Maximum size for the Vers field */
#define DX_VERS_MAX_SIZE_V01 4

/**  Maximum size for dx data */
#define DXOP_MAX_DATA_SIZE_V01 512

/**  Maximum size for dx record data */
#define DXOP_MAX_REC_DATA_SIZE_V01 2048

/**  Maximum size for dx notification data */
#define DXOP_MAX_NOTIF_DATA_SIZE_V01 128

/**  Maximum size for "short" DX packet
 Over 90% of DX Item packets are less than 64 bytes,
 but we use 150 to pass DX's worst-case, long-name, length check   */
#define DXPACKET_SHORT_MAX_SIZE_V01 150

/**  Maximum number of longs for PC operation */
#define PCOP_MAX_DATA_SIZE_V01 10
/**
    @}
  */

/** @addtogroup sierra_modem_api_qmi_messages
    @{
  */
/** Request Message; This message sends an NV request message */
typedef struct {

  /* Mandatory */
  /*  NV Operation Type
 r - read
 w - write
 dl- delete */
  uint32_t operation_len;  /**< Must be set to # of elements in operation */
  char operation[NV_OPERATION_MAX_SIZE_V01];

  /* Optional */
  /*  NV Item Name */
  uint8_t nvname_valid;  /**< Must be set to true if nvname is being passed */
  uint32_t nvname_len;  /**< Must be set to # of elements in nvname */
  char nvname[NV_ITEM_NAME_MAX_SIZE_V01];

  /* Optional */
  /*  NV Array Index - only valid for nv delete and nv update operation */
  uint8_t nvarrayindex_valid;  /**< Must be set to true if nvarrayindex is being passed */
  uint16_t nvarrayindex;

  /* Optional */
  /*  NV Item Data */
  uint8_t nvdata_valid;  /**< Must be set to true if nvdata is being passed */
  uint32_t nvdata_len;  /**< Must be set to # of elements in nvdata */
  uint8_t nvdata[NVOP_MAX_DATA_SIZE_V01];
}swi_nv_op_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_modem_api_qmi_messages
    @{
  */
/** Response Message; This message sends an NV request message */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  oem_response_type_v01 resp;

  /* Mandatory */
  /*  NV Operation Type
 r - read
 w - write */
  uint32_t operation_len;  /**< Must be set to # of elements in operation */
  char operation[NV_OPERATION_MAX_SIZE_V01];

  /* Optional */
  /*  NV Item Name */
  uint8_t nvname_valid;  /**< Must be set to true if nvname is being passed */
  uint32_t nvname_len;  /**< Must be set to # of elements in nvname */
  char nvname[NV_ITEM_NAME_MAX_SIZE_V01];

  /* Optional */
  /*  NV Item Data */
  uint8_t nvdata_valid;  /**< Must be set to true if nvdata is being passed */
  uint32_t nvdata_len;  /**< Must be set to # of elements in nvdata */
  uint8_t nvdata[NVOP_MAX_DATA_SIZE_V01];
}swi_nv_op_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_modem_api_qmi_messages
    @{
  */
/** Request Message; Used by the apps side to read DX items from the mpss side */
typedef struct {

  /* Mandatory */
  /*  DX Operation Type (read, write, change notification, delete) */
  uint32_t operation_len;  /**< Must be set to # of elements in operation */
  char operation[DX_OPERATION_MAX_SIZE_V01];

  /* Mandatory */
  /*  Packet direction (request, response/notification) */
  char direction;

  /* Mandatory */
  /*  DX Item Name */
  uint32_t dxItem_len;  /**< Must be set to # of elements in dxItem */
  char dxItem[DX_ITEM_NAME_MAX_SIZE_V01];

  /* Mandatory */
  /*  DX Record Operation */
  uint32_t recOp_len;  /**< Must be set to # of elements in recOp */
  char recOp[DX_REC_OP_MAX_SIZE_V01];

  /* Mandatory */
  /*  Count */
  uint32_t count_len;  /**< Must be set to # of elements in count */
  char count[DX_COUNT_MAX_SIZE_V01];

  /* Optional */
  /*  Vers */
  uint8_t vers_valid;  /**< Must be set to true if vers is being passed */
  uint32_t vers_len;  /**< Must be set to # of elements in vers */
  char vers[DX_VERS_MAX_SIZE_V01];

  /* Optional */
  /*  DX Data */
  uint8_t dxData_valid;  /**< Must be set to true if dxData is being passed */
  uint32_t dxData_len;  /**< Must be set to # of elements in dxData */
  uint8_t dxData[DXOP_MAX_DATA_SIZE_V01];
}swi_dx_op_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_modem_api_qmi_messages
    @{
  */
/** Response Message; Used by the apps side to read DX items from the mpss side */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  oem_response_type_v01 resp;

  /* Mandatory */
  /*  DX Operation Type (read, write, change notification, delete) */
  uint32_t operation_len;  /**< Must be set to # of elements in operation */
  char operation[DX_OPERATION_MAX_SIZE_V01];

  /* Mandatory */
  /*  Packet direction (request, response/notification) */
  char direction;

  /* Mandatory */
  /*  DX Item Name */
  uint32_t dxItem_len;  /**< Must be set to # of elements in dxItem */
  char dxItem[DX_ITEM_NAME_MAX_SIZE_V01];

  /* Mandatory */
  /*  DX Record Operation */
  uint32_t recOp_len;  /**< Must be set to # of elements in recOp */
  char recOp[DX_REC_OP_MAX_SIZE_V01];

  /* Mandatory */
  /*  Count */
  uint32_t count_len;  /**< Must be set to # of elements in count */
  char count[DX_COUNT_MAX_SIZE_V01];

  /* Optional */
  /*  Vers */
  uint8_t vers_valid;  /**< Must be set to true if vers is being passed */
  uint32_t vers_len;  /**< Must be set to # of elements in vers */
  char vers[DX_VERS_MAX_SIZE_V01];

  /* Optional */
  /*  DX Data */
  uint8_t dxData_valid;  /**< Must be set to true if dxData is being passed */
  uint32_t dxData_len;  /**< Must be set to # of elements in dxData */
  uint8_t dxData[DXOP_MAX_DATA_SIZE_V01];
}swi_dx_op_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_modem_api_qmi_messages
    @{
  */
/** Request Message; This message sends an PC operation request message */
typedef struct {

  /* Mandatory */
  /*  PC Operation Type */
  uint16_t optype;

  /* Optional */
  /*  PC function parameter  */
  uint8_t data_valid;  /**< Must be set to true if data is being passed */
  uint32_t data_len;  /**< Must be set to # of elements in data */
  uint32_t data[PCOP_MAX_DATA_SIZE_V01];
}swi_pc_op_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_modem_api_qmi_messages
    @{
  */
/** Response Message; This message sends an PC operation request message */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  oem_response_type_v01 resp;

  /* Mandatory */
  /*  PC Operation Type */
  uint16_t optype;

  /* Optional */
  /*  PC function parameter  */
  uint8_t data_valid;  /**< Must be set to true if data is being passed */
  uint32_t data_len;  /**< Must be set to # of elements in data */
  uint32_t data[PCOP_MAX_DATA_SIZE_V01];
}swi_pc_op_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_modem_api_qmi_messages
    @{
  */
/** Indication Message; This message sends an PC operation request message */
typedef struct {

  /* Mandatory */
  /*  PC function parameter  */
  uint32_t data_len;  /**< Must be set to # of elements in data */
  uint32_t data[PCOP_MAX_DATA_SIZE_V01];
}swi_pc_op_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_modem_api_qmi_messages
    @{
  */
/** Request Message; Used by the apps side to read DX records from the mpss side */
typedef struct {

  /* Mandatory */
  /*  DX Operation Type (read, write, change notification, delete) */
  uint32_t operation_len;  /**< Must be set to # of elements in operation */
  char operation[DX_OPERATION_MAX_SIZE_V01];

  /* Mandatory */
  /*  Packet direction (request, response/notification) */
  char direction;

  /* Mandatory */
  /*  DX Item Name */
  uint32_t dxItem_len;  /**< Must be set to # of elements in dxItem */
  char dxItem[DX_ITEM_NAME_MAX_SIZE_V01];

  /* Mandatory */
  /*  DX Record Operation */
  uint32_t recOp_len;  /**< Must be set to # of elements in recOp */
  char recOp[DX_REC_OP_MAX_SIZE_V01];

  /* Mandatory */
  /*  Count */
  uint32_t count_len;  /**< Must be set to # of elements in count */
  char count[DX_COUNT_MAX_SIZE_V01];

  /* Optional */
  /*  Vers */
  uint8_t vers_valid;  /**< Must be set to true if vers is being passed */
  uint32_t vers_len;  /**< Must be set to # of elements in vers */
  char vers[DX_VERS_MAX_SIZE_V01];

  /* Optional */
  /*  DX Data */
  uint8_t dxData_valid;  /**< Must be set to true if dxData is being passed */
  uint32_t dxData_len;  /**< Must be set to # of elements in dxData */
  uint8_t dxData[DXOP_MAX_REC_DATA_SIZE_V01];
}swi_dxrec_op_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_modem_api_qmi_messages
    @{
  */
/** Response Message; Used by the apps side to read DX records from the mpss side */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  oem_response_type_v01 resp;

  /* Mandatory */
  /*  DX Operation Type (read, write, change notification, delete) */
  uint32_t operation_len;  /**< Must be set to # of elements in operation */
  char operation[DX_OPERATION_MAX_SIZE_V01];

  /* Mandatory */
  /*  Packet direction (request, response/notification) */
  char direction;

  /* Mandatory */
  /*  DX Item Name */
  uint32_t dxItem_len;  /**< Must be set to # of elements in dxItem */
  char dxItem[DX_ITEM_NAME_MAX_SIZE_V01];

  /* Mandatory */
  /*  DX Record Operation */
  uint32_t recOp_len;  /**< Must be set to # of elements in recOp */
  char recOp[DX_REC_OP_MAX_SIZE_V01];

  /* Mandatory */
  /*  Count */
  uint32_t count_len;  /**< Must be set to # of elements in count */
  char count[DX_COUNT_MAX_SIZE_V01];

  /* Optional */
  /*  Vers */
  uint8_t vers_valid;  /**< Must be set to true if vers is being passed */
  uint32_t vers_len;  /**< Must be set to # of elements in vers */
  char vers[DX_VERS_MAX_SIZE_V01];

  /* Optional */
  /*  DX Data */
  uint8_t dxData_valid;  /**< Must be set to true if dxData is being passed */
  uint32_t dxData_len;  /**< Must be set to # of elements in dxData */
  uint8_t dxData[DXOP_MAX_REC_DATA_SIZE_V01];
}swi_dxrec_op_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_modem_api_qmi_messages
    @{
  */
/** Indication Message; Used by the apps side to send indication */
typedef struct {

  /* Mandatory */
  /*  packet_len */
  uint16_t packet_len;
  /**<   length of DX Packet  */

  /* Mandatory */
  /*  packet_short */
  uint8_t packet_short[DXPACKET_SHORT_MAX_SIZE_V01];
  /**<   short DX Packet  */
}swi_dxnot_op_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_modem_api_qmi_messages
    @{
  */
/** Request Message; DX Packet (short length) passing between client and service */
typedef struct {

  /* Mandatory */
  /*  packet_len */
  uint16_t packet_len;
  /**<   length of DX Packet  */

  /* Mandatory */
  /*  packet_short */
  uint8_t packet_short[DXPACKET_SHORT_MAX_SIZE_V01];
  /**<   short DX Packet  */
}swi_dxpacket_short_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_modem_api_qmi_messages
    @{
  */
/** Response Message; DX Packet (short length) passing between client and service */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  uint16_t result;
  /**<   QMI Transport result (0=success, 1=fail)  */

  /* Mandatory */
  /*  packet_len */
  uint16_t packet_len;
  /**<   length of DX Packet (byte count)  */

  /* Mandatory */
  /*  packet_short */
  uint8_t packet_short[DXPACKET_SHORT_MAX_SIZE_V01];
  /**<   short DX Packet  */
}swi_dxpacket_short_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * swi_nv_flag_req_msg is empty
 * typedef struct {
 * }swi_nv_flag_req_msg_v01;
 */

/** @addtogroup sierra_modem_api_qmi_messages
    @{
  */
/** Response Message; This message indicate the data reliability feature is enabled/disabled in APP side  */
typedef struct {

  /* Mandatory */
  /*  Pong */
  int32_t data_reliability_flag;
  /**<   Simple 'flag' response  */

  /* Mandatory */
  /*  Result Code */
  oem_response_type_v01 resp;
  /**<   Standard response type.  */
}swi_nv_flag_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_modem_api_qmi_messages
    @{
  */
/** Indication Message; This message indicate the data reliability feature is enabled/disabled in APP side  */
typedef struct {

  /* Mandatory */
  /*  Indication */
  int32_t data_reliability_flag;
  /**<   Simple 'flag' indication  */
}swi_nv_flag_ind_msg_v01;  /* Message */
/**
    @}
  */

/*Service Message Definition*/
/** @addtogroup sierra_modem_api_qmi_msg_ids
    @{
  */
#define SWI_MODEM_PING_REQ_V01 0x0001
#define SWI_MODEM_PING_RESP_V01 0x0001
#define SWI_MODEM_PING_IND_V01 0x0001
#define SWI_NV_OP_REQ_V01 0x0002
#define SWI_NV_OP_RESP_V01 0x0002
#define SWI_DX_OP_REQ_V01 0x0003
#define SWI_DX_OP_RESP_V01 0x0003
#define SWI_PC_OP_REQ_V01 0x0004
#define SWI_PC_OP_RESP_V01 0x0004
#define SWI_PC_OP_IND_V01 0x0005
#define SWI_DXREC_OP_REQ_V01 0x0006
#define SWI_DXREC_OP_RESP_V01 0x0006
#define SWI_DXNOT_OP_IND_V01 0x0007
#define SWI_DXPACKET_SHORT_REQ_V01 0x0008
#define SWI_DXPACKET_SHORT_RESP_V01 0x0008
#define SWI_NV_FLAG_REQ_V01 0x0009
#define SWI_NV_FLAG_RESP_V01 0x0009
#define SWI_NV_FLAG_IND_V01 0x0009
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor 
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro sierra_modem_api_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type sierra_modem_api_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );
 
/** This macro should be used to get the service object */ 
#define sierra_modem_api_get_service_object_v01( ) \
          sierra_modem_api_get_service_object_internal_v01( \
            SIERRA_MODEM_API_V01_IDL_MAJOR_VERS, SIERRA_MODEM_API_V01_IDL_MINOR_VERS, \
            SIERRA_MODEM_API_V01_IDL_TOOL_VERS )
/** 
    @} 
  */


#ifdef __cplusplus
}
#endif
#endif

