#include "wav_wrapper.h"

#include "SpanU8.h"
#include "WAVheader.h"
#include "AudioHeader.h"
#include "AudioDatapoints.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

/*
TODO:
	Ablility to skip junk between RIFF chunk and Subchunk1
*/

enum ntrb_GetWAVheaderStatus get_ntrb_WAVheader(ntrb_WAVheader* const return_arg, size_t* const audiodata_offset, const ntrb_SpanU8 file_buffer){
	const size_t min_wav_header_size = 44;
	if(file_buffer.elem < min_wav_header_size)
		return ntrb_GetWAVheader_buffer_too_small;
		
	const int strcmp_equal = 0;	
	
	//RIFF filetype descriptor
	*(uint32_t*)(return_arg->ChunkID) = *(uint32_t*)(file_buffer.ptr + 0);	
	if(strncmp((char*)(return_arg->ChunkID), "RIFF", 4) != strcmp_equal)
		return ntrb_GetWAVheader_invalid_RIFF_ID;
	
	return_arg->ChunkSize = *(uint32_t*)(file_buffer.ptr + 4);
	if(return_arg->ChunkSize + 8 > file_buffer.elem) return ntrb_GetWAVheader_invalid_chunk_size;
	
	*(uint32_t*)(return_arg->Format) = *(uint32_t*)(file_buffer.ptr + 8);
	if(strncmp((char*)(return_arg->Format), "WAVE", 4) != strcmp_equal)
		return ntrb_GetWAVheader_invalid_WAVE_ID;
	
	*(uint32_t*)(return_arg->Subchunk1ID) = *(uint32_t*)(file_buffer.ptr + 12);
	if(strncmp((char*)(return_arg->Subchunk1ID), "fmt ", 4)  != strcmp_equal)
		return ntrb_GetWAVheader_invalid_fmt_ID;
	
	//Subchunk1 - Audiodata information
	return_arg->Subchunk1Size = *(uint32_t*)(file_buffer.ptr + 16);
	if(return_arg->Subchunk1Size >= return_arg->ChunkSize) return ntrb_GetWAVheader_invalid_Subchunk1Size;
	
	return_arg->AudioFormat = *(uint16_t*)(file_buffer.ptr + 20);
	return_arg->NumChannels = *(uint16_t*)(file_buffer.ptr + 22);
	return_arg->SampleRate = *(uint32_t*)(file_buffer.ptr + 24);
	return_arg->ByteRate = *(uint32_t*)(file_buffer.ptr + 28);
	return_arg->BlockAlign = *(uint16_t*)(file_buffer.ptr + 32);
	return_arg->BitsPerSample = *(uint16_t*)(file_buffer.ptr + 34);
		
	//Subchunk2 - Audiodata
	//data subchunk, start point is at least a 36 byte offset, depending on the "extra parameters" after bits_per_sample
	const uint8_t end_of_bits_per_sample = 36;
	const uint16_t end_of_file = return_arg->ChunkSize + 8;
	
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
	if(start_of_data_chunk == 0) return ntrb_GetWAVheader_invalid_data_ID;
	
	*(uint32_t*)(return_arg->Subchunk2ID) = *(uint32_t*)(file_buffer.ptr + start_of_data_chunk);
	return_arg->Subchunk2Size = *(uint32_t*)(file_buffer.ptr + start_of_data_chunk + 4);
	if(return_arg->Subchunk2Size + 8 + start_of_data_chunk > file_buffer.elem) return ntrb_GetWAVheader_invalid_Subchunk2Size;
	
	*audiodata_offset = start_of_data_chunk + 8;
	return ntrb_GetWAVheader_ok;
}

void print_ntrb_WAVheader(const ntrb_WAVheader* const header, FILE* const printstream){
	fprintf(printstream, "ntrb_WAVheader contents from print_ntrb_WAVheader():\n");
	fprintf(printstream, "ChunkID (82 73 70 70)d: %u %u %u %u\n", 
			(uint32_t)(header->ChunkID[0]),
			(uint32_t)(header->ChunkID[1]),
			(uint32_t)(header->ChunkID[2]),
			(uint32_t)(header->ChunkID[3])
	);
	
	fprintf(printstream, "ChunkSize: %u\n", header->ChunkSize);
	
	fprintf(printstream, "Format (87 65 86 69)d: %u %u %u %u\n", 
			(uint32_t)(header->Format[0]),
			(uint32_t)(header->Format[1]),
			(uint32_t)(header->Format[2]),
			(uint32_t)(header->Format[3])
	);	
	
	fprintf(printstream, "Subchunk1ID (102 109 116 32)d: %u %u %u %u\n", 
			(uint32_t)(header->Subchunk1ID[0]),
			(uint32_t)(header->Subchunk1ID[1]),
			(uint32_t)(header->Subchunk1ID[2]),
			(uint32_t)(header->Subchunk1ID[3])
	);
	
	fprintf(printstream, "Subchunk1Size: %u\n", header->Subchunk1Size);
	fprintf(printstream, "AudioFormat: %hu\n", header->AudioFormat);
	fprintf(printstream, "NumChannels: %hu\n", header->NumChannels);
	fprintf(printstream, "SampleRate: %u\n", header->SampleRate);
	fprintf(printstream, "ByteRate: %u\n", header->ByteRate);
	fprintf(printstream, "BlockAlign: %hu\n", header->BlockAlign);
	fprintf(printstream, "BitsPerSample: %hu\n", header->BitsPerSample);
	
	fprintf(printstream, "Subchunk2ID (100 97 116 97)d: %u %u %u %u\n", 
			(uint32_t)(header->Subchunk2ID[0]),
			(uint32_t)(header->Subchunk2ID[1]),
			(uint32_t)(header->Subchunk2ID[2]),
			(uint32_t)(header->Subchunk2ID[3])
	);

	fprintf(printstream, "Subchunk2Size: %u\n", header->Subchunk2Size);
	
	fprintf(printstream, "\n");	
	fflush(printstream);
}

ntrb_AudioDatapoints ntrb_get_WAV_audiodata(const ntrb_WAVheader* const header, const ntrb_SpanU8 wavfile, const size_t audiodata_offset){
	ntrb_AudioDatapoints audiodata = new_ntrb_AudioDatapoints(header->Subchunk2Size * sizeof(uint8_t));
	if(audiodata.bytes == NULL) return audiodata;
	
	memcpy(audiodata.bytes, wavfile.ptr + audiodata_offset, header->Subchunk2Size);
	return audiodata;
}

float ntrb_WAVheader_audio_msec(const ntrb_WAVheader* const header){
	//subchunk_size = seconds * ByteRate
	//therefore seconds = subchunk_size / ByteRate;
	//byte rate is bytes-per-sample x NumChannels per second, i.e., size of a single frame.
	const float seconds = (float)(header->Subchunk2Size) / (float)(header->ByteRate);
	return seconds * 1000;
}


ntrb_AudioHeader ntrb_WAVheader_to_ntrb_AudioHeader(const ntrb_WAVheader* const wavheader){
	ntrb_AudioHeader audhd;
	audhd.AudioFormat = ntrb_WAV_PaSampleFormat(wavheader);
	audhd.AudioDataSize = wavheader->Subchunk2Size;
	audhd.SampleRate = wavheader->SampleRate;
	audhd.ByteRate = wavheader->ByteRate;
	audhd.NumChannels = wavheader->NumChannels;
	audhd.BlockAlign = wavheader->BlockAlign;
	audhd.BitsPerSample = wavheader->BitsPerSample;
	
	return audhd;
}

enum ntrb_GetWAVheaderStatus WAVfile_to_ntrb_AudioHeader(ntrb_AudioHeader* const ret, size_t* const audiodata_offset, const ntrb_SpanU8 file_buffer){
	ntrb_WAVheader wavheader;
	const enum ntrb_GetWAVheaderStatus wavheader_status = get_ntrb_WAVheader(&wavheader, audiodata_offset, file_buffer);
	if(wavheader_status == ntrb_GetWAVheader_ok){
		*ret = ntrb_WAVheader_to_ntrb_AudioHeader(&wavheader);
	}
	
	return wavheader_status;
}

ntrb_AudioDatapoints ntrb_get_WAV_audiodata_ntrb_AudioHeader(const ntrb_AudioHeader header, const ntrb_SpanU8 wavfile, const size_t audiodata_offset){
	ntrb_AudioDatapoints audiodata = new_ntrb_AudioDatapoints(header.AudioDataSize * sizeof(uint8_t));
	if(audiodata.bytes == NULL) return audiodata;
	
	memcpy(audiodata.bytes, wavfile.ptr + audiodata_offset, header.AudioDataSize);
	return audiodata;	
}


//this could be further expanded
PaSampleFormat ntrb_WAV_PaSampleFormat(const ntrb_WAVheader* const header){
	const uint16_t PCM_format = 1;
	const uint16_t IEEE_float_format = 3;
	if(header->AudioFormat == PCM_format && header->BitsPerSample == 16) return paInt16;
	if(header->AudioFormat == PCM_format && header->BitsPerSample == 32) return paInt32;
	if(header->AudioFormat == IEEE_float_format && header->BitsPerSample == 32) return paFloat32;
	
	else return paCustomFormat;
}