/**
Copyright 2024 Pawikan Boonnaum

This file is part of naturau-base/ntrb.

naturau-base/ntrb is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

naturau-base/ntrb is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with naturau-base/ntrb. If not, see <https://www.gnu.org/licenses/>.
*/

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
		//Doubling obj's capacity.
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