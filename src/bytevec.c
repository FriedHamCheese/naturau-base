#include "bytevec.h"
#include "alloc.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

const ntrb_bytevec failed_ntrb_bytevec = {.base_ptr = NULL, .elements = 0, .capacity = 0};

ntrb_bytevec ntrb_bytevec_new(const size_t alloc_bytes){
	ntrb_bytevec obj;
	obj.base_ptr = ntrb_malloc(alloc_bytes);
	if(obj.base_ptr == NULL) return failed_ntrb_bytevec;
	
	obj.elements = 0;
	obj.capacity = alloc_bytes;
	return obj;
}
bool ntrb_bytevec_reserve(ntrb_bytevec* const obj, const size_t additional_bytes){
	const size_t new_capacity = obj->capacity + additional_bytes;
	void* new_ptr = ntrb_realloc(obj->base_ptr, new_capacity);
	if(new_ptr == NULL) return false;
	
	obj->base_ptr = new_ptr;
	obj->capacity = new_capacity;
	return true;
}

bool ntrb_bytevec_append(ntrb_bytevec* const obj, const size_t typesize, const void* const value){
	while(obj->elements + typesize >= obj->capacity){
		bool reserve_success = ntrb_bytevec_reserve(obj, obj->capacity);
		if(!reserve_success) return false;
	}
	
	memcpy(obj->base_ptr + obj->elements, value, typesize);
	obj->elements += typesize;
	return true;
}	

void ntrb_bytevec_free(ntrb_bytevec* const obj){
	ntrb_free(obj->base_ptr);
	obj->base_ptr = NULL;
	obj->elements = 0;
	obj->capacity = 0;
}