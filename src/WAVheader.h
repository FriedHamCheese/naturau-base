#ifndef ntrb_WAVheader_h
#define ntrb_WAVheader_h

#include <stdint.h>
#include <stdbool.h>

typedef struct{
	uint8_t ChunkID[4];
	uint32_t ChunkSize;
	//64 bits
	
	uint8_t Format[4];
	uint8_t Subchunk1ID[4];
	//64 bits
	
	uint32_t Subchunk1Size;
	uint16_t AudioFormat;
	uint16_t NumChannels;
	//64 bits
	
	uint32_t SampleRate;
	uint32_t ByteRate;
	//64 bits
	
	uint16_t BlockAlign;
	uint16_t BitsPerSample;
	uint8_t Subchunk2ID[4];
	//64 bits
	
	uint32_t Subchunk2Size;
	//padding of 32 bits to align to 64 bits
} ntrb_WAVheader;

bool ntrb_WAVheader_equal(const ntrb_WAVheader* const a, const ntrb_WAVheader* const b);

#endif