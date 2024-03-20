#include "bytevec.h"

#include <stdlib.h>
#include <stdbool.h>

bool ntrb_bytevec_new(ntrb_bytevec* const obj, const size_t alloc_bytes){
	obj->base_ptr = malloc(alloc_bytes);
	if(obj->base_ptr == NULL) return false;
	
	obj->elements = 0;
	obj->capacity = alloc_bytes;
	return true;
}
bool ntrb_bytevec_reserve(ntrb_bytevec* const obj, const size_t additional_bytes){
	const size_t new_capacity = obj->capacity + additional_bytes;
	void* new_ptr = realloc(obj->base_ptr, new_capacity);
	if(new_ptr == NULL) return false;
	
	obj->base_ptr = new_ptr;
	obj->capacity = new_capacity;
	return true;
}

void ntrb_bytevec_free(ntrb_bytevec* const obj){
	free(obj->base_ptr);
	obj->base_ptr = NULL;
	obj->elements = 0;
	obj->capacity = 0;
}