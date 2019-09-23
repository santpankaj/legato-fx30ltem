/**
 * @file pa_media.c
 *
 * This file contains the source code of the low level Audio API for playback / capture
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#include "legato.h"
#include "interfaces.h"
#include "le_audio_local.h"
#include "pa_audio.h"
#include "pa_pcm.h"

#include <alsa-intf/alsa_audio.h>
#include <sys/poll.h>


//--------------------------------------------------------------------------------------------------
// Symbol and Enum definitions.
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
/**
 * Define the thread name length.
 */
//--------------------------------------------------------------------------------------------------
#define PCM_THREAD_NAME_LEN 30

//--------------------------------------------------------------------------------------------------
// Data structures.
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Platform Adaptor Context.
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    struct pcm*         pcmPtr;             ///< ALSA-intf handle
    struct pollfd       pfd[1];             ///< file descriptor used to monitor to alsa timer
    int                 start;              ///< ALSA driver is started
    GetSetFramesFunc_t  framesFunc;         ///< Upper layer (i.e. le_media) get/set frames callback
    ResultFunc_t        resultFunc;         ///< Upper layer (i.e. le_media) result callback
    void*               contextPtr;         ///< Upper layer (i.e. le_media) context
    le_thread_Ref_t     pcmThreadRef;       ///< Playback/Capture thread reference
    bool                isPlaybackRunning;  ///< True when playback is running, false otherwise
}
AlsaIntf_t;

//--------------------------------------------------------------------------------------------------
//                                       Static declarations
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * The memory pool for the Alsa interface
 */
//--------------------------------------------------------------------------------------------------
static le_mem_PoolRef_t AlsaIntfPool;

//--------------------------------------------------------------------------------------------------
// Data structures.
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Prototype for set ALSA parameters function.
 */
//--------------------------------------------------------------------------------------------------
typedef le_result_t (*SetPcmParamsFunc_t)(struct pcm *pcm);

//--------------------------------------------------------------------------------------------------
//                                       Static declarations
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to set "playback" internal PCM parameter for the Qualcomm alsa
 * driver
 *
 */
//--------------------------------------------------------------------------------------------------
static le_result_t SetPcmParamsPlayback
(
    struct pcm* pcmPtr
)
{
    struct snd_pcm_hw_params *params = NULL;
    struct snd_pcm_sw_params *sparams = NULL;
    uint32_t samplingRes;

    int channels = (pcmPtr->flags & PCM_MONO) ? 1 : ((pcmPtr->flags & PCM_5POINT1)? 6 : 2 );

    // can't use le_mem service, the memory is released in pcm_close()
    params = (struct snd_pcm_hw_params*) calloc(1, sizeof(struct snd_pcm_hw_params));

    if (!params)
    {
        LE_ERROR("Failed to allocate ALSA hardware parameters!");
        return LE_FAULT;
    }

    param_init(params);

    param_set_mask(params, SNDRV_PCM_HW_PARAM_ACCESS,
                   (pcmPtr->flags & PCM_MMAP)?
                   SNDRV_PCM_ACCESS_MMAP_INTERLEAVED :
                   SNDRV_PCM_ACCESS_RW_INTERLEAVED);

    param_set_mask(params, SNDRV_PCM_HW_PARAM_FORMAT, pcmPtr->format);
    param_set_mask(params, SNDRV_PCM_HW_PARAM_SUBFORMAT, SNDRV_PCM_SUBFORMAT_STD);

    param_set_min(params, SNDRV_PCM_HW_PARAM_PERIOD_TIME, 10);

    switch(pcmPtr->format)
    {
        case SNDRV_PCM_FORMAT_S8:
        samplingRes=8;
        break;

        case SNDRV_PCM_FORMAT_S16_LE:
        samplingRes=16;
        break;

        case SNDRV_PCM_FORMAT_S24_LE:
        samplingRes=24;
        break;

        case SNDRV_PCM_FORMAT_S32_LE:
        samplingRes=32;
        break;

        default:
        LE_ERROR("Unsupported sampling resolution (%d)!", pcmPtr->format);
        free(params);
        return LE_FAULT;
    }

    param_set_int(params, SNDRV_PCM_HW_PARAM_SAMPLE_BITS, samplingRes);
    param_set_int(params, SNDRV_PCM_HW_PARAM_FRAME_BITS,pcmPtr->channels * samplingRes);

    param_set_int(params, SNDRV_PCM_HW_PARAM_CHANNELS,pcmPtr->channels);
    param_set_int(params, SNDRV_PCM_HW_PARAM_RATE, pcmPtr->rate);

    if (param_set_hw_params(pcmPtr, params))
    {
        LE_ERROR("Cannot set hw params");
        free(params);
        return LE_FAULT;
    }

    pcmPtr->buffer_size = pcm_buffer_size(params);
    pcmPtr->period_size = pcm_period_size(params);
    pcmPtr->period_cnt = pcmPtr->buffer_size/pcmPtr->period_size;

    LE_DEBUG("buffer_size %d period_size %d period_cnt %d", pcmPtr->buffer_size, pcmPtr->period_size,
                                                            pcmPtr->period_cnt);

    // can't use le_mem service, the memory is released in pcm_close()
    sparams = (struct snd_pcm_sw_params*) calloc(1, sizeof(struct snd_pcm_sw_params));

    if (!sparams)
    {
        LE_ERROR("Failed to allocate ALSA software parameters!");
        free(params);
        return LE_FAULT;
    }

    // Get the current software parameters
    sparams->tstamp_mode = SNDRV_PCM_TSTAMP_NONE;
    sparams->period_step = 1;

    sparams->avail_min = pcmPtr->period_size/(channels * 2) ;
    sparams->start_threshold =  pcmPtr->period_size/(channels * 2) ;
    sparams->stop_threshold =  pcmPtr->buffer_size ;
    sparams->xfer_align =  pcmPtr->period_size/(channels * 2) ; /* needed for old kernels */

    sparams->silence_size = 0;
    sparams->silence_threshold = 0;

    if (param_set_sw_params(pcmPtr, sparams))
    {
        LE_ERROR("Cannot set sw params");
        free(params);
        free(sparams);
        return LE_FAULT;
    }

    // Do not free params and sparams here since the data is owned by
    // ALSA at this point; they will automatically be freed by
    // pcm_close().
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to set "capture" internal PCM parameter for the Qualcomm alsa driver
 *
 */
//--------------------------------------------------------------------------------------------------
static le_result_t SetPcmParamsCapture
(
    struct pcm* pcmPtr
)
{
    struct snd_pcm_hw_params *params = NULL;
    struct snd_pcm_sw_params *sparams = NULL;
    uint32_t samplingRes;

    // can't use le_mem service, the memory is released by pcm_close()
    params = (struct snd_pcm_hw_params*) calloc(1, sizeof(struct snd_pcm_hw_params));

    if (!params)
    {
        LE_ERROR("Failed to allocate ALSA hardware parameters!");
        return LE_FAULT;
    }

    param_init(params);

    param_set_mask(params, SNDRV_PCM_HW_PARAM_ACCESS,
                   (pcmPtr->flags & PCM_MMAP)?
                   SNDRV_PCM_ACCESS_MMAP_INTERLEAVED :
                   SNDRV_PCM_ACCESS_RW_INTERLEAVED);

    param_set_mask(params, SNDRV_PCM_HW_PARAM_FORMAT, pcmPtr->format);
    param_set_mask(params, SNDRV_PCM_HW_PARAM_SUBFORMAT, SNDRV_PCM_SUBFORMAT_STD);

    param_set_min(params, SNDRV_PCM_HW_PARAM_PERIOD_TIME, 10);

    switch(pcmPtr->format)
    {
        case SNDRV_PCM_FORMAT_S8:
        samplingRes=8;
        break;

        case SNDRV_PCM_FORMAT_S16_LE:
        samplingRes=16;
        break;

        case SNDRV_PCM_FORMAT_S24_LE:
        samplingRes=24;
        break;

        case SNDRV_PCM_FORMAT_S32_LE:
        samplingRes=32;
        break;

        default:
        LE_ERROR("Unsupported sampling resolution (%d)!", pcmPtr->format);
        free(params);
        return LE_FAULT;
    }

    param_set_int(params, SNDRV_PCM_HW_PARAM_SAMPLE_BITS, samplingRes);
    param_set_int(params, SNDRV_PCM_HW_PARAM_FRAME_BITS,pcmPtr->channels * samplingRes);

    param_set_int(params, SNDRV_PCM_HW_PARAM_CHANNELS, pcmPtr->channels);
    param_set_int(params, SNDRV_PCM_HW_PARAM_RATE, pcmPtr->rate);

    param_set_hw_refine(pcmPtr, params);

    if (param_set_hw_params(pcmPtr, params))
    {
        LE_ERROR("Cannot set hw params");
        free(params);
        return LE_FAULT;
    }

    pcmPtr->buffer_size = pcm_buffer_size(params);
    pcmPtr->period_size = pcm_period_size(params);
    pcmPtr->period_cnt = pcmPtr->buffer_size/pcmPtr->period_size;

    // can't use le_mem service, the memory is released in pcm_close()
    sparams = (struct snd_pcm_sw_params*) calloc(1, sizeof(struct snd_pcm_sw_params));

    if (!sparams)
    {
        LE_ERROR("Failed to allocate ALSA software parameters!");
        free(params);
        return LE_FAULT;
    }

    sparams->tstamp_mode = SNDRV_PCM_TSTAMP_NONE;
    sparams->period_step = 1;

    if (pcmPtr->flags & PCM_MONO)
    {
        sparams->avail_min = pcmPtr->period_size/2;
        sparams->xfer_align = pcmPtr->period_size/2;
    }
    else if (pcmPtr->flags & PCM_QUAD)
    {
        sparams->avail_min = pcmPtr->period_size/8;
        sparams->xfer_align = pcmPtr->period_size/8;
    }
    else if (pcmPtr->flags & PCM_5POINT1)
    {
        sparams->avail_min = pcmPtr->period_size/12;
        sparams->xfer_align = pcmPtr->period_size/12;
    }
    else
    {
        sparams->avail_min = pcmPtr->period_size/4;
        sparams->xfer_align = pcmPtr->period_size/4;
    }

    sparams->start_threshold = 1;
    sparams->stop_threshold = INT_MAX;
    sparams->silence_size = 0;
    sparams->silence_threshold = 0;

    if (param_set_sw_params(pcmPtr, sparams))
    {
        LE_ERROR("Cannot set sw params");
        free(params);
        free(sparams);
        return LE_FAULT;
    }

    // Do not free params and sparams here since the data is owned by
    // ALSA at this point; they will automatically be freed by
    // pcm_close().
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Init Alsa driver for pcm playback/capture.
 *
 */
//--------------------------------------------------------------------------------------------------
static le_result_t InitPcmPlaybackCapture
(
    pcm_Handle_t *pcmHandlePtr,
    const char* devicePtr,
    le_audio_SamplePcmConfig_t* pcmConfig,
    uint32_t flags,
    SetPcmParamsFunc_t paramsFunc
)
{
    uint32_t format;

    if (pcmConfig->channelsCount == 1)
    {
        flags |= PCM_MONO;
    }
    else if (pcmConfig->channelsCount == 2)
    {
        flags |= PCM_STEREO;
    }
    else if (pcmConfig->channelsCount == 4)
    {
        flags |= PCM_QUAD;
    }
    else if (pcmConfig->channelsCount == 6)
    {
        flags |= PCM_5POINT1;
    }
    else
    {
        flags |= PCM_MONO;
    }

    switch(pcmConfig->bitsPerSample)
    {
        case 8:
        format = SNDRV_PCM_FORMAT_S8;
        break;

        case 16:
        format = SNDRV_PCM_FORMAT_S16_LE;
        break;

        case 24:
        format = SNDRV_PCM_FORMAT_S24_LE;
        break;

        case 32:
        format = SNDRV_PCM_FORMAT_S32_LE;
        break;

        default:
        LE_ERROR("Unsupported sampling resolution (%d)!", pcmConfig->bitsPerSample);
        return LE_FAULT;
    }

    struct pcm *myPcmPtr = pcm_open(flags, (char*) devicePtr);

    if (myPcmPtr < 0)
    {
        return LE_FAULT;
    }

    AlsaIntf_t* alsaIntfPtr = le_mem_ForceAlloc(AlsaIntfPool);
    memset(alsaIntfPtr, 0, sizeof(AlsaIntf_t));
    alsaIntfPtr->pcmPtr = myPcmPtr;

    myPcmPtr->channels = pcmConfig->channelsCount;
    myPcmPtr->rate     = pcmConfig->sampleRate;
    myPcmPtr->flags    = flags;
    myPcmPtr->format   = format;

    if (pcm_ready(myPcmPtr))
    {
        myPcmPtr->channels = pcmConfig->channelsCount;
        myPcmPtr->rate     = pcmConfig->sampleRate;
        myPcmPtr->flags    = flags;
        myPcmPtr->format   = format;

        if (paramsFunc(myPcmPtr)==LE_OK)
        {
            // MMAP
            if (flags & PCM_MMAP)
            {
                if (mmap_buffer(myPcmPtr))
                {
                     LE_ERROR("mmap_buffer failed");
                     pcm_close(myPcmPtr);
                     le_mem_Release(alsaIntfPtr);
                     return LE_FAULT;
                }

                if (pcm_prepare(myPcmPtr))
                {
                    LE_ERROR("Failed in pcm_prepare");
                    pcm_close(myPcmPtr);
                    le_mem_Release(alsaIntfPtr);
                    return LE_FAULT;
                }

                alsaIntfPtr->pfd[0].fd = myPcmPtr->fd;
                alsaIntfPtr->pfd[0].events = POLLOUT;
            }
            // NMMAP
            else
            {
                if (pcm_prepare(myPcmPtr))
                {
                    LE_ERROR("Failed in pcm_prepare");
                    pcm_close(myPcmPtr);
                    le_mem_Release(alsaIntfPtr);
                    return LE_FAULT;
                }
            }
        }
        else
        {
            LE_ERROR("Failed in set_params");
            le_mem_Release(alsaIntfPtr);
            return LE_FAULT;
        }
    }
    else
    {
        LE_ERROR("PCM is not ready (pcm error: %s)", pcm_error(myPcmPtr));
        le_mem_Release(alsaIntfPtr);
        return LE_FAULT;
    }

    *pcmHandlePtr = (pcm_Handle_t) alsaIntfPtr;

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Capture thread
 *
 */
//--------------------------------------------------------------------------------------------------
static void* CaptureThread
(
    void* contextPtr
)
{
    AlsaIntf_t* alsaIntfPtr = (AlsaIntf_t*) contextPtr;
    struct pcm *pcm = (struct pcm *) alsaIntfPtr->pcmPtr;
    uint32_t bufsize;
    le_result_t res = LE_OK;

    bufsize = pcm->period_size;

    uint8_t data[bufsize];
    memset(data,0,bufsize);

    while (res == LE_OK)
    {
        int myErrno;

        // Start acquisition
        if ( (myErrno = pcm_read(pcm, data, bufsize)) != 0 )
        {
            LE_ERROR("Could not read %d void bytes! errno %d", bufsize, -myErrno);
            res = LE_FAULT;
        }
        else
        {
            if ( !alsaIntfPtr->framesFunc ||
                 ( alsaIntfPtr->framesFunc ( data,
                                             &bufsize,
                                             alsaIntfPtr->contextPtr ) != LE_OK ) )
            {
                res = LE_FAULT;
            }
        }
    }

    if (alsaIntfPtr->resultFunc)
    {
        LE_DEBUG("Record end, res = %d", res);
        alsaIntfPtr->resultFunc(res, alsaIntfPtr->contextPtr);
    }

    le_event_RunLoop();
    // Should never happened
    return NULL;
}



//--------------------------------------------------------------------------------------------------
/**
 * Run playback: send PCM samples to ALSA until all are sent
 *
 */
//--------------------------------------------------------------------------------------------------
static le_result_t RunPlayback
(
    AlsaIntf_t* alsaIntfPtr
)
{
    struct pcm *pcm = (struct pcm *) alsaIntfPtr->pcmPtr;
    int err;
    long avail;
    int nfds = 1;
    uint8_t* bufferPtr = NULL;
    uint32_t bufLen=0;
    le_result_t res = LE_UNAVAILABLE;
    int myErrno;

    // loop until all PCM frames have been sent
    for(;;)
    {
        if (!pcm->running)
        {
            if (pcm_prepare(pcm))
            {
                return LE_FAULT;
            }

            pcm->running = 1;
            alsaIntfPtr->start = 0;
        }

        // Sync the current Application pointer from the kernel
        pcm->sync_ptr->flags = SNDRV_PCM_SYNC_PTR_APPL | SNDRV_PCM_SYNC_PTR_AVAIL_MIN;

        err = sync_ptr(pcm);
        if (err == EPIPE)
        {
            LE_ERROR("Failed in sync_ptr");
            // We failed to make our window -- try to restart
            pcm->underruns++;
            pcm->running = 0;
            continue;
        }

        // Check for the available buffer in driver. If available buffer is less than
        // avail_min we need to wait
        avail = pcm_avail(pcm);

        if (avail < 0)
        {
            return LE_FAULT;
        }

        // If no space left in the driver, wait the timeout of snd timer
        if (avail < pcm->sw_p->avail_min)
        {
            err = poll(alsaIntfPtr->pfd, nfds, TIMEOUT_INFINITE);
            if (err < 0)
            {
                LE_ERROR("Failed in poll: %m");
                return LE_FAULT;
            }
            if (alsaIntfPtr->pfd[0].revents & POLLERR)
            {
                LE_ERROR("Event POLLERR returned by poll");
                return LE_IO_ERROR;
            }
            continue;
        }

        // Now that we have buffer size greater than avail_min available to to be written we
        // need to calcutate the buffer offset where we can start writting.

        bufLen = pcm->period_size;
        bufferPtr = dst_address(pcm);

        memset(bufferPtr, 0, bufLen);

        // Get the PCM frames (from upper layer) to be sent to ALSA
        if ( alsaIntfPtr->framesFunc )
        {
            if ( alsaIntfPtr->framesFunc ( bufferPtr,
                                           &bufLen,
                                           alsaIntfPtr->contextPtr ) != LE_OK )
            {
                return LE_FAULT;
            }
        }
        else
        {
            return LE_FAULT;
        }

        // All frames have been sent => playback ended
        if (bufLen == 0)
        {
            if (!alsaIntfPtr->isPlaybackRunning)
            {
                res = LE_UNDERFLOW;
            }
            else
            {
                alsaIntfPtr->isPlaybackRunning = false;
                res = LE_OK;
            }
        }
        else
        {
            alsaIntfPtr->isPlaybackRunning = true;
            res = LE_OK;
        }

        if ( (myErrno = pcm_write(pcm, bufferPtr, pcm->period_size)) != 0 )
        {
            LE_ERROR("Could not write, errno %d", -myErrno);
            res = LE_FAULT;
        }

        // End reached => exit the loop
        if (!alsaIntfPtr->isPlaybackRunning)
        {
            break;
        }
    }

    return res;
}

//--------------------------------------------------------------------------------------------------
/**
 * Playback thread
 *
 */
//--------------------------------------------------------------------------------------------------
static void* PlaybackThread
(
    void* contextPtr
)
{
    AlsaIntf_t* alsaIntfPtr = (AlsaIntf_t*) contextPtr;
    struct pcm *pcmPtr = (struct pcm *) alsaIntfPtr->pcmPtr;
    le_result_t res = LE_OK;

    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    if (pcmPtr->flags & PCM_MMAP)
    {
        alsaIntfPtr->isPlaybackRunning = true;
        // run in loop if no error, until the thread is killed
        // This loop is useful for the play samples use case.
        while (res != LE_FAULT)
        {
            res = RunPlayback(alsaIntfPtr);

            if (alsaIntfPtr->resultFunc)
            {
                LE_DEBUG("Play end, res = %d", res);
                alsaIntfPtr->resultFunc(res, alsaIntfPtr->contextPtr);
            }
        }
    }

    le_event_RunLoop();

    return NULL;
}

//--------------------------------------------------------------------------------------------------
//                                       Public declarations
//--------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------
/**
 * Start the playback.
 * The function is asynchronous: it starts the playback thread, then returns.
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_pcm_Play
(
    pcm_Handle_t pcmHandle                  ///< [IN] Handle of the audio resource given by
                                            ///< pa_pcm_InitPlayback() / pa_pcm_InitCapture()
                                            ///< initialization functions
)
{
    AlsaIntf_t* alsaIntfPtr = (AlsaIntf_t*) pcmHandle;
    char threadName[PCM_THREAD_NAME_LEN]="\0";
    static int counter = 0;

    snprintf(threadName, PCM_THREAD_NAME_LEN, "Playback-%d", counter);
    counter++;

    alsaIntfPtr->pcmThreadRef = le_thread_Create(threadName,
                                                PlaybackThread,
                                                alsaIntfPtr);

    le_thread_SetPriority(alsaIntfPtr->pcmThreadRef, LE_THREAD_PRIORITY_RT_2);

    le_thread_SetJoinable(alsaIntfPtr->pcmThreadRef);

    le_thread_Start(alsaIntfPtr->pcmThreadRef);

    return LE_OK;

}

//--------------------------------------------------------------------------------------------------
/**
 * Start the recording.
 * The function is asynchronous: it starts the recording thread, then returns.
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_pcm_Capture
(
    pcm_Handle_t pcmHandle                 ///< [IN] Handle of the audio resource given by
                                            ///< pa_pcm_InitPlayback() / pa_pcm_InitCapture()
                                            ///< initialization functions
)
{
    AlsaIntf_t* alsaIntfPtr = (AlsaIntf_t*) pcmHandle;
    char threadName[PCM_THREAD_NAME_LEN]="\0";
    static int counter = 0;

    snprintf(threadName, PCM_THREAD_NAME_LEN, "AudioCapture-%d", counter);
    counter++;

    alsaIntfPtr->pcmThreadRef = le_thread_Create(threadName,
                                            CaptureThread,
                                            alsaIntfPtr);

    le_thread_SetJoinable(alsaIntfPtr->pcmThreadRef);

    le_thread_Start(alsaIntfPtr->pcmThreadRef);

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the callbacks called during a playback/recording to:
 * - get/set PCM frames thanks to getFramesFunc callback: this callback will be called by the pa_pcm
 * to get the next PCM frames to play (playback case), or to send back PCM frames to record
 * (recording case).
 * - get the playback/recording result thanks to setResultFunc callback: this callback will be
 * called by the PA_PCM to inform the caller about the status of the playback or the recording.
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_pcm_SetCallbackHandlers
(
    pcm_Handle_t pcmHandle,
    GetSetFramesFunc_t framesFunc,
    ResultFunc_t resultFunc,
    void* contextPtr
)
{
    AlsaIntf_t* alsaIntfPtr = (AlsaIntf_t*) pcmHandle;

    alsaIntfPtr->framesFunc = framesFunc;
    alsaIntfPtr->resultFunc = resultFunc;
    alsaIntfPtr->contextPtr = contextPtr;

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Close sound driver.
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_pcm_Close
(
    pcm_Handle_t pcmHandle                 ///< [IN] Handle of the audio resource given by
                                           ///< pa_pcm_InitPlayback() / pa_pcm_InitCapture()
                                           ///< initialization functions
)
{
    AlsaIntf_t* alsaIntfPtr = (AlsaIntf_t*) pcmHandle;

    if (pcmHandle)
    {
        le_thread_Ref_t pcmThreadRef = alsaIntfPtr->pcmThreadRef;

        if (pcmThreadRef)
        {
            le_thread_Cancel(pcmThreadRef);
            le_thread_Join(pcmThreadRef, NULL);

            LE_DEBUG("Close ALSA");
            if (pcm_close(alsaIntfPtr->pcmPtr) == 0)
            {
                LE_DEBUG("end PCM thread");
                le_mem_Release(alsaIntfPtr);
            }
            else
            {
                LE_ERROR("Error during pcm close");
                return LE_FAULT;
            }
        }
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the period Size from sound driver.
 *
 */
//--------------------------------------------------------------------------------------------------
uint32_t pa_pcm_GetPeriodSize
(
    pcm_Handle_t pcmHandle                  ///< [IN] Handle of the audio resource given by
                                            ///< pa_pcm_InitPlayback() / pa_pcm_InitCapture()
                                            ///< initialization functions
)
{
    AlsaIntf_t* alsaIntfPtr = (AlsaIntf_t*) pcmHandle;
    struct pcm *pcm = (struct pcm *) alsaIntfPtr->pcmPtr;
    return pcm->period_size;
}

//--------------------------------------------------------------------------------------------------
/**
 * Initialize sound driver for PCM capture.
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_pcm_InitCapture
(
    pcm_Handle_t *pcmHandlePtr,             ///< [OUT] Handle of the audio resource, to be used on
                                            ///< further PA PCM functions call
    char* devicePtr,                        ///< [IN] Device to be initialized
    le_audio_SamplePcmConfig_t* pcmConfig   ///< [IN] Samples PCM configuration
)
{
    uint32_t flags = PCM_NMMAP | PCM_IN;

    return InitPcmPlaybackCapture( pcmHandlePtr,
                                   devicePtr,
                                   pcmConfig,
                                   flags,
                                   SetPcmParamsCapture );
}

//--------------------------------------------------------------------------------------------------
/**
 * Initialize sound driver for PCM playback.
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_pcm_InitPlayback
(
    pcm_Handle_t *pcmHandlePtr,             ///< [OUT] Handle of the audio resource, to be used on
                                            ///< further PA PCM functions call
    char* devicePtr,                        ///< [IN] Device to be initialized
    le_audio_SamplePcmConfig_t* pcmConfig   ///< [IN] Samples PCM configuration
)
{
    uint32_t flags = PCM_MMAP | PCM_OUT;

    return InitPcmPlaybackCapture( pcmHandlePtr,
                                   devicePtr,
                                   pcmConfig,
                                   flags,
                                   SetPcmParamsPlayback );
}

//--------------------------------------------------------------------------------------------------
/**
 * Component initializer.  Called automatically by the application framework at process start.
 */
//--------------------------------------------------------------------------------------------------
COMPONENT_INIT
{
    // Allocate the Media thread context pool.
    AlsaIntfPool = le_mem_CreatePool("AlsaIntfPool", sizeof(AlsaIntf_t));

    le_mem_ExpandPool(AlsaIntfPool, 2);
}
