#ifndef ntrb_AudioBuffer_h
#define ntrb_AudioBuffer_h

#include "AudioHeader.h"
#include "BufferSource_WAVfile.h"
#include "BufferSource_FLACfile.h"

#include <pthread.h>

#include <stdio.h>
#include <stdbool.h>

enum ntrb_AudioBufferLoad_Error{
	ntrb_AudioBufferLoad_OK,
	ntrb_AudioBufferLoad_AcqWritelockError,
	ntrb_AudioBufferLoad_UnequalRead,
	ntrb_AudioBufferLoad_EOF,
	ntrb_AudioBufferLoad_StdaudConversionError,
	ntrb_AudioBufferLoad_RwlockUnlockError,
	
	ntrb_AudioBufferLoad_FLACBadData,
};

typedef struct{
	pthread_rwlock_t buffer_access;
	float* datapoints;
	size_t monochannel_samples;
	
	union{
		ntrb_BufferSource_WAVfile wav_file;
		ntrb_BufferSource_FLACfile flac_file;
	} source;
	
	void* (*load_buffer_callback)(void*);
	enum ntrb_AudioBufferLoad_Error load_err;	
} ntrb_AudioBuffer;


enum ntrb_AudioBufferNew_Error{
	ntrb_AudioBufferNew_OK,
	ntrb_AudioBufferNew_AllocError,
	ntrb_AudioBufferNew_FileOpenError,
	ntrb_AudioBufferNew_FileReadError,
	ntrb_AudioBufferNew_InvalidAudFiletype,
	ntrb_AudioBufferNew_WAVheaderError,
	ntrb_AudioBufferNew_RwlockInitError,
	
	ntrb_AudioBufferNew_FLACcontainerError,
	
	ntrb_AudioBufferNew_InvalidAudFormat,
	
	ntrb_AudioBufferNew_UnknownError,
};

enum ntrb_AudioBufferFree_Error{
	ntrb_AudioBufferFree_OK,
	ntrb_AudioBufferFree_AcqWritelockError,
	ntrb_AudioBufferFree_RwlockUnlockError,
	ntrb_AudioBufferFree_RwlockDestroyError,
};

enum ntrb_LoadAudheader_status{
	ntrb_LoadAudheader_OK,
	ntrb_LoadAudheader_FileError,
	
	ntrb_LoadAudheader_WAVHeaderConversionError,
};

enum ntrb_AudioBufferNew_Error ntrb_AudioBuffer_new(ntrb_AudioBuffer* const ret, const char* const filename, const size_t stdaud_frame_count);

enum ntrb_AudioBufferFree_Error ntrb_AudioBuffer_free(ntrb_AudioBuffer* const obj);

void load_flac_header(void* const void_ntrb_AudioBuffer);
void* load_flac_buffer(void* const void_ntrb_AudioBuffer);

#endif