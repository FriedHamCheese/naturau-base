#include "test_aud_std_fmt.h"
#include "aud_std_fmt.h"

#include "utils.h"
#include "AudioDatapoints.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

static void test_ntrb_AudioDatapoints_i16_to_f32(FILE*, FILE*){
	const size_t datapoint_count = 9;
	const int16_t i16_datapoints[] = {INT16_MAX, -INT16_MAX, 4095, -4095, -13, -15, 6, 9, 8};
	
	float* f32_ref_datapoints = malloc(datapoint_count * sizeof(float));
	assert(f32_ref_datapoints != NULL);
	
	for(size_t i = 0; i < datapoint_count; i++){
		f32_ref_datapoints[i] = (float)(i16_datapoints[i]) / (float)INT16_MAX;
	}
	
	ntrb_AudioDatapoints i16_aud;
	i16_aud.bytes = (uint8_t*)i16_datapoints;
	i16_aud.byte_count = sizeof(int16_t) * datapoint_count;
	i16_aud.byte_pos = 0;
	
	ntrb_AudioDatapoints f32_aud = ntrb_AudioDatapoints_i16_to_f32(i16_aud);
	assert(f32_aud.bytes != NULL);
	assert(f32_aud.byte_count == datapoint_count * sizeof(float));
	
	assert(memcmp(f32_aud.bytes, f32_ref_datapoints, f32_aud.byte_count) == 0);
	free(f32_aud.bytes);
	free(f32_ref_datapoints);
}

static void test_ntrb_split_as_mono(FILE*, FILE*){
	const size_t mono_float_count = 3;
	const size_t channels = 3;
	const size_t float_count = mono_float_count * channels;
	ntrb_AudioDatapoints aud = new_ntrb_AudioDatapoints(sizeof(float) * float_count);
	assert(aud.bytes != NULL);
	
	//0.0f through 8.0f
	for(size_t i = 0; i < float_count; i++){
		((float*)(aud.bytes))[i] = (float)i;
	}
	
	ntrb_AudioDatapoints ch2_aud =  ntrb_split_as_mono(aud, channels, 1);
	assert(ch2_aud.bytes != NULL);
	assert(ch2_aud.byte_count == sizeof(float) * mono_float_count);
	
	assert(((float*)(ch2_aud.bytes))[0] == 1.0f);
	assert(((float*)(ch2_aud.bytes))[1] == 4.0f);
	assert(((float*)(ch2_aud.bytes))[2] == 7.0f);
	
	free(ch2_aud.bytes);
	free(aud.bytes);
}

static void test_ntrb_mono_to_xchannels(FILE*, FILE*){
	const size_t mono_float_count = 5;
	const size_t dest_channels = 4;
	
	ntrb_AudioDatapoints mono_aud = new_ntrb_AudioDatapoints(sizeof(float) * mono_float_count);
	assert(mono_aud.bytes != NULL);
	
	//0.0f to 4.0f
	for(size_t i = 0; i < mono_float_count; i++){
		((float*)(mono_aud.bytes))[i] = (float)i;
	}
	
	ntrb_AudioDatapoints dup4ch_aud = ntrb_mono_to_xchannels(mono_aud, dest_channels);
	assert(dup4ch_aud.bytes != NULL);
	assert(dup4ch_aud.byte_count == sizeof(float) * mono_float_count * dest_channels);
	assert(dup4ch_aud.byte_count == mono_aud.byte_count * dest_channels);	
	
	for(size_t i_mono = 0; i_mono < mono_float_count; i_mono++){
		const float mono = ((float*)(mono_aud.bytes))[i_mono];
		
		for(size_t channel = 0; channel < dest_channels; channel++){
			assert( ((float*)(dup4ch_aud.bytes))[i_mono*dest_channels +channel] == mono );
		}
	}
	
	free(mono_aud.bytes);
	free(dup4ch_aud.bytes);
}

static void test_ntrb_merge_to_stereo(FILE*, FILE*){
	const size_t mono_float_count = 4;
	
	ntrb_AudioDatapoints l_ch = new_ntrb_AudioDatapoints(sizeof(float) * mono_float_count);
	assert(l_ch.bytes != NULL);
	//1.0, 3.0, 5.0, 7.0
	for(size_t i = 0; i < mono_float_count; i++){
		((float*)(l_ch.bytes))[i] = (float)((2*i)+1);
	}
	
	ntrb_AudioDatapoints r_ch = new_ntrb_AudioDatapoints(sizeof(float) * mono_float_count);	
	assert(r_ch.bytes != NULL);
	//2.0, 4.0, 6.0, 8.0
	for(size_t i = 0; i < mono_float_count; i++){
		((float*)(r_ch.bytes))[i] = (float)((2*i)+2);
	}
	
	const size_t stereo_float_count = 2 * mono_float_count;	
	ntrb_AudioDatapoints stereo_aud = ntrb_merge_to_stereo(l_ch, r_ch);
	assert(stereo_aud.bytes != NULL);
	assert(stereo_aud.byte_count == sizeof(float) * stereo_float_count);
	
	for(size_t i = 0; i < stereo_float_count; i++){
		assert(((float*)(stereo_aud.bytes))[i] == (float)i + 1.0);
	}
	
	//inequal byte count, should trip and return NULL
	l_ch.byte_count--;	
	assert(ntrb_merge_to_stereo(l_ch, r_ch).bytes == NULL);
	
	free(l_ch.bytes);
	free(r_ch.bytes);
	free(stereo_aud.bytes);
}


static void test_ntrb_to_samplerate_mono_upscale(FILE*, FILE*){
	const size_t seconds = 1;
	const size_t orig_samplerate = 3;
	const float orig_datapoints[] = {1.5, -1.5, 1.2};
	
	const size_t dest_samplerate = 5;
	const float expected_dest_datapoints[] = {1.5, 0, -1.5, -0.15, 1.2};
	
	const ntrb_AudioDatapoints orig_aud = {(uint8_t*)orig_datapoints, seconds * orig_samplerate * sizeof(float), 0};
	ntrb_AudioDatapoints dest_aud = ntrb_to_samplerate_mono(orig_aud, orig_samplerate, dest_samplerate);
	assert(dest_aud.bytes != NULL);
	assert(dest_aud.byte_count == sizeof(float) * seconds * dest_samplerate);
	
	for(size_t i = 0; i < dest_samplerate * seconds; i++){
		const float allowed_error = 0.0000001;
		assert(ntrb_float_equal(expected_dest_datapoints[i], ((float*)(dest_aud.bytes))[i], allowed_error));
	}
	
	free(dest_aud.bytes);
}

static void test_ntrb_to_samplerate_mono_downscale(FILE*, FILE*){
	const size_t seconds = 1;
	const size_t orig_samplerate = 6;
	const float orig_datapoints[] = {-1.3, 0.15, -1.7, 1.6, 0.5, -1.2};
	const ntrb_AudioDatapoints orig_aud = {(uint8_t*)orig_datapoints, seconds * orig_samplerate * sizeof(float), 0};
	
	const size_t dest_samplerate = 4;
	const float expected_dest_datapoints[] = {-1.3, -1.08333333, 1.2333333, -1.2};
	
	ntrb_AudioDatapoints dest_aud = ntrb_to_samplerate_mono(orig_aud, orig_samplerate, dest_samplerate);
	assert(dest_aud.bytes != NULL);
	assert(dest_aud.byte_count == sizeof(float) * seconds * dest_samplerate);
	
	for(size_t i = 0; i < dest_samplerate * seconds; i++){
		const float allowed_error = 0.000001;
		assert(ntrb_float_equal(expected_dest_datapoints[i], ((float*)(dest_aud.bytes))[i], allowed_error));
	}	
	
	
	free(dest_aud.bytes);
}

static void test_ntrb_to_samplerate_mono(FILE* outstream, FILE* errstream){
	test_ntrb_to_samplerate_mono_upscale(outstream, errstream);
	test_ntrb_to_samplerate_mono_downscale(outstream, errstream);
}


void test_suite_ntrb_aud_std_fmt(FILE* const outstream, FILE* const errstream){
	test_ntrb_AudioDatapoints_i16_to_f32(outstream, errstream);
	test_ntrb_split_as_mono(outstream, errstream);
	test_ntrb_to_samplerate_mono(outstream, errstream);
	test_ntrb_mono_to_xchannels(outstream, errstream);
	test_ntrb_merge_to_stereo(outstream, errstream);
}
