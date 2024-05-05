#ifndef ntrb_decode_flac_h
#define ntrb_decode_flac_h

#include "AudioHeader.h"
#include "bytevec.h"

/**
\file decode_flac.h
A module for reading and decoding FLAC files.
*/

/**
Enum reporting errors while reading and decoding FLAC file.

\todo this enum spacing is a bad technique D:
*/
enum ntrb_FLAC_decode_status{
	///Read and decoded the FLAC file successfully.
	ntrb_FLAC_decode_OK,
	///The FLAC file uses unsupported bits per sample.
	ntrb_FLAC_decode_unsupported_bps,
	///The FLAC file uses more than 2 audio channels.
	ntrb_FLAC_decode_unsupported_channelcount,
	///The FLAC did decode successfully, but its contents does not match the MD5 signature.
	ntrb_FLAC_decode_MD5_not_equal,
	
	///Offset for FLAC__StreamDecoderInitStatus.
	ntrb_FLAC_decode_FLAC__StreamDecoderInitStatus = 10,
	//20
	///Offset for FLAC__StreamDecoderState.	
	ntrb_FLAC_decode_FLAC__StreamDecoderState = ntrb_FLAC_decode_FLAC__StreamDecoderInitStatus + 10,
	///Offset for FLAC__StreamDecoderErrorStatus.		
	//35
	ntrb_FLAC_decode_FLAC__StreamDecoderErrorStatus = ntrb_FLAC_decode_FLAC__StreamDecoderState + 15,
};

/**
Just a struct with ntrb_AudioHeader and ntrb_bytevec for the datapoints container.

Needed this because libFLAC only provides 1 single void* to pass user data.
*/
typedef struct{
	ntrb_AudioHeader header;
	///Contents from datapoints.base_ptr to datapoints.base_ptr + datapoints.elem - 1 can be converted to ntrb_AudioDatapoints.
	ntrb_bytevec datapoints;
	///Needed a way to report an error from FLAC__StreamDecoderErrorCallback(), this is used internally.
	enum ntrb_FLAC_decode_status _decoder_error;
} ntrb_AudioDataFLAC;

/**
Reads and decodes the FLAC file with the given filename to aud_data.

aud_data->datapoints must be in an uninitialised state, the function will initialise it.

No matter the error codes, aud_data->datapoints is not freed if any error occurred.

\todo shouldn't it cleanup after itself if it failed
*/
enum ntrb_FLAC_decode_status ntrb_decode_FLAC_file(const char* const filename, ntrb_AudioDataFLAC* const aud_data);

#endif