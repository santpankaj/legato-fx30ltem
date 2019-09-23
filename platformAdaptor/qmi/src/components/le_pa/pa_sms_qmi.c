/**
 * @file pa_sms_qmi.c
 *
 * QMI implementation of @ref c_pa_sms API.
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#include "legato.h"
#include "pa_sms.h"
#include "swiQmi.h"

// Include macros for printing out values
#include "le_print.h"

#include "watchdogChain.h"
#include "le_ms_local.h"
#include "thread.h"

//--------------------------------------------------------------------------------------------------
// Symbol and Enum definitions.
//--------------------------------------------------------------------------------------------------

/// Macro used to generate trace output in this module.
/// Takes the same parameters as LE_DEBUG() et. al.
#define TRACE(...) LE_TRACE(TraceRef, ##__VA_ARGS__)

/// Macro used to query current trace state in this module
#define IS_TRACE_ENABLED LE_IS_TRACE_ENABLED(TraceRef)


//--------------------------------------------------------------------------------------------------
/** This event is reported when a new SMS message is received from the modem.  The report data is
 * allocated from the associated pool.  Only one event handler is allowed to be registered at a
 * time, so its reference is stored, in case it needs to be removed later.
 */
//--------------------------------------------------------------------------------------------------
static le_event_Id_t ReceivedSMSEvent;
static le_event_HandlerRef_t ReceivedSMSHandlerRef = NULL;

//--------------------------------------------------------------------------------------------------
/** This event is reported when a storage status indication is received from the modem.
 */
//--------------------------------------------------------------------------------------------------
static le_event_Id_t StorageStatusEvent;

//--------------------------------------------------------------------------------------------------
/**
 * The WMS (Wireless Messaging Services) client.  Must be obtained by calling
 * swiQmi_GetClientHandle() before it can be used.
 */
//--------------------------------------------------------------------------------------------------
static qmi_client_type WmsClient;

//--------------------------------------------------------------------------------------------------
/**
 * Trace reference used for controlling tracing in this module.
 */
//--------------------------------------------------------------------------------------------------
static le_log_TraceRef_t TraceRef;

typedef struct
{
    uint32_t nbCell3GPPConfig;
    uint32_t nbCell3GPP2Config;
    wms_3gpp_broadcast_config_info_type_v01 Cell3GPPBroadcast[WMS_3GPP_BROADCAST_CONFIG_MAX_V01];
    wms_3gpp2_broadcast_config_info_type_v01 Cell3GPP2Broadcast[WMS_3GPP2_BROADCAST_CONFIG_MAX_V01];
} CellBroadCast_t;

static CellBroadCast_t CellBroadcastConfig;


// =============================================
//  PRIVATE FUNCTIONS
// =============================================

//--------------------------------------------------------------------------------------------------
/**
 * Kick the sms watchdog chain
 */
//--------------------------------------------------------------------------------------------------
static void KickWatchdog
(
    void
)
{
    char threadName[MAX_THREAD_NAME_SIZE];

    le_thread_GetName(le_thread_GetCurrent(), threadName, MAX_THREAD_NAME_SIZE);

    if (strncmp(threadName, WDOG_THREAD_NAME_SMS_COMMAND_SENDING,
                strlen(WDOG_THREAD_NAME_SMS_COMMAND_SENDING)) == 0)
    {
        le_wdogChain_Kick(MS_WDOG_SMS_LOOP);
    }
    else
    {
        le_wdogChain_Kick(MS_WDOG_MAIN_LOOP);
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the SMS routes (mainly for testing)
 *
 * @return
 *      LE_OK
 *      LE_FAULT
 */
//--------------------------------------------------------------------------------------------------
static le_result_t GetRoutes(void)
{
    qmi_client_error_type rc;
    int i;

    // There is no data for GetRoutes req, so only need variable for resp.
    wms_get_routes_resp_msg_v01 wmsGetRoutesResp;

    memset(&wmsGetRoutesResp, 0, sizeof(wmsGetRoutesResp) );

    rc = qmi_client_send_msg_sync(WmsClient,
            QMI_WMS_GET_ROUTES_REQ_V01,
            NULL, 0,
            &wmsGetRoutesResp, sizeof(wmsGetRoutesResp),
            COMM_UICC_TIMEOUT);

    le_result_t result = swiQmi_CheckResponseCode(STRINGIZE_EXPAND(QMI_WMS_GET_ROUTES_REQ_V01),
                                                  rc,
                                                  wmsGetRoutesResp.resp);
    if ( result != LE_OK )
    {
        return result;
    }

    if ( IS_TRACE_ENABLED )
    {
        // Print out the received info
        for (i=0; i<wmsGetRoutesResp.route_list_len; i++)
        {
            LE_PRINT_VALUE("%i", i);
            LE_PRINT_VALUE("%i", wmsGetRoutesResp.route_list[i].route_class);
            LE_PRINT_VALUE("%i", wmsGetRoutesResp.route_list[i].route_memory);
            LE_PRINT_VALUE("%i", wmsGetRoutesResp.route_list[i].route_type);
            LE_PRINT_VALUE("%i", wmsGetRoutesResp.route_list[i].route_value);
        }
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function translate the QMI protocol into QMI mode
 */
//--------------------------------------------------------------------------------------------------
static pa_sms_Protocol_t TranslateQmiModeIntoProtocol
(
    wms_message_mode_enum_v01 mode  ///< [IN] the protocol used
)
{
    switch (mode)
    {
    case WMS_MESSAGE_MODE_GW_V01 :
        return PA_SMS_PROTOCOL_GSM;
    case WMS_MESSAGE_MODE_CDMA_V01 :
        return PA_SMS_PROTOCOL_CDMA;
    default:
        return PA_SMS_PROTOCOL_UNKNOWN; // Should not happen
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * This function translate the QMI format into pa protocol
 */
//--------------------------------------------------------------------------------------------------
static wms_message_format_enum_v01 TranslateProtocolIntoQmiFormat
(
    pa_sms_Protocol_t protocol  ///< [IN] the QMI format
)
{
    switch (protocol)
    {
    case PA_SMS_PROTOCOL_GSM:
        return WMS_MESSAGE_FORMAT_GW_PP_V01;
    case PA_SMS_PROTOCOL_CDMA :
        return WMS_MESSAGE_FORMAT_CDMA_V01;
    default:
        return WMS_MESSAGE_FORMAT_GW_PP_V01; // default protocol GSM
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * This function translate the QMI format into pa protocol
 */
//--------------------------------------------------------------------------------------------------
static pa_sms_Protocol_t TranslateQmiFormatIntoProtocol
(
    wms_message_format_enum_v01 format  ///< [IN] the QMI format
)
{
    switch (format)
    {
    case WMS_MESSAGE_FORMAT_GW_PP_V01 :
        return PA_SMS_PROTOCOL_GSM;
    case WMS_MESSAGE_FORMAT_CDMA_V01 :
        return PA_SMS_PROTOCOL_CDMA;
    case WMS_MESSAGE_FORMAT_GW_BC_V01:
    case WMS_MESSAGE_FORMAT_MWI_V01:
    default:
        return PA_SMS_PROTOCOL_UNKNOWN;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * New message handler function called by the QMI WMS service indications.
 */
//--------------------------------------------------------------------------------------------------
static void NewMsgHandler
(
    void*           indBufPtr,  // [IN] The indication message data.
    unsigned int    indBufLen,  // [IN] The indication message data length in bytes.
    void*           contextPtr  // [IN] Context value that was passed to
                                //      swiQmi_AddIndicationHandler().
)
{
    wms_event_report_ind_msg_v01 wmsReport;

    qmi_client_error_type clientMsgErr = qmi_client_message_decode(WmsClient,
                                                                   QMI_IDL_INDICATION,
                                                                   QMI_WMS_EVENT_REPORT_IND_V01,
                                                                   indBufPtr, indBufLen,
                                                                   &wmsReport, sizeof(wmsReport));

    if (QMI_NO_ERR != clientMsgErr)
    {
        LE_ERROR("Error in decoding QMI_WMS_EVENT_REPORT_IND_V01, error = %i", clientMsgErr);
        return;
    }

    if (IS_TRACE_ENABLED)
    {
        LE_DEBUG("MT %c, route %c, mode %c, ETWS %c, ETWS PLMN %c, SMSC %c, IMS %c, call %c",
                 (wmsReport.mt_message_valid ? 'Y' : 'N'),
                 (wmsReport.transfer_route_mt_message_valid ? 'Y' : 'N'),
                 (wmsReport.message_mode_valid ? 'Y' : 'N'),
                 (wmsReport.etws_message_valid ? 'Y' : 'N'),
                 (wmsReport.etws_plmn_info_valid ? 'Y' : 'N'),
                 (wmsReport.mt_message_smsc_address_valid ? 'Y' : 'N'),
                 (wmsReport.sms_on_ims_valid ? 'Y' : 'N'),
                 (wmsReport.call_control_info_valid ? 'Y' : 'N'));

        LE_PRINT_VALUE_IF(wmsReport.mt_message_valid, "%i", wmsReport.mt_message.storage_type);
        LE_PRINT_VALUE_IF(wmsReport.mt_message_valid, "%i", wmsReport.mt_message.storage_index);
        LE_PRINT_VALUE_IF(wmsReport.message_mode_valid, "%i", wmsReport.message_mode);
    }

    if (wmsReport.etws_message_valid)
    {
        LE_DEBUG("ETWS Message type %d", wmsReport.etws_message.notification_type);
        LE_DUMP(wmsReport.etws_message.data, wmsReport.etws_message.data_len);
    }

    // Initialize the data for the event report
    SWIQMI_DECLARE_MESSAGE(pa_sms_NewMessageIndication_t, msgIndication);

    msgIndication.protocol = PA_SMS_PROTOCOL_UNKNOWN;

    if (wmsReport.transfer_route_mt_message_valid)
    {
        LE_DEBUG("Route Message indication => Format %d, Data len %d",
                 wmsReport.transfer_route_mt_message.format,
                 wmsReport.transfer_route_mt_message.data_len);
        LE_DUMP(wmsReport.transfer_route_mt_message.data,
                wmsReport.transfer_route_mt_message.data_len);

        memcpy(msgIndication.pduCB,
               wmsReport.transfer_route_mt_message.data,
               wmsReport.transfer_route_mt_message.data_len);
        msgIndication.pduLen = wmsReport.transfer_route_mt_message.data_len;

        switch (wmsReport.transfer_route_mt_message.format)
        {
            case WMS_MESSAGE_FORMAT_GW_BC_V01:
                msgIndication.protocol = PA_SMS_PROTOCOL_GW_CB;
                msgIndication.storage = PA_SMS_STORAGE_NONE;
                break;

            case WMS_MESSAGE_FORMAT_CDMA_V01:
                msgIndication.protocol = PA_SMS_PROTOCOL_CDMA;
                msgIndication.storage = PA_SMS_STORAGE_NONE;
                break;

            case WMS_MESSAGE_FORMAT_GW_PP_V01:
                msgIndication.protocol = PA_SMS_PROTOCOL_GSM;
                msgIndication.storage = PA_SMS_STORAGE_NONE;
                break;

            case WMS_MESSAGE_FORMAT_MWI_V01:
                msgIndication.protocol = PA_SMS_PROTOCOL_UNKNOWN;
                msgIndication.storage = PA_SMS_STORAGE_NONE;
                break;

            default:
                msgIndication.protocol = PA_SMS_PROTOCOL_UNKNOWN;
                break;
        }
    }

    if (wmsReport.mt_message_valid)
    {
        msgIndication.msgIndex = wmsReport.mt_message.storage_index;

        LE_DEBUG("Storage type %d", wmsReport.mt_message.storage_type);
        switch (wmsReport.mt_message.storage_type)
        {
            case WMS_STORAGE_TYPE_NV_V01:
                msgIndication.storage = PA_SMS_STORAGE_NV;
                break;

            case WMS_STORAGE_TYPE_UIM_V01:
                msgIndication.storage = PA_SMS_STORAGE_SIM;
                break;

            case WMS_STORAGE_TYPE_NONE_V01:
                msgIndication.storage = PA_SMS_STORAGE_NONE;
                break;

            default:
                msgIndication.storage = PA_SMS_STORAGE_UNKNOWN;
                LE_ERROR("New message doesn't provide Storage area indication");
                break;
        }
    }
    else
    {
        if (PA_SMS_STORAGE_UNKNOWN == msgIndication.storage)
        {
            LE_ERROR("New message doesn't provide Storage area indication");
        }
    }

    if (wmsReport.message_mode_valid && (PA_SMS_PROTOCOL_UNKNOWN == msgIndication.protocol))
    {
        msgIndication.protocol = TranslateQmiModeIntoProtocol(wmsReport.message_mode);
    }

    le_event_Report(ReceivedSMSEvent, &msgIndication, sizeof(msgIndication));
}

//--------------------------------------------------------------------------------------------------
/**
 * Sms storage full handler function called by the QMI WMS service indication.
 */
//--------------------------------------------------------------------------------------------------
static void StorageStatusHandler
(
    void*           indBufPtr,  // [IN] The indication message data.
    unsigned int    indBufLen,  // [IN] The indication message data length in bytes.
    void*           contextPtr  // [IN] Context value that was passed to
                                //      swiQmi_AddIndicationHandler().
)
{
    wms_memory_full_ind_msg_v01 wmsReport;

    qmi_client_error_type clientMsgErr = qmi_client_message_decode(
        WmsClient, QMI_IDL_INDICATION, QMI_WMS_MEMORY_FULL_IND_V01,
        indBufPtr, indBufLen,
        &wmsReport, sizeof(wmsReport));

    if (clientMsgErr != QMI_NO_ERR)
    {
        LE_ERROR("Error in decoding QMI_WMS_MEMORY_FULL_IND_V01, error = %i", clientMsgErr);
        return;
    }

    if ( IS_TRACE_ENABLED )
    {
        LE_PRINT_VALUE("%i", wmsReport.memory_full_info.storage_type);
    }

    // Init the data for the event report
    pa_sms_StorageStatusInd_t storageStatus;
    memset(&storageStatus, 0, sizeof(pa_sms_StorageStatusInd_t));

    LE_DEBUG("Storage type %d", wmsReport.memory_full_info.storage_type);


    switch(wmsReport.memory_full_info.storage_type)
    {
        case WMS_STORAGE_TYPE_NV_V01:
        {
            storageStatus.storage = PA_SMS_STORAGE_NV;
        }
        break;

        case WMS_STORAGE_TYPE_UIM_V01:
        {
            storageStatus.storage = PA_SMS_STORAGE_SIM;
        }
        break;

        default:
        {
            storageStatus.storage = PA_SMS_STORAGE_UNKNOWN;
            LE_ERROR("new indication doesn't content Storage area indication");
        }
    }

    le_event_Report(StorageStatusEvent,
                    &storageStatus,
                    sizeof(pa_sms_StorageStatusInd_t));
}

//--------------------------------------------------------------------------------------------------
/**
 * Translate the message status/tag from a Legato enum to a QMI enum
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
static le_result_t TranslateMessageStatus
(
    le_sms_Status_t  status,                   ///< [IN] The status as a Legato enum
    wms_message_tag_type_enum_v01* qmiTagPtr   ///< [OUT] The status as a QMI enum

)
{
    // Map from Legato type to QMI type
    switch (status)
    {
        case LE_SMS_RX_READ:
            *qmiTagPtr = WMS_TAG_TYPE_MT_READ_V01;
            break;

        case LE_SMS_RX_UNREAD:
            *qmiTagPtr = WMS_TAG_TYPE_MT_NOT_READ_V01;
            break;

        case LE_SMS_STORED_SENT:
            *qmiTagPtr = WMS_TAG_TYPE_MO_SENT_V01;
            break;

        case LE_SMS_STORED_UNSENT:
            *qmiTagPtr = WMS_TAG_TYPE_MO_NOT_SENT_V01;
            break;

        default:
            LE_ERROR("Status %i is invalid", status);
            return LE_FAULT;
    }

    return LE_OK;
}


//--------------------------------------------------------------------------------------------------
/**
 * This function translate the QMI tl_cause_code for 3GPP2 message into
 *   le_sms_ErrorCode3GPP2_t value.
 */
//--------------------------------------------------------------------------------------------------
static le_sms_ErrorCode3GPP2_t Translate3GPP2ErrorCode
(
    wms_tl_cause_code_enum_v01 cause_code
)
{
    le_sms_ErrorCode3GPP2_t errorCode = LE_SMS_ERROR_3GPP2_MAX;

    switch(cause_code)
    {
        case WMS_TL_CAUSE_CODE_ADDR_VACANT_V01:
        {
            errorCode = LE_SMS_ERROR_ADDR_VACANT;
        }
        break;

        case WMS_TL_CAUSE_CODE_ADDR_TRANSLATION_FAILURE_V01:
        {
            errorCode = LE_SMS_ERROR_ADDR_TRANSLATION_FAILURE;
        }
        break;

        case WMS_TL_CAUSE_CODE_NETWORK_RESOURCE_SHORTAGE_V01:
        {
            errorCode = LE_SMS_ERROR_RADIO_IF_RESOURCE_SHORTAGE;
        }
        break;

        case WMS_TL_CAUSE_CODE_NETWORK_FAILURE_V01:
        {
            errorCode = LE_SMS_ERROR_NETWORK_FAILURE;
        }
        break;

        case WMS_TL_CAUSE_CODE_INVALID_TELESERVICE_ID_V01:
        {
            errorCode = LE_SMS_ERROR_INVALID_TELESERVICE_ID;
        }
        break;

        case WMS_TL_CAUSE_CODE_NETWORK_OTHER_V01:
        {
            errorCode = LE_SMS_ERROR_NETWORK_OTHER;
        }
        break;

        case WMS_TL_CAUSE_CODE_NO_PAGE_RESPONSE_V01:
        {
            errorCode = LE_SMS_ERROR_NO_PAGE_RESPONSE;
        }
        break;

        case WMS_TL_CAUSE_CODE_DEST_BUSY_V01:
        {
            errorCode = LE_SMS_ERROR_DEST_BUSY;
        }
        break;

        case WMS_TL_CAUSE_CODE_NO_ACK_V01:
        {
            errorCode = LE_SMS_ERROR_NO_ACK;
        }
        break;

        case WMS_TL_CAUSE_CODE_DEST_RESOURCE_SHORTAGE_V01:
        {
            errorCode = LE_SMS_ERROR_DEST_RESOURCE_SHORTAGE;
        }
        break;

        case WMS_TL_CAUSE_CODE_SMS_DELIVERY_POSTPONED_V01:
        {
            errorCode = LE_SMS_ERROR_SMS_DELIVERY_POSTPONED;
        }
        break;

        case WMS_TL_CAUSE_CODE_DEST_OUT_OF_SERV_V01:
        {
            errorCode = LE_SMS_ERROR_DEST_OUT_OF_SERV;
        }
        break;

        case WMS_TL_CAUSE_CODE_DEST_NOT_AT_ADDR_V01:
        {
            errorCode = LE_SMS_ERROR_DEST_NOT_AT_ADDR;
        }
        break;

        case WMS_TL_CAUSE_CODE_DEST_OTHER_V01:
        {
            errorCode = LE_SMS_ERROR_DEST_OTHER;
        }
        break;

        case WMS_TL_CAUSE_CODE_RADIO_IF_RESOURCE_SHORTAGE_V01:
        {
            errorCode = LE_SMS_ERROR_RADIO_IF_RESOURCE_SHORTAGE;
        }
        break;

        case WMS_TL_CAUSE_CODE_RADIO_IF_INCOMPATABILITY_V01:
        {
            errorCode = LE_SMS_ERROR_RADIO_IF_INCOMPATABILITY;
        }
        break;

        case WMS_TL_CAUSE_CODE_RADIO_IF_OTHER_V01:
        {
            errorCode = LE_SMS_ERROR_RADIO_IF_OTHER;
        }
        break;

        case WMS_TL_CAUSE_CODE_ENCODING_V01:
        {
            errorCode = LE_SMS_ERROR_ENCODING;
        }
        break;

        case WMS_TL_CAUSE_CODE_SMS_ORIG_DENIED_V01:
        {
            errorCode = LE_SMS_ERROR_SMS_ORIG_DENIED;
        }
        break;

        case WMS_TL_CAUSE_CODE_SMS_TERM_DENIED_V01:
        {
            errorCode = LE_SMS_ERROR_SMS_TERM_DENIED;
        }
        break;

        case WMS_TL_CAUSE_CODE_SUPP_SERV_NOT_SUPP_V01:
        {
            errorCode = LE_SMS_ERROR_SUPP_SERV_NOT_SUPP;
        }
        break;

        case WMS_TL_CAUSE_CODE_SMS_NOT_SUPP_V01:
        {
            errorCode = LE_SMS_ERROR_SMS_NOT_SUPP;
        }
        break;

        case WMS_TL_CAUSE_CODE_MISSING_EXPECTED_PARAM_V01:
        {
            errorCode = LE_SMS_ERROR_MISSING_EXPECTED_PARAM;
        }
        break;

        case WMS_TL_CAUSE_CODE_MISSING_MAND_PARAM_V01:
        {
            errorCode = LE_SMS_ERROR_MISSING_MAND_PARAM;
        }
        break;

        case WMS_TL_CAUSE_CODE_UNRECOGNIZED_PARAM_VAL_V01:
        {
            errorCode = LE_SMS_ERROR_UNRECOGNIZED_PARAM_VAL;
        }
        break;

        case WMS_TL_CAUSE_CODE_UNEXPECTED_PARAM_VAL_V01:
        {
            errorCode = LE_SMS_ERROR_UNEXPECTED_PARAM_VAL;
        }
        break;

        case WMS_TL_CAUSE_CODE_USER_DATA_SIZE_ERR_V01:
        {
            errorCode = LE_SMS_ERROR_USER_DATA_SIZE_ERR;
        }
        break;

        case WMS_TL_CAUSE_CODE_GENERAL_OTHER_V01:
        {
            errorCode = LE_SMS_ERROR_GENERAL_OTHER;
        }
        break;

        default:
            LE_ERROR("cause_code %i is invalid", cause_code);
            break;
    }

    return errorCode;
}


//--------------------------------------------------------------------------------------------------
/**
 * This function translate the QMI Radio Protocol cause_code for 3GPP message into
 *   le_sms_ErrorCode_t value.
 */
//--------------------------------------------------------------------------------------------------
static le_sms_ErrorCode_t TranslateRPErrorCode
(
    wms_rp_cause_enum_v01 cause_code
)
{
    le_sms_ErrorCode_t errorCode = LE_SMS_ERROR_3GPP_MAX;

    switch(cause_code)
    {
        case WMS_RP_CAUSE_UNASSIGNED_NUMBER_V01:
        {
            errorCode = LE_SMS_RP_ERROR_UNASSIGNED_NUMBER;
        }
        break;

        case WMS_RP_CAUSE_OPERATOR_DETERMINED_BARRING_V01:
        {
            errorCode = LE_SMS_RP_ERROR_OPERATOR_DETERMINED_BARRING;
        }
        break;

        case WMS_RP_CAUSE_CALL_BARRED_V01:
        {
            errorCode = LE_SMS_RP_ERROR_CALL_BARRED;
        }
        break;

        case WMS_RP_CAUSE_RESERVED_V01:
        {
            errorCode = LE_SMS_RP_ERROR_RESERVED;
        }
        break;

        case WMS_RP_CAUSE_SMS_TRANSFER_REJECTED_V01:
        {
            errorCode = LE_SMS_RP_ERROR_SMS_TRANSFER_REJECTED;
        }
        break;

        case WMS_RP_CAUSE_MEMORY_CAP_EXCEEDED_V01:
        {
            errorCode = LE_SMS_RP_ERROR_MEMORY_CAP_EXCEEDED;
        }
        break;

        case WMS_RP_CAUSE_DESTINATION_OUT_OF_ORDER_V01:
        {
            errorCode = LE_SMS_RP_ERROR_DESTINATION_OUT_OF_ORDER;
        }
        break;

        case WMS_RP_CAUSE_UNIDENTIFIED_SUBSCRIBER_V01:
        {
            errorCode = LE_SMS_RP_ERROR_UNIDENTIFIED_SUBSCRIBER;
        }
        break;

        case WMS_RP_CAUSE_FACILITY_REJECTED_V01:
        {
            errorCode = LE_SMS_RP_ERROR_FACILITY_REJECTED;
        }
        break;

        case WMS_RP_CAUSE_UNKNOWN_SUBSCRIBER_V01:
        {
            errorCode = LE_SMS_RP_ERROR_UNKNOWN_SUBSCRIBER;
        }
        break;

        case WMS_RP_CAUSE_NETWORK_OUT_OF_ORDER_V01:
        {
            errorCode = LE_SMS_RP_ERROR_NETWORK_OUT_OF_ORDER;
        }
        break;

        case WMS_RP_CAUSE_TEMPORARY_FAILURE_V01:
        {
            errorCode = LE_SMS_RP_ERROR_TEMPORARY_FAILURE;
        }
        break;

        case WMS_RP_CAUSE_CONGESTION_V01:
        {
            errorCode = LE_SMS_RP_ERROR_CONGESTION;
        }
        break;

        case WMS_RP_CAUSE_RESOURCES_UNAVAILABLE_V01:
        {
            errorCode = LE_SMS_RP_ERROR_RESOURCES_UNAVAILABLE;
        }
        break;

        case WMS_RP_CAUSE_REQUESTED_FACILITY_NOT_SUBSCRIBED_V01:
        {
            errorCode = LE_SMS_RP_ERROR_REQUESTED_FACILITY_NOT_SUBSCRIBED;
        }
        break;

        case WMS_RP_CAUSE_REQUESTED_FACILITY_NOT_IMPLEMENTED_V01:
        {
            errorCode = LE_SMS_RP_ERROR_REQUESTED_FACILITY_NOT_IMPLEMENTED;
        }
        break;

        case WMS_RP_CAUSE_INVALID_SMS_TRANSFER_REFERENCE_VALUE_V01:
        {
            errorCode = LE_SMS_RP_ERROR_INVALID_SMS_TRANSFER_REFERENCE_VALUE;
        }
        break;

        case WMS_RP_CAUSE_SEMANTICALLY_INCORRECT_MESSAGE_V01:
        {
            errorCode = LE_SMS_RP_ERROR_SEMANTICALLY_INCORRECT_MESSAGE;
        }
        break;

        case WMS_RP_CAUSE_INVALID_MANDATORY_INFO_V01:
        {
            errorCode = LE_SMS_RP_ERROR_INVALID_MANDATORY_INFO;
        }
        break;

        case WMS_RP_CAUSE_MESSAGE_TYPE_NOT_IMPLEMENTED_V01:
        {
            errorCode = LE_SMS_RP_ERROR_MESSAGE_TYPE_NOT_IMPLEMENTED;
        }
        break;

        case WMS_RP_CAUSE_MESSAGE_NOT_COMPATABLE_WITH_SMS_V01:
        {
            errorCode = LE_SMS_RP_ERROR_MESSAGE_NOT_COMPATABLE_WITH_SMS;
        }
        break;

        case WMS_RP_CAUSE_INFO_ELEMENT_NOT_IMPLEMENTED_V01:
        {
            errorCode = LE_SMS_RP_ERROR_INFO_ELEMENT_NOT_IMPLEMENTED;
        }
        break;

        case WMS_RP_CAUSE_PROTOCOL_ERROR_V01:
        {
            errorCode = LE_SMS_RP_ERROR_PROTOCOL_ERROR;
        }
        break;

        case WMS_RP_CAUSE_INTERWORKING_V01:
        {
            errorCode = LE_SMS_RP_ERROR_INTERWORKING;
        }
        break;

        default:
        {
           LE_ERROR("cause_code %i is invalid", cause_code);
        }
        break;
    }

    return errorCode;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function translate the QMI Transfert protocol cause_code for 3GPP message into
 *   le_sms_ErrorCode_t value.
 */
//--------------------------------------------------------------------------------------------------
static le_sms_ErrorCode_t TranslateTPErrorCode
(
    wms_tp_cause_enum_v01 cause_code
)
{
    le_sms_ErrorCode_t errorCode = LE_SMS_ERROR_3GPP_MAX;

    switch(cause_code)
    {
        case WMS_TP_CAUSE_TELE_INTERWORKING_NOT_SUPPORTED_V01:
        {
            errorCode = LE_SMS_TP_ERROR_TELE_INTERWORKING_NOT_SUPPORTED;
        }
        break;

        case WMS_TP_CAUSE_SHORT_MESSAGE_TYPE_0_NOT_SUPPORTED_V01:
        {
            errorCode = LE_SMS_TP_ERROR_SHORT_MESSAGE_TYPE_0_NOT_SUPPORTED;
        }
        break;

        case WMS_TP_CAUSE_SHORT_MESSAGE_CANNOT_BE_REPLACED_V01:
        {
            errorCode = LE_SMS_TP_ERROR_SHORT_MESSAGE_CANNOT_BE_REPLACED;
        }
        break;

        case WMS_TP_CAUSE_UNSPECIFIED_PID_ERROR_V01:
        {
            errorCode = LE_SMS_TP_ERROR_UNSPECIFIED_PID_ERROR;
        }
        break;

        case WMS_TP_CAUSE_DCS_NOT_SUPPORTED_V01:
        {
            errorCode = LE_SMS_TP_ERROR_DCS_NOT_SUPPORTED;
        }
        break;

        case WMS_TP_CAUSE_MESSAGE_CLASS_NOT_SUPPORTED_V01:
        {
            errorCode = LE_SMS_TP_ERROR_MESSAGE_CLASS_NOT_SUPPORTED;
        }
        break;

        case WMS_TP_CAUSE_UNSPECIFIED_DCS_ERROR_V01:
        {
            errorCode = LE_SMS_TP_ERROR_UNSPECIFIED_DCS_ERROR;
        }
        break;

        case WMS_TP_CAUSE_COMMAND_CANNOT_BE_ACTIONED_V01:
        {
            errorCode = LE_SMS_TP_ERROR_COMMAND_CANNOT_BE_ACTIONED;
        }
        break;

        case WMS_TP_CAUSE_COMMAND_UNSUPPORTED_V01:
        {
            errorCode = LE_SMS_TP_ERROR_COMMAND_UNSUPPORTED;
        }
        break;

        case WMS_TP_CAUSE_UNSPECIFIED_COMMAND_ERROR_V01:
        {
            errorCode = LE_SMS_TP_ERROR_UNSPECIFIED_COMMAND_ERROR;
        }
        break;

        case WMS_TP_CAUSE_TPDU_NOT_SUPPORTED_V01:
        {
            errorCode = LE_SMS_TP_ERROR_TPDU_NOT_SUPPORTED;
        }
        break;

        case WMS_TP_CAUSE_SC_BUSY_V01:
        {
            errorCode = LE_SMS_TP_ERROR_SC_BUSY;
        }
        break;

        case WMS_TP_CAUSE_NO_SC_SUBSCRIPTION_V01:
        {
            errorCode = LE_SMS_TP_ERROR_NO_SC_SUBSCRIPTION;
        }
        break;

        case WMS_TP_CAUSE_SC_SYS_FAILURE_V01:
        {
            errorCode = LE_SMS_TP_ERROR_SC_SYS_FAILURE;
        }
        break;

        case WMS_TP_CAUSE_INVALID_SME_ADDRESS_V01:
        {
            errorCode = LE_SMS_TP_ERROR_INVALID_SME_ADDRESS;
        }
        break;

        case WMS_TP_CAUSE_DESTINATION_SME_BARRED_V01:
        {
            errorCode = LE_SMS_TP_ERROR_DESTINATION_SME_BARRED;
        }
        break;

        case WMS_TP_CAUSE_SM_REJECTED_OR_DUPLICATE_V01:
        {
            errorCode = LE_SMS_TP_ERROR_SM_REJECTED_OR_DUPLICATE;
        }
        break;

        case WMS_TP_CAUSE_TP_VPF_NOT_SUPPORTED_V01:
        {
            errorCode = LE_SMS_TP_ERROR_TP_VPF_NOT_SUPPORTED;
        }
        break;


        case WMS_TP_CAUSE_TP_VP_NOT_SUPPORTED_V01:
        {
            errorCode = LE_SMS_TP_ERROR_TP_VP_NOT_SUPPORTED;
        }
        break;

        case WMS_TP_CAUSE_SIM_SMS_STORAGE_FULL_V01:
        {
            errorCode = LE_SMS_TP_ERROR_SIM_SMS_STORAGE_FULL;
        }
        break;

        case WMS_TP_CAUSE_NO_SMS_STORAGE_CAP_IN_SIM_V01:
        {
            errorCode = LE_SMS_TP_ERROR_NO_SMS_STORAGE_CAP_IN_SIM;
        }
        break;

        case WMS_TP_CAUSE_MS_ERROR_V01:
        {
            errorCode = LE_SMS_TP_ERROR_MS_ERROR;
        }
        break;

        case WMS_TP_CAUSE_MEMORY_CAP_EXCEEDED_V01:
        {
            errorCode = LE_SMS_TP_ERROR_MEMORY_CAP_EXCEEDED;
        }
        break;

        case WMS_TP_CAUSE_SIM_APP_TOOLKIT_BUSY_V01:
        {
            errorCode = LE_SMS_TP_ERROR_SIM_APP_TOOLKIT_BUSY;
        }
        break;

        case WMS_TP_CAUSE_SIM_DATA_DOWNLOAD_ERROR_V01:
        {
            errorCode = LE_SMS_TP_ERROR_SIM_DATA_DOWNLOAD_ERROR;
        }
        break;

        case WMS_TP_CAUSE_UNSPECIFIED_ERROR_V01:
        {
            errorCode = LE_SMS_TP_ERROR_UNSPECIFIED_ERROR;
        }
        break;

        default:
            LE_ERROR("cause_code %i is invalid", cause_code);
            break;
    }

    return errorCode;
}


/*
 * Activate or Deactivate Cell Broadcast message notification
 */
static le_result_t ActivateCellBroadcast
(
    pa_sms_Protocol_t protocol,
    bool activation
)
{
    wms_set_broadcast_activation_req_msg_v01  wms_msg_req = { {0} };
    wms_set_broadcast_activation_resp_msg_v01 wms_msg_rsp = { {0} };
    qmi_client_error_type rc;
    le_result_t res;

    if (activation)
    {
        wms_msg_req.broadcast_activation_info.bc_activate = 1;
        wms_msg_req.activate_all_valid = 1;
        wms_msg_req.activate_all = 1;
    }
    else
    {
        wms_msg_req.broadcast_activation_info.bc_activate = 0;
        wms_msg_req.activate_all_valid = 0;
        wms_msg_req.activate_all = 0;
    }

    if (protocol == PA_SMS_PROTOCOL_CDMA)
    {
        wms_msg_req.broadcast_activation_info.message_mode = WMS_MESSAGE_MODE_CDMA_V01;
    }
    else
    {
        wms_msg_req.broadcast_activation_info.message_mode = WMS_MESSAGE_MODE_GW_V01;
    }

    LE_DEBUG("CellBroadcast activate '%c', message_mode %d",
        (wms_msg_req.broadcast_activation_info.bc_activate ? 'Y' : 'N'),
        wms_msg_req.broadcast_activation_info.message_mode);

    rc = qmi_client_send_msg_sync(WmsClient,
                    QMI_WMS_SET_BROADCAST_ACTIVATION_REQ_V01,
                    &wms_msg_req, sizeof(wms_msg_req),
                    &wms_msg_rsp, sizeof(wms_msg_rsp),
                    COMM_UICC_TIMEOUT);

    res = swiQmi_CheckResponseCode(STRINGIZE_EXPAND(QMI_WMS_SET_BROADCAST_ACTIVATION_REQ_V01),
                    rc,
                    wms_msg_rsp.resp);

    return res;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function configure CDMA Cell Broadcast Services list.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t UpdateCdmaCellBroadcastServices
(
    void
)
{
    wms_set_broadcast_config_req_msg_v01  wms_msg_req = {0};
    wms_set_broadcast_config_resp_msg_v01 wms_msg_rsp = { {0} };
    qmi_client_error_type rc;
    le_result_t res;

    wms_msg_req.wms_3gpp_broadcast_config_info_valid = false;
    wms_msg_req.wms_3gpp2_broadcast_config_info_valid = true;
    memcpy(wms_msg_req.wms_3gpp2_broadcast_config_info,
        CellBroadcastConfig.Cell3GPP2Broadcast,
        sizeof(CellBroadcastConfig.Cell3GPP2Broadcast));
    wms_msg_req.wms_3gpp2_broadcast_config_info_len = CellBroadcastConfig.nbCell3GPP2Config;
    wms_msg_req.message_mode = WMS_MESSAGE_MODE_CDMA_V01;

    LE_DEBUG("Nb Elements 3GPP %c (%d), 3GPP2 %c (%d)",
        (wms_msg_req.wms_3gpp_broadcast_config_info_valid ? 'Y' :'N'),
        wms_msg_req.wms_3gpp_broadcast_config_info_len,
        (wms_msg_req.wms_3gpp2_broadcast_config_info_valid ? 'Y' :'N'),
        wms_msg_req.wms_3gpp2_broadcast_config_info_len);

    rc = qmi_client_send_msg_sync(WmsClient,
                    QMI_WMS_SET_BROADCAST_CONFIG_REQ_V01,
                    &wms_msg_req, sizeof(wms_msg_req),
                    &wms_msg_rsp, sizeof(wms_msg_rsp),
                    COMM_UICC_TIMEOUT);

    res = swiQmi_CheckResponseCode(STRINGIZE_EXPAND(QMI_WMS_SET_BROADCAST_CONFIG_REQ_V01),
                    rc,
                    wms_msg_rsp.resp);

    return res;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function configure Cell Broadcast message Identifiers range.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t UpdateCellBroadcastIds
(
    void
)
{
    wms_set_broadcast_config_req_msg_v01  wms_msg_req = {0};
    wms_set_broadcast_config_resp_msg_v01 wms_msg_rsp = { {0} };
    qmi_client_error_type rc;
    le_result_t res;

    wms_msg_req.wms_3gpp2_broadcast_config_info_valid = false;
    wms_msg_req.wms_3gpp_broadcast_config_info_valid = true;
    memcpy(wms_msg_req.wms_3gpp_broadcast_config_info,
        CellBroadcastConfig.Cell3GPPBroadcast,
        sizeof(CellBroadcastConfig.Cell3GPPBroadcast));
    wms_msg_req.wms_3gpp_broadcast_config_info_len = CellBroadcastConfig.nbCell3GPPConfig;
    wms_msg_req.message_mode = WMS_MESSAGE_MODE_GW_V01;

    LE_DEBUG("Nb Elements 3GPP %c (%d), 3GPP2 %c (%d)",
        (wms_msg_req.wms_3gpp_broadcast_config_info_valid ? 'Y' :'N'),
        wms_msg_req.wms_3gpp_broadcast_config_info_len,
        (wms_msg_req.wms_3gpp2_broadcast_config_info_valid ? 'Y' :'N'),
        wms_msg_req.wms_3gpp2_broadcast_config_info_len);

    rc = qmi_client_send_msg_sync(WmsClient,
                    QMI_WMS_SET_BROADCAST_CONFIG_REQ_V01,
                    &wms_msg_req, sizeof(wms_msg_req),
                    &wms_msg_rsp, sizeof(wms_msg_rsp),
                    COMM_UICC_TIMEOUT);

    res = swiQmi_CheckResponseCode(STRINGIZE_EXPAND(QMI_WMS_SET_BROADCAST_CONFIG_REQ_V01),
                    rc,
                    wms_msg_rsp.resp);

    return res;
}

// =============================================
//  MODULE/COMPONENT FUNCTIONS
// =============================================

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to initialize the QMI SMS module.
 *
 * @return
 *      LE_OK if successful.
 *      LE_FAULT if unsuccessful.
 */
//--------------------------------------------------------------------------------------------------
le_result_t sms_qmi_Init
(
    void
)
{
    qmi_client_error_type clientMsgErr;
    le_result_t result;

    // Get a reference to the trace keyword that is used to control tracing in this module.
    TraceRef = le_log_GetTraceRef("sms");

    memset(&CellBroadcastConfig, 0, sizeof(CellBroadcastConfig));

    // Create the event for signaling user handlers.
    ReceivedSMSEvent = le_event_CreateId("ReceivedSMSEvent",
                                         sizeof(pa_sms_NewMessageIndication_t));

    // Create the event for signaling user handlers.
    StorageStatusEvent = le_event_CreateId("StorageStatusEvent",
                                         sizeof(pa_sms_StorageStatusInd_t));

    if (swiQmi_InitServices(QMI_SERVICE_WMS) != LE_OK)
    {
        LE_CRIT("QMI_SERVICE_WMS cannot be initialized.");
        return LE_FAULT;
    }

    // Get the qmi client service handle for later use.
    if ((WmsClient = swiQmi_GetClientHandle(QMI_SERVICE_WMS)) == NULL)
    {
        return LE_FAULT;
    }

    // Register our own handler with the QMI DMS service.
    swiQmi_AddIndicationHandler(NewMsgHandler,
                                QMI_SERVICE_WMS,
                                QMI_WMS_EVENT_REPORT_IND_V01, NULL);
    swiQmi_AddIndicationHandler(StorageStatusHandler,
                                QMI_SERVICE_WMS,
                                QMI_WMS_MEMORY_FULL_IND_V01, NULL);

    // Set the indications to receive for the DMS service.
    wms_set_event_report_req_msg_v01 wmsSetEventReq = {0};
    wms_set_event_report_resp_msg_v01 wmsSetEventResp = { {0} };

    // Set new message indications.
    wmsSetEventReq.report_mt_message_valid = 1;
    wmsSetEventReq.report_mt_message = 1;

    // Set message waiting indications.
    wmsSetEventReq.report_mwi_message_valid = 1;
    wmsSetEventReq.report_mwi_message = 1;

    clientMsgErr = qmi_client_send_msg_sync(WmsClient,
                                            QMI_WMS_SET_EVENT_REPORT_REQ_V01,
                                            &wmsSetEventReq, sizeof(wmsSetEventReq),
                                            &wmsSetEventResp, sizeof(wmsSetEventResp),
                                            COMM_DEFAULT_PLATFORM_TIMEOUT);

    result = swiQmi_CheckResponseCode(STRINGIZE_EXPAND(QMI_WMS_SET_EVENT_REPORT_REQ_V01),
                                      clientMsgErr,
                                      wmsSetEventResp.resp);
    if ( result != LE_OK )
    {
        LE_ERROR("Could not register message indication");
        return result;
    }

    GetRoutes();    // for testing, get original settings
    return result;
}


// =============================================
//  PUBLIC API FUNCTIONS
// =============================================

//--------------------------------------------------------------------------------------------------
/**
 * This function is called to set the preferred SMS storage area.
 *
 * @return LE_FAULT         The function failed.
 * @return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sms_SetPreferredStorage
(
    le_sms_Storage_t prefStorage  ///< [IN] The preferred SMS storage area
)
{
    qmi_client_error_type rc;
    le_result_t result = LE_OK;

    wms_set_routes_req_msg_v01  wmsSetRoutesReq;
    wms_set_routes_resp_msg_v01 wmsSetRoutesRsp;

    memset(&wmsSetRoutesReq, 0, sizeof(wmsSetRoutesReq) );
    memset(&wmsSetRoutesRsp, 0, sizeof(wmsSetRoutesRsp) );

    // set the prefered SMS storage area
    wmsSetRoutesReq.route_list_tuple_len = 1;
    wmsSetRoutesReq.route_list_tuple[0].message_type =
                                             WMS_MESSAGE_TYPE_POINT_TO_POINT_V01;
    wmsSetRoutesReq.route_list_tuple[0].message_class = WMS_MESSAGE_CLASS_NONE_V01;

    if (prefStorage == LE_SMS_STORAGE_SIM)
    {
        wmsSetRoutesReq.route_list_tuple[0].route_storage = WMS_STORAGE_TYPE_UIM_V01;
    }
    else if (prefStorage == LE_SMS_STORAGE_NV)
    {
        wmsSetRoutesReq.route_list_tuple[0].route_storage = WMS_STORAGE_TYPE_NV_V01;
    }
    else
    {
        LE_ERROR("wrong storage");
        return LE_FAULT;
    }
    wmsSetRoutesReq.route_list_tuple[0].receipt_action = WMS_STORE_AND_NOTIFY_V01;

    rc = qmi_client_send_msg_sync(WmsClient,
            QMI_WMS_SET_ROUTES_REQ_V01,
            &wmsSetRoutesReq, sizeof(wmsSetRoutesReq),
            &wmsSetRoutesRsp, sizeof(wmsSetRoutesRsp),
            COMM_UICC_TIMEOUT);

    result = swiQmi_CheckResponseCode(
                                 STRINGIZE_EXPAND(QMI_WMS_SET_ROUTES_RESP_V01),
                                 rc,
                                 wmsSetRoutesRsp.resp);
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function is called to get the preferred SMS storage area.
 *
 * @return LE_FAULT         The function failed.
 * @return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sms_GetPreferredStorage
(
    le_sms_Storage_t* prefStoragePtr  ///< [OUT] The preferred SMS storage area
)
{

    qmi_client_error_type rc;
    int i;
    le_sms_Storage_t  storage;

    // There is no data for GetRoutes req, so only need variable for resp.
    wms_get_routes_resp_msg_v01 wmsGetRoutesResp;

    memset(&wmsGetRoutesResp, 0, sizeof(wmsGetRoutesResp) );

    rc = qmi_client_send_msg_sync(WmsClient,
            QMI_WMS_GET_ROUTES_REQ_V01,
            NULL, 0,
            &wmsGetRoutesResp, sizeof(wmsGetRoutesResp),
            COMM_UICC_TIMEOUT);

    le_result_t result = swiQmi_CheckResponseCode(STRINGIZE_EXPAND(QMI_WMS_GET_ROUTES_REQ_V01),
                                                  rc,
                                                  wmsGetRoutesResp.resp);
    if ( result != LE_OK )
    {
        return result;
    }

    if ( IS_TRACE_ENABLED )
    {
        for (i=0; i<wmsGetRoutesResp.route_list_len; i++)
        {
            LE_PRINT_VALUE("GetRoutesResp tuple %i", i);
            LE_PRINT_VALUE("GetRoutesResp type %i", wmsGetRoutesResp.route_list[i].route_type);
            LE_PRINT_VALUE("GetRoutesResp class %i", wmsGetRoutesResp.route_list[i].route_class);
            LE_PRINT_VALUE("GetRoutesResp memory %i", wmsGetRoutesResp.route_list[i].route_memory);
            LE_PRINT_VALUE("GetRoutesResp reoute_value %i", wmsGetRoutesResp.route_list[i].route_value);
        }
    }

    for (i=0; i<wmsGetRoutesResp.route_list_len; i++)
    {
        // get class None storage
        if (wmsGetRoutesResp.route_list[i].route_class == WMS_MESSAGE_CLASS_NONE_V01)
        {
            // translate response
            if (wmsGetRoutesResp.route_list[i].route_memory == WMS_STORAGE_TYPE_UIM_V01)
            {
                storage = LE_SMS_STORAGE_SIM;
            }
            else if (wmsGetRoutesResp.route_list[i].route_memory == WMS_STORAGE_TYPE_NV_V01)
            {
                storage = LE_SMS_STORAGE_NV;
            }
            else
            {
                storage = LE_SMS_STORAGE_MAX;
            }
            *prefStoragePtr = storage;
            return LE_OK;
        }
    }
    return LE_FAULT;
}


//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to register a handler for a new message reception handling.
 *
 * @return LE_FAULT         The function failed to register a new handler.
 * @return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sms_SetNewMsgHandler
(
    pa_sms_NewMsgHdlrFunc_t msgHandler   ///< [IN] The handler function to handle a new message
                                         ///       reception.
)
{
    TRACE("called");
    LE_ASSERT(msgHandler != NULL);

    if ( ReceivedSMSHandlerRef != NULL )
    {
        LE_INFO("Clearing old handler first");
        pa_sms_ClearNewMsgHandler();
    }

    ReceivedSMSHandlerRef = le_event_AddHandler(
                                "ReceivedSMSHandler",
                                ReceivedSMSEvent,
                                (le_event_HandlerFunc_t) msgHandler);

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to unregister the handler for a new message reception handling.
 *
 * @return
 *      \return LE_FAULT         The function failed to unregister the handler.
 *      \return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sms_ClearNewMsgHandler
(
    void
)
{
    le_event_RemoveHandler(ReceivedSMSHandlerRef);
    ReceivedSMSHandlerRef = NULL;

    // todo: does this function need a return result;
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 *
 * This function is used to add a status SMS storage notification handler
 *
 * @return A handler reference, which is only needed for later removal of the handler.
 */
//--------------------------------------------------------------------------------------------------
le_event_HandlerRef_t pa_sms_AddStorageStatusHandler
(
    pa_sms_StorageMsgHdlrFunc_t statusHandler   ///< [IN] The handler function to handle a status
                                                ///  notification reception.
)
{
    le_event_HandlerRef_t StorageHandler=NULL;
    LE_ASSERT(statusHandler != NULL);

    StorageHandler =  le_event_AddHandler(
                                "PaStorageStatusHandler",
                                StorageStatusEvent,
                                (le_event_HandlerFunc_t) statusHandler);

    return (le_event_HandlerRef_t) StorageHandler;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function is called to unregister from a storage message notification handler.
 */
//--------------------------------------------------------------------------------------------------
void pa_sms_RemoveStorageStatusHandler
(
    le_event_HandlerRef_t storageHandler
)
{
    le_event_RemoveHandler(storageHandler);
}

//--------------------------------------------------------------------------------------------------
/**
 * This function sends a message in PDU mode.
 *
 * @return LE_OK              The function succeeded.
 * @return LE_FAULT           The function failed to send a message in PDU mode.
 * @return LE_TIMEOUT         No response was received from the Modem.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sms_SendPduMsg
(
    pa_sms_Protocol_t        protocol,   ///< [IN] protocol to use
    uint32_t                 length,     ///< [IN] The length of the TP data unit in bytes.
    const uint8_t*           dataPtr,    ///< [IN] The message.
    uint8_t*                 msgRef,     ///< [OUT] Message reference (TP-MR)
    uint32_t                 timeout,    ///< [IN] Timeout in seconds.
    pa_sms_SendingErrCode_t* errorCode   ///< [OUT] The error code.
)
{
    le_result_t res;

    LE_DEBUG("SendPdu proto %d, len %d, timeout %d",protocol, length, timeout);

    if (length > WMS_MESSAGE_LENGTH_MAX_V01)
    {
        LE_ERROR("PDU message exceeds maximum length (%d>%d)", length, WMS_MESSAGE_LENGTH_MAX_V01);
        res = LE_FAULT;
    }
    else
    {
        qmi_client_error_type rc;

        if (IS_TRACE_ENABLED)
        {
            LE_PRINT_VALUE("%i", length);
            LE_DUMP(dataPtr, length);
        }

        /* SMS transaction */
        wms_raw_send_req_msg_v01   wms_msg_req;
        wms_raw_send_resp_msg_v01  wms_msg_rsp;

        memset(&wms_msg_req, 0, sizeof(wms_msg_req) );
        memset(&wms_msg_rsp, 0, sizeof(wms_msg_rsp) );

        wms_msg_req.raw_message_data.format = TranslateProtocolIntoQmiFormat(protocol);
        wms_msg_req.raw_message_data.raw_message_len = length;
        errorCode->rp = LE_SMS_ERROR_3GPP_MAX;
        errorCode->tp = LE_SMS_ERROR_3GPP_MAX;
        errorCode->code3GPP2 = LE_SMS_ERROR_3GPP2_MAX;
        errorCode->platformSpecific = 0;
        *msgRef = 0;

        /* Copy binary payload */
        memcpy(wms_msg_req.raw_message_data.raw_message, dataPtr, length);

        rc = qmi_client_send_msg_sync(WmsClient,
                QMI_WMS_RAW_SEND_REQ_V01,
                &wms_msg_req, sizeof(wms_msg_req),
                &wms_msg_rsp, sizeof(wms_msg_rsp),
                timeout * 1000);

        res = swiQmi_CheckResponseCode(STRINGIZE_EXPAND(QMI_WMS_RAW_SEND_REQ_V01),
                                        rc,
                                        wms_msg_rsp.resp);

        //The QMI timer out above is too long, kick watchdog here to avoid watchdog expiry.
        KickWatchdog();

        if( res != LE_OK)
        {
            LE_ERROR_IF(wms_msg_rsp.cause_code_valid, "Return cause code 0x%X",
                            wms_msg_rsp.cause_code);

            LE_ERROR_IF(wms_msg_rsp.error_class_valid, "Return error class 0x%X",
                wms_msg_rsp.error_class);

            LE_ERROR_IF(wms_msg_rsp.gw_cause_info_valid,
                "Return gw error cause rp 0x%X, tp 0x%X",
                wms_msg_rsp.gw_cause_info.rp_cause, wms_msg_rsp.gw_cause_info.tp_cause);

            LE_ERROR_IF(wms_msg_rsp.message_delivery_failure_type_valid,
                "Return message delivery failure type 0x%X",
                wms_msg_rsp.message_delivery_failure_type);

            LE_ERROR_IF(wms_msg_rsp.message_delivery_failure_cause_valid,
                "Return message delivery failure cause 0x%X",
                wms_msg_rsp.message_delivery_failure_cause);

            if( wms_msg_rsp.resp.error == QMI_ERR_CAUSE_CODE_V01)
            {
                if (protocol == PA_SMS_PROTOCOL_CDMA)
                {
                    if (wms_msg_rsp.cause_code_valid)
                    {
                        // Translate QMI cause code
                        errorCode->code3GPP2 = Translate3GPP2ErrorCode(wms_msg_rsp.cause_code);
                    }
                }
                else
                {
                    if (wms_msg_rsp.gw_cause_info_valid)
                    {
                        LE_DEBUG("GW RPcause 0x%02X, TPcause 0x%02X",
                                        wms_msg_rsp.gw_cause_info.rp_cause,
                                        wms_msg_rsp.gw_cause_info.tp_cause);
                        // Translate GW error code
                        errorCode->rp = TranslateRPErrorCode(wms_msg_rsp.gw_cause_info.rp_cause);
                        errorCode->tp = TranslateTPErrorCode(wms_msg_rsp.gw_cause_info.tp_cause);
                    }
                }
            }
            else
            {
                if (protocol == PA_SMS_PROTOCOL_CDMA)
                {
                    errorCode->code3GPP2 = LE_SMS_ERROR_3GPP2_PLATFORM_SPECIFIC;
                }
                else
                {
                    errorCode->rp = LE_SMS_ERROR_3GPP2_PLATFORM_SPECIFIC;
                    errorCode->tp = LE_SMS_ERROR_3GPP2_PLATFORM_SPECIFIC;
                }
                errorCode->platformSpecific = wms_msg_rsp.resp.error;
                LE_DEBUG("Platform specific %d", errorCode->platformSpecific);
            }

            if(rc == QMI_TIMEOUT_ERR)
            {
                res = LE_TIMEOUT;
            }
        }
        else
        {
            *msgRef = wms_msg_rsp.message_id;
        }
    }

    return res;
}


//-------------------------------------------------------------------------------------------------
/**
 * This function gets the message from the preferred message storage for protocol.
 *
 * @return LE_FAULT        The function failed to get the message from the preferred message
 *                         storage.
 * @return LE_UNSUPPORTED  unsupported protocol.
 * @return LE_OK           The function succeeded.
 */
//-------------------------------------------------------------------------------------------------
le_result_t pa_sms_RdPDUMsgFromMem
(
    uint32_t            index,      ///< [IN] The place of storage in memory.
    pa_sms_Protocol_t   protocol,   ///< [IN] The protocol used.
    pa_sms_Storage_t    storage,    ///< [IN] SMS Storage used.
    pa_sms_Pdu_t*       msgPtr      ///< [OUT] The message.
)
{
    qmi_client_error_type rc;
    le_result_t result;
    SWIQMI_DECLARE_MESSAGE(wms_raw_read_req_msg_v01, wms_msg_req);
    SWIQMI_DECLARE_MESSAGE(wms_raw_read_resp_msg_v01, wms_msg_rsp);

    LE_DEBUG("index: %d, protocol: %d, storage: %d", index, protocol, storage);

    if (storage == PA_SMS_STORAGE_NV)
    {
        wms_msg_req.message_memory_storage_identification.storage_type = WMS_STORAGE_TYPE_NV_V01;
    }
    else
    {
        wms_msg_req.message_memory_storage_identification.storage_type = WMS_STORAGE_TYPE_UIM_V01;
    }
    wms_msg_req.message_memory_storage_identification.storage_index = index;

    switch (protocol)
    {
        case PA_SMS_PROTOCOL_GSM :
            wms_msg_req.message_mode_valid = 1;
            wms_msg_req.message_mode = WMS_MESSAGE_MODE_GW_V01;
            break;

        case PA_SMS_PROTOCOL_CDMA :
            wms_msg_req.message_mode_valid = 1;
            wms_msg_req.message_mode = WMS_MESSAGE_MODE_CDMA_V01;
            break;

        default:
            LE_WARN("Protocol %d is not supported", protocol);
            return LE_UNSUPPORTED;
    }

    rc = qmi_client_send_msg_sync(WmsClient,
                                  QMI_WMS_RAW_READ_REQ_V01,
                                  &wms_msg_req, sizeof(wms_msg_req),
                                  &wms_msg_rsp, sizeof(wms_msg_rsp),
                                  COMM_UICC_TIMEOUT);

    if (LE_OK != swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_WMS_RAW_READ_REQ_V01),
                                      rc,
                                      wms_msg_rsp.resp.result,
                                      wms_msg_rsp.resp.error))
    {
        return LE_FAULT;
    }

    if (IS_TRACE_ENABLED)
    {
        LE_PRINT_VALUE("%i", wms_msg_rsp.raw_message_data.tag_type);
        LE_PRINT_VALUE("%i", wms_msg_rsp.raw_message_data.format);
        LE_PRINT_VALUE("%i", wms_msg_rsp.raw_message_data.data_len);
        LE_DUMP(wms_msg_rsp.raw_message_data.data, wms_msg_rsp.raw_message_data.data_len);
    }

    // Fill in the results in the msg return value
    msgPtr->status = LE_SMS_RX_READ;    // Hard-code for now.

    // Fill in the message the protocol
    msgPtr->protocol = TranslateQmiFormatIntoProtocol(wms_msg_rsp.raw_message_data.format);

    // Copy binary payload
    if (wms_msg_rsp.raw_message_data.data_len <= LE_SMS_PDU_MAX_BYTES)
    {
        msgPtr->dataLen = wms_msg_rsp.raw_message_data.data_len;
        memcpy(msgPtr->data,
               wms_msg_rsp.raw_message_data.data,
               wms_msg_rsp.raw_message_data.data_len);
        result = LE_OK;
    }
    else
    {
        LE_ERROR("Message length is > PDU_MAX_BYTES (%d>%d)!",
                 wms_msg_rsp.raw_message_data.data_len, LE_SMS_PDU_MAX_BYTES);
        msgPtr->dataLen = LE_SMS_PDU_MAX_BYTES;
        memcpy(msgPtr->data, wms_msg_rsp.raw_message_data.data, LE_SMS_PDU_MAX_BYTES);
        result = LE_FAULT;
    }

    return result;
}


//--------------------------------------------------------------------------------------------------
/**
 * This function gets the indexes of messages stored in the preferred memory for a specific
 * status.
 *
 * @return LE_FAULT          The function failed to get the indexes of messages stored in the
 *                           preferred memory.
 * @return LE_UNSUPPORTED    Unsupported protocol.
 * @return LE_OK             The function succeeded.
 *
 * @todo: The numPtr parameter needs to be an IN/OUT parameter, because otherwise we don't know
 *        the size of the array pointed to be idxPtr.  For now, we can only assume it will always
 *        be sufficiently large.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sms_ListMsgFromMem
(
    le_sms_Status_t     status,     ///< [IN] The status of message in memory.
    pa_sms_Protocol_t   protocol,   ///< [IN] The protocol to read
    uint32_t           *numPtr,     ///< [OUT] The number of indexes retrieved.
    uint32_t           *idxPtr,     ///< [OUT] The pointer to an array of indexes.
                                    ///        The array is filled with 'num' index values.
    pa_sms_Storage_t    storage     ///< [IN] SMS Storage used
)
{
    TRACE("called");

    qmi_client_error_type rc;
    wms_message_tag_type_enum_v01 qmiTagType;

    /* SMS transaction */
    wms_list_messages_req_msg_v01 wmsListReq = {0};
    SWIQMI_DECLARE_MESSAGE(wms_list_messages_resp_msg_v01, wmsListResp);

    if ( TranslateMessageStatus(status, &qmiTagType) != LE_OK )
    {
        return LE_FAULT;
    }

    if (storage == PA_SMS_STORAGE_NV)
    {
        wmsListReq.storage_type = WMS_STORAGE_TYPE_NV_V01;
    }
    else
    {
        wmsListReq.storage_type = WMS_STORAGE_TYPE_UIM_V01;
    }
    wmsListReq.tag_type_valid = true;
    wmsListReq.tag_type = qmiTagType;

    switch (protocol)
    {
        case PA_SMS_PROTOCOL_GSM :
        {
            wmsListReq.message_mode_valid = true;
            wmsListReq.message_mode = WMS_MESSAGE_MODE_GW_V01;
            break;
        }
        case PA_SMS_PROTOCOL_CDMA :
        {
            wmsListReq.message_mode_valid = true;
            wmsListReq.message_mode = WMS_MESSAGE_MODE_CDMA_V01;
            break;
        }
        default:
        {
            LE_ERROR("Protocol %d not supported", protocol);
            return LE_UNSUPPORTED;
        }
    }

    rc = qmi_client_send_msg_sync(WmsClient,
                                  QMI_WMS_LIST_MESSAGES_REQ_V01,
                                  &wmsListReq, sizeof(wmsListReq),
                                  &wmsListResp, sizeof(wmsListResp),
                                  COMM_UICC_TIMEOUT);

    le_result_t result = swiQmi_CheckResponseCode(STRINGIZE_EXPAND(QMI_WMS_LIST_MESSAGES_REQ_V01),
                                                  rc,
                                                  wmsListResp.resp);
    if ( result != LE_OK )
    {

        return result;
    }

    int i;
    uint32_t numIndices;

    numIndices = wmsListResp.message_tuple_len;
    for ( i=0; i<numIndices; i++ )
    {
        idxPtr[i] = wmsListResp.message_tuple[i].message_index;
    }

    if ( IS_TRACE_ENABLED )
    {
        LE_PRINT_VALUE("%u", numIndices);
        LE_PRINT_ARRAY("%u", numIndices, idxPtr);
    }

    *numPtr = numIndices;

    return LE_OK;
}


//--------------------------------------------------------------------------------------------------
/**
 * This function deletes one specific Message from preferred message storage.
 *
 * @return LE_FAULT          The function failed to delete one specific Message from preferred
 *                           message storage.
 * @return LE_UNSUPPORTED    Unsupported protocol.
 * @return LE_OK             The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sms_DelMsgFromMem
(
    uint32_t            index,    ///< [IN] Index of the message to be deleted.
    pa_sms_Protocol_t   protocol, ///< [IN] protocol
    pa_sms_Storage_t    storage   ///< [IN] SMS Storage used
)
{
    qmi_client_error_type rc;

    /* SMS transaction */
    wms_delete_req_msg_v01   wms_msg_req;
    wms_delete_resp_msg_v01  wms_msg_rsp;

    LE_PRINT_VALUE("%u", index);
    LE_DEBUG("idx %d, proto %d, storage %d", index, protocol, storage);

    memset(&wms_msg_req, 0, sizeof(wms_msg_req) );
    memset(&wms_msg_rsp, 0, sizeof(wms_msg_rsp) );

    if (storage == PA_SMS_STORAGE_NV)
    {
        wms_msg_req.storage_type = WMS_STORAGE_TYPE_NV_V01;
    }
    else
    {
        wms_msg_req.storage_type = WMS_STORAGE_TYPE_UIM_V01;
    }
    wms_msg_req.index_valid = true;
    wms_msg_req.index = index;

    switch (protocol)
    {
        case PA_SMS_PROTOCOL_GSM :
        {
            wms_msg_req.message_mode_valid = true;
            wms_msg_req.message_mode = WMS_MESSAGE_MODE_GW_V01;
            break;
        }
        case PA_SMS_PROTOCOL_CDMA :
        {
            wms_msg_req.message_mode_valid = true;
            wms_msg_req.message_mode = WMS_MESSAGE_MODE_CDMA_V01;
            break;
        }
        default:
        {
            LE_ERROR("Protocol %d not supported", protocol);
            return LE_UNSUPPORTED;
        }
    }

    rc = qmi_client_send_msg_sync(WmsClient,
            QMI_WMS_DELETE_REQ_V01,
            &wms_msg_req, sizeof(wms_msg_req),
            &wms_msg_rsp, sizeof(wms_msg_rsp),
            COMM_UICC_TIMEOUT);

    le_result_t res = swiQmi_CheckResponseCode(STRINGIZE_EXPAND(QMI_WMS_DELETE_REQ_V01),
                                    rc,
                                    wms_msg_rsp.resp);

    return res;
}


//--------------------------------------------------------------------------------------------------
/**
 * This function deletes all Messages from preferred message storage.
 *
 * @return LE_FAULT        The function failed to delete all Messages from preferred message storage.
 * @return LE_OK           The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sms_DelAllMsg
(
    void
)
{
    // todo: implement

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function changes the message status.
 *
 * @return LE_FAULT         The function failed.
 * @return LE_UNSUPPORTED   Unsupported protocol.
 * @return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sms_ChangeMessageStatus
(
    uint32_t            index,    ///< [IN] Index of the message to be deleted.
    pa_sms_Protocol_t   protocol, ///< [IN] protocol
    le_sms_Status_t     status,   ///< [IN] The status of message in memory.
    pa_sms_Storage_t    storage   ///< [IN] SMS Storage used
)
{
    TRACE("called");

    qmi_client_error_type rc;
    wms_message_tag_type_enum_v01 qmiTagType;

    /* SMS transaction */
    wms_modify_tag_req_msg_v01 wmsModReq = { {0} };
    wms_modify_tag_resp_msg_v01 wmsModResp = { {0} };

    LE_DEBUG("idx %d, proto %d, storage %d",index, protocol, storage);
    if ( TranslateMessageStatus(status, &qmiTagType) != LE_OK )
    {
        return LE_FAULT;
    }

    if (storage == PA_SMS_STORAGE_NV)
    {
        wmsModReq.wms_message_tag.storage_type = WMS_STORAGE_TYPE_NV_V01;
    }
    else
    {
        wmsModReq.wms_message_tag.storage_type = WMS_STORAGE_TYPE_UIM_V01;
    }
    wmsModReq.wms_message_tag.storage_index = index;
    wmsModReq.wms_message_tag.tag_type = qmiTagType;

    switch (protocol)
    {
        case PA_SMS_PROTOCOL_GSM :
        {
            wmsModReq.message_mode_valid = true;
            wmsModReq.message_mode = WMS_MESSAGE_MODE_GW_V01;
            break;
        }
        case PA_SMS_PROTOCOL_CDMA :
        {
            wmsModReq.message_mode_valid = true;
            wmsModReq.message_mode = WMS_MESSAGE_MODE_CDMA_V01;
            break;
        }
        default:
        {
            LE_ERROR("Protocol %d not supported", protocol);
            return LE_UNSUPPORTED;
        }
    }

    rc = qmi_client_send_msg_sync(WmsClient,
                                  QMI_WMS_MODIFY_TAG_REQ_V01,
                                  &wmsModReq, sizeof(wmsModReq),
                                  &wmsModResp, sizeof(wmsModResp),
                                  COMM_UICC_TIMEOUT);

    le_result_t res = swiQmi_CheckResponseCode(STRINGIZE_EXPAND(QMI_WMS_MODIFY_TAG_REQ_V01),
                                    rc,
                                    wmsModResp.resp);

    return res;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to get the SMS center.
 *
 * @return
 *   - LE_FAULT        The function failed.
 *   - LE_OK           The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sms_GetSmsc
(
    char*        smscPtr,  ///< [OUT] The Short message service center string.
    size_t       len       ///< [IN] The length of SMSC string.
)
{
    TRACE("called");

    qmi_client_error_type rc;

    LE_ASSERT(len-1 < WMS_ADDRESS_DIGIT_MAX_V01);

    /* SMS transaction */
    wms_get_smsc_address_resp_msg_v01  wms_msg_rsp;

    memset(&wms_msg_rsp, 0, sizeof(wms_msg_rsp) );

    rc = qmi_client_send_msg_sync(WmsClient,
                QMI_WMS_GET_SMSC_ADDRESS_REQ_V01,
                NULL, 0,
                &wms_msg_rsp, sizeof(wms_msg_rsp),
                COMM_UICC_TIMEOUT);

    le_result_t result = swiQmi_CheckResponseCode(STRINGIZE_EXPAND(
                                                  QMI_WMS_GET_SMSC_ADDRESS_REQ_V01),
                                                  rc,
                                                  wms_msg_rsp.resp);
    if ( result == LE_OK )
    {
        result = le_utf8_Copy(smscPtr,
                              wms_msg_rsp.smsc_address.smsc_address_digits,
                              len,
                              NULL);
        if ( result != LE_OK )
        {
            result = LE_FAULT;
        }
        else
        {
            result = LE_OK;
        }
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to set the SMS center.
 *
 * @return
 *   - LE_FAULT        The function failed.
 *   - LE_OK           The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sms_SetSmsc
(
    const char*    smscPtr  ///< [IN] The Short message service center.
)
{
    TRACE("called");
    le_result_t res;
    qmi_client_error_type rc;

    LE_ASSERT(strlen(smscPtr) < WMS_ADDRESS_DIGIT_MAX_V01);

    /* SMS transaction */
    wms_set_smsc_address_req_msg_v01   wms_msg_req;
    wms_set_smsc_address_resp_msg_v01  wms_msg_rsp;

    memset(&wms_msg_req, 0, sizeof(wms_msg_req) );
    memset(&wms_msg_rsp, 0, sizeof(wms_msg_rsp) );

    if (le_utf8_Copy(wms_msg_req.smsc_address_digits,
                     smscPtr,
                     sizeof(wms_msg_req.smsc_address_digits),
                     NULL) == LE_OK)
    {
        rc = qmi_client_send_msg_sync(WmsClient,
                QMI_WMS_SET_SMSC_ADDRESS_REQ_V01,
                &wms_msg_req, sizeof(wms_msg_req),
                &wms_msg_rsp, sizeof(wms_msg_rsp),
                COMM_UICC_TIMEOUT);

        res = swiQmi_CheckResponseCode(STRINGIZE_EXPAND(QMI_WMS_SET_SMSC_ADDRESS_REQ_V01),
                                        rc,
                                        wms_msg_rsp.resp);
    }
    else
    {
        res = LE_FAULT;
    }

    return res;
}


//--------------------------------------------------------------------------------------------------
/**
 * Activate Cell Broadcast message notification
 *
 * @return
 *  - LE_FAULT         Function failed.
 *  - LE_OK            Function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sms_ActivateCellBroadcast
(
    pa_sms_Protocol_t protocol
)
{
    return ActivateCellBroadcast(protocol, true);
}


//--------------------------------------------------------------------------------------------------
/**
 * Deactivate Cell Broadcast message notification.
 *
 * @return
 *  - LE_FAULT         Function failed.
 *  - LE_OK            Function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sms_DeactivateCellBroadcast
(
    pa_sms_Protocol_t protocol
)
{
    return ActivateCellBroadcast(protocol, false);
}


//--------------------------------------------------------------------------------------------------
/**
 * Add Cell Broadcast message Identifiers range.
 *
 * @return
 *  - LE_FAULT         Function failed.
 *  - LE_OK            Function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sms_AddCellBroadcastIds
(
    uint16_t fromId,
        ///< [IN]
        ///< Starting point of the range of cell broadcast message identifier.

    uint16_t toId
        ///< [IN]
        ///< Ending point of the range of cell broadcast message identifier.
)
{
    le_result_t res = LE_FAULT;
    int i;

    if (CellBroadcastConfig.nbCell3GPPConfig >= WMS_3GPP_BROADCAST_CONFIG_MAX_V01)
    {
        LE_ERROR("Max Cell Broadcast service number reached!!");
        return res;
    }

    for(i=0; (i < CellBroadcastConfig.nbCell3GPPConfig)
    && (i < WMS_3GPP_BROADCAST_CONFIG_MAX_V01); i++)
    {
        if ( (CellBroadcastConfig.Cell3GPPBroadcast[i].from_service_id == fromId)
                        && (CellBroadcastConfig.Cell3GPPBroadcast[i].to_service_id == toId))
        {
            LE_DEBUG("Parameter already set");
            return res;
        }
    }

    CellBroadcastConfig.Cell3GPPBroadcast[CellBroadcastConfig.nbCell3GPPConfig].from_service_id =
                    fromId;
    CellBroadcastConfig.Cell3GPPBroadcast[CellBroadcastConfig.nbCell3GPPConfig].to_service_id =
                    toId;
    CellBroadcastConfig.Cell3GPPBroadcast[CellBroadcastConfig.nbCell3GPPConfig].selected = 1;
    CellBroadcastConfig.nbCell3GPPConfig++;

    res = UpdateCellBroadcastIds();
    return res;
}


//--------------------------------------------------------------------------------------------------
/**
 * Remove Cell Broadcast message Identifiers range.
 *
 * @return
 *  - LE_FAULT         Function failed.
 *  - LE_OK            Function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sms_RemoveCellBroadcastIds
(
    uint16_t fromId,
        ///< [IN]
        ///< Starting point of the range of cell broadcast message identifier.

    uint16_t toId
        ///< [IN]
        ///< Ending point of the range of cell broadcast message identifier.
)
{
    le_result_t res = LE_FAULT;
    int i,j;

    for(i=0; (i < CellBroadcastConfig.nbCell3GPPConfig)
        && (i < WMS_3GPP_BROADCAST_CONFIG_MAX_V01); i++)
    {
        if ( (CellBroadcastConfig.Cell3GPPBroadcast[i].from_service_id == fromId)
              && (CellBroadcastConfig.Cell3GPPBroadcast[i].to_service_id == toId))
        {
            CellBroadcastConfig.Cell3GPPBroadcast[i].selected = 0;
            res = UpdateCellBroadcastIds();

            // Not applicable if it is the only or the last entry in the array
            if(i < (CellBroadcastConfig.nbCell3GPPConfig - 1))
            {
                for(j = i+1; (j < (CellBroadcastConfig.nbCell3GPPConfig))
                    && (j < WMS_3GPP2_BROADCAST_CONFIG_MAX_V01); j++ )
                {
                    CellBroadcastConfig.Cell3GPPBroadcast[j-1].from_service_id =
                                    CellBroadcastConfig.Cell3GPPBroadcast[j].from_service_id;
                    CellBroadcastConfig.Cell3GPPBroadcast[j-1].to_service_id =
                                    CellBroadcastConfig.Cell3GPPBroadcast[j].to_service_id;
                    CellBroadcastConfig.Cell3GPPBroadcast[j-1].selected =
                                    CellBroadcastConfig.Cell3GPPBroadcast[j].selected;
                }
            }
            memset(&CellBroadcastConfig.Cell3GPPBroadcast[CellBroadcastConfig.nbCell3GPPConfig], 0,
                sizeof(wms_3gpp_broadcast_config_info_type_v01));
            CellBroadcastConfig.nbCell3GPPConfig--;

            return res;
        }
    }
    LE_ERROR("Entry not Found!");

    return res;
}


//--------------------------------------------------------------------------------------------------
/**
 * Clear Cell Broadcast message Identifiers range.
 *
 * @return
 *  - LE_FAULT         Function failed.
 *  - LE_OK            Function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sms_ClearCellBroadcastIds
(
    void
)
{
    memset(&CellBroadcastConfig.Cell3GPPBroadcast[0], 0,
        sizeof(CellBroadcastConfig.Cell3GPPBroadcast));
    CellBroadcastConfig.nbCell3GPPConfig = 0;

    return UpdateCellBroadcastIds();
}


//--------------------------------------------------------------------------------------------------
/**
 * Add CDMA Cell Broadcast category services.
 *
 * @return
 *  - LE_FAULT         Function failed.
 *  - LE_OK            Function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sms_AddCdmaCellBroadcastServices
(
    le_sms_CdmaServiceCat_t serviceCat,
        ///< [IN]
        ///< Service category assignment. Reference to 3GPP2 C.R1001-D
        ///< v1.0 Section 9.3.1 Standard Service Category Assignments.

    le_sms_Languages_t language
        ///< [IN]
        ///< Language Indicator. Reference to
        ///< 3GPP2 C.R1001-D v1.0 Section 9.2.1 Language Indicator
        ///< Value Assignments
)
{
    le_result_t res = LE_FAULT;
    int i;


    if (CellBroadcastConfig.nbCell3GPP2Config >= WMS_3GPP2_BROADCAST_CONFIG_MAX_V01)
    {
        LE_ERROR("Max CDMA Cell Broadcast service number reached!!");
        return res;
    }

    for(i=0; (i < CellBroadcastConfig.nbCell3GPP2Config)
        && (i < WMS_3GPP2_BROADCAST_CONFIG_MAX_V01); i++)
    {
        if ( (CellBroadcastConfig.Cell3GPP2Broadcast[i].service_category
                        == (wms_service_category_enum_v01) serviceCat)
                        && (CellBroadcastConfig.Cell3GPP2Broadcast[i].language
                        == (wms_language_enum_v01) language))
        {
            LE_ERROR("Cell Broadcast service number aleary set");
            return res;
        }
    }

    CellBroadcastConfig.Cell3GPP2Broadcast[CellBroadcastConfig.nbCell3GPP2Config].service_category
    = serviceCat;
    CellBroadcastConfig.Cell3GPP2Broadcast[CellBroadcastConfig.nbCell3GPP2Config].language
    = language;
    CellBroadcastConfig.Cell3GPP2Broadcast[CellBroadcastConfig.nbCell3GPP2Config].selected = 1;
    CellBroadcastConfig.nbCell3GPP2Config++;

    res = UpdateCdmaCellBroadcastServices();

    return res;
}


//--------------------------------------------------------------------------------------------------
/**
 * Remove CDMA Cell Broadcast category services.
 *
 * @return
 *  - LE_FAULT         Function failed.
 *  - LE_OK            Function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sms_RemoveCdmaCellBroadcastServices
(
    le_sms_CdmaServiceCat_t serviceCat,
        ///< [IN]
        ///< Service category assignment. Reference to 3GPP2 C.R1001-D
        ///< v1.0 Section 9.3.1 Standard Service Category Assignments.

    le_sms_Languages_t language
        ///< [IN]
        ///< Language Indicator. Reference to
        ///< 3GPP2 C.R1001-D v1.0 Section 9.2.1 Language Indicator
        ///< Value Assignments
)
{
    le_result_t res = LE_FAULT;
    int i,j;

    for(i=0; (i < CellBroadcastConfig.nbCell3GPP2Config)
        && (i < WMS_3GPP2_BROADCAST_CONFIG_MAX_V01); i++)
    {
        if ( (CellBroadcastConfig.Cell3GPP2Broadcast[i].service_category
                        == (wms_service_category_enum_v01) serviceCat)
                        && (CellBroadcastConfig.Cell3GPP2Broadcast[i].language
                                        == (wms_language_enum_v01) language))
        {
            CellBroadcastConfig.Cell3GPP2Broadcast[i].selected = 0;
            res = UpdateCdmaCellBroadcastServices();

            // Not applicable if it is the only or the last entry in the array
            if(i < (CellBroadcastConfig.nbCell3GPP2Config - 1))
            {
                for(j = i+1 ; (j < CellBroadcastConfig.nbCell3GPP2Config)
                && (j < WMS_3GPP2_BROADCAST_CONFIG_MAX_V01); j++ )
                {
                    CellBroadcastConfig.Cell3GPP2Broadcast[j-1].service_category =
                                    CellBroadcastConfig.Cell3GPP2Broadcast[j].service_category;
                    CellBroadcastConfig.Cell3GPP2Broadcast[j-1].language =
                                    CellBroadcastConfig.Cell3GPP2Broadcast[j].language;
                    CellBroadcastConfig.Cell3GPP2Broadcast[j-1].selected =
                                    CellBroadcastConfig.Cell3GPP2Broadcast[j].selected;
                }
            }
            memset(&CellBroadcastConfig.Cell3GPP2Broadcast[CellBroadcastConfig.nbCell3GPP2Config],
                0, sizeof(wms_3gpp2_broadcast_config_info_type_v01));

            CellBroadcastConfig.nbCell3GPP2Config--;
            return res;
        }
    }

    return res;
}

//--------------------------------------------------------------------------------------------------
/**
 * Clear CDMA Cell Broadcast category services.
 *
 * @return
 *  - LE_FAULT         Function failed.
 *  - LE_OK            Function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sms_ClearCdmaCellBroadcastServices
(
    void
)
{
    memset(&CellBroadcastConfig.Cell3GPP2Broadcast[0], 0,
        sizeof(CellBroadcastConfig.Cell3GPP2Broadcast));
    CellBroadcastConfig.nbCell3GPP2Config = 0;

    return UpdateCdmaCellBroadcastServices();
}
