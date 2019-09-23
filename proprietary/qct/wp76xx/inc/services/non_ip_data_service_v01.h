#ifndef NIPD_SERVICE_01_H
#define NIPD_SERVICE_01_H
/**
  @file non_ip_data_service_v01.h

  @brief This is the public header file which defines the nipd service Data structures.

  This header file defines the types and structures that were defined in
  nipd. It contains the constant values defined, enums, structures,
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
  Copyright (c) 2017 Qualcomm Technologies, Inc.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.


  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.14.7 
   It was generated on: Thu Jun  8 2017 (Spin 0)
   From IDL File: non_ip_data_service_v01.idl */

/** @defgroup nipd_qmi_consts Constant values defined in the IDL */
/** @defgroup nipd_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup nipd_qmi_enums Enumerated types used in QMI messages */
/** @defgroup nipd_qmi_messages Structures sent as QMI messages */
/** @defgroup nipd_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup nipd_qmi_accessor Accessor for QMI service object */
/** @defgroup nipd_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "common_v01.h"
#include "data_common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup nipd_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define NIPD_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define NIPD_V01_IDL_MINOR_VERS 0x00
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define NIPD_V01_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
#define NIPD_V01_MAX_MESSAGE_ID 0x0024
/**
    @}
  */


/** @addtogroup nipd_qmi_consts
    @{
  */
#define QMI_NIPD_MAX_PAYLOAD_SIZE_V01 16384
/**
    @}
  */

/** @addtogroup nipd_qmi_enums
    @{
  */
typedef enum {
  NIPD_BIND_SUBSCRIPTION_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NIPD_DEFAULT_SUBS_V01 = 0x0000, /**<  Default data subscription \n  */
  NIPD_PRIMARY_SUBS_V01 = 0x0001, /**<  Primary \n  */
  NIPD_SECONDARY_SUBS_V01 = 0x0002, /**<  Secondary \n  */
  NIPD_TERTIARY_SUBS_V01 = 0x0003, /**<  Tertiary \n  */
  NIPD_DONT_CARE_SUBS_V01 = 0x00FF, /**<  Default value used in the absence of
       explicit binding  */
  NIPD_BIND_SUBSCRIPTION_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nipd_bind_subscription_enum_v01;
/**
    @}
  */

/** @addtogroup nipd_qmi_messages
    @{
  */
/** Request Message; Sets the registration state for different QMI_NIPD indications
           for the requesting control point. */
typedef struct {

  /* Optional */
  /*  Report Downlink Data */
  uint8_t report_downlink_data_valid;  /**< Must be set to true if report_downlink_data is being passed */
  uint8_t report_downlink_data;
  /**<   Values:  \n
       - 0 -- Do not report \n
       - 1 -- Report downlink NON IP data
  */
}nipd_indication_register_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nipd_qmi_messages
    @{
  */
/** Response Message; Sets the registration state for different QMI_NIPD indications
           for the requesting control point. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}nipd_indication_register_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nipd_qmi_messages
    @{
  */
/** Request Message; Binds a control point to specified preferences */
typedef struct {

  /* Optional */
  /*  Peripheral Endpoint ID */
  uint8_t ep_id_valid;  /**< Must be set to true if ep_id is being passed */
  data_ep_id_type_v01 ep_id;
  /**<   \n(The peripheral endpoint (physical data channel) to which
       the client binds.)
  */

  /* Optional */
  /*  Mux ID */
  uint8_t mux_id_valid;  /**< Must be set to true if mux_id is being passed */
  uint8_t mux_id;
  /**<   The mux ID of the logical data channel to which
       the client binds; default value is 0. 
  */

  /* Optional */
  /*  Bind Subscription */
  uint8_t bind_subs_valid;  /**< Must be set to true if bind_subs is being passed */
  nipd_bind_subscription_enum_v01 bind_subs;
  /**<   Subscription to which to bind. Values: \n
      - NIPD_DEFAULT_SUBS (0x0000) --  Default data subscription \n 
      - NIPD_PRIMARY_SUBS (0x0001) --  Primary \n 
      - NIPD_SECONDARY_SUBS (0x0002) --  Secondary \n 
      - NIPD_TERTIARY_SUBS (0x0003) --  Tertiary \n 
      - NIPD_DONT_CARE_SUBS (0x00FF) --  Default value used in the absence of
       explicit binding 
 */
}nipd_bind_client_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nipd_qmi_messages
    @{
  */
/** Response Message; Binds a control point to specified preferences */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}nipd_bind_client_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nipd_qmi_messages
    @{
  */
/** Request Message; Gets all the preferences that the control point is bound to. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}nipd_get_client_binding_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup nipd_qmi_messages
    @{
  */
/** Response Message; Gets all the preferences that the control point is bound to. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Bound Peripheral Endpoint ID */
  uint8_t bound_ep_id_valid;  /**< Must be set to true if bound_ep_id is being passed */
  data_ep_id_type_v01 bound_ep_id;
  /**<   \n(The peripheral endpoint (physical data channel) to which
       the client is bound.)
  */

  /* Optional */
  /*  Bound Mux ID */
  uint8_t bound_mux_id_valid;  /**< Must be set to true if bound_mux_id is being passed */
  uint8_t bound_mux_id;
  /**<   The mux ID of the logical data channel to which
       the client is bound. 
  */

  /* Optional */
  /*  Bound Subscription  */
  uint8_t bound_subs_valid;  /**< Must be set to true if bound_subs is being passed */
  nipd_bind_subscription_enum_v01 bound_subs;
  /**<   Subscription to which is bound. Values: \n
      - NIPD_DEFAULT_SUBS (0x0000) --  Default data subscription \n 
      - NIPD_PRIMARY_SUBS (0x0001) --  Primary \n 
      - NIPD_SECONDARY_SUBS (0x0002) --  Secondary \n 
      - NIPD_TERTIARY_SUBS (0x0003) --  Tertiary \n 
      - NIPD_DONT_CARE_SUBS (0x00FF) --  Default value used in the absence of
       explicit binding 
 */
}nipd_get_client_binding_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nipd_qmi_messages
    @{
  */
/** Request Message; Queries the capabilities supported by the QMI_NIPD service. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}nipd_get_capability_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup nipd_qmi_messages
    @{
  */
/** Response Message; Queries the capabilities supported by the QMI_NIPD service. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */

  /* Optional */
  /*  Allowed NON IP Payload Length */
  uint8_t allowed_data_payload_length_valid;  /**< Must be set to true if allowed_data_payload_length is being passed */
  uint32_t allowed_data_payload_length;
  /**<   Allowed payload length for NON IP data transfer in bytes: \n
  */
}nipd_get_capability_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nipd_qmi_messages
    @{
  */
/** Indication Message; Send down link NON IP data to control point. */
typedef struct {

  /* Mandatory */
  /*  Payload */
  uint32_t payload_len;  /**< Must be set to # of elements in payload */
  uint8_t payload[QMI_NIPD_MAX_PAYLOAD_SIZE_V01];
  /**<   \n Downlink data payload.
  */

  /* Optional */
  /*  Final Segment Flag  */
  uint8_t is_final_segment_valid;  /**< Must be set to true if is_final_segment is being passed */
  uint8_t is_final_segment;
  /**<   Set to TRUE when current seqment is final 
       segment of a sequence. Default value: FALSE\n
  */

  /* Optional */
  /*  Segment Number  */
  uint8_t segment_number_valid;  /**< Must be set to true if segment_number is being passed */
  uint8_t segment_number;
  /**<   Current seqment number of a sequence. \n
  */
}nipd_downlink_data_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nipd_qmi_messages
    @{
  */
/** Request Message; Sends uplink NON IP data to modem. */
typedef struct {

  /* Mandatory */
  /*  Payload */
  uint32_t payload_len;  /**< Must be set to # of elements in payload */
  uint8_t payload[QMI_NIPD_MAX_PAYLOAD_SIZE_V01];
  /**<   \n Uplink data payload.
   */

  /* Optional */
  /*  Final Segment Flag  */
  uint8_t is_final_segment_valid;  /**< Must be set to true if is_final_segment is being passed */
  uint8_t is_final_segment;
  /**<   Set to TRUE when current seqment is final 
       segment of a sequence. Default value: FALSE\n
  */

  /* Optional */
  /*  Segment Number  */
  uint8_t segment_number_valid;  /**< Must be set to true if segment_number is being passed */
  uint8_t segment_number;
  /**<   Current seqment number of a sequence. \n
  */

  /* Optional */
  /*  MO Exceptional Data Flag */
  uint8_t is_mo_exceptional_data_valid;  /**< Must be set to true if is_mo_exceptional_data is being passed */
  uint8_t is_mo_exceptional_data;
  /**<   Set to TRUE in order to transmit MO exceptional data. Default value: FALSE \n
  */
}nipd_uplink_data_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nipd_qmi_messages
    @{
  */
/** Response Message; Sends uplink NON IP data to modem. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}nipd_uplink_data_resp_msg_v01;  /* Message */
/**
    @}
  */

/* Conditional compilation tags for message removal */ 
//#define REMOVE_QMI_NIPD_BIND_CLIENT_V01 
//#define REMOVE_QMI_NIPD_DOWNLINK_DATA_IND_V01 
//#define REMOVE_QMI_NIPD_GET_CAPABILITY_V01 
//#define REMOVE_QMI_NIPD_GET_CLIENT_BINDING_V01 
//#define REMOVE_QMI_NIPD_INDICATION_REGISTER_V01 
//#define REMOVE_QMI_NIPD_UPLINK_DATA_V01 

/*Service Message Definition*/
/** @addtogroup nipd_qmi_msg_ids
    @{
  */
#define QMI_NIPD_INDICATION_REGISTER_REQ_V01 0x0003
#define QMI_NIPD_INDICATION_REGISTER_RESP_V01 0x0003
#define QMI_NIPD_BIND_CLIENT_REQ_V01 0x0020
#define QMI_NIPD_BIND_CLIENT_RESP_V01 0x0020
#define QMI_NIPD_GET_CLIENT_BINDING_REQ_V01 0x0021
#define QMI_NIPD_GET_CLIENT_BINDING_RESP_V01 0x0021
#define QMI_NIPD_GET_CAPABILITY_REQ_V01 0x0022
#define QMI_NIPD_GET_CAPABILITY_RESP_V01 0x0022
#define QMI_NIPD_DOWN_LINK_DATA_IND_V01 0x0023
#define QMI_NIPD_UPLINK_DATA_REQ_V01 0x0024
#define QMI_NIPD_UPLINK_DATA_RESP_V01 0x0024
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro nipd_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type nipd_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );

/** This macro should be used to get the service object */
#define nipd_get_service_object_v01( ) \
          nipd_get_service_object_internal_v01( \
            NIPD_V01_IDL_MAJOR_VERS, NIPD_V01_IDL_MINOR_VERS, \
            NIPD_V01_IDL_TOOL_VERS )
/**
    @}
  */


#ifdef __cplusplus
}
#endif
#endif

