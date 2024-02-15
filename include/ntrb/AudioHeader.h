#ifndef ntrb_AudioHeader_h
#define ntrb_AudioHeader_h

#include "portaudio.h"
#include <stdint.h>
#include <stdio.h>

typedef struct{
	//"unsigned long", probably uint32_t.
	PaSampleFormat AudioFormat;
	uint32_t AudioDataSize;
	uint32_t SampleRate;
	uint32_t ByteRate;
	uint16_t NumChannels;
	uint16_t BlockAlign;
	uint16_t BitsPerSample;
} ntrb_AudioHeader;

void print_ntrb_AudioHeader(const ntrb_AudioHeader header, FILE* const printstream);

#endif