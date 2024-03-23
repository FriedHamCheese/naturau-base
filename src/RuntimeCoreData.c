#include "RuntimeCoreData.h"

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

int ntrb_RuntimeCoreData_new(ntrb_RuntimeCoreData* const rcd, const uint16_t track_count){	
	if(track_count == 0) return ENOMEM;
	rcd->audio_tracks = calloc(track_count, sizeof(ntrb_AudioDatapoints*));
	if(rcd->audio_tracks == NULL) return ENOMEM;
	
	rcd->audio_track_count = track_count;
	rcd->requested_exit = false;
	rcd->in_pause_state = true;
	
	return pthread_rwlock_init(&(rcd->audio_track_rwlock), NULL);
}

int ntrb_RuntimeCoreData_free(ntrb_RuntimeCoreData* const rcd){
	pthread_rwlock_wrlock(&(rcd->audio_track_rwlock));
	
	if(rcd->audio_tracks != NULL){
		for(size_t i = 0; i < rcd->audio_track_count; i++){			
			if(rcd->audio_tracks[i] != NULL)
				ntrb_RuntimeCoreData_free_track(rcd, i);
		}
		free(rcd->audio_tracks);
		rcd->audio_tracks = NULL;
	}	
	rcd->audio_track_count = 0;
	
	pthread_rwlock_unlock(&(rcd->audio_track_rwlock));
	
	rcd->requested_exit = true;
	rcd->in_pause_state = true;
	
	return pthread_rwlock_destroy(&(rcd->audio_track_rwlock));
}

void ntrb_RuntimeCoreData_free_track(ntrb_RuntimeCoreData* const rcd, const size_t track_index){
	free(rcd->audio_tracks[track_index]->bytes);
	free(rcd->audio_tracks[track_index]);
	rcd->audio_tracks[track_index] = NULL;
}

enum ntrb_RCD_QueueAudioReturn ntrb_RuntimeCoreData_queue_audio(ntrb_RuntimeCoreData* const runtime_data, const char* filename){
	ntrb_AudioDatapoints* aud = malloc(sizeof(ntrb_AudioDatapoints));
	if(aud == NULL) return ntrb_RCD_QueueAudio_MallocError;
	
	enum ntrb_LoadStdFmtAudioResult audio_load_result = ntrb_load_std_fmt_audio(aud, filename);
	if(audio_load_result != ntrb_LoadStdFmtAudioResult_OK){
		free(aud);
		return ntrb_RCD_QueueAudio_ntrb_LoadStdFmtAudioResult + audio_load_result;
	}
	
	bool wrote_track = false;
	
	pthread_rwlock_wrlock(&(runtime_data->audio_track_rwlock));
	
	for(uint16_t i = 0; i < runtime_data->audio_track_count; i++){
		const bool track_empty = runtime_data->audio_tracks[i] == NULL;
		if(track_empty){
			runtime_data->audio_tracks[i] = aud;
			wrote_track = true;
			break;
		}
	}
	
	pthread_rwlock_unlock(&(runtime_data->audio_track_rwlock));
	if(!wrote_track) return ntrb_RCD_QueueAudio_TracksAllFull;
	else return ntrb_RCD_QueueAudio_OK;
}