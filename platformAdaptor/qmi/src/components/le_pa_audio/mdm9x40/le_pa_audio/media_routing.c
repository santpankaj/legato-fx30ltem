/**
 * @file media_routing.c
 *
 * QMI implementation for media routing.
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#include "legato.h"
#include "interfaces.h"
#include "le_audio_local.h"
#include "pa_audio.h"
#include "media_routing.h"
#include "pa_audio_local.h"
#include "swiQmi.h"

//--------------------------------------------------------------------------------------------------
// Symbol and Enum definitions.
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Symbols used to specify the Audio media backend
 */
//--------------------------------------------------------------------------------------------------
#define QMI_MEDIA_PCM              0
#define QMI_MEDIA_I2S              1
#define QMI_MEDIA_ANALOG           2
#define QMI_MEDIA_USB              3
#define QMI_MEDIA_PCM_2            4
#define QMI_MEDIA_I2S_2            5
#define QMI_MEDIA_ETHERNET         6
#define QMI_MEDIA_OTA              7

//--------------------------------------------------------------------------------------------------
/**
 * Symbols used to specify the Audio media operation
 */
//--------------------------------------------------------------------------------------------------
#define QMI_MEDIA_OP_PLAYBACK      0
#define QMI_MEDIA_OP_RECORD        1

//--------------------------------------------------------------------------------------------------
/**
 * Symbols used to specify the Audio frontend HW id (0xFF is Unused)
 */
//--------------------------------------------------------------------------------------------------
#define QMI_MEDIA_FRONTENDHWID_UNUSED   0xFF

//--------------------------------------------------------------------------------------------------
// Data structures.
//--------------------------------------------------------------------------------------------------

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
//                                       Public declarations
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Initialize the routing PA
 *
 */
//--------------------------------------------------------------------------------------------------
void media_routing_Initialize
(
    void
)
{
    // Get the qmi client service handle for later use.
    if ( (AudioClient = swiQmi_GetClientHandle(QMI_SERVICE_AUDIO)) == NULL)
    {
        LE_CRIT("Cannot get QMI_SERVICE_AUDIO client!");
        return;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the private routing PA parameters of a stream
 *
 */
//--------------------------------------------------------------------------------------------------
void media_routing_SetPaParams
(
    media_routing_Parameters_t *paMediaRoutingParamPtr
)
{
    if (paMediaRoutingParamPtr)
    {
        paMediaRoutingParamPtr->multimediaUsageCount = 0;
        paMediaRoutingParamPtr->frontEndHwId = QMI_MEDIA_FRONTENDHWID_UNUSED;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to set the DSP Audio path
 *
 * @return LE_FAULT         The function failed.
 * @return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t media_routing_SetDspAudioPath
(
    le_audio_Stream_t* inputStreamPtr,   ///< [IN] input audio stream
    le_audio_Stream_t* outputStreamPtr   ///< [IN] output audio stream
)
{
    uint8_t backEndDai;
    uint8_t operateType;

    if (!inputStreamPtr || !outputStreamPtr)
    {
        LE_ERROR("Bad input parameters");
        return LE_FAULT;
    }

    le_audio_If_t inputInterface = inputStreamPtr->audioInterface;
    le_audio_If_t outputInterface = outputStreamPtr->audioInterface;

    PaParameters_t* paStreamParamPtr;

    LE_DEBUG("inputInterface %d outputInterface %d", inputInterface, outputInterface);

    le_audio_If_t interface;
    le_audio_Stream_t* streamPtr;

    if (inputInterface == LE_AUDIO_IF_DSP_FRONTEND_FILE_PLAY)
    {
        operateType = QMI_MEDIA_OP_PLAYBACK;
        interface = outputInterface;
        streamPtr = inputStreamPtr;
    }
    else if (outputInterface == LE_AUDIO_IF_DSP_FRONTEND_FILE_CAPTURE)
    {
        operateType = QMI_MEDIA_OP_RECORD;
        interface = inputInterface;
        streamPtr = outputStreamPtr;
    }
    else
    {
        return LE_FAULT;
    }

    switch (interface)
    {
        case LE_AUDIO_IF_CODEC_MIC:
        case LE_AUDIO_IF_CODEC_SPEAKER:
            backEndDai = QMI_MEDIA_ANALOG;
            break;
        case LE_AUDIO_IF_DSP_FRONTEND_USB_RX:
        case LE_AUDIO_IF_DSP_FRONTEND_USB_TX:
            backEndDai = QMI_MEDIA_USB;
            break;
        case LE_AUDIO_IF_DSP_FRONTEND_PCM_RX:
        case LE_AUDIO_IF_DSP_FRONTEND_PCM_TX:
            backEndDai = QMI_MEDIA_PCM;
            break;
        case LE_AUDIO_IF_DSP_FRONTEND_I2S_RX:
        case LE_AUDIO_IF_DSP_FRONTEND_I2S_TX:
            backEndDai = QMI_MEDIA_I2S;
            break;
        case LE_AUDIO_IF_DSP_BACKEND_MODEM_VOICE_RX:
        case LE_AUDIO_IF_DSP_BACKEND_MODEM_VOICE_TX:
            backEndDai = QMI_MEDIA_OTA;
            break;
        default:
            LE_ERROR("Unknown interface %d", interface);
            return LE_FAULT;
    }

    paStreamParamPtr = GetPaParams(streamPtr);

    if (paStreamParamPtr->mediaRoutingParams.frontEndHwId == QMI_MEDIA_FRONTENDHWID_UNUSED)
    {
        // Set the physical interface
        SWIQMI_DECLARE_MESSAGE (swi_m2m_audio_set_media_req_msg_v01, mediaSetReq);
        SWIQMI_DECLARE_MESSAGE (swi_m2m_audio_set_media_resp_msg_v01, mediaSetResp);

        mediaSetReq.backenddai = backEndDai;
        mediaSetReq.operatetype = operateType;

        LE_DEBUG("Call QMI_SWI_M2M_AUDIO_SET_MEDIA_REQ_V01 { %u, %u } interface %u",
                 mediaSetReq.backenddai, mediaSetReq.operatetype, interface);

        qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(AudioClient,
                                                 QMI_SWI_M2M_AUDIO_SET_MEDIA_REQ_V01,
                                                 &mediaSetReq, sizeof(mediaSetReq),
                                                 &mediaSetResp, sizeof(mediaSetResp),
                                                 COMM_DEFAULT_PLATFORM_TIMEOUT);

        if ( swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_SWI_M2M_AUDIO_SET_MEDIA_REQ_V01),
                                  clientMsgErr,
                                  mediaSetResp.resp.result,
                                  mediaSetResp.resp.error) != LE_OK )
        {
            LE_ERROR("Cannot set the audio media interface to %d", interface);
            return LE_FAULT;
        }

        streamPtr->hwDeviceId =
            paStreamParamPtr->mediaRoutingParams.frontEndHwId = mediaSetResp.frontendhwid;
        LE_DEBUG("%d audio media interface successfully set: frontendhwid %d.",
                 interface, paStreamParamPtr->mediaRoutingParams.frontEndHwId);
    }
    else
    {
        // Add the physical interface
        SWIQMI_DECLARE_MESSAGE (swi_m2m_audio_add_media_req_msg_v01, mediaAddReq);
        SWIQMI_DECLARE_MESSAGE (swi_m2m_audio_add_media_resp_msg_v01, mediaAddResp);

        mediaAddReq.backenddai = backEndDai;
        mediaAddReq.frontendhwid = paStreamParamPtr->mediaRoutingParams.frontEndHwId;
        LE_DEBUG("Call QMI_SWI_M2M_AUDIO_ADD_MEDIA_REQ_V01 { %u, %u } interface %u",
                 mediaAddReq.backenddai, mediaAddReq.frontendhwid, interface);
        qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(AudioClient,
                                                 QMI_SWI_M2M_AUDIO_ADD_MEDIA_REQ_V01,
                                                 &mediaAddReq, sizeof(mediaAddReq),
                                                 &mediaAddResp, sizeof(mediaAddResp),
                                                 COMM_DEFAULT_PLATFORM_TIMEOUT);

        if ( swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_SWI_M2M_AUDIO_ADD_MEDIA_REQ_V01),
                                  clientMsgErr,
                                  mediaAddResp.resp.result,
                                  mediaAddResp.resp.error) != LE_OK )
        {
            LE_ERROR("Cannot add the audio media interface to %d with fronthwid %d",
                     interface, paStreamParamPtr->mediaRoutingParams.frontEndHwId);
            return LE_FAULT;
        }
        LE_DEBUG("%d audio media interface successfully added to frontendhwid %d.",

                 interface, paStreamParamPtr->mediaRoutingParams.frontEndHwId);
    }

    paStreamParamPtr->mediaRoutingParams.multimediaUsageCount++;

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to reset the DSP Audio path
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t media_routing_ResetDspAudioPath
(
    le_audio_Stream_t* inputStreamPtr,   ///< [IN] input audio stream
    le_audio_Stream_t* outputStreamPtr   ///< [IN] output audio stream
)
{
    if (!inputStreamPtr || !outputStreamPtr)
    {
        LE_ERROR("Bad input parameters");
        return LE_FAULT;
    }

    LE_DEBUG("Reset audio path streamRef %p and streamRef %p", inputStreamPtr->streamRef,
                                                               outputStreamPtr->streamRef );

    if ((inputStreamPtr->audioInterface == LE_AUDIO_IF_DSP_FRONTEND_FILE_PLAY) ||
        (outputStreamPtr->audioInterface == LE_AUDIO_IF_DSP_FRONTEND_FILE_CAPTURE))
    {

        le_audio_If_t inputInterface = inputStreamPtr->audioInterface;
        le_audio_If_t outputInterface = outputStreamPtr->audioInterface;

        // Reset the physical interface
        SWIQMI_DECLARE_MESSAGE (swi_m2m_audio_reset_media_req_msg_v01, mediaResetReq);
        SWIQMI_DECLARE_MESSAGE (swi_m2m_audio_reset_media_resp_msg_v01, mediaResetResp);
        le_audio_If_t interface;
        le_audio_Stream_t* streamPtr;

        if (inputInterface == LE_AUDIO_IF_DSP_FRONTEND_FILE_PLAY)
        {
            interface = outputInterface;
            streamPtr = inputStreamPtr;
        }
        else if (outputInterface == LE_AUDIO_IF_DSP_FRONTEND_FILE_CAPTURE)
        {
            interface = inputInterface;
            streamPtr = outputStreamPtr;
        }

        PaParameters_t* paStreamParamPtr = GetPaParams(streamPtr);

        if (paStreamParamPtr->mediaRoutingParams.multimediaUsageCount == 0)
        {
            return LE_OK;
        }

        switch (interface)
        {
            case LE_AUDIO_IF_CODEC_MIC:
            case LE_AUDIO_IF_CODEC_SPEAKER:
                mediaResetReq.backenddai = QMI_MEDIA_ANALOG;
                break;
            case LE_AUDIO_IF_DSP_FRONTEND_USB_RX:
            case LE_AUDIO_IF_DSP_FRONTEND_USB_TX:
                mediaResetReq.backenddai = QMI_MEDIA_USB;
                break;
            case LE_AUDIO_IF_DSP_FRONTEND_PCM_RX:
            case LE_AUDIO_IF_DSP_FRONTEND_PCM_TX:
                mediaResetReq.backenddai = QMI_MEDIA_PCM;
                break;
            case LE_AUDIO_IF_DSP_FRONTEND_I2S_RX:
            case LE_AUDIO_IF_DSP_FRONTEND_I2S_TX:
                mediaResetReq.backenddai = QMI_MEDIA_I2S;
                break;
            case LE_AUDIO_IF_DSP_BACKEND_MODEM_VOICE_RX:
            case LE_AUDIO_IF_DSP_BACKEND_MODEM_VOICE_TX:
                mediaResetReq.backenddai = QMI_MEDIA_OTA;
                break;
            default:
                LE_ERROR("Unknown interface %d", interface);
                return LE_FAULT;
        }

        mediaResetReq.frontendhwid = paStreamParamPtr->mediaRoutingParams.frontEndHwId;
        LE_DEBUG("Call QMI_SWI_M2M_AUDIO_RESET_MEDIA_REQ_V01 { %u, %u } interface %u",
                mediaResetReq.backenddai, mediaResetReq.frontendhwid, interface);
        qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(AudioClient,
                                                QMI_SWI_M2M_AUDIO_RESET_MEDIA_REQ_V01,
                                                &mediaResetReq, sizeof(mediaResetReq),
                                                &mediaResetResp, sizeof(mediaResetResp),
                                                COMM_DEFAULT_PLATFORM_TIMEOUT);

        if (swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_SWI_M2M_AUDIO_RESET_MEDIA_REQ_V01),
                                        clientMsgErr,
                                        mediaResetResp.resp.result,
                                        mediaResetResp.resp.error) != LE_OK)
        {
            LE_ERROR("Cannot reset the audio media interface to %d from frontendhwid %d",
                     interface, paStreamParamPtr->mediaRoutingParams.frontEndHwId);
            return LE_FAULT;
        }

        if (--paStreamParamPtr->mediaRoutingParams.multimediaUsageCount == 0)
        {
            paStreamParamPtr->mediaRoutingParams.frontEndHwId = QMI_MEDIA_FRONTENDHWID_UNUSED;
        }

        LE_DEBUG("%d audio media interface successfully reset.", interface);
        return LE_OK;

    }
    return LE_FAULT;

}
