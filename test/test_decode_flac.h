#ifndef test_ntrb_decode_flac_h
#define test_ntrb_decode_flac_h

#include "decode_flac.h"

#include "AudioDatapoints.h"

#include "FLAC/stream_decoder.h"

#include <stdlib.h>
#include <assert.h>

static void test_ntrb_decode_FLAC_file(){
	ntrb_AudioDataFLAC aud;
	aud.datapoints = ntrb_bytevec_new(1);
	assert(aud.datapoints.base_ptr != NULL);	
	assert(ntrb_decode_FLAC_file("./test/decode_flac/portal_radio.flac", &aud) == ntrb_FLAC_decode_OK);
	ntrb_bytevec_free(&(aud.datapoints));
	
	assert(ntrb_decode_FLAC_file("./test/decode_flac/invalid_channels.flac", &aud) == ntrb_FLAC_decode_unsupported_channelcount);
	
	assert(ntrb_decode_FLAC_file("./test/decode_flac/invalid_bps.flac", &aud) == ntrb_FLAC_decode_unsupported_bps);	
	
	aud.datapoints= ntrb_bytevec_new(1);
	assert(aud.datapoints.base_ptr != NULL);
	assert(ntrb_decode_FLAC_file("./test/decode_flac/invalid_md5.flac", &aud) == ntrb_FLAC_decode_MD5_not_equal);
	ntrb_bytevec_free(&(aud.datapoints));

	assert(ntrb_decode_FLAC_file("./test/decode_flac/non-existent.flac", &aud) == ntrb_FLAC_decode_FLAC__StreamDecoderInitStatus + FLAC__STREAM_DECODER_INIT_STATUS_ERROR_OPENING_FILE);
}

void test_suite_ntrb_decode_flac(){
	test_ntrb_decode_FLAC_file();
}

#endif