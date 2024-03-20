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

bool ntrb_bytevec_new(ntrb_bytevec* const obj, const size_t alloc_bytes);
bool ntrb_bytevec_reserve(ntrb_bytevec* const obj, const size_t additional_bytes);
void ntrb_bytevec_free(ntrb_bytevec* const obj);

#endif