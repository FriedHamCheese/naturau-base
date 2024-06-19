#ifndef ntrb_AudioHeader_h
#define ntrb_AudioHeader_h

/*
Copyright 2024 Pawikan Boonnaum

This file is part of naturau-base/ntrb.

naturau-base/ntrb is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

naturau-base/ntrb is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with naturau-base/ntrb. If not, see <https://www.gnu.org/licenses/>.
*/


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
#ifdef __cplusplus
extern "C"{
#endif

 void ntrb_AudioHeader_print(const ntrb_AudioHeader header, FILE* const printstream);
 
#ifdef __cplusplus
};
#endif

#endif