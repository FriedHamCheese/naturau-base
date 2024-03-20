#ifndef ntrb_decode_flac_h
#define ntrb_decode_flac_h

#include "AudioHeader.h"
#include "bytevec.h"

enum ntrb_FLAC_decode_status{
	ntrb_FLAC_decode_OK,
	ntrb_FLAC_decode_unsupported_bps,
	ntrb_FLAC_decode_unsupported_channelcount,
	ntrb_FLAC_decode_MD5_not_equal,
	
	ntrb_FLAC_decode_FLAC__StreamDecoderInitStatus = 10,
	//20
	ntrb_FLAC_decode_FLAC__StreamDecoderState = ntrb_FLAC_decode_FLAC__StreamDecoderInitStatus + 10,
	//35
	ntrb_FLAC_decode_FLAC__StreamDecoderErrorStatus = ntrb_FLAC_decode_FLAC__StreamDecoderState + 15,
};

typedef struct{
	ntrb_AudioHeader header;
	ntrb_bytevec datapoints;
	//Needed a way to report an error from FLAC__StreamDecoderErrorCallback(), this is used internally.
	enum ntrb_FLAC_decode_status _decoder_error;
} ntrb_AudioDataFLAC;

enum ntrb_FLAC_decode_status ntrb_decode_FLAC_file(const char* const filename, ntrb_AudioDataFLAC* const aud_data);

#endif