#include "test_RuntimeCoreData.h"
#include "RuntimeCoreData.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>

static void test_ntrb_RuntimeCoreData_new(FILE*, FILE*){	
	const uint16_t requested_audio_tracks = 256;
	ntrb_RuntimeCoreData rcd = ntrb_RuntimeCoreData_new(requested_audio_tracks);
	assert(rcd.audio_tracks != NULL);
	
	assert(rcd.audio_track_count == requested_audio_tracks);
	assert(rcd.requested_exit == false);
	assert(rcd.in_pause_state == true);	
	assert(rcd.streaming_audio == false);	
	assert(rcd.writing_tracks == false);
	
	ntrb_RuntimeCoreData_free(&rcd);
	
	ntrb_RuntimeCoreData rcd2 = ntrb_RuntimeCoreData_new(0);
	assert(rcd2.audio_tracks == NULL);
}

static void test_free_ntrb_RuntimeCoreData(FILE*, FILE*){
	ntrb_RuntimeCoreData rcd = ntrb_RuntimeCoreData_new(16);
	assert(rcd.audio_tracks != NULL);
	
	ntrb_AudioDatapoints* aud_1 = malloc(sizeof(ntrb_AudioDatapoints));
	assert(aud_1 != NULL);
	*aud_1 = ntrb_AudioDatapoints_new(sizeof(float));
	assert(aud_1->bytes != NULL);
	
	ntrb_AudioDatapoints* aud_2 = malloc(sizeof(ntrb_AudioDatapoints));
	assert(aud_2 != NULL);
	*aud_2 = ntrb_AudioDatapoints_new(sizeof(float));
	assert(aud_2->bytes != NULL);

	rcd.audio_tracks[3] = aud_1;
	rcd.audio_tracks[7] = aud_2;
	
	ntrb_RuntimeCoreData_free(&rcd);
	
	assert(rcd.audio_tracks == NULL);
	assert(rcd.audio_track_count == 0);
	
	ntrb_RuntimeCoreData rcd2 = failed_ntrb_RuntimeCoreData;
	ntrb_RuntimeCoreData_free(&rcd2);
	
	ntrb_RuntimeCoreData rcd3 = ntrb_RuntimeCoreData_new(16);
	assert(rcd3.audio_tracks != NULL);
	ntrb_RuntimeCoreData_free(&rcd3);	
}

static void test_free_ntrb_RuntimeCoreData_track(FILE*, FILE*){
	ntrb_RuntimeCoreData rcd = ntrb_RuntimeCoreData_new(16);
	assert(rcd.audio_tracks != NULL);
	
	ntrb_AudioDatapoints* aud_1 = malloc(sizeof(ntrb_AudioDatapoints));
	assert(aud_1 != NULL);
	*aud_1 = ntrb_AudioDatapoints_new(sizeof(float));
	assert(aud_1->bytes != NULL);
	
	rcd.audio_tracks[10] = aud_1;
	ntrb_RuntimeCoreData_free_track(&rcd, 10);
	assert(rcd.audio_tracks[10] == NULL);

	ntrb_RuntimeCoreData_free(&rcd);	
}


void test_suite_ntrb_RuntimeCoreData(FILE* const outstream, FILE* const errstream){
	test_ntrb_RuntimeCoreData_new(outstream, errstream);
	test_free_ntrb_RuntimeCoreData(outstream, errstream);
	test_free_ntrb_RuntimeCoreData_track(outstream, errstream);
}
