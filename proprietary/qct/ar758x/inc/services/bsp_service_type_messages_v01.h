#ifndef SIERRA_SWI_BSP_API_SERVICE_H
#define SIERRA_SWI_BSP_API_SERVICE_H
/**
  @file bsp_service_type_messages_v01.h
  
  @brief This is the public header file which defines the sierra_swi_bsp_api service Data structures.

  This header file defines the types and structures that were defined in 
  sierra_swi_bsp_api. It contains the constant values defined, enums, structures,
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
   From IDL File: bsp_service_type_messages_v01.idl */

/** @defgroup sierra_swi_bsp_api_qmi_consts Constant values defined in the IDL */
/** @defgroup sierra_swi_bsp_api_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup sierra_swi_bsp_api_qmi_enums Enumerated types used in QMI messages */
/** @defgroup sierra_swi_bsp_api_qmi_messages Structures sent as QMI messages */
/** @defgroup sierra_swi_bsp_api_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup sierra_swi_bsp_api_qmi_accessor Accessor for QMI service object */
/** @defgroup sierra_swi_bsp_api_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup sierra_swi_bsp_api_qmi_version 
    @{ 
  */ 
/** Major Version Number of the IDL used to generate this file */
#define SIERRA_SWI_BSP_API_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define SIERRA_SWI_BSP_API_V01_IDL_MINOR_VERS 0x02
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define SIERRA_SWI_BSP_API_V01_IDL_TOOL_VERS 0x05
/** Maximum Defined Message ID */
#define SIERRA_SWI_BSP_API_V01_MAX_MESSAGE_ID 0x0004
/** 
    @} 
  */

/** @addtogroup sierra_swi_bsp_api_qmi_messages
    @{
  */
/** Request Message; Request to read the GPIO pin level. */
typedef struct {

  /* Mandatory */
  /*  gpio_pin_number */
  uint8_t gpio_pin_number;
  /**<  
    Possible range is from 1 to max of uint8 (actual number varied by product).
    At power-on, all GPIOs are configured for input.
   */
}swi_bsp_gpio_read_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_bsp_api_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t gpio_pin_number;
  /**<  
    GPIO pin number
   */

  uint8_t gpio_value;
  /**<  
    0 for active LOW
    1 for active HIGH
   */
}swi_bsp_gpio_pin_value_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup sierra_swi_bsp_api_qmi_messages
    @{
  */
/** Response Message; Request to read the GPIO pin level. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  gpio_pin_value */
  uint8_t gpio_pin_value_valid;  /**< Must be set to true if gpio_pin_value is being passed */
  swi_bsp_gpio_pin_value_type_v01 gpio_pin_value;
}swi_bsp_gpio_read_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_bsp_api_qmi_messages
    @{
  */
/** Request Message; This object is used to read the specified I/O port. */
typedef struct {

  /* Mandatory */
  /*  gpio_pin_value */
  swi_bsp_gpio_pin_value_type_v01 gpio_pin_value;
}swi_bsp_gpio_write_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_bsp_api_qmi_messages
    @{
  */
/** Response Message; This object is used to read the specified I/O port. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}swi_bsp_gpio_write_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_bsp_api_qmi_enums
    @{
  */
typedef enum {
  SWI_BSP_LEVEL_CHECKING_INTERVAL_ENUM_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  LEVEL_CHECKING_INTERVAL_50MS_V01 = 0, 
  LEVEL_CHECKING_INTERVAL_1000MS_V01 = 1, 
  SWI_BSP_LEVEL_CHECKING_INTERVAL_ENUM_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}swi_bsp_level_checking_interval_enum_type_v01;
/**
    @}
  */

/** @addtogroup sierra_swi_bsp_api_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t gpio_pin_number;
  /**<  
    GPIO pin number
   */

  uint8_t enable;
  /**<  
    0 for disable
    1 for enable
   */

  uint8_t direction;
  /**<  
    0 Input
    1 Output
   */

  uint8_t gpio_pull_type;
  /**<  
    0 - No pull
    1 - Pull down
    2 - Keeper
    3 - Pull up
   */

  uint8_t initval_or_notify;
  /**<  
    If GPIO direction type is Output
    : 0 - active low at power-up
    1 - active high at power-up
    If GPIO direction type is Input
    : 0 - disable level/edge change notification
    1 - level changed to active high notification
    2 - level changed to active low notification
    3 - edge rising notification
    4 - edge falling notification
   */

  swi_bsp_level_checking_interval_enum_type_v01 level_checking_interval;
  /**<  
    0 ¨C 50ms
    1 ¨C 1000ms
    For level change interrupt, the module will read the level again after level_checking_interval.
   */
}swi_bsp_gpio_configure_type_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup sierra_swi_bsp_api_qmi_messages
    @{
  */
/** Request Message; This object is used to read the specified I/O port. */
typedef struct {

  /* Mandatory */
  /*  gpio_configure_type */
  swi_bsp_gpio_configure_type_type_v01 gpio_configure_type;
}swi_bsp_gpio_cfg_trigger_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_bsp_api_qmi_messages
    @{
  */
/** Response Message; This object is used to read the specified I/O port. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}swi_bsp_gpio_cfg_trigger_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_bsp_api_qmi_messages
    @{
  */
/** Request Message; This object is used to read PM ADC1-4. */
typedef struct {

  /* Mandatory */
  /*  adc_input */
  uint8_t adc_input;
  /**<  
    0 VBATT
    2 PA_THERM
    3 PMIC_THERM
    4 XO_THERM
    5 ADC1
    6 ADC2
    11 ADC3
    12 ADC4
    new item will be just added at the end.
    */
}swi_bsp_adc_read_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_bsp_api_qmi_messages
    @{
  */
/** Response Message; This object is used to read PM ADC1-4. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  adc_value */
  uint8_t adc_value_valid;  /**< Must be set to true if adc_value is being passed */
  uint16_t adc_value;
  /**<  
    The ADC value measured for the requested ADC. Possible values are 0-32767.
   */
}swi_bsp_adc_read_resp_msg_v01;  /* Message */
/**
    @}
  */

/*Service Message Definition*/
/** @addtogroup sierra_swi_bsp_api_qmi_msg_ids
    @{
  */
#define QMI_SWI_BSP_GPIO_READ_REQ_V01 0x0001
#define QMI_SWI_BSP_GPIO_READ_RESP_V01 0x0001
#define QMI_SWI_BSP_GPIO_WRITE_REQ_V01 0x0002
#define QMI_SWI_BSP_GPIO_WRITE_RESP_V01 0x0002
#define QMI_SWI_BSP_GPIO_CFG_TRIGGER_REQ_V01 0x0003
#define QMI_SWI_BSP_GPIO_CFG_TRIGGER_RESP_V01 0x0003
#define QMI_SWI_BSP_ADC_READ_REQ_V01 0x0004
#define QMI_SWI_BSP_ADC_READ_RESP_V01 0x0004
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor 
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro sierra_swi_bsp_api_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type sierra_swi_bsp_api_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );
 
/** This macro should be used to get the service object */ 
#define sierra_swi_bsp_api_get_service_object_v01( ) \
          sierra_swi_bsp_api_get_service_object_internal_v01( \
            SIERRA_SWI_BSP_API_V01_IDL_MAJOR_VERS, SIERRA_SWI_BSP_API_V01_IDL_MINOR_VERS, \
            SIERRA_SWI_BSP_API_V01_IDL_TOOL_VERS )
/** 
    @} 
  */


#ifdef __cplusplus
}
#endif
#endif

