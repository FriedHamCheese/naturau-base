#ifndef test_ntrb_AudioDatapoints_h
#define test_ntrb_AudioDatapoints_h

#include "AudioDatapoints.h"

#include <assert.h>

#include <stdlib.h>
#include <string.h>

static void test_ntrb_AudioDatapoints_new(){
	const size_t alloc_size_bytes = 15;
	void* all_zero_array = calloc(alloc_size_bytes, sizeof(uint8_t));
	
	const ntrb_AudioDatapoints aud = ntrb_AudioDatapoints_new(alloc_size_bytes);
	
	assert(aud.bytes != NULL); 
	assert(aud.byte_count == alloc_size_bytes);
	assert(aud.byte_pos == 0);
	
	assert(memcmp(aud.bytes, all_zero_array, alloc_size_bytes) == 0);
	
	free(aud.bytes);
	free(all_zero_array);
	
	assert(ntrb_AudioDatapoints_new(0).bytes == NULL);
}

static void test_ntrb_AudioDatapoints_copy(){
	const size_t alloc_size_bytes = 32;
	ntrb_AudioDatapoints aud_1 = ntrb_AudioDatapoints_new(alloc_size_bytes);
	assert(aud_1.bytes != NULL);
	
	aud_1.bytes[18] = 35;
	aud_1.bytes[5] = 69;
	aud_1.bytes[26] = 11;
	
	const ntrb_AudioDatapoints aud_2 = ntrb_AudioDatapoints_copy(aud_1);
	assert(aud_2.bytes != NULL);
	
	assert(aud_1.bytes != aud_2.bytes);
	assert(memcmp(aud_1.bytes, aud_2.bytes, alloc_size_bytes) == 0);
	assert(aud_1.byte_pos == aud_2.byte_pos);
	assert(aud_2.byte_count == aud_1.byte_count);
	
	free(aud_1.bytes);
	free(aud_2.bytes);
}

static void test_ntrb_AudioDatapoints_free(){
	ntrb_AudioDatapoints aud = ntrb_AudioDatapoints_new(35);
	assert(aud.bytes != NULL);
	
	ntrb_AudioDatapoints_free(&aud);
	assert(aud.bytes == NULL);
	assert(aud.byte_count == 0);
}

static void test_suite_ntrb_AudioDatapoints(){
	test_ntrb_AudioDatapoints_new();
	test_ntrb_AudioDatapoints_copy();
	test_ntrb_AudioDatapoints_free();
}

#endif