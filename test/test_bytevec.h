/*
Copyright 2024 Pawikan Boonnaum

This file is part of naturau-base/ntrb.

naturau-base/ntrb is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

naturau-base/ntrb is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with naturau-base/ntrb. If not, see <https://www.gnu.org/licenses/>.
*/

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