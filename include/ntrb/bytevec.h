#ifndef ntrb_bytevec_h
#define ntrb_bytevec_h

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct{
	uint8_t* base_ptr;
	size_t elements;
	size_t capacity;
} ntrb_bytevec;

//from bytevec.c
extern const ntrb_bytevec failed_ntrb_bytevec;

ntrb_bytevec ntrb_bytevec_new(const size_t alloc_bytes);
bool ntrb_bytevec_reserve(ntrb_bytevec* const obj, const size_t additional_bytes);
bool ntrb_bytevec_append(ntrb_bytevec* const obj, const size_t typesize, const void* const value);
void ntrb_bytevec_free(ntrb_bytevec* const obj);

#endif