#ifndef QAPI_LWM2M_API_SERVICE_01_H
#define QAPI_LWM2M_API_SERVICE_01_H
/**
  @file qapi_lwm2m_v01.h

  @brief This is the public header file which defines the qapi_lwm2m_api service Data structures.

  This header file defines the types and structures that were defined in
  qapi_lwm2m_api. It contains the constant values defined, enums, structures,
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

/* This file was generated with Tool version 6.13 
   It was generated on: Mon Mar 20 2017 (Spin 0)
   From IDL File: qapi_lwm2m_v01.idl */

/** @defgroup qapi_lwm2m_api_qmi_consts Constant values defined in the IDL */
/** @defgroup qapi_lwm2m_api_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup qapi_lwm2m_api_qmi_enums Enumerated types used in QMI messages */
/** @defgroup qapi_lwm2m_api_qmi_messages Structures sent as QMI messages */
/** @defgroup qapi_lwm2m_api_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup qapi_lwm2m_api_qmi_accessor Accessor for QMI service object */
/** @defgroup qapi_lwm2m_api_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "qapi_swi_common_v01.h"
#include "common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup qapi_lwm2m_api_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define QAPI_LWM2M_API_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define QAPI_LWM2M_API_V01_IDL_MINOR_VERS 0x01
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define QAPI_LWM2M_API_V01_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
#define QAPI_LWM2M_API_V01_MAX_MESSAGE_ID 0x0005
/**
    @}
  */


/** @addtogroup qapi_lwm2m_api_qmi_consts 
    @{ 
  */
#define PREFIX_MAX_LEN_V01 64
#define PAYLOAD_MAX_LEN_V01 1280
#define TOKEN_MAX_LEN_V01 8
#define OBJ_PATH_MAX_LEN_V01 4032
#define URI_PATH_MAX_LEN_V01 256
/**
    @}
  */

/** @addtogroup qapi_lwm2m_api_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t block_offset;
  /**<   offset of the whole data payload for read or write operation. */

  uint32_t block_size;
  /**<   size of the playload can be transfered in next operation report */
}lwm2m_operation_ind_block_mode_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qapi_lwm2m_api_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t is_more;
  /**<   1: more data waiting for transfered; 0 block transfer is done */

  uint8_t is_tlv_encoded;
  /**<   Set to TRUE if payload has been TLV encode */
}lwm2m_operation_req_block_mode_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qapi_lwm2m_api_qmi_messages
    @{
  */
/** Indication Message; This notification indicates that a server is attempting to 
    perform a logical operation on a specified LWM2M object, 
    object instance, resource or resource instance */
typedef struct {

  /* Mandatory */
  uint8_t op_type;
  /**<   Operation type
                                                   0x01 - Read
                                                   0x02 - Discover
                                                   0x04 - Write
                                                   0x08 - Write-Attr
                                                   0x10 - Execute
                                                   0x20 - Create
                                                   0x40 - Delete
                                                   0x80 - Observe
                                                   0x81 - Notify
                                                   0x82 - Observe-Cancel
                                                   0x83 - Observe-Reset
                                                  */

  /* Mandatory */
  uint32_t obj_prefix_len;  /**< Must be set to # of elements in obj_prefix */
  char obj_prefix[PREFIX_MAX_LEN_V01];
  /**<   String value of object prefix, ie. "legato" */

  /* Mandatory */
  uint16_t obj_id;
  /**<   Object ID 
                                                   0xFFFF means obj_ID is not used for the op-type 
                                                  */

  /* Optional */
  uint8_t obj_iid_valid;  /**< Must be set to true if obj_iid is being passed */
  uint16_t obj_iid;
  /**<   Object instance ID
                                                   Mandatory for logical operations Write and Execute
                                                   Optionally for logical operations Read and Discover
                                                   0xFFFF means obj_IID is not specified in the URI
                                                  */

  /* Optional */
  uint8_t res_id_valid;  /**< Must be set to true if res_id is being passed */
  uint16_t res_id;
  /**<   Resource ID.  
                                                   Mandatory for logical operation Execute.
                                                   Optionally for logical operations Read, Write and Discover.
                                                   0xFFFF means res_ID is not specified in the URI
                                                  */

  /* Optional */
  uint8_t payload_valid;  /**< Must be set to true if payload is being passed */
  uint32_t payload_len;  /**< Must be set to # of elements in payload */
  char payload[PAYLOAD_MAX_LEN_V01];
  /**<   The payload for the operation.  
                                                   For example, this would contain the new value in a Write operation
                                                  */

  /* Optional */
  uint8_t blockmode_valid;  /**< Must be set to true if blockmode is being passed */
  lwm2m_operation_ind_block_mode_v01 blockmode;
  /**<   block mode */

  /* Optional */
  uint8_t observe_token_valid;  /**< Must be set to true if observe_token is being passed */
  uint32_t observe_token_len;  /**< Must be set to # of elements in observe_token */
  uint8_t observe_token[TOKEN_MAX_LEN_V01];
  /**<   Observe Token for object.  
                                                   For example, this could be AB CD EF 01
                                                  */

  /* Optional */
  uint8_t content_type_valid;  /**< Must be set to true if content_type is being passed */
  uint16_t content_type;
  /**<   Content-type for Notify message.  
                                                   12118 - CBOR/ZLIB Time Series format
                                                  */
}lwm2m_operation_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_lwm2m_api_qmi_messages
    @{
  */
/** Request Message; Reports the results of a LWM2M operation. */
typedef struct {

  /* Mandatory */
  uint8_t op_type;
  /**<   Operation type that we are reporting the result for
                                                   0x01 - Read
                                                   0x02 - Discover
                                                   0x04 - Write
                                                   0x08 - Write-Attr
                                                   0x10 - Execute
                                                   0x20 - Create
                                                   0x40 - Delete
                                                   0x80 - Observe
                                                   0x81 - Notify
                                                   0x82 - Observe-Cancel
                                                   0x83 - Observe-Reset
                                                  */

  /* Mandatory */
  uint32_t obj_prefix_len;  /**< Must be set to # of elements in obj_prefix */
  char obj_prefix[PREFIX_MAX_LEN_V01];
  /**<   String value of object prefix, ie. "legato" */

  /* Mandatory */
  uint16_t obj_id;
  /**<   Object ID */

  /* Optional */
  uint8_t obj_iid_valid;  /**< Must be set to true if obj_iid is being passed */
  uint16_t obj_iid;
  /**<   Object instance ID
                                                   Mandatory for logical operations Write and Execute
                                                   Optionally for logical operations Read and Discover
                                                   0xFFFF means obj_IID is not specified in the URI
                                                  */

  /* Optional */
  uint8_t res_id_valid;  /**< Must be set to true if res_id is being passed */
  uint16_t res_id;
  /**<   Resource ID.  
                                                   Mandatory for logical operation Execute.
                                                   Optionally for logical operations Read, Write and Discover.
                                                   0xFFFF means res_ID is not specified in the URI
                                                  */

  /* Optional */
  uint8_t payload_valid;  /**< Must be set to true if payload is being passed */
  uint32_t payload_len;  /**< Must be set to # of elements in payload */
  char payload[PAYLOAD_MAX_LEN_V01];
  /**<   The payload for the operation.  
                                                   For example, this would contain the new value in a Write operation
                                                  */

  /* Optional */
  uint8_t op_err_valid;  /**< Must be set to true if op_err is being passed */
  uint8_t op_err;
  /**<   Operation Error
                                                   0x01 - Operation unsupported
                                                   0x02 - Object unsupported
                                                   0x03 - Object Instance unavailable
                                                   0x04 - Resource unsupported
                                                   0x05 - Resource Instance unavailable
                                                   0x06 - Internal error
                                                  */

  /* Optional */
  uint8_t blockmode_valid;  /**< Must be set to true if blockmode is being passed */
  lwm2m_operation_req_block_mode_v01 blockmode;
  /**<   block mode */

  /* Optional */
  uint8_t observe_token_valid;  /**< Must be set to true if observe_token is being passed */
  uint32_t observe_token_len;  /**< Must be set to # of elements in observe_token */
  uint8_t observe_token[TOKEN_MAX_LEN_V01];
  /**<   Observe Token for object.  
                                                   For example, this could be AB CD EF 01
                                                  */

  /* Optional */
  uint8_t content_type_valid;  /**< Must be set to true if content_type is being passed */
  uint16_t content_type;
  /**<   Content-type for Notify message.  
                                                   12118 - CBOR/ZLIB Time Series format
                                                  */
}lwm2m_operation_report_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_lwm2m_api_qmi_messages
    @{
  */
/** Response Message; Reports the results of a LWM2M operation. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/
}lwm2m_operation_report_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}lwm2m_update_required_ind_msg_v01;

/** @addtogroup qapi_lwm2m_api_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t update_supported_obj;
  /**<  
                                            If set to TRUE, the device shall send the supported object list 
                                            to the LWM2M server, else value shall be set to FALSE
                                           */

  uint16_t num_supported_obj;
  /**<        
                                            Number of supported Objects.
                                            Number of "<one object instance path name>" items in the following elements: obj_path
                                           */

  uint32_t obj_path_len;  /**< Must be set to # of elements in obj_path */
  char obj_path[OBJ_PATH_MAX_LEN_V01];
  /**<  
                                            String that specifies one supported object or object instance. 
                                            Ie. "</legato/0/0>"
                                           */
}update_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qapi_lwm2m_api_qmi_messages
    @{
  */
/** Request Message; This message is used to report updated list of object instances. */
typedef struct {

  /* Mandatory */
  update_type_v01 update;
}lwm2m_reg_update_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_lwm2m_api_qmi_messages
    @{
  */
/** Response Message; This message is used to report updated list of object instances. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/
}lwm2m_reg_update_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_lwm2m_api_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t uri_len;  /**< Must be set to # of elements in uri */
  char uri[URI_PATH_MAX_LEN_V01];
  /**<   URI path */
}pkg_uri_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qapi_lwm2m_api_qmi_messages
    @{
  */
/** Request Message; This message is used to request a package download, or cancel, suspend it. */
typedef struct {

  /* Mandatory */
  uint8_t action;
  /**<  
                                     0x00 - Cancel/Suspend
                                     0x01 - Download request
                                     */

  /* Optional */
  uint8_t pkg_uri_valid;  /**< Must be set to true if pkg_uri is being passed */
  pkg_uri_type_v01 pkg_uri;
  /**<   Package URI (in case of download request only) */
}lwm2m_download_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_lwm2m_api_qmi_messages
    @{
  */
/** Response Message; This message is used to request a package download, or cancel, suspend it. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}lwm2m_download_resp_msg_v01;  /* Message */
/**
    @}
  */

/* Conditional compilation tags for message removal */ 
//#define REMOVE_QMI_LWM2M_DOWNLOAD_V01 
//#define REMOVE_QMI_LWM2M_OPERATION_IND_V01 
//#define REMOVE_QMI_LWM2M_OPERATION_REPORT_V01 
//#define REMOVE_QMI_LWM2M_PING_V01 
//#define REMOVE_QMI_LWM2M_REG_UPDATE_V01 
//#define REMOVE_QMI_LWM2M_UPDATE_REQUIRED_IND_V01 

/*Service Message Definition*/
/** @addtogroup qapi_lwm2m_api_qmi_msg_ids
    @{
  */
#define QMI_LWM2M_PING_REQ_V01 0x0000
#define QMI_LWM2M_PING_RESP_V01 0x0000
#define QMI_LWM2M_PING_IND_V01 0x0000
#define QMI_LWM2M_OPERATION_IND_V01 0x0001
#define QMI_LWM2M_OPERATION_REPORT_REQ_V01 0x0002
#define QMI_LWM2M_OPERATION_REPORT_RESP_V01 0x0002
#define QMI_LWM2M_UPDATE_REQUIRED_IND_V01 0x0003
#define QMI_LWM2M_REG_UPDATE_REQ_V01 0x0004
#define QMI_LWM2M_REG_UPDATE_RESP_V01 0x0004
#define QMI_LWM2M_DOWNLOAD_REQ_V01 0x0005
#define QMI_LWM2M_DOWNLOAD_RESP_V01 0x0005
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor 
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro qapi_lwm2m_api_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type qapi_lwm2m_api_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );
 
/** This macro should be used to get the service object */ 
#define qapi_lwm2m_api_get_service_object_v01( ) \
          qapi_lwm2m_api_get_service_object_internal_v01( \
            QAPI_LWM2M_API_V01_IDL_MAJOR_VERS, QAPI_LWM2M_API_V01_IDL_MINOR_VERS, \
            QAPI_LWM2M_API_V01_IDL_TOOL_VERS )
/** 
    @} 
  */


#ifdef __cplusplus
}
#endif
#endif

