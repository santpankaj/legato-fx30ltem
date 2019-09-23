/**
 * @file media_routing.h
 *
 * Header for media routing PA local definitions
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#ifndef LEGATO_MEDIAROUTING_INCLUDE_GUARD
#define LEGATO_MEDIAROUTING_INCLUDE_GUARD

//--------------------------------------------------------------------------------------------------
/**
 * Media Routing PA private parameters.
 *
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    bool            closeAllowed;           // close allowed by swisync
    uint32_t        multimediaUsageCount;   // stream usage counter in a multimedia purpose
}
media_routing_Parameters_t;

//--------------------------------------------------------------------------------------------------
/**
 * This function set the media routing PA private parameters
 *
 */
//--------------------------------------------------------------------------------------------------
void media_routing_SetPaParams
(
    media_routing_Parameters_t *paMediaRoutingParamsPtr
);

//--------------------------------------------------------------------------------------------------
/**
 * This function initialize the media routing PA
 *
 */
//--------------------------------------------------------------------------------------------------
void media_routing_Initialize
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to reset the DSP Audio path
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t media_routing_SetDspAudioPath
(
    le_audio_Stream_t* inputStreamPtr,   ///< [IN] input audio stream
    le_audio_Stream_t* outputStreamPtr   ///< [IN] output audio stream
);

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
);

#endif // LEGATO_MEDIAROUTING_INCLUDE_GUARD
