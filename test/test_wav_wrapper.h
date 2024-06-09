/*
Copyright 2024 Pawikan Boonnaum

This file is part of naturau-base/ntrb.

naturau-base/ntrb is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

naturau-base/ntrb is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with naturau-base/ntrb. If not, see <https://www.gnu.org/licenses/>.
*/

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

static void test_ntrb_getSubchunk1Start(){
	const size_t typical_RIFF_chunk_end = 12;
	
	ntrb_SpanU8 file_buffer;
	assert(ntrb_read_entire_file_rb(&file_buffer, "test/wav_wrapper/regular_wav.wav") == ntrb_ReadFileResult_OK);

	assert(ntrb_getSubchunk1Start(file_buffer, typical_RIFF_chunk_end) == 12);
	ntrb_free(file_buffer.ptr);
	
	assert(ntrb_read_entire_file_rb(&file_buffer, "test/wav_wrapper/subchunk1_seek.wav") == ntrb_ReadFileResult_OK);
	assert(ntrb_getSubchunk1Start(file_buffer, typical_RIFF_chunk_end) == 48);
	ntrb_free(file_buffer.ptr);	
}

static void test_ntrb_getSubchunk2Start(){
	const size_t typical_RIFF_chunk_end = 12;
	const size_t minimum_subchunk1_size = 24;	
	
	ntrb_SpanU8 file_buffer;
	assert(ntrb_read_entire_file_rb(&file_buffer, "test/wav_wrapper/regular_wav.wav") == ntrb_ReadFileResult_OK);

	const size_t file_1_subchunk1_start = ntrb_getSubchunk1Start(file_buffer, typical_RIFF_chunk_end);
	assert(file_1_subchunk1_start != 0);
	assert(ntrb_getSubchunk2Start(file_buffer, file_1_subchunk1_start + minimum_subchunk1_size) == 198);	
	ntrb_free(file_buffer.ptr);
	
	assert(ntrb_read_entire_file_rb(&file_buffer, "test/wav_wrapper/subchunk1_seek.wav") == ntrb_ReadFileResult_OK);
	const size_t file_2_subchunk1_start = ntrb_getSubchunk1Start(file_buffer, typical_RIFF_chunk_end);
	assert(file_2_subchunk1_start != 0);
	assert(ntrb_getSubchunk2Start(file_buffer, file_2_subchunk1_start + minimum_subchunk1_size) == 72);	
	ntrb_free(file_buffer.ptr);
}

static void test_ntrb_AudioHeader_from_WAVfile(){
	ntrb_SpanU8 file_buffer;
	ntrb_AudioHeader header;
	size_t audiodata_size = 0;
	size_t audiodata_offset = 0;
	
	//test a regular wav file (ntrb_AudioHeaderFromWAVFile_ok)
	assert(ntrb_read_entire_file_rb(&file_buffer, "test/wav_wrapper/regular_wav.wav") == ntrb_ReadFileResult_OK);
	assert(ntrb_AudioHeader_from_WAVfile(&header, &audiodata_offset, &audiodata_size, file_buffer) == ntrb_AudioHeaderFromWAVFile_ok);

	ntrb_free(file_buffer.ptr);
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
	ntrb_free(file_buffer.ptr);
	
	//test ntrb_AudioHeaderFromWAVFile_invalid_WAVE_ID
	assert(ntrb_read_entire_file_rb(&file_buffer, "test/wav_wrapper/invalid_wave.wav") == ntrb_ReadFileResult_OK);
	assert(ntrb_AudioHeader_from_WAVfile(&header, &audiodata_offset, &audiodata_size, file_buffer) == ntrb_AudioHeaderFromWAVFile_invalid_WAVE_ID);
	ntrb_free(file_buffer.ptr);

	//test ntrb_AudioHeaderFromWAVFile_invalid_fmt_ID
	assert(ntrb_read_entire_file_rb(&file_buffer, "test/wav_wrapper/invalid_fmt.wav") == ntrb_ReadFileResult_OK);
	assert(ntrb_AudioHeader_from_WAVfile(&header, &audiodata_offset, &audiodata_size, file_buffer) == ntrb_AudioHeaderFromWAVFile_invalid_fmt_ID);
	ntrb_free(file_buffer.ptr);

	//test ntrb_AudioHeaderFromWAVFile_invalid_data_ID
	assert(ntrb_read_entire_file_rb(&file_buffer, "test/wav_wrapper/invalid_data_id.wav") == ntrb_ReadFileResult_OK);
	assert(ntrb_AudioHeader_from_WAVfile(&header, &audiodata_offset, &audiodata_size, file_buffer) == ntrb_AudioHeaderFromWAVFile_invalid_data_ID);
	ntrb_free(file_buffer.ptr);

	//test ntrb_AudioHeaderFromWAVFile_invalid_chunk_size for ChunkSize
	assert(ntrb_read_entire_file_rb(&file_buffer, "test/wav_wrapper/chunksize_overflow.wav") == ntrb_ReadFileResult_OK);
	assert(ntrb_AudioHeader_from_WAVfile(&header, &audiodata_offset, &audiodata_size, file_buffer) == ntrb_AudioHeaderFromWAVFile_invalid_chunk_size);
	ntrb_free(file_buffer.ptr);	
	
	//test ntrb_AudioHeaderFromWAVFile_invalid_Subchunk1Size for Subchunk1Size
	assert(ntrb_read_entire_file_rb(&file_buffer, "test/wav_wrapper/subchunk1_size_overflow.wav") == ntrb_ReadFileResult_OK);
	assert(ntrb_AudioHeader_from_WAVfile(&header, &audiodata_offset, &audiodata_size, file_buffer) == ntrb_AudioHeaderFromWAVFile_invalid_Subchunk1Size);
	ntrb_free(file_buffer.ptr);	
	
	//test ntrb_AudioHeaderFromWAVFile_invalid_Subchunk2Size for Subchunk2Size
	assert(ntrb_read_entire_file_rb(&file_buffer, "test/wav_wrapper/subchunk2_size_overflow.wav") == ntrb_ReadFileResult_OK);
	assert(ntrb_AudioHeader_from_WAVfile(&header, &audiodata_offset, &audiodata_size, file_buffer) == ntrb_AudioHeaderFromWAVFile_invalid_Subchunk2Size);
	ntrb_free(file_buffer.ptr);
}

void test_suite_ntrb_wav_wrapper(){
	test_ntrb_getSubchunk1Start();
	test_ntrb_getSubchunk2Start();
	test_ntrb_AudioHeader_from_WAVfile();
}


#endif