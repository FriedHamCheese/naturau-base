#ifndef ntrb_RuntimeCoreData_h
#define ntrb_RuntimeCoreData_h

#include "AudioDatapoints.h"

#include <stdint.h>
#include <stdatomic.h>

#include <pthread.h>

typedef struct{
	//these two should be locked with audio_track_rwlock when attempting to read or write
	ntrb_AudioDatapoints** audio_tracks;
	uint16_t audio_track_count;
	
	pthread_rwlock_t audio_track_rwlock;		
	
	//no conditional exchange, can be primtive type = atomic_bool
	atomic_bool requested_exit;
	
	//no conditional exchange, can be primtive type = atomic_bool	
	atomic_bool in_pause_state;
} ntrb_RuntimeCoreData;

enum ntrb_RCD_QueueAudioReturn{
	ntrb_RCD_QueueAudio_OK,
	ntrb_RCD_QueueAudio_MallocError,
	ntrb_RCD_QueueAudio_SleepInterrupt,
	ntrb_RCD_QueueAudio_TracksAllFull,
	
	ntrb_RCD_QueueAudio_ntrb_LoadStdFmtAudioResult = 15,
};

//from RuntimeCoreData.c
extern const ntrb_RuntimeCoreData failed_ntrb_RuntimeCoreData;

ntrb_RuntimeCoreData ntrb_RuntimeCoreData_new(const uint16_t track_count);
void ntrb_RuntimeCoreData_free(ntrb_RuntimeCoreData* const rcd);

void ntrb_RuntimeCoreData_free_track(ntrb_RuntimeCoreData* const rcd, const size_t track_index);

enum ntrb_RCD_QueueAudioReturn ntrb_RuntimeCoreData_queue_audio(ntrb_RuntimeCoreData* const runtime_data, const char* filename);

#endif