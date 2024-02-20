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


const ntrb_RuntimeCoreData failed_ntrb_RuntimeCoreData = {
	.audio_tracks=NULL,
	.audio_track_count = 0,
	.requested_exit = true,
	.in_pause_state = true,
	.streaming_audio = true,
	.writing_tracks = true
};

static const unsigned long wait_busy_stream_usecs = 10 * 1000;


ntrb_RuntimeCoreData new_ntrb_RuntimeCoreData(const uint16_t track_count){
	ntrb_RuntimeCoreData rcd;
	
	if(track_count == 0) return failed_ntrb_RuntimeCoreData;
	rcd.audio_tracks = calloc(track_count, sizeof(ntrb_AudioDatapoints*));
	if(rcd.audio_tracks == NULL) return failed_ntrb_RuntimeCoreData;
	
	rcd.audio_track_count = track_count;
	rcd.requested_exit = false;
	rcd.in_pause_state = true;
	rcd.streaming_audio = false;
	rcd.writing_tracks = false;
	
	return rcd;
}

void free_ntrb_RuntimeCoreData(ntrb_RuntimeCoreData* const rcd){
	if(rcd->audio_tracks != NULL){
		for(size_t i = 0; i < rcd->audio_track_count; i++){			
			if(rcd->audio_tracks[i] != NULL)
				free_ntrb_RuntimeCoreData_track(rcd, i);
		}
		free(rcd->audio_tracks);
		rcd->audio_tracks = NULL;
	}
	
	rcd->audio_track_count = 0;
	rcd->requested_exit = true;
	rcd->in_pause_state = true;
	rcd->streaming_audio = false;
	rcd->writing_tracks = false;
}

void free_ntrb_RuntimeCoreData_track(ntrb_RuntimeCoreData* const rcd, const size_t track_index){
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
	
	while(runtime_data->streaming_audio || runtime_data->writing_tracks){
		if(usleep(wait_busy_stream_usecs) == EINTR){
			free(aud->bytes);
			free(aud);
			return ntrb_RCD_QueueAudio_SleepInterrupt;
		}
	}
	
	bool wrote_track = false;
	runtime_data->writing_tracks = true;
	
	for(uint16_t i = 0; i < runtime_data->audio_track_count; i++){
		const bool track_empty = runtime_data->audio_tracks[i] == NULL;
		if(track_empty){
			runtime_data->audio_tracks[i] = aud;
			wrote_track = true;
			break;
		}
	}
	
	runtime_data->writing_tracks = false;
	if(!wrote_track) return ntrb_RCD_QueueAudio_TracksAllFull;
	else return ntrb_RCD_QueueAudio_OK;
}