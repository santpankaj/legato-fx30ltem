#ifndef QAPI_M2M_REALTIME_SERVICE_01_H
#define QAPI_M2M_REALTIME_SERVICE_01_H
/**
  @file qapi_m2m_realtime_v01.h

  @brief This is the public header file which defines the qapi_m2m_realtime service Data structures.

  This header file defines the types and structures that were defined in
  qapi_m2m_realtime. It contains the constant values defined, enums, structures,
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
   From IDL File: qapi_m2m_realtime_v01.idl */

/** @defgroup qapi_m2m_realtime_qmi_consts Constant values defined in the IDL */
/** @defgroup qapi_m2m_realtime_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup qapi_m2m_realtime_qmi_enums Enumerated types used in QMI messages */
/** @defgroup qapi_m2m_realtime_qmi_messages Structures sent as QMI messages */
/** @defgroup qapi_m2m_realtime_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup qapi_m2m_realtime_qmi_accessor Accessor for QMI service object */
/** @defgroup qapi_m2m_realtime_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "qapi_swi_common_v01.h"
#include "common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup qapi_m2m_realtime_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define QAPI_M2M_REALTIME_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define QAPI_M2M_REALTIME_V01_IDL_MINOR_VERS 0x01
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define QAPI_M2M_REALTIME_V01_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
#define QAPI_M2M_REALTIME_V01_MAX_MESSAGE_ID 0x000B
/**
    @}
  */


/** @addtogroup qapi_m2m_realtime_qmi_consts 
    @{ 
  */
#define PRIMARY_ANTENNA_V01 0x01
#define RX2_ANTENNA_V01 0x02
#define GNSS_ANTENNA_V01 0x04
/**
    @}
  */

/** @addtogroup qapi_m2m_realtime_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t antenna_selection;
  /**<   
                               Antenna Type
                                 PRIMARY_ANTENNA = 0x01                                 
                                 RX2_ANTENNA     = 0x02
                                 GNSS_ANTENNA    = 0x04
                           */

  uint16_t short_limit;
  /**<   
                           The ADC value used to detect a shorted condition. For the primary antenna, 
                           a short is reported if the ADC value is lower than the short limit. For 
                           the GNSS antenna, a short is reported if it is higher than the short limit. 
                           Possible values are 0-32767.
                        */

  uint16_t open_limit;
  /**<    
                           The ADC value used to detect an open condition. For the primary antenna, 
                           an open is reported if the ADC value is higher than the open limit. For 
                           the GNSS antenna, an open is reported if it is lower than the open limit. 
                           Possible values are 0-32767.
                        */
}antenna_select_configuration_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qapi_m2m_realtime_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t antenna;
  /**<   Antenna type*/

  uint8_t status;
  /**<   
                    Status of the antenna.
                    "-1" Inactive 
                    0 Shorted 
                    1 Normal 
                    2 Open 
                    3 Over Current 
                  */
}antenna_status_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qapi_m2m_realtime_qmi_messages
    @{
  */
/** Request Message; Request to configure antenna limit threshold setting */
typedef struct {

  /* Mandatory */
  antenna_select_configuration_type_v01 antenna_select_configure;
  /**<   Antenna selection and parameters to configure*/
}swi_m2m_antenna_cfg_limit_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_realtime_qmi_messages
    @{
  */
/** Response Message; Request to configure antenna limit threshold setting */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}swi_m2m_antenna_cfg_limit_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_realtime_qmi_messages
    @{
  */
/** Request Message; Request to read antenna limit threshold setting */
typedef struct {

  /* Mandatory */
  uint8_t antenna_selection;
  /**<   
                                          0x01 ?Primary antenna
                                          0x02 ?RX2 antenna 
                                          0x04 ?GNSS antenna*/
}swi_m2m_antenna_read_limit_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_realtime_qmi_messages
    @{
  */
/** Response Message; Request to read antenna limit threshold setting */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */

  /* Optional */
  uint8_t antenna_cfg_setting_valid;  /**< Must be set to true if antenna_cfg_setting is being passed */
  antenna_select_configuration_type_v01 antenna_cfg_setting;
  /**<   (same) setting for all antennas*/
}swi_m2m_antenna_read_limit_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_realtime_qmi_messages
    @{
  */
/** Request Message; Request to configure antenna polling period */
typedef struct {

  /* Mandatory */
  /*  Antenna selection mask. Antenna not selected will have its periodic checking disabled */
  uint8_t antenna_selection_mask;
  /**<   antenna selection*/

  /* Optional */
  /*  Poll interval for the group of antenna selected (same value for all selected antenna) */
  uint8_t interval_s_valid;  /**< Must be set to true if interval_s is being passed */
  uint32_t interval_s;
  /**<   1-86400: Interval in seconds between antenna diagnostic checks*/
}swi_m2m_antenna_cfg_poll_period_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_realtime_qmi_messages
    @{
  */
/** Response Message; Request to configure antenna polling period */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}swi_m2m_antenna_cfg_poll_period_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_m2m_antenna_read_poll_duration_req_msg_v01;

/** @addtogroup qapi_m2m_realtime_qmi_messages
    @{
  */
/** Response Message; Request to read antenna polling period */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  uint8_t antenna_group_valid;  /**< Must be set to true if antenna_group is being passed */
  uint8_t antenna_group;
  /**<   
                                                      0 ?No periodic checking (PC) on any antenna
                                                    0x1 ?Enable PC on primary antenna                                                    
                                                    0x2 ?Enable PC RX2 antenna
                                                    0x4 ?Enable PC GNSS antenna
                                                */

  /* Optional */
  /*  If the selected antenna group is not 0, then this optional tag is required.                                               */
  uint8_t interval_s_valid;  /**< Must be set to true if interval_s is being passed */
  uint32_t interval_s;
  /**<   
                                                    1-86400: Interval in seconds between antenna 
                                                    diagnostic checks
                                                */
}swi_m2m_antenna_read_poll_duration_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_realtime_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t antenna_selection;
  /**<   
                                0x01 ?Primary antenna
                                0x02 ?RX2 antenna
                                0x04 ?GNSS antenna */

  uint8_t adc_type;
  /**<    
                        0 ?Internal ADC or None
                        1 ?External ADC_0
                        2 ?External ADC_1
          */
}antenna_adc_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qapi_m2m_realtime_qmi_messages
    @{
  */
/** Request Message; Request to select ADC for antenna */
typedef struct {

  /* Mandatory */
  /*  ADC selection type */
  uint32_t antenna_adc_selection_len;  /**< Must be set to # of elements in antenna_adc_selection */
  antenna_adc_type_v01 antenna_adc_selection[2];
}swi_m2m_antenna_set_select_adc_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_realtime_qmi_messages
    @{
  */
/** Response Message; Request to select ADC for antenna */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}swi_m2m_antenna_set_select_adc_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_m2m_antenna_get_select_adc_req_msg_v01;

/** @addtogroup qapi_m2m_realtime_qmi_messages
    @{
  */
/** Response Message; Request to get select ADC for any antenna */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Antennas and theirs ADC */
  uint8_t antenna_adc_selection_valid;  /**< Must be set to true if antenna_adc_selection is being passed */
  uint32_t antenna_adc_selection_len;  /**< Must be set to # of elements in antenna_adc_selection */
  antenna_adc_type_v01 antenna_adc_selection[2];
}swi_m2m_antenna_get_select_adc_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_m2m_antenna_read_status_req_msg_v01;

/** @addtogroup qapi_m2m_realtime_qmi_messages
    @{
  */
/** Response Message; Request to select ADC for primary antenna */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Antennas and theirs status */
  uint8_t antenna_status_valid;  /**< Must be set to true if antenna_status is being passed */
  uint32_t antenna_status_len;  /**< Must be set to # of elements in antenna_status */
  antenna_status_type_v01 antenna_status[3];
}swi_m2m_antenna_read_status_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_realtime_qmi_messages
    @{
  */
/** Indication Message; Indication reporting antenna status */
typedef struct {

  /* Mandatory */
  /*  Antenna status indication */
  uint32_t antenna_status_len;  /**< Must be set to # of elements in antenna_status */
  antenna_status_type_v01 antenna_status[3];
}swi_m2m_antenna_status_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_realtime_qmi_messages
    @{
  */
/** Request Message; Request to turn on/off GNSS antenna power */
typedef struct {

  /* Mandatory */
  /*  GNSS power state */
  uint8_t gnss_power_state;
  /**<   
                                          0 ?Disable (turn off)
                                          1 ?Enable (turn on)
                                      */
}swi_m2m_antenna_cfg_gnss_power_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_realtime_qmi_messages
    @{
  */
/** Response Message; Request to turn on/off GNSS antenna power */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}swi_m2m_antenna_cfg_gnss_power_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_m2m_antenna_read_gnss_power_req_msg_v01;

/** @addtogroup qapi_m2m_realtime_qmi_messages
    @{
  */
/** Response Message; Request to read the GNSS power state */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  GNSS power state */
  uint8_t gnss_power_state_valid;  /**< Must be set to true if gnss_power_state is being passed */
  uint8_t gnss_power_state;
  /**<   
                                          0 ?Disable (turn off)
                                          1 ?Enable (turn on)
                                       */
}swi_m2m_antenna_read_gnss_power_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_realtime_qmi_messages
    @{
  */
/** Request Message; Request to read ADC */
typedef struct {

  /* Mandatory */
  /*  ADC input type */
  uint8_t adc_input;
  /**<  
                              0  VBATT
                              1  VCOIN
                              2  PA_THERM
                              3  PMIC_THERM
                              4  XO_THERM
                              5  EXT_ADC1
                              6  EXT_ADC2
                              7  PRI_ANT
                              8  SEC_ANT
                              9  GNSS_ANT
                              new item will be just added at the end
                                */
}swi_m2m_adc_read_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_realtime_qmi_messages
    @{
  */
/** Response Message; Request to read ADC */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  ADC value */
  uint8_t adc_value_valid;  /**< Must be set to true if adc_value is being passed */
  uint16_t adc_value;
  /**<   The ADC value measured for the requested ADC. Possible values are 0-32767.                              */
}swi_m2m_adc_read_resp_msg_v01;  /* Message */
/**
    @}
  */

/* Conditional compilation tags for message removal */ 
//#define REMOVE_QMI_SWI_M2M_ADC_READ_V01 
//#define REMOVE_QMI_SWI_M2M_ANTENNA_CFG_GNSS_POWER_V01 
//#define REMOVE_QMI_SWI_M2M_ANTENNA_CFG_LIMIT_V01 
//#define REMOVE_QMI_SWI_M2M_ANTENNA_CFG_POLL_PERIOD_V01 
//#define REMOVE_QMI_SWI_M2M_ANTENNA_GET_SELECT_ADC_V01 
//#define REMOVE_QMI_SWI_M2M_ANTENNA_READ_GNSS_POWER_V01 
//#define REMOVE_QMI_SWI_M2M_ANTENNA_READ_LIMIT_V01 
//#define REMOVE_QMI_SWI_M2M_ANTENNA_READ_POLL_DURATION_V01 
//#define REMOVE_QMI_SWI_M2M_ANTENNA_READ_STATUS_V01 
//#define REMOVE_QMI_SWI_M2M_ANTENNA_SET_SELECT_ADC_V01 
//#define REMOVE_QMI_SWI_M2M_ANTENNA_STATUS_IND_V01 
//#define REMOVE_QMI_SWI_M2M_PING_V01 

/*Service Message Definition*/
/** @addtogroup qapi_m2m_realtime_qmi_msg_ids
    @{
  */
#define QMI_SWI_M2M_PING_REQ_V01 0x0000
#define QMI_SWI_M2M_PING_RESP_V01 0x0000
#define QMI_SWI_M2M_PING_IND_V01 0x0000
#define QMI_SWI_M2M_ANTENNA_CFG_LIMIT_REQ_V01 0x0001
#define QMI_SWI_M2M_ANTENNA_CFG_LIMIT_RESP_V01 0x0001
#define QMI_SWI_M2M_ANTENNA_READ_LIMIT_REQ_V01 0x0002
#define QMI_SWI_M2M_ANTENNA_READ_LIMIT_RESP_V01 0x0002
#define QMI_SWI_M2M_ANTENNA_CFG_POLL_PERIOD_REQ_V01 0x0003
#define QMI_SWI_M2M_ANTENNA_CFG_POLL_PERIOD_RESP_V01 0x0003
#define QMI_SWI_M2M_ANTENNA_READ_POLL_DURATION_REQ_V01 0x0004
#define QMI_SWI_M2M_ANTENNA_READ_POLL_DURATION_RESP_V01 0x0004
#define QMI_SWI_M2M_ANTENNA_SET_SELECT_ADC_REQ_V01 0x0005
#define QMI_SWI_M2M_ANTENNA_SET_SELECT_ADC_RESP_V01 0x0005
#define QMI_SWI_M2M_ANTENNA_GET_SELECT_ADC_REQ_V01 0x0006
#define QMI_SWI_M2M_ANTENNA_GET_SELECT_ADC_RESP_V01 0x0006
#define QMI_SWI_M2M_ANTENNA_READ_STATUS_REQ_V01 0x0007
#define QMI_SWI_M2M_ANTENNA_READ_STATUS_RESP_V01 0x0007
#define QMI_SWI_M2M_ANTENNA_STATUS_IND_V01 0x0008
#define QMI_SWI_M2M_ANTENNA_CFG_GNSS_POWER_REQ_V01 0x0009
#define QMI_SWI_M2M_ANTENNA_CFG_GNSS_POWER_RESP_V01 0x0009
#define QMI_SWI_M2M_ANTENNA_READ_GNSS_POWER_REQ_V01 0x000A
#define QMI_SWI_M2M_ANTENNA_READ_GNSS_POWER_RESP_V01 0x000A
#define QMI_SWI_M2M_ADC_READ_REQ_V01 0x000B
#define QMI_SWI_M2M_ADC_READ_RESP_V01 0x000B
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor 
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro qapi_m2m_realtime_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type qapi_m2m_realtime_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );
 
/** This macro should be used to get the service object */ 
#define qapi_m2m_realtime_get_service_object_v01( ) \
          qapi_m2m_realtime_get_service_object_internal_v01( \
            QAPI_M2M_REALTIME_V01_IDL_MAJOR_VERS, QAPI_M2M_REALTIME_V01_IDL_MINOR_VERS, \
            QAPI_M2M_REALTIME_V01_IDL_TOOL_VERS )
/** 
    @} 
  */


#ifdef __cplusplus
}
#endif
#endif

