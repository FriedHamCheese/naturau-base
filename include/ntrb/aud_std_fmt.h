#ifndef ntrb_aud_std_fmt_h
#define ntrb_aud_std_fmt_h

/*
Copyright 2024 Pawikan Boonnaum

This file is part of naturau-base/ntrb.

naturau-base/ntrb is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

naturau-base/ntrb is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with naturau-base/ntrb. If not, see <https://www.gnu.org/licenses/>.
*/


/**
\file aud_std_fmt.h
A module providing functions converting unprocessed audio to the standard audio format used in ntrb's audio loop.
And functions for loading audio and processing it.
*/

#include "AudioDatapoints.h"
#include "AudioHeader.h"

#include "portaudio.h"

#include <stdio.h>
#include <stdint.h>

extern const uint32_t ntrb_std_samplerate;
extern const uint8_t ntrb_std_audchannels;
extern const PaSampleFormat ntrb_std_sample_fmt;

/**
An enum for errors while converting audio to ntrb audio loop standard audio format.
*/
enum ntrb_StdAudFmtConversionResult{
	ntrb_StdAudFmtConversion_OK,					///< Successfully converted the audio
	ntrb_StdAudFmtConversion_AllocFailure,			///< Failed to allocate memory
	ntrb_StdAudFmtConversion_UnknownSampleFormat,	///< Could not convert from the original format
	ntrb_StdAudFmtConversion_UnsupportedChannels,	///< Could not convert from that amount of channels
};

#ifdef __cplusplus
extern "C"{	
#endif

/**
Separately allocates the float32 conversion from the provided int16 argument.

The size of the output would be 2x of the int16.

If the function failed to allocate memory for the return container, it returns failed_ntrb_AudioDatapoints.
*/
ntrb_AudioDatapoints ntrb_AudioDatapoints_i16_to_f32(const ntrb_AudioDatapoints int16);

/**
Copies the (offset) channel of the datapoints, and separately allocate ntrb_AudioDatapoints for containing it.

The multichannel_aud must be encoded in float32 and has at least 2 channels.

\param multichannel_aud An ntrb_AudioDatapoints which contains multiple audio channels interleaved in a frame.
  For example, frame 0: 0ch 1ch 2ch, frame 1: 0ch 1ch 2ch, etc...
\param channels The amount of audio channels which multichannel_aud contains.
\param offset The channel which you choose to split off of multichannel_aud.
  Must be a value between [0, channels - 1]. For stereo, 0 means left and 1 means right.
  
If the function failed to allocate memory for the return container, it returns failed_ntrb_AudioDatapoints.
*/
ntrb_AudioDatapoints ntrb_split_as_mono(const ntrb_AudioDatapoints multichannel_aud, const size_t channels, const size_t offset);

/**
Converts a mono channeled ntrb_AudioDatapoints to one with (dest_channels) and allocates the return ntrb_AudioDatapoints separately for containing it.

orig must be encoded in float32 format and has only 1 channel.

It copies each of the frame in orig next to each other by (dest_channels - 1) copy.
So the frame of the output has (dest_channels)x copy of the same datapoint.

For example, if dest_channels is 3:

orig frame 0: 0ch, orig frame 1: 0ch, orig frame 2: 0ch -> dest frame 0: 0ch 1ch 2ch, dest frame 1: 0ch 1ch 2ch, dest frame 2: 0ch 1ch 2ch.

Where 0ch 1ch and 2ch of each frame is the same value.

If the function failed to allocate memory for the return container, it returns failed_ntrb_AudioDatapoints.
*/
ntrb_AudioDatapoints ntrb_mono_to_xchannels(const ntrb_AudioDatapoints orig, const size_t dest_channels);

/**
Creates an ntrb_Audiodatapoint with each of the float32 datapoints of l_ch and r_ch interleaved.

For example: frame 0: 0l, 0r; frame 1: 1l, 1r, frame 2: 2l, 2r, etc...

If l_ch.elem is not equal to r_ch.elem or it failed to allocate the return container, the function returns failed_ntrb_AudioDatapoints.

*/
ntrb_AudioDatapoints ntrb_merge_to_stereo(const ntrb_AudioDatapoints l_ch, const ntrb_AudioDatapoints r_ch);

/**
Converts a mono-channel float32 orig with orig_samplerate to an ntrb_AudioDatapoints with dest_samplerate. The return container is allocated separately.

This only is used for converting mono to mono, see ntrb_to_samplerate() for one which does stereo.

If the function failed to allocate the return container, it returns failed_ntrb_AudioDatapoints.
*/
ntrb_AudioDatapoints ntrb_to_samplerate_mono(const ntrb_AudioDatapoints orig, const double orig_samplerate, const double dest_samplerate);

/**
Converts a stereo float32 orig with orig_samplerate to an ntrb_AudioDatapoints with dest_samplerate. The return container is allocated separately.

If the function failed to allocate the return container, it returns failed_ntrb_AudioDatapoints.
*/
ntrb_AudioDatapoints ntrb_to_samplerate(const ntrb_AudioDatapoints orig, const uint32_t orig_samplerate, const uint32_t dest_samplerate);

/**
Returns a separately allocated ret_aud which is either a clone of orig or a float32 converted orig, if orig is float32 or int16 respectively.

- If an ntrb_StdAudFmtConversion_AllocFailure is returned, ret_aud couldn't be allocated.
- If an ntrb_StdAudFmtConversion_UnknownSampleFormat is returned, originalFormat is not supported.
*/
enum ntrb_StdAudFmtConversionResult ntrb_to_std_sample_fmt(ntrb_AudioDatapoints* const ret_aud, const ntrb_AudioDatapoints orig, const PaSampleFormat originalFormat);

/**
Returns a separately allocated ret_aud which is either a clone of orig or a stereo converted orig, if orig is stereo or mono respectively.

orig must be in float32 format.

- If an ntrb_StdAudFmtConversion_AllocFailure is returned, ret_aud couldn't be allocated.
- If an ntrb_StdAudFmtConversion_UnsupportedChannels is returned, audchannels is not supported.

\param[out] ret_aud Uninitialised ntrb_AudioDatapoints for output.
\param[in] orig
\param[in] audchannels Audio channel count of orig.
*/
enum ntrb_StdAudFmtConversionResult ntrb_to_std_aud_channels(ntrb_AudioDatapoints* const ret_aud, const ntrb_AudioDatapoints orig, const uint8_t audchannels);

/**
Returns a separately allocated ret_aud which is either a copy of orig or one converted to 48khz sample rate.

orig must be in stereo channeled float32.

If an ntrb_StdAudFmtConversion_AllocFailure is returned, ret_aud couldn't be allocated.

\param[out] ret_aud Uninitialised ntrb_AudioDatapoints for output.
\param[in] orig
\param[in] samplerate Sample rate of orig.
*/
enum ntrb_StdAudFmtConversionResult ntrb_to_std_samplerate(ntrb_AudioDatapoints* const ret_aud, const ntrb_AudioDatapoints orig, const uint32_t samplerate);

/**
Returns a separately allocated ret_aud, which is orig converted to float32 stereo 48khz format.

\param[out] ret_aud Uninitialised ntrb_AudioDatapoints for output.
\param[in] orig
\param[in] orig_header The header which comes with orig when reading an audio file.
*/
enum ntrb_StdAudFmtConversionResult ntrb_to_standard_format(ntrb_AudioDatapoints* const ret_aud, const ntrb_AudioDatapoints orig, const ntrb_AudioHeader* const orig_header);

#ifdef __cplusplus
};
#endif

#endif