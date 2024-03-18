#include "test_decode_flac.h"
#include "decode_flac.h"

#include "AudioDatapoints.h"

#include "FLAC/stream_decoder.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


static void test_ntrb_decode_FLAC_file(){
	ntrb_AudioDataFLAC aud;
	
	aud.datapoints = new_ntrb_AudioDatapoints(1);
	assert(aud.datapoints.bytes != NULL);	
	assert(ntrb_decode_FLAC_file("./test/decode_flac/portal_radio.flac", &aud) == ntrb_FLAC_decode_OK);
	free(aud.datapoints.bytes);
	aud.datapoints.byte_count = 0;
	aud.datapoints.byte_pos = 0;	
	
	assert(ntrb_decode_FLAC_file("./test/decode_flac/invalid_channels.flac", &aud) == ntrb_FLAC_decode_unsupported_channelcount);
	
	assert(ntrb_decode_FLAC_file("./test/decode_flac/invalid_bps.flac", &aud) == ntrb_FLAC_decode_unsupported_bps);	
	
	aud.datapoints = new_ntrb_AudioDatapoints(1);
	assert(aud.datapoints.bytes != NULL);
	assert(ntrb_decode_FLAC_file("./test/decode_flac/invalid_md5.flac", &aud) == ntrb_FLAC_decode_MD5_not_equal);
	free(aud.datapoints.bytes);
	aud.datapoints.byte_count = 0;
	aud.datapoints.byte_pos = 0;

	assert(ntrb_decode_FLAC_file("./test/decode_flac/non-existent.flac", &aud) == ntrb_FLAC_decode_FLAC__StreamDecoderInitStatus + FLAC__STREAM_DECODER_INIT_STATUS_ERROR_OPENING_FILE);	
}

void test_suite_ntrb_decode_flac(FILE*, FILE*){
	test_ntrb_decode_FLAC_file();
}