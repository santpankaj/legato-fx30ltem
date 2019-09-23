/**
 * @file atForward.c: Implements wp85 QMI AT commands forwarding service
 *
 * <HR>
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#include <legato.h>
#include <pthread.h>
#include <qmi.h>
#include <qmi_platform_config.h>
#include <qmi_atcop_srvc.h>
#include <atLinker.h>
#include <utils/utils.h>

//--------------------------------------------------------------------------------------------------
/**
 * Default forwarding port
 */
//--------------------------------------------------------------------------------------------------
#define DEFAULT_PORT            QMI_PORT_RMNET_0

//--------------------------------------------------------------------------------------------------
/**
 * QMI Client handle
*/
//--------------------------------------------------------------------------------------------------
static int QmiHandle = 0;

//--------------------------------------------------------------------------------------------------
/**
 * Atcop Client handle
*/
//--------------------------------------------------------------------------------------------------
static int ClientHandle = 0;

//--------------------------------------------------------------------------------------------------
/**
 * AT user handle
*/
//--------------------------------------------------------------------------------------------------
static int AtHandle = 0;

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
 * AT command callback
 */
//--------------------------------------------------------------------------------------------------
static void QmiAtCmdCb
(
    int                             clientHandle,
    qmi_service_id_type             service,
    void*                           userDataPtr,
    qmi_atcop_indication_id_type    type,
    qmi_atcop_indication_data_type* dataPtr
)
{
    char cmd[QMI_CMD_LEN] = {0};
    AtEvent_t *atEventPtr = (AtEvent_t *)userDataPtr;

    LE_DEBUG("Received: service %d, type: %d", service, type);

    if (QMI_ATCOP_SERVICE != service)
    {
        LE_ERROR("Invalid service %d", service);
        return;
    }

    switch (type)
    {
        case QMI_ATCOP_SRVC_AT_FWD_MSG_IND_TYPE:
            pthread_mutex_lock(&atEventPtr->mutex);
            utils_BuildAtCommand(&dataPtr->at_cmd_fwd_type, cmd);
            AtHandle = dataPtr->at_hndl;
            snprintf(atEventPtr->cmd, MAX_CMD_LEN, cmd);
            LE_DEBUG("Received:`%s'", cmd);
            InProgress = true;
            pthread_cond_signal(&atEventPtr->cond);
            pthread_mutex_unlock(&atEventPtr->mutex);
            break;
        case QMI_ATCOP_SRVC_INVALID_IND_TYPE:
        case QMI_ATCOP_SRVC_ABORT_MSG_IND_TYPE:
            LE_DEBUG("Invalid or abort indication received");
            break;
        default:
            break;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Initialize qmi client
*/
//--------------------------------------------------------------------------------------------------
static int InitQmiClient
(
    int *handlePtr
)
{
    int handle, rc, err;

    LE_DEBUG("Initializing libqmi client");

    handle = qmi_init(NULL, NULL);
    if (handle < 0)
    {
        LE_ERROR("Failed to initialize libqmi client: %d", handle);
        return -1;
    }

    LE_DEBUG("Successfully initialized libqmi client %d", handle);

    LE_DEBUG("Connecting to %s", DEFAULT_PORT);

    rc = qmi_connection_init(DEFAULT_PORT, &err);
    if (rc < 0)
    {
        LE_ERROR("Failed to initialize connection to %s: %d", DEFAULT_PORT, err);
        goto qmi_err;
    }

    LE_DEBUG("Successfully conneceted to %s", DEFAULT_PORT);

    *handlePtr = handle;

    return 0;

qmi_err:
    qmi_release(handle);
    return -1;
}

//--------------------------------------------------------------------------------------------------
/**
 * Initialize atcop client
*/
//--------------------------------------------------------------------------------------------------
static int InitAtcopClient
(
    int *handlePtr,
    AtEvent_t *atEventPtr
)
{
    int handle, err;

    LE_DEBUG("Connecting to QMI atcop service");

    handle = qmi_atcop_srvc_init_client(DEFAULT_PORT, QmiAtCmdCb, atEventPtr, &err);
    if (handle < 0)
    {
        LE_ERROR("Failed to initialize atcop service: %d", err);
        return -1;
    }

    LE_DEBUG("Successfully intialized atcop service: %d", handle);

    *handlePtr = handle;

    return 0;
}

//--------------------------------------------------------------------------------------------------
/**
 * Release qmi client
*/
//--------------------------------------------------------------------------------------------------
static int ReleaseQmiClient
(
    int handle
)
{
    int rc;

    if (!handle)
    {
        LE_WARN("Trying to release a non intialized libqmi client");
        return -1;
    }

    LE_DEBUG("Releasing libqmi client %d", handle);

    rc = qmi_release(handle);
    if (rc < 0)
    {
        LE_ERROR("Failed to release libqmi client: %d", rc);
        return -1;
    }

    LE_DEBUG("Successfully released libqmi client %d", handle);

    return 0;
}

//--------------------------------------------------------------------------------------------------
/**
 * Release atcop client
*/
//--------------------------------------------------------------------------------------------------
static int ReleaseAtcopClient
(
    int handle
)
{
    int rc, err;

    if (!handle)
    {
        LE_WARN("Trying to release a non initialized atcop client");
        return -1;
    }

    LE_DEBUG("Releasing atcop client %d", handle);

    rc = qmi_atcop_srvc_release_client(handle, &err);
    if (rc < 0)
    {
        LE_ERROR("Failed to release atcop client: %d", err);
        return -1;
    }

    LE_DEBUG("Successfully released atcop client %d", handle);

    return 0;
}

//--------------------------------------------------------------------------------------------------
/**
 * Send an intermediate response to the forwarded at command
 */
//--------------------------------------------------------------------------------------------------
static int SendIntermediateResponse
(
    char*  bufPtr
)
{
    int rc, index = 0, err;
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
        rc = qmi_atcop_fwd_at_cmd_resp(ClientHandle, &resp, &err);
        if (rc < 0)
        {
            LE_ERROR("Failed to send response: (%d - %d)", rc, err);
            return -1;
        }

        index += QMI_ATCOP_AT_RESP_MAX_LEN;

    } while(index < maxLen);

    return 0;
}

//--------------------------------------------------------------------------------------------------
/**
 * Send an unsolicited response
 */
//--------------------------------------------------------------------------------------------------
static int SendUnsolicitedResponse
(
    char*  buf
)
{
    int rc, err;
    size_t len;
    qmi_atcop_at_fwd_urc_req_type resp;

    len = strlen(buf);

    if (len >= QMI_ATCOP_AT_URC_MAX_LEN)
    {
        LE_ERROR("Command is too long to be forwarded: %d > %d", len, QMI_ATCOP_AT_URC_MAX_LEN);
        return -1;
    }

    resp.at_hndl = AtHandle;
    resp.at_urc = (unsigned char *)buf;
    resp.status = QMI_ATCOP_AT_URC_COMPLETE;

    rc = qmi_atcop_fwd_at_urc_req(ClientHandle, &resp, &err);
    if (rc < 0)
    {
        LE_ERROR("Failed to send response: (%d - %d)", rc, err);
        return -1;
    }

    LE_DEBUG("\n**%s**", buf);

    return 0;
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
    if (!atEventPtr)
    {
        LE_ERROR("Invalid parameter");
        return -1;
    }

    if (-1 == InitQmiClient(&QmiHandle))
    {
        return -1;
    }

    if (-1 == InitAtcopClient(&ClientHandle, atEventPtr))
    {
        ReleaseQmiClient(QmiHandle);
        return -1;
    }

    return 0;
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
    int err = 0;

    if (-1 == ReleaseAtcopClient(ClientHandle))
    {
        err = -1;
    }

    if (-1 == ReleaseQmiClient(QmiHandle))
    {
        err = -1;
    }

    return err;
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
    int rc, err, errCount = count;

    if (!cmdsPtr)
    {
        LE_ERROR("Invalid parameter");
        return -1;
    }

    qmi_atcop_at_cmd_fwd_req_type cmd = {
        1,
        {
            {QMI_ATCOP_AT_CMD_NOT_ABORTABLE, ""}
        }
    };

    while (count)
    {
        snprintf((char *)cmd.qmi_atcop_at_cmd_fwd_req_type[0].at_cmd_name,
                QMI_ATCOP_AT_CMD_NAME_MAX_LEN, cmdsPtr[count-1]);
        rc = qmi_atcop_reg_at_command_fwd_req(ClientHandle, &cmd, &err);
        if (rc < 0)
        {
            LE_ERROR("Failed to register command \"%s\": (%d - %d)",
                cmd.qmi_atcop_at_cmd_fwd_req_type[0].at_cmd_name, rc, err);
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
        ReleaseAtcopClient(ClientHandle);
        ReleaseQmiClient(ClientHandle);
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
