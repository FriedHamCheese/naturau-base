#include "wav_wrapper.h"

#include "SpanU8.h"
#include "AudioHeader.h"
#include "AudioDatapoints.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

//this could be further expanded
PaSampleFormat ntrb_WAV_PaSampleFormat(const uint16_t WAV_audioFormat, const uint16_t BitsPerSample){
	const uint16_t PCM_format = 1;
	const uint16_t IEEE_float_format = 3;
	if(WAV_audioFormat == PCM_format && BitsPerSample == 16) return paInt16;
	if(WAV_audioFormat == PCM_format && BitsPerSample == 32) return paInt32;
	if(WAV_audioFormat == IEEE_float_format && BitsPerSample == 32) return paFloat32;
	
	else return paCustomFormat;
}

enum ntrb_AudioHeaderFromWAVFileStatus ntrb_AudioHeader_from_WAVfile(ntrb_AudioHeader* const return_arg, size_t* const audiodata_offset, size_t* const audiodata_bytes, const ntrb_SpanU8 file_buffer){
	const size_t min_wav_header_size = 44;
	if(file_buffer.elem < min_wav_header_size)
		return ntrb_AudioHeaderFromWAVFile_buffer_too_small;
		
	const int strcmp_equal = 0;	
	
	//RIFF filetype descriptor
	char ChunkID[4];
	*(uint32_t*)ChunkID = *(uint32_t*)(file_buffer.ptr + 0);	
	if(strncmp(ChunkID, "RIFF", 4) != strcmp_equal)
		return ntrb_AudioHeaderFromWAVFile_invalid_RIFF_ID;
	
	const uint32_t ChunkSize = *(uint32_t*)(file_buffer.ptr + 4);
	if(ChunkSize + 8 != file_buffer.elem) return ntrb_AudioHeaderFromWAVFile_invalid_chunk_size;
	
	char FormatID[4];
	*(uint32_t*)FormatID = *(uint32_t*)(file_buffer.ptr + 8);

	if(strncmp(FormatID, "WAVE", 4) != strcmp_equal)
		return ntrb_AudioHeaderFromWAVFile_invalid_WAVE_ID;
	
	//future code for scanning for 'fmt ' because junk data could be place between riff and fmt .
	
	char Subchunk1ID[4];
	*(uint32_t*)Subchunk1ID = *(uint32_t*)(file_buffer.ptr + 12);
	if(strncmp(Subchunk1ID, "fmt ", 4)  != strcmp_equal)
		return ntrb_AudioHeaderFromWAVFile_invalid_fmt_ID;
	
	
	//Subchunk1 - Audiodata information
	const uint32_t Subchunk1Size = *(uint32_t*)(file_buffer.ptr + 16);
	if(Subchunk1Size >= ChunkSize) return ntrb_AudioHeaderFromWAVFile_invalid_Subchunk1Size;
	
	const uint16_t WAV_AudioFormat = *(uint16_t*)(file_buffer.ptr + 20);
	return_arg->NumChannels = *(uint16_t*)(file_buffer.ptr + 22);
	return_arg->SampleRate = *(uint32_t*)(file_buffer.ptr + 24);
	return_arg->BitsPerSample = *(uint16_t*)(file_buffer.ptr + 34);
	return_arg->AudioFormat = ntrb_WAV_PaSampleFormat(WAV_AudioFormat, return_arg->BitsPerSample);
		
		
	//Subchunk2 - Audiodata
	//data subchunk, start point is at least a 36 byte offset, depending on the "extra parameters" after bits_per_sample
	const uint8_t end_of_bits_per_sample = 36;
	const uint16_t end_of_file = ChunkSize + 8;
	
	size_t start_of_data_chunk = 0;
	for(uint16_t i = end_of_bits_per_sample; i < end_of_file; i++)
	{
		const bool has_atleast_3_bytes_ahead = (i+3) < end_of_file;
		if(file_buffer.ptr[i] == 'd' && has_atleast_3_bytes_ahead){
			const bool valid_data_id = strncmp((char*)(file_buffer.ptr + i), "data", 4) == strcmp_equal;
			if(valid_data_id){
				start_of_data_chunk = i;
				break;
			}
		}
	}
	if(start_of_data_chunk == 0) return ntrb_AudioHeaderFromWAVFile_invalid_data_ID;
	
	*audiodata_bytes = *(uint32_t*)(file_buffer.ptr + start_of_data_chunk + 4);
	if(*audiodata_bytes + 8 + start_of_data_chunk > file_buffer.elem) return ntrb_AudioHeaderFromWAVFile_invalid_Subchunk2Size;
	
	*audiodata_offset = start_of_data_chunk + 8;
	return ntrb_AudioHeaderFromWAVFile_ok;
}

ntrb_AudioDatapoints ntrb_get_WAV_audiodata(const ntrb_SpanU8 wavfile, const size_t audiodata_size, const size_t audiodata_offset){
	ntrb_AudioDatapoints audiodata = ntrb_AudioDatapoints_new(audiodata_size * sizeof(uint8_t));
	if(audiodata.bytes == NULL) return audiodata;
	
	memcpy(audiodata.bytes, wavfile.ptr + audiodata_offset, audiodata_size);
	return audiodata;	
}