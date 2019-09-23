#ifndef SIERRA_M2M_APPS_GENERAL_API_SERVICE_H
#define SIERRA_M2M_APPS_GENERAL_API_SERVICE_H
/**
  @file qapi_m2m_apps_general_v01.h
  
  @brief This is the public header file which defines the sierra_m2m_apps_general_api service Data structures.

  This header file defines the types and structures that were defined in 
  sierra_m2m_apps_general_api. It contains the constant values defined, enums, structures,
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
   From IDL File: qapi_m2m_apps_general_v01.idl */

/** @defgroup sierra_m2m_apps_general_api_qmi_consts Constant values defined in the IDL */
/** @defgroup sierra_m2m_apps_general_api_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup sierra_m2m_apps_general_api_qmi_enums Enumerated types used in QMI messages */
/** @defgroup sierra_m2m_apps_general_api_qmi_messages Structures sent as QMI messages */
/** @defgroup sierra_m2m_apps_general_api_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup sierra_m2m_apps_general_api_qmi_accessor Accessor for QMI service object */
/** @defgroup sierra_m2m_apps_general_api_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "qapi_swi_common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup sierra_m2m_apps_general_api_qmi_version 
    @{ 
  */ 
/** Major Version Number of the IDL used to generate this file */
#define SIERRA_M2M_APPS_GENERAL_API_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define SIERRA_M2M_APPS_GENERAL_API_V01_IDL_MINOR_VERS 0x01
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define SIERRA_M2M_APPS_GENERAL_API_V01_IDL_TOOL_VERS 0x05
/** Maximum Defined Message ID */
#define SIERRA_M2M_APPS_GENERAL_API_V01_MAX_MESSAGE_ID 0x0001;
/** 
    @} 
  */


/** @addtogroup sierra_m2m_apps_general_api_qmi_consts 
    @{ 
  */
#define RAW_TUNNEL_MAX_SIZE_V01 32768
/**
    @}
  */

/** @addtogroup sierra_m2m_apps_general_api_qmi_messages
    @{
  */
/** Request Message; This message used to construct raw tunnel between Host side and Linux side. */
typedef struct {

  /* Optional */
  /*  Transfer byte mode */
  uint8_t raw_data_valid;  /**< Must be set to true if raw_data is being passed */
  uint32_t raw_data_len;  /**< Must be set to # of elements in raw_data */
  char raw_data[RAW_TUNNEL_MAX_SIZE_V01];
  /**<   Raw data in byte  */
}swi_m2m_raw_tunnel_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_apps_general_api_qmi_messages
    @{
  */
/** Response Message; This message used to construct raw tunnel between Host side and Linux side. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  oem_response_type_v01 resp;
  /**<   Standard response type.  */

  /* Optional */
  /*  Transfer byte mode */
  uint8_t raw_data_valid;  /**< Must be set to true if raw_data is being passed */
  uint32_t raw_data_len;  /**< Must be set to # of elements in raw_data */
  char raw_data[RAW_TUNNEL_MAX_SIZE_V01];
  /**<   Raw data in byte  */
}swi_m2m_raw_tunnel_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_apps_general_api_qmi_messages
    @{
  */
/** Indication Message; This message used to construct raw tunnel between Host side and Linux side. */
typedef struct {

  /* Optional */
  /*  Transfer byte mode */
  uint8_t raw_data_valid;  /**< Must be set to true if raw_data is being passed */
  uint32_t raw_data_len;  /**< Must be set to # of elements in raw_data */
  char raw_data[RAW_TUNNEL_MAX_SIZE_V01];
  /**<   Raw data in byte  */
}swi_m2m_raw_tunnel_ind_msg_v01;  /* Message */
/**
    @}
  */

/*Service Message Definition*/
/** @addtogroup sierra_m2m_apps_general_api_qmi_msg_ids
    @{
  */
#define QMI_SWI_M2M_PING_REQ_V01 0x0000
#define QMI_SWI_M2M_PING_RESP_V01 0x0000
#define QMI_SWI_M2M_PING_IND_V01 0x0000
#define QMI_SWI_M2M_RAW_TUNNEL_REQ_V01 0x0001
#define QMI_SWI_M2M_RAW_TUNNEL_RESP_V01 0x0001
#define QMI_SWI_M2M_RAW_TUNNEL_IND_V01 0x0001
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor 
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro sierra_m2m_apps_general_api_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type sierra_m2m_apps_general_api_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );
 
/** This macro should be used to get the service object */ 
#define sierra_m2m_apps_general_api_get_service_object_v01( ) \
          sierra_m2m_apps_general_api_get_service_object_internal_v01( \
            SIERRA_M2M_APPS_GENERAL_API_V01_IDL_MAJOR_VERS, SIERRA_M2M_APPS_GENERAL_API_V01_IDL_MINOR_VERS, \
            SIERRA_M2M_APPS_GENERAL_API_V01_IDL_TOOL_VERS )
/** 
    @} 
  */


#ifdef __cplusplus
}
#endif
#endif

