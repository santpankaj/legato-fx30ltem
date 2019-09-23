/******************************************************************************
 ----------------------------------------------------------------------------
 Copyright (c) 2011-2013 Qualcomm Technologies, Inc.
 All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 ----------------------------------------------------------------------------
*******************************************************************************/

#ifndef QMI_FRAMEWORK_ERR_H_
#define QMI_FRAMEWORK_ERR_H_

#include <stdlib.h>
#include <stdio.h>
#include "qmi_common.h"
#include "smem_log.h"

#ifdef __cplusplus
extern "C" {
#endif

/* SWISTART */
#ifdef SIERRA
#include "msg.h"
#endif
/* SWISTOP */
#ifdef QMI_FW_ADB_LOG
#define LOG_TAG "QMI_FW"
#include <utils/Log.h>
#include "common_log.h"

#ifdef QMI_CCI_ANDROID
extern unsigned int qmi_cci_debug_level;
#define QMI_CCI_LOGI(x...) do { \
  if(qmi_cci_debug_level <= ANDROID_LOG_INFO) \
    SLOGI("QCCI: "x); \
} while(0)
#define QMI_CCI_LOGD(x...) do { \
  if(qmi_cci_debug_level <= ANDROID_LOG_DEBUG) \
    SLOGD("QCCI: "x); \
} while(0)
#else
#define QMI_CCI_LOGI(x...)
#define QMI_CCI_LOGD(x...)
#endif

#ifdef QMI_CSI_ANDROID
extern unsigned int qmi_csi_debug_level;
#define QMI_CSI_LOGI(x...) do { \
  if(qmi_csi_debug_level <= ANDROID_LOG_INFO) \
    SLOGI("QCSI: "x); \
} while(0)
#define QMI_CSI_LOGD(x...) do { \
  if(qmi_csi_debug_level <= ANDROID_LOG_DEBUG) \
    SLOGD("QCSI: "x); \
} while(0)
#else
#define QMI_CSI_LOGI(x...)
#define QMI_CSI_LOGD(x...)
#endif

#define QMI_FW_LOGE(x...) LOGE(x);

/* SWISTART */
#ifdef defined(QMI_FW_SYSLOG)
#include <syslog.h>
#endif
/* SWISTOP */

#if (QMI_FW_SYSLOG_LEVEL >= LOG_INFO)
#define QMI_CCI_LOGI(x...) syslog(LOG_INFO, "QMI_FW: QCCI: "x)
#define QMI_CSI_LOGI(x...) syslog(LOG_INFO, "QMI_FW: QCSI: "x)
#else
#define QMI_CCI_LOGI(x...)
#define QMI_CSI_LOGI(x...)
#endif

#if (QMI_FW_SYSLOG_LEVEL >= LOG_DEBUG)
#define QMI_CCI_LOGD(x...) syslog(LOG_DEBUG, "QMI_FW: QCCI: "x)
#define QMI_CSI_LOGD(x...) syslog(LOG_DEBUG, "QMI_FW: QCSI: "x)
#else
#define QMI_CCI_LOGD(x...)
#define QMI_CSI_LOGD(x...)
#endif

#define QMI_FW_LOGE(x...)  syslog(LOG_ERR, x)

#else
/* SWISTART */
#ifdef SIERRA
#define QMI_CCI_LOGI(x...) do { } while(0)
#define QMI_CCI_LOGD(x...) do { } while(0)
#define QMI_CSI_LOGI(x...) do { } while(0)
#define QMI_CSI_LOGD(x...) do { } while(0)
#define QMI_FW_LOGE(x...)  do { } while(0)

#define QMI_DEBUG_MSG_LOW(x_fmt, a, b, c) \
    MSG_3 (MSG_SSID_LINUX_DATA, MSG_LEGACY_LOW, x_fmt, a, b, c)

#define QMI_DEBUG_MSG_MED(x_fmt, a, b, c) \
    MSG_3 (MSG_SSID_LINUX_DATA, MSG_LEGACY_MED, x_fmt, a, b, c)

#define QMI_DEBUG_MSG_HIGH(x_fmt, a, b, c) \
    MSG_3 (MSG_SSID_LINUX_DATA, MSG_LEGACY_HIGH, x_fmt, a, b, c)

#define QMI_DEBUG_MSG_ERROR(x_fmt, a, b, c) \
    MSG_3 (MSG_SSID_LINUX_DATA, MSG_LEGACY_ERROR, x_fmt, a, b, c)

#define QMI_DEBUG_MSG_FATAL(x_fmt, a, b, c) \
    MSG_3 (MSG_SSID_LINUX_DATA, MSG_LEGACY_FATAL, x_fmt, a, b, c)
#else
/* SWISTOP */
#define QMI_CCI_LOGI(x...) do { \
        fprintf(stdout, "%s(%d) ", __FUNCTION__, __LINE__); \
        fprintf(stdout, ##x);                               \
    } while(0)

#define QMI_CCI_LOGD(x...) do { \
        fprintf(stdout, "%s(%d) ", __FUNCTION__, __LINE__); \
        fprintf(stdout, ##x);                               \
    } while(0)

#define QMI_CSI_LOGI(x...) do { \
        fprintf(stdout, "%s(%d) ", __FUNCTION__, __LINE__); \
        fprintf(stdout, ##x);                               \
    } while(0)

#define QMI_CSI_LOGD(x...) do { \
        fprintf(stdout, "%s(%d) ", __FUNCTION__, __LINE__); \
        fprintf(stdout, ##x);                               \
    } while(0)

#define QMI_FW_LOGE(x...) do { \
        fprintf(stderr, "%s(%d) ", __FUNCTION__, __LINE__); \
        fprintf(stderr, ##x);                               \
    } while(0)
/* SWISTART */
#endif
/* SWISTOP */
#endif

#define ASSERT( xx_exp ) \
    if( !(xx_exp) ) \
    { \
        QMI_FW_LOGE("Assertion "  #xx_exp " failed"); \
        exit(1); \
    } \

#define QMI_CCI_LOG_EVENT_TX            (SMEM_LOG_QCCI_EVENT_BASE + 0x00)
#define QMI_CCI_LOG_EVENT_RX            (SMEM_LOG_QCCI_EVENT_BASE + 0x01)
#define QMI_CCI_LOG_EVENT_ERROR         (SMEM_LOG_QCCI_EVENT_BASE + 0x03)

#define QMI_CCI_OS_LOG_TX(header) \
  do { \
    uint8_t cntl_flag; \
    uint16_t txn_id, msg_id, msg_len; \
    decode_header(header, &cntl_flag, &txn_id, &msg_id, &msg_len); \
    SMEM_LOG_EVENT(QMI_CCI_LOG_EVENT_TX, cntl_flag << 16 | txn_id, msg_id << 16 | msg_len, 0); \
    QMI_CCI_LOGD("QMI_CCI_TX: cntl_flag - %02x, txn_id - %04x, msg_id - %04x, msg_len - %04x\n", \
                  cntl_flag, txn_id, msg_id, msg_len); \
  } while(0)

#define QMI_CCI_OS_LOG_RX(header) \
  do { \
    uint8_t cntl_flag; \
    uint16_t txn_id, msg_id, msg_len; \
    decode_header(header, &cntl_flag, &txn_id, &msg_id, &msg_len); \
    SMEM_LOG_EVENT(QMI_CCI_LOG_EVENT_RX, cntl_flag << 16 | txn_id, msg_id << 16 | msg_len, 0); \
    QMI_CCI_LOGD("QMI_CCI_RX: cntl_flag - %02x, txn_id - %04x, msg_id - %04x, msg_len - %04x\n", \
                  cntl_flag, txn_id, msg_id, msg_len); \
  } while(0)

#define QMI_CSI_LOG_EVENT_TX            (SMEM_LOG_QCSI_EVENT_BASE + 0x00)
#define QMI_CSI_LOG_EVENT_RX            (SMEM_LOG_QCSI_EVENT_BASE + 0x01)
#define QMI_CSI_LOG_EVENT_ERROR         (SMEM_LOG_QCSI_EVENT_BASE + 0x03)

#define QMI_CSI_OS_LOG_TX(header) \
  do { \
    uint8_t cntl_flag; \
    uint16_t txn_id, msg_id, msg_len; \
    decode_header(header, &cntl_flag, &txn_id, &msg_id, &msg_len); \
    SMEM_LOG_EVENT(QMI_CSI_LOG_EVENT_TX, cntl_flag << 16 | txn_id, msg_id << 16 | msg_len, 0); \
    QMI_CSI_LOGD("QMI_CSI_TX: cntl_flag - %02x, txn_id - %04x, msg_id - %04x, msg_len - %04x\n", \
                  cntl_flag, txn_id, msg_id, msg_len); \
  } while(0)

#define QMI_CSI_OS_LOG_RX(header) \
  do { \
    uint8_t cntl_flag; \
    uint16_t txn_id, msg_id, msg_len; \
    decode_header(header, &cntl_flag, &txn_id, &msg_id, &msg_len); \
    SMEM_LOG_EVENT(QMI_CSI_LOG_EVENT_RX, cntl_flag << 16 | txn_id, msg_id << 16 | msg_len, 0); \
    QMI_CSI_LOGD("QMI_CSI_RX: cntl_flag - %02x, txn_id - %04x, msg_id - %04x, msg_len - %04x\n", \
                  cntl_flag, txn_id, msg_id, msg_len); \
  } while(0)

#ifdef __cplusplus
}
#endif

#endif
