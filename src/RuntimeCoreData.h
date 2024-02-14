#ifndef ntrb_RuntimeCoreData_h
#define ntrb_RuntimeCoreData_h

#include "AudioDatapoints.h"

#include <stdint.h>
#include <stdatomic.h>

typedef struct{
	ntrb_AudioDatapoints** audio_tracks;
	uint16_t audio_track_count;
	
	atomic_bool requested_exit;
	atomic_bool in_pause_state;
	atomic_bool streaming_audio;
	atomic_bool writing_tracks;
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

ntrb_RuntimeCoreData new_ntrb_RuntimeCoreData(const uint16_t track_count);
void free_ntrb_RuntimeCoreData(ntrb_RuntimeCoreData* const rcd);

void free_ntrb_RuntimeCoreData_track(ntrb_RuntimeCoreData* const rcd, const size_t track_index);

enum ntrb_RCD_QueueAudioReturn ntrb_RuntimeCoreData_queue_audio(ntrb_RuntimeCoreData* const runtime_data, const char* filename);

#endif