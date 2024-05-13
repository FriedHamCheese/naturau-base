#ifndef ntrb_BufferSource_FLACfile_h
#define ntrb_BufferSource_FLACfile_h

#include "AudioHeader.h"

#include "FLAC/stream_decoder.h"

#include <stdio.h>
#include <stdint.h>

typedef struct{
	FLAC__StreamDecoder* decoder;
	uint8_t* read_bytes;
	uint64_t buffersize_bytes;
	
	uint64_t bytes_in_buffer;
	uint64_t current_frame;
	
	ntrb_AudioHeader aud_header;
} ntrb_BufferSource_FLACfile;

int ntrb_BufferSource_FLACfile_new(void* const void_ntrb_AudioBuffer, const char* const filename, const size_t frame_count);
void ntrb_BufferSource_FLACfile_free(ntrb_BufferSource_FLACfile* const ret);

void* ntrb_BufferSource_FLACfile_load_buffer(void* const void_ntrb_AudioBuffer);

#endif