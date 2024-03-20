#include "test_bytevec.h"
#include "bytevec.h"

#include <stddef.h>
#include <assert.h>

static void test_ntrb_bytevec_new(){
	ntrb_bytevec v;
	assert(ntrb_bytevec_new(&v, 300));
	assert(v.base_ptr != NULL);
	assert(v.elements == 0);
	assert(v.capacity == 300);
	
	ntrb_bytevec_free(&v);
}

static void test_ntrb_bytevec_reserve(){
	ntrb_bytevec v;
	assert(ntrb_bytevec_new(&v, 300));
	
	v.elements = 299;
	*(v.base_ptr + v.elements) = 97;
	
	assert(ntrb_bytevec_reserve(&v, 225));
	assert(v.base_ptr != NULL);
	assert(v.elements == 299);
	assert(v.capacity == 525);
	assert(v.base_ptr[v.elements] == 97);		
	
	ntrb_bytevec_free(&v);
}

static void test_ntrb_bytevec_free(){
	ntrb_bytevec v;
	assert(ntrb_bytevec_new(&v, 300));
	ntrb_bytevec_free(&v);
	
	assert(v.base_ptr == NULL);
	assert(v.elements == 0);
	assert(v.capacity == 0);	
}

void test_suite_ntrb_bytevec(){
	test_ntrb_bytevec_new();
	test_ntrb_bytevec_reserve();
	test_ntrb_bytevec_free();
}