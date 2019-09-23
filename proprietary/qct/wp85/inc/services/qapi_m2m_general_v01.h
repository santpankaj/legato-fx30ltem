#ifndef QAPI_M2M_GENERAL_SERVICE_01_H
#define QAPI_M2M_GENERAL_SERVICE_01_H
/**
  @file qapi_m2m_general_v01.h

  @brief This is the public header file which defines the qapi_m2m_general service Data structures.

  This header file defines the types and structures that were defined in
  qapi_m2m_general. It contains the constant values defined, enums, structures,
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
   It was generated on: Mon Mar 27 2017 (Spin 0)
   From IDL File: qapi_m2m_general_v01.idl */

/** @defgroup qapi_m2m_general_qmi_consts Constant values defined in the IDL */
/** @defgroup qapi_m2m_general_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup qapi_m2m_general_qmi_enums Enumerated types used in QMI messages */
/** @defgroup qapi_m2m_general_qmi_messages Structures sent as QMI messages */
/** @defgroup qapi_m2m_general_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup qapi_m2m_general_qmi_accessor Accessor for QMI service object */
/** @defgroup qapi_m2m_general_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "qapi_swi_common_v01.h"
#include "common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup qapi_m2m_general_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define QAPI_M2M_GENERAL_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define QAPI_M2M_GENERAL_V01_IDL_MINOR_VERS 0x01
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define QAPI_M2M_GENERAL_V01_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
#define QAPI_M2M_GENERAL_V01_MAX_MESSAGE_ID 0x00C3
/**
    @}
  */


/** @addtogroup qapi_m2m_general_qmi_consts 
    @{ 
  */
#define PRIMARY_ANTENNA_V01 0x01
#define TOTAL_BAND_MAX_SIZE_V01 60
#define ANTENNA_GPIO_LOW_V01 0x00
#define ANTENNA_GPIO_HIGH_V01 0x01
#define ANTENNA_GPIO_NOT_IN_USED_V01 0x02
#define NV_SWI_LD_LEDNUM_MAX_V01 9
#define NV_SWI_LD_EVTNAME_MAXLEN_V01 64
#define NV_SWI_LD_EVTNUM_MAX_V01 32
#define NV_SWI_LD_PAT_NUM_V01 20
#define QAPI_LD_BUFF_MAXLEN_V01 128
#define MAX_DIAL_DIGITS_V01 32
#define MSD_BLOB_MAX_V01 140
#define MSD_DATA_MAX_V01 103
#define MAX_HEX_DATA_LEN_V01 32768
#define SOURCE_MAX_LEN_V01 0x80
#define PACKAGE_MAX_LEN_V01 0x80
#define PACKAGE_DESCRIPTION_MAX_LEN_V01 0x400
#define TIME_MAX_LEN_V01 0x80
#define DATE_MAX_LEN_V01 0x80
#define VERSION_MAX_LEN_V01 0x80
#define ALER_MAX_LEN_V01 0xC8
#define APN_MAX_LEN_V01 49
#define UNAME_MAX_LEN_V01 29
#define PWD_MAX_LEN_V01 29
#define NONCE_LEN_V01 0x10
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t gpio_pin_number;
  /**<   GPIO pin number*/

  uint8_t gpio_value;
  /**<   
                          0 for active LOW
                          1 for active HIGH
                        */
}gpio_pin_value_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Request Message; Request to read the GPIO pin level */
typedef struct {

  /* Mandatory */
  /*  GPIO pin */
  uint8_t gpio_pin_number;
  /**<   
                                          Possible range is from 1 to max of uint8 (actual number varied by product).
                                          At power-on, all GPIOs are configured for input.
                                        */
}swi_m2m_gpio_read_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; Request to read the GPIO pin level */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */

  /* Optional */
  uint8_t gpio_pin_value_valid;  /**< Must be set to true if gpio_pin_value is being passed */
  gpio_pin_value_type_v01 gpio_pin_value;
}swi_m2m_gpio_read_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Request Message; Request to set the specified I/O port */
typedef struct {

  /* Mandatory */
  /*  GPIO pin and value. */
  gpio_pin_value_type_v01 gpio_pin_value;
}swi_m2m_gpio_write_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; Request to set the specified I/O port */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}swi_m2m_gpio_write_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_enums
    @{
  */
typedef enum {
  GPIO_LEVEL_CHECKING_INTERVAL_ENUM_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  GPIO_LEVEL_CHECK_50_MS_V01 = 0, 
  GPIO_LEVEL_CHECK_1000_MS_V01 = 1, 
  GPIO_LEVEL_CHECKING_INTERVAL_ENUM_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}gpio_level_checking_interval_enum_type_v01;
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_aggregates
    @{
  */
typedef struct {

  /*  GPIO pin number */
  uint8_t gpio_pin_number;
  /**<   Possible range is from 1 to max of uint8 (actual number varied by product).*/

  /*  GPIO enable */
  uint8_t enable;
  /**<    
                      0 for disable
                      1 for enable
                    */

  /*  GPIO direction type */
  uint8_t direction;
  /**<    
                      0  Input
                      1  Output 
                    */

  /*  GPIO Pull type */
  uint8_t gpio_pull_type;
  /**<    
                           0 - No pull
                           1 - Pull down
                           2 - Keeper
                           3 - Pull up
                        */

  /*  Initial pin level for OUT direction OR Interrupt type for IN direction */
  uint8_t initval_or_notify;
  /**<    
                           If GPIO direction type is Output:
                           0 - active low at power-up
                           1 - active high at power-up
                           If GPIO direction type is Input:
                           0 - disable level/edge change notification
                           1 - level changed to active high notification
                           2 - level changed to active low notification
                           3 - edge rising notification
                           4 - edge falling notification
                           */

  /*  Level change interval */
  gpio_level_checking_interval_enum_type_v01 level_checking_interval;
  /**<    
                      For level change interrupt, the module will read the level again after level_checking_interval
                      0 for 50 ms
                      1 for 1000 ms
                    */
}gpio_configure_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Request Message; Request to configure GPIO direction and edge/level change notification for input GPIO */
typedef struct {

  /* Mandatory */
  /*  GPIO configure */
  gpio_configure_type_v01 gpio_configure;
}swi_m2m_gpio_cfg_trigger_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; Request to configure GPIO direction and edge/level change notification for input GPIO */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}swi_m2m_gpio_cfg_trigger_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Request Message; Notification of GPIO changed based on trigger configuration */
typedef struct {

  /* Mandatory */
  gpio_pin_value_type_v01 gpio_pin_value;
  /**<   GPIO pin and value*/
}swi_m2m_gpio_change_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Request Message; This command selected the owner of RI PIN */
typedef struct {

  /* Mandatory */
  /*  ri_owner */
  uint8_t ri_owner;
  /**<  
    Set the owner of RI pin:
    0 ¨C The RI owned by Modem core
    1 ¨C The RI owned by application core
  */
}swi_m2m_set_ri_owner_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; This command selected the owner of RI PIN */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}swi_m2m_set_ri_owner_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_m2m_get_ri_owner_req_msg_v01;

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; This command queried the owner of RI PIN. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  ri_owner */
  uint8_t ri_owner;
  /**<  
    Return the owner of RI pin:
    0 ¨C The RI owned by Modem core
    1 ¨C The RI owned by application core
  */
}swi_m2m_get_ri_owner_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t ldno;
  /**<        
                      led index number¡­,start from 0
                      */

  uint16_t ctrlmethod;
  /**<  
                        led control,drive type 
                        */

  uint16_t ctrldata;
  /**<  
                        led control pin number
                        */
}nv_swi_ld_driver_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t eventid;
  /**<  
                      Event ID
                      */

  char eventname[NV_SWI_LD_EVTNAME_MAXLEN_V01];
  /**<  
                                             Event name
                                             */
}nv_swi_ld_event_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t priperiod;
  /**<  
                      the primary cycle period
                      */

  uint16_t priduty;
  /**<  
                      the primary cycle duty
                      */

  uint16_t pricycle;
  /**<  
                      the number of  primary cycle
                      */

  uint16_t secperiod;
  /**<  
                      the secondary cycle period
                      */

  uint16_t secduty;
  /**<  
                      the secondary cycle duty
                      */

  uint16_t seccycle;
  /**<  
                      the number of  secondary cycle
                      */

  uint16_t invert;
  /**<  
                      whether invert the led on/off
                      */
}ldledhw_t_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t patternno;
  /**<  
                       pattern number 
                       */

  ldledhw_t_v01 pattern;
  /**<  
                       pattern structure 
                       */
}nv_swi_ld_pattern_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t eventid;
  /**<  
                       event index
                       */

  uint16_t ldpattern[NV_SWI_LD_LEDNUM_MAX_V01];
  /**<  
                                           LEDx(ldno=x start from 0) pattern type
                                           */
}nv_swi_ld_displaystate_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Request Message; Request to switch LED */
typedef struct {

  /* Optional */
  /*  Led drive type definition/cfg */
  uint8_t nv_swi_ld_driver_valid;  /**< Must be set to true if nv_swi_ld_driver is being passed */
  uint32_t nv_swi_ld_driver_len;  /**< Must be set to # of elements in nv_swi_ld_driver */
  nv_swi_ld_driver_type_v01 nv_swi_ld_driver[9];

  /* Optional */
  uint8_t nv_swi_ld_event_valid;  /**< Must be set to true if nv_swi_ld_event is being passed */
  uint32_t nv_swi_ld_event_len;  /**< Must be set to # of elements in nv_swi_ld_event */
  nv_swi_ld_event_type_v01 nv_swi_ld_event[32];

  /* Optional */
  uint8_t nv_swi_ld_pattern_valid;  /**< Must be set to true if nv_swi_ld_pattern is being passed */
  uint32_t nv_swi_ld_pattern_len;  /**< Must be set to # of elements in nv_swi_ld_pattern */
  nv_swi_ld_pattern_type_v01 nv_swi_ld_pattern[20];

  /* Optional */
  uint8_t nv_swi_ld_displaystate_valid;  /**< Must be set to true if nv_swi_ld_displaystate is being passed */
  uint32_t nv_swi_ld_displaystate_len;  /**< Must be set to # of elements in nv_swi_ld_displaystate */
  nv_swi_ld_displaystate_type_v01 nv_swi_ld_displaystate[32];
}swi_m2m_led_cfg_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; Request to switch LED */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}swi_m2m_led_cfg_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Request Message; Request to read LED */
typedef struct {

  /* Mandatory */
  uint8_t cfg_type;
  /**<  
                            Query cfg type:
                            1(LED_CFG_TYPE_DRV): set the led control method, drive mode.
                            2(LED_CFG _TYPE_EVENT): define a event
                            3(LED_CFG_ TYPE_PATTYPE): define a blink pattern
                            4(LED_CFG_ TYPE_STATE):cfg a event corresponding LEDs blink pattern
                            5(LED_CFG_TYPE_ALL):all cfg about 1-4.
                            */

  /* Mandatory */
  uint8_t index;
  /**<  
                            For LED_CFG_TYPE_DRV, 
                            it mean ledno;for LED_CFG _TYPE_EVENT and LED_CFG_TYPE_STATE, it mean eventId,
                            for LED_CFG_ TYPE_PATTYPE, it mean Patternno. 
                            index =0xFF mean query all this type cfg.
                                                 */
}swi_m2m_led_read_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; Request to read LED */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  ld driver type */
  uint8_t nv_swi_ld_driver_valid;  /**< Must be set to true if nv_swi_ld_driver is being passed */
  uint32_t nv_swi_ld_driver_len;  /**< Must be set to # of elements in nv_swi_ld_driver */
  nv_swi_ld_driver_type_v01 nv_swi_ld_driver[9];

  /* Optional */
  uint8_t nv_swi_ld_event_valid;  /**< Must be set to true if nv_swi_ld_event is being passed */
  uint32_t nv_swi_ld_event_len;  /**< Must be set to # of elements in nv_swi_ld_event */
  nv_swi_ld_event_type_v01 nv_swi_ld_event[32];

  /* Optional */
  uint8_t nv_swi_ld_pattern_valid;  /**< Must be set to true if nv_swi_ld_pattern is being passed */
  uint32_t nv_swi_ld_pattern_len;  /**< Must be set to # of elements in nv_swi_ld_pattern */
  nv_swi_ld_pattern_type_v01 nv_swi_ld_pattern[20];

  /* Optional */
  uint8_t nv_swi_ld_displaystate_valid;  /**< Must be set to true if nv_swi_ld_displaystate is being passed */
  uint32_t nv_swi_ld_displaystate_len;  /**< Must be set to # of elements in nv_swi_ld_displaystate */
  nv_swi_ld_displaystate_type_v01 nv_swi_ld_displaystate[32];
}swi_m2m_led_read_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_aggregates
    @{
  */
typedef struct {

  /*  Voltage type */
  uint8_t voltage_type;
  /**<  
                          0 ?3.0V 
                          1 ?3.1V
                          2 (Default) ?3.2V
                        */

  /*  Resistance type */
  uint8_t resistance_type;
  /**<  
                            0 (Default) ?2100 ohm
                            1 ?1700 ohm
                            2 ?1200 ohm
                            3 ?800 ohm
                          */
}voltage_resistance_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Request Message; Request to configure cell charging feature */
typedef struct {

  /* Mandatory */
  voltage_resistance_type_v01 voltage_resistance;
  /**<   Voltage and resistance type*/
}swi_m2m_coin_cell_cfg_setting_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; Request to configure cell charging feature */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}swi_m2m_coin_cell_cfg_setting_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_m2m_coin_cell_cfg_read_req_msg_v01;

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; Request to read cell charging feature setting */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  uint8_t voltage_resistance_valid;  /**< Must be set to true if voltage_resistance is being passed */
  voltage_resistance_type_v01 voltage_resistance;
  /**<   Voltage and resistance type*/
}swi_m2m_coin_cell_cfg_read_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Request Message; Request to enable/disable cell charging feature */
typedef struct {

  /* Mandatory */
  uint8_t enable;
  /**<   Enable/Disable coin cell charging feature*/
}swi_m2m_coin_cell_ctrl_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; Request to enable/disable cell charging feature */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}swi_m2m_coin_cell_ctrl_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_m2m_coin_cell_check_ctrl_req_msg_v01;

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; Request to query if the coin cell charging feature is enabled or not */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  uint8_t enable_valid;  /**< Must be set to true if enable is being passed */
  uint8_t enable;
  /**<   0 - Disable, 1 - Enable*/
}swi_m2m_coin_cell_check_ctrl_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Indication Message; Indication reporting on jamming */
typedef struct {

  /* Mandatory */
  uint32_t jamming_response_type;
  /**<  
                                  Response Type:
                                  0: Final
                                  1: Intermediate
                                  2: Unknown
                                */

  /* Mandatory */
  uint32_t jamming_status;
  /**<  
                                  Jamming Status:
                                  0: Unknown
                                  1: Null
                                  2: Low
                                  3: Medium
                                  4: High
                                  5: Jammed
                                */
}swi_m2m_jamming_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Request Message; This message is used to register for the QMI_SWI_M2M_JAMMING_IND */
typedef struct {

  /* Mandatory */
  uint8_t enable;
  /**<   
                          0: Disable event reporting
                          1: Enable event reporting
                          */
}swi_m2m_jamming_set_event_report_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; This message is used to register for the QMI_SWI_M2M_JAMMING_IND */
typedef struct {

  /* Mandatory */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/
}swi_m2m_jamming_set_event_report_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t gpio_num;
  /**<        
                        A GPIO pin number
                       */

  uint8_t trigger;
  /**<  
                         State at which eCall is triggered {0 for active low or 1 for active high}
                        */
}ecall_config_gpio_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Request Message; Request to assign an external GPIO to be used to initiate an eCall */
typedef struct {

  /* Mandatory */
  uint8_t mode;
  /**<    
                                   0 for disable
                                   1 for enable
                                    */

  /* Optional */
  /*  ecall gpio type definition/cfg */
  uint8_t ecall_config_gpio_valid;  /**< Must be set to true if ecall_config_gpio is being passed */
  ecall_config_gpio_type_v01 ecall_config_gpio;
}swi_m2m_ecall_config_gpio_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; Request to assign an external GPIO to be used to initiate an eCall */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}swi_m2m_ecall_config_gpio_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_m2m_ecall_read_gpio_req_msg_v01;

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; Request to read the current external GPIO pin being assigned to initiate an eCall and its configured state.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Mandatory */
  uint8_t mode;
  /**<    
                                   0 for disable
                                   1 for enable
                                    */

  /* Optional */
  uint8_t ecall_config_gpio_valid;  /**< Must be set to true if ecall_config_gpio is being passed */
  ecall_config_gpio_type_v01 ecall_config_gpio;
}swi_m2m_ecall_read_gpio_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t voc_mode;
  /**<        
                        0: Deregister the (speaker) Rx input of the vocoder
                        1: Do not deregister Rx input of the vocoder
                       */

  uint8_t host_build_msd;
  /**<  
                                0: This instructs the modem to build the MSD blob without involving the Host.
                                1: The Host is entirely responsible to provide the MSD blob.
                               */

  uint8_t dial_type;
  /**<  
                           0: Read the number to dial from the FDN/SDN, depending upon the eCall operating mode.
                           1: The eCall modem will dial the number specified in the optional num field.     
                                                                        */
}configure_eCall_session_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Request Message; Request to setup necessary eCall parameters. */
typedef struct {

  /* Mandatory */
  configure_eCall_session_type_v01 configure_eCall_session;

  /* Mandatory */
  uint8_t modem_msd_type;
  /**<  
                                            0: Send Real MSD
                                            1: Send canned MSD     
                                           */

  /* Optional */
  uint8_t num_valid;  /**< Must be set to true if num is being passed */
  uint32_t num_len;  /**< Must be set to # of elements in num */
  uint8_t num[MAX_DIAL_DIGITS_V01];
  /**<  
                                            Number to dial.     
                                           */

  /* Optional */
  uint8_t max_redial_attempt_valid;  /**< Must be set to true if max_redial_attempt is being passed */
  uint8_t max_redial_attempt;
  /**<  
                                            The max redail attempt.     
                                           */

  /* Optional */
  uint8_t gnss_update_time_valid;  /**< Must be set to true if gnss_update_time is being passed */
  uint8_t gnss_update_time;
  /**<  
                                            GNSS update timer.     
                                           */

  /* Optional */
  uint8_t nad_deregistration_time_valid;  /**< Must be set to true if nad_deregistration_time is being passed */
  uint8_t nad_deregistration_time;
  /**<  
                                            NAD deregistration timer.
                                           */

  /* Optional */
  uint8_t ecall_usim_slot_id_valid;  /**< Must be set to true if ecall_usim_slot_id is being passed */
  uint8_t ecall_usim_slot_id;
  /**<  
                                            The sim slot eCall happens on .     
                                           */
}swi_m2m_ecall_config_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; Request to setup necessary eCall parameters. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}swi_m2m_ecall_config_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_m2m_ecall_read_config_req_msg_v01;

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; Request to query all the eCall parameters that are set into the modem */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/

  /* Mandatory */
  configure_eCall_session_type_v01 configure_eCall_session;

  /* Mandatory */
  uint8_t modem_msd_type;
  /**<  
                                            0: Send Real MSD
                                            1: Send canned MSD     
                                             */

  /* Optional */
  uint8_t num_valid;  /**< Must be set to true if num is being passed */
  uint32_t num_len;  /**< Must be set to # of elements in num */
  uint8_t num[MAX_DIAL_DIGITS_V01];
  /**<  
                                            Number to dial.     
                                           */

  /* Optional */
  uint8_t max_redial_attempt_valid;  /**< Must be set to true if max_redial_attempt is being passed */
  uint8_t max_redial_attempt;
  /**<  
                                            The max redail attempt.     
                                           */

  /* Optional */
  uint8_t gnss_update_time_valid;  /**< Must be set to true if gnss_update_time is being passed */
  uint8_t gnss_update_time;
  /**<  
                                            GNSS update timer.     
                                           */

  /* Optional */
  uint8_t nad_deregistration_time_valid;  /**< Must be set to true if nad_deregistration_time is being passed */
  uint8_t nad_deregistration_time;
  /**<  
                                            NAD deregistration timer.
                                           */

  /* Optional */
  uint8_t ecall_usim_slot_id_valid;  /**< Must be set to true if ecall_usim_slot_id is being passed */
  uint8_t ecall_usim_slot_id;
  /**<  
                                            The sim slot eCall happens on.     
                                           */
}swi_m2m_ecall_read_config_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Request Message; Request to set the eCall tx mode. */
typedef struct {

  /* Mandatory */
  uint8_t tx_mode;
  /**<  
                                   0: Pull mode (modem/host waits for MSD request from PSAP to send MSD)
                                   1: Push mode (modem/host sends MSD to PSAP right after eCall is connected). This is the default mode.
                                  */
}swi_m2m_ecall_set_tx_mode_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; Request to set the eCall tx mode. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/
}swi_m2m_ecall_set_tx_mode_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_m2m_ecall_get_tx_mode_req_msg_v01;

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; Request to get the eCall tx mode */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/

  /* Mandatory */
  uint8_t tx_mode;
  /**<  
                                   0: Pull mode (modem/host waits for MSD request from PSAP to send MSD)
                                   1: Push mode (modem/host sends MSD to PSAP right after eCall is connected). This is the default mode.
                                  */
}swi_m2m_ecall_get_tx_mode_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Request Message; Request to set the eCall op mode */
typedef struct {

  /* Mandatory */
  uint8_t op_mode;
  /**<  
                                   0: eCall and normal call mode.
                                   1: eCall only mode.
                                  */
}swi_m2m_ecall_set_op_mode_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; Request to set the eCall op mode */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/
}swi_m2m_ecall_set_op_mode_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_m2m_ecall_get_op_mode_req_msg_v01;

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; Request to get the eCall op mode */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/

  /* Mandatory */
  uint8_t op_mode;
  /**<  
                                   0: eCall and normal call mode.
                                   1: eCall only mode.
                                  */
}swi_m2m_ecall_get_op_mode_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Request Message; Request to send an entire encoded MSD blob */
typedef struct {

  /* Mandatory */
  uint32_t msd_blob_len;  /**< Must be set to # of elements in msd_blob */
  uint8_t msd_blob[MSD_BLOB_MAX_V01];
  /**<  
                                               Encoded MSD blob (140).
                                              */
}swi_m2m_ecall_send_msd_blob_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; Request to send an entire encoded MSD blob */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/
}swi_m2m_ecall_send_msd_blob_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t block_num;
  /**<        
                        The block number
                        */

  uint32_t msd_data_len;  /**< Must be set to # of elements in msd_data */
  uint8_t msd_data[MSD_DATA_MAX_V01];
  /**<  
                                        MSD block following the format specified in EN 15722 (103 bytes are remaining for additional data).     
                                                           */
}msd_block_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Request Message; Request to update 1 of the 12 MSD field  */
typedef struct {

  /* Mandatory */
  msd_block_type_v01 msd_block;
  /**<  
                                                  MSD block
                                                 */
}swi_m2m_ecall_update_msd_block_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; Request to update 1 of the 12 MSD field  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/
}swi_m2m_ecall_update_msd_block_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Request Message; Request to allows starting an eCall */
typedef struct {

  /* Mandatory */
  uint8_t call_type;
  /**<  
                                                  0: test call
                                                  1: reconfiguration call
                                                  2: manually initiated voice call
                                                  3: automatically initiated voice call
                                                 */
}swi_m2m_ecall_start_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; Request to allows starting an eCall */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/
}swi_m2m_ecall_start_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_m2m_ecall_stop_req_msg_v01;

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; Request to allows stopping an eCall. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/
}swi_m2m_ecall_stop_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Request Message; Notify the important eCall events */
typedef struct {

  /* Mandatory */
  uint32_t indication;
  /**<  
                                Event value as shown in table below:
                                0: eCall session started
                                1: Get GPS Fix
                                2: GPS Fix Received
                                3: GPS Fix Timeout
                                4: MO call connected
                                5: MO call Disconnected
                                6: MT call connected
                                7: MT call Disconnected
                                8: Waiting for PSAP START indication
                                9: PSAP START received but no MSD available
                                10: PSAP START received and MSD available
                                11: PSAP START received and MSD sent
                                12: LL ack received
                                13: 2LL acks received
                                14: LL nack received
                                15: HL ack received
                                16: IVS Transmission completed
                                17: 2AL acks received
                                18: eCall session completed
                                19: eCall clear-down received
                                20: eCall session reset
                                21: eCall session failure
                                22: MSD update request available
                                23: eCall session stop
                                24: eCall operating mode is eCall and normal call mode
                                25: eCall operating mode is eCall only mode
                                26: eCall transmission mode is PUSH mode
                                27: eCall transmission mode is PULL mode
                                28: eCall timer timeout reached
                                */

  /* Optional */
  uint8_t timer_id_valid;  /**< Must be set to true if timer_id is being passed */
  uint8_t timer_id;
}swi_m2m_ecall_status_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Request Message; Request to get the eCall msd block context according to block number */
typedef struct {

  /* Mandatory */
  uint8_t block_num;
  /**<  
                                   the block number
                                    */
}swi_m2m_ecall_read_msd_block_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; Request to get the eCall msd block context according to block number */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/

  /* Mandatory */
  msd_block_type_v01 msd_block;
  /**<  
                                            MSD block
                                           */
}swi_m2m_ecall_read_msd_block_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Request Message; an exclusive access to the SWI FOTA partition. */
typedef struct {

  /* Optional */
  /*  Write or read access flag. Absent means write access to support legacy product. */
  uint8_t fdt_access_flag_valid;  /**< Must be set to true if fdt_access_flag is being passed */
  uint8_t fdt_access_flag;
}swi_m2m_fdt_open_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; an exclusive access to the SWI FOTA partition. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/

  /* Mandatory */
  /*  SWI FOTA open context handle */
  uint32_t fdt_context_handle;
  /**<  
                                          Open context handle for write and close operation
                                          */

  /* Mandatory */
  uint32_t fdt_pkg_size;
  /**<  read operation :return bindary size, write operation :return fota partition size*/
}swi_m2m_fdt_open_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_aggregates
    @{
  */
typedef struct {

  /*  Open context handle for write operation */
  uint32_t fdt_context_handle;

  /*  Data payload in hex (max is 32K bytes) */
  uint32_t binary_data_len;  /**< Must be set to # of elements in binary_data */
  uint8_t binary_data[MAX_HEX_DATA_LEN_V01];
}write_context_handle_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Request Message; allows the QMI client to write a contiguous block of memory to the SWI FOTA partition.. */
typedef struct {

  /* Mandatory */
  write_context_handle_type_v01 write_context_handle;
}swi_m2m_fdt_write_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; allows the QMI client to write a contiguous block of memory to the SWI FOTA partition.. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/
}swi_m2m_fdt_write_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_aggregates
    @{
  */
typedef struct {

  /*  Data payload in hex (max is 32K bytes) */
  uint32_t binary_data_len;  /**< Must be set to # of elements in binary_data */
  uint8_t binary_data[MAX_HEX_DATA_LEN_V01];
}read_context_handle_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Request Message; allows the QMI client to read a contiguous block of memory from the SWI FOTA partition. */
typedef struct {

  /* Mandatory */
  uint32_t fdt_context_handle;
}swi_m2m_fdt_read_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; allows the QMI client to read a contiguous block of memory from the SWI FOTA partition. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/

  /* Mandatory */
  /*  SWI FOTA read context handle */
  read_context_handle_type_v01 read_context_handle;
}swi_m2m_fdt_read_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_aggregates
    @{
  */
typedef struct {

  /*  Open context handle for write operation */
  uint32_t fdt_context_handle;

  /*  Reason of closing 0: normal,Other value: writing session is aborted.  */
  uint32_t reason;
}close_context_handle_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Request Message; release access to SWI FOTA partition due to complete successful writing session or due to premature abort. */
typedef struct {

  /* Mandatory */
  close_context_handle_type_v01 close_context_handle;
}swi_m2m_fdt_close_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; release access to SWI FOTA partition due to complete successful writing session or due to premature abort. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/
}swi_m2m_fdt_close_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_aggregates
    @{
  */
typedef struct {

  /*  Open context handle for write operation */
  uint32_t fdt_context_handle;

  /*  Reason of closing0: normal, FW ready for next installation.1: OMA DM pre-empts for FUMO/FOTA writing.2: Flash writing problem.3: invalid image signature */
  uint32_t reason;
}ind_context_handle_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Request Message; This indication is SWI FOTA partition opened. It notifies events related to SWI FOTA writing operation.  */
typedef struct {

  /* Mandatory */
  ind_context_handle_type_v01 ind_context_handle;
  /**<   SWI FOTA open context handle*/
}swi_m2m_fdt_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_aggregates
    @{
  */
typedef struct {

  /*  Offset to start FDT write */
  uint32_t fdt_write_offset;

  /*  CRC calculated from the data block spanning from the start of SWIFOTA partition FOTA Location up to the fdt_write_offset - 1  */
  uint32_t crc;
}fdt_read_download_marker_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Request Message; requests the FW to return the last offset where the FDT session has been aborted. */
typedef struct {

  /* Mandatory */
  uint32_t fdt_context_handle;
  /**<   Open context handle for close operation*/
}swi_m2m_fdt_read_download_marker_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; requests the FW to return the last offset where the FDT session has been aborted. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/

  /* Mandatory */
  fdt_read_download_marker_type_v01 fdt_read_download_marker;
  /**<  
                                                                       Offset to start FDT WRITE and CRC
                                                                       */
}swi_m2m_fdt_read_download_marker_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_aggregates
    @{
  */
typedef struct {

  /*  Open context handle for close operation */
  uint32_t fdt_context_handle;

  /*  Offset to start FDT write */
  uint32_t fdt_write_offset;
}fdt_seek_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Request Message; an exclusive access to the SWI FOTA partition. */
typedef struct {

  /* Mandatory */
  fdt_seek_type_v01 fdt_seek;
  /**<  
                                       SWI FOTA open context handle
                                       */
}swi_m2m_fdt_seek_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; an exclusive access to the SWI FOTA partition. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/
}swi_m2m_fdt_seek_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Request Message; requests the FW to return the maximum storage size reserved for SWIFOTA partition. */
typedef struct {

  /* Mandatory */
  /*  Open context handle for close operation */
  uint32_t fdt_context_handle;
}swi_m2m_fdt_max_size_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; requests the FW to return the maximum storage size reserved for SWIFOTA partition. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/

  /* Mandatory */
  uint32_t fdt_max_size;
  /**<  
                                          Max size reserved for SWIFOTA partition (in number of bytes)
                                          */
}swi_m2m_fdt_max_size_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_aggregates
    @{
  */
typedef struct {

  /*  Band number  */
  uint8_t band_number;
  /**<   RF band 3GPP band number. For a full listing of 3GPP band numbers, see 3GPP bands table below.
                            Valid range: 0¨C60. Band support is product specific ¡ª see the device¡¯s Product Specification or Product Technical Specification document for details.
                       */

  /*  band indicator state */
  uint8_t band_ind_1;
  /**<    
                       Band indicator (mapped to a GPIO pin) configurations. 
                       0=Logic low
                       1=Logic high
                       2=Not used for antenna selection (Default value for <band_ind_4>.)
                         */

  /*  band indicator state */
  uint8_t band_ind_2;
  /**<   same 
                       */

  /*  band indicator state */
  uint8_t band_ind_3;
  /**<   same 
                       */
}indicator_config_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Request Message; configure the device to drive (high or low) on band_indicator_1 up to band_indicator_4 for specific bands. */
typedef struct {

  /* Mandatory */
  /*  Band number and band indicator state */
  indicator_config_type_v01 indicator_config;
  /**<  
                        Band number and band indicator
                        */

  /* Optional */
  /*  Band indicator 4 state */
  uint8_t band_ind_4_valid;  /**< Must be set to true if band_ind_4 is being passed */
  uint8_t band_ind_4;
  /**<  
                        Availability is device-specific ¡ª see the appropriate Product Technical Specification for details.
                        */
}swi_m2m_band_indicator_config_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; configure the device to drive (high or low) on band_indicator_1 up to band_indicator_4 for specific bands. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/
}swi_m2m_band_indicator_config_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_aggregates
    @{
  */
typedef struct {

  /*  Band number  */
  uint8_t band_number;
  /**<   RF band 3GPP band number. For a full listing of 3GPP band numbers, see 3GPP bands table below.
                            Valid range: 0¨C60. Band support is product specific ¡ª see the device¡¯s Product Specification or Product Technical Specification document for details.
                       */

  /*  band indicator state */
  uint8_t band_ind_1;
  /**<    
                       Band indicator (mapped to a GPIO pin) configurations. 
                       0=Logic low
                       1=Logic high
                       2=Not used for antenna selection (Default value for <band_ind_4>.)
                         */

  /*  band indicator state */
  uint8_t band_ind_2;
  /**<   same 
                       */

  /*  band indicator state */
  uint8_t band_ind_3;
  /**<   same 
                       */

  /*  band indicator state */
  uint8_t band_ind_4;
  /**<   same 
                       					   */
}indicator_read_type_v01;  /* Type */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_m2m_band_indicator_read_req_msg_v01;

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; This object is used to read the TX Burst HW indication setting value from NV. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/

  /* Mandatory */
  /*  Band number and band indicator state */
  uint32_t indicator_read_len;  /**< Must be set to # of elements in indicator_read */
  indicator_read_type_v01 indicator_read[TOTAL_BAND_MAX_SIZE_V01];
}swi_m2m_band_indicator_read_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Request Message; This object is used to configure the network search mode. */
typedef struct {

  /* Mandatory */
  /*  Continuous network search mode */
  uint8_t enable;
  /**<   0: disable the mode
                               1: enable the mode 
                          */
}swi_m2m_nw_search_config_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; This object is used to configure the network search mode. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/
}swi_m2m_nw_search_config_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_m2m_nw_search_read_req_msg_v01;

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; This object is used to query the continuous network search mode setting. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/

  /* Mandatory */
  /*  Continuous network search mode */
  uint8_t enable;
  /**<   0: disable the mode
                               1: enable the mode 
                          */
}swi_m2m_nw_search_read_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Request Message; This command enables/disables the TX Burst HW Indication setting in NV as similar to AT+WTBI in [R-8]. */
typedef struct {

  /* Mandatory */
  /*  TXBURST /cfg */
  uint8_t TXBURSTIND_select;
  /**<  
                                     0 ¡V Disable TXBURST indication
                                     1 ¡V Enable TXBURST indication
                                     */
}swi_m2m_txburstind_config_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; This command enables/disables the TX Burst HW Indication setting in NV as similar to AT+WTBI in [R-8]. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}swi_m2m_txburstind_config_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_m2m_txburstind_read_req_msg_v01;

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; This object is used to read the TX Burst HW indication setting value from NV. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */

  /* Mandatory */
  /*   */
  uint8_t enabled;
  /**<  
                                              TX Burst HW indication is
                                              0 - Disabled
                                              1 - Enabled  
                                              */
}swi_m2m_txburstind_read_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Request Message; This command set the Ring Indicator Duration, equivalent to AT+WRID in [R-8]. */
typedef struct {

  /* Mandatory */
  /*  RINGIND_CONFIG */
  uint16_t ringind_duration;
  /**<  
                                     Ring Indicator Duration(in msec)
                                     (50-10000)
                                     */
}swi_m2m_ringind_config_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; This command set the Ring Indicator Duration, equivalent to AT+WRID in [R-8]. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}swi_m2m_ringind_config_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_m2m_ringind_read_req_msg_v01;

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; This command is used to read the Ring Indicator Duration value. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */

  /* Mandatory */
  /*   */
  uint16_t ringind_duration;
  /**<  
                                              Ring Indicator Duration(in msec)
                                              */
}swi_m2m_ringind_read_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Request Message; This command set the Wakeup Mask setting, equivalent to AT+WWAKESET in [R-8]. */
typedef struct {

  /* Mandatory */
  /*  WWAKESET_CONFIG */
  uint16_t wwakeset_config;
  /**<  
                                     Bit Mask of the Wakeup Mask setting
                                     0000 - No notification of any of the events
                                     0001 - Notification when the service has been lost.  i.e.  Going from Digital service to NO SERVICE. If the module is in deep sleep (32Khz), the RI will assert and the module will remain asleep
                                     0002 - Notification when going from no service to service. If the module is in deep sleep (32Khz), the RI will assert and the module will remain asleep. Note: Changing SID and remaining on the same service type will NOT trigger the Ring Indicator
                                     0004 - Notification of an incoming voice call.  This is the factory default
                                     0008 - Notification of an incoming data call
                                     0016 - Notification of an incoming SMS message
                                     0032 - Notification of an incoming voice mail indication
                                     0064 - Reserved
                                     0128 - Reserved
                                     0256 - Reserved
                                     0512 - Reserved
                                     1024 - Reserved
                                     2048 - Reserved
                                     4095 - Activation of all assert events
                                     */
}swi_m2m_wwakeset_config_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; This command set the Wakeup Mask setting, equivalent to AT+WWAKESET in [R-8]. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}swi_m2m_wwakeset_config_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_m2m_wwakeset_read_req_msg_v01;

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; This command is used to read the Wakeup Mask setting. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */

  /* Mandatory */
  /*   */
  uint16_t wwakeset_read;
  /**<  
                                              Wakeup Mask setting (bit mask)
                                              (0 - 4095)
                                              */
}swi_m2m_wwakeset_read_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_m2m_wwake_read_req_msg_v01;

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; This command is used to read the wakeup reason, equivalent to AT+WWAKE in [R-8]. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */

  /* Mandatory */
  /*   */
  uint16_t wwake;
  /**<  
                                           Wakeup reason read(bit mask)
                                           (0 - 4095) 
                                           */
}swi_m2m_wwake_read_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Request Message; Request to configure device to Switch-off or enter Low Power Mode, when the temperature reaches critical. */
typedef struct {

  /* Mandatory */
  uint8_t mode;
  /**<  
                                  0x00 ¨C Power Down
                                  0x01 ¨C Low Power Mode
                                  */
}swi_m2m_overtemp_mode_cfg_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; Request to configure device to Switch-off or enter Low Power Mode, when the temperature reaches critical. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/
}swi_m2m_overtemp_mode_cfg_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_m2m_overtemp_mode_read_req_msg_v01;

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; read the over temperature mode configuration for device. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/

  /* Optional */
  uint8_t mode_valid;  /**< Must be set to true if mode is being passed */
  uint8_t mode;
  /**<  
                                  0x00 ¨C Power Down
                                  0x01 ¨C Low Power Mode
                                  */
}swi_m2m_overtemp_mode_read_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Request Message; Perform an OMA-DM AVMS session with the AVMS server  */
typedef struct {

  /* Mandatory */
  /*  SESSION START */
  uint8_t session_type;
  /**<  
                                     0x01 ¨C FOTA, to check availability of FW Update
                                     */
}swi_m2m_avms_session_start_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; Perform an OMA-DM AVMS session with the AVMS server  */
typedef struct {

  /* Mandatory */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/

  /* Mandatory */
  /*  Result Code */
  uint32_t start_session_rsp;
  /**<  
                                  32-bit enum of supported responses:
                                  0x00000001 - Available.
                                  0x00000002 -  Not Available
                                  0x00000003 - Check Timed Out
                                  */
}swi_m2m_avms_session_start_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Request Message; This message requests the device to cancel an ongoing OMA-DM AVMS session */
typedef struct {

  /* Mandatory */
  /*  SESSION STOP */
  uint8_t session_type;
  /**<  
                                 0x01 ¨C FOTA, to suspend FOTA session 
                                 0x02 ¨C Stop active DM session and Disable LWM2M Client 
                                 0xFF ¨C Suspend ongoing FOTA session 
                                        or stop any other active OMADM AVMS session. 
                                 */
}swi_m2m_avms_session_stop_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; This message requests the device to cancel an ongoing OMA-DM AVMS session */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/

  /* Optional */
  /*  SESSION STOP */
  uint8_t session_type_valid;  /**< Must be set to true if session_type is being passed */
  uint8_t session_type;
  /**<  
                                  0x01 ¨C FOTA
                                  */
}swi_m2m_avms_session_stop_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_m2m_avms_session_getinfo_req_msg_v01;

/** @addtogroup qapi_m2m_general_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t binary_type;
  /**<  
                            0x01 ¨C FW
                            0x02 ¨C User app
                            0x03 ¨C Legato
                            */

  uint8_t status;
  /**<        
                            0x01 ¨C No Firmware available
                            0x02 ¨CQuery Firmware Download
                            0x03 ¨C Firmware Downloading
                            0x04 ¨C Firmware downloaded
                            0x05 ¨CQuery Firmware Update
                            0x06 ¨C Firmware updating
                            0x07 ¨C Firmware updated
                             */

  uint8_t user_input_req;
  /**<  
                            Bit mask of available user inputs.
                            0x00 ¨C No user input required. Informational indication
                            0x01 ¨C Accept
                            0x02 ¨C Reject
                            */

  uint16_t user_input_timeout;
  /**<  
                        Timeout for user input in minutes. 
						A value of 0 means no time-out
                        */

  uint32_t fw_dload_size;
  /**<  
                            The size (in bytes) of the firmware update package
                            */

  uint32_t fw_dload_complete;
  /**<  
                                The number of bytes downloaded.
                                			*/

  uint16_t update_complete_status;
  /**<  
                                    See table of session completion codes below.
                                    This field should be looked at only when the OMA-DM AVMS session is complete.
                                    		*/

  uint8_t severity;
  /**<  
                                    0x01 ¨C Mandatory
                                    0x02 - Optional
                                    		*/

  uint16_t version_name_len;
  /**<  
                                Length of FW Version string in bytes
                                	*/

  char version_name[VERSION_MAX_LEN_V01];
  /**<  
                                        version Name in ASCII
                                        */

  uint16_t package_name_len;
  /**<  
                            Length of Package Name string in bytes
                            */

  char package_name[PACKAGE_MAX_LEN_V01];
  /**<  
                                            Package Name in ASCII
                                            		*/

  uint16_t package_description_len;
  /**<  
                                    Length of Package Description string in bytes
                                    */

  char package_description[PACKAGE_DESCRIPTION_MAX_LEN_V01];
  /**<  
                                                                Package Description in ASCII
                                                                	*/
}avms_session_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t state;
  /**<        
                            0x01 ¨C OMA-DM AVMS Read Request
                            0x02 ¨C OMA-DM AVMS Change Request
                            0x03 ¨C OMA-DM AVMS Config Complete
                            */

  uint8_t user_input_req;
  /**<  
                            Bit mask of available user inputs.
                            0x00 ¨C No user input required. Informational indication
                            0x01 ¨C Accept
                            0x02 ¨C Reject
                            */

  uint16_t user_input_timeout;
  /**<  
                        Timeout for user input in minutes. 
						A value of 0 means no time-out
                        */

  uint16_t alertmsglength;
  /**<  
                            Length of Alert message string in bytes
                            	*/

  char alertmsg[ALER_MAX_LEN_V01];
  /**<  
                                    Alert message in UCS2
                                    */
}avms_config_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_aggregates
    @{
  */
typedef struct {

  /*  SESSION GETINFO */
  uint8_t notification;
  /**<  
                            0x12 ¨C Module starts sending data to server
                            0x13 ¨C Authentication with the server
                            0x14 ¨C session with the server is ended
                            	*/

  /*  SESSION GETINFO                               */
  uint16_t session_status;
  /**<  
                                This field will set to the session status for notifications that occur
                                at the end of a session, zero for all other notifications 
                                (see Session Completion Code table below for a summary of session_status codes)
                                */
}avms_notifications_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_aggregates
    @{
  */
typedef struct {

  /*  SESSION GETINFO */
  uint8_t user_input_req;
  /**<  
                             Bit mask of available user inputs.
                             0x00 ¨C No user input required. Informational indication
                             0x01 ¨C Accept
                             0x02 ¨C Reject
                             	*/

  /*  SESSION GETINFO                               */
  uint16_t user_input_timeout;
  /**<  
                                    Timeout for user input in minutes. A value of 0 means no time-out
                                    */
}avms_connection_request_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t notif_type;
  /**<   Notification type
                                     0: Data session closed
                                     1: Data session activated
                                     2: Data session error
                                     */

  /*  Data session error code. */
  uint16_t err_code;
  /**<   Session error code :
                                     0x0000: none
                                     Else see table above.
                                     */
}avms_data_session_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; This message requests the device to cancel an ongoing OMA-DM AVMS session */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/

  /* Optional */
  /*  SESSION INFO */
  uint8_t avms_session_info_valid;  /**< Must be set to true if avms_session_info is being passed */
  avms_session_info_type_v01 avms_session_info;
  /**<   OMA-DM AVMS FOTA Session information */

  /* Optional */
  /*  OMA-DM AVMS Config */
  uint8_t avms_config_info_valid;  /**< Must be set to true if avms_config_info is being passed */
  avms_config_info_type_v01 avms_config_info;
  /**<   OMA-DM AVMS Config information */

  /* Optional */
  /*  OMA-DM AVMS Notification */
  uint8_t avms_notifications_valid;  /**< Must be set to true if avms_notifications is being passed */
  avms_notifications_type_v01 avms_notifications;
  /**<   OMA-DM AVMS Notifications */
}swi_m2m_avms_session_getinfo_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Indication Message; This unsolicited message is sent by the service to 
	a control point that registers for QMI_SWI_M2M_AVMS_SET_EVENT_REPORT when any information in the TLV  */
typedef struct {

  /* Optional */
  /*  OMA-DM AVMS FOTA Session information */
  uint8_t avms_session_info_valid;  /**< Must be set to true if avms_session_info is being passed */
  avms_session_info_type_v01 avms_session_info;
  /**<   OMA-DM AVMS FOTA Session information */

  /* Optional */
  /*  OMA-DM AVMS Config */
  uint8_t avms_config_info_valid;  /**< Must be set to true if avms_config_info is being passed */
  avms_config_info_type_v01 avms_config_info;
  /**<   OMA-DM AVMS Config information */

  /* Optional */
  /*  OMA-DM AVMS Notification */
  uint8_t avms_notifications_valid;  /**< Must be set to true if avms_notifications is being passed */
  avms_notifications_type_v01 avms_notifications;
  /**<   OMA-DM AVMS Notifications */

  /* Optional */
  /*  AVMS Connection Request */
  uint8_t avms_connection_request_valid;  /**< Must be set to true if avms_connection_request is being passed */
  avms_connection_request_type_v01 avms_connection_request;
  /**<   AVMS Connection Request */

  /* Optional */
  /*  OMA-DM WASM parameters changed. */
  uint8_t wams_changed_mask_valid;  /**< Must be set to true if wams_changed_mask is being passed */
  uint32_t wams_changed_mask;
  /**<   Mask of WAMS parameters changed. By default set to 0xFF for all changes.
                                     0x1 - device_login
                                     0x2 - device_MD5_key
                                     0x4 - server_login
                                     0x8 - server_MD5_key
                                     0x10 - server_URL
                                     0x20 ¨C Nonce
                                     0x40 ¨C Application key
                                     */

  /* Optional */
  /*  Package ID (relevant to Application binary package). */
  uint8_t package_ID_valid;  /**< Must be set to true if package_ID is being passed */
  uint8_t package_ID;
  /**<   Package ID of the application binary that this AVMS_EVENT_ID notification is for */

  /* Optional */
  /*  LWM2M Registration status. */
  uint8_t reg_status_valid;  /**< Must be set to true if reg_status is being passed */
  uint8_t reg_status;
  /**<   
                                     0: Need Bootstrap
                                     1: Bootstrap made
                                     2: Register made
                                     3: Update made
                                     */

  /* Optional */
  /*  Data session status. */
  uint8_t avms_data_session_valid;  /**< Must be set to true if avms_data_session is being passed */
  avms_data_session_type_v01 avms_data_session;
  /**<   data session status*/

  /* Optional */
  /*  LWM2M session type. */
  uint8_t session_type_valid;  /**< Must be set to true if session_type is being passed */
  uint8_t session_type;
  /**<   
                                     0: Bootstrap session
                                     1: DM session
                                     */

  /* Optional */
  /*  LWM2M http status. */
  uint8_t http_status_valid;  /**< Must be set to true if http_status is being passed */
  uint16_t http_status;
  /**<   see RFC 7231 Section 6 */
}swi_m2m_avms_event_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Request Message; This message is used to make a selection after the host has received QMI_SWI_M2M_AVMS_EVENT_IND. */
typedef struct {

  /* Mandatory */
  /*  User selection */
  uint8_t user_input;
  /**<  
                                0x01 ¨C Accept
                                0x02 ¨C Reject
                                0x03 ¨C Defer
                                */

  /* Optional */
  /*  Defer time */
  uint8_t defer_time_valid;  /**< Must be set to true if defer_time is being passed */
  uint32_t defer_time;
  /**<  
                                Defer time in minutes. A value of 0 will cause the prompt to be resent immediately.
                                */

  /* Optional */
  /*  Client operation flag after accept */
  uint8_t client_perform_operation_flag_valid;  /**< Must be set to true if client_perform_operation_flag is being passed */
  uint8_t client_perform_operation_flag;
  /**<  
                                                   0: if modem performs the operation (download or update)
                                                   1: if client performs the operation (download or update)
                                                   */

  /* Optional */
  /*  Package ID (relevant to Application binary package) */
  uint8_t package_ID_valid;  /**< Must be set to true if package_ID is being passed */
  uint8_t package_ID;
  /**<   
                                    Package ID of the application binary that the client is referring to for its selection. 
                                    If this field is absent, 
                                    the modem will assume the package of the most recent AVMS_EVENT_ID notification
                                    */

  /* Optional */
  /*  Reject reason */
  uint8_t reject_reason_valid;  /**< Must be set to true if reject_reason is being passed */
  uint16_t reject_reason;
  /**<   Reject reason                                 */
}swi_m2m_avms_selection_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; This message is used to make a selection after the host has received QMI_SWI_M2M_AVMS_EVENT_IND. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/
}swi_m2m_avms_selection_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t apn_len;
  /**<   Length of APN string in bytes 	*/

  char apn[APN_MAX_LEN_V01];
  /**<   APN in ASCII */

  uint8_t uname_len;
  /**<   Length of Username string in bytes 	*/

  char uname[UNAME_MAX_LEN_V01];
  /**<   Username in ASCII */

  uint8_t pwd_len;
  /**<   Length of Username string in bytes 	*/

  char pwd[PWD_MAX_LEN_V01];
  /**<   Username in ASCII */
}apn_config_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t p_min;
  /**<   Minimum default reporting period in seconds 	*/

  uint32_t p_max;
  /**<   Maximum default reporting period in seconds 	*/
}notif_period_type_v01;  /* Type */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_m2m_avms_get_setting_req_msg_v01;

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; This message gets settings related to OMA DM AVMS */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/

  /* Mandatory */
  /*  OMA-DM Enabled */
  uint32_t enabled;
  /**<  
                                32-bit bitmask of supported sessions: 
                                0x00000010: Client Initiated FUMO (CIFUMO) 
                                0x00000020: Network Initiated  FUMO (NIFUMO) 
                                */

  /* Mandatory */
  /*  FOTA Automatic Download */
  uint8_t fw_autodload;
  /**<  
                                0x00 ¨C FALSE
                                0x01 ¨C TRUE
                                */

  /* Mandatory */
  /*  FOTA Automatic Update */
  uint8_t fw_autoupdate;
  /**<  
                                0x00 ¨C FALSE
                                0x01 ¨C TRUE
                                */

  /* Mandatory */
  /*  OMA Automatic UI Alert Response */
  uint8_t fw_autosdm;
  /**<  
                                0x00 ¨C DISABLED
                                0x01 ¨C ENABLED ACCEPT
                                0x02 ¨C ENABLED REJECT
                                */

  /* Mandatory */
  /*  Automatic Connect */
  uint8_t autoconnect;
  /**<  
                                  0x00 ¨C FALSE
                                  0x01 ¨C TRUE
                                     */

  /* Optional */
  /*  Polling Timer */
  uint8_t polling_timer_valid;  /**< Must be set to true if polling_timer is being passed */
  uint32_t polling_timer;
  /**<  0-525600 minutes; 0: disabled */

  /* Optional */
  /*  Connection Retry Timer */
  uint8_t retry_timer_valid;  /**< Must be set to true if retry_timer is being passed */
  uint16_t retry_timer[8];
  /**<  0-20160 minutes; retry_timer[0]=0 to disable */

  /* Optional */
  /*  APN Configuration */
  uint8_t apn_config_valid;  /**< Must be set to true if apn_config is being passed */
  apn_config_type_v01 apn_config;
  /**<   APN Configuration */

  /* Optional */
  /*  Notification Storage */
  uint8_t notif_store_valid;  /**< Must be set to true if notif_store is being passed */
  uint8_t notif_store;
  /**<  0: disabled; 1: enabled */

  /* Optional */
  /*  Default Reporting Period */
  uint8_t notif_period_valid;  /**< Must be set to true if notif_period is being passed */
  notif_period_type_v01 notif_period;
  /**<   Notification Period p_min and p_max */

  /* Optional */
  /*  WDSI Bitmask */
  uint8_t wdsi_mask_valid;  /**< Must be set to true if wdsi_mask is being passed */
  uint32_t wdsi_mask;
  /**<  0-0x1FFF */
}swi_m2m_avms_get_setting_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Request Message; This message gets settings related to OMA DM AVMS */
typedef struct {

  /* Mandatory */
  /*  FOTA Automatic Download */
  uint8_t fw_autodload;
  /**<  
                                0x00 ¨C FALSE
                                0x01 ¨C TRUE
                                */

  /* Mandatory */
  /*  FOTA Automatic Update */
  uint8_t fw_autoupdate;
  /**<  
                                0x00 ¨C FALSE
                                0x01 ¨C TRUE
                                */

  /* Mandatory */
  /*  Automatic Connect */
  uint8_t autoconnect;
  /**<  
                                0x00 ¨C FALSE
                                0x01 ¨C TRUE
                                                                */

  /* Optional */
  /*  OMA Automatic UI Alert Response */
  uint8_t fw_autosdm_valid;  /**< Must be set to true if fw_autosdm is being passed */
  uint8_t fw_autosdm;
  /**<  
                                0x00 ¨C DISABLED
                                0x01 ¨C ENABLED ACCEPT
                                0x02 ¨C ENABLED REJECT
                                */

  /* Optional */
  /*  Polling Timer */
  uint8_t polling_timer_valid;  /**< Must be set to true if polling_timer is being passed */
  uint32_t polling_timer;
  /**<  0-525600 minutes; 0: disabled */

  /* Optional */
  /*  Connection Retry Timer */
  uint8_t retry_timer_valid;  /**< Must be set to true if retry_timer is being passed */
  uint16_t retry_timer[8];
  /**<  0-20160 minutes; retry_timer[0]=0 to disable */

  /* Optional */
  /*  APN Configuration */
  uint8_t apn_config_valid;  /**< Must be set to true if apn_config is being passed */
  apn_config_type_v01 apn_config;
  /**<   APN Configuration */

  /* Optional */
  /*  Notification Storage */
  uint8_t notif_store_valid;  /**< Must be set to true if notif_store is being passed */
  uint8_t notif_store;
  /**<  0: disabled; 1: enabled */

  /* Optional */
  /*  Default Reporting Period */
  uint8_t notif_period_valid;  /**< Must be set to true if notif_period is being passed */
  notif_period_type_v01 notif_period;
  /**<   Notification Period p_min and p_max */

  /* Optional */
  /*  WDSI Bitmask */
  uint8_t wdsi_mask_valid;  /**< Must be set to true if wdsi_mask is being passed */
  uint32_t wdsi_mask;
  /**<  0-0x1FFF */
}swi_m2m_avms_set_setting_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; This message gets settings related to OMA DM AVMS */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/
}swi_m2m_avms_set_setting_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_m2m_avms_set_event_report_req_msg_v01;

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; This message is used to register for the QMI_SWI_M2M_AVMS_EVENT_IND */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/
}swi_m2m_avms_set_event_report_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_m2m_avms_get_wams_req_msg_v01;

/** @addtogroup qapi_m2m_general_qmi_aggregates
    @{
  */
typedef struct {

  /*  Device login. ID specific to each device */
  uint8_t device_login[16];

  /*  Device MD5 key (password) */
  uint8_t device_MD5_key[16];

  /*  Server login */
  uint8_t server_login[16];

  /*  Server password */
  uint8_t server_MD5_key[16];

  /*  ASCII string of the AVMS server URL */
  uint8_t server_URL[80];

  /*  Complete Nonce value to be used for next authentication */
  uint8_t Nonce[16];
}wams_service_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_aggregates
    @{
  */
typedef struct {

  /*  Number of parameter sets that follow */
  uint8_t Num_instance;

  /*  Index of the key */
  uint8_t key_index;

  /*  Version of the key */
  uint8_t key_version;

  /*  Length of the key */
  uint16_t key_length;

  /*  Key data */
  uint8_t key_data[294];

  /*  Spare meta data */
  uint8_t key_meta_data[8];
}wams_key_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; This message is retrieves only those that are used by the application processor */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/

  /* Mandatory */
  /*  WAMS parameters */
  wams_service_type_v01 wams_service;
  /**<  
                                             WAMS parameters relevant to the application processor side
                                             */

  /* Mandatory */
  /*  Application public key */
  wams_key_type_v01 wams_key;
  /**<  
                                        Application public key
                                        */
}swi_m2m_avms_get_wams_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_m2m_avms_open_nonce_req_msg_v01;

/** @addtogroup qapi_m2m_general_qmi_aggregates
    @{
  */
typedef struct {

  /*  The delta binary of the nonce value */
  uint8_t delta_nonce[16];

  /*  CRC of the actual nonce (for comparison¡¯s purpose) */
  uint32_t crc_actual_nonce;
}Delta_nonce_value_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; This message informs the modem that the application side is
     about to enter the authentication phase */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/

  /* Mandatory */
  /*  Delta binary and CRC  */
  Delta_nonce_value_v01 Delta_nonce;
  /**<  
                                             Delta binary and CRC
                                             */
}swi_m2m_avms_open_nonce_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Request Message; This message informs the modem that the application side is
     about to enter the authentication phase */
typedef struct {

  /* Mandatory */
  /*  The delta binary of the nonce value */
  uint8_t delta_nonce[NONCE_LEN_V01];
}swi_m2m_avms_close_nonce_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; This message informs the modem that the application side is
     about to enter the authentication phase */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/
}swi_m2m_avms_close_nonce_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_m2m_sim_refresh_req_msg_v01;

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; Request to trigger sim refresh */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/
}swi_m2m_sim_refresh_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_m2m_lpm_mode_read_req_msg_v01;

/** @addtogroup qapi_m2m_general_qmi_aggregates
    @{
  */
typedef struct {

  /*  The sleep mode */
  uint8_t mode;
  /**<  
                  0:  Disallow deep sleep mode from being entered
                  1:  Allow deep sleep mode to be entered when all sleep conditions are met
              */

  /*  UART1 DTR condition */
  uint8_t uart1_dtr_condition;
  /**<  
                               0:  UART1 DTR condition does not control module sleep/wake state
                               1:  UART1 DTR condition controls module sleep/wake state if UART1 is mapped to the AT command service. 
                               UART1 DTR asserted (high) wakes up module. 
                               UART1 DTR de-asserted (low) allows the module to enter sleep when all other conditions are met.
                            */
}sleep_mode_config_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; Request to read the LPM mode setting */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/

  /* Optional */
  /*  sleep mode config */
  uint8_t sleep_mode_valid;  /**< Must be set to true if sleep_mode is being passed */
  sleep_mode_config_v01 sleep_mode;
}swi_m2m_lpm_mode_read_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Request Message; Request to config the LPM mode setting */
typedef struct {

  /* Mandatory */
  /*  sleep mode config */
  sleep_mode_config_v01 sleep_mode_cfg;
}swi_m2m_lpm_mode_cfg_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; Request to config the LPM mode setting */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/
}swi_m2m_lpm_mode_cfg_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Request Message; This command enable/disable GLONASS. */
typedef struct {

  /* Mandatory */
  /*  gpsonly */
  uint8_t GPSOnly;
  /**<  
    Set GNSS work mode:
    0 - GPS+GLONASS
    1 - GPS Only
  */
}swi_m2m_gps_set_gpsonly_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; This command enable/disable GLONASS. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}swi_m2m_gps_set_gpsonly_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_m2m_gps_get_gpsonly_req_msg_v01;

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; This command query GNSS work mode. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  gpsonly */
  uint8_t GPSOnly;
  /**<  
    Return GNSS work mode:
    0 - GPS+GLONASS
    1 - GPS Only
  */
}swi_m2m_gps_get_gpsonly_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t xtra_data_retry_num;
  /**<  
    XTRA number of download retries.
    0 - 10
  */

  uint8_t xtra_data_retry_interval;
  /**<  
    XTRA time between retries, in minute
    1 - 120
  */
}gps_xtra_data_retry_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t xtra_data_auto_download_flg;
  /**<  
    XTRA auto download
    0- Disable
    1- Enable
  */

  uint16_t xtra_data_auto_download_interval;
  /**<  
    XTRA auto download interval, in hour
    24-168
  */
}gps_xtra_data_auto_download_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Request Message; This command controls GPS XTRA data related settings. */
typedef struct {

  /* Mandatory */
  /*  gps_xtra_data_enable_flag */
  uint8_t gps_xtra_data_enable_flag;
  /**<  
    Set GPS XTRA data enable flag:
    0 - Disable
    1 - Enable, XTRA data can be injected from external such as QMI API
    2 - Enable gpsOneXTRA and embedded QCT XTRA Client. (QMI mode only)
  */

  /* Optional */
  /*  gps_xtra_data_retry */
  uint8_t gps_xtra_data_retry_valid;  /**< Must be set to true if gps_xtra_data_retry is being passed */
  gps_xtra_data_retry_type_v01 gps_xtra_data_retry;

  /* Optional */
  /*  gps_xtra_data_auto_download */
  uint8_t gps_xtra_data_auto_download_valid;  /**< Must be set to true if gps_xtra_data_auto_download is being passed */
  gps_xtra_data_auto_download_type_v01 gps_xtra_data_auto_download;

  /* Optional */
  /*  gps_xtra_data_valid_time */
  uint8_t gps_xtra_data_valid_time_valid;  /**< Must be set to true if gps_xtra_data_valid_time is being passed */
  uint16_t gps_xtra_data_valid_time;
  /**<  
    Number of hours the XTRA data is considered Valid.
    1-168
  */
}swi_m2m_gps_set_xtra_data_enable_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; This command controls GPS XTRA data related settings. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}swi_m2m_gps_set_xtra_data_enable_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_m2m_gps_get_xtra_data_enable_req_msg_v01;

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; This command query GPS XTRA data related settings. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  gps_xtra_data_enable_flag */
  uint8_t gps_xtra_data_enable_flag;
  /**<  
    Set GPS XTRA data enable flag:
    0 - Disable
    1 - Enable, XTRA data can be injected from external such as QMI API
    2 - Enable gpsOneXTRA and embedded QCT XTRA Client. (QMI mode only)
  */

  /* Mandatory */
  /*  gps_xtra_data_retry */
  gps_xtra_data_retry_type_v01 gps_xtra_data_retry;

  /* Mandatory */
  /*  gps_xtra_data_auto_download */
  gps_xtra_data_auto_download_type_v01 gps_xtra_data_auto_download;

  /* Mandatory */
  /*  gps_xtra_data_valid_time */
  uint16_t gps_xtra_data_valid_time;
  /**<  
    Number of hours the XTRA data is considered Valid.
    1-168
  */
}swi_m2m_gps_get_xtra_data_enable_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t gpio_id;
  /**<  
    Possible range is from 1 to max of uint8(actual number varied by product)
  */

  uint8_t gpio_fun;
  /**<  
    0 - UNALLOCATED
    4 - GENERAL
    16 - EMBEDDED_HOST
  */
}cfg_gpio_fun_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Request Message; This command configures the function of I/O port.The I/O port can work as below function. 0: UNALLOCATED      (GPIO unused and will be set as inactive)  1: ECALL_TRIGGER   ( GPIO is used to originate ECALL)  2: ANT_SEL                  (GPIO is used as band indication pin)  3: EXT_SIM2_DET       (GPIO is used as detection pin of external SIM slot 2)  4: GENERAL             (GPIO works as general function and controlled by AT command)  5: SLEEP_IND           (GPIO is used to indicate device sleeping state)  16: EMBEDDED_HOST   (GPIO is assigned to App side, and can be controlled by Legato) Please note that this QMI only support to change the IO function between  0: UNALLOCATED, 4: GENERAL; 16: EMBEDDED_HOST , in other words, once the IO pin has work as especial function (1: ECALL_TRIGGER, 2: ANT_SEL, 3: EXT_SIM2_DET, 5: SLEEP_IND) it can¡¯t be change to other function via this QMI command. */
typedef struct {

  /* Mandatory */
  /*  cfg_gpio_fun */
  cfg_gpio_fun_type_v01 cfg_gpio_fun;
}swi_m2m_gpio_cfg_function_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; This command configures the function of I/O port.The I/O port can work as below function. 0: UNALLOCATED      (GPIO unused and will be set as inactive)  1: ECALL_TRIGGER   ( GPIO is used to originate ECALL)  2: ANT_SEL                  (GPIO is used as band indication pin)  3: EXT_SIM2_DET       (GPIO is used as detection pin of external SIM slot 2)  4: GENERAL             (GPIO works as general function and controlled by AT command)  5: SLEEP_IND           (GPIO is used to indicate device sleeping state)  16: EMBEDDED_HOST   (GPIO is assigned to App side, and can be controlled by Legato) Please note that this QMI only support to change the IO function between  0: UNALLOCATED, 4: GENERAL; 16: EMBEDDED_HOST , in other words, once the IO pin has work as especial function (1: ECALL_TRIGGER, 2: ANT_SEL, 3: EXT_SIM2_DET, 5: SLEEP_IND) it can¡¯t be change to other function via this QMI command. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}swi_m2m_gpio_cfg_function_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Request Message; This object is used to read the function of specified I/O port. */
typedef struct {

  /* Mandatory */
  /*  gpio_id */
  uint8_t gpio_id;
  /**<  
    Possible range is from 1 to max of uint8 (actual number varied by product).
  */
}swi_m2m_gpio_read_function_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_m2m_general_qmi_messages
    @{
  */
/** Response Message; This object is used to read the function of specified I/O port. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  cfg_gpio_fun */
  cfg_gpio_fun_type_v01 cfg_gpio_fun;
}swi_m2m_gpio_read_function_resp_msg_v01;  /* Message */
/**
    @}
  */

/* Conditional compilation tags for message removal */ 
//#define REMOVE_QMI_SWI_M2M_AVMS_CLOSE_NONCE_V01 
//#define REMOVE_QMI_SWI_M2M_AVMS_EVENT_IND_V01 
//#define REMOVE_QMI_SWI_M2M_AVMS_GET_SETTINGS_V01 
//#define REMOVE_QMI_SWI_M2M_AVMS_GET_WAMS_V01 
//#define REMOVE_QMI_SWI_M2M_AVMS_OPEN_NONCE_V01 
//#define REMOVE_QMI_SWI_M2M_AVMS_SELECTION_V01 
//#define REMOVE_QMI_SWI_M2M_AVMS_SESSION_GETINFO_V01 
//#define REMOVE_QMI_SWI_M2M_AVMS_SESSION_START_V01 
//#define REMOVE_QMI_SWI_M2M_AVMS_SESSION_STOP_V01 
//#define REMOVE_QMI_SWI_M2M_AVMS_SET_EVENT_REPORT_V01 
//#define REMOVE_QMI_SWI_M2M_AVMS_SET_SETTINGS_V01 
//#define REMOVE_QMI_SWI_M2M_BAND_INDICATOR_CONFIG_V01 
//#define REMOVE_QMI_SWI_M2M_BAND_INDICATOR_READ_V01 
//#define REMOVE_QMI_SWI_M2M_COIN_CELL_CFG_READ_V01 
//#define REMOVE_QMI_SWI_M2M_COIN_CELL_CFG_SETTING_V01 
//#define REMOVE_QMI_SWI_M2M_COIN_CELL_CHECK_CTRL_V01 
//#define REMOVE_QMI_SWI_M2M_COIN_CELL_CTRL_V01 
//#define REMOVE_QMI_SWI_M2M_ECALL_CONFIG_V01 
//#define REMOVE_QMI_SWI_M2M_ECALL_CONFIG_GPIO_V01 
//#define REMOVE_QMI_SWI_M2M_ECALL_GET_OP_MODE_V01 
//#define REMOVE_QMI_SWI_M2M_ECALL_GET_TX_MODE_V01 
//#define REMOVE_QMI_SWI_M2M_ECALL_READ_CONFIG_V01 
//#define REMOVE_QMI_SWI_M2M_ECALL_READ_GPIO_V01 
//#define REMOVE_QMI_SWI_M2M_ECALL_READ_MSD_BLOCK_V01 
//#define REMOVE_QMI_SWI_M2M_ECALL_SEND_MSD_BLOB_V01 
//#define REMOVE_QMI_SWI_M2M_ECALL_SET_OP_MODE_V01 
//#define REMOVE_QMI_SWI_M2M_ECALL_SET_TX_MODE_V01 
//#define REMOVE_QMI_SWI_M2M_ECALL_START_V01 
//#define REMOVE_QMI_SWI_M2M_ECALL_STATUS_IND_V01 
//#define REMOVE_QMI_SWI_M2M_ECALL_STOP_V01 
//#define REMOVE_QMI_SWI_M2M_ECALL_UPDATE_MSD_BLOCK_V01 
//#define REMOVE_QMI_SWI_M2M_FDT_CLOSE_V01 
//#define REMOVE_QMI_SWI_M2M_FDT_IND_V01 
//#define REMOVE_QMI_SWI_M2M_FDT_MAX_SIZE_V01 
//#define REMOVE_QMI_SWI_M2M_FDT_OPEN_V01 
//#define REMOVE_QMI_SWI_M2M_FDT_READ_V01 
//#define REMOVE_QMI_SWI_M2M_FDT_READ_DOWNLOAD_MARKER_V01 
//#define REMOVE_QMI_SWI_M2M_FDT_SEEK_V01 
//#define REMOVE_QMI_SWI_M2M_FDT_WRITE_V01 
//#define REMOVE_QMI_SWI_M2M_GET_RI_OWNER_V01 
//#define REMOVE_QMI_SWI_M2M_GPIO_CFG_FUNCTION_V01 
//#define REMOVE_QMI_SWI_M2M_GPIO_CFG_TRIGGER_V01 
//#define REMOVE_QMI_SWI_M2M_GPIO_CHANGE_IND_V01 
//#define REMOVE_QMI_SWI_M2M_GPIO_READ_V01 
//#define REMOVE_QMI_SWI_M2M_GPIO_READ_FUNCTION_V01 
//#define REMOVE_QMI_SWI_M2M_GPIO_WRITE_V01 
//#define REMOVE_QMI_SWI_M2M_GPS_GET_GPSONLY_V01 
//#define REMOVE_QMI_SWI_M2M_GPS_GET_XTRA_DATA_ENABLE_V01 
//#define REMOVE_QMI_SWI_M2M_GPS_SET_GPSONLY_V01 
//#define REMOVE_QMI_SWI_M2M_GPS_SET_XTRA_DATA_ENABLE_V01 
//#define REMOVE_QMI_SWI_M2M_JAMMING_IND_V01 
//#define REMOVE_QMI_SWI_M2M_JAMMING_SET_EVENT_REPORT_V01 
//#define REMOVE_QMI_SWI_M2M_LED_CFG_V01 
//#define REMOVE_QMI_SWI_M2M_LED_READ_V01 
//#define REMOVE_QMI_SWI_M2M_LPM_MODE_CFG_V01 
//#define REMOVE_QMI_SWI_M2M_LPM_MODE_READ_V01 
//#define REMOVE_QMI_SWI_M2M_NW_SEARCH_CONFIG_V01 
//#define REMOVE_QMI_SWI_M2M_NW_SEARCH_READ_V01 
//#define REMOVE_QMI_SWI_M2M_OVERTEMP_MODE_CFG_V01 
//#define REMOVE_QMI_SWI_M2M_OVERTEMP_MODE_READ_V01 
//#define REMOVE_QMI_SWI_M2M_PING_V01 
//#define REMOVE_QMI_SWI_M2M_RINGIND_CONFIG_V01 
//#define REMOVE_QMI_SWI_M2M_RINGIND_READ_V01 
//#define REMOVE_QMI_SWI_M2M_SET_RI_OWNER_V01 
//#define REMOVE_QMI_SWI_M2M_SIM_REFRESH_V01 
//#define REMOVE_QMI_SWI_M2M_TXBURSTIND_CONFIG_V01 
//#define REMOVE_QMI_SWI_M2M_TXBURSTIND_READ_V01 
//#define REMOVE_QMI_SWI_M2M_WWAKESET_CONFIG_V01 
//#define REMOVE_QMI_SWI_M2M_WWAKESET_READ_V01 
//#define REMOVE_QMI_SWI_M2M_WWAKE_READ_V01 

/*Service Message Definition*/
/** @addtogroup qapi_m2m_general_qmi_msg_ids
    @{
  */
#define QMI_SWI_M2M_PING_REQ_V01 0x0000
#define QMI_SWI_M2M_PING_RESP_V01 0x0000
#define QMI_SWI_M2M_PING_IND_V01 0x0000
#define QMI_SWI_M2M_GPIO_READ_REQ_V01 0x0001
#define QMI_SWI_M2M_GPIO_READ_RESP_V01 0x0001
#define QMI_SWI_M2M_GPIO_WRITE_REQ_V01 0x0002
#define QMI_SWI_M2M_GPIO_WRITE_RESP_V01 0x0002
#define QMI_SWI_M2M_GPIO_CFG_TRIGGER_REQ_V01 0x0003
#define QMI_SWI_M2M_GPIO_CFG_TRIGGER_RESP_V01 0x0003
#define QMI_SWI_M2M_GPIO_CHANGE_IND_V01 0x0004
#define QMI_SWI_M2M_SET_RI_OWNER_REQ_V01 0x0005
#define QMI_SWI_M2M_SET_RI_OWNER_RESP_V01 0x0005
#define QMI_SWI_M2M_GET_RI_OWNER_REQ_V01 0x0006
#define QMI_SWI_M2M_GET_RI_OWNER_RESP_V01 0x0006
#define QMI_SWI_M2M_GPIO_CFG_FUNCTION_REQ_V01 0x0007
#define QMI_SWI_M2M_GPIO_CFG_FUNCTION_RESP_V01 0x0007
#define QMI_SWI_M2M_GPIO_READ_FUNCTION_REQ_V01 0x0008
#define QMI_SWI_M2M_GPIO_READ_FUNCTION_RESP_V01 0x0008
#define QMI_SWI_M2M_LED_CFG_REQ_V01 0x0010
#define QMI_SWI_M2M_LED_CFG_RESP_V01 0x0010
#define QMI_SWI_M2M_LED_READ_REQ_V01 0x0011
#define QMI_SWI_M2M_LED_READ_RESP_V01 0x0011
#define QMI_SWI_M2M_COIN_CELL_CFG_SETTING_REQ_V01 0x0020
#define QMI_SWI_M2M_COIN_CELL_CFG_SETTING_RESP_V01 0x0020
#define QMI_SWI_M2M_COIN_CELL_CFG_READ_REQ_V01 0x0021
#define QMI_SWI_M2M_COIN_CELL_CFG_READ_RESP_V01 0x0021
#define QMI_SWI_M2M_COIN_CELL_CTRL_REQ_V01 0x0022
#define QMI_SWI_M2M_COIN_CELL_CTRL_RESP_V01 0x0022
#define QMI_SWI_M2M_COIN_CELL_CHECK_CTRL_REQ_V01 0x0023
#define QMI_SWI_M2M_COIN_CELL_CHECK_CTRL_RESP_V01 0x0023
#define QMI_SWI_M2M_JAMMING_IND_V01 0x0035
#define QMI_SWI_M2M_JAMMING_SET_EVENT_REPORT_REQ_V01 0x0036
#define QMI_SWI_M2M_JAMMING_SET_EVENT_REPORT_RESP_V01 0x0036
#define QMI_SWI_M2M_ECALL_CONFIG_GPIO_REQ_V01 0x0040
#define QMI_SWI_M2M_ECALL_CONFIG_GPIO_RESP_V01 0x0040
#define QMI_SWI_M2M_ECALL_READ_GPIO_REQ_V01 0x0041
#define QMI_SWI_M2M_ECALL_READ_GPIO_RESP_V01 0x0041
#define QMI_SWI_M2M_ECALL_CONFIG_REQ_V01 0x0042
#define QMI_SWI_M2M_ECALL_CONFIG_RESP_V01 0x0042
#define QMI_SWI_M2M_ECALL_READ_CONFIG_REQ_V01 0x0043
#define QMI_SWI_M2M_ECALL_READ_CONFIG_RESP_V01 0x0043
#define QMI_SWI_M2M_ECALL_SET_TX_MODE_REQ_V01 0x0044
#define QMI_SWI_M2M_ECALL_SET_TX_MODE_RESP_V01 0x0044
#define QMI_SWI_M2M_ECALL_GET_TX_MODE_REQ_V01 0x0045
#define QMI_SWI_M2M_ECALL_GET_TX_MODE_RESP_V01 0x0045
#define QMI_SWI_M2M_ECALL_SET_OP_MODE_REQ_V01 0x0046
#define QMI_SWI_M2M_ECALL_SET_OP_MODE_RESP_V01 0x0046
#define QMI_SWI_M2M_ECALL_GET_OP_MODE_REQ_V01 0x0047
#define QMI_SWI_M2M_ECALL_GET_OP_MODE_RESP_V01 0x0047
#define QMI_SWI_M2M_ECALL_SEND_MSD_BLOB_REQ_V01 0x0049
#define QMI_SWI_M2M_ECALL_SEND_MSD_BLOB_RESP_V01 0x0049
#define QMI_SWI_M2M_ECALL_UPDATE_MSD_BLOCK_REQ_V01 0x004A
#define QMI_SWI_M2M_ECALL_UPDATE_MSD_BLOCK_RESP_V01 0x004A
#define QMI_SWI_M2M_ECALL_START_REQ_V01 0x004B
#define QMI_SWI_M2M_ECALL_START_RESP_V01 0x004B
#define QMI_SWI_M2M_ECALL_STOP_REQ_V01 0x004C
#define QMI_SWI_M2M_ECALL_STOP_RESP_V01 0x004C
#define QMI_SWI_M2M_ECALL_STATUS_IND_V01 0x004D
#define QMI_SWI_M2M_ECALL_READ_MSD_BLOCK_REQ_V01 0x004E
#define QMI_SWI_M2M_ECALL_READ_MSD_BLOCK_RESP_V01 0x004E
#define QMI_SWI_M2M_FDT_OPEN_REQ_V01 0x0050
#define QMI_SWI_M2M_FDT_OPEN_RESP_V01 0x0050
#define QMI_SWI_M2M_FDT_WRITE_REQ_V01 0x0051
#define QMI_SWI_M2M_FDT_WRITE_RESP_V01 0x0051
#define QMI_SWI_M2M_FDT_CLOSE_REQ_V01 0x0052
#define QMI_SWI_M2M_FDT_CLOSE_RESP_V01 0x0052
#define QMI_SWI_M2M_FDT_IND_V01 0x0053
#define QMI_SWI_M2M_FDT_READ_DOWNLOAD_MARKER_REQ_V01 0x0054
#define QMI_SWI_M2M_FDT_READ_DOWNLOAD_MARKER_RESP_V01 0x0054
#define QMI_SWI_M2M_FDT_SEEK_REQ_V01 0x0055
#define QMI_SWI_M2M_FDT_SEEK_RESP_V01 0x0055
#define QMI_SWI_M2M_FDT_MAX_SIZE_REQ_V01 0x0056
#define QMI_SWI_M2M_FDT_MAX_SIZE_RESP_V01 0x0056
#define QMI_SWI_M2M_FDT_READ_REQ_V01 0x0057
#define QMI_SWI_M2M_FDT_READ_RESP_V01 0x0057
#define QMI_SWI_M2M_BAND_INDICATOR_CONFIG_REQ_V01 0x0060
#define QMI_SWI_M2M_BAND_INDICATOR_CONFIG_RESP_V01 0x0060
#define QMI_SWI_M2M_BAND_INDICATOR_READ_REQ_V01 0x0061
#define QMI_SWI_M2M_BAND_INDICATOR_READ_RESP_V01 0x0061
#define QMI_SWI_M2M_NW_SEARCH_CONFIG_REQ_V01 0x0065
#define QMI_SWI_M2M_NW_SEARCH_CONFIG_RESP_V01 0x0065
#define QMI_SWI_M2M_NW_SEARCH_READ_REQ_V01 0x0066
#define QMI_SWI_M2M_NW_SEARCH_READ_RESP_V01 0x0066
#define QMI_SWI_M2M_TXBURSTIND_CONFIG_REQ_V01 0x0070
#define QMI_SWI_M2M_TXBURSTIND_CONFIG_RESP_V01 0x0070
#define QMI_SWI_M2M_TXBURSTIND_READ_REQ_V01 0x0071
#define QMI_SWI_M2M_TXBURSTIND_READ_RESP_V01 0x0071
#define QMI_SWI_M2M_RINGIND_CONFIG_REQ_V01 0x0080
#define QMI_SWI_M2M_RINGIND_CONFIG_RESP_V01 0x0080
#define QMI_SWI_M2M_RINGIND_READ_REQ_V01 0x0081
#define QMI_SWI_M2M_RINGIND_READ_RESP_V01 0x0081
#define QMI_SWI_M2M_WWAKESET_CONFIG_REQ_V01 0x0082
#define QMI_SWI_M2M_WWAKESET_CONFIG_RESP_V01 0x0082
#define QMI_SWI_M2M_WWAKESET_READ_REQ_V01 0x0083
#define QMI_SWI_M2M_WWAKESET_READ_RESP_V01 0x0083
#define QMI_SWI_M2M_WWAKE_READ_REQ_V01 0x0084
#define QMI_SWI_M2M_WWAKE_READ_RESP_V01 0x0084
#define QMI_SWI_M2M_LPM_MODE_READ_REQ_V01 0x0085
#define QMI_SWI_M2M_LPM_MODE_READ_RESP_V01 0x0085
#define QMI_SWI_M2M_LPM_MODE_CFG_REQ_V01 0x0086
#define QMI_SWI_M2M_LPM_MODE_CFG_RESP_V01 0x0086
#define QMI_SWI_M2M_OVERTEMP_MODE_CFG_REQ_V01 0x0090
#define QMI_SWI_M2M_OVERTEMP_MODE_CFG_RESP_V01 0x0090
#define QMI_SWI_M2M_OVERTEMP_MODE_READ_REQ_V01 0x0091
#define QMI_SWI_M2M_OVERTEMP_MODE_READ_RESP_V01 0x0091
#define QMI_SWI_M2M_AVMS_SESSION_START_REQ_V01 0x00A0
#define QMI_SWI_M2M_AVMS_SESSION_START_RESP_V01 0x00A0
#define QMI_SWI_M2M_AVMS_SESSION_STOP_REQ_V01 0x00A1
#define QMI_SWI_M2M_AVMS_SESSION_STOP_RESP_V01 0x00A1
#define QMI_SWI_M2M_AVMS_SESSION_GETINFO_REQ_V01 0x00A2
#define QMI_SWI_M2M_AVMS_SESSION_GETINFO_RESP_V01 0x00A2
#define QMI_SWI_M2M_AVMS_EVENT_IND_V01 0x00A3
#define QMI_SWI_M2M_AVMS_SELECTION_REQ_V01 0x00A4
#define QMI_SWI_M2M_AVMS_SELECTION_RESP_V01 0x00A4
#define QMI_SWI_M2M_AVMS_SET_SETTINGS_REQ_V01 0x00A5
#define QMI_SWI_M2M_AVMS_SET_SETTINGS_RESP_V01 0x00A5
#define QMI_SWI_M2M_AVMS_GET_SETTINGS_REQ_V01 0x00A6
#define QMI_SWI_M2M_AVMS_GET_SETTINGS_RESP_V01 0x00A6
#define QMI_SWI_M2M_AVMS_SET_EVENT_REPORT_REQ_V01 0x00A7
#define QMI_SWI_M2M_AVMS_SET_EVENT_REPORT_RESP_V01 0x00A7
#define QMI_SWI_M2M_AVMS_GET_WAMS_REQ_V01 0x00A8
#define QMI_SWI_M2M_AVMS_GET_WAMS_RESP_V01 0x00A8
#define QMI_SWI_M2M_AVMS_OPEN_NONCE_REQ_V01 0x00A9
#define QMI_SWI_M2M_AVMS_OPEN_NONCE_RESP_V01 0x00A9
#define QMI_SWI_M2M_AVMS_CLOSE_NONCE_REQ_V01 0x00AA
#define QMI_SWI_M2M_AVMS_CLOSE_NONCE_RESP_V01 0x00AA
#define QMI_SWI_M2M_SIM_REFRESH_REQ_V01 0x00B0
#define QMI_SWI_M2M_SIM_REFRESH_RESP_V01 0x00B0
#define QMI_SWI_M2M_GPS_SET_GPSONLY_REQ_V01 0x00C0
#define QMI_SWI_M2M_GPS_SET_GPSONLY_RESP_V01 0x00C0
#define QMI_SWI_M2M_GPS_GET_GPSONLY_REQ_V01 0x00C1
#define QMI_SWI_M2M_GPS_GET_GPSONLY_RESP_V01 0x00C1
#define QMI_SWI_M2M_GPS_SET_XTRA_DATA_ENABLE_REQ_V01 0x00C2
#define QMI_SWI_M2M_GPS_SET_XTRA_DATA_ENABLE_RESP_V01 0x00C2
#define QMI_SWI_M2M_GPS_GET_XTRA_DATA_ENABLE_REQ_V01 0x00C3
#define QMI_SWI_M2M_GPS_GET_XTRA_DATA_ENABLE_RESP_V01 0x00C3
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor 
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro qapi_m2m_general_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type qapi_m2m_general_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );
 
/** This macro should be used to get the service object */ 
#define qapi_m2m_general_get_service_object_v01( ) \
          qapi_m2m_general_get_service_object_internal_v01( \
            QAPI_M2M_GENERAL_V01_IDL_MAJOR_VERS, QAPI_M2M_GENERAL_V01_IDL_MINOR_VERS, \
            QAPI_M2M_GENERAL_V01_IDL_TOOL_VERS )
/** 
    @} 
  */


#ifdef __cplusplus
}
#endif
#endif

