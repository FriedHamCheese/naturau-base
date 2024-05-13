#ifndef ntrb_BufferSource_WAVfile_h
#define ntrb_BufferSource_WAVfile_h

#include "AudioHeader.h"

#include <stdio.h>
#include <stdint.h>

typedef struct{
	uint8_t* read_bytes;
	size_t bytes_to_read;
	FILE* aud_file;
	
	ntrb_AudioHeader aud_header;
	size_t audiodataOffset;
	size_t audiodataSize;
} ntrb_BufferSource_WAVfile;

int ntrb_BufferSource_WAVfile_new(ntrb_BufferSource_WAVfile* const ret, const char* const filename, const size_t frame_count);
int ntrb_BufferSource_WAVfile_free(ntrb_BufferSource_WAVfile* const ret);

int ntrb_BufferSource_WAVfile_load_header(ntrb_BufferSource_WAVfile* const ret);
void* ntrb_BufferSource_WAVfile_load_buffer(void* const void_ntrb_AudioBuffer);


#endif