#ifndef DSD_SERVICE_01_H
#define DSD_SERVICE_01_H
/**
  @file data_system_determination_v01.h

  @brief This is the public header file which defines the dsd service Data structures.

  This header file defines the types and structures that were defined in
  dsd. It contains the constant values defined, enums, structures,
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
  Copyright (c) 2012-2015 Qualcomm Technologies, Inc. All rights reserved.
  Qualcomm Technologies Proprietary and Confidential.


  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.14.5
   It was generated on: Fri May  1 2015 (Spin 0)
   From IDL File: data_system_determination_v01.idl */

/** @defgroup dsd_qmi_consts Constant values defined in the IDL */
/** @defgroup dsd_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup dsd_qmi_enums Enumerated types used in QMI messages */
/** @defgroup dsd_qmi_messages Structures sent as QMI messages */
/** @defgroup dsd_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup dsd_qmi_accessor Accessor for QMI service object */
/** @defgroup dsd_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup dsd_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define DSD_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define DSD_V01_IDL_MINOR_VERS 0x0D
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define DSD_V01_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
#define DSD_V01_MAX_MESSAGE_ID 0x003D
/**
    @}
  */


/** @addtogroup dsd_qmi_consts
    @{
  */
#define QMI_DSD_MAC_ADDR_LEN_V01 6
#define QMI_DSD_IPV6_ADDR_LEN_V01 16
#define QMI_DSD_MAX_AVAIL_SYS_V01 15
#define QMI_DSD_MAX_APNS_V01 15
#define QMI_DSD_MAX_APN_LEN_V01 100
#define QMI_DSD_MAX_SSID_LEN_V01 32
#define QMI_DSD_MAX_SYSTEMS_V01 3
#define QMI_DSD_MAX_CHANNELS_V01 16
#define QMI_DSD_MAX_BSSIDS_V01 16
#define QMI_DSD_MAX_BSSIDS_PER_REPORT_V01 50
#define QMI_DSD_MAX_SSIDS_V01 8
#define QMI_DSD_MAX_NETWORK_MODE_LEN_V01 16
/**
    @}
  */

/** @addtogroup dsd_qmi_enums
    @{
  */
typedef enum {
  DSD_BIND_SUBSCRIPTION_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  DSD_PRIMARY_SUBS_V01 = 0x0001, /**<  Primary \n  */
  DSD_SECONDARY_SUBS_V01 = 0x0002, /**<  Secondary \n  */
  DSD_TERTIARY_SUBS_V01 = 0x0003, /**<  Tertiary  */
  DSD_BIND_SUBSCRIPTION_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}dsd_bind_subscription_enum_v01;
/**
    @}
  */

/** @addtogroup dsd_qmi_enums
    @{
  */
typedef enum {
  DSD_SYS_NETWORK_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  DSD_SYS_NETWORK_3GPP_V01 = 0, /**<  3GPP \n  */
  DSD_SYS_NETWORK_3GPP2_V01 = 1, /**<  3GPP2 \n  */
  DSD_SYS_NETWORK_WLAN_V01 = 2, /**<  WLAN  */
  DSD_SYS_NETWORK_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}dsd_sys_network_enum_v01;
/**
    @}
  */

/** @addtogroup dsd_qmi_enums
    @{
  */
typedef enum {
  DSD_SYS_RAT_EX_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  DSD_SYS_RAT_EX_NULL_BEARER_V01 = 0x0, /**<  NULL bearer  */
  DSD_SYS_RAT_EX_3GPP_WCDMA_V01 = 0x1, /**<  3GPP WCDMA     */
  DSD_SYS_RAT_EX_3GPP_GERAN_V01 = 0x2, /**<  3GPP GERAN  */
  DSD_SYS_RAT_EX_3GPP_LTE_V01 = 0x3, /**<  3GPP LTE  */
  DSD_SYS_RAT_EX_3GPP_TDSCDMA_V01 = 0x4, /**<  3GPP TDSCDMA  */
  DSD_SYS_RAT_EX_3GPP_WLAN_V01 = 0x5, /**<  3GPP WLAN  */
  DSD_SYS_RAT_EX_3GPP_MAX_V01 = 0x64, /**<  3GPP maximum  */
  DSD_SYS_RAT_EX_3GPP2_1X_V01 = 0x65, /**<  3GPP2 1X  */
  DSD_SYS_RAT_EX_3GPP2_HRPD_V01 = 0x66, /**<  3GPP2 HRPD  */
  DSD_SYS_RAT_EX_3GPP2_EHRPD_V01 = 0x67, /**<  3GPP2 EHRPD  */
  DSD_SYS_RAT_EX_3GPP2_WLAN_V01 = 0x68, /**<  3GPP2 WLAN  */
  DSD_SYS_RAT_EX_3GPP2_MAX_V01 = 0xC8, /**<  3GPP2 maximum  */
  DSD_SYS_RAT_EX_WLAN_V01 = 0xC9, /**<  WLAN  */
  DSD_SYS_RAT_EX_WLAN_MAX_V01 = 0x12C, /**<  WLAN maximum  */
  DSD_SYS_RAT_EX_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}dsd_sys_rat_ex_enum_v01;
/**
    @}
  */

typedef uint64_t dsd_sys_so_mask_v01;
#define QMI_DSD_SO_MASK_UNSPECIFIED_V01 ((dsd_sys_so_mask_v01)0x0000000000000000ull)
#define QMI_DSD_3GPP_SO_MASK_WCDMA_V01 ((dsd_sys_so_mask_v01)0x0000000000000001ull)
#define QMI_DSD_3GPP_SO_MASK_HSDPA_V01 ((dsd_sys_so_mask_v01)0x0000000000000002ull)
#define QMI_DSD_3GPP_SO_MASK_HSUPA_V01 ((dsd_sys_so_mask_v01)0x0000000000000004ull)
#define QMI_DSD_3GPP_SO_MASK_HSDPAPLUS_V01 ((dsd_sys_so_mask_v01)0x0000000000000008ull)
#define QMI_DSD_3GPP_SO_MASK_DC_HSDPAPLUS_V01 ((dsd_sys_so_mask_v01)0x0000000000000010ull)
#define QMI_DSD_3GPP_SO_MASK_64_QAM_V01 ((dsd_sys_so_mask_v01)0x0000000000000020ull)
#define QMI_DSD_3GPP_SO_MASK_HSPA_V01 ((dsd_sys_so_mask_v01)0x0000000000000040ull)
#define QMI_DSD_3GPP_SO_MASK_GPRS_V01 ((dsd_sys_so_mask_v01)0x0000000000000080ull)
#define QMI_DSD_3GPP_SO_MASK_EDGE_V01 ((dsd_sys_so_mask_v01)0x0000000000000100ull)
#define QMI_DSD_3GPP_SO_MASK_GSM_V01 ((dsd_sys_so_mask_v01)0x0000000000000200ull)
#define QMI_DSD_3GPP_SO_MASK_S2B_V01 ((dsd_sys_so_mask_v01)0x0000000000000400ull)
#define QMI_DSD_3GPP_SO_MASK_LTE_LIMITED_SRVC_V01 ((dsd_sys_so_mask_v01)0x0000000000000800ull)
#define QMI_DSD_3GPP_SO_MASK_LTE_FDD_V01 ((dsd_sys_so_mask_v01)0x0000000000001000ull)
#define QMI_DSD_3GPP_SO_MASK_LTE_TDD_V01 ((dsd_sys_so_mask_v01)0x0000000000002000ull)
#define QMI_DSD_3GPP_SO_MASK_TDSCDMA_V01 ((dsd_sys_so_mask_v01)0x0000000000004000ull)
#define QMI_DSD_3GPP_SO_MASK_DC_HSUPA_V01 ((dsd_sys_so_mask_v01)0x0000000000008000ull)
#define QMI_DSD_3GPP2_SO_MASK_1X_IS95_V01 ((dsd_sys_so_mask_v01)0x0000000001000000ull)
#define QMI_DSD_3GPP2_SO_MASK_1X_IS2000_V01 ((dsd_sys_so_mask_v01)0x0000000002000000ull)
#define QMI_DSD_3GPP2_SO_MASK_1X_IS2000_REL_A_V01 ((dsd_sys_so_mask_v01)0x0000000004000000ull)
#define QMI_DSD_3GPP2_SO_MASK_HDR_REV0_DPA_V01 ((dsd_sys_so_mask_v01)0x0000000008000000ull)
#define QMI_DSD_3GPP2_SO_MASK_HDR_REVA_DPA_V01 ((dsd_sys_so_mask_v01)0x0000000010000000ull)
#define QMI_DSD_3GPP2_SO_MASK_HDR_REVB_DPA_V01 ((dsd_sys_so_mask_v01)0x0000000020000000ull)
#define QMI_DSD_3GPP2_SO_MASK_HDR_REVA_MPA_V01 ((dsd_sys_so_mask_v01)0x0000000040000000ull)
#define QMI_DSD_3GPP2_SO_MASK_HDR_REVB_MPA_V01 ((dsd_sys_so_mask_v01)0x0000000080000000ull)
#define QMI_DSD_3GPP2_SO_MASK_HDR_REVA_EMPA_V01 ((dsd_sys_so_mask_v01)0x0000000100000000ull)
#define QMI_DSD_3GPP2_SO_MASK_HDR_REVB_EMPA_V01 ((dsd_sys_so_mask_v01)0x0000000200000000ull)
#define QMI_DSD_3GPP2_SO_MASK_HDR_REVB_MMPA_V01 ((dsd_sys_so_mask_v01)0x0000000400000000ull)
#define QMI_DSD_3GPP2_SO_MASK_HDR_EVDO_FMC_V01 ((dsd_sys_so_mask_v01)0x0000000800000000ull)
#define QMI_DSD_3GPP2_SO_MASK_1X_CS_V01 ((dsd_sys_so_mask_v01)0x0000001000000000ull)
/** @addtogroup dsd_qmi_aggregates
    @{
  */
typedef struct {

  dsd_sys_network_enum_v01 technology;
  /**<   Technology type. Values:\n
      - DSD_SYS_NETWORK_3GPP (0) --  3GPP \n
      - DSD_SYS_NETWORK_3GPP2 (1) --  3GPP2 \n
      - DSD_SYS_NETWORK_WLAN (2) --  WLAN
 */

  dsd_sys_rat_ex_enum_v01 rat_value;
  /**<   RAT value. Values:\n
      - DSD_SYS_RAT_EX_NULL_BEARER (0x0) --  NULL bearer
      - DSD_SYS_RAT_EX_3GPP_WCDMA (0x1) --  3GPP WCDMA
      - DSD_SYS_RAT_EX_3GPP_GERAN (0x2) --  3GPP GERAN
      - DSD_SYS_RAT_EX_3GPP_LTE (0x3) --  3GPP LTE
      - DSD_SYS_RAT_EX_3GPP_TDSCDMA (0x4) --  3GPP TDSCDMA
      - DSD_SYS_RAT_EX_3GPP_WLAN (0x5) --  3GPP WLAN
      - DSD_SYS_RAT_EX_3GPP_MAX (0x64) --  3GPP maximum
      - DSD_SYS_RAT_EX_3GPP2_1X (0x65) --  3GPP2 1X
      - DSD_SYS_RAT_EX_3GPP2_HRPD (0x66) --  3GPP2 HRPD
      - DSD_SYS_RAT_EX_3GPP2_EHRPD (0x67) --  3GPP2 EHRPD
      - DSD_SYS_RAT_EX_3GPP2_WLAN (0x68) --  3GPP2 WLAN
      - DSD_SYS_RAT_EX_3GPP2_MAX (0xC8) --  3GPP2 maximum
      - DSD_SYS_RAT_EX_WLAN (0xC9) --  WLAN
      - DSD_SYS_RAT_EX_WLAN_MAX (0x12C) --  WLAN maximum  */

  dsd_sys_so_mask_v01 so_mask;
  /**<   Service Option (SO) mask to indicate the service option or type of
       application.

       An SO mask value of zero indicates that this field is ignored.
       Values: \n
       - 0x00 -- SO Mask Unspecified \n
       3GPP SO Mask: \n
       - 0x01  -- WCDMA \n
       - 0x02  -- HSDPA \n
       - 0x04  -- HSUPA \n
       - 0x08  -- HSDPAPLUS \n
       - 0x10  -- DC HSDPAPLUS \n
       - 0x20  -- 64 QAM \n
       - 0x40  -- HSPA \n
       - 0x80  -- GPRS \n
       - 0x100 -- EDGE \n
       - 0x200 -- GSM \n
       - 0x400 -- S2B \n
       - 0x800 -- LTE Limited Service \n
       - 0x1000 -- LTE FDD \n
       - 0x2000 -- LTE TDD \n
       - 0x4000 -- TDSCDMA \n
       - 0x8000 -- DC HSUPA \n
       3GPP2 SO Mask: \n
       - 0x0001000000  -- 1X IS95 \n
       - 0x0002000000  -- 1X IS2000 \n
       - 0x0004000000  -- 1X IS2000 REL A \n
       - 0x0008000000  -- HDR REV0 DPA \n
       - 0x0010000000  -- HDR REVA DPA \n
       - 0x0020000000  -- HDR REVB DPA \n
       - 0x0040000000  -- HDR REVA MPA \n
       - 0x0080000000  -- HDR REVB MPA \n
       - 0x0100000000  -- HDR REVA EMPA \n
       - 0x0200000000  -- HDR REVB EMPA \n
       - 0x0400000000  -- HDR REVB MMPA \n
       - 0x0800000000  -- HDR EVDO FMC  \n
       - 0x1000000000  -- 1X Circuit Switched */
}dsd_system_status_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dsd_qmi_aggregates
    @{
  */
typedef struct {

  char apn_name[QMI_DSD_MAX_APN_LEN_V01 + 1];
  /**<   APN name. */

  uint32_t apn_avail_sys_len;  /**< Must be set to # of elements in apn_avail_sys */
  dsd_system_status_info_type_v01 apn_avail_sys[QMI_DSD_MAX_AVAIL_SYS_V01];
  /**<  \n Array of all available systems for the APN. The first entry in the
       list contains the preferred system for the APN.
  */
}dsd_apn_avail_sys_info_type_v01;  /* Type */
/**
    @}
  */

typedef uint64_t dsd_wifi_meas_param_mask_v01;
#define QMI_DSD_WIFI_MEAS_PARAM_NONE_V01 ((dsd_wifi_meas_param_mask_v01)0x0000000000000000ull) /**<  No Parameters  */
#define QMI_DSD_WIFI_MEAS_PARAM_RSSI_V01 ((dsd_wifi_meas_param_mask_v01)0x0000000000000001ull) /**<  Received Signal Strength Indication  */
#define QMI_DSD_WIFI_MEAS_PARAM_SINR_V01 ((dsd_wifi_meas_param_mask_v01)0x0000000000000002ull) /**<  Signal to Inteference plus Noise Ratio  */
#define QMI_DSD_WIFI_MEAS_PARAM_BSS_LOAD_V01 ((dsd_wifi_meas_param_mask_v01)0x0000000000000004ull) /**<  Basic Service Set Load  */
#define QMI_DSD_WIFI_MEAS_PARAM_PHY_RATE_V01 ((dsd_wifi_meas_param_mask_v01)0x0000000000000008ull) /**<  Physical Layer tranmission Rate  */
#define QMI_DSD_WIFI_MEAS_PARAM_PACKET_ERROR_RATE_V01 ((dsd_wifi_meas_param_mask_v01)0x0000000000000010ull) /**<  Packet Error Rate  */
/** @addtogroup dsd_qmi_enums
    @{
  */
typedef enum {
  DSD_WIFI_MEAS_REPORT_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  DSD_WIFI_MEAS_REPORT_TYPE_PERIODIC_RAW_V01 = 0, /**<  Report Raw Wifi Measurements values periodically.  */
  DSD_WIFI_MEAS_REPORT_TYPE_ONE_TIME_AFTER_THRESHOLD_V01 = 1, /**<  Report Wifi Measurements once after crossing the threshold.  */
  DSD_WIFI_MEAS_REPORT_TYPE_PERIODIC_AFTER_THRESHOLD_V01 = 2, /**<  Report Wifi Measurements periodically after crossing the threshold.  */
  DSD_WIFI_MEAS_REPORT_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}dsd_wifi_meas_report_type_enum_v01;
/**
    @}
  */

/** @addtogroup dsd_qmi_enums
    @{
  */
typedef enum {
  DSD_WIFI_NETWORK_MODE_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_DSD_WIFI_NETWORK_MODE_802_11_1997_V01 = 0, /**<  Legacy network Mode.  */
  QMI_DSD_WIFI_NETWORK_MODE_802_11a_V01 = 1, /**<  OFDM Network mode.  */
  QMI_DSD_WIFI_NETWORK_MODE_802_11b_V01 = 2, /**<  802.11b  */
  QMI_DSD_WIFI_NETWORK_MODE_802_11g_V01 = 3, /**<  802.11g  */
  QMI_DSD_WIFI_NETWORK_MODE_802_11_2007_V01 = 4, /**<  802.11.2007  */
  QMI_DSD_WIFI_NETWORK_MODE_802_11n_V01 = 5, /**<  802.11n  */
  QMI_DSD_WIFI_NETWORK_MODE_802_11_2012_V01 = 6, /**<  802.11.2012  */
  QMI_DSD_WIFI_NETWORK_MODE_802_11ac_V01 = 7, /**<  802.11.ac  */
  QMI_DSD_WIFI_NETWORK_MODE_802_11ad_V01 = 8, /**<  802.11.ad  */
  QMI_DSD_WIFI_NETWORK_MODE_802_11af_V01 = 9, /**<  802.11.af  */
  QMI_DSD_WIFI_NETWORK_MODE_802_11ah_V01 = 10, /**<  802.11.ah  */
  QMI_DSD_WIFI_NETWORK_MODE_802_11ai_V01 = 11, /**<  802.11.ai  */
  QMI_DSD_WIFI_NETWORK_MODE_802_11aj_V01 = 12, /**<  802.11.aj  */
  QMI_DSD_WIFI_NETWORK_MODE_802_11aq_V01 = 13, /**<  802.11.aq  */
  QMI_DSD_WIFI_NETWORK_MODE_802_11ax_V01 = 14, /**<  802.11.ax  */
  QMI_DSD_WIFI_NETWORK_MODE_802_11ay_V01 = 15, /**<  802.11.ay  */
  DSD_WIFI_NETWORK_MODE_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}dsd_wifi_network_mode_type_enum_v01;
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Request Message; Queries the current system status. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}dsd_get_system_status_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Response Message; Queries the current system status. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members: \n
     - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE \n
     - qmi_error_type  -- Error code. Possible error code values are described in
                          the error codes section of each message definition.
   */

  /* Optional */
  /*  Available Systems */
  uint8_t avail_sys_valid;  /**< Must be set to true if avail_sys is being passed */
  uint32_t avail_sys_len;  /**< Must be set to # of elements in avail_sys */
  dsd_system_status_info_type_v01 avail_sys[QMI_DSD_MAX_AVAIL_SYS_V01];
  /**<   Array of all available systems. The first entry in the list is
       the preferred system.
  */

  /* Optional */
  /*  APN Available System Info */
  uint8_t apn_avail_sys_info_valid;  /**< Must be set to true if apn_avail_sys_info is being passed */
  uint32_t apn_avail_sys_info_len;  /**< Must be set to # of elements in apn_avail_sys_info */
  dsd_apn_avail_sys_info_type_v01 apn_avail_sys_info[QMI_DSD_MAX_APNS_V01];
  /**<  \n Array of all available APNs and their preferred/available systems.
  */
}dsd_get_system_status_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Request Message; Registers the requesting control point for data system status indications. */
typedef struct {

  /* Optional */
  /*  Suppress SO changes */
  uint8_t limit_so_mask_change_ind_valid;  /**< Must be set to true if limit_so_mask_change_ind is being passed */
  uint8_t limit_so_mask_change_ind;
  /**<   If this TLV is provided in the request message, the indication is
       generated only if the rat_value changes from the last indication.
  */
}dsd_system_status_change_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Response Message; Registers the requesting control point for data system status indications. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members: \n
     - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE \n
     - qmi_error_type  -- Error code. Possible error code values are described in
                          the error codes section of each message definition.
            */
}dsd_system_status_change_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Indication Message; Indicates the system status state changes. */
typedef struct {

  /* Optional */
  /*  Available Systems */
  uint8_t avail_sys_valid;  /**< Must be set to true if avail_sys is being passed */
  uint32_t avail_sys_len;  /**< Must be set to # of elements in avail_sys */
  dsd_system_status_info_type_v01 avail_sys[QMI_DSD_MAX_AVAIL_SYS_V01];
  /**<  \n Array of all available systems. The first entry in the list is
       the preferred system.
  */

  /* Optional */
  /*  APN Available System Info */
  uint8_t apn_avail_sys_info_valid;  /**< Must be set to true if apn_avail_sys_info is being passed */
  uint32_t apn_avail_sys_info_len;  /**< Must be set to # of elements in apn_avail_sys_info */
  dsd_apn_avail_sys_info_type_v01 apn_avail_sys_info[QMI_DSD_MAX_APNS_V01];
  /**<  \n Array of all available APNs and their preferred/available systems.
  */
}dsd_system_status_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Request Message; Associates the requesting control point to the subscription requested. */
typedef struct {

  /* Mandatory */
  /*  Bind Subscription */
  dsd_bind_subscription_enum_v01 bind_subs;
  /**<   Subscription to bind to. Values: \n
      - DSD_PRIMARY_SUBS (0x0001) --  Primary \n
      - DSD_SECONDARY_SUBS (0x0002) --  Secondary \n
      - DSD_TERTIARY_SUBS (0x0003) --  Tertiary
 */
}dsd_bind_subscription_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Response Message; Associates the requesting control point to the subscription requested. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members: \n
     - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE \n
     - qmi_error_type  -- Error code. Possible error code values are described in
                          the error codes section of each message definition.
            */
}dsd_bind_subscription_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Request Message; Queries the subscription associated with the control point. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}dsd_get_bind_subscription_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Response Message; Queries the subscription associated with the control point. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.
 Standard response type. Contains the following data members: \n
     - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE \n
     - qmi_error_type  -- Error code. Possible error code values are described in
                          the error codes section of each message definition.
   */

  /* Optional */
  /*  Bound Subscription */
  uint8_t bind_subscription_valid;  /**< Must be set to true if bind_subscription is being passed */
  dsd_bind_subscription_enum_v01 bind_subscription;
  /**<   Values: \n
      - DSD_PRIMARY_SUBS (0x0001) --  Primary \n
      - DSD_SECONDARY_SUBS (0x0002) --  Secondary \n
      - DSD_TERTIARY_SUBS (0x0003) --  Tertiary
 */
}dsd_get_bind_subscription_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_enums
    @{
  */
typedef enum {
  DSD_WQE_STATUS_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  DSD_WQE_INACTIVE_V01 = 0, /**<  Inactive (Default) \n */
  DSD_WQE_ACTIVE_V01 = 1, /**<  Active  */
  DSD_WQE_STATUS_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}dsd_wqe_status_enum_v01;
/**
    @}
  */

/** @addtogroup dsd_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t ipv6_address[QMI_DSD_IPV6_ADDR_LEN_V01];
  /**<   IPV6 address.
   */

  uint8_t prefix_len;
  /**<   IPV6 address prefix length.
   */
}dsd_ipv6_addr_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dsd_qmi_enums
    @{
  */
typedef enum {
  DSD_WIFI_ASSOC_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  DSD_WIFI_ASSOC_P2P_GO_V01 = 0, /**<  Peer to Peer Group Owner wifi association  */
  DSD_WIFI_ASSOC_P2P_CLIENT_V01 = 1, /**<  Peer to Peer Client wifi association  */
  DSD_WIFI_ASSOC_STA_V01 = 2, /**<  Station wifi association  */
  DSD_WIFI_ASSOC_SOFTAP_V01 = 3, /**<  Software Access Point wifi association  */
  DSD_WIFI_ASSOC_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}dsd_wifi_assoc_type_enum_v01;
/**
    @}
  */

/** @addtogroup dsd_qmi_enums
    @{
  */
typedef enum {
  DSD_WIFI_CONN_STATUS_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  DSD_WIFI_CONN_STATUS_DISASSOCIATED_V01 = 0,
  DSD_WIFI_CONN_STATUS_ASSOCIATED_V01 = 1,
  DSD_WIFI_CONN_STATUS_IP_CONNECTED_V01 = 2,
  DSD_WIFI_CONN_STATUS_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}dsd_wifi_conn_status_type_enum_v01;
/**
    @}
  */

/**  Connection status specifying if wifi is
     disassociated from an access point
 Connection status specifying if wifi is
     associated with an access point
 Connection status specifying if wifi is
     IP connected  */
typedef uint64_t dsd_wqe_profile_type_mask_v01;
#define QMI_DSD_WQE_PROFILE_INACTIVE_V01 ((dsd_wqe_profile_type_mask_v01)0x0000000000000000ull) /**<  WiFi Quality Estimation profile inactive  */
#define QMI_DSD_WQE_PROFILE_INTERNET_V01 ((dsd_wqe_profile_type_mask_v01)0x0000000000000001ull) /**<  WiFi Quality Estimation profile internet  */
#define QMI_DSD_WQE_PROFILE_AUDIO_V01 ((dsd_wqe_profile_type_mask_v01)0x0000000000000002ull) /**<  WiFi Quality Estimation profile audio  */
#define QMI_DSD_WQE_PROFILE_VIDEO_V01 ((dsd_wqe_profile_type_mask_v01)0x0000000000000004ull) /**<  WiFi Quality Estimation profile video  */
/** @addtogroup dsd_qmi_messages
    @{
  */
/** Request Message; Indicates if WLAN is available. Provides the IP and
           MAC addresses if WLAN is up. */
typedef struct {

  /* Mandatory */
  /*  WLAN Access Point MAC address. */
  uint8_t wlan_ap_mac_address[QMI_DSD_MAC_ADDR_LEN_V01];
  /**<   WLAN access point MAC address.  */

  /* Optional */
  /*  WLAN IPV4 address. */
  uint8_t wlan_ipv4_address_valid;  /**< Must be set to true if wlan_ipv4_address is being passed */
  uint32_t wlan_ipv4_address;
  /**<   WLAN IPV4 address. */

  /* Optional */
  /*  WLAN IPV6 address. */
  uint8_t wlan_ipv6_address_valid;  /**< Must be set to true if wlan_ipv6_address is being passed */
  dsd_ipv6_addr_type_v01 wlan_ipv6_address;

  /* Optional */
  /*  WiFi Quality Estimation (WQE) status. */
  uint8_t wqe_status_valid;  /**< Must be set to true if wqe_status is being passed */
  dsd_wqe_status_enum_v01 wqe_status;
  /**<   Conveys if WQE was performed on the application
 processor before declaring that WLAN is available. \n
 Values: \n
      - DSD_WQE_INACTIVE (0) --  Inactive (Default) \n
      - DSD_WQE_ACTIVE (1) --  Active
 */

  /* Optional */
  /*  DNS IPV4 address 1. */
  uint8_t dns_ipv4_address_1_valid;  /**< Must be set to true if dns_ipv4_address_1 is being passed */
  uint32_t dns_ipv4_address_1;
  /**<   DNS IPV4 address. */

  /* Optional */
  /*  DNS IPV4 address 2. */
  uint8_t dns_ipv4_address_2_valid;  /**< Must be set to true if dns_ipv4_address_2 is being passed */
  uint32_t dns_ipv4_address_2;
  /**<   DNS IPV4 address. */

  /* Optional */
  /*  DNS IPV6 address 1. */
  uint8_t dns_ipv6_address_1_valid;  /**< Must be set to true if dns_ipv6_address_1 is being passed */
  uint8_t dns_ipv6_address_1[QMI_DSD_IPV6_ADDR_LEN_V01];
  /**<   DNS IPV6 address. */

  /* Optional */
  /*  DNS IPV6 address 2. */
  uint8_t dns_ipv6_address_2_valid;  /**< Must be set to true if dns_ipv6_address_2 is being passed */
  uint8_t dns_ipv6_address_2[QMI_DSD_IPV6_ADDR_LEN_V01];
  /**<   DNS IPV6 address. */

  /* Optional */
  /*  ePDG IPV4 address 1. */
  uint8_t epdg_ipv4_address_1_valid;  /**< Must be set to true if epdg_ipv4_address_1 is being passed */
  uint32_t epdg_ipv4_address_1;
  /**<   ePDG IPV4 address. */

  /* Optional */
  /*  ePDG IPV4 address 2. */
  uint8_t epdg_ipv4_address_2_valid;  /**< Must be set to true if epdg_ipv4_address_2 is being passed */
  uint32_t epdg_ipv4_address_2;
  /**<   ePDG IPV4 address. */

  /* Optional */
  /*  ePDG IPV6 address 1. */
  uint8_t epdg_ipv6_address_1_valid;  /**< Must be set to true if epdg_ipv6_address_1 is being passed */
  uint8_t epdg_ipv6_address_1[QMI_DSD_IPV6_ADDR_LEN_V01];
  /**<   ePDG IPV6 address. */

  /* Optional */
  /*  ePDG IPV6 address 2. */
  uint8_t epdg_ipv6_address_2_valid;  /**< Must be set to true if epdg_ipv6_address_2 is being passed */
  uint8_t epdg_ipv6_address_2[QMI_DSD_IPV6_ADDR_LEN_V01];
  /**<   ePDG IPV6 address. */

  /* Optional */
  /*  SSID. */
  uint8_t ssid_valid;  /**< Must be set to true if ssid is being passed */
  uint32_t ssid_len;  /**< Must be set to # of elements in ssid */
  uint8_t ssid[QMI_DSD_MAX_SSID_LEN_V01];
  /**<   Service Set Identifier. */

  /* Optional */
  /*  Channel */
  uint8_t channel_valid;  /**< Must be set to true if channel is being passed */
  uint16_t channel;
  /**<   Channel in MHz. */

  /* Optional */
  /*  Channel bandwidth */
  uint8_t channel_bandwidth_valid;  /**< Must be set to true if channel_bandwidth is being passed */
  uint16_t channel_bandwidth;
  /**<   Channel bandwith in MHz
  */

  /* Optional */
  /*  WiFi Quality Estimation (WQE)Profile type. */
  uint8_t wqe_profile_type_valid;  /**< Must be set to true if wqe_profile_type is being passed */
  dsd_wqe_profile_type_mask_v01 wqe_profile_type;
  /**<   Conveys if WQE was performed on the Application
 for audio, video or internet data. Values \n
      - QMI_DSD_WQE_PROFILE_INACTIVE (0x0000000000000000) --  WiFi Quality Estimation profile inactive
      - QMI_DSD_WQE_PROFILE_INTERNET (0x0000000000000001) --  WiFi Quality Estimation profile internet
      - QMI_DSD_WQE_PROFILE_AUDIO (0x0000000000000002) --  WiFi Quality Estimation profile audio
      - QMI_DSD_WQE_PROFILE_VIDEO (0x0000000000000004) --  WiFi Quality Estimation profile video
 */

  /* Optional */
  /*  Wifi Association Type */
  uint8_t wifi_assoc_type_valid;  /**< Must be set to true if wifi_assoc_type is being passed */
  dsd_wifi_assoc_type_enum_v01 wifi_assoc_type;
  /**<   Wifi Association type. Values \n
      - DSD_WIFI_ASSOC_P2P_GO (0) --  Peer to Peer Group Owner wifi association
      - DSD_WIFI_ASSOC_P2P_CLIENT (1) --  Peer to Peer Client wifi association
      - DSD_WIFI_ASSOC_STA (2) --  Station wifi association
      - DSD_WIFI_ASSOC_SOFTAP (3) --  Software Access Point wifi association
 */

  /* Optional */
  /*  Network Mode */
  uint8_t network_mode_valid;  /**< Must be set to true if network_mode is being passed */
  dsd_wifi_network_mode_type_enum_v01 network_mode;
  /**<   Enum specifying the network mode for the channel. Values \n
      - QMI_DSD_WIFI_NETWORK_MODE_802_11_1997 (0) --  Legacy network Mode.
      - QMI_DSD_WIFI_NETWORK_MODE_802_11a (1) --  OFDM Network mode.
      - QMI_DSD_WIFI_NETWORK_MODE_802_11b (2) --  802.11b
      - QMI_DSD_WIFI_NETWORK_MODE_802_11g (3) --  802.11g
      - QMI_DSD_WIFI_NETWORK_MODE_802_11_2007 (4) --  802.11.2007
      - QMI_DSD_WIFI_NETWORK_MODE_802_11n (5) --  802.11n
      - QMI_DSD_WIFI_NETWORK_MODE_802_11_2012 (6) --  802.11.2012
      - QMI_DSD_WIFI_NETWORK_MODE_802_11ac (7) --  802.11.ac
      - QMI_DSD_WIFI_NETWORK_MODE_802_11ad (8) --  802.11.ad
      - QMI_DSD_WIFI_NETWORK_MODE_802_11af (9) --  802.11.af
      - QMI_DSD_WIFI_NETWORK_MODE_802_11ah (10) --  802.11.ah
      - QMI_DSD_WIFI_NETWORK_MODE_802_11ai (11) --  802.11.ai
      - QMI_DSD_WIFI_NETWORK_MODE_802_11aj (12) --  802.11.aj
      - QMI_DSD_WIFI_NETWORK_MODE_802_11aq (13) --  802.11.aq
      - QMI_DSD_WIFI_NETWORK_MODE_802_11ax (14) --  802.11.ax
      - QMI_DSD_WIFI_NETWORK_MODE_802_11ay (15) --  802.11.ay
 */

  /* Optional */
  /*  Connection status */
  uint8_t connection_status_valid;  /**< Must be set to true if connection_status is being passed */
  dsd_wifi_conn_status_type_enum_v01 connection_status;
  /**<   Wifi Connection status type. Values \n
      - DSD_WIFI_CONN_STATUS_DISASSOCIATED (0) --
      - DSD_WIFI_CONN_STATUS_ASSOCIATED (1) --
      - DSD_WIFI_CONN_STATUS_IP_CONNECTED (2) --
 */
}dsd_wlan_available_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Response Message; Indicates if WLAN is available. Provides the IP and
           MAC addresses if WLAN is up. */
typedef struct {

  /* Mandatory */
  /*  Result Code. */
  qmi_response_type_v01 resp;
  /**<   Standard response type.     */
}dsd_wlan_available_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Request Message; Indicates if WLAN is not available.  */
typedef struct {

  /* Optional */
  /*  WiFi Quality Estimation (WQE) status. */
  uint8_t wqe_status_valid;  /**< Must be set to true if wqe_status is being passed */
  dsd_wqe_status_enum_v01 wqe_status;
  /**<   Conveys if WQE was performed on the Application
 Processor before declaring WLAN is unavailable. \n
 Values: \n
      - DSD_WQE_INACTIVE (0) --  Inactive (Default) \n
      - DSD_WQE_ACTIVE (1) --  Active
 */
}dsd_wlan_not_available_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Response Message; Indicates if WLAN is not available.  */
typedef struct {

  /* Mandatory */
  /*  Result Code. */
  qmi_response_type_v01 resp;
  /**<   Standard response type.   */
}dsd_wlan_not_available_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_enums
    @{
  */
typedef enum {
  DSD_WLAN_PREFERENCE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  DSD_WLAN_LB_PREFERRED_V01 = 0, /**<  WLAN Local Breakout is preferred.  */
  DSD_WLAN_LB_NOT_PREFERRED_V01 = 1, /**<  WLAN Local Breakout is not preferred.  */
  DSD_WLAN_PREFERENCE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}dsd_wlan_preference_enum_v01;
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Request Message; Sets the preference for WLAN  */
typedef struct {

  /* Mandatory */
  /*  WLAN preference. */
  dsd_wlan_preference_enum_v01 wlan_preference;
  /**<   Preference for WLAN. \n
 Values: \n
      - DSD_WLAN_LB_PREFERRED (0) --  WLAN Local Breakout is preferred.
      - DSD_WLAN_LB_NOT_PREFERRED (1) --  WLAN Local Breakout is not preferred.
 */
}dsd_set_wlan_preference_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Response Message; Sets the preference for WLAN  */
typedef struct {

  /* Mandatory */
  /*  Result code. */
  qmi_response_type_v01 resp;
  /**<   Standard response type.       */
}dsd_set_wlan_preference_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Request Message; Gets the preference for WLAN  */
typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}dsd_get_wlan_preference_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Response Message; Gets the preference for WLAN  */
typedef struct {

  /* Mandatory */
  /*  Result code. */
  qmi_response_type_v01 resp;
  /**<   Standard response type.       */

  /* Optional */
  /*  WLAN preference. */
  uint8_t wlan_preference_valid;  /**< Must be set to true if wlan_preference is being passed */
  dsd_wlan_preference_enum_v01 wlan_preference;
  /**<   Preference for WLAN. \n
 Values: \n
      - DSD_WLAN_LB_PREFERRED (0) --  WLAN Local Breakout is preferred.
      - DSD_WLAN_LB_NOT_PREFERRED (1) --  WLAN Local Breakout is not preferred.
 */
}dsd_get_wlan_preference_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_enums
    @{
  */
typedef enum {
  DSD_APN_PREF_SYS_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  DSD_APN_PREF_SYS_WWAN_V01 = 0, /**<  WWAN is preferred.  */
  DSD_APN_PREF_SYS_WLAN_V01 = 1, /**<  WLAN is preferred.  */
  DSD_APN_PREF_SYS_IWLAN_V01 = 2, /**<  IWLAN is preferred.  */
  DSD_APN_PREF_SYS_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}dsd_apn_pref_sys_enum_v01;
/**
    @}
  */

/** @addtogroup dsd_qmi_aggregates
    @{
  */
typedef struct {

  char apn_name[QMI_DSD_MAX_APN_LEN_V01 + 1];
  /**<   APN name. */

  dsd_apn_pref_sys_enum_v01 pref_sys;
  /**<   Preferred system for the APN. Values: \n
      - DSD_APN_PREF_SYS_WWAN (0) --  WWAN is preferred.
      - DSD_APN_PREF_SYS_WLAN (1) --  WLAN is preferred.
      - DSD_APN_PREF_SYS_IWLAN (2) --  IWLAN is preferred.
 */
}dsd_apn_pref_sys_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Request Message; Sets the highest technology preference for an APN. */
typedef struct {

  /* Mandatory */
  /*  APN Preferred System. */
  dsd_apn_pref_sys_type_v01 apn_pref_sys;
  /**<   Highest technology preference for an APN. \n
  */
}dsd_set_apn_preferred_system_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Response Message; Sets the highest technology preference for an APN. */
typedef struct {

  /* Mandatory */
  /*  Result code. */
  qmi_response_type_v01 resp;
  /**<   Standard response type.       */
}dsd_set_apn_preferred_system_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_enums
    @{
  */
typedef enum {
  DSD_MODEM_POWER_COST_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  DSD_MODEM_POWER_COST_NOT_SUPPORTED_V01 = 0, /**<  Not supported.  */
  DSD_MODEM_POWER_COST_LOW_V01 = 1, /**<  Low.  */
  DSD_MODEM_POWER_COST_MEDIUM_V01 = 2, /**<  Medium.  */
  DSD_MODEM_POWER_COST_HIGH_V01 = 3, /**<  High.  */
  DSD_MODEM_POWER_COST_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}dsd_modem_power_cost_enum_v01;
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Request Message; Queries for the cost of modem power. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}dsd_get_modem_power_cost_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Response Message; Queries for the cost of modem power. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.       */

  /* Optional */
  /*  Modem Power Cost */
  uint8_t power_cost_valid;  /**< Must be set to true if power_cost is being passed */
  dsd_modem_power_cost_enum_v01 power_cost;
  /**<   Modem power cost. \n
  */
}dsd_get_modem_power_cost_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Request Message; Starts a transaction to access the PDN policy database. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}dsd_pdn_policy_start_txn_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Response Message; Starts a transaction to access the PDN policy database. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.       */

  /* Optional */
  /*  Transaction ID */
  uint8_t txn_id_valid;  /**< Must be set to true if txn_id is being passed */
  uint32_t txn_id;
  /**<   Handle to a new transaction. \n
  */
}dsd_pdn_policy_start_txn_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_enums
    @{
  */
typedef enum {
  DSD_PDN_POLICY_OVERRIDE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  DSD_PDN_POLICY_OVERRIDE_NONE_V01 = 0, /**<  No override (default). */
  DSD_PDN_POLICY_OVERRIDE_API_V01 = 1, /**<  API override.  */
  DSD_PDN_POLICY_OVERRIDE_OPTION_V01 = 2, /**<  UI option override.  */
  DSD_PDN_POLICY_OVERRIDE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}dsd_pdn_policy_override_enum_v01;
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Request Message; Adds an entry to the PDN policy database. */
typedef struct {

  /* Mandatory */
  /*  Transaction ID */
  uint32_t txn_id;
  /**<   Transaction ID handle obtained using QMI_DSD_PDN_POLICY_START_ TXN.
  */

  /* Mandatory */
  /*  APN Name */
  char apn_name[QMI_DSD_MAX_APN_LEN_V01 + 1];
  /**<   The APN uniquely identifies each entry.
  */

  /* Mandatory */
  /*  Supported System Priority List */
  uint32_t supported_system_priority_list_len;  /**< Must be set to # of elements in supported_system_priority_list */
  dsd_apn_pref_sys_enum_v01 supported_system_priority_list[QMI_DSD_MAX_SYSTEMS_V01];
  /**<   Supported systems for the APN in decreasing order of priority.\n
      - DSD_APN_PREF_SYS_WWAN (0) --  WWAN is preferred.
      - DSD_APN_PREF_SYS_WLAN (1) --  WLAN is preferred.
      - DSD_APN_PREF_SYS_IWLAN (2) --  IWLAN is preferred.
 */

  /* Optional */
  /*  Is Default */
  uint8_t is_default_valid;  /**< Must be set to true if is_default is being passed */
  uint8_t is_default;
  /**<   Specifies whether this is the default APN. Values: \n
       - 0 -- FALSE (Default value when TLV is absent) \n
       - 1 -- TRUE
  */

  /* Optional */
  /*  Override Type */
  uint8_t override_type_valid;  /**< Must be set to true if override_type is being passed */
  dsd_pdn_policy_override_enum_v01 override_type;
  /**<   Specifies whether an override is possible for the preferred system of the PDN.
 Values: \n
      - DSD_PDN_POLICY_OVERRIDE_NONE (0) --  No override (default).
      - DSD_PDN_POLICY_OVERRIDE_API (1) --  API override.
      - DSD_PDN_POLICY_OVERRIDE_OPTION (2) --  UI option override.
 */
}dsd_add_pdn_policy_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Response Message; Adds an entry to the PDN policy database. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.       */
}dsd_add_pdn_policy_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Request Message; Modifies an existing entry in the PDN policy database. */
typedef struct {

  /* Mandatory */
  /*  Transaction ID */
  uint32_t txn_id;
  /**<   Transaction ID handle obtained using QMI_DSD_PDN_POLICY_START_ TXN.
  */

  /* Mandatory */
  /*  APN Name */
  char apn_name[QMI_DSD_MAX_APN_LEN_V01 + 1];
  /**<   The APN that uniquely identifies each entry.
  */

  /* Optional */
  /*  Supported System Priority List */
  uint8_t supported_system_priority_list_valid;  /**< Must be set to true if supported_system_priority_list is being passed */
  uint32_t supported_system_priority_list_len;  /**< Must be set to # of elements in supported_system_priority_list */
  dsd_apn_pref_sys_enum_v01 supported_system_priority_list[QMI_DSD_MAX_SYSTEMS_V01];
  /**<   Supported systems for the APN in decreasing order of priority.\n
      - DSD_APN_PREF_SYS_WWAN (0) --  WWAN is preferred.
      - DSD_APN_PREF_SYS_WLAN (1) --  WLAN is preferred.
      - DSD_APN_PREF_SYS_IWLAN (2) --  IWLAN is preferred.
 */

  /* Optional */
  /*  Is Default */
  uint8_t is_default_valid;  /**< Must be set to true if is_default is being passed */
  uint8_t is_default;
  /**<   Specifies whether this is the default APN. Values: \n
       - 0 -- FALSE  \n
       - 1 -- TRUE
  */

  /* Optional */
  /*  Override Type */
  uint8_t override_type_valid;  /**< Must be set to true if override_type is being passed */
  dsd_pdn_policy_override_enum_v01 override_type;
  /**<   Specifies whether an override is possible for the preferred system of the PDN.
 Values: \n
      - DSD_PDN_POLICY_OVERRIDE_NONE (0) --  No override (default).
      - DSD_PDN_POLICY_OVERRIDE_API (1) --  API override.
      - DSD_PDN_POLICY_OVERRIDE_OPTION (2) --  UI option override.
 */
}dsd_modify_pdn_policy_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Response Message; Modifies an existing entry in the PDN policy database. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.       */
}dsd_modify_pdn_policy_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Request Message; Deletes an entry in the PDN policy database. */
typedef struct {

  /* Mandatory */
  /*  Transaction ID */
  uint32_t txn_id;
  /**<   Transaction ID handle obtained using QMI_DSD_PDN_POLICY_START_ TXN.
  */

  /* Mandatory */
  /*  APN Name */
  char apn_name[QMI_DSD_MAX_APN_LEN_V01 + 1];
  /**<   The APN that uniquely identifies each entry.
  */
}dsd_delete_pdn_policy_by_apn_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Response Message; Deletes an entry in the PDN policy database. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.       */
}dsd_delete_pdn_policy_by_apn_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_aggregates
    @{
  */
typedef struct {

  char apn_name[QMI_DSD_MAX_APN_LEN_V01 + 1];
  /**<   APN name.
   */
}dsd_apn_name_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Request Message; Retrieves a list of APNs from the PDN policy database. */
typedef struct {

  /* Mandatory */
  /*  Transaction ID */
  uint32_t txn_id;
  /**<   Transaction ID handle obtained using QMI_DSD_PDN_POLICY_START_ TXN.
  */
}dsd_get_pdn_policy_apn_list_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Response Message; Retrieves a list of APNs from the PDN policy database. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.     */

  /* Optional */
  /*  APN List */
  uint8_t apn_list_valid;  /**< Must be set to true if apn_list is being passed */
  uint32_t apn_list_len;  /**< Must be set to # of elements in apn_list */
  dsd_apn_name_type_v01 apn_list[QMI_DSD_MAX_APNS_V01];
  /**<   \n List of APNs.
  */
}dsd_get_pdn_policy_apn_list_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Request Message; Retrieves settings for a particular APN from the PDN policy database. */
typedef struct {

  /* Mandatory */
  /*  Transaction ID */
  uint32_t txn_id;
  /**<   Transaction ID handle obtained using QMI_DSD_PDN_POLICY_START_ TXN.
  */

  /* Mandatory */
  /*  APN Name */
  char apn_name[QMI_DSD_MAX_APN_LEN_V01 + 1];
  /**<   The APN that uniquely identifies each entry.
  */
}dsd_get_pdn_policy_settings_for_apn_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Response Message; Retrieves settings for a particular APN from the PDN policy database. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.     */

  /* Optional */
  /*  APN Name */
  uint8_t apn_name_valid;  /**< Must be set to true if apn_name is being passed */
  char apn_name[QMI_DSD_MAX_APN_LEN_V01 + 1];
  /**<   The APN that uniquely identifies each entry.
  */

  /* Optional */
  /*  Supported System Priority List */
  uint8_t supported_system_priority_list_valid;  /**< Must be set to true if supported_system_priority_list is being passed */
  uint32_t supported_system_priority_list_len;  /**< Must be set to # of elements in supported_system_priority_list */
  dsd_apn_pref_sys_enum_v01 supported_system_priority_list[QMI_DSD_MAX_SYSTEMS_V01];
  /**<   Supported systems for the APN in decreasing order of priority.\n
      - DSD_APN_PREF_SYS_WWAN (0) --  WWAN is preferred.
      - DSD_APN_PREF_SYS_WLAN (1) --  WLAN is preferred.
      - DSD_APN_PREF_SYS_IWLAN (2) --  IWLAN is preferred.
 */

  /* Optional */
  /*  Is Default */
  uint8_t is_default_valid;  /**< Must be set to true if is_default is being passed */
  uint8_t is_default;
  /**<   Specifies whether this is the default APN. Values: \n
       - 0 -- FALSE  \n
       - 1 -- TRUE
  */

  /* Optional */
  /*  Override Type */
  uint8_t override_type_valid;  /**< Must be set to true if override_type is being passed */
  dsd_pdn_policy_override_enum_v01 override_type;
  /**<   Specifies whether an override is possible for the preferred system of the PDN.
 Values: \n
      - DSD_PDN_POLICY_OVERRIDE_NONE (0) --  No override (default).
      - DSD_PDN_POLICY_OVERRIDE_API (1) --  API override.
      - DSD_PDN_POLICY_OVERRIDE_OPTION (2) --  UI option override.
 */
}dsd_get_pdn_policy_settings_for_apn_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_enums
    @{
  */
typedef enum {
  DSD_PDN_POLICY_END_TRANSACTION_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  DSD_PDN_POLICY_TRANSACTION_COMMIT_V01 = 0, /**<  Commit the transaction changes to persistent storage.  */
  DSD_PDN_POLICY_TRANSACTION_CANCEL_V01 = 1, /**<  Rollback the transaction changes.  */
  DSD_PDN_POLICY_END_TRANSACTION_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}dsd_pdn_policy_end_transaction_enum_v01;
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Request Message; Ends the outstanding transaction on the PDN policy database. */
typedef struct {

  /* Mandatory */
  /*  Transaction ID */
  uint32_t txn_id;
  /**<   Transaction ID handle obtained using QMI_DSD_PDN_POLICY_START_ TXN.
  */

  /* Mandatory */
  /*  Txn Exec Type */
  dsd_pdn_policy_end_transaction_enum_v01 txn_exec_type;
  /**<   Either commit or cancel the transaction changes. Values: \n
      - DSD_PDN_POLICY_TRANSACTION_COMMIT (0) --  Commit the transaction changes to persistent storage.
      - DSD_PDN_POLICY_TRANSACTION_CANCEL (1) --  Rollback the transaction changes.
 */
}dsd_pdn_policy_end_txn_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Response Message; Ends the outstanding transaction on the PDN policy database. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.     */
}dsd_pdn_policy_end_txn_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_enums
    @{
  */
typedef enum {
  DSD_APN_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  DSD_APN_TYPE_DEFAULT_V01 = 0, /**<  APN type for Default/Internet traffic  */
  DSD_APN_TYPE_IMS_V01 = 1, /**<  APN type for IMS  */
  DSD_APN_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}dsd_apn_type_enum_v01;
/**
    @}
  */

/** @addtogroup dsd_qmi_aggregates
    @{
  */
typedef struct {

  dsd_apn_type_enum_v01 apn_type;
  /**<   APN type.
  */

  char apn_name[QMI_DSD_MAX_APN_LEN_V01 + 1];
  /**<   APN name.
   */
}dsd_apn_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Request Message; Sets the APN information. */
typedef struct {

  /* Mandatory */
  /*  APN Information */
  dsd_apn_info_type_v01 apn_info;

  /* Optional */
  /*  APN Invalid Flag */
  uint8_t apn_invalid_valid;  /**< Must be set to true if apn_invalid is being passed */
  uint8_t apn_invalid;
  /**<   Values: \n
       - 0 -- FALSE = APN is valid \n
       - 1 -- TRUE = APN is invalid \n
  */
}dsd_set_apn_info_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Response Message; Sets the APN information. */
typedef struct {

  /* Mandatory */
  /*  Result code. */
  qmi_response_type_v01 resp;
  /**<   Standard response type.       */
}dsd_set_apn_info_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Request Message; Gets the APN information. */
typedef struct {

  /* Mandatory */
  /*  APN type enum */
  dsd_apn_type_enum_v01 apn_type;
  /**<   APN type enum. Values : \n
      - DSD_APN_TYPE_DEFAULT (0) --  APN type for Default/Internet traffic
      - DSD_APN_TYPE_IMS (1) --  APN type for IMS
 */
}dsd_get_apn_info_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Response Message; Gets the APN information. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.     */

  /* Optional */
  /*  APN Name */
  uint8_t apn_name_valid;  /**< Must be set to true if apn_name is being passed */
  char apn_name[QMI_DSD_MAX_APN_LEN_V01 + 1];
  /**<   APN name.
   */
}dsd_get_apn_info_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_enums
    @{
  */
typedef enum {
  DSD_RAT_PREFERENCE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  DSD_RAT_PREFERENCE_INACTIVE_V01 = 0, /**<  RAT preference not set \n  */
  DSD_RAT_PREFERENCE_CELLULAR_ONLY_V01 = 1, /**<  Cellular Only \n  */
  DSD_RAT_PREFERENCE_WIFI_ONLY_V01 = 2, /**<  Wifi only \n  */
  DSD_RAT_PREFERENCE_CELLULAR_PREFERRED_V01 = 3, /**<  Cellular preferred \n  */
  DSD_RAT_PREFERENCE_WIFI_PREFERRED_V01 = 4, /**<  Wifi preferred  */
  DSD_RAT_PREFERENCE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}dsd_rat_preference_enum_v01;
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Request Message; Configures the data setting. */
typedef struct {

  /* Optional */
  /*  Data Service ON/OFF Switch */
  uint8_t data_service_switch_valid;  /**< Must be set to true if data_service_switch is being passed */
  uint8_t data_service_switch;
  /**<   Specifies whether data is ON/OFF \n
       Values: \n
       - 0 -- FALSE = Data Service is OFF \n
       - 1 -- TRUE = Data Service is ON
  */

  /* Optional */
  /*  Data Service Roaming ON/OFF Switch */
  uint8_t data_service_roaming_switch_valid;  /**< Must be set to true if data_service_roaming_switch is being passed */
  uint8_t data_service_roaming_switch;
  /**<   Specifies whether data roaming is ON/OFF. Values: \n
       - 0 -- FALSE = Data Service Roaming is OFF \n
       - 1 -- TRUE = Data Service Roaming is ON
  */

  /* Optional */
  /*  Radio Access Technology preference  */
  uint8_t rat_preference_valid;  /**< Must be set to true if rat_preference is being passed */
  dsd_rat_preference_enum_v01 rat_preference;
  /**<   Indicates RAT preference \n
 Values: \n
      - DSD_RAT_PREFERENCE_INACTIVE (0) --  RAT preference not set \n
      - DSD_RAT_PREFERENCE_CELLULAR_ONLY (1) --  Cellular Only \n
      - DSD_RAT_PREFERENCE_WIFI_ONLY (2) --  Wifi only \n
      - DSD_RAT_PREFERENCE_CELLULAR_PREFERRED (3) --  Cellular preferred \n
      - DSD_RAT_PREFERENCE_WIFI_PREFERRED (4) --  Wifi preferred
 */
}dsd_notify_data_settings_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Response Message; Configures the data setting. */
typedef struct {

  /* Mandatory */
  /*  Result code. */
  qmi_response_type_v01 resp;
  /**<   Standard response type.       */
}dsd_notify_data_settings_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Request Message; Queries the data settings. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}dsd_get_data_settings_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Response Message; Queries the data settings. */
typedef struct {

  /* Mandatory */
  /*  Result code. */
  qmi_response_type_v01 resp;
  /**<   Standard response type.       */

  /* Optional */
  /*  Data Service ON/OFF Switch */
  uint8_t data_service_switch_valid;  /**< Must be set to true if data_service_switch is being passed */
  uint8_t data_service_switch;
  /**<   Specifies whether data service is ON/OFF. \n
       Values: \n
       - 0 -- TRUE = Data Service is OFF \n
       - 1 -- TRUE = Data Service is ON
  */

  /* Optional */
  /*  Data Service Roaming ON/OFF Switch */
  uint8_t data_service_roaming_switch_valid;  /**< Must be set to true if data_service_roaming_switch is being passed */
  uint8_t data_service_roaming_switch;
  /**<   Specifies whether data roaming is ON/OFF. \n
       Values: \n
       - 0 -- TRUE = Data Service Roaming is OFF \n
       - 1 -- TRUE = Data Service Roaming is ON
  */

  /* Optional */
  /*  Radio Access Technology Preference  */
  uint8_t rat_preference_valid;  /**< Must be set to true if rat_preference is being passed */
  dsd_rat_preference_enum_v01 rat_preference;
  /**<   RAT preference \n
 Values: \n
      - DSD_RAT_PREFERENCE_INACTIVE (0) --  RAT preference not set \n
      - DSD_RAT_PREFERENCE_CELLULAR_ONLY (1) --  Cellular Only \n
      - DSD_RAT_PREFERENCE_WIFI_ONLY (2) --  Wifi only \n
      - DSD_RAT_PREFERENCE_CELLULAR_PREFERRED (3) --  Cellular preferred \n
      - DSD_RAT_PREFERENCE_WIFI_PREFERRED (4) --  Wifi preferred
 */
}dsd_get_data_settings_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_enums
    @{
  */
typedef enum {
  DSD_THERMAL_MITIGATION_ACTION_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  DSD_THERMAL_MITIGATION_ACTION_ALL_CALLS_ALLOWED_V01 = 0, /**<  All calls allowed \n  */
  DSD_THERMAL_MITIGATION_ACTION_IMS_CALLS_ONLY_V01 = 1, /**<  IMS calls only \n  */
  DSD_THERMAL_MITIGATION_ACTION_NO_CALLS_ALLOWED_V01 = 2, /**<  Data calls not allowed  */
  DSD_THERMAL_MITIGATION_ACTION_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}dsd_thermal_mitigation_action_enum_v01;
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Indication Message; Indicates the thermal info changes. */
typedef struct {

  /* Optional */
  /*  Thermal Mitigation Info */
  uint8_t thermal_action_valid;  /**< Must be set to true if thermal_action is being passed */
  dsd_thermal_mitigation_action_enum_v01 thermal_action;
  /**<   Thermal mitigation action information.
      - DSD_THERMAL_MITIGATION_ACTION_ALL_CALLS_ALLOWED (0) --  All calls allowed \n
      - DSD_THERMAL_MITIGATION_ACTION_IMS_CALLS_ONLY (1) --  IMS calls only \n
      - DSD_THERMAL_MITIGATION_ACTION_NO_CALLS_ALLOWED (2) --  Data calls not allowed
 */
}dsd_thermal_info_change_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Request Message; Queries the current thermal mitigation information. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}dsd_get_thermal_mitigation_info_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Response Message; Queries the current thermal mitigation information. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.
 Standard response type. Contains the following data members: \n
     - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE \n
     - qmi_error_type  -- Error code. Possible error code values are described in
                          the error codes section of each message definition.
   */

  /* Optional */
  /*  Thermal Mitigation Info */
  uint8_t thermal_action_valid;  /**< Must be set to true if thermal_action is being passed */
  dsd_thermal_mitigation_action_enum_v01 thermal_action;
  /**<   Thermal mitigation action information.
      - DSD_THERMAL_MITIGATION_ACTION_ALL_CALLS_ALLOWED (0) --  All calls allowed \n
      - DSD_THERMAL_MITIGATION_ACTION_IMS_CALLS_ONLY (1) --  IMS calls only \n
      - DSD_THERMAL_MITIGATION_ACTION_NO_CALLS_ALLOWED (2) --  Data calls not allowed
 */
}dsd_get_thermal_mitigation_info_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Request Message; Registers the requesting control point for various indications. */
typedef struct {

  /* Optional */
  /*  Thermal Mitigation Info */
  uint8_t report_thermal_info_changes_valid;  /**< Must be set to true if report_thermal_info_changes is being passed */
  uint8_t report_thermal_info_changes;
  /**<   Values: \n
	  - 0 -- Do not report \n
	  - 1 -- Report thermal info change.
  */

  /* Optional */
  /*  WiFi Measurement Trigger */
  uint8_t report_wifi_meas_trigger_valid;  /**< Must be set to true if report_wifi_meas_trigger is being passed */
  uint8_t report_wifi_meas_trigger;
  /**<   Values: \n
	  - 0 -- Do not report \n
	  - 1 -- Report WiFi measurement trigger.
  */
}dsd_indication_register_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Response Message; Registers the requesting control point for various indications. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members: \n
     - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE \n
     - qmi_error_type  -- Error code. Possible error code values are described in
                          the error codes section of each message definition.
            */
}dsd_indication_register_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t bssid[QMI_DSD_MAC_ADDR_LEN_V01];
  /**<   WLAN access point MAC address.  */

  uint32_t channel_list_len;  /**< Must be set to # of elements in channel_list */
  uint16_t channel_list[QMI_DSD_MAX_CHANNELS_V01];
}dsd_wifi_ident_bssid_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dsd_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t ssid_len;  /**< Must be set to # of elements in ssid */
  uint8_t ssid[QMI_DSD_MAX_SSID_LEN_V01];
  /**<   Service Set Identifier.  */

  uint8_t is_hidden;
  /**<   Specifies whether Service Set Identifier is hidden or not
       Values: \n
       - 0 -- FALSE = SSID is not hidden \n
       - 1 -- TRUE  = SSID is hidden \n
  */

  uint32_t bssid_list_len;  /**< Must be set to # of elements in bssid_list */
  dsd_wifi_ident_bssid_type_v01 bssid_list[QMI_DSD_MAX_BSSIDS_V01];
}dsd_wifi_ident_ssid_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dsd_qmi_aggregates
    @{
  */
typedef struct {

  int16_t rssi_high;

  int16_t rssi_low;
}dsd_wifi_ident_rssi_threshold_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dsd_qmi_aggregates
    @{
  */
typedef struct {

  int16_t sinr_high;

  int16_t sinr_low;
}dsd_wifi_ident_sinr_threshold_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dsd_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t station_count_high;
  /**<   Station count high threshold. */

  uint16_t station_count_low;
  /**<   Station count low threshold.  */

  uint16_t channel_utilization_high;
  /**<   Channel utilization high threshold.  */

  uint16_t channel_utilization_low;
  /**<   Channel utilization low threshold. */

  uint16_t available_admission_capacity_high;
  /**<   Available Admission Capacity high threshold. */

  uint16_t available_admission_capacity_low;
  /**<   Available Admission Capacity low threshold.  */
}dsd_wifi_ident_bss_load_threshold_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Indication Message; Indicates that the control point needs to start sending WiFi measurements. */
typedef struct {

  /* Mandatory */
  /*  WiFi Measurement ID */
  uint32_t wifi_meas_id;
  /**<   ID used to identify this particular start indication.
  */

  /* Mandatory */
  /*  Measurement Parameters */
  dsd_wifi_meas_param_mask_v01 meas_param;
  /**<   Bitmask of parameters for which measurements are to be captured. Values \n
      - QMI_DSD_WIFI_MEAS_PARAM_NONE (0x0000000000000000) --  No Parameters
      - QMI_DSD_WIFI_MEAS_PARAM_RSSI (0x0000000000000001) --  Received Signal Strength Indication
      - QMI_DSD_WIFI_MEAS_PARAM_SINR (0x0000000000000002) --  Signal to Inteference plus Noise Ratio
      - QMI_DSD_WIFI_MEAS_PARAM_BSS_LOAD (0x0000000000000004) --  Basic Service Set Load
      - QMI_DSD_WIFI_MEAS_PARAM_PHY_RATE (0x0000000000000008) --  Physical Layer tranmission Rate
      - QMI_DSD_WIFI_MEAS_PARAM_PACKET_ERROR_RATE (0x0000000000000010) --  Packet Error Rate
 */

  /* Mandatory */
  /*  Report Type */
  dsd_wifi_meas_report_type_enum_v01 report_type;
  /**<   Specifies the type of wifi measurement report to be captured. Values \n
      - DSD_WIFI_MEAS_REPORT_TYPE_PERIODIC_RAW (0) --  Report Raw Wifi Measurements values periodically.
      - DSD_WIFI_MEAS_REPORT_TYPE_ONE_TIME_AFTER_THRESHOLD (1) --  Report Wifi Measurements once after crossing the threshold.
      - DSD_WIFI_MEAS_REPORT_TYPE_PERIODIC_AFTER_THRESHOLD (2) --  Report Wifi Measurements periodically after crossing the threshold.
 */

  /* Optional */
  /*  RSSI Threshold */
  uint8_t rssi_threshold_valid;  /**< Must be set to true if rssi_threshold is being passed */
  dsd_wifi_ident_rssi_threshold_type_v01 rssi_threshold;
  /**<   Contains the threshold values for received Signal Strength
       Indication value for the channel above or below which TE sends
       measurement reports.
  */

  /* Optional */
  /*  SINR Threshold */
  uint8_t sinr_threshold_valid;  /**< Must be set to true if sinr_threshold is being passed */
  dsd_wifi_ident_sinr_threshold_type_v01 sinr_threshold;
  /**<   Contains the threshold values for Signal Interference
       to noise ratio value for the channel above or below which TE sends
       measurement reports.
  */

  /* Optional */
  /*  BSS Load Threshold */
  uint8_t bss_load_threshold_valid;  /**< Must be set to true if bss_load_threshold is being passed */
  dsd_wifi_ident_bss_load_threshold_type_v01 bss_load_threshold;
  /**<   Contains the threshold values for all BSS load values above or
       below which TE sends measurement reports.
  */

  /* Optional */
  /*  Identifying Parameter List */
  uint8_t ident_param_list_valid;  /**< Must be set to true if ident_param_list is being passed */
  uint32_t ident_param_list_len;  /**< Must be set to # of elements in ident_param_list */
  dsd_wifi_ident_ssid_type_v01 ident_param_list[QMI_DSD_MAX_SSIDS_V01];
  /**<   Contains SSIDs, BSSIDs for each SSID and channel for each BSSID, for which
       measurements are desired.
  */

  /* Optional */
  /*  Sampling Timer */
  uint8_t sampling_timer_valid;  /**< Must be set to true if sampling_timer is being passed */
  uint32_t sampling_timer;
  /**<   Time period in milliseconds after which all measurements are to be sampled.
       If not specified, default value configured on the TE will be used.
  */

  /* Optional */
  /*  Report Interval */
  uint8_t report_interval_valid;  /**< Must be set to true if report_interval is being passed */
  uint32_t report_interval;
  /**<   Interval of time in milliseconds after which measurements are to be
       sent from TE. If not specified, default value configured on the TE
       will be used.
  */

  /* Optional */
  /*  Alpha */
  uint8_t alpha_valid;  /**< Must be set to true if alpha is being passed */
  float alpha;
  /**<   Coefficient used to calculate the average. If not specified, default value
       configured on the TE will be used.
  */

  /* Optional */
  /*  Time To Trigger */
  uint8_t time_to_trigger_valid;  /**< Must be set to true if time_to_trigger is being passed */
  uint64_t time_to_trigger;
  /**<   Specifies the amount of time in milliseconds, the TE has to wait before
       sending out the wifi measurement reports, once the threshold is hit.
       If not specified, the TE will send reports whenever the threshold
       conditions are met */
}dsd_start_wifi_meas_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Indication Message; Indicates that the control point needs to stop sending WiFi measurements. */
typedef struct {

  /* Mandatory */
  /*  WiFi Measurement ID */
  uint32_t wifi_meas_id;
  /**<   ID used to link it to previous start indication.
   */
}dsd_stop_wifi_meas_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_enums
    @{
  */
typedef enum {
  DSD_WIFI_MEAS_REPORT_CONFIG_STATUS_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  DSD_WIFI_MEAS_REPORT_CONFIG_OK_V01 = 0, /**<  Configuration OK \n  */
  DSD_WIFI_MEAS_REPORT_CONFIG_ERROR_V01 = 1, /**<  Configuration error \n  */
  DSD_WIFI_MEAS_REPORT_STOPPED_V01 = 2, /**<  Report Stopped  */
  DSD_WIFI_MEAS_REPORT_CONFIG_STATUS_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}dsd_wifi_meas_report_config_status_enum_v01;
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Request Message; Status regarding handling the WiFi measurement report. */
typedef struct {

  /* Mandatory */
  /*  WiFi Measurement ID */
  uint32_t wifi_meas_id;
  /**<   ID provided in QMI_DSD_START_WIFI_MEAS_IND.
  */

  /* Mandatory */
  /*  WiFi Report Configuration Status */
  dsd_wifi_meas_report_config_status_enum_v01 status;
  /**<   TE status regarding WiFi report configuration. Values :\n
      - DSD_WIFI_MEAS_REPORT_CONFIG_OK (0) --  Configuration OK \n
      - DSD_WIFI_MEAS_REPORT_CONFIG_ERROR (1) --  Configuration error \n
      - DSD_WIFI_MEAS_REPORT_STOPPED (2) --  Report Stopped
 */
}dsd_wifi_meas_report_config_status_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Response Message; Status regarding handling the WiFi measurement report. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members: \n
     - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE \n
     - qmi_error_type  -- Error code. Possible error code values are described in
                          the error codes section of each message definition.
            */
}dsd_wifi_meas_report_config_status_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t station_count;
  /**<   Station count. */

  uint16_t channel_utilization;
  /**<   Channel utilization. */

  uint16_t available_admission_capacity;
  /**<   Available Admission Capacity. */
}dsd_wifi_bss_load_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dsd_qmi_enums
    @{
  */
typedef enum {
  DSD_WIFI_MEAS_THRESHOLD_STATE_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  DSD_WIFI_MEAS_THRESHOLD_NOT_CROSSED_V01 = 0, /**<  Threshold not crossed.  */
  DSD_WIFI_MEAS_THRESHOLD_HIGH_CROSSED_V01 = 1, /**<  High Threshold crossed.  */
  DSD_WIFI_MEAS_THRESHOLD_LOW_CROSSED_V01 = 2, /**<  Low Threshold crossed.  */
  DSD_WIFI_MEAS_THRESHOLD_STATE_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}dsd_wifi_meas_threshold_state_type_enum_v01;
/**
    @}
  */

/** @addtogroup dsd_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t downlink_phy_rate;
  /**<   Downlink Data Rate of Physical layer. */

  uint16_t uplink_phy_rate;
  /**<   Uplink Data Rate of Physical layer. */
}dsd_wifi_phy_rate_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dsd_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t downlink_packet_error_rate;
  /**<   Downlink Packet Error Rate. */

  uint16_t uplink_packet_error_rate;
  /**<   Uplink Packet Error Rate. */
}dsd_wifi_packet_error_rate_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dsd_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t channel;
  /**<   Channel in MHz.  */

  dsd_wifi_meas_threshold_state_type_enum_v01 threshold_state;
  /**<   Value specify the state of the threshold. Values \n
      - DSD_WIFI_MEAS_THRESHOLD_NOT_CROSSED (0) --  Threshold not crossed.
      - DSD_WIFI_MEAS_THRESHOLD_HIGH_CROSSED (1) --  High Threshold crossed.
      - DSD_WIFI_MEAS_THRESHOLD_LOW_CROSSED (2) --  Low Threshold crossed.
 */

  dsd_wifi_meas_param_mask_v01 valid_params;
  /**<   Mask of valid parameters
      - QMI_DSD_WIFI_MEAS_PARAM_NONE (0x0000000000000000) --  No Parameters
      - QMI_DSD_WIFI_MEAS_PARAM_RSSI (0x0000000000000001) --  Received Signal Strength Indication
      - QMI_DSD_WIFI_MEAS_PARAM_SINR (0x0000000000000002) --  Signal to Inteference plus Noise Ratio
      - QMI_DSD_WIFI_MEAS_PARAM_BSS_LOAD (0x0000000000000004) --  Basic Service Set Load
      - QMI_DSD_WIFI_MEAS_PARAM_PHY_RATE (0x0000000000000008) --  Physical Layer tranmission Rate
      - QMI_DSD_WIFI_MEAS_PARAM_PACKET_ERROR_RATE (0x0000000000000010) --  Packet Error Rate
 */

  int16_t rssi;
  /**<   Received Signal Strength Indication value for the channel
  */

  int16_t sinr;
  /**<   Signal to Inteference plus Noise Ratio value for the channel
  */

  dsd_wifi_bss_load_type_v01 bss_load;
  /**<   Basic Service Set Load value for the channel
  */

  dsd_wifi_phy_rate_type_v01 phy_rate;
  /**<   Physical layer Transmission rate for the given BSSID
  */

  dsd_wifi_packet_error_rate_type_v01 packet_error_rate;
  /**<   Packet Error Ratio for the given BSSID
  */

  dsd_wifi_network_mode_type_enum_v01 network_mode;
  /**<   ENUM specifying the network mode for the channel. Values \n
      - QMI_DSD_WIFI_NETWORK_MODE_802_11_1997 (0) --  Legacy network Mode.
      - QMI_DSD_WIFI_NETWORK_MODE_802_11a (1) --  OFDM Network mode.
      - QMI_DSD_WIFI_NETWORK_MODE_802_11b (2) --  802.11b
      - QMI_DSD_WIFI_NETWORK_MODE_802_11g (3) --  802.11g
      - QMI_DSD_WIFI_NETWORK_MODE_802_11_2007 (4) --  802.11.2007
      - QMI_DSD_WIFI_NETWORK_MODE_802_11n (5) --  802.11n
      - QMI_DSD_WIFI_NETWORK_MODE_802_11_2012 (6) --  802.11.2012
      - QMI_DSD_WIFI_NETWORK_MODE_802_11ac (7) --  802.11.ac
      - QMI_DSD_WIFI_NETWORK_MODE_802_11ad (8) --  802.11.ad
      - QMI_DSD_WIFI_NETWORK_MODE_802_11af (9) --  802.11.af
      - QMI_DSD_WIFI_NETWORK_MODE_802_11ah (10) --  802.11.ah
      - QMI_DSD_WIFI_NETWORK_MODE_802_11ai (11) --  802.11.ai
      - QMI_DSD_WIFI_NETWORK_MODE_802_11aj (12) --  802.11.aj
      - QMI_DSD_WIFI_NETWORK_MODE_802_11aq (13) --  802.11.aq
      - QMI_DSD_WIFI_NETWORK_MODE_802_11ax (14) --  802.11.ax
      - QMI_DSD_WIFI_NETWORK_MODE_802_11ay (15) --  802.11.ay
 */
}dsd_wifi_meas_info_per_channel_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dsd_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t bssid[QMI_DSD_MAC_ADDR_LEN_V01];
  /**<   Basic Service Set Identifier.
  */

  dsd_wifi_meas_info_per_channel_type_v01 channel_info;
  /**<   WiFi measurement information for a channel.
  */
}dsd_wifi_meas_info_per_bssid_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Request Message; Report sent by TE with WiFi measurements. */
typedef struct {

  /* Mandatory */
  /*  WiFi Measurement ID */
  uint32_t wifi_meas_id;
  /**<   ID provided in QMI_DSD_START_WIFI_MEAS_IND.
  */

  /* Mandatory */
  /*  Transaction ID */
  uint32_t tx_id;
  /**<   Since WiFi measurements are sent per SSID and there could be multiple SSIDs
       for which information needs to be sent, or more than QMI_DSD_MAX_BSSIDS for
       a SSID, the information will be split into multiple
       QMI_DSD_WIFI_MEAS_REPORT_REQ messages, all of them identified by the
       same transaction id.
  */

  /* Mandatory */
  /*  Report Type */
  dsd_wifi_meas_report_type_enum_v01 report_type;
  /**<   Specifies the type of wifi measurement report that was specified in the
 QMI_DSD_START_WIFI_MEAS_IND. Values \n
      - DSD_WIFI_MEAS_REPORT_TYPE_PERIODIC_RAW (0) --  Report Raw Wifi Measurements values periodically.
      - DSD_WIFI_MEAS_REPORT_TYPE_ONE_TIME_AFTER_THRESHOLD (1) --  Report Wifi Measurements once after crossing the threshold.
      - DSD_WIFI_MEAS_REPORT_TYPE_PERIODIC_AFTER_THRESHOLD (2) --  Report Wifi Measurements periodically after crossing the threshold.
 */

  /* Mandatory */
  /*  Is Last Report Of Transaction */
  uint8_t is_last_report_of_transaction;
  /**<   If there are multiple QMI_DSD_WIFI_MEAS_REPORT_REQ messages being sent
       for the transaction, this TLV identifies if there are more to follow.
       Values :\n
       - 0 -- More to follow \n
	   - 1 -- Is the last report of the transaction.
  */

  /* Mandatory */
  /*  Service Set Identifier */
  uint32_t ssid_len;  /**< Must be set to # of elements in ssid */
  uint8_t ssid[QMI_DSD_MAX_SSID_LEN_V01];
  /**<   Service Set Identifier.
  */

  /* Optional */
  /*  Per BSSID Info */
  uint8_t per_bssid_info_valid;  /**< Must be set to true if per_bssid_info is being passed */
  uint32_t per_bssid_info_len;  /**< Must be set to # of elements in per_bssid_info */
  dsd_wifi_meas_info_per_bssid_type_v01 per_bssid_info[QMI_DSD_MAX_BSSIDS_PER_REPORT_V01];
  /**<   WiFi measurement information for each BSSID.
  */
}dsd_wifi_meas_report_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Response Message; Report sent by TE with WiFi measurements. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members: \n
     - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE \n
     - qmi_error_type  -- Error code. Possible error code values are described in
                          the error codes section of each message definition.
            */
}dsd_wifi_meas_report_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef uint64_t dsd_apn_call_type_mask_v01;
#define QMI_DSD_APN_CALL_TYPE_IDLE_V01 ((dsd_apn_call_type_mask_v01)0x0000000000000000ull) /**<  Idle  */
#define QMI_DSD_APN_CALL_TYPE_GENERAL_V01 ((dsd_apn_call_type_mask_v01)0x0000000000000001ull) /**<  General  */
#define QMI_DSD_APN_CALL_TYPE_VOICE_V01 ((dsd_apn_call_type_mask_v01)0x0000000000000002ull) /**<  Voice call  */
#define QMI_DSD_APN_CALL_TYPE_VIDEO_V01 ((dsd_apn_call_type_mask_v01)0x0000000000000004ull) /**<  Video call  */
/** @addtogroup dsd_qmi_messages
    @{
  */
/** Request Message; Sets the highest technology preference for an APN. */
typedef struct {

  /* Mandatory */
  /*  APN */
  char apn[QMI_DSD_MAX_APN_LEN_V01 + 1];
  /**<   APN \n
  */

  /* Mandatory */
  /*  APN Call Type */
  dsd_apn_call_type_mask_v01 apn_call_type;
  /**<   APN Call Type. Values \n:
      - QMI_DSD_APN_CALL_TYPE_IDLE (0x0000000000000000) --  Idle
      - QMI_DSD_APN_CALL_TYPE_GENERAL (0x0000000000000001) --  General
      - QMI_DSD_APN_CALL_TYPE_VOICE (0x0000000000000002) --  Voice call
      - QMI_DSD_APN_CALL_TYPE_VIDEO (0x0000000000000004) --  Video call
 */
}dsd_set_apn_call_type_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dsd_qmi_messages
    @{
  */
/** Response Message; Sets the highest technology preference for an APN. */
typedef struct {

  /* Mandatory */
  /*  Result code. */
  qmi_response_type_v01 resp;
  /**<   Standard response type.       */
}dsd_set_apn_call_type_resp_msg_v01;  /* Message */
/**
    @}
  */

/* Conditional compilation tags for message removal */
//#define REMOVE_QMI_DSD_ADD_PDN_POLICY_V01
//#define REMOVE_QMI_DSD_BIND_SUBSCRIPTION_V01
//#define REMOVE_QMI_DSD_DELETE_PDN_POLICY_BY_APN_V01
//#define REMOVE_QMI_DSD_GET_APN_INFO_V01
//#define REMOVE_QMI_DSD_GET_BIND_SUBSCRIPTION_V01
//#define REMOVE_QMI_DSD_GET_DATA_SETTING_V01
//#define REMOVE_QMI_DSD_GET_MODEM_POWER_COST_V01
//#define REMOVE_QMI_DSD_GET_PDN_POLICY_APN_LIST_V01
//#define REMOVE_QMI_DSD_GET_PDN_POLICY_SETTINGS_FOR_APN_V01
//#define REMOVE_QMI_DSD_GET_SYSTEM_STATUS_V01
//#define REMOVE_QMI_DSD_GET_THERMAL_MITIGATION_INFO_V01
//#define REMOVE_QMI_DSD_GET_WLAN_PREFERENCE_V01
//#define REMOVE_QMI_DSD_INDICATION_REGISTER_V01
//#define REMOVE_QMI_DSD_MODIFY_PDN_POLICY_V01
//#define REMOVE_QMI_DSD_NOTIFY_DATA_SETTING_V01
//#define REMOVE_QMI_DSD_PDN_POLICY_END_TXN_V01
//#define REMOVE_QMI_DSD_PDN_POLICY_START_TXN_V01
//#define REMOVE_QMI_DSD_SET_APN_CALL_TYPE_V01
//#define REMOVE_QMI_DSD_SET_APN_INFO_V01
//#define REMOVE_QMI_DSD_SET_APN_PREFERRED_SYSTEM_V01
//#define REMOVE_QMI_DSD_SET_WLAN_PREFERENCE_V01
//#define REMOVE_QMI_DSD_START_WIFI_MEAS_IND_V01
//#define REMOVE_QMI_DSD_STOP_WIFI_MEAS_IND_V01
//#define REMOVE_QMI_DSD_SYSTEM_STATUS_CHANGE_V01
//#define REMOVE_QMI_DSD_SYSTEM_STATUS_IND_V01
//#define REMOVE_QMI_DSD_THERMAL_INFO_CHANGE_IND_V01
//#define REMOVE_QMI_DSD_WIFI_MEAS_REPORT_V01
//#define REMOVE_QMI_DSD_WIFI_MEAS_REPORT_CONFIG_STATUS_V01
//#define REMOVE_QMI_DSD_WLAN_AVAILABLE_V01
//#define REMOVE_QMI_DSD_WLAN_NOT_AVAILABLE_V01

/*Service Message Definition*/
/** @addtogroup dsd_qmi_msg_ids
    @{
  */
#define QMI_DSD_WLAN_AVAILABLE_REQ_V01 0x0020
#define QMI_DSD_WLAN_AVAILABLE_RESP_V01 0x0020
#define QMI_DSD_WLAN_NOT_AVAILABLE_REQ_V01 0x0021
#define QMI_DSD_WLAN_NOT_AVAILABLE_RESP_V01 0x0021
#define QMI_DSD_SET_WLAN_PREFERENCE_REQ_V01 0x0022
#define QMI_DSD_SET_WLAN_PREFERENCE_RESP_V01 0x0022
#define QMI_DSD_GET_WLAN_PREFERENCE_REQ_V01 0x0023
#define QMI_DSD_GET_WLAN_PREFERENCE_RESP_V01 0x0023
#define QMI_DSD_GET_SYSTEM_STATUS_REQ_V01 0x0024
#define QMI_DSD_GET_SYSTEM_STATUS_RESP_V01 0x0024
#define QMI_DSD_SYSTEM_STATUS_CHANGE_REQ_V01 0x0025
#define QMI_DSD_SYSTEM_STATUS_CHANGE_RESP_V01 0x0025
#define QMI_DSD_SYSTEM_STATUS_IND_V01 0x0026
#define QMI_DSD_BIND_SUBSCRIPTION_REQ_V01 0x0027
#define QMI_DSD_BIND_SUBSCRIPTION_RESP_V01 0x0027
#define QMI_DSD_GET_BIND_SUBSCRIPTION_REQ_V01 0x0028
#define QMI_DSD_GET_BIND_SUBSCRIPTION_RESP_V01 0x0028
#define QMI_DSD_SET_APN_PREFERRED_SYSTEM_REQ_V01 0x0029
#define QMI_DSD_SET_APN_PREFERRED_SYSTEM_RESP_V01 0x0029
#define QMI_DSD_GET_MODEM_POWER_COST_REQ_V01 0x002A
#define QMI_DSD_GET_MODEM_POWER_COST_RESP_V01 0x002A
#define QMI_DSD_PDN_POLICY_START_TXN_REQ_V01 0x002B
#define QMI_DSD_PDN_POLICY_START_TXN_RESP_V01 0x002B
#define QMI_DSD_ADD_PDN_POLICY_REQ_V01 0x002C
#define QMI_DSD_ADD_PDN_POLICY_RESP_V01 0x002C
#define QMI_DSD_MODIFY_PDN_POLICY_REQ_V01 0x002D
#define QMI_DSD_MODIFY_PDN_POLICY_RESP_V01 0x002D
#define QMI_DSD_DELETE_PDN_POLICY_BY_APN_REQ_V01 0x002E
#define QMI_DSD_DELETE_PDN_POLICY_BY_APN_RESP_V01 0x002E
#define QMI_DSD_GET_PDN_POLICY_APN_LIST_REQ_V01 0x002F
#define QMI_DSD_GET_PDN_POLICY_APN_LIST_RESP_V01 0x002F
#define QMI_DSD_GET_PDN_POLICY_SETTINGS_FOR_APN_REQ_V01 0x0030
#define QMI_DSD_GET_PDN_POLICY_SETTINGS_FOR_APN_RESP_V01 0x0030
#define QMI_DSD_PDN_POLICY_END_TXN_REQ_V01 0x0031
#define QMI_DSD_PDN_POLICY_END_TXN_RESP_V01 0x0031
#define QMI_DSD_SET_APN_INFO_REQ_V01 0x0032
#define QMI_DSD_SET_APN_INFO_RESP_V01 0x0032
#define QMI_DSD_GET_APN_INFO_REQ_V01 0x0033
#define QMI_DSD_GET_APN_INFO_RESP_V01 0x0033
#define QMI_DSD_NOTIFY_DATA_SETTING_REQ_V01 0x0034
#define QMI_DSD_NOTIFY_DATA_SETTING_RESP_V01 0x0034
#define QMI_DSD_GET_DATA_SETTING_REQ_V01 0x0035
#define QMI_DSD_GET_DATA_SETTING_RESP_V01 0x0035
#define QMI_DSD_THERMAL_INFO_CHANGE_IND_V01 0x0036
#define QMI_DSD_GET_THERMAL_MITIGATION_INFO_REQ_V01 0x0037
#define QMI_DSD_GET_THERMAL_MITIGATION_INFO_RESP_V01 0x0037
#define QMI_DSD_INDICATION_REGISTER_REQ_V01 0x0038
#define QMI_DSD_INDICATION_REGISTER_RESP_V01 0x0038
#define QMI_DSD_START_WIFI_MEAS_IND_V01 0x0039
#define QMI_DSD_STOP_WIFI_MEAS_IND_V01 0x003A
#define QMI_DSD_WIFI_MEAS_REPORT_CONFIG_STATUS_REQ_V01 0x003B
#define QMI_DSD_WIFI_MEAS_REPORT_CONFIG_STATUS_RESP_V01 0x003B
#define QMI_DSD_WIFI_MEAS_REPORT_REQ_V01 0x003C
#define QMI_DSD_WIFI_MEAS_REPORT_RESP_V01 0x003C
#define QMI_DSD_SET_APN_CALL_TYPE_REQ_V01 0x003D
#define QMI_DSD_SET_APN_CALL_TYPE_RESP_V01 0x003D
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro dsd_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type dsd_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );

/** This macro should be used to get the service object */
#define dsd_get_service_object_v01( ) \
          dsd_get_service_object_internal_v01( \
            DSD_V01_IDL_MAJOR_VERS, DSD_V01_IDL_MINOR_VERS, \
            DSD_V01_IDL_TOOL_VERS )
/**
    @}
  */


#ifdef __cplusplus
}
#endif
#endif

