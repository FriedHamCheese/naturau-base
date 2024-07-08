#ifndef ntrb_BufferSource_FLACfile_h
#define ntrb_BufferSource_FLACfile_h

/*
Copyright 2024 Pawikan Boonnaum

This file is part of naturau-base/ntrb.

naturau-base/ntrb is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

naturau-base/ntrb is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with naturau-base/ntrb. If not, see <https://www.gnu.org/licenses/>.
*/


/**
\file BufferSource_FLACfile.h

A module providing audio loading from FLAC file to ntrb_AudioBuffer.

\note FLAC frame differs from our frame definition.
  FLAC frame means an encoded data chunk, while our frame means a single smallest interval of audio.
*/

#include "AudioHeader.h"

#include "FLAC/stream_decoder.h"

#include <stdio.h>
#include <stdint.h>

/**
A struct containing variables required for loading audio from FLAC file.

The struct contains ntrb_BufferSource_FLACfile.read_bytes which contains the raw bytes decoded from ntrb_BufferSource_FLACfile.decoder.
The ntrb_BufferSource_FLACfile_load_buffer() function will keep decoding the FLAC frames until the buffer is full,
which is indicated by ntrb_BufferSource_FLACfile.bytes_in_buffer being equal to ntrb_BufferSource_FLACfile.buffersize_bytes.

ntrb_BufferSource_FLACfile.current_frame keeps track of the audio frame we have read to,
so ntrb_BufferSource_FLACfile_load_buffer() can jump to it and decode a partial or full FLAC frame.
*/
typedef struct{
	///The decoder object of the FLAC file.
	FLAC__StreamDecoder* decoder;
	/**
	A buffer containing the bytes of audio data read directly from the decoder. 
	It has the size of ntrb_BufferSource_FLACfile.buffersize_bytes bytes.
	*/
	uint8_t* read_bytes;
	///The size of ntrb_BufferSource_FLACfile.read_bytes.
	uint64_t buffersize_bytes;
	///Keeps track of how many bytes read_bytes currently contain. 
	uint64_t bytes_in_buffer;
	///This contains the information of the FLAC file from its metadata.
	ntrb_AudioHeader aud_header;
	
	float file_samplerate_over_stdaud;
} ntrb_BufferSource_FLACfile;

#ifdef __cplusplus
extern "C"{	
#endif

/**
A function redirecting values in FLAC__StreamDecoderState to ntrb_AudioBufferLoad_Error values.

See the redirected equivalents in BufferSource_FLACfile.c
*/
int FLAC__StreamDecoderState_to_ntrb_AudioBufferLoad_Error(const FLAC__StreamDecoderState arg);

/**
Initialises the ntrb_BufferSource_FLACfile in void_ntrb_AudioBuffer once casted to ntrb_AudioBuffer,
decodes the STREAMINFO part of the FLAC metadata of the file and writes it to ntrb_BufferSource_FLACfile in ntrb_AudioBuffer.

\param[out] void_ntrb_AudioBuffer The ntrb_AudioBuffer object which the function will write to its ntrb_BufferSource_FLACfile.
\param[in]  filename The filename of the FLAC file to read from.
\param[in]  frame_count The amount of standard format audio frames which the ntrb_BufferSource_FLACfile needs to write to its ntrb_AudioBuffer.

\returns values in enum ntrb_AudioBufferNew_Error representing errors while initialising the object.

\todo why is this taking the entire ntrb_AudioBuffer when it only writes to itself in it? - 0.3 fix ok?
*/
int ntrb_BufferSource_FLACfile_new(void* const void_ntrb_AudioBuffer, const char* const filename, const size_t frame_count);

///Frees resources in ret.
void ntrb_BufferSource_FLACfile_free(ntrb_BufferSource_FLACfile* const ret);

/**
Loads unprocessed audio from FLAC file and writes the processed audio to void_ntrb_AudioBuffer casted to ntrb_AudioBuffer type.
*/
void* ntrb_BufferSource_FLACfile_load_buffer(void* const void_ntrb_AudioBuffer);

#ifdef __cplusplus
};
#endif

#endif