#include "test_RuntimeCoreData.h"
#include "../src/RuntimeCoreData.h"

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>

static void test_new_ntrb_RuntimeCoreData(FILE*, FILE*){	
	const uint16_t requested_audio_tracks = 256;
	const ntrb_RuntimeCoreData rcd = new_ntrb_RuntimeCoreData(requested_audio_tracks);
	assert(rcd.audio_tracks != NULL);
	
	assert(rcd.audio_track_count == requested_audio_tracks);
	assert(rcd.requested_exit == false);
	assert(rcd.in_pause_state == true);	
	assert(rcd.streaming_audio == false);	
	assert(rcd.writing_tracks == false);	
}


void test_suite_ntrb_RuntimeCoreData(FILE* const outstream, FILE* const errstream){
	test_new_ntrb_RuntimeCoreData(outstream, errstream);
}
