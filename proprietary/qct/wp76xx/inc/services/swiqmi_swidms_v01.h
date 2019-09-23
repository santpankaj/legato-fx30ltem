#ifndef SWI_DMS_SERVICE_01_H
#define SWI_DMS_SERVICE_01_H
/**
  @file swiqmi_swidms_v01.h
  
  @brief This is the public header file which defines the swi_dms service Data structures.

  This header file defines the types and structures that were defined in 
  swi_dms. It contains the constant values defined, enums, structures,
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
   It was generated on: Fri Mar 30 2018 (Spin 1)
   From IDL File: swiqmi_swidms_v01.idl */

/** @defgroup swi_dms_qmi_consts Constant values defined in the IDL */
/** @defgroup swi_dms_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup swi_dms_qmi_enums Enumerated types used in QMI messages */
/** @defgroup swi_dms_qmi_messages Structures sent as QMI messages */
/** @defgroup swi_dms_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup swi_dms_qmi_accessor Accessor for QMI service object */
/** @defgroup swi_dms_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup swi_dms_qmi_version 
    @{ 
  */ 
/** Major Version Number of the IDL used to generate this file */
#define SWI_DMS_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define SWI_DMS_V01_IDL_MINOR_VERS 0x00
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define SWI_DMS_V01_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
#define SWI_DMS_V01_MAX_MESSAGE_ID 0x0006;
/** 
    @} 
  */


/** @addtogroup swi_dms_qmi_consts 
    @{ 
  */
#define QMI_SERVICE_SWI_DMS_ID_V01 0xFE
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_dms_get_host_intf_comp_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup swi_dms_qmi_aggregates
    @{
  */
typedef struct {

  /*  Confugration type */
  uint32_t cfg_type;

  /*  Configuration bitmask value */
  uint32_t cfg_val;
}get_host_intf_comp_resp_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup swi_dms_qmi_messages
    @{
  */
/** Response Message; This message is used to query number of RmNet interface. */
typedef struct {

  /* Mandatory */
  /*  Response Data
 Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Mandatory */
  get_host_intf_comp_resp_type_v01 host_intf_comp_type;

  /* Optional */
  /*  return the USB configuration type and composition setting bit mask. */
  uint8_t valid_bitmasks_valid;  /**< Must be set to true if valid_bitmasks is being passed */
  uint32_t valid_bitmasks;
}swi_dms_get_host_intf_comp_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup swi_dms_qmi_messages
    @{
  */
/** Request Message; This message is used to set number of RmNet interface. */
typedef struct {

  /* Mandatory */
  /*  Set USB composition settings */
  uint32_t cfg_value;
}swi_dms_set_host_intf_comp_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup swi_dms_qmi_messages
    @{
  */
/** Response Message; This message is used to set number of RmNet interface. */
typedef struct {

  /* Mandatory */
  /*  Response Data
 Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}swi_dms_set_host_intf_comp_resp_msg_v01;  /* Message */
/**
    @}
  */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_dms_get_usb_net_num_req_msg_v01;

/** @addtogroup swi_dms_qmi_messages
    @{
  */
/** Response Message; This message is used to query number of RmNet interface. */
typedef struct {

  /* Mandatory */
  /*  usb_net_num */
  uint8_t usb_net_num;
  /**<  
    Number of network interfaces supported over USB (RmNet)
    Range: 0-255
  */

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}swi_dms_get_usb_net_num_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup swi_dms_qmi_messages
    @{
  */
/** Request Message; This message is used to set number of RmNet interface. */
typedef struct {

  /* Mandatory */
  /*  usb_net_num */
  uint8_t usb_net_num;
  /**<  
    Number of network interfaces supported over USB (RmNet)
    Range: 0-255
  */
}swi_dms_set_usb_net_num_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup swi_dms_qmi_messages
    @{
  */
/** Response Message; This message is used to set number of RmNet interface. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}swi_dms_set_usb_net_num_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_dms_get_mtu_req_msg_v01;

/** @addtogroup swi_dms_qmi_messages
    @{
  */
/** Response Message; This message requests device to return MTU size for the following interfaces: */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  mtu */
  uint8_t mtu_valid;  /**< Must be set to true if mtu is being passed */
  uint16_t mtu;
  /**<  
    Size of 3GPP MTU
  */

  /* Optional */
  /*  hrpd_mtu */
  uint8_t hrpd_mtu_valid;  /**< Must be set to true if hrpd_mtu is being passed */
  uint16_t hrpd_mtu;
  /**<  
    Size of HRPD MTU
  */

  /* Optional */
  /*  ehrpd_mtu */
  uint8_t ehrpd_mtu_valid;  /**< Must be set to true if ehrpd_mtu is being passed */
  uint16_t ehrpd_mtu;
  /**<  
    Size of EHRPD MTU
  */

  /* Optional */
  /*  usb_mtu */
  uint8_t usb_mtu_valid;  /**< Must be set to true if usb_mtu is being passed */
  uint16_t usb_mtu;
  /**<  
    Size of USB MTU
  */
}swi_dms_get_mtu_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup swi_dms_qmi_messages
    @{
  */
/** Request Message; This message allows to set MTU size for the following interfaces: */
typedef struct {

  /* Mandatory */
  /*  set_mtu_size */
  uint16_t set_mtu_size;
  /**<  
    MTU:
    0 : use default value
    576 - 2000: other values required by carrier.
    Set the same MTU for all RAT/interfaces.
  */
}swi_dms_set_mtu_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup swi_dms_qmi_messages
    @{
  */
/** Response Message; This message allows to set MTU size for the following interfaces: */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}swi_dms_set_mtu_resp_msg_v01;  /* Message */
/**
    @}
  */

/*Service Message Definition*/
/** @addtogroup swi_dms_qmi_msg_ids
    @{
  */
#define QMI_SWI_DMS_GET_HOST_INTF_COMP_REQ_V01 0x0001
#define QMI_SWI_DMS_GET_HOST_INTF_COMP_RESP_V01 0x0001
#define QMI_SWI_DMS_SET_HOST_INTF_COMP_REQ_V01 0x0002
#define QMI_SWI_DMS_SET_HOST_INTF_COMP_RESP_V01 0x0002
#define QMI_SWI_DMS_GET_USB_NET_NUM_REQ_V01 0x0003
#define QMI_SWI_DMS_GET_USB_NET_NUM_RESP_V01 0x0003
#define QMI_SWI_DMS_SET_USB_NET_NUM_REQ_V01 0x0004
#define QMI_SWI_DMS_SET_USB_NET_NUM_RESP_V01 0x0004
#define QMI_SWI_DMS_GET_MTU_REQ_V01 0x0005
#define QMI_SWI_DMS_GET_MTU_RESP_V01 0x0005
#define QMI_SWI_DMS_SET_MTU_REQ_V01 0x0006
#define QMI_SWI_DMS_SET_MTU_RESP_V01 0x0006
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor 
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro swi_dms_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type swi_dms_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );
 
/** This macro should be used to get the service object */ 
#define swi_dms_get_service_object_v01( ) \
          swi_dms_get_service_object_internal_v01( \
            SWI_DMS_V01_IDL_MAJOR_VERS, SWI_DMS_V01_IDL_MINOR_VERS, \
            SWI_DMS_V01_IDL_TOOL_VERS )
/** 
    @} 
  */


#ifdef __cplusplus
}
#endif
#endif

