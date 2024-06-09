/*
Copyright 2024 Pawikan Boonnaum

This file is part of naturau-base/ntrb.

naturau-base/ntrb is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

naturau-base/ntrb is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with naturau-base/ntrb. If not, see <https://www.gnu.org/licenses/>.
*/

#include "AudioBuffer.h"

#include "BufferSource_WAVfile.h"
#include "BufferSource_FLACfile.h"

#include "alloc.h"
#include "str_utils.h"
#include "aud_std_fmt.h"

#include <pthread.h>

#include <string.h>
#include <stdbool.h>

const ntrb_AudioBuffer failed_ntrb_AudioBuffer = {
	.datapoints = NULL,
};

enum ntrb_AudioBufferNew_Error ntrb_AudioBuffer_new(ntrb_AudioBuffer* const ret, const char* const filename, const size_t stdaud_frame_count){
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
		source_init_error = ntrb_BufferSource_WAVfile_new(&(ret->source.wav_file), filename, stdaud_frame_count);
	}
	else if(strcmp(filetype, "flac") == 0){
		ret->load_buffer_callback = ntrb_BufferSource_FLACfile_load_buffer;
		source_init_error = ntrb_BufferSource_FLACfile_new(ret, filename, stdaud_frame_count);
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
	
	ret->monochannel_samples = stdaud_frame_count;
	const size_t stereo_samples = stdaud_frame_count * ntrb_std_audchannels;
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
	
	if(obj->load_buffer_callback == ntrb_BufferSource_FLACfile_load_buffer){
		ntrb_BufferSource_FLACfile_free(&(obj->source.flac_file));
	}
	
	const int unlock_rwlock_error = pthread_rwlock_unlock(&(obj->buffer_access));
	if(unlock_rwlock_error) return ntrb_AudioBufferFree_RwlockUnlockError;
		
	const int destroy_rwlock_error = pthread_rwlock_destroy(&(obj->buffer_access));
	if(destroy_rwlock_error) return ntrb_AudioBufferFree_RwlockDestroyError;
	
	return ntrb_AudioBufferFree_OK;
}