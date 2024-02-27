#include "test_RuntimeCoreData.h"
#include "RuntimeCoreData.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>

static void test_new_ntrb_RuntimeCoreData(FILE*, FILE*){	
	const uint16_t requested_audio_tracks = 256;
	ntrb_RuntimeCoreData rcd = new_ntrb_RuntimeCoreData(requested_audio_tracks);
	assert(rcd.audio_tracks != NULL);
	
	assert(rcd.audio_track_count == requested_audio_tracks);
	assert(rcd.requested_exit == false);
	assert(rcd.in_pause_state == true);	
	assert(rcd.streaming_audio == false);	
	assert(rcd.writing_tracks == false);
	
	free_ntrb_RuntimeCoreData(&rcd);
	
	ntrb_RuntimeCoreData rcd2 = new_ntrb_RuntimeCoreData(0);
	assert(rcd2.audio_tracks == NULL);
}

static void test_free_ntrb_RuntimeCoreData(FILE*, FILE*){
	ntrb_RuntimeCoreData rcd = new_ntrb_RuntimeCoreData(16);
	assert(rcd.audio_tracks != NULL);
	
	ntrb_AudioDatapoints* aud_1 = malloc(sizeof(ntrb_AudioDatapoints));
	assert(aud_1 != NULL);
	*aud_1 = new_ntrb_AudioDatapoints(sizeof(float));
	assert(aud_1->bytes != NULL);
	
	ntrb_AudioDatapoints* aud_2 = malloc(sizeof(ntrb_AudioDatapoints));
	assert(aud_2 != NULL);
	*aud_2 = new_ntrb_AudioDatapoints(sizeof(float));
	assert(aud_2->bytes != NULL);

	rcd.audio_tracks[3] = aud_1;
	rcd.audio_tracks[7] = aud_2;
	
	free_ntrb_RuntimeCoreData(&rcd);
	
	assert(rcd.audio_tracks == NULL);
	assert(rcd.audio_track_count == 0);
	
	ntrb_RuntimeCoreData rcd2 = failed_ntrb_RuntimeCoreData;
	free_ntrb_RuntimeCoreData(&rcd2);
	
	ntrb_RuntimeCoreData rcd3 = new_ntrb_RuntimeCoreData(16);
	assert(rcd3.audio_tracks != NULL);
	free_ntrb_RuntimeCoreData(&rcd3);	
}

static void test_free_ntrb_RuntimeCoreData_track(FILE*, FILE*){
	ntrb_RuntimeCoreData rcd = new_ntrb_RuntimeCoreData(16);
	assert(rcd.audio_tracks != NULL);
	
	ntrb_AudioDatapoints* aud_1 = malloc(sizeof(ntrb_AudioDatapoints));
	assert(aud_1 != NULL);
	*aud_1 = new_ntrb_AudioDatapoints(sizeof(float));
	assert(aud_1->bytes != NULL);
	
	rcd.audio_tracks[10] = aud_1;
	free_ntrb_RuntimeCoreData_track(&rcd, 10);
	assert(rcd.audio_tracks[10] == NULL);

	free_ntrb_RuntimeCoreData(&rcd);	
}


void test_suite_ntrb_RuntimeCoreData(FILE* const outstream, FILE* const errstream){
	test_new_ntrb_RuntimeCoreData(outstream, errstream);
	test_free_ntrb_RuntimeCoreData(outstream, errstream);
	test_free_ntrb_RuntimeCoreData_track(outstream, errstream);
}
