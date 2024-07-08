/*
Copyright 2024 Pawikan Boonnaum

This file is part of naturau-base/ntrb.

naturau-base/ntrb is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

naturau-base/ntrb is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with naturau-base/ntrb. If not, see <https://www.gnu.org/licenses/>.
*/

#include "aud_std_fmt.h"

#include "AudioDatapoints.h"
#include "AudioHeader.h"
#include "SpanU8.h"

#include "wav_wrapper.h"
#include "file_wrapper.h"

#include "utils.h"
#include "alloc.h"
#include "str_utils.h"

#include "portaudio.h"

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

///Standard sample rate used in audio loop: 48000 hz.
const uint32_t ntrb_std_samplerate = 48000;
///Standard audio channel count used in audio loop: 2.
const uint8_t ntrb_std_audchannels = 2;
///Standard sample format used in audio loop: paFloat32.
const PaSampleFormat ntrb_std_sample_fmt = paFloat32;

//Can accept any sample rate and any channels, just required the input to be an int16 AudioDatapoints.
ntrb_AudioDatapoints ntrb_AudioDatapoints_i16_to_f32(const ntrb_AudioDatapoints int16){
	const size_t int16_count = int16.byte_count / sizeof(int16_t);
	ntrb_AudioDatapoints float32 = ntrb_AudioDatapoints_new(int16_count * sizeof(float));
	if(float32.bytes == NULL) return failed_ntrb_AudioDatapoints;

	for(size_t i = 0; i < int16_count; i++){
		((float*)(float32.bytes))[i] = (float)(((int16_t*)(int16.bytes))[i]) / (float)INT16_MAX;
	}
	
	return float32;
}

//requires the sample format to be float32 and multi-channeled audio encoded in a consistent interleaved fashion.
//returns every offset(th) element as a mono channel.
//For example, 2 channels: 0 offset is left, 1 is right. For other channel encoding, it depends.
ntrb_AudioDatapoints ntrb_split_as_mono(const ntrb_AudioDatapoints multichannel_aud, const size_t channels, const size_t offset){
	//checks that the input audio has no trailing channels. 
	//For example, stereo should not have 1 trailing datapoint, or 4 channel having 1,2,3 trailing datapoints.
	//This means the data is incomplete, and we chose to return an error.
	const bool unaligned_float_count = (multichannel_aud.byte_count % channels) != 0;
	if(unaligned_float_count) return failed_ntrb_AudioDatapoints;
	
	const size_t multichannel_aud_float_count = (multichannel_aud.byte_count / sizeof(float));
	
	const size_t mono_float_count = multichannel_aud_float_count / channels;
	ntrb_AudioDatapoints mono = ntrb_AudioDatapoints_new(mono_float_count * sizeof(float));
	if(mono.bytes == NULL) return failed_ntrb_AudioDatapoints;
	
	for(size_t i = 0; i < mono_float_count; i++){
		((float*)(mono.bytes))[i] = ((float*)(multichannel_aud.bytes))[(i*channels) + offset];
	}
	return mono;
}

//requires the audio to be in float32 and mono channeled.
ntrb_AudioDatapoints ntrb_mono_to_xchannels(const ntrb_AudioDatapoints orig, const size_t dest_channels){
	ntrb_AudioDatapoints multichannel_aud = ntrb_AudioDatapoints_new(orig.byte_count * dest_channels);
	if(multichannel_aud.bytes == NULL) return failed_ntrb_AudioDatapoints;
	
	const size_t float_points = orig.byte_count / sizeof(float);
	
	//loop order can be swapped for cache friendliness.
	for(size_t channel = 0; channel < dest_channels; channel++){		
		for(size_t i_orig = 0; i_orig < float_points; i_orig++){
			((float*)(multichannel_aud.bytes))[(i_orig*dest_channels) +channel] = ((float*)(orig.bytes))[i_orig];
		}
	}
	
	return multichannel_aud;
}

//requires 2 float32 AudioDatpoints with equal length to merge as an interleaved stereo AudioDatapoints.
ntrb_AudioDatapoints ntrb_merge_to_stereo(const ntrb_AudioDatapoints l_ch, const ntrb_AudioDatapoints r_ch){
	if(l_ch.byte_count != r_ch.byte_count) return failed_ntrb_AudioDatapoints;
	
	const size_t mono_float_count = r_ch.byte_count / sizeof(float);
	const size_t stereo_float_count = mono_float_count * 2;
	
	ntrb_AudioDatapoints stereo_aud = ntrb_AudioDatapoints_new(stereo_float_count * sizeof(float));
	if(stereo_aud.bytes == NULL) return failed_ntrb_AudioDatapoints;
	
	for(size_t i = 0; i < stereo_float_count; i+=2){
		((float*)(stereo_aud.bytes))[i] = ((float*)(l_ch.bytes))[i/2];
		((float*)(stereo_aud.bytes))[i+1] = ((float*)(r_ch.bytes))[i/2];
	}
	
	return stereo_aud;
}


//requires input to be in mono channeled float32
ntrb_AudioDatapoints ntrb_to_samplerate_mono(const ntrb_AudioDatapoints orig, const double orig_samplerate, const double dest_samplerate){
	const size_t orig_float_count = orig.byte_count / sizeof(float);
	
	size_t dest_float_count = floor((float)orig_float_count * (dest_samplerate / orig_samplerate));	
	ntrb_AudioDatapoints dest = ntrb_AudioDatapoints_new(dest_float_count * sizeof(float));
	if(dest.bytes == NULL) return failed_ntrb_AudioDatapoints;
	
	const double dest_over_orig_samplerate = (dest_samplerate - 1.0) / (orig_samplerate - 1.0);
	const double orig_over_dest_samplerate = 1.0f / dest_over_orig_samplerate;	
	
	//predicts unaligned samplerate conversions linearly, instead of flooring the index, or other ways.
	for(size_t i_dest = 0; i_dest < dest_float_count; i_dest++){
		const double i_orig = (double)i_dest * (double)orig_over_dest_samplerate;
		const double i_orig_floor = ntrb_clamp_float(floor(i_orig), 0, orig_float_count - 1);
		const double i_orig_ceil = ntrb_clamp_float(ceil(i_orig), 0, orig_float_count - 1);
		
		const float fp_floor = ((float*)(orig.bytes))[(size_t)i_orig_floor];
		const float fp_ceil  = ((float*)(orig.bytes))[(size_t)i_orig_ceil];
		
		((float*)(dest.bytes))[i_dest] = fp_floor + ((i_orig - i_orig_floor) * (fp_ceil - fp_floor));
	}
	
	return dest;
}

//requires the input to be in float32 and stereo.
//If orig and dest samplerate are equal, copies orig to a new allocated AudioDatapoints.
//If not, interpolates the dest datapoints from orig.
ntrb_AudioDatapoints ntrb_to_samplerate(const ntrb_AudioDatapoints orig, const uint32_t orig_samplerate, const uint32_t dest_samplerate){
	if(orig_samplerate == dest_samplerate) return ntrb_AudioDatapoints_copy(orig);
		
	ntrb_AudioDatapoints dest_aud = failed_ntrb_AudioDatapoints;
	const size_t stereo_channels = 2;
	const size_t left_channel_offset = 0;
	const size_t right_channel_offset = 1;
	
	ntrb_AudioDatapoints orig_l = ntrb_split_as_mono(orig, stereo_channels, left_channel_offset);
	ntrb_AudioDatapoints dest_l = ntrb_to_samplerate_mono(orig_l, (double)orig_samplerate, (double)dest_samplerate);
	if(orig_l.bytes == NULL || dest_l.bytes == NULL){
		ntrb_AudioDatapoints_free(&orig_l);
		ntrb_AudioDatapoints_free(&dest_l);
	}
	ntrb_AudioDatapoints_free(&orig_l);	
		
	ntrb_AudioDatapoints orig_r = ntrb_split_as_mono(orig, stereo_channels, right_channel_offset);
	ntrb_AudioDatapoints dest_r = ntrb_to_samplerate_mono(orig_r, (double)orig_samplerate, (double)dest_samplerate);
	if(orig_r.bytes == NULL || dest_r.bytes == NULL){
		ntrb_AudioDatapoints_free(&dest_l);	
		ntrb_AudioDatapoints_free(&orig_r);
		ntrb_AudioDatapoints_free(&dest_r);
	}
	ntrb_AudioDatapoints_free(&orig_r);
	
	dest_aud = ntrb_merge_to_stereo(dest_l, dest_r);
	if(dest_aud.bytes == NULL) dest_aud = failed_ntrb_AudioDatapoints;
	
	ntrb_AudioDatapoints_free(&dest_l);
	ntrb_AudioDatapoints_free(&dest_r);	
	
	return dest_aud;
}

//input could be in any sample format, any channels, any samplerates.
enum ntrb_StdAudFmtConversionResult ntrb_to_standard_format(ntrb_AudioDatapoints* const ret_aud, const ntrb_AudioDatapoints orig, const ntrb_AudioHeader* const orig_header){
	ntrb_AudioDatapoints prev = orig;
	ntrb_AudioDatapoints current;

	if(orig_header->AudioFormat != paFloat32)
		current = ntrb_AudioDatapoints_i16_to_f32(orig);
	else
		current = ntrb_AudioDatapoints_copy(orig);
	
	if(current.bytes == NULL)
		return ntrb_StdAudFmtConversion_AllocFailure;
	
	if(orig_header->NumChannels == 1){
		if(prev.bytes != orig.bytes) ntrb_AudioDatapoints_free(&prev);		
		
		prev = current;
		current = ntrb_mono_to_xchannels(prev, ntrb_std_audchannels);
		if(current.bytes == NULL){
			if(prev.bytes != orig.bytes) ntrb_AudioDatapoints_free(&prev);
			return ntrb_StdAudFmtConversion_AllocFailure;
		}
	}
	
	if(orig_header->SampleRate != ntrb_std_samplerate){
		if(prev.bytes != orig.bytes) ntrb_AudioDatapoints_free(&prev);
		prev = current;
		current = ntrb_to_samplerate(prev, orig_header->SampleRate, ntrb_std_samplerate);
		if(current.bytes == NULL){
			if(prev.bytes != orig.bytes) ntrb_AudioDatapoints_free(&prev);
			return ntrb_StdAudFmtConversion_AllocFailure;
		}
	}

	if(prev.bytes != orig.bytes) ntrb_AudioDatapoints_free(&prev);	
	*ret_aud = current;
	
	return ntrb_StdAudFmtConversion_OK;
}