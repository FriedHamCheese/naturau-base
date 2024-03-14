#include "test_AudioDatapoints.h"
#include "AudioDatapoints.h"

#include <assert.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

static void test_new_ntrb_AudioDatapoints(FILE* const, FILE* const){
	const size_t alloc_size_bytes = 15;
	void* all_zero_array = calloc(alloc_size_bytes, sizeof(uint8_t));
	
	const ntrb_AudioDatapoints aud = new_ntrb_AudioDatapoints(alloc_size_bytes);
	
	assert(aud.bytes != NULL); 
	assert(aud.byte_count == alloc_size_bytes);
	assert(aud.byte_pos == 0);
	
	assert(memcmp(aud.bytes, all_zero_array, alloc_size_bytes) == 0);
	
	free(aud.bytes);
	free(all_zero_array);
	
	assert(new_ntrb_AudioDatapoints(0).bytes == NULL);
}

static void test_extend_ntrb_AudioDatapoints_capacity(FILE* const, FILE* const){
	ntrb_AudioDatapoints aud = new_ntrb_AudioDatapoints(15);
	assert(aud.bytes != NULL);
	
	assert(extend_ntrb_AudioDatapoints_capacity(&aud, 300));
	assert(aud.bytes != NULL);
	assert(aud.byte_count == 300 + 15);
	
	free(aud.bytes);
}

static void test_copy_ntrb_AudioDatapoints(FILE* const, FILE* const){
	const size_t alloc_size_bytes = 32;
	ntrb_AudioDatapoints aud_1 = new_ntrb_AudioDatapoints(alloc_size_bytes);
	assert(aud_1.bytes != NULL);
	
	aud_1.bytes[18] = 35;
	aud_1.bytes[5] = 69;
	aud_1.bytes[26] = 11;
	
	const ntrb_AudioDatapoints aud_2 = copy_ntrb_AudioDatapoints(aud_1);
	assert(aud_2.bytes != NULL);
	
	assert(aud_1.bytes != aud_2.bytes);
	assert(memcmp(aud_1.bytes, aud_2.bytes, alloc_size_bytes) == 0);
	assert(aud_1.byte_pos == aud_2.byte_pos);
	assert(aud_2.byte_count == aud_1.byte_count);
	
	free(aud_1.bytes);
	free(aud_2.bytes);
}

void test_suite_ntrb_AudioDatapoints(FILE* const outstream, FILE* const errstream){
	test_new_ntrb_AudioDatapoints(outstream, errstream);
	test_extend_ntrb_AudioDatapoints_capacity(outstream, errstream);
	test_copy_ntrb_AudioDatapoints(outstream, errstream);
}