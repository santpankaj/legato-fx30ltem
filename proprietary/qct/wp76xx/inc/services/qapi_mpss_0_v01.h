#ifndef QAPI_MPSS_0_SERVICE_01_H
#define QAPI_MPSS_0_SERVICE_01_H
/**
  @file qapi_mpss_0_v01.h

  @brief This is the public header file which defines the qapi_mpss_0 service Data structures.

  This header file defines the types and structures that were defined in
  qapi_mpss_0. It contains the constant values defined, enums, structures,
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
   It was generated on: Wed Oct 25 2017 (Spin 0)
   From IDL File: qapi_mpss_0_v01.idl */

/** @defgroup qapi_mpss_0_qmi_consts Constant values defined in the IDL */
/** @defgroup qapi_mpss_0_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup qapi_mpss_0_qmi_enums Enumerated types used in QMI messages */
/** @defgroup qapi_mpss_0_qmi_messages Structures sent as QMI messages */
/** @defgroup qapi_mpss_0_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup qapi_mpss_0_qmi_accessor Accessor for QMI service object */
/** @defgroup qapi_mpss_0_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "qapi_common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup qapi_mpss_0_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define QAPI_MPSS_0_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define QAPI_MPSS_0_V01_IDL_MINOR_VERS 0x01
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define QAPI_MPSS_0_V01_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
#define QAPI_MPSS_0_V01_MAX_MESSAGE_ID 0x000E
/**
    @}
  */


/** @addtogroup qapi_mpss_0_qmi_consts
    @{
  */
/**
    @}
  */

/* Conditional compilation tags for message removal */ 
//#define REMOVE_QAPI_DATA_LINUX_OPERATION_V01 
//#define REMOVE_QAPI_DBI_OPERATION_V01 
//#define REMOVE_QAPI_FM_OPERATION_V01 
//#define REMOVE_QAPI_LEGATO_VERSION_READ_V01 
//#define REMOVE_QAPI_NV_OPERATION_V01 
//#define REMOVE_QAPI_PC_OPERATION_V01 
//#define REMOVE_QAPI_PING_V01 
//#define REMOVE_QAPI_QFPROM_OPERATION_V01 
//#define REMOVE_QAPI_RESET_V01 
//#define REMOVE_QAPI_SGMII_HSIC_TEST_OPERATION_V01 
//#define REMOVE_QAPI_TCXO_OPERATION_V01 
//#define REMOVE_QAPI_UBIST_OPERATION_V01 
//#define REMOVE_QAPI_UD_OPERATION_V01 
//#define REMOVE_QAPI_WAKELOCK_OP_V01 

/*Service Message Definition*/
/** @addtogroup qapi_mpss_0_qmi_msg_ids
    @{
  */
/* NOTE: imorrison 2014:03:18 - these need to be on the public API
 * whereas the rest of this file should be private.  This is just a
 * consequence of the way that the IDL compiler deals with the
 * included common IDL.  Move these to the common API
 */
#if 0
#define QAPI_PING_REQ_V01 0x0001
#define QAPI_PING_RESP_V01 0x0001
#define QAPI_PING_IND_V01 0x0001
#define QAPI_RESET_REQ_V01 0x0002
#define QAPI_RESET_RESP_V01 0x0002
#define QAPI_NV_OP_REQ_V01 0x0003
#define QAPI_NV_OP_RESP_V01 0x0003
#define QAPI_NV_OP_IND_V01 0x0003
#define QAPI_UBIST_OP_REQ_V01 0x0004
#define QAPI_UBIST_OP_RESP_V01 0x0004
#define QAPI_UBIST_OP_IND_V01 0x0004
#define QAPI_UD_OP_REQ_V01 0x0005
#define QAPI_UD_OP_RESP_V01 0x0005
#define QAPI_QFPROM_OP_REQ_V01 0x0006
#define QAPI_QFPROM_OP_RESP_V01 0x0006
#define QAPI_PC_OP_REQ_V01 0x0007
#define QAPI_PC_OP_RESP_V01 0x0007
#define QAPI_DATA_LINUX_OP_REQ_V01 0x0008
#define QAPI_DATA_LINUX_OP_RESP_V01 0x0008
#define QAPI_SGMII_HSIC_TEST_OP_REQ_V01 0x0009
#define QAPI_SGMII_HSIC_TEST_OP_RESP_V01 0x0009
#define QAPI_LEGATO_VERSION_READ_REQ_V01 0x000A
#define QAPI_LEGATO_VERSION_READ_RESP_V01 0x000A
#define QAPI_DBI_OP_REQ_V01 0x000B
#define QAPI_DBI_OP_RESP_V01 0x000B
#define QAPI_TCXO_OP_REQ_V01 0x000C
#define QAPI_TCXO_OP_RESP_V01 0x000C
#define QAPI_FM_OP_REQ_V01 0x000D
#define QAPI_FM_OP_RESP_V01 0x000D
#define QAPI_WAKELOCK_OP_REQ_V01 0x000E
#define QAPI_WAKELOCK_OP_RESP_V01 0x000E
#endif
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro qapi_mpss_0_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type qapi_mpss_0_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );

/** This macro should be used to get the service object */
#define qapi_mpss_0_get_service_object_v01( ) \
          qapi_mpss_0_get_service_object_internal_v01( \
            QAPI_MPSS_0_V01_IDL_MAJOR_VERS, QAPI_MPSS_0_V01_IDL_MINOR_VERS, \
            QAPI_MPSS_0_V01_IDL_TOOL_VERS )
/**
    @}
  */


#ifdef __cplusplus
}
#endif
#endif

