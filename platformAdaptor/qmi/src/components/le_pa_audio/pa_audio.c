/**
 * @file pa_audio.c
 *
 * Control and configuration implementation of pa_audio API.
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#include "legato.h"
#include "interfaces.h"
#include "le_audio_local.h"
#include "pa_audio.h"
#include "media_routing.h"
#include "pa_audio_local.h"
#include <alsa-intf/alsa_audio.h>
#include <sys/ioctl.h>
#include "swiQmi.h"


//--------------------------------------------------------------------------------------------------
// Symbol and Enum definitions.
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Define the pool size.
 */
//--------------------------------------------------------------------------------------------------
#define THREADS_PARAMS_DEFAULT_POOL_SIZE        1

//--------------------------------------------------------------------------------------------------
/**
 * Define a default audio profile for Legato
 */
//--------------------------------------------------------------------------------------------------
#define LEGATO_AUDIO_PROFILE        0

//--------------------------------------------------------------------------------------------------
/**
 * Symbols used to specify the Audio calibration data base (ACDB).
 */
//--------------------------------------------------------------------------------------------------
#define QMI_ACDB_DEVICE_VEHICLE_HF  0
#define QMI_ACDB_DEVICE_HANDSET     1
#define QMI_ACDB_DEVICE_TTY         2
#define QMI_ACDB_DEVICE_USB         3
#define QMI_ACDB_DEVICE_NA          4

//--------------------------------------------------------------------------------------------------
/**
 * Symbols used to specify the Audio physical interface.
 */
//--------------------------------------------------------------------------------------------------
#define QMI_PIFACE_PCM              0
#define QMI_PIFACE_I2S              1
#define QMI_PIFACE_ANALOG           2
#define QMI_PIFACE_USB              3

//--------------------------------------------------------------------------------------------------
/**
 * Symbols used to specify gain coefficients.
 */
//--------------------------------------------------------------------------------------------------
#define GAIN_16BIT_MAX_VALUE  0xFFFF
#define GAIN_16BIT_COEFF      (GAIN_16BIT_MAX_VALUE/100)
#define GAIN_15BIT_MAX_VALUE  0x7FFF
#define GAIN_15BIT_COEFF      (GAIN_15BIT_MAX_VALUE/100)

//--------------------------------------------------------------------------------------------------
/**
 * Number of profiles.
 *
 * The message QMI_SWI_M2M_AUDIO_GET_AVCFG_RESP_V01 has been extended to define more profiles,
 * resulting in a change of name for the PROFILE_BIND_MAX_SIZE_V01 define that has been renamed
 * as QMI_SWI_M2M_PROFILE_MIN_SIZE_V01.
 */
//--------------------------------------------------------------------------------------------------
#ifndef QMI_SWI_M2M_PROFILE_MIN_SIZE_V01
#define QMI_SWI_M2M_PROFILE_MIN_SIZE_V01 PROFILE_BIND_MAX_SIZE_V01
#endif

//--------------------------------------------------------------------------------------------------
/**
 * Seconds to wait for QMI Audio ready indication before aborting initialization
 */
//--------------------------------------------------------------------------------------------------
#define AUDIO_READY_ABORT_TIMEOUT 60

//--------------------------------------------------------------------------------------------------
/**
 * Enum used to group Rx/Tx Legato interfaces of the same audio interface
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    AUDIO_IF_UNKNOWN,   ///< Unknown
    AUDIO_IF_ANALOG,    ///< Analog
    AUDIO_IF_USB,       ///< USB
    AUDIO_IF_MODEM,     ///< Modem
    AUDIO_IF_PCM,       ///< PCM
    AUDIO_IF_I2S,       ///< I2S
    AUDIO_IF_NUM        ///< Number of interfaces
}
AudioInterface_t;


//--------------------------------------------------------------------------------------------------
// Data structures.
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * The resource structure associated to the DTMF thread.
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    char     dtmf[LE_AUDIO_DTMF_MAX_BYTES]; ///< The DTMFs to play.
    uint32_t duration;                      ///< The DTMF duration in milliseconds.
    uint32_t pause;                         ///< The pause duration between tones in milliseconds.
}
PlayDtmfThreadResource_t;

//--------------------------------------------------------------------------------------------------
/**
 * PCM settings structure.
 *
 */
//--------------------------------------------------------------------------------------------------
typedef struct pcmSettings
{
    uint8_t mode;       ///< Mode: 0-slave, 1-master, 2-Auxiliary PCM
    uint8_t rate;       ///< Rate: 0-8KHz, 1-16KHz
    uint8_t bitsFrame;  ///< Bits-frame: 0-8 BPF, 1-16 BPF, 2-32 BPF, 3-64 BPF, 4-128 BPF, 5-256 BPF
    uint8_t companding; ///< Format: 0-Linear, 1- μ-law, 2- A-law
    uint8_t padding;    ///< Padding: 0-disable, 1-enable
}
PcmSettings_t;


//--------------------------------------------------------------------------------------------------
//                                       Static declarations
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * The QMI clients. Must be obtained by calling swiQmi_GetClientHandle() before it can be used.
 */
//--------------------------------------------------------------------------------------------------
static qmi_client_type AudioClient;

//--------------------------------------------------------------------------------------------------
/**
 * The QMI Voice client. Must be obtained by calling swiQmi_GetClientHandle() before it can be used.
 */
//--------------------------------------------------------------------------------------------------
static qmi_client_type VoiceClient;

//--------------------------------------------------------------------------------------------------
/**
 * The DMS (Device Management Service) client.
 */
//--------------------------------------------------------------------------------------------------
static qmi_client_type DmsClient;

//--------------------------------------------------------------------------------------------------
/**
 * The active QMI audio profile.
 */
//--------------------------------------------------------------------------------------------------
static uint8_t ActiveQmiProfile;

//--------------------------------------------------------------------------------------------------
/**
 * The current call's ID.
 */
//--------------------------------------------------------------------------------------------------
static uint8_t CurrentCallID;

//--------------------------------------------------------------------------------------------------
/**
 * The default ACDB value retrieved from the modem.
 */
//--------------------------------------------------------------------------------------------------
static uint8_t DefaultAcdbDevice;

//--------------------------------------------------------------------------------------------------
/**
 * The physical interface currently in use.
 */
//--------------------------------------------------------------------------------------------------
static uint8_t IfaceInUse;

//--------------------------------------------------------------------------------------------------
/**
 * The PCM settings in use.
 */
//--------------------------------------------------------------------------------------------------
static PcmSettings_t PcmSettings;

//--------------------------------------------------------------------------------------------------
/**
 * File event ID used to report file's events to the registered event handlers.
 */
//--------------------------------------------------------------------------------------------------
static le_event_Id_t StreamEventId;

//--------------------------------------------------------------------------------------------------
/**
 * Capture, playback threads and DTMF decoding flags
 */
//--------------------------------------------------------------------------------------------------
static uint32_t DtmfDecodingCount=0;

//--------------------------------------------------------------------------------------------------
/**
 * Memory Pool for PA parameters.
 *
 */
//--------------------------------------------------------------------------------------------------
static le_mem_PoolRef_t   PaParametersPool;

//--------------------------------------------------------------------------------------------------
/**
 * Memory Pool for DTMF encoding thread.
 *
 */
//--------------------------------------------------------------------------------------------------
static le_mem_PoolRef_t DtmfThreadParamsPool;

//--------------------------------------------------------------------------------------------------
/**
 * Reference of DTMF encoding thread.
 *
 */
//--------------------------------------------------------------------------------------------------
static le_thread_Ref_t  PlayDtmfThreadRef=NULL;

//--------------------------------------------------------------------------------------------------
/**
 * The DSP audio path matrix.
 */
//--------------------------------------------------------------------------------------------------
static int8_t  ConnectionMatrix[LE_AUDIO_NUM_INTERFACES][LE_AUDIO_NUM_INTERFACES];

//--------------------------------------------------------------------------------------------------
/**
 * Semaphore used to synchronized play signalling dtmf and call event.
 */
//--------------------------------------------------------------------------------------------------
static le_sem_Ref_t SignallingDtmfSemaphore;

//--------------------------------------------------------------------------------------------------
/**
 * Semaphore to synchronize component initialization.
 */
//--------------------------------------------------------------------------------------------------
static le_sem_Ref_t  InitSemaphore;

//--------------------------------------------------------------------------------------------------
/**
 * Flag to know the play signalling dtmf state.
 */
//--------------------------------------------------------------------------------------------------
static bool SignallingDtmfWaiting = false;

//--------------------------------------------------------------------------------------------------
/**
 * Flag to know the audio initialization state.
 * It allows returning LE_UNAVAILABLE at the Audio API level if audio service init failed.
 */
//--------------------------------------------------------------------------------------------------
static bool IsAudioAvailable = false;

//--------------------------------------------------------------------------------------------------
/**
 * Table associating a Rx/Tx Legato audio interface to an audio interface
 */
//--------------------------------------------------------------------------------------------------
static AudioInterface_t ConvertAudioInterface[LE_AUDIO_NUM_INTERFACES] =
{
    AUDIO_IF_ANALOG,    ///< LE_AUDIO_IF_CODEC_MIC
    AUDIO_IF_ANALOG,    ///< LE_AUDIO_IF_CODEC_SPEAKER
    AUDIO_IF_USB,       ///< LE_AUDIO_IF_DSP_FRONTEND_USB_RX
    AUDIO_IF_USB,       ///< LE_AUDIO_IF_DSP_FRONTEND_USB_TX
    AUDIO_IF_MODEM,     ///< LE_AUDIO_IF_DSP_BACKEND_MODEM_VOICE_RX
    AUDIO_IF_MODEM,     ///< LE_AUDIO_IF_DSP_BACKEND_MODEM_VOICE_TX
    AUDIO_IF_PCM,       ///< LE_AUDIO_IF_DSP_FRONTEND_PCM_RX
    AUDIO_IF_PCM,       ///< LE_AUDIO_IF_DSP_FRONTEND_PCM_TX
    AUDIO_IF_I2S,       ///< LE_AUDIO_IF_DSP_FRONTEND_I2S_RX
    AUDIO_IF_I2S,       ///< LE_AUDIO_IF_DSP_FRONTEND_I2S_TX
    AUDIO_IF_UNKNOWN,   ///< LE_AUDIO_IF_DSP_FRONTEND_FILE_PLAY
    AUDIO_IF_UNKNOWN    ///< LE_AUDIO_IF_DSP_FRONTEND_FILE_CAPTURE
};

//--------------------------------------------------------------------------------------------------
/**
 * This function initializes the Connection Matrix as follows:
 *
 *
 * IN\OUT         |  MODEM_VOICE_TX   |       USB_TX       |       SPEAKER          |     PCM_TX      |      I2S_TX        |
 * ------------------------------------------------------------------------------------------------------------------------|
 * MODEM_VOICE_RX |      N/A          |  QMI_PIFACE_USB    |   QMI_PIFACE_ANALOG    |  QMI_PIFACE_PCM |   QMI_PIFACE_I2S   |
 * ------------------------------------------------------------------------------------------------------------------------|
 * USB_RX         |  QMI_PIFACE_USB   |      N/A           |        N/A             |       N/A       |         N/A        |
 * ------------------------------------------------------------------------------------------------------------------------|
 * PCM_RX         |  QMI_PIFACE_PCM   |      N/A           |        N/A             |       N/A       |         N/A        |
 * ------------------------------------------------------------------------------------------------------------------------|
 * I2S_RX         |  QMI_PIFACE_I2S   |      N/A           |        N/A             |       N/A       |         N/A        |
 * ------------------------------------------------------------------------------------------------------------------------|
 * MIC            | QMI_PIFACE_ANALOG |      N/A           |        N/A             |       N/A       |         N/A        |
 * ------------------------------------------------------------------------------------------------------------------------|
 *
 */
//--------------------------------------------------------------------------------------------------
static void InitializeConnectionMatrix
(
    void
)
{
    memset(ConnectionMatrix, -1, sizeof(ConnectionMatrix));

    ConnectionMatrix[LE_AUDIO_IF_DSP_BACKEND_MODEM_VOICE_RX][LE_AUDIO_IF_CODEC_SPEAKER]
                   = QMI_PIFACE_ANALOG;
    ConnectionMatrix[LE_AUDIO_IF_CODEC_MIC][LE_AUDIO_IF_DSP_BACKEND_MODEM_VOICE_TX]
                   = QMI_PIFACE_ANALOG;
    ConnectionMatrix[LE_AUDIO_IF_DSP_BACKEND_MODEM_VOICE_RX][LE_AUDIO_IF_DSP_FRONTEND_USB_TX]
                   = QMI_PIFACE_USB;
    ConnectionMatrix[LE_AUDIO_IF_DSP_FRONTEND_USB_RX][LE_AUDIO_IF_DSP_BACKEND_MODEM_VOICE_TX]
                   = QMI_PIFACE_USB;
    ConnectionMatrix[LE_AUDIO_IF_DSP_BACKEND_MODEM_VOICE_RX][LE_AUDIO_IF_DSP_FRONTEND_PCM_TX]
                   = QMI_PIFACE_PCM;
    ConnectionMatrix[LE_AUDIO_IF_DSP_FRONTEND_PCM_RX][LE_AUDIO_IF_DSP_BACKEND_MODEM_VOICE_TX]
                   = QMI_PIFACE_PCM;
    ConnectionMatrix[LE_AUDIO_IF_DSP_BACKEND_MODEM_VOICE_RX][LE_AUDIO_IF_DSP_FRONTEND_I2S_TX]
                   = QMI_PIFACE_I2S;
    ConnectionMatrix[LE_AUDIO_IF_DSP_FRONTEND_I2S_RX][LE_AUDIO_IF_DSP_BACKEND_MODEM_VOICE_TX]
                   = QMI_PIFACE_I2S;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the private PA parameters of a stream
 *
 */
//--------------------------------------------------------------------------------------------------
PaParameters_t* GetPaParams
(
    le_audio_Stream_t* streamPtr
)
{
    if (streamPtr->PaParams == NULL)
    {
        streamPtr->PaParams = le_mem_ForceAlloc(PaParametersPool);
        memset(streamPtr->PaParams, 0, sizeof(PaParameters_t));
        media_routing_SetPaParams(&(((PaParameters_t*)streamPtr->PaParams)->mediaRoutingParams));
    }

    return (PaParameters_t*) streamPtr->PaParams;
}

//--------------------------------------------------------------------------------------------------
/**
 * Handler function called when there is an All Call Status indication.
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
    LE_WARN("CallHandler called");
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

    if (allCallInd.call_info_len > 0)
    {
        voice_call_info2_type_v02 * callInfoPtr = &allCallInd.call_info[0];

        if ((callInfoPtr->call_state == CALL_STATE_END_V02) ||
            (callInfoPtr->call_state == CALL_STATE_DISCONNECTING_V02))
        {
            CurrentCallID = 0;

            if (PlayDtmfThreadRef)
            {
                LE_WARN("Stop PlayDtmfThread");
                le_thread_Cancel(PlayDtmfThreadRef);
                le_thread_Join(PlayDtmfThreadRef,NULL);
            }
        }
        // Get the call id of the call which is connected
        else if (callInfoPtr->call_state == CALL_STATE_CONVERSATION_V02)
        {
            CurrentCallID = callInfoPtr->call_id;

            if (SignallingDtmfWaiting)
            {
                le_sem_Post(SignallingDtmfSemaphore);
            }
        }
        // in other cases, we have no call in progress
        else
        {
            CurrentCallID = 0;
        }

        LE_WARN("Identify call ID %d", CurrentCallID);
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * The first-layer File Event Handler.
 *
 */
//--------------------------------------------------------------------------------------------------
static void FirstLayerFileEventHandler
(
    void* reportPtr,
    void* secondLayerHandlerFunc
)
{
    le_audio_StreamEvent_t*           streamEventPtr = reportPtr;
    le_audio_DtmfStreamEventHandlerFunc_t clientHandlerFunc = secondLayerHandlerFunc;

    LE_DEBUG("stream event detected");

    clientHandlerFunc(streamEventPtr, le_event_GetContextPtr());
}

//--------------------------------------------------------------------------------------------------
/**
 * Handler function called when a DTMF character is detected on the Modem Voice call interface
 * downlink.
 *
 */
//--------------------------------------------------------------------------------------------------
static void DtmfRxHandler
(
    void*           indBufPtr,  // [IN] The indication message data.
    unsigned int    indBufLen,  // [IN] The indication message data length in bytes.
    void*           contextPtr  // [IN] Context value that was passed to
                                //      swiQmi_AddIndicationHandler().
)
{
    LE_DEBUG("%d", DtmfDecodingCount);

    if (DtmfDecodingCount)
    {
        if (indBufPtr == NULL)
        {
            LE_ERROR("Indication message empty.");
            return;
        }

        swi_m2m_audio_dtmf_ind_msg_v01 dtmfInd;

        qmi_client_error_type clientMsgErr = qmi_client_message_decode(
            AudioClient, QMI_IDL_INDICATION, QMI_SWI_M2M_AUDIO_DTMF_IND_V01,
            indBufPtr, indBufLen,
            &dtmfInd, sizeof(dtmfInd));

        if (clientMsgErr != QMI_NO_ERR)
        {
            LE_ERROR("Error in decoding QMI_SWI_M2M_AUDIO_DTMF_IND_V01, error = %i", clientMsgErr);
            return;
        }

        LE_DEBUG("DTMF.%c detected", dtmfInd.DTMF_Value);

        le_audio_StreamEvent_t streamEvent;

        // streamPtr will be reported into th handler context
        streamEvent.streamPtr = NULL;
        streamEvent.streamEvent = LE_AUDIO_BITMASK_DTMF_DETECTION;
        streamEvent.event.dtmf = dtmfInd.DTMF_Value;

        le_event_Report(StreamEventId, &streamEvent, sizeof(le_audio_StreamEvent_t));
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * This thread plays the DTMFs.
 */
//--------------------------------------------------------------------------------------------------
static void* PlayDtmfThread
(
    void* contextPtr
)
{
    PlayDtmfThreadResource_t*  resourcePtr = contextPtr;
    qmi_client_error_type      clientMsgErr;
    le_result_t                result;
    uint32_t                   duration = resourcePtr->duration;
    uint32_t                   pause = resourcePtr->pause;
    size_t                     dtmfLen = strlen(resourcePtr->dtmf);
    uint32_t                   i;

    voice_start_cont_dtmf_req_msg_v02  dtmfStartReq = {{0}};
    voice_start_cont_dtmf_resp_msg_v02 dtmfStartResp = {{0}};
    voice_stop_cont_dtmf_req_msg_v02  dtmfStopReq = {0};
    voice_stop_cont_dtmf_resp_msg_v02 dtmfStopResp = {{0}};

    for (i=0 ; i< dtmfLen ; i++)
    {
        dtmfStartReq.cont_dtmf_info.call_id = CurrentCallID;
        dtmfStartReq.cont_dtmf_info.digit = toupper(resourcePtr->dtmf[i]);

        LE_INFO("Start Signalling DTMF %c playback on Call ID.%d",
                dtmfStartReq.cont_dtmf_info.digit, CurrentCallID);
        clientMsgErr = qmi_client_send_msg_sync(VoiceClient,
                                                QMI_VOICE_START_CONT_DTMF_REQ_V02,
                                                &dtmfStartReq, sizeof(dtmfStartReq),
                                                &dtmfStartResp, sizeof(dtmfStartResp),
                                                COMM_DEFAULT_PLATFORM_TIMEOUT);

        result = swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_VOICE_START_CONT_DTMF_REQ_V02),
                                            clientMsgErr,
                                            dtmfStartResp.resp.result,
                                            dtmfStartResp.resp.error);

        if (result == LE_OK)
        {
            sleep (duration/1000);
            usleep ((duration%1000)*1000);
            LE_INFO("DTMF Signalling %c has been played on Call ID.%d",
                    dtmfStartReq.cont_dtmf_info.digit, CurrentCallID);
            dtmfStopReq.call_id = CurrentCallID;
            clientMsgErr = qmi_client_send_msg_sync(VoiceClient,
                                                    QMI_VOICE_STOP_CONT_DTMF_REQ_V02,
                                                    &dtmfStopReq, sizeof(dtmfStopReq),
                                                    &dtmfStopResp, sizeof(dtmfStopResp),
                                                    COMM_DEFAULT_PLATFORM_TIMEOUT);

            result = swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_VOICE_STOP_CONT_DTMF_REQ_V02),
                                                clientMsgErr,
                                                dtmfStopResp.resp.result,
                                                dtmfStopResp.resp.error);

            if (result == LE_OK)
            {
                sleep (pause/1000);
                usleep ((pause%1000)*1000);
            }
            else
            {
                LE_ERROR("Cannot stop the DTMF signalling!");
                return NULL;
            }
        }
        else
        {
            LE_ERROR("Cannot play the DTMF!");
            return NULL;
        }
    }

    return NULL;
}

//--------------------------------------------------------------------------------------------------
/**
 * Play DTMF thread destructor
 *
 */
//--------------------------------------------------------------------------------------------------
static void DestroyPlayDtmfThread
(
    void *contextPtr
)
{
    PlayDtmfThreadRef = NULL;
    LE_DEBUG("PlayDtmf Thread stopped");
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to enable/disable the DTMF detection.
 *
 * @return LE_OK              on success
 * @return LE_UNAVAILABLE     on audio service initialization failure
 * @return LE_FAULT           on any other failure
 */
//--------------------------------------------------------------------------------------------------
static le_result_t EnableDtmfDetection
(
    bool enable     ///< [IN] true to enable the DTMF detection, false otherwise
)
{
    // Set the indications to detect DTMF.
    swi_m2m_audio_set_wddm_req_msg_v01 indRegReq = {0};
    swi_m2m_audio_set_wddm_resp_msg_v01 indRegResp = { {0} };

    if (enable)
    {
        indRegReq.enable = 1;
    }
    else
    {
        indRegReq.enable = 0;
    }

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(AudioClient,
                                            QMI_SWI_M2M_AUDIO_SET_WDDM_REQ_V01,
                                            &indRegReq, sizeof(indRegReq),
                                            &indRegResp, sizeof(indRegResp),
                                            COMM_DEFAULT_PLATFORM_TIMEOUT);

    if ( swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_SWI_M2M_AUDIO_SET_WDDM_REQ_V01),
                                    clientMsgErr,
                                    indRegResp.resp.result,
                                    indRegResp.resp.error) != LE_OK )
    {
        LE_ERROR("Cannot activate the DTMF detection");
        return IsAudioAvailable ? LE_FAULT : LE_UNAVAILABLE;
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Initialize the audio client when QMI Audio service is ready
 */
//--------------------------------------------------------------------------------------------------
static void InitAudioClient
(
    void
)
{
    uint32_t i = 0;

    // Get active profile
    uint32_t profile;
    qmi_client_error_type clientMsgErr;
    if (LE_OK != pa_audio_GetProfile(&profile))
    {
        ActiveQmiProfile = LEGATO_AUDIO_PROFILE;
        LE_WARN("Cannot retrieve active QMI audio profile value, set it to %d", ActiveQmiProfile);

        // Set the profile.
        SWIQMI_DECLARE_MESSAGE(swi_m2m_audio_set_profile_req_msg_v01, profileSetReq);
        SWIQMI_DECLARE_MESSAGE(swi_m2m_audio_set_profile_resp_msg_v01, profileSetResp);

        profileSetReq.profile = ActiveQmiProfile;
        profileSetReq.earmute_valid = true;
        profileSetReq.earmute = 0;
        profileSetReq.micmute_valid = true;
        profileSetReq.micmute = 0;
        profileSetReq.generator_valid = true;
        profileSetReq.generator = 0;
        profileSetReq.volume_valid = true;
        profileSetReq.volume = 3;
        profileSetReq.cwtmute_valid = true;
        profileSetReq.cwtmute = 0;

        clientMsgErr = qmi_client_send_msg_sync(AudioClient,
                                                QMI_SWI_M2M_AUDIO_SET_PROFILE_REQ_V01,
                                                &profileSetReq, sizeof(profileSetReq),
                                                &profileSetResp, sizeof(profileSetResp),
                                                COMM_DEFAULT_PLATFORM_TIMEOUT);

        if ( swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_SWI_M2M_AUDIO_SET_PROFILE_REQ_V01),
                                        clientMsgErr,
                                        profileSetResp.resp.result,
                                        profileSetResp.resp.error) != LE_OK)
        {
            LE_ERROR("Cannot activate the QMI audio profile value to .%d", ActiveQmiProfile);
        }
    }
    else
    {
        ActiveQmiProfile = (uint8_t)profile;
        LE_INFO("Active QMI audio profile for audio is %d", ActiveQmiProfile);
    }

    // Get AVCFG values
    SWIQMI_DECLARE_MESSAGE(swi_m2m_audio_get_avcfg_resp_msg_v01, avcfgGetResp);

    clientMsgErr = qmi_client_send_msg_sync(AudioClient,
                                            QMI_SWI_M2M_AUDIO_GET_AVCFG_REQ_V01,
                                            NULL, 0,
                                            &avcfgGetResp, sizeof(avcfgGetResp),
                                            COMM_DEFAULT_PLATFORM_TIMEOUT);

    if ( swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_SWI_M2M_AUDIO_GET_AVCFG_REQ_V01),
                                     clientMsgErr,
                                     avcfgGetResp.resp.result,
                                     avcfgGetResp.resp.error) != LE_OK)
    {
        DefaultAcdbDevice = 0; // Vehicle HF (Handsfree)
        LE_WARN("Cannot retrieve ACDB value, set it to %d", DefaultAcdbDevice);
    }

    else
    {
        for (i = 0; i < QMI_SWI_M2M_PROFILE_MIN_SIZE_V01; i++)
        {
            if (avcfgGetResp.bind_info[i].profile == ActiveQmiProfile)
            {
                DefaultAcdbDevice = avcfgGetResp.bind_info[i].device;
            }
        }

#ifdef QMI_SWI_M2M_PROFILE_EXT_SIZE_V01
        if(avcfgGetResp.bind_info_ext_valid)
        {
            for (i = 0; i < avcfgGetResp.bind_info_ext_len; i++)
            {
                if (avcfgGetResp.bind_info_ext[i].profile == ActiveQmiProfile)
                {
                    DefaultAcdbDevice = avcfgGetResp.bind_info_ext[i].device;
                }
            }
        }
#endif
    }

    IfaceInUse = -1;

    LE_INFO("DefaultAcdbDevice.%d", DefaultAcdbDevice);

    // Provision default settings for PCM interface
    PcmSettings.mode = 1;       // master,
    PcmSettings.rate = 0;       // 8KHz
    PcmSettings.bitsFrame = 5;  // 256 Bits-frame
    PcmSettings.companding = 0; // Linear
    PcmSettings.padding = 0;    // disable

    // Register our own DTMF handler with the QMI audio service.
    LE_ERROR_IF((swiQmi_AddIndicationHandler(DtmfRxHandler,
                                             QMI_SERVICE_AUDIO,
                                             QMI_SWI_M2M_AUDIO_DTMF_IND_V01,
                                             NULL) != LE_OK),
                "Cannot Add Indication Handler for DTMF detection!");

    // Register our own call handler with the QMI voice service.
    LE_ERROR_IF((swiQmi_AddIndicationHandler(CallHandler,
                                             QMI_SERVICE_VOICE,
                                             QMI_VOICE_ALL_CALL_STATUS_IND_V02,
                                             NULL) != LE_OK),
                "Cannot Add Indication Handler for Voice Call!");

    // Set the indications to receive for the voice service.
    SWIQMI_DECLARE_MESSAGE(voice_indication_register_req_msg_v02, voiceIndRegReq);
    SWIQMI_DECLARE_MESSAGE(voice_indication_register_resp_msg_v02, voiceIndRegResp);

    // Set the call notification event.
    voiceIndRegReq.call_events_valid = true;
    voiceIndRegReq.call_events = 1;

    clientMsgErr = qmi_client_send_msg_sync(VoiceClient,
                                            QMI_VOICE_INDICATION_REGISTER_REQ_V02,
                                            &voiceIndRegReq, sizeof(voiceIndRegReq),
                                            &voiceIndRegResp, sizeof(voiceIndRegResp),
                                            COMM_DEFAULT_PLATFORM_TIMEOUT);

    swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_VOICE_INDICATION_REGISTER_REQ_V02),
                             clientMsgErr,
                             voiceIndRegResp.resp.result,
                             voiceIndRegResp.resp.error);
    IsAudioAvailable = true;
}

#ifdef QMI_SWI_M2M_AUDIO_INDICATION_REGISTER_REQ_V01
//--------------------------------------------------------------------------------------------------
/**
 * Handler function called when a QMI Audio is ready to service requests
 */
//--------------------------------------------------------------------------------------------------
static void AudioReadyHandler
(
    void*           indBufPtr,  // [IN] The indication message data.
    unsigned int    indBufLen,  // [IN] The indication message data length in bytes.
    void*           contextPtr  // [IN] Context value passed to swiQmi_AddIndicationHandler().
)
{
    if (indBufPtr == NULL)
    {
        LE_ERROR("Indication message empty.");
        return;
    }

    SWIQMI_DECLARE_MESSAGE(swi_m2m_audio_ready_ind_msg_v01, audioReadyInd);

    qmi_client_error_type clientMsgErr = qmi_client_message_decode(
        AudioClient, QMI_IDL_INDICATION, QMI_SWI_M2M_AUDIO_READY_IND_V01,
        indBufPtr, indBufLen, &audioReadyInd, sizeof(audioReadyInd));

    if (clientMsgErr != QMI_NO_ERR)
    {
        LE_ERROR("Error in decoding QMI_SWI_M2M_AUDIO_READY_IND_V01, error = %i", clientMsgErr);
        return;
    }

    LE_INFO("QMI Audio %s indication received", audioReadyInd.av_ready ? "READY" : "NOT READY");
    // Post semaphore to resume PA Audio init
    le_sem_Post(InitSemaphore);
}
#endif

//--------------------------------------------------------------------------------------------------
//                                       Public declarations
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to initialize the PA Audio module.
 */
//--------------------------------------------------------------------------------------------------
COMPONENT_INIT
{
    // Create a semaphore to synchronize initialization
    InitSemaphore = le_sem_Create("InitSem", 0);

    // Allocate the audio threads params pool.
    DtmfThreadParamsPool = le_mem_CreatePool("DtmfThreadParamsPool",
                                             sizeof(PlayDtmfThreadResource_t));
    le_mem_ExpandPool(DtmfThreadParamsPool, THREADS_PARAMS_DEFAULT_POOL_SIZE);

    // Allocate the private parameters pool
    PaParametersPool = le_mem_CreatePool("PaParametersPool", sizeof(PaParameters_t));
    le_mem_ExpandPool(PaParametersPool, LE_AUDIO_NUM_INTERFACES);

    // Create the event for File playback/recording handlers.
    StreamEventId = le_event_CreateId("AudioFileEvent", sizeof(le_audio_StreamEvent_t));

    // Create a semaphore to coordinate the dtmf signalling
    SignallingDtmfSemaphore = le_sem_Create("SignallingDtmfSemaphore",0);

    // Start the qmi service.
    if (swiQmi_InitServices(QMI_SERVICE_AUDIO) != LE_OK)
    {
        LE_CRIT("Could not initialize QMI_SERVICE_AUDIO service");
        return;
    }

    if (swiQmi_InitServices(QMI_SERVICE_VOICE) != LE_OK)
    {
        LE_CRIT("Could not initialize the QMI_SERVICE_VOICE Services.");
        // I don't return since the major part of the functions can work without QMI_SERVICE_VOICE
    }

    if (swiQmi_InitServices(QMI_SERVICE_DMS) != LE_OK)
    {
        LE_CRIT("Could not initialize the QMI_SERVICE_DMS Services.");
        // I don't return since the major part of the functions can work without QMI_SERVICE_DMS
    }

    // Get the qmi client service handle for later use.
    if ( (AudioClient = swiQmi_GetClientHandle(QMI_SERVICE_AUDIO)) == NULL)
    {
        LE_CRIT("Cannot get QMI_SERVICE_AUDIO client!");
        return;
    }
    if ( (VoiceClient = swiQmi_GetClientHandle(QMI_SERVICE_VOICE)) == NULL)
    {
        LE_CRIT("Cannot get QMI_SERVICE_VOICE client!");
        // I don't return since the major part of the functions can work without QMI_SERVICE_VOICE
    }

    if ( (DmsClient = swiQmi_GetClientHandle(QMI_SERVICE_DMS)) == NULL)
    {
        LE_CRIT("Cannot get QMI_SERVICE_DMS client!");
        // I don't return since the major part of the functions can work without QMI_SERVICE_DMS
    }

    CurrentCallID = 0;

    InitializeConnectionMatrix();

    media_routing_Initialize();

#ifdef QMI_SWI_M2M_AUDIO_INDICATION_REGISTER_REQ_V01
    SWIQMI_DECLARE_MESSAGE(swi_m2m_audio_indication_register_req_msg_v01, audioIndReq);
    SWIQMI_DECLARE_MESSAGE(swi_m2m_audio_indication_register_resp_msg_v01, audioIndResp);
    audioIndReq.av_ready_event_valid = true;
    audioIndReq.av_ready_event = 1;
    audioIndReq.dtmf_events_valid = true;
    audioIndReq.dtmf_events = 1;

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(AudioClient,
                                            QMI_SWI_M2M_AUDIO_INDICATION_REGISTER_REQ_V01,
                                            &audioIndReq, sizeof(audioIndReq),
                                            &audioIndResp, sizeof(audioIndResp),
                                            COMM_DEFAULT_PLATFORM_TIMEOUT);

    if(swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_SWI_M2M_AUDIO_INDICATION_REGISTER_REQ_V01),
                            clientMsgErr,
                            audioIndResp.resp.result,
                            audioIndResp.resp.error) != LE_OK)
    {
        LE_ERROR("Cannot request registration to audio service indications");
    }

    // Register an indication handler to be notified when QMI Audio service is ready
    LE_ERROR_IF((swiQmi_AddIndicationHandler(AudioReadyHandler,
                                             QMI_SERVICE_AUDIO,
                                             QMI_SWI_M2M_AUDIO_READY_IND_V01,
                                             NULL) != LE_OK),
                "Cannot add indication handler for QMI Audio service readiness");

    // Suspend init until QMI Audio ready notification is received
    le_clk_Time_t timeout = {AUDIO_READY_ABORT_TIMEOUT, 0};
    LE_INFO("PA Audio waiting for QMI Audio to be ready");
    le_result_t res = le_sem_WaitWithTimeOut(InitSemaphore, timeout);

    if (LE_OK == res)
    {
        // Finish PA Audio initialization
        InitAudioClient();
        LE_INFO("PA Audio init done");
    }
    else
    {
        LE_WARN("QMI Audio ready indication not received: initialization aborted");
        return;
    }
#else
        // Finish PA Audio initialization
        InitAudioClient();
        LE_INFO("PA Audio init done");
#endif
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to set the timeslot number of a PCM interface.
 *
 * @return LE_BAD_PARAMETER The audio stream is not valid.
 * @return LE_FAULT         The function failed to set the timeslot number.
 * @return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_audio_SetPcmTimeSlot
(
    le_audio_Stream_t* streamPtr,
    uint32_t           timeslot
)
{
    if (streamPtr == NULL)
    {
        return LE_BAD_PARAMETER;
    }

    le_audio_If_t interface = streamPtr->audioInterface;



    LE_DEBUG("Use timeslot.%d for interface.%d", timeslot, interface);

    switch (interface)
    {
        case LE_AUDIO_IF_DSP_FRONTEND_PCM_RX:
        case LE_AUDIO_IF_DSP_FRONTEND_PCM_TX:
        {
            if (timeslot > 0)
            {
                LE_ERROR("Timeslot %zu is out of range (>0).", timeslot);
                return LE_FAULT;
            }
            else
            {
                return LE_OK;
            }
        }
        case LE_AUDIO_IF_CODEC_SPEAKER:
        case LE_AUDIO_IF_CODEC_MIC:
        case LE_AUDIO_IF_DSP_FRONTEND_USB_RX:
        case LE_AUDIO_IF_DSP_FRONTEND_USB_TX:
        case LE_AUDIO_IF_DSP_BACKEND_MODEM_VOICE_RX:
        case LE_AUDIO_IF_DSP_BACKEND_MODEM_VOICE_TX:
        case LE_AUDIO_IF_DSP_FRONTEND_I2S_RX:
        case LE_AUDIO_IF_DSP_FRONTEND_I2S_TX:
        case LE_AUDIO_IF_DSP_FRONTEND_FILE_PLAY:
        case LE_AUDIO_IF_DSP_FRONTEND_FILE_CAPTURE:
        case LE_AUDIO_NUM_INTERFACES:
        {
            break;
        }
    }
    LE_ERROR("This interface (%d) is not supported",interface);
    return LE_FAULT;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to set the channel mode of an I2S interface.
 *
 * @return LE_BAD_PARAMETER The audio stream or interface is not valid.
 * @return LE_FAULT         The function failed to set the channel mode.
 * @return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_audio_SetI2sChannelMode
(
    le_audio_Stream_t*     streamPtr,
    le_audio_I2SChannel_t  mode
)
{
    if (streamPtr == NULL)
    {
        return LE_BAD_PARAMETER;
    }

    le_audio_If_t interface = streamPtr->audioInterface;

    LE_DEBUG("Use channel mode.%d for interface.%d", mode, interface);

    switch (interface)
    {
        case LE_AUDIO_IF_DSP_FRONTEND_I2S_RX:
        case LE_AUDIO_IF_DSP_FRONTEND_I2S_TX:
        {
            if (mode != LE_AUDIO_I2S_STEREO)
            {
                LE_ERROR("This I2S mode (%d) is not supported on this platform", mode);
                return LE_FAULT;
            }
            else
            {
                return LE_OK;
            }
        }
        case LE_AUDIO_IF_CODEC_SPEAKER:
        case LE_AUDIO_IF_CODEC_MIC:
        case LE_AUDIO_IF_DSP_FRONTEND_USB_RX:
        case LE_AUDIO_IF_DSP_FRONTEND_USB_TX:
        case LE_AUDIO_IF_DSP_BACKEND_MODEM_VOICE_RX:
        case LE_AUDIO_IF_DSP_BACKEND_MODEM_VOICE_TX:
        case LE_AUDIO_IF_DSP_FRONTEND_PCM_RX:
        case LE_AUDIO_IF_DSP_FRONTEND_PCM_TX:
        case LE_AUDIO_IF_DSP_FRONTEND_FILE_PLAY:
        case LE_AUDIO_IF_DSP_FRONTEND_FILE_CAPTURE:
        case LE_AUDIO_NUM_INTERFACES:
        {
            break;
        }
    }
    LE_ERROR("This interface (%d) is not supported",interface);
    return LE_FAULT;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to start the DTMF Decoder.
 *
 * @return LE_OK            The decoder is started
 * @return LE_BAD_PARAMETER The audio stream or interface is not valid
 * @return LE_FAULT         On other errors
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_audio_StartDtmfDecoder
(
    le_audio_Stream_t* streamPtr
)
{
    if (streamPtr == NULL)
    {
        return LE_BAD_PARAMETER;
    }

    le_audio_If_t interface = streamPtr->audioInterface;

    LE_DEBUG("pa_audio_StartDtmfDecoder called, DtmfDecodingCount %d.", DtmfDecodingCount);

    if (interface != LE_AUDIO_IF_DSP_BACKEND_MODEM_VOICE_RX)
    {
        LE_ERROR("DTMF detection is not supported on interface.%d!", interface);
        return LE_BAD_PARAMETER;
    }
    else
    {
        if (CurrentCallID)
        {
            LE_ERROR("Cannot activate DTMF detection while a call is in progress!");
            return LE_FAULT;
        }
        else
        {
            DtmfDecodingCount++;
            if(DtmfDecodingCount == 1)
            {
                return (EnableDtmfDetection(true));
            }
            return LE_OK;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to stop the DTMF Decoder.
 *
 * @return LE_OK            The decoder is started
 * @return LE_BAD_PARAMETER The interface is not valid
 * @return LE_FAULT         On other errors
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_audio_StopDtmfDecoder
(
    le_audio_Stream_t* streamPtr   ///< [IN] input audio stream
)
{
    if (streamPtr == NULL)
    {
        return LE_BAD_PARAMETER;
    }

    le_audio_If_t interface = streamPtr->audioInterface;

    LE_DEBUG("pa_audio_StopDtmfDecoder called DtmfDecodingCount %d.", DtmfDecodingCount);
    if (interface == LE_AUDIO_IF_DSP_BACKEND_MODEM_VOICE_RX)
    {
        if (DtmfDecodingCount)
        {
            DtmfDecodingCount--;

            if(!DtmfDecodingCount)
            {
                return (EnableDtmfDetection(false));
            }
        }
        return LE_OK;
    }
    else
    {
        return LE_BAD_PARAMETER;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to enable or disable the Noise Suppressor.
 *
 * @return LE_BAD_PARAMETER The audio stream or interface is invalid.
 * @return LE_UNAVAILABLE   The audio service initialization failed.
 * @return LE_FAULT         On any other failure.
 * @return LE_OK            Function succeeded.
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_audio_NoiseSuppressorSwitch
(
    le_audio_Stream_t* streamPtr,   ///< [IN] input audio stream
    le_onoff_t         switchOnOff  ///< [IN] switch ON or OFF
)
{
    if (streamPtr == NULL)
    {
        return LE_BAD_PARAMETER;
    }

    le_audio_If_t interface = streamPtr->audioInterface;

    if (interface != LE_AUDIO_IF_DSP_BACKEND_MODEM_VOICE_TX)
    {
        LE_ERROR("Noise Suppressor is not supported on this interface (%d)", interface);
        return LE_BAD_PARAMETER;
    }

    swi_m2m_audio_set_avns_req_msg_v01 req = {0};
    swi_m2m_audio_set_avns_resp_msg_v01 resp = { {0} };

    req.profile = ActiveQmiProfile;

    req.FNS_valid = 1;
    if (switchOnOff == LE_ON)
    {
        req.NS = 1;
        req.FNS = 1;
    }
    else
    {
        req.NS = 0;
        req.FNS = 0;
    }

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(AudioClient,
                                            QMI_SWI_M2M_AUDIO_SET_AVNS_REQ_V01,
                                            &req, sizeof(req),
                                            &resp, sizeof(resp),
                                            COMM_DEFAULT_PLATFORM_TIMEOUT);

    if(swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_SWI_M2M_AUDIO_SET_AVNS_REQ_V01),
                                                    clientMsgErr,
                                                    resp.resp.result,
                                                    resp.resp.error) != LE_OK )
    {
        return IsAudioAvailable ? LE_FAULT : LE_UNAVAILABLE;
    }
    else
    {
        return LE_OK;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to enable or disable the Echo Canceller.
 *
 * @return LE_BAD_PARAMETER The audio stream or interface is invalid.
 * @return LE_UNAVAILABLE   The audio service initialization failed.
 * @return LE_FAULT         On any other failure.
 * @return LE_OK            Function succeeded.
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_audio_EchoCancellerSwitch
(
    le_audio_Stream_t* streamPtr,   ///< [IN] input audio stream
    le_onoff_t         switchOnOff  ///< [IN] switch ON or OFF
)
{
    if (streamPtr == NULL)
    {
        return LE_BAD_PARAMETER;
    }

    le_audio_If_t interface = streamPtr->audioInterface;

    if (interface != LE_AUDIO_IF_DSP_BACKEND_MODEM_VOICE_TX)
    {
        LE_ERROR("Noise Suppressor is not supported on this interface (%d)", interface);
        return LE_BAD_PARAMETER;
    }

    swi_m2m_audio_set_avec_req_msg_v01 req = {0};
    swi_m2m_audio_set_avec_resp_msg_v01 resp = { {0} };

    req.profile = ActiveQmiProfile;

    if (switchOnOff == LE_ON)
    {
        req.switch_on = 1;
    }
    else
    {
        req.switch_on = 0;
    }

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(AudioClient,
                                            QMI_SWI_M2M_AUDIO_SET_AVEC_REQ_V01,
                                            &req, sizeof(req),
                                            &resp, sizeof(resp),
                                            COMM_DEFAULT_PLATFORM_TIMEOUT);

    if(swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_SWI_M2M_AUDIO_SET_AVEC_REQ_V01),
                                                    clientMsgErr,
                                                    resp.resp.result,
                                                    resp.resp.error) != LE_OK )
    {
        return IsAudioAvailable ? LE_FAULT : LE_UNAVAILABLE;
    }
    else
    {
        return LE_OK;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to enable or disable the FIR (Finite Impulse Response) filter on the
 * downlink or uplink audio path.
 *
 * @return LE_BAD_PARAMETER The audio stream or interface is invalid.
 * @return LE_UNAVAILABLE   The audio service initialization failed.
 * @return LE_FAULT         On any other failure.
 * @return LE_OK            Function succeeded.
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_audio_FirFilterSwitch
(
    le_audio_Stream_t* streamPtr,    ///< [IN] input audio stream
    le_onoff_t         switchOnOff   ///< [IN] switch ON or OFF
)
{
    if (streamPtr == NULL)
    {
        return LE_BAD_PARAMETER;
    }

    le_audio_If_t interface = streamPtr->audioInterface;

    if ((interface != LE_AUDIO_IF_DSP_BACKEND_MODEM_VOICE_RX) &&
        (interface != LE_AUDIO_IF_DSP_BACKEND_MODEM_VOICE_TX))
    {
        LE_ERROR("Noise Suppressor is not supported on this interface (%d)", interface);
        return LE_BAD_PARAMETER;
    }

    swi_m2m_audio_set_avfltren_req_msg_v01 req = {0};
    swi_m2m_audio_set_avfltren_resp_msg_v01 resp = { {0} };

    req.profile = ActiveQmiProfile;
    if (interface == LE_AUDIO_IF_DSP_BACKEND_MODEM_VOICE_TX)
    {
        req.fltr_enum = 0;
    }
    else
    {
        req.fltr_enum = 1;
    }
    if (switchOnOff == LE_ON)
    {
        req.enable = 1;
    }
    else
    {
        req.enable = 0;
    }

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(AudioClient,
                                            QMI_SWI_M2M_AUDIO_SET_AVFLTREN_REQ_V01,
                                            &req, sizeof(req),
                                            &resp, sizeof(resp),
                                            COMM_DEFAULT_PLATFORM_TIMEOUT);

    if(swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_SWI_M2M_AUDIO_SET_AVFLTREN_REQ_V01),
                                                    clientMsgErr,
                                                    resp.resp.result,
                                                    resp.resp.error) != LE_OK )
    {
        return IsAudioAvailable ? LE_FAULT : LE_UNAVAILABLE;
    }
    else
    {
        return LE_OK;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to enable or disable the IIR (Infinite Impulse Response) filter on
 * the downlink or uplink audio path.
 *
 * @return LE_BAD_PARAMETER The audio stream or interface is invalid.
 * @return LE_UNAVAILABLE   The audio service initialization failed.
 * @return LE_FAULT         On any other failure.
 * @return LE_OK            Function succeeded.
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_audio_IirFilterSwitch
(
    le_audio_Stream_t* streamPtr,    ///< [IN] input audio stream
    le_onoff_t         switchOnOff   ///< [IN] switch ON or OFF
)
{
    if (streamPtr == NULL)
    {
        return LE_BAD_PARAMETER;
    }

    le_audio_If_t interface = streamPtr->audioInterface;

    if ((interface != LE_AUDIO_IF_DSP_BACKEND_MODEM_VOICE_RX) &&
        (interface != LE_AUDIO_IF_DSP_BACKEND_MODEM_VOICE_TX))
    {
        LE_ERROR("Noise Suppressor is not supported on this interface (%d)", interface);
        return LE_BAD_PARAMETER;
    }

    swi_m2m_audio_set_avfltren_req_msg_v01 req = {0};
    swi_m2m_audio_set_avfltren_resp_msg_v01 resp = { {0} };

    req.profile = ActiveQmiProfile;
    if (interface == LE_AUDIO_IF_DSP_BACKEND_MODEM_VOICE_TX)
    {
        req.fltr_enum = 2;
    }
    else
    {
        req.fltr_enum = 3;
    }
    if (switchOnOff == LE_ON)
    {
        req.enable = 1;
    }
    else
    {
        req.enable = 0;
    }

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(AudioClient,
                                            QMI_SWI_M2M_AUDIO_SET_AVFLTREN_REQ_V01,
                                            &req, sizeof(req),
                                            &resp, sizeof(resp),
                                            COMM_DEFAULT_PLATFORM_TIMEOUT);

    if(swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_SWI_M2M_AUDIO_SET_AVFLTREN_REQ_V01),
                                                    clientMsgErr,
                                                    resp.resp.result,
                                                    resp.resp.error) != LE_OK )
    {
        return IsAudioAvailable ? LE_FAULT : LE_UNAVAILABLE;
    }
    else
    {
        return LE_OK;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to set the audio profile.
 *
 * @return LE_UNAVAILABLE   On audio service initialization failure.
 * @return LE_FAULT         On any other failure.
 * @return LE_OK            Function succeeded.
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_audio_SetProfile
(
    uint32_t profile   ///< [IN] The audio profile.
)
{
    // Set the profile.
    swi_m2m_audio_set_profile_req_msg_v01 profileSetReq = {0};
    swi_m2m_audio_set_profile_resp_msg_v01 profileSetResp = { {0} };

    profileSetReq.profile = profile;
    profileSetReq.earmute_valid = false;
    profileSetReq.micmute_valid = false;
    profileSetReq.generator_valid = false;
    profileSetReq.volume_valid = false;
    profileSetReq.cwtmute_valid = false;

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(AudioClient,
                                            QMI_SWI_M2M_AUDIO_SET_PROFILE_REQ_V01,
                                            &profileSetReq, sizeof(profileSetReq),
                                            &profileSetResp, sizeof(profileSetResp),
                                            COMM_DEFAULT_PLATFORM_TIMEOUT);

    if ( swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_SWI_M2M_AUDIO_SET_PROFILE_REQ_V01),
                                    clientMsgErr,
                                    profileSetResp.resp.result,
                                    profileSetResp.resp.error) != LE_OK )
    {
        LE_ERROR("Cannot activate the QMI audio profile value to .%d", profile);
        return IsAudioAvailable ? LE_FAULT : LE_UNAVAILABLE;
    }

    ActiveQmiProfile = profile;
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to get the audio profile in use.
 *
 * @return LE_UNAVAILABLE   On audio service initialization failure.
 * @return LE_FAULT         On any other failure.
 * @return LE_OK            Function succeeded.
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_audio_GetProfile
(
    uint32_t* profilePtr  ///< [OUT] The audio profile.
)
{
    // Get active profile
    SWIQMI_DECLARE_MESSAGE(swi_m2m_audio_get_profile_resp_msg_v01, profileGetResp);

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(AudioClient,
                                            QMI_SWI_M2M_AUDIO_GET_PROFILE_REQ_V01,
                                            NULL, 0,
                                            &profileGetResp, sizeof(profileGetResp),
                                            COMM_DEFAULT_PLATFORM_TIMEOUT);

    if ( swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_SWI_M2M_AUDIO_GET_PROFILE_REQ_V01),
                                     clientMsgErr,
                                     profileGetResp.resp.result,
                                     profileGetResp.resp.error) == LE_OK )
    {
        // update global variable
        ActiveQmiProfile = profileGetResp.profile;
        *profilePtr = ActiveQmiProfile;
        return LE_OK;
    }

    return IsAudioAvailable ? LE_FAULT : LE_UNAVAILABLE;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the default PCM time slot used on the current platform.
 *
 * @return the time slot number.
 */
//--------------------------------------------------------------------------------------------------
uint32_t pa_audio_GetDefaultPcmTimeSlot
(
    void
)
{
    return 0;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the default I2S channel mode used on the current platform.
 *
 * @return the I2S channel mode.
 */
//--------------------------------------------------------------------------------------------------
le_audio_I2SChannel_t pa_audio_GetDefaultI2sMode
(
    void
)
{
    return LE_AUDIO_I2S_STEREO;
}

//--------------------------------------------------------------------------------------------------
/**
 * Configure the PCM Sampling Rate.
 *
 * @return LE_FAULT         Function failed.
 * @return LE_OUT_OF_RANGE  Your platform does not support the setting's value.
 * @return LE_BUSY          The PCM interface is already active.
 * @return LE_OK            Function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_audio_SetPcmSamplingRate
(
    uint32_t    rate         ///< [IN] Sampling rate in Hz.
)
{
    if (IfaceInUse == QMI_PIFACE_PCM)
    {
        LE_ERROR("The cannot set PCM settings, PCM interface is already active.");
        return LE_BUSY;
    }
    else if ((rate != 8000) && (rate != 16000))
    {
        LE_ERROR("This PCM rate is not supported on this platform.");
        return LE_OUT_OF_RANGE;
    }
    else
    {
        if (rate == 8000)
        {
            PcmSettings.rate = 0;
        }
        else if (rate == 16000)
        {
            PcmSettings.rate = 1;
        }
        return LE_OK;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Configure the PCM Sampling Resolution.
 *
 * @return LE_FAULT         Function failed.
 * @return LE_OUT_OF_RANGE  Your platform does not support the setting's value.
 * @return LE_BUSY          The PCM interface is already active.
 * @return LE_OK            Function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_audio_SetPcmSamplingResolution
(
    uint32_t  bitsPerSample   ///< [IN] Sampling resolution (bits/sample).
)
{
    if (IfaceInUse == QMI_PIFACE_PCM)
    {
        LE_ERROR("The cannot set PCM settings, PCM interface is already active.");
        return LE_BUSY;
    }
    else if ((bitsPerSample != 8) && (bitsPerSample != 16) && (bitsPerSample != 32) &&
             (bitsPerSample != 64) && (bitsPerSample != 128) && (bitsPerSample != 256))
    {
        LE_ERROR("This PCM sampling resolution is not supported on this platform.");
        return LE_OUT_OF_RANGE;
    }
    else
    {
        if (bitsPerSample == 8)
        {
            PcmSettings.bitsFrame = 0;
        }
        else if (bitsPerSample == 16)
        {
            PcmSettings.bitsFrame = 1;
        }
        else if (bitsPerSample == 32)
        {
            PcmSettings.bitsFrame = 2;
        }
        else if (bitsPerSample == 64)
        {
            PcmSettings.bitsFrame = 3;
        }
        else if (bitsPerSample == 128)
        {
            PcmSettings.bitsFrame = 4;
        }
        else if (bitsPerSample == 256)
        {
            PcmSettings.bitsFrame = 5;
        }
        return LE_OK;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Configure the PCM Companding.
 *
 * @return LE_FAULT         Function failed.
 * @return LE_OUT_OF_RANGE  Your platform does not support the setting's value.
 * @return LE_BUSY          The PCM interface is already active.
 * @return LE_OK            Function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_audio_SetPcmCompanding
(
    le_audio_Companding_t companding   ///< [IN] Companding.
)
{
    if (IfaceInUse == QMI_PIFACE_PCM)
    {
        LE_ERROR("The cannot set PCM settings, PCM interface is already active.");
        return LE_BUSY;
    }
    else if ((companding != LE_AUDIO_COMPANDING_ALAW) &&
             (companding != LE_AUDIO_COMPANDING_ULAW) &&
             (companding != LE_AUDIO_COMPANDING_NONE))
    {
        LE_ERROR("This PCM companding is not supported on this platform.");
        return LE_OUT_OF_RANGE;
    }
    else
    {
        if (companding == LE_AUDIO_COMPANDING_NONE)
        {
            PcmSettings.companding = 0;
        }
        else if (companding == LE_AUDIO_COMPANDING_ULAW)
        {
            PcmSettings.companding = 1;
        }
        else if (companding == LE_AUDIO_COMPANDING_ALAW)
        {
            PcmSettings.companding = 2;
        }
        return LE_OK;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Retrieve the PCM Sampling Rate.
 *
 * @return The sampling rate in Hz.
 */
//--------------------------------------------------------------------------------------------------
uint32_t pa_audio_GetPcmSamplingRate
(
    void
)
{
    if (PcmSettings.rate == 0)
    {
        return 8000;
    }
    else if (PcmSettings.rate == 1)
    {
        return 16000;
    }

    // Will never get here
    return 16000;
}

//--------------------------------------------------------------------------------------------------
/**
 * Retrieve the PCM Sampling Resolution.
 *
 * @return The sampling resolution (bits/sample).
 */
//--------------------------------------------------------------------------------------------------
uint32_t pa_audio_GetPcmSamplingResolution
(
    void
)
{
    if (PcmSettings.bitsFrame == 0)
    {
        return 8;
    }
    else if (PcmSettings.bitsFrame == 1)
    {
        return 16;
    }
    else if (PcmSettings.bitsFrame == 2)
    {
        return 32;
    }
    else if (PcmSettings.bitsFrame == 3)
    {
        return 64;
    }
    else if (PcmSettings.bitsFrame == 4)
    {
        return 128;
    }
    else if (PcmSettings.bitsFrame == 5)
    {
        return 256;
    }

    // Will never get here
    return 1;
}


//--------------------------------------------------------------------------------------------------
/**
 * Retrieve the PCM Companding.
 *
 * @return The PCM companding.
 */
//--------------------------------------------------------------------------------------------------
le_audio_Companding_t pa_audio_GetPcmCompanding
(
    void
)
{
    if (PcmSettings.companding == 0)
    {
        return LE_AUDIO_COMPANDING_NONE;
    }
    else if (PcmSettings.companding == 1)
    {
        return LE_AUDIO_COMPANDING_ULAW;
    }
    else if (PcmSettings.companding == 2)
    {
        return LE_AUDIO_COMPANDING_ALAW;
    }

    // Will never get here
    return LE_AUDIO_COMPANDING_NONE;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to configure an interface as a Master.
 *
 * @return LE_FAULT         The function failed to configure the interface as a Master.
 * @return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_audio_SetMasterMode
(
    le_audio_Stream_t* streamPtr
)
{
    if (streamPtr == NULL)
    {
        return LE_FAULT;
    }

    le_audio_If_t interface = streamPtr->audioInterface;

    LE_DEBUG("Configure interface.%d as a Master", interface);

    switch (interface)
    {
        case LE_AUDIO_IF_DSP_FRONTEND_PCM_RX:
        case LE_AUDIO_IF_DSP_FRONTEND_PCM_TX:
        {
            PcmSettings.mode = 1;
            return LE_OK;
        }
        case LE_AUDIO_IF_CODEC_SPEAKER:
        case LE_AUDIO_IF_CODEC_MIC:
        case LE_AUDIO_IF_DSP_FRONTEND_USB_RX:
        case LE_AUDIO_IF_DSP_FRONTEND_USB_TX:
        case LE_AUDIO_IF_DSP_BACKEND_MODEM_VOICE_RX:
        case LE_AUDIO_IF_DSP_BACKEND_MODEM_VOICE_TX:
        case LE_AUDIO_IF_DSP_FRONTEND_I2S_RX:
        case LE_AUDIO_IF_DSP_FRONTEND_I2S_TX:
        case LE_AUDIO_IF_DSP_FRONTEND_FILE_PLAY:
        case LE_AUDIO_IF_DSP_FRONTEND_FILE_CAPTURE:
        case LE_AUDIO_NUM_INTERFACES:
        {
            break;
        }
    }
    LE_ERROR("This interface (%d) is not supported",interface);
    return LE_FAULT;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to configure an interface as a Slave.
 *
 * @return LE_FAULT         The function failed to configure the interface as a Slave.
 * @return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_audio_SetSlaveMode
(
    le_audio_Stream_t* streamPtr
)
{
    if (streamPtr == NULL)
    {
        return LE_FAULT;
    }

    le_audio_If_t interface = streamPtr->audioInterface;

    LE_DEBUG("Configure interface.%d as a Slave", interface);

    switch (interface)
    {
        case LE_AUDIO_IF_DSP_FRONTEND_PCM_RX:
        case LE_AUDIO_IF_DSP_FRONTEND_PCM_TX:
        {
            PcmSettings.mode = 0;
            return LE_OK;
        }
        case LE_AUDIO_IF_CODEC_SPEAKER:
        case LE_AUDIO_IF_CODEC_MIC:
        case LE_AUDIO_IF_DSP_FRONTEND_USB_RX:
        case LE_AUDIO_IF_DSP_FRONTEND_USB_TX:
        case LE_AUDIO_IF_DSP_BACKEND_MODEM_VOICE_RX:
        case LE_AUDIO_IF_DSP_BACKEND_MODEM_VOICE_TX:
        case LE_AUDIO_IF_DSP_FRONTEND_I2S_RX:
        case LE_AUDIO_IF_DSP_FRONTEND_I2S_TX:
        case LE_AUDIO_IF_DSP_FRONTEND_FILE_PLAY:
        case LE_AUDIO_IF_DSP_FRONTEND_FILE_CAPTURE:
        case LE_AUDIO_NUM_INTERFACES:
        {
            break;
        }
    }
    LE_ERROR("This interface (%d) is not supported",interface);
    return LE_FAULT;
}

/* These compilation flags are necessary until we have a dedicated
 * version for the QMI Audio API the code is relying on. To be fixed
 * when available.
 */
#if defined(SIERRA_MDM9X40) || defined(SIERRA_MDM9X28)
//--------------------------------------------------------------------------------------------------
/**
 * Helper function to check if an audio interface is already in use
 */
//--------------------------------------------------------------------------------------------------
static bool IsInterfaceInUse
(
    void
)
{
    return (255 != IfaceInUse);
}
#endif

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to set the DSP Audio path
 *
 * @return LE_BAD_PARAMETER The input or output audio stream is invalid.
 * @return LE_UNAVAILABLE   The audio service initialization failed.
 * @return LE_FAULT         On any other failure.
 * @return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_audio_SetDspAudioPath
(
    le_audio_Stream_t* inputStreamPtr,   ///< [IN] input audio stream
    le_audio_Stream_t* outputStreamPtr   ///< [IN] output audio stream
)
{
    le_result_t result = LE_OK;

    if (!inputStreamPtr)
    {
        LE_ERROR("Invalid input stream");
        return LE_BAD_PARAMETER;
    }
    if (!outputStreamPtr)
    {
        LE_ERROR("Invalid ouput stream");
        return LE_BAD_PARAMETER;
    }

    le_audio_If_t inputInterface = inputStreamPtr->audioInterface;
    le_audio_If_t outputInterface = outputStreamPtr->audioInterface;

    LE_DEBUG("inputInterface %d outputInterface %d", inputInterface, outputInterface);

    if ((inputInterface == LE_AUDIO_IF_DSP_FRONTEND_FILE_PLAY) ||
        (outputInterface == LE_AUDIO_IF_DSP_FRONTEND_FILE_CAPTURE))
    {
        result = media_routing_SetDspAudioPath(inputStreamPtr, outputStreamPtr);
        if (result != LE_OK)
        {
            return IsAudioAvailable ? result : LE_UNAVAILABLE;
        }
    }

    if ( (ConnectionMatrix[inputInterface][outputInterface] != -1) &&
         (ConnectionMatrix[inputInterface][outputInterface] != IfaceInUse) )
    {
        LE_INFO("Set the following path: %d (in.%d out.%d)",
                ConnectionMatrix[inputInterface][outputInterface],
                inputInterface,
                outputInterface);

        // Set the physical interface
        swi_m2m_audio_set_avcfg_req_msg_v01  avcfgSetReq = {0};
        swi_m2m_audio_set_avcfg_resp_msg_v01 avcfgSetResp = {{0}};

        avcfgSetReq.profile = ActiveQmiProfile;
        avcfgSetReq.device = DefaultAcdbDevice;
        avcfgSetReq.piface_id = ConnectionMatrix[inputInterface][outputInterface];

        if (QMI_PIFACE_PCM == avcfgSetReq.piface_id)
        {
            avcfgSetReq.iface_table_valid = true;
            avcfgSetReq.iface_table_len = 5;
            avcfgSetReq.iface_table[0] = PcmSettings.mode;
            avcfgSetReq.iface_table[1] = PcmSettings.rate;
            avcfgSetReq.iface_table[2] = PcmSettings.companding;
            avcfgSetReq.iface_table[3] = PcmSettings.padding;
            avcfgSetReq.iface_table[4] = PcmSettings.bitsFrame;

            LE_DEBUG("PCM interface set with mode.%d, rate.%d,"
                     " companding.%d, padding.%d, bitsFrame.%d",
                     avcfgSetReq.iface_table[0],
                     avcfgSetReq.iface_table[1],
                     avcfgSetReq.iface_table[2],
                     avcfgSetReq.iface_table[3],
                     avcfgSetReq.iface_table[4]);
        }
        else
        {
            avcfgSetReq.iface_table_valid = false;
        }

/* These compilation flags are necessary until we have a dedicated
 * version for the QMI Audio API the code is relying on. To be fixed
 * when available.
 */
#if defined(SIERRA_MDM9X40) || defined(SIERRA_MDM9X28)
        if (IsInterfaceInUse())
        {
            LE_DEBUG("Audio interface %d currently in use, trying to set interface %d",
                     IfaceInUse, ConnectionMatrix[inputInterface][outputInterface]);

            if (LE_AUDIO_IF_DSP_BACKEND_MODEM_VOICE_RX == inputInterface)
            {
                avcfgSetReq.tx_piface_id = IfaceInUse;
            }
            if (LE_AUDIO_IF_DSP_BACKEND_MODEM_VOICE_TX == outputInterface)
            {
                avcfgSetReq.tx_piface_id = ConnectionMatrix[inputInterface][outputInterface];
                avcfgSetReq.piface_id = IfaceInUse;
            }
            avcfgSetReq.tx_piface_id_valid = 1;

            if (QMI_PIFACE_PCM == avcfgSetReq.tx_piface_id)
            {
                avcfgSetReq.tx_iface_table_valid = true;
                avcfgSetReq.tx_iface_table_len = 5;
                avcfgSetReq.tx_iface_table[0] = PcmSettings.mode;
                avcfgSetReq.tx_iface_table[1] = PcmSettings.rate;
                avcfgSetReq.tx_iface_table[2] = PcmSettings.companding;
                avcfgSetReq.tx_iface_table[3] = PcmSettings.padding;
                avcfgSetReq.tx_iface_table[4] = PcmSettings.bitsFrame;

                LE_DEBUG("PCM interface set with mode.%d, rate.%d,"
                         " companding.%d, padding.%d, bitsFrame.%d",
                         avcfgSetReq.tx_iface_table[0],
                         avcfgSetReq.tx_iface_table[1],
                         avcfgSetReq.tx_iface_table[2],
                         avcfgSetReq.tx_iface_table[3],
                         avcfgSetReq.tx_iface_table[4]);
            }
            else
            {
                avcfgSetReq.tx_iface_table_valid = false;
            }
        }
#endif

        qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(AudioClient,
                                                QMI_SWI_M2M_AUDIO_SET_AVCFG_REQ_V01,
                                                &avcfgSetReq, sizeof(avcfgSetReq),
                                                &avcfgSetResp, sizeof(avcfgSetResp),
                                                COMM_DEFAULT_PLATFORM_TIMEOUT);

        if ( swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_SWI_M2M_AUDIO_SET_AVCFG_REQ_V01),
                                        clientMsgErr,
                                        avcfgSetResp.resp.result,
                                        avcfgSetResp.resp.error) != LE_OK )
        {
            LE_ERROR("Cannot set the audio physical interface to %d",
                     ConnectionMatrix[inputInterface][outputInterface]);
            return IsAudioAvailable ? LE_FAULT : LE_UNAVAILABLE;
        }
        LE_DEBUG("%d audio physical interface successfully set.",
                 ConnectionMatrix[inputInterface][outputInterface]);
        IfaceInUse = ConnectionMatrix[inputInterface][outputInterface];
    }
    else if (ConnectionMatrix[inputInterface][outputInterface] == IfaceInUse)
    {
        LE_DEBUG("DSP audio path %d already set (in.%d out.%d)",
                 ConnectionMatrix[inputInterface][outputInterface],
                 inputInterface,
                 outputInterface);
        return LE_OK;
    }

    LE_INFO("IfaceInUse is now %d", IfaceInUse);
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to reset the DSP Audio path
 *
 * @return LE_BAD_PARAMETER The input or output audio stream is invalid.
 * @return LE_UNAVAILABLE   The audio service initialization failed.
 * @return LE_FAULT         On any other failure.
 * @return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_audio_ResetDspAudioPath
(
    le_audio_Stream_t* inputStreamPtr,   ///< [IN] input audio stream
    le_audio_Stream_t* outputStreamPtr   ///< [IN] output audio stream
)
{
    le_result_t  res = LE_FAULT;

    if (!inputStreamPtr)
    {
        LE_ERROR("Invalid input stream");
        return LE_BAD_PARAMETER;
    }
    if (!outputStreamPtr)
    {
        LE_ERROR("Invalid ouput stream");
        return LE_BAD_PARAMETER;
    }

    if ((inputStreamPtr->audioInterface == LE_AUDIO_IF_DSP_FRONTEND_FILE_PLAY) ||
        (outputStreamPtr->audioInterface == LE_AUDIO_IF_DSP_FRONTEND_FILE_CAPTURE))
    {
        res = media_routing_ResetDspAudioPath(inputStreamPtr, outputStreamPtr);
        if (res != LE_OK)
        {
            return IsAudioAvailable ? res : LE_UNAVAILABLE;
        }
    }

    else if (ConnectionMatrix[inputStreamPtr->audioInterface][outputStreamPtr->audioInterface]
                                                                            == IfaceInUse)
    {
        IfaceInUse = -1;
        res = LE_OK;
    }

    return res;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to mute or unmute the interface
 *
 * @return LE_BAD_PARAMETER The input audio stream is invalid.
 * @return LE_UNAVAILABLE   The audio service initialization failed.
 * @return LE_FAULT         On any other failure.
 * @return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_audio_Mute
(
    le_audio_Stream_t* streamPtr,   ///< [IN] input audio stream
    bool               mute         ///< [IN] true to mute the interface, false to unmute
)
{
    le_result_t           res = LE_FAULT;
    qmi_client_error_type clientMsgErr;
    PaParameters_t*       paParamsPtr = GetPaParams(streamPtr);

    LE_DEBUG("Request to %s audio for interface %d", (mute ? "mute":"unmute"),
                                                     streamPtr->audioInterface);

    if (!paParamsPtr)
    {
        LE_ERROR("Invalid input stream");
        return LE_BAD_PARAMETER;
    }

    // Mute/unmute configuration
    switch (streamPtr->audioInterface)
    {
        case LE_AUDIO_IF_CODEC_MIC:
        case LE_AUDIO_IF_CODEC_SPEAKER:
        case LE_AUDIO_IF_DSP_FRONTEND_USB_RX:
        case LE_AUDIO_IF_DSP_FRONTEND_USB_TX:
        case LE_AUDIO_IF_DSP_BACKEND_MODEM_VOICE_RX:
        case LE_AUDIO_IF_DSP_BACKEND_MODEM_VOICE_TX:
        case LE_AUDIO_IF_DSP_FRONTEND_PCM_RX:
        case LE_AUDIO_IF_DSP_FRONTEND_PCM_TX:
        case LE_AUDIO_IF_DSP_FRONTEND_I2S_RX:
        case LE_AUDIO_IF_DSP_FRONTEND_I2S_TX:
        {
            // Each audio interface is defined by two Legato interfaces: one for Rx and one for Tx.
            // It is therefore necessary to associate both Rx and Tx interfaces to the same audio
            // interface in order to keep a consistent mute state when (un)muting the downlink Rx or
            // the uplink Tx.
            static bool speakerMute[AUDIO_IF_NUM] = {false};
            static bool micMute[AUDIO_IF_NUM] = {false};
            AudioInterface_t audioInterface = ConvertAudioInterface[streamPtr->audioInterface];

            SWIQMI_DECLARE_MESSAGE(swi_m2m_audio_set_avmute_req_msg_v01, muteReq);
            SWIQMI_DECLARE_MESSAGE(swi_m2m_audio_set_avmute_resp_msg_v01, muteResp);

            // Update audio interface mute status
            if (   (LE_AUDIO_IF_CODEC_MIC == streamPtr->audioInterface)
                || (LE_AUDIO_IF_DSP_BACKEND_MODEM_VOICE_TX == streamPtr->audioInterface)
                || (LE_AUDIO_IF_DSP_FRONTEND_USB_TX == streamPtr->audioInterface)
                || (LE_AUDIO_IF_DSP_FRONTEND_PCM_TX == streamPtr->audioInterface)
                || (LE_AUDIO_IF_DSP_FRONTEND_I2S_TX == streamPtr->audioInterface))
            {
                micMute[audioInterface] = mute;
            }
            else if (   (LE_AUDIO_IF_CODEC_SPEAKER == streamPtr->audioInterface)
                     || (LE_AUDIO_IF_DSP_BACKEND_MODEM_VOICE_RX == streamPtr->audioInterface)
                     || (LE_AUDIO_IF_DSP_FRONTEND_USB_RX == streamPtr->audioInterface)
                     || (LE_AUDIO_IF_DSP_FRONTEND_PCM_RX == streamPtr->audioInterface)
                     || (LE_AUDIO_IF_DSP_FRONTEND_I2S_RX == streamPtr->audioInterface))
            {
                speakerMute[audioInterface] = mute;
            }

            // Set audio mute value
            muteReq.profile = ActiveQmiProfile;
            if (speakerMute[audioInterface])
            {
                muteReq.earmute = 1;
            }
            else
            {
                muteReq.earmute = 0;
            }

            if (micMute[audioInterface])
            {
                muteReq.micmute = 1;
            }
            else
            {
                muteReq.micmute = 0;
            }

            LE_DEBUG("QMI profile #%d: earmute %d - micmute %d",
                     muteReq.profile, muteReq.earmute, muteReq.micmute);

            clientMsgErr = qmi_client_send_msg_sync(AudioClient,
                                                    QMI_SWI_M2M_AUDIO_SET_AVMUTE_REQ_V01,
                                                    &muteReq, sizeof(muteReq),
                                                    &muteResp, sizeof(muteResp),
                                                    COMM_DEFAULT_PLATFORM_TIMEOUT);

            if (swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_SWI_M2M_AUDIO_SET_AVMUTE_REQ_V01),
                                     clientMsgErr,
                                     muteResp.resp.result,
                                     muteResp.resp.error) != LE_OK)
            {
                LE_ERROR("Cannot %s audio for interface %d", (mute ? "mute":"unmute"),
                                                             streamPtr->audioInterface);
                res = IsAudioAvailable ? LE_FAULT : LE_UNAVAILABLE;
            }
            else
            {
                LE_DEBUG("Audio %s for interface %d", (mute ? "muted":"unmuted"),
                                                      streamPtr->audioInterface);
                res = LE_OK;
            }
        }
        break;

        case LE_AUDIO_IF_DSP_FRONTEND_FILE_PLAY:
        {
            SWIQMI_DECLARE_MESSAGE(swi_m2m_audio_get_avmmute_resp_msg_v01, getMmuteResp);

            clientMsgErr = qmi_client_send_msg_sync(AudioClient,
                                                    QMI_SWI_M2M_AUDIO_GET_AVMMUTE_REQ_V01,
                                                    NULL, 0,
                                                    &getMmuteResp, sizeof(getMmuteResp),
                                                    COMM_DEFAULT_PLATFORM_TIMEOUT);

            if (swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_SWI_M2M_AUDIO_GET_AVMMUTE_REQ_V01),
                                     clientMsgErr,
                                     getMmuteResp.resp.result,
                                     getMmuteResp.resp.error) != LE_OK)
            {
                LE_WARN("Cannot get Mmute status for interface %d", streamPtr->audioInterface);
            }
            else
            {
                if ((mute && getMmuteResp.mmute) || (!mute && !getMmuteResp.mmute))
                {
                    LE_DEBUG("%s audio for interface %d already done",
                             (mute ? "Mute":"Unmute"),
                             streamPtr->audioInterface);
                    res = LE_OK;
                }
            }

            if (res != LE_OK)
            {
                SWIQMI_DECLARE_MESSAGE(swi_m2m_audio_set_avmmute_req_msg_v01, mmuteReq);
                SWIQMI_DECLARE_MESSAGE(swi_m2m_audio_set_avmmute_resp_msg_v01, mmuteResp);

                // Update audio interface mute status
                if (mute)
                {
                    mmuteReq.mmute = 1;
                }
                else
                {
                    mmuteReq.mmute = 0;
                }

                clientMsgErr = qmi_client_send_msg_sync(AudioClient,
                                                        QMI_SWI_M2M_AUDIO_SET_AVMMUTE_REQ_V01,
                                                        &mmuteReq, sizeof(mmuteReq),
                                                        &mmuteResp, sizeof(mmuteResp),
                                                        COMM_DEFAULT_PLATFORM_TIMEOUT);

                if (swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_SWI_M2M_AUDIO_SET_AVMMUTE_REQ_V01),
                                         clientMsgErr,
                                         mmuteResp.resp.result,
                                         mmuteResp.resp.error) != LE_OK)
                {
                    LE_ERROR("Cannot %s audio for interface %d",
                             (mute ? "mute":"unmute"),
                             streamPtr->audioInterface);
                    res = IsAudioAvailable ? LE_FAULT : LE_UNAVAILABLE;
                }
                else
                {
                    LE_DEBUG("Audio %s for interface %d",
                             (mute ? "muted":"unmuted"),
                             streamPtr->audioInterface);
                    res = LE_OK;
                }
            }
        }
        break;

        case LE_AUDIO_IF_DSP_FRONTEND_FILE_CAPTURE:
        case LE_AUDIO_NUM_INTERFACES:
        default:
        {
            LE_ERROR("Interface %d is not supported", streamPtr->audioInterface);
            res = LE_FAULT;
        }
        break;
    }

    return res;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to set the interface gain
 *
 * @return LE_BAD_PARAMETER The input audio stream is invalid.
 * @return LE_OUT_OF_RANGE  The gain parameter is out of range.
 * @return LE_UNAVAILABLE   The audio service initialization failed.
 * @return LE_FAULT         On any other failure.
 * @return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_audio_SetGain
(
    le_audio_Stream_t* streamPtr,   ///< [IN] input audio stream
    int32_t            gain         ///< [IN] gain value
)
{
    le_result_t             res = LE_FAULT;
    qmi_client_error_type   clientMsgErr;

    if (!streamPtr)
    {
        LE_ERROR("Invalid input stream");
        return LE_BAD_PARAMETER;
    }

    LE_DEBUG("Set gain for [%d] to %d", streamPtr->audioInterface, gain);

    switch (streamPtr->audioInterface)
    {
        case LE_AUDIO_IF_CODEC_MIC:
        {
            swi_m2m_audio_set_avcodecmictxg_req_msg_v01  setMicGainReq = {0};
            swi_m2m_audio_set_avcodecmictxg_resp_msg_v01 setMicGainResp = {{0}};

            setMicGainReq.profile = ActiveQmiProfile;

            if (gain > GAIN_16BIT_MAX_VALUE)
            {
                LE_ERROR("Cannot set the Mic gain for %d interface", streamPtr->audioInterface);
                res = LE_OUT_OF_RANGE;
            }
            else
            {
                setMicGainReq.gain = gain;
                clientMsgErr = qmi_client_send_msg_sync(AudioClient,
                                QMI_SWI_M2M_AUDIO_SET_AVCODECMICTXG_REQ_V01,
                                &setMicGainReq, sizeof(setMicGainReq),
                                &setMicGainResp, sizeof(setMicGainResp),
                                COMM_DEFAULT_PLATFORM_TIMEOUT);

                if ( swiQmi_CheckResponse(
                                STRINGIZE_EXPAND(QMI_SWI_M2M_AUDIO_SET_AVCODECMICTXG_REQ_V01),
                                clientMsgErr,
                                setMicGainResp.resp.result,
                                setMicGainResp.resp.error) != LE_OK )
                {
                    LE_ERROR("Cannot set the Mic gain for %d interface",
                                streamPtr->audioInterface);
                    res = IsAudioAvailable ? LE_FAULT : LE_UNAVAILABLE;
                }
                else
                {
                    res = LE_OK;
                }
            }
            break;
        }

        case LE_AUDIO_IF_DSP_BACKEND_MODEM_VOICE_RX:
        {
            swi_m2m_audio_set_volume_req_msg_v01  setReq = {0};
            swi_m2m_audio_set_volume_resp_msg_v01 setResp = {{0}};

            if (gain > 8)
            {
                LE_ERROR("Cannot set the Mic gain for %d interface", streamPtr->audioInterface);
                res = LE_OUT_OF_RANGE;
            }
            else
            {
                setReq.profile = ActiveQmiProfile;
                setReq.generator = 0; // 0 – voice
                setReq.level = gain;

                clientMsgErr = qmi_client_send_msg_sync(AudioClient,
                                                        QMI_SWI_M2M_AUDIO_SET_VOLUME_REQ_V01,
                                                        &setReq, sizeof(setReq),
                                                        &setResp, sizeof(setResp),
                                                        COMM_DEFAULT_PLATFORM_TIMEOUT);

                if ( swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_SWI_M2M_AUDIO_SET_VOLUME_REQ_V01),
                                                clientMsgErr,
                                                setResp.resp.result,
                                                setResp.resp.error) != LE_OK )
                {
                    LE_ERROR("Cannot set the gain for %d interface", streamPtr->audioInterface);
                    res = IsAudioAvailable ? LE_FAULT : LE_UNAVAILABLE;
                }
                else
                {
                    res = LE_OK;
                }
            }
            break;
        }

        case LE_AUDIO_IF_DSP_BACKEND_MODEM_VOICE_TX:
        {
            swi_m2m_audio_set_txvol_req_msg_v01  setReq = {0};
            swi_m2m_audio_set_txvol_resp_msg_v01 setResp = {{0}};

            setReq.profile = ActiveQmiProfile;

            if (gain > GAIN_16BIT_MAX_VALUE)
            {
                LE_ERROR("Cannot set the Mic gain for %d interface", streamPtr->audioInterface);
                res = LE_OUT_OF_RANGE;
            }
            else
            {
                setReq.value = gain;
                clientMsgErr = qmi_client_send_msg_sync(AudioClient,
                                                        QMI_SWI_M2M_AUDIO_SET_TXVOL_REQ_V01,
                                                        &setReq, sizeof(setReq),
                                                        &setResp, sizeof(setResp),
                                                        COMM_DEFAULT_PLATFORM_TIMEOUT);

                if ( swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_SWI_M2M_AUDIO_SET_TXVOL_REQ_V01),
                                                clientMsgErr,
                                                setResp.resp.result,
                                                setResp.resp.error) != LE_OK )
                {
                    LE_ERROR("Cannot set the gain for %d interface", streamPtr->audioInterface);
                    res = IsAudioAvailable ? LE_FAULT : LE_UNAVAILABLE;
                }
                else
                {
                    if (setReq.value)
                    {
                        PaParameters_t* paParamsPtr = GetPaParams(streamPtr);

                        if (!paParamsPtr)
                        {
                            LE_ERROR("Bad parameter");
                            return LE_FAULT;
                        }
                    }
                    res = LE_OK;
                }
            }
            break;
        }

        case LE_AUDIO_IF_DSP_FRONTEND_FILE_PLAY:
        {
            swi_m2m_audio_set_avaudvol_req_msg_v01  setReq = {0};
            swi_m2m_audio_set_avaudvol_resp_msg_v01 setResp = {{0}};

            if (gain > GAIN_16BIT_MAX_VALUE)
            {
                LE_ERROR("Cannot set the Mic gain for %d interface", streamPtr->audioInterface);
                res = LE_OUT_OF_RANGE;
            }
            else
            {
                setReq.volume = gain;

                clientMsgErr = qmi_client_send_msg_sync(AudioClient,
                                                        QMI_SWI_M2M_AUDIO_SET_AVAUDVOL_REQ_V01,
                                                        &setReq, sizeof(setReq),
                                                        &setResp, sizeof(setResp),
                                                        COMM_DEFAULT_PLATFORM_TIMEOUT);

                if ( swiQmi_CheckResponse(
                                STRINGIZE_EXPAND(QMI_SWI_M2M_AUDIO_SET_AVAUDVOL_REQ_V01),
                                                clientMsgErr,
                                                setResp.resp.result,
                                                setResp.resp.error) != LE_OK )
                {
                    LE_ERROR("Cannot set the gain for %d interface", streamPtr->audioInterface);
                    res = IsAudioAvailable ? LE_FAULT : LE_UNAVAILABLE;
                }
                else
                {
                    if (setReq.volume)
                    {
                        PaParameters_t* paParamsPtr = GetPaParams(streamPtr);

                        if (!paParamsPtr)
                        {
                            LE_ERROR("Bad parameter");
                            return LE_FAULT;
                        }
                    }
                    res = LE_OK;
                }
            }

            break;
        }

        case LE_AUDIO_IF_CODEC_SPEAKER:
        case LE_AUDIO_IF_DSP_FRONTEND_USB_TX:
        case LE_AUDIO_IF_DSP_FRONTEND_PCM_TX:
        case LE_AUDIO_IF_DSP_FRONTEND_I2S_TX:
        case LE_AUDIO_IF_DSP_FRONTEND_FILE_CAPTURE:
        case LE_AUDIO_IF_DSP_FRONTEND_USB_RX:
        case LE_AUDIO_IF_DSP_FRONTEND_PCM_RX:
        case LE_AUDIO_IF_DSP_FRONTEND_I2S_RX:
        case LE_AUDIO_NUM_INTERFACES:
        {
            LE_ERROR("This interface (%d) is not supported", streamPtr->audioInterface);
            res = LE_FAULT;
            break;
        }
    }

    return res;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to get the interface gain
 *
 * @return LE_BAD_PARAMETER The input audio stream or gain value is invalid.
 * @return LE_UNAVAILABLE   The audio service initialization failed.
 * @return LE_FAULT         On any other failure.
 * @return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_audio_GetGain
(
    le_audio_Stream_t* streamPtr,   ///< [IN] input audio stream
    int32_t*           gainPtr      ///< [OUT] gain value
)
{
    le_result_t             res = LE_FAULT;
    qmi_client_error_type   clientMsgErr;

    if (!streamPtr)
    {
        LE_ERROR("Invalid input stream");
        return LE_BAD_PARAMETER;
    }
    if (!gainPtr)
    {
        LE_ERROR("Invalid gain");
        return LE_BAD_PARAMETER;
    }

    LE_DEBUG("Get gain for [%d]",streamPtr->audioInterface);

    switch (streamPtr->audioInterface)
    {
        case LE_AUDIO_IF_CODEC_MIC:
        {
            SWIQMI_DECLARE_MESSAGE(swi_m2m_audio_get_avcodecmictxg_req_msg_v01, getMicGainReq);
            SWIQMI_DECLARE_MESSAGE(swi_m2m_audio_get_avcodecmictxg_resp_msg_v01, getMicGainResp);

            getMicGainReq.profile = ActiveQmiProfile;

            clientMsgErr = qmi_client_send_msg_sync(AudioClient,
                            QMI_SWI_M2M_AUDIO_GET_AVCODECMICTXG_REQ_V01,
                            &getMicGainReq, sizeof(getMicGainReq),
                            &getMicGainResp, sizeof(getMicGainResp),
                            COMM_DEFAULT_PLATFORM_TIMEOUT);

            if ( swiQmi_CheckResponse(
                            STRINGIZE_EXPAND(QMI_SWI_M2M_AUDIO_GET_AVCODECMICTXG_REQ_V01),
                            clientMsgErr,
                            getMicGainResp.resp.result,
                            getMicGainResp.resp.error) != LE_OK )
            {
                LE_ERROR("Cannot get the Mic gain for %d interface", streamPtr->audioInterface);
                res = IsAudioAvailable ? LE_FAULT : LE_UNAVAILABLE;
            }
            else
            {
                *gainPtr = getMicGainResp.gain;
                res = LE_OK;
            }
            break;
        }

        case LE_AUDIO_IF_DSP_BACKEND_MODEM_VOICE_RX:
        {
            SWIQMI_DECLARE_MESSAGE(swi_m2m_audio_get_volume_req_msg_v01, getReq);
            SWIQMI_DECLARE_MESSAGE(swi_m2m_audio_get_volume_resp_msg_v01, getResp);

            getReq.profile = ActiveQmiProfile;
            getReq.generator = 0; // 0 – voice

            clientMsgErr = qmi_client_send_msg_sync(AudioClient,
                                                    QMI_SWI_M2M_AUDIO_GET_VOLUME_REQ_V01,
                                                    &getReq, sizeof(getReq),
                                                    &getResp, sizeof(getResp),
                                                    COMM_DEFAULT_PLATFORM_TIMEOUT);

            if ( swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_SWI_M2M_AUDIO_GET_VOLUME_REQ_V01),
                                            clientMsgErr,
                                            getResp.resp.result,
                                            getResp.resp.error) != LE_OK )
            {
                LE_ERROR("Cannot set the gain for %d interface", streamPtr->audioInterface);
                res = IsAudioAvailable ? LE_FAULT : LE_UNAVAILABLE;
            }
            else
            {
                *gainPtr = getResp.level;
                res = LE_OK;
            }
            break;
        }

        case LE_AUDIO_IF_DSP_BACKEND_MODEM_VOICE_TX:
        {
            SWIQMI_DECLARE_MESSAGE(swi_m2m_audio_get_txvol_req_msg_v01, getReq);
            SWIQMI_DECLARE_MESSAGE(swi_m2m_audio_get_txvol_resp_msg_v01, getResp);

            getReq.profile = ActiveQmiProfile;

            clientMsgErr = qmi_client_send_msg_sync(AudioClient,
                                                    QMI_SWI_M2M_AUDIO_GET_TXVOL_REQ_V01,
                                                    &getReq, sizeof(getReq),
                                                    &getResp, sizeof(getResp),
                                                    COMM_DEFAULT_PLATFORM_TIMEOUT);

            if ( swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_SWI_M2M_AUDIO_GET_TXVOL_REQ_V01),
                                            clientMsgErr,
                                            getResp.resp.result,
                                            getResp.resp.error) != LE_OK )
            {
                LE_ERROR("Cannot get the gain for %d interface", streamPtr->audioInterface);
                res = IsAudioAvailable ? LE_FAULT : LE_UNAVAILABLE;
            }
            else
            {
                *gainPtr = getResp.value;
                res = LE_OK;
            }
            break;
        }

        case LE_AUDIO_IF_DSP_FRONTEND_FILE_PLAY:
        {
            SWIQMI_DECLARE_MESSAGE(swi_m2m_audio_get_avaudvol_resp_msg_v01, getResp);

            clientMsgErr = qmi_client_send_msg_sync(AudioClient,
                                                    QMI_SWI_M2M_AUDIO_GET_AVAUDVOL_REQ_V01,
                                                    NULL, 0,
                                                    &getResp, sizeof(getResp),
                                                    COMM_DEFAULT_PLATFORM_TIMEOUT);

            if ( swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_SWI_M2M_AUDIO_GET_AVAUDVOL_REQ_V01),
                                            clientMsgErr,
                                            getResp.resp.result,
                                            getResp.resp.error) != LE_OK )
            {
                LE_ERROR("Cannot get the gain for %d interface", streamPtr->audioInterface);
                res = IsAudioAvailable ? LE_FAULT : LE_UNAVAILABLE;
            }
            else
            {
                *gainPtr = getResp.volume;
                res = LE_OK;
            }
            break;
        }

        case LE_AUDIO_IF_CODEC_SPEAKER:
        case LE_AUDIO_IF_DSP_FRONTEND_USB_TX:
        case LE_AUDIO_IF_DSP_FRONTEND_PCM_TX:
        case LE_AUDIO_IF_DSP_FRONTEND_I2S_TX:
        case LE_AUDIO_IF_DSP_FRONTEND_FILE_CAPTURE:
        case LE_AUDIO_IF_DSP_FRONTEND_USB_RX:
        case LE_AUDIO_IF_DSP_FRONTEND_PCM_RX:
        case LE_AUDIO_IF_DSP_FRONTEND_I2S_RX:
        case LE_AUDIO_NUM_INTERFACES:
        {
            LE_ERROR("This interface (%d) is not supported", streamPtr->audioInterface);
            res = LE_FAULT;
            break;
        }
    }

    return res;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to get the Noise Suppressor status
 *
 * @return LE_BAD_PARAMETER The input audio stream or noise suppressor status is invalid.
 * @return LE_UNAVAILABLE   The audio service initialization failed.
 * @return LE_FAULT         On any other failure.
 * @return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_audio_GetNoiseSuppressorStatus
(
    le_audio_Stream_t* streamPtr,                   ///< [IN] input audio stream
    bool*              noiseSuppressorStatusPtr     ///< [OUT] Noise Suppressor status
)
{
    le_result_t             res;
    qmi_client_error_type   clientMsgErr;

    if (!streamPtr)
    {
        LE_ERROR("Invalid input stream");
        return LE_BAD_PARAMETER;
    }
    if (!noiseSuppressorStatusPtr)
    {
        LE_ERROR("Invalid noise suppressor");
        return LE_BAD_PARAMETER;
    }

    LE_DEBUG("Get status of Noise Suppressor for %d",streamPtr->audioInterface);

    switch (streamPtr->audioInterface)
    {
        case LE_AUDIO_IF_DSP_BACKEND_MODEM_VOICE_TX:
        {
            SWIQMI_DECLARE_MESSAGE(swi_m2m_audio_get_avns_req_msg_v01, getReq);
            SWIQMI_DECLARE_MESSAGE(swi_m2m_audio_get_avns_resp_msg_v01, getResp);

            getReq.profile = ActiveQmiProfile;

            clientMsgErr = qmi_client_send_msg_sync(AudioClient,
                                                    QMI_SWI_M2M_AUDIO_GET_AVNS_REQ_V01,
                                                    &getReq, sizeof(getReq),
                                                    &getResp, sizeof(getResp),
                                                    COMM_DEFAULT_PLATFORM_TIMEOUT);

            if( swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_SWI_M2M_AUDIO_GET_AVNS_REQ_V01),
                                     clientMsgErr,
                                     getResp.resp.result,
                                     getResp.resp.error) != LE_OK )
            {
                LE_ERROR("Cannot get the status of Noise Suppressor for %d interface",
                        streamPtr->audioInterface);
                res = IsAudioAvailable ? LE_FAULT : LE_UNAVAILABLE;
            }
            else
            {
                *noiseSuppressorStatusPtr = getResp.NS;
                res = LE_OK;
            }
            break;
        }
        default:
        {
            LE_ERROR("This interface (%d) is not supported", streamPtr->audioInterface);
            res = LE_FAULT;
            break;
        }
    }
    return res;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to get the Echo Canceller status
 *
 * @return LE_BAD_PARAMETER The input audio stream or echo canceller status is invalid.
 * @return LE_UNAVAILABLE   The audio service initialization failed.
 * @return LE_FAULT         On any other failure.
 * @return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_audio_GetEchoCancellerStatus
(
    le_audio_Stream_t* streamPtr,                   ///< [IN] input audio stream
    bool*              echoCancellerStatusPtr       ///< [OUT] Echo Canceller status
)
{
    le_result_t             res;
    qmi_client_error_type   clientMsgErr;

    if (!streamPtr)
    {
        LE_ERROR("Invalid input stream");
        return LE_BAD_PARAMETER;
    }
    if (!echoCancellerStatusPtr)
    {
        LE_ERROR("Invalid echo canceller status");
        return LE_BAD_PARAMETER;
    }

    LE_DEBUG("Get status of Echo Canceller for %d",streamPtr->audioInterface);

    switch (streamPtr->audioInterface)
    {
        case LE_AUDIO_IF_DSP_BACKEND_MODEM_VOICE_TX:
        {
            SWIQMI_DECLARE_MESSAGE(swi_m2m_audio_get_avec_req_msg_v01, getReq);
            SWIQMI_DECLARE_MESSAGE(swi_m2m_audio_get_avec_resp_msg_v01, getResp);

            getReq.profile = ActiveQmiProfile;

            clientMsgErr = qmi_client_send_msg_sync(AudioClient,
                                                    QMI_SWI_M2M_AUDIO_GET_AVEC_REQ_V01,
                                                    &getReq, sizeof(getReq),
                                                    &getResp, sizeof(getResp),
                                                    COMM_DEFAULT_PLATFORM_TIMEOUT);

            if( swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_SWI_M2M_AUDIO_GET_AVEC_REQ_V01),
                                     clientMsgErr,
                                     getResp.resp.result,
                                     getResp.resp.error) != LE_OK )
            {
                LE_ERROR("Cannot get the status of Echo Canceller for %d interface",
                        streamPtr->audioInterface);
                res = IsAudioAvailable ? LE_FAULT : LE_UNAVAILABLE;
            }
            else
            {
                *echoCancellerStatusPtr = getResp.switch_on;
                res = LE_OK;
            }
            break;
        }
        default:
        {
            LE_ERROR("This interface (%d) is not supported", streamPtr->audioInterface);
            res = LE_FAULT;
            break;
        }
    }
    return res;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the value of a platform specific gain in the audio subsystem.
 *
 * @return LE_BAD_PARAMETER The pointer to name of the platform specific gain is invalid.
 * @return LE_NOT_FOUND     The specified gain's name is not recognized in your audio subsystem.
 * @return LE_OUT_OF_RANGE  The gain parameter is out of range.
 * @return LE_UNAVAILABLE   The audio service initialization failed.
 * @return LE_FAULT         On any other failure.
 * @return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_audio_SetPlatformSpecificGain
(
    const char*    gainNamePtr, ///< [IN] Name of the platform specific gain.
    int32_t        gain         ///< [IN] The gain value (specific to the platform)
)
{
    le_result_t             res;
    qmi_client_error_type   clientMsgErr;

    if (!gainNamePtr)
    {
        LE_ERROR("Invalid gain name");
        return LE_BAD_PARAMETER;
    }
    if (gain > GAIN_16BIT_MAX_VALUE)
    {
        return LE_OUT_OF_RANGE;
    }

    LE_DEBUG("Set the %s gain to %d", gainNamePtr, gain);

    if (strncmp(gainNamePtr, "D_AFE_GAIN_RX", strlen("D_AFE_GAIN_RX")) == 0)
    {
        // Handle Digital Audio Front End Rx gain
        swi_m2m_audio_set_avtxg_req_msg_v01  setReq = {0};
        swi_m2m_audio_set_avtxg_resp_msg_v01 setResp = {{0}};

        setReq.profile = ActiveQmiProfile;

        setReq.gain = gain;

        clientMsgErr = qmi_client_send_msg_sync(AudioClient,
                                                QMI_SWI_M2M_AUDIO_SET_AVTXG_REQ_V01,
                                                &setReq, sizeof(setReq),
                                                &setResp, sizeof(setResp),
                                                COMM_DEFAULT_PLATFORM_TIMEOUT);

        if ( swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_SWI_M2M_AUDIO_SET_AVTXG_REQ_V01),
                                        clientMsgErr,
                                        setResp.resp.result,
                                        setResp.resp.error) != LE_OK )
        {
            LE_ERROR("Cannot set the %s gain", gainNamePtr);
            res = IsAudioAvailable ? LE_FAULT : LE_UNAVAILABLE;
        }
        else
        {
            res = LE_OK;
        }
    }
    else if (strncmp(gainNamePtr, "D_AFE_GAIN_TX", strlen("D_AFE_GAIN_TX")) == 0)
    {
        // Handle Digital Audio Front End Tx gain
        swi_m2m_audio_set_avrxg_req_msg_v01  setReq = {0};
        swi_m2m_audio_set_avrxg_resp_msg_v01 setResp = {{0}};

        setReq.profile = ActiveQmiProfile;

        setReq.gain = gain;

        clientMsgErr = qmi_client_send_msg_sync(AudioClient,
                                                QMI_SWI_M2M_AUDIO_SET_AVRXG_REQ_V01,
                                                &setReq, sizeof(setReq),
                                                &setResp, sizeof(setResp),
                                                COMM_DEFAULT_PLATFORM_TIMEOUT);

        if ( swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_SWI_M2M_AUDIO_SET_AVRXG_REQ_V01),
                                        clientMsgErr,
                                        setResp.resp.result,
                                        setResp.resp.error) != LE_OK )
        {
            LE_ERROR("Cannot set the %s gain", gainNamePtr);
            res = IsAudioAvailable ? LE_FAULT : LE_UNAVAILABLE;
        }
        else
        {
            res = LE_OK;
        }
    }
    else
    {
        LE_ERROR("%s gain is not supported", gainNamePtr);
        res = LE_NOT_FOUND;
    }

    return res;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the value of a platform specific gain in the audio subsystem.
 *
 * @return LE_BAD_PARAMETER The pointer to name of the platform specific gain is invalid.
 * @return LE_NOT_FOUND     The specified gain's name is not recognized in your audio subsystem.
 * @return LE_UNAVAILABLE   The audio service initialization failed.
 * @return LE_FAULT         On any other failure.
 * @return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_audio_GetPlatformSpecificGain
(
    const char* gainNamePtr, ///< [IN] Name of the platform specific gain.
    int32_t*    gainPtr      ///< [OUT] gain value (specific to the platform)
)
{
    le_result_t             res;
    qmi_client_error_type   clientMsgErr;

    if (!gainNamePtr)
    {
        LE_ERROR("Invalid gain name");
        return LE_BAD_PARAMETER;
    }

    LE_DEBUG("Get the %s gain", gainNamePtr);

    if (strncmp(gainNamePtr, "D_AFE_GAIN_RX", strlen("D_AFE_GAIN_RX")) == 0)
    {
        // Handle Digital Audio Front End Rx gain
        SWIQMI_DECLARE_MESSAGE(swi_m2m_audio_get_avtxg_req_msg_v01, getReq);
        SWIQMI_DECLARE_MESSAGE(swi_m2m_audio_get_avtxg_resp_msg_v01, getResp);

        getReq.profile = ActiveQmiProfile;

        clientMsgErr = qmi_client_send_msg_sync(AudioClient,
                                                QMI_SWI_M2M_AUDIO_GET_AVTXG_REQ_V01,
                                                &getReq, sizeof(getReq),
                                                &getResp, sizeof(getResp),
                                                COMM_DEFAULT_PLATFORM_TIMEOUT);

        if ( swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_SWI_M2M_AUDIO_GET_AVTXG_REQ_V01),
                                        clientMsgErr,
                                        getResp.resp.result,
                                        getResp.resp.error) != LE_OK )
        {
            LE_ERROR("Cannot get the %s gain", gainNamePtr);
            res = IsAudioAvailable ? LE_FAULT : LE_UNAVAILABLE;
        }
        else
        {
            *gainPtr = getResp.gain;
            res = LE_OK;
        }
    }
    else if (strncmp(gainNamePtr, "D_AFE_GAIN_TX", strlen("D_AFE_GAIN_TX")) == 0)
    {
        // Handle Digital Audio Front End Tx gain
        SWIQMI_DECLARE_MESSAGE(swi_m2m_audio_get_avrxg_req_msg_v01, getReq);
        SWIQMI_DECLARE_MESSAGE(swi_m2m_audio_get_avrxg_resp_msg_v01, getResp);

        getReq.profile = ActiveQmiProfile;

        clientMsgErr = qmi_client_send_msg_sync(AudioClient,
                                                QMI_SWI_M2M_AUDIO_GET_AVRXG_REQ_V01,
                                                &getReq, sizeof(getReq),
                                                &getResp, sizeof(getResp),
                                                COMM_DEFAULT_PLATFORM_TIMEOUT);

        if ( swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_SWI_M2M_AUDIO_GET_AVRXG_REQ_V01),
                                        clientMsgErr,
                                        getResp.resp.result,
                                        getResp.resp.error) != LE_OK )
        {
            LE_ERROR("Cannot get the %s gain", gainNamePtr);
            res = IsAudioAvailable ? LE_FAULT : LE_UNAVAILABLE;
        }
        else
        {
            *gainPtr = getResp.gain;
            res = LE_OK;
        }
    }
    else
    {
        LE_ERROR("%s gain is not supported", gainNamePtr);
        res = LE_NOT_FOUND;
    }

    return res;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to enable or disable the automatic gain control on the selected
 * audio stream.
 *
 * @return LE_BAD_PARAMETER The audio stream is invalid.
 * @return LE_UNAVAILABLE   The audio service initialization failed.
 * @return LE_FAULT         On any other failure.
 * @return LE_OK            Function succeeded.
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_audio_AutomaticGainControlSwitch
(
    le_audio_Stream_t* streamPtr,   ///< [IN] input audio stream
    le_onoff_t         switchOnOff  ///< [IN] switch ON or OFF
)
{
    if (streamPtr == NULL)
    {
        return LE_BAD_PARAMETER;
    }

    le_audio_If_t interface = streamPtr->audioInterface;

    switch (interface)
    {
        case LE_AUDIO_IF_DSP_BACKEND_MODEM_VOICE_TX:
        {
            swi_m2m_audio_set_avtxagc_req_msg_v01 req = {0};
            swi_m2m_audio_set_avtxagc_resp_msg_v01 resp = { {0} };

            req.profile = ActiveQmiProfile;
            if (switchOnOff == LE_ON)
            {
                req.switch_on = 1;
            }
            else
            {
                req.switch_on = 0;
            }

            qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(AudioClient,
                                                    QMI_SWI_M2M_AUDIO_SET_AVTXAGC_REQ_V01,
                                                    &req, sizeof(req),
                                                    &resp, sizeof(resp),
                                                    COMM_DEFAULT_PLATFORM_TIMEOUT);

            if(swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_SWI_M2M_AUDIO_SET_AVTXAGC_REQ_V01),
                                                            clientMsgErr,
                                                            resp.resp.result,
                                                            resp.resp.error) != LE_OK )
            {
                return IsAudioAvailable ? LE_FAULT : LE_UNAVAILABLE;
            }
            else
            {
                return LE_OK;
            }
        }
        case LE_AUDIO_IF_DSP_BACKEND_MODEM_VOICE_RX:
        {
            swi_m2m_audio_set_avrxagc_req_msg_v01 req = {0};
            swi_m2m_audio_set_avrxagc_resp_msg_v01 resp = { {0} };

            req.profile = ActiveQmiProfile;
            if (switchOnOff == LE_ON)
            {
                req.switch_on = 1;
            }
            else
            {
                req.switch_on = 0;
            }

            qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(AudioClient,
                                                    QMI_SWI_M2M_AUDIO_SET_AVRXAGC_REQ_V01,
                                                    &req, sizeof(req),
                                                    &resp, sizeof(resp),
                                                    COMM_DEFAULT_PLATFORM_TIMEOUT);

            if(swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_SWI_M2M_AUDIO_SET_AVRXAGC_REQ_V01),
                                                            clientMsgErr,
                                                            resp.resp.result,
                                                            resp.resp.error) != LE_OK )
            {
                return IsAudioAvailable ? LE_FAULT : LE_UNAVAILABLE;
            }
            else
            {
                return LE_OK;
            }
        }
        case LE_AUDIO_IF_CODEC_SPEAKER:
        case LE_AUDIO_IF_DSP_FRONTEND_USB_TX:
        case LE_AUDIO_IF_DSP_FRONTEND_PCM_TX:
        case LE_AUDIO_IF_DSP_FRONTEND_I2S_TX:
        {
            swi_m2m_audio_set_avrxavc_req_msg_v01 req = {0};
            swi_m2m_audio_set_avrxavc_resp_msg_v01 resp = { {0} };

            req.profile = ActiveQmiProfile;
            if (switchOnOff == LE_ON)
            {
                req.switch_on = 1;
            }
            else
            {
                req.switch_on = 0;
            }

            qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(AudioClient,
                                                    QMI_SWI_M2M_AUDIO_SET_AVRXAVC_REQ_V01,
                                                    &req, sizeof(req),
                                                    &resp, sizeof(resp),
                                                    COMM_DEFAULT_PLATFORM_TIMEOUT);

            if(swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_SWI_M2M_AUDIO_SET_AVRXAVC_REQ_V01),
                                                            clientMsgErr,
                                                            resp.resp.result,
                                                            resp.resp.error) != LE_OK )
            {
                return IsAudioAvailable ? LE_FAULT : LE_UNAVAILABLE;
            }
            else
            {
                return LE_OK;
            }
        }
        case LE_AUDIO_IF_CODEC_MIC:
        case LE_AUDIO_IF_DSP_FRONTEND_USB_RX:
        case LE_AUDIO_IF_DSP_FRONTEND_PCM_RX:
        case LE_AUDIO_IF_DSP_FRONTEND_FILE_PLAY:
        case LE_AUDIO_IF_DSP_FRONTEND_FILE_CAPTURE:
        case LE_AUDIO_IF_DSP_FRONTEND_I2S_RX:
        case LE_AUDIO_NUM_INTERFACES:
        {
            break;
        }
    }
    LE_ERROR("This interface (%d) is not supported",interface);
    return LE_BAD_PARAMETER;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to register a handler for stream events notifications.
 *
 * @return an handler reference.
 */
//--------------------------------------------------------------------------------------------------
le_audio_DtmfStreamEventHandlerRef_t pa_audio_AddDtmfStreamEventHandler

(
    le_audio_DtmfStreamEventHandlerFunc_t handlerFuncPtr, ///< [IN] The event handler function.
    void*                             contextPtr          ///< [IN] The handler's context.
)
{
    le_event_HandlerRef_t  handlerRef;


    LE_FATAL_IF(handlerFuncPtr == NULL, "The audio file Event handler is NULL.");

    handlerRef = le_event_AddLayeredHandler("PAAudioStreamEventHandler",
                                            StreamEventId,
                                            FirstLayerFileEventHandler,
                                            (le_event_HandlerFunc_t)handlerFuncPtr);

    le_event_SetContextPtr(handlerRef, contextPtr);

    return (le_audio_DtmfStreamEventHandlerRef_t) handlerRef;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to unregister the handler for audio stream events.
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_audio_RemoveDtmfStreamEventHandler
(
    le_audio_DtmfStreamEventHandlerRef_t addHandlerRef ///< [IN]
)
{
    le_event_RemoveHandler((le_event_HandlerRef_t)addHandlerRef);
}

//--------------------------------------------------------------------------------------------------
/**
 * Play signalling DTMFs
 *
 * @return LE_OK            on success
 * @return LE_DUPLICATE     the thread is already started
 * @return LE_FAULT         on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_audio_PlaySignallingDtmf
(
    const char*          dtmfPtr,   ///< [IN] The DTMFs to play.
    uint32_t             duration,  ///< [IN] The DTMF duration in milliseconds.
    uint32_t             pause      ///< [IN] The pause duration between tones in milliseconds.
)
{
    if (!CurrentCallID)
    {
        static le_clk_Time_t timeToWait = { 1, 0 };

        SignallingDtmfWaiting = true;

        LE_DEBUG("Wait for call");

        if ((le_sem_WaitWithTimeOut(SignallingDtmfSemaphore, timeToWait) == LE_TIMEOUT)
            && !CurrentCallID)
        {
            LE_ERROR("There is no call in progress");
            SignallingDtmfWaiting = false;
            return LE_FAULT;
        }
        SignallingDtmfWaiting = false;
    }
    if (PlayDtmfThreadRef)
    {
        LE_ERROR("PlayDtmf thread is already started");
        return LE_DUPLICATE;
    }
    else
    {
        PlayDtmfThreadResource_t*  dtmfParamPtr = le_mem_ForceAlloc(DtmfThreadParamsPool);

        strncpy(dtmfParamPtr->dtmf, dtmfPtr, LE_AUDIO_DTMF_MAX_BYTES);
        dtmfParamPtr->duration = duration;
        dtmfParamPtr->pause = pause;

        PlayDtmfThreadRef = le_thread_Create("PlayDtmfThread", PlayDtmfThread, dtmfParamPtr);

        le_thread_AddChildDestructor(PlayDtmfThreadRef,
                                     DestroyPlayDtmfThread,
                                     NULL);

        LE_INFO("Spawn PlayDtmfThread");
        le_thread_Start(PlayDtmfThreadRef);

        return LE_OK;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Release pa internal parameters.
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_audio_ReleasePaParameters
(
    le_audio_Stream_t* streamPtr
)
{
    if (streamPtr->PaParams)
    {
        le_mem_Release(streamPtr->PaParams);
        streamPtr->PaParams = NULL;
    }

}

//--------------------------------------------------------------------------------------------------
/**
 * Mute/Unmute the Call Waiting Tone.
 *
 * @return LE_UNAVAILABLE   On audio service initialization failure.
 * @return LE_FAULT         On any other failure.
 * @return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_audio_MuteCallWaitingTone
(
    bool    mute    ///< [IN] true to mute the Call Waiting tone, false otherwise.
)
{
    swi_m2m_audio_set_avmute_req_msg_v01  muteReq = {0};
    swi_m2m_audio_set_avmute_resp_msg_v01 muteResp = {{0}};

    // Set audio mute value
    muteReq.profile = ActiveQmiProfile;
    muteReq.cwtmute_valid = true;
    if(mute)
    {
        muteReq.cwtmute = 1;
    }
    else
    {
        muteReq.cwtmute = 0;
    }

    LE_DEBUG("cwtmute %d", muteReq.cwtmute);

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(AudioClient,
                                            QMI_SWI_M2M_AUDIO_SET_AVMUTE_REQ_V01,
                                            &muteReq, sizeof(muteReq),
                                            &muteResp, sizeof(muteResp),
                                            COMM_DEFAULT_PLATFORM_TIMEOUT);

    if ( swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_SWI_M2M_AUDIO_SET_AVMUTE_REQ_V01),
                                        clientMsgErr,
                                        muteResp.resp.result, muteResp.resp.error) != LE_OK )
    {
        LE_ERROR("Cannot %s Call Waiting Tone",  (mute ? "mute":"unmute"));
        return IsAudioAvailable ? LE_FAULT : LE_UNAVAILABLE;
    }
    else
    {
        LE_DEBUG("%s Call Waiting Tone",  (mute ? "mute":"unmute"));
        return LE_OK;
    }
}
