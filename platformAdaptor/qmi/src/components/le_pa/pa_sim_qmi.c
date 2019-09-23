/**
 * @file pa_sim_qmi.c
 *
 * QMI implementation of @ref c_pa_sim API relying mainly on the UIM QMI interface.
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#include "legato.h"
#include "interfaces.h"
#include "pa_qmi_local.h"
#include "pa_sim.h"
#include "pa_info.h"
#include "mrc_qmi.h"
#include "swiQmi.h"
#include "sim_qmi.h"

//--------------------------------------------------------------------------------------------------
// Symbol and Enum definitions.
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Function prototype for the send SIM command API
 */
//--------------------------------------------------------------------------------------------------
typedef le_result_t (*SendSimCommand_t)  (  const char* fileIdentifierPtr,
                                            uint8_t p1,
                                            uint8_t p2,
                                            uint8_t p3,
                                            const uint8_t* dataPtr,
                                            size_t dataNumElements,
                                            const char* pathPtr,
                                            uint8_t* sw1Ptr,
                                            uint8_t* sw2Ptr,
                                            uint8_t* responsePtr,
                                            size_t* responseNumElementsPtr
                                          );


//--------------------------------------------------------------------------------------------------
// Define definitions.
// Define constants as per ICCID coding in ETSI 102.221 and ITU E.118
//--------------------------------------------------------------------------------------------------
#define PA_NATIONAL_CODE_CN          0x68
#define PA_OPERATOR_CODE_CM          0x00
#define PA_OPERATOR_CODE_CM_2        0x20
#define PA_OPERATOR_CODE_CU          0x10
#define PA_NATIONAL_CODE_IDX         0X01
#define PA_OPERATOR_CODE_IDX         0X02

//--------------------------------------------------------------------------------------------------
/**
 * Value reserved to specify that an UIM mapping entry is invalid.
 */
//--------------------------------------------------------------------------------------------------
#define UIM_MAPPING_INVALID          0xFF

// GSM11.11 Extended BCD coding: 'D' - "Wild" value
#define mmt_WildChar    'D'
#define mmt_PhBDtmfPauseChar 'p'


#define BCD2ASC(_x_)  ((_x_ == 0x0A) ? '*'  : (_x_ == 0x0B) ? '#' : (_x_ == 0x0C) \
? mmt_PhBDtmfPauseChar : (_x_ == 0x00) ? mmt_WildChar : (_x_ == 0x0F) ? 0 : (_x_ + '0'))

#define SMSBCD2ASC(_x_)  ((_x_ == 0x0A) ? '*'  : (_x_ == 0x0B) ? '#' : (_x_ == 0x0C)\
? 'A' : (_x_ == 0x0D) ? 'B' : (_x_ == 0x0E) ? 'C' : (_x_ + '0'))

//--------------------------------------------------------------------------------------------------
/**
 * IMSI buffer length.
 */
//--------------------------------------------------------------------------------------------------
#define IMSI_BYTES 20

/// Macro used to generate trace output in this module.
/// Takes the same parameters as LE_DEBUG() et. al.
#define TRACE(...) LE_TRACE(TraceRef, ##__VA_ARGS__)

/// Macro used to query current trace state in this module
#define IS_TRACE_ENABLED LE_IS_TRACE_ENABLED(TraceRef)

//--------------------------------------------------------------------------------------------------
/**
 * Timeout for PIN/PUK operations
 */
//--------------------------------------------------------------------------------------------------
#define TIMEOUT_SEC         20
#define TIMEOUT_USEC        0

//--------------------------------------------------------------------------------------------------
/**
 * Timeout for phonebook operations
 */
//--------------------------------------------------------------------------------------------------
#define PBM_TIMEOUT_SEC     1


//--------------------------------------------------------------------------------------------------
//                                       Static declarations
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Trace reference used for controlling tracing in this module.
 */
//--------------------------------------------------------------------------------------------------
static le_log_TraceRef_t TraceRef;

//--------------------------------------------------------------------------------------------------
/**
 * The DMS (Device Management Service) client.
 * Must be obtained by calling swiQmi_GetClientHandle() before it can be used.
 */
//--------------------------------------------------------------------------------------------------
static qmi_client_type DmsClient;

//--------------------------------------------------------------------------------------------------
/**
 * The UIM (User Identity Module) client.
 * Must be obtained by calling swiQmi_GetClientHandle() before it can be used.
 */
//--------------------------------------------------------------------------------------------------
static qmi_client_type UimClient;

//--------------------------------------------------------------------------------------------------
/**
 * The MGS (M2M General Service) client.
 * Must be obtained by calling swiQmi_GetClientHandle() before it can be used.
 */
//--------------------------------------------------------------------------------------------------
static qmi_client_type MgsClient;

//--------------------------------------------------------------------------------------------------
/**
 * The CAT (Card Application Toolkit) client.
 * Must be obtained by calling swiQmi_GetClientHandle() before it can be used.
 */
//--------------------------------------------------------------------------------------------------
static qmi_client_type CatClient;

//--------------------------------------------------------------------------------------------------
/**
 * The NAS (Network Access Service) client.
 * Must be obtained by calling swiQmi_GetClientHandle() before it can be used.
 */
//--------------------------------------------------------------------------------------------------
static qmi_client_type NasClient;

//--------------------------------------------------------------------------------------------------
/**
 * The PBM (Phonebook Manager) client.
 * Must be obtained by calling swiQmi_GetClientHandle() before it can be used.
 */
//--------------------------------------------------------------------------------------------------
static qmi_client_type PbmClient;

//--------------------------------------------------------------------------------------------------
/**
 * SIM state event ID used to report SIM state events to the registered event handlers.
 */
//--------------------------------------------------------------------------------------------------
static le_event_Id_t SimStateEvent;
static le_event_Id_t InternalSimStateEvent;

//--------------------------------------------------------------------------------------------------
/**
 * SIM Toolkit event ID.
 */
//--------------------------------------------------------------------------------------------------
static le_event_Id_t SimToolkitEvent;
static le_event_Id_t InternalSimToolkitEvent;

//--------------------------------------------------------------------------------------------------
/**
 * Last SIM state event reported by handler.
 */
//--------------------------------------------------------------------------------------------------
static le_sim_States_t LastState = LE_SIM_STATE_UNKNOWN;

//--------------------------------------------------------------------------------------------------
/**
 * UIM Card selected
 */
//--------------------------------------------------------------------------------------------------
static le_sim_Id_t UimSelect = LE_SIM_EXTERNAL_SLOT_1; // external SIM selected by default

//--------------------------------------------------------------------------------------------------
/**
 * Map between le_sim_Id_t and actual UIM indexes.
 * UIM_MAPPING_INVALID is an invalid value (which means that the index is not mapped to anything).
 */
//--------------------------------------------------------------------------------------------------
static uint8_t UimMapping[LE_SIM_ID_MAX];

//--------------------------------------------------------------------------------------------------
/**
 * Last raised SIM Toolkit event
 */
//--------------------------------------------------------------------------------------------------
static pa_sim_StkEvent_t LastStkEvent;

//--------------------------------------------------------------------------------------------------
/**
 * Memory pool for event state parameters.
 */
//--------------------------------------------------------------------------------------------------
static le_mem_PoolRef_t SimStateEventPool;

//--------------------------------------------------------------------------------------------------
/**
 * PIN/PUK operations handler structure.
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    bool inProgress;          ///< PIN/PUK unlocking operation in progress flag
    bool success;             ///< PIN/PUK unlocking operation success flag
    le_sem_Ref_t sem;         ///< Semaphore to synchronize the PIN/PUK unlocking commands with
                              ///< the SIM status sevent handler
}
KeyUnlockHandler_t;

//--------------------------------------------------------------------------------------------------
/**
 * PIN/PUK operations handlers. Each PIN/PUK couple uses a dedicated handler.
 */
//--------------------------------------------------------------------------------------------------
static KeyUnlockHandler_t Pin1Unlock;
static KeyUnlockHandler_t Pin2Unlock;

//--------------------------------------------------------------------------------------------------
/**
 * Mutex to prevent race condition with asynchronous functions when handling PIN/PUK operations.
 */
//--------------------------------------------------------------------------------------------------
static pthread_mutex_t KeyUnlockMutex = PTHREAD_MUTEX_INITIALIZER;

#define LOCK    LE_ASSERT(pthread_mutex_lock(&KeyUnlockMutex) == 0);
#define UNLOCK  LE_ASSERT(pthread_mutex_unlock(&KeyUnlockMutex) == 0);

//--------------------------------------------------------------------------------------------------
/**
 * Refresh registration count.
 */
//--------------------------------------------------------------------------------------------------
static uint8_t RefreshRegisterCount = 0;

//--------------------------------------------------------------------------------------------------
/**
 * Semaphore for phonebook operations.
 */
//--------------------------------------------------------------------------------------------------
static le_sem_Ref_t PbmSem = NULL;

//--------------------------------------------------------------------------------------------------
/**
 * Phonebook record data.
 */
//--------------------------------------------------------------------------------------------------
static pbm_record_instance_type_v01 PbmRecordData;

//--------------------------------------------------------------------------------------------------
/**
 * Enumeration for SIM application type
 *
 */
//--------------------------------------------------------------------------------------------------
typedef enum {
    SIM_UIM_APP_TYPE_SIM = 0,    ///< 2G application
    SIM_UIM_APP_TYPE_USIM,       ///< 3G application
    SIM_UIM_APP_TYPE_RUIM,       ///< CDMA application
    SIM_UIM_APP_TYPE_CSIM,       ///< CDMA application
    SIM_UIM_APP_TYPE_ISIM,       ///< LTE application
    SIM_UNKNOWN_APP_TYPE         ///< Unknown application type
}
SimApplicationType_t;

//--------------------------------------------------------------------------------------------------
/**
 * USIM card path
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    uint16_t   length;         ///< SIM IMSI length
    uint16_t   fileId;         ///< SIM file identifier
    const char pathFile[4];    ///< SIM card path
}
SimImsi_t;

//--------------------------------------------------------------------------------------------------
/**
 * List of all SIM applications.
 */
//--------------------------------------------------------------------------------------------------
static const SimImsi_t SimImsiList[SIM_UNKNOWN_APP_TYPE] =
{
    {9, 0x6F07, {0x3F, 0x00, 0x7F, 0x20}},   // 2G SIM card path
    {9, 0x6F07, {0x3F, 0x00, 0x7F, 0xFF}},   // 3G USIM card path
    {10, 0x6F22, {0x3F, 0x00, 0x7F, 0x25}},  // CDMA RUIM card path
    {10, 0x6F22, {0x3F, 0x00, 0x7F, 0x25}},  // CDMA CSIM card path
    {9, 0x6F07, {0x3F, 0x00, 0x7F, 0xFF}},   // LTE ISIM card path
};

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to fill the file ID field.
 *
 * @return
 *      - LE_OK             Function succeeded.
 *      - LE_FAULT          The function failed.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t FillFileId
(
    const char* fileIdentifierPtr,          ///< [IN] File identifier string
    const char* pathPtr,                    ///< [IN] Path
    uim_file_id_type_v01* fileIdPtr         ///< [OUT] File identifier converted in QMI structure
)
{
    uint8_t len = strlen(pathPtr);
    uint8_t i = 0;
    uint8_t numElem = 0;

    fileIdPtr->file_id = le_hex_HexaToInteger((char*) fileIdentifierPtr);
    LE_DEBUG("file id %d", fileIdPtr->file_id);

    if (0 != (len % 2))
    {
        LE_ERROR("bad path length");
        return LE_FAULT;
    }

    while (i < len)
    {
        char string[5];
        string[0] = pathPtr[i++];
        string[1] = pathPtr[i++];
        string[2] = pathPtr[i++];
        string[3] = pathPtr[i++];
        string[4] = '\0';
        uint16_t value = le_hex_HexaToInteger((char*) string);
        memcpy((fileIdPtr->path+numElem), &value, sizeof(value));
        if (IS_TRACE_ENABLED)
        {
            LE_DEBUG("path 0x%x", value);
        }
        numElem+=2;
    }

    fileIdPtr->path_len = numElem;
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to send a read binary command to the SIM.
 *
 * @return
 *      - LE_OK             Function succeeded.
 *      - LE_FAULT          The function failed.
 *      - LE_BAD_PARAMETER  A parameter is invalid.
 *      - LE_NOT_FOUND      - The function failed to select the SIM card for this operation
 *                          - The requested SIM file is not found
 *      - LE_OVERFLOW       Response buffer is too small to copy the SIM answer.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t SendReadBinaryCommand
(
    const char* fileIdentifierPtr,
        ///< [IN]
        ///< File identifier

    uint8_t p1,
        ///< [IN]
        ///< Parameter P1 passed to the SIM

    uint8_t p2,
        ///< [IN]
        ///< Parameter P2 passed to the SIM

    uint8_t p3,
        ///< [IN]
        ///< Parameter P3 passed to the SIM

    const uint8_t* dataPtr,
        ///< [IN]
        ///< data command.

    size_t dataNumElements,
        ///< [IN]

    const char* pathPtr,
        ///< [IN]
        ///< path of the elementary file

    uint8_t* sw1Ptr,
        ///< [OUT]
        ///< SW1 received from the SIM

    uint8_t* sw2Ptr,
        ///< [OUT]
        ///< SW2 received from the SIM

    uint8_t* responsePtr,
        ///< [OUT]
        ///< SIM response.

    size_t* responseNumElementsPtr
        ///< [INOUT]
)
{
    SWIQMI_DECLARE_MESSAGE(uim_read_transparent_req_msg_v01, transparentReq);
    SWIQMI_DECLARE_MESSAGE(uim_read_transparent_resp_msg_v01, transparentRsp);

    transparentReq.session_information.session_type = UIM_SESSION_TYPE_PRIMARY_GW_V01;
    transparentReq.session_information.aid_len = 0;
    transparentReq.session_information.aid[0] = 0;

    if (FillFileId(fileIdentifierPtr, pathPtr, &transparentReq.file_id) != LE_OK)
    {
        return LE_FAULT;
    }

    transparentReq.read_transparent.offset = p1;
    transparentReq.read_transparent.length = 0;
    transparentReq.encryption_valid = false;
    transparentReq.indication_token_valid = false;

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(UimClient,
                                            QMI_UIM_READ_TRANSPARENT_REQ_V01,
                                            &transparentReq, sizeof(transparentReq),
                                            &transparentRsp, sizeof(transparentRsp),
                                            COMM_UICC_TIMEOUT);

    if (LE_OK == swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_UIM_READ_TRANSPARENT_REQ_V01),
            clientMsgErr, transparentRsp.resp.result, transparentRsp.resp.error))
    {
        if (transparentRsp.card_result_valid)
        {
            *sw1Ptr = transparentRsp.card_result.sw1;
            *sw2Ptr = transparentRsp.card_result.sw2;
        }

        if (transparentRsp.read_result_valid)
        {
            if (transparentRsp.read_result.content_len <= *responseNumElementsPtr)
            {
                *responseNumElementsPtr = transparentRsp.read_result.content_len;
                memcpy(responsePtr, transparentRsp.read_result.content, *responseNumElementsPtr);
            }
            else
            {
                return LE_OVERFLOW;
            }
        }

        return LE_OK;
    }

    switch(transparentRsp.resp.error)
    {
        case QMI_ERR_INVALID_ARG_V01:
            return LE_BAD_PARAMETER;
        case QMI_ERR_SIM_FILE_NOT_FOUND_V01:
            return LE_NOT_FOUND;
        default:
            return LE_FAULT;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to send a read record command to the SIM.
 *
 * @return
 *      - LE_OK             Function succeeded.
 *      - LE_FAULT          The function failed.
 *      - LE_BAD_PARAMETER  A parameter is invalid.
 *      - LE_NOT_FOUND      - The function failed to select the SIM card for this operation
 *                          - The requested SIM file is not found
 *      - LE_OVERFLOW       Response buffer is too small to copy the SIM answer.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t SendReadRecordCommand
(
    const char* fileIdentifierPtr,
        ///< [IN]
        ///< File identifier

    uint8_t p1,
        ///< [IN]
        ///< Parameter P1 passed to the SIM

    uint8_t p2,
        ///< [IN]
        ///< Parameter P2 passed to the SIM

    uint8_t p3,
        ///< [IN]
        ///< Parameter P3 passed to the SIM

    const uint8_t* dataPtr,
        ///< [IN]
        ///< data command.

    size_t dataNumElements,
        ///< [IN]

    const char* pathPtr,
        ///< [IN]
        ///< path of the elementary file

    uint8_t* sw1Ptr,
        ///< [OUT]
        ///< SW1 received from the SIM

    uint8_t* sw2Ptr,
        ///< [OUT]
        ///< SW2 received from the SIM

    uint8_t* responsePtr,
        ///< [OUT]
        ///< SIM response.

    size_t* responseNumElementsPtr
        ///< [INOUT]
)
{
    SWIQMI_DECLARE_MESSAGE(uim_read_record_req_msg_v01, recordReq);
    SWIQMI_DECLARE_MESSAGE(uim_read_record_resp_msg_v01, recordRsp);

    recordReq.session_information.session_type = UIM_SESSION_TYPE_PRIMARY_GW_V01;
    recordReq.session_information.aid_len = 0;
    recordReq.session_information.aid[0] = 0;

    if (FillFileId(fileIdentifierPtr, pathPtr, &recordReq.file_id) != LE_OK)
    {
        return LE_FAULT;
    }

    recordReq.read_record.record = p1;
    recordReq.read_record.length = 0;
    recordReq.last_record_valid = false;
    recordReq.indication_token_valid = false;

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(UimClient,
                                            QMI_UIM_READ_RECORD_REQ_V01,
                                            &recordReq, sizeof(recordReq),
                                            &recordRsp, sizeof(recordRsp),
                                            COMM_UICC_TIMEOUT);

    if (LE_OK == swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_UIM_READ_RECORD_REQ_V01), clientMsgErr,
                recordRsp.resp.result, recordRsp.resp.error))
    {
        if (recordRsp.card_result_valid)
        {
            *sw1Ptr = recordRsp.card_result.sw1;
            *sw2Ptr = recordRsp.card_result.sw2;
        }

        if (recordRsp.read_result_valid)
        {
            if (recordRsp.read_result.content_len <= *responseNumElementsPtr)
            {
                *responseNumElementsPtr = recordRsp.read_result.content_len;
                memcpy(responsePtr, recordRsp.read_result.content, *responseNumElementsPtr);
            }
            else
            {
                return LE_OVERFLOW;
            }
        }

        return LE_OK;
    }

    switch(recordRsp.resp.error)
    {
        case QMI_ERR_INVALID_ARG_V01:
            return LE_BAD_PARAMETER;
        case QMI_ERR_SIM_FILE_NOT_FOUND_V01:
            return LE_NOT_FOUND;
        default:
            return LE_FAULT;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to send an update record command to the SIM.
 *
 * @return
 *      - LE_OK             Function succeeded.
 *      - LE_FAULT          The function failed.
 *      - LE_BAD_PARAMETER  A parameter is invalid.
 *      - LE_NOT_FOUND      - The function failed to select the SIM card for this operation
 *                          - The requested SIM file is not found
 *      - LE_OVERFLOW       Response buffer is too small to copy the SIM answer.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t SendUpdateRecordCommand
(
    const char* fileIdentifierPtr,
        ///< [IN]
        ///< File identifier

    uint8_t p1,
        ///< [IN]
        ///< Parameter P1 passed to the SIM

    uint8_t p2,
        ///< [IN]
        ///< Parameter P2 passed to the SIM

    uint8_t p3,
        ///< [IN]
        ///< Parameter P3 passed to the SIM

    const uint8_t* dataPtr,
        ///< [IN]
        ///< data command.

    size_t dataNumElements,
        ///< [IN]

    const char* pathPtr,
        ///< [IN]
        ///< path of the elementary file

    uint8_t* sw1Ptr,
        ///< [OUT]
        ///< SW1 received from the SIM

    uint8_t* sw2Ptr,
        ///< [OUT]
        ///< SW2 received from the SIM

    uint8_t* responsePtr,
        ///< [OUT]
        ///< SIM response.

    size_t* responseNumElementsPtr
        ///< [INOUT]
)
{
    SWIQMI_DECLARE_MESSAGE(uim_write_record_req_msg_v01, recordReq);
    SWIQMI_DECLARE_MESSAGE(uim_write_record_resp_msg_v01, recordRsp);

    recordReq.session_information.session_type = UIM_SESSION_TYPE_PRIMARY_GW_V01;
    recordReq.session_information.aid_len = 0;
    recordReq.session_information.aid[0] = 0;

    if (FillFileId(fileIdentifierPtr, pathPtr, &recordReq.file_id) != LE_OK)
    {
        return LE_FAULT;
    }

    if (dataNumElements > QMI_UIM_CONTENT_RECORD_MAX_V01)
    {
        return LE_FAULT;
    }

    recordReq.write_record.record = p1;
    recordReq.write_record.data_len = dataNumElements;
    memcpy(recordReq.write_record.data, dataPtr, dataNumElements);

    recordReq.indication_token_valid = false;

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(UimClient,
                                            QMI_UIM_WRITE_RECORD_REQ_V01,
                                            &recordReq, sizeof(recordReq),
                                            &recordRsp, sizeof(recordRsp),
                                            COMM_UICC_TIMEOUT);

    if (LE_OK == swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_UIM_WRITE_RECORD_REQ_V01),
                    clientMsgErr, recordRsp.resp.result, recordRsp.resp.error))
    {
        if (recordRsp.card_result_valid)
        {
            *sw1Ptr = recordRsp.card_result.sw1;
            *sw2Ptr = recordRsp.card_result.sw2;
        }
        return LE_OK;
    }

    switch(recordRsp.resp.error)
    {
        case QMI_ERR_INVALID_ARG_V01:
            return LE_BAD_PARAMETER;
        case QMI_ERR_SIM_FILE_NOT_FOUND_V01:
            return LE_NOT_FOUND;
        default:
            return LE_FAULT;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to send an update binary command to the SIM.
 *
 * @return
 *      - LE_OK             Function succeeded.
 *      - LE_FAULT          The function failed.
 *      - LE_BAD_PARAMETER  A parameter is invalid.
 *      - LE_NOT_FOUND      - The function failed to select the SIM card for this operation
 *                          - The requested SIM file is not found
 *      - LE_OVERFLOW       Response buffer is too small to copy the SIM answer.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t SendUpdateBinaryCommand
(
    const char* fileIdentifierPtr,
        ///< [IN]
        ///< File identifier

    uint8_t p1,
        ///< [IN]
        ///< Parameter P1 passed to the SIM

    uint8_t p2,
        ///< [IN]
        ///< Parameter P2 passed to the SIM

    uint8_t p3,
        ///< [IN]
        ///< Parameter P3 passed to the SIM

    const uint8_t* dataPtr,
        ///< [IN]
        ///< data command.

    size_t dataNumElements,
        ///< [IN]

    const char* pathPtr,
        ///< [IN]
        ///< path of the elementary file

    uint8_t* sw1Ptr,
        ///< [OUT]
        ///< SW1 received from the SIM

    uint8_t* sw2Ptr,
        ///< [OUT]
        ///< SW2 received from the SIM

    uint8_t* responsePtr,
        ///< [OUT]
        ///< SIM response.

    size_t* responseNumElementsPtr
        ///< [INOUT]
)
{
    SWIQMI_DECLARE_MESSAGE(uim_write_transparent_req_msg_v01, transparentReq);
    SWIQMI_DECLARE_MESSAGE(uim_write_transparent_resp_msg_v01, transparentRsp);

    transparentReq.session_information.session_type = UIM_SESSION_TYPE_PRIMARY_GW_V01;
    transparentReq.session_information.aid_len = 0;
    transparentReq.session_information.aid[0] = 0;

    if (FillFileId(fileIdentifierPtr, pathPtr, &transparentReq.file_id) != LE_OK)
    {
        return LE_FAULT;
    }

    if (dataNumElements > QMI_UIM_CONTENT_TRANSPARENT_MAX_V01)
    {
        return LE_FAULT;
    }

    transparentReq.write_transparent.offset = p1;
    transparentReq.write_transparent.data_len = dataNumElements;
    memcpy(transparentReq.write_transparent.data, dataPtr, dataNumElements);

    transparentReq.indication_token_valid = false;

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(UimClient,
                                            QMI_UIM_WRITE_TRANSPARENT_REQ_V01,
                                            &transparentReq, sizeof(transparentReq),
                                            &transparentRsp, sizeof(transparentRsp),
                                            COMM_UICC_TIMEOUT);

    if (LE_OK == swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_UIM_WRITE_TRANSPARENT_REQ_V01),
        clientMsgErr, transparentRsp.resp.result, transparentRsp.resp.error))
    {
        if (transparentRsp.card_result_valid)
        {
            *sw1Ptr = transparentRsp.card_result.sw1;
            *sw2Ptr = transparentRsp.card_result.sw2;
        }

        return LE_OK;
    }

    switch(transparentRsp.resp.error)
    {
        case QMI_ERR_INVALID_ARG_V01:
            return LE_BAD_PARAMETER;
        case QMI_ERR_SIM_FILE_NOT_FOUND_V01:
            return LE_NOT_FOUND;
        default:
            return LE_FAULT;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Maps the QMI SIM state to the Legato SIM states.
 *
 * @todo Currently we just use the card state but maybe we should use a combination of the card
 * state and the PIN1 state.  Will also need more documentation from Qualcomm that describes the QMI
 * states.
 */
//--------------------------------------------------------------------------------------------------
static le_sim_States_t MapSimState
(
    uint8_t simIndex,
    uim_get_card_status_resp_msg_v01 * cardStatusMsg
)
{
    LE_DEBUG("QMI SIM index %d", simIndex);

    // Debug trace
    if (cardStatusMsg->card_status_validity_valid)
    {
        LE_DEBUG("Card Status Validity %d"
                , (int)cardStatusMsg->card_status_validity[simIndex]);
    }

    if (cardStatusMsg->card_status_valid)
    {
        LE_DEBUG("SIM state %d (err %d)",
                cardStatusMsg->card_status.card_info[simIndex].card_state,
                cardStatusMsg->card_status.card_info[simIndex].error_code);
        LE_DEBUG("PIN1 state %d - PIN2 state %d",
                cardStatusMsg->card_status.card_info[simIndex].app_info[0].pin1.pin_state,
                cardStatusMsg->card_status.card_info[simIndex].app_info[0].pin2.pin_state);
    }

    // Check if the SIM is Busy
    if (cardStatusMsg->sim_busy_status_valid)
    {
        LE_DEBUG("SIM busy %d",
                 (int)cardStatusMsg->sim_busy_status[simIndex]);
        if (cardStatusMsg->sim_busy_status[simIndex])
        {
            LE_INFO("return SIM busy");
            return LE_SIM_BUSY;
        }
    }

    // Retrieve SIM state from QMI message
    if (cardStatusMsg->card_status_valid)
    {
        switch (cardStatusMsg->card_status.card_info[simIndex].card_state)
        {
            case UIM_CARD_STATE_ABSENT_V01:
            {
                return LE_SIM_ABSENT;
            }
            break;

            case UIM_CARD_STATE_PRESENT_V01:
            {
                if (0 == cardStatusMsg->card_status.card_info[simIndex].app_info_len)
                {
                    LE_ERROR("SIM application is missing (internal error)");
                    return LE_SIM_STATE_UNKNOWN;
                }
                else
                {
                    if (cardStatusMsg->card_status.card_info[simIndex].app_info_len > 1)
                    {
                        LE_WARN("More than one application %d",
                                    cardStatusMsg->card_status.card_info[simIndex].app_info_len);
                    }

                    switch (
                          cardStatusMsg->card_status.card_info[simIndex].app_info[0].pin1.pin_state)
                    {
                        case UIM_PIN_STATE_ENABLED_NOT_VERIFIED_V01:
                        {
                            return LE_SIM_INSERTED;
                        }
                        break;
                        case UIM_PIN_STATE_DISABLED_V01:
                        case UIM_PIN_STATE_ENABLED_VERIFIED_V01:
                        {
                            return LE_SIM_READY;
                        }
                        break;
                        case UIM_PIN_STATE_BLOCKED_V01:
                        case UIM_PIN_STATE_PERMANENTLY_BLOCKED_V01:
                        {
                            return LE_SIM_BLOCKED;
                        }
                        break;
                        case UIM_PIN_STATE_UNKNOWN_V01:
                        {
                            return LE_SIM_INSERTED;
                        }
                        break;
                        default:
                        {
                            LE_ERROR("Unknown SIM state %d)",
                                     cardStatusMsg->card_status.card_info[simIndex].upin.pin_state);
                            return LE_SIM_STATE_UNKNOWN;
                        }
                        break;
                    }
                }
            }
            break;

            case UIM_CARD_STATE_ERROR_V01:
            {
                switch (cardStatusMsg->card_status.card_info[simIndex].error_code)
                {
                    case UIM_CARD_ERROR_CODE_NO_ATR_RECEIVED_V01:
                    {
                        // When SIM is not inserted, that error code is received:
                        // UIM_CARD_ERROR_CODE_NO_ATR_RECEIVED (0x03) --  No ATR received
                        // Confirmed by SHZ- Ticket TRAC
                        return LE_SIM_ABSENT;
                    }
                    break;
                    case UIM_CARD_ERROR_CODE_POWER_DOWN_V01:
                    {
                        //SIM is powered down
                        return LE_SIM_POWER_DOWN;
                    }
                    break;
                    default:
                    {
                        LE_ERROR("Error %d for SIM state",
                             cardStatusMsg->card_status.card_info[simIndex].error_code);
                        return LE_SIM_STATE_UNKNOWN;
                    }
                    break;
                }
            }
            break;

            default:
            {
                LE_ERROR("Unknown SIM state %d)",
                         cardStatusMsg->card_status.card_info[simIndex].card_state);
                return LE_SIM_STATE_UNKNOWN;
            }
            break;
        }
    }

    return LE_SIM_STATE_UNKNOWN;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set Pin protection
 *
 *  @return
 *          LE_BAD_PARAMETER
 *          LE_FAULT
 *          LE_OK
 */
//--------------------------------------------------------------------------------------------------
static le_result_t SetPinProtection
(
    bool               enablePin,   ///< [IN] true to enable Pin, false to disable PIN.
    pa_sim_PinType_t   type,        ///< [IN] The pin type
    const pa_sim_Pin_t code         ///< [IN] code
)
{
    SWIQMI_DECLARE_MESSAGE(uim_set_pin_protection_req_msg_v01, pinProtectionReq);
    SWIQMI_DECLARE_MESSAGE(uim_set_pin_protection_resp_msg_v01, pinProtectionResp);

    LE_DEBUG("Pin state %d, type %d"
            , enablePin
            , type);

    // Indicates the PIN ID to be enabled or disabled
    switch (type)
    {
        case PA_SIM_PIN:
            pinProtectionReq.set_pin_protection.pin_id = UIM_PIN_ID_PIN_1_V01;
            break;

        case PA_SIM_PIN2:
            pinProtectionReq.set_pin_protection.pin_id = UIM_PIN_ID_PIN_2_V01;
            break;

        default:
            return LE_BAD_PARAMETER;
    }

    // Set the PIN operation
    if (enablePin)
    {
        pinProtectionReq.set_pin_protection.pin_operation = UIM_PIN_OPERATION_ENABLE_V01;
    }
    else
    {
        pinProtectionReq.set_pin_protection.pin_operation = UIM_PIN_OPERATION_DISABLE_V01;
    }

    // Set the PIN value
    LE_ASSERT(le_utf8_Copy((char*)pinProtectionReq.set_pin_protection.pin_value,
                           code,
                           PA_SIM_PIN_MAX_LEN+1,
                           &(pinProtectionReq.set_pin_protection.pin_value_len)) == LE_OK);


    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(UimClient,
                                            QMI_UIM_SET_PIN_PROTECTION_REQ_V01,
                                            &pinProtectionReq, sizeof(pinProtectionReq),
                                            &pinProtectionResp, sizeof(pinProtectionResp),
                                            COMM_UICC_TIMEOUT);

    return swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_UIM_SET_PIN_PROTECTION_RESP_V01), clientMsgErr,
        pinProtectionResp.resp.result, pinProtectionResp.resp.error);
}

//--------------------------------------------------------------------------------------------------
/**
 * Handler function called when there is a SIM Toolkit indication.
 */
//--------------------------------------------------------------------------------------------------
static void SimToolkitHandler
(
    void*           indBufPtr,  ///< [IN] The indication message data.
    unsigned int    indBufLen , ///< [IN] The indication message data length in bytes.
    void*           contextPtr  ///< [IN] Context value that was passed to
                                ///       swiQmi_AddIndicationHandler().
)
{
    LE_DEBUG("SIM Toolkit indication received");

    qmi_client_error_type clientMsgErr;
    SWIQMI_DECLARE_MESSAGE(cat_event_report_ind_msg_v02, catReport);

    clientMsgErr = qmi_client_message_decode(CatClient,
                                             QMI_IDL_INDICATION,
                                             QMI_CAT_EVENT_REPORT_IND_V02,
                                             indBufPtr, indBufLen,
                                             &catReport, sizeof(catReport));
    if (QMI_NO_ERR != clientMsgErr)
    {
        LE_ERROR("Error in decoding QMI_CAT_EVENT_REPORT_IND_V02, error = %d", clientMsgErr);
        return;
    }

    LE_DEBUG("Decoded header valid: %c", (catReport.decoded_header_valid ? 'Y' : 'N'));
    if (!catReport.decoded_header_valid)
    {
        LE_INFO("Decoded header is not valid");
        return;
    }

    LE_DEBUG("Decoded header: command_id=0x%X, uim_ref_id=%d, command_number=%d",
             catReport.decoded_header.command_id,
             catReport.decoded_header.uim_ref_id,
             catReport.decoded_header.command_number);

    if (CAT_COMMAND_ID_OPEN_CHANNEL_V02 == catReport.decoded_header.command_id)
    {
        LastStkEvent.simId = UimSelect;
        LastStkEvent.stkEvent = LE_SIM_OPEN_CHANNEL;
        LastStkEvent.stkRefreshMode = LE_SIM_REFRESH_MODE_MAX;
        LastStkEvent.stkRefreshStage = LE_SIM_STAGE_MAX;

        LE_DEBUG("STK event LE_SIM_OPEN_CHANNEL reported for card %d", LastStkEvent.simId);
        le_event_Report(SimToolkitEvent, &LastStkEvent, sizeof(LastStkEvent));
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Handler function called when there is a SIM Refresh indication.
 */
//--------------------------------------------------------------------------------------------------
static void SimRefreshHandler
(
    void*           indBufPtr,  ///< [IN] The indication message data.
    unsigned int    indBufLen , ///< [IN] The indication message data length in bytes.
    void*           contextPtr  ///< [IN] Context value that was passed to
                                ///       swiQmi_AddIndicationHandler().
)
{
    LE_DEBUG("SIM Refresh indication received");

    qmi_client_error_type clientMsgErr;
    SWIQMI_DECLARE_MESSAGE(uim_refresh_ind_msg_v01, refreshInd);

    clientMsgErr = qmi_client_message_decode(UimClient,
                                             QMI_IDL_INDICATION,
                                             QMI_UIM_REFRESH_IND_V01,
                                             indBufPtr, indBufLen,
                                             &refreshInd, sizeof(refreshInd));
    if (QMI_NO_ERR != clientMsgErr)
    {
        LE_ERROR("Error in decoding QMI_UIM_REFRESH_IND_V01, error = %d", clientMsgErr);
        return;
    }

    LE_DEBUG("Refresh event valid: %c", (refreshInd.refresh_event_valid ? 'Y' : 'N'));
    if (!refreshInd.refresh_event_valid)
    {
        LE_INFO("Refresh event is not valid");
        return;
    }

    LE_DEBUG("Refresh event: stage=0x%X, mode=0x%X",
             refreshInd.refresh_event.stage,
             refreshInd.refresh_event.mode);

    // Tables used to convert QMI values to Legato values
    const le_sim_StkRefreshMode_t refreshMode[] =
    {
        LE_SIM_REFRESH_RESET,           // UIM_REFRESH_MODE_RESET_V01
        LE_SIM_REFRESH_INIT,            // UIM_REFRESH_MODE_INIT_V01
        LE_SIM_REFRESH_INIT_FCN,        // UIM_REFRESH_MODE_INIT_FCN_V01
        LE_SIM_REFRESH_FCN,             // UIM_REFRESH_MODE_FCN_V01
        LE_SIM_REFRESH_INIT_FULL_FCN,   // UIM_REFRESH_MODE_INIT_FULL_FCN_V01
        LE_SIM_REFRESH_APP_RESET,       // UIM_REFRESH_MODE_APP_RESET_V01
        LE_SIM_REFRESH_SESSION_RESET    // UIM_REFRESH_MODE_3G_RESET_V01
    };
    const le_sim_StkRefreshStage_t refreshStage[] =
    {
        LE_SIM_STAGE_WAITING_FOR_OK,    // UIM_REFRESH_STAGE_WAIT_FOR_OK_V01
        LE_SIM_STAGE_MAX,               // UIM_REFRESH_STAGE_START_V01
        LE_SIM_STAGE_END_WITH_SUCCESS,  // UIM_REFRESH_STAGE_END_WITH_SUCCESS_V01
        LE_SIM_STAGE_END_WITH_FAILURE   // UIM_REFRESH_STAGE_END_WITH_FAILURE_V01
    };

    LastStkEvent.simId = UimSelect;
    LastStkEvent.stkEvent = LE_SIM_REFRESH;
    LastStkEvent.stkRefreshMode = refreshMode[refreshInd.refresh_event.mode];
    LastStkEvent.stkRefreshStage = refreshStage[refreshInd.refresh_event.stage];


    switch (refreshInd.refresh_event.stage)
    {
        case UIM_REFRESH_STAGE_START_V01:
            // This event is an internal state and will be processed locally.
            // It should not be reported to legato API.
            le_event_Report(InternalSimToolkitEvent, NULL, 0);
            break;

        case UIM_REFRESH_STAGE_WAIT_FOR_OK_V01:
        case UIM_REFRESH_STAGE_END_WITH_SUCCESS_V01:
        {
            LE_DEBUG("STK event LE_SIM_REFRESH reported for card %d", LastStkEvent.simId);
            le_event_Report(SimToolkitEvent, &LastStkEvent, sizeof(LastStkEvent));
        }
        break;

        case UIM_REFRESH_STAGE_END_WITH_FAILURE_V01:
        {
            pa_sim_Event_t* eventPtr = le_mem_ForceAlloc(SimStateEventPool);

            eventPtr->simId = UimSelect;
            eventPtr->state = LE_SIM_ABSENT;

            LE_DEBUG("STK event LE_SIM_REFRESH failed, send SIM state %d for card %d",
                     eventPtr->state, eventPtr->simId);
            le_event_ReportWithRefCounting(SimStateEvent, eventPtr);
        }
        break;

        default:
            LE_ERROR("Unrecognized STK event: %d", refreshInd.refresh_event.stage);
            break;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Get active UIM Slots
 *
 * @return
 * - LE_OK if successful
 * - LE_FAULT if cannot get the active UIM Slots
 */
//--------------------------------------------------------------------------------------------------
static le_result_t GetUimSlots
(
    uint8_t* simSlotsPtr
)
{
    uim_get_slots_status_resp_msg_v01 resp = { {0} };
    int i = 0;
    int validSlots = 0;

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(UimClient,
                                                QMI_UIM_GET_SLOTS_STATUS_REQ_V01,
                                                NULL, 0,
                                                &resp, sizeof(resp),
                                                COMM_DEFAULT_PLATFORM_TIMEOUT);


    if (LE_OK == swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_UIM_GET_SLOTS_STATUS_REQ_V01),
                                      clientMsgErr,
                                      resp.resp.result,
                                      resp.resp.error))
    {
        if (resp.physical_slot_status_valid)
        {
            LE_DEBUG("physical_slot_status_len.%d (QMI value)", resp.physical_slot_status_len);

            for ( i=0; i<resp.physical_slot_status_len; i++)
            {
                if (IS_TRACE_ENABLED)
                {
                    LE_DEBUG("physical_slot_state[%d].%d (QMI value)",
                             i, resp.physical_slot_status[i].physical_slot_state);
                }

                if (UIM_PHYSICAL_SLOT_STATE_ACTIVE_V01 ==
                   resp.physical_slot_status[i].physical_slot_state)
                {
                    validSlots++;

                    if (IS_TRACE_ENABLED)
                    {
                        LE_DEBUG("logical_slot.%d (QMI value)",
                                 resp.physical_slot_status[i].logical_slot);
                        LE_DEBUG("physical_card_status.%d (QMI value)",
                                 resp.physical_slot_status[i].physical_card_status);
                        LE_DEBUG("iccid.%s (QMI value)",
                                 resp.physical_slot_status[i].iccid);
                    }
                }
            }
        }
        *simSlotsPtr = validSlots;
        return LE_OK;
    }

    LE_ERROR("Cannot get the SIM state");
    return LE_FAULT;
}


//--------------------------------------------------------------------------------------------------
/**
 * Translate SIM type in QMI SIM identifier
 * @return
 * - LE_OK if successful
 * - LE_FAULT if the given sim type parameter is unknown
 */
//--------------------------------------------------------------------------------------------------
static le_result_t TranslateSimId
(
    le_sim_Id_t   simId,          ///< SIM identifier
    uint8_t*      simIdPtr        ///< Actual SIM index
)
{
    if (simId >= LE_SIM_ID_MAX)
    {
        LE_ERROR("SIM id %d is out of range", simId);
        return LE_FAULT;
    }

    if (UIM_MAPPING_INVALID == UimMapping[simId])
    {
        LE_INFO("SIM id %d not mapped on platform", simId);
        return LE_FAULT;
    }

    *simIdPtr = UimMapping[simId];
    return LE_OK;
}


//--------------------------------------------------------------------------------------------------
/**
 * Translate SIM Id from active slots.
 *
 * @return
 * - LE_OK if successful
 * - LE_FAULT if the given SIM Id parameter is unknown
 */
//--------------------------------------------------------------------------------------------------
static le_result_t TranslateSimIdFromActiveSlots
(
    le_sim_Id_t   simId,
    uint8_t*      simIdPtr
)
{
    uint8_t activeSlots = 0;
    le_result_t res;

    res = GetUimSlots(&activeSlots);

    if (res == LE_OK)
    {
        if(activeSlots <= 1)
        {
            *simIdPtr = 0;
        }
        else
        {
            res = TranslateSimId(simId, simIdPtr);
        }
    }

    return res;
}

//--------------------------------------------------------------------------------------------------
/**
 * Handler function called when there is a UIM indication.  This handler reports an event to users
 * when there is a state change.
 */
//--------------------------------------------------------------------------------------------------
static void UimStateChangeHandler
(
    void*           indBufPtr,  ///< [IN] The indication message data.
    unsigned int    indBufLen , ///< [IN] The indication message data length in bytes.
    void*           contextPtr  ///< [IN] Context value that was passed to
                                ///       swiQmi_AddIndicationHandler().
)
{
    uint8_t simIndex;
    SWIQMI_DECLARE_MESSAGE(uim_status_change_ind_msg_v01, uimReport);

    qmi_client_error_type clientMsgErr = qmi_client_message_decode(UimClient,
                                                                   QMI_IDL_INDICATION,
                                                                   QMI_UIM_STATUS_CHANGE_IND_V01,
                                                                   indBufPtr, indBufLen,
                                                                   &uimReport, sizeof(uimReport));
    if (QMI_NO_ERR != clientMsgErr)
    {
        LE_ERROR("Error in decoding QMI_UIM_STATUS_CHANGE_IND_V01, error = %d", clientMsgErr);
        return;
    }

    if (uimReport.card_status_valid)
    {
        if (LE_OK != TranslateSimId(UimSelect, &simIndex))
        {
            LE_ERROR("Unable to translate SIM Identifier!");
            return;
        }

        if (simIndex > uimReport.card_status.card_info_len)
        {
            LE_ERROR("Current SIM is not mentionned in the event report");
            return;
        }

        if (uimReport.card_status.card_info[simIndex].app_info_len)
        {
            app_info_type_v01 app = uimReport.card_status.card_info[simIndex].app_info[0];

            LOCK
            if (Pin1Unlock.inProgress)
            {
                // UIM_PIN_STATE_ENABLED_VERIFIED_V01 is reported when PIN1 is correct or when
                // PUK1 is used to change PIN code.
                if (UIM_PIN_STATE_ENABLED_VERIFIED_V01 == app.pin1.pin_state)
                {
                    Pin1Unlock.success = true;
                    Pin1Unlock.inProgress = false;
                    le_sem_Post(Pin1Unlock.sem);
                }
                LE_INFO("UIM PIN1 state changed: %d", app.pin1.pin_state);
            }
            if (Pin2Unlock.inProgress)
            {
                // UIM_PIN_STATE_ENABLED_VERIFIED_V01 is reported when PIN2 is correct or when
                // PUK2 is used to change PIN code.
                if (UIM_PIN_STATE_ENABLED_VERIFIED_V01 == app.pin2.pin_state)
                {
                    Pin2Unlock.success = true;
                    Pin2Unlock.inProgress = false;
                    le_sem_Post(Pin2Unlock.sem);
                }
                LE_INFO("UIM PIN2 state changed: %d", app.pin2.pin_state);
            }
            UNLOCK
        }

        le_event_Report(InternalSimStateEvent, NULL, 0);
    }
}




//--------------------------------------------------------------------------------------------------
/**
 * Handler called when the state of the SIM is changing
 *
 */
//--------------------------------------------------------------------------------------------------
static void InternalStateChangeHandler
(
    void* reportPtr
)
{
    le_sim_States_t simState;
    le_sim_Id_t  simId;

    if (pa_sim_GetState(&simState) != LE_OK)
    {
        LE_WARN("Could not get sim state");
        return;
    }

    if (pa_sim_GetSelectedCard(&simId) != LE_OK)
    {
        LE_WARN("Could not get selected card");
        return;
    }

    if (( LastState != simState ) || ( simId != UimSelect ))
    {
        // Build the data for the event report.
        pa_sim_Event_t* eventPtr = le_mem_ForceAlloc(SimStateEventPool);
        eventPtr->simId = simId;
        eventPtr->state = simState;
        // @todo: This needs to change to le_event_ReportWithPoolObj (yet to be implemented).
        LE_DEBUG("Send Event SIM identifier %d, SIM state %d", eventPtr->simId, eventPtr->state);
        le_event_ReportWithRefCounting(SimStateEvent, eventPtr);
    }
    LastState = simState;
    UimSelect = simId;
}

//--------------------------------------------------------------------------------------------------
/**
 * Handler called when an internal refresh stage is received
 *
 */
//--------------------------------------------------------------------------------------------------
static void InternalSimToolkitHandler
(
    void* reportPtr
)
{
    // Event confirmation
    SWIQMI_DECLARE_MESSAGE(uim_refresh_complete_req_msg_v01, refreshReq);
    SWIQMI_DECLARE_MESSAGE(uim_refresh_complete_resp_msg_v01, refreshResp);

    refreshReq.session_information.session_type = UIM_SESSION_TYPE_PRIMARY_GW_V01;
    refreshReq.session_information.aid_len = 0;
    refreshReq.refresh_success = 1;

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(UimClient,
                                            QMI_UIM_REFRESH_COMPLETE_REQ_V01,
                                            &refreshReq, sizeof(refreshReq),
                                            &refreshResp, sizeof(refreshResp),
                                            COMM_DEFAULT_PLATFORM_TIMEOUT);

    LE_INFO("QMI_UIM_REFRESH_COMPLETE_REQ_V01: Err %d resp %d", clientMsgErr,
            refreshResp.resp.result);

    if( swiQmi_CheckResponseCode(STRINGIZE_EXPAND(QMI_UIM_REFRESH_COMPLETE_REQ_V01),
                                clientMsgErr,
                                refreshResp.resp) != LE_OK)
    {
        LE_ERROR("SIM Refresh complete request failed");
    }
    else
    {
        LE_DEBUG("SIM Refresh complete request success");
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * This function enables/disables the SIM Refresh confirmation
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
static le_result_t EnableRefreshConfirmation
(
    bool    enable
)
{
    SWIQMI_DECLARE_MESSAGE(uim_refresh_register_all_req_msg_v01, refreshReq);
    SWIQMI_DECLARE_MESSAGE(uim_refresh_register_all_resp_msg_v01, refreshResp);

    if (enable)
    {
        refreshReq.register_for_refresh = 1;
        refreshReq.vote_for_init_valid = 1;
        refreshReq.vote_for_init = 1; // Client votes for initialization
    }
    else
    {
        refreshReq.register_for_refresh = 0;
        refreshReq.vote_for_init_valid = 1;
        refreshReq.vote_for_init = 0; // Client does not vote for initialization
    }

    refreshReq.session_information.session_type = UIM_SESSION_TYPE_PRIMARY_GW_V01;
    refreshReq.session_information.aid_len = 0;

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(UimClient,
                                            QMI_UIM_REFRESH_REGISTER_ALL_REQ_V01,
                                            &refreshReq, sizeof(refreshReq),
                                            &refreshResp, sizeof(refreshResp),
                                            COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_UIM_REFRESH_REGISTER_ALL_REQ_V01),
                                    clientMsgErr,
                                    refreshResp.resp.result,
                                    refreshResp.resp.error) != LE_OK)
    {
        LE_ERROR("Cannot register for Sim Refresh events");
        return LE_FAULT;
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Handler function called when Phonebook Manager record read indication is received.
 */
//--------------------------------------------------------------------------------------------------
static void PbmRecordReadHandler
(
    void*           indBufPtr,  ///< [IN] The indication message data.
    unsigned int    indBufLen , ///< [IN] The indication message data length in bytes.
    void*           contextPtr  ///< [IN] Context value that was passed to
                                ///       swiQmi_AddIndicationHandler().
)
{
    qmi_client_error_type clientMsgErr;
    SWIQMI_DECLARE_MESSAGE(pbm_record_read_ind_msg_v01, recordReadInd);

    clientMsgErr = qmi_client_message_decode(PbmClient,
                                             QMI_IDL_INDICATION,
                                             QMI_PBM_RECORD_READ_IND_V01,
                                             indBufPtr, indBufLen,
                                             &recordReadInd, sizeof(recordReadInd));
    if (QMI_NO_ERR != clientMsgErr)
    {
        LE_ERROR("Error in decoding QMI_PBM_RECORD_READ_IND_V01, error=%d", clientMsgErr);
        return;
    }

    LE_DEBUG("Record read indication: session type=%d, phonebook type=%d, number of instances=%d",
             recordReadInd.basic_record_data.session_type,
             recordReadInd.basic_record_data.pb_type,
             recordReadInd.basic_record_data.record_instances_len);

    if (!recordReadInd.basic_record_data.record_instances_len)
    {
        LE_ERROR("No record instances!");
        return;
    }

    // Only copy the first record instance, this should be adapted if multiple records are read
    memcpy(&PbmRecordData,
           &recordReadInd.basic_record_data.record_instances[0],
           sizeof(PbmRecordData));

    // Post the semaphore indicating that the record is read
    if (PbmSem)
    {
        le_sem_Post(PbmSem);
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * This function initialize the mapping between le_sim_Id_t and modem SIM indexes
 */
//--------------------------------------------------------------------------------------------------
static void InitUimIndexMapping
(
    void
)
{
    le_sim_Id_t simId;

    for (simId = LE_SIM_EMBEDDED; simId < LE_SIM_ID_MAX; simId++)
    {
        UimMapping[simId] = UIM_MAPPING_INVALID;
    }

    // UIM mapping in firmware side is done as follows:
    // - 0: First external UIM interface
    // - 1: Embedded UIMor second external UIM interface
    // - 2: Remote UIM

    // On all platforms index 0 is associated with the first external slot.
    UimMapping[LE_SIM_EXTERNAL_SLOT_1] = 0;

    // So far platforms that support the usage of a remote SIM are using index 2.
    UimMapping[LE_SIM_REMOTE] = 2;

#if defined (EUICC)
    // On platforms that supports embedded SIM, the index 1 is used.
    LE_DEBUG("LE_SIM_EMBEDDED SIM type available");
    UimMapping[LE_SIM_EMBEDDED] = 1;
#else
    // On other platforms index 1 is used for the second external slot.
    UimMapping[LE_SIM_EXTERNAL_SLOT_2] = 1;
#endif
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to initialize the QMI SIM module.
 *
 * @return
 *      LE_OK if successful.
 *      LE_FAULT if unsuccessful.
 */
//--------------------------------------------------------------------------------------------------
le_result_t sim_qmi_Init
(
    void
)
{
    qmi_client_error_type clientMsgErr;

    // Get a reference to the trace keyword that is used to control tracing in this module.
    TraceRef = le_log_GetTraceRef("sim");

    // Create the event for signaling user handlers.
    SimStateEvent = le_event_CreateIdWithRefCounting("SimStateEvent");
    InternalSimStateEvent = le_event_CreateId("InternalSimStateEvent", 0);
    SimStateEventPool = le_mem_CreatePool("SimEventPool", sizeof(pa_sim_Event_t));

    // Init UIM mapping to perform PA <=> modem SIM indexes translations
    InitUimIndexMapping();

    // SIM Toolkit
    LastStkEvent.stkEvent = LE_SIM_STK_EVENT_MAX;
    LastStkEvent.stkRefreshMode = LE_SIM_REFRESH_MODE_MAX;
    LastStkEvent.stkRefreshStage = LE_SIM_STAGE_MAX;

    // Create an event Id for SIM Toolkit notifications
    SimToolkitEvent = le_event_CreateId("SimToolkitEvent", sizeof(pa_sim_StkEvent_t));
    InternalSimToolkitEvent = le_event_CreateId("InternalSimToolkitEvent", 0);

    // QMI services initialization
    if (LE_OK != swiQmi_InitServices(QMI_SERVICE_DMS))
    {
        LE_CRIT("QMI_SERVICE_DMS cannot be initialized.");
        return LE_FAULT;
    }
    DmsClient = swiQmi_GetClientHandle(QMI_SERVICE_DMS);
    if (!DmsClient)
    {
        return LE_FAULT;
    }

    if (LE_OK != swiQmi_InitServices(QMI_SERVICE_UIM))
    {
        LE_CRIT("QMI_SERVICE_UIM cannot be initialized.");
        return LE_FAULT;
    }
    UimClient = swiQmi_GetClientHandle(QMI_SERVICE_UIM);
    if (!UimClient)
    {
        return LE_FAULT;
    }

    if (LE_OK != swiQmi_InitServices(QMI_SERVICE_MGS))
    {
        LE_CRIT("QMI_SERVICE_MGS cannot be initialized.");
        return LE_FAULT;
    }
    MgsClient = swiQmi_GetClientHandle(QMI_SERVICE_MGS);
    if (!MgsClient)
    {
        return LE_FAULT;
    }

    if (LE_OK != swiQmi_InitServices(QMI_SERVICE_CAT))
    {
        LE_CRIT("QMI_SERVICE_CAT cannot be initialized.");
        return LE_FAULT;
    }
    CatClient = swiQmi_GetClientHandle(QMI_SERVICE_CAT);
    if (!CatClient)
    {
        return LE_FAULT;
    }

    if (LE_OK != swiQmi_InitServices(QMI_SERVICE_NAS))
    {
        LE_CRIT("QMI_SERVICE_NAS cannot be initialized.");
        return LE_FAULT;
    }
    NasClient = swiQmi_GetClientHandle(QMI_SERVICE_NAS);
    if (!NasClient)
    {
        return LE_FAULT;
    }

    if (LE_OK != swiQmi_InitServices(QMI_SERVICE_PBM))
    {
        LE_CRIT("QMI_SERVICE_PBM cannot be initialized.");
        return LE_FAULT;
    }
    PbmClient = swiQmi_GetClientHandle(QMI_SERVICE_PBM);
    if (!PbmClient)
    {
        return LE_FAULT;
    }

    // Register our own handler with the QMI CAT service.
    swiQmi_AddIndicationHandler(SimToolkitHandler,
                                QMI_SERVICE_CAT,
                                QMI_CAT_EVENT_REPORT_IND_V02,
                                NULL);

    SWIQMI_DECLARE_MESSAGE(cat_get_configuration_resp_msg_v02, getCfgResp);

    clientMsgErr = qmi_client_send_msg_sync(CatClient,
                                            QMI_CAT_GET_CONFIGURATION_REQ_V02,
                                            NULL, 0,
                                            &getCfgResp, sizeof(getCfgResp),
                                            COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (LE_OK != swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_CAT_GET_CONFIGURATION_REQ_V02),
                                      clientMsgErr,
                                      getCfgResp.resp.result,
                                      getCfgResp.resp.error))
    {
        LE_WARN("Cannot get the SIM Toolkit configuration");
    }
    else
    {
        if (getCfgResp.cat_config_mode_valid)
        {
            if (CAT_CONFIG_MODE_GOBI_V02 != getCfgResp.cat_config_mode)
            {
                LE_WARN("SIM Toolkit may not work since cat_config_mode is %d!",
                        getCfgResp.cat_config_mode);
            }
            else
            {
                LE_WARN("cat_config_mode is correct, SIM Toolkit can work.");
            }
        }
        else
        {
            LE_WARN("Cannot get the Sim Toolkit configuration, consider it as Disabled");
        }
    }

    SWIQMI_DECLARE_MESSAGE(cat_set_event_report_req_msg_v02, setEventReq);
    SWIQMI_DECLARE_MESSAGE(cat_set_event_report_resp_msg_v02, setEventResp);

    setEventReq.slot_mask_valid = 1;
    setEventReq.slot_mask = 0x01;
    setEventReq.pc_evt_report_req_mask_valid = 1;
    // Bit 11 – Refresh (used only when QMI_CAT is configured in Gobi™ mode))
    setEventReq.pc_evt_report_req_mask = 0x800;
    setEventReq.pc_dec_evt_report_req_mask_valid = 1;
    // Bit 3 – Setup Menu, Bit 12 – End Proactive Session, Bit 20 – Bearer Independent Protocol)
    setEventReq.pc_dec_evt_report_req_mask = 0x101008;

    clientMsgErr = qmi_client_send_msg_sync(CatClient,
                                            QMI_CAT_SET_EVENT_REPORT_REQ_V02,
                                            &setEventReq, sizeof(setEventReq),
                                            &setEventResp, sizeof(setEventResp),
                                            COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (LE_OK != swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_CAT_SET_EVENT_REPORT_REQ_V02),
                                      clientMsgErr,
                                      setEventResp.resp.result,
                                      setEventResp.resp.error))
    {
        LE_WARN("Cannot set the SIM Toolkit events");
    }
    else
    {
        LE_DEBUG("Set the SIM Toolkit events");
    }
    // End SIM Toolkit

    SWIQMI_DECLARE_MESSAGE(uim_event_reg_req_msg_v01, uimEventReq);
    SWIQMI_DECLARE_MESSAGE(uim_event_reg_resp_msg_v01, uimEventResp);

    uimEventReq.event_mask = 0x3F;
    clientMsgErr = qmi_client_send_msg_sync(UimClient,
                                            QMI_UIM_EVENT_REG_REQ_V01,
                                            &uimEventReq, sizeof(uimEventReq),
                                            &uimEventResp, sizeof(uimEventResp),
                                            COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (LE_OK != swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_UIM_EVENT_REG_REQ_V01),
                                      clientMsgErr,
                                      uimEventResp.resp.result,
                                      uimEventResp.resp.error))
    {
        LE_ERROR("Cannot set the SIM status events");
        return LE_FAULT;
    }
    else
    {
        LE_DEBUG("Set the SIM Status events");
    }

    // Register our own handler to SIM status change events
    swiQmi_AddIndicationHandler(UimStateChangeHandler,
                                QMI_SERVICE_UIM,
                                QMI_UIM_STATUS_CHANGE_IND_V01,
                                NULL);

    // Register our own handler to SIM Refresh events
    swiQmi_AddIndicationHandler(SimRefreshHandler,
                                QMI_SERVICE_UIM,
                                QMI_UIM_REFRESH_IND_V01,
                                NULL);

    // Register our own handler to PBM record read events
    swiQmi_AddIndicationHandler(PbmRecordReadHandler,
                                QMI_SERVICE_PBM,
                                QMI_PBM_RECORD_READ_IND_V01,
                                NULL);

    // Register our internal handler to get sim state.
    le_event_AddHandler("InternalSimStateHandler",
                        InternalSimStateEvent,
                        (le_event_HandlerFunc_t)InternalStateChangeHandler);

    // Register our internal handler to get SIM toolkit events.
    le_event_AddHandler("InternalSimToolkitHandler",
                        InternalSimToolkitEvent,
                        (le_event_HandlerFunc_t)InternalSimToolkitHandler);

    return pa_sim_GetSelectedCard(&UimSelect);
}

//--------------------------------------------------------------------------------------------------
/**
 * This function flips UIM data
 *
 */
//--------------------------------------------------------------------------------------------------
static void UimFlipData
(
    uint8_t                      * des,
    const uint8_t                * src,
    uint32_t                       data_len
)
{
    unsigned int     i             =   0;
    unsigned short   temp_path     =   0;

   for(i = 0; i < (data_len + 1); i += sizeof(unsigned short))
   {
        temp_path = (*(src + i) << 8) |
                     (*(src + i + 1));

        memcpy(des, (unsigned char*)&temp_path, sizeof(unsigned short));
        des += sizeof(unsigned short);
   }
}

//--------------------------------------------------------------------------------------------------
/**
 * This function converts a BCD string into an ASCII one.
 *
 */
//--------------------------------------------------------------------------------------------------
static void Bcd2Asc (char * str, uint8_t * bcd, uint8_t len, bool sms)
{
    uint8_t i;

    for ( i = 0 ; i < len ; i ++ )
    {
        /*In case of a SMS command different conversion is required*/
        if(sms)
        {
            str[i] = SMSBCD2ASC (((bcd[i/2] >> ((i & 1) ? 4 : 0)) & 0x0F));
        }
        else
        {
            str[i] = BCD2ASC (((bcd[i/2] >> ((i & 1) ? 4 : 0)) & 0x0F));
        }
    }
    str[len] = 0;
}

//--------------------------------------------------------------------------------------------------
/**
 *  Retrieve the SIM application type from QMI message
 *
 * @return
 *      - LE_OK             Function succeeded.
 *      - LE_BAD_PARAMETER  Bad parameter
 *      - LE_FAULT          The function failed.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t RetrieveSimApplicationType
(
    uint8_t simIndex,                                     ///< [IN] SIM index
    uim_get_card_status_resp_msg_v01* cardStatusMsgPtr,   ///< [IN] Status message
    SimApplicationType_t* applicationTypePtr          ///< [OUT] SIM appication type
)
{
    if (NULL == applicationTypePtr)
    {
        LE_ERROR("applicationTypePtr is NULL");
        return LE_BAD_PARAMETER;
    }

    *applicationTypePtr = SIM_UNKNOWN_APP_TYPE;

    LE_DEBUG("QMI SIM index %d", simIndex);

    // Debug trace
    if (cardStatusMsgPtr->card_status_validity_valid)
    {
        LE_DEBUG("Card Status Validity %d"
                , (int)cardStatusMsgPtr->card_status_validity[simIndex]);
    }

    // Retrieve SIM state from QMI message
    if (!cardStatusMsgPtr->card_status_valid)
    {
        LE_ERROR("Card Status is not valid");
        return LE_FAULT;
    }

    LE_DEBUG("Card state %d (err %d)",
              cardStatusMsgPtr->card_status.card_info[simIndex].card_state,
              cardStatusMsgPtr->card_status.card_info[simIndex].error_code);

    if (UIM_CARD_STATE_PRESENT_V01 != cardStatusMsgPtr->card_status.card_info[simIndex].card_state)
    {
        LE_ERROR("SIM card error %d)",
                  cardStatusMsgPtr->card_status.card_info[simIndex].card_state);
        return LE_FAULT;
    }

    if (0 == cardStatusMsgPtr->card_status.card_info[simIndex].app_info_len)
    {
        LE_ERROR("SIM application is missing (internal error)");
        return LE_FAULT;
    }

    if (cardStatusMsgPtr->card_status.card_info[simIndex].app_info_len > 1)
    {
        LE_WARN("More than one application %d)",
                 cardStatusMsgPtr->card_status.card_info[simIndex].app_info_len);
    }

    switch (cardStatusMsgPtr->card_status.card_info[simIndex].app_info[0].app_type)
    {
        case UIM_APP_TYPE_SIM_V01:
        {
            *applicationTypePtr = SIM_UIM_APP_TYPE_SIM;
        }
        break;

        case UIM_APP_TYPE_USIM_V01:
        {
            *applicationTypePtr = SIM_UIM_APP_TYPE_USIM;
        }
        break;

        case UIM_APP_TYPE_RUIM_V01:
        {
            *applicationTypePtr = SIM_UIM_APP_TYPE_RUIM;
        }
        break;

        case UIM_APP_TYPE_CSIM_V01:
        {
            *applicationTypePtr = SIM_UIM_APP_TYPE_CSIM;
        }
        break;

        case UIM_APP_TYPE_ISIM_V01:
        {
            *applicationTypePtr = SIM_UIM_APP_TYPE_ISIM;
        }
        break;

        default:
        {
            LE_ERROR("Unknown SIM application type %d)",
                      cardStatusMsgPtr->card_status.card_info[simIndex].app_info[0].app_type);
        }
        break;
    };

    LE_DEBUG("SIM application type %d", *applicationTypePtr);
    if (SIM_UNKNOWN_APP_TYPE == *applicationTypePtr)
    {
        return LE_FAULT;
    }
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function get the SIM appication type.
 *
 * @return LE_OK             The function succeeded.
 * @return LE_BAD_PARAMETER  Bad parameter
 * @return LE_FAULT          The function failed.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t GetSimApplicationType
(
    SimApplicationType_t* applicationTypePtr   ///< [OUT] SIM appication type
)
{
    SWIQMI_DECLARE_MESSAGE(uim_get_card_status_resp_msg_v01, statusResp);
    uint8_t simIndex = 0;

    if (NULL == applicationTypePtr)
    {
        LE_ERROR("applicationTypePtr is NULL");
        return LE_BAD_PARAMETER;
    }

    *applicationTypePtr = SIM_UNKNOWN_APP_TYPE;
    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(UimClient,
                    QMI_UIM_GET_CARD_STATUS_REQ_V01,
                    NULL, 0,
                    &statusResp, sizeof(statusResp),
                    COMM_UICC_TIMEOUT);

    if (LE_OK == swiQmi_CheckResponseCode(
                    STRINGIZE_EXPAND(QMI_UIM_GET_CARD_STATUS_RESP_V01),
                    clientMsgErr,
                    statusResp.resp))
    {
        if (statusResp.card_status_valid)
        {
            if (LE_OK != TranslateSimIdFromActiveSlots( UimSelect, &simIndex))
            {
                LE_ERROR("USIM index error (%d)", UimSelect);
                return LE_FAULT;
            }

            if (simIndex > statusResp.card_status.card_info_len)
            {
                LE_ERROR("SIM index out of range (%d,%d)",
                         simIndex,
                         statusResp.card_status.card_info_len);
                return LE_FAULT;
            }

            // Retrieve the SIM application type from QMI message
            if (LE_OK == RetrieveSimApplicationType(simIndex, &statusResp, applicationTypePtr))
            {
                LE_DEBUG("SIM application type %d", *applicationTypePtr);
                return LE_OK;
            }
        }
    }

    LE_ERROR("Cannot get the SIM application type");
    return LE_FAULT;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function get the International Mobile Subscriber Identity (IMSI) for 2G, 3G, LTE.
 *
 * @return LE_FAULT         The function failed.
 * @return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t GetUsimIMSI
(
    SimApplicationType_t applicationType,   ///< [IN] SIM application type
    pa_sim_Imsi_t imsi                          ///< [OUT] IMSI value
)
{
    SWIQMI_DECLARE_MESSAGE(uim_read_transparent_req_msg_v01, readReq);
    SWIQMI_DECLARE_MESSAGE(uim_read_transparent_resp_msg_v01, readResp);

    // QMI message configuration
    readReq.session_information.session_type = UIM_SESSION_TYPE_PRIMARY_GW_V01;
    readReq.session_information.aid_len = 0;
    readReq.read_transparent.offset = 0;
    readReq.file_id.path_len = 4;
    readReq.read_transparent.length = SimImsiList[applicationType].length;
    readReq.file_id.file_id = SimImsiList[applicationType].fileId;

    // Flip data for the file path
    UimFlipData(readReq.file_id.path,
                (uint8_t*)SimImsiList[applicationType].pathFile,
                readReq.file_id.path_len);

    // Send QMI message
    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(UimClient,
        QMI_UIM_READ_TRANSPARENT_REQ_V01,
        &readReq, sizeof(readReq),
        &readResp, sizeof(readResp),
        COMM_UICC_TIMEOUT);

    // Check QMI response
    if (LE_OK != swiQmi_CheckResponse(
        STRINGIZE_EXPAND(QMI_UIM_READ_TRANSPARENT_RESP_V01),
        clientMsgErr,
        readResp.resp.result,
        readResp.resp.error))
    {
        LE_ERROR("Failed to copy IMSI");
        return LE_FAULT;
    }

    LE_DEBUG("IMSI len %d", readResp.read_result.content_len);

    // IMSI Debug information
    if (IS_TRACE_ENABLED)
    {
        int i;
        for (i=0; i<readResp.read_result.content_len; i++)
        {
            LE_DEBUG("IMSI[%d][0x%02X]", i, readResp.read_result.content[i]);
        }
    }

    // Copy the imsi to the user's buffer.
    if (true == readResp.read_result_valid)
    {
        char ImsiStr[IMSI_BYTES] = {0};
        Bcd2Asc( ImsiStr, &readResp.read_result.content[1],
                 readResp.read_result.content[0]*2, true);

        if (IS_TRACE_ENABLED)
        {
            int i;
            for (i=0; i<IMSI_BYTES; i++)
            {
                LE_DEBUG("ImsiStr[%d][0x%02X]", i, ImsiStr[i]);
            }
        }

        if (LE_OK == le_utf8_Copy(imsi, (const char*)ImsiStr+1, PA_SIM_IMSI_MAX_LEN+1, NULL))
        {
            return LE_OK;
        }
    }
    LE_ERROR("Failed to copy IMSI");
    return LE_FAULT;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function get the International Mobile Subscriber Identity (IMSI) for CDMA.
 *
 * @return LE_FAULT         The function failed.
 * @return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t GetCdmaIMSI
(
    pa_sim_Imsi_t imsi    ///< [OUT] IMSI value
)
{
    le_result_t result = LE_FAULT;
    SWIQMI_DECLARE_MESSAGE(nas_get_3gpp2_subscription_info_req_msg_v01, readReq);
    SWIQMI_DECLARE_MESSAGE(nas_get_3gpp2_subscription_info_resp_msg_v01, readResp);

    // QMI message configuration
    readReq.nam_id = 0;
    readReq.get_3gpp2_info_mask_valid = 1;
    readReq.get_3gpp2_info_mask = QMI_NAS_GET_3GPP2_SUBS_INFO_MIN_BASED_IMSI_V01;

    // Send QMI message
    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(NasClient,
        QMI_NAS_GET_3GPP2_SUBSCRIPTION_INFO_REQ_MSG_V01,
        &readReq, sizeof(readReq),
        &readResp, sizeof(readResp),
        COMM_UICC_TIMEOUT);

    // Check QMI response
    if (LE_OK != swiQmi_CheckResponseCode(
        STRINGIZE_EXPAND(QMI_NAS_GET_3GPP2_SUBSCRIPTION_INFO_RESP_MSG_V01),
        clientMsgErr,
        readResp.resp))
    {
        // retry with True IMSI
        readReq.get_3gpp2_info_mask = QMI_NAS_GET_3GPP2_SUBS_INFO_TRUE_IMSI_V01;

        // Send QMI message
        qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(NasClient,
            QMI_NAS_GET_3GPP2_SUBSCRIPTION_INFO_REQ_MSG_V01,
            &readReq, sizeof(readReq),
            &readResp, sizeof(readResp),
            COMM_UICC_TIMEOUT);

        // Check QMI response
        if (LE_OK != swiQmi_CheckResponseCode(
            STRINGIZE_EXPAND(QMI_NAS_GET_3GPP2_SUBSCRIPTION_INFO_RESP_MSG_V01),
            clientMsgErr,
            readResp.resp))

        {
            LE_ERROR("Failed to copy IMSI");
            return LE_FAULT;
        }
    }

    result = LE_OK;
    char ImsiStr[IMSI_BYTES] = {0};

    // MIN-based IMSI
    if (true == readResp.min_based_info_valid)
    {
        // MCC_M
        if (LE_OK != le_utf8_Copy(ImsiStr,
                                  (const char*) &readResp.min_based_info.mcc_m[0],
                                  NAS_MCC_LEN_V01, NULL))
        {
            result = LE_FAULT;
        }
        //  IMSI_M_11_12
        if (LE_OK != le_utf8_Copy((ImsiStr+NAS_MCC_LEN_V01),
                                  (const char*) &readResp.min_based_info.imsi_m_11_12[0],
                                  NAS_IMSI_11_12_LEN_V01, NULL))
        {
            result = LE_FAULT;
        }

        //  IMSI_M_S1
        if (LE_OK != le_utf8_Copy((ImsiStr+ NAS_MCC_LEN_V01+NAS_IMSI_11_12_LEN_V01),
                                  (const char*) &readResp.min_based_info.imsi_m_s1[0],
                                  NAS_IMSI_MIN1_LEN_V01, NULL))
        {
            result = LE_FAULT;
        }

        // IMSI_M_S2
        if (LE_OK != le_utf8_Copy((ImsiStr+ NAS_MCC_LEN_V01+NAS_IMSI_11_12_LEN_V01+
                                  NAS_IMSI_MIN1_LEN_V01),
                                  (const char*) &readResp.min_based_info.imsi_m_s2[0],
                                  NAS_IMSI_MIN2_LEN_V01, NULL))
        {
            result = LE_FAULT;
        }
    }
    // True IMSI
    else if (true == readResp.true_imsi_valid)
    {
         char ImsiTAddrNumStr[IMSI_BYTES] = {0};

        // MCC_T
        if (LE_OK != le_utf8_Copy(ImsiStr,
                                  (const char*) &readResp.true_imsi.mcc_t[0],
                                  NAS_MCC_LEN_V01, NULL))
        {
            result = LE_FAULT;
        }
        //  IMSI_T_11_12
        if (LE_OK != le_utf8_Copy((ImsiStr+NAS_MCC_LEN_V01),
                                  (const char*) &readResp.true_imsi.imsi_t_11_12[0],
                                  NAS_IMSI_11_12_LEN_V01, NULL))
        {
            result = LE_FAULT;
        }

        //  IMSI_T_S1
        if (LE_OK != le_utf8_Copy((ImsiStr+NAS_MCC_LEN_V01+NAS_IMSI_11_12_LEN_V01),
                                  (const char*) &readResp.true_imsi.imsi_t_s1[0],
                                  NAS_IMSI_MIN1_LEN_V01, NULL))
        {
            result = LE_FAULT;
        }

        // IMSI_T_S2
        if (LE_OK != le_utf8_Copy((ImsiStr+NAS_MCC_LEN_V01+NAS_IMSI_11_12_LEN_V01+
                                   NAS_IMSI_MIN1_LEN_V01),
                                  (const char*) &readResp.true_imsi.imsi_t_s2[0],
                                  NAS_IMSI_MIN2_LEN_V01, NULL))
        {
            result = LE_FAULT;
        }

        // IMSI_T_ADDR_NUM
        snprintf(ImsiTAddrNumStr, sizeof(ImsiTAddrNumStr),"%d", readResp.true_imsi.imsi_t_addr_num);
        strncat(ImsiStr, (const char *)ImsiTAddrNumStr, sizeof(ImsiStr)-strlen(ImsiStr)-1);
    }
    else
    {
        result = LE_FAULT;
    }

    if (LE_OK == result)
    {
        if (LE_OK == le_utf8_Copy(imsi, (const char*)ImsiStr+1, PA_SIM_IMSI_MAX_LEN+1, NULL))
        {
            return LE_OK;
        }
    }
    LE_ERROR("Failed to copy IMSI");
    return LE_FAULT;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function initializes the given key handler and allocates needed resources.
 *
 * @return LE_FAULT         The function failed.
 * @return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t KeyUnlockInit
(
    KeyUnlockHandler_t* pinUnlockPtr
)
{
    if (NULL == pinUnlockPtr)
    {
        LE_ERROR("Null pointer");
        return LE_FAULT;
    }

    if (pinUnlockPtr->sem)
    {
        LE_WARN("Semaphore already created");
        return LE_OK;
    }

    // Init the semaphore
    pinUnlockPtr->sem = le_sem_Create("PinPukSem", 0);

    // PIN/PUK state initialisation
    pinUnlockPtr->inProgress = true;
    pinUnlockPtr->success = false;

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function frees allocated resources and deinitializes the given key handler
 *
 */
//--------------------------------------------------------------------------------------------------
static void KeyUnlockDeInit
(
    KeyUnlockHandler_t* pinUnlockPtr
)
{
    if (NULL == pinUnlockPtr)
    {
        LE_ERROR("Null pointer");
        return;
    }

    // Lock the mutex
    LOCK

    if (pinUnlockPtr->sem)
    {
        le_sem_Delete(pinUnlockPtr->sem);
        pinUnlockPtr->sem = 0;
    }

    // Deinitialize PIN/PUK states
    pinUnlockPtr->inProgress = false;
    pinUnlockPtr->success = false;

    // Unlock the mutex
    UNLOCK
}

//--------------------------------------------------------------------------------------------------
/**
 * This function waits for the PIN/PUK status report in SIM status handler with with a limit on how
 * long to wait.
 *
 * @return LE_OK            The function succeeded.
 * @return LE_FAULT         The function failed.
 * @return LE_TIMEOUT       No response from the SIM card.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t KeyUnlockWait
(
    KeyUnlockHandler_t* pinUnlockPtr
)
{
    le_clk_Time_t timer = {.sec= TIMEOUT_SEC, .usec= TIMEOUT_USEC};
    le_result_t result = LE_FAULT;

    if (NULL == pinUnlockPtr)
    {
        goto error;
    }

    if (NULL == pinUnlockPtr->sem)
    {
        goto error;
    }

    result = le_sem_WaitWithTimeOut(pinUnlockPtr->sem,timer);
    if (LE_TIMEOUT == result)
    {
        LE_ERROR("Failed to enter PIN , timeout occured");
        result = LE_TIMEOUT;
    }
    else
    {
        if (pinUnlockPtr->success)
        {
            result = LE_OK;
        }
    }

error:
    KeyUnlockDeInit(pinUnlockPtr);
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the SIM Phone Number with the QMI Device Management Service.
 *
 * @note The MSISDN retrieved with DMS doesn't include the '+' sign for international numbers.
 *
 * @return
 *  - LE_OK on success
 *  - LE_OVERFLOW if the Phone Number can't fit in phoneNumberStr
 *  - LE_FAULT on any other failure
 */
//--------------------------------------------------------------------------------------------------
static le_result_t GetSubscriberPhoneNumberWithDms
(
    char*   phoneNumberStr,     ///< [OUT] The phone Number
    size_t  phoneNumberStrSize  ///< [IN]  Size of phoneNumberStr
)
{
    qmi_client_error_type clientMsgErr;
    SWIQMI_DECLARE_MESSAGE(dms_get_msisdn_resp_msg_v01, getMsisdnResp);

    clientMsgErr = qmi_client_send_msg_sync(DmsClient,
                                            QMI_DMS_GET_MSISDN_REQ_V01,
                                            NULL, 0,
                                            &getMsisdnResp, sizeof(getMsisdnResp),
                                            COMM_UICC_TIMEOUT);
    if (LE_OK != swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_DMS_GET_MSISDN_REQ_V01),
                                      clientMsgErr,
                                      getMsisdnResp.resp.result,
                                      getMsisdnResp.resp.error))
    {
        if (QMI_ERR_NOT_PROVISIONED_V01 == getMsisdnResp.resp.error)
        {
            LE_DEBUG("Phone number has not been provisioned");
            le_utf8_Copy(phoneNumberStr, "", phoneNumberStrSize, NULL);
            return LE_OK;
        }

        return LE_FAULT;
    }

    if (LE_OK != le_utf8_Copy(phoneNumberStr,
                              getMsisdnResp.voice_number,
                              phoneNumberStrSize,
                              NULL))
    {
        LE_ERROR("Phone number '%s' is too long", getMsisdnResp.voice_number);
        return LE_OVERFLOW;
    }

    LE_DEBUG("Read phone number '%s' with DMS", phoneNumberStr);
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the SIM Phone Number with the QMI Phonebook Manager.
 *
 * @return
 *  - LE_OK on success
 *  - LE_OVERFLOW if the Phone Number can't fit in phoneNumberStr
 *  - LE_FAULT on any other failure
 */
//--------------------------------------------------------------------------------------------------
static le_result_t GetSubscriberPhoneNumberWithPbm
(
    char*   phoneNumberStr,     ///< [OUT] The phone Number
    size_t  phoneNumberStrSize  ///< [IN]  Size of phoneNumberStr
)
{
    le_result_t result = LE_OK;
    qmi_client_error_type clientMsgErr;
    SWIQMI_DECLARE_MESSAGE(pbm_read_records_req_msg_v01, readRecordReq);
    SWIQMI_DECLARE_MESSAGE(pbm_read_records_resp_msg_v01, readRecordResp);

    PbmSem = le_sem_Create("Phonebook semaphore", 0);

    // Read only first record of MSISDN phonebook
    readRecordReq.record_info.session_type = PBM_SESSION_TYPE_GW_PRIMARY_V01;
    readRecordReq.record_info.pb_type = PBM_PB_TYPE_MSISDN_V01;
    readRecordReq.record_info.record_start_id = 1;
    readRecordReq.record_info.record_end_id = 1;

    clientMsgErr = qmi_client_send_msg_sync(PbmClient,
                                            QMI_PBM_READ_RECORDS_REQ_V01,
                                            &readRecordReq, sizeof(readRecordReq),
                                            &readRecordResp, sizeof(readRecordResp),
                                            COMM_UICC_TIMEOUT);
    if (LE_OK != swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_PBM_READ_RECORDS_REQ_V01),
                                      clientMsgErr,
                                      readRecordResp.resp.result,
                                      readRecordResp.resp.error))
    {
        result = LE_FAULT;
        goto end;
    }

    // Check the number of records (if available)
    if ((readRecordResp.num_of_recs_valid) && (!readRecordResp.num_of_recs))
    {
        LE_DEBUG("Phone number has not been provisioned");
        le_utf8_Copy(phoneNumberStr, "", phoneNumberStrSize, NULL);
        goto end;
    }

    // Wait for record read indication
    le_clk_Time_t timeout = {PBM_TIMEOUT_SEC, 0};
    if (LE_OK != le_sem_WaitWithTimeOut(PbmSem, timeout))
    {
        LE_DEBUG("Phone number has not been provisioned");
        le_utf8_Copy(phoneNumberStr, "", phoneNumberStrSize, NULL);
        goto end;
    }

    if (LE_OK != le_utf8_Copy(phoneNumberStr,
                              PbmRecordData.number,
                              phoneNumberStrSize,
                              NULL))
    {
        LE_ERROR("Phone number '%s' is too long", PbmRecordData.number);
        result = LE_OVERFLOW;
        goto end;
    }

    LE_DEBUG("Read phone number '%s' in MSISDN phonebook", phoneNumberStr);

end:
    memset(&PbmRecordData, 0, sizeof(PbmRecordData));
    le_sem_Delete(PbmSem);
    PbmSem = NULL;
    return result;
}

//--------------------------------------------------------------------------------------------------
// APIs.
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * This function selects the Card on which all further SIM operations have to be operated.
 *
 * @return
 * - LE_OK            The function succeeded.
 * - LE_FAULT         on failure.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_SelectCard
(
    le_sim_Id_t  sim     ///< The SIM to be selected
)
{
    dms_swi_uim_select_req_msg_v01 uimselectReq = { 0 };
    dms_swi_uim_select_resp_msg_v01 uimselectRsp = { {0} };

    if ( TranslateSimId(sim, &uimselectReq.uim_select ) != LE_OK )
    {
        return LE_FAULT;
    }

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(DmsClient,
                                            QMI_DMS_SWI_UIM_SELECT_REQ_V01,
                                            &uimselectReq, sizeof(uimselectReq),
                                            &uimselectRsp, sizeof(uimselectRsp),
                                            COMM_UICC_TIMEOUT);

    if ( swiQmi_CheckResponseCode(
                                STRINGIZE_EXPAND(QMI_DMS_SWI_UIM_SELECT_REQ_V01),
                                clientMsgErr,
                                uimselectRsp.resp) == LE_OK )
    {
        UimSelect = sim;
        return LE_OK;
    }
    else
    {
        return LE_FAULT;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * This function get the card on which operations are operated.
 *
 * @return LE_FAULT         The function failed.
 * @return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_GetSelectedCard
(
    le_sim_Id_t*  simIdPtr     ///< [OUT] The SIM identifier selected.
)
{
    dms_swi_get_uim_selection_resp_msg_v01 resp = { 0 };

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(DmsClient,
                                                QMI_DMS_SWI_GET_UIM_SELECTION_REQ_V01,
                                                NULL, 0,
                                                &resp, sizeof(resp),
                                                COMM_DEFAULT_PLATFORM_TIMEOUT);


    if ( swiQmi_CheckResponseCode(STRINGIZE_EXPAND(QMI_DMS_SWI_GET_UIM_SELECTION_REQ_V01),
                                    clientMsgErr,
                                    resp.resp) == LE_OK )
    {
        le_sim_Id_t simId;
        LE_DEBUG("uim_select.%d (QMI value)", resp.uim_select);

        for (simId = LE_SIM_EMBEDDED; simId < LE_SIM_ID_MAX; simId++)
        {
            if (UimMapping[simId] == resp.uim_select)
            {
                *simIdPtr = simId;
                return LE_OK;
            }
        }

        LE_ERROR("Unexpected uim_select.%d value", resp.uim_select);
        return LE_FAULT;
    }
    else
    {
        return LE_FAULT;
    }
}


//--------------------------------------------------------------------------------------------------
/**
 * This function get the card identification (ICCID).
 *
 * @return LE_FAULT         The function failed.
 * @return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_GetCardIdentification
(
    pa_sim_CardId_t iccid     ///< [OUT] CCID value
)
{
    uim_read_transparent_req_msg_v01 readReq;
    uim_read_transparent_resp_msg_v01 readResp = { {0} };
    char iccidSim[PA_SIM_CARDID_MAX_LEN+1] = { 0 };

    memset(&readReq, 0, sizeof(readReq));

    /* Mandatory */
    /*  Session Information */
    readReq.session_information.session_type = UIM_SESSION_TYPE_PRIMARY_GW_V01;
    /*   Application identifier value or channel ID. This value is required
         for non provisioning and for logical channel session types. It is
         ignored in all other cases. */
    readReq.session_information.aid_len = 0;
    readReq.session_information.aid[0] = 0;

    /* Mandatory */
    /*  File EF ICCID ID */
    readReq.file_id.file_id = 0x2FE2;
    /* Set the master File path Id */
    uint16_t filePathId = 0x3F00;
    memcpy(readReq.file_id.path, &filePathId, sizeof(filePathId));
    readReq.file_id.path_len = 2;

    /* Mandatory */
    /* Read Transparent */
    /* Read at the beginning of the file */
    readReq.read_transparent.offset = 0;
    /* Length of the content to be read. The value 0 is used to read the complete file */
    readReq.read_transparent.length = 0;

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(UimClient,
                    QMI_UIM_READ_TRANSPARENT_REQ_V01,
                    &readReq, sizeof(readReq),
                    &readResp, sizeof(readResp),
                    COMM_UICC_TIMEOUT);

    if (LE_OK == swiQmi_CheckResponse(
                    STRINGIZE_EXPAND(QMI_UIM_READ_TRANSPARENT_REQ_V01),
                    clientMsgErr,
                    readResp.resp.result,
                    readResp.resp.error))
    {
        if (readResp.read_result_valid)
        {
            size_t index, i;
            uint8_t lowIccid, highIccid;
            uint8_t nationalCode = 0;
            uint8_t operatorCode = 0;

            LE_DEBUG("ICCID len %d", readResp.read_result.content_len);
            if (IS_TRACE_ENABLED)
            {
                for (i=0; i<readResp.read_result.content_len; i++)
                {
                    LE_DEBUG("ICCID[%d][0x%02X]", i, readResp.read_result.content[i]);
                }
            }

            if (10 != readResp.read_result.content_len)
            {
                LE_ERROR("Error len %d", readResp.read_result.content_len);
                return LE_FAULT;
            }

            index = 0;
            for ( i=0; i < readResp.read_result.content_len; i++)
            {
                lowIccid = readResp.read_result.content[i] & 0xF;
                highIccid = (readResp.read_result.content[i] >> 4) & 0xF;

                /* Please note that character A~F is valid for operator CMCC and CU.
                 * This CR is initiated by CMCC. */
                if (PA_NATIONAL_CODE_IDX == i)
                {
                    nationalCode = readResp.read_result.content[i];
                }

                if (PA_OPERATOR_CODE_IDX == i)
                {
                    operatorCode = readResp.read_result.content[i];
                }

                if (lowIccid > 9)
                {
                    if( (PA_NATIONAL_CODE_CN    == nationalCode)
                        && ( (PA_OPERATOR_CODE_CM   == operatorCode)
                             || (PA_OPERATOR_CODE_CM_2 == operatorCode)
                             || (PA_OPERATOR_CODE_CU   == operatorCode) ) )
                    {
                        iccidSim[index++] = 'A' + lowIccid - 0x0A;
                        LE_DEBUG("A~F exits in ICCID low BCD ");
                    }
                    else
                    {
                        iccidSim[index] = 0;
                        LE_DEBUG("Truncated ICCID as a unsupported Char is found ");
                        break;
                    }
                }
                else
                {
                    iccidSim[index++] = '0' + lowIccid;
                }

                if (highIccid > 9)
                {
                    if( (PA_NATIONAL_CODE_CN    == nationalCode)
                        && ( (PA_OPERATOR_CODE_CM   == operatorCode)
                             || (PA_OPERATOR_CODE_CM_2 == operatorCode)
                             || (PA_OPERATOR_CODE_CU   == operatorCode) ) )
                    {
                        iccidSim[index++] = 'A' + highIccid - 0x0A;
                        LE_DEBUG("A~F exits in ICCID high BCD");
                    }
                    else
                    {
                        iccidSim[index] = 0;
                        LE_DEBUG("Truncated ICCID as a unsupported Char is found ");
                        break;
                    }
                }
                else
                {
                    iccidSim[index++] = '0' + highIccid;
                }
            }

            LE_DEBUG("ICCID %s", iccidSim);
            // Copy the ICCID to the user's buffer.
            le_utf8_Copy(iccid, iccidSim, PA_SIM_CARDID_MAX_LEN+1, NULL);
            return LE_OK;
        }
    }

    return LE_FAULT;
}

#if defined(QMI_UIM_GET_EID_RESP_V01)
//--------------------------------------------------------------------------------------------------
/**
 * This function rerieves the identifier for the embedded Universal Integrated Circuit Card (EID)
 * (32 digits)
 *
 * @return LE_OK            The function succeeded.
 * @return LE_FAULT         The function failed.
 * @return LE_UNSUPPORTED   The platform does not support this operation.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_GetCardEID
(
   pa_sim_Eid_t eid               ///< [OUT] the EID value
)
{
    SWIQMI_DECLARE_MESSAGE(uim_get_eid_req_msg_v01, readReq);
    SWIQMI_DECLARE_MESSAGE(uim_get_eid_resp_msg_v01, readResp);

    // QMI message configuration
    readReq.slot = 1;

    // Send QMI message
    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(UimClient,
        QMI_UIM_GET_EID_REQ_V01,
        &readReq, sizeof(readReq),
        &readResp, sizeof(readResp),
        COMM_UICC_TIMEOUT);

    // Check QMI response
    if (LE_OK != swiQmi_CheckResponse(
        STRINGIZE_EXPAND(QMI_UIM_GET_EID_RESP_V01),
        clientMsgErr,
        readResp.resp.result,
        readResp.resp.error))
    {
        LE_ERROR("Failed to get the EID");
        return LE_FAULT;
    }

    // Copy the EID to the user's buffer.
    if (true == readResp.eid_value_valid)
    {
        size_t i;
        char eidSim[PA_SIM_EID_MAX_LEN+1] = {0};

        if (IS_TRACE_ENABLED)
        {
            LE_DEBUG("EID len %d", readResp.eid_value_len);
            for (i=0; i < readResp.eid_value_len; i++)
            {
                LE_DEBUG("EID[%d][0x%02X]", i, readResp.eid_value[i]);
            }
        }

        for (i=0; i < readResp.eid_value_len; i++)
        {
            sprintf(&eidSim[2*i], "%02X", readResp.eid_value[i]);
        }

        if (LE_OK == le_utf8_Copy(eid, eidSim, PA_SIM_EID_MAX_LEN+1, NULL))
        {
            return LE_OK;
        }
        LE_ERROR("Failed to copy the EID");
    }

    LE_ERROR("EID not provided");
    return LE_FAULT;
}
#endif

//--------------------------------------------------------------------------------------------------
/**
 * This function get the International Mobile Subscriber Identity (IMSI).
 *
 * @return LE_FAULT         The function failed.
 * @return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_GetIMSI
(
    pa_sim_Imsi_t imsi     ///< [OUT] IMSI value
)
{
    SimApplicationType_t applicationType;
    // get the SIM application type
    if (LE_OK != GetSimApplicationType(&applicationType))
    {
        LE_ERROR("Unknown SIM application");
        return LE_FAULT;
    }

    if ((SIM_UIM_APP_TYPE_RUIM == applicationType) ||
        (SIM_UIM_APP_TYPE_CSIM == applicationType))
    {
        // CDMA application type
        return GetCdmaIMSI(imsi);
    }
    // 2G, 3G, LTE appication type
    return GetUsimIMSI(applicationType, imsi);
}

//--------------------------------------------------------------------------------------------------
/**
 * This function get the SIM State.
 *
 * @return LE_FAULT         The function failed.
 * @return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_GetState
(
    le_sim_States_t* statePtr    ///< [OUT] SIM state
)
{
    SWIQMI_DECLARE_MESSAGE(uim_get_card_status_resp_msg_v01, statusResp);
    le_result_t result = LE_FAULT;
    uint8_t simIndex = 0;

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(UimClient,
                    QMI_UIM_GET_CARD_STATUS_REQ_V01,
                    NULL, 0,
                    &statusResp, sizeof(statusResp),
                    COMM_UICC_TIMEOUT);

    if( swiQmi_CheckResponseCode(
                    STRINGIZE_EXPAND(QMI_UIM_GET_CARD_STATUS_RESP_V01),
                    clientMsgErr,
                    statusResp.resp) == LE_OK )
    {
        if (statusResp.card_status_valid)
        {
            if ( TranslateSimIdFromActiveSlots( UimSelect, &simIndex ) != LE_OK )
            {
                    LE_ERROR("USIM index error (%d)"
                        , UimSelect);
                    return LE_FAULT;
            }
            else if(simIndex > statusResp.card_status.card_info_len)
            {
                LE_ERROR("SIM index out of range (%d,%d)"
                    , simIndex
                    , statusResp.card_status.card_info_len);
                return LE_FAULT;
            }

            // Map the SIM state from QMI to Legato
            *statePtr = MapSimState(simIndex, &statusResp);
            result = LE_OK;
        }
    }
    else
    {
        LE_ERROR("Cannot get the SIM state");
        *statePtr = LE_SIM_STATE_UNKNOWN;
        result = LE_FAULT;
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to register a handler for new SIM state notification handling.
 *
 * @return A handler reference, which is only needed for later removal of the handler.
 *
 * @note Doesn't return on failure, so there's no need to check the return value for errors.
 */
//--------------------------------------------------------------------------------------------------
le_event_HandlerRef_t pa_sim_AddNewStateHandler
(
    pa_sim_NewStateHdlrFunc_t handler ///< [IN] The handler function.
)
{
    LE_DEBUG("Set new SIM State handler");

    if (handler == NULL)
    {
        LE_FATAL("new SIM State handler is NULL");
    }

    return le_event_AddHandler("SimStateHandler",
                               SimStateEvent,
                               (le_event_HandlerFunc_t) handler);
}


//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to unregister the handler for new SIM state notification handling.
 *
 * @note Doesn't return on failure, so there's no need to check the return value for errors.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_RemoveNewStateHandler
(
    le_event_HandlerRef_t handlerRef
)
{
    le_event_RemoveHandler(handlerRef);

    return LE_OK;
}


//--------------------------------------------------------------------------------------------------
/**
 * This function enters the PIN code
 *
 * @return LE_OK            The function succeeded.
 * @return LE_BAD_PARAMETER The parameters are invalid.
 * @return LE_FAULT         The function failed.
 * @return LE_TIMEOUT       No response received from the SIM card.
  */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_EnterPIN
(
    pa_sim_PinType_t   type,  ///< [IN] pin type
    const pa_sim_Pin_t pin    ///< [IN] pin code
)
{
    KeyUnlockHandler_t* pinUnlockPtr;

    SWIQMI_DECLARE_MESSAGE(uim_verify_pin_req_msg_v01, pinReq);
    SWIQMI_DECLARE_MESSAGE(uim_verify_pin_resp_msg_v01, pinResp);

    // QMI message configuration
    pinReq.session_information.session_type = UIM_SESSION_TYPE_PRIMARY_GW_V01;
    pinReq.session_information.aid_len = 0;

    // Set the PIN id
    switch (type)
    {
        case PA_SIM_PIN:
            pinReq.verify_pin.pin_id = UIM_PIN_ID_PIN_1_V01;
            pinUnlockPtr = &Pin1Unlock;
            break;
        case PA_SIM_PIN2:
            pinReq.verify_pin.pin_id = UIM_PIN_ID_PIN_2_V01;
            pinUnlockPtr = &Pin2Unlock;
            break;
        default:
            return LE_BAD_PARAMETER;
    }

    // Set the PIN
    LE_ASSERT_OK(le_utf8_Copy((char*)pinReq.verify_pin.pin_value,
                              pin,
                              PA_SIM_PIN_MAX_LEN+1,
                              &(pinReq.verify_pin.pin_value_len)));

    LE_DEBUG("PIN: id %d, value %s", pinReq.verify_pin.pin_id,
                                      pinReq.verify_pin.pin_value);

    // PIN/PUK state initialisation
    if (LE_OK != KeyUnlockInit(pinUnlockPtr))
    {
        return LE_FAULT;
    }

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(UimClient,
                                            QMI_UIM_VERIFY_PIN_REQ_V01,
                                            &pinReq, sizeof(pinReq),
                                            &pinResp, sizeof(pinResp),
                                            COMM_UICC_TIMEOUT);

    if (LE_OK != swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_UIM_VERIFY_PIN_RESP_V01),
                                      clientMsgErr,
                                      pinResp.resp.result,
                                      pinResp.resp.error))
     {
        LE_WARN("PIN not entered");
        if (pinResp.retries_left_valid)
        {
            LE_DEBUG("Verify_left %d", pinResp.retries_left.verify_left);
            LE_DEBUG("Unblock_left %d", pinResp.retries_left.unblock_left);
        }
        KeyUnlockDeInit(pinUnlockPtr);
        return LE_FAULT;
    }
    else
    {
        LE_DEBUG("PIN entered");
    }

    LE_DEBUG("LastState %d", LastState);

    // The function to enter pin succeed,
    // now we need to synchronize the DMS_UIM_INITIALIZATION_COMPLETED_V01 state
    if (LE_SIM_READY == LastState)
    {
        KeyUnlockDeInit(pinUnlockPtr);
        return LE_OK;
    }

    return KeyUnlockWait(pinUnlockPtr);
}


//--------------------------------------------------------------------------------------------------
/**
 * This function set the new PIN code.
 *
 *  - use to set PIN code by providing the PUK
 *
 * All depends on SIM state which must be retreived by @ref pa_sim_GetState
 *
 * @return LE_BAD_PARAMETER The parameters are invalid.
 * @return LE_FAULT         The function failed.
 * @return LE_TIMEOUT       No response was received from the SIM card.
 * @return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_EnterPUK
(
    pa_sim_PukType_t   type, ///< [IN] puk type
    const pa_sim_Puk_t puk,  ///< [IN] PUK code
    const pa_sim_Pin_t pin   ///< [IN] new PIN code
)
{
    KeyUnlockHandler_t* pinUnlockPtr;

    SWIQMI_DECLARE_MESSAGE(uim_unblock_pin_req_msg_v01, unblockReq);
    SWIQMI_DECLARE_MESSAGE(uim_unblock_pin_resp_msg_v01, unblockResp);

    // QMI message configuration
    unblockReq.session_information.session_type = UIM_SESSION_TYPE_PRIMARY_GW_V01;
    unblockReq.session_information.aid_len = 0;

    // Set the PIN id
    switch (type)
    {
        case PA_SIM_PUK:
            unblockReq.unblock_pin.pin_id = UIM_PIN_ID_PIN_1_V01;
            pinUnlockPtr = &Pin1Unlock;
            break;
        case PA_SIM_PUK2:
            unblockReq.unblock_pin.pin_id = UIM_PIN_ID_PIN_2_V01;
            pinUnlockPtr = &Pin2Unlock;
            break;
        default:
            return LE_BAD_PARAMETER;
    }

    // Set the PUK
    LE_ASSERT_OK(le_utf8_Copy((char*)unblockReq.unblock_pin.puk_value,
                              puk,
                              PA_SIM_PUK_MAX_LEN+1,
                              &(unblockReq.unblock_pin.puk_value_len)));

    // Set the new PIN
    LE_ASSERT_OK(le_utf8_Copy((char*)unblockReq.unblock_pin.new_pin_value,
                              pin,
                              PA_SIM_PIN_MAX_LEN+1,
                              &(unblockReq.unblock_pin.new_pin_value_len)));

    // PIN/PUK state initialisation
    if (LE_OK != KeyUnlockInit(pinUnlockPtr))
    {
        return LE_FAULT;
    }

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(UimClient,
                                            QMI_UIM_UNBLOCK_PIN_REQ_V01,
                                            &unblockReq, sizeof(unblockReq),
                                            &unblockResp, sizeof(unblockResp),
                                            COMM_UICC_TIMEOUT);

    if (LE_OK != swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_UIM_UNBLOCK_PIN_RESP_V01),
                                      clientMsgErr,
                                      unblockResp.resp.result,
                                      unblockResp.resp.error))
    {
        LE_WARN("PUK error");
        KeyUnlockDeInit(pinUnlockPtr);
        return LE_FAULT;
    }
    else
    {
        LE_DEBUG("PUK entered and PIN unblocked");
    }

    // The function to enter PUK succeed,
    // now we need to synchronize the DMS_UIM_INITIALIZATION_COMPLETED_V01 state
    if (LE_SIM_READY == LastState)
    {
        KeyUnlockDeInit(pinUnlockPtr);
        return LE_OK;
    }

    return KeyUnlockWait(pinUnlockPtr);
}


//--------------------------------------------------------------------------------------------------
/**
 * This function gets the number of remaining PIN insertion tries.
 *
 * @return LE_FAULT          The function failed.
 * @return LE_BAD_PARAMETER  If attemptsPtr is NULL.
 * @return LE_TIMEOUT        No response was received.
 * @return LE_OK             The function succeeded.
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_GetPINRemainingAttempts
(
    pa_sim_PinType_t   type,         ///< [IN] The PIN type
    uint32_t*          attemptsPtr   ///< [OUT] The number of remaining PIN insertion tries
)
{
    SWIQMI_DECLARE_MESSAGE(uim_get_card_status_resp_msg_v01, statusResp);
    le_result_t result = LE_FAULT;
    uint8_t simIndex = 0;

    if (NULL == attemptsPtr)
    {
        LE_ERROR("NULL pointer");
        return LE_BAD_PARAMETER;
    }

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(UimClient,
                    QMI_UIM_GET_CARD_STATUS_REQ_V01,
                    NULL, 0,
                    &statusResp, sizeof(statusResp),
                    COMM_UICC_TIMEOUT);

    if (LE_OK != swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_UIM_GET_CARD_STATUS_REQ_V01),
                                      clientMsgErr,
                                      statusResp.resp.result,
                                      statusResp.resp.error))
    {
        LE_ERROR("Cannot get the number of remaining PIN insertion tries");
        return LE_FAULT;
    }

    if ((!statusResp.card_status_valid) || (!statusResp.card_status.card_info_len))
    {
        LE_ERROR("PIN info invalid (%d, %d)", statusResp.card_status_valid,
                                              statusResp.card_status.card_info_len);
        return LE_FAULT;
    }

    if (LE_OK != TranslateSimIdFromActiveSlots(UimSelect, &simIndex))
    {
        LE_ERROR("USIM index error (%d)", UimSelect);
        return LE_FAULT;
    }

    if (simIndex > statusResp.card_status.card_info_len)
    {
        LE_ERROR("SIM index out of range (%d,%d)", simIndex,
                                                   statusResp.card_status.card_info_len);
        return LE_FAULT;
    }

    LE_DEBUG("SIM application: type %d, state %d",
             statusResp.card_status.card_info[simIndex].app_info[0].app_type,
             statusResp.card_status.card_info[simIndex].app_info[0].app_state);

    if (PA_SIM_PIN == type)
    {
        LE_DEBUG("PIN state %d",
                 statusResp.card_status.card_info[simIndex].app_info[0].pin1.pin_state);

        *attemptsPtr = statusResp.card_status.card_info[simIndex].app_info[0].pin1.pin_retries;
        LE_DEBUG("PIN retries %d", *attemptsPtr);
        result = LE_OK;
    }
    else if (PA_SIM_PIN2 == type)
    {
        LE_DEBUG("PIN state %d",
                 statusResp.card_status.card_info[simIndex].app_info[0].pin2.pin_state);

        *attemptsPtr = statusResp.card_status.card_info[simIndex].app_info[0].pin2.pin_retries;
        LE_DEBUG("PIN retries %d", *attemptsPtr);
        result = LE_OK;
    }
    else
    {
        LE_ERROR("Wrong PIN type %d", type);
    }
    return result;
}


//--------------------------------------------------------------------------------------------------
/**
 * This function gets the number of remaining PUK insertion tries.
 *
 * @return LE_FAULT          The function failed.
 * @return LE_BAD_PARAMETER  If attemptsPtr is NULL.
 * @return LE_TIMEOUT        No response was received.
 * @return LE_OK             The function succeeded.
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_GetPUKRemainingAttempts
(
    pa_sim_PukType_t type,        ///< [IN] The PUK type
    uint32_t*        attemptsPtr  ///< [OUT] The number of remaining PUK insertion tries
)
{
    SWIQMI_DECLARE_MESSAGE(uim_get_card_status_resp_msg_v01, statusResp);
    le_result_t result = LE_FAULT;
    uint8_t simIndex = 0;

    if (NULL == attemptsPtr)
    {
        LE_ERROR("NULL pointer");
        return LE_BAD_PARAMETER;
    }

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(UimClient,
                    QMI_UIM_GET_CARD_STATUS_REQ_V01,
                    NULL, 0,
                    &statusResp, sizeof(statusResp),
                    COMM_UICC_TIMEOUT);

    if (LE_OK != swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_UIM_GET_CARD_STATUS_REQ_V01),
                                      clientMsgErr,
                                      statusResp.resp.result,
                                      statusResp.resp.error))
    {
        LE_ERROR("Cannot get the number of remaining PUK insertion tries");
        return LE_FAULT;
    }

    if ((!statusResp.card_status_valid) || (!statusResp.card_status.card_info_len))
    {
        LE_ERROR("PIN info invalid (%d, %d)", statusResp.card_status_valid,
                                              statusResp.card_status.card_info_len);
        return LE_FAULT;
    }

    if (LE_OK != TranslateSimIdFromActiveSlots(UimSelect, &simIndex))
    {
        LE_ERROR("USIM index error (%d)", UimSelect);
        return LE_FAULT;
    }

    if (simIndex > statusResp.card_status.card_info_len)
    {
        LE_ERROR("SIM index out of range (%d,%d)", simIndex,
                                                   statusResp.card_status.card_info_len);
        return LE_FAULT;
    }

    LE_DEBUG("SIM application: type %d, state %d",
             statusResp.card_status.card_info[simIndex].app_info[0].app_type,
             statusResp.card_status.card_info[simIndex].app_info[0].app_state);

    if (PA_SIM_PUK == type)
    {
        LE_DEBUG("PIN state %d",
                 statusResp.card_status.card_info[simIndex].app_info[0].pin1.pin_state);

        *attemptsPtr = statusResp.card_status.card_info[simIndex].app_info[0].pin1.puk_retries;
        LE_DEBUG("PUK retries %d", *attemptsPtr);
        result = LE_OK;
    }
    else if (PA_SIM_PUK2 == type)
    {
        LE_DEBUG("PIN state %d",
                 statusResp.card_status.card_info[simIndex].app_info[0].pin2.pin_state);

        *attemptsPtr = statusResp.card_status.card_info[simIndex].app_info[0].pin2.puk_retries;
        LE_DEBUG("PUK retries %d", *attemptsPtr);
        result = LE_OK;
    }
    else
    {
        LE_ERROR("Wrong PUK type %d", type);
    }
    return result;
}


//--------------------------------------------------------------------------------------------------
/**
 * This function change a code.
 *
 * @return LE_BAD_PARAMETER The parameters are invalid.
 * @return LE_FAULT         The function failed.
 * @return LE_TIMEOUT       No response was received from the SIM card.
 * @return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_ChangePIN
(
    pa_sim_PinType_t   type,    ///< [IN] The code type
    const pa_sim_Pin_t oldcode, ///< [IN] Old code
    const pa_sim_Pin_t newcode  ///< [IN] New code
)
{
    uim_change_pin_req_msg_v01 changeReq = { {0} };
    SWIQMI_DECLARE_MESSAGE(uim_change_pin_resp_msg_v01, changeResp);


    // QMI message configuration
    changeReq.session_information.session_type = UIM_SESSION_TYPE_PRIMARY_GW_V01;
    changeReq.session_information.aid_len = 0;

    // Set the PIN id
    switch (type)
    {
        case PA_SIM_PIN:
            changeReq.change_pin.pin_id = UIM_PIN_ID_PIN_1_V01;
            break;

        case PA_SIM_PIN2:
            changeReq.change_pin.pin_id = UIM_PIN_ID_PIN_2_V01;
            break;

        default:
            return LE_BAD_PARAMETER;
    }

    // Set the old pin.
    LE_ASSERT(le_utf8_Copy((char*)changeReq.change_pin.old_pin_value,
                           oldcode,
                           PA_SIM_PIN_MAX_LEN+1,
                           &(changeReq.change_pin.old_pin_value_len)) == LE_OK);

    // Set the new pin.
    LE_ASSERT(le_utf8_Copy((char*)changeReq.change_pin.new_pin_value,
                           newcode,
                           PA_SIM_PIN_MAX_LEN+1,
                           &(changeReq.change_pin.new_pin_value_len)) == LE_OK);

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(UimClient,
                                            QMI_UIM_CHANGE_PIN_REQ_V01,
                                            &changeReq, sizeof(changeReq),
                                            &changeResp, sizeof(changeResp),
                                            COMM_UICC_TIMEOUT);

    if ( swiQmi_CheckResponseCode(
                                STRINGIZE_EXPAND(QMI_UIM_CHANGE_PIN_RESP_V01),
                                clientMsgErr,
                                changeResp.resp) == LE_OK )
    {
        LE_DEBUG("PIN changed");
        return LE_OK;
    }
    else
    {
        LE_DEBUG("PIN not changed");
        return LE_FAULT;
    }
}


//--------------------------------------------------------------------------------------------------
/**
 * This function enables PIN locking (PIN or PIN2).
 *
 * @return LE_BAD_PARAMETER The parameters are invalid.
 * @return LE_FAULT         The function failed.
 * @return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_EnablePIN
(
    pa_sim_PinType_t   type,  ///< [IN] The pin type
    const pa_sim_Pin_t code   ///< [IN] code
)
{
    return SetPinProtection(true, type, code);
}


//--------------------------------------------------------------------------------------------------
/**
 * This function disables PIN locking (PIN or PIN2).
 *
 * @return LE_BAD_PARAMETER The parameters are invalid.
 * @return LE_FAULT         The function failed to set the value.
 * @return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_DisablePIN
(
    pa_sim_PinType_t   type,  ///< [IN] The code type.
    const pa_sim_Pin_t code   ///< [IN] code
)
{
    return SetPinProtection(false, type, code);
}


//--------------------------------------------------------------------------------------------------
/**
 * Get the SIM Phone Number.
 *
 * @return
 *  - LE_OK on success
 *  - LE_OVERFLOW if the Phone Number can't fit in phoneNumberStr
 *  - LE_FAULT on any other failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_GetSubscriberPhoneNumber
(
    char*   phoneNumberStr,     ///< [OUT] The phone Number
    size_t  phoneNumberStrSize  ///< [IN]  Size of phoneNumberStr
)
{
    // Get the MSISDN with the QMI Phonebook Manager, which includes the '+' sign
    // for international numbers.
    le_result_t result = GetSubscriberPhoneNumberWithPbm(phoneNumberStr, phoneNumberStrSize);

    // If the MSISDN is not available with the Phonebook Manager, get it with the
    // QMI Device Management Service, even if it doesn't include the '+' sign
    // for international numbers.
    if (LE_FAULT == result)
    {
        result = GetSubscriberPhoneNumberWithDms(phoneNumberStr, phoneNumberStrSize);
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to get the Home Network Name information.
 *
 * @return
 *      - LE_OK on success
 *      - LE_OVERFLOW if the Home Network Name can't fit in nameStr
 *      - LE_FAULT on any other failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_GetHomeNetworkOperator
(
    char       *nameStr,               ///< [OUT] the home network Name
    size_t      nameStrSize            ///< [IN]  the nameStr size
)
{
    return mrc_qmi_GetHomeNetworkName(nameStr,nameStrSize);
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to get the Home Network MCC MNC.
 *
 * @return
 *      - LE_OK on success
 *      - LE_OVERFLOW if the Home Network MCC/MNC can't fit in mccPtr and mncPtr
 *      - LE_FAULT for unexpected error
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_GetHomeNetworkMccMnc
(
    char     *mccPtr,                ///< [OUT] Mobile Country Code
    size_t    mccPtrSize,            ///< [IN] mccPtr buffer size
    char     *mncPtr,                ///< [OUT] Mobile Network Code
    size_t    mncPtrSize             ///< [IN] mncPtr buffer size
)
{
    uint16_t mcc;
    uint16_t mnc;
    bool     isMncIncPcsDigit;

    if ((mccPtrSize < LE_MRC_MCC_BYTES) || (mncPtrSize < LE_MRC_MNC_BYTES))
    {
        return LE_OVERFLOW;
    }

    if (mrc_qmi_GetHomeNetworkMccMnc(&mcc, &mnc, &isMncIncPcsDigit) == LE_OK)
    {
        return (ConvertMccMncIntoStringFormat(mcc, mnc, isMncIncPcsDigit, mccPtr, mncPtr));
    }
    else
    {
        LE_ERROR("Cannot get Home Network MCC/MNC");
        return LE_FAULT;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to open a logical channel on the SIM card.
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT for unexpected error
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_OpenLogicalChannel
(
    uint8_t* channelPtr  ///< [OUT] channel number
)
{
    qmi_client_error_type clientMsgErr;
    SWIQMI_DECLARE_MESSAGE(uim_open_logical_channel_req_msg_v01, openReq);
    SWIQMI_DECLARE_MESSAGE(uim_open_logical_channel_resp_msg_v01, openResp);

    if (!channelPtr)
    {
        LE_ERROR("No channel pointer");
        return LE_FAULT;
    }

    openReq.slot = UIM_SLOT_1_V01;

    clientMsgErr = qmi_client_send_msg_sync(UimClient,
                                            QMI_UIM_OPEN_LOGICAL_CHANNEL_REQ_V01,
                                            &openReq, sizeof(openReq),
                                            &openResp, sizeof(openResp),
                                            COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (LE_OK != swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_UIM_OPEN_LOGICAL_CHANNEL_REQ_V01),
                                      clientMsgErr,
                                      openResp.resp.result,
                                      openResp.resp.error))
    {
        LE_ERROR("Cannot open logical channel on slot %d", openReq.slot);
        return LE_FAULT;
    }

    if (!openResp.channel_id_valid)
    {
        LE_ERROR("No valid channel id");
        return LE_FAULT;
    }

    LE_DEBUG("Opened channel %d", openResp.channel_id);
    *channelPtr = openResp.channel_id;
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to close a logical channel on the SIM card.
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT for unexpected error
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_CloseLogicalChannel
(
    uint8_t channel  ///< [IN] channel number
)
{
    qmi_client_error_type clientMsgErr;
    SWIQMI_DECLARE_MESSAGE(uim_logical_channel_req_msg_v01, closeReq);
    SWIQMI_DECLARE_MESSAGE(uim_logical_channel_resp_msg_v01, closeResp);

    closeReq.slot = UIM_SLOT_1_V01;
    closeReq.channel_id_valid = true;
    closeReq.channel_id = channel;

    clientMsgErr = qmi_client_send_msg_sync(UimClient,
                                            QMI_UIM_LOGICAL_CHANNEL_REQ_V01,
                                            &closeReq, sizeof(closeReq),
                                            &closeResp, sizeof(closeResp),
                                            COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (LE_OK != swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_UIM_LOGICAL_CHANNEL_REQ_V01),
                                      clientMsgErr,
                                      closeResp.resp.result,
                                      closeResp.resp.error))
    {
        LE_ERROR("Cannot close logical channel on slot %d", closeReq.slot);
        return LE_FAULT;
    }

    LE_DEBUG("Closed channel %d", closeReq.channel_id);
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to send an APDU message to the SIM card.
 *
 * @return
 *      - LE_OK on success
 *      - LE_OVERFLOW the response length exceed the maximum buffer length.
 *      - LE_FAULT for unexpected error
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_SendApdu
(
    uint8_t        channel, ///< [IN] Logical channel.
    const uint8_t* apduPtr, ///< [IN] APDU message buffer
    uint32_t       apduLen, ///< [IN] APDU message length in bytes
    uint8_t*       respPtr, ///< [OUT] APDU message response.
    size_t*        lenPtr   ///< [IN,OUT] APDU message response length in bytes.
)
{
    qmi_client_error_type clientMsgErr;
    SWIQMI_DECLARE_MESSAGE(uim_send_apdu_req_msg_v01, req);
    SWIQMI_DECLARE_MESSAGE(uim_send_apdu_resp_msg_v01, resp);

    if (apduLen > QMI_UIM_APDU_DATA_MAX_V01)
    {
        LE_ERROR("APDU message length > QMI_UIM_APDU_DATA_MAX_V01 (%d > %d)",
                 apduLen, QMI_UIM_APDU_DATA_MAX_V01);
        return LE_OVERFLOW;
    }

    req.slot = UIM_SLOT_1_V01;
    req.apdu_len = apduLen;
    memcpy(req.apdu, apduPtr, apduLen);

    // Set channel_id field only if it's not the basic logical channel, i.e. channel is not 0.
    if (channel)
    {
        req.channel_id_valid = true;
        req.channel_id = channel;
    }

    clientMsgErr = qmi_client_send_msg_sync(UimClient,
                                            QMI_UIM_SEND_APDU_REQ_V01,
                                            &req, sizeof(req),
                                            &resp, sizeof(resp),
                                            COMM_UICC_TIMEOUT);

    if (LE_OK == swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_UIM_SEND_APDU_REQ_V01),
                                      clientMsgErr,
                                      resp.resp.result,
                                      resp.resp.error))
    {
        if ((resp.apdu_valid) && (respPtr != NULL) && (lenPtr != NULL))
        {
            if (resp.apdu_len > *lenPtr)
            {
                LE_ERROR("The response length exceed the maximum buffer length!");
                return LE_OVERFLOW;
            }

            memcpy(respPtr, resp.apdu, resp.apdu_len);
            *lenPtr = resp.apdu_len;
        }
        else if (lenPtr != NULL)
        {
            *lenPtr = 0;
        }

        return LE_OK;
    }

    return LE_FAULT;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to trigger a SIM refresh.
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT for unexpected error
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_Refresh
(
    void
)
{
    swi_m2m_sim_refresh_resp_msg_v01 resp = { {0} };

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(MgsClient,
                                            QMI_SWI_M2M_SIM_REFRESH_REQ_V01,
                                            NULL, 0,
                                            &resp, sizeof(resp),
                                            COMM_UICC_TIMEOUT);

    if ( swiQmi_OEMCheckResponseCode(STRINGIZE_EXPAND(QMI_SWI_M2M_SIM_REFRESH_REQ_V01),
                                     clientMsgErr,
                                     resp.resp) == LE_OK )
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
 * This function must be called to register a handler for SIM Toolkit event notification handling.
 *
 * @return
 *      - A handler reference on success, which is only needed for later removal of the handler.
 *      - NULL on failure
 */
//--------------------------------------------------------------------------------------------------
le_event_HandlerRef_t pa_sim_AddSimToolkitEventHandler
(
    pa_sim_SimToolkitEventHdlrFunc_t handler,    ///< [IN] The handler function.
    void*                            contextPtr  ///< [IN] The context to be given to the handler.
)
{
    le_event_HandlerRef_t handlerRef = NULL;

    LE_FATAL_IF((handler == NULL), "Sim Toolkit handler is NULL");

    if (!RefreshRegisterCount)
    {
        if (LE_OK != EnableRefreshConfirmation(true))
        {
            LE_ERROR("Cannot enable Sim Toolkit configuration");
            return NULL;
        }
    }

    RefreshRegisterCount++;

    LE_DEBUG("Register for Sim Refresh events with vote_for_init=1");
    handlerRef = le_event_AddHandler("SimToolkitEventHandler",
                                     SimToolkitEvent,
                                     (le_event_HandlerFunc_t) handler);
    le_event_SetContextPtr(handlerRef, contextPtr);
    return (handlerRef);
}


//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to unregister the handler for SIM Toolkit event notification
 * handling.
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_RemoveSimToolkitEventHandler
(
    le_event_HandlerRef_t handlerRef
)
{
    if ((!handlerRef) || (!RefreshRegisterCount))
    {
        LE_ERROR("invalid handlerRef");
        return LE_FAULT;
    }

    le_event_RemoveHandler(handlerRef);

    RefreshRegisterCount--;

    if (!RefreshRegisterCount)
    {
        return EnableRefreshConfirmation(false);
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to confirm a SIM Toolkit command.
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_ConfirmSimToolkitCommand
(
    bool  confirmation ///< [IN] true to accept, false to reject
)
{
    switch (LastStkEvent.stkEvent)
    {
        case LE_SIM_OPEN_CHANNEL:
        {
            qmi_client_error_type clientMsgErr;
            SWIQMI_DECLARE_MESSAGE(cat_event_confirmation_req_msg_v02, confirmReq);
            SWIQMI_DECLARE_MESSAGE(cat_event_confirmation_resp_msg_v02, confirmResp);

            confirmReq.confirm_valid = 1;
            if (confirmation)
            {
                confirmReq.confirm.confirm = 0x01;
            }
            else
            {
                confirmReq.confirm.confirm = 0x00;
            }
            confirmReq.display_valid = 0;
            confirmReq.slot_valid = 1;
            confirmReq.slot.slot = CAT_SLOT1_V02;

            clientMsgErr = qmi_client_send_msg_sync(CatClient,
                                                    QMI_CAT_EVENT_CONFIRMATION_REQ_V02,
                                                    &confirmReq, sizeof(confirmReq),
                                                    &confirmResp, sizeof(confirmResp),
                                                    COMM_UICC_TIMEOUT);
            if (LE_OK != swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_CAT_EVENT_CONFIRMATION_REQ_V02),
                                              clientMsgErr,
                                              confirmResp.resp.result,
                                              confirmResp.resp.error))
            {
                LE_ERROR("Cannot confirm \"%s\" to SIM Toolkit OPEN_CHANNEL command",
                         (confirmation ? "yes":"no"));
                return LE_FAULT;
            }

            LE_DEBUG("Confirm \"%s\" to SIM Toolkit OPEN_CHANNEL command",
                     (confirmation ? "yes":"no"));
            return LE_OK;
        }
        break;

        case LE_SIM_REFRESH:
        {
            qmi_client_error_type clientMsgErr;
            SWIQMI_DECLARE_MESSAGE(uim_refresh_ok_req_msg_v01, refreshReq);
            SWIQMI_DECLARE_MESSAGE(uim_refresh_ok_resp_msg_v01, refreshResp);

            refreshReq.session_information.session_type = UIM_SESSION_TYPE_PRIMARY_GW_V01;
            refreshReq.session_information.aid_len = 0;
            if (confirmation)
            {
                refreshReq.ok_to_refresh = 1;
            }
            else
            {
                refreshReq.ok_to_refresh = 0;
            }

            clientMsgErr = qmi_client_send_msg_sync(UimClient,
                                                    QMI_UIM_REFRESH_OK_REQ_V01,
                                                    &refreshReq, sizeof(refreshReq),
                                                    &refreshResp, sizeof(refreshResp),
                                                    COMM_DEFAULT_PLATFORM_TIMEOUT);
            if (LE_OK != swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_UIM_REFRESH_OK_REQ_V01),
                                              clientMsgErr,
                                              refreshResp.resp.result,
                                              refreshResp.resp.error))
            {
                LE_ERROR("Cannot confirm \"%s\" to SIM Refresh", (confirmation ? "yes":"no"));
                return LE_FAULT;
            }

            LE_DEBUG("Confirm \"%s\" to SIM Refresh", (confirmation ? "yes":"no"));
            return LE_OK;
        }
        break;

        default:
            return LE_OK;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to send a generic command to the SIM.
 *
 * @return
 *      - LE_OK             Function succeeded.
 *      - LE_FAULT          The function failed.
 *      - LE_BAD_PARAMETER  A parameter is invalid.
 *      - LE_NOT_FOUND      - The function failed to select the SIM card for this operation
 *                          - The requested SIM file is not found
 *      - LE_OVERFLOW       Response buffer is too small to copy the SIM answer.
 *      - LE_UNSUPPORTED    The platform does not support this operation.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_SendCommand
(
    le_sim_Command_t command,               ///< [IN] The SIM command
    const char*      fileIdentifierPtr,     ///< [IN] File identifier
    uint8_t          p1,                    ///< [IN] Parameter P1 passed to the SIM
    uint8_t          p2,                    ///< [IN] Parameter P2 passed to the SIM
    uint8_t          p3,                    ///< [IN] Parameter P3 passed to the SIM
    const uint8_t*   dataPtr,               ///< [IN] Data command
    size_t           dataNumElements,       ///< [IN] Size of data command
    const char*      pathPtr,               ///< [IN] Path of the elementary file
    uint8_t*         sw1Ptr,                ///< [OUT] SW1 received from the SIM
    uint8_t*         sw2Ptr,                ///< [OUT] SW2 received from the SIM
    uint8_t*         responsePtr,           ///< [OUT] SIM response
    size_t*          responseNumElementsPtr ///< [IN/OUT] Size of response
)
{
    const SendSimCommand_t simCommand[LE_SIM_COMMAND_MAX] = {
                                                                 SendReadRecordCommand,
                                                                 SendReadBinaryCommand,
                                                                 SendUpdateRecordCommand,
                                                                 SendUpdateBinaryCommand
                                                            };

    if ( (!fileIdentifierPtr) || (!dataPtr) || (!pathPtr)
        || (!sw1Ptr) || (!sw2Ptr) || (!responsePtr)
        || (!responseNumElementsPtr) )
    {
        LE_ERROR("Bad parameters");
        return LE_FAULT;
    }

    if ((command < LE_SIM_COMMAND_MAX) && (simCommand[command] != NULL))
    {
        return simCommand[command](fileIdentifierPtr,
                                    p1,
                                    p2,
                                    p3,
                                    dataPtr,
                                    dataNumElements,
                                    pathPtr,
                                    sw1Ptr,
                                    sw2Ptr,
                                    responsePtr,
                                    responseNumElementsPtr);
    }

    return LE_FAULT;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to reset the SIM.
 *
 * @return
 *      - LE_OK             On success.
 *      - LE_FAULT          On failure.
 *      - LE_UNSUPPORTED    The platform does not support this operation.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_Reset
(
    void
)
{
#if defined(QMI_UIM_RESET_REQ_V01)
    SWIQMI_DECLARE_MESSAGE(uim_reset_resp_msg_v01, resetResp);

    LE_DEBUG("Call QMI_UIM_RESET_REQ_V01");

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(
                                            UimClient,
                                            QMI_UIM_RESET_REQ_V01,
                                            NULL, 0,
                                            &resetResp, sizeof(resetResp),
                                            COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_UIM_RESET_REQ_V01),
                             clientMsgErr,
                             resetResp.resp.result,
                             resetResp.resp.error) != LE_OK)
    {
        LE_ERROR("Cannot reset the SIM");
        return LE_FAULT;
    }

    LE_DEBUG("SIM reset successfully");
    return LE_OK;
#else
    LE_ERROR("Unable to reset the SIM on this platform");
    return LE_UNSUPPORTED;
#endif
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to write the FPLMN list into the modem.
 *
 * @return
 *      - LE_OK             On success.
 *      - LE_FAULT          On failure.
 *      - LE_BAD_PARAMETER  A parameter is invalid.
 *      - LE_UNSUPPORTED    The platform does not support this operation.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_WriteFPLMNList
(
    le_dls_List_t *FPLMNListPtr ///< [IN] List of FPLMN operators
)
{
#if defined(QMI_NAS_SWI_SET_FORBIDDEN_NETWORK_REQ_V01)
    size_t nbElem = le_dls_NumLinks(FPLMNListPtr);

    SWIQMI_DECLARE_MESSAGE(nas_set_forbidden_networks_req_msg_v01, setForbiddenNetworkReq);
    SWIQMI_DECLARE_MESSAGE(nas_set_forbidden_networks_resp_msg_v01, setForbiddenNetworkResp);

    if (NULL == FPLMNListPtr)
    {
        LE_ERROR("FPLMNListPtr is NULL !");
        return LE_BAD_PARAMETER;
    }

    if (nbElem > NAS_3GPP_FORBIDDEN_NETWORKS_LIST_MAX_V01)
    {
        nbElem = NAS_3GPP_FORBIDDEN_NETWORKS_LIST_MAX_V01;
    }

    setForbiddenNetworkReq.nas_3gpp_forbidden_networks_len = nbElem;
    setForbiddenNetworkReq.nas_3gpp_forbidden_networks_valid = true;

    LE_DEBUG("FPLMN list: len[%d]", setForbiddenNetworkReq.nas_3gpp_forbidden_networks_len);

    if (setForbiddenNetworkReq.nas_3gpp_forbidden_networks_len)
    {
        pa_sim_FPLMNOperator_t* nodePtr;
        le_dls_Link_t *linkPtr;

        int i;
        for(i = 0, linkPtr = le_dls_Peek(FPLMNListPtr);
            (i < setForbiddenNetworkReq.nas_3gpp_forbidden_networks_len) && (NULL != linkPtr);
            i++, linkPtr = le_dls_PeekNext(FPLMNListPtr, linkPtr))
        {
            // Get the node from FPLMNList
            nodePtr = CONTAINER_OF(linkPtr, pa_sim_FPLMNOperator_t, link);

            setForbiddenNetworkReq.nas_3gpp_forbidden_networks[i].mobile_country_code =
                                                                atoi(nodePtr->mobileCode.mcc);

            setForbiddenNetworkReq.nas_3gpp_forbidden_networks[i].mobile_network_code =
                                                                atoi(nodePtr->mobileCode.mnc);
        }
    }

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(
                    NasClient,
                    QMI_NAS_SWI_SET_FORBIDDEN_NETWORK_REQ_V01,
                    &setForbiddenNetworkReq, sizeof(setForbiddenNetworkReq),
                    &setForbiddenNetworkResp, sizeof(setForbiddenNetworkResp),
                    COMM_UICC_TIMEOUT);

    if (swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_NAS_SWI_SET_FORBIDDEN_NETWORK_REQ_V01),
                             clientMsgErr,
                             setForbiddenNetworkResp.resp.result,
                             setForbiddenNetworkResp.resp.error) != LE_OK)
    {
        LE_ERROR("Failed to write FPLMN List into the SIM");
        return LE_FAULT;
    }

    LE_DEBUG("FPLMN list is written into the SIM");
    return LE_OK;
#else
    LE_ERROR("Unable to write FPLMN list on this platform");
    return LE_UNSUPPORTED;
#endif
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to get the number of FPLMN operators present in the list.
 *
 * @return
 *      - LE_OK             On success.
 *      - LE_FAULT          On failure.
 *      - LE_BAD_PARAMETER  A parameter is invalid.
 *      - LE_UNSUPPORTED    The platform does not support this operation.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_CountFPLMNOperators
(
    uint32_t*  nbItemPtr     ///< [OUT] number of FPLMN operator found if success.
)
{
#if defined(QMI_NAS_GET_FORBIDDEN_NETWORKS_REQ_MSG_V01)
    if (NULL == nbItemPtr)
    {
        LE_ERROR("nbItemPtr is NULL");
        return LE_BAD_PARAMETER;
    }

    SWIQMI_DECLARE_MESSAGE(nas_get_forbidden_networks_resp_msg_v01, getForbiddenNetworkResp);

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(
                    NasClient,
                    QMI_NAS_GET_FORBIDDEN_NETWORKS_REQ_MSG_V01,
                    NULL, 0,
                    &getForbiddenNetworkResp, sizeof(getForbiddenNetworkResp),
                    COMM_UICC_TIMEOUT);

    if (swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_NAS_GET_FORBIDDEN_NETWORKS_REQ_MSG_V01),
                             clientMsgErr,
                             getForbiddenNetworkResp.resp.result,
                             getForbiddenNetworkResp.resp.error) != LE_OK)
    {
        LE_ERROR("Failed to retrieve FPLMN Operators List Information!");
        return LE_FAULT;
    }

    LE_DEBUG("Forbidden PLMN network valid: %c",
            (getForbiddenNetworkResp.nas_3gpp_forbidden_networks_valid ? 'Y' :'N'));

    // User FPLMN operators list
    if (getForbiddenNetworkResp.nas_3gpp_forbidden_networks_valid)
    {
        LE_DEBUG("3GPP Forbidden network information are valid with %d Operators",
                 getForbiddenNetworkResp.nas_3gpp_forbidden_networks_len);

        *nbItemPtr = getForbiddenNetworkResp.nas_3gpp_forbidden_networks_len;

        return LE_OK;
    }

    LE_ERROR("Failed to retrieve FPLMN Operators List Information!");
    return LE_FAULT;
#else
    LE_ERROR("Unable to retrieve FPLMN Operators List on this platform");
    return LE_UNSUPPORTED;
#endif
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to read the FPLMN list.
 *
 * @return
 *      - LE_OK             On success.
 *      - LE_NOT_FOUND      If no FPLMN network is available.
 *      - LE_BAD_PARAMETER  A parameter is invalid.
 *      - LE_UNSUPPORTED    The platform does not support this operation.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_ReadFPLMNOperators
(
    pa_sim_FPLMNOperator_t* FPLMNOperatorPtr,   ///< [OUT] FPLMN operators.
    uint32_t* FPLMNOperatorCountPtr             ///< [IN/OUT] FPLMN operator count.
)
{
#if defined(QMI_NAS_GET_FORBIDDEN_NETWORKS_REQ_MSG_V01)
    uint32_t i = 0;
    SWIQMI_DECLARE_MESSAGE(nas_get_forbidden_networks_resp_msg_v01, getForbiddenNetworkResp);

    if (NULL == FPLMNOperatorPtr)
    {
        LE_ERROR("FPLMNOperatorPtr is NULL !");
        return LE_BAD_PARAMETER;
    }

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(
                    NasClient,
                    QMI_NAS_GET_FORBIDDEN_NETWORKS_REQ_MSG_V01,
                    NULL, 0,
                    &getForbiddenNetworkResp, sizeof(getForbiddenNetworkResp),
                    COMM_UICC_TIMEOUT);

    if (swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_NAS_GET_FORBIDDEN_NETWORKS_REQ_MSG_V01),
                             clientMsgErr,
                             getForbiddenNetworkResp.resp.result,
                             getForbiddenNetworkResp.resp.error) != LE_OK)
    {
        LE_ERROR("Failed to retrieve FPLMN Operators List Information!");
        return LE_FAULT;
    }

    LE_DEBUG("Forbidden PLMN network valid: %c",
            (getForbiddenNetworkResp.nas_3gpp_forbidden_networks_valid ? 'Y' : 'N'));

    // User FPLMN operators list
    if (getForbiddenNetworkResp.nas_3gpp_forbidden_networks_valid)
    {
        if (getForbiddenNetworkResp.nas_3gpp_forbidden_networks_len < (*FPLMNOperatorCountPtr))
        {
            *FPLMNOperatorCountPtr = getForbiddenNetworkResp.nas_3gpp_forbidden_networks_len;
        }

        LE_DEBUG("3GPP Forbidden network information are valid with %d Operators",
                  (*FPLMNOperatorCountPtr));

        for (i = 0; i < *FPLMNOperatorCountPtr; i++)
        {
            ConvertMccMncIntoStringFormat(
                    getForbiddenNetworkResp.nas_3gpp_forbidden_networks[i].mobile_country_code,
                    getForbiddenNetworkResp.nas_3gpp_forbidden_networks[i].mobile_network_code,
                    false,
                    FPLMNOperatorPtr[i].mobileCode.mcc,
                    FPLMNOperatorPtr[i].mobileCode.mnc);

            LE_DEBUG("3GPP[%d] MCC.%s MNC.%s", i+1,
                     FPLMNOperatorPtr[i].mobileCode.mcc,
                     FPLMNOperatorPtr[i].mobileCode.mnc);
        }

        return LE_OK;
    }

    LE_ERROR("Failed to retrieve FPLMN Operators List Information!");
    return LE_NOT_FOUND;
#else
    LE_ERROR("Unable to retrieve FPLMN Operators List on this platform");
    return LE_UNSUPPORTED;
#endif
}

//--------------------------------------------------------------------------------------------------
/**
 * Retrieve the last SIM Toolkit status.
 *
 * @return
 *      - LE_OK             On success.
 *      - LE_BAD_PARAMETER  A parameter is invalid.
 *      - LE_UNSUPPORTED    The platform does not support this operation.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_GetLastStkStatus
(
    pa_sim_StkEvent_t*  stkStatus  ///< [OUT] last SIM Toolkit event status
)
{
    if (NULL == stkStatus)
    {
        return LE_BAD_PARAMETER;
    }

    memcpy(stkStatus, &LastStkEvent, sizeof(LastStkEvent));
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Powers up or down the current SIM card.
 *
 * @return
 *      - LE_OK           On success
 *      - LE_FAULT        For unexpected error
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_SetPower
(
    le_onoff_t power     ///< [IN] The power state.
)
{
    if (LE_ON == power)
    {
        SWIQMI_DECLARE_MESSAGE(uim_power_up_req_msg_v01, powerUpReq);
        SWIQMI_DECLARE_MESSAGE(uim_power_up_resp_msg_v01, powerUpResp);

        // When sending QMI_POWER_UP_REQ request, the value of powerUpReq.slot should be always
        // "0x1", then AMSS will operate this command with the slot currently selected.
        powerUpReq.slot = 0x1;

        qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(UimClient,
                                             QMI_UIM_POWER_UP_REQ_V01,
                                             &powerUpReq, sizeof(powerUpReq),
                                             &powerUpResp, sizeof(powerUpResp),
                                             COMM_UICC_TIMEOUT);

        if (LE_OK != swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_UIM_POWER_UP_REQ_V01), clientMsgErr,
                                          powerUpResp.resp.result, powerUpResp.resp.error))
        {
            LE_ERROR("Not able to power up current SIM!");
            return LE_FAULT;
        }

        return LE_OK;
    }
    else if (LE_OFF == power)
    {
        SWIQMI_DECLARE_MESSAGE(uim_power_down_req_msg_v01, powerDownReq);
        SWIQMI_DECLARE_MESSAGE(uim_power_down_resp_msg_v01, powerDownResp);

        // When sending QMI_POWER_DOWN_REQ request, the value of powerUpReq.slot should be always
        // "0x1", then AMSS will operate this command with the slot currently selected.
        powerDownReq.slot = 0x1;

        qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(UimClient,
                                             QMI_UIM_POWER_DOWN_REQ_V01,
                                             &powerDownReq, sizeof(powerDownReq),
                                             &powerDownResp, sizeof(powerDownResp),
                                             COMM_UICC_TIMEOUT);

        if (LE_OK != swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_UIM_POWER_DOWN_REQ_V01),
                                          clientMsgErr, powerDownResp.resp.result,
                                          powerDownResp.resp.error))
        {
            LE_ERROR("Not able to power down current SIM!");
            return LE_FAULT;
        }

        return LE_OK;
    }
    else
    {
        LE_ERROR("Incorrect power state: %d", power);
        return LE_FAULT;
    }
}
