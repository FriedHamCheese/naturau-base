#ifndef ntrb_RuntimeCoreData_h
#define ntrb_RuntimeCoreData_h

/**
Copyright 2024 Pawikan Boonnaum

This file is part of naturau-base/ntrb.

naturau-base/ntrb is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

naturau-base/ntrb is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with naturau-base/ntrb. If not, see <https://www.gnu.org/licenses/>.
*/


/**
\file RuntimeCoreData.h
A module containing ntrb_RuntimeCoreData and functions for interacting with it.
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
	atomic_bool requested_exit;
	
	///A boolean which tells the audio engine loop to be in a paused state.
	atomic_bool in_pause_state;
} ntrb_RuntimeCoreData;

/**
Generic enums of RuntimeCoreData errors.
*/
enum ntrb_RuntimeCoreData_Error{
	///cool
	ntrb_RuntimeCoreData_OK,
	///Memory allocation failure.
	ntrb_RuntimeCoreData_AllocError,
	///Error initialising the rwlock for the ntrb_RuntimeCoreData.
	ntrb_RuntimeCoreData_RwlockInitError,
	
	///Error acquiring the rwlock of the ntrb_RuntimeCoreData.
	ntrb_RuntimeCoreData_AcqWritelockError,
	///Error unlocking the rwlock of the ntrb_RuntimeCoreData.	
	ntrb_RuntimeCoreData_RwlockUnlockError,
	///Error destroying the rwlock of the ntrb_RuntimeCoreData.	
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
	
	///An error opening a file.
	ntrb_RCD_QueueAudio_FileOpenError,
	///An error reading from a file.	
	ntrb_RCD_QueueAudio_FileReadError,
	///Invalid audio filetype.
	ntrb_RCD_QueueAudio_InvalidAudFiletype,
	///An error interpreting the WAV file header.
	ntrb_RCD_QueueAudio_WAVheaderError,	

	///A redirect of FLAC__STREAM_DECODER_INIT_STATUS_UNSUPPORTED_CONTAINER.
	ntrb_RCD_QueueAudio_FLACcontainerError,
	///Unsupported or invalid audio encoding from a valid audio filetype.
	ntrb_RCD_QueueAudio_InvalidAudFormat,
	///An unknown error from initialising an ntrb_AudioBuffer.
	ntrb_RCD_QueueAudio_NewAudUnknownError,	

	///Any errors from ntrb_StdAudFmtConversionResult while converting the unprocessed audio to standard audio format.
	ntrb_RCD_QueueAudio_StdaudConversionError,
	///The loading callback read the WAV audio data bytes less than it supposed to.
	ntrb_RCD_QueueAudio_WAVUnequalRead,
	///An error mostly from decoding a bad FLAC file.
	///Any FLAC__StreamDecoderErrorStatus caught will result in this being returned.	
	ntrb_RCD_QueueAudio_FLACBadData,

	///An error initialising the rwlock of an ntrb_RuntimeCoreData.
	ntrb_RCD_QueueAudio_RwlockInitError,
	///An error acquiring the rwlock of an ntrb_RuntimeCoreData.	
	ntrb_RCD_QueueAudio_AcqWritelockError,
	///An error unlocking the rwlock of an ntrb_RuntimeCoreData.		
	ntrb_RCD_QueueAudio_RwlockUnlockError,
	///Unspecified errors.	
	ntrb_RCD_QueueAudio_UnknownError,
};

///Converts values in ntrb_AudioBufferNew_Error to values in ntrb_RCD_QueueAudioReturn.
enum ntrb_RCD_QueueAudioReturn ntrb_AudioBufferNew_Error_to_ntrb_RCD_QueueAudioReturn(const enum ntrb_AudioBufferNew_Error arg);
///Converts values in ntrb_AudioBufferLoad_Error to values in ntrb_RCD_QueueAudioReturn.
enum ntrb_RCD_QueueAudioReturn ntrb_AudioBufferLoad_Error_to_ntrb_RCD_QueueAudioReturn(const enum ntrb_AudioBufferLoad_Error arg);

/**
A value for returning an object which indicates a failure.

Check the audio_tracks of the ntrb_RuntimeCoreData object. 
If it is equal to NULL, the object is invalid.

Defined in RuntimeCoreData.c
*/
extern const ntrb_RuntimeCoreData failed_ntrb_RuntimeCoreData;

/**
Creates and initialises a new object to *rcd. 

- rcd->audio_tracks will be allocated to hold (track_count) ntrb_AudioDatapoints pointers.
- rcd->audio_tracks be set to (track_count).
- rcd->request_exit be set to false.
- rcd->in_pause_state be set to true.
*/
enum ntrb_RuntimeCoreData_Error ntrb_RuntimeCoreData_new(ntrb_RuntimeCoreData* const rcd, const uint16_t track_count);

/**
If rcd->audio_tracks isn't NULL: frees each of the tracks in rcd->audio_tracks, frees rcd->audio_tracks itself, 
set rcd->request_exit to true, set rcd->in_pause_state to true and destroys the rwlock of the object.

If rcd->audio_tracks or its tracks is equal to NULL, the function will proceed to skip to setting the booleans and destroy the rwlock.
*/
enum ntrb_RuntimeCoreData_Error ntrb_RuntimeCoreData_free(ntrb_RuntimeCoreData* const rcd);

/**
Frees a track of *rcd which is in rcd->audio_tracks[track_index] and sets its track pointer to NULL.

track_index should be a value between [0, rcd->audio_tracks - 1].

It does not perform bounds check or NULL check of the track pointer.
*/
enum ntrb_AudioBufferFree_Error ntrb_RuntimeCoreData_free_track(ntrb_RuntimeCoreData* const rcd, const uint16_t track_index);


///Creates and loads an audio track from an audio file with the provided filename to an empty track in runtime_data.
enum ntrb_RCD_QueueAudioReturn ntrb_RuntimeCoreData_queue_audio(ntrb_RuntimeCoreData* const runtime_data, const char* filename);

#endif