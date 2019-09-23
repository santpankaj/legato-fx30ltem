/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        S W I _ L E G A T O _ S E R V I C E _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the swi_legato service Data structures.




 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.14.9
   It was generated on: Mon Mar 26 2018 (Spin 0)
   From IDL File: swi_legato_service_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "swi_legato_service_v01.h"
#include "common_v01.h"


/*Type Definitions*/
static const uint8_t avms_session_info_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(avms_session_info_type_v01, binary_type),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(avms_session_info_type_v01, status),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(avms_session_info_type_v01, user_input_req),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(avms_session_info_type_v01, user_input_timeout),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(avms_session_info_type_v01, fw_dload_size),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(avms_session_info_type_v01, fw_dload_complete),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(avms_session_info_type_v01, update_complete_status),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(avms_session_info_type_v01, severity),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(avms_session_info_type_v01, version_name_len),

  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(avms_session_info_type_v01, version_name),
  VERSION_MAX_LEN_V01,

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(avms_session_info_type_v01, package_name_len),

  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(avms_session_info_type_v01, package_name),
  PACKAGE_MAX_LEN_V01,

  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(avms_session_info_type_v01, package_description_len),

  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(avms_session_info_type_v01, package_description),
  ((PACKAGE_DESCRIPTION_MAX_LEN_V01) & 0xFF), ((PACKAGE_DESCRIPTION_MAX_LEN_V01) >> 8),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t avms_config_info_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(avms_config_info_type_v01, state),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(avms_config_info_type_v01, user_input_req),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(avms_config_info_type_v01, user_input_timeout),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(avms_config_info_type_v01, alertmsglength),

  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(avms_config_info_type_v01, alertmsg),
  ALER_MAX_LEN_V01,

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t avms_notifications_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(avms_notifications_type_v01, notification),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(avms_notifications_type_v01, session_status),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t avms_connection_request_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(avms_connection_request_type_v01, user_input_req),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(avms_connection_request_type_v01, user_input_timeout),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t avms_data_session_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(avms_data_session_type_v01, notif_type),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(avms_data_session_type_v01, err_code),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t apn_config_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(apn_config_type_v01, apn_len),

  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(apn_config_type_v01, apn),
  APN_MAX_LEN_V01,

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(apn_config_type_v01, uname_len),

  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(apn_config_type_v01, uname),
  UNAME_MAX_LEN_V01,

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(apn_config_type_v01, pwd_len),

  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(apn_config_type_v01, pwd),
  PWD_MAX_LEN_V01,

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t wams_service_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wams_service_type_v01, device_login),
  16,

  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wams_service_type_v01, device_MD5_key),
  16,

  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wams_service_type_v01, server_login),
  16,

  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wams_service_type_v01, server_MD5_key),
  16,

  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wams_service_type_v01, server_URL),
  80,

  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wams_service_type_v01, Nonce),
  16,

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t wams_key_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wams_key_type_v01, Num_instance),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wams_key_type_v01, key_index),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wams_key_type_v01, key_version),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(wams_key_type_v01, key_length),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wams_key_type_v01, key_data),
  ((294) & 0xFF), ((294) >> 8),

  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(wams_key_type_v01, key_meta_data),
  8,

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t Delta_nonce_value_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(Delta_nonce_value_v01, delta_nonce),
  16,

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(Delta_nonce_value_v01, crc_actual_nonce),

  QMI_IDL_FLAG_END_VALUE
};

/*Message Definitions*/
/*
 * swi_legato_test_req_msg is empty
 * static const uint8_t swi_legato_test_req_msg_data_v01[] = {
 * };
 */

static const uint8_t swi_legato_test_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(swi_legato_test_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t swi_legato_avms_session_start_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(swi_legato_avms_session_start_req_msg_v01, session_type)
};

static const uint8_t swi_legato_avms_session_start_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(swi_legato_avms_session_start_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(swi_legato_avms_session_start_resp_msg_v01, start_session_rsp)
};

static const uint8_t swi_legato_avms_session_stop_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(swi_legato_avms_session_stop_req_msg_v01, session_type)
};

static const uint8_t swi_legato_avms_session_stop_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(swi_legato_avms_session_stop_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(swi_legato_avms_session_stop_resp_msg_v01, session_type) - QMI_IDL_OFFSET8(swi_legato_avms_session_stop_resp_msg_v01, session_type_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(swi_legato_avms_session_stop_resp_msg_v01, session_type)
};

/*
 * swi_legato_avms_session_getinfo_req_msg is empty
 * static const uint8_t swi_legato_avms_session_getinfo_req_msg_data_v01[] = {
 * };
 */

static const uint8_t swi_legato_avms_session_getinfo_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(swi_legato_avms_session_getinfo_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(swi_legato_avms_session_getinfo_resp_msg_v01, avms_session_info) - QMI_IDL_OFFSET8(swi_legato_avms_session_getinfo_resp_msg_v01, avms_session_info_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(swi_legato_avms_session_getinfo_resp_msg_v01, avms_session_info),
  QMI_IDL_TYPE88(0, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(swi_legato_avms_session_getinfo_resp_msg_v01, avms_config_info) - QMI_IDL_OFFSET16RELATIVE(swi_legato_avms_session_getinfo_resp_msg_v01, avms_config_info_valid)),
  0x11,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(swi_legato_avms_session_getinfo_resp_msg_v01, avms_config_info),
  QMI_IDL_TYPE88(0, 1),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(swi_legato_avms_session_getinfo_resp_msg_v01, avms_notifications) - QMI_IDL_OFFSET16RELATIVE(swi_legato_avms_session_getinfo_resp_msg_v01, avms_notifications_valid)),
  0x12,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(swi_legato_avms_session_getinfo_resp_msg_v01, avms_notifications),
  QMI_IDL_TYPE88(0, 2)
};

static const uint8_t swi_legato_avms_event_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(swi_legato_avms_event_ind_msg_v01, avms_session_info) - QMI_IDL_OFFSET8(swi_legato_avms_event_ind_msg_v01, avms_session_info_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(swi_legato_avms_event_ind_msg_v01, avms_session_info),
  QMI_IDL_TYPE88(0, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(swi_legato_avms_event_ind_msg_v01, avms_config_info) - QMI_IDL_OFFSET16RELATIVE(swi_legato_avms_event_ind_msg_v01, avms_config_info_valid)),
  0x11,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(swi_legato_avms_event_ind_msg_v01, avms_config_info),
  QMI_IDL_TYPE88(0, 1),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(swi_legato_avms_event_ind_msg_v01, avms_notifications) - QMI_IDL_OFFSET16RELATIVE(swi_legato_avms_event_ind_msg_v01, avms_notifications_valid)),
  0x12,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(swi_legato_avms_event_ind_msg_v01, avms_notifications),
  QMI_IDL_TYPE88(0, 2),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(swi_legato_avms_event_ind_msg_v01, avms_connection_request) - QMI_IDL_OFFSET16RELATIVE(swi_legato_avms_event_ind_msg_v01, avms_connection_request_valid)),
  0x13,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(swi_legato_avms_event_ind_msg_v01, avms_connection_request),
  QMI_IDL_TYPE88(0, 3),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(swi_legato_avms_event_ind_msg_v01, wams_changed_mask) - QMI_IDL_OFFSET16RELATIVE(swi_legato_avms_event_ind_msg_v01, wams_changed_mask_valid)),
  0x14,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(swi_legato_avms_event_ind_msg_v01, wams_changed_mask),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(swi_legato_avms_event_ind_msg_v01, package_ID) - QMI_IDL_OFFSET16RELATIVE(swi_legato_avms_event_ind_msg_v01, package_ID_valid)),
  0x15,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(swi_legato_avms_event_ind_msg_v01, package_ID),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(swi_legato_avms_event_ind_msg_v01, reg_status) - QMI_IDL_OFFSET16RELATIVE(swi_legato_avms_event_ind_msg_v01, reg_status_valid)),
  0x16,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(swi_legato_avms_event_ind_msg_v01, reg_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(swi_legato_avms_event_ind_msg_v01, avms_data_session) - QMI_IDL_OFFSET16RELATIVE(swi_legato_avms_event_ind_msg_v01, avms_data_session_valid)),
  0x17,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(swi_legato_avms_event_ind_msg_v01, avms_data_session),
  QMI_IDL_TYPE88(0, 4),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(swi_legato_avms_event_ind_msg_v01, session_type) - QMI_IDL_OFFSET16RELATIVE(swi_legato_avms_event_ind_msg_v01, session_type_valid)),
  0x18,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(swi_legato_avms_event_ind_msg_v01, session_type)
};

static const uint8_t swi_legato_avms_selection_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(swi_legato_avms_selection_req_msg_v01, user_input),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(swi_legato_avms_selection_req_msg_v01, defer_time) - QMI_IDL_OFFSET8(swi_legato_avms_selection_req_msg_v01, defer_time_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(swi_legato_avms_selection_req_msg_v01, defer_time),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(swi_legato_avms_selection_req_msg_v01, client_perform_operation_flag) - QMI_IDL_OFFSET8(swi_legato_avms_selection_req_msg_v01, client_perform_operation_flag_valid)),
  0x11,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(swi_legato_avms_selection_req_msg_v01, client_perform_operation_flag),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(swi_legato_avms_selection_req_msg_v01, package_ID) - QMI_IDL_OFFSET8(swi_legato_avms_selection_req_msg_v01, package_ID_valid)),
  0x12,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(swi_legato_avms_selection_req_msg_v01, package_ID),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(swi_legato_avms_selection_req_msg_v01, reject_reason) - QMI_IDL_OFFSET8(swi_legato_avms_selection_req_msg_v01, reject_reason_valid)),
  0x13,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(swi_legato_avms_selection_req_msg_v01, reject_reason)
};

static const uint8_t swi_legato_avms_selection_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(swi_legato_avms_selection_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/*
 * swi_legato_avms_get_setting_req_msg is empty
 * static const uint8_t swi_legato_avms_get_setting_req_msg_data_v01[] = {
 * };
 */

static const uint8_t swi_legato_avms_get_setting_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(swi_legato_avms_get_setting_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(swi_legato_avms_get_setting_resp_msg_v01, enabled),

  0x03,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(swi_legato_avms_get_setting_resp_msg_v01, fw_autodload),

  0x04,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(swi_legato_avms_get_setting_resp_msg_v01, fw_autoupdate),

  0x05,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(swi_legato_avms_get_setting_resp_msg_v01, fw_autosdm),

  0x06,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(swi_legato_avms_get_setting_resp_msg_v01, autoconnect),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(swi_legato_avms_get_setting_resp_msg_v01, polling_timer) - QMI_IDL_OFFSET8(swi_legato_avms_get_setting_resp_msg_v01, polling_timer_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(swi_legato_avms_get_setting_resp_msg_v01, polling_timer),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(swi_legato_avms_get_setting_resp_msg_v01, retry_timer) - QMI_IDL_OFFSET8(swi_legato_avms_get_setting_resp_msg_v01, retry_timer_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY |  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(swi_legato_avms_get_setting_resp_msg_v01, retry_timer),
  8,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(swi_legato_avms_get_setting_resp_msg_v01, apn_config) - QMI_IDL_OFFSET8(swi_legato_avms_get_setting_resp_msg_v01, apn_config_valid)),
  0x12,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(swi_legato_avms_get_setting_resp_msg_v01, apn_config),
  QMI_IDL_TYPE88(0, 5)
};

static const uint8_t swi_legato_avms_set_setting_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(swi_legato_avms_set_setting_req_msg_v01, fw_autodload),

  0x02,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(swi_legato_avms_set_setting_req_msg_v01, fw_autoupdate),

  0x03,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(swi_legato_avms_set_setting_req_msg_v01, autoconnect),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(swi_legato_avms_set_setting_req_msg_v01, fw_autosdm) - QMI_IDL_OFFSET8(swi_legato_avms_set_setting_req_msg_v01, fw_autosdm_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(swi_legato_avms_set_setting_req_msg_v01, fw_autosdm),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(swi_legato_avms_set_setting_req_msg_v01, polling_timer) - QMI_IDL_OFFSET8(swi_legato_avms_set_setting_req_msg_v01, polling_timer_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(swi_legato_avms_set_setting_req_msg_v01, polling_timer),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(swi_legato_avms_set_setting_req_msg_v01, retry_timer) - QMI_IDL_OFFSET8(swi_legato_avms_set_setting_req_msg_v01, retry_timer_valid)),
  0x12,
  QMI_IDL_FLAGS_IS_ARRAY |  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(swi_legato_avms_set_setting_req_msg_v01, retry_timer),
  8,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(swi_legato_avms_set_setting_req_msg_v01, apn_config) - QMI_IDL_OFFSET8(swi_legato_avms_set_setting_req_msg_v01, apn_config_valid)),
  0x13,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(swi_legato_avms_set_setting_req_msg_v01, apn_config),
  QMI_IDL_TYPE88(0, 5)
};

static const uint8_t swi_legato_avms_set_setting_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(swi_legato_avms_set_setting_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/*
 * swi_legato_avms_set_event_report_req_msg is empty
 * static const uint8_t swi_legato_avms_set_event_report_req_msg_data_v01[] = {
 * };
 */

static const uint8_t swi_legato_avms_set_event_report_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(swi_legato_avms_set_event_report_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/*
 * swi_legato_avms_get_wams_req_msg is empty
 * static const uint8_t swi_legato_avms_get_wams_req_msg_data_v01[] = {
 * };
 */

static const uint8_t swi_legato_avms_get_wams_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(swi_legato_avms_get_wams_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(swi_legato_avms_get_wams_resp_msg_v01, wams_service),
  QMI_IDL_TYPE88(0, 6),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x03,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(swi_legato_avms_get_wams_resp_msg_v01, wams_key),
  QMI_IDL_TYPE88(0, 7)
};

/*
 * swi_legato_avms_open_nonce_req_msg is empty
 * static const uint8_t swi_legato_avms_open_nonce_req_msg_data_v01[] = {
 * };
 */

static const uint8_t swi_legato_avms_open_nonce_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(swi_legato_avms_open_nonce_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(swi_legato_avms_open_nonce_resp_msg_v01, Delta_nonce),
  QMI_IDL_TYPE88(0, 8)
};

static const uint8_t swi_legato_avms_close_nonce_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_FLAGS_IS_ARRAY |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(swi_legato_avms_close_nonce_req_msg_v01, delta_nonce),
  NONCE_LEN_V01
};

static const uint8_t swi_legato_avms_close_nonce_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(swi_legato_avms_close_nonce_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* Type Table */
static const qmi_idl_type_table_entry  swi_legato_type_table_v01[] = {
  {sizeof(avms_session_info_type_v01), avms_session_info_type_data_v01},
  {sizeof(avms_config_info_type_v01), avms_config_info_type_data_v01},
  {sizeof(avms_notifications_type_v01), avms_notifications_type_data_v01},
  {sizeof(avms_connection_request_type_v01), avms_connection_request_type_data_v01},
  {sizeof(avms_data_session_type_v01), avms_data_session_type_data_v01},
  {sizeof(apn_config_type_v01), apn_config_type_data_v01},
  {sizeof(wams_service_type_v01), wams_service_type_data_v01},
  {sizeof(wams_key_type_v01), wams_key_type_data_v01},
  {sizeof(Delta_nonce_value_v01), Delta_nonce_value_data_v01}
};

/* Message Table */
static const qmi_idl_message_table_entry swi_legato_message_table_v01[] = {
  {sizeof(swi_legato_test_req_msg_v01), 0},
  {sizeof(swi_legato_test_resp_msg_v01), swi_legato_test_resp_msg_data_v01},
  {sizeof(swi_legato_avms_session_start_req_msg_v01), swi_legato_avms_session_start_req_msg_data_v01},
  {sizeof(swi_legato_avms_session_start_resp_msg_v01), swi_legato_avms_session_start_resp_msg_data_v01},
  {sizeof(swi_legato_avms_session_stop_req_msg_v01), swi_legato_avms_session_stop_req_msg_data_v01},
  {sizeof(swi_legato_avms_session_stop_resp_msg_v01), swi_legato_avms_session_stop_resp_msg_data_v01},
  {sizeof(swi_legato_avms_session_getinfo_req_msg_v01), 0},
  {sizeof(swi_legato_avms_session_getinfo_resp_msg_v01), swi_legato_avms_session_getinfo_resp_msg_data_v01},
  {sizeof(swi_legato_avms_event_ind_msg_v01), swi_legato_avms_event_ind_msg_data_v01},
  {sizeof(swi_legato_avms_selection_req_msg_v01), swi_legato_avms_selection_req_msg_data_v01},
  {sizeof(swi_legato_avms_selection_resp_msg_v01), swi_legato_avms_selection_resp_msg_data_v01},
  {sizeof(swi_legato_avms_get_setting_req_msg_v01), 0},
  {sizeof(swi_legato_avms_get_setting_resp_msg_v01), swi_legato_avms_get_setting_resp_msg_data_v01},
  {sizeof(swi_legato_avms_set_setting_req_msg_v01), swi_legato_avms_set_setting_req_msg_data_v01},
  {sizeof(swi_legato_avms_set_setting_resp_msg_v01), swi_legato_avms_set_setting_resp_msg_data_v01},
  {sizeof(swi_legato_avms_set_event_report_req_msg_v01), 0},
  {sizeof(swi_legato_avms_set_event_report_resp_msg_v01), swi_legato_avms_set_event_report_resp_msg_data_v01},
  {sizeof(swi_legato_avms_get_wams_req_msg_v01), 0},
  {sizeof(swi_legato_avms_get_wams_resp_msg_v01), swi_legato_avms_get_wams_resp_msg_data_v01},
  {sizeof(swi_legato_avms_open_nonce_req_msg_v01), 0},
  {sizeof(swi_legato_avms_open_nonce_resp_msg_v01), swi_legato_avms_open_nonce_resp_msg_data_v01},
  {sizeof(swi_legato_avms_close_nonce_req_msg_v01), swi_legato_avms_close_nonce_req_msg_data_v01},
  {sizeof(swi_legato_avms_close_nonce_resp_msg_v01), swi_legato_avms_close_nonce_resp_msg_data_v01}
};

/* Range Table */
/* No Ranges Defined in IDL */

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object swi_legato_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *swi_legato_qmi_idl_type_table_object_referenced_tables_v01[] =
{&swi_legato_qmi_idl_type_table_object_v01, &common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object swi_legato_qmi_idl_type_table_object_v01 = {
  sizeof(swi_legato_type_table_v01)/sizeof(qmi_idl_type_table_entry ),
  sizeof(swi_legato_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  swi_legato_type_table_v01,
  swi_legato_message_table_v01,
  swi_legato_qmi_idl_type_table_object_referenced_tables_v01,
  NULL
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry swi_legato_service_command_messages_v01[] = {
  {QMI_SWI_LEGATO_TEST_REQ_V01, QMI_IDL_TYPE16(0, 0), 0},
  {QMI_SWI_LEGATO_AVMS_SESSION_START_REQ_V01, QMI_IDL_TYPE16(0, 2), 4},
  {QMI_SWI_LEGATO_AVMS_SESSION_STOP_REQ_V01, QMI_IDL_TYPE16(0, 4), 4},
  {QMI_SWI_LEGATO_AVMS_SESSION_GETINFO_REQ_V01, QMI_IDL_TYPE16(0, 6), 0},
  {QMI_SWI_LEGATO_AVMS_SELECTION_REQ_V01, QMI_IDL_TYPE16(0, 9), 24},
  {QMI_SWI_LEGATO_AVMS_SET_SETTINGS_REQ_V01, QMI_IDL_TYPE16(0, 13), 155},
  {QMI_SWI_LEGATO_AVMS_GET_SETTINGS_REQ_V01, QMI_IDL_TYPE16(0, 11), 0},
  {QMI_SWI_LEGATO_AVMS_SET_EVENT_REPORT_REQ_V01, QMI_IDL_TYPE16(0, 15), 0},
  {QMI_SWI_LEGATO_AVMS_GET_WAMS_REQ_V01, QMI_IDL_TYPE16(0, 17), 0},
  {QMI_SWI_LEGATO_AVMS_OPEN_NONCE_REQ_V01, QMI_IDL_TYPE16(0, 19), 0},
  {QMI_SWI_LEGATO_AVMS_CLOSE_NONCE_REQ_V01, QMI_IDL_TYPE16(0, 21), 19}
};

static const qmi_idl_service_message_table_entry swi_legato_service_response_messages_v01[] = {
  {QMI_SWI_LEGATO_TEST_RESP_V01, QMI_IDL_TYPE16(0, 1), 7},
  {QMI_SWI_LEGATO_AVMS_SESSION_START_RESP_V01, QMI_IDL_TYPE16(0, 3), 14},
  {QMI_SWI_LEGATO_AVMS_SESSION_STOP_RESP_V01, QMI_IDL_TYPE16(0, 5), 11},
  {QMI_SWI_LEGATO_AVMS_SESSION_GETINFO_RESP_V01, QMI_IDL_TYPE16(0, 7), 1527},
  {QMI_SWI_LEGATO_AVMS_SELECTION_RESP_V01, QMI_IDL_TYPE16(0, 10), 7},
  {QMI_SWI_LEGATO_AVMS_SET_SETTINGS_RESP_V01, QMI_IDL_TYPE16(0, 14), 7},
  {QMI_SWI_LEGATO_AVMS_GET_SETTINGS_RESP_V01, QMI_IDL_TYPE16(0, 12), 169},
  {QMI_SWI_LEGATO_AVMS_SET_EVENT_REPORT_RESP_V01, QMI_IDL_TYPE16(0, 16), 7},
  {QMI_SWI_LEGATO_AVMS_GET_WAMS_RESP_V01, QMI_IDL_TYPE16(0, 18), 480},
  {QMI_SWI_LEGATO_AVMS_OPEN_NONCE_RESP_V01, QMI_IDL_TYPE16(0, 20), 30},
  {QMI_SWI_LEGATO_AVMS_CLOSE_NONCE_RESP_V01, QMI_IDL_TYPE16(0, 22), 7}
};

static const qmi_idl_service_message_table_entry swi_legato_service_indication_messages_v01[] = {
  {QMI_SWI_LEGATO_AVMS_EVENT_IND_V01, QMI_IDL_TYPE16(0, 8), 1551}
};

/*Service Object*/
struct qmi_idl_service_object swi_legato_qmi_idl_service_object_v01 = {
  0x06,
  0x01,
  QMI_SERVICE_SWI_LEGATO_ID_V01,
  1551,
  { sizeof(swi_legato_service_command_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(swi_legato_service_response_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(swi_legato_service_indication_messages_v01)/sizeof(qmi_idl_service_message_table_entry) },
  { swi_legato_service_command_messages_v01, swi_legato_service_response_messages_v01, swi_legato_service_indication_messages_v01},
  &swi_legato_qmi_idl_type_table_object_v01,
  0x01,
  NULL
};

/* Service Object Accessor */
qmi_idl_service_object_type swi_legato_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( SWI_LEGATO_V01_IDL_MAJOR_VERS != idl_maj_version || SWI_LEGATO_V01_IDL_MINOR_VERS != idl_min_version
       || SWI_LEGATO_V01_IDL_TOOL_VERS != library_version)
  {
    return NULL;
  }
  return (qmi_idl_service_object_type)&swi_legato_qmi_idl_service_object_v01;
}

