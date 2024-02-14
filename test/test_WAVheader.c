#include "test_WAVheader.h"
#include "../src/WAVheader.h"

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

static void test_ntrb_WAVheader_equal(FILE*, FILE*){
	const ntrb_WAVheader a = {
		.ChunkID = {'R', 'I', 'F', 'F'},
		.ChunkSize = 4222146,
		.Format = {'W', 'A', 'V', 'E'},
		
		.Subchunk1ID = {'f', 'm', 't', ' '},
		.Subchunk1Size = 16,
		.AudioFormat = 1,
		.NumChannels = 2,
		.SampleRate = 48000,
		.ByteRate = 192000,
		.BlockAlign = 4,
		.BitsPerSample = 16,
		
		.Subchunk2ID = {'d', 'a', 't', 'a'},
		.Subchunk2Size = 4221948
	};
	
	const ntrb_WAVheader b = a;
	assert(ntrb_WAVheader_equal(&a, &b) == true);
	
	ntrb_WAVheader c = a;
	c.ChunkID[0] = 'c';
	assert(ntrb_WAVheader_equal(&a, &c) == false);	
	
	ntrb_WAVheader d = a;
	d.ByteRate = 96000;
	assert(ntrb_WAVheader_equal(&b, &d) == false);		
}

void test_suite_ntrb_WAVheader(FILE* const outstream, FILE* const errstream){
	test_ntrb_WAVheader_equal(outstream, errstream);
}
