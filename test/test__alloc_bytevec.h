#ifndef test__ntrb_alloc_bytevec_h
#define test__ntrb_alloc_bytevec_h

#include "_alloc_bytevec.h"

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>

static void test__ntrb_alloc_bytevec_new(){
	_ntrb_alloc_bytevec v = _ntrb_alloc_bytevec_new(300);
	assert(v.base_ptr != NULL);
	assert(v.elements == 0);
	assert(v.capacity == 300);
	
	_ntrb_alloc_bytevec_free(&v);
}

static void test__ntrb_alloc_bytevec_reserve(){
	_ntrb_alloc_bytevec v = _ntrb_alloc_bytevec_new(300);
	assert(v.base_ptr != NULL);
	
	//Verify that the last element of the container before extension,
	//is the same after the extension.
	v.elements = 300;
	*(v.base_ptr + v.elements - 1) = 97;
	
	assert(_ntrb_alloc_bytevec_reserve(&v, 225));
	assert(v.base_ptr != NULL);
	assert(v.elements == 300);
	assert(v.capacity == 525);
	assert(v.base_ptr[v.elements - 1] == 97);
	
	_ntrb_alloc_bytevec_free(&v);
}

static void test__ntrb_alloc_bytevec_append(){
	_ntrb_alloc_bytevec v = _ntrb_alloc_bytevec_new(1);
	assert(v.base_ptr != NULL);
	
	for(size_t i = 0; i < UINT16_MAX; i++){
		assert(_ntrb_alloc_bytevec_append(&v, sizeof(size_t), &i));
	}
	
	assert(v.elements == sizeof(size_t) * UINT16_MAX);
	
	for(size_t i = 0; i < UINT16_MAX; i++){
		assert(*(size_t*)(v.base_ptr + i*sizeof(size_t)) == i);
	}
	
	_ntrb_alloc_bytevec_free(&v);
}

static void test__ntrb_alloc_bytevec_free(){
	_ntrb_alloc_bytevec v = _ntrb_alloc_bytevec_new(300);
	assert(v.base_ptr != NULL);
	_ntrb_alloc_bytevec_free(&v);
	
	assert(v.base_ptr == NULL);
	assert(v.elements == 0);
	assert(v.capacity == 0);	
}

void test_suite__ntrb_alloc_bytevec(){	
	test__ntrb_alloc_bytevec_new();
	test__ntrb_alloc_bytevec_reserve();
	test__ntrb_alloc_bytevec_append();
	test__ntrb_alloc_bytevec_free();
}

#endif