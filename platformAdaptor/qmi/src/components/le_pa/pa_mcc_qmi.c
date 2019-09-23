/**
 * @file pa_mcc_qmi.c
 *
 * QMI implementation of pa_mcc API.
 *
 * @todo This implementation is not thread-safe.  Should it be?
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#include <pa_mcc.h>
#include <pa_mrc.h>
#include "mcc_qmi.h"
#include "swiQmi.h"
#include "legato.h"
#include "le_print.h"
#include <time.h>
#include <semaphore.h>

//--------------------------------------------------------------------------------------------------
/**
 * Semaphore used to make the API (pa_mcc_Answer, pa_mcc_HangUp) synchronous
 * Cannot use legato semaphore because CallHandler is not a Legato thread... (due to QMI)
 */
//--------------------------------------------------------------------------------------------------
static bool IsAnswer=false;
static bool IsHangUp=false;
static sem_t CallSynchronization;

//--------------------------------------------------------------------------------------------------
/**
 * The Voice client.  Must be obtained by calling swiQmi_GetClientHandle() before it can be used.
 */
//--------------------------------------------------------------------------------------------------
static qmi_client_type VoiceClient;


//--------------------------------------------------------------------------------------------------
/**
 * The DMS (Device Management Services) client.  Must be obtained by calling
 * swiQmi_GetClientHandle() before it can be used.
 */
//--------------------------------------------------------------------------------------------------
static qmi_client_type DmsClient;

//--------------------------------------------------------------------------------------------------
/**
 * Call event ID used to report call events to the registered event handlers.
 */
//--------------------------------------------------------------------------------------------------
static le_event_Id_t CallEvent;


//--------------------------------------------------------------------------------------------------
/**
 * Memory pool for call event data.
 */
//--------------------------------------------------------------------------------------------------
static le_mem_PoolRef_t CallEventDataPool;


//--------------------------------------------------------------------------------------------------
/**
 * The user's event handler reference.
 */
//--------------------------------------------------------------------------------------------------
static le_event_HandlerRef_t CallEventHandlerRef = NULL;

//--------------------------------------------------------------------------------------------------
/**
 * Hangup is requested by pa_ecall.
 *
 */
//--------------------------------------------------------------------------------------------------
static bool EcallRequestedHangup = false;

//--------------------------------------------------------------------------------------------------
/**
 * Maps the Qmi call state to a call event.
 *
 * @return
 *      A call event on success.
 *      -1 if the event does not map to a supported event.
 */
//--------------------------------------------------------------------------------------------------
static le_mcc_Event_t MapQmiCallStateToCallEvent
(
    call_state_enum_v02 qmiCallState
)
{
    switch (qmiCallState)
    {
        case CALL_STATE_SETUP_V02:
            return LE_MCC_EVENT_SETUP;

        case CALL_STATE_INCOMING_V02:
            return LE_MCC_EVENT_INCOMING;

        case CALL_STATE_ORIGINATING_V02:
            return LE_MCC_EVENT_ORIGINATING;

        case CALL_STATE_CONVERSATION_V02:
            return LE_MCC_EVENT_CONNECTED;

        case CALL_STATE_WAITING_V02:
            return LE_MCC_EVENT_WAITING;

        case CALL_STATE_ALERTING_V02:
            return LE_MCC_EVENT_ALERTING;

        case CALL_STATE_HOLD_V02:
            return LE_MCC_EVENT_ON_HOLD;

        case CALL_STATE_END_V02:
            return LE_MCC_EVENT_TERMINATED;

        default:
            LE_WARN("QMI call state discarded: 0x%02x", qmiCallState);
            return -1;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Return strings associated with the given QMI call state for debug purposes.
 */
//--------------------------------------------------------------------------------------------------
static const char * QmiCallStateToString
(
    call_state_enum_v02 qmiCallState
)
{
    switch(qmiCallState)
    {
        case CALL_STATE_ORIGINATING_V02:
            return "ORIGINATING";

        case CALL_STATE_INCOMING_V02:
            return "INCOMING";

        case CALL_STATE_CONVERSATION_V02:
            return "CONVERSATION";

        case CALL_STATE_CC_IN_PROGRESS_V02:
            return "CC IN PROGRESS";

        case CALL_STATE_ALERTING_V02:
            return "ALERTING";

        case CALL_STATE_HOLD_V02:
            return "HOLD";

        case CALL_STATE_WAITING_V02:
            return "WAITING";

        case CALL_STATE_DISCONNECTING_V02:
            return "DISCONNECTING";

        case CALL_STATE_END_V02:
            return "END";

        case CALL_STATE_SETUP_V02:
            return "SETUP";

        default:
            break;
    }

    return NULL;
}

//--------------------------------------------------------------------------------------------------
/**
 * Manage voice call.
 *
 *  @return
 *      LE_OK
 *      LE_FAULT
 */
//--------------------------------------------------------------------------------------------------
static le_result_t ManageCall
(
    uint8_t  callId,
    sups_type_enum_v02 callType
)
{
    le_result_t res;

#if defined(SIERRA_MDM9X40) || defined(SIERRA_MDM9X28)
    le_mrc_Rat_t rat;

    res = pa_mrc_GetRadioAccessTechInUse(&rat);
    if (res != LE_OK)
    {
        LE_ERROR("Can't get techno in use");
        return LE_FAULT;
    }

    if ((rat == LE_MRC_RAT_LTE) || (rat == LE_MRC_RAT_CDMA))
    {
        voice_end_call_req_msg_v02 endReq = {0};
        voice_end_call_resp_msg_v02 endResp = { {0} };

        endReq.call_id = callId;

        qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(VoiceClient,
            QMI_VOICE_END_CALL_REQ_V02,
            &endReq, sizeof(endReq),
            &endResp, sizeof(endResp),
            COMM_DEFAULT_PLATFORM_TIMEOUT);

        res = swiQmi_CheckResponseCode(STRINGIZE_EXPAND(QMI_VOICE_END_CALL_REQ_V02),
            clientMsgErr,
            endResp.resp);
    }
    else
#endif
    {
        voice_manage_calls_req_msg_v02 voiceManageCallsReq;
        voice_manage_calls_resp_msg_v02 voiceManageCallsResp;

        memset(&voiceManageCallsReq,0,sizeof(voice_manage_calls_req_msg_v02));
        memset(&voiceManageCallsResp,0,sizeof(voice_manage_calls_resp_msg_v02));

        voiceManageCallsReq.sups_type = callType;

        if (callId > 0)
        {
            voiceManageCallsReq.call_id_valid = true;
            voiceManageCallsReq.call_id = callId;
        }

        // QMI_VOICE_MANAGE_CALLS_REQ_V02 request is applicable only for 3GPP
        qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(
            VoiceClient,
            QMI_VOICE_MANAGE_CALLS_REQ_V02,
            &voiceManageCallsReq, sizeof(voice_manage_calls_req_msg_v02),
            &voiceManageCallsResp, sizeof(voice_manage_calls_resp_msg_v02),
            COMM_HANGUP_VOICE_CALL_TIMEOUT);

        res = swiQmi_CheckResponseCode(STRINGIZE_EXPAND(QMI_VOICE_MANAGE_CALLS_REQ_V02),
            clientMsgErr,
            voiceManageCallsResp.resp);
    }

    if (res != LE_OK)
    {
        return LE_FAULT;
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Copies the remote number from the allCallIndPtr message for a specific call (specified by callID)
 * to bufPtr.  If the remote number is not available then nothing is copied to bufPtr.
 */
//--------------------------------------------------------------------------------------------------
static void GetRemoteNumber
(
    voice_all_call_status_ind_msg_v02* allCallIndPtr,   // The call indication message that contains
                                                        // all call status.
    uint8_t callID,     // The unique call id for the call we are interested in.
    char* bufPtr,       // The buffer to copy the remote number too.
    size_t bufSize      // The size of the buffer.
)
{
    // Get the remote number if available.
    if (allCallIndPtr->remote_party_number_valid)
    {
        // Find the call remote number by call id.
        uint32_t i = 0;
        for (; i < allCallIndPtr->remote_party_number_len; i++)
        {
            LE_DEBUG("Remote Number (%d) Id %d => %s",
                i,
                allCallIndPtr->remote_party_number[i].call_id,
                allCallIndPtr->remote_party_number[i].number );

            if (allCallIndPtr->remote_party_number[i].call_id == callID)
            {
                // Calculate the number of bytes to copy.  The remote number in the indication
                // message is in ascii but is not NULL-terminated.
                size_t numBytesToCopy = allCallIndPtr->remote_party_number[i].number_len + 1;

                if (numBytesToCopy > bufSize)
                {
                    numBytesToCopy = bufSize;
                }

                // Don't need to check the return value because an overflow may be intentional.
                le_utf8_Copy(bufPtr, allCallIndPtr->remote_party_number[i].number,
                             numBytesToCopy, NULL);

                return;
            }
        }
    }
}


//--------------------------------------------------------------------------------------------------
/**
 * Gets the call termination reason from the call indication message for a specified call id.
 *
 * @return
 *      The call termination reason.
 */
//--------------------------------------------------------------------------------------------------
static void GetTerminationReason
(
    voice_all_call_status_ind_msg_v02* allCallIndPtr,   ///< [IN] The call indication message that contains
                                                        ///< that contains all call status.
    uint8_t callID,      ///< [IN] The unique call id for the call we are interested in.

    le_mcc_TerminationReason_t*      terminationReasonPtr, ///< [IN] the termination reason
    int32_t*                         terminationCodePtr ///< [IN] the corresponding termination code
)
{
    if (allCallIndPtr->call_end_reason_valid)
    {
        uint32_t i = 0;
        for (; i < allCallIndPtr->call_end_reason_len; i++)
        {
            if (allCallIndPtr->call_end_reason[i].call_id == callID)
            {
                LE_DEBUG("QMI termination reason: 0x%02x",
                         allCallIndPtr->call_end_reason[i].call_end_reason);

                *terminationCodePtr = (int32_t)(allCallIndPtr->call_end_reason[i].call_end_reason);

                switch(allCallIndPtr->call_end_reason[i].call_end_reason)
                {
                    case CALL_END_CAUSE_NO_RESOURCES_V02:
                    case CALL_END_CAUSE_RADIO_LINK_LOST_V02:
                    case CALL_END_CAUSE_ACCESS_STRATUM_REJ_LOW_LEVEL_FAIL_REDIAL_NOT_ALLOWED_V02:
                        *terminationReasonPtr = LE_MCC_TERM_NETWORK_FAIL;
                        break;

                    case CALL_END_CAUSE_UNASSIGNED_NUMBER_V02:
                        *terminationReasonPtr = LE_MCC_TERM_UNASSIGNED_NUMBER;
                        break;

                    case CALL_END_CAUSE_NO_ROUTE_TO_DESTINATION_V02:
                        *terminationReasonPtr = LE_MCC_TERM_NO_ROUTE_TO_DESTINATION;
                        break;

                    case CALL_END_CAUSE_CHANNEL_UNACCEPTABLE_V02:
                        *terminationReasonPtr = LE_MCC_TERM_CHANNEL_UNACCEPTABLE;
                        break;

                    case CALL_END_CAUSE_OPERATOR_DETERMINED_BARRING_V02:
                        *terminationReasonPtr = LE_MCC_TERM_OPERATOR_DETERMINED_BARRING;
                        break;

                    case CALL_END_CAUSE_CLIENT_END_V02:
                    case CALL_END_CAUSE_REL_NORMAL_V02:
                    case CALL_END_CAUSE_NORMAL_CALL_CLEARING_V02:
                        LE_DEBUG("IsHangUp %d, EcallRequestedHangup %d", IsHangUp,
                                 EcallRequestedHangup);
                        if((IsHangUp) || (EcallRequestedHangup))
                        {
                            *terminationReasonPtr = LE_MCC_TERM_LOCAL_ENDED;
                            EcallRequestedHangup = false;
                        }
                        else
                        {
                            *terminationReasonPtr = LE_MCC_TERM_REMOTE_ENDED;
                        }
                        break;

                    case CALL_END_CAUSE_USER_BUSY_V02:
                        *terminationReasonPtr = LE_MCC_TERM_USER_BUSY;
                        break;

                    case CALL_END_CAUSE_NO_USER_RESPONDING_V02:
                        *terminationReasonPtr = LE_MCC_TERM_NO_USER_RESPONDING;
                        break;

                    case CALL_END_CAUSE_USER_ALERTING_NO_ANSWER_V02:
                        *terminationReasonPtr = LE_MCC_TERM_USER_ALERTING_NO_ANSWER;
                        break;

                    case CALL_END_CAUSE_CALL_REJECTED_V02:
                        *terminationReasonPtr = LE_MCC_TERM_CALL_REJECTED;
                        break;

                    case CALL_END_CAUSE_NUMBER_CHANGED_V02:
                        *terminationReasonPtr = LE_MCC_TERM_NUMBER_CHANGED;
                        break;

                    case CALL_END_CAUSE_PREEMPTION_V02:
                        *terminationReasonPtr = LE_MCC_TERM_PREEMPTION;
                        break;

                    case CALL_END_CAUSE_DESTINATION_OUT_OF_ORDER_V02:
                        *terminationReasonPtr = LE_MCC_TERM_DESTINATION_OUT_OF_ORDER;
                        break;

                    case CALL_END_CAUSE_INVALID_NUMBER_FORMAT_V02:
                        *terminationReasonPtr = LE_MCC_TERM_INVALID_NUMBER_FORMAT;
                        break;

                    case CALL_END_CAUSE_FACILITY_REJECTED_V02:
                        *terminationReasonPtr = LE_MCC_TERM_FACILITY_REJECTED;
                        break;

                    case CALL_END_CAUSE_RESP_TO_STATUS_ENQUIRY_V02:
                        *terminationReasonPtr = LE_MCC_TERM_RESP_TO_STATUS_ENQUIRY;
                        break;

                    case CALL_END_CAUSE_NORMAL_UNSPECIFIED_V02:
                        *terminationReasonPtr = LE_MCC_TERM_NORMAL_UNSPECIFIED;
                        break;

                    case CALL_END_CAUSE_NO_CIRCUIT_OR_CHANNEL_AVAILABLE_V02:
                        *terminationReasonPtr = LE_MCC_TERM_NO_CIRCUIT_OR_CHANNEL_AVAILABLE;
                        break;

                    case CALL_END_CAUSE_NETWORK_OUT_OF_ORDER_V02:
                        *terminationReasonPtr = LE_MCC_TERM_NETWORK_OUT_OF_ORDER;
                        break;

                    case CALL_END_CAUSE_TEMPORARY_FAILURE_V02:
                        *terminationReasonPtr = LE_MCC_TERM_TEMPORARY_FAILURE;
                        break;

                    case CALL_END_CAUSE_SWITCHING_EQUIPMENT_CONGESTION_V02:
                        *terminationReasonPtr = LE_MCC_TERM_SWITCHING_EQUIPMENT_CONGESTION;
                        break;

                    case CALL_END_CAUSE_ACCESS_INFORMATION_DISCARDED_V02:
                        *terminationReasonPtr = LE_MCC_TERM_ACCESS_INFORMATION_DISCARDED;
                        break;

                    case CALL_END_CAUSE_REQUESTED_CIRCUIT_OR_CHANNEL_NOT_AVAILABLE_V02:
                        *terminationReasonPtr = LE_MCC_TERM_REQUESTED_CIRCUIT_OR_CHANNEL_NOT_AVAILABLE;
                        break;

                    case CALL_END_CAUSE_RESOURCES_UNAVAILABLE_OR_UNSPECIFIED_V02:
                        *terminationReasonPtr = LE_MCC_TERM_RESOURCES_UNAVAILABLE_OR_UNSPECIFIED;
                        break;

                    case CALL_END_CAUSE_QOS_UNAVAILABLE_V02:
                        *terminationReasonPtr = LE_MCC_TERM_QOS_UNAVAILABLE;
                        break;

                    case CALL_END_CAUSE_REQUESTED_FACILITY_NOT_SUBSCRIBED_V02:
                        *terminationReasonPtr = LE_MCC_TERM_REQUESTED_FACILITY_NOT_SUBSCRIBED;
                        break;

                    case CALL_END_CAUSE_INCOMING_CALLS_BARRED_WITHIN_CUG_V02:
                        *terminationReasonPtr = LE_MCC_TERM_INCOMING_CALLS_BARRED_WITHIN_CUG;
                        break;

                    case CALL_END_CAUSE_BEARER_CAPABILITY_NOT_AUTH_V02:
                        *terminationReasonPtr = LE_MCC_TERM_BEARER_CAPABILITY_NOT_AUTH;
                        break;

                    case CALL_END_CAUSE_BEARER_CAPABILITY_UNAVAILABLE_V02:
                        *terminationReasonPtr = LE_MCC_TERM_BEARER_CAPABILITY_UNAVAILABLE;
                        break;

                    case CALL_END_CAUSE_SERVICE_OPTION_NOT_AVAILABLE_V02:
                        *terminationReasonPtr = LE_MCC_TERM_SERVICE_OPTION_NOT_AVAILABLE;
                        break;

                    case CALL_END_CAUSE_ACM_LIMIT_EXCEEDED_V02:
                        *terminationReasonPtr = LE_MCC_TERM_ACM_LIMIT_EXCEEDED;
                        break;

                    case CALL_END_CAUSE_BEARER_SERVICE_NOT_IMPLEMENTED_V02:
                        *terminationReasonPtr = LE_MCC_TERM_BEARER_SERVICE_NOT_IMPLEMENTED;
                        break;

                    case CALL_END_CAUSE_REQUESTED_FACILITY_NOT_IMPLEMENTED_V02:
                        *terminationReasonPtr = LE_MCC_TERM_REQUESTED_FACILITY_NOT_IMPLEMENTED;
                        break;

                    case CALL_END_CAUSE_ONLY_DIGITAL_INFORMATION_BEARER_AVAILABLE_V02:
                        *terminationReasonPtr = LE_MCC_TERM_ONLY_DIGITAL_INFORMATION_BEARER_AVAILABLE;
                        break;

                    case CALL_END_CAUSE_SERVICE_OR_OPTION_NOT_IMPLEMENTED_V02:
                        *terminationReasonPtr = LE_MCC_TERM_SERVICE_OR_OPTION_NOT_IMPLEMENTED;
                        break;

                    case CALL_END_CAUSE_INVALID_TRANSACTION_IDENTIFIER_V02:
                        *terminationReasonPtr = LE_MCC_TERM_INVALID_TRANSACTION_IDENTIFIER;
                        break;

                    case CALL_END_CAUSE_USER_NOT_MEMBER_OF_CUG_V02:
                        *terminationReasonPtr = LE_MCC_TERM_USER_NOT_MEMBER_OF_CUG;
                        break;

                    case CALL_END_CAUSE_INCOMPATIBLE_DESTINATION_V02:
                        *terminationReasonPtr = LE_MCC_TERM_INCOMPATIBLE_DESTINATION;
                        break;

                    case CALL_END_CAUSE_INVALID_TRANSIT_NW_SELECTION_V02:
                        *terminationReasonPtr = LE_MCC_TERM_INVALID_TRANSIT_NW_SELECTION;
                        break;

                    case CALL_END_CAUSE_SEMANTICALLY_INCORRECT_MESSAGE_V02:
                        *terminationReasonPtr = LE_MCC_TERM_SEMANTICALLY_INCORRECT_MESSAGE;
                        break;

                    case CALL_END_CAUSE_INVALID_MANDATORY_INFORMATION_V02:
                        *terminationReasonPtr = LE_MCC_TERM_INVALID_MANDATORY_INFORMATION;
                        break;

                    case CALL_END_CAUSE_MESSAGE_TYPE_NON_IMPLEMENTED_V02:
                        *terminationReasonPtr = LE_MCC_TERM_MESSAGE_TYPE_NON_IMPLEMENTED;
                        break;

                    case CALL_END_CAUSE_MESSAGE_TYPE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE_V02:
                        *terminationReasonPtr = LE_MCC_TERM_MESSAGE_TYPE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE;
                        break;

                    case CALL_END_CAUSE_INFORMATION_ELEMENT_NON_EXISTENT_V02:
                        *terminationReasonPtr = LE_MCC_TERM_INFORMATION_ELEMENT_NON_EXISTENT;
                        break;

                    case CALL_END_CAUSE_CONDITONAL_IE_ERROR_V02:
                        *terminationReasonPtr = LE_MCC_TERM_CONDITONAL_IE_ERROR;
                        break;

                    case CALL_END_CAUSE_MESSAGE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE_V02:
                        *terminationReasonPtr = LE_MCC_TERM_MESSAGE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE;
                        break;

                    case CALL_END_CAUSE_RECOVERY_ON_TIMER_EXPIRED_V02:
                        *terminationReasonPtr = LE_MCC_TERM_RECOVERY_ON_TIMER_EXPIRY;
                        break;

                    case CALL_END_CAUSE_PROTOCOL_ERROR_UNSPECIFIED_V02:
                        *terminationReasonPtr = LE_MCC_TERM_PROTOCOL_ERROR_UNSPECIFIED;
                        break;

                    case CALL_END_CAUSE_INTERWORKING_UNSPECIFIED_V02:
                        *terminationReasonPtr = LE_MCC_TERM_INTERWORKING_UNSPECIFIED;
                        break;

                    case CALL_END_CAUSE_SERVICE_TEMPORARILY_OUT_OF_ORDER_V02:
                        *terminationReasonPtr = LE_MCC_TERM_SERVICE_TEMPORARILY_OUT_OF_ORDER;
                        break;

                    case CALL_END_CAUSE_NO_SRV_V02:
                    case CALL_END_CAUSE_ACCESS_STRATUM_REJ_RR_RANDOM_ACCESS_FAILURE_V02:
                    case CALL_END_CAUSE_WRONG_STATE_V02:
                        *terminationReasonPtr = LE_MCC_TERM_NO_SERVICE;
                        break;

                    default:
                        LE_INFO("Specific QMI termination reason: 0x%02x",
                                allCallIndPtr->call_end_reason[i].call_end_reason);
                        *terminationReasonPtr = LE_MCC_TERM_PLATFORM_SPECIFIC;
                        break;
                }
                break;
            }
        }
    }
    else
    {
        *terminationReasonPtr =  LE_MCC_TERM_UNDEFINED;
        *terminationCodePtr = -1;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Handler function called when there is an CDMA Call Status indication.  This handler reports a
 * call event to the user when there is a valid call event.
 */
//--------------------------------------------------------------------------------------------------
static void CallCdmaHandler
(
    void*           indBufPtr,  // [IN] The indication message data.
    unsigned int    indBufLen,  // [IN] The indication message data length in bytes.
    void*           contextPtr  // [IN] Context value that was passed to
                                //      swiQmi_AddIndicationHandler().
)
{
    if (indBufPtr == NULL)
    {
        LE_ERROR("Indication message empty.");
        return;
    }

    voice_info_rec_ind_msg_v02 infoCallInd;

    qmi_client_error_type clientMsgErr = qmi_client_message_decode(
        VoiceClient, QMI_IDL_INDICATION, QMI_VOICE_INFO_REC_IND_V02,
        indBufPtr, indBufLen,
        &infoCallInd, sizeof(infoCallInd));

    if (clientMsgErr != QMI_NO_ERR)
    {
        LE_ERROR("Error in decoding QMI_VOICE_INFO_REC_IND_V02, error = %i", clientMsgErr);
        return;
    }

    LE_DEBUG("CallCdmaHandler Event: call_id (%d) name %c IdInfo %c NumInfo %c",
        infoCallInd.call_id,
    (infoCallInd.caller_name_valid ? 'Y' : 'N'),
    (infoCallInd.caller_id_info_valid ? 'Y' : 'N'),
    (infoCallInd.conn_num_info_valid? 'Y' : 'N'));

    if (infoCallInd.caller_id_info_valid)
    {
        LE_DEBUG("Call ID Len (%d) %s",
            infoCallInd.caller_id_info.caller_id_len,
            infoCallInd.caller_id_info.caller_id);
        pa_mcc_CallEventData_t* eventDataPtr = le_mem_ForceAlloc(CallEventDataPool);
        eventDataPtr->callId = infoCallInd.call_id;
        eventDataPtr->event = LE_MCC_EVENT_INCOMING;
        eventDataPtr->terminationEvent = LE_MCC_TERM_UNDEFINED;
        eventDataPtr->terminationCode = -1;
        le_utf8_Copy(eventDataPtr->phoneNumber, infoCallInd.caller_id_info.caller_id,
            sizeof(eventDataPtr->phoneNumber), NULL);
        le_event_ReportWithRefCounting(CallEvent, eventDataPtr);
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Handler function called when there is an All Call Status indication.  This handler reports a
 * call event to the user when there is a valid call event.
 */
//--------------------------------------------------------------------------------------------------
static void CallHandler
(
    void*           indBufPtr,  // [IN] The indication message data.
    unsigned int    indBufLen,  // [IN] The indication message data length in bytes.
    void*           contextPtr  // [IN] Context value that was passed to
                                //      swiQmi_AddIndicationHandler().
)
{

    if (indBufPtr == NULL)
    {
        LE_ERROR("Indication message empty.");
        return;
    }

    voice_all_call_status_ind_msg_v02 allCallInd;

    qmi_client_error_type clientMsgErr = qmi_client_message_decode(
        VoiceClient, QMI_IDL_INDICATION, QMI_VOICE_ALL_CALL_STATUS_IND_V02,
        indBufPtr, indBufLen,
        &allCallInd, sizeof(allCallInd));

    if (clientMsgErr != QMI_NO_ERR)
    {
        LE_ERROR("Error in decoding QMI_VOICE_ALL_CALL_STATUS_IND_V02, error = %i", clientMsgErr);
        return;
    }

    LE_DEBUG("Call Event: call_info_len %d", allCallInd.call_info_len);

    if (allCallInd.call_info_len > 0)
    {
        // This indication is for all calls but we are interested only in voice calls for specific
        // events.  Find the first voice call that has an event we are interested in.
        le_mcc_Event_t callEvent = -1;

        uint32_t i = 0;
        for (; i < allCallInd.call_info_len; i++)
        {
            voice_call_info2_type_v02 * callInfoPtr = &allCallInd.call_info[i];

            switch(allCallInd.call_info[i].call_type)
            {
                case CALL_TYPE_VOICE_V02:
                case CALL_TYPE_VOICE_IP_V02:
                case CALL_TYPE_EMERGENCY_IP_V02:
                case CALL_TYPE_EMERGENCY_V02:
                {
                    LE_INFO("QMI ID.%u State.%s (0x%02X) Type.0x%02X Dir.0x%02X"
                                    " Mode.0x%02X Mparty.%u ALS.%u",
                                    callInfoPtr->call_id,
                                    QmiCallStateToString(callInfoPtr->call_state),
                                    callInfoPtr->call_state,
                                    callInfoPtr->call_type,
                                    callInfoPtr->direction,
                                    callInfoPtr->mode,
                                    callInfoPtr->is_mpty,
                                    callInfoPtr->als);

                    callEvent = MapQmiCallStateToCallEvent(callInfoPtr->call_state);

                    LE_DEBUG("callEvent %d", callEvent);

                    if(callEvent == -1)
                    {
                        break;
                    }

                    // Build the data for the user's event handler.
                    pa_mcc_CallEventData_t* eventDataPtr = le_mem_ForceAlloc(CallEventDataPool);
                    eventDataPtr->callId = callInfoPtr->call_id;
                    eventDataPtr->event = callEvent;
                    eventDataPtr->terminationEvent = LE_MCC_TERM_UNDEFINED;
                    eventDataPtr->terminationCode = -1;

                    LE_DEBUG_IF(allCallInd.remote_party_name_valid,
                        "remote_party_name_valid Len %d",
                        allCallInd.remote_party_name_len);

                    LE_DEBUG_IF(allCallInd.remote_party_number_ext2_valid,
                        "remote_party_number_ext2_valid Len %d",
                        allCallInd.remote_party_number_ext2_len);

                    GetRemoteNumber(&allCallInd, callInfoPtr->call_id, eventDataPtr->phoneNumber,
                                      LE_MDMDEFS_PHONE_NUM_MAX_BYTES);

                    switch(callEvent)
                    {
                        case LE_MCC_EVENT_SETUP:
                        case LE_MCC_EVENT_INCOMING:
                        case LE_MCC_EVENT_ORIGINATING:
                            EcallRequestedHangup = false;
                        break;

                        case LE_MCC_EVENT_CONNECTED:
                        {
                            if (IsAnswer)
                            {
                                IsAnswer = false;
                                sem_post(&CallSynchronization);
                            }
                        }
                        break;


                        case LE_MCC_EVENT_TERMINATED:
                            GetTerminationReason(&allCallInd, callInfoPtr->call_id,
                                                 &(eventDataPtr->terminationEvent),
                                                 &(eventDataPtr->terminationCode));

                            // If this event follows a HangUp command
                            if(IsHangUp)
                            {
                               IsHangUp = false;
                               sem_post(&CallSynchronization);
                            }
                            break;

                        default:
                        break;
                    }

                    le_event_ReportWithRefCounting(CallEvent, eventDataPtr);
                }
                break;
                default:
                    LE_WARN("QMI call type 0x%02X not handled (call id %d)",
                                  callInfoPtr->call_type, callInfoPtr->call_id);
                break;
            }
        }
    }
}



//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to initialize the QMI MCC module.
 *
 * @return LE_FAULT         The function failed to register the handler.
 * @return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t mcc_qmi_Init
(
    void
)
{
    // Create the event for signaling user handlers.
    CallEvent = le_event_CreateIdWithRefCounting("CallEvent");
    CallEventDataPool = le_mem_CreatePool("CallEventDataPool", sizeof(pa_mcc_CallEventData_t));

    // Init the semaphore for synchronous API (hangup, answer)
    sem_init(&CallSynchronization,0,0);

    if (swiQmi_InitServices(QMI_SERVICE_VOICE) != LE_OK)
    {
        LE_CRIT("QMI_SERVICE_VOICE cannot be initialized.");
        return LE_FAULT;
    }

    // Get the qmi client service handle for later use.
    if ( (VoiceClient = swiQmi_GetClientHandle(QMI_SERVICE_VOICE)) == NULL)
    {
        return LE_FAULT;
    }

    if (swiQmi_InitServices(QMI_SERVICE_DMS) != LE_OK)
    {
        LE_CRIT("QMI_SERVICE_DMS cannot be initialized.");
        return LE_FAULT;
    }

    // Get the qmi client service handle for later use.
    if ( (DmsClient = swiQmi_GetClientHandle(QMI_SERVICE_DMS)) == NULL)
    {
        return LE_FAULT;
    }

    // Register our own handler with the QMI voice service.
    swiQmi_AddIndicationHandler(CallHandler, QMI_SERVICE_VOICE, QMI_VOICE_ALL_CALL_STATUS_IND_V02,
                                NULL);

    // Register our own handler with the QMI voice service for CDMA Call.
    swiQmi_AddIndicationHandler(CallCdmaHandler, QMI_SERVICE_VOICE, QMI_VOICE_INFO_REC_IND_V02,
                                NULL);

    // Set the indications to receive for the voice service.
    voice_indication_register_req_msg_v02 indRegReq = {0};
    voice_indication_register_resp_msg_v02 indRegResp = { {0} };

    // Set the call notification event.
    indRegReq.call_events_valid = true;
    indRegReq.call_events = 1;

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(VoiceClient,
                                            QMI_VOICE_INDICATION_REGISTER_REQ_V02,
                                            &indRegReq, sizeof(indRegReq),
                                            &indRegResp, sizeof(indRegResp),
                                            COMM_DEFAULT_PLATFORM_TIMEOUT);

    return swiQmi_CheckResponseCode(
                                STRINGIZE_EXPAND(QMI_VOICE_INDICATION_REGISTER_REQ_V02),
                                clientMsgErr,
                                indRegResp.resp);
}


//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to register a handler for Call event notifications.
 *
 * @return LE_FAULT         The function failed to register the handler.
 * @return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mcc_SetCallEventHandler
(
    pa_mcc_CallEventHandlerFunc_t   handlerFuncPtr ///< [IN] The event handler function.
)
{
    LE_DEBUG("Set new Call Event handler.");

    LE_FATAL_IF(handlerFuncPtr == NULL, "The new Call Event handler is NULL.");

    CallEventHandlerRef = le_event_AddHandler("CallEventHandler",
                                              CallEvent,
                                              (le_event_HandlerFunc_t)handlerFuncPtr);

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to unregister the handler for incoming calls handling.
 *
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_mcc_ClearCallEventHandler
(
    void
)
{
    le_event_RemoveHandler(CallEventHandlerRef);
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to set a voice call.
 *
 * @return LE_FAULT         The function failed.
 * @return LE_BUSY          A call is already ongoing.
 * @return LE_OVERFLOW      Phone number is too long.
 * @return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mcc_VoiceDial
(
    const char*    phoneNumberPtr,          ///< [IN] The phone number.
    pa_mcc_clir_t  clir,                    ///< [IN] The CLIR supplementary service subscription.
    pa_mcc_cug_t   cug,                     ///< [IN] The CUG supplementary service information.
    uint8_t*       callIdPtr,               ///< [OUT] The outgoing call ID.
    le_mcc_TerminationReason_t*  errorPtr   ///< [OUT] Call termination error.
)
{
    if (phoneNumberPtr == NULL)
    {
        LE_ERROR("Phone number is NULL.");
        return LE_FAULT;
    }

    voice_dial_call_req_msg_v02 dialReq = { {0} };
    voice_dial_call_resp_msg_v02 dialResp = { {0} };

    // Set the phone number.
    if (le_utf8_Copy(dialReq.calling_number, phoneNumberPtr, QMI_VOICE_NUMBER_MAX_V02 + 1, NULL) != LE_OK)
    {
        LE_ERROR("The phone number '%s' is too long.", phoneNumberPtr);
        return LE_OVERFLOW;
    }

    // Set the CLIR value.
    switch (clir)
    {
        case PA_MCC_ACTIVATE_CLIR:
            // Disable presentation of own phone number to remote.
            dialReq.clir_type_valid = true;
            dialReq.clir_type = CLIR_INVOCATION_V02;
            break;

        case PA_MCC_DEACTIVATE_CLIR:
            // Enable presentation of own phone number to the remote end.
            dialReq.clir_type_valid = true;
            dialReq.clir_type = CLIR_SUPPRESSION_V02;
            break;

        case PA_MCC_NO_CLIR:
            // No CLIR field
            dialReq.clir_type_valid = false;
            break;

        default:
            LE_ERROR("The CLIR value is not recognized.");
            return LE_FAULT;
    }

    // Set the CUG value.
    switch (cug)
    {
        case PA_MCC_ACTIVATE_CUG:
            dialReq.cug_valid = true;
            dialReq.cug.cug_index = 0;  // @note: Just use this cug index for now because we do not
                                        //        support setting up CUGs.
            dialReq.cug.suppress_pref_cug = false;
            dialReq.cug.suppress_oa = false;
            break;

        case PA_MCC_DEACTIVATE_CUG:
            dialReq.cug_valid = true;
            dialReq.cug.cug_index = 0;
            dialReq.cug.suppress_pref_cug = true;
            dialReq.cug.suppress_oa = true;
            break;

        case PA_MCC_NO_CUG:
            // No CUG field
            dialReq.cug_valid = false;
            break;

        default:
            LE_ERROR("The CUG value is not recognized.");
            return LE_FAULT;
    }


    // Send the request message.
    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(VoiceClient,
                                            QMI_VOICE_DIAL_CALL_REQ_V02,
                                            &dialReq, sizeof(dialReq),
                                            &dialResp, sizeof(dialResp),
                                            COMM_DEFAULT_PLATFORM_TIMEOUT);

    le_result_t r = swiQmi_CheckResponseCode(STRINGIZE_EXPAND(QMI_VOICE_DIAL_CALL_REQ_V02),
                                    clientMsgErr,
                                    dialResp.resp);

    if ( (r == LE_OK) && dialResp.call_id_valid )
    {
        *callIdPtr = dialResp.call_id;
        return LE_OK;
    }

    if( (r != LE_OK) && errorPtr)
    {
        switch(dialResp.resp.error)
        {
            // FDN restriction
            case QMI_ERR_FDN_RESTRICT_V01:
                *errorPtr = LE_MCC_TERM_FDN_ACTIVE;
                break;

            // Device is offline or in Low Power mode
            case QMI_ERR_NO_NETWORK_FOUND_V01:
                *errorPtr = LE_MCC_TERM_NOT_ALLOWED;
                break;

            default:
                LE_ERROR("Error %d", dialResp.resp.error);
                *errorPtr = LE_MCC_TERM_UNDEFINED;
                break;
        }
    }

    return LE_FAULT;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to answer a call.
 *
 * @return LE_FAULT         The function failed.
 * @return LE_TIMEOUT       No response was received.
 * @return LE_OK            The function succeeded.
 *
 * @todo Maybe return LE_NOT_FOUND if there is no call to answer.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mcc_Answer
(
    uint8_t  callId     ///< [IN] The call ID to answer
)
{
    if (callId == -1)
    {
        LE_WARN("There is no call to answer.");
        return LE_FAULT;
    }

    // @note GCC gives warnings when using extra {} braces are used in initializing structures that
    //       start with a scalar value but also warns if {0} is used for non-scalar structs.  This
    //       is a bug in GCC.  So we drop the extra braces when the struct starts with a scalar.
    voice_answer_call_req_msg_v02 answerReq = {0};
    voice_answer_call_resp_msg_v02 answerResp = { {0} };

    answerReq.call_id = callId;
    IsAnswer = true;

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(VoiceClient,
                                            QMI_VOICE_ANSWER_CALL_REQ_V02,
                                            &answerReq, sizeof(answerReq),
                                            &answerResp, sizeof(answerResp),
                                            COMM_DEFAULT_PLATFORM_TIMEOUT);

    le_result_t r = swiQmi_CheckResponseCode(STRINGIZE_EXPAND(QMI_VOICE_ANSWER_CALL_REQ_V02),
                                            clientMsgErr,
                                            answerResp.resp);

    if (r == LE_OK)
    {
        // Store the call id so that we can hangup later.
        if (answerResp.call_id_valid)
        {
            struct timespec ts;
            if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
                IsAnswer = false;
                LE_WARN("Cannot get current time");
                return LE_FAULT;
            }

            ts.tv_sec += (COMM_DEFAULT_PLATFORM_TIMEOUT/1000);
            ts.tv_nsec += (COMM_DEFAULT_PLATFORM_TIMEOUT%1000)*1000;

            // Make the API synchronous
            if ( sem_timedwait(&CallSynchronization,&ts) == -1 )
            {
                if ( errno == EINTR) {
                    IsAnswer = false;
                    return LE_TIMEOUT;
                }
            }
        }
        else
        {
            // Should never get here.
            LE_CRIT("Did not get a valid call ID.");
            return LE_FAULT;
        }
    }

    IsAnswer = false;
    return r;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to disconnect the remote user.
 *
 * @return LE_FAULT         The function failed.
 * @return LE_TIMEOUT       No response was received.
 * @return LE_OK            The function succeeded.
 *
 * @return Maybe return LE_NOT_FOUND if there is no on-going call.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mcc_HangUp
(
    uint8_t  callId     ///< [IN] The call ID to hangup
)
{
    le_result_t res;

    if (callId == -1)
    {
        LE_WARN("There is no call to terminate.");
        return LE_FAULT;
    }

    IsHangUp = true;

    res = ManageCall(callId, SUPS_TYPE_RELEASE_SPECIFIED_CALL_V02);

    if(res != LE_OK)
    {
        LE_DEBUG("ManageCall() returned res.%d", res);
    }
    else
    {
        struct timespec ts;
        if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
        {
            IsHangUp = false;
            LE_WARN("Cannot get current time");
            return LE_FAULT;
        }

        ts.tv_sec += (COMM_DEFAULT_PLATFORM_TIMEOUT/1000);
        ts.tv_nsec += (COMM_DEFAULT_PLATFORM_TIMEOUT%1000)*1000;

        // Make the API synchronous
        if ( sem_timedwait(&CallSynchronization,&ts) == -1 )
        {
            if ( errno == EINTR)
            {
                IsHangUp = false;
                return LE_TIMEOUT;
            }
        }
    }

    IsHangUp = false;
    return res;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to end all the ongoing calls.
 *
 * @return LE_FAULT         The function failed.
 * @return LE_TIMEOUT       No response was received.
 * @return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mcc_HangUpAll
(
    void
)
{
    le_result_t res;
    voice_get_all_call_info_resp_msg_v02 allCallInfoResp;
    uint32_t callIdx;

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(VoiceClient,
                                            QMI_VOICE_GET_ALL_CALL_INFO_REQ_V02,
                                            NULL, 0,
                                            &allCallInfoResp, sizeof(allCallInfoResp),
                                            COMM_DEFAULT_PLATFORM_TIMEOUT);

    res = swiQmi_CheckResponseCode(STRINGIZE_EXPAND(QMI_VOICE_GET_ALL_CALL_INFO_REQ_V02),
                                   clientMsgErr,
                                   allCallInfoResp.resp);

    LE_ASSERT(res == LE_OK);
    LE_ASSERT(allCallInfoResp.call_info_valid);

    LE_DEBUG("QMI: %u calls", allCallInfoResp.call_info_len);

    for(callIdx = 0; callIdx < allCallInfoResp.call_info_len; callIdx++)
    {
        voice_call_info2_type_v02 * callInfoPtr = &allCallInfoResp.call_info[callIdx];

        LE_DEBUG("Hang-up call id.%u", callInfoPtr->call_id);

        IsHangUp = true;
        res = ManageCall(callInfoPtr->call_id, SUPS_TYPE_RELEASE_SPECIFIED_CALL_V02);
        if(res != LE_OK)
        {
            return LE_FAULT;
        }
        else
        {
            struct timespec ts;
            if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
            {
                IsHangUp = false;
                LE_WARN("Cannot get current time");
                return LE_FAULT;
            }

            ts.tv_sec += (COMM_DEFAULT_PLATFORM_TIMEOUT/1000);
            ts.tv_nsec += (COMM_DEFAULT_PLATFORM_TIMEOUT%1000)*1000;

            // Make the API synchronous
            if ( sem_timedwait(&CallSynchronization,&ts) == -1 )
            {
                if ( errno == EINTR)
                {
                    IsHangUp = false;
                    return LE_TIMEOUT;
                }
            }
        }
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function is called to inform pa_mcc that pa_ecall is requesting the end of the call.
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t mcc_qmi_EcallStop
(
    void
)
{
    EcallRequestedHangup = true;
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function activates or deactivates the call waiting service.
 *
 * @return
 *     - LE_OK        The function succeed.
 *     - LE_FAULT     The function failed.
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mcc_SetCallWaitingService
(
    bool active
        ///< [IN] The call waiting activation.
)
{
    voice_set_sups_service_req_msg_v02 setSupsServiceReq;
    voice_set_sups_service_resp_msg_v02 setSupsServiceResp;
    le_result_t res;

    memset(&setSupsServiceReq,0,sizeof(voice_set_sups_service_req_msg_v02));
    memset(&setSupsServiceResp,0,sizeof(voice_set_sups_service_resp_msg_v02));

    if (active == true)
    {
        setSupsServiceReq.supplementary_service_info.voice_service = VOICE_SERVICE_ACTIVATE_V02;
    }
    else
    {
        setSupsServiceReq.supplementary_service_info.voice_service = VOICE_SERVICE_DEACTIVATE_V02;
    }

    setSupsServiceReq.supplementary_service_info.reason = VOICE_REASON_CALLWAITING_V02;

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(
                                VoiceClient,
                                QMI_VOICE_SET_SUPS_SERVICE_REQ_V02,
                                &setSupsServiceReq, sizeof(voice_set_sups_service_req_msg_v02),
                                &setSupsServiceResp, sizeof(voice_set_sups_service_resp_msg_v02),
                                COMM_LONG_PLATFORM_TIMEOUT);

    res = swiQmi_CheckResponseCode(STRINGIZE_EXPAND(QMI_VOICE_SET_SUPS_SERVICE_REQ_V02),
                                   clientMsgErr,
                                   setSupsServiceResp.resp);

    if (res == LE_OK)
    {
        return LE_OK;
    }
    else
    {
        return LE_FAULT;
    }
}


//--------------------------------------------------------------------------------------------------
/**
 * This function gets the call waiting service status.
 *
 * @return
 *     - LE_OK        The function succeed.
 *     - LE_FAULT     The function failed.
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mcc_GetCallWaitingService
(
    bool* activePtr
        ///< [OUT] The call waiting activation.
)
{
    voice_get_call_waiting_req_msg_v02 getCallWaitingReq;
    voice_get_call_waiting_resp_msg_v02 getCallWaitingResp;

    memset(&getCallWaitingReq,0,sizeof(voice_get_call_waiting_req_msg_v02));
    memset(&getCallWaitingResp,0,sizeof(voice_get_call_waiting_resp_msg_v02));

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(
                                VoiceClient,
                                QMI_VOICE_GET_CALL_WAITING_REQ_V02,
                                &getCallWaitingReq, sizeof(getCallWaitingReq),
                                &getCallWaitingResp, sizeof(getCallWaitingResp),
                                COMM_LONG_PLATFORM_TIMEOUT);

    le_result_t res = swiQmi_CheckResponseCode(STRINGIZE_EXPAND(QMI_VOICE_GET_CALL_WAITING_REQ_V02),
                                                clientMsgErr,
                                                getCallWaitingResp.resp);

    if (res == LE_OK)
    {
        LE_PRINT_VALUE("%i", getCallWaitingResp.service_class_valid);
        LE_PRINT_VALUE("%i", getCallWaitingResp.service_class);

        if ((getCallWaitingResp.service_class_valid == true) &&
            ((getCallWaitingResp.service_class & VOICE_SUPS_CLASS_VOICE_V02 ) ==
                                                                        VOICE_SUPS_CLASS_VOICE_V02))
        {
            *activePtr = true;
        }
        else
        {
            *activePtr = false;
        }

        return LE_OK;
    }

    return LE_FAULT;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function activates the specified call. Other calls are placed on hold.
 *
 * @return
 *     - LE_OK        The function succeed.
 *     - LE_FAULT     The function failed.
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mcc_ActivateCall
(
    uint8_t  callId     ///< [IN] The active call ID
)
{
    return ManageCall(callId, SUPS_TYPE_HOLD_ALL_EXCEPT_SPECIFIED_CALL_V02);
}

#if defined(QMI_DMS_SWI_SET_WB_CAP_REQ_V01)
//--------------------------------------------------------------------------------------------------
/**
 * This function enables/disables the audio AMR Wideband capability.
 *
 * @return
 *     - LE_OK       The function succeeded.
 *     - LE_FAULT    The function failed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mcc_SetAmrWbCapability
(
    bool  enable    ///< [IN] True enables the AMR Wideband capability, false disables it.
)
{
    SWIQMI_DECLARE_MESSAGE(dms_swi_set_wb_cap_req_msg_v01, setAmrWBCapabilityReq);
    SWIQMI_DECLARE_MESSAGE(dms_swi_set_wb_cap_resp_msg_v01, setAmrWBCapabilityRsp);

    setAmrWBCapabilityReq.Wbcapenable = (uint8_t)enable;

    // Send QMI message
    qmi_client_error_type clientMsgErr =
          qmi_client_send_msg_sync(DmsClient,
                                   QMI_DMS_SWI_SET_WB_CAP_REQ_V01,
                                   &setAmrWBCapabilityReq, sizeof(dms_swi_set_wb_cap_req_msg_v01),
                                   &setAmrWBCapabilityRsp, sizeof(dms_swi_set_wb_cap_resp_msg_v01),
                                   COMM_DEFAULT_PLATFORM_TIMEOUT);

    // Check the response
    if (LE_OK != swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_DMS_SWI_SET_WB_CAP_REQ_V01),
                                     clientMsgErr,
                                     setAmrWBCapabilityRsp.resp.result,
                                     setAmrWBCapabilityRsp.resp.error))
    {
        LE_ERROR("Error when setting the audio AMR Wideband capability");
        return LE_FAULT;
    }

    LE_DEBUG("Audio AMR Wideband capability %d is set", enable);
    return LE_OK;
}
#endif

#if defined(QMI_DMS_SWI_GET_WB_CAP_REQ_V01)
//--------------------------------------------------------------------------------------------------
/**
 * This function gets the audio AMR Wideband capability
 *
 * @return
 *     - LE_OK       The function succeeded.
 *     - LE_FAULT    The function failed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mcc_GetAmrWbCapability
(
    bool*  enabledPtr   ///< [OUT] True if AMR Wideband capability is enabled, false otherwise.
)
{
    if (NULL == enabledPtr)
    {
        LE_ERROR("enabledPtr is NULL");
        return LE_FAULT;
    }

    SWIQMI_DECLARE_MESSAGE(dms_swi_get_wb_cap_resp_msg_v01, getAmrWBCapabilityResp);
    // dms_swi_get_wb_cap_req_msg_v01 is empty

    // Send QMI message
    qmi_client_error_type clientMsgErr =
          qmi_client_send_msg_sync(DmsClient,
                                   QMI_DMS_SWI_GET_WB_CAP_REQ_V01,
                                   NULL, 0,
                                   &getAmrWBCapabilityResp, sizeof(dms_swi_get_wb_cap_resp_msg_v01),
                                   COMM_DEFAULT_PLATFORM_TIMEOUT);

    // Check the response
    if (LE_OK != swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_DMS_SWI_GET_WB_CAP_REQ_V01),
                                     clientMsgErr,
                                     getAmrWBCapabilityResp.resp.result,
                                     getAmrWBCapabilityResp.resp.error))
    {
        LE_ERROR("Error when getting the audio AMR Wideband capability");
        return LE_FAULT;
    }

    *enabledPtr = getAmrWBCapabilityResp.Wbcapenable;
    return LE_OK;
}
#endif
