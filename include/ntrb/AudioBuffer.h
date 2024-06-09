#ifndef ntrb_AudioBuffer_h
#define ntrb_AudioBuffer_h

/**
Copyright 2024 Pawikan Boonnaum

This file is part of naturau-base/ntrb.

naturau-base/ntrb is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

naturau-base/ntrb is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with naturau-base/ntrb. If not, see <https://www.gnu.org/licenses/>.
*/

/**
\file AudioBuffer.h
A module providing the ntrb_AudioBuffer: an abstracted audio buffer, 
functions for creating and deleting the type, and enums for reporting errors relating to the type.

To add the support for your source, your module must contain the function for loading the audio from the source type.
Create your else-if which sets ntrb_AudioBuffer.load_buffer_callback to your function in the if(strcmp()) section of ntrb_AudioBuffer_new().
Refer to the documentation of ntrb_AudioBuffer.load_buffer_callback for the requirements of your audio loading function.
  
In most cases, loading an audio from a source requires variables to keep track of the audio loading.
1. Create a struct which contains the necessary information for loading the audio from your source, 
then add it in ntrb_AudioBuffer.source.
2. Provide a function which initialises your struct. The parameters are up to you, 
  but the return type should be ntrb_AudioBufferNew_Error. Feel free to add your specific error in the enum if need to.
  The function should not load the audio buffer yet. Leave that to the loading callback which will be called somewhere else.
  The function will be called in the same else if in ntrb_AudioBuffer_new().
  You don't have to actually make a function, but it just keeps things readable in ntrb_AudioBuffer_new().
3. Provide a function which uninitialises your struct. The parameters are up to you,
  but the return type should be ntrb_AudioBufferFree_Error. Feel free to add your specific error  in the enum if need to.
  Create an if in ntrb_AudioBuffer_free() which compares if ntrb_AudioBuffer.load_buffer_callback is the same to that of your loading function.
  If it is equal, add your freeing function in there.


\todo don't rely on code modifications for source support in 0.3
*/

#include "AudioHeader.h"
#include "BufferSource_WAVfile.h"
#include "BufferSource_FLACfile.h"

#include <pthread.h>

#include <stdio.h>
#include <stdbool.h>

/**
An enum for reporting audio loading errors from an ntrb_AudioBuffer.

The raised error is acknowledged and acted on **after** the contents of the audio buffer is read, simplifying EOF scenarios.
Which means when an error or typically an EOF occurs, the buffer must contain no garbage if possible.
Typically by always zero filling the buffer in every loading callbacks, guaranteeing no garbage is present in the buffer.

In ntrb and naturau, once any error is detected by the audio engine lopp, the loop stops reading from the buffer and the buffer gets uninitialised.
*/
enum ntrb_AudioBufferLoad_Error{
	///Audio buffer is all good
	ntrb_AudioBufferLoad_OK,
	///Failed to acquire a write lock to the buffer, this can cause the audio in the buffer to be played repeatedly.
	ntrb_AudioBufferLoad_AcqWritelockError,
	///The underlying fread() in the WAV file audio loading callback read its file contents more or less than what it's supposed to read.
	ntrb_AudioBufferLoad_UnequalRead,
	///Generic end of file/stream. Indicating no further audio can be provided by the callback.
	ntrb_AudioBufferLoad_EOF,
	///An error occured while converting the audio the the standard audio format used by the audio engine loop.
	ntrb_AudioBufferLoad_StdaudConversionError,
	///Indicating the rwlock of the buffer couldn't be unlocked.
	ntrb_AudioBufferLoad_RwlockUnlockError,
	///This is returned if the FLAC loading callback encounters an FLAC__StreamDecoderErrorStatus while reading the audio.
	ntrb_AudioBufferLoad_FLACBadData,
	///A redirect of FLAC__STREAM_DECODER_OGG_ERROR in the FLAC loading callback.
	ntrb_AudioBufferLoad_OGGError,
	///A redirect of FLAC__STREAM_DECODER_SEEK_ERROR in the FLAC loading callback.	
	ntrb_AudioBufferLoad_SeekError,	
	///Raised when the FLAC loading callback recieved an abort return from its underlying FLAC decoding function.
	ntrb_AudioBufferLoad_Aborted,	
	///A generic memory allocation error.
	ntrb_AudioBufferLoad_AllocError,		
};

/**
A struct representing an audio buffer with its non-source-specific design, 
abstracting the source-specific interactions from the user.
*/
typedef struct{
	/**
	An rwlock for ntrb_AudioBuffer.datapoints.
	
	The audio engine loop will try (pthread_rwlock_try_rdlock()) to access the buffer.
	If the buffer is not done loading the audio, the audio engine will get a denied access to the buffer, 
	guaranteeing prevention of reading the buffer.
	*/
	pthread_rwlock_t buffer_access;
	
	/**
	An array of standard audio format samples to be read by the audio engine loop.
	
	The buffer has to contain (ntrb_AudioBuffer.monochannel_samples * ntrb_std_audchannels) floats.
	*/
	float* datapoints;
	
	/**
	The amount of frames in ntrb_AudioBuffer.datapoints. 
	This needs to be multiplied by ntrb_std_audchannels to get the exact amount of floats in ntrb_AudioBuffer.datapoints.
	*/
	size_t monochannel_samples;
	
	/**
	A union containing source-specifc data required to read the audio from a source.
	
	The methods for initialising and uninitialising the data must be provided in ntrb_AudioBuffer_new() and ntrb_AudioBuffer_free() respectively.
	
	Refer to the Detailed Description part of the AudioBuffer.h if viewed in Doxygen. Else, refer to the comment at the very top.
	*/
	union{
		ntrb_BufferSource_WAVfile wav_file;
		ntrb_BufferSource_FLACfile flac_file;
	} source;
	
	/**
	A pointer to a source-specific reading function.
	
	A few specifications of the function:
	- The function will be called in a multithreaded environment has to be mulithreaded-safe.
	  Just by acquiring pthread_rwlock_wrlock() before writing, and unlocking by pthread_rwlock_unlock() after writing.
	- The void* return is for pthread_t, and in ntrb and naturau, we don't care what it returns. Just return NULL.
	- The void* parameter will be casted to the ntrb_AudioBuffer* which the function would write to.
	- The function will report audio loading errors through ntrb_AudioBuffer.load_err.
	- The audio buffer should always contain no garbage, even when an error occurred, unless you can't get a lock to the buffer.
	  Typically this is done by zero-filling the ntrb_AudioBuffer.datapoints first in every loading.
	- You can have your source-specifc data in ntrb_AudioBuffer.source and do whatever you wish there.
	  The audio engine loop does not care about it.
	*/
	void* (*load_buffer_callback)(void*);
	
	/**
	A loading error for the audio engine loop to read.
	The audio engine loop will acknowledge and act on the error **after** the buffer contents is read from,
	which means the buffer must contain no garbage in almost every type of error.
	*/
	enum ntrb_AudioBufferLoad_Error load_err;	
} ntrb_AudioBuffer;

/**
An enum representing errors while initialising ntrb_AudioBuffer with ntrb_AudioBuffer_new().
*/
enum ntrb_AudioBufferNew_Error{
	///wow!	
	ntrb_AudioBufferNew_OK,
	///Generic memory allocation failure.
	ntrb_AudioBufferNew_AllocError,
	///Unable to open the file of the provided filename.
	ntrb_AudioBufferNew_FileOpenError,
	///A generic file read error.
	ntrb_AudioBufferNew_FileReadError,
	///Generic unsupported audio filetype. This is returned if the filetype extension is not supported.
	ntrb_AudioBufferNew_InvalidAudFiletype,
	///Error interpreting the header of the WAV file.
	ntrb_AudioBufferNew_WAVheaderError,
	///Unable to create the rwlock for the buffer.
	ntrb_AudioBufferNew_RwlockInitError,
	///A redirect of FLAC__STREAM_DECODER_INIT_STATUS_UNSUPPORTED_CONTAINER.
	ntrb_AudioBufferNew_FLACcontainerError,
	///Generic unsupported audio format.
	///This is different from ntrb_AudioBufferNew_InvalidAudFiletype 
	///where this indicates that the audio filetype is supported, but the format which the audio was encoded in is not.
	ntrb_AudioBufferNew_InvalidAudFormat,
	///An unknown error occurred.
	ntrb_AudioBufferNew_UnknownError,
};

/**
An enum representing errors from ntrb_AudioBuffer_free().
*/
enum ntrb_AudioBufferFree_Error{
	///wow!
	ntrb_AudioBufferFree_OK,
	///The function was unable to acquire a write lock of the buffer.
	///The buffer is left unchanged.
	ntrb_AudioBufferFree_AcqWritelockError,
	///The function was unable to unlock the write lock of the buffer.
	///The ntrb_AudioBuffer.datapoints and the data in ntrb_AudioBuffer.source are freed.
	ntrb_AudioBufferFree_RwlockUnlockError,
	///The function was unable to uninitialise the rwlock.
	///The ntrb_AudioBuffer.datapoints and the data in ntrb_AudioBuffer.source are freed.
	ntrb_AudioBufferFree_RwlockDestroyError,
};

/**
A function which initialises an ntrb_AudioBuffer object in *ret.

This function allows for modification in order to support a source type.
You can do so by inserting your own source condition (filetype extension or just a specfic filename) 
in the if(strcmp()) section of the code. 

For the specifications of the modification, refer to the Detailed Description part of AudioBuffer.h if viewed in Doxygen,
else the very top comment of this file.
*/
enum ntrb_AudioBufferNew_Error ntrb_AudioBuffer_new(ntrb_AudioBuffer* const ret, const char* const filename, const size_t stdaud_frame_count);

/**
A function which uninitialises an ntrb_AudioBuffer object in *ret.

This function allows for modification in order to free a source-specific data in ntrb_AudioBuffer.source.
You can do so by inserting a 'if' which checks if the ntrb_AudioBuffer.load_buffer_callback function pointer is equal to your loading callback.
If it is equal, this indicates that the buffer loads from your source type and you should free your data type in ntrb_AudioBuffer.source.

For the specifications of the modification, refer to the Detailed Description part of AudioBuffer.h if viewed in Doxygen,
else the very top comment of this file.
*/
enum ntrb_AudioBufferFree_Error ntrb_AudioBuffer_free(ntrb_AudioBuffer* const obj);
#endif