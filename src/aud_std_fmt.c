#include "aud_std_fmt.h"

#include "AudioDatapoints.h"
#include "AudioHeader.h"
#include "SpanU8.h"

#include "wav_wrapper.h"
#include "file_wrapper.h"

#include "utils.h"

#include "portaudio.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>


/*
TODO:
	Standardised enum for error handling of conversion functions.
\*/


const uint32_t ntrb_std_samplerate = 48000;
const uint8_t ntrb_std_audchannels = 2;
const PaSampleFormat ntrb_std_sample_fmt = paFloat32;

//Can accept any sample rate and any channels, just required the input to be an int16 AudioDatapoints.
ntrb_AudioDatapoints ntrb_AudioDatapoints_i16_to_f32(const ntrb_AudioDatapoints int16){
	const size_t int16_count = int16.byte_count / sizeof(int16_t);
	ntrb_AudioDatapoints float32 = new_ntrb_AudioDatapoints(int16_count * sizeof(float));
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
	ntrb_AudioDatapoints mono = new_ntrb_AudioDatapoints(mono_float_count * sizeof(float));
	if(mono.bytes == NULL) return failed_ntrb_AudioDatapoints;
	
	for(size_t i = 0; i < mono_float_count; i++){
		((float*)(mono.bytes))[i] = ((float*)(multichannel_aud.bytes))[(i*channels) + offset];
	}
	return mono;
}

//requires input to be in mono channeled float32
ntrb_AudioDatapoints ntrb_to_samplerate_mono(const ntrb_AudioDatapoints orig, const double orig_samplerate, const double dest_samplerate){
	const size_t orig_float_count = orig.byte_count / sizeof(float);
	
	size_t dest_float_count = floor((float)orig_float_count * (dest_samplerate / orig_samplerate));	
	ntrb_AudioDatapoints dest = new_ntrb_AudioDatapoints(dest_float_count * sizeof(float));
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
	if(orig_samplerate == dest_samplerate) return copy_ntrb_AudioDatapoints(orig);
	
	const double dest_over_orig_samplerate = (double)dest_samplerate / (double)orig_samplerate;
	
	ntrb_AudioDatapoints dest_aud = failed_ntrb_AudioDatapoints;
	const size_t stereo_channels = 2;
	const size_t left_channel_offset = 0;
	const size_t right_channel_offset = 1;
	
	ntrb_AudioDatapoints orig_l = ntrb_split_as_mono(orig, stereo_channels, left_channel_offset);
	ntrb_AudioDatapoints dest_l = ntrb_to_samplerate_mono(orig_l, (double)orig_samplerate, (double)dest_samplerate);
	if(orig_l.bytes == NULL || dest_l.bytes == NULL){
		free(orig_l.bytes);
		free(dest_l.bytes);
	}
	free(orig_l.bytes);	
		
	ntrb_AudioDatapoints orig_r = ntrb_split_as_mono(orig, stereo_channels, right_channel_offset);
	ntrb_AudioDatapoints dest_r = ntrb_to_samplerate_mono(orig_r, (double)orig_samplerate, (double)dest_samplerate);
	if(orig_r.bytes == NULL || dest_r.bytes == NULL){
		free(dest_l.bytes);	
		free(orig_r.bytes);
		free(dest_r.bytes);
	}
	free(orig_r.bytes);
	
	dest_aud = ntrb_merge_to_stereo(dest_l, dest_r);
	if(dest_aud.bytes == NULL) dest_aud = failed_ntrb_AudioDatapoints;
	
	free(dest_l.bytes);
	free(dest_r.bytes);	
	
	return dest_aud;
}

//requires the audio to be in float32 and mono channeled.
ntrb_AudioDatapoints ntrb_mono_to_xchannels(const ntrb_AudioDatapoints orig, const size_t dest_channels){
	ntrb_AudioDatapoints multichannel_aud = new_ntrb_AudioDatapoints(orig.byte_count * dest_channels);
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
	
	ntrb_AudioDatapoints stereo_aud = new_ntrb_AudioDatapoints(stereo_float_count * sizeof(float));
	if(stereo_aud.bytes == NULL) return failed_ntrb_AudioDatapoints;
	
	for(size_t i = 0; i < stereo_float_count; i+=2){
		((float*)(stereo_aud.bytes))[i] = ((float*)(l_ch.bytes))[i/2];
		((float*)(stereo_aud.bytes))[i+1] = ((float*)(r_ch.bytes))[i/2];
	}
	
	return stereo_aud;
}

//input could be in any sample format, any channels, any samplerates.
enum ntrb_StdAudFmtConversionResult ntrb_to_std_sample_fmt(ntrb_AudioDatapoints* const ret_aud, const ntrb_AudioDatapoints orig, const PaSampleFormat originalFormat){
	if(originalFormat == paInt16){
		*ret_aud = ntrb_AudioDatapoints_i16_to_f32(orig);
	}
	else if(originalFormat == ntrb_std_sample_fmt){
		*ret_aud = copy_ntrb_AudioDatapoints(orig);
	}
	else return ntrb_StdAudFmtConversion_UnknownSampleFormat;

	if(ret_aud->bytes == NULL) 
		return ntrb_StdAudFmtConversion_AllocFailure;
	else return ntrb_StdAudFmtConversion_OK;
}

//input must be in float32 sample format.
enum ntrb_StdAudFmtConversionResult ntrb_to_std_aud_channels(ntrb_AudioDatapoints* const ret_aud, const ntrb_AudioDatapoints orig, const uint8_t audchannels){
	if(audchannels == ntrb_std_audchannels){
		*ret_aud = copy_ntrb_AudioDatapoints(orig);
	}
	else if(audchannels == 1){
		*ret_aud = ntrb_mono_to_xchannels(orig, 2);
	}
	else return ntrb_StdAudFmtConversion_UnsupportedChannels;
	
	if(ret_aud->bytes == NULL) return ntrb_StdAudFmtConversion_AllocFailure;
	else return ntrb_StdAudFmtConversion_OK;
}

//input must be in stereo float32 sample format.
enum ntrb_StdAudFmtConversionResult ntrb_to_std_samplerate(ntrb_AudioDatapoints* const ret_aud, const ntrb_AudioDatapoints orig, const uint32_t samplerate){
	*ret_aud = ntrb_to_samplerate(orig, samplerate, ntrb_std_samplerate);				
	
	if(ret_aud->bytes == NULL) 
		return ntrb_StdAudFmtConversion_AllocFailure;
	else return ntrb_StdAudFmtConversion_OK;
}

//input could be in any sample format, any channels, any samplerates.
enum ntrb_StdAudFmtConversionResult ntrb_to_standard_format(ntrb_AudioDatapoints* const ret_aud, const ntrb_AudioDatapoints orig, const ntrb_AudioHeader* const orig_header){
	
	ntrb_AudioDatapoints audF32;
	//audF32.bytes Alloc
	const enum ntrb_StdAudFmtConversionResult to_f32_result = ntrb_to_std_sample_fmt(&audF32, orig, orig_header->AudioFormat);
	if(to_f32_result != ntrb_StdAudFmtConversion_OK)
		return to_f32_result;
	
	ntrb_AudioDatapoints audF32_2ch;
	const enum ntrb_StdAudFmtConversionResult to_2ch_result = ntrb_to_std_aud_channels(&audF32_2ch, audF32, orig_header->NumChannels);
	//ret_aud.bytes Alloc
	free(audF32.bytes);
	if(to_2ch_result != ntrb_StdAudFmtConversion_OK)
		return to_2ch_result;
	
	//audF32_96k.bytes Alloc
	const enum ntrb_StdAudFmtConversionResult to_96k_result = ntrb_to_std_samplerate(ret_aud, audF32_2ch, orig_header->SampleRate);
	free(audF32_2ch.bytes);
	if(to_96k_result != ntrb_StdAudFmtConversion_OK)
		return to_96k_result;
	
	return ntrb_StdAudFmtConversion_OK;
}


enum ntrb_LoadStdFmtAudioResult ntrb_load_std_fmt_audio(ntrb_AudioDatapoints* const ret, const char* const filename){
	ntrb_SpanU8 audiofile_data;
	//audiofile_data.ptr Alloc
	const enum ntrb_ReadFileResult file_retreive_result = ntrb_read_entire_file_rb(&audiofile_data, filename);
	//for anything error, we don't want to keep the read data. We are okay with EOF due to how the read function uses calloc,
	//so anything after the EOF would be 0, which we chose to accept.
	if(file_retreive_result != ntrb_ReadFileResult_OK && file_retreive_result != ntrb_ReadFileResult_ReachedEOF){			
		return ntrb_LoadStdFmtAudioResult_ntrb_ReadFileResult + file_retreive_result;
	}
	
	ntrb_AudioHeader audioheader;
	size_t audiodata_offset = 0;
	const enum ntrb_GetWAVheaderStatus audioheader_result = WAVfile_to_ntrb_AudioHeader(&audioheader, &audiodata_offset, audiofile_data);

	if(audioheader_result != ntrb_GetWAVheader_ok){	
		//audiofile_data.ptr Free
		free(audiofile_data.ptr);
		print_ntrb_AudioHeader(audioheader, stdout);
		return ntrb_LoadStdFmtAudioResult_ntrb_GetWAVheaderStatus + audioheader_result;
	}

	//audiodata.bytes Alloc
	const ntrb_AudioDatapoints audiodata = ntrb_get_WAV_audiodata_ntrb_AudioHeader(audioheader, audiofile_data, audiodata_offset);
	//audiofile_data.ptr Free
	free(audiofile_data.ptr);
	
	if(audiodata.bytes == NULL) return ntrb_LoadStdFmtAudioResult_AllocError;

	//ret.bytes Alloc
	const enum ntrb_StdAudFmtConversionResult conversion_result = ntrb_to_standard_format(ret, audiodata, &audioheader);
	//audiodata.bytes Free
	free(audiodata.bytes);

	if(conversion_result != ntrb_StdAudFmtConversion_OK)
		return ntrb_LoadStdFmtAudioResult_ntrb_StdAudFmtConversionResult + conversion_result;
	else return ntrb_LoadStdFmtAudioResult_OK;
}