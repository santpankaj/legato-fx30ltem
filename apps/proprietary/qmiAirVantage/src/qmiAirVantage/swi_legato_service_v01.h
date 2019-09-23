#ifndef SWI_LEGATO_SERVICE_01_H
#define SWI_LEGATO_SERVICE_01_H
/**
  @file swi_legato_service_v01.h

  @brief This is the public header file which defines the swi_legato service Data structures.

  This header file defines the types and structures that were defined in
  swi_legato. It contains the constant values defined, enums, structures,
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

/* This file was generated with Tool version 6.14.9
   It was generated on: Mon Mar 26 2018 (Spin 0)
   From IDL File: swi_legato_service_v01.idl */

/** @defgroup swi_legato_qmi_consts Constant values defined in the IDL */
/** @defgroup swi_legato_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup swi_legato_qmi_enums Enumerated types used in QMI messages */
/** @defgroup swi_legato_qmi_messages Structures sent as QMI messages */
/** @defgroup swi_legato_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup swi_legato_qmi_accessor Accessor for QMI service object */
/** @defgroup swi_legato_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup swi_legato_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define SWI_LEGATO_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define SWI_LEGATO_V01_IDL_MINOR_VERS 0x01
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define SWI_LEGATO_V01_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
#define SWI_LEGATO_V01_MAX_MESSAGE_ID 0x00AA
/**
    @}
  */


/** @addtogroup swi_legato_qmi_consts
    @{
  */
#define QMI_SERVICE_SWI_LEGATO_ID_V01 0xFD
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

/** @addtogroup swi_legato_qmi_messages
    @{
  */
/** Request Message; This is a message for testing client to service communication */
typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_legato_test_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup swi_legato_qmi_messages
    @{
  */
/** Response Message; This is a message for testing client to service communication */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}swi_legato_test_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup swi_legato_qmi_messages
    @{
  */
/** Request Message; Perform an OMA-DM AVMS session with the AVMS server */
typedef struct {

  /* Mandatory */
  /*  SESSION START */
  uint8_t session_type;
  /**<
                                     0x01 ¨C FOTA, to check availability of FW Update
                                     */
}swi_legato_avms_session_start_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup swi_legato_qmi_messages
    @{
  */
/** Response Message; Perform an OMA-DM AVMS session with the AVMS server */
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
}swi_legato_avms_session_start_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup swi_legato_qmi_messages
    @{
  */
/** Request Message; This message requests the device to cancel an ongoing OMA-DM AVMS session */
typedef struct {

  /* Mandatory */
  /*  SESSION STOP */
  uint8_t session_type;
  /**<
                                 0x01 ¨C FOTA, to suspend FOTA session
                                 0xFF ¨C Suspend ongoing FOTA session
                                        or stop any other active OMADM AVMS session.
                                 */
}swi_legato_avms_session_stop_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup swi_legato_qmi_messages
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
}swi_legato_avms_session_stop_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup swi_legato_qmi_messages
    @{
  */
/** Request Message; This message requests the device to cancel an ongoing OMA-DM AVMS session */
typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_legato_avms_session_getinfo_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup swi_legato_qmi_aggregates
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

/** @addtogroup swi_legato_qmi_aggregates
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

/** @addtogroup swi_legato_qmi_aggregates
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

  /*  SESSION GETINFO */
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

/** @addtogroup swi_legato_qmi_aggregates
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

  /*  SESSION GETINFO */
  uint16_t user_input_timeout;
  /**<
                                    Timeout for user input in minutes. A value of 0 means no time-out
                                    */
}avms_connection_request_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup swi_legato_qmi_aggregates
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

/** @addtogroup swi_legato_qmi_messages
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
}swi_legato_avms_session_getinfo_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup swi_legato_qmi_messages
    @{
  */
/** Indication Message; This unsolicited message is sent by the service to
	a control point that registers for QMI_SWI_LEGATO_AVMS_SET_EVENT_REPORT when any information in the TLV */
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
}swi_legato_avms_event_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup swi_legato_qmi_messages
    @{
  */
/** Request Message; This message is used to make a selection after the host has received QMI_SWI_LEGATO_AVMS_EVENT_IND. */
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
  /**<   Reject reason */
}swi_legato_avms_selection_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup swi_legato_qmi_messages
    @{
  */
/** Response Message; This message is used to make a selection after the host has received QMI_SWI_LEGATO_AVMS_EVENT_IND. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/
}swi_legato_avms_selection_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup swi_legato_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t apn_len;
  /**<   Length of APN string in bytes */

  char apn[APN_MAX_LEN_V01];
  /**<   APN in ASCII */

  uint8_t uname_len;
  /**<   Length of Username string in bytes */

  char uname[UNAME_MAX_LEN_V01];
  /**<   Username in ASCII */

  uint8_t pwd_len;
  /**<   Length of Username string in bytes */

  char pwd[PWD_MAX_LEN_V01];
  /**<   Username in ASCII */
}apn_config_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup swi_legato_qmi_messages
    @{
  */
/** Request Message; This message gets settings related to OMA DM AVMS */
typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_legato_avms_get_setting_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup swi_legato_qmi_messages
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
}swi_legato_avms_get_setting_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup swi_legato_qmi_messages
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
}swi_legato_avms_set_setting_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup swi_legato_qmi_messages
    @{
  */
/** Response Message; This message gets settings related to OMA DM AVMS */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/
}swi_legato_avms_set_setting_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup swi_legato_qmi_messages
    @{
  */
/** Request Message; This message is used to register for the QMI_SWI_LEGATO_AVMS_EVENT_IND */
typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_legato_avms_set_event_report_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup swi_legato_qmi_messages
    @{
  */
/** Response Message; This message is used to register for the QMI_SWI_LEGATO_AVMS_EVENT_IND */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/
}swi_legato_avms_set_event_report_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup swi_legato_qmi_messages
    @{
  */
/** Request Message; This message is retrieves only those that are used by the application processor */
typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_legato_avms_get_wams_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup swi_legato_qmi_aggregates
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

/** @addtogroup swi_legato_qmi_aggregates
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

/** @addtogroup swi_legato_qmi_messages
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
}swi_legato_avms_get_wams_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup swi_legato_qmi_messages
    @{
  */
/** Request Message; This message informs the modem that the application side is
     about to enter the authentication phase */
typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_legato_avms_open_nonce_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup swi_legato_qmi_aggregates
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

/** @addtogroup swi_legato_qmi_messages
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
  /*  Delta binary and CRC */
  Delta_nonce_value_v01 Delta_nonce;
  /**<
                                             Delta binary and CRC
                                             */
}swi_legato_avms_open_nonce_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup swi_legato_qmi_messages
    @{
  */
/** Request Message; This message informs the modem that the application side is
     about to enter the authentication phase */
typedef struct {

  /* Mandatory */
  /*  The delta binary of the nonce value */
  uint8_t delta_nonce[NONCE_LEN_V01];
}swi_legato_avms_close_nonce_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup swi_legato_qmi_messages
    @{
  */
/** Response Message; This message informs the modem that the application side is
     about to enter the authentication phase */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/
}swi_legato_avms_close_nonce_resp_msg_v01;  /* Message */
/**
    @}
  */

/* Conditional compilation tags for message removal */
//#define REMOVE_QMI_SWI_LEGATO_AVMS_CLOSE_NONCE_V01
//#define REMOVE_QMI_SWI_LEGATO_AVMS_EVENT_IND_V01
//#define REMOVE_QMI_SWI_LEGATO_AVMS_GET_SETTINGS_V01
//#define REMOVE_QMI_SWI_LEGATO_AVMS_GET_WAMS_V01
//#define REMOVE_QMI_SWI_LEGATO_AVMS_OPEN_NONCE_V01
//#define REMOVE_QMI_SWI_LEGATO_AVMS_SELECTION_V01
//#define REMOVE_QMI_SWI_LEGATO_AVMS_SESSION_GETINFO_V01
//#define REMOVE_QMI_SWI_LEGATO_AVMS_SESSION_START_V01
//#define REMOVE_QMI_SWI_LEGATO_AVMS_SESSION_STOP_V01
//#define REMOVE_QMI_SWI_LEGATO_AVMS_SET_EVENT_REPORT_V01
//#define REMOVE_QMI_SWI_LEGATO_AVMS_SET_SETTINGS_V01
//#define REMOVE_QMI_SWI_LEGATO_TEST_V01

/*Service Message Definition*/
/** @addtogroup swi_legato_qmi_msg_ids
    @{
  */
#define QMI_SWI_LEGATO_TEST_REQ_V01 0x0001
#define QMI_SWI_LEGATO_TEST_RESP_V01 0x0001
#define QMI_SWI_LEGATO_AVMS_SESSION_START_REQ_V01 0x00A0
#define QMI_SWI_LEGATO_AVMS_SESSION_START_RESP_V01 0x00A0
#define QMI_SWI_LEGATO_AVMS_SESSION_STOP_REQ_V01 0x00A1
#define QMI_SWI_LEGATO_AVMS_SESSION_STOP_RESP_V01 0x00A1
#define QMI_SWI_LEGATO_AVMS_SESSION_GETINFO_REQ_V01 0x00A2
#define QMI_SWI_LEGATO_AVMS_SESSION_GETINFO_RESP_V01 0x00A2
#define QMI_SWI_LEGATO_AVMS_EVENT_IND_V01 0x00A3
#define QMI_SWI_LEGATO_AVMS_SELECTION_REQ_V01 0x00A4
#define QMI_SWI_LEGATO_AVMS_SELECTION_RESP_V01 0x00A4
#define QMI_SWI_LEGATO_AVMS_SET_SETTINGS_REQ_V01 0x00A5
#define QMI_SWI_LEGATO_AVMS_SET_SETTINGS_RESP_V01 0x00A5
#define QMI_SWI_LEGATO_AVMS_GET_SETTINGS_REQ_V01 0x00A6
#define QMI_SWI_LEGATO_AVMS_GET_SETTINGS_RESP_V01 0x00A6
#define QMI_SWI_LEGATO_AVMS_SET_EVENT_REPORT_REQ_V01 0x00A7
#define QMI_SWI_LEGATO_AVMS_SET_EVENT_REPORT_RESP_V01 0x00A7
#define QMI_SWI_LEGATO_AVMS_GET_WAMS_REQ_V01 0x00A8
#define QMI_SWI_LEGATO_AVMS_GET_WAMS_RESP_V01 0x00A8
#define QMI_SWI_LEGATO_AVMS_OPEN_NONCE_REQ_V01 0x00A9
#define QMI_SWI_LEGATO_AVMS_OPEN_NONCE_RESP_V01 0x00A9
#define QMI_SWI_LEGATO_AVMS_CLOSE_NONCE_REQ_V01 0x00AA
#define QMI_SWI_LEGATO_AVMS_CLOSE_NONCE_RESP_V01 0x00AA
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro swi_legato_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type swi_legato_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );

/** This macro should be used to get the service object */
#define swi_legato_get_service_object_v01( ) \
          swi_legato_get_service_object_internal_v01( \
            SWI_LEGATO_V01_IDL_MAJOR_VERS, SWI_LEGATO_V01_IDL_MINOR_VERS, \
            SWI_LEGATO_V01_IDL_TOOL_VERS )
/**
    @}
  */


#ifdef __cplusplus
}
#endif
#endif

