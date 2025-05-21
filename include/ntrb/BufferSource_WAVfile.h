#ifndef ntrb_BufferSource_WAVfile_h
#define ntrb_BufferSource_WAVfile_h

/*
Copyright 2024 Pawikan Boonnaum

This file is part of naturau-base/ntrb.

naturau-base/ntrb is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

naturau-base/ntrb is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with naturau-base/ntrb. If not, see <https://www.gnu.org/licenses/>.
*/


/**
\file BufferSource_WAVfile.h
A module providing audio loading from WAV file to ntrb_AudioBuffer.
*/
#include "export.h"
#include "AudioHeader.h"

#include <stdio.h>
#include <stdint.h>

/**
A struct containing variables required to keep track of the WAV file audio loading process.
*/
typedef struct{
	///Unprocessed audio read from the WAV file.
	uint8_t* read_bytes;
	
	/**
	The amount of bytes of unprocessed audio to read from the WAV file in a single loading call.
	
	This is also the size of ntrb_BufferSource_WAVfile.read_bytes in bytes. 
	The ntrb_BufferSource_WAVfile_load_buffer() function prefills the buffer in every call with zeroes before reading from the file,
	guaranteeing no garbage is sent for processing the audio to standard audio format.
	*/
	size_t bytes_to_read;
	
	///The file descriptor of the WAV file.
	FILE* aud_file;
	
	///Audio encoding information of the WAV file.
	ntrb_AudioHeader aud_header;
	
	/**
	\todo !!!
	\attention nope
	It initially contains the amount of bytes of audio data in the file.
	But in every call of ntrb_BufferSource_WAVfile_load_buffer(), this gets reduced by ntrb_BufferSource_WAVfile.bytes_to_read.
	If this reaches 0 during the subtraction in the function, it indicates and end of file and the function stops reading the file.
	*/
	size_t audiodataSize;
	size_t audiodataOffset;
	
	float file_samplerate_over_stdaud;
} ntrb_BufferSource_WAVfile;

#ifdef __cplusplus
extern "C"{	
#endif

/**
Initialises an uninitialised ntrb_BufferSource_WAVfile object in *ret.

\param[out] ret The object to initialise.
\param[in]  filename The filename of the WAV file to read from.
\param[in]  frame_count The amount of frames in standard audio format which the source has to write to its ntrb_AudioBuffer.

\return values in enum ntrb_AudioBufferNew_Error reporting an error.
*/
NTRB_DLL_VISIBILITY int ntrb_BufferSource_WAVfile_new(ntrb_BufferSource_WAVfile* const ret, const char* const filename, const size_t frame_count);

/**
Frees resources in the ntrb_BufferSource_WAVfile object.

\return values in enum ntrb_AudioBufferFree_Error reporting an error.
*/
NTRB_DLL_VISIBILITY int ntrb_BufferSource_WAVfile_free(ntrb_BufferSource_WAVfile* const ret);

/**
Reads the header within the first 65535 bytes of the file, writes the read header to the ntrb_AudioHeader in ret, 
and seek the position of the file to the beginning of the audio data.

- If the header is invalid within said range, the function returns ntrb_AudioBufferNew_FileReadError.
- If the file seek failed, the header will be written to the ntrb_AudioHeader in ret and the function returns ntrb_AudioBufferNew_FileReadError.

\param ret An ntrb_BufferSource_WAVfile, with its FILE* initialised, to write the read header to its ntrb_AudioHeader.
\return values in enum ntrb_AudioBufferNew_Error representing errors.
*/
NTRB_DLL_VISIBILITY int ntrb_BufferSource_WAVfile_load_header(ntrb_BufferSource_WAVfile* const ret);

/**
Reads the unprocessed audio from the WAV file, processes it to be in standard audio format, 
and writes it to void_ntrb_AudioBuffer which will be casted to an ntrb_AudioBuffer.

The void* is for pthread_t. The function returns NULL both in success and errors.
*/
NTRB_DLL_VISIBILITY void* ntrb_BufferSource_WAVfile_load_buffer(void* const void_ntrb_AudioBuffer);

#ifdef __cplusplus
};
#endif

#endif