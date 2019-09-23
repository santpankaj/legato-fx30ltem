/**
 * @file pa_rsim_qmi.c
 *
 * QMI implementation of @ref c_pa_rsim API.
 *
 * <HR>
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#include "legato.h"
#include "pa_rsim.h"
#include "swiQmi.h"


//--------------------------------------------------------------------------------------------------
// Symbol and Enum definitions
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Default identifier of a command/response APDU pair.
 */
//--------------------------------------------------------------------------------------------------
#define DEFAULT_APDU_ID         (0xFFFFFFFF)

//--------------------------------------------------------------------------------------------------
/**
 * Slot used for the remote SIM card: remote slot 1 by default
 */
//--------------------------------------------------------------------------------------------------
#define REMOTE_SIM_CARD_SLOT    UIM_REMOTE_SLOT_1_V01

//--------------------------------------------------------------------------------------------------
/**
 * The UIMRMT (User Identity Module Remote) client.
 * Must be obtained by calling swiQmi_GetClientHandle() before it can be used.
 */
//--------------------------------------------------------------------------------------------------
static qmi_client_type UimrmtClient;

//--------------------------------------------------------------------------------------------------
/**
 * The DMS (Device Management Service) client.  Must be obtained by calling
 * swiQmi_GetClientHandle() before it can be used.
 */
//--------------------------------------------------------------------------------------------------
static qmi_client_type DmsClient;

//--------------------------------------------------------------------------------------------------
/**
 * This event is reported when a SIM action request is received from the modem.
 */
//--------------------------------------------------------------------------------------------------
static le_event_Id_t ActionRequestEvent;

//--------------------------------------------------------------------------------------------------
/**
 * This event is reported when an APDU indication is received from the modem.
 */
//--------------------------------------------------------------------------------------------------
static le_event_Id_t ApduIndicationEvent;

//--------------------------------------------------------------------------------------------------
/**
 * Identifier of the current command/response APDU pair.
 */
//--------------------------------------------------------------------------------------------------
static uint32_t CurrentApduId = DEFAULT_APDU_ID;


//--------------------------------------------------------------------------------------------------
// Private functions
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * SIM connection request handler function called by the QMI UIMRMT service indications.
 */
//--------------------------------------------------------------------------------------------------
static void ConnectionRequestHandler
(
    void*           indBufPtr,  // [IN] The indication message data.
    unsigned int    indBufLen,  // [IN] The indication message data length in bytes.
    void*           contextPtr  // [IN] Context value that was passed to
                                //      swiQmi_AddIndicationHandler().
)
{
    uim_remote_connect_ind_msg_v01 uimrmtConnect;

    qmi_client_error_type clientMsgErr = qmi_client_message_decode(
        UimrmtClient, QMI_IDL_INDICATION, QMI_UIM_REMOTE_CONNECT_IND_V01,
        indBufPtr, indBufLen,
        &uimrmtConnect, sizeof(uimrmtConnect));

    if (QMI_NO_ERR != clientMsgErr)
    {
        LE_ERROR("Error in decoding QMI_UIM_REMOTE_CONNECT_IND_V01, error = %i", clientMsgErr);
        return;
    }

    LE_DEBUG("QMI_UIM_REMOTE_CONNECT_IND_V01 received for SIM slot %d", uimrmtConnect.slot);

    if (REMOTE_SIM_CARD_SLOT == uimrmtConnect.slot)
    {
        pa_rsim_Action_t action = PA_RSIM_CONNECTION;
        le_event_Report(ActionRequestEvent, (void *)&action, sizeof(action));
    }
    else
    {
        LE_ERROR("Unexpected SIM card slot %d used for connection", uimrmtConnect.slot);
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * SIM disconnection request handler function called by the QMI UIMRMT service indications.
 */
//--------------------------------------------------------------------------------------------------
static void DisconnectionRequestHandler
(
    void*           indBufPtr,  // [IN] The indication message data.
    unsigned int    indBufLen,  // [IN] The indication message data length in bytes.
    void*           contextPtr  // [IN] Context value that was passed to
                                //      swiQmi_AddIndicationHandler().
)
{
    uim_remote_disconnect_ind_msg_v01 uimrmtDisconnect;

    qmi_client_error_type clientMsgErr = qmi_client_message_decode(
        UimrmtClient, QMI_IDL_INDICATION, QMI_UIM_REMOTE_DISCONNECT_IND_V01,
        indBufPtr, indBufLen,
        &uimrmtDisconnect, sizeof(uimrmtDisconnect));

    if (QMI_NO_ERR != clientMsgErr)
    {
        LE_ERROR("Error in decoding QMI_UIM_REMOTE_DISCONNECT_IND_V01, error = %i", clientMsgErr);
        return;
    }

    LE_DEBUG("QMI_UIM_REMOTE_DISCONNECT_IND_V01 received for SIM slot %d", uimrmtDisconnect.slot);

    if (REMOTE_SIM_CARD_SLOT == uimrmtDisconnect.slot)
    {
        pa_rsim_Action_t action = PA_RSIM_DISCONNECTION;
        le_event_Report(ActionRequestEvent, (void *)&action, sizeof(action));
    }
    else
    {
        LE_ERROR("Unexpected SIM card slot %d used for disconnection", uimrmtDisconnect.slot);
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * SIM reset request handler function called by the QMI UIMRMT service indications.
 */
//--------------------------------------------------------------------------------------------------
static void ResetRequestHandler
(
    void*           indBufPtr,  // [IN] The indication message data.
    unsigned int    indBufLen,  // [IN] The indication message data length in bytes.
    void*           contextPtr  // [IN] Context value that was passed to
                                //      swiQmi_AddIndicationHandler().
)
{
    uim_remote_card_reset_ind_msg_v01 uimrmtReset;

    qmi_client_error_type clientMsgErr = qmi_client_message_decode(
        UimrmtClient, QMI_IDL_INDICATION, QMI_UIM_REMOTE_CARD_RESET_IND_V01,
        indBufPtr, indBufLen,
        &uimrmtReset, sizeof(uimrmtReset));

    if (QMI_NO_ERR != clientMsgErr)
    {
        LE_ERROR("Error in decoding QMI_UIM_REMOTE_CARD_RESET_IND_V01, error = %i", clientMsgErr);
        return;
    }

    LE_DEBUG("QMI_UIM_REMOTE_CARD_RESET_IND_V01 received for SIM slot %d", uimrmtReset.slot);

    if (REMOTE_SIM_CARD_SLOT == uimrmtReset.slot)
    {
        pa_rsim_Action_t action = PA_RSIM_RESET;
        le_event_Report(ActionRequestEvent, (void *)&action, sizeof(action));
    }
    else
    {
        LE_ERROR("Unexpected SIM card slot %d used for reset", uimrmtReset.slot);
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * SIM power up request handler function called by the QMI UIMRMT service indications.
 */
//--------------------------------------------------------------------------------------------------
static void PowerUpRequestHandler
(
    void*           indBufPtr,  // [IN] The indication message data.
    unsigned int    indBufLen,  // [IN] The indication message data length in bytes.
    void*           contextPtr  // [IN] Context value that was passed to
                                //      swiQmi_AddIndicationHandler().
)
{
    uim_remote_card_power_up_ind_msg_v01 uimrmtPowerUp;

    qmi_client_error_type clientMsgErr = qmi_client_message_decode(
        UimrmtClient, QMI_IDL_INDICATION, QMI_UIM_REMOTE_CARD_POWER_UP_IND_V01,
        indBufPtr, indBufLen,
        &uimrmtPowerUp, sizeof(uimrmtPowerUp));

    if (QMI_NO_ERR != clientMsgErr)
    {
        LE_ERROR("Error in decoding QMI_UIM_REMOTE_CARD_POWER_UP_IND_V01, error = %i",
                 clientMsgErr);
        return;
    }

    LE_DEBUG("QMI_UIM_REMOTE_CARD_POWER_UP_IND_V01 received for SIM slot %d", uimrmtPowerUp.slot);

    if (REMOTE_SIM_CARD_SLOT == uimrmtPowerUp.slot)
    {
        // Note: additional optional parameters are not used:
        // - Requested power up voltage cannot be transmitted through the RSIM service
        // - Response timeout action is not clearly defined (maybe send UIM_REMOTE_CARD_ERROR_V01
        //   with UIM_REMOTE_CARD_ERROR_NO_LINK_ESTABLISHED_V01 cause?)
        pa_rsim_Action_t action = PA_RSIM_POWER_UP;
        le_event_Report(ActionRequestEvent, (void *)&action, sizeof(action));
    }
    else
    {
        LE_ERROR("Unexpected SIM card slot %d used for power up", uimrmtPowerUp.slot);
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * SIM power down request handler function called by the QMI UIMRMT service indications.
 */
//--------------------------------------------------------------------------------------------------
static void PowerDownRequestHandler
(
    void*           indBufPtr,  // [IN] The indication message data.
    unsigned int    indBufLen,  // [IN] The indication message data length in bytes.
    void*           contextPtr  // [IN] Context value that was passed to
                                //      swiQmi_AddIndicationHandler().
)
{
    uim_remote_card_power_down_ind_msg_v01 uimrmtPowerDown;

    qmi_client_error_type clientMsgErr = qmi_client_message_decode(
        UimrmtClient, QMI_IDL_INDICATION, QMI_UIM_REMOTE_CARD_POWER_DOWN_IND_V01,
        indBufPtr, indBufLen,
        &uimrmtPowerDown, sizeof(uimrmtPowerDown));

    if (QMI_NO_ERR != clientMsgErr)
    {
        LE_ERROR("Error in decoding QMI_UIM_REMOTE_CARD_POWER_DOWN_IND_V01, error = %i",
                 clientMsgErr);
        return;
    }

    LE_DEBUG("QMI_UIM_REMOTE_CARD_POWER_DOWN_IND_V01 received for SIM slot %d",
             uimrmtPowerDown.slot);

    if (REMOTE_SIM_CARD_SLOT == uimrmtPowerDown.slot)
    {
        // Note: additional optional parameter Power down mode is not used, as it cannot
        // be transmitted through the RSIM service
        pa_rsim_Action_t action = PA_RSIM_POWER_DOWN;
        le_event_Report(ActionRequestEvent, (void *)&action, sizeof(action));
    }
    else
    {
        LE_ERROR("Unexpected SIM card slot %d used for power down", uimrmtPowerDown.slot);
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * APDU indication handler function called by the QMI UIMRMT service indications.
 */
//--------------------------------------------------------------------------------------------------
static void ApduIndicationHandler
(
    void*           indBufPtr,  // [IN] The indication message data.
    unsigned int    indBufLen,  // [IN] The indication message data length in bytes.
    void*           contextPtr  // [IN] Context value that was passed to
                                //      swiQmi_AddIndicationHandler().
)
{
    uim_remote_apdu_ind_msg_v01 uimrmtApduInd;

    qmi_client_error_type clientMsgErr = qmi_client_message_decode(
        UimrmtClient, QMI_IDL_INDICATION, QMI_UIM_REMOTE_APDU_IND_V01,
        indBufPtr, indBufLen,
        &uimrmtApduInd, sizeof(uimrmtApduInd));

    if (QMI_NO_ERR != clientMsgErr)
    {
        LE_ERROR("Error in decoding QMI_UIM_REMOTE_APDU_IND_V01, error = %i", clientMsgErr);
        return;
    }

    LE_DEBUG("QMI_UIM_REMOTE_APDU_IND_V01 received for SIM slot %d, id %d", uimrmtApduInd.slot,
        uimrmtApduInd.apdu_id);

    if (REMOTE_SIM_CARD_SLOT == uimrmtApduInd.slot)
    {
        if (uimrmtApduInd.command_apdu_len <= PA_RSIM_APDU_MAX_SIZE)
        {
            pa_rsim_ApduInd_t apduInd;
            memset(&apduInd, 0, sizeof(apduInd));

            CurrentApduId = uimrmtApduInd.apdu_id;
            apduInd.apduLength = uimrmtApduInd.command_apdu_len;
            memcpy(apduInd.apduData, uimrmtApduInd.command_apdu, uimrmtApduInd.command_apdu_len);

            le_event_Report(ApduIndicationEvent, &apduInd, sizeof(apduInd));
        }
        else
        {
            LE_ERROR("APDU is too long! Size=%d, MaxSize=%d",
                     uimrmtApduInd.command_apdu_len, PA_RSIM_APDU_MAX_SIZE);
            // Send an APDU response error
            pa_rsim_TransferApduRespError();
        }
    }
    else
    {
        LE_ERROR("Unexpected SIM card slot %d used for APDU indication", uimrmtApduInd.slot);
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * The first-layer SIM action request handler
 */
//--------------------------------------------------------------------------------------------------
static void FirstLayerActionRequestHandler
(
    void* reportPtr,
    void* secondLayerHandlerFunc
)
{
    pa_rsim_Action_t* eventDataPtr = reportPtr;
    pa_rsim_SimActionHdlrFunc_t clientHandlerFunc = secondLayerHandlerFunc;

    clientHandlerFunc(*eventDataPtr);
}


//--------------------------------------------------------------------------------------------------
/**
 * This function checks if the remote SIM card is selected.
 *
 * @return true         If the remote SIM is selected.
 * @return false        It the remote SIM is not selected.
 */
//--------------------------------------------------------------------------------------------------
bool pa_rsim_IsRemoteSimSelected
(
    void
)
{
    dms_swi_get_uim_selection_resp_msg_v01 resp = { 0 };

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(DmsClient,
                                                QMI_DMS_SWI_GET_UIM_SELECTION_REQ_V01,
                                                NULL, 0,
                                                &resp, sizeof(resp),
                                                COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (LE_OK == swiQmi_CheckResponseCode(STRINGIZE_EXPAND(QMI_DMS_SWI_GET_UIM_SELECTION_REQ_V01),
                                    clientMsgErr,
                                    resp.resp))
    {
        LE_DEBUG("uim_select.%d (QMI value)", resp.uim_select);

        /* 0 ?External UIM Interface
         * 1 ?Embedded UIM
         * 2 ?Remote UIM
         */
        if (2 == resp.uim_select)
        {
            // Remote UIM
            return true;
        }
    }

    return false;
}



//--------------------------------------------------------------------------------------------------
// Public functions
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * This function is used to add an APDU indication notification handler
 *
 * @return A handler reference, which is only needed for later removal of the handler.
 */
//--------------------------------------------------------------------------------------------------
le_event_HandlerRef_t pa_rsim_AddApduNotificationHandler
(
    pa_rsim_ApduIndHdlrFunc_t indicationHandler ///< [IN] The handler function to handle an APDU
                                                ///  notification reception.
)
{
    le_event_HandlerRef_t apduIndHandler;
    LE_ASSERT(NULL != indicationHandler);

    apduIndHandler = le_event_AddHandler("PaApduNotificationHandler",
                                         ApduIndicationEvent,
                                         (le_event_HandlerFunc_t) indicationHandler);

    return (le_event_HandlerRef_t) apduIndHandler;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function is called to unregister an APDU indication notification handler.
 */
//--------------------------------------------------------------------------------------------------
void pa_rsim_RemoveApduNotificationHandler
(
    le_event_HandlerRef_t apduIndHandler
)
{
    le_event_RemoveHandler(apduIndHandler);
}

//--------------------------------------------------------------------------------------------------
/**
 * This function is used to add a SIM action request notification handler
 *
 * @return A handler reference, which is only needed for later removal of the handler.
 */
//--------------------------------------------------------------------------------------------------
le_event_HandlerRef_t pa_rsim_AddSimActionRequestHandler
(
    pa_rsim_SimActionHdlrFunc_t actionHandler   ///< [IN] The handler function to handle a SIM
                                                ///  action request notification reception.
)
{
    le_event_HandlerRef_t actionRequestHandler;
    LE_ASSERT(NULL != actionHandler);

    actionRequestHandler = le_event_AddLayeredHandler("PaSimActionRequestHandler",
                                                      ActionRequestEvent,
                                                      FirstLayerActionRequestHandler,
                                                      actionHandler);

    return (le_event_HandlerRef_t) actionRequestHandler;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function is called to unregister a SIM action request notification handler.
 */
//--------------------------------------------------------------------------------------------------
void pa_rsim_RemoveSimActionRequestHandler
(
    le_event_HandlerRef_t actionRequestHandler
)
{
    le_event_RemoveHandler(actionRequestHandler);
}

//--------------------------------------------------------------------------------------------------
/**
 * This function is called to notify the modem of the remote SIM disconnection.
 *
 * @return
 *  - LE_OK            The function succeeded.
 *  - LE_FAULT         The function failed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_rsim_Disconnect
(
    void
)
{
    le_result_t result;
    qmi_client_error_type clientMsgErr;

    uim_remote_event_req_msg_v01 uimrmtMsgReq;
    memset(&uimrmtMsgReq, 0, sizeof(uimrmtMsgReq));
    uim_remote_event_resp_msg_v01 uimrmtMsgRsp;
    memset(&uimrmtMsgRsp, 0, sizeof(uimrmtMsgRsp));

    // Set disconnection indication in event information
    uimrmtMsgReq.event_info.slot = REMOTE_SIM_CARD_SLOT;
    uimrmtMsgReq.event_info.event = UIM_REMOTE_CONNECTION_UNAVAILABLE_V01;

    clientMsgErr = qmi_client_send_msg_sync(UimrmtClient,
                                            QMI_UIM_REMOTE_EVENT_REQ_V01,
                                            &uimrmtMsgReq, sizeof(uimrmtMsgReq),
                                            &uimrmtMsgRsp, sizeof(uimrmtMsgRsp),
                                            COMM_UICC_TIMEOUT);

    result = swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_UIM_REMOTE_EVENT_REQ_V01),
                                  clientMsgErr,
                                  uimrmtMsgRsp.resp.result,
                                  uimrmtMsgRsp.resp.error);

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function is called to notify the modem of a remote SIM status change.
 *
 * @return
 *  - LE_OK             The function succeeded.
 *  - LE_FAULT          The function failed.
 *  - LE_BAD_PARAMETER  Unknown SIM status.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_rsim_NotifyStatus
(
    pa_rsim_SimStatus_t simStatus   ///< [IN] SIM status change
)
{
    le_result_t result;
    qmi_client_error_type clientMsgErr;

    // Check if status is correct
    if (simStatus >= PA_RSIM_STATUS_COUNT)
    {
        LE_ERROR("Unknown SIM status %d reported!", simStatus);
        return LE_BAD_PARAMETER;
    }

    uim_remote_event_req_msg_v01 uimrmtMsgReq;
    memset(&uimrmtMsgReq, 0, sizeof(uimrmtMsgReq));
    uim_remote_event_resp_msg_v01 uimrmtMsgRsp;
    memset(&uimrmtMsgRsp, 0, sizeof(uimrmtMsgRsp));

    // Set status indication in event information
    uimrmtMsgReq.event_info.slot = REMOTE_SIM_CARD_SLOT;
    switch (simStatus)
    {
        case PA_RSIM_STATUS_RESET:
            uimrmtMsgReq.event_info.event = UIM_REMOTE_CARD_RESET_V01;
        break;

        case PA_RSIM_STATUS_REMOVED:
            uimrmtMsgReq.event_info.event = UIM_REMOTE_CARD_REMOVED_V01;
        break;

        case PA_RSIM_STATUS_INSERTED:
            uimrmtMsgReq.event_info.event = UIM_REMOTE_CARD_INSERTED_V01;
        break;

        case PA_RSIM_STATUS_RECOVERED:
        case PA_RSIM_STATUS_AVAILABLE:
            uimrmtMsgReq.event_info.event = UIM_REMOTE_CONNECTION_AVAILABLE_V01;
        break;

        case PA_RSIM_STATUS_NO_LINK:
            uimrmtMsgReq.event_info.event = UIM_REMOTE_CARD_ERROR_V01;
            uimrmtMsgReq.error_cause_valid = 1;
            uimrmtMsgReq.error_cause = UIM_REMOTE_CARD_ERROR_NO_LINK_ESTABLISHED_V01;
        break;

        case PA_RSIM_STATUS_NOT_ACCESSIBLE:
        case PA_RSIM_STATUS_UNKNOWN_ERROR:
        default:
            uimrmtMsgReq.event_info.event = UIM_REMOTE_CARD_ERROR_V01;
            uimrmtMsgReq.error_cause_valid = 1;
            uimrmtMsgReq.error_cause = UIM_REMOTE_CARD_ERROR_UNKNOWN_ERROR_V01;
        break;
    }

    clientMsgErr = qmi_client_send_msg_sync(UimrmtClient,
                                            QMI_UIM_REMOTE_EVENT_REQ_V01,
                                            &uimrmtMsgReq, sizeof(uimrmtMsgReq),
                                            &uimrmtMsgRsp, sizeof(uimrmtMsgRsp),
                                            COMM_UICC_TIMEOUT);

    result = swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_UIM_REMOTE_EVENT_REQ_V01),
                                  clientMsgErr,
                                  uimrmtMsgRsp.resp.result,
                                  uimrmtMsgRsp.resp.error);

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function is called to transfer an APDU response to the modem.
 *
 * @return
 *  - LE_OK             The function succeeded.
 *  - LE_FAULT          The function failed.
 *  - LE_BAD_PARAMETER  APDU too long.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_rsim_TransferApduResp
(
    const uint8_t* apduPtr,     ///< [IN] APDU buffer
    uint16_t       apduLen      ///< [IN] APDU length in bytes
)
{
    le_result_t result;
    qmi_client_error_type clientMsgErr;

    // Check APDU length
    if (apduLen > QMI_UIM_REMOTE_MAX_RESPONSE_APDU_SEGMENT_LEN_V01)
    {
        // Note: the QMI specification requires to split the APDU response into smaller chunks
        // if its length exceeds the maximum response length supported.
        // This is not necessary for the Remote SIM service as the maximal length supported by
        // this service is smaller than the QMI maximum response APDU length.
        LE_ERROR("APDU is too long! Size=%d, MaxSize=%d",
                 apduLen, QMI_UIM_REMOTE_MAX_RESPONSE_APDU_SEGMENT_LEN_V01);
        return LE_BAD_PARAMETER;
    }
    LE_DEBUG("Transfert _APDU_ id %d, pdulen %d", CurrentApduId, apduLen);
    uim_remote_apdu_req_msg_v01 uimrmtMsgReq;
    memset(&uimrmtMsgReq, 0, sizeof(uimrmtMsgReq));
    uim_remote_apdu_resp_msg_v01 uimrmtMsgRsp;
    memset(&uimrmtMsgRsp, 0, sizeof(uimrmtMsgRsp));

    // Set APDU information
    uimrmtMsgReq.apdu_status = QMI_RESULT_SUCCESS;
    uimrmtMsgReq.slot = REMOTE_SIM_CARD_SLOT;
    uimrmtMsgReq.apdu_id = CurrentApduId;
    uimrmtMsgReq.response_apdu_info_valid = 1;
    uimrmtMsgReq.response_apdu_info.total_response_apdu_size = apduLen;
    uimrmtMsgReq.response_apdu_info.response_apdu_segment_offset = 0;
    uimrmtMsgReq.response_apdu_segment_valid = 1;
    uimrmtMsgReq.response_apdu_segment_len = apduLen;
    memcpy(uimrmtMsgReq.response_apdu_segment, apduPtr, apduLen);

    // Reset current APDU id
    CurrentApduId = DEFAULT_APDU_ID;

    clientMsgErr = qmi_client_send_msg_sync(UimrmtClient,
                                            QMI_UIM_REMOTE_APDU_REQ_V01,
                                            &uimrmtMsgReq, sizeof(uimrmtMsgReq),
                                            &uimrmtMsgRsp, sizeof(uimrmtMsgRsp),
                                            COMM_UICC_TIMEOUT);

    result = swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_UIM_REMOTE_APDU_REQ_V01),
                                  clientMsgErr,
                                  uimrmtMsgRsp.resp.result,
                                  uimrmtMsgRsp.resp.error);

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function is called to indicate an APDU response error to the modem.
 *
 * @return
 *  - LE_OK             The function succeeded.
 *  - LE_FAULT          The function failed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_rsim_TransferApduRespError
(
    void
)
{
    le_result_t result;
    qmi_client_error_type clientMsgErr;

    uim_remote_apdu_req_msg_v01 uimrmtMsgReq;
    memset(&uimrmtMsgReq, 0, sizeof(uimrmtMsgReq));
    uim_remote_apdu_resp_msg_v01 uimrmtMsgRsp;
    memset(&uimrmtMsgRsp, 0, sizeof(uimrmtMsgRsp));

    // Set APDU information
    uimrmtMsgReq.apdu_status = QMI_RESULT_FAILURE;
    uimrmtMsgReq.slot = REMOTE_SIM_CARD_SLOT;
    uimrmtMsgReq.apdu_id = CurrentApduId;

    // Reset current APDU id
    CurrentApduId = DEFAULT_APDU_ID;

    clientMsgErr = qmi_client_send_msg_sync(UimrmtClient,
                                            QMI_UIM_REMOTE_APDU_REQ_V01,
                                            &uimrmtMsgReq, sizeof(uimrmtMsgReq),
                                            &uimrmtMsgRsp, sizeof(uimrmtMsgRsp),
                                            COMM_UICC_TIMEOUT);

    result = swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_UIM_REMOTE_APDU_REQ_V01),
                                  clientMsgErr,
                                  uimrmtMsgRsp.resp.result,
                                  uimrmtMsgRsp.resp.error);

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function is called to transfer an Answer to Reset (ATR) response to the modem.
 *
 * @return
 *  - LE_OK             The function succeeded.
 *  - LE_FAULT          The function failed.
 *  - LE_BAD_PARAMETER  ATR too long.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_rsim_TransferAtrResp
(
    pa_rsim_SimStatus_t simStatus,  ///< [IN] SIM status change
    const uint8_t* atrPtr,          ///< [IN] ATR buffer
    uint16_t       atrLen           ///< [IN] ATR length in bytes
)
{
    le_result_t result;
    qmi_client_error_type clientMsgErr;

    // Check ATR length
    if (atrLen > QMI_UIM_REMOTE_MAX_ATR_LEN_V01)
    {
        LE_ERROR("ATR is too long! Size=%d, MaxSize=%d", atrLen, QMI_UIM_REMOTE_MAX_ATR_LEN_V01);
        return LE_BAD_PARAMETER;
    }

    uim_remote_event_req_msg_v01 uimrmtMsgReq;
    memset(&uimrmtMsgReq, 0, sizeof(uimrmtMsgReq));
    uim_remote_event_resp_msg_v01 uimrmtMsgRsp;
    memset(&uimrmtMsgRsp, 0, sizeof(uimrmtMsgRsp));

    // Set reset indication in event information
    uimrmtMsgReq.event_info.slot = REMOTE_SIM_CARD_SLOT;
    switch (simStatus)
    {
        case PA_RSIM_STATUS_RESET:
            uimrmtMsgReq.event_info.event = UIM_REMOTE_CARD_RESET_V01;
        break;

        case PA_RSIM_STATUS_INSERTED:
            uimrmtMsgReq.event_info.event = UIM_REMOTE_CARD_INSERTED_V01;
        break;

        default:
            LE_ERROR("Unsupported SIM status change %d", simStatus);
            return LE_FAULT;
    }
    uimrmtMsgReq.atr_valid = 1;
    uimrmtMsgReq.atr_len = atrLen;
    memcpy(uimrmtMsgReq.atr, atrPtr, atrLen);

    clientMsgErr = qmi_client_send_msg_sync(UimrmtClient,
                                            QMI_UIM_REMOTE_EVENT_REQ_V01,
                                            &uimrmtMsgReq, sizeof(uimrmtMsgReq),
                                            &uimrmtMsgRsp, sizeof(uimrmtMsgRsp),
                                            COMM_UICC_TIMEOUT);

    result = swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_UIM_REMOTE_EVENT_REQ_V01),
                                  clientMsgErr,
                                  uimrmtMsgRsp.resp.result,
                                  uimrmtMsgRsp.resp.error);

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function indicates if the Remote SIM service is supported by the PA.
 *
 * @return
 *  - true      Remote SIM service is supported.
 *  - false     Remote SIM service is not supported.
 */
//--------------------------------------------------------------------------------------------------
bool pa_rsim_IsRsimSupported
(
    void
)
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to initialize the QMI Remote SIM service module.
 *
 * @return
 *  - LE_OK             The function succeeded.
 *  - LE_FAULT          The function failed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_rsim_Init
(
    void
)
{
    // Create the events for signaling user handlers.
    ActionRequestEvent = le_event_CreateId("ActionRequestEvent",
                                           sizeof(pa_rsim_Action_t));
    ApduIndicationEvent = le_event_CreateId("ApduIndicationEvent",
                                            sizeof(pa_rsim_ApduInd_t));

    if (LE_OK != swiQmi_InitServices(QMI_SERVICE_DMS))
    {
        LE_CRIT("QMI_SERVICE_DMS cannot be initialized.");
        return LE_FAULT;
    }

    // Get the QMI client service handle for later use.
    DmsClient = swiQmi_GetClientHandle(QMI_SERVICE_DMS);
    if ( NULL == DmsClient)
    {
        LE_ERROR("QMI_SERVICE_DMS cannot be initialized.");
        return LE_FAULT;
    }

    // Get the selected UIM interface.
    // Do not initialize QMI UIM Remote services if UIM isn't set to remote.
    if (true != pa_rsim_IsRemoteSimSelected())
    {
        LE_WARN("Remote UIM is not selected");
        return LE_FAULT;
    }

    // Initialize QMI UIM Remote services
    if (LE_OK != swiQmi_InitServices(QMI_SERVICE_UIMRMT))
    {
        LE_CRIT("QMI_SERVICE_UIMRMT cannot be initialized.");
        return LE_FAULT;
    }

    // Get the QMI client service handle for later use.
    UimrmtClient = swiQmi_GetClientHandle(QMI_SERVICE_UIMRMT);
    if (NULL == UimrmtClient)
    {
        LE_ERROR("Unable to get client handle for QMI_SERVICE_UIMRMT");
        return LE_FAULT;
    }

    // Register our own handlers with the QMI UIMRMT service.
    swiQmi_AddIndicationHandler(ConnectionRequestHandler,
                                QMI_SERVICE_UIMRMT,
                                QMI_UIM_REMOTE_CONNECT_IND_V01, NULL);
    swiQmi_AddIndicationHandler(DisconnectionRequestHandler,
                                QMI_SERVICE_UIMRMT,
                                QMI_UIM_REMOTE_DISCONNECT_IND_V01, NULL);
    swiQmi_AddIndicationHandler(ResetRequestHandler,
                                QMI_SERVICE_UIMRMT,
                                QMI_UIM_REMOTE_CARD_RESET_IND_V01, NULL);
    swiQmi_AddIndicationHandler(PowerUpRequestHandler,
                                QMI_SERVICE_UIMRMT,
                                QMI_UIM_REMOTE_CARD_POWER_UP_IND_V01, NULL);
    swiQmi_AddIndicationHandler(PowerDownRequestHandler,
                                QMI_SERVICE_UIMRMT,
                                QMI_UIM_REMOTE_CARD_POWER_DOWN_IND_V01, NULL);
    swiQmi_AddIndicationHandler(ApduIndicationHandler,
                                QMI_SERVICE_UIMRMT,
                                QMI_UIM_REMOTE_APDU_IND_V01, NULL);

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Component initializer automatically called by the application framework when the process starts.
 *
 **/
//--------------------------------------------------------------------------------------------------
COMPONENT_INIT
{
    LE_INFO("Start the rSim QMI PA initialization.");

    LE_WARN_IF((pa_rsim_Init() != LE_OK), "Error on PA RSIM initialization.");

    LE_INFO("QMI PA rSim initialization ends.");
}
