#ifndef SWILOC_SERVICE_H
#define SWILOC_SERVICE_H
/**
  @file qapi_loc_v01.h
  
  @brief This is the public header file which defines the swiloc service Data structures.

  This header file defines the types and structures that were defined in 
  swiloc. It contains the constant values defined, enums, structures,
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
   It was generated on: Thu Jun 15 2017
   From IDL File: qapi_loc_v01.idl */

/** @defgroup swiloc_qmi_consts Constant values defined in the IDL */
/** @defgroup swiloc_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup swiloc_qmi_enums Enumerated types used in QMI messages */
/** @defgroup swiloc_qmi_messages Structures sent as QMI messages */
/** @defgroup swiloc_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup swiloc_qmi_accessor Accessor for QMI service object */
/** @defgroup swiloc_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup swiloc_qmi_version 
    @{ 
  */ 
/** Major Version Number of the IDL used to generate this file */
#define SWILOC_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define SWILOC_V01_IDL_MINOR_VERS 0x02
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define SWILOC_V01_IDL_TOOL_VERS 0x05
/** Maximum Defined Message ID */
#define SWILOC_V01_MAX_MESSAGE_ID 0x0004
/** 
    @} 
  */

/*
 * swiloc_get_auto_start_settings_req_msg is empty
 * typedef struct {
 * }swiloc_get_auto_start_settings_req_msg_v01;
 */

/** @addtogroup swiloc_qmi_messages
    @{
  */
/** Response Message; This command queries the current GNSS Auto Start settings. The Auto Start feature controls how a GNSS session is started: Manually, by opening the NMEA port, or on power up. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  auto_start_function */
  uint8_t auto_start_function_valid;  /**< Must be set to true if auto_start_function is being passed */
  uint8_t auto_start_function;
  /**<  
    Setting to indicate when modem should start an automatic GNSS fix.
    0 每 Disabled
    1 每 At bootup
    2 每 When NMEA port is opened
   */

  /* Optional */
  /*  fix_type */
  uint8_t fix_type_valid;  /**< Must be set to true if fix_type is being passed */
  uint8_t fix_type;
  /**<  
    Type of GNSS fix:
    1 每 Default Engine mode
    2 每 MS-Based
    3 每 MS-Assisted
    4 每 Standalone
   */

  /* Optional */
  /*  max_time */
  uint8_t max_time_valid;  /**< Must be set to true if max_time is being passed */
  uint8_t max_time;
  /**<  
    Maximum time allowed for the receiver to get a fix in seconds. Valid range: 1 -255
   */

  /* Optional */
  /*  max_distance */
  uint8_t max_distance_valid;  /**< Must be set to true if max_distance is being passed */
  uint32_t max_distance;
  /**<  
    Maximum uncertainty of a fix measured by distance in meters. Valid range:1 - 4294967280
   */

  /* Optional */
  /*  fix_rate */
  uint8_t fix_rate_valid;  /**< Must be set to true if fix_rate is being passed */
  uint32_t fix_rate;
  /**<  
    Time between fixes in seconds. Valid range:1 - 65535
   */
}swiloc_get_auto_start_settings_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup swiloc_qmi_messages
    @{
  */
/** Request Message; This command sets the GNSS Auto Start settings. The Auto Start feature controls how a GNSS session is started: Manually, by opening the NMEA port, or on power up. */
typedef struct {

  /* Optional */
  /*  auto_start_function */
  uint8_t auto_start_function_valid;  /**< Must be set to true if auto_start_function is being passed */
  uint8_t auto_start_function;
  /**<  
    Setting to indicate when modem should start an automatic GNSS fix.
    0 每 Disabled
    1 每 At bootup
    2 每 When NMEA port is opened
   */

  /* Optional */
  /*  fix_type */
  uint8_t fix_type_valid;  /**< Must be set to true if fix_type is being passed */
  uint8_t fix_type;
  /**<  
    Type of GNSS fix:
    1 每 Default Engine mode
    2 每 MS-Based
    3 每 MS-Assisted
    4 每 Standalone
   */

  /* Optional */
  /*  max_time */
  uint8_t max_time_valid;  /**< Must be set to true if max_time is being passed */
  uint8_t max_time;
  /**<  
    Maximum time allowed for the receiver to get a fix in seconds. Valid range: 1 每 255
   */

  /* Optional */
  /*  max_distance */
  uint8_t max_distance_valid;  /**< Must be set to true if max_distance is being passed */
  uint32_t max_distance;
  /**<  
    Maximum uncertainty of a fix measured by distance in meters. Valid range:1 每 4294967280
   */

  /* Optional */
  /*  fix_rate */
  uint8_t fix_rate_valid;  /**< Must be set to true if fix_rate is being passed */
  uint32_t fix_rate;
  /**<  
    Time between fixes in seconds. Valid range:1 每 65535
   */
}swiloc_set_auto_start_settings_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup swiloc_qmi_messages
    @{
  */
/** Response Message; This command sets the GNSS Auto Start settings. The Auto Start feature controls how a GNSS session is started: Manually, by opening the NMEA port, or on power up. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}swiloc_set_auto_start_settings_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * swiloc_get_min_elevation_req_msg is empty
 * typedef struct {
 * }swiloc_get_min_elevation_req_msg_v01;
 */

/** @addtogroup swiloc_qmi_messages
    @{
  */
/** Response Message; This command queries the current GNSS min elevation settings. The min elevation setting will ignore low elevation GNSS satellite signal. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  min_elevation */
  uint8_t min_elevation_valid;  /**< Must be set to true if min_elevation is being passed */
  uint8_t min_elevation;
  /**<  
    Min elevation which satellite signal will be received.
    0 每 90 degree.
   */
}swiloc_get_min_elevation_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup swiloc_qmi_messages
    @{
  */
/** Request Message; This command setting the current GNSS min elevation, the satellite which elevation lower than the min elevation will be ignored. */
typedef struct {

  /* Mandatory */
  /*  min_elevation */
  uint8_t min_elevation;
  /**<  
    Min elevation which satellite signal will be received.
    0 每 90 degree.
   */
}swiloc_set_min_elevation_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup swiloc_qmi_messages
    @{
  */
/** Response Message; This command setting the current GNSS min elevation, the satellite which elevation lower than the min elevation will be ignored. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}swiloc_set_min_elevation_resp_msg_v01;  /* Message */
/**
    @}
  */

/*Service Message Definition*/
/** @addtogroup swiloc_qmi_msg_ids
    @{
  */
#define QMI_SWILOC_GET_AUTO_START_SETTINGS_REQ_V01 0x0001
#define QMI_SWILOC_GET_AUTO_START_SETTINGS_RESP_V01 0x0001
#define QMI_SWILOC_SET_AUTO_START_SETTINGS_REQ_V01 0x0002
#define QMI_SWILOC_SET_AUTO_START_SETTINGS_RESP_V01 0x0002
#define QMI_SWILOC_GET_MIN_ELEVATION_REQ_V01 0x0003
#define QMI_SWILOC_GET_MIN_ELEVATION_RESP_V01 0x0003
#define QMI_SWILOC_SET_MIN_ELEVATION_REQ_V01 0x0004
#define QMI_SWILOC_SET_MIN_ELEVATION_RESP_V01 0x0004
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor 
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro swiloc_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type swiloc_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );
 
/** This macro should be used to get the service object */ 
#define swiloc_get_service_object_v01( ) \
          swiloc_get_service_object_internal_v01( \
            SWILOC_V01_IDL_MAJOR_VERS, SWILOC_V01_IDL_MINOR_VERS, \
            SWILOC_V01_IDL_TOOL_VERS )
/** 
    @} 
  */


#ifdef __cplusplus
}
#endif
#endif

