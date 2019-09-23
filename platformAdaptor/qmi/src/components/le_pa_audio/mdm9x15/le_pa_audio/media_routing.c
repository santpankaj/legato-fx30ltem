/**
 * @file media_routing.c
 *
 * Amix implementation for media routing.
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
#include "sdudefs.h"


//--------------------------------------------------------------------------------------------------
// Symbol and Enum definitions.
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Macro to determine whether the codec is wired on SLIMBUS_0.
 */
//--------------------------------------------------------------------------------------------------
#define CODEC_ON_SLIMBUS_0 \
    (strncmp(STRINGIZE(CODEC_IF), "SLIMBUS_0", strlen("SLIMBUS_0")) == 0)

//--------------------------------------------------------------------------------------------------
/**
 * Macro to determine whether a HW interface is defined for a codec.
 */
//--------------------------------------------------------------------------------------------------
#define CODEC_IF_PRESENT  (strlen(STRINGIZE(CODEC_IF)))

//--------------------------------------------------------------------------------------------------
/**
 * Define the Audio device.
 */
//--------------------------------------------------------------------------------------------------
#define AUDIO_QUALCOMM_DEVICE_PATH  "/dev/snd/controlC0"

//--------------------------------------------------------------------------------------------------
/**
 * Define the Audio device.
 */
//--------------------------------------------------------------------------------------------------
#define AUDIO_QUALCOMM_DEVICE_PATH  "/dev/snd/controlC0"

//--------------------------------------------------------------------------------------------------
/**
 * Max multimedia devices.
 */
//--------------------------------------------------------------------------------------------------
#define MULTIMEDIA_DEVICE_MAX 2

//--------------------------------------------------------------------------------------------------
/**
 * Multimedia ctrl name devices max len.
 */
//--------------------------------------------------------------------------------------------------
#define MULTIMEDIA_ALSA_CTRL_NAME_MAX_LEN    20

//--------------------------------------------------------------------------------------------------
/**
 * DSP audio path structure.
 *
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    le_audio_Stream_t* inputStreamPtr;
    le_audio_Stream_t* outputStreamPtr;
    le_dls_Link_t      link;
}
DspAudioPath_t;

//--------------------------------------------------------------------------------------------------
/**
 * Multimedia device type
 *
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    bool                inUse;
    le_audio_Stream_t*  playCaptStreamPtr;           // stream which use the multimedia
    char                alsaCtrlNameStr[MULTIMEDIA_ALSA_CTRL_NAME_MAX_LEN+1];
                                                     // Multimedia device name
    int8_t              alsaHwIndex;                 // Hardware index
}
MultimediaDevice_t;

//--------------------------------------------------------------------------------------------------
//                                       Static declarations
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Multimedia devices parameters. MULTIMEDIA_DEVICE_MAX multimedia devices can be used. The
 * aditional context is used for the close operation.
 */
//--------------------------------------------------------------------------------------------------
static MultimediaDevice_t MultimediaDevice[MULTIMEDIA_DEVICE_MAX+1];


//--------------------------------------------------------------------------------------------------
/**
 * Create and initialize the DSP Audio Path list.
 *
 */
//--------------------------------------------------------------------------------------------------
static le_dls_List_t DspAudioPathList = LE_DLS_LIST_INIT;

//--------------------------------------------------------------------------------------------------
/**
 * Memory Pool for DSP Audio Path objects.
 *
 */
//--------------------------------------------------------------------------------------------------
static le_mem_PoolRef_t   DspAudioPathPool;

//--------------------------------------------------------------------------------------------------
/**
 * This function requests to swisync to perform a specific action
 * @return TRUE permission allowed
 * @return FALSE permission not allowed
 */
//--------------------------------------------------------------------------------------------------
static bool RequestPermission
(
    int type,
    sd_audio_op_e operation,
    int param
)
{
    sdmsg_type msg;

    msg.header.msgId = type;
    msg.header.len = sizeof(sd_audio_type);
    msg.header.pid = getpid();
    msg.u.audio.operation = operation;
    msg.u.audio.param = param;

    uint32_t answer;

    if (sdclient_send_msg(&msg) < 0)
    {
        LE_ERROR("Error during request sending 0x%x", type);
        // Operation allowed by default
        return true;
    }

    if (sdclient_receive_rsp((uint8_t*) &answer, sizeof(answer)) < 0)
    {
        LE_ERROR("Error during receiving answer 0x%x", type);
        // Operation allowed by default
        return true;
    }

    if (answer == 1)
    {
        return true;
    }
    else
    {
        return false;
    }
}


//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to flag for reset the DSP Audio path
 *
 * @return LE_FAULT         The function failed.
 * @return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t FlagForResetDspAudioPath
(
    le_audio_Stream_t* inputStreamPtr,
    le_audio_Stream_t* outputStreamPtr
)
{
    LE_DEBUG("Flag for Reset the following path: inputInterface %d outputInterface %d",
              inputStreamPtr->audioInterface, outputStreamPtr->audioInterface);

    DspAudioPath_t *dspAudioPathPtr=NULL;

    dspAudioPathPtr = (DspAudioPath_t*)le_mem_ForceAlloc(DspAudioPathPool);
    dspAudioPathPtr->link = LE_DLS_LINK_INIT;
    dspAudioPathPtr->inputStreamPtr = inputStreamPtr;
    dspAudioPathPtr->outputStreamPtr = outputStreamPtr;
    le_dls_Queue(&DspAudioPathList, &(dspAudioPathPtr->link));

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the device identifier for an audio path
 *
 */
//--------------------------------------------------------------------------------------------------
static le_result_t GetDeviceIdentifier
(
    le_audio_Stream_t* inputStreamPtr,
    le_audio_Stream_t* outputStreamPtr
)
{
    int i;
    le_audio_Stream_t* streamPtr;
    PaParameters_t* paInputStreamParamPtr = GetPaParams(inputStreamPtr);
    PaParameters_t* paOutputStreamParamPtr = GetPaParams(outputStreamPtr);

    if (!paInputStreamParamPtr || !paOutputStreamParamPtr)
    {
        return LE_FAULT;
    }

    if (inputStreamPtr->audioInterface == LE_AUDIO_IF_DSP_FRONTEND_FILE_PLAY)
    {
        streamPtr = inputStreamPtr;
    }
    else if (outputStreamPtr->audioInterface == LE_AUDIO_IF_DSP_FRONTEND_FILE_CAPTURE)
    {
        streamPtr = outputStreamPtr;
    }
    else
    {
        LE_WARN("Invalid audio interface type.  "
                "Expecting input interface (%d) == %d or output interface (%d) == %d",
                inputStreamPtr->audioInterface, LE_AUDIO_IF_DSP_FRONTEND_FILE_PLAY,
                outputStreamPtr->audioInterface, LE_AUDIO_IF_DSP_FRONTEND_FILE_CAPTURE);
        return LE_FAULT;
    }

    // first, look if the stream is already in use
    for (i=0; i < MULTIMEDIA_DEVICE_MAX; i++)
    {
        if ((MultimediaDevice[i].inUse == true) &&
            (MultimediaDevice[i].playCaptStreamPtr == streamPtr))
        {
            // Stream in use: use the same multimedia device
            streamPtr->deviceIdentifier = i;
            streamPtr->hwDeviceId =
                            MultimediaDevice[streamPtr->deviceIdentifier].alsaHwIndex;
            LE_DEBUG("Use device identifier:%d, Hw:%d",
                streamPtr->deviceIdentifier,
                streamPtr->hwDeviceId);

            paInputStreamParamPtr->mediaRoutingParams.multimediaUsageCount++;
            paOutputStreamParamPtr->mediaRoutingParams.multimediaUsageCount++;

            return LE_OK;
        }
    }

    // stream not in use: allocate a free multimedia

    int start=0, increment=0;

    // Regarding to available amix, MM_2_ALSA_MIXER_CTRL stream can't be used for recording.
    // So MM_1_ALSA_MIXER_CTRL is booked for recorder.
    // The first allocated playback stream uses the MM_2_ALSA_MIXER_CTRL to let free the
    // MM_1_ALSA_MIXER_CTRL for recording.
    // The first allocated capture stream uses MM_1_ALSA_MIXER_CTRL.
    if (inputStreamPtr->audioInterface == LE_AUDIO_IF_DSP_FRONTEND_FILE_PLAY)
    {
        start = MULTIMEDIA_DEVICE_MAX-1;
        increment = -1;
    }
    else
    {
        start = 0;
        increment = 1;
    }

    for (i=start; (i >=0 && i < MULTIMEDIA_DEVICE_MAX); i+=increment)
    {
        if (MultimediaDevice[i].inUse == false)
        {
            MultimediaDevice[i].playCaptStreamPtr = streamPtr;

            streamPtr->deviceIdentifier = i;
            MultimediaDevice[i].inUse = true;

            streamPtr->hwDeviceId =
                   MultimediaDevice[streamPtr->deviceIdentifier].alsaHwIndex;

            LE_DEBUG("Use device identifier:%d, Hw:%d",
                streamPtr->deviceIdentifier,
                streamPtr->hwDeviceId);

            paInputStreamParamPtr->mediaRoutingParams.multimediaUsageCount++;
            paOutputStreamParamPtr->mediaRoutingParams.multimediaUsageCount++;

            return LE_OK;
        }
    }

    LE_DEBUG("Failed to get a device identifier");

    return LE_FAULT;
}

//--------------------------------------------------------------------------------------------------
/**
 * Reset the device identifier of an audio path
 *
 */
//--------------------------------------------------------------------------------------------------
static void ResetDeviceIdentifier
(
    le_audio_Stream_t* inputStreamPtr,
    le_audio_Stream_t* outputStreamPtr
)
{
    PaParameters_t* paInputStreamParamPtr = GetPaParams(inputStreamPtr);
    PaParameters_t* paOutputStreamParamPtr = GetPaParams(outputStreamPtr);

    if (!paOutputStreamParamPtr)
    {
        return;
    }

    LE_DEBUG("input multimedia usage %d, output multimedia usage %d",
              paInputStreamParamPtr->mediaRoutingParams.multimediaUsageCount,
              paOutputStreamParamPtr->mediaRoutingParams.multimediaUsageCount);



    if (inputStreamPtr->audioInterface == LE_AUDIO_IF_DSP_FRONTEND_FILE_PLAY)
    {
        // Sanity check
        if ((inputStreamPtr->deviceIdentifier >= 0) &&
            (inputStreamPtr->deviceIdentifier < MULTIMEDIA_DEVICE_MAX) &&
            (MultimediaDevice[inputStreamPtr->deviceIdentifier].inUse == true) &&
            (MultimediaDevice[inputStreamPtr->deviceIdentifier].playCaptStreamPtr ==
                                                                                inputStreamPtr) &&
            (paInputStreamParamPtr->mediaRoutingParams.multimediaUsageCount > 0))
        {
            paInputStreamParamPtr->mediaRoutingParams.multimediaUsageCount--;

            if (paInputStreamParamPtr->mediaRoutingParams.multimediaUsageCount == 0)
            {
                MultimediaDevice[inputStreamPtr->deviceIdentifier].inUse = false;
                MultimediaDevice[inputStreamPtr->deviceIdentifier].playCaptStreamPtr = NULL;
                inputStreamPtr->deviceIdentifier = -1;
            }
        }

        if (paOutputStreamParamPtr->mediaRoutingParams.closeAllowed &&
           (paOutputStreamParamPtr->mediaRoutingParams.multimediaUsageCount > 0))
        {
            paOutputStreamParamPtr->mediaRoutingParams.multimediaUsageCount--;
        }


    }
    else if (outputStreamPtr->audioInterface == LE_AUDIO_IF_DSP_FRONTEND_FILE_CAPTURE)
    {
        // Sanity check
        if ((outputStreamPtr->deviceIdentifier >= 0) &&
            (outputStreamPtr->deviceIdentifier < MULTIMEDIA_DEVICE_MAX) &&
            (MultimediaDevice[outputStreamPtr->deviceIdentifier].inUse == true) &&
            (MultimediaDevice[outputStreamPtr->deviceIdentifier].playCaptStreamPtr !=
                                                                                outputStreamPtr) &&
            (paOutputStreamParamPtr->mediaRoutingParams.multimediaUsageCount > 0))
        {
            paOutputStreamParamPtr->mediaRoutingParams.multimediaUsageCount--;

            if (paOutputStreamParamPtr->mediaRoutingParams.multimediaUsageCount == 0)
            {
                MultimediaDevice[outputStreamPtr->deviceIdentifier].inUse = false;
                MultimediaDevice[outputStreamPtr->deviceIdentifier].playCaptStreamPtr = NULL;
                outputStreamPtr->deviceIdentifier = -1;
            }
        }

        if (paInputStreamParamPtr->mediaRoutingParams.multimediaUsageCount > 0)
        {
            paInputStreamParamPtr->mediaRoutingParams.multimediaUsageCount--;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Check if close audio path is possible
 *
 */
//--------------------------------------------------------------------------------------------------
static bool CheckMultimediaClosurePossibility
(
    le_audio_Stream_t* streamPtr
)
{
    PaParameters_t* paParamsPtr = GetPaParams(streamPtr);

    if (!paParamsPtr)
    {
        LE_ERROR("Bad parameter");
        return false;
    }

    LE_DEBUG("streamRef %p, closeAllowed %d multimediaUsageCount %d", streamPtr->streamRef,
                paParamsPtr->mediaRoutingParams.closeAllowed, paParamsPtr->mediaRoutingParams.multimediaUsageCount);

    if((paParamsPtr->mediaRoutingParams.closeAllowed == false) || (paParamsPtr->mediaRoutingParams.multimediaUsageCount != 1))
    {
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to set a mixer value
 *
 */
//--------------------------------------------------------------------------------------------------
static void SetMixerParameter
(
    const char* namePtr,
    const char* valuePtr
)
{
    struct mixer *mixerPtr;
    struct mixer_ctl *ctlPtr;

    LE_DEBUG("Set '%s' with value '%s'", namePtr, valuePtr);

    mixerPtr = mixer_open(AUDIO_QUALCOMM_DEVICE_PATH);
    if (mixerPtr==NULL)
    {
        LE_CRIT("Cannot open <%s>", AUDIO_QUALCOMM_DEVICE_PATH);
        return;
    }

    ctlPtr = mixer_get_control(mixerPtr, namePtr, 0);
    if (ctlPtr==NULL)
    {
        LE_CRIT("Cannot get mixer controler <%s>", namePtr);
        mixer_close(mixerPtr);
        return;
    }

    if (isdigit(valuePtr[0]))
    {
        LE_CRIT_IF((mixer_ctl_set_value(ctlPtr, 1, (char**)&valuePtr)),
                    "Cannot set the value <%s>", valuePtr);
    } else
    {
        LE_CRIT_IF((mixer_ctl_select(ctlPtr, valuePtr)),
                    "Cannot select the value <%s>", valuePtr);
    }
    mixer_close(mixerPtr);
}


//--------------------------------------------------------------------------------------------------
/**
 * This function sets the mixer parameters for a local play
 * @return LE_OK    on success
 * @return LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
static le_result_t SetMixerParameterForLocalPlay
(
    le_audio_Stream_t* inputStreamPtr,
    le_audio_Stream_t* outputStreamPtr
)
{
    if (!inputStreamPtr || !outputStreamPtr)
    {
        return LE_FAULT;
    }

    le_audio_If_t outputInterface = outputStreamPtr->audioInterface;
    char amixMultimedia[50]="\0";

    LE_DEBUG("Set local play %d", outputInterface);

    if (outputInterface == LE_AUDIO_IF_DSP_FRONTEND_PCM_TX)
    {
        snprintf(amixMultimedia, 50, "%s_PCM_RX Audio Mixer %s",
                    STRINGIZE(PCM_IF),
                    MultimediaDevice[inputStreamPtr->deviceIdentifier].alsaCtrlNameStr);
        SetMixerParameter(amixMultimedia, "1");
    }
    else if (outputInterface == LE_AUDIO_IF_DSP_FRONTEND_I2S_TX)
    {

        snprintf(amixMultimedia, 50, "%s_RX Audio Mixer %s",
                STRINGIZE(I2S_IF),
                MultimediaDevice[inputStreamPtr->deviceIdentifier].alsaCtrlNameStr);
        LE_INFO("Set Mixer %s", amixMultimedia);
        SetMixerParameter(amixMultimedia, "1");

    }
    else if (outputInterface == LE_AUDIO_IF_CODEC_SPEAKER)
    {
        if (CODEC_ON_SLIMBUS_0)
        {
            SetMixerParameter("SLIM_0_RX Channels", "One");
            SetMixerParameter("DAC3 MUX", "INV_RX1");
            SetMixerParameter("DAC2 MUX", "RX1");
            SetMixerParameter("RX1 MIX1 INP1", "RX1");
            SetMixerParameter("RX1 Digital Volume", "100");
            SetMixerParameter("Speaker Function", "On");

            snprintf(amixMultimedia, 50, "SLIMBUS_0_RX Audio Mixer %s",
                MultimediaDevice[inputStreamPtr->deviceIdentifier].alsaCtrlNameStr);

            SetMixerParameter(amixMultimedia, "1");
        }
        else if (CODEC_IF_PRESENT)
        {
            SetMixerParameter("SPKOUTP Mixer Speak PGA to Speaker P Switch", "1");
            SetMixerParameter("SPKOUTN Mixer Speak PGA to Speaker N Switch", "1");
            SetMixerParameter("Speaker Mixer DAC to Speak PGA Switch", "1");
            SetMixerParameter("Digital Playback Volume", "192");
            SetMixerParameter("Speaker Function", "On");

            snprintf(amixMultimedia, 50, "%s_RX Audio Mixer %s",
                     STRINGIZE(CODEC_IF),
                     MultimediaDevice[inputStreamPtr->deviceIdentifier].alsaCtrlNameStr);

            SetMixerParameter(amixMultimedia, "1");

        }
        else
        {
            LE_ERROR("Codec HW interface is not defined!");
            return LE_FAULT;
        }
    }
    else if (outputInterface == LE_AUDIO_IF_DSP_FRONTEND_USB_TX)
    {
        snprintf(amixMultimedia, 50, "%s_RX Audio Mixer %s",
                 STRINGIZE(USB_IF),
                 MultimediaDevice[inputStreamPtr->deviceIdentifier].alsaCtrlNameStr);
        SetMixerParameter(amixMultimedia, "1");
    }
    else
    {
        LE_ERROR("This interface (%d) is not supported",outputInterface);
        return LE_FAULT;
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function requests to swisync the permission to set the mixer parameters for a local play,
 * and set the parameters if allowed.
 *
 * @return LE_OK    on success
 * @return LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
static le_result_t SetMixerParameterForLocalPlayWithPermission
(
    le_audio_Stream_t* inputStreamPtr,
    le_audio_Stream_t* outputStreamPtr
)
{
    if (!RequestPermission( SD_SYNC_AUDIO_LOCAL_PLAY,
                            SYNC_AUDIO_OP_ACTIVATED,
                            0 ))
    {
        // Anyway, swisync always gives the permission to open an audio path
        // The request is done to warn swisync that Legato is starting a playback
        LE_ERROR("swisync discards the playback !");
        return LE_FAULT;
    }

    return SetMixerParameterForLocalPlay(inputStreamPtr, outputStreamPtr);
}

//--------------------------------------------------------------------------------------------------
/**
 * This function resets the mixer parameter for a local play
 * @return LE_OK    on success
 * @return LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
static le_result_t ResetMixerParameterForLocalPlay
(
    le_audio_Stream_t* inputStreamPtr,
    le_audio_Stream_t* outputStreamPtr
)
{
    if (!inputStreamPtr || !outputStreamPtr)
    {
        return LE_FAULT;
    }

    le_audio_If_t outputInterface = outputStreamPtr->audioInterface;
    char amixMultimedia[50]="\0";

    LE_DEBUG("Reset local play %d", outputInterface);

    if (outputInterface == LE_AUDIO_IF_DSP_FRONTEND_PCM_TX)
    {
        snprintf(amixMultimedia, 50, "%s_PCM_RX Audio Mixer %s",
                    STRINGIZE(PCM_IF),
                    MultimediaDevice[inputStreamPtr->deviceIdentifier].alsaCtrlNameStr);
        SetMixerParameter(amixMultimedia, "0");
    }
    else if (outputInterface == LE_AUDIO_IF_DSP_FRONTEND_I2S_TX)
    {

        snprintf(amixMultimedia, 50, "%s_RX Audio Mixer %s",
                    STRINGIZE(I2S_IF),
                    MultimediaDevice[inputStreamPtr->deviceIdentifier].alsaCtrlNameStr);
        SetMixerParameter(amixMultimedia, "0");
    }
    else if (outputInterface == LE_AUDIO_IF_CODEC_SPEAKER)
    {
        if (CODEC_ON_SLIMBUS_0)
        {
            if (CheckMultimediaClosurePossibility(outputStreamPtr) == true)
            {
                SetMixerParameter("DAC3 MUX", "ZERO");
                SetMixerParameter("DAC2 MUX", "ZERO");
                SetMixerParameter("RX1 MIX1 INP1", "ZERO");
                SetMixerParameter("RX1 Digital Volume", "0");
                SetMixerParameter("Speaker Function", "Off");
            }
            else
            {
                LE_DEBUG("Closure postponed");
            }

            if ((CheckMultimediaClosurePossibility(inputStreamPtr) == true)
                && (inputStreamPtr->deviceIdentifier < MULTIMEDIA_DEVICE_MAX))
            {
                snprintf(amixMultimedia, 50, "SLIMBUS_0_RX Audio Mixer %s",
                    MultimediaDevice[inputStreamPtr->deviceIdentifier].alsaCtrlNameStr);
                SetMixerParameter(amixMultimedia, "0");
            }
        }
        else if (CODEC_IF_PRESENT)
        {
            if (CheckMultimediaClosurePossibility(outputStreamPtr) == true)
            {
                SetMixerParameter("SPKOUTP Mixer Speak PGA to Speaker P Switch", "0");
                SetMixerParameter("SPKOUTN Mixer Speak PGA to Speaker N Switch", "0");
                SetMixerParameter("Speaker Mixer DAC to Speak PGA Switch", "0");
                SetMixerParameter("Digital Playback Volume", "0");
                SetMixerParameter("Speaker Function", "Off");
            }
            else
            {
                LE_DEBUG("Closure postponed");
            }

            if ((CheckMultimediaClosurePossibility(inputStreamPtr) == true)
                && (inputStreamPtr->deviceIdentifier < MULTIMEDIA_DEVICE_MAX))
            {
                snprintf(amixMultimedia, 50, "%s_RX Audio Mixer %s",
                         STRINGIZE(CODEC_IF),
                         MultimediaDevice[inputStreamPtr->deviceIdentifier].alsaCtrlNameStr);
                SetMixerParameter(amixMultimedia, "0");
            }
        }
        else
        {
            LE_ERROR("Codec HW interface is not defined!");
            return LE_FAULT;
        }
    }
    else if (outputInterface == LE_AUDIO_IF_DSP_FRONTEND_USB_TX)
    {
        if ((CheckMultimediaClosurePossibility(inputStreamPtr) == true)
            && (inputStreamPtr->deviceIdentifier < MULTIMEDIA_DEVICE_MAX))
        {
            snprintf(amixMultimedia, 50, "%s_RX Audio Mixer %s",
                     STRINGIZE(USB_IF),
                     MultimediaDevice[inputStreamPtr->deviceIdentifier].alsaCtrlNameStr);

            SetMixerParameter(amixMultimedia, "0");
        }
    }
    else
    {
        LE_ERROR("This interface (%d) is not supported",outputInterface);
        return LE_FAULT;
    }

    ResetDeviceIdentifier(inputStreamPtr, outputStreamPtr);

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to reset the DSP Audio path
 *
 */
//--------------------------------------------------------------------------------------------------
static void ResetDspAudioPath
(
    void
)
{
    DspAudioPath_t* dspAudioPathPtr=NULL;
    le_dls_Link_t*  linkPtr;

    while ((linkPtr = le_dls_Pop(&DspAudioPathList)) != NULL)
    {
        dspAudioPathPtr = CONTAINER_OF(linkPtr, DspAudioPath_t, link);

        if (dspAudioPathPtr->inputStreamPtr->audioInterface == LE_AUDIO_IF_DSP_FRONTEND_FILE_PLAY)
        {
            LE_DEBUG("Reset mixer input %p output %p",
                     dspAudioPathPtr->inputStreamPtr,dspAudioPathPtr->outputStreamPtr);

            GetPaParams(dspAudioPathPtr->outputStreamPtr)->mediaRoutingParams.closeAllowed = true;

            ResetMixerParameterForLocalPlay(dspAudioPathPtr->inputStreamPtr,
                                            dspAudioPathPtr->outputStreamPtr);
        }

        le_mem_Release(dspAudioPathPtr);
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * This function requests to swisync the permission to reset the mixer parameters for a local play,
 * and reset the parameters if allowed.
 *
 * @return LE_OK    on success
 * @return LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
static le_result_t ResetMixerParameterForLocalPlayWithPermission
(
    le_audio_Stream_t* inputStreamPtr,
    le_audio_Stream_t* outputStreamPtr
)
{
    if (!inputStreamPtr || !outputStreamPtr)
    {
        LE_ERROR("Bad input parameters");
        return LE_FAULT;
    }

    PaParameters_t* paParamsPtr = GetPaParams(inputStreamPtr);

    if (!paParamsPtr)
    {
        LE_ERROR("Bad parameter");
        return LE_FAULT;
    }

    // Check if we have other playback in progress
    int i;
    uint8_t deviceId = inputStreamPtr->deviceIdentifier;
    bool otherPlaybackInProgress = false;
    bool permission = false;

    for (i=0; i<MULTIMEDIA_DEVICE_MAX; i++)
    {
        if ((i != deviceId) && MultimediaDevice[i].inUse)
        {
            if (MultimediaDevice[i].playCaptStreamPtr->audioInterface ==
                LE_AUDIO_IF_DSP_FRONTEND_FILE_PLAY )
            {
                otherPlaybackInProgress = true;
                break;
            }
        }
    }

    if (!otherPlaybackInProgress)
    {
        permission = RequestPermission( SD_SYNC_AUDIO_LOCAL_PLAY,
                                        SYNC_AUDIO_OP_DEACTIVATED,
                                        0 );
    }

    LE_DEBUG("otherPlaybackInProgress %d, permission %d", otherPlaybackInProgress, permission);

    if (!permission)
    {
        paParamsPtr->mediaRoutingParams.closeAllowed = false;
        FlagForResetDspAudioPath(inputStreamPtr, outputStreamPtr);
    }
    else
    {
        paParamsPtr->mediaRoutingParams.closeAllowed = true;
        // As we can close audio path now, flush the old audio paths
        ResetDspAudioPath();
    }

    return ResetMixerParameterForLocalPlay(inputStreamPtr, outputStreamPtr);
 }

//--------------------------------------------------------------------------------------------------
/**
 * This function sets the mixer parameters for a local capture
 * @return LE_OK    on success
 * @return LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
static le_result_t SetMixerParameterForLocalCapture
(
    le_audio_Stream_t* inputStreamPtr,
    le_audio_Stream_t* outputStreamPtr
)
{
    if (!inputStreamPtr || !outputStreamPtr)
    {
        LE_ERROR("Bad input parameters");
        return LE_FAULT;
    }

    le_audio_If_t inputInterface = inputStreamPtr->audioInterface;
    char amixMultimedia[50]="\0";

    LE_DEBUG("Set local capture %d", inputInterface);

    if (inputInterface == LE_AUDIO_IF_DSP_FRONTEND_PCM_RX)
    {
        snprintf(amixMultimedia, 50, "%s Mixer %s_PCM_UL_TX",
                MultimediaDevice[outputStreamPtr->deviceIdentifier].alsaCtrlNameStr,
                 STRINGIZE(PCM_IF));

        SetMixerParameter(amixMultimedia, "1");
    }
    else if (inputInterface == LE_AUDIO_IF_DSP_FRONTEND_I2S_RX)
    {
        snprintf(amixMultimedia, 50, "%s Mixer %s_TX",
            MultimediaDevice[outputStreamPtr->deviceIdentifier].alsaCtrlNameStr,
            STRINGIZE(I2S_IF));

        SetMixerParameter(amixMultimedia, "1");
    }
    else if (inputInterface == LE_AUDIO_IF_CODEC_MIC)
    {
        if (CODEC_ON_SLIMBUS_0)
        {
            SetMixerParameter("SLIM_0_TX Channels", "One");
            SetMixerParameter("SLIM TX1 MUX", "DEC1");
            SetMixerParameter("DEC1 MUX", "ADC1");

            snprintf(amixMultimedia, 50, "%s Mixer SLIM_0_TX",
                MultimediaDevice[outputStreamPtr->deviceIdentifier].alsaCtrlNameStr);

            SetMixerParameter(amixMultimedia, "1");
        }
        else if (CODEC_IF_PRESENT)
        {
            SetMixerParameter("Input PGA IN1 to InPGA Switch", "1");
            SetMixerParameter("Input PGA AUX to Invert InPGA Switch", "1");
            SetMixerParameter("Capture PGA Mute Switch", "1");
            SetMixerParameter("ADC Mute All Switch", "1");

            snprintf(amixMultimedia, 50, "%s Mixer %s_TX",
                MultimediaDevice[outputStreamPtr->deviceIdentifier].alsaCtrlNameStr,
                STRINGIZE(CODEC_IF));

            SetMixerParameter(amixMultimedia, "1");
        }
        else
        {
            LE_ERROR("Codec HW interface is not defined!");
            return LE_FAULT;
        }
    }
    else if (inputInterface == LE_AUDIO_IF_DSP_FRONTEND_USB_RX)
    {
        snprintf(amixMultimedia, 50, "%s Mixer %s_TX",
            MultimediaDevice[outputStreamPtr->deviceIdentifier].alsaCtrlNameStr,
            STRINGIZE(USB_IF));

        SetMixerParameter(amixMultimedia, "1");
    }
    else
    {
        LE_DEBUG("This interface (%d) is not supported",inputInterface);
        return LE_FAULT;
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function resets the mixer parameter for a local capture
 * @return LE_OK    on success
 * @return LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
static le_result_t ResetMixerParameterForLocalCapture
(
    le_audio_Stream_t* inputStreamPtr,
    le_audio_Stream_t* outputStreamPtr
)
{
    if (!inputStreamPtr || !outputStreamPtr)
    {
        LE_ERROR("Bad input parameters");
        return LE_FAULT;
    }

    le_audio_If_t inputInterface = inputStreamPtr->audioInterface;
    char amixMultimedia[50]="\0";

    LE_DEBUG("Reset local capture %d", inputInterface);

    if (inputInterface == LE_AUDIO_IF_DSP_FRONTEND_PCM_RX)
    {
        if ((CheckMultimediaClosurePossibility(outputStreamPtr) == true)
            && (inputStreamPtr->deviceIdentifier < MULTIMEDIA_DEVICE_MAX))
        {
            snprintf(amixMultimedia, 50, "%s Mixer %s_PCM_UL_TX",
                MultimediaDevice[outputStreamPtr->deviceIdentifier].alsaCtrlNameStr,
                     STRINGIZE(PCM_IF));

            SetMixerParameter(amixMultimedia, "0");
        }
    }
    else if (inputInterface == LE_AUDIO_IF_DSP_FRONTEND_I2S_RX)
    {
        if ((CheckMultimediaClosurePossibility(outputStreamPtr) == true)
            && (inputStreamPtr->deviceIdentifier < MULTIMEDIA_DEVICE_MAX))
        {
            snprintf(amixMultimedia, 50, "%s Mixer %s_TX",
                MultimediaDevice[outputStreamPtr->deviceIdentifier].alsaCtrlNameStr,
                STRINGIZE(I2S_IF));

            SetMixerParameter(amixMultimedia, "0");
        }
    }
    else if (inputInterface == LE_AUDIO_IF_CODEC_MIC)
    {
        if (CODEC_ON_SLIMBUS_0)
        {
            if (CheckMultimediaClosurePossibility(inputStreamPtr) == true)
            {
                SetMixerParameter("SLIM TX1 MUX", "ZERO");
                SetMixerParameter("DEC1 MUX", "ZERO");
            }

            if ((CheckMultimediaClosurePossibility(outputStreamPtr) == true)
                && (inputStreamPtr->deviceIdentifier < MULTIMEDIA_DEVICE_MAX))
            {
                snprintf(amixMultimedia, 50, "%s Mixer SLIM_0_TX",
                    MultimediaDevice[outputStreamPtr->deviceIdentifier].alsaCtrlNameStr);

                SetMixerParameter(amixMultimedia, "0");
            }
        }
        else if (CODEC_IF_PRESENT)
        {
            if (CheckMultimediaClosurePossibility(inputStreamPtr) == true)
            {
                SetMixerParameter("Input PGA IN1 to InPGA Switch", "0");
                SetMixerParameter("Input PGA AUX to Invert InPGA Switch", "0");
                SetMixerParameter("Capture PGA Mute Switch", "0");
                SetMixerParameter("ADC Mute All Switch", "0");
            }

            if ((CheckMultimediaClosurePossibility(outputStreamPtr) == true)
                && (inputStreamPtr->deviceIdentifier < MULTIMEDIA_DEVICE_MAX))
            {
                snprintf(amixMultimedia, 50, "%s Mixer %s_TX",
                    MultimediaDevice[outputStreamPtr->deviceIdentifier].alsaCtrlNameStr,
                    STRINGIZE(CODEC_IF));

                SetMixerParameter(amixMultimedia, "0");
            }
        }
        else
        {
            LE_ERROR("Codec HW interface is not defined!");
            return LE_FAULT;
        }
    }
    else if (inputInterface == LE_AUDIO_IF_DSP_FRONTEND_USB_RX)
    {
        if ((CheckMultimediaClosurePossibility(outputStreamPtr) == true)
             && (inputStreamPtr->deviceIdentifier < MULTIMEDIA_DEVICE_MAX))
        {
            snprintf(amixMultimedia, 50, "%s Mixer %s_TX",
                    MultimediaDevice[outputStreamPtr->deviceIdentifier].alsaCtrlNameStr,
                    STRINGIZE(USB_IF));

            SetMixerParameter(amixMultimedia, "0");
        }
    }
    else
    {
        LE_DEBUG("This interface (%d) is not supported",inputInterface);
        return LE_FAULT;
    }

    ResetDeviceIdentifier(inputStreamPtr, outputStreamPtr);

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function sets the mixer parameters for a remote play
 * @return LE_OK    on success
 * @return LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
static le_result_t SetMixerParameterForRemotePlay
(
    le_audio_Stream_t* inputStreamPtr,
    le_audio_Stream_t* outputStreamPtr
)
{
    LE_DEBUG("Set remote play");
    char amixMultimedia[50]="\0";

    snprintf(amixMultimedia, 50, "Incall_Music Audio Mixer %s",
        MultimediaDevice[inputStreamPtr->deviceIdentifier].alsaCtrlNameStr);

    SetMixerParameter(amixMultimedia, "1");

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function resets the mixer parameters for a remote play
 * @return LE_OK    on success
 * @return LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
static le_result_t ResetMixerParameterForRemotePlay
(
    le_audio_Stream_t* inputStreamPtr,
    le_audio_Stream_t* outputStreamPtr
)
{
    char amixMultimedia[50]="\0";

    LE_DEBUG("Reset remote play");

    if ((CheckMultimediaClosurePossibility(inputStreamPtr) == true)
         && (inputStreamPtr->deviceIdentifier < MULTIMEDIA_DEVICE_MAX))
    {
        snprintf(amixMultimedia, 50, "Incall_Music Audio Mixer %s",
            MultimediaDevice[inputStreamPtr->deviceIdentifier].alsaCtrlNameStr);

        SetMixerParameter(amixMultimedia, "0");
    }

    ResetDeviceIdentifier(inputStreamPtr, outputStreamPtr);

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function sets the mixer parameters for a remote capture
 * @return LE_OK    on success
 * @return LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
static le_result_t SetMixerParameterForRemoteCapture
(
    le_audio_Stream_t* inputStreamPtr,
    le_audio_Stream_t* outputStreamPtr
)
{
    LE_DEBUG("Set remote capture");

    char amixMultimedia[50]="\0";

    snprintf(amixMultimedia, 50, "%s Mixer VOC_REC_DL",
        MultimediaDevice[outputStreamPtr->deviceIdentifier].alsaCtrlNameStr);

    SetMixerParameter(amixMultimedia, "1");

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function resets the mixer parameter for a remote capture
 * @return LE_OK    on success
 * @return LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
static le_result_t ResetMixerParameterForRemoteCapture
(
    le_audio_Stream_t* inputStreamPtr,
    le_audio_Stream_t* outputStreamPtr
)
{
    LE_DEBUG("Reset remote capture");

    char amixMultimedia[50]="\0";

    snprintf(amixMultimedia, 50, "%s Mixer VOC_REC_DL",
        MultimediaDevice[outputStreamPtr->deviceIdentifier].alsaCtrlNameStr);


    SetMixerParameter(amixMultimedia, "0");

    ResetDeviceIdentifier(inputStreamPtr, outputStreamPtr);

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function sets the mixer parameter for playback and capture
 */
//--------------------------------------------------------------------------------------------------
static le_result_t SetMixerParameterForPlaybackAndCapture
(
    le_audio_Stream_t* inputStreamPtr,   ///< [IN] input audio stream
    le_audio_Stream_t* outputStreamPtr   ///< [IN] output audio stream
)
{
    le_audio_If_t inputInterface = inputStreamPtr->audioInterface;
    le_audio_If_t outputInterface = outputStreamPtr->audioInterface;

    if ( GetDeviceIdentifier(inputStreamPtr, outputStreamPtr) != LE_OK )
    {
        return LE_FAULT;
    }

    if (inputInterface == LE_AUDIO_IF_DSP_FRONTEND_FILE_PLAY)
    {
        if (outputInterface != LE_AUDIO_IF_DSP_BACKEND_MODEM_VOICE_TX)
        {
            return SetMixerParameterForLocalPlayWithPermission(inputStreamPtr, outputStreamPtr);
        }
        else
        {
            // Don't ask permission to swisync for remote play: voice call and playback are
            // synchronized in this case
            return SetMixerParameterForRemotePlay(inputStreamPtr, outputStreamPtr);
        }
    }

    if (outputInterface == LE_AUDIO_IF_DSP_FRONTEND_FILE_CAPTURE)
    {
        if (inputInterface != LE_AUDIO_IF_DSP_BACKEND_MODEM_VOICE_RX)
        {
            return SetMixerParameterForLocalCapture(inputStreamPtr, outputStreamPtr);
        }
        else
        {
            return SetMixerParameterForRemoteCapture(inputStreamPtr, outputStreamPtr);
        }
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function resets the mixer parameter for playback and capture
 */
//--------------------------------------------------------------------------------------------------
static le_result_t ResetMixerParameterForPlaybackAndCapture
(
    le_audio_Stream_t* inputStreamPtr,
    le_audio_Stream_t* outputStreamPtr
)
{
    if (!inputStreamPtr || !outputStreamPtr)
    {
        LE_ERROR("Bad input parameters");
        return LE_FAULT;
    }

    LE_DEBUG("Reset audio path streamRef %p and streamRef %p", inputStreamPtr->streamRef,
                                                               outputStreamPtr->streamRef );

    le_audio_If_t inputInterface = inputStreamPtr->audioInterface;
    le_audio_If_t outputInterface = outputStreamPtr->audioInterface;

    if (inputInterface == LE_AUDIO_IF_DSP_FRONTEND_FILE_PLAY)
    {
        if (outputInterface != LE_AUDIO_IF_DSP_BACKEND_MODEM_VOICE_TX)
        {
            return ResetMixerParameterForLocalPlayWithPermission(inputStreamPtr, outputStreamPtr);
        }
        else
        {
            // Don't ask to swisync as we didn't ask for setting the parameters
            return ResetMixerParameterForRemotePlay(inputStreamPtr, outputStreamPtr);
        }
    }

    if (outputInterface == LE_AUDIO_IF_DSP_FRONTEND_FILE_CAPTURE)
    {
        if (inputInterface != LE_AUDIO_IF_DSP_BACKEND_MODEM_VOICE_RX)
        {
            return ResetMixerParameterForLocalCapture(inputStreamPtr, outputStreamPtr);
        }
        else
        {
            return ResetMixerParameterForRemoteCapture(inputStreamPtr, outputStreamPtr);
        }
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to initialize the DSP audio path resources.
 *
 * @return
 *      LE_OK if successful.
 *      LE_FAULT if unsuccessful.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t InitializeDspAudioPathResources
(
    void
)
{
    DspAudioPathPool = le_mem_CreatePool("DspAudioPathPool", sizeof(DspAudioPath_t));

    le_audio_Stream_t inputStream;
    le_audio_Stream_t outputStream;
    PaParameters_t    inputPaParameters;
    PaParameters_t    outputPaParameters;

    memset(&inputStream, 0, sizeof(le_audio_Stream_t));
    memset(&outputStream, 0, sizeof(le_audio_Stream_t));
    memset(&inputPaParameters, 0, sizeof(PaParameters_t));
    memset(&outputPaParameters, 0, sizeof(PaParameters_t));
    inputStream.PaParams = (pa_audio_Params_t) &inputPaParameters;
    outputStream.PaParams = (pa_audio_Params_t) &outputPaParameters;
    inputPaParameters.mediaRoutingParams.closeAllowed = true;
    outputPaParameters.mediaRoutingParams.closeAllowed = true;


    uint8_t deviceId = 0;

    for (;deviceId<MULTIMEDIA_DEVICE_MAX;deviceId++)
    {
        le_audio_If_t audioIf = LE_AUDIO_IF_CODEC_MIC;

        for(; audioIf<LE_AUDIO_NUM_INTERFACES; audioIf++)
        {
            MultimediaDevice[deviceId].inUse = true;
            inputPaParameters.mediaRoutingParams.multimediaUsageCount = 1;
            outputPaParameters.mediaRoutingParams.multimediaUsageCount = 1;

            switch (audioIf)
            {
                case LE_AUDIO_IF_CODEC_SPEAKER:
                case LE_AUDIO_IF_DSP_FRONTEND_USB_TX:
                case LE_AUDIO_IF_DSP_FRONTEND_PCM_TX:
                case LE_AUDIO_IF_DSP_FRONTEND_I2S_TX:
                    inputStream.deviceIdentifier = deviceId;
                    outputStream.audioInterface = audioIf;
                    MultimediaDevice[deviceId].playCaptStreamPtr = &inputStream;
                    inputStream.audioInterface = LE_AUDIO_IF_DSP_FRONTEND_FILE_PLAY;
                    ResetMixerParameterForLocalPlay(&inputStream, &outputStream);
                break;
                case LE_AUDIO_IF_DSP_BACKEND_MODEM_VOICE_TX:
                    inputStream.deviceIdentifier = deviceId;
                    outputStream.audioInterface = audioIf;
                    MultimediaDevice[deviceId].playCaptStreamPtr = &inputStream;
                    inputStream.audioInterface = LE_AUDIO_IF_DSP_FRONTEND_FILE_PLAY;
                    ResetMixerParameterForRemotePlay(&inputStream, &outputStream);
                break;
                case LE_AUDIO_IF_CODEC_MIC:
                case LE_AUDIO_IF_DSP_FRONTEND_USB_RX:
                case LE_AUDIO_IF_DSP_FRONTEND_PCM_RX:
                case LE_AUDIO_IF_DSP_FRONTEND_I2S_RX:
                    outputStream.deviceIdentifier = deviceId;
                    inputStream.audioInterface = audioIf;
                    MultimediaDevice[deviceId].playCaptStreamPtr = &outputStream;
                    outputStream.audioInterface = LE_AUDIO_IF_DSP_FRONTEND_FILE_CAPTURE;
                    ResetMixerParameterForLocalCapture(&inputStream, &outputStream);
                break;
                case LE_AUDIO_IF_DSP_BACKEND_MODEM_VOICE_RX:
                    outputStream.deviceIdentifier = deviceId;
                    inputStream.audioInterface = audioIf;
                    MultimediaDevice[deviceId].playCaptStreamPtr = &outputStream;
                    outputStream.audioInterface = LE_AUDIO_IF_DSP_FRONTEND_FILE_CAPTURE;
                    ResetMixerParameterForRemoteCapture(&inputStream, &outputStream);
                break;
                case LE_AUDIO_NUM_INTERFACES:
                default:
                break;
            }
        }

        MultimediaDevice[deviceId].inUse = false;
        MultimediaDevice[deviceId].playCaptStreamPtr = NULL;
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * swisync handler (receive notification from swisync)
 *
 */
//--------------------------------------------------------------------------------------------------
static void SdclientHandler
(
    sdmsg_type* msgPtr
)
{
    if ( msgPtr->header.msgId == SD_SYNC_AUDIO_LOCAL_PLAY )
    {
        LE_DEBUG("SD_SYNC_AUDIO_LOCAL_PLAY received from swisync");

        if (msgPtr->u.audio.operation == SYNC_AUDIO_OP_ACTIVATED)
        {
            LE_ERROR("swisync postponed an playback !!");
        }
        else
        {
            ResetDspAudioPath();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Registration with swisync
 *
 */
//--------------------------------------------------------------------------------------------------
static void SwisyncInit
(
    void
)
{
    // legato is registrated to swisync
    // The parameter is the socket name to be opened by Legato to receive notification from swisync
    // (done by the library libsierra_sd_client.a)
    sdclient_init("/tmp/swisync/legato");

    // set handler to receive swisync notification
    sdclient_set_handler(SdclientHandler);
}

//--------------------------------------------------------------------------------------------------
//                                       Public declarations
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to initialize the PA Audio module.
 */
//--------------------------------------------------------------------------------------------------
void media_routing_Initialize
(
    void
)
{
    memset(MultimediaDevice, 0, MULTIMEDIA_DEVICE_MAX*sizeof(MultimediaDevice_t));

    // MultiMedia interface name list. Hardware interface index will be filled during
    // the component initialization.
    strncpy(MultimediaDevice[0].alsaCtrlNameStr, STRINGIZE(MM_1_ALSA_MIXER_CTRL),
        MULTIMEDIA_ALSA_CTRL_NAME_MAX_LEN);
    MultimediaDevice[0].alsaHwIndex = MM_1_ALSA_PCM_DEVICE_ID;
    strncpy(MultimediaDevice[1].alsaCtrlNameStr, STRINGIZE(MM_2_ALSA_MIXER_CTRL),
        MULTIMEDIA_ALSA_CTRL_NAME_MAX_LEN);
    MultimediaDevice[1].alsaHwIndex = MM_2_ALSA_PCM_DEVICE_ID;

    LE_INFO("MM_CTRL_1=%s Hw:0,%d  MM_CTRL_2=%s Hw:0,%d",
        MultimediaDevice[0].alsaCtrlNameStr,
        MultimediaDevice[0].alsaHwIndex,
        MultimediaDevice[1].alsaCtrlNameStr,
        MultimediaDevice[1].alsaHwIndex);

    SwisyncInit();

    InitializeDspAudioPathResources();
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
        paMediaRoutingParamPtr->closeAllowed = true;
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
    le_result_t result = LE_OK;

    if (!inputStreamPtr || !outputStreamPtr)
    {
        LE_ERROR("Bad input !!");
        return LE_FAULT;
    }

    le_audio_If_t inputInterface = inputStreamPtr->audioInterface;
    le_audio_If_t outputInterface = outputStreamPtr->audioInterface;

    LE_DEBUG("inputInterface %d outputInterface %d", inputInterface, outputInterface);

    if ((inputInterface == LE_AUDIO_IF_DSP_FRONTEND_FILE_PLAY) ||
        (outputInterface == LE_AUDIO_IF_DSP_FRONTEND_FILE_CAPTURE))
    {
        result = SetMixerParameterForPlaybackAndCapture(inputStreamPtr, outputStreamPtr);
    }

    return result;
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
    le_result_t  res = LE_FAULT;

    if (!inputStreamPtr || !outputStreamPtr)
    {
        LE_ERROR("Bad input !!");
        return LE_FAULT;
    }

    if ((inputStreamPtr->audioInterface == LE_AUDIO_IF_DSP_FRONTEND_FILE_PLAY) ||
        (outputStreamPtr->audioInterface == LE_AUDIO_IF_DSP_FRONTEND_FILE_CAPTURE))
    {
        res = ResetMixerParameterForPlaybackAndCapture(inputStreamPtr, outputStreamPtr);
    }

    return res;
}
