#ifndef _ntrb_alloc_bytevec_h
#define _ntrb_alloc_bytevec_h

/**
\file _alloc_bytevec.h
A module providing bytevec: a variable length array container, for the alloc module.
*/

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/**
A variable length array container for the alloc module specifically.

The ntrb_bytevec container itself uses memory management functions from the alloc module if NTRB_MEMDEBUG is defined. 
Which will cause a cyclic dependency issue from the alloc module using the ntrb_bytevec container to expand its memory allocation list, which leads to an infinitely recursive function calling between ntrb_bytevec and the alloc module.
We decided to make a copy of ntrb_bytevec which uses the standard library memory management function calls, hence this container.

If the base_ptr is NULL, this indicates a failure from a function and the container should not be further accessed.

This struct has methods for manipulating it which are in _alloc_bytevec.h
*/
typedef struct{
	uint8_t* base_ptr;	///< The pointer to the first byte of the array.
	size_t elements;	///< How many bytes you can access from the pointer before running into garbage.
	size_t capacity;	///< How many bytes the pointer have access to, including garbage.
} _ntrb_alloc_bytevec;

/**
A quick object used for returning a fail state. 

Functions can return this to indicate that either the function has failed or the allocation of the container has failed.

If the base_ptr is NULL, this indicates a failure from a function and the container should not be further accessed.

Defined in _alloc_bytevec.c
*/
extern const _ntrb_alloc_bytevec failed__ntrb_alloc_bytevec;

/**
Initialises a new _ntrb_alloc_bytevec object.
The base_ptr gets allocated with alloc_bytes bytes of heap memory.
elements is set to 0 bytes and capacity is set to alloc_bytes.

If the function fails to allocate the memory, it returns failed__ntrb_alloc_bytevec.

If alloc_bytes is 0, the behaviour depends on the implementation of malloc.
If malloc returns NULL when allocating 0 bytes, the function returns failed__ntrb_alloc_bytevec. Else, it returns a valid object and should be usable.
*/
_ntrb_alloc_bytevec _ntrb_alloc_bytevec_new(const size_t alloc_bytes);

/**
Reserves additional_bytes bytes ahead from obj->capacity, so the new capacity would be additional_bytes + obj->capacity.

If the function fails, it returns false and the state of the container is left unchanged. 

If succeeded, returns true, and the obj->base_ptr has the exact amount of additional space requested and obj->capacity is incremented by additional_bytes.
obj->elements does not change.
*/
bool _ntrb_alloc_bytevec_reserve(_ntrb_alloc_bytevec* const obj, const size_t additional_bytes);

/**
Adds the value at the end of the container.

The function dereferences the pointer of the value and copies its contents by typesize bytes next to obj->elements. The obj->elements is incremented by typesize bytes.

If obj->capacity is not enough for containing the value, the container reserves additional space and then adds the value to the end of the array.

If the typesize is more than the type of the value argument, memcpy would read the contents beyond the value and adds them to the array. If it is less, then memcpy would only copy the first typesize bytes to the array.

The function returns true if the append process is successful. If the container required additional space and was not able to allocate the required space, the state of the container is left unchanged and the function returns false.
*/
bool _ntrb_alloc_bytevec_append(_ntrb_alloc_bytevec* const obj, const size_t typesize, const void* const value);

/**
Frees the base_ptr, then sets it to NULL and sets both the elements and capacity to 0.
*/
void _ntrb_alloc_bytevec_free(_ntrb_alloc_bytevec* const obj);

#endif