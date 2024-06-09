#ifndef test_ntrb_bytevec_h
#define test_ntrb_bytevec_h

#include "bytevec.h"
#include "alloc.h"

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>

static void test_ntrb_bytevec_new(){
	ntrb_bytevec v = ntrb_bytevec_new(300);
	assert(v.base_ptr != NULL);
	assert(v.elements == 0);
	assert(v.capacity == 300);
	
	ntrb_bytevec_free(&v);
}

static void test_ntrb_bytevec_reserve(){
	ntrb_bytevec v = ntrb_bytevec_new(300);
	assert(v.base_ptr != NULL);
	
	//Verify that the last element of the container before extension,
	//is the same after the extension.
	v.elements = 300;
	*(v.base_ptr + v.elements - 1) = 97;
	
	assert(ntrb_bytevec_reserve(&v, 225));
	assert(v.base_ptr != NULL);
	assert(v.elements == 300);
	assert(v.capacity == 525);
	assert(v.base_ptr[v.elements - 1] == 97);
	
	ntrb_bytevec_free(&v);
}

static void test_ntrb_bytevec_append(){
	ntrb_bytevec v = ntrb_bytevec_new(1);
	assert(v.base_ptr != NULL);
	
	for(size_t i = 0; i < UINT16_MAX; i++){
		assert(ntrb_bytevec_append(&v, sizeof(size_t), &i));
	}
	
	assert(v.elements == sizeof(size_t) * UINT16_MAX);
	
	for(size_t i = 0; i < UINT16_MAX; i++){
		assert(*(size_t*)(v.base_ptr + i*sizeof(size_t)) == i);
	}
	
	ntrb_bytevec_free(&v);
}

static void test_ntrb_bytevec_free(){
	ntrb_bytevec v = ntrb_bytevec_new(300);
	assert(v.base_ptr != NULL);
	ntrb_bytevec_free(&v);
	
	assert(v.base_ptr == NULL);
	assert(v.elements == 0);
	assert(v.capacity == 0);	
}

void test_suite_ntrb_bytevec(){
	test_ntrb_bytevec_new();
	test_ntrb_bytevec_reserve();
	test_ntrb_bytevec_append();
	test_ntrb_bytevec_free();
}

#endif