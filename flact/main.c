#define NATURAU_TYPE_TEST 0
#if NATURAU_TYPE_TEST == 0

#include "RuntimeCoreData.h"
#include "audeng_wrapper.h"
#include "aud_std_fmt.h"
#include "decode_flac.h"

#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>

int main(){
	ntrb_RuntimeCoreData rcd = new_ntrb_RuntimeCoreData(8);
	assert(rcd.audio_tracks != NULL);
	
	ntrb_AudioDataFLAC aud_data;
	aud_data.datapoints = new_ntrb_AudioDatapoints(2 * 48000 * sizeof(uint16_t) * 1);
	
	assert(ntrb_decode_FLAC_file("../../ntr-aud/long road.flac", &aud_data) == ntrb_FLAC_decode_OK);
	
	ntrb_AudioDatapoints* std_aud = malloc(sizeof(ntrb_AudioDatapoints));
	assert(ntrb_to_standard_format(std_aud, aud_data.datapoints, &(aud_data.header)) == ntrb_StdAudFmtConversion_OK);
	free(aud_data.datapoints.bytes);
	
	rcd.audio_tracks[0] = std_aud;
	
	rcd.in_pause_state = false;
	printf("playing...\n");
	ntrb_run_audio_engine(&rcd);
	
	free_ntrb_RuntimeCoreData(&rcd);
	printf("Wow! :D\n");
	return 0;
}

#endif
#if NATURAU_TYPE_TEST == 1

int main(){
	return 0;
}

#endif