#include "AudioBuffer.h"

#include "alloc.h"
#include "str_utils.h"
#include "aud_std_fmt.h"

#include <pthread.h>

#include <string.h>
#include <stdbool.h>

const ntrb_AudioBuffer failed_ntrb_AudioBuffer = {
	.datapoints = NULL,
};

enum ntrb_AudioBufferNew_Error ntrb_AudioBuffer_new(ntrb_AudioBuffer* const ret, const char* const filename, const size_t monochannel_samples){
	ret->load_err = ntrb_AudioBufferLoad_OK;
	
	//Alloc ret->buffer_access, don't free with pthread_rwlock_destroy() unless an error occurred in this scope.
	const int pthread_rwlock_init_error = pthread_rwlock_init(&(ret->buffer_access), NULL);
	if(pthread_rwlock_init_error) return ntrb_AudioBufferNew_RwlockInitError;
	
	//Alloc filetype, free with ntrb_free()
	char* const filetype = ntrb_get_filetype(filename);
	if(filetype == NULL)
		return ntrb_AudioBufferNew_InvalidAudFiletype;

	enum ntrb_AudioBufferNew_Error source_init_error = ntrb_AudioBufferNew_OK;
	if(strcmp(filetype, "wav") == 0){
		ret->load_buffer_callback = ntrb_BufferSource_WAVfile_load_buffer;
		source_init_error = ntrb_BufferSource_WAVfile_new(&(ret->source.wav_file), filename, monochannel_samples);
	}
	else if(strcmp(filetype, "flac") == 0){
		ret->load_buffer_callback = load_flac_buffer;	
	}
	else{
		source_init_error = ntrb_AudioBufferNew_InvalidAudFiletype;
	}
	
	if(source_init_error){
		pthread_rwlock_destroy(&(ret->buffer_access));
		ntrb_free(filetype);
		return source_init_error;
	}	
	
	//Free filetype
	ntrb_free(filetype);
	
	ret->monochannel_samples = monochannel_samples;
	const size_t stereo_samples = monochannel_samples * ntrb_std_audchannels;
	ret->datapoints = ntrb_calloc(stereo_samples, sizeof(float));
	if(ret->datapoints == NULL){
		ntrb_AudioBuffer_free(ret);
		return ntrb_AudioBufferNew_AllocError;
	}
	
	return ntrb_AudioBufferNew_OK;
}

enum ntrb_AudioBufferFree_Error ntrb_AudioBuffer_free(ntrb_AudioBuffer* const obj){
	const int acq_writelock_error = pthread_rwlock_wrlock(&(obj->buffer_access));
	if(acq_writelock_error) return ntrb_AudioBufferFree_AcqWritelockError;
		
	ntrb_free(obj->datapoints);	
	obj->monochannel_samples = 0;
	
	if(obj->load_buffer_callback == ntrb_BufferSource_WAVfile_load_buffer){
		ntrb_BufferSource_WAVfile_free(&(obj->source.wav_file));
	}
	
	const int unlock_rwlock_error = pthread_rwlock_unlock(&(obj->buffer_access));
	if(unlock_rwlock_error) return ntrb_AudioBufferFree_RwlockUnlockError;
		
	const int destroy_rwlock_error = pthread_rwlock_destroy(&(obj->buffer_access));
	if(destroy_rwlock_error) return ntrb_AudioBufferFree_RwlockDestroyError;
	
	return ntrb_AudioBufferFree_OK;
}

void load_flac_header(void* const void_ntrb_AudioBuffer){
}


void* load_flac_buffer(void* const void_ntrb_AudioBuffer){
	return NULL;
}
