#ifndef SIERRA_M2M_AUDIO_API_SERVICE_01_H
#define SIERRA_M2M_AUDIO_API_SERVICE_01_H
/**
  @file swiqmi_audio_v01.h

  @brief This is the public header file which defines the sierra_m2m_audio_api service Data structures.

  This header file defines the types and structures that were defined in
  sierra_m2m_audio_api. It contains the constant values defined, enums, structures,
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
   It was generated on: Tue Mar 28 2017 (Spin 0)
   From IDL File: swiqmi_audio_v01.idl */

/** @defgroup sierra_m2m_audio_api_qmi_consts Constant values defined in the IDL */
/** @defgroup sierra_m2m_audio_api_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup sierra_m2m_audio_api_qmi_enums Enumerated types used in QMI messages */
/** @defgroup sierra_m2m_audio_api_qmi_messages Structures sent as QMI messages */
/** @defgroup sierra_m2m_audio_api_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup sierra_m2m_audio_api_qmi_accessor Accessor for QMI service object */
/** @defgroup sierra_m2m_audio_api_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "qapi_swi_common_v01.h"
#include "common_v01.h"
#include "qapi_common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup sierra_m2m_audio_api_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define SIERRA_M2M_AUDIO_API_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define SIERRA_M2M_AUDIO_API_V01_IDL_MINOR_VERS 0x02
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define SIERRA_M2M_AUDIO_API_V01_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
#define SIERRA_M2M_AUDIO_API_V01_MAX_MESSAGE_ID 0x0054
/**
    @}
  */


/** @addtogroup sierra_m2m_audio_api_qmi_consts 
    @{ 
  */
#define FILENAME_MAX_SIZE_V01 64

/**  Maximum size for audio ring file name and path */
#define AUDIO_PLAY_IND_MAX_SIZE_V01 8
#define QMI_SWI_M2M_IFACE_TABLE_SIZE_V01 9
#define QMI_SWI_M2M_PROFILE_MAX_SIZE_V01 10
#define QMI_SWI_M2M_PROFILE_MIN_SIZE_V01 6
#define QMI_SWI_M2M_PROFILE_EXT_SIZE_V01 4
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; This message get the profile content */
typedef struct {

  /* Optional */
  /*  generator */
  uint8_t generator_valid;  /**< Must be set to true if generator is being passed */
  uint8_t generator;
  /**<    0 – voice */
}swi_m2m_audio_get_profile_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; This message get the profile content */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */

  /* Mandatory */
  /*  Audio profile */
  uint8_t profile;
  /**<    0-5 */

  /* Mandatory */
  /*  Ear mute */
  uint8_t earmute;
  /**<   0 – mute
                                1 – unmute
                           */

  /* Mandatory */
  /*  MIC mute */
  uint8_t micmute;
  /**<   0 – mute
                                1 – unmute
                           */

  /* Mandatory */
  /*  generator */
  uint8_t generator;
  /**<    0 – voice */

  /* Mandatory */
  /*  RX volume level */
  uint8_t volume;
  /**<    0 – 5 */

  /* Mandatory */
  /*  Call waiting tone mute */
  uint8_t cwtmute;
  /**<    0 – mute
                                 1 – unmute
                           */
}swi_m2m_audio_get_profile_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; This message set the profile content */
typedef struct {

  /* Mandatory */
  /*  Audio profile */
  uint8_t profile;
  /**<    0-5 */

  /* Optional */
  /*  Ear mute */
  uint8_t earmute_valid;  /**< Must be set to true if earmute is being passed */
  uint8_t earmute;
  /**<   0 – mute
                                1 – unmute
                           */

  /* Optional */
  /*  MIC mute */
  uint8_t micmute_valid;  /**< Must be set to true if micmute is being passed */
  uint8_t micmute;
  /**<   0 – mute
                                1 – unmute
                           */

  /* Optional */
  /*  generator */
  uint8_t generator_valid;  /**< Must be set to true if generator is being passed */
  uint8_t generator;
  /**<    0 – voice */

  /* Optional */
  /*  RX volume level */
  uint8_t volume_valid;  /**< Must be set to true if volume is being passed */
  uint8_t volume;
  /**<    0 – 5 */

  /* Optional */
  /*  Call waiting tone mute */
  uint8_t cwtmute_valid;  /**< Must be set to true if cwtmute is being passed */
  uint8_t cwtmute;
  /**<    0 – mute
                                 1 – unmute
                           */
}swi_m2m_audio_set_profile_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; This message set the profile content */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}swi_m2m_audio_set_profile_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; This message get the RX volume level */
typedef struct {

  /* Mandatory */
  /*  Audio profile */
  uint8_t profile;
  /**<    0-5 */

  /* Mandatory */
  /*  generator */
  uint8_t generator;
  /**<    0 – voice */
}swi_m2m_audio_get_volume_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; This message get the RX volume level */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Mandatory */
  /*  The RX Volume level */
  uint8_t level;
  /**<   volume level */
}swi_m2m_audio_get_volume_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; This message set the RX volume level */
typedef struct {

  /* Mandatory */
  /*  Audio profile */
  uint8_t profile;
  /**<    0-5 */

  /* Mandatory */
  /*  generator */
  uint8_t generator;
  /**<    0 – voice */

  /* Mandatory */
  /*  Audio volume level */
  uint8_t level;
  /**<    0-5 */
}swi_m2m_audio_set_volume_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; This message set the RX volume level */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}swi_m2m_audio_set_volume_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; This message get the RX volume of the specified volume level */
typedef struct {

  /* Mandatory */
  /*  Audio profile */
  uint8_t profile;
  /**<    0-5 */

  /* Mandatory */
  /*  generator */
  uint8_t generator;
  /**<    0 – voice */

  /* Mandatory */
  /*  Audio volume level */
  uint8_t level;
  /**<    0-5 */
}swi_m2m_audio_get_voldb_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; This message get the RX volume of the specified volume level */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Mandatory */
  /*  Volume value */
  uint16_t volume_value;
  /**<   Volume value */
}swi_m2m_audio_get_voldb_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; This message set the RX volume of the specified volume level */
typedef struct {

  /* Mandatory */
  /*  Audio profile */
  uint8_t profile;
  /**<    0-5 */

  /* Mandatory */
  /*  generator */
  uint8_t generator;
  /**<    0 – voice */

  /* Mandatory */
  /*  Audio volume level */
  uint8_t volume_level;
  /**<    0 – 5 */

  /* Mandatory */
  /*  Audio volume value */
  uint16_t volume_value;
  /**<    Value to be set to the volume table */
}swi_m2m_audio_set_voldb_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; This message set the RX volume of the specified volume level */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}swi_m2m_audio_set_voldb_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; This message get the side tone value */
typedef struct {

  /* Mandatory */
  /*  Audio profile */
  uint8_t profile;
  /**<    0-5 */
}swi_m2m_audio_get_stg_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; This message get the side tone value */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Mandatory */
  /*  Item value */
  uint16_t value;
  /**<   Value of side tone gain */
}swi_m2m_audio_get_stg_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; This message set the side tone value */
typedef struct {

  /* Mandatory */
  /*  Audio profile */
  uint8_t profile;
  /**<    0-5 */

  /* Mandatory */
  /*  Item value */
  uint16_t value;
  /**<   Value of side tone gain */
}swi_m2m_audio_set_stg_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; This message set the side tone value */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}swi_m2m_audio_set_stg_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; This message set an audio loopback */
typedef struct {

  /* Mandatory */
  /*  operation */
  uint8_t enable;
  /**<   0 – stop
                                1 – vocoder
                                2 – codec
                           */
}swi_m2m_audio_set_lpbk_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; This message set an audio loopback */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}swi_m2m_audio_set_lpbk_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; This message play an audio file */
typedef struct {

  /* Mandatory */
  /*  generator */
  uint8_t generator;
  /**<   0 - voice */

  /* Mandatory */
  /*  audio tone */
  uint8_t tone;
  /**<   0x00-0x57 tone value */

  /* Optional */
  /*  Tone duration */
  uint8_t duration_valid;  /**< Must be set to true if duration is being passed */
  uint16_t duration;
  /**<   0-65535  */
}swi_m2m_audio_play_tone_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; This message play an audio file */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}swi_m2m_audio_play_tone_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; This message play an audio file */
typedef struct {

  /* Mandatory */
  /*  operation */
  uint8_t playback;
  /**<   0 – stop
                                 1 – start
                            */

  /* Mandatory */
  /*  File index */
  uint8_t index;
  /**<   0 – 15 */
}swi_m2m_audio_play_file_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; This message play an audio file */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}swi_m2m_audio_play_file_resp_msg_v01;  /* Message */
/**
    @}
  */
/** @addtogroup sierra_m2m_audio_api_qmi_messages
	@{
  */
/** Request Message; This message set all audio NV to default value */
typedef struct {

  /* Optional */
  /*  Audio Profile */
  uint8_t profile_valid;  /**< Must be set to true if profile is being passed */
  uint8_t profile;
  /**<	 0-9  */
}swi_m2m_audio_nv_def_req_msg_v01;	/* Message */
/**
	@}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; This message set all audio NV to default value */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}swi_m2m_audio_nv_def_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t audio_play_status;
  /**<   Refer to audio status table.*/

  uint8_t audio_play_file[FILENAME_MAX_SIZE_V01];
  /**<   Path and filename of the audio file to play.*/

  uint8_t audio_record_status;
  /**<   Refer to audio status table  */

  uint8_t audio_record_file[FILENAME_MAX_SIZE_V01];
  /**<   Path and filename of the recorded audio file.   */
}audio_play_and_record_status_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Indication Message; This message reported to the application when there is a change in an audio playing or audio recording state. */
typedef struct {

  /* Optional */
  /*  Audio play and record status */
  uint8_t audio_play_and_record_status_valid;  /**< Must be set to true if audio_play_and_record_status is being passed */
  uint32_t audio_play_and_record_status_len;  /**< Must be set to # of elements in audio_play_and_record_status */
  audio_play_and_record_status_type_v01 audio_play_and_record_status[AUDIO_PLAY_IND_MAX_SIZE_V01];
  /**<   Audio playing and recording status */
}swi_m2m_audio_play_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; This message query the TX volume. */
typedef struct {

  /* Mandatory */
  /*  Audio profile */
  uint8_t profile;
  /**<    0-5 */
}swi_m2m_audio_get_txvol_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; This message query the TX volume. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Mandatory */
  /*  TX volume */
  uint16_t value;
  /**<    Value of TX volume */
}swi_m2m_audio_get_txvol_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; This message set the TX volume. */
typedef struct {

  /* Mandatory */
  /*  Audio profile */
  uint8_t profile;
  /**<    0-5 */

  /* Mandatory */
  /*  TX volume */
  uint16_t value;
  /**<    Value of TX volume */
}swi_m2m_audio_set_txvol_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; This message set the TX volume. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}swi_m2m_audio_set_txvol_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; This message query the TX microphone gain */
typedef struct {

  /* Mandatory */
  /*  Audio profile */
  uint8_t profile;
  /**<    0-5 */
}swi_m2m_audio_get_micgain_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; This message query the TX microphone gain */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Mandatory */
  /*  TX microphone gain */
  uint16_t value;
  /**<    Value of mic gain */
}swi_m2m_audio_get_micgain_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; This message query the TX microphone gain */
typedef struct {

  /* Mandatory */
  /*  Audio profile */
  uint8_t profile;
  /**<    0-5 */

  /* Mandatory */
  /*  TX microphone gain */
  uint16_t value;
  /**<    Value of mic gain */
}swi_m2m_audio_set_micgain_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; This message query the TX microphone gain */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}swi_m2m_audio_set_micgain_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; This message query the RX speakerphone gain */
typedef struct {

  /* Mandatory */
  /*  Audio profile */
  uint8_t profile;
  /**<    0-5 */
}swi_m2m_audio_get_spkrgain_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; This message query the RX speakerphone gain */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Mandatory */
  /*  RX speakerphone gain */
  uint16_t value;
  /**<    Value of speaker  gain */
}swi_m2m_audio_get_spkrgain_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; This message set the RX speakerphone gain */
typedef struct {

  /* Mandatory */
  /*  Audio profile */
  uint8_t profile;
  /**<    0-5 */

  /* Mandatory */
  /*  RX speakerphone gain */
  uint16_t value;
  /**<    Value of speaker  gain */
}swi_m2m_audio_set_spkrgain_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; This message set the RX speakerphone gain */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}swi_m2m_audio_set_spkrgain_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; This message query the TX PCM IIR filter value */
typedef struct {

  /* Mandatory */
  /*  Audio profile */
  uint8_t profile;
  /**<    0-5 */

  /* Mandatory */
  /*  Switch or stage */
  uint8_t value;
  /**<    0 – query switch and stages
                               1 to stages - query parameters of one stage
                         */
}swi_m2m_audio_get_txpcmiir_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t B0;
  /**<  
                       Value of b0
                       */

  uint32_t B1;
  /**<  
                       Value of b1
                       */

  uint32_t B2;
  /**<  
                       Value of b2
                       */

  uint32_t A1;
  /**<  
                       Value of a1
                       */

  uint32_t A2;
  /**<  
                       Value of a2
                       */
}coefficients_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; This message query the TX PCM IIR filter value */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  switch */
  uint8_t switch_op_valid;  /**< Must be set to true if switch_op is being passed */
  uint8_t switch_op;
  /**<    0 – disable
                                  1 – enable
                            */

  /* Optional */
  /*  stages */
  uint8_t stages_valid;  /**< Must be set to true if stages is being passed */
  uint8_t stages;
  /**<   Number of stages */

  /* Optional */
  /*  Coefficients */
  uint8_t coefficients_valid;  /**< Must be set to true if coefficients is being passed */
  coefficients_type_v01 coefficients;
  /**<   Coefficients */
}swi_m2m_audio_get_txpcmiir_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t profile;
  /**<  
                       0-5
                       */

  uint8_t flag;
  /**<  
                       0 – disable
                       1 – enable
                       */

  uint8_t stages;
  /**<  
                       1 - 10
                       */
}configure_switch_and_stages_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t profile;
  /**<  
                       0-5
                       */

  uint8_t stages;
  /**<  
                       1 - 10
                       */

  uint32_t B0;
  /**<  
                       0x0 – 0xffffffff
                       */

  uint32_t B1;
  /**<  
                       0x0 – 0xffffffff
                       */

  uint32_t B2;
  /**<  
                       0x0 – 0xffffffff
                       */

  uint32_t A1;
  /**<  
                       0x0 – 0xffffffff
                       */

  uint32_t A2;
  /**<  
                       0x0 – 0xffffffff
                       */
}configure_coefficients_stage_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; This message set the RX encoder gain. */
typedef struct {

  /* Optional */
  /*  Configure switch and stages */
  uint8_t configure_switch_and_stages_valid;  /**< Must be set to true if configure_switch_and_stages is being passed */
  configure_switch_and_stages_type_v01 configure_switch_and_stages;

  /* Optional */
  /*  Configure coefficients of one stage */
  uint8_t configure_coefficients_stage_valid;  /**< Must be set to true if configure_coefficients_stage is being passed */
  configure_coefficients_stage_type_v01 configure_coefficients_stage;
}swi_m2m_audio_set_txpcmiir_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; This message set the RX encoder gain. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}swi_m2m_audio_set_txpcmiir_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; This message query the RX PCM IIR filter value */
typedef struct {

  /* Mandatory */
  /*  Audio profile */
  uint8_t profile;
  /**<    0-5 */

  /* Mandatory */
  /*  Switch or stage */
  uint8_t value;
  /**<    0 – query switch and stages
                               1 to stages - query parameters of one stage
                         */
}swi_m2m_audio_get_rxpcmiir_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; This message query the RX PCM IIR filter value */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  switch */
  uint8_t switch_op_valid;  /**< Must be set to true if switch_op is being passed */
  uint8_t switch_op;
  /**<    0 – disable
                                  1 – enable
                            */

  /* Optional */
  /*  stages */
  uint8_t stages_valid;  /**< Must be set to true if stages is being passed */
  uint8_t stages;
  /**<   Number of stages */

  /* Optional */
  /*  Coefficients */
  uint8_t coefficients_valid;  /**< Must be set to true if coefficients is being passed */
  coefficients_type_v01 coefficients;
  /**<   Coefficients */
}swi_m2m_audio_get_rxpcmiir_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; This message set the RX PCM IIR filter */
typedef struct {

  /* Optional */
  /*  Configure switch and stages */
  uint8_t configure_switch_and_stages_valid;  /**< Must be set to true if configure_switch_and_stages is being passed */
  configure_switch_and_stages_type_v01 configure_switch_and_stages;

  /* Optional */
  /*  Configure coefficients of one stage */
  uint8_t configure_coefficients_stage_valid;  /**< Must be set to true if configure_coefficients_stage is being passed */
  configure_coefficients_stage_type_v01 configure_coefficients_stage;
}swi_m2m_audio_set_rxpcmiir_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; This message set the RX PCM IIR filter */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}swi_m2m_audio_set_rxpcmiir_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_m2m_audio_get_ldtmfen_req_msg_v01;

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; This message get local DTMF enable value */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Mandatory */
  /*  Local DTMF enable */
  uint8_t switch_op;
  /**<   0 – disable
                                  1 – enable
                             */
}swi_m2m_audio_get_ldtmfen_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; This message get local DTMF enable value */
typedef struct {

  /* Mandatory */
  /*  Local DTMF enable */
  uint8_t switch_op;
  /**<   0 – disable
                                  1 – enable
                             */
}swi_m2m_audio_set_ldtmfen_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; This message get local DTMF enable value */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}swi_m2m_audio_set_ldtmfen_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_m2m_audio_get_avcfg_req_msg_v01;

/** @addtogroup sierra_m2m_audio_api_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t profile;
  /**<  
    0-5
  */

  uint8_t device;
  /**<  
    Refer to ACDB device table
  */

  uint8_t p_iface_id;
  /**<  
    Refer to PIFACE table
  */

  uint8_t iface_table[QMI_SWI_M2M_IFACE_TABLE_SIZE_V01];
  /**<  
    Refer to PIFACE table
  */
}swi_m2m_bind_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t profile;
  /**<  
    0-9 for 9x28
  */

  uint8_t tx_p_iface_id;
  /**<  
    Refer to PIFACE table
  */

  uint8_t tx_iface_table[QMI_SWI_M2M_IFACE_TABLE_SIZE_V01];
  /**<  
    Refer to PIFACE table
  */
}swi_m2m_bind_info_ext_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; The message requests to get audio bind information. It is similar with AT!AVCFG?. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  bind_info */
  swi_m2m_bind_info_type_v01 bind_info[QMI_SWI_M2M_PROFILE_MIN_SIZE_V01];

  /* Optional */
  /*  bind_info */
  uint8_t bind_info_ext_valid;  /**< Must be set to true if bind_info_ext is being passed */
  uint32_t bind_info_ext_len;  /**< Must be set to # of elements in bind_info_ext */
  swi_m2m_bind_info_type_v01 bind_info_ext[QMI_SWI_M2M_PROFILE_EXT_SIZE_V01];

  /* Optional */
  /*  bind_info_ext */
  uint8_t bind_info_ext2_valid;  /**< Must be set to true if bind_info_ext2 is being passed */
  uint32_t bind_info_ext2_len;  /**< Must be set to # of elements in bind_info_ext2 */
  swi_m2m_bind_info_ext_type_v01 bind_info_ext2[QMI_SWI_M2M_PROFILE_MAX_SIZE_V01];
}swi_m2m_audio_get_avcfg_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; The message requests to bind audio profile with ACDB device and physical interface, configure that interface. It is similar with AT!AVCFG=<profile>,<device>,<PIFACE>[,<param1>[,<paramN>]]. */
typedef struct {

  /* Mandatory */
  /*  profile */
  uint8_t profile;
  /**<  
    0-9
  */

  /* Mandatory */
  /*  device */
  uint8_t device;
  /**<  
    Refer to ACDB device table
  */

  /* Mandatory */
  /*  piface_id */
  uint8_t piface_id;
  /**<  
    Refer to ACDB device table
  */

  /* Optional */
  /*  iface_table */
  uint8_t iface_table_valid;  /**< Must be set to true if iface_table is being passed */
  uint32_t iface_table_len;  /**< Must be set to # of elements in iface_table */
  uint8_t iface_table[QMI_SWI_M2M_IFACE_TABLE_SIZE_V01];
  /**<  
    Refer to PIFACE table
  */

  /* Optional */
  /*  tx_piface_id */
  uint8_t tx_piface_id_valid;  /**< Must be set to true if tx_piface_id is being passed */
  uint8_t tx_piface_id;
  /**<  
    Refer to PIFACE table
  */

  /* Optional */
  /*  tx_iface_table */
  uint8_t tx_iface_table_valid;  /**< Must be set to true if tx_iface_table is being passed */
  uint32_t tx_iface_table_len;  /**< Must be set to # of elements in tx_iface_table */
  uint8_t tx_iface_table[QMI_SWI_M2M_IFACE_TABLE_SIZE_V01];
  /**<  
    Refer to PIFACE table
  */
}swi_m2m_audio_set_avcfg_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; The message requests to bind audio profile with ACDB device and physical interface, configure that interface. It is similar with AT!AVCFG=<profile>,<device>,<PIFACE>[,<param1>[,<paramN>]]. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}swi_m2m_audio_set_avcfg_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; This message tests basic message passing between
  client and service */
typedef struct {

  /* Mandatory */
  /*  Audio profile */
  uint8_t profile;
  /**<    0-5 */
}swi_m2m_audio_get_avmute_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; This message tests basic message passing between
  client and service */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Mandatory */
  /*  Ear mute */
  uint8_t earmute;
  /**<   0 – mute
                                1 – unmute
                           */

  /* Mandatory */
  /*  MIC mute */
  uint8_t micmute;
  /**<   0 – mute
                                1 – unmute
                           */

  /* Mandatory */
  /*  Waiting tone mute */
  uint8_t cwtmute;
  /**<   0 – mute
                                1 – unmute
                           */
}swi_m2m_audio_get_avmute_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; This message tests basic message passing between
  client and service */
typedef struct {

  /* Mandatory */
  /*  Audio profile */
  uint8_t profile;
  /**<    0-5 */

  /* Mandatory */
  /*  Ear mute */
  uint8_t earmute;
  /**<   0 – mute
                                1 – unmute
                           */

  /* Mandatory */
  /*  MIC mute */
  uint8_t micmute;
  /**<   0 – mute
                                1 – unmute
                           */

  /* Optional */
  /*  Waiting tone mute */
  uint8_t cwtmute_valid;  /**< Must be set to true if cwtmute is being passed */
  uint8_t cwtmute;
  /**<   0 – mute
                                1 – unmute
                           */
}swi_m2m_audio_set_avmute_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; This message tests basic message passing between
  client and service */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}swi_m2m_audio_set_avmute_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; The message requests to query audio TX path encoder gain value */
typedef struct {

  /* Mandatory */
  /*  Audio profile */
  uint8_t profile;
  /**<    0-5 */
}swi_m2m_audio_get_avtxg_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; The message requests to query audio TX path encoder gain value */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Mandatory */
  /*  ENCODER gain */
  uint16_t gain;
  /**<    0 – 0xFFFF */
}swi_m2m_audio_get_avtxg_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; The message requests to set audio TX path encoder gain value. */
typedef struct {

  /* Mandatory */
  /*  Audio profile */
  uint8_t profile;
  /**<    0-5 */

  /* Mandatory */
  /*  ENCODER gain */
  uint16_t gain;
  /**<    0 – 0xFFFF */
}swi_m2m_audio_set_avtxg_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; The message requests to set audio TX path encoder gain value. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}swi_m2m_audio_set_avtxg_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; The message requests to query audio RX path decoder gain value */
typedef struct {

  /* Mandatory */
  /*  Audio profile */
  uint8_t profile;
  /**<    0-5 */
}swi_m2m_audio_get_avrxg_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; The message requests to query audio RX path decoder gain value */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Mandatory */
  /*  DECODER gain */
  uint16_t gain;
  /**<    0 – 0xFFFF */
}swi_m2m_audio_get_avrxg_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; The message requests to set audio RX path decoder gain value */
typedef struct {

  /* Mandatory */
  /*  Audio profile */
  uint8_t profile;
  /**<    0-5 */

  /* Mandatory */
  /*  DECODER gain */
  uint16_t gain;
  /**<    0 – 0xFFFF */
}swi_m2m_audio_set_avrxg_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; The message requests to set audio RX path decoder gain value */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}swi_m2m_audio_set_avrxg_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; The message requests to query audio TX path AGC enable switch value */
typedef struct {

  /* Mandatory */
  /*  Audio profile */
  uint8_t profile;
  /**<    0-5 */
}swi_m2m_audio_get_avtxagc_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; The message requests to query audio TX path AGC enable switch value */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Mandatory */
  /*  Enable/disable switch */
  uint8_t switch_on;
  /**<    0 – 1 */
}swi_m2m_audio_get_avtxagc_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; The message requests to set audio TX path AGC enable switch value */
typedef struct {

  /* Mandatory */
  /*  Audio profile */
  uint8_t profile;
  /**<    0-5 */

  /* Mandatory */
  /*  Enable/disable switch */
  uint8_t switch_on;
  /**<    0 – 1 */
}swi_m2m_audio_set_avtxagc_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; The message requests to set audio TX path AGC enable switch value */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}swi_m2m_audio_set_avtxagc_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; The message requests to query audio RX path AGC enable switch value */
typedef struct {

  /* Mandatory */
  /*  Audio profile */
  uint8_t profile;
  /**<    0-5 */
}swi_m2m_audio_get_avrxagc_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; The message requests to query audio RX path AGC enable switch value */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Mandatory */
  /*  Enable/disable switch */
  uint8_t switch_on;
  /**<    0 – 1 */
}swi_m2m_audio_get_avrxagc_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; The message requests to set audio RX path AGC enable switch value */
typedef struct {

  /* Mandatory */
  /*  Audio profile */
  uint8_t profile;
  /**<    0-5 */

  /* Mandatory */
  /*  Enable/disable switch */
  uint8_t switch_on;
  /**<    0 – 1 */
}swi_m2m_audio_set_avrxagc_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; The message requests to set audio RX path AGC enable switch value */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}swi_m2m_audio_set_avrxagc_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; The message requests to query audio RX path AVC enable switch value */
typedef struct {

  /* Mandatory */
  /*  Audio profile */
  uint8_t profile;
  /**<    0-5 */
}swi_m2m_audio_get_avrxavc_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; The message requests to query audio RX path AVC enable switch value */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Mandatory */
  /*  Enable/disable switch */
  uint8_t switch_on;
  /**<    0 – 1 */
}swi_m2m_audio_get_avrxavc_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; The message requests to set audio RX path AVC enable switch value */
typedef struct {

  /* Mandatory */
  /*  Audio profile */
  uint8_t profile;
  /**<    0-5 */

  /* Mandatory */
  /*  Enable/disable switch */
  uint8_t switch_on;
  /**<    0 – 1 */
}swi_m2m_audio_set_avrxavc_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; The message requests to set audio RX path AVC enable switch value */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}swi_m2m_audio_set_avrxavc_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; The message requests to query audio echo canceller enable mode */
typedef struct {

  /* Mandatory */
  /*  Audio profile */
  uint8_t profile;
  /**<    0-5 */
}swi_m2m_audio_get_avec_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; The message requests to query audio echo canceller enable mode */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Mandatory */
  /*  Enable/disable switch */
  uint8_t switch_on;
  /**<    0 – 1 */
}swi_m2m_audio_get_avec_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; The message requests to set audio echo canceller enable mode */
typedef struct {

  /* Mandatory */
  /*  Audio profile */
  uint8_t profile;
  /**<    0-5 */

  /* Mandatory */
  /*  Enable/disable switch */
  uint8_t switch_on;
  /**<    0 – 1 */
}swi_m2m_audio_set_avec_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; The message requests to set audio echo canceller enable mode */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}swi_m2m_audio_set_avec_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; This message requests to query audio noise suppression enable mode. It is similar with AT!AVNS?<profile>. */
typedef struct {

  /* Mandatory */
  /*  Audio profile */
  uint8_t profile;
  /**<  
    0 - 5
  */
}swi_m2m_audio_get_avns_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; This message requests to query audio noise suppression enable mode. It is similar with AT!AVNS?<profile>. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  NS */
  uint8_t NS;
  /**<  
    0 ¨C disable
    1 ¨C enable
  */

  /* Mandatory */
  /*  FNS */
  uint8_t FNS;
  /**<  
    0 ¨C disable
    1 ¨C enable
  */
}swi_m2m_audio_get_avns_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; This message requests to set audio noise suppression enable mode. It is similar with AT!AVNS=<profile>, <NS>[,<FNS>] */
typedef struct {

  /* Mandatory */
  /*  Profile */
  uint8_t profile;
  /**<  
    0 - 5
  */

  /* Mandatory */
  /*  NS */
  uint8_t NS;
  /**<  
    0 ¨C disable
    1 ¨C enable
  */

  /* Optional */
  /*  FNS */
  uint8_t FNS_valid;  /**< Must be set to true if FNS is being passed */
  uint8_t FNS;
  /**<  
    0 ¨C disable
    1 ¨C enable
  */
}swi_m2m_audio_set_avns_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; This message requests to set audio noise suppression enable mode. It is similar with AT!AVNS=<profile>, <NS>[,<FNS>] */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}swi_m2m_audio_set_avns_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; The message requests to query audio internal codec TX path overall gain value */
typedef struct {

  /* Mandatory */
  /*  Audio profile */
  uint8_t profile;
  /**<    0-5 */
}swi_m2m_audio_get_avcodecmictxg_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; The message requests to query audio internal codec TX path overall gain value */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Mandatory */
  /*  Gain value */
  uint16_t gain;
  /**<    0 – 0xFFFF */
}swi_m2m_audio_get_avcodecmictxg_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; The message requests to set audio noise suppression enable mode */
typedef struct {

  /* Mandatory */
  /*  Audio profile */
  uint8_t profile;
  /**<    0-5 */

  /* Mandatory */
  /*  Gain value */
  uint16_t gain;
  /**<    0 – 0xFFFF */
}swi_m2m_audio_set_avcodecmictxg_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; The message requests to set audio noise suppression enable mode */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}swi_m2m_audio_set_avcodecmictxg_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; The message requests to set audio noise suppression enable mode */
typedef struct {

  /* Mandatory */
  /*  Audio profile */
  uint8_t profile;
  /**<    0-5 */

  /* Mandatory */
  /*  Filter enum */
  uint8_t fltr_enum;
  /**<    0 – 3 */
}swi_m2m_audio_get_avfltren_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; The message requests to set audio noise suppression enable mode */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Mandatory */
  /*  Gain value */
  uint8_t enable;
  /**<     0 – disable
                                 1 – enable
                          */
}swi_m2m_audio_get_avfltren_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; The message requests to set audio noise suppression enable mode */
typedef struct {

  /* Mandatory */
  /*  Audio profile */
  uint8_t profile;
  /**<    0-5 */

  /* Mandatory */
  /*  Filter enum */
  uint8_t fltr_enum;
  /**<    0 – 3 */

  /* Mandatory */
  /*  Filter enum */
  uint8_t enable;
  /**<     0 – disable
                                 1 – enable
                          */
}swi_m2m_audio_set_avfltren_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; The message requests to set audio noise suppression enable mode */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}swi_m2m_audio_set_avfltren_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_m2m_audio_get_wddm_req_msg_v01;

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; The message requests to query DTMF detecting enable value. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Mandatory */
  /*  Audio enum */
  uint8_t enable;
  /**<     0 - disable
                                 1 - enable
                          */
}swi_m2m_audio_get_wddm_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; The message requests to set DTMF detecting enable value. */
typedef struct {

  /* Mandatory */
  /*  Audio WDDM */
  uint8_t enable;
  /**<     0 - disable
                                 1 - enable
                          */
}swi_m2m_audio_set_wddm_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; The message requests to set DTMF detecting enable value. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}swi_m2m_audio_set_wddm_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Indication Message; This unsolicited message is reported to the application when there is a DTMF sent from far end. */
typedef struct {

  /* Mandatory */
  /*  Unicast (per control point) */
  char DTMF_Value;
  /**<     0-9,*,#,A-D
                              */
}swi_m2m_audio_dtmf_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; The message requests to set play audio file/record voice locally
	or play local file to far-end/record voice call during call. */
typedef struct {

  /* Mandatory */
  /*  Audio Operation value */
  uint8_t operation;
  /**<     1 - audio play
                                    2 - voice record
                                    3 - WWAN audio play
                                    4 - WWAN audio record
                             */

  /* Mandatory */
  /*  Audio Switch value */
  uint8_t swith;
  /**<         0 - stop
                                    1 - start
                          */

  /* Optional */
  /*  Refer to PIFACE table */
  uint8_t file_path_valid;  /**< Must be set to true if file_path is being passed */
  uint32_t file_path_len;  /**< Must be set to # of elements in file_path */
  char file_path[50];
}swi_m2m_audio_set_avaudio_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; The message requests to set play audio file/record voice locally
	or play local file to far-end/record voice call during call. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}swi_m2m_audio_set_avaudio_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; The message requests to query TTY mode. */
typedef struct {

  /* Mandatory */
  /*  Audio profile */
  uint8_t profile;
  /**<    0-5 */
}swi_m2m_audio_get_tty_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; The message requests to query TTY mode. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Mandatory */
  /*  TTY Mode */
  uint8_t mode;
  /**<     0 - FULL
                               1 - VCO
                               2 - HCO
                          */
}swi_m2m_audio_get_tty_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; The message requests to set TTY mode. */
typedef struct {

  /* Mandatory */
  /*  Audio profile */
  uint8_t profile;
  /**<    0-5 */

  /* Mandatory */
  /*  TTY Mode */
  uint8_t mode;
  /**<     0 - FULL
                               1 - VCO
                               2 - HCO
                          */
}swi_m2m_audio_set_tty_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; The message requests to set TTY mode. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}swi_m2m_audio_set_tty_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_m2m_audio_get_avaudvol_req_msg_v01;

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; The message requests to query multimedia volume value. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Mandatory */
  /*  Volume value */
  uint16_t volume;
  /**<  
                           0 - FFFF
                           */
}swi_m2m_audio_get_avaudvol_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; The message requests to set multimedia volume value. */
typedef struct {

  /* Mandatory */
  /*  Volume value */
  uint16_t volume;
  /**<  
                           0 - FFFF
                           */
}swi_m2m_audio_set_avaudvol_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; The message requests to set multimedia volume value. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}swi_m2m_audio_set_avaudvol_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_m2m_audio_get_avmmute_req_msg_v01;

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; This message requests to query audio Multimedia mute configuration. It is similar with AT!AVMMUTE?. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  mmute */
  uint8_t mmute;
  /**<  
    0 ¨C unmute
    1 ¨C mute
  */
}swi_m2m_audio_get_avmmute_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; This message requests to set audio Multimedia mute configuration. It is similar with AT!AVMMUTE=<mmute>. */
typedef struct {

  /* Mandatory */
  /*  mmute */
  uint8_t mmute;
  /**<  
    0 ¨C unmute
    1 ¨C mute
  */
}swi_m2m_audio_set_avmmute_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; This message requests to set audio Multimedia mute configuration. It is similar with AT!AVMMUTE=<mmute>. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}swi_m2m_audio_set_avmmute_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_m2m_audio_get_avmminuse_req_msg_v01;

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; This message requests to query audio Multimedia state. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  mm_inuse */
  uint8_t mm_inuse;
  /**<  
    0 ¡V mm idle
    1 ¡V mm play
    2 ¡V mm record
    3 ¡V ww play
    4 ¡V ww record
  */
}swi_m2m_audio_get_avmminuse_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; This message requests to set audio Multimedia state. */
typedef struct {

  /* Mandatory */
  /*  mm_inuse */
  uint8_t mm_inuse;
  /**<  
    0 ¡V mm idle
    1 ¡V mm play
    2 ¡V mm record
    3 ¡V ww play
    4 ¡V ww record
  */
}swi_m2m_audio_set_avmminuse_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; This message requests to set audio Multimedia state. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}swi_m2m_audio_set_avmminuse_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_m2m_audio_get_lpbk_req_msg_v01;

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; The message requests to get audio loopback status. It is similar with at!avaudiolpbk?. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  enable */
  uint8_t enable;
  /**<  
    0 - Loopback disable
    1 - VOCODER loopback
    2 - Audio loopback - Not supported for MDM9x15
    3 - PCM loopback
    4 - Internal codec loopback
  */
}swi_m2m_audio_get_lpbk_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; This message requests to initialize WM8940 codec via I2C interface. */
typedef struct {

  /* Mandatory */
  /*  codec_type */
  uint8_t codec_type;
  /**<  
    0 - PCM slave mode(WM8994)
    1 - I2S slave mode(WM8994)
    2 - PCM master mode(WM8994)
    3 - I2S master mode(WM8994)
    4 - PCM slave mode(WM8940)
    5 - I2S slave mode(WM8940) - default MCLK
    6 - I2S slave mode (WM8940) - TCXO as MCLK
  */
}swi_m2m_audio_set_mft_codec_mode_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; This message requests to initialize WM8940 codec via I2C interface. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}swi_m2m_audio_set_mft_codec_mode_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; This message requests to query the various chipset dependent audio parameters of the module. */
typedef struct {

  /* Mandatory */
  /*  profile */
  uint8_t profile;
  /**<  
    0-9
  */

  /* Mandatory */
  /*  block */
  uint8_t block;
  /**<  
    0 - RX_AIG
    1 - RX_DRC
    2 - RX_FIR
    3 - RX_IIR
    4 - RX_VOL
    5 - RX_AVC_RVE
    6 - RX_DEC_GAIN
    7 - RX_ECRX
    8 - RX_MBDRC
    9 - TX_AIG
    10 - TX_DRC
    11 - TX_ENC_GAIN
    12 - TX_FIR
    13 - TX_IIR
    14 - TX_SMECNS
    15 - TX_VOL
  */

  /* Mandatory */
  /*  index */
  uint8_t index;
  /**<  
    variable for different block
  */

  /* Optional */
  /*  volume */
  uint8_t volume_valid;  /**< Must be set to true if volume is being passed */
  uint8_t volume;
  /**<  
    0 - 8 RX volume level
  */

  /* Optional */
  /*  bandwidth */
  uint8_t bandwidth_valid;  /**< Must be set to true if bandwidth is being passed */
  uint8_t bandwidth;
  /**<  
    0 - narrow_band
    1 - wide_band
  */
}swi_m2m_audio_get_csd_ap_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; This message requests to query the various chipset dependent audio parameters of the module. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  value */
  uint32_t value;
  /**<  
    0x00000000 - 0xFFFFFFFF
  */

  /* Mandatory */
  /*  val_size */
  uint8_t val_size;
  /**<  
    2 - characters
    4 - characters
    8 - characters
  */
}swi_m2m_audio_get_csd_ap_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; This message requests to tune the various chipset dependent audio parameters of the module. */
typedef struct {

  /* Mandatory */
  /*  profile */
  uint8_t profile;
  /**<  
    0-9
  */

  /* Mandatory */
  /*  block */
  uint8_t block;
  /**<  
    0 - RX_AIG
    1 - RX_DRC
    2 - RX_FIR
    3 - RX_IIR
    4 - RX_VOL
    5 - RX_AVC_RVE
    6 - RX_DEC_GAIN
    7 - RX_ECRX
    8 - RX_MBDRC
    9 - TX_AIG
    10 - TX_DRC
    11 - TX_ENC_GAIN
    12 - TX_FIR
    13 - TX_IIR
    14 - TX_SMECNS
    15 - TX_VOL
  */

  /* Mandatory */
  /*  index */
  uint8_t index;
  /**<  
    variable for different block
  */

  /* Mandatory */
  /*  value */
  uint32_t value;
  /**<  
    0x00000000 - 0xFFFFFFFF
  */

  /* Optional */
  /*  volume */
  uint8_t volume_valid;  /**< Must be set to true if volume is being passed */
  uint8_t volume;
  /**<  
    0 - 8 RX volume level
  */

  /* Optional */
  /*  bandwidth */
  uint8_t bandwidth_valid;  /**< Must be set to true if bandwidth is being passed */
  uint8_t bandwidth;
  /**<  
    0 - narrow_band
    1 - wide_band
  */
}swi_m2m_audio_set_csd_ap_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; This message requests to tune the various chipset dependent audio parameters of the module. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}swi_m2m_audio_set_csd_ap_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; The message requests to set configuration of the audio path to playback or record (local or WWAN). */
typedef struct {

  /* Mandatory */
  /*  backenddai */
  uint8_t backenddai;
  /**<  
    0 - PCM
    1 - I2S
    2 - Analog
    3 - USB
    4 - PCM_2
    5 - I2S_2
    6 - Ethernet
    7 - OTA
  */

  /* Mandatory */
  /*  operatetype */
  uint8_t operatetype;
  /**<  
    0 - PLAYBACK
    1 - RECORD
  */
}swi_m2m_audio_set_media_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; The message requests to set configuration of the audio path to playback or record (local or WWAN). */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  frontendhwid */
  uint8_t frontendhwid;
  /**<  
    0~254
    0xFF - Invalid
  */
}swi_m2m_audio_set_media_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; The message requests to add a different backend DAI onto the in-use frontend. */
typedef struct {

  /* Mandatory */
  /*  backenddai */
  uint8_t backenddai;
  /**<  
    0 - PCM
    1 - I2S
    2 - Analog
    3 - USB
    4 - PCM_2
    5 - I2S_2
    6 - Ethernet
    7 - OTA
  */

  /* Mandatory */
  /*  frontendhwid */
  uint8_t frontendhwid;
  /**<  
    0~254
    0xff - invalid
  */
}swi_m2m_audio_add_media_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; The message requests to add a different backend DAI onto the in-use frontend. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}swi_m2m_audio_add_media_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; The message requests to reset configuration of the audio path to playback or record (local or WWAN). */
typedef struct {

  /* Mandatory */
  /*  backenddai */
  uint8_t backenddai;
  /**<  
    0 - PCM
    1 - I2S
    2 - Analog
    3 - USB
    4 - PCM_2
    5 - I2S_2
    6 - Ethernet
    7 - OTA
  */

  /* Mandatory */
  /*  frontendhwid */
  uint8_t frontendhwid;
  /**<  
    0~254
    0xff - invalid
  */
}swi_m2m_audio_reset_media_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; The message requests to reset configuration of the audio path to playback or record (local or WWAN). */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}swi_m2m_audio_reset_media_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_m2m_audio_get_piface_config_req_msg_v01;

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; The message requests to get the configuration of audio physical interfaces. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  audio_piface_cfg */
  uint16_t audio_piface_cfg;
  /**<  
    Bit0 - 1:
    00 - not supported       11 - Digital audio 1 is supported
    Bit2:
    0 - not supported        1 - analog audio is supported
    Bit3:
    0 - not supported
    1 - USB audio is supported
    Bit4-5:
    00 - not supported       11 - Digital audio 2 is supported
    Bit6:
    0 - not supported Ethernet audio
    1 - Ethernet audio is supported
    Bit7-15:
    Reserved
  */
}swi_m2m_audio_get_piface_config_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_m2m_audio_get_piface_pref_req_msg_v01;

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; The message requests to get the preference setting of audio physical interfaces. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  audio_piface_pref */
  uint16_t audio_piface_pref;
  /**<  
    Bit0-1:
    00 - not enabled
    01 - PCM
    02 - I2S
    11 - Invalid
    Bit2:
    0 - not enabled
    1 - analog audio is enabled
    Bit3:
    0 - not enabled
    1 - USB audio is enabled
    Bit4-5:
    00 - not enabled
    01 - PCM
    02 - I2S
    11 - Invalid
    Bit6:
    0 - not enabled
    1 - Ethernet audio is enabled
    Bit7-15:
    Reserved
  */
}swi_m2m_audio_get_piface_pref_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; The message requests to set the preference setting of audio physical interfaces. */
typedef struct {

  /* Mandatory */
  /*  audio_piface_pref */
  uint16_t audio_piface_pref;
  /**<  
    Bit0 - 1:
    00 - not enabled
    01 - PCM
    02 - I2S
    11 - Invalid
    Bit2:
    0 - not enabled
    1 - analog audio is enabled
    Bit3:
    0 - not enabled
    1 - USB audio is enabled
    Bit4-5:
    00 - not enabled
    01 - PCM
    02 - I2S
    11 - Invalid
    Bit6:
    0 - not enabled
    1 - Ethernet audio is enabled
    Bit7-15:
    Reserved
  */
}swi_m2m_audio_set_piface_pref_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; The message requests to set the preference setting of audio physical interfaces. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}swi_m2m_audio_set_piface_pref_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_m2m_audio_get_clvl_req_msg_v01;

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; This message requests to get the RX Volume level for the active AV profile. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  level */
  uint8_t level;
  /**<  
    0 - 8 RX volume level
  */
}swi_m2m_audio_get_clvl_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; This message requests to set the RX Volume level for the active AV profile. */
typedef struct {

  /* Mandatory */
  /*  level */
  uint8_t level;
  /**<  
    0 - 8 RX volume level
  */
}swi_m2m_audio_set_clvl_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; This message requests to set the RX Volume level for the active AV profile. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}swi_m2m_audio_set_clvl_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; The message requests to set DTMF duration. */
typedef struct {

  /* Mandatory */
  /*  duration */
  uint8_t duration;
  /**<  
    0~255(*100ms)
  */
}swi_m2m_audio_set_vtd_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; The message requests to set DTMF duration. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}swi_m2m_audio_set_vtd_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_m2m_audio_get_vtd_req_msg_v01;

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; The message requests to get DTMF duration */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  duration */
  uint8_t duration;
  /**<  
    0~255(*100ms)
  */
}swi_m2m_audio_get_vtd_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; The message requests to start a DTMF. */
typedef struct {

  /* Mandatory */
  /*  dtmfchar */
  uint8_t dtmfchar;
  /**<  
    ' 0 ' - ' 9 '
    ' A ' - ' D '
    ' * ' and ' # '
  */
}swi_m2m_audio_set_vts_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; The message requests to start a DTMF. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}swi_m2m_audio_set_vts_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; The message requests to query audio codec RX path overall gain value. It is similar with AT!AVCODECRXG?<profile>. */
typedef struct {

  /* Mandatory */
  /*  profile */
  uint8_t profile;
  /**<  
    0-9
  */
}swi_m2m_audio_get_avcodecrxg_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; The message requests to query audio codec RX path overall gain value. It is similar with AT!AVCODECRXG?<profile>. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  gain */
  uint16_t gain;
  /**<  
    0 - 0xFFFF
  */
}swi_m2m_audio_get_avcodecrxg_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; The message requests to set audio codec RX path overall gain value. It is similar with AT!AVCODECRXG=<profile>,<gain>. */
typedef struct {

  /* Mandatory */
  /*  profile */
  uint8_t profile;
  /**<  
    0-9
  */

  /* Mandatory */
  /*  gain */
  uint16_t gain;
  /**<  
    0 - 0xFFFF
  */
}swi_m2m_audio_set_avcodecrxg_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; The message requests to set audio codec RX path overall gain value. It is similar with AT!AVCODECRXG=<profile>,<gain>. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}swi_m2m_audio_set_avcodecrxg_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; The message requests to query audio codec side-tone gain value. It is similar with AT!AVCODECSTG?<profile>. */
typedef struct {

  /* Mandatory */
  /*  profile */
  uint8_t profile;
  /**<  
    0-9
  */
}swi_m2m_audio_get_avcodecstg_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; The message requests to query audio codec side-tone gain value. It is similar with AT!AVCODECSTG?<profile>. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  gain */
  uint16_t gain;
  /**<  
    0 - 0xFFFF
  */
}swi_m2m_audio_get_avcodecstg_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; The message requests to set audio codec side-tone gain value. It is similar with AT!AVCODECSTG=<profile>,<gain>. */
typedef struct {

  /* Mandatory */
  /*  profile */
  uint8_t profile;
  /**<  
    0-9
  */

  /* Mandatory */
  /*  gain */
  uint16_t gain;
  /**<  
    0 - 0xFFFF
  */
}swi_m2m_audio_set_avcodecstg_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; The message requests to set audio codec side-tone gain value. It is similar with AT!AVCODECSTG=<profile>,<gain>. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}swi_m2m_audio_set_avcodecstg_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; The message requests to query audio codec register value. It is similar with AT!AVGETCODEC?<reg>. */
typedef struct {

  /* Mandatory */
  /*  reg */
  uint16_t reg;
  /**<  
    0-0xFFFF
  */
}swi_m2m_audio_get_avcodec_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; The message requests to query audio codec register value. It is similar with AT!AVGETCODEC?<reg>. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  value */
  uint16_t value;
  /**<  
    0 - 0xFFFF
  */
}swi_m2m_audio_get_avcodec_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; The message requests to set audio codec register value. It is similar with AT!AVSETCODEC=<reg>,<gain>. */
typedef struct {

  /* Mandatory */
  /*  reg */
  uint16_t reg;
  /**<  
    0-0xFFFF
  */

  /* Mandatory */
  /*  value */
  uint16_t value;
  /**<  
    0 - 0xFFFF
  */
}swi_m2m_audio_set_avcodec_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; The message requests to set audio codec register value. It is similar with AT!AVSETCODEC=<reg>,<gain>. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}swi_m2m_audio_set_avcodec_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_m2m_audio_get_cmute_req_msg_v01;

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; This message requests to get the mute/unmute state for for voice call on TX path. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  txmute */
  uint8_t txmute;
  /**<  
    0 - 1 unmute/mute
  */
}swi_m2m_audio_get_cmute_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Request Message; This message requests to mute/unmute TX path for voice call. */
typedef struct {

  /* Mandatory */
  /*  txmute */
  uint8_t txmute;
  /**<  
    0 - 1 unmute/mute
  */
}swi_m2m_audio_set_cmute_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
    @{
  */
/** Response Message; This message requests to mute/unmute TX path for voice call. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}swi_m2m_audio_set_cmute_resp_msg_v01;  /* Message */
/**
    @}
  */
typedef struct {
  /* This element is a placeholder to prevent the declaration of
	 an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_m2m_audio_get_ec_off_mask_req_msg_v01;

/** @addtogroup sierra_m2m_audio_api_qmi_messages
	@{
  */
/** Response Message; This message requests to get the value of EC_OFF mask pattern. */
typedef struct {

  /* Mandatory */
  /*  ec_off */
  uint16_t ec_off;
  /**<
	Static value of EC_OFF mask pattern
  */

  /* Mandatory */
  /*  Result Code */
  qapi_status_type_v01 resp;
}swi_m2m_audio_get_ec_off_mask_resp_msg_v01;  /* Message */
/**
	@}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
	@{
  */
/** Request Message; This message requests to set the value of EC_OFF mask pattern. */
typedef struct {

  /* Mandatory */
  /*  ec_off */
  uint16_t ec_off;
  /**<
	Static value of EC_OFF mask pattern
  */
}swi_m2m_audio_set_ec_off_mask_req_msg_v01;  /* Message */
/**
	@}
  */

/** @addtogroup sierra_m2m_audio_api_qmi_messages
	@{
  */
/** Response Message; This message requests to set the value of EC_OFF mask pattern. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qapi_status_type_v01 resp;
}swi_m2m_audio_set_ec_off_mask_resp_msg_v01;  /* Message */
/**
	@}
  */

/* Conditional compilation tags for message removal */ 
//#define REMOVE_QMI_SWI_M2M_AUDIO_ADD_MEDIA_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_DTMF_IND_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_GET_AVAUDVOL_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_GET_AVCFG_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_GET_AVCODEC_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_GET_AVCODECMICTXG_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_GET_AVCODECRXG_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_GET_AVCODECSTG_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_GET_AVEC_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_GET_AVFLTREN_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_GET_AVMMINUSE_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_GET_AVMMUTE_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_GET_AVMUTE_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_GET_AVNS_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_GET_AVRXAGC_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_GET_AVRXAVC_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_GET_AVRXG_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_GET_AVTXAGC_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_GET_AVTXG_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_GET_CLVL_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_GET_CMUTE_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_GET_CSD_AP_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_GET_LDTMFEN_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_GET_LPBK_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_GET_MICGAIN_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_GET_PIFACE_CONFIG_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_GET_PIFACE_PREF_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_GET_PROFILE_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_GET_RXPCMIIR_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_GET_SPKRGAIN_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_GET_STG_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_GET_TTY_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_GET_TXPCMIIR_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_GET_TXVOL_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_GET_VOLDB_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_GET_VOLUME_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_GET_VTD_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_GET_WDDM_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_NV_DEF_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_PLAY_FILE_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_PLAY_IND_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_PLAY_TONE_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_RESET_MEDIA_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_SET_AVAUDIO_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_SET_AVAUDVOL_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_SET_AVCFG_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_SET_AVCODEC_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_SET_AVCODECMICTXG_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_SET_AVCODECRXG_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_SET_AVCODECSTG_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_SET_AVEC_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_SET_AVFLTREN_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_SET_AVMMINUSE_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_SET_AVMMUTE_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_SET_AVMUTE_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_SET_AVNS_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_SET_AVRXAGC_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_SET_AVRXAVC_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_SET_AVRXG_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_SET_AVTXAGC_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_SET_AVTXG_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_SET_CLVL_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_SET_CMUTE_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_SET_CSD_AP_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_SET_LDTMFEN_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_SET_LPBK_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_SET_MEDIA_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_SET_MFT_CODEC_MODE_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_SET_MICGAIN_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_SET_PIFACE_PREF_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_SET_PROFILE_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_SET_RXPCMIIR_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_SET_SPKRGAIN_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_SET_STG_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_SET_TTY_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_SET_TXPCMIIR_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_SET_TXVOL_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_SET_VOLDB_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_SET_VOLUME_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_SET_VTD_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_SET_VTS_V01 
//#define REMOVE_QMI_SWI_M2M_AUDIO_SET_WDDM_V01 
//#define REMOVE_QMI_SWI_M2M_PING_V01 

/*Service Message Definition*/
/** @addtogroup sierra_m2m_audio_api_qmi_msg_ids
    @{
  */
#define QMI_SWI_M2M_PING_REQ_V01 0x0000
#define QMI_SWI_M2M_PING_RESP_V01 0x0000
#define QMI_SWI_M2M_PING_IND_V01 0x0000
#define QMI_SWI_M2M_AUDIO_GET_PROFILE_REQ_V01 0x0001
#define QMI_SWI_M2M_AUDIO_GET_PROFILE_RESP_V01 0x0001
#define QMI_SWI_M2M_AUDIO_SET_PROFILE_REQ_V01 0x0002
#define QMI_SWI_M2M_AUDIO_SET_PROFILE_RESP_V01 0x0002
#define QMI_SWI_M2M_AUDIO_GET_VOLUME_REQ_V01 0x0003
#define QMI_SWI_M2M_AUDIO_GET_VOLUME_RESP_V01 0x0003
#define QMI_SWI_M2M_AUDIO_SET_VOLUME_REQ_V01 0x0004
#define QMI_SWI_M2M_AUDIO_SET_VOLUME_RESP_V01 0x0004
#define QMI_SWI_M2M_AUDIO_GET_VOLDB_REQ_V01 0x0005
#define QMI_SWI_M2M_AUDIO_GET_VOLDB_RESP_V01 0x0005
#define QMI_SWI_M2M_AUDIO_SET_VOLDB_REQ_V01 0x0006
#define QMI_SWI_M2M_AUDIO_SET_VOLDB_RESP_V01 0x0006
#define QMI_SWI_M2M_AUDIO_GET_STG_REQ_V01 0x0007
#define QMI_SWI_M2M_AUDIO_GET_STG_RESP_V01 0x0007
#define QMI_SWI_M2M_AUDIO_SET_STG_REQ_V01 0x0008
#define QMI_SWI_M2M_AUDIO_SET_STG_RESP_V01 0x0008
#define QMI_SWI_M2M_AUDIO_SET_LPBK_REQ_V01 0x0009
#define QMI_SWI_M2M_AUDIO_SET_LPBK_RESP_V01 0x0009
#define QMI_SWI_M2M_AUDIO_PLAY_TONE_REQ_V01 0x000A
#define QMI_SWI_M2M_AUDIO_PLAY_TONE_RESP_V01 0x000A
#define QMI_SWI_M2M_AUDIO_PLAY_FILE_REQ_V01 0x000B
#define QMI_SWI_M2M_AUDIO_PLAY_FILE_RESP_V01 0x000B
#define QMI_SWI_M2M_AUDIO_NV_DEF_REQ_V01 0x000C
#define QMI_SWI_M2M_AUDIO_NV_DEF_RESP_V01 0x000C
#define QMI_SWI_M2M_AUDIO_PLAY_IND_V01 0x000D
#define QMI_SWI_M2M_AUDIO_GET_TXVOL_REQ_V01 0x000E
#define QMI_SWI_M2M_AUDIO_GET_TXVOL_RESP_V01 0x000E
#define QMI_SWI_M2M_AUDIO_SET_TXVOL_REQ_V01 0x000F
#define QMI_SWI_M2M_AUDIO_SET_TXVOL_RESP_V01 0x000F
#define QMI_SWI_M2M_AUDIO_GET_MICGAIN_REQ_V01 0x0010
#define QMI_SWI_M2M_AUDIO_GET_MICGAIN_RESP_V01 0x0010
#define QMI_SWI_M2M_AUDIO_SET_MICGAIN_REQ_V01 0x0011
#define QMI_SWI_M2M_AUDIO_SET_MICGAIN_RESP_V01 0x0011
#define QMI_SWI_M2M_AUDIO_GET_SPKRGAIN_REQ_V01 0x0012
#define QMI_SWI_M2M_AUDIO_GET_SPKRGAIN_RESP_V01 0x0012
#define QMI_SWI_M2M_AUDIO_SET_SPKRGAIN_REQ_V01 0x0013
#define QMI_SWI_M2M_AUDIO_SET_SPKRGAIN_RESP_V01 0x0013
#define QMI_SWI_M2M_AUDIO_GET_TXPCMIIR_REQ_V01 0x0014
#define QMI_SWI_M2M_AUDIO_GET_TXPCMIIR_RESP_V01 0x0014
#define QMI_SWI_M2M_AUDIO_SET_TXPCMIIR_REQ_V01 0x0015
#define QMI_SWI_M2M_AUDIO_SET_TXPCMIIR_RESP_V01 0x0015
#define QMI_SWI_M2M_AUDIO_GET_RXPCMIIR_REQ_V01 0x0016
#define QMI_SWI_M2M_AUDIO_GET_RXPCMIIR_RESP_V01 0x0016
#define QMI_SWI_M2M_AUDIO_SET_RXPCMIIR_REQ_V01 0x0017
#define QMI_SWI_M2M_AUDIO_SET_RXPCMIIR_RESP_V01 0x0017
#define QMI_SWI_M2M_AUDIO_GET_LDTMFEN_REQ_V01 0x0018
#define QMI_SWI_M2M_AUDIO_GET_LDTMFEN_RESP_V01 0x0018
#define QMI_SWI_M2M_AUDIO_SET_LDTMFEN_REQ_V01 0x0019
#define QMI_SWI_M2M_AUDIO_SET_LDTMFEN_RESP_V01 0x0019
#define QMI_SWI_M2M_AUDIO_GET_AVCFG_REQ_V01 0x001A
#define QMI_SWI_M2M_AUDIO_GET_AVCFG_RESP_V01 0x001A
#define QMI_SWI_M2M_AUDIO_SET_AVCFG_REQ_V01 0x001B
#define QMI_SWI_M2M_AUDIO_SET_AVCFG_RESP_V01 0x001B
#define QMI_SWI_M2M_AUDIO_GET_AVMUTE_REQ_V01 0x001C
#define QMI_SWI_M2M_AUDIO_GET_AVMUTE_RESP_V01 0x001C
#define QMI_SWI_M2M_AUDIO_SET_AVMUTE_REQ_V01 0x001D
#define QMI_SWI_M2M_AUDIO_SET_AVMUTE_RESP_V01 0x001D
#define QMI_SWI_M2M_AUDIO_GET_AVTXG_REQ_V01 0x001E
#define QMI_SWI_M2M_AUDIO_GET_AVTXG_RESP_V01 0x001E
#define QMI_SWI_M2M_AUDIO_SET_AVTXG_REQ_V01 0x001F
#define QMI_SWI_M2M_AUDIO_SET_AVTXG_RESP_V01 0x001F
#define QMI_SWI_M2M_AUDIO_GET_AVRXG_REQ_V01 0x0020
#define QMI_SWI_M2M_AUDIO_GET_AVRXG_RESP_V01 0x0020
#define QMI_SWI_M2M_AUDIO_SET_AVRXG_REQ_V01 0x0021
#define QMI_SWI_M2M_AUDIO_SET_AVRXG_RESP_V01 0x0021
#define QMI_SWI_M2M_AUDIO_GET_AVTXAGC_REQ_V01 0x0022
#define QMI_SWI_M2M_AUDIO_GET_AVTXAGC_RESP_V01 0x0022
#define QMI_SWI_M2M_AUDIO_SET_AVTXAGC_REQ_V01 0x0023
#define QMI_SWI_M2M_AUDIO_SET_AVTXAGC_RESP_V01 0x0023
#define QMI_SWI_M2M_AUDIO_GET_AVRXAGC_REQ_V01 0x0024
#define QMI_SWI_M2M_AUDIO_GET_AVRXAGC_RESP_V01 0x0024
#define QMI_SWI_M2M_AUDIO_SET_AVRXAGC_REQ_V01 0x0025
#define QMI_SWI_M2M_AUDIO_SET_AVRXAGC_RESP_V01 0x0025
#define QMI_SWI_M2M_AUDIO_GET_AVRXAVC_REQ_V01 0x0026
#define QMI_SWI_M2M_AUDIO_GET_AVRXAVC_RESP_V01 0x0026
#define QMI_SWI_M2M_AUDIO_SET_AVRXAVC_REQ_V01 0x0027
#define QMI_SWI_M2M_AUDIO_SET_AVRXAVC_RESP_V01 0x0027
#define QMI_SWI_M2M_AUDIO_GET_AVEC_REQ_V01 0x0028
#define QMI_SWI_M2M_AUDIO_GET_AVEC_RESP_V01 0x0028
#define QMI_SWI_M2M_AUDIO_SET_AVEC_REQ_V01 0x0029
#define QMI_SWI_M2M_AUDIO_SET_AVEC_RESP_V01 0x0029
#define QMI_SWI_M2M_AUDIO_GET_AVNS_REQ_V01 0x002A
#define QMI_SWI_M2M_AUDIO_GET_AVNS_RESP_V01 0x002A
#define QMI_SWI_M2M_AUDIO_SET_AVNS_REQ_V01 0x002B
#define QMI_SWI_M2M_AUDIO_SET_AVNS_RESP_V01 0x002B
#define QMI_SWI_M2M_AUDIO_GET_AVCODECMICTXG_REQ_V01 0x002C
#define QMI_SWI_M2M_AUDIO_GET_AVCODECMICTXG_RESP_V01 0x002C
#define QMI_SWI_M2M_AUDIO_SET_AVCODECMICTXG_REQ_V01 0x002D
#define QMI_SWI_M2M_AUDIO_SET_AVCODECMICTXG_RESP_V01 0x002D
#define QMI_SWI_M2M_AUDIO_GET_AVFLTREN_REQ_V01 0x002E
#define QMI_SWI_M2M_AUDIO_GET_AVFLTREN_RESP_V01 0x002E
#define QMI_SWI_M2M_AUDIO_SET_AVFLTREN_REQ_V01 0x002F
#define QMI_SWI_M2M_AUDIO_SET_AVFLTREN_RESP_V01 0x002F
#define QMI_SWI_M2M_AUDIO_GET_WDDM_REQ_V01 0x0030
#define QMI_SWI_M2M_AUDIO_GET_WDDM_RESP_V01 0x0030
#define QMI_SWI_M2M_AUDIO_SET_WDDM_REQ_V01 0x0031
#define QMI_SWI_M2M_AUDIO_SET_WDDM_RESP_V01 0x0031
#define QMI_SWI_M2M_AUDIO_DTMF_IND_V01 0x0032
#define QMI_SWI_M2M_AUDIO_SET_AVAUDIO_REQ_V01 0x0033
#define QMI_SWI_M2M_AUDIO_SET_AVAUDIO_RESP_V01 0x0033
#define QMI_SWI_M2M_AUDIO_GET_TTY_REQ_V01 0x0034
#define QMI_SWI_M2M_AUDIO_GET_TTY_RESP_V01 0x0034
#define QMI_SWI_M2M_AUDIO_SET_TTY_REQ_V01 0x0035
#define QMI_SWI_M2M_AUDIO_SET_TTY_RESP_V01 0x0035
#define QMI_SWI_M2M_AUDIO_GET_AVAUDVOL_REQ_V01 0x0036
#define QMI_SWI_M2M_AUDIO_GET_AVAUDVOL_RESP_V01 0x0036
#define QMI_SWI_M2M_AUDIO_SET_AVAUDVOL_REQ_V01 0x0037
#define QMI_SWI_M2M_AUDIO_SET_AVAUDVOL_RESP_V01 0x0037
#define QMI_SWI_M2M_AUDIO_GET_AVMMUTE_REQ_V01 0x0038
#define QMI_SWI_M2M_AUDIO_GET_AVMMUTE_RESP_V01 0x0038
#define QMI_SWI_M2M_AUDIO_SET_AVMMUTE_REQ_V01 0x0039
#define QMI_SWI_M2M_AUDIO_SET_AVMMUTE_RESP_V01 0x0039
#define QMI_SWI_M2M_AUDIO_GET_AVMMINUSE_REQ_V01 0x003A
#define QMI_SWI_M2M_AUDIO_GET_AVMMINUSE_RESP_V01 0x003A
#define QMI_SWI_M2M_AUDIO_SET_AVMMINUSE_REQ_V01 0x003B
#define QMI_SWI_M2M_AUDIO_SET_AVMMINUSE_RESP_V01 0x003B
#define QMI_SWI_M2M_AUDIO_GET_LPBK_REQ_V01 0x003C
#define QMI_SWI_M2M_AUDIO_GET_LPBK_RESP_V01 0x003C
#define QMI_SWI_M2M_AUDIO_SET_MFT_CODEC_MODE_REQ_V01 0x003D
#define QMI_SWI_M2M_AUDIO_SET_MFT_CODEC_MODE_RESP_V01 0x003D
#define QMI_SWI_M2M_AUDIO_GET_CSD_AP_REQ_V01 0x003E
#define QMI_SWI_M2M_AUDIO_GET_CSD_AP_RESP_V01 0x003E
#define QMI_SWI_M2M_AUDIO_SET_CSD_AP_REQ_V01 0x003F
#define QMI_SWI_M2M_AUDIO_SET_CSD_AP_RESP_V01 0x003F
#define QMI_SWI_M2M_AUDIO_SET_MEDIA_REQ_V01 0x0040
#define QMI_SWI_M2M_AUDIO_SET_MEDIA_RESP_V01 0x0040
#define QMI_SWI_M2M_AUDIO_ADD_MEDIA_REQ_V01 0x0041
#define QMI_SWI_M2M_AUDIO_ADD_MEDIA_RESP_V01 0x0041
#define QMI_SWI_M2M_AUDIO_RESET_MEDIA_REQ_V01 0x0042
#define QMI_SWI_M2M_AUDIO_RESET_MEDIA_RESP_V01 0x0042
#define QMI_SWI_M2M_AUDIO_GET_PIFACE_CONFIG_REQ_V01 0x0043
#define QMI_SWI_M2M_AUDIO_GET_PIFACE_CONFIG_RESP_V01 0x0043
#define QMI_SWI_M2M_AUDIO_GET_PIFACE_PREF_REQ_V01 0x0044
#define QMI_SWI_M2M_AUDIO_GET_PIFACE_PREF_RESP_V01 0x0044
#define QMI_SWI_M2M_AUDIO_SET_PIFACE_PREF_REQ_V01 0x0045
#define QMI_SWI_M2M_AUDIO_SET_PIFACE_PREF_RESP_V01 0x0045
#define QMI_SWI_M2M_AUDIO_GET_CLVL_REQ_V01 0x0046
#define QMI_SWI_M2M_AUDIO_GET_CLVL_RESP_V01 0x0046
#define QMI_SWI_M2M_AUDIO_SET_CLVL_REQ_V01 0x0047
#define QMI_SWI_M2M_AUDIO_SET_CLVL_RESP_V01 0x0047
#define QMI_SWI_M2M_AUDIO_SET_VTD_REQ_V01 0x0048
#define QMI_SWI_M2M_AUDIO_SET_VTD_RESP_V01 0x0048
#define QMI_SWI_M2M_AUDIO_GET_VTD_REQ_V01 0x0049
#define QMI_SWI_M2M_AUDIO_GET_VTD_RESP_V01 0x0049
#define QMI_SWI_M2M_AUDIO_SET_VTS_REQ_V01 0x004A
#define QMI_SWI_M2M_AUDIO_SET_VTS_RESP_V01 0x004A
#define QMI_SWI_M2M_AUDIO_GET_AVCODECRXG_REQ_V01 0x004B
#define QMI_SWI_M2M_AUDIO_GET_AVCODECRXG_RESP_V01 0x004B
#define QMI_SWI_M2M_AUDIO_SET_AVCODECRXG_REQ_V01 0x004C
#define QMI_SWI_M2M_AUDIO_SET_AVCODECRXG_RESP_V01 0x004C
#define QMI_SWI_M2M_AUDIO_GET_AVCODECSTG_REQ_V01 0x004D
#define QMI_SWI_M2M_AUDIO_GET_AVCODECSTG_RESP_V01 0x004D
#define QMI_SWI_M2M_AUDIO_SET_AVCODECSTG_REQ_V01 0x004E
#define QMI_SWI_M2M_AUDIO_SET_AVCODECSTG_RESP_V01 0x004E
#define QMI_SWI_M2M_AUDIO_GET_AVCODEC_REQ_V01 0x004F
#define QMI_SWI_M2M_AUDIO_GET_AVCODEC_RESP_V01 0x004F
#define QMI_SWI_M2M_AUDIO_SET_AVCODEC_REQ_V01 0x0050
#define QMI_SWI_M2M_AUDIO_SET_AVCODEC_RESP_V01 0x0050
#define QMI_SWI_M2M_AUDIO_GET_CMUTE_REQ_V01 0x0051
#define QMI_SWI_M2M_AUDIO_GET_CMUTE_RESP_V01 0x0051
#define QMI_SWI_M2M_AUDIO_SET_CMUTE_REQ_V01 0x0052
#define QMI_SWI_M2M_AUDIO_SET_CMUTE_RESP_V01 0x0052
#define QMI_SWI_M2M_AUDIO_GET_EC_OFF_MASK_REQ_V01 0x0053
#define QMI_SWI_M2M_AUDIO_GET_EC_OFF_MASK_RESP_V01 0x0053
#define QMI_SWI_M2M_AUDIO_SET_EC_OFF_MASK_REQ_V01 0x0054
#define QMI_SWI_M2M_AUDIO_SET_EC_OFF_MASK_RESP_V01 0x0054

/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor 
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro sierra_m2m_audio_api_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type sierra_m2m_audio_api_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );
 
/** This macro should be used to get the service object */ 
#define sierra_m2m_audio_api_get_service_object_v01( ) \
          sierra_m2m_audio_api_get_service_object_internal_v01( \
            SIERRA_M2M_AUDIO_API_V01_IDL_MAJOR_VERS, SIERRA_M2M_AUDIO_API_V01_IDL_MINOR_VERS, \
            SIERRA_M2M_AUDIO_API_V01_IDL_TOOL_VERS )
/** 
    @} 
  */


#ifdef __cplusplus
}
#endif
#endif

