#ifndef _ntrb_alloc_bytevec_h
#define _ntrb_alloc_bytevec_h

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct{
	uint8_t* base_ptr;
	size_t elements;
	size_t capacity;
} _ntrb_alloc_bytevec;

//from bytevec.c
extern const _ntrb_alloc_bytevec failed__ntrb_alloc_bytevec;

_ntrb_alloc_bytevec _ntrb_alloc_bytevec_new(const size_t alloc_bytes);
bool _ntrb_alloc_bytevec_reserve(_ntrb_alloc_bytevec* const obj, const size_t additional_bytes);
bool _ntrb_alloc_bytevec_append(_ntrb_alloc_bytevec* const obj, const size_t typesize, const void* const value);
void _ntrb_alloc_bytevec_free(_ntrb_alloc_bytevec* const obj);

#endif