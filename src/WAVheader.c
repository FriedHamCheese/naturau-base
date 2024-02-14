#include "WAVheader.h"

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

bool ntrb_WAVheader_equal(const ntrb_WAVheader* const a, const ntrb_WAVheader* const b){
	const int memcmp_equal = 0;
	if(memcmp(a->ChunkID, b->ChunkID, sizeof(uint8_t) * 4) != memcmp_equal) return false;
	if(a->ChunkSize != b->ChunkSize) return false;
	if(memcmp(a->Format, b->Format, sizeof(uint8_t) * 4) != memcmp_equal) return false;
	
	if(memcmp(a->Subchunk1ID, b->Subchunk1ID, sizeof(uint8_t) * 4) != memcmp_equal) return false;
	if(a->ChunkSize != b->ChunkSize) return false;
	if(a->Subchunk1Size != b->Subchunk1Size) return false;
	if(a->AudioFormat != b->AudioFormat) return false;
	if(a->NumChannels != b->NumChannels) return false;
	if(a->SampleRate != b->SampleRate) return false;
	if(a->ByteRate != b->ByteRate) return false;
	if(a->BlockAlign != b->BlockAlign) return false;
	if(a->BitsPerSample != b->BitsPerSample) return false;
		
	if(memcmp(a->Subchunk2ID, b->Subchunk2ID, sizeof(uint8_t) * 4) != memcmp_equal) return false;
	if(a->Subchunk2Size != b->Subchunk2Size) return false;
	
	return true;
}