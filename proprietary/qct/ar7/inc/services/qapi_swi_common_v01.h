#ifndef QAPI_SWI_COMMON_SERVICE_H
#define QAPI_SWI_COMMON_SERVICE_H
/**
  @file qapi_swi_common_v01.h
  
  @brief This is the public header file which defines the qapi_swi_common service Data structures.

  This header file defines the types and structures that were defined in 
  qapi_swi_common. It contains the constant values defined, enums, structures,
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
   It was generated on: Thu Apr  2 2015
   From IDL File: qapi_swi_common_v01.idl */

/** @defgroup qapi_swi_common_qmi_consts Constant values defined in the IDL */
/** @defgroup qapi_swi_common_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup qapi_swi_common_qmi_enums Enumerated types used in QMI messages */
/** @defgroup qapi_swi_common_qmi_messages Structures sent as QMI messages */
/** @defgroup qapi_swi_common_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup qapi_swi_common_qmi_accessor Accessor for QMI service object */
/** @defgroup qapi_swi_common_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup qapi_swi_common_qmi_version 
    @{ 
  */ 
/** Major Version Number of the IDL used to generate this file */
#define QAPI_SWI_COMMON_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define QAPI_SWI_COMMON_V01_IDL_MINOR_VERS 0x01
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define QAPI_SWI_COMMON_V01_IDL_TOOL_VERS 0x05

/** 
    @} 
  */

/** @addtogroup qapi_swi_common_qmi_aggregates
    @{
  */
typedef struct {

  /*  0=Success, 1=Failure */
  uint16_t result;

  /*  OEM defined error values */
  uint16_t error;
}oem_response_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qapi_swi_common_qmi_messages
    @{
  */
/** Request Message; This message tests basic message passing between 
  client and service */
typedef struct {

  /* Mandatory */
  /*  Ping */
  char ping[4];
  /**<   Simple 'ping' request  */
}swi_modem_ping_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_swi_common_qmi_messages
    @{
  */
/** Response Message; This message tests basic message passing between 
  client and service */
typedef struct {

  /* Mandatory */
  /*  Pong */
  char pong[4];
  /**<   Simple 'pong' response  */

  /* Mandatory */
  /*  Result Code */
  oem_response_type_v01 resp;
  /**<   Standard response type.  */
}swi_modem_ping_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_swi_common_qmi_messages
    @{
  */
/** Indication Message; This message tests basic message passing between 
  client and service */
typedef struct {

  /* Mandatory */
  /*  Hello indication */
  char indication[5];
  /**<   Simple 'hello' indication  */
}swi_modem_ping_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_swi_common_qmi_messages
    @{
  */
/** Request Message; This message tests basic message passing between 
  client and service */
typedef struct {

  /* Mandatory */
  /*  Ping */
  char ping[4];
  /**<   Simple 'ping' request  */
}swi_m2m_ping_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_swi_common_qmi_messages
    @{
  */
/** Response Message; This message tests basic message passing between 
  client and service */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  oem_response_type_v01 resp;
  /**<   Standard response type.  */

  /* Mandatory */
  /*  Pong */
  char pong[4];
  /**<   Simple 'pong' response    */
}swi_m2m_ping_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_swi_common_qmi_messages
    @{
  */
/** Indication Message; This message tests basic message passing between 
  client and service */
typedef struct {

  /* Mandatory */
  /*  Hello indication */
  char indication[5];
  /**<   Simple 'hello' indication  */
}swi_m2m_ping_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_swi_common_qmi_messages
    @{
  */
/** Request Message; This message tests basic message passing between 
  client and service */
typedef struct {

  /* Mandatory */
  /*  Ping */
  char ping[4];
  /**<   Simple 'ping' request  */
}qcsi_apps_ping_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_swi_common_qmi_messages
    @{
  */
/** Response Message; This message tests basic message passing between 
  client and service */
typedef struct {

  /* Mandatory */
  /*  Pong */
  char pong[4];
  /**<   Simple 'pong' response  */

  /* Mandatory */
  /*  Result Code */
  oem_response_type_v01 resp;
  /**<   Standard response type.  */
}qcsi_apps_ping_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_swi_common_qmi_messages
    @{
  */
/** Indication Message; This message tests basic message passing between 
  client and service */
typedef struct {

  /* Mandatory */
  /*  Hello indication */
  char indication[5];
  /**<   Simple 'hello' indication  */
}qcsi_apps_ping_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_swi_common_qmi_messages
    @{
  */
/** Request Message; This message tests basic message passing between 
  client and service */
typedef struct {

  /* Mandatory */
  /*  Ping */
  char ping[4];
  /**<   Simple 'ping' request  */
}lwm2m_ping_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_swi_common_qmi_messages
    @{
  */
/** Response Message; This message tests basic message passing between 
  client and service */
typedef struct {

  /* Mandatory */
  /*  Pong */
  char pong[4];
  /**<   Simple 'pong' response  */

  /* Mandatory */
  /*  Result Code */
  oem_response_type_v01 resp;
  /**<   Standard response type.  */
}lwm2m_ping_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_swi_common_qmi_messages
    @{
  */
/** Indication Message; This message tests basic message passing between 
  client and service */
typedef struct {

  /* Mandatory */
  /*  Hello indication */
  char indication[5];
  /**<   Simple 'hello' indication  */
}lwm2m_ping_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_swi_common_qmi_messages
    @{
  */
/** Request Message; This message tests basic message passing between 
  client and service */
typedef struct {

  /* Mandatory */
  /*  Ping */
  char ping[4];
  /**<   Simple 'ping' request  */
}swi_sfs_ping_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_swi_common_qmi_messages
    @{
  */
/** Response Message; This message tests basic message passing between 
  client and service */
typedef struct {

  /* Mandatory */
  /*  Pong */
  char pong[4];
  /**<   Simple 'pong' response  */

  /* Mandatory */
  /*  Result Code */
  oem_response_type_v01 resp;
  /**<   Standard response type.  */
}swi_sfs_ping_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_swi_common_qmi_messages
    @{
  */
/** Indication Message; This message tests basic message passing between 
  client and service */
typedef struct {

  /* Mandatory */
  /*  Hello indication */
  char indication[5];
  /**<   Simple 'hello' indication  */
}swi_sfs_ping_ind_msg_v01;  /* Message */
/**
    @}
  */

/*Extern Definition of Type Table Object*/
/*THIS IS AN INTERNAL OBJECT AND SHOULD ONLY*/
/*BE ACCESSED BY AUTOGENERATED FILES*/
extern const qmi_idl_type_table_object qapi_swi_common_qmi_idl_type_table_object_v01;


#ifdef __cplusplus
}
#endif
#endif

