#ifndef test_ntrb_wav_wrapper_h
#define test_ntrb_wav_wrapper_h

#include "wav_wrapper.h"

#include "SpanU8.h"
#include "AudioHeader.h"
#include "file_wrapper.h"

#include "portaudio.h"	//paInt16

#include <stdlib.h>
#include <string.h>
#include <assert.h>

static void test_ntrb_AudioHeader_from_WAVfile(){
	ntrb_SpanU8 file_buffer;
	ntrb_AudioHeader header;
	size_t audiodata_size = 0;
	size_t audiodata_offset = 0;
	
	//test a regular wav file (ntrb_AudioHeaderFromWAVFile_ok)
	assert(ntrb_read_entire_file_rb(&file_buffer, "test/wav_wrapper/regular_wav.wav") == ntrb_ReadFileResult_OK);
	assert(ntrb_AudioHeader_from_WAVfile(&header, &audiodata_offset, &audiodata_size, file_buffer) == ntrb_AudioHeaderFromWAVFile_ok);

	free(file_buffer.ptr);
	assert(header.AudioFormat == paInt16);
	assert(header.NumChannels == 2);
	assert(header.SampleRate == 48000);
	assert(header.BitsPerSample == 16);	
	
	assert(audiodata_offset == 206);
	assert(audiodata_size == 4221948);

	//test ntrb_AudioHeaderFromWAVFile_buffer_too_small
	const size_t minimum_filesize_bytes = 44;
	const size_t too_small_filesize = minimum_filesize_bytes - 15;
	file_buffer.elem = too_small_filesize;
	assert(ntrb_AudioHeader_from_WAVfile(&header, &audiodata_offset, &audiodata_size, file_buffer) == ntrb_AudioHeaderFromWAVFile_buffer_too_small);
	
	//test ntrb_AudioHeaderFromWAVFile_invalid_RIFF_ID
	assert(ntrb_read_entire_file_rb(&file_buffer, "test/wav_wrapper/invalid_riff.wav") == ntrb_ReadFileResult_OK);
	assert(ntrb_AudioHeader_from_WAVfile(&header, &audiodata_offset, &audiodata_size, file_buffer) == ntrb_AudioHeaderFromWAVFile_invalid_RIFF_ID);
	free(file_buffer.ptr);
	
	//test ntrb_AudioHeaderFromWAVFile_invalid_WAVE_ID
	assert(ntrb_read_entire_file_rb(&file_buffer, "test/wav_wrapper/invalid_wave.wav") == ntrb_ReadFileResult_OK);
	assert(ntrb_AudioHeader_from_WAVfile(&header, &audiodata_offset, &audiodata_size, file_buffer) == ntrb_AudioHeaderFromWAVFile_invalid_WAVE_ID);
	free(file_buffer.ptr);

	//test ntrb_AudioHeaderFromWAVFile_invalid_fmt_ID
	assert(ntrb_read_entire_file_rb(&file_buffer, "test/wav_wrapper/invalid_fmt.wav") == ntrb_ReadFileResult_OK);
	assert(ntrb_AudioHeader_from_WAVfile(&header, &audiodata_offset, &audiodata_size, file_buffer) == ntrb_AudioHeaderFromWAVFile_invalid_fmt_ID);
	free(file_buffer.ptr);

	//test ntrb_AudioHeaderFromWAVFile_invalid_data_ID
	assert(ntrb_read_entire_file_rb(&file_buffer, "test/wav_wrapper/invalid_data_id.wav") == ntrb_ReadFileResult_OK);
	assert(ntrb_AudioHeader_from_WAVfile(&header, &audiodata_offset, &audiodata_size, file_buffer) == ntrb_AudioHeaderFromWAVFile_invalid_data_ID);
	free(file_buffer.ptr);

	//test ntrb_AudioHeaderFromWAVFile_invalid_chunk_size for ChunkSize
	assert(ntrb_read_entire_file_rb(&file_buffer, "test/wav_wrapper/chunksize_overflow.wav") == ntrb_ReadFileResult_OK);
	assert(ntrb_AudioHeader_from_WAVfile(&header, &audiodata_offset, &audiodata_size, file_buffer) == ntrb_AudioHeaderFromWAVFile_invalid_chunk_size);
	free(file_buffer.ptr);	
	
	//test ntrb_AudioHeaderFromWAVFile_invalid_Subchunk1Size for Subchunk1Size
	assert(ntrb_read_entire_file_rb(&file_buffer, "test/wav_wrapper/subchunk1_size_overflow.wav") == ntrb_ReadFileResult_OK);
	assert(ntrb_AudioHeader_from_WAVfile(&header, &audiodata_offset, &audiodata_size, file_buffer) == ntrb_AudioHeaderFromWAVFile_invalid_Subchunk1Size);
	free(file_buffer.ptr);	
	
	//test ntrb_AudioHeaderFromWAVFile_invalid_Subchunk2Size for Subchunk2Size
	assert(ntrb_read_entire_file_rb(&file_buffer, "test/wav_wrapper/subchunk2_size_overflow.wav") == ntrb_ReadFileResult_OK);
	assert(ntrb_AudioHeader_from_WAVfile(&header, &audiodata_offset, &audiodata_size, file_buffer) == ntrb_AudioHeaderFromWAVFile_invalid_Subchunk2Size);
	free(file_buffer.ptr);
}


static void test_ntrb_get_WAV_audiodata(){
	ntrb_SpanU8 file_buffer;
	ntrb_AudioHeader header;
	size_t audiodata_bytes = 0;
	size_t audiodata_offset = 0;
	
	assert(ntrb_read_entire_file_rb(&file_buffer, "test/wav_wrapper/regular_wav.wav") == ntrb_ReadFileResult_OK);
	assert(ntrb_AudioHeader_from_WAVfile(&header, &audiodata_offset, &audiodata_bytes, file_buffer) == ntrb_AudioHeaderFromWAVFile_ok);

	const ntrb_AudioDatapoints aud = ntrb_get_WAV_audiodata(file_buffer, audiodata_bytes, audiodata_offset);
	assert(aud.bytes != NULL);
	assert(aud.byte_pos == 0);
	assert(aud.byte_count == audiodata_bytes);
	
	const int memcmp_equal = 0;
	assert(memcmp(aud.bytes, file_buffer.ptr + audiodata_offset, audiodata_bytes) == memcmp_equal);
	
	free(aud.bytes);
	free(file_buffer.ptr);
}

void test_suite_ntrb_wav_wrapper(){
	test_ntrb_AudioHeader_from_WAVfile();
	test_ntrb_get_WAV_audiodata();
}


#endif