/**
 * @file pa_audio_local.h
 *
 * Control and configuration implementation of pa_audio API.
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#ifndef LEGATO_PAAUDIOLOCAL_INCLUDE_GUARD
#define LEGATO_PAAUDIOLOCAL_INCLUDE_GUARD

//--------------------------------------------------------------------------------------------------
/**
 * PA private parameters.
 *
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    int32_t                    gain;                // audio interface gain value for unmute
    media_routing_Parameters_t mediaRoutingParams;  // routing PA parameters.
}
PaParameters_t;

//--------------------------------------------------------------------------------------------------
/**
 * Get the private PA parameters of a stream
 *
 */
//--------------------------------------------------------------------------------------------------
PaParameters_t* GetPaParams
(
    le_audio_Stream_t* streamPtr
);

#endif // LEGATO_PAAUDIOLOCAL_INCLUDE_GUARD
