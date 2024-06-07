#include "RuntimeCoreData.h"

#include "alloc.h"
#include "aud_std_fmt.h"
#include "audeng_wrapper.h"
#include "AudioDatapoints.h"

#include <unistd.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdatomic.h>

#include <pthread.h>

const ntrb_RuntimeCoreData failed_ntrb_RuntimeCoreData = {
	.audio_tracks=NULL,
	.audio_track_count = 0,
	.requested_exit = true,
	.in_pause_state = true
};

enum ntrb_RuntimeCoreData_Error ntrb_RuntimeCoreData_new(ntrb_RuntimeCoreData* const rcd, const uint16_t track_count){
	if(track_count == 0) return ntrb_RuntimeCoreData_AllocError;
	
	rcd->audio_tracks = ntrb_calloc(track_count, sizeof(ntrb_AudioBuffer*));
	if(rcd->audio_tracks == NULL) 
		return ntrb_RuntimeCoreData_AllocError;
	
	rcd->audio_track_count = track_count;
	rcd->requested_exit = false;
	rcd->in_pause_state = true;
	
	const int rwlock_init_error = pthread_rwlock_init(&(rcd->audio_track_rwlock), NULL);
	if(rwlock_init_error){
		ntrb_free(rcd->audio_tracks);
		return ntrb_RuntimeCoreData_RwlockInitError;
	}
	
	return ntrb_RuntimeCoreData_OK;
}

enum ntrb_RuntimeCoreData_Error ntrb_RuntimeCoreData_free(ntrb_RuntimeCoreData* const rcd){
	const int acq_writelock_error = pthread_rwlock_wrlock(&(rcd->audio_track_rwlock));
	if(acq_writelock_error) return ntrb_RuntimeCoreData_AcqWritelockError;
	
	if(rcd->audio_tracks != NULL){
		for(uint16_t i = 0; i < rcd->audio_track_count; i++){
			if(rcd->audio_tracks[i] == NULL) continue;
			
			const int free_track_error = ntrb_RuntimeCoreData_free_track(rcd, i);
			if(free_track_error)
				fprintf(stderr, "[Error]: ntrb_RuntimeCoreData_free(): unable to free track %hu (ntrb_AudioBufferFree_Error: %d).", i, free_track_error);
		}
		
		ntrb_free(rcd->audio_tracks);
		rcd->audio_tracks = NULL;
	}
	
	rcd->audio_track_count = 0;
	rcd->requested_exit = true;
	rcd->in_pause_state = true;	

	const int rwlock_unlock_error = pthread_rwlock_unlock(&(rcd->audio_track_rwlock));
	if(rwlock_unlock_error) return ntrb_RuntimeCoreData_RwlockUnlockError;

	const int rwlock_destroy_error = pthread_rwlock_destroy(&(rcd->audio_track_rwlock));
	if(rwlock_destroy_error) return ntrb_RuntimeCoreData_RwlockDestroyError;
	
	return ntrb_RuntimeCoreData_OK;	
}

enum ntrb_AudioBufferFree_Error ntrb_RuntimeCoreData_free_track(ntrb_RuntimeCoreData* const rcd, const uint16_t track_index){
	const enum ntrb_AudioBufferFree_Error aud_free_error = ntrb_AudioBuffer_free(rcd->audio_tracks[track_index]);
	ntrb_free(rcd->audio_tracks[track_index]);
	rcd->audio_tracks[track_index] = NULL;
	
	return aud_free_error;
}

enum ntrb_RCD_QueueAudioReturn ntrb_RuntimeCoreData_queue_audio(ntrb_RuntimeCoreData* const runtime_data, const char* filename){
	ntrb_AudioBuffer* aud = ntrb_malloc(sizeof(ntrb_AudioBuffer));
	if(aud == NULL) return ntrb_RCD_QueueAudio_MallocError;
	
	const enum ntrb_AudioBufferNew_Error new_audbuf_error = ntrb_AudioBuffer_new(aud, filename, ntrb_std_frame_count);
	if(new_audbuf_error){
		ntrb_free(aud);
		return ntrb_AudioBufferNew_Error_to_ntrb_RCD_QueueAudioReturn(new_audbuf_error);
	}
	
	aud->load_buffer_callback(aud);
	const enum ntrb_RCD_QueueAudioReturn aud_load_err = ntrb_AudioBufferLoad_Error_to_ntrb_RCD_QueueAudioReturn(aud->load_err);
	if(aud_load_err){
		ntrb_AudioBuffer_free(aud);
		ntrb_free(aud);
		return aud_load_err;
	}
	
	const int acq_wrlock_error = pthread_rwlock_wrlock(&(runtime_data->audio_track_rwlock));
	if(acq_wrlock_error){
		ntrb_AudioBuffer_free(aud);
		ntrb_free(aud);		
		return ntrb_RCD_QueueAudio_AcqWritelockError;
	}
	
	bool wrote_track = false;	
	for(uint16_t i = 0; i < runtime_data->audio_track_count; i++){
		const bool track_empty = runtime_data->audio_tracks[i] == NULL;
		if(track_empty){
			runtime_data->audio_tracks[i] = aud;
			wrote_track = true;
			break;
		}
	}

	if(!wrote_track){
		pthread_rwlock_unlock(&(runtime_data->audio_track_rwlock));
		ntrb_AudioBuffer_free(aud);
		ntrb_free(aud);
		return ntrb_RCD_QueueAudio_TracksAllFull;
	}
	
	const int rwlock_unlock_error = pthread_rwlock_unlock(&(runtime_data->audio_track_rwlock));
	if(rwlock_unlock_error) return ntrb_RCD_QueueAudio_RwlockUnlockError;
	
	return ntrb_RCD_QueueAudio_OK;
}

enum ntrb_RCD_QueueAudioReturn ntrb_AudioBufferNew_Error_to_ntrb_RCD_QueueAudioReturn(const enum ntrb_AudioBufferNew_Error arg){
	switch(arg){
		case ntrb_AudioBufferNew_AllocError: 			return ntrb_RCD_QueueAudio_MallocError;
		case ntrb_AudioBufferNew_FileOpenError:			return ntrb_RCD_QueueAudio_FileOpenError;
		case ntrb_AudioBufferNew_FileReadError:		 	return ntrb_RCD_QueueAudio_FileReadError;
		case ntrb_AudioBufferNew_InvalidAudFiletype: 	return ntrb_RCD_QueueAudio_InvalidAudFiletype;
		case ntrb_AudioBufferNew_WAVheaderError: 		return ntrb_RCD_QueueAudio_WAVheaderError;
		case ntrb_AudioBufferNew_RwlockInitError: 		return ntrb_RCD_QueueAudio_RwlockInitError;
		case ntrb_AudioBufferNew_FLACcontainerError:	return ntrb_RCD_QueueAudio_FLACcontainerError;
		case ntrb_AudioBufferNew_InvalidAudFormat:		return ntrb_RCD_QueueAudio_InvalidAudFormat;
		case ntrb_AudioBufferNew_UnknownError:			return ntrb_RCD_QueueAudio_NewAudUnknownError;
		
		default:
		return ntrb_RCD_QueueAudio_OK;
	}
}

enum ntrb_RCD_QueueAudioReturn ntrb_AudioBufferLoad_Error_to_ntrb_RCD_QueueAudioReturn(const enum ntrb_AudioBufferLoad_Error arg){
	switch(arg){
		case ntrb_AudioBufferLoad_AcqWritelockError:	return ntrb_RCD_QueueAudio_AcqWritelockError;
		case ntrb_AudioBufferLoad_UnequalRead:			return ntrb_RCD_QueueAudio_WAVUnequalRead;
		
		case ntrb_AudioBufferLoad_StdaudConversionError:
			return ntrb_RCD_QueueAudio_StdaudConversionError;
		default:
			return ntrb_RCD_QueueAudio_OK;
	}
}