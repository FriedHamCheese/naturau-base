#ifndef ntrb_AudioHeader_h
#define ntrb_AudioHeader_h

/**
\file AudioHeader.h
A module providing ntrb_AudioHeader and a function printing it to a FILE*.
*/

#include "portaudio.h"
#include <stdint.h>
#include <stdio.h>

/**
A struct containing the minimum information of an unprocessed audio required for converting to standard audio format of ntrb.
*/
typedef struct{
	PaSampleFormat AudioFormat;		///< The format for a single sample (single mono datapoint) from the unprocessed audio, like an int16, float32, etc...
	uint32_t SampleRate;
	uint16_t NumChannels;
	uint16_t BitsPerSample;			///< How many bits are in a sample, 16 32, etc... 
} ntrb_AudioHeader;

///A function which prints the contents of the provided header to the stream.
void ntrb_AudioHeader_print(const ntrb_AudioHeader header, FILE* const printstream);

#endif