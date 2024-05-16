#ifndef ntrb_RuntimeCoreData_h
#define ntrb_RuntimeCoreData_h

/**
\file RuntimeCoreData.h
A module for interacting with ntrb_RuntimeCoreData.
*/

#include "AudioBuffer.h"

#include <stdint.h>
#include <stdatomic.h>

#include <pthread.h>

/**
A struct containing loaded audio tracks and state of the audio engine loop.

If the struct indicates failure from a function or memory allocation, the audio_tracks will be NULL.
*/
typedef struct{
	///An allocated pointer which points to ntrb_AudioDatapoints.
	///The datapoints (tracks) which the pointer points to should be ready for deallocation when the datapoints are played through. And should not be in the stack.
	///In order to access this, you should request a lock from audio_track_rwlock.
	ntrb_AudioBuffer** audio_tracks;
	
	///The number of tracks which audio_tracks hold.
	///In order to access this, you should request a lock from audio_track_rwlock.	
	uint16_t audio_track_count;
	
	///The rwlock for accessing audio_tracks and audio_track_count.
	pthread_rwlock_t audio_track_rwlock;		
	
	///A boolean which tells the audio engine loop to start uninitialising.
	///You can read or write to this, but don't do logical check then write.
	atomic_bool requested_exit;
	
	///A boolean which tells the audio engine loop to be in a paused state.
	///You can read or write to this, but don't do logical check then write.
	atomic_bool in_pause_state;
} ntrb_RuntimeCoreData;

enum ntrb_RuntimeCoreData_Error{
	ntrb_RuntimeCoreData_OK,
	ntrb_RuntimeCoreData_AllocError,
	ntrb_RuntimeCoreData_RwlockInitError,
	
	ntrb_RuntimeCoreData_AcqWritelockError,
	ntrb_RuntimeCoreData_RwlockUnlockError,
	ntrb_RuntimeCoreData_RwlockDestroyError,
};

/**
Enum for errors while loading (queueing) the audio track.
*/
enum ntrb_RCD_QueueAudioReturn{
	///Loaded the audio to the track.
	ntrb_RCD_QueueAudio_OK,
	///Memory allocation error.
	ntrb_RCD_QueueAudio_MallocError,
	///No empty track to add audio to.
	ntrb_RCD_QueueAudio_TracksAllFull,
	
	ntrb_RCD_QueueAudio_FileOpenError,
	ntrb_RCD_QueueAudio_FileReadError,
	ntrb_RCD_QueueAudio_InvalidAudFiletype,
	ntrb_RCD_QueueAudio_WAVheaderError,	
	ntrb_RCD_QueueAudio_FLACcontainerError,
	ntrb_RCD_QueueAudio_InvalidAudFormat,
	ntrb_RCD_QueueAudio_NewAudUnknownError,	

	ntrb_RCD_QueueAudio_StdaudConversionError,
	ntrb_RCD_QueueAudio_WAVUnequalRead,
	ntrb_RCD_QueueAudio_FLACBadData,

	ntrb_RCD_QueueAudio_RwlockInitError,
	ntrb_RCD_QueueAudio_AcqWritelockError,
	ntrb_RCD_QueueAudio_RwlockUnlockError,
	
	ntrb_RCD_QueueAudio_UnknownError,
};

enum ntrb_RCD_QueueAudioReturn ntrb_AudioBufferNew_Error_to_ntrb_RCD_QueueAudioReturn(const enum ntrb_AudioBufferNew_Error arg);
enum ntrb_RCD_QueueAudioReturn ntrb_AudioBufferLoad_Error_to_ntrb_RCD_QueueAudioReturn(const enum ntrb_AudioBufferLoad_Error arg);

/**
A value for returning an object which indicates a failure.

Defined in RuntimeCoreData.c
*/
extern const ntrb_RuntimeCoreData failed_ntrb_RuntimeCoreData;

/**
Creates and initialises a new object to *rcd. 

- rcd->audio_tracks will be allocated to hold (track_count) ntrb_AudioDatapoints pointers.
- rcd->audio_tracks be set to (track_count).
- rcd->request_exit be set to false.
- rcd->in_pause_state be set to true.

Returns 0 if sucessfully initialised rcd. If it failed to allocate memory, ENOMEM will be returned.
Else, the value is from pthread_rwlock_init().

*/
enum ntrb_RuntimeCoreData_Error ntrb_RuntimeCoreData_new(ntrb_RuntimeCoreData* const rcd, const uint16_t track_count);

/**
Frees each of the tracks in rcd->audio_tracks if it isn't NULL, free rcd->audio_tracks itself, 
set rcd->request_exit to true, set rcd->in_pause_state to true and destroys the rwlock of the object.

The function can handle rcd->audio_tracks and its tracks being NULL, and will proceed to set the booleans and destroy the rwlock.

\return return value of pthread_rwlock_destroy(). If it isn't 0, the function couldn't destroy the rwlock, which is fine in most cases.
*/
enum ntrb_RuntimeCoreData_Error ntrb_RuntimeCoreData_free(ntrb_RuntimeCoreData* const rcd);

/**
Frees a track of *rcd which is in rcd->audio_tracks[track_index] and sets its track pointer to NULL.

track_index should be a value between 0 to rcd->audio_tracks - 1.

It does not perform bounds check or NULL check of the track pointer.
*/
enum ntrb_AudioBufferFree_Error ntrb_RuntimeCoreData_free_track(ntrb_RuntimeCoreData* const rcd, const size_t track_index);

/**
Loads an audio track from an audio file with the provided filename to an empty track in runtime_data.
*/
enum ntrb_RCD_QueueAudioReturn ntrb_RuntimeCoreData_queue_audio(ntrb_RuntimeCoreData* const runtime_data, const char* filename);

#endif