#ifndef ntrb_AudioHeader_h
#define ntrb_AudioHeader_h

#include "portaudio.h"
#include <stdint.h>
#include <stdio.h>

typedef struct{
	PaSampleFormat AudioFormat;
	uint32_t SampleRate;
	uint16_t NumChannels;
	uint16_t BitsPerSample;
} ntrb_AudioHeader;

void ntrb_AudioHeader_print(const ntrb_AudioHeader header, FILE* const printstream);

#endif