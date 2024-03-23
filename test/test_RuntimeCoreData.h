#ifndef test_ntrb_RuntimeCoreData_h
#define test_ntrb_RuntimeCoreData_h

#include "RuntimeCoreData.h"

#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

static void test_ntrb_RuntimeCoreData_new(){	
	const uint16_t requested_audio_tracks = 256;
	ntrb_RuntimeCoreData rcd;
	assert(ntrb_RuntimeCoreData_new(&rcd, requested_audio_tracks) == 0);
	
	assert(rcd.audio_tracks != NULL);
	assert(rcd.audio_track_count == requested_audio_tracks);
	assert(rcd.requested_exit == false);
	assert(rcd.in_pause_state == true);	
	
	ntrb_RuntimeCoreData_free(&rcd);
	
	ntrb_RuntimeCoreData rcd2;
	assert(ntrb_RuntimeCoreData_new(&rcd2, 0) == ENOMEM);
}

static void test_free_ntrb_RuntimeCoreData(){
	ntrb_RuntimeCoreData rcd;
	assert(ntrb_RuntimeCoreData_new(&rcd, 16) == 0);
	
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
	
	assert(ntrb_RuntimeCoreData_free(&rcd) == 0);
	
	assert(rcd.audio_tracks == NULL);
	assert(rcd.audio_track_count == 0);
	
	ntrb_RuntimeCoreData rcd2 = failed_ntrb_RuntimeCoreData;
	assert(ntrb_RuntimeCoreData_free(&rcd2) != 0);
}

static void test_free_ntrb_RuntimeCoreData_track(){
	ntrb_RuntimeCoreData rcd;
	assert(ntrb_RuntimeCoreData_new(&rcd, 16) == 0);
	
	ntrb_AudioDatapoints* aud_1 = malloc(sizeof(ntrb_AudioDatapoints));
	assert(aud_1 != NULL);
	*aud_1 = ntrb_AudioDatapoints_new(sizeof(float));
	assert(aud_1->bytes != NULL);
	
	rcd.audio_tracks[10] = aud_1;
	ntrb_RuntimeCoreData_free_track(&rcd, 10);
	assert(rcd.audio_tracks[10] == NULL);

	assert(ntrb_RuntimeCoreData_free(&rcd) == 0);	
}


void test_suite_ntrb_RuntimeCoreData(){
	test_ntrb_RuntimeCoreData_new();
	test_free_ntrb_RuntimeCoreData();
	test_free_ntrb_RuntimeCoreData_track();
}


#endif