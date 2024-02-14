#include "test_wav_wrapper.h"
#include "../src/wav_wrapper.h"

#include "../src/SpanU8.h"
#include "../src/WAVheader.h"
#include "../src/file_wrapper.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

static void test_get_ntrb_WAVheader(FILE*, FILE*){
	ntrb_SpanU8 file_buffer;
	ntrb_WAVheader wavheader;
	size_t audiodata_offset = 0;
	
	//test a regular wav file (ntrb_GetWAVheader_ok)
	assert(ntrb_read_entire_file_rb(&file_buffer, "test/wav_wrapper/regular_wav.wav") == ntrb_ReadFileResult_OK);
	assert(get_ntrb_WAVheader(&wavheader, &audiodata_offset, file_buffer) == ntrb_GetWAVheader_ok);
		
	const ntrb_WAVheader regular_wav_header = {
		.ChunkID = {'R', 'I', 'F', 'F'},
		.ChunkSize = 4222146,
		.Format = {'W', 'A', 'V', 'E'},
		
		.Subchunk1ID = {'f', 'm', 't', ' '},
		.Subchunk1Size = 16,
		.AudioFormat = 1,
		.NumChannels = 2,
		.SampleRate = 48000,
		.ByteRate = 192000,
		.BlockAlign = 4,
		.BitsPerSample = 16,
		
		.Subchunk2ID = {'d', 'a', 't', 'a'},
		.Subchunk2Size = 4221948
	};
	assert(ntrb_WAVheader_equal(&wavheader, &regular_wav_header));
	free(file_buffer.ptr);
	assert(audiodata_offset == 206);
	
	//test ntrb_GetWAVheader_buffer_too_small
	const size_t minimum_filesize_bytes = 44;
	const size_t too_small_filesize = minimum_filesize_bytes - 15;
	file_buffer.elem = too_small_filesize;
	assert(get_ntrb_WAVheader(&wavheader, &audiodata_offset, file_buffer) == ntrb_GetWAVheader_buffer_too_small);
	
	//test ntrb_GetWAVheader_invalid_RIFF_ID
	assert(ntrb_read_entire_file_rb(&file_buffer, "test/wav_wrapper/invalid_riff.wav") == ntrb_ReadFileResult_OK);
	assert(get_ntrb_WAVheader(&wavheader, &audiodata_offset, file_buffer) == ntrb_GetWAVheader_invalid_RIFF_ID);
	free(file_buffer.ptr);
	
	//test ntrb_GetWAVheader_invalid_WAVE_ID
	assert(ntrb_read_entire_file_rb(&file_buffer, "test/wav_wrapper/invalid_wave.wav") == ntrb_ReadFileResult_OK);
	assert(get_ntrb_WAVheader(&wavheader, &audiodata_offset, file_buffer) == ntrb_GetWAVheader_invalid_WAVE_ID);
	free(file_buffer.ptr);

	//test ntrb_GetWAVheader_invalid_fmt_ID
	assert(ntrb_read_entire_file_rb(&file_buffer, "test/wav_wrapper/invalid_fmt.wav") == ntrb_ReadFileResult_OK);
	assert(get_ntrb_WAVheader(&wavheader, &audiodata_offset, file_buffer) == ntrb_GetWAVheader_invalid_fmt_ID);
	free(file_buffer.ptr);

	//test ntrb_GetWAVheader_invalid_data_ID
	assert(ntrb_read_entire_file_rb(&file_buffer, "test/wav_wrapper/invalid_data_id.wav") == ntrb_ReadFileResult_OK);
	assert(get_ntrb_WAVheader(&wavheader, &audiodata_offset, file_buffer) == ntrb_GetWAVheader_invalid_data_ID);
	free(file_buffer.ptr);

	//test ntrb_GetWAVheader_invalid_chunk_size for ChunkSize
	assert(ntrb_read_entire_file_rb(&file_buffer, "test/wav_wrapper/chunksize_overflow.wav") == ntrb_ReadFileResult_OK);
	assert(get_ntrb_WAVheader(&wavheader, &audiodata_offset, file_buffer) == ntrb_GetWAVheader_invalid_chunk_size);
	free(file_buffer.ptr);	
	
	//test ntrb_GetWAVheader_invalid_Subchunk1Size for Subchunk1Size
	assert(ntrb_read_entire_file_rb(&file_buffer, "test/wav_wrapper/subchunk1_size_overflow.wav") == ntrb_ReadFileResult_OK);
	assert(get_ntrb_WAVheader(&wavheader, &audiodata_offset, file_buffer) == ntrb_GetWAVheader_invalid_Subchunk1Size);
	free(file_buffer.ptr);	
	
	//test ntrb_GetWAVheader_invalid_Subchunk2Size for Subchunk2Size
	assert(ntrb_read_entire_file_rb(&file_buffer, "test/wav_wrapper/subchunk2_size_overflow.wav") == ntrb_ReadFileResult_OK);
	assert(get_ntrb_WAVheader(&wavheader, &audiodata_offset, file_buffer) == ntrb_GetWAVheader_invalid_Subchunk2Size);
	free(file_buffer.ptr);		
}

static void test_ntrb_get_WAV_audiodata(FILE*, FILE*){
	ntrb_SpanU8 file_buffer;
	ntrb_WAVheader wav_header;
	size_t audiodata_offset = 0;
	
	assert(ntrb_read_entire_file_rb(&file_buffer, "test/wav_wrapper/regular_wav.wav") == ntrb_ReadFileResult_OK);
	assert(get_ntrb_WAVheader(&wav_header, &audiodata_offset, file_buffer) == ntrb_GetWAVheader_ok);
	assert(audiodata_offset == 206);

	const ntrb_AudioDatapoints aud = ntrb_get_WAV_audiodata(&wav_header, file_buffer, audiodata_offset);
	assert(aud.bytes != NULL);
	assert(aud.byte_pos == 0);
	assert(aud.byte_count == wav_header.Subchunk2Size);
	
	const int memcmp_equal = 0;
	assert(memcmp(aud.bytes, file_buffer.ptr + audiodata_offset, wav_header.Subchunk2Size) == memcmp_equal);
	
	free(aud.bytes);
	free(file_buffer.ptr);
}

static void test_ntrb_WAVheader_audio_msec(FILE*, FILE*){
	ntrb_SpanU8 file_buffer;
	ntrb_WAVheader wav_header;
	size_t audiodata_offset = 0;
	
	assert(ntrb_read_entire_file_rb(&file_buffer, "test/wav_wrapper/regular_wav.wav") == ntrb_ReadFileResult_OK);
	assert(get_ntrb_WAVheader(&wav_header, &audiodata_offset, file_buffer) == ntrb_GetWAVheader_ok);
	free(file_buffer.ptr);
	
	const float measured_msecs = 21989.3125;
	const float aud_allow_msec_error = 1 * 100;	
	const float aud_min_msec = measured_msecs - aud_allow_msec_error;
	const float aud_max_msec = measured_msecs + aud_allow_msec_error;
	
	const float calculated_msec = ntrb_WAVheader_audio_msec(&wav_header);
	
	assert(calculated_msec >= aud_min_msec);
	assert(calculated_msec <= aud_max_msec);
}

void test_suite_ntrb_wav_wrapper(FILE* const outstream, FILE* const errstream){
	test_get_ntrb_WAVheader(outstream, errstream);
	test_ntrb_get_WAV_audiodata(outstream, errstream);
	test_ntrb_WAVheader_audio_msec(outstream, errstream);
}
