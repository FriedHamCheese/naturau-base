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
	if(WAV_audioFormat == IEEE_float_format && BitsPerSample == 32) return paFloat32;
	
	else return paCustomFormat;
}

size_t ntrb_getSubchunk1Start(const ntrb_SpanU8 fileBuffer, const size_t RIFFchunkEnd){
	const size_t Subchunk1IDSize = 4;
	const size_t Subchunk1SizeSize = 4;
	const size_t minEntireRIFFChunksize = Subchunk1IDSize + Subchunk1SizeSize + 16;
	
	for(size_t i = RIFFchunkEnd; i < fileBuffer.elem; i++){
		const bool enoughSpaceForSubchunk1 = (i+minEntireRIFFChunksize) < fileBuffer.elem;
		if(fileBuffer.ptr[i] == 'f' && enoughSpaceForSubchunk1){
			if(strncmp((char*)(fileBuffer.ptr + i), "fmt ", 4) == 0) return i;
		}
	}
	
	return 0;
}

size_t ntrb_getSubchunk2Start(const ntrb_SpanU8 fileBuffer, const size_t Subchunk1End){
	const size_t MinimumSubchunk2Size = 8;
	for(size_t i = Subchunk1End; i < fileBuffer.elem; i++)
	{
		const bool enoughSpaceForSubchunk2 = (i+MinimumSubchunk2Size) < fileBuffer.elem;
		if(fileBuffer.ptr[i] == 'd' && enoughSpaceForSubchunk2){
			const bool valid_data_id = strncmp((char*)(fileBuffer.ptr + i), "data", 4) == 0;
			if(valid_data_id) return i;
		}
	}
	
	return 0;
}

enum ntrb_AudioHeaderFromWAVFileStatus ntrb_AudioHeader_from_WAVfile(ntrb_AudioHeader* const returnArg, size_t* const audiodataOffset, size_t* const audiodataSize, const ntrb_SpanU8 fileBuffer){
	const size_t minWAVheaderSize = 44;
	if(fileBuffer.elem < minWAVheaderSize)
		return ntrb_AudioHeaderFromWAVFile_buffer_too_small;
		
	const int strcmpEqual = 0;	
	
	//RIFF filetype descriptor
	char ChunkID[4];
	*(uint32_t*)ChunkID = *(uint32_t*)(fileBuffer.ptr + 0);	
	if(strncmp(ChunkID, "RIFF", 4) != strcmpEqual)
		return ntrb_AudioHeaderFromWAVFile_invalid_RIFF_ID;
	
	const uint32_t ChunkSize = *(uint32_t*)(fileBuffer.ptr + 4);
	if(ChunkSize + 8 != fileBuffer.elem) return ntrb_AudioHeaderFromWAVFile_invalid_chunk_size;
	
	char FormatID[4];
	*(uint32_t*)FormatID = *(uint32_t*)(fileBuffer.ptr + 8);

	if(strncmp(FormatID, "WAVE", 4) != strcmpEqual)
		return ntrb_AudioHeaderFromWAVFile_invalid_WAVE_ID;
		
	const size_t Subchunk1Start = ntrb_getSubchunk1Start(fileBuffer, 12);
	if(Subchunk1Start == 0) return ntrb_AudioHeaderFromWAVFile_invalid_fmt_ID;
		
		
	//Subchunk1 - Audiodata information
	const uint32_t Subchunk1Size = *(uint32_t*)(fileBuffer.ptr + Subchunk1Start + 4);
	if(Subchunk1Size >= ChunkSize) return ntrb_AudioHeaderFromWAVFile_invalid_Subchunk1Size;
	
	const uint16_t WAV_AudioFormat = *(uint16_t*)(fileBuffer.ptr + Subchunk1Start + 8);
	returnArg->NumChannels = *(uint16_t*)(fileBuffer.ptr + Subchunk1Start + 10);
	returnArg->SampleRate = *(uint32_t*)(fileBuffer.ptr + Subchunk1Start + 12);
	returnArg->BitsPerSample = *(uint16_t*)(fileBuffer.ptr + Subchunk1Start + 22);
	returnArg->AudioFormat = ntrb_WAV_PaSampleFormat(WAV_AudioFormat, returnArg->BitsPerSample);
		
		
	//Subchunk2 - Audiodata
	const size_t Subchunk2Start = ntrb_getSubchunk2Start(fileBuffer, Subchunk1Start + 24);
	if(Subchunk2Start == 0) return ntrb_AudioHeaderFromWAVFile_invalid_data_ID;
	
	*audiodataSize = *(uint32_t*)(fileBuffer.ptr + Subchunk2Start + 4);
	if(*audiodataSize + 8 + Subchunk2Start > fileBuffer.elem) return ntrb_AudioHeaderFromWAVFile_invalid_Subchunk2Size;
	
	*audiodataOffset = Subchunk2Start + 8;
	return ntrb_AudioHeaderFromWAVFile_ok;
}

ntrb_AudioDatapoints ntrb_get_WAV_audiodata(const ntrb_SpanU8 wavfile, const size_t audiodata_size, const size_t audiodata_offset){
	ntrb_AudioDatapoints audiodata = ntrb_AudioDatapoints_new(audiodata_size * sizeof(uint8_t));
	if(audiodata.bytes == NULL) return audiodata;
	
	memcpy(audiodata.bytes, wavfile.ptr + audiodata_offset, audiodata_size);
	return audiodata;	
}