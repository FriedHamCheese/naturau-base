#include "_alloc_bytevec.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

const _ntrb_alloc_bytevec failed__ntrb_alloc_bytevec = {.base_ptr = NULL, .elements = 0, .capacity = 0};

_ntrb_alloc_bytevec _ntrb_alloc_bytevec_new(const size_t alloc_bytes){
	_ntrb_alloc_bytevec obj;
	obj.base_ptr = malloc(alloc_bytes);
	if(obj.base_ptr == NULL) return failed__ntrb_alloc_bytevec;
	
	obj.elements = 0;
	obj.capacity = alloc_bytes;
	return obj;
}
bool _ntrb_alloc_bytevec_reserve(_ntrb_alloc_bytevec* const obj, const size_t additional_bytes){
	const size_t new_capacity = obj->capacity + additional_bytes;
	void* new_ptr = realloc(obj->base_ptr, new_capacity);
	if(new_ptr == NULL) return false;
	
	obj->base_ptr = new_ptr;
	obj->capacity = new_capacity;
	return true;
}

bool _ntrb_alloc_bytevec_append(_ntrb_alloc_bytevec* const obj, const size_t typesize, const void* const value){
	while(obj->elements + typesize >= obj->capacity){
		bool reserve_success = _ntrb_alloc_bytevec_reserve(obj, obj->capacity);
		if(!reserve_success) return false;
	}
	
	memcpy(obj->base_ptr + obj->elements, value, typesize);
	obj->elements += typesize;
	return true;
}

void _ntrb_alloc_bytevec_free(_ntrb_alloc_bytevec* const obj){
	free(obj->base_ptr);
	obj->base_ptr = NULL;
	obj->elements = 0;
	obj->capacity = 0;
}