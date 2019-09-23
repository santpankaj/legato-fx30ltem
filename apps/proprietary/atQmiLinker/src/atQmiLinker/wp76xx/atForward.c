/**
 * @file atForward.c: Implements wp76xx QMI AT commands forwarding service
 *
 * <HR>
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#include <legato.h>
#include <pthread.h>
#include <qmi_client.h>
#include <qmi_includes.h>
#include <qmi.h>
#include <qmi_platform_config.h>
#include <qmi_atcop_srvc.h>
#include <atLinker.h>
#include <utils/utils.h>
#include "atForward.h"

//--------------------------------------------------------------------------------------------------
/**
 * Maximum attempts for qmi client initialization
 */
//--------------------------------------------------------------------------------------------------
#define MAX_QMI_AT_SVC_CONNECT_ATTEMPTS     20

//--------------------------------------------------------------------------------------------------
/**
 * Timeout for sending ATFWD responses
 */
//--------------------------------------------------------------------------------------------------
#define QMI_SYNC_MSG_TIMEOUT                10000

//--------------------------------------------------------------------------------------------------
/**
 * Maximum QMI message size for ATFWD
 */
//--------------------------------------------------------------------------------------------------
#define QMI_AT_MAX_MSG_SIZE                 512

//--------------------------------------------------------------------------------------------------
/**
 * Timeout in milliseconds for qmi client initialization
 */
//--------------------------------------------------------------------------------------------------
#define QMI_AT_CLIENT_INIT_TIMEOUT          1000

//--------------------------------------------------------------------------------------------------
/**
 * AT command response
*/
//--------------------------------------------------------------------------------------------------
static char Resp[MAX_RESP_LEN] = { 0 };

//--------------------------------------------------------------------------------------------------
/**
 * Intermediate response buffer offset
*/
//--------------------------------------------------------------------------------------------------
static size_t Offset = 0;

//--------------------------------------------------------------------------------------------------
/**
 * Is a forwarded command in progress
*/
//--------------------------------------------------------------------------------------------------
static bool InProgress = false;

//--------------------------------------------------------------------------------------------------
/**
 * AT user handle
*/
//--------------------------------------------------------------------------------------------------
static int AtHandle = 0;

//--------------------------------------------------------------------------------------------------
/**
 * AT service client
*/
//--------------------------------------------------------------------------------------------------
static qmi_client_type QmiAtSvcClient = NULL;

//--------------------------------------------------------------------------------------------------
/**
 * Flag to indicate at service is up
*/
//--------------------------------------------------------------------------------------------------
static bool RegForPrimaryPort = false;

//--------------------------------------------------------------------------------------------------
/**
 * Msg_t struct definition
*/
//--------------------------------------------------------------------------------------------------
typedef struct {
    unsigned char msg[QMI_AT_MAX_MSG_SIZE];
    int len;
}
Msg_t;

//--------------------------------------------------------------------------------------------------
/**
 * QmiMsg_t struct definition
*/
//--------------------------------------------------------------------------------------------------
typedef struct {
    Msg_t req;
    Msg_t resp;
}
QmiMsg_t;

//--------------------------------------------------------------------------------------------------
/**
 * QmiMsg_t struct initialization
*/
//--------------------------------------------------------------------------------------------------
static QmiMsg_t QmiMsg = {
    .req = {
        .msg = {0},
        .len = 0,
    },
    .resp = {
        .msg = {0},
        .len = 0,
    },
};

//--------------------------------------------------------------------------------------------------
/**
 * AT command callback
 */
//--------------------------------------------------------------------------------------------------
static void QmiAtCmdCb
(
    qmi_client_type                userHandle,
    unsigned int                   msgId,
    void                          *indBufPtr,
    unsigned int                   indBufLen,
    void                          *userDataPtr
)
{
    char cmd[QMI_CMD_LEN] = {0};
    qmi_atcop_indication_data_type indData;
    AtEvent_t *atEventPtr = (AtEvent_t *)userDataPtr;

    switch (msgId)
    {
        case QMI_AT_ABORT_AT_CMD_IND_V01:
            LE_DEBUG("Abort message received from QMI\n");
            break;
        case QMI_AT_FWD_AT_CMD_IND_V01:
            qmi_atcop_srvc_indication_cb_helper(msgId,
                                                indBufPtr,
                                                indBufLen,
                                                &indData);
            pthread_mutex_lock(&atEventPtr->mutex);
            utils_BuildAtCommand(&(indData.at_cmd_fwd_type), cmd);
            AtHandle = indData.at_hndl;
            snprintf(atEventPtr->cmd, MAX_CMD_LEN, cmd);
            LE_DEBUG("Received:`%s'", cmd);
            InProgress = true;
            pthread_cond_signal(&atEventPtr->cond);
            pthread_mutex_unlock(&atEventPtr->mutex);
            break;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Error callback
 */
//--------------------------------------------------------------------------------------------------
static void ErrorCb
(
    qmi_client_type        userHandle,
    qmi_client_error_type  error,
    void                   *errorCbDataPtr
)
{
    qmi_client_error_type err;

    err = qmi_client_release(QmiAtSvcClient);
    if (err != QMI_NO_ERR)
    {
        LE_ERROR("Failed to release qmi client: %d", err);
    }

    QmiAtSvcClient = NULL;
    RegForPrimaryPort = false;

    LE_DEBUG("Qmi client released");
}

//--------------------------------------------------------------------------------------------------
/**
 * Notify callback
 */
//--------------------------------------------------------------------------------------------------
static void NotifyCb
(
    qmi_client_type                userHandle,
    qmi_idl_service_object_type    serviceObj,
    qmi_client_notify_event_type   serviceEvent,
    void                           *notifyCbDataPtr
)
{
    LE_DEBUG("Received QMI AT notification: %d", serviceEvent);
    switch (serviceEvent)
    {
        case QMI_CLIENT_SERVICE_COUNT_INC:
            LE_DEBUG("AT service has started.");
            RegForPrimaryPort = true;
            break;
        case QMI_CLIENT_SERVICE_COUNT_DEC:
            LE_DEBUG("AT service will stop now.");
            break;
        default:
            LE_DEBUG("Received unsupported event from QCCI notifier.");
            break;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Send a synchronous QMI message
 */
//--------------------------------------------------------------------------------------------------
static qmi_client_error_type QmiSendMsg
(
    qmi_client_type           userHandle,
    unsigned int              msgId,
    unsigned int              timeoutMsecs,
    int                       respMsgLen
)
{
    memset(QmiMsg.resp.msg, 0, QMI_AT_MAX_MSG_SIZE);

    return qmi_client_send_raw_msg_sync(userHandle,
                                        msgId,
                                        QmiMsg.req.msg,
                                        respMsgLen,
                                        QmiMsg.resp.msg,
                                        sizeof(QmiMsg.resp.msg),
                                        (unsigned *)&QmiMsg.resp.len,
                                        timeoutMsecs);
}

//--------------------------------------------------------------------------------------------------
/**
 * QMI forward intermediate response
 */
//--------------------------------------------------------------------------------------------------
static int QmiFwdIntermediateResp
(
    qmi_atcop_fwd_resp_at_resp_type *respPtr
)
{
    qmi_client_error_type err;

    memset(QmiMsg.req.msg, 0, QMI_AT_MAX_MSG_SIZE);

    int respMsgLen = qmi_atcop_fwd_at_cmd_resp_helper(respPtr,
                                                      NULL,
                                                      QmiMsg.req.msg,
                                                      &QmiMsg.req.len);

    err = QmiSendMsg(QmiAtSvcClient,
                     QMI_AT_FWD_RESP_AT_CMD_RESP_V01,
                     QMI_SYNC_MSG_TIMEOUT,
                     respMsgLen);
    if (err < 0)
    {
        LE_ERROR("Failed to send response: (%d)", err);
        return -1;
    }

    return 0;
}

//--------------------------------------------------------------------------------------------------
/**
 * QMI forward unsolicited response
 */
//--------------------------------------------------------------------------------------------------
static int QmiFwdUnsolicitedResp
(
    qmi_atcop_at_fwd_urc_req_type *respPtr
)
{
    qmi_client_error_type err;

    memset(QmiMsg.req.msg, 0, QMI_AT_MAX_MSG_SIZE);

    int respMsgLen = qmi_atcop_fwd_at_urc_req_msg_helper(respPtr,
                                                         QmiMsg.req.msg,
                                                         &QmiMsg.req.len);

    err = QmiSendMsg(QmiAtSvcClient,
                     QMI_AT_SEND_AT_URC_REQ_V01,
                     QMI_SYNC_MSG_TIMEOUT,
                     respMsgLen);
    if (err < 0)
    {
        LE_ERROR("Failed to send an unsolicited request message: (%d)", err);
        return -1;
    }

    return 0;
}

//--------------------------------------------------------------------------------------------------
/**
 * Release qmi client
 */
//--------------------------------------------------------------------------------------------------
static int ReleaseQmiClient
(
    qmi_client_type handle
)
{
    qmi_client_error_type err;

    LE_DEBUG("Releasing qmi client %p", handle);

    err = qmi_client_release(handle);
    if (err != QMI_NO_ERR)
    {
        LE_ERROR("Failed to release qmi client: %d", err);
        return -1;
    }

    LE_DEBUG("Successfully released qmi client %p", handle);

    return 0;
}

//--------------------------------------------------------------------------------------------------
/**
 * Send an intermediate response to the forwarded AT command
 */
//--------------------------------------------------------------------------------------------------
static int SendIntermediateResponse
(
    char*  bufPtr
)
{
    int rc, index = 0;
    char at_resp[QMI_ATCOP_AT_RESP_MAX_LEN+1] = {0};
    qmi_atcop_fwd_resp_at_resp_type resp = {0};
    char atRespOk[] = "\r\nOK\r\n";
    char atRespError[] = "\r\nERROR\r\n";
    size_t len = 0;
    size_t maxLen = 0;

    if (!bufPtr)
    {
        LE_ERROR("Invalid parameter");
        return -1;
    }

    maxLen = strlen(bufPtr);

    // Response Buffer should end with an OK pattern in order to match QMI requirement.
    if (utils_EndsWith(bufPtr, atRespOk))
    {
        maxLen -= strlen(atRespOk);
        resp.result = QMI_ATCOP_RESULT_OK;
    }
    else if (utils_EndsWith(bufPtr, atRespError))
    {
        resp.result = QMI_ATCOP_RESULT_ERROR;
    }
    else
    {
        resp.result = QMI_ATCOP_RESULT_OTHER;
    }

    // Split the buffer into small chunks
    do
    {
        bool isLast = ((index + QMI_ATCOP_AT_RESP_MAX_LEN) >= maxLen) ? true : false;
        bool isFirst = (index == 0) ? true : false;

        // Label the chunk according to its order
        if (isFirst && isLast)
        {
            resp.response = QMI_ATCOP_RESP_COMPLETE;
        }
        else if (isFirst)
        {
            resp.response = QMI_ATCOP_RESP_START;
        }
        else if (isLast)
        {
            resp.response = QMI_ATCOP_RESP_END;
        }
        else
        {
            resp.response = QMI_ATCOP_RESP_INTERMEDIATE;
        }

        // Each chunk should not exceed the maximum allowed length for the QMI command
        len = maxLen - index;
        if (len > QMI_ATCOP_AT_RESP_MAX_LEN)
        {
            len = QMI_ATCOP_AT_RESP_MAX_LEN;
        }

        // Prepare the buffer
        memset(at_resp, 0x00, QMI_ATCOP_AT_RESP_MAX_LEN+1);
        memcpy(at_resp, bufPtr + index, len);
        resp.at_hndl = AtHandle;
        resp.at_resp = (unsigned char *)at_resp;

        LE_DEBUG("\n##%s##", at_resp);

        // Send the QMI response
        rc = QmiFwdIntermediateResp(&resp);
        if (rc < 0)
        {
            LE_ERROR("Failed to send response: %d", rc);
            return -1;
        }

        index += QMI_ATCOP_AT_RESP_MAX_LEN;

    } while (index < maxLen);

    return 0;
}

//--------------------------------------------------------------------------------------------------
/**
 * Send an unsolicited response
 */
//--------------------------------------------------------------------------------------------------
static int SendUnsolicitedResponse
(
    char*  bufPtr
)
{
    size_t len;
    qmi_atcop_at_fwd_urc_req_type resp;

    len = strlen(bufPtr);

    if (len >= QMI_ATCOP_AT_URC_MAX_LEN)
    {
        LE_ERROR("Command is too long to be forwarded: %d > %d", len, QMI_ATCOP_AT_URC_MAX_LEN);
        return -1;
    }

    resp.at_hndl = AtHandle;
    resp.at_urc = (unsigned char *)bufPtr;
    resp.status = QMI_ATCOP_AT_URC_COMPLETE;

    LE_DEBUG("\n**%s**", bufPtr);

    return QmiFwdUnsolicitedResp(&resp);
}

//--------------------------------------------------------------------------------------------------
/**
 * AtLinker init callback implementation
 */
//--------------------------------------------------------------------------------------------------
static int Init
(
    AtEvent_t *atEventPtr
)
{
    qmi_client_error_type err;
    qmi_client_type qmiAtSvcClient;
    qmi_idl_service_object_type qmiAtSvcObj;
    qmi_client_os_params qmiAtOsParams;
    qmi_client_type qmiAtNotifier;
    int numRetries = 1;

    if (!atEventPtr)
    {
        LE_ERROR("Invalid parameter");
        return -1;
    }

    LE_DEBUG("Connecting to QMI atcop service");

    qmiAtSvcObj = at_get_service_object_v01();

    while (numRetries <= MAX_QMI_AT_SVC_CONNECT_ATTEMPTS)
    {
        memset(&qmiAtOsParams, 0, sizeof(qmiAtOsParams));
        err = qmi_client_init_instance( qmiAtSvcObj,
                                        QMI_CLIENT_INSTANCE_ANY,
                                        QmiAtCmdCb,
                                        atEventPtr,
                                        &qmiAtOsParams,
                                        QMI_AT_CLIENT_INIT_TIMEOUT,
                                        &qmiAtSvcClient);
        if (err == QMI_NO_ERR)
        {
            LE_DEBUG("Successfully registered client instance");
            LE_DEBUG("Tried %d time%s", numRetries, (numRetries > 1)?"s":"");
            break;
        }
        else
        {
            if (numRetries == MAX_QMI_AT_SVC_CONNECT_ATTEMPTS)
            {
                LE_ERROR("Failed to register client instance: %d", err);
                return -1;
            }
        }
        numRetries++;
    }

    err = qmi_client_register_error_cb(qmiAtSvcClient, ErrorCb, NULL);
    if (err != QMI_NO_ERR)
    {
        LE_ERROR("Failed to register error callback: %d", err);
    }

    memset(&qmiAtOsParams, 0, sizeof(qmiAtOsParams));
    err = qmi_client_notifier_init(qmiAtSvcObj, &qmiAtOsParams, &qmiAtNotifier);
    if (err != QMI_NO_ERR)
    {
        LE_ERROR("Failed to initialize notifier: %d", err);
        goto atcop_err;
    }

    err = qmi_client_register_notify_cb(qmiAtNotifier, NotifyCb, NULL);
    if (err != QMI_NO_ERR)
    {
        LE_ERROR("Failed to register notifier callback: %d", err);
        goto notifier_err;
    }

    QmiAtSvcClient = qmiAtSvcClient;

    LE_INFO("Successfully initialized qmi atcop service");

    return 0;

notifier_err:
    qmi_client_release(qmiAtNotifier);
atcop_err:
    qmi_client_release(qmiAtSvcClient);
    return -1;
}

//--------------------------------------------------------------------------------------------------
/**
 * AtLinker release callback implementation
 */
//--------------------------------------------------------------------------------------------------
static int Release
(
    void
)
{
    if (-1 == ReleaseQmiClient(QmiAtSvcClient))
    {
        return -1;
    }

    QmiAtSvcClient = NULL;

    return 0;
}

//--------------------------------------------------------------------------------------------------
/**
 * AtLinker registerCmds callback implementation
 */
//--------------------------------------------------------------------------------------------------
static int RegisterCmds
(
    char **cmdsPtr,
    unsigned int count
)
{
    int errCount = count;
    qmi_client_error_type err;
    qmi_atcop_at_cmd_fwd_req_type cmd = {
        1,
        {
            {QMI_ATCOP_AT_CMD_NOT_ABORTABLE, ""}
        }
    };

    if (!cmdsPtr)
    {
        LE_ERROR("Invalid parameter");
        return -1;
    }

    if (!RegForPrimaryPort)
    {
        LE_ERROR("Service not ready");
        return -1;
    }

    while (count)
    {
        snprintf((char *)cmd.qmi_atcop_at_cmd_fwd_req_type[0].at_cmd_name,
                QMI_ATCOP_AT_CMD_NAME_MAX_LEN, cmdsPtr[count-1]);
        memset(QmiMsg.req.msg , 0, QMI_AT_MAX_MSG_SIZE);
        int respMsgLen = qmi_atcop_reg_at_command_fwd_req_msg_helper(&cmd,
                                                                     QmiMsg.req.msg,
                                                                     &QmiMsg.req.len);
        err = QmiSendMsg(QmiAtSvcClient,
                         QMI_AT_REG_AT_CMD_FWD_REQ_V01,
                         QMI_SYNC_MSG_TIMEOUT,
                         respMsgLen);
        if (err != QMI_NO_ERR)
        {
            LE_ERROR("Failed to register command \"%s\": %d",
                cmd.qmi_atcop_at_cmd_fwd_req_type[0].at_cmd_name, err);
            errCount--;
        }
        else
        {
            LE_DEBUG("Cmd \"%s\" registered",
                cmd.qmi_atcop_at_cmd_fwd_req_type[0].at_cmd_name);
        }
        count--;
    }

    if (!errCount)
    {
        ReleaseQmiClient(QmiAtSvcClient);
        return -1;
    }

    return 0;
}

//--------------------------------------------------------------------------------------------------
/**
 * AtLinker sendResponse callback implementation
 */
//--------------------------------------------------------------------------------------------------
static int SendResponse
(
    char *bufPtr,
    size_t len
)
{
    int rc;

    if (!bufPtr)
    {
        LE_ERROR("Invalid parameter");
        return -1;
    }

    if ((Offset + len) > MAX_RESP_LEN)
    {
        LE_ERROR("Response size exceeds AT buffer size. Dropping the response");
        return -1;
    }

    memcpy(Resp + Offset, bufPtr, len);

    if (!InProgress)
    {
        rc = SendUnsolicitedResponse(Resp);
        memset(Resp, 0, MAX_RESP_LEN);
        return rc;
    }

    if (!utils_CheckResponse(bufPtr))
    {
        rc = SendIntermediateResponse(Resp);
        memset(Resp, 0, MAX_RESP_LEN);
        Offset = 0;
        InProgress = false;
        return rc;
    }

    Offset += len;

    return 0;
}

//--------------------------------------------------------------------------------------------------
/**
 * Initialize AtLinker
 */
//--------------------------------------------------------------------------------------------------
void atLinker_Init
(
    AtLinker_t *atLinkerPtr
)
{
    if (!atLinkerPtr)
    {
        LE_ERROR("Invalid parameter");
        return;
    }

    LE_DEBUG("Initializing atLinker");

    atLinkerPtr->init = Init;
    atLinkerPtr->registerCmds = RegisterCmds;
    atLinkerPtr->release = Release;
    atLinkerPtr->sendResponse = SendResponse;
}
