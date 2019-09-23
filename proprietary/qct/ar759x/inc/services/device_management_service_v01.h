#ifndef DMS_SERVICE_01_H
#define DMS_SERVICE_01_H
/**
  @file device_management_service_v01.h

  @brief This is the public header file which defines the dms service Data structures.

  This header file defines the types and structures that were defined in
  dms. It contains the constant values defined, enums, structures,
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
  Copyright (c) 2006-2014 Qualcomm Technologies, Inc.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.



  $Header: //source/qcom/qct/interfaces/qmi/dms/main/latest/api/device_management_service_v01.h#42 $
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.7 
   It was generated on: Fri May 30 2014 (Spin 0)
   From IDL File: device_management_service_v01.idl */

/** @defgroup dms_qmi_consts Constant values defined in the IDL */
/** @defgroup dms_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup dms_qmi_enums Enumerated types used in QMI messages */
/** @defgroup dms_qmi_messages Structures sent as QMI messages */
/** @defgroup dms_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup dms_qmi_accessor Accessor for QMI service object */
/** @defgroup dms_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup dms_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define DMS_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define DMS_V01_IDL_MINOR_VERS 0x27
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define DMS_V01_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
/* SWISTART */
#ifndef SIERRA
#define DMS_V01_MAX_MESSAGE_ID 0x005D
#else
#define DMS_V01_MAX_MESSAGE_ID 0x656F
#endif
/* SWISTOP */
/**
    @}
  */


/** @addtogroup dms_qmi_consts 
    @{ 
  */
#define QMI_DMS_POWER_SOURCE_BATTERY_V01 0
#define QMI_DMS_POWER_SOURCE_EXT_SOURCE_V01 1
#define QMI_DMS_RADIO_IF_LIST_MAX_V01 20
#define QMI_DMS_DEVICE_MANUFACTURER_MAX_V01 128
#define QMI_DMS_DEVICE_MODEL_ID_MAX_V01 256
#define QMI_DMS_DEVICE_REV_ID_MAX_V01 256
#define QMI_DMS_VOICE_NUMBER_MAX_V01 32
#define QMI_DMS_MOBILE_ID_NUMBER_MAX_V01 32
#define QMI_DMS_ESN_MAX_V01 32
#define QMI_DMS_IMEI_MAX_V01 32
#define QMI_DMS_MEID_MAX_V01 32
#define QMI_DMS_ENCRYPTED_ESN_MAX_V01 255
#define QMI_DMS_ENCRYPTED_IMEI_MAX_V01 255
#define QMI_DMS_ENCRYPTED_MEID_MAX_V01 255
#define QMI_DMS_ENCRYPTED_IMEISV_MAX_V01 255
#define QMI_DMS_PUK_VALUE_MAX_V01 16
#define QMI_DMS_PIN_VALUE_MAX_V01 16
#define QMI_DMS_HARDWARE_REV_MAX_V01 256
#define QMI_DMS_UIM_ID_MAX_V01 20
#define QMI_DMS_FACILITY_CK_MAX_V01 8
#define QMI_DMS_FACILITY_UNBLOCK_CK_MAX_V01 8
#define QMI_DMS_IMSI_MAX_V01 32
#define QMI_DMS_BOOT_CODE_REV_MAX_V01 255
#define QMI_DMS_PRI_REV_MAX_V01 16
#define QMI_DMS_IMEISV_MAX_V01 255
#define QMI_DMS_SW_VERSION_MAX_V01 32
#define QMI_DMS_SPC_LEN_V01 6
#define QMI_DMS_LOCK_CODE_LEN_V01 4
#define QMI_DMS_MDN_MAX_V01 15
#define QMI_DMS_MIN_MAX_V01 15
#define QMI_DMS_HA_KEY_MAX_V01 16
#define QMI_DMS_AAA_KEY_MAX_V01 16
#define QMI_DMS_USER_DATA_MAX_V01 512
#define QMI_DMS_ERI_DATA_MAX_V01 1024
#define QMI_DMS_PRL_DATA_MAX_V01 1536
#define QMI_DMS_FACTORY_SN_MAX_V01 128
#define QMI_DMS_ACTIVATION_CODE_MAX_V01 81
#define QMI_DMS_MAX_CONFIG_LIST_LEN_V01 32
#define QMI_DMS_MAX_SUBSCRIPTION_LIST_LEN_V01 6
#define QMI_DMS_MAC_ADDR_MAX_V01 8
#define QMI_DMS_IMAGE_VER_MAX_V01 32
#define QMI_DMS_IMAGE_VER_LEN_MAX_V01 128
/* SWISTART */
#ifdef SIERRA
#define QMI_DMS_PRODUCT_NAME_SIZE_V01 16
#define QMI_DMS_BCH_VERSTR_SIZE_V01 84
#define QMI_DMS_PRODUCT_SKU_SIZE_V01 4
#define QMI_DMS_CARRIERID_SIZE_V01 2
#define QMI_DMS_PRIID_SIZE_V01 15
#define QMI_DMS_USB_TABLE_MAX_V01 255
#define QMI_DMS_USB_GUID_MAX_V01 32
#define QMI_DMS_USB_PRIID_MAX_V01 16
#define QMI_DMS_PROVIDER_ID_MAX_V01 6
#define QMI_DMS_PROVIDER_NAME_MAX_V01 20
#define QMI_DMS_NUM_CARRIER_IMAGES_MAX_V01 20
#define QMI_DMS_GEO_LOC_MAX_V01 4
#define QMI_DMS_PROVIDER_ID_V01 6
#define QMI_DMS_PROVIDER_NAME_V01 16
#define QMI_DMS_NUM_SUPPORTED_IMAGE_TYPES_V01 2
#define QMI_DMS_GOBI_IM_UNIQUE_ID_MAX_V01 32
#define QMI_DMS_GOBI_IM_BUILD_ID_MAX_V01 16
#define QMI_DMS_FWUPDATERESTRLEN_V01 15
#define QMI_DMS_CUSTECCNUM_MAX_V01 7
#define QMI_DMS_RST_DATE_MAX_V01 17
#define QMI_DMS_SWI_FSN_SIZE_V01 16
#define QMI_DMS_ER_ERROR_STRING_MAX_V01 64
#define QMI_DMS_ER_DUMP_STRING_MAX_V01 692
#define QMI_DMS_SERIAL_STRING_MAX_V01 127
#define QMI_DMS_SERIAL_NUMBER_SIZE_V01 127
#define QMI_DMS_HOST_DEV_INFO_MAX_V01 255
#define QMI_DMS_OS_INFO_MAX_V01 256
#define QMI_DMS_L_MASK_SIZE_V01 2
#endif /* SIERRA */
/* SWISTOP */
/**
    @}
  */

typedef uint8_t dms_power_status_mask_v01;
#define QMI_DMS_MASK_POWER_SOURCE_V01 ((dms_power_status_mask_v01)0x01) 
#define QMI_DMS_MASK_BATTERY_CONNECTED_V01 ((dms_power_status_mask_v01)0x02) 
#define QMI_DMS_MASK_BATTERY_CHARGING_V01 ((dms_power_status_mask_v01)0x04) 
#define QMI_DMS_MASK_POWER_FAULT_V01 ((dms_power_status_mask_v01)0x08) 
typedef uint16_t dms_offline_reason_mask_v01;
#define QMI_DMS_MASK_OFFLINE_REASON_HOST_IMAGE_V01 ((dms_offline_reason_mask_v01)0x0001) 
#define QMI_DMS_MASK_OFFLINE_REASON_PRI_IMAGE_V01 ((dms_offline_reason_mask_v01)0x0002) 
#define QMI_DMS_MASK_OFFLINE_REASON_PRI_VERSION_V01 ((dms_offline_reason_mask_v01)0x0004) 
#define QMI_DMS_MASK_OFFLINE_REASON_DEVICE_MEMORY_V01 ((dms_offline_reason_mask_v01)0x0008) 
typedef uint64_t dms_band_capability_mask_v01;
#define QMI_DMS_MASK_BAND_PREF_BC0_A_V01 ((dms_band_capability_mask_v01)0x0000000000000001ull) 
#define QMI_DMS_MASK_BAND_PREF_BC0_B_V01 ((dms_band_capability_mask_v01)0x0000000000000002ull) 
#define QMI_DMS_MASK_BAND_PREF_BC1_V01 ((dms_band_capability_mask_v01)0x0000000000000004ull) 
#define QMI_DMS_MASK_BAND_PREF_BC2_V01 ((dms_band_capability_mask_v01)0x0000000000000008ull) 
#define QMI_DMS_MASK_BAND_PREF_BC3_V01 ((dms_band_capability_mask_v01)0x0000000000000010ull) 
#define QMI_DMS_MASK_BAND_PREF_BC4_V01 ((dms_band_capability_mask_v01)0x0000000000000020ull) 
#define QMI_DMS_MASK_BAND_PREF_BC5_V01 ((dms_band_capability_mask_v01)0x0000000000000040ull) 
#define QMI_DMS_MASK_BAND_PREF_GSM_DCS_1800_V01 ((dms_band_capability_mask_v01)0x0000000000000080ull) 
#define QMI_DMS_MASK_BAND_PREF_GSM_EGSM_900_V01 ((dms_band_capability_mask_v01)0x0000000000000100ull) 
#define QMI_DMS_MASK_BAND_PREF_GSM_PGSM_900_V01 ((dms_band_capability_mask_v01)0x0000000000000200ull) 
#define QMI_DMS_MASK_BAND_PREF_BC6_V01 ((dms_band_capability_mask_v01)0x0000000000000400ull) 
#define QMI_DMS_MASK_BAND_PREF_BC7_V01 ((dms_band_capability_mask_v01)0x0000000000000800ull) 
#define QMI_DMS_MASK_BAND_PREF_BC8_V01 ((dms_band_capability_mask_v01)0x0000000000001000ull) 
#define QMI_DMS_MASK_BAND_PREF_BC9_V01 ((dms_band_capability_mask_v01)0x0000000000002000ull) 
#define QMI_DMS_MASK_BAND_PREF_BC10_V01 ((dms_band_capability_mask_v01)0x0000000000004000ull) 
#define QMI_DMS_MASK_BAND_PREF_BC11_V01 ((dms_band_capability_mask_v01)0x0000000000008000ull) 
#define QMI_DMS_MASK_BAND_PREF_GSM_450_V01 ((dms_band_capability_mask_v01)0x0000000000010000ull) 
#define QMI_DMS_MASK_BAND_PREF_GSM_480_V01 ((dms_band_capability_mask_v01)0x0000000000020000ull) 
#define QMI_DMS_MASK_BAND_PREF_GSM_750_V01 ((dms_band_capability_mask_v01)0x0000000000040000ull) 
#define QMI_DMS_MASK_BAND_PREF_GSM_850_V01 ((dms_band_capability_mask_v01)0x0000000000080000ull) 
#define QMI_DMS_MASK_BAND_PREF_GSM_RGSM_900_V01 ((dms_band_capability_mask_v01)0x0000000000100000ull) 
#define QMI_DMS_MASK_BAND_PREF_GSM_PCS_1900_V01 ((dms_band_capability_mask_v01)0x0000000000200000ull) 
#define QMI_DMS_MASK_BAND_PREF_WCDMA_IMT_2100_V01 ((dms_band_capability_mask_v01)0x0000000000400000ull) 
#define QMI_DMS_MASK_BAND_PREF_WCDMA_PCS_1900_V01 ((dms_band_capability_mask_v01)0x0000000000800000ull) 
#define QMI_DMS_MASK_BAND_PREF_WCDMA_1800_V01 ((dms_band_capability_mask_v01)0x0000000001000000ull) 
#define QMI_DMS_MASK_BAND_PREF_WCDMA_1700_US_V01 ((dms_band_capability_mask_v01)0x0000000002000000ull) 
#define QMI_DMS_MASK_BAND_PREF_WCDMA_850_V01 ((dms_band_capability_mask_v01)0x0000000004000000ull) 
#define QMI_DMS_MASK_BAND_PREF_WCDMA_800_V01 ((dms_band_capability_mask_v01)0x0000000008000000ull) 
#define QMI_DMS_MASK_BAND_PREF_BC12_V01 ((dms_band_capability_mask_v01)0x0000000010000000ull) 
#define QMI_DMS_MASK_BAND_PREF_B14_V01 ((dms_band_capability_mask_v01)0x0000000020000000ull) 
#define QMI_DMS_MASK_BAND_PREF_B15_V01 ((dms_band_capability_mask_v01)0x0000000080000000ull) 
#define QMI_DMS_MASK_BAND_PREF_WCDMA_2600_V01 ((dms_band_capability_mask_v01)0x0001000000000000ull) 
#define QMI_DMS_MASK_BAND_PREF_WCDMA_900_V01 ((dms_band_capability_mask_v01)0x0002000000000000ull) 
#define QMI_DMS_MASK_BAND_PREF_WCDMA_1700_JPN_V01 ((dms_band_capability_mask_v01)0x0004000000000000ull) 
#define QMI_DMS_MASK_BAND_PREF_BC16_V01 ((dms_band_capability_mask_v01)0x0100000000000000ull) 
#define QMI_DMS_MASK_BAND_PREF_BC17_V01 ((dms_band_capability_mask_v01)0x0200000000000000ull) 
#define QMI_DMS_MASK_BAND_PREF_BC18_V01 ((dms_band_capability_mask_v01)0x0400000000000000ull) 
#define QMI_DMS_MASK_BAND_PREF_BC19_V01 ((dms_band_capability_mask_v01)0x0800000000000000ull) 
#define QMI_DMS_MASK_BAND_PREF_WCDMA_XIX_850_V01 ((dms_band_capability_mask_v01)0x1000000000000000ull) 
#define QMI_DMS_MASK_BAND_PREF_WCDMA_XI_1500_V01 ((dms_band_capability_mask_v01)0x2000000000000000ull) 
typedef uint64_t dms_lte_band_capability_mask_v01;
#define QMI_DMS_MASK_BAND_PREF_LTE_EB1_V01 ((dms_lte_band_capability_mask_v01)0x0000000000000001ull) 
#define QMI_DMS_MASK_BAND_PREF_LTE_EB2_V01 ((dms_lte_band_capability_mask_v01)0x0000000000000002ull) 
#define QMI_DMS_MASK_BAND_PREF_LTE_EB3_V01 ((dms_lte_band_capability_mask_v01)0x0000000000000004ull) 
#define QMI_DMS_MASK_BAND_PREF_LTE_EB4_V01 ((dms_lte_band_capability_mask_v01)0x0000000000000008ull) 
#define QMI_DMS_MASK_BAND_PREF_LTE_EB5_V01 ((dms_lte_band_capability_mask_v01)0x0000000000000010ull) 
#define QMI_DMS_MASK_BAND_PREF_LTE_EB6_V01 ((dms_lte_band_capability_mask_v01)0x0000000000000020ull) 
#define QMI_DMS_MASK_BAND_PREF_LTE_EB7_V01 ((dms_lte_band_capability_mask_v01)0x0000000000000040ull) 
#define QMI_DMS_MASK_BAND_PREF_LTE_EB8_V01 ((dms_lte_band_capability_mask_v01)0x0000000000000080ull) 
#define QMI_DMS_MASK_BAND_PREF_LTE_EB9_V01 ((dms_lte_band_capability_mask_v01)0x0000000000000100ull) 
#define QMI_DMS_MASK_BAND_PREF_LTE_EB10_V01 ((dms_lte_band_capability_mask_v01)0x0000000000000200ull) 
#define QMI_DMS_MASK_BAND_PREF_LTE_EB11_V01 ((dms_lte_band_capability_mask_v01)0x0000000000000400ull) 
#define QMI_DMS_MASK_BAND_PREF_LTE_EB12_V01 ((dms_lte_band_capability_mask_v01)0x0000000000000800ull) 
#define QMI_DMS_MASK_BAND_PREF_LTE_EB13_V01 ((dms_lte_band_capability_mask_v01)0x0000000000001000ull) 
#define QMI_DMS_MASK_BAND_PREF_LTE_EB14_V01 ((dms_lte_band_capability_mask_v01)0x0000000000002000ull) 
#define QMI_DMS_MASK_BAND_PREF_LTE_EB17_V01 ((dms_lte_band_capability_mask_v01)0x0000000000010000ull) 
#define QMI_DMS_MASK_BAND_PREF_LTE_EB18_V01 ((dms_lte_band_capability_mask_v01)0x0000000000020000ull) 
#define QMI_DMS_MASK_BAND_PREF_LTE_EB19_V01 ((dms_lte_band_capability_mask_v01)0x0000000000040000ull) 
#define QMI_DMS_MASK_BAND_PREF_LTE_EB20_V01 ((dms_lte_band_capability_mask_v01)0x0000000000080000ull) 
#define QMI_DMS_MASK_BAND_PREF_LTE_EB21_V01 ((dms_lte_band_capability_mask_v01)0x0000000000100000ull) 
#define QMI_DMS_MASK_BAND_PREF_LTE_EB23_V01 ((dms_lte_band_capability_mask_v01)0x0000000000400000ull) 
#define QMI_DMS_MASK_BAND_PREF_LTE_EB24_V01 ((dms_lte_band_capability_mask_v01)0x0000000000800000ull) 
#define QMI_DMS_MASK_BAND_PREF_LTE_EB25_V01 ((dms_lte_band_capability_mask_v01)0x0000000001000000ull) 
#define QMI_DMS_MASK_BAND_PREF_LTE_EB26_V01 ((dms_lte_band_capability_mask_v01)0x0000000002000000ull) 
#define QMI_DMS_MASK_BAND_PREF_LTE_EB28_V01 ((dms_lte_band_capability_mask_v01)0x0000000008000000ull) 
#define QMI_DMS_MASK_BAND_PREF_LTE_EB29_V01 ((dms_lte_band_capability_mask_v01)0x0000000010000000ull) 
#define QMI_DMS_MASK_BAND_PREF_LTE_EB30_V01 ((dms_lte_band_capability_mask_v01)0x0000000020000000ull) 
#define QMI_DMS_MASK_BAND_PREF_LTE_EB33_V01 ((dms_lte_band_capability_mask_v01)0x0000000100000000ull) 
#define QMI_DMS_MASK_BAND_PREF_LTE_EB34_V01 ((dms_lte_band_capability_mask_v01)0x0000000200000000ull) 
#define QMI_DMS_MASK_BAND_PREF_LTE_EB35_V01 ((dms_lte_band_capability_mask_v01)0x0000000400000000ull) 
#define QMI_DMS_MASK_BAND_PREF_LTE_EB36_V01 ((dms_lte_band_capability_mask_v01)0x0000000800000000ull) 
#define QMI_DMS_MASK_BAND_PREF_LTE_EB37_V01 ((dms_lte_band_capability_mask_v01)0x0000001000000000ull) 
#define QMI_DMS_MASK_BAND_PREF_LTE_EB38_V01 ((dms_lte_band_capability_mask_v01)0x0000002000000000ull) 
#define QMI_DMS_MASK_BAND_PREF_LTE_EB39_V01 ((dms_lte_band_capability_mask_v01)0x0000004000000000ull) 
#define QMI_DMS_MASK_BAND_PREF_LTE_EB40_V01 ((dms_lte_band_capability_mask_v01)0x0000008000000000ull) 
#define QMI_DMS_MASK_BAND_PREF_LTE_EB41_V01 ((dms_lte_band_capability_mask_v01)0x0000010000000000ull) 
#define QMI_DMS_MASK_BAND_PREF_LTE_EB42_V01 ((dms_lte_band_capability_mask_v01)0x0000020000000000ull) 
#define QMI_DMS_MASK_BAND_PREF_LTE_EB43_V01 ((dms_lte_band_capability_mask_v01)0x0000040000000000ull) 
typedef uint64_t dms_tds_band_capability_mask_v01;
#define QMI_DMS_MASK_BAND_PREF_TDS_BANDA_V01 ((dms_tds_band_capability_mask_v01)0x0000000000000001ull) 
#define QMI_DMS_MASK_BAND_PREF_TDS_BANDB_V01 ((dms_tds_band_capability_mask_v01)0x0000000000000002ull) 
#define QMI_DMS_MASK_BAND_PREF_TDS_BANDC_V01 ((dms_tds_band_capability_mask_v01)0x0000000000000004ull) 
#define QMI_DMS_MASK_BAND_PREF_TDS_BANDD_V01 ((dms_tds_band_capability_mask_v01)0x0000000000000008ull) 
#define QMI_DMS_MASK_BAND_PREF_TDS_BANDE_V01 ((dms_tds_band_capability_mask_v01)0x0000000000000010ull) 
#define QMI_DMS_MASK_BAND_PREF_TDS_BANDF_V01 ((dms_tds_band_capability_mask_v01)0x0000000000000020ull) 
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}dms_reset_req_msg_v01;

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Resets the DMS state variables of the requesting
           control point. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}dms_reset_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dms_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t battery_lvl_lower_limit;
  /**<   The battery level is reported to the control point if the battery
       level falls below this lower limit (specified as percentage of
       remaining battery power from 0 to 100).
  */

  uint8_t battery_lvl_upper_limit;
  /**<   The battery level is reported to the control point if the battery
       level rises above the upper limit (specified as percentage of
       remaining battery power from 0 to 100).
   */
}dms_battery_lvl_limits_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dms_qmi_messages
    @{
  */
/** Request Message; Sets the device management state reporting conditions
           for the requesting control point. */
typedef struct {

  /* Optional */
  /*  Power State Reporting  */
  uint8_t report_power_state_valid;  /**< Must be set to true if report_power_state is being passed */
  uint8_t report_power_state;
  /**<   Values: \n
       - 0 -- Do not report \n
       - 1 -- Report on change in power state
  */

  /* Optional */
  /*  Battery Level Report Limits */
  uint8_t lvl_limits_valid;  /**< Must be set to true if lvl_limits is being passed */
  dms_battery_lvl_limits_type_v01 lvl_limits;

  /* Optional */
  /*  PIN State Reporting */
  uint8_t report_pin_state_valid;  /**< Must be set to true if report_pin_state is being passed */
  uint8_t report_pin_state;
  /**<   Values: \n
       - 0 -- Do not report \n
       - 1 -- Report on change in PIN state
  */

  /* Optional */
  /*  Activation State Reporting */
  uint8_t report_activation_state_valid;  /**< Must be set to true if report_activation_state is being passed */
  uint8_t report_activation_state;
  /**<   Values: \n
       - 0 -- Do not report \n
       - 1 -- Report activation state changes
  */

  /* Optional */
  /*  Operating Mode Reporting */
  uint8_t report_oprt_mode_state_valid;  /**< Must be set to true if report_oprt_mode_state is being passed */
  uint8_t report_oprt_mode_state;
  /**<   Values: \n
       - 0 -- Do not report \n
       - 1 -- Report operating mode changes     
  */

  /* Optional */
  /*  UIM State Reporting */
  uint8_t report_uim_state_valid;  /**< Must be set to true if report_uim_state is being passed */
  uint8_t report_uim_state;
  /**<   Values: \n
       - 0 -- Do not report \n
       - 1 -- Report UIM state changes
  */

  /* Optional */
  /*  Wireless Disable State Reporting */
  uint8_t report_wireless_disable_state_valid;  /**< Must be set to true if report_wireless_disable_state is being passed */
  uint8_t report_wireless_disable_state;
  /**<   Values: \n
       - 0 -- Do not report \n
       - 1 -- Report wireless disable state changes
  */

  /* Optional */
  /*  PRL Init Reporting */
  uint8_t report_prl_init_valid;  /**< Must be set to true if report_prl_init is being passed */
  uint8_t report_prl_init;
  /**<   Values: \n
       - 0 -- Do not report \n
       - 1 -- Report PRL initialized notification
  */

  /* Optional */
  /*  CDMA Lock Mode Reporting */
  uint8_t report_cdma_lock_mode_valid;  /**< Must be set to true if report_cdma_lock_mode is being passed */
  uint8_t report_cdma_lock_mode;
  /**<   Values: \n
       - 0 -- Do not report (default value) \n
       - 1 -- Report CDMA Lock mode state changes
  */

  /* Optional */
  /*  Device Multisim Information */
  uint8_t report_device_multisim_info_valid;  /**< Must be set to true if report_device_multisim_info is being passed */
  uint8_t report_device_multisim_info;
  /**<   Values: \n
       - 0 -- Do not report (default value) \n
       - 1 -- Report device multisim changes
  */
}dms_set_event_report_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Sets the device management state reporting conditions
           for the requesting control point. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}dms_set_event_report_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dms_qmi_aggregates
    @{
  */
typedef struct {

  dms_power_status_mask_v01 power_status;
  /**<   Power status flags. Values:  \n
       Bit 0 -- Power source  \n
       - 0 -- Powered by battery \n
       - 1 -- Powered by external source \n

       Bit 1 -- Battery connected \n
       - 0 -- Not connected \n
       - 1 -- Connected \n

       Bit 2 -- Battery charging \n
       - 0 -- Not charging \n
       - 1 -- Charging \n

       Bit 3 -- Power fault \n
       - 0 -- No power fault \n
       - 1 -- Recognized power fault, calls inhibited
  */

  uint8_t battery_lvl;
  /**<   Level of the battery. Values: \n
       - 0x00 -- Battery is exhausted or the mobile device does not have a
                battery connected \n
       - 1 through 100 (0x64) -- Percentage of battery capacity remaining
  */
}dms_power_state_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dms_qmi_enums
    @{
  */
typedef enum {
  DMS_PIN_STATUS_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  DMS_PIN_STATUS_NOT_INITIALIZED_V01 = 0x00, 
  DMS_PIN_STATUS_ENABLED_NOT_VERIFIED_V01 = 0x01, 
  DMS_PIN_STATUS_ENABLED_VERIFIED_V01 = 0x02, 
  DMS_PIN_STATUS_DISABLED_V01 = 0x03, 
  DMS_PIN_STATUS_BLOCKED_V01 = 0x04, 
  DMS_PIN_STATUS_PERMANENTLY_BLOCKED_V01 = 0x05, 
  DMS_PIN_STATUS_UNBLOCKED_V01 = 0x06, 
  DMS_PIN_STATUS_CHANGED_V01 = 0x07, 
  DMS_PIN_STATUS_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}dms_pin_status_enum_v01;
/**
    @}
  */

/** @addtogroup dms_qmi_aggregates
    @{
  */
typedef struct {

  dms_pin_status_enum_v01 status;
  /**<   Current status of the PIN. Values: \n
       - 0 -- PIN is not initialized \n
       - 1 -- PIN is enabled, not verified \n
       - 2 -- PIN is enabled, verified \n
       - 3 -- PIN is disabled \n
       - 4 -- PIN is blocked \n
       - 5 -- PIN is permanently blocked \n
       - 6 -- PIN is unblocked \n
       - 7 -- PIN is changed 
  */

  uint8_t verify_retries_left;
  /**<   Number of retries left, after which the PIN is blocked. */

  uint8_t unblock_retries_left;
  /**<   Number of unblock retries left, after which the PIN is
       permanently blocked, i.e., the UIM is unusable.
  */
}dms_pin_status_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dms_qmi_enums
    @{
  */
typedef enum {
  DMS_OPERATING_MODE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  DMS_OP_MODE_ONLINE_V01 = 0x00, 
  DMS_OP_MODE_LOW_POWER_V01 = 0x01, 
  DMS_OP_MODE_FACTORY_TEST_MODE_V01 = 0x02, 
  DMS_OP_MODE_OFFLINE_V01 = 0x03, 
  DMS_OP_MODE_RESETTING_V01 = 0x04, 
  DMS_OP_MODE_SHUTTING_DOWN_V01 = 0x05, 
  DMS_OP_MODE_PERSISTENT_LOW_POWER_V01 = 0x06, 
  DMS_OP_MODE_MODE_ONLY_LOW_POWER_V01 = 0x07, 
  DMS_OP_MODE_NET_TEST_GW_V01 = 0x08, 
  DMS_OPERATING_MODE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}dms_operating_mode_enum_v01;
/**
    @}
  */

/** @addtogroup dms_qmi_enums
    @{
  */
typedef enum {
  DMS_UIM_STATE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  DMS_UIM_INITIALIZATION_COMPLETED_V01 = 0x00, 
  DMS_UIM_INITIALIZATION_FAILED_V01 = 0x01, 
  DMS_UIM_NOT_PRESENT_V01 = 0x02, 
  DMS_UIM_STATE_UNAVAILABLE_V01 = -1, 
  DMS_UIM_STATE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}dms_uim_state_enum_v01;
/**
    @}
  */

/** @addtogroup dms_qmi_enums
    @{
  */
typedef enum {
  DMS_ACTIVATION_STATE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  DMS_ACTIVATION_NOT_ACTIVATED_V01 = 0x00, 
  DMS_ACTIVATION_ACTIVATED_V01 = 0x01, 
  DMS_ACTIVATION_CONNECTING_V01 = 0x02, 
  DMS_ACTIVATION_CONNECTED_V01 = 0x03, 
  DMS_ACTIVATION_OTASP_SEC_AUTHENTICATED_V01 = 0x4, 
  DMS_ACTIVATION_OTASP_NAM_DOWNLOADED_V01 = 0x05, 
  DMS_ACTIVATION_OTASP_MDN_DOWNLOADED_V01 = 0x06, 
  DMS_ACTIVATION_OTASP_IMSI_DOWNLOADED_V01 = 0x07, 
  DMS_ACTIVATION_OTASP_PRL_DOWNLOADED_V01 = 0x08, 
  DMS_ACTIVATION_OTASP_SPC_DOWNLOADED_V01 = 0x09, 
  DMS_ACTIVATION_OTASP_SETTINGS_COMMITTED_V01 = 0x0A, 
  DMS_ACTIVATION_STATE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}dms_activation_state_enum_v01;
/**
    @}
  */

/** @addtogroup dms_qmi_enums
    @{
  */
typedef enum {
  DMS_WIRELESS_DISABLE_STATE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  DMS_WIRELESS_DISABLE_OFF_V01 = 0x00, 
  DMS_WIRELESS_DISABLE_ON_V01 = 0x01, 
  DMS_WIRELESS_DISABLE_STATE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}dms_wireless_disable_state_enum_v01;
/**
    @}
  */

/** @addtogroup dms_qmi_enums
    @{
  */
typedef enum {
  DMS_PRL_INIT_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  DMS_PRL_INIT_COMPLETED_V01 = 0x01, 
  DMS_PRL_INIT_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}dms_prl_init_enum_v01;
/**
    @}
  */

/** @addtogroup dms_qmi_enums
    @{
  */
typedef enum {
  DMS_CDMA_LOCK_MODE_STATE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  DMS_CDMA_LOCK_MODE_OFF_V01 = 0, /**<  Phone is not CDMA locked  */
  DMS_CDMA_LOCK_MODE_ON_V01 = 1, /**<  Phone is CDMA locked  */
  DMS_CDMA_LOCK_MODE_STATE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}dms_cdma_lock_mode_state_enum_v01;
/**
    @}
  */

typedef uint64_t dms_subs_capability_mask_type_v01;
#define DMS_SUBS_CAPABILITY_AMPS_V01 ((dms_subs_capability_mask_type_v01)0x00000001ull) 
#define DMS_SUBS_CAPABILITY_CDMA_V01 ((dms_subs_capability_mask_type_v01)0x00000002ull) 
#define DMS_SUBS_CAPABILITY_HDR_V01 ((dms_subs_capability_mask_type_v01)0x00000004ull) 
#define DMS_SUBS_CAPABILITY_GSM_V01 ((dms_subs_capability_mask_type_v01)0x00000008ull) 
#define DMS_SUBS_CAPABILITY_WCDMA_V01 ((dms_subs_capability_mask_type_v01)0x00000010ull) 
#define DMS_SUBS_CAPABILITY_LTE_V01 ((dms_subs_capability_mask_type_v01)0x00000020ull) 
#define DMS_SUBS_CAPABILITY_TDS_V01 ((dms_subs_capability_mask_type_v01)0x00000040ull) 
/** @addtogroup dms_qmi_enums
    @{
  */
typedef enum {
  DMS_SUBS_VOICE_DATA_CAPABILITY_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  DMS_SUBS_VOICE_DATA_CAPABILITY_NORMAL_V01 = 0x01, 
  DMS_SUBS_VOICE_DATA_CAPABILITY_SGLTE_V01 = 0x02, 
  DMS_SUBS_VOICE_DATA_CAPABILITY_CSFB_V01 = 0x03, 
  DMS_SUBS_VOICE_DATA_CAPABILITY_SVLTE_V01 = 0x04, 
  DMS_SUBS_VOICE_DATA_CAPABILITY_SRLTE_V01 = 0x05, 
  DMS_SUBS_VOICE_DATA_CAPABILITY_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}dms_subs_voice_data_capability_enum_v01;
/**
    @}
  */

/** @addtogroup dms_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t max_active;
  /**<   The maximum number of subscriptions listed in this configuration that can be 
       simultaneously active. If this number is less than max_subscriptions it implies
       that any combination of the subscriptions in this configuration can be active 
       and the remaining can be in standby */

  uint32_t subscription_list_len;  /**< Must be set to # of elements in subscription_list */
  dms_subs_capability_mask_type_v01 subscription_list[QMI_DMS_MAX_SUBSCRIPTION_LIST_LEN_V01];
  /**<   An array of max_subscriptions entries where each entry is a mask of capabilities.
 The client ignores any bits in the mask that it does not recognize. Values: 
      - DMS_SUBS_CAPABILITY_AMPS (0x00000001) -- 
      - DMS_SUBS_CAPABILITY_CDMA (0x00000002) -- 
      - DMS_SUBS_CAPABILITY_HDR (0x00000004) -- 
      - DMS_SUBS_CAPABILITY_GSM (0x00000008) -- 
      - DMS_SUBS_CAPABILITY_WCDMA (0x00000010) -- 
      - DMS_SUBS_CAPABILITY_LTE (0x00000020) -- 
      - DMS_SUBS_CAPABILITY_TDS (0x00000040) --  */
}dms_subs_config_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dms_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t max_subscriptions;
  /**<   The maximum number of subscriptions that can be supported simultaneously. */

  uint32_t subscription_config_list_len;  /**< Must be set to # of elements in subscription_config_list */
  dms_subs_config_type_v01 subscription_config_list[QMI_DMS_MAX_CONFIG_LIST_LEN_V01];
  /**<   List of supported multi-SIM configurations. */
}dms_multisim_capability_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dms_qmi_aggregates
    @{
  */
typedef struct {

  dms_subs_voice_data_capability_enum_v01 subs_voice_data_capability;
  /**<   The simultaneous voice eand data capability type of a subscription 
    */

  uint8_t simul_voice_data_capable;
  /**<   The simultaneous voice eand data capability of a subscription 
  */
}dms_subs_voice_data_capability_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dms_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t max_subscriptions;
  /**<   The maximum number of subscriptions that can be supported simultaneously.   */

  uint8_t max_active;
  /**<   The maximum number of subscriptions that can be simultaneously active. 
       If this number is less than max_subscriptions it implies that any combination 
       of the subscriptions in this configuration can be active 
       and the remaining can be in standby */
}dms_current_multisim_capability_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dms_qmi_messages
    @{
  */
/** Indication Message; Sets the device management state reporting conditions
           for the requesting control point. */
typedef struct {

  /* Optional */
  /*  Power State */
  uint8_t power_state_valid;  /**< Must be set to true if power_state is being passed */
  dms_power_state_type_v01 power_state;

  /* Optional */
  /*  PIN 1 Status */
  uint8_t pin1_status_valid;  /**< Must be set to true if pin1_status is being passed */
  dms_pin_status_type_v01 pin1_status;

  /* Optional */
  /*  PIN 2 Status */
  uint8_t pin2_status_valid;  /**< Must be set to true if pin2_status is being passed */
  dms_pin_status_type_v01 pin2_status;

  /* Optional */
  /*  Activation State */
  uint8_t activation_state_valid;  /**< Must be set to true if activation_state is being passed */
  dms_activation_state_enum_v01 activation_state;
  /**<   Service activation state. Values: \n
       - 0x00 -- Service is not activated \n
       - 0x01 -- Service is activated \n
       - 0x02 -- Activation connecting -- Network
                connection is in progress for automatic activation of service \n
       - 0x03 -- Activation connected -- Network
                connection is connected for automatic activation of service \n
       - 0x04 -- OTASP security is authenticated \n
       - 0x05 - OTASP NAM is downloaded \n
       - 0x06 - OTASP MDN is downloaded \n
       - 0x07 - OTASP IMSI downloaded \n
       - 0x08 - OTASP PRL is downloaded \n
       - 0x09 - OTASP SPC is downloaded \n
       - 0x0A - OTASP settings are committed 
  */

  /* Optional */
  /*  Operating Mode */
  uint8_t operating_mode_valid;  /**< Must be set to true if operating_mode is being passed */
  dms_operating_mode_enum_v01 operating_mode;
  /**<   Current operating mode. Values: \n
       - 0 -- Online \n
       - 1 -- Low power \n
       - 2 -- Factory Test mode \n
       - 3 -- Offline \n
       - 4 -- Resetting \n
       - 5 -- Shutting down \n
       - 6 -- Persistent low power \n
       - 7 -- Mode-only low power \n
       - 8 -- Conducting network test for GSM/WCDMA
  */

  /* Optional */
  /*  UIM State */
  uint8_t uim_state_valid;  /**< Must be set to true if uim_state is being passed */
  dms_uim_state_enum_v01 uim_state;
  /**<   UIM state. Values: \n
       - 0x00 -- UIM initialization completed \n
       - 0x01 -- UIM failed \n
       - 0x02 -- UIM is not present \n
       - 0xFF -- UIM state is currently unavailable
  */

  /* Optional */
  /*  Wireless Disable State */
  uint8_t wireless_disable_state_valid;  /**< Must be set to true if wireless_disable_state is being passed */
  dms_wireless_disable_state_enum_v01 wireless_disable_state;
  /**<   Wireless disable state. Values: \n
       - 0x00 -- Wireless disable switch is turned off \n
       - 0x01 -- Wireless disable switch is turned on
  */

  /* Optional */
  /*  PRL Init Notification */
  uint8_t prl_init_valid;  /**< Must be set to true if prl_init is being passed */
  dms_prl_init_enum_v01 prl_init;
  /**<   PRL initialized. Values: \n
       - 0x01 -- PRL is completely loaded into the device
       (could be the default PRL). 
  */

  /* Optional */
  /*  CDMA Lock Mode State */
  uint8_t cdma_lock_mode_state_valid;  /**< Must be set to true if cdma_lock_mode_state is being passed */
  dms_cdma_lock_mode_state_enum_v01 cdma_lock_mode_state;
  /**<   CDMA Lock mode state. Values: \n
      - DMS_CDMA_LOCK_MODE_OFF (0) --  Phone is not CDMA locked 
      - DMS_CDMA_LOCK_MODE_ON (1) --  Phone is CDMA locked  
 */

  /* Optional */
  /*  Device Multisim Capability
 Note: Below TLV is deprecated from QMI DMS version 1.37 in favor of current_multisim_capability TLV */
  uint8_t multisim_capability_valid;  /**< Must be set to true if multisim_capability is being passed */
  dms_multisim_capability_type_v01 multisim_capability;
  /**<   \n Device capability for supporting multiple simultaneously active radio interfaces.
  */

  /* Optional */
  /*  Device Multisim Voice Data Capability */
  uint8_t current_multisim_capability_valid;  /**< Must be set to true if current_multisim_capability is being passed */
  dms_current_multisim_capability_type_v01 current_multisim_capability;
  /**<   \n Device voice and data capability for supporting multiple simultaneously 
        active radio interfaces.
  */

  /* Optional */
  /*  Current Subscription Capability */
  uint8_t current_subscription_capability_valid;  /**< Must be set to true if current_subscription_capability is being passed */
  uint32_t current_subscription_capability_len;  /**< Must be set to # of elements in current_subscription_capability */
  dms_subs_capability_mask_type_v01 current_subscription_capability[QMI_DMS_MAX_SUBSCRIPTION_LIST_LEN_V01];
  /**<   An array of max_subscriptions entries where each entry is a mask of capabilities.
 The client ignores any bits in the mask that it does not recognize. Values: 
      - DMS_SUBS_CAPABILITY_AMPS (0x00000001) -- 
      - DMS_SUBS_CAPABILITY_CDMA (0x00000002) -- 
      - DMS_SUBS_CAPABILITY_HDR (0x00000004) -- 
      - DMS_SUBS_CAPABILITY_GSM (0x00000008) -- 
      - DMS_SUBS_CAPABILITY_WCDMA (0x00000010) -- 
      - DMS_SUBS_CAPABILITY_LTE (0x00000020) -- 
      - DMS_SUBS_CAPABILITY_TDS (0x00000040) --  
 */

  /* Optional */
  /*  Subscription Voice Data Capability of the Device */
  uint8_t subs_voice_data_capability_valid;  /**< Must be set to true if subs_voice_data_capability is being passed */
  uint32_t subs_voice_data_capability_len;  /**< Must be set to # of elements in subs_voice_data_capability */
  dms_subs_voice_data_capability_type_v01 subs_voice_data_capability[QMI_DMS_MAX_SUBSCRIPTION_LIST_LEN_V01];
  /**<   \n Voice and data capability of each subscription of the device
  */
}dms_event_report_ind_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}dms_get_device_cap_req_msg_v01;

/** @addtogroup dms_qmi_enums
    @{
  */
typedef enum {
  DMS_DATA_SERVICE_CAPABILITY_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  DMS_DATA_CAP_NONE_V01 = 0x00, 
  DMS_DATA_CAP_CS_ONLY_V01 = 0x01, 
  DMS_DATA_CAP_PS_ONLY_V01 = 0x02, 
  DMS_DATA_CAP_SIMUL_CS_AND_PS_V01 = 0x03, 
  DMS_DATA_CAP_NONSIMUL_CS_AND_PS_V01 = 0x04, 
  DMS_DATA_SERVICE_CAPABILITY_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}dms_data_service_capability_enum_v01;
/**
    @}
  */

/** @addtogroup dms_qmi_enums
    @{
  */
typedef enum {
  DMS_SIM_CAPABILITY_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  DMS_SIM_NOT_SUPPORTED_V01 = 0x01, 
  DMS_SIM_SUPPORTED_V01 = 0x02, 
  DMS_SIM_CAPABILITY_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}dms_sim_capability_enum_v01;
/**
    @}
  */

/** @addtogroup dms_qmi_enums
    @{
  */
typedef enum {
  DMS_RADIO_IF_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  DMS_RADIO_IF_1X_V01 = 0x01, 
  DMS_RADIO_IF_1X_EVDO_V01 = 0x02, 
  DMS_RADIO_IF_GSM_V01 = 0x04, 
  DMS_RADIO_IF_UMTS_V01 = 0x05, 
  DMS_RADIO_IF_LTE_V01 = 0x08, 
  DMS_RADIO_IF_TDS_V01 = 0x09, 
  DMS_RADIO_IF_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}dms_radio_if_enum_v01;
/**
    @}
  */

/** @addtogroup dms_qmi_enums
    @{
  */
typedef enum {
  DMS_DEVICE_SERVICE_CAPABILITY_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  DMS_DEVICE_CAP_DATA_ONLY_V01 = 0x01, 
  DMS_DEVICE_CAP_VOICE_ONLY_V01 = 0x02, 
  DMS_DEVICE_CAP_SIMUL_VOICE_AND_DATA_V01 = 0x03, 
  DMS_DEVICE_CAP_NONSIMUL_VOICE_AND_DATA_V01 = 0x04, 
  DMS_DEVICE_SERVICE_CAPABILITY_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}dms_device_service_capability_enum_v01;
/**
    @}
  */

typedef uint64_t dms_voice_support_capability_mask_v01;
#define QMI_DMS_MASK_VOICE_SUPPORT_GW_CSFB_CAPABLE_V01 ((dms_voice_support_capability_mask_v01)0x0001ull) 
#define QMI_DMS_MASK_VOICE_SUPPORT_1x_CSFB_CAPABLE_V01 ((dms_voice_support_capability_mask_v01)0x0002ull) 
#define QMI_DMS_MASK_VOICE_SUPPORT_VOLTE_CAPABLE_V01 ((dms_voice_support_capability_mask_v01)0x0004ull) 
typedef uint64_t dms_simul_voice_and_data_capability_mask_v01;
#define QMI_DMS_MASK_SVLTE_CAPABLE_V01 ((dms_simul_voice_and_data_capability_mask_v01)0x0001ull) 
#define QMI_DMS_MASK_SVDO_CAPABLE_V01 ((dms_simul_voice_and_data_capability_mask_v01)0x0002ull) 
#define QMI_DMS_MASK_SGLTE_CAPABLE_V01 ((dms_simul_voice_and_data_capability_mask_v01)0x0004ull) 
/** @addtogroup dms_qmi_enums
    @{
  */
typedef enum {
  DMS_DEVICE_SUBS_FEATURE_MODE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  DMS_DEVICE_SUBS_FEATURE_MODE_NORMAL_V01 = 0, 
  DMS_DEVICE_SUBS_FEATURE_MODE_SGLTE_V01 = 1, 
  DMS_DEVICE_SUBS_FEATURE_MODE_SVLTE_V01 = 2, 
  DMS_DEVICE_SUBS_FEATURE_MODE_SRLTE_V01 = 3, 
  DMS_DEVICE_SUBS_FEATURE_MODE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}dms_device_subs_feature_mode_enum_v01;
/**
    @}
  */

/** @addtogroup dms_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t max_tx_channel_rate;
  /**<   Maximum Tx transmission rate in bits per second (bps) supported
       by the device. The value 0xFFFFFFFF implies a rate greater than 
       or equal to 0xFFFFFFFF (4 Gbps). In multitechnology devices, this 
       value is the greatest rate among all supported technologies.
  */

  uint32_t max_rx_channel_rate;
  /**<   Maximum Rx transmission rate in bits per second (bps) supported
       by the device. The value 0xFFFFFFFF implies rate greater than or 
       equal to 0xFFFFFFFF (4 Gbps). In multitechnology devices, this
       value is the greatest rate among all supported technologies.
  */

  /*  Note: Below data item is deprecated from QMI DMS version 1.11 in favor of device_service_capability TLV */
  dms_data_service_capability_enum_v01 data_service_capability;
  /**<   Values: \n
       - 0 -- No data services supported \n
       - 1 -- Only circuit-switched (CS) services are supported \n
       - 2 -- Only packet-switched (PS) services are supported \n
       - 3 -- Simultaneous CS and PS \n
       - 4 -- Nonsimultaneous CS and PS
  */

  dms_sim_capability_enum_v01 sim_capability;
  /**<   Values: \n
       - 1 -- SIM is not supported \n
       - 2 -- SIM is supported
  */

  uint32_t radio_if_list_len;  /**< Must be set to # of elements in radio_if_list */
  dms_radio_if_enum_v01 radio_if_list[QMI_DMS_RADIO_IF_LIST_MAX_V01];
  /**<   List of N one-byte elements describing the radio interfaces
       supported by the device. Values: \n
       - 1 -- CDMA2000 1X \n
       - 2 -- CDMA2000 HRPD (1xEV-DO) \n
       - 4 -- GSM \n
       - 5 -- UMTS \n
       - 8 -- LTE \n
       - 9 -- TDS
  */
}dms_device_capabilities_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Requests the device capabilities. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  Device Capabilities */
  dms_device_capabilities_type_v01 device_capabilities;

  /* Optional */
  /*  Device Service Capability */
  uint8_t device_service_capability_valid;  /**< Must be set to true if device_service_capability is being passed */
  dms_device_service_capability_enum_v01 device_service_capability;
  /**<   Values: \n
       - 1 -- Only data services are supported \n
       - 2 -- Only voice services are supported \n
       - 3 -- Simultaneous voice and data \n
       - 4 -- Nonsimultaneous voice and data
  */

  /* Optional */
  /*  Voice Support Capability */
  uint8_t voice_support_capability_valid;  /**< Must be set to true if voice_support_capability is being passed */
  dms_voice_support_capability_mask_v01 voice_support_capability;
  /**<   Bitmask of voice support available on device. Values: \n
       Bit 0 -- GW CSFB        \n
       - 0 -- Not capable      \n
       - 1 -- Capable          \n
       Bit 1 -- 1x CSFB        \n
       - 0 -- Not capable      \n
       - 1 -- Capable          \n
       Bit 2 -- VoLTE          \n
       - 0 -- Not capable      \n
       - 1 -- Capable          
  */

  /* Optional */
  /*  Simultaneous Voice and Data Capability */
  uint8_t simul_voice_and_data_capability_valid;  /**< Must be set to true if simul_voice_and_data_capability is being passed */
  dms_simul_voice_and_data_capability_mask_v01 simul_voice_and_data_capability;
  /**<   Bitmask of simultaneous voice and data support available on the device. Values: \n
       - Bit 0 -- SVLTE capability \n
       - Bit 1 -- SVDO capability  \n
       - Bit 2 -- SGLTE capability  \n
       Note: Zero bits set means that none of the defined capabilities are supported.
  */

  /* Optional */
  /*  Device Multisim Capability
 Note: Below TLV is deprecated from QMI DMS version 1.37 in favor of current_multisim_capability TLV */
  uint8_t multisim_capability_valid;  /**< Must be set to true if multisim_capability is being passed */
  dms_multisim_capability_type_v01 multisim_capability;
  /**<   \n Device capability for supporting multiple simultaneously active radio interfaces.
  */

  /* Optional */
  /*  Device Multisim Voice Data Capability */
  uint8_t current_multisim_capability_valid;  /**< Must be set to true if current_multisim_capability is being passed */
  dms_current_multisim_capability_type_v01 current_multisim_capability;
  /**<   \n Device voice and data capability for supporting multiple simultaneously 
        active radio interfaces.
  */

  /* Optional */
  /*  Current Subscription Capability */
  uint8_t current_subscription_capability_valid;  /**< Must be set to true if current_subscription_capability is being passed */
  uint32_t current_subscription_capability_len;  /**< Must be set to # of elements in current_subscription_capability */
  dms_subs_capability_mask_type_v01 current_subscription_capability[QMI_DMS_MAX_SUBSCRIPTION_LIST_LEN_V01];
  /**<   An array of max_subscriptions entries where each entry is a mask of capabilities.
 The client ignores any bits in the mask that it does not recognize. Values: 
      - DMS_SUBS_CAPABILITY_AMPS (0x00000001) -- 
      - DMS_SUBS_CAPABILITY_CDMA (0x00000002) -- 
      - DMS_SUBS_CAPABILITY_HDR (0x00000004) -- 
      - DMS_SUBS_CAPABILITY_GSM (0x00000008) -- 
      - DMS_SUBS_CAPABILITY_WCDMA (0x00000010) -- 
      - DMS_SUBS_CAPABILITY_LTE (0x00000020) -- 
      - DMS_SUBS_CAPABILITY_TDS (0x00000040) --  
 */

  /* Optional */
  /*  Subscription Voice Data Capability of the Device */
  uint8_t subs_voice_data_capability_valid;  /**< Must be set to true if subs_voice_data_capability is being passed */
  uint32_t subs_voice_data_capability_len;  /**< Must be set to # of elements in subs_voice_data_capability */
  dms_subs_voice_data_capability_type_v01 subs_voice_data_capability[QMI_DMS_MAX_SUBSCRIPTION_LIST_LEN_V01];
  /**<   \n Voice and data capability of each subscription of the device
  */

  /* Optional */
  /*  Subscription feature mode of the Device */
  uint8_t subs_device_feature_mode_valid;  /**< Must be set to true if subs_device_feature_mode is being passed */
  uint32_t subs_device_feature_mode_len;  /**< Must be set to # of elements in subs_device_feature_mode */
  dms_device_subs_feature_mode_enum_v01 subs_device_feature_mode[QMI_DMS_MAX_SUBSCRIPTION_LIST_LEN_V01];
  /**<   \n Device feature mode of each subscription. Values: \n
      - DMS_DEVICE_SUBS_FEATURE_MODE_NORMAL (0) -- 
      - DMS_DEVICE_SUBS_FEATURE_MODE_SGLTE (1) -- 
      - DMS_DEVICE_SUBS_FEATURE_MODE_SVLTE (2) -- 
      - DMS_DEVICE_SUBS_FEATURE_MODE_SRLTE (3) -- 
 */
}dms_get_device_cap_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}dms_get_device_mfr_req_msg_v01;

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Requests the device the manufacturer information. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  Device Manufacturer */
  char device_manufacturer[QMI_DMS_DEVICE_MANUFACTURER_MAX_V01 + 1];
  /**<   String identifying the device manufacturer. */
}dms_get_device_mfr_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}dms_get_device_model_id_req_msg_v01;

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Requests the device model identification. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  Device Model */
  char device_model_id[QMI_DMS_DEVICE_MODEL_ID_MAX_V01 + 1];
  /**<   String identifying the device model. */
}dms_get_device_model_id_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}dms_get_device_rev_id_req_msg_v01;

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Requests the device firmware revision identification. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  Revision ID   */
  char device_rev_id[QMI_DMS_DEVICE_REV_ID_MAX_V01 + 1];
  /**<   String containing the device revision ID. */

  /* Optional */
  /*  Boot Code Revision */
  uint8_t boot_code_rev_valid;  /**< Must be set to true if boot_code_rev is being passed */
  char boot_code_rev[QMI_DMS_BOOT_CODE_REV_MAX_V01 + 1];
  /**<   String containing the boot code revision.
  */

  /* Optional */
  /*  PRI Revision */
  uint8_t pri_rev_valid;  /**< Must be set to true if pri_rev is being passed */
  char pri_rev[QMI_DMS_PRI_REV_MAX_V01 + 1];
  /**<   String containing the device PRI revision.
  */
}dms_get_device_rev_id_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}dms_get_msisdn_req_msg_v01;

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Requests the assigned voice number. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  Voice Number */
  char voice_number[QMI_DMS_VOICE_NUMBER_MAX_V01 + 1];
  /**<   String containing the voice number in use by the device. */

  /* Optional */
  /*  Mobile ID */
  uint8_t mobile_id_number_valid;  /**< Must be set to true if mobile_id_number is being passed */
  char mobile_id_number[QMI_DMS_MOBILE_ID_NUMBER_MAX_V01 + 1];
  /**<   String containing the mobile ID number of the device. */

  /* Optional */
  /*  International Mobile Subscriber ID */
  uint8_t imsi_valid;  /**< Must be set to true if imsi is being passed */
  char imsi[QMI_DMS_IMSI_MAX_V01 + 1];
  /**<   String containing the international mobile subscriber ID of the device.
  */
}dms_get_msisdn_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}dms_get_device_serial_numbers_req_msg_v01;

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Requests the serial numbers of the device. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  ESN */
  uint8_t esn_valid;  /**< Must be set to true if esn is being passed */
  char esn[QMI_DMS_ESN_MAX_V01 + 1];
  /**<   String containing the Electronic Serial Number (ESN) of the device. */

  /* Optional */
  /*  IMEI */
  uint8_t imei_valid;  /**< Must be set to true if imei is being passed */
  char imei[QMI_DMS_IMEI_MAX_V01 + 1];
  /**<   String containing the International Mobile Equipment Identity
      (IMEI) of the device.
  */

  /* Optional */
  /*  MEID */
  uint8_t meid_valid;  /**< Must be set to true if meid is being passed */
  char meid[QMI_DMS_MEID_MAX_V01 + 1];
  /**<   String containing the Mobile Equipment Identifier (MEID) of the device. */

  /* Optional */
  /*  IMEI SVN */
  uint8_t imeisv_svn_valid;  /**< Must be set to true if imeisv_svn is being passed */
  char imeisv_svn[QMI_DMS_IMEISV_MAX_V01 + 1];
  /**<   IMEI software version number
  */
}dms_get_device_serial_numbers_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}dms_get_encrypted_device_serial_numbers_req_msg_v01;

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Requests the encrypted serial numbers of the device. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  ESN */
  uint8_t esn_valid;  /**< Must be set to true if esn is being passed */
  char esn[QMI_DMS_ENCRYPTED_ESN_MAX_V01 + 1];
  /**<   String containing the encrypted Electronic Serial Number (ESN) of the device. */

  /* Optional */
  /*  IMEI */
  uint8_t imei_valid;  /**< Must be set to true if imei is being passed */
  char imei[QMI_DMS_ENCRYPTED_IMEI_MAX_V01 + 1];
  /**<   String containing the encrypted International Mobile Equipment Identity
      (IMEI) of the device.
  */

  /* Optional */
  /*  MEID */
  uint8_t meid_valid;  /**< Must be set to true if meid is being passed */
  char meid[QMI_DMS_ENCRYPTED_MEID_MAX_V01 + 1];
  /**<   String containing the encrypted Mobile Equipment Identifier (MEID) of the device. */

  /* Optional */
  /*  IMEI SVN */
  uint8_t imeisv_svn_valid;  /**< Must be set to true if imeisv_svn is being passed */
  char imeisv_svn[QMI_DMS_ENCRYPTED_IMEISV_MAX_V01 + 1];
  /**<   String containing the encrypted IMEI software version number
  */
}dms_get_encrypted_device_serial_numbers_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}dms_get_power_state_req_msg_v01;

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Requests the power status of the device. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  Power State */
  dms_power_state_type_v01 power_state;
}dms_get_power_state_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dms_qmi_enums
    @{
  */
typedef enum {
  DMS_PIN_ID_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  DMS_QMI_PIN_ID_PIN_1_V01 = 0x01, 
  DMS_QMI_PIN_ID_PIN_2_V01 = 0x02, 
  DMS_PIN_ID_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}dms_pin_id_enum_v01;
/**
    @}
  */

/** @addtogroup dms_qmi_aggregates
    @{
  */
typedef struct {

  dms_pin_id_enum_v01 pin_id;
  /**<   Specifies the ID of the PIN to be enabled or disabled. Values: \n
       - 1 -- PIN1 (also called PIN) \n
       - 2 -- PIN2
  */

  uint8_t protection_setting_enabled;
  /**<   Specifies whether the PIN is enabled. Values: \n 
     - 0 -- Disable PIN \n
     - 1 -- Enable PIN
  */

  uint32_t pin_value_len;  /**< Must be set to # of elements in pin_value */
  uint8_t pin_value[QMI_DMS_PIN_VALUE_MAX_V01];
  /**<   Specifies the PIN value of the PIN to be enabled/disabled. The
       protection setting is only changed if this value is
       successfully verified by the SIM.
  */
}dms_pin_protection_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dms_qmi_messages
    @{
  */
/** Request Message; Enables or disables protection of UIM contents by a
           specified PIN. (Deprecated) */
typedef struct {

  /* Mandatory */
  /*  PIN Protection Information */
  dms_pin_protection_info_type_v01 pin_protection_info;
}dms_uim_set_pin_protection_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dms_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t verify_retries_left;
  /**<   Number of retries left, after which the PIN is blocked. */

  uint8_t unblock_retries_left;
  /**<   Number of unblock retries left, after which the PIN is
       permanently blocked, i.e., the UIM is unusable.
  */
}dms_pin_retries_status_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Enables or disables protection of UIM contents by a
           specified PIN. (Deprecated) */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Pin Retries Status */
  uint8_t pin_retries_status_valid;  /**< Must be set to true if pin_retries_status is being passed */
  dms_pin_retries_status_type_v01 pin_retries_status;
}dms_uim_set_pin_protection_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dms_qmi_aggregates
    @{
  */
typedef struct {

  dms_pin_id_enum_v01 pin_id;
  /**<   Specifies the ID of the PIN to be enabled or disabled. Values: \n
       - 1 -- PIN1 (also called PIN) \n
       - 2 -- PIN2
  */

  uint32_t pin_value_len;  /**< Must be set to # of elements in pin_value */
  uint8_t pin_value[QMI_DMS_PIN_VALUE_MAX_V01];
  /**<   Specifies the PIN value of the PIN to be verified; the
       protection setting is only changed if this value is
       successfully verified by the SIM.
  */
}dms_pin_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dms_qmi_messages
    @{
  */
/** Request Message; Verifies the PIN before accessing the UIM contents. (Deprecated) */
typedef struct {

  /* Mandatory */
  /*  PIN Value */
  dms_pin_info_type_v01 pin_info;
}dms_uim_verify_pin_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Verifies the PIN before accessing the UIM contents. (Deprecated) */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  PIN Retries Status */
  uint8_t pin_retries_status_valid;  /**< Must be set to true if pin_retries_status is being passed */
  dms_pin_retries_status_type_v01 pin_retries_status;
}dms_uim_verify_pin_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dms_qmi_aggregates
    @{
  */
typedef struct {

  dms_pin_id_enum_v01 unblock_pin_id;
  /**<   Specifies the ID of the PIN to be unblocked. Values: \n
       - 1 -- PIN1 (also called PIN) \n
       - 2 -- PIN2
  */

  uint32_t puk_value_len;  /**< Must be set to # of elements in puk_value */
  uint8_t puk_value[QMI_DMS_PUK_VALUE_MAX_V01];
  /**<   Specifies the PUK value (password) of the PIN to be unblocked. */

  uint32_t new_pin_value_len;  /**< Must be set to # of elements in new_pin_value */
  uint8_t new_pin_value[QMI_DMS_PUK_VALUE_MAX_V01];
  /**<   Specifies the new PIN value (password) for the PIN to be unblocked. */
}dms_pin_unblock_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dms_qmi_messages
    @{
  */
/** Request Message; Unblocks a blocked PIN. (Deprecated) */
typedef struct {

  /* Mandatory */
  /*  PIN Unblock Information */
  dms_pin_unblock_info_type_v01 pin_unblock_info;
}dms_uim_unblock_pin_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Unblocks a blocked PIN. (Deprecated) */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  PIN Retries Status */
  uint8_t pin_retries_status_valid;  /**< Must be set to true if pin_retries_status is being passed */
  dms_pin_retries_status_type_v01 pin_retries_status;
}dms_uim_unblock_pin_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dms_qmi_aggregates
    @{
  */
typedef struct {

  dms_pin_id_enum_v01 pin_id;
  /**<   Specifies the ID of the PIN to be changed. Values: \n
       - 1 -- PIN1 (also called PIN) \n
       - 2 -- PIN2
  */

  uint32_t old_pin_value_len;  /**< Must be set to # of elements in old_pin_value */
  uint8_t old_pin_value[QMI_DMS_PIN_VALUE_MAX_V01];
  /**<   Specifies the old PIN value (old password) of the PIN. */

  uint32_t new_pin_value_len;  /**< Must be set to # of elements in new_pin_value */
  uint8_t new_pin_value[QMI_DMS_PIN_VALUE_MAX_V01];
  /**<   Specifies the new PIN value (new password) of the PIN. */
}dms_pin_change_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dms_qmi_messages
    @{
  */
/** Request Message; Changes the PIN value. (Deprecated) */
typedef struct {

  /* Mandatory */
  /*  PIN Change Information */
  dms_pin_change_info_type_v01 pin_change_info;
}dms_uim_change_pin_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Changes the PIN value. (Deprecated) */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  PIN Retries Status */
  uint8_t pin_retries_status_valid;  /**< Must be set to true if pin_retries_status is being passed */
  dms_pin_retries_status_type_v01 pin_retries_status;
}dms_uim_change_pin_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}dms_uim_get_pin_status_req_msg_v01;

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Gets the status of a PIN. (Deprecated) */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  PIN 1 Status */
  uint8_t pin1_status_valid;  /**< Must be set to true if pin1_status is being passed */
  dms_pin_status_type_v01 pin1_status;

  /* Optional */
  /*  PIN 2 Status */
  uint8_t pin2_status_valid;  /**< Must be set to true if pin2_status is being passed */
  dms_pin_status_type_v01 pin2_status;
}dms_uim_get_pin_status_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}dms_get_device_hardware_rev_req_msg_v01;

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Queries the hardware revision of the device. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  Hardware Revision */
  char hardware_rev[QMI_DMS_HARDWARE_REV_MAX_V01 + 1];
  /**<   String containing the hardware revision of the device. */
}dms_get_device_hardware_rev_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}dms_get_operating_mode_req_msg_v01;

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Queries the current operating mode of the device. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  Operating Mode */
  dms_operating_mode_enum_v01 operating_mode;
  /**<   Selected operating mode. Values: \n
       - 0 -- Online \n
       - 1 -- Low power \n
       - 2 -- Factory Test mode \n
       - 3 -- Offline \n
       - 4 -- Resetting \n
       - 5 -- Shutting down \n
       - 6 -- Persistent low power \n
       - 8 -- Conducting network test for GSM/WCDMA
  */

  /* Optional */
  /*  Offline Reason */
  uint8_t offline_reason_valid;  /**< Must be set to true if offline_reason is being passed */
  dms_offline_reason_mask_v01 offline_reason;
  /**<   Offline reason bitmask. All unlisted bits are reserved for 
       future use and are ignored. Values: \n
       - 0x0001 -- Host image misconfiguration \n
       - 0x0002 -- PRI image misconfiguration \n
       - 0x0004 -- PRI version incompatible \n
       - 0x0008 -- Device memory is full, cannot copy PRI information
       
  */

  /* Optional */
  /*  Hardware-Restricted Mode */
  uint8_t hardware_controlled_mode_valid;  /**< Must be set to true if hardware_controlled_mode is being passed */
  uint8_t hardware_controlled_mode;
  /**<   Hardware-Restricted mode. Values: \n
       - 0x01 -- TRUE
  */
}dms_get_operating_mode_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dms_qmi_messages
    @{
  */
/** Request Message; Sets the operating mode of the device. */
typedef struct {

  /* Mandatory */
  /*  Operating Mode */
  dms_operating_mode_enum_v01 operating_mode;
  /**<   Selected operating mode. Values:  \n
       - 0 -- Online \n
       - 1 -- Low power \n
       - 2 -- Factory Test mode \n
       - 3 -- Offline \n
       - 4 -- Resetting \n
       - 5 -- Shutting down \n
       - 6 -- Persistent low power \n
       - 7 -- Mode-only low power
  */
}dms_set_operating_mode_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Sets the operating mode of the device. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}dms_set_operating_mode_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}dms_get_time_req_msg_v01;

/** @addtogroup dms_qmi_enums
    @{
  */
typedef enum {
  DMS_TIME_SOURCE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  DMS_TIME_SOURCE_DEVICE_CLOCK_V01 = 0x00, 
  DMS_TIME_SOURCE_CDMA_V01 = 0x01, 
  DMS_TIME_SOURCE_HDR_V01 = 0x02, 
  DMS_TIME_SOURCE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}dms_time_source_enum_v01;
/**
    @}
  */

/** @addtogroup dms_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t time_count[6];
  /**<   Count of 1.25 ms that have elapsed from the start of GPS Epoch 
       time (January 6, 1980). A 6-byte integer in little-endian format.
  */

  dms_time_source_enum_v01 time_source;
  /**<   Source of the timestamp. Values: \n 
       - 0 -- 32 kHz device clock \n
       - 1 -- CDMA network \n
       - 2 -- HDR network
  */
}dms_device_time_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Queries the current time of the device. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  Device Time */
  dms_device_time_type_v01 device_time;

  /* Optional */
  /*  System Time in Milliseconds */
  uint8_t sys_time_in_ms_valid;  /**< Must be set to true if sys_time_in_ms is being passed */
  uint64_t sys_time_in_ms;
  /**<   Count of system time in milliseconds that have
       elapsed from the start of GPS Epoch time
       (Jan 6, 1980).
  */

  /* Optional */
  /*  User Time in Milliseconds */
  uint8_t user_time_in_ms_valid;  /**< Must be set to true if user_time_in_ms is being passed */
  uint64_t user_time_in_ms;
  /**<   Count of user time in milliseconds that have
       elapsed from the start of GPS Epoch time
       (Jan 6, 1980).
  */
}dms_get_time_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}dms_get_prl_ver_req_msg_v01;

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Queries the version of the active Preferred Roaming
           List (PRL) of the device. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  PRL Version */
  uint16_t prl_version;
  /**<   PRL version.*/

  /* Optional */
  /*  PRL-Only Preference */
  uint8_t prl_only_valid;  /**< Must be set to true if prl_only is being passed */
  uint8_t prl_only;
  /**<   Values: \n
       - 0 -- Unset \n
       - 1 -- Set
  */
}dms_get_prl_ver_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}dms_get_activation_state_req_msg_v01;

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Queries the activation state of the device. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  Activation State */
  dms_activation_state_enum_v01 activation_state;
  /**<   Service activation state. Values: \n
       - 0x00 -- Service is not activated \n
       - 0x01 --Service is activated \n
       - 0x02 -- Activation is connecting - Network
                connection in progress for automatic activation of service \n
       - 0x03 -- Activation is connected - Network
                connection is connected for automatic activation of service \n
       - 0x04 -- OTASP security is authenticated \n
       - 0x05 -- OTASP NAM is downloaded \n
       - 0x06 -- OTASP MDN is downloaded \n
       - 0x07 -- OTASP IMSI is downloaded \n
       - 0x08 -- OTASP PRL is downloaded \n
       - 0x09 -- OTASP SPC is downloaded \n
       - 0x0A -- OTASP settings are committed 
  */
}dms_get_activation_state_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dms_qmi_aggregates
    @{
  */
typedef struct {

  char act_code[QMI_DMS_ACTIVATION_CODE_MAX_V01 + 1];
  /**<   Activation code to be used by the default activation type
       for the device in ASCII format (maximum 81 bytes).
  */
}dms_activation_code_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dms_qmi_messages
    @{
  */
/** Request Message; Requests that the device perform automatic
           service activation. */
typedef struct {

  /* Mandatory */
  /*  Activation Code */
  dms_activation_code_type_v01 activation_code;
}dms_activate_automatic_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Requests that the device perform automatic
           service activation. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}dms_activate_automatic_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dms_qmi_aggregates
    @{
  */
typedef struct {

  char spc[QMI_DMS_SPC_LEN_V01];
  /**<   Service programming code in ASCII format (digits 0 to 9 only).
  */

  uint16_t sid;
  /**<   System identification number
  */

  char mdn[QMI_DMS_MDN_MAX_V01 + 1];
  /**<   String containing the mobile directory number (maximum 15 bytes).
  */

  char min[QMI_DMS_MIN_MAX_V01 + 1];
  /**<   String containing the mobile identification number (maximum 15 bytes).
  */
}dms_manual_act_data_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dms_qmi_aggregates
    @{
  */
typedef struct {

  char mn_ha_key[QMI_DMS_HA_KEY_MAX_V01 + 1];
  /**<   String containing the MN-HA key (maximum 16 bytes).*/
}dms_mn_ha_key_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dms_qmi_aggregates
    @{
  */
typedef struct {

  char mn_aaa_key[QMI_DMS_AAA_KEY_MAX_V01 + 1];
  /**<   String containing the MN-AAA key (maximum 16 bytes).*/
}dms_mn_aaa_key_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dms_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t prl_total_len;
  /**<    PRL total length (maximum 16384)*/

  uint8_t prl_seg_num;
  /**<   PRL segment sequence number*/

  uint32_t prl_len;  /**< Must be set to # of elements in prl */
  uint8_t prl[QMI_DMS_PRL_DATA_MAX_V01];
  /**<   PRL segment data*/
}dms_pref_roaming_list_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dms_qmi_messages
    @{
  */
/** Request Message; Requests that the device perform manual service activation. */
typedef struct {

  /* Mandatory */
  /*  Manual Activation Data */
  dms_manual_act_data_type_v01 activation_data;

  /* Optional */
  /*  MN-HA Key */
  uint8_t mn_ha_key_valid;  /**< Must be set to true if mn_ha_key is being passed */
  dms_mn_ha_key_type_v01 mn_ha_key;

  /* Optional */
  /*  MN-AAA Key */
  uint8_t mn_aaa_key_valid;  /**< Must be set to true if mn_aaa_key is being passed */
  dms_mn_aaa_key_type_v01 mn_aaa_key;

  /* Optional */
  /*  Preferred Roaming List */
  uint8_t pref_roaming_list_valid;  /**< Must be set to true if pref_roaming_list is being passed */
  dms_pref_roaming_list_type_v01 pref_roaming_list;
}dms_activate_manual_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Requests that the device perform manual service activation. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}dms_activate_manual_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * dms_swi_get_reset_info_req_msg is empty
 * typedef struct {
 * }dms_swi_get_reset_info_req_msg_v01;
 */

/** @addtogroup reset_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t type;
  /**<
    0 - unknown
    1 - warm
    2 - hard
    3 - crash
    4 - power-down
   */


  uint8_t source;
  /**<
    0 - unknown
    1 - user requested
    2 - hardware switch
    3 - temperature critical
    4 - voltage critical
    5 - configuration update
    6 - LWM2M
    7 - OMA-DM
    8 - FOTA
    9 - SW update
    10 - SWAP
    11 - SWAP_SYNC
   */
}reset_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup reset_qmi_messages
    @{
  */
/** Response Message; This command queries the reason for the most recent device reset or powerdown. */
typedef struct {

  /* Mandatory */
  /*  reset_information */
  reset_info_type_v01 reset_info;

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}dms_swi_get_reset_info_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}dms_get_user_lock_state_req_msg_v01;

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Queries the state of the user lock maintained by the
           device. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  User Lock State */
  uint8_t lock_enabled;
  /**<   Current state of the lock. Values: \n
       - 0 -- Disabled \n
       - 1 -- Enabled
  */
}dms_get_user_lock_state_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dms_qmi_enums
    @{
  */
typedef enum {
  DMS_LOCK_STATE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  DMS_LOCK_DISABLED_V01 = 0x00, 
  DMS_LOCK_ENABLED_V01 = 0x01, 
  DMS_LOCK_STATE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}dms_lock_state_enum_v01;
/**
    @}
  */

/** @addtogroup dms_qmi_aggregates
    @{
  */
typedef struct {

  dms_lock_state_enum_v01 lock_state;
  /**<   Current state of the lock. Values: \n
       - 0 -- Disabled \n
       - 1 -- Enabled
  */

  char lock_code[QMI_DMS_LOCK_CODE_LEN_V01];
  /**<   4-byte code set for the lock in ASCII format (digits 0 to 9 only). */
}dms_user_lock_state_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dms_qmi_messages
    @{
  */
/** Request Message; Sets the user lock state maintained by the device. */
typedef struct {

  /* Mandatory */
  /*  User Lock State */
  dms_user_lock_state_info_type_v01 lock_info;
}dms_set_user_lock_state_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Sets the user lock state maintained by the device. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}dms_set_user_lock_state_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dms_qmi_aggregates
    @{
  */
typedef struct {

  char cur_code[QMI_DMS_LOCK_CODE_LEN_V01];
  /**<   Current 4-byte code to use for the lock in ASCII format (digits 0 to
       9 only).
  */

  char new_code[QMI_DMS_LOCK_CODE_LEN_V01];
  /**<   New 4-byte code to use for the lock in ASCII format (digits 0 to
       9 only).
  */
}dms_user_lock_set_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dms_qmi_messages
    @{
  */
/** Request Message; Sets the user lock code maintained by the device. */
typedef struct {

  /* Mandatory */
  /*  User Lock Code */
  dms_user_lock_set_info_type_v01 lock_info;
}dms_set_user_lock_code_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Sets the user lock code maintained by the device. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}dms_set_user_lock_code_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}dms_read_user_data_req_msg_v01;

/** @addtogroup dms_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t data_len;  /**< Must be set to # of elements in data */
  uint8_t data[QMI_DMS_USER_DATA_MAX_V01];
  /**<   User data from/to persistent storage (maximum 512).*/
}dms_user_data_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Queries the user data maintained by the device. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  User Data */
  dms_user_data_type_v01 user_data;
}dms_read_user_data_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dms_qmi_messages
    @{
  */
/** Request Message; Writes user data maintained by the device. */
typedef struct {

  /* Mandatory */
  /*  User Data */
  dms_user_data_type_v01 user_data;
}dms_write_user_data_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Writes user data maintained by the device. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}dms_write_user_data_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}dms_read_eri_file_req_msg_v01;

/** @addtogroup dms_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t eri_data_len;  /**< Must be set to # of elements in eri_data */
  uint8_t eri_data[QMI_DMS_ERI_DATA_MAX_V01];
  /**<   ERI data read from persistent storage (maximum 1024).*/
}dms_eri_data_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Queries the Extended Roaming Indicator (ERI) file stored on the device. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  ERI File */
  dms_eri_data_type_v01 eri_file;
}dms_read_eri_file_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dms_qmi_messages
    @{
  */
/** Request Message; Requests that the device reset all settings to
           factory defined values. */
typedef struct {

  /* Mandatory */
  /*  Service Programming Code */
  char spc[QMI_DMS_SPC_LEN_V01];
  /**<   Service programming code in ASCII format (digits 0 to 9 only).
  */
}dms_restore_factory_defaults_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Requests that the device reset all settings to
           factory defined values. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}dms_restore_factory_defaults_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dms_qmi_messages
    @{
  */
/** Request Message; Requests the device to validate a specified service
           programming code. */
typedef struct {

  /* Mandatory */
  /*  Programming Code */
  char spc[QMI_DMS_SPC_LEN_V01];
  /**<   Service programming code in ASCII format (digits 0 to 9 only). */
}dms_validate_service_programming_code_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Requests the device to validate a specified service
           programming code. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}dms_validate_service_programming_code_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}dms_uim_get_iccid_req_msg_v01;

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Queries the Integrated Circuit Card ID (ICCID) of the UIM for the device. (Deprecated) */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  UIM ICCID */
  char uim_id[QMI_DMS_UIM_ID_MAX_V01 + 1];
  /**<   String containing the UIM ICCID. */
}dms_uim_get_iccid_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dms_qmi_enums
    @{
  */
typedef enum {
  DMS_GSM_PERSO_FACILITY_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  DMS_PERSO_FACILITY_NETWORK_V01 = 0x00, 
  DMS_PERSO_FACILITY_NETWORK_SUBSET_V01 = 0x01, 
  DMS_PERSO_FACILITY_SERVICE_PROVIDER_V01 = 0x02, 
  DMS_PERSO_FACILITY_CORPORATE_V01 = 0x03, 
  DMS_PERSO_FACILITY_UIM_V01 = 0x04, 
  DMS_GSM_PERSO_FACILITY_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}dms_gsm_perso_facility_enum_v01;
/**
    @}
  */

/** @addtogroup dms_qmi_messages
    @{
  */
/** Request Message; Queries the status of a UIM facility control key. (Deprecated) */
typedef struct {

  /* Mandatory */
  /*  UIM Personalization Facility */
  dms_gsm_perso_facility_enum_v01 facility;
  /**<   MT or network facility (corresponding AT+CLCK value). Values: \n
       - 0 -- Network personalization (PN) \n
       - 1 -- Network subset personalization (PU) \n
       - 2 -- Service provider personalization (PP) \n
       - 3 -- Corporate personalization (PC) \n
       - 4 -- UIM personalization (PF)
  */
}dms_uim_get_ck_status_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dms_qmi_enums
    @{
  */
typedef enum {
  DMS_FACILITY_STATE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  DMS_FACILITY_DEACTIVATED_V01 = 0x00, 
  DMS_FACILITY_ACTIVATED_V01 = 0x01, 
  DMS_FACILITY_BLOCKED_V01 = 0x02, 
  DMS_FACILITY_STATE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}dms_facility_state_enum_v01;
/**
    @}
  */

/** @addtogroup dms_qmi_aggregates
    @{
  */
typedef struct {

  dms_facility_state_enum_v01 facility_state;
  /**<   UIM facility state. Values: \n
       - 0 -- Deactivated \n
       - 1 -- Activated \n
       - 2 -- Blocked
  */

  uint8_t verify_reties_left;
  /**<   Indicates the number of retries left, after which the CK is blocked. */

  uint8_t unblock_retries_left;
  /**<   Number of unblock retries left, after which the CK is permanently blocked.
   */
}dms_facility_state_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Queries the status of a UIM facility control key. (Deprecated) */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  Facility CK Status */
  dms_facility_state_info_type_v01 facility_info;

  /* Optional */
  /*  Operation Blocking Facility */
  uint8_t operation_blocking_valid;  /**< Must be set to true if operation_blocking is being passed */
  uint8_t operation_blocking;
  /**<   Presence of this TLV indicates that this facility is currently
       blocking normal operation of the device. This value can be
       returned only if the facility_state is not 0 (deactivated).

       Note: This value is set to 1 when the TLV is provided.
  */
}dms_uim_get_ck_status_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dms_qmi_aggregates
    @{
  */
typedef struct {

  dms_gsm_perso_facility_enum_v01 facility;
  /**<   UIM Personalization facility (corresponding AT+CLCK value). Values: \n
       - 0 -- Network personalization (PN) \n
       - 1 -- Network subset personalization (PU) \n
       - 2 -- Service provider personalization (PP) \n
       - 3 -- Corporate personalization (PC) \n
       - 4 -- UIM personalization (PF)
  */

  dms_facility_state_enum_v01 facility_state;
  /**<   UIM facility state. Values: \n
       - 0 -- Deactivated
  */

  char facility_ck[QMI_DMS_FACILITY_CK_MAX_V01 + 1];
  /**<   Facility depersonalization control key string in ASCII text
       (maximum 8 bytes).
   */
}dms_facility_set_ck_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dms_qmi_messages
    @{
  */
/** Request Message; Sets the protection of a UIM facility control key. (Deprecated) */
typedef struct {

  /* Mandatory */
  /*  UIM Personalization Facility */
  dms_facility_set_ck_info_type_v01 facility_set_ck_info;
}dms_uim_set_ck_protection_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Sets the protection of a UIM facility control key. (Deprecated) */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Facility CK Retry Status */
  uint8_t verify_retries_left_valid;  /**< Must be set to true if verify_retries_left is being passed */
  uint8_t verify_retries_left;
  /**<   Number of retries left, after which the CK is blocked.
   */
}dms_uim_set_ck_protection_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dms_qmi_aggregates
    @{
  */
typedef struct {

  dms_gsm_perso_facility_enum_v01 facility;
  /**<   UIM personalization facility (corresponding AT+CLCK value). Values:\n
       - 0 -- Network personalization (PN) \n
       - 1 -- Network subset personalization (PU) \n
       - 2 -- Service provider personalization (PP) \n
       - 3 -- Corporate personalization (PC) \n
       - 4 -- UIM personalization (PF)
  */

  char facility_unblock_ck[QMI_DMS_FACILITY_UNBLOCK_CK_MAX_V01 + 1];
  /**<   Facility control key string in ASCII text (maximum 8 bytes). */
}dms_facility_unblock_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dms_qmi_messages
    @{
  */
/** Request Message; Unblocks a UIM facility control key. (Deprecated) */
typedef struct {

  /* Mandatory */
  /*  UIM Personalization Facility */
  dms_facility_unblock_info_type_v01 facility_unblock_info;
}dms_uim_unblock_ck_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Unblocks a UIM facility control key. (Deprecated) */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Facility CK Retry Status */
  uint8_t unblock_retries_left_valid;  /**< Must be set to true if unblock_retries_left is being passed */
  uint8_t unblock_retries_left;
  /**<   Number of unblock retries left, after which the CK is
       permanently blocked.
   */
}dms_uim_unblock_ck_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}dms_uim_get_imsi_req_msg_v01;

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Queries the International Mobile Station Identity (IMSI) 
           of the UIM for the device. (Deprecated) */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  International Mobile Subscriber ID */
  char imsi[QMI_DMS_IMSI_MAX_V01 + 1];
  /**<   String containing the international mobile subscriber ID. */
}dms_uim_get_imsi_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}dms_uim_get_state_req_msg_v01;

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Queries the state of the UIM. (Deprecated) */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  UIM State */
  dms_uim_state_enum_v01 uim_state;
  /**<   UIM state. Values: \n
       - 0x00 -- UIM initialization completed \n
       - 0x01 -- UIM is locked or the UIM failed \n
       - 0x02 -- UIM is not present \n
       - 0x03 -- Reserved \n
       - 0xFF -- UIM state is currently unavailable 
  */
}dms_uim_get_state_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}dms_get_band_capability_req_msg_v01;

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Queries the band capability of the device. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  Band Capability */
  dms_band_capability_mask_v01 band_capability;
  /**<   Bitmask of bands supported by the device; see Appendix  
  \ref{app:BandCapability} for the definition of these 
       values.
  */

  /* Optional */
  /*  LTE Band Capability */
  uint8_t lte_band_capability_valid;  /**< Must be set to true if lte_band_capability is being passed */
  dms_lte_band_capability_mask_v01 lte_band_capability;
  /**<   This TLV is present on devices that support LTE bands.
       Bitmask of LTE bands supported by the device; see Appendix  
       \ref{app:LTEBandCapability} for the definition of these 
       values. 
  */

  /* Optional */
  /*  TDS Band Capability */
  uint8_t tds_band_capability_valid;  /**< Must be set to true if tds_band_capability is being passed */
  dms_tds_band_capability_mask_v01 tds_band_capability;
  /**<   This TLV is present on devices that support TDS bands.
       Bitmask of TDS bands supported by the device.        
       Values: \n
       - Bit 0 -- TDS Band A 1900 to 1920 MHz, 2010 to 2020 MHz \n
       - Bit 1 -- TDS Band B 1850 to 1910 MHz, 1930 to 1990 MHz \n
       - Bit 2 -- TDS Band C 1910 to 1930 MHz \n
       - Bit 3 -- TDS Band D 2570 to 2620 MHz \n
       - Bit 4 -- TDS Band E 2300 to 2400 MHz \n
       - Bit 5 -- TDS Band F 1880 to 1920 MHz
  */
}dms_get_band_capability_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}dms_get_factory_sku_req_msg_v01;

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Queries the factory provisioned Stock Keeping
           Unit (SKU). */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  Factory SKU */
  char factory_serial_number[QMI_DMS_FACTORY_SN_MAX_V01 + 1];
  /**<   Factory serial number string in ASCII format (maximum 128 bytes).
  */
}dms_get_factory_sku_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dms_qmi_enums
    @{
  */
typedef enum {
  DMS_TIME_REF_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  DMS_TIME_REF_TYPE_USER_V01 = 0x00000000, 
  DMS_TIME_REF_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}dms_time_ref_type_enum_v01;
/**
    @}
  */

/** @addtogroup dms_qmi_messages
    @{
  */
/** Request Message; Sets the time on the device. */
typedef struct {

  /* Mandatory */
  /*  Time */
  uint64_t time_in_ms;
  /**<   Count of time in milliseconds that have elapsed
         from the start of GPS Epoch time (Jan 6, 1980).
   */

  /* Optional */
  /*  Time Reference Type */
  uint8_t time_reference_type_valid;  /**< Must be set to true if time_reference_type is being passed */
  dms_time_ref_type_enum_v01 time_reference_type;
  /**<   Time reference used while setting the time. Values: \n
       - 0x00000000 -- User time \n
       - 0x00000001 to 0xFFFFFFFF -- Reserved for
         future extension
  */
}dms_set_time_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Sets the time on the device. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}dms_set_time_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}dms_get_alt_net_config_req_msg_v01;

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Queries the alternative network interface configuration 
           used for the device. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  Alternative Net Configuration */
  uint8_t alt_net_config;
  /**<   Alternative network interface configuration. If not provisioned, the 
       Disabled setting is used by the device as default. Values: \n
       - 0 -- Disabled \n
       - 1 -- Enabled 
    */
}dms_get_alt_net_config_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dms_qmi_messages
    @{
  */
/** Request Message; Sets the alternative network interface configuration
           used for the device. */
typedef struct {

  /* Mandatory */
  /*  Alternative Net Configuration */
  uint8_t alt_net_config;
  /**<   Alternative network interface configuration. Values: \n
       - 0 -- Disabled \n
       - 1 -- Enabled 
  */
}dms_set_alt_net_config_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Sets the alternative network interface configuration
           used for the device. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}dms_set_alt_net_config_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dms_qmi_enums
    @{
  */
typedef enum {
  DMS_IMAGE_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  DMS_IMAGE_UNKNOWN_V01 = 0, /**<  Unknown \n  */
  DMS_IMAGE_SBL_V01 = 1, /**<  SBL \n  */
  DMS_IMAGE_TZ_V01 = 2, /**<  TZ \n  */
  DMS_IMAGE_TZSECAPP_V01 = 3, /**<  TZSECAPP \n  */
  DMS_IMAGE_RPM_V01 = 4, /**<  RPM \n  */
  DMS_IMAGE_SDI_V01 = 5, /**<  SDI \n  */
  DMS_IMAGE_HYPERVISOR_V01 = 6, /**<  HYPERVISOR \n  */
  DMS_IMAGE_APPSBL_V01 = 7, /**<  APPSBL \n  */
  DMS_IMAGE_APPS_V01 = 8, /**<  APPS \n  */
  DMS_IMAGE_MPSS_V01 = 9, /**<  MPSS \n  */
  DMS_IMAGE_ADSP_V01 = 10, /**<  ADSP \n  */
  DMS_IMAGE_WCNS_V01 = 11, /**<  WCNS \n  */
  DMS_IMAGE_VENUS_V01 = 12, /**<  VENUS \n  */
  DMS_IMAGE_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}dms_image_type_enum_v01;
/**
    @}
  */

/** @addtogroup dms_qmi_aggregates
    @{
  */
typedef struct {

  dms_image_type_enum_v01 image_type;
  /**<   Image type. Values: \n
      - DMS_IMAGE_UNKNOWN (0) --  Unknown \n 
      - DMS_IMAGE_SBL (1) --  SBL \n 
      - DMS_IMAGE_TZ (2) --  TZ \n 
      - DMS_IMAGE_TZSECAPP (3) --  TZSECAPP \n 
      - DMS_IMAGE_RPM (4) --  RPM \n 
      - DMS_IMAGE_SDI (5) --  SDI \n 
      - DMS_IMAGE_HYPERVISOR (6) --  HYPERVISOR \n 
      - DMS_IMAGE_APPSBL (7) --  APPSBL \n 
      - DMS_IMAGE_APPS (8) --  APPS \n 
      - DMS_IMAGE_MPSS (9) --  MPSS \n 
      - DMS_IMAGE_ADSP (10) --  ADSP \n 
      - DMS_IMAGE_WCNS (11) --  WCNS \n 
      - DMS_IMAGE_VENUS (12) --  VENUS \n 
 */

  char image_ver[QMI_DMS_IMAGE_VER_LEN_MAX_V01 + 1];
  /**<   Image version string. The format of the string is 
       [QC version]:[Variant]:[OEM version]
  */
}dms_image_ver_type_v01;  /* Type */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}dms_get_sw_version_req_msg_v01;

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Queries the software version from the device. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  Software Version Information */
  char sw_version[QMI_DMS_SW_VERSION_MAX_V01 + 1];
  /**<   String representing the software version information.
  */

  /* Optional */
  /*  Image Versions */
  uint8_t image_versions_valid;  /**< Must be set to true if image_versions is being passed */
  uint32_t image_versions_len;  /**< Must be set to # of elements in image_versions */
  dms_image_ver_type_v01 image_versions[QMI_DMS_IMAGE_VER_MAX_V01];
  /**<   Image versions of the builds loaded on the device.
  */
}dms_get_sw_version_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dms_qmi_messages
    @{
  */
/** Request Message; Changes the service programming code of the device
           after authentication. */
typedef struct {

  /* Mandatory */
  /*  Current SPC */
  char curr_spc[QMI_DMS_SPC_LEN_V01];
  /**<   SPC for authentication in ASCII format (digits 0 to 9 only).
  */

  /* Mandatory */
  /*  New SPC */
  char new_spc[QMI_DMS_SPC_LEN_V01];
  /**<   New SPC in ASCII format (digits 0 to 9 only).
  */
}dms_set_spc_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Changes the service programming code of the device
           after authentication. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}dms_set_spc_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}dms_get_current_prl_info_req_msg_v01;

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Queries the currently active PRL information of the device. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  PRL Version */
  uint8_t prl_version_valid;  /**< Must be set to true if prl_version is being passed */
  uint16_t prl_version;
  /**<   PRL version */

  /* Optional */
  /*  PRL Only Preference */
  uint8_t prl_only_valid;  /**< Must be set to true if prl_only is being passed */
  uint8_t prl_only;
  /**<   Values: \n
       - 0 -- Unset \n
       - 1 -- Set
  */
}dms_get_current_prl_info_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dms_qmi_enums
    @{
  */
typedef enum {
  DMS_BIND_SUBSCRIPTION_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  DMS_PRIMARY_SUBS_V01 = 0x0001, /**<  Primary \n  */
  DMS_SECONDARY_SUBS_V01 = 0x0002, /**<  Secondary \n  */
  DMS_TERTIARY_SUBS_V01 = 0x0003, /**<  Tertiary   */
  DMS_BIND_SUBSCRIPTION_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}dms_bind_subscription_enum_v01;
/**
    @}
  */

/** @addtogroup dms_qmi_messages
    @{
  */
/** Request Message; Associates the requesting control point with the requested subscription. */
typedef struct {

  /* Mandatory */
  /*  Bind Subscription */
  dms_bind_subscription_enum_v01 bind_subs;
  /**<   Subscription to which to bind. Values: \n
      - DMS_PRIMARY_SUBS (0x0001) --  Primary \n 
      - DMS_SECONDARY_SUBS (0x0002) --  Secondary \n 
      - DMS_TERTIARY_SUBS (0x0003) --  Tertiary  
 */
}dms_bind_subscription_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Associates the requesting control point with the requested subscription. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}dms_bind_subscription_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}dms_get_bind_subscription_req_msg_v01;

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Queries the subscription associated with the control point. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Bound Subscription */
  uint8_t bind_subscription_valid;  /**< Must be set to true if bind_subscription is being passed */
  dms_bind_subscription_enum_v01 bind_subscription;
  /**<   Values: \n
      - DMS_PRIMARY_SUBS (0x0001) --  Primary \n 
      - DMS_SECONDARY_SUBS (0x0002) --  Secondary \n 
      - DMS_TERTIARY_SUBS (0x0003) --  Tertiary  
 */
}dms_get_bind_subscription_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dms_qmi_messages
    @{
  */
/** Request Message; Sets the AP software version on the modem required for an
           Auto Register Short message. */
typedef struct {

  /* Mandatory */
  /*  AP Software Version */
  char ap_sw_version[QMI_DMS_SW_VERSION_MAX_V01 + 1];
  /**<   String representing the AP software version information. 
  */
}dms_set_ap_sw_version_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Sets the AP software version on the modem required for an
           Auto Register Short message. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}dms_set_ap_sw_version_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}dms_get_cdma_lock_mode_req_msg_v01;

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Requests the CDMA Lock mode status. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  CDMA Lock Mode */
  uint8_t cdma_lock_mode_status_valid;  /**< Must be set to true if cdma_lock_mode_status is being passed */
  dms_cdma_lock_mode_state_enum_v01 cdma_lock_mode_status;
  /**<   CDMA Lock mode status. Values: \n
      - DMS_CDMA_LOCK_MODE_OFF (0) --  Phone is not CDMA locked 
      - DMS_CDMA_LOCK_MODE_ON (1) --  Phone is CDMA locked  
 */
}dms_get_cdma_lock_mode_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dms_qmi_enums
    @{
  */
typedef enum {
  DMS_TEST_CONFIG_TDS_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  DMS_TEST_CONFIG_TDS_PRODUCTION_V01 = 0, /**<  Use the configuration applicable in production 
                                       (in the field) \n  */
  DMS_TEST_CONFIG_TDS_LAB_V01 = 1, /**<  Use the configuration applicable in the 
                                  lab \n  */
  DMS_TEST_CONFIG_TDS_USER_V01 = 2, /**<  Use the user-defined configuration   */
  DMS_TEST_CONFIG_TDS_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}dms_test_config_tds_enum_v01;
/**
    @}
  */

/** @addtogroup dms_qmi_messages
    @{
  */
/** Request Message; Sets the configuration type used while testing. */
typedef struct {

  /* Optional */
  /*  TDS CDMA Configuration */
  uint8_t tds_config_valid;  /**< Must be set to true if tds_config is being passed */
  dms_test_config_tds_enum_v01 tds_config;
  /**<   Configuration parameters to be used for TDS CDMA. Values: \n
      - DMS_TEST_CONFIG_TDS_PRODUCTION (0) --  Use the configuration applicable in production 
                                       (in the field) \n 
      - DMS_TEST_CONFIG_TDS_LAB (1) --  Use the configuration applicable in the 
                                  lab \n 
      - DMS_TEST_CONFIG_TDS_USER (2) --  Use the user-defined configuration  
 */
}dms_set_test_config_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Sets the configuration type used while testing. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}dms_set_test_config_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}dms_get_test_config_req_msg_v01;

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Gets the configuration type used for testing. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Active TDS CDMA Configuration */
  uint8_t active_tds_config_valid;  /**< Must be set to true if active_tds_config is being passed */
  dms_test_config_tds_enum_v01 active_tds_config;
  /**<   Configuration parameters currently used for TDS CDMA. Values: \n
      - DMS_TEST_CONFIG_TDS_PRODUCTION (0) --  Use the configuration applicable in production 
                                       (in the field) \n 
      - DMS_TEST_CONFIG_TDS_LAB (1) --  Use the configuration applicable in the 
                                  lab \n 
      - DMS_TEST_CONFIG_TDS_USER (2) --  Use the user-defined configuration  
 */

  /* Optional */
  /*  Desired TDS CDMA Configuration */
  uint8_t desired_tds_config_valid;  /**< Must be set to true if desired_tds_config is being passed */
  dms_test_config_tds_enum_v01 desired_tds_config;
  /**<   Configuration parameters for TDS CDMA that were set using the last 
 QMI_DMS_SET_TEST_CONFIG command. Values: \n
      - DMS_TEST_CONFIG_TDS_PRODUCTION (0) --  Use the configuration applicable in production 
                                       (in the field) \n 
      - DMS_TEST_CONFIG_TDS_LAB (1) --  Use the configuration applicable in the 
                                  lab \n 
      - DMS_TEST_CONFIG_TDS_USER (2) --  Use the user-defined configuration  
 */
}dms_get_test_config_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}dms_clear_test_config_req_msg_v01;

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Resets the modem configuration to production values. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}dms_clear_test_config_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dms_qmi_enums
    @{
  */
typedef enum {
  DMS_OEM_CHINA_OPERATOR_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  DMS_OPERATOR_CT_V01 = 0, 
  DMS_OPERATOR_CU_V01 = 1, 
  DMS_OPERATOR_CMCC_V01 = 2, 
  DMS_OEM_CHINA_OPERATOR_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}dms_oem_china_operator_enum_v01;
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}dms_oem_china_operator_req_msg_v01;

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Gets the modem China production configuration values. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Operator Information */
  uint8_t operator_info_valid;  /**< Must be set to true if operator_info is being passed */
  dms_oem_china_operator_enum_v01 operator_info;
  /**<   Mobile operator to be used. Values: \n
      - DMS_OPERATOR_CT (0) -- 
      - DMS_OPERATOR_CU (1) -- 
      - DMS_OPERATOR_CMCC (2) -- 
 */
}dms_oem_china_operator_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dms_qmi_enums
    @{
  */
typedef enum {
  DMS_DEVICE_MAC_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  DMS_DEVICE_MAC_WLAN_V01 = 0, /**<  WLAN device MAC address \n  */
  DMS_DEVICE_MAC_BT_V01 = 1, /**<  Bluetooth device MAC address \n  */
  DMS_DEVICE_MAC_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}dms_device_mac_enum_v01;
/**
    @}
  */

/** @addtogroup dms_qmi_messages
    @{
  */
/** Request Message; Retrieves the MAC address configured by default. */
typedef struct {

  /* Mandatory */
  /*  Device Information */
  dms_device_mac_enum_v01 device;
  /**<   Device for which MAC address is being retrieved. Values: \n
      - DMS_DEVICE_MAC_WLAN (0) --  WLAN device MAC address \n 
      - DMS_DEVICE_MAC_BT (1) --  Bluetooth device MAC address \n  
 */
}dms_get_mac_address_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; Retrieves the MAC address configured by default. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Device MAC Address */
  uint8_t mac_address_valid;  /**< Must be set to true if mac_address is being passed */
  uint32_t mac_address_len;  /**< Must be set to # of elements in mac_address */
  uint8_t mac_address[QMI_DMS_MAC_ADDR_MAX_V01];
}dms_get_mac_address_resp_msg_v01;  /* Message */
/**
    @}
  */
/* SWISTART */
#ifdef SIERRA
/**
    @}
  */

/*
 * dms_swi_get_cwe_pkgs_info_req_msg is empty
 * typedef struct {
 * }dms_swi_get_cwe_pkgs_info_req_msg_v01;
 */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Response Message; This message	provides firmware management info including */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */

  /* Optional */
  /*  Product Model */
  uint8_t model_id_valid;  /**< Must be set to true if model_id is being passed */
  uint32_t model_id_len;  /**< Must be set to # of elements in model_id */
  char model_id[QMI_DMS_PRODUCT_NAME_SIZE_V01];
  /**<   Device Model. 
                                                      E.g. MC7700
                                                  */

  /* Optional */
  /*  Boot Version */
  uint8_t boot_ver_valid;  /**< Must be set to true if boot_ver is being passed */
  uint32_t boot_ver_len;  /**< Must be set to # of elements in boot_ver */
  char boot_ver[QMI_DMS_BCH_VERSTR_SIZE_V01];
  /**<   Boot Version string
                                                    as specified in OSBL CWE header. 
                                                    Max 84 char 
                                                */

  /* Optional */
  /*  Application Version */
  uint8_t Appl_ver_valid;  /**< Must be set to true if Appl_ver is being passed */
  uint32_t Appl_ver_len;  /**< Must be set to # of elements in Appl_ver */
  char Appl_ver[QMI_DMS_BCH_VERSTR_SIZE_V01];
  /**<   Application version string 
                                                    as specified in AMSS CWE header. 
                                      Max 84 char 
                                  */

  /* Optional */
  /*  Product SKU */
  uint8_t sku_id_valid;  /**< Must be set to true if sku_id is being passed */
  char sku_id[QMI_DMS_PRODUCT_SKU_SIZE_V01];
  /**<   Product SKU ID  */

  /* Optional */
  /*  Package Name */
  uint8_t pkg_name_valid;  /**< Must be set to true if pkg_name is being passed */
  uint32_t pkg_name_len;  /**< Must be set to # of elements in pkg_name */
  char pkg_name[QMI_DMS_BCH_VERSTR_SIZE_V01];
  /**<   Package name as stored 
                                                    in NV SWI_PACKAGE+NAME_I .
                                                    Max 84 char
                                                */

  /* Optional */
  /*  Carrier ID */
  uint8_t carrier_id_valid;  /**< Must be set to true if carrier_id is being passed */
  char carrier_id[QMI_DMS_CARRIERID_SIZE_V01];
  /**<   Carrier ID as stored in NV NV_SWI_CARRIERID_I. 
                                                     Numeric value returned as a string. 
                                                 */

  /* Optional */
  /*  PRI revision */
  uint8_t pri_rev_valid;  /**< Must be set to true if pri_rev is being passed */
  uint32_t pri_rev_len;  /**< Must be set to # of elements in pri_rev */
  char pri_rev[QMI_DMS_PRIID_SIZE_V01];
  /**<   PRI revision in major.minor numeric format    */
}dms_swi_get_cwe_pkgs_info_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Request Message; This message enables indications for Sierra defined events. */
typedef struct {

  /* Optional */
  /*  Temperature */
  uint8_t temperature_valid;  /**< Must be set to true if temperature is being passed */
  uint8_t temperature;
  /**<   0 - do not report
                                   1 - report when state changes
                               */

  /* Optional */
  /*  Voltage */
  uint8_t voltage_valid;  /**< Must be set to true if voltage is being passed */
  uint8_t voltage;
  /**<   0 - do not report
                                   1 - report when state changes
                               */

  /* Optional */
  /*  Deferred shutdown */
  uint8_t Deferred_valid;  /**< Must be set to true if Deferred is being passed */
  uint8_t Deferred;
  /**<   0 - do not report
                                   1 - report when state changes
                               */

  /* Optional */
  /*  UimStatus */
  uint8_t uimStatus_valid;  /**< Must be set to true if uimStatus is being passed */
  uint8_t uimStatus;
  /**<   0 - do not report
                                   1 - report when state changes
                               */

  /* Optional */
  /*  PaTemp_Report */
  uint8_t patemp_report_valid;  /**< Must be set to true if patemp_report is being passed */
  uint8_t patemp_report;
  /**<   0 - do not report
                                   1 - report when state changes
                               */

  /* Optional */
  /*  Callst_Report */
  uint8_t callst_report_valid;  /**< Must be set to true if callst_report is being passed */
  uint8_t callst_report;
  /**<   0 - do not report
                                   1 - report when state changes
                                                                               */
}dms_swi_set_event_report_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Response Message; This message enables indications for Sierra defined events. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */
}dms_swi_set_event_report_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t state;
  /**<   0 - unknown
                    1 - normal
                    2 - high (warning)
                    3 - high (critical)
                    4 - low (warning)
                    5 - low (critical)
                */

  int16_t temperature;
  /**<   Temperature in degrees Celsius. */
}temperature_status_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t state;
  /**<   temprature state */

  int16_t temperature;
  /**<   Temperature in degrees Celsius. */
}patemperature_status_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t state;
  /**<   0 - unknown
                    1 - normal
                    2 - high (warning)
                    3 - high (critical)
                    4 - low (critical)
                */

  uint16_t voltage;
  /**<   Voltage in mV. */
}voltage_status_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t interface;
  /**<   0 - External UIM
                        1 - Embedded UIM
                        2 - Remote UIM
                    */

  uint8_t event;
  /**<   0 - UIM card removed
                     1 - UIM card inserted
                 */
}uim_status_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Indication Message; This message enables indications for Sierra defined events. */
typedef struct {

  /* Optional */
  /*  temperature status */
  uint8_t temperature_status_valid;  /**< Must be set to true if temperature_status is being passed */
  temperature_status_type_v01 temperature_status;
  /**<   temperature status  */

  /* Optional */
  /*  voltage status */
  uint8_t voltage_status_valid;  /**< Must be set to true if voltage_status is being passed */
  voltage_status_type_v01 voltage_status;
  /**<   voltage status  */

  /* Optional */
  /*  deferred_shutdown status */
  uint8_t deferred_shutdown_state_valid;  /**< Must be set to true if deferred_shutdown_state is being passed */
  uint8_t deferred_shutdown_state;
  /**<   0 -  exit
                                               1 -  enter
                                           */

  /* Optional */
  /*  uim status */
  uint8_t uim_status_valid;  /**< Must be set to true if uim_status is being passed */
  uim_status_type_v01 uim_status;
  /**<   uim status    */

  /* Optional */
  /*  uim status */
  uint8_t patemperature_status_valid;  /**< Must be set to true if patemperature_status is being passed */
  patemperature_status_type_v01 patemperature_status;
  /**<   patemperature status  */

  /* Optional */
  uint8_t call_state_valid;  /**< Must be set to true if call_state is being passed */
  int8_t call_state;
  /**<   Call state  */
}dms_swi_event_report_ind_msg_v01;  /* Message */
/**
    @}
  */

/*
 * dms_swi_get_environment_req_msg is empty
 * typedef struct {
 * }dms_swi_get_environment_req_msg_v01;
 */

/** @addtogroup sierra_swi_dms_api_qmi_aggregates
    @{
  */
typedef struct {

  int16_t temperature;
  /**<   Temperature in degrees Celsius. */

  int16_t hi_critical;
  /**<   High Critical threshold. */

  int16_t hi_warning;
  /**<   High Warning threshold. */

  int16_t hi_normal;
  /**<   High Normal threshold. */

  int16_t lo_normal;
  /**<   Low Normal threshold.   */

  int16_t lo_critical;
  /**<   Low Critical threshold.   */
}get_temperature_environment_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t voltage;
  /**<   Voltage (mV). */

  uint16_t hi_critical;
  /**<   High Critical threshold. */

  uint16_t hi_normal;
  /**<   High Normal threshold. */

  uint16_t lo_normal;
  /**<   Low Normal threshold.   */

  uint16_t lo_warning;
  /**<  Low Warning threshold   */
  uint16_t lo_critical;
  /**<   Low Critical threshold.     */
}get_voltage_environment_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_aggregates
    @{
  */
typedef struct {

  int16_t temperature;
  /**<   Temperature in degrees Celsius. */

  int16_t hi_critical;
  /**<   High Critical threshold. */

  int16_t hi_warning;
  /**<   High Warning threshold. */

  int16_t hi_normal;
  /**<   High Normal threshold. */
}get_patemperature_environment_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Response Message; This message requests device to return environmental data. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */

  /* Optional */
  /*  get_temperature_environment */
  uint8_t get_temperature_environment_valid;  /**< Must be set to true if get_temperature_environment is being passed */
  get_temperature_environment_type_v01 get_temperature_environment;

  /* Optional */
  /*  get_voltage_environment */
  uint8_t get_voltage_environment_valid;  /**< Must be set to true if get_voltage_environment is being passed */
  get_voltage_environment_type_v01 get_voltage_environment;

  /* Optional */
  /*  get_patemperature_environment */
  uint8_t get_patemperature_environment_valid;  /**< Must be set to true if get_patemperature_environment is being passed */
  get_patemperature_environment_type_v01 get_patemperature_environment;
}dms_swi_get_environment_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_aggregates
    @{
  */
typedef struct {

  int16_t hi_critical;
  /**<   High Critical threshold. */

  int16_t hi_warning;
  /**<   High Warning threshold. */

  int16_t hi_normal;
  /**<   High Normal threshold. */

  int16_t lo_normal;
  /**<   Low Normal threshold.   */

  int16_t lo_critical;
  /**<   Low Critical threshold.     */
}set_temperature_environment_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t hi_critical;
  /**<   High Critical threshold. */


  uint16_t hi_normal;
  /**<   High Normal threshold. */

  uint16_t lo_normal;
  /**<   Low Normal threshold.   */

  uint16_t lo_warning;
  /**<  Low Warning threshold   */

  uint16_t lo_critical;
  /**<   Low Critical threshold.     */
}set_voltage_environment_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_aggregates
    @{
  */
typedef struct {

  int16_t hi_critical;
  /**<   High Critical threshold. */

  int16_t hi_warning;
  /**<   High Warning threshold. */

  int16_t hi_normal;
  /**<   High Normal threshold. */
}set_patemperature_environment_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Request Message; This message requests device to update environmental limits */
typedef struct {

  /* Optional */
  /*  set_temperature_environment */
  uint8_t set_temperature_environment_valid;  /**< Must be set to true if set_temperature_environment is being passed */
  set_temperature_environment_type_v01 set_temperature_environment;

  /* Optional */
  /*  set_voltage_environment */
  uint8_t set_voltage_environment_valid;  /**< Must be set to true if set_voltage_environment is being passed */
  set_voltage_environment_type_v01 set_voltage_environment;

  /* Optional */
  /*  set_patemperature_environment */
  uint8_t set_patemperature_environment_valid;  /**< Must be set to true if set_patemperature_environment is being passed */
  set_patemperature_environment_type_v01 set_patemperature_environment;
}dms_swi_set_environment_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Response Message; This message requests device to update environmental limits */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */
}dms_swi_set_environment_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * dms_swi_reset_to_download_mode_req_msg is empty
 * typedef struct {
 * }dms_swi_reset_to_download_mode_req_msg_v01;
 */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Response Message; This message requests the device to reset into firmware download mode. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */
}dms_swi_reset_to_download_mode_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * dms_swi_get_usb_comp_req_msg is empty
 * typedef struct {
 * }dms_swi_get_usb_comp_req_msg_v01;
 */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Response Message; This message query the modem’s USB interface configuration, 
	and supported configuration parameters. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */

  /* Optional */
  /*  USB Composition */
  uint8_t usb_comp_valid;  /**< Must be set to true if usb_comp is being passed */
  uint8_t usb_comp;
  /**<   t0..5 - Reserved (non-QMI)
                                 6  - DM NMEA AT QMI
                                 7  - DM NMEA AT QMI1 QMI2 QMI3
                                 8  - DM NMEA AT MBIM
                                 9  - MBIM
                                 10 - NMEA MBIM
                                 11 - DM MBIM
                                 12 - DM NMEA MBIM
                                 
                                 13~22 are combined compositions. One is for Win8 MBIM interfaces, another is for legacy QMI interfaces
                                 
                                 13 - 6 for QMI, 8 for MBIM
                                 14 - 6 for QMI, 9 for MBIM
                                 15 - 6 for QMI, 10 for MBIM
                                 16 - 6 for QMI, 11 for MBIM
                                 17 - 6 for QMI, 12 for MBIM
                                 18 - 7 for QMI, 8 for MBIM
                                 19 - 7 for QMI, 9 for MBIM
                                 20 - 7 for QMI, 10 for MBIM
                                 21 - 7 for QMI, 11 for MBIM
                                 22 - 7 for QMI, 12 for MBIM
                           */

  /* Optional */
  /*  Supported USB Composition */
  uint8_t usb_comp_list_valid;  /**< Must be set to true if usb_comp_list is being passed */
  uint32_t usb_comp_list_len;  /**< Must be set to # of elements in usb_comp_list */
  char usb_comp_list[QMI_DMS_USB_TABLE_MAX_V01];
  /**<   0..5 ?Reserved (non-QMI)
                                                        6  - DM NMEA AT QMI
                                                        7  - DM NMEA AT QMI1 QMI2 QMI3
                                                        8  - DM NMEA AT MBIM
                                                        9  - MBIM
                                                        10 - NMEA MBIM
                                                        11 - DM MBIM
                                                        12 - DM NMEA MBIM
                                                        
                                                        13~22 are combined compositions. One is for Win8 MBIM interfaces, another is for legacy QMI interfaces
                                                        
                                                        13 - 6 for QMI, 8 for MBIM
                                                        14 - 6 for QMI, 9 for MBIM
                                                        15 - 6 for QMI, 10 for MBIM
                                                        16 - 6 for QMI, 11 for MBIM
                                                        17 - 6 for QMI, 12 for MBIM
                                                        18 - 7 for QMI, 8 for MBIM
                                                        19 - 7 for QMI, 9 for MBIM
                                                        20 - 7 for QMI, 10 for MBIM
                                                        21 - 7 for QMI, 11 for MBIM
                                                        22 - 7 for QMI, 12 for MBIM
                                                    */
}dms_swi_get_usb_comp_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Request Message; This message tests basic message passing between 
  client and service */
typedef struct {

  /* Mandatory */
  /*  USB Composition */
  uint8_t usb_comp;
  /**<   t0..5 - Reserved (non-QMI)
                                 6  - DM NMEA AT QMI
                                 7  - DM NMEA AT QMI1 QMI2 QMI3
                                 8  - DM NMEA AT MBIM
                                 9  - MBIM
                                 10 - NMEA MBIM
                                 11 - DM MBIM
                                 12 - DM NMEA MBIM
                                 
                                 13~22 are combined compositions. One is for Win8 MBIM interfaces, another is for legacy QMI interfaces
                                 
                                 13 - 6 for QMI, 8 for MBIM
                                 14 - 6 for QMI, 9 for MBIM
                                 15 - 6 for QMI, 10 for MBIM
                                 16 - 6 for QMI, 11 for MBIM
                                 17 - 6 for QMI, 12 for MBIM
                                 18 - 7 for QMI, 8 for MBIM
                                 19 - 7 for QMI, 9 for MBIM
                                 20 - 7 for QMI, 10 for MBIM
                                 21 - 7 for QMI, 11 for MBIM
                                 22 - 7 for QMI, 12 for MBIM
                           */
}dms_swi_set_usb_comp_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Response Message; This message tests basic message passing between 
  client and service */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */
}dms_swi_set_usb_comp_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * dms_swi_get_wu_fwid_req_msg is empty
 * typedef struct {
 * }dms_swi_get_wu_fwid_req_msg_v01;
 */

/** @addtogroup sierra_swi_dms_api_qmi_aggregates
    @{
  */
typedef struct {

  char wu_id[QMI_DMS_USB_GUID_MAX_V01];
  /**<  
                                             ID in the form of a GUID
                                            (32 hexadecimal characters)
                                         */

  uint8_t wu_ver_len;
  /**<  
                                             Length of WU version field
                                         */

  uint8_t wu_ver[QMI_DMS_USB_PRIID_MAX_V01];
  /**<  
                                             ASCII string representing the version number
                                                */
}windows_update_package_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Response Message; This message tests basic message passing between 
  client and service */
typedef struct {

  /* Mandatory */
  /*  Windows Update package */
  windows_update_package_type_v01 windows_update_package;
  /**<   Windows Update package  */

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.    */
}dms_swi_get_wu_fwid_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * dms_swi_get_fcc_auth_req_msg is empty
 * typedef struct {
 * }dms_swi_get_fcc_auth_req_msg_v01;
 */

/** @addtogroup sierra_swi_dms_api_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t auth_enabled;
  /**<  
                                0 - Disabled
                                1 - Enabled
                             */

  uint8_t lock_state;
  /**<  
                                0  - modem in an unlocked state
                                1  ?modem in a locked state
                              */
}authentication_setting_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Response Message; This message query the modem’s FCC authentication setting */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */

  /* Optional */
  /*  Pong */
  uint8_t authentication_setting_valid;  /**< Must be set to true if authentication_setting is being passed */
  authentication_setting_type_v01 authentication_setting;
  /**<   Authentication setting  */
}dms_swi_get_fcc_auth_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * dms_swi_set_fcc_auth_req_msg is empty
 * typedef struct {
 * }dms_swi_set_fcc_auth_req_msg_v01;
 */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Response Message; This message perform FCC authentication */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */
}dms_swi_set_fcc_auth_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_aggregates
    @{
  */
typedef struct {

  char provider_id[QMI_DMS_PROVIDER_ID_MAX_V01];

  uint32_t provider_state;

  char provider_name[QMI_DMS_PROVIDER_NAME_MAX_V01];

  uint32_t cellular_class;

  uint32_t rssi;

  uint32_t error_rate;
}home_providers_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Request Message; This message tests basic message passing between 
  client and service */
typedef struct {

  /* Optional */
  /*  Ping */
  uint8_t home_providers_valid;  /**< Must be set to true if home_providers is being passed */
  uint32_t home_providers_len;  /**< Must be set to # of elements in home_providers */
  home_providers_type_v01 home_providers[QMI_DMS_NUM_CARRIER_IMAGES_MAX_V01];
  /**<    request  */
}dms_swi_set_multi_carrier_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Response Message; This message tests basic message passing between 
  client and service */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */
}dms_swi_set_multi_carrier_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_aggregates
    @{
  */
typedef struct {

  char provider_id[QMI_DMS_PROVIDER_ID_V01];

  uint32_t provider_state;

  char provider_name[QMI_DMS_PROVIDER_NAME_V01];

  uint32_t cellular_class;
}homeProviderSelection_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Indication Message; This message tests basic message passing between 
  client and service */
typedef struct {

  /* Optional */
  /*  LocationSelection */
  uint8_t geoLocation_valid;  /**< Must be set to true if geoLocation is being passed */
  uint32_t geoLocation_len;  /**< Must be set to # of elements in geoLocation */
  uint8_t geoLocation[QMI_DMS_GEO_LOC_MAX_V01];

  /* Optional */
  /*  HomeProviderSelection */
  uint8_t homeProviderSelection_valid;  /**< Must be set to true if homeProviderSelection is being passed */
  homeProviderSelection_type_v01 homeProviderSelection;
}dms_swi_set_multi_carrier_ind_msg_v01;  /* Message */
/**
    @}
  */

/*
 * dms_swi_get_multi_carrier_req_msg is empty
 * typedef struct {
 * }dms_swi_get_multi_carrier_req_msg_v01;
 */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Response Message; This message tests basic message passing between 
  client and service */
typedef struct {

  /* Mandatory */
  /*  HomeProviderList */
  uint32_t home_providers_len;  /**< Must be set to # of elements in home_providers */
  home_providers_type_v01 home_providers[QMI_DMS_NUM_CARRIER_IMAGES_MAX_V01];
  /**<    HomeProviderList  */

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */
}dms_swi_get_multi_carrier_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Request Message; This message created for internal FW use between the QBI 
  and QMI packages for Host Based Image switching. */
typedef struct {

  /* Optional */
  /*  LocationSelection */
  uint8_t geoLocation_valid;  /**< Must be set to true if geoLocation is being passed */
  uint32_t geoLocation_len;  /**< Must be set to # of elements in geoLocation */
  uint8_t geoLocation[QMI_DMS_GEO_LOC_MAX_V01];

  /* Optional */
  /*  HomeProviderSelection */
  uint8_t homeProviderSelection_valid;  /**< Must be set to true if homeProviderSelection is being passed */
  homeProviderSelection_type_v01 homeProviderSelection;
}dms_swi_set_loc_home_prov_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Response Message; This message created for internal FW use between the QBI 
  and QMI packages for Host Based Image switching. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */
}dms_swi_set_loc_home_prov_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t image_type;

  uint8_t unique_id[QMI_DMS_GOBI_IM_UNIQUE_ID_MAX_V01];

  uint8_t build_id_len;

  char build_id[QMI_DMS_GOBI_IM_BUILD_ID_MAX_V01];
}current_image_list_type_v01;  /* Type */
/**
    @}
  */

/*
 * dms_swi_get_firmware_curr_req_msg is empty
 * typedef struct {
 * }dms_swi_get_firmware_curr_req_msg_v01;
 */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Response Message; This message created for internal FW use between the QBI and 
  QMI packages for Host Based Image switching. */
typedef struct {

  /* Mandatory */
  /*  Ping */
  uint32_t current_image_list_len;  /**< Must be set to # of elements in current_image_list */
  current_image_list_type_v01 current_image_list[QMI_DMS_NUM_SUPPORTED_IMAGE_TYPES_V01];

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */
}dms_swi_get_firmware_curr_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * dms_swi_get_firmware_status_req_msg is empty
 * typedef struct {
 * }dms_swi_get_firmware_status_req_msg_v01;
 */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Response Message; This message query last firmware update status */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */

  /* Optional */
  /*  FW Update Result Code */
  uint8_t FW_update_result_valid;  /**< Must be set to true if FW_update_result is being passed */
  uint32_t FW_update_result;

  /* Optional */
  /*  ImageType */
  uint8_t image_type_valid;  /**< Must be set to true if image_type is being passed */
  uint8_t image_type;

  /* Optional */
  /*  RefData */
  uint8_t ref_data_valid;  /**< Must be set to true if ref_data is being passed */
  uint32_t ref_data;

  /* Optional */
  /*  RefString */
  uint8_t refstring_valid;  /**< Must be set to true if refstring is being passed */
  uint32_t refstring_len;  /**< Must be set to # of elements in refstring */
  char refstring[QMI_DMS_FWUPDATERESTRLEN_V01];
}dms_swi_get_firmware_status_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * dms_swi_get_firmware_features_req_msg is empty
 * typedef struct {
 * }dms_swi_get_firmware_features_req_msg_v01;
 */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Response Message; This message queries the settings for SWI CUSTOM commands. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */

  /* Optional */
  /*  GPSENABLE */
  uint8_t gpsenable_valid;  /**< Must be set to true if gpsenable is being passed */
  uint32_t gpsenable;
  /**<   GPSENABLE flag  */

  /* Optional */
  /*  Disable Display of IMSI */
  uint8_t disableIMSI_valid;  /**< Must be set to true if disableIMSI is being passed */
  uint8_t disableIMSI;
  /**<   Disable Display of IMSI  */

  /* Optional */
  /*  IP Family Support */
  uint8_t IPFamSupport_valid;  /**< Must be set to true if IPFamSupport is being passed */
  uint8_t IPFamSupport;
  /**<   IP Family Support  */

  /* Optional */
  /*  QMI Mode RMNET Autoconnect Support */
  uint8_t RMAutoconnect_valid;  /**< Must be set to true if RMAutoconnect is being passed */
  uint8_t RMAutoconnect;
  /**<   QMI Mode RMNET Autoconnect Support  */

  /* Optional */
  /*  GPSSEL */
  uint8_t GPSSEL_valid;  /**< Must be set to true if GPSSEL is being passed */
  uint8_t GPSSEL;
  /**<   GPSSEL  */

  /* Optional */
  /*  SMS Support */
  uint8_t SMS_valid;  /**< Must be set to true if SMS is being passed */
  uint8_t SMS;
  /**<   SMS Support  */

  /* Optional */
  /*  ISVOICEN Support */
  uint8_t ISVOICEN_valid;  /**< Must be set to true if ISVOICEN is being passed */
  uint8_t ISVOICEN;
  /**<   ISVOICEN Support  */

  /* Optional */
  /*  DHCPRELAYENABLE  Support */
  uint8_t DHCPRELAYENABLE_valid;  /**< Must be set to true if DHCPRELAYENABLE is being passed */
  uint8_t DHCPRELAYENABLE;
  /**<   DHCPRELAYENABLE  Support  */

  /* Optional */
  /*  GPSLPM Support */
  uint8_t GPSLPM_valid;  /**< Must be set to true if GPSLPM is being passed */
  uint8_t GPSLPM;
  /**<   GPSLPM Support    */
}dms_swi_get_firmware_features_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Request Message; This message allows for the setting of SWI CUSTOM commands */
typedef struct {

  /* Optional */
  /*  GPSENABLE */
  uint8_t gpsenable_valid;  /**< Must be set to true if gpsenable is being passed */
  uint32_t gpsenable;
  /**<   GPSENABLE flag  */

  /* Optional */
  /*  Disable Display of IMSI */
  uint8_t disableIMSI_valid;  /**< Must be set to true if disableIMSI is being passed */
  uint8_t disableIMSI;
  /**<   Disable Display of IMSI  */

  /* Optional */
  /*  IP Family Support */
  uint8_t IPFamSupport_valid;  /**< Must be set to true if IPFamSupport is being passed */
  uint8_t IPFamSupport;
  /**<   IP Family Support  */

  /* Optional */
  /*  QMI Mode RMNET Autoconnect Support */
  uint8_t RMAutoconnect_valid;  /**< Must be set to true if RMAutoconnect is being passed */
  uint8_t RMAutoconnect;
  /**<   QMI Mode RMNET Autoconnect Support  */

  /* Optional */
  /*  GPSSEL */
  uint8_t GPSSEL_valid;  /**< Must be set to true if GPSSEL is being passed */
  uint8_t GPSSEL;
  /**<   GPSSEL  */

  /* Optional */
  /*  SMS Support */
  uint8_t SMS_valid;  /**< Must be set to true if SMS is being passed */
  uint8_t SMS;
  /**<   SMS Support  */

  /* Optional */
  /*  ISVOICEN Support */
  uint8_t ISVOICEN_valid;  /**< Must be set to true if ISVOICEN is being passed */
  uint8_t ISVOICEN;
  /**<   ISVOICEN Support  */

  /* Optional */
  /*  DHCPRELAYENABLE  Support */
  uint8_t DHCPRELAYENABLE_valid;  /**< Must be set to true if DHCPRELAYENABLE is being passed */
  uint8_t DHCPRELAYENABLE;
  /**<   DHCPRELAYENABLE  Support  */

  /* Optional */
  /*  GPSLPM Support */
  uint8_t GPSLPM_valid;  /**< Must be set to true if GPSLPM is being passed */
  uint8_t GPSLPM;
  /**<   GPSLPM Support    */
}dms_swi_set_firmware_features_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Response Message; This message allows for the setting of SWI CUSTOM commands */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */
}dms_swi_set_firmware_features_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Request Message; This message allows for the setting of EMERGENCY NUMBERS items */
typedef struct {

  /* Optional */
  /*  Emergency number type */
  uint8_t ping_valid;  /**< Must be set to true if ping is being passed */
  uint32_t ping_len;  /**< Must be set to # of elements in ping */
  char ping[QMI_DMS_CUSTECCNUM_MAX_V01];
  /**<   Emergency number type  */
}dms_swi_set_emergency_number_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Response Message; This message allows for the setting of EMERGENCY NUMBERS items */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */
}dms_swi_set_emergency_number_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Request Message; This message clear part/all emergency numbers items from existing list */
typedef struct {

  /* Optional */
  /*  Emergency number type */
  uint8_t ping_valid;  /**< Must be set to true if ping is being passed */
  uint32_t ping_len;  /**< Must be set to # of elements in ping */
  char ping[QMI_DMS_CUSTECCNUM_MAX_V01];
  /**<   Emergency number type  */
}dms_swi_clr_emergency_number_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Response Message; This message clear part/all emergency numbers items from existing list */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */
}dms_swi_clr_emergency_number_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * dms_swi_get_emergency_number_req_msg is empty
 * typedef struct {
 * }dms_swi_get_emergency_number_req_msg_v01;
 */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Response Message; This message gets emergency number from existing list. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */

  /* Optional */
  /*  Emergency number type */
  uint8_t ping_valid;  /**< Must be set to true if ping is being passed */
  uint32_t ping_len;  /**< Must be set to # of elements in ping */
  char ping[QMI_DMS_CUSTECCNUM_MAX_V01];
  /**<   Emergency number type  */
}dms_swi_get_emergency_number_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * dms_swi_get_rtn_data_req_msg is empty
 * typedef struct {
 * }dms_swi_get_rtn_data_req_msg_v01;
 */

/** @addtogroup sierra_swi_dms_api_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t rst_flag;

  char rst_date[QMI_DMS_RST_DATE_MAX_V01];

  uint32_t activation_date;
}rtn_data_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Response Message; This message retrieves device’s recondition flag,
  recondition date and activation date */
typedef struct {

  /* Mandatory */
  /*  rtn_data */
  rtn_data_type_v01 rtn_data;
  /**<   Simple rtn_data  */

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */
}dms_swi_get_rtn_data_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Request Message; This message allows the selection of 
  which UIM to be used by the module */
typedef struct {

  /* Mandatory */
  /*  Selection of UIM Card */
  uint8_t uim_select;
  /**<   Selection of UIM Card  */
}dms_swi_uim_select_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Response Message; This message allows the selection of 
  which UIM to be used by the module */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */
}dms_swi_uim_select_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Request Message; This message sets SWI specific setting for whether GPS 
  end user license agreement (EULA) has been accepted by the user.  */
typedef struct {

  /* Mandatory */
  /*  GPS EULA accepted */
  uint8_t GPS_EULA_ACCEPTED;
  /**<   0 - declined
                                          1 - accepted
                                      */
}dms_swi_set_eula_accepted_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Response Message; This message sets SWI specific setting for whether GPS 
  end user license agreement (EULA) has been accepted by the user.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}dms_swi_set_eula_accepted_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * dms_swi_get_eula_accepted_req_msg is empty
 * typedef struct {
 * }dms_swi_get_eula_accepted_req_msg_v01;
 */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Response Message; This messagespecific setting for whether GPS end user license 
  agreement (EULA) has been accepted by the user.   */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */

  /* Optional */
  /*  GPS EULA accepted */
  uint8_t GPS_EULA_ACCEPTED_valid;  /**< Must be set to true if GPS_EULA_ACCEPTED is being passed */
  uint8_t GPS_EULA_ACCEPTED;
  /**<   0 - declined
                                          1 - accepted
                                      */
}dms_swi_get_eula_accepted_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * dms_swi_get_fsn_req_msg is empty
 * typedef struct {
 * }dms_swi_get_fsn_req_msg_v01;
 */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Response Message; This message gets the module FSN string. */
typedef struct {

  /* Mandatory */
  /*  fsn */
  char fsn[QMI_DMS_SWI_FSN_SIZE_V01 + 1];
  /**<  
    FSN string.
   */

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}dms_swi_get_fsn_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Request Message; This message queries the crash info. */
typedef struct {

  /* Optional */
  /*  Clear */
  uint8_t clear_valid;  /**< Must be set to true if clear is being passed */
  char clear;
  /**<   0 - Do not clear crash data after response
                            1 - Clear crash data after response
                        */
}dms_swi_get_crash_info_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t error_string_len;

  uint32_t error_data;

  uint32_t error_id;

  uint16_t error_str_len;

  char error_string[QMI_DMS_ER_ERROR_STRING_MAX_V01];

  uint16_t gcdump_str_len;

  char gcdump_string[QMI_DMS_ER_DUMP_STRING_MAX_V01];
}er_user_data_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Response Message; This message queries the crash info. */
typedef struct {

  /* Mandatory */
  /*  Device Crash Status */
  char crash_status;
  /**<   0 - no crash
                                    1 - crash has occurred
                                */

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */

  /* Optional */
  /*  Crash info  */
  uint8_t er_user_data_valid;  /**< Must be set to true if er_user_data is being passed */
  er_user_data_type_v01 er_user_data;
}dms_swi_get_crash_info_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * dms_swi_get_map_uart_req_msg is empty
 * typedef struct {
 * }dms_swi_get_map_uart_req_msg_v01;
 */

/** @addtogroup sierra_swi_dms_api_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t UART1_service;

  uint8_t UART2_service;
}transfer_byte_mode_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Response Message; This message get UART1/UART2 service. */
typedef struct {

  /* Mandatory */
  /*  Transfer byte mode */
  transfer_byte_mode_type_v01 transfer_byte_mode;
  /**<   Transfer byte mode  */

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */
}dms_swi_get_map_uart_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Request Message; This message set UART1/UART2 service. */
typedef struct {

  /* Mandatory */
  /*  Transfer byte mode */
  transfer_byte_mode_type_v01 transfer_byte_mode;
  /**<   Transfer byte mode  */
}dms_swi_set_map_uart_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Response Message; This message set UART1/UART2 service. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */
}dms_swi_set_map_uart_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Request Message; This message tests basic message passing between 
  client and service */
/*
 * dms_swi_get_usb_id_req_msg is empty
 * typedef struct {
 * }dms_swi_get_usb_id_req_msg_v01;
 */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Response Message; This object is used to get PID/VID/SERIAL service. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  vendor_id */
  uint8_t vendor_id_valid;  /**< Must be set to true if vendor_id is being passed */
  uint16_t vendor_id;
  /**<  
    Maximum is 0xffff, it must be greater than the minimum(0x0)
   */

  /* Optional */
  /*  product_id */
  uint8_t product_id_valid;  /**< Must be set to true if product_id is being passed */
  uint16_t product_id;
  /**<  
    Maximum is 0xffff, it must be greater than the minimum(0x0)
   */

  /* Optional */
  /*  serial_number */
  uint8_t serial_number_valid;  /**< Must be set to true if serial_number is being passed */
  char serial_number[QMI_DMS_SERIAL_NUMBER_SIZE_V01 + 1];
  /**<  
    Default is 123456789ABCDEF Maximum is 127 bytes, minimum is 12 bytes
   */
}dms_swi_get_usb_id_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Request Message; This message set PID/VID/SERIAL service */
typedef struct {

  /* Optional */
  /*  Vendor ID in USB descriptor */
  uint8_t vendor_ID_valid;  /**< Must be set to true if vendor_ID is being passed */
  uint16_t vendor_ID;
  /**<   Vendor ID in USB descriptor  */

  /* Optional */
  /*  The PID in the USB descriptor */
  uint8_t product_ID_valid;  /**< Must be set to true if product_ID is being passed */
  uint16_t product_ID;
  /**<   The PID in the USB descriptor  */

  /* Optional */
  /*  Serial number in the USB descriptor */
  uint8_t serial_number_valid;  /**< Must be set to true if serial_number is being passed */
  uint32_t serial_number_len;  /**< Must be set to # of elements in serial_number */
  char serial_number[QMI_DMS_SERIAL_STRING_MAX_V01];
  /**<   Serial number in the USB descriptor  */
}dms_swi_set_usb_id_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Response Message; This message set PID/VID/SERIAL service */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */
}dms_swi_set_usb_id_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * dms_swi_get_crash_action_req_msg is empty
 * typedef struct {
 * }dms_swi_get_crash_action_req_msg_v01;
 */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Response Message; This message queries the action performed in the event of a crash */
typedef struct {

  /* Mandatory */
  /*  Device Crash State */
  uint8_t crash_action;
  /**<   0 - USB Memory Download
                                     1 - Reset
                                     2 - No action
                                 */

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */
}dms_swi_get_crash_action_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Request Message; This message configures the action performed in the event of a crash */
typedef struct {

  /* Mandatory */
  /*  Device Crash State */
  uint8_t crash_action;
  /**<   0 - USB Memory Download
                                     1 - Reset
                                     2 - No action
                                 */
}dms_swi_set_crash_action_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Response Message; This message configures the action performed in the event of a crash */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */
}dms_swi_set_crash_action_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * dms_swi_get_host_dev_info_req_msg is empty
 * typedef struct {
 * }dms_swi_get_host_dev_info_req_msg_v01;
 */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Response Message; This message queries the host device info configured
  on the modem for OMA-DM reporting */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */

  /* Optional */
  /*  Host Device Manufacturer */
  uint8_t hostman_valid;  /**< Must be set to true if hostman is being passed */
  char hostman[QMI_DMS_HOST_DEV_INFO_MAX_V01 + 1];
  /**<   Host Device Manufacturer response  */

  /* Optional */
  /*  Host Device Model */
  uint8_t hostmod_valid;  /**< Must be set to true if hostmod is being passed */
  char hostmod[QMI_DMS_HOST_DEV_INFO_MAX_V01 + 1];
  /**<   Host Device Model response  */

  /* Optional */
  /*  Host Device Software Version */
  uint8_t hostswv_valid;  /**< Must be set to true if hostswv is being passed */
  char hostswv[QMI_DMS_HOST_DEV_INFO_MAX_V01 + 1];
  /**<   Host Device Software Version response  */

  /* Optional */
  /*  Host Device Plasma ID */
  uint8_t hostplasmaid_valid;  /**< Must be set to true if hostplasmaid is being passed */
  char hostplasmaid[QMI_DMS_HOST_DEV_INFO_MAX_V01 + 1];
  /**<   Host Device Plasma ID response    */
}dms_swi_get_host_dev_info_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Request Message; This message sets the host device info configured 
    on the modem for OMA-DM reporting */
typedef struct {

  /* Optional */
  /*  Host Device Manufacturer */
  uint8_t hostman_valid;  /**< Must be set to true if hostman is being passed */
  char hostman[QMI_DMS_HOST_DEV_INFO_MAX_V01 + 1];
  /**<   Host Device Manufacturer response  */

  /* Optional */
  /*  Host Device Model */
  uint8_t hostmod_valid;  /**< Must be set to true if hostmod is being passed */
  char hostmod[QMI_DMS_HOST_DEV_INFO_MAX_V01 + 1];
  /**<   Host Device Model response  */

  /* Optional */
  /*  Host Device Software Version */
  uint8_t hostswv_valid;  /**< Must be set to true if hostswv is being passed */
  char hostswv[QMI_DMS_HOST_DEV_INFO_MAX_V01 + 1];
  /**<   Host Device Software Version response  */

  /* Optional */
  /*  Host Device Plasma ID */
  uint8_t hostplasmaid_valid;  /**< Must be set to true if hostplasmaid is being passed */
  char hostplasmaid[QMI_DMS_HOST_DEV_INFO_MAX_V01 + 1];
  /**<   Host Device Plasma ID response   */
}dms_swi_set_host_dev_info_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Response Message; This message sets the host device info configured 
    on the modem for OMA-DM reporting */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */
}dms_swi_set_host_dev_info_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * dms_swi_get_os_info_req_msg is empty
 * typedef struct {
 * }dms_swi_get_os_info_req_msg_v01;
 */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Response Message; This message queries the device operating system info configured 
    on the modem for OMA-DM reporting */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */

  /* Optional */
  /*  Operating System Name */
  uint8_t osname_valid;  /**< Must be set to true if osname is being passed */
  uint32_t osname_len;  /**< Must be set to # of elements in osname */
  char osname[QMI_DMS_OS_INFO_MAX_V01];
  /**<   Null terminated ASCII string.  
                                                     Max length of 256 bytes. 
                        */

  /* Optional */
  /*  Operating System version */
  uint8_t osversion_valid;  /**< Must be set to true if osversion is being passed */
  uint32_t osversion_len;  /**< Must be set to # of elements in osversion */
  char osversion[QMI_DMS_OS_INFO_MAX_V01];
  /**<   Null terminated ASCII string.  
                                                     Max length of 256 bytes. 
                           */
}dms_swi_get_os_info_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Request Message; This message sets the device operating system info configured
    on the modem for OMA-DM reporting. */
typedef struct {

  /* Optional */
  /*  Operating System Name */
  uint8_t osname_valid;  /**< Must be set to true if osname is being passed */
  uint32_t osname_len;  /**< Must be set to # of elements in osname */
  char osname[QMI_DMS_OS_INFO_MAX_V01];
  /**<   Null terminated ASCII string.  
                                                     Max length of 256 bytes. 
                        */

  /* Optional */
  /*  Operating System version */
  uint8_t osversion_valid;  /**< Must be set to true if osversion is being passed */
  uint32_t osversion_len;  /**< Must be set to # of elements in osversion */
  char osversion[QMI_DMS_OS_INFO_MAX_V01];
  /**<   Null terminated ASCII string.  
                                                     Max length of 256 bytes. 
                           */
}dms_swi_set_os_info_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Response Message; This message sets the device operating system info configured
    on the modem for OMA-DM reporting. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */
}dms_swi_set_os_info_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Request Message; This message queries the device power control status information. */
typedef struct {

  /* Mandatory */
  /*  Operating Mode */
  uint8_t mode;
  /**<   Selected operating mode. Values:
                              0 - Online
                              1 - Low power
                              2 - Factory Test mode
                              3 - Offline
                              4 - Resetting
                              5 - Shutting down
                              6 - Persistent low power
                              8 - Conducting network test for GSM/WCDMA
                         */

  /* Optional */
  /*  LPM Force Flags */
  uint8_t lpmflags_valid;  /**< Must be set to true if lpmflags is being passed */
  uint32_t lpmflags;
  /**<   Bitmask
                                 bit0: W_DISABLE
                                 bit1: User
                                 bit2: Temperature
                                 bit3: Voltage
                                 bit4: BIOS Lock
                                 bit5: GOBIIM
                                 bit6-31: reserved for future use
                             */

  /* Optional */
  /*  W_DISABLE */
  uint8_t wdisable_valid;  /**< Must be set to true if wdisable is being passed */
  uint8_t wdisable;
  /**<   0 - Switch set to ON position
                                1 - Switch set to OFF position
                            */

  /* Optional */
  /*  Power-off Mode */
  uint8_t poweroffmode_valid;  /**< Must be set to true if poweroffmode is being passed */
  uint8_t poweroffmode;
  /**<   Action taken when W_DISABLE is switched to the OFF position:
                                    0 - LPM
                                    1 - Shutdown
                                    2 - Ignore
                                */

  /* Optional */
  /*  LPM Persistence */
  uint8_t persistentlpm_valid;  /**< Must be set to true if persistentlpm is being passed */
  uint8_t persistentlpm;
  /**<   0 - Non-persistent LPM
                                     1 - Persistent LPM
                                 */
}dms_swi_get_pc_info_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Response Message; This message queries the device power control status information. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */
}dms_swi_get_pc_info_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * dms_swi_get_gps_service_type_req_msg is empty
 * typedef struct {
 * }dms_swi_get_gps_service_type_req_msg_v01;
 */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Response Message; This message queries the device for the active GPS service type. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */

  /* Optional */
  /*  GPS Service Type */
  uint8_t service_type_valid;  /**< Must be set to true if service_type is being passed */
  uint8_t service_type;
  /**<   0x00 - Default, device dependent
                                    0x01 - QMI LOC
                                    0x02 - QMI PDS
                                */
}dms_swi_get_gps_service_type_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Request Message; This message changes the device’s active GPS service type.  */
typedef struct {

  /* Mandatory */
  /*  GPS Service Type */
  uint8_t service_type;
  /**<   0x00 - Default, device dependent
                                    0x01 - QMI LOC
                                    0x02 - QMI PDS
                                */
}dms_swi_set_gps_service_type_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Response Message; This message changes the device’s active GPS service type.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}dms_swi_set_gps_service_type_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * dms_swi_get_g3k_download_en_req_msg is empty
 * typedef struct {
 * }dms_swi_get_g3k_download_en_req_msg_v01;
 */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Response Message; This message queries whether the modem is allow 
  for G3K FW download method  */
typedef struct {

  /* Mandatory */
  /*  G3K FW Download Enable */
  uint8_t g3kdlen;
  /**<   0x00 - G3k FW download  is disabled
                                0x01 - G3k FW download is enabled
                            */

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}dms_swi_get_g3k_download_en_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Request Message; This message enables/disables the G3K FW download */
typedef struct {

  /* Mandatory */
  /*  G3K FW Download Enable */
  uint8_t g3kdlen;
  /**<   0x00 - G3k FW download  is disabled
                                0x01 - G3k FW download is enabled
                            */
}dms_swi_set_g3k_download_en_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Response Message; This message enables/disables the G3K FW download */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}dms_swi_set_g3k_download_en_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * dms_swi_get_uim_selection_req_msg is empty
 * typedef struct {
 * }dms_swi_get_uim_selection_req_msg_v01;
 */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Response Message; This object is used to get the selected UIM interface number, set in QMI_DMS_SWI_UIM_SELECT */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  uint8_t uim_select;
  /**<  0 ?External UIM Interface
                                        1 ?Embedded UIM
                                        2 ?Remote UIM
                                     */

  /* Mandatory */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}dms_swi_get_uim_selection_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}dms_swi_dssd_read_req_msg_v01;

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Response Message; This command is used to read dual system [R-23] shared data from DSSD partition. It includes boot system flag, dual system swap reason, out of sync flag, SW update state, updated image flag, bad image flag, refresh flag, etc. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  ssid_modem_idx */
  uint8_t ssid_modem_idx;
  /**<  
    1 ¡§C sub system 1
    2 ¡§C sub system 2
  */

  /* Mandatory */
  /*  ssid_lk_idx */
  uint8_t ssid_lk_idx;
  /**<  
    1 ¡§C sub system 1
    2 ¡§C sub system 2
   */

  /* Mandatory */
  /*  ssid_linux_idx */
  uint8_t ssid_linux_idx;
  /**<  
    1 ¡§C sub system 1
    2 ¡§C sub system 2
   */

  /* Mandatory */
  /*  swap_reason */
  uint32_t swap_reason;
  /**<  
    0 ¡§C No swap since power up
    1 ¡§C Swap due to bad image
    2 ¡§C Swap due to normal SW update
    3 ¡§C Swap triggered by AT command
    4 ¡§C Swap triggered by Legato API
   */

  /* Mandatory */
  /*  sw_update_state */
  uint32_t sw_update_state;
  /**<  
    1 ¡§C Normal state
    2 ¡§C Sync state between system 1 and system 2
    3 ¡§C Sync and swap state from active system to inactive system
    4 ¡§C SW recovery phase 1
    5 ¡§C SW recovery phase 2
   */

  /* Mandatory */
  /*  out_of_sync */
  uint32_t out_of_sync;
  /**<  
    0x0 ¡§C flag is cleared
    0x4F6F5300 ¡§C means dual system is out of sync
    0x73796E63 ¡§C means dual system is sync
   */

  /* Mandatory */
  /*  efs_corruption_in_sw_update */
  uint32_t efs_corruption_in_sw_update;
  /**<  
    0x0 ¡§C flag is cleared
    0x45465343 ¡§C means it is necessary to do EFS corruption action
   */

  /* Mandatory */
  /*  edb_in_sw_update */
  uint32_t edb_in_sw_update;
  /**<  
    0 ¡§C flag is cleared
    1 ¡§C means to do EDB in system 1
    2 ¡§C means to do EDB in system 2
   */

  /* Mandatory */
  /*  bad_image */
  uint64_t bad_image;
  /**<  
    Refer to Table 3 2 Bitmask for images in file(Filehold ID 4110880)
   */
}dms_swi_dssd_read_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Request Message; This command is used to write some necessary flags of dual system shared data in Linux side. */
typedef struct {

  /* Optional */
  /*  ssid_modem_idx */
  uint8_t ssid_modem_idx_valid;  /**< Must be set to true if ssid_modem_idx is being passed */
  uint8_t ssid_modem_idx;
  /**<  
    1 ¡§C sub system 1
    2 ¡§C sub system 2
   */

  /* Optional */
  /*  ssid_lk_idx */
  uint8_t ssid_lk_idx_valid;  /**< Must be set to true if ssid_lk_idx is being passed */
  uint8_t ssid_lk_idx;
  /**<  
    1 ¡§C sub system 1
    2 ¡§C sub system 2
   */

  /* Optional */
  /*  ssid_linux_idx */
  uint8_t ssid_linux_idx_valid;  /**< Must be set to true if ssid_linux_idx is being passed */
  uint8_t ssid_linux_idx;
  /**<  
    1 ¡§C sub system 1
    2 ¡§C sub system 2
   */

  /* Optional */
  /*  swap_reason */
  uint8_t swap_reason_valid;  /**< Must be set to true if swap_reason is being passed */
  uint32_t swap_reason;
  /**<  
    0 ¡§C No swap since power up
    1 ¡§C Swap due to bad image
    2 ¡§C Swap due to normal SW update
    3 ¡§C Swap triggered by AT command
    4 ¡§C Swap triggered by Legato API
   */

  /* Optional */
  /*  sw_update_state */
  uint8_t sw_update_state_valid;  /**< Must be set to true if sw_update_state is being passed */
  uint32_t sw_update_state;
  /**<  
    1 ¡§C Normal state
    2 ¡§C Sync state between system 1 and system 2
    3 ¡§C Sync and swap state from active system to inactive system
    4 ¡§C SW recovery phase 1
    5 ¡§C SW recovery phase 2
   */

  /* Optional */
  /*  out_of_sync */
  uint8_t out_of_sync_valid;  /**< Must be set to true if out_of_sync is being passed */
  uint32_t out_of_sync;
  /**<  
    0x0 ¡§C flag is cleared
    0x4F6F5300 ¡§C means dual system is out of sync
    0x73796E63 ¡§C means dual system is sync
   */

  /* Optional */
  /*  efs_corruption_in_sw_update */
  uint8_t efs_corruption_in_sw_update_valid;  /**< Must be set to true if efs_corruption_in_sw_update is being passed */
  uint32_t efs_corruption_in_sw_update;
  /**<  
    0x0 ¡§C flag is cleared
    0x45465343 ¡§C means it is necessary to do EFS corruption action
   */

  /* Optional */
  /*  edb_in_sw_update */
  uint8_t edb_in_sw_update_valid;  /**< Must be set to true if edb_in_sw_update is being passed */
  uint32_t edb_in_sw_update;
  /**<  
    0 ¡§C flag is cleared
    1 ¡§C means to do EDB in system 1
    2 ¡§C means to do EDB in system 2
   */

  /* Optional */
  /*  bad_image */
  uint8_t bad_image_valid;  /**< Must be set to true if bad_image is being passed */
  uint64_t bad_image;
  /**<  
    Refer to Table 3 2 Bitmask for images in file(Filehold ID 4110880)
   */
}dms_swi_dssd_write_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Response Message; This command is used to write some necessary flags of dual system shared data in Linux side. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}dms_swi_dssd_write_resp_msg_v01;  /* Message */
/**
    @}
  */
  
  typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}dms_swi_rf_status_read_req_msg_v01;

/** @addtogroup sierra_swi_dms_api_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t Manufactured_ID;
  /**<  
    Manufactured ID
  */

  uint8_t Product_ID;
  /**<  
    Product ID
  */

  uint8_t Device_status;
  /**<  
    0x0 - something wrong in the RF device
    0x1 ¡§C no error found in the RF device
  */
}dms_swi_RF_status_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Response Message; This command is used to write some necessary flags of dual system shared data in Linux side. */
typedef struct {

  /* Mandatory */
  /*  RF_status */
  uint32_t RF_status_len;  /**< Must be set to # of elements in RF_status */
  dms_swi_RF_status_type_v01 RF_status[10];

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}dms_swi_rf_status_read_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Indication Message; This indication provides bad image information to the host. */
typedef struct {

  /* Mandatory */
  /*  bad_image */
  uint64_t bad_image;
  /**<  
    Refer to Table 3 2 Bitmask for images in file(Filehold ID 4110880)
   */
}dms_swi_bad_image_ind_msg_v01;  /* Message */
/**
    @}
  *//**
    @}
  */
/*
 * dms_swi_get_support_bands_req_msg is empty
 * typedef struct {
 * }dms_swi_get_support_bands_req_msg_v01;
 */

/** @addtogroup dms_qmi_messages
    @{
  */
/** Response Message; This QMI interface is used to query the band sets the device support. The QMI interface will respond all band masks. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  gw_mask */
  uint8_t gw_mask_valid;  /**< Must be set to true if gw_mask is being passed */
  uint64_t gw_mask;
  /**<

   */

  /* Optional */
  /*  l_mask */
  uint8_t l_mask_valid;  /**< Must be set to true if l_mask is being passed */
  uint64_t l_mask[QMI_DMS_L_MASK_SIZE_V01];
  /**<

   */

  /* Optional */
  /*  tds_mask */
  uint8_t tds_mask_valid;  /**< Must be set to true if tds_mask is being passed */
  uint64_t tds_mask;
  /**<

   */
}dms_swi_get_support_bands_resp_msg_v01;  /* Message */
/** 
    @} 
  */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Request Message; This message requests to change the audio capability to enable/disable audio wideband. */
typedef struct {

  /* Mandatory */
  /*  Wbcapenable */
  uint8_t Wbcapenable;
  /**<  
    0 - on the top of SWI default settings, only narrowband is available for network even the network only supports wideband.
    1 - follow SWI default settings.
   */
}dms_swi_set_wb_cap_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Response Message; This message requests to change the audio capability to enable/disable audio wideband. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}dms_swi_set_wb_cap_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * dms_swi_get_wb_cap_req_msg is empty
 * typedef struct {
 * }dms_swi_get_wb_cap_req_msg_v01;
 */

/** @addtogroup sierra_swi_dms_api_qmi_messages
    @{
  */
/** Response Message; This message is used to query the setting for audio wideband capability enable or disable. */
typedef struct {

  /* Mandatory */
  /*  Wbcapenable */
  uint8_t Wbcapenable;
  /**<  
    0 - on the top of SWI default settings, only narrowband is available for network even the network only supports wideband.
    1 - follow SWI default settings.
   */

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}dms_swi_get_wb_cap_resp_msg_v01;  /* Message */
/**
    @}
  */

#endif /* SIERRA */
/* SWISTOP */


/*Service Message Definition*/
/** @addtogroup dms_qmi_msg_ids
    @{
  */
#define QMI_DMS_RESET_REQ_V01 0x0000
#define QMI_DMS_RESET_RESP_V01 0x0000
#define QMI_DMS_SET_EVENT_REPORT_REQ_V01 0x0001
#define QMI_DMS_SET_EVENT_REPORT_RESP_V01 0x0001
#define QMI_DMS_EVENT_REPORT_IND_V01 0x0001
#define QMI_DMS_GET_SUPPORTED_MSGS_REQ_V01 0x001E
#define QMI_DMS_GET_SUPPORTED_MSGS_RESP_V01 0x001E
#define QMI_DMS_GET_SUPPORTED_FIELDS_REQ_V01 0x001F
#define QMI_DMS_GET_SUPPORTED_FIELDS_RESP_V01 0x001F
#define QMI_DMS_GET_DEVICE_CAP_REQ_V01 0x0020
#define QMI_DMS_GET_DEVICE_CAP_RESP_V01 0x0020
#define QMI_DMS_GET_DEVICE_MFR_REQ_V01 0x0021
#define QMI_DMS_GET_DEVICE_MFR_RESP_V01 0x0021
#define QMI_DMS_GET_DEVICE_MODEL_ID_REQ_V01 0x0022
#define QMI_DMS_GET_DEVICE_MODEL_ID_RESP_V01 0x0022
#define QMI_DMS_GET_DEVICE_REV_ID_REQ_V01 0x0023
#define QMI_DMS_GET_DEVICE_REV_ID_RESP_V01 0x0023
#define QMI_DMS_GET_MSISDN_REQ_V01 0x0024
#define QMI_DMS_GET_MSISDN_RESP_V01 0x0024
#define QMI_DMS_GET_DEVICE_SERIAL_NUMBERS_REQ_V01 0x0025
#define QMI_DMS_GET_DEVICE_SERIAL_NUMBERS_RESP_V01 0x0025
#define QMI_DMS_GET_POWER_STATE_REQ_V01 0x0026
#define QMI_DMS_GET_POWER_STATE_RESP_V01 0x0026
#define QMI_DMS_UIM_SET_PIN_PROTECTION_REQ_V01 0x0027
#define QMI_DMS_UIM_SET_PIN_PROTECTION_RESP_V01 0x0027
#define QMI_DMS_UIM_VERIFY_PIN_REQ_V01 0x0028
#define QMI_DMS_UIM_VERIFY_PIN_RESP_V01 0x0028
#define QMI_DMS_UIM_UNBLOCK_PIN_REQ_V01 0x0029
#define QMI_DMS_UIM_UNBLOCK_PIN_RESP_V01 0x0029
#define QMI_DMS_UIM_CHANGE_PIN_REQ_V01 0x002A
#define QMI_DMS_UIM_CHANGE_PIN_RESP_V01 0x002A
#define QMI_DMS_UIM_GET_PIN_STATUS_REQ_V01 0x002B
#define QMI_DMS_UIM_GET_PIN_STATUS_RESP_V01 0x002B
#define QMI_DMS_GET_DEVICE_HARDWARE_REV_REQ_V01 0x002C
#define QMI_DMS_GET_DEVICE_HARDWARE_REV_RESP_V01 0x002C
#define QMI_DMS_GET_OPERATING_MODE_REQ_V01 0x002D
#define QMI_DMS_GET_OPERATING_MODE_RESP_V01 0x002D
#define QMI_DMS_SET_OPERATING_MODE_REQ_V01 0x002E
#define QMI_DMS_SET_OPERATING_MODE_RESP_V01 0x002E
#define QMI_DMS_GET_TIME_REQ_V01 0x002F
#define QMI_DMS_GET_TIME_RESP_V01 0x002F
#define QMI_DMS_GET_PRL_VER_REQ_V01 0x0030
#define QMI_DMS_GET_PRL_VER_RESP_V01 0x0030
#define QMI_DMS_GET_ACTIVATION_STATE_REQ_V01 0x0031
#define QMI_DMS_GET_ACTIVATION_STATE_RESP_V01 0x0031
#define QMI_DMS_ACTIVATE_AUTOMATIC_REQ_V01 0x0032
#define QMI_DMS_ACTIVATE_AUTOMATIC_RESP_V01 0x0032
#define QMI_DMS_ACTIVATE_MANUAL_REQ_V01 0x0033
#define QMI_DMS_ACTIVATE_MANUAL_RESP_V01 0x0033
#define QMI_DMS_GET_USER_LOCK_STATE_REQ_V01 0x0034
#define QMI_DMS_GET_USER_LOCK_STATE_RESP_V01 0x0034
#define QMI_DMS_SET_USER_LOCK_STATE_REQ_V01 0x0035
#define QMI_DMS_SET_USER_LOCK_STATE_RESP_V01 0x0035
#define QMI_DMS_SET_USER_LOCK_CODE_REQ_V01 0x0036
#define QMI_DMS_SET_USER_LOCK_CODE_RESP_V01 0x0036
#define QMI_DMS_READ_USER_DATA_REQ_V01 0x0037
#define QMI_DMS_READ_USER_DATA_RESP_V01 0x0037
#define QMI_DMS_WRITE_USER_DATA_REQ_V01 0x0038
#define QMI_DMS_WRITE_USER_DATA_RESP_V01 0x0038
#define QMI_DMS_READ_ERI_FILE_REQ_V01 0x0039
#define QMI_DMS_READ_ERI_FILE_RESP_V01 0x0039
#define QMI_DMS_RESTORE_FACTORY_DEFAULTS_REQ_V01 0x003A
#define QMI_DMS_RESTORE_FACTORY_DEFAULTS_RESP_V01 0x003A
#define QMI_DMS_VALIDATE_SERVICE_PROGRAMMING_CODE_REQ_V01 0x003B
#define QMI_DMS_VALIDATE_SERVICE_PROGRAMMING_CODE_RESP_V01 0x003B
#define QMI_DMS_UIM_GET_ICCID_REQ_V01 0x003C
#define QMI_DMS_UIM_GET_ICCID_RESP_V01 0x003C
#define QMI_DMS_UIM_GET_CK_STATUS_REQ_V01 0x0040
#define QMI_DMS_UIM_GET_CK_STATUS_RESP_V01 0x0040
#define QMI_DMS_UIM_SET_CK_PROTECTION_REQ_V01 0x0041
#define QMI_DMS_UIM_SET_CK_PROTECTION_RESP_V01 0x0041
#define QMI_DMS_UIM_UNBLOCK_CK_REQ_V01 0x0042
#define QMI_DMS_UIM_UNBLOCK_CK_RESP_V01 0x0042
#define QMI_DMS_UIM_GET_IMSI_REQ_V01 0x0043
#define QMI_DMS_UIM_GET_IMSI_RESP_V01 0x0043
#define QMI_DMS_UIM_GET_STATE_REQ_V01 0x0044
#define QMI_DMS_UIM_GET_STATE_RESP_V01 0x0044
#define QMI_DMS_GET_BAND_CAPABILITY_REQ_V01 0x0045
#define QMI_DMS_GET_BAND_CAPABILITY_RESP_V01 0x0045
#define QMI_DMS_GET_FACTORY_SKU_REQ_V01 0x0046
#define QMI_DMS_GET_FACTORY_SKU_RESP_V01 0x0046
#define QMI_DMS_SET_TIME_REQ_V01 0x004B
#define QMI_DMS_SET_TIME_RESP_V01 0x004B
#define QMI_DMS_GET_ALT_NET_CONFIG_REQ_V01 0x004D
#define QMI_DMS_GET_ALT_NET_CONFIG_RESP_V01 0x004D
#define QMI_DMS_SET_ALT_NET_CONFIG_REQ_V01 0x004E
#define QMI_DMS_SET_ALT_NET_CONFIG_RESP_V01 0x004E
#define QMI_DMS_GET_SW_VERSION_REQ_V01 0x0051
#define QMI_DMS_GET_SW_VERSION_RESP_V01 0x0051
#define QMI_DMS_SET_SPC_REQ_V01 0x0052
#define QMI_DMS_SET_SPC_RESP_V01 0x0052
#define QMI_DMS_GET_CURRENT_PRL_INFO_REQ_V01 0x0053
#define QMI_DMS_GET_CURRENT_PRL_INFO_RESP_V01 0x0053
#define QMI_DMS_BIND_SUBSCRIPTION_REQ_V01 0x0054
#define QMI_DMS_BIND_SUBSCRIPTION_RESP_V01 0x0054
#define QMI_DMS_GET_BIND_SUBSCRIPTION_REQ_V01 0x0055
#define QMI_DMS_GET_BIND_SUBSCRIPTION_RESP_V01 0x0055
#define QMI_DMS_SET_AP_SW_VERSION_REQ_V01 0x0056
#define QMI_DMS_SET_AP_SW_VERSION_RESP_V01 0x0056
#define QMI_DMS_GET_CDMA_LOCK_MODE_REQ_V01 0x0057
#define QMI_DMS_GET_CDMA_LOCK_MODE_RESP_V01 0x0057
#define QMI_DMS_SET_TEST_CONFIG_REQ_V01 0x0058
#define QMI_DMS_SET_TEST_CONFIG_RESP_V01 0x0058
#define QMI_DMS_GET_TEST_CONFIG_REQ_V01 0x0059
#define QMI_DMS_GET_TEST_CONFIG_RESP_V01 0x0059
#define QMI_DMS_CLEAR_TEST_CONFIG_REQ_V01 0x005A
#define QMI_DMS_CLEAR_TEST_CONFIG_RESP_V01 0x005A
#define QMI_DMS_OEM_CHINA_OPERATOR_REQ_V01 0x005B
#define QMI_DMS_OEM_CHINA_OPERATOR_RESP_V01 0x005B
#define QMI_DMS_GET_MAC_ADDRESS_REQ_V01 0x005C
#define QMI_DMS_GET_MAC_ADDRESS_RESP_V01 0x005C
#define QMI_DMS_GET_ENCRYPTED_DEVICE_SERIAL_NUMBERS_REQ_V01 0x005D
#define QMI_DMS_GET_ENCRYPTED_DEVICE_SERIAL_NUMBERS_RESP_V01 0x005D
/* SWISTART */
#ifdef SIERRA
#define QMI_DMS_SWI_GET_CWE_PKGS_INFO_REQ_V01 0x5556
#define QMI_DMS_SWI_GET_CWE_PKGS_INFO_RESP_V01 0x5556
#define QMI_DMS_SWI_SET_EVENT_REPORT_REQ_V01 0x5557
#define QMI_DMS_SWI_SET_EVENT_REPORT_RESP_V01 0x5557
#define QMI_DMS_SWI_EVENT_REPORT_IND_V01 0x5557
#define QMI_DMS_SWI_GET_ENVIRONMENT_REQ_V01 0x5558
#define QMI_DMS_SWI_GET_ENVIRONMENT_RESP_V01 0x5558
#define QMI_DMS_SWI_SET_ENVIRONMENT_REQ_V01 0x5559
#define QMI_DMS_SWI_SET_ENVIRONMENT_RESP_V01 0x5559
#define QMI_DMS_SWI_RESET_TO_DOWNLOAD_MODE_REQ_V01 0x555A
#define QMI_DMS_SWI_RESET_TO_DOWNLOAD_MODE_RESP_V01 0x555A
#define QMI_DMS_SWI_GET_USB_COMP_REQ_V01 0x555B
#define QMI_DMS_SWI_GET_USB_COMP_RESP_V01 0x555B
#define QMI_DMS_SWI_SET_USB_COMP_REQ_V01 0x555C
#define QMI_DMS_SWI_SET_USB_COMP_RESP_V01 0x555C
#define QMI_DMS_SWI_GET_WU_FWID_REQ_V01 0x555D
#define QMI_DMS_SWI_GET_WU_FWID_RESP_V01 0x555D
#define QMI_DMS_SWI_GET_FCC_AUTH_REQ_V01 0x555E
#define QMI_DMS_SWI_GET_FCC_AUTH_RESP_V01 0x555E
#define QMI_DMS_SWI_SET_FCC_AUTH_REQ_V01 0x555F
#define QMI_DMS_SWI_SET_FCC_AUTH_RESP_V01 0x555F
#define QMI_DMS_SWI_SET_MULTI_CARRIER_REQ_V01 0x5560
#define QMI_DMS_SWI_SET_MULTI_CARRIER_RESP_V01 0x5560
#define QMI_DMS_SWI_SET_MULTI_CARRIER_IND_V01 0x5560
#define QMI_DMS_SWI_GET_MULTI_CARRIER_REQ_V01 0x5561
#define QMI_DMS_SWI_GET_MULTI_CARRIER_RESP_V01 0x5561
#define QMI_DMS_SWI_SET_LOC_HOME_PROV_REQ_V01 0x5562
#define QMI_DMS_SWI_SET_LOC_HOME_PROV_RESP_V01 0x5562
#define QMI_DMS_SWI_GET_FIRMWARE_CURR_REQ_V01 0x5563
#define QMI_DMS_SWI_GET_FIRMWARE_CURR_RESP_V01 0x5563
#define QMI_DMS_SWI_GET_FWUPDATE_STATUS_REQ_V01 0x5564
#define QMI_DMS_SWI_GET_FWUPDATE_STATUS_RESP_V01 0x5564
#define QMI_DMS_SWI_GET_CRASH_ACTION_REQ_V01 0x5565
#define QMI_DMS_SWI_GET_CRASH_ACTION_RESP_V01 0x5565
#define QMI_DMS_SWI_GET_FSN_REQ_V01 0x5567
#define QMI_DMS_SWI_GET_FSN_RESP_V01 0x5567
#define QMI_DMS_SWI_GET_CRASH_INFO_REQ_V01 0x5568
#define QMI_DMS_SWI_GET_CRASH_INFO_RESP_V01 0x5568
#define QMI_DMS_SWI_SET_CRASH_ACTION_REQ_V01 0x5569
#define QMI_DMS_SWI_SET_CRASH_ACTION_RESP_V01 0x5569
#define QMI_DMS_SWI_GET_HOST_DEV_INFO_REQ_V01 0x556A
#define QMI_DMS_SWI_GET_HOST_DEV_INFO_RESP_V01 0x556A
#define QMI_DMS_SWI_SET_HOST_DEV_INFO_REQ_V01 0x556B
#define QMI_DMS_SWI_SET_HOST_DEV_INFO_RESP_V01 0x556B
#define QMI_DMS_SWI_GET_OS_INFO_REQ_V01 0x556C
#define QMI_DMS_SWI_GET_OS_INFO_RESP_V01 0x556C
#define QMI_DMS_SWI_SET_OS_INFO_REQ_V01 0x556D
#define QMI_DMS_SWI_SET_OS_INFO_RESP_V01 0x556D
#define QMI_DMS_SWI_GET_PC_INFO_REQ_V01 0x556E
#define QMI_DMS_SWI_GET_PC_INFO_RESP_V01 0x556E
#define QMI_DMS_SWI_GET_GPS_SERVICE_TYPE_REQ_V01 0x556F
#define QMI_DMS_SWI_GET_GPS_SERVICE_TYPE_RESP_V01 0x556F
#define QMI_DMS_SWI_SET_GPS_SERVICE_TYPE_REQ_V01 0x5570
#define QMI_DMS_SWI_SET_GPS_SERVICE_TYPE_RESP_V01 0x5570
#define QMI_DMS_SWI_GET_G3K_DOWNLOAD_EN_REQ_V01 0x5571
#define QMI_DMS_SWI_GET_G3K_DOWNLOAD_EN_RESP_V01 0x5571
#define QMI_DMS_SWI_SET_G3K_DOWNLOAD_EN_REQ_V01 0x5572
#define QMI_DMS_SWI_SET_G3K_DOWNLOAD_EN_RESP_V01 0x5572
#define QMI_DMS_SWI_GET_SUPPORT_BANDS_REQ_V01  0x5577
#define QMI_DMS_SWI_GET_SUPPORT_BANDS_RESP_V01 0x5577
#define QMI_DMS_SWI_GET_RESET_INFO_REQ_V01 0x5580
#define QMI_DMS_SWI_GET_RESET_INFO_RESP_V01 0x5580
#define QMI_DMS_SWI_GET_CUST_FEATURES_REQ_V01 0x6557
#define QMI_DMS_SWI_GET_CUST_FEATURES_RESP_V01 0x6557
#define QMI_DMS_SWI_SET_CUST_FEATURES_REQ_V01 0x6558
#define QMI_DMS_SWI_SET_CUST_FEATURES_RESP_V01 0x6558
#define QMI_DMS_SWI_GET_RTN_DATA_REQ_V01 0x6559
#define QMI_DMS_SWI_GET_RTN_DATA_RESP_V01 0x6559
#define QMI_DMS_SWI_UIM_SELECT_REQ_V01 0x655A
#define QMI_DMS_SWI_UIM_SELECT_RESP_V01 0x655A
#define QMI_DMS_SWI_SET_GPS_EULA_ACCEPTED_REQ_V01 0x655B
#define QMI_DMS_SWI_SET_GPS_EULA_ACCEPTED_RESP_V01 0x655B
#define QMI_DMS_SWI_GET_GPS_EULA_ACCEPTED_REQ_V01 0x655C
#define QMI_DMS_SWI_GET_GPS_EULA_ACCEPTED_RESP_V01 0x655C
#define QMI_DMS_SWI_SET_EMERGENCY_NUMBER_REQ_V01 0x6560
#define QMI_DMS_SWI_SET_EMERGENCY_NUMBER_RESP_V01 0x6560
#define QMI_DMS_SWI_CLR_EMERGENCY_NUMBER_REQ_V01 0x6561
#define QMI_DMS_SWI_CLR_EMERGENCY_NUMBER_RESP_V01 0x6561
#define QMI_DMS_SWI_GET_EMERGENCY_NUMBER_REQ_V01 0x6562
#define QMI_DMS_SWI_GET_EMERGENCY_NUMBER_RESP_V01 0x6562
#define QMI_DMS_SWI_GET_MAP_UART_REQ_V01 0x6563
#define QMI_DMS_SWI_GET_MAP_UART_RESP_V01 0x6563
#define QMI_DMS_SWI_SET_MAP_UART_REQ_V01 0x6564
#define QMI_DMS_SWI_SET_MAP_UART_RESP_V01 0x6564
#define QMI_DMS_SWI_GET_USB_ID_REQ_V01 0x6565
#define QMI_DMS_SWI_GET_USB_ID_RESP_V01 0x6565
#define QMI_DMS_SWI_SET_USB_ID_REQ_V01 0x6566
#define QMI_DMS_SWI_SET_USB_ID_RESP_V01 0x6566
#define QMI_DMS_SWI_GET_UIM_SELECTION_REQ_V01 0x6569
#define QMI_DMS_SWI_GET_UIM_SELECTION_RESP_V01 0x6569
#define QMI_DMS_SWI_DSSD_READ_REQ_V01 0x656A
#define QMI_DMS_SWI_DSSD_READ_RESP_V01 0x656A
#define QMI_DMS_SWI_DSSD_WRITE_REQ_V01 0x656B
#define QMI_DMS_SWI_DSSD_WRITE_RESP_V01 0x656B
#define QMI_DMS_SWI_RF_STATUS_READ_REQ_V01 0x656C
#define QMI_DMS_SWI_RF_STATUS_READ_RESP_V01 0x656C
#define QMI_DMS_SWI_BAD_IMAGE_IND_V01 0x656D
#define QMI_DMS_SWI_SET_WB_CAP_REQ_V01 0x656E
#define QMI_DMS_SWI_SET_WB_CAP_RESP_V01 0x656E
#define QMI_DMS_SWI_GET_WB_CAP_REQ_V01 0x656F
#define QMI_DMS_SWI_GET_WB_CAP_RESP_V01 0x656F
#endif /* SIERRA */
/* SWISTOP */
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor 
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro dms_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type dms_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );
 
/** This macro should be used to get the service object */ 
#define dms_get_service_object_v01( ) \
          dms_get_service_object_internal_v01( \
            DMS_V01_IDL_MAJOR_VERS, DMS_V01_IDL_MINOR_VERS, \
            DMS_V01_IDL_TOOL_VERS )
/** 
    @} 
  */


#ifdef __cplusplus
}
#endif
#endif

