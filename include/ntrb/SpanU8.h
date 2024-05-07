#ifndef ntrb_SpanU8_h
#define ntrb_SpanU8_h

#include <stdint.h>
#include <stddef.h>

/**
A struct containing an allocated pointer and its fixed allocated size in bytes.

For a variable-size equivalent see: ntrb_bytevec.
*/
typedef struct{
	uint8_t* ptr; 	///< Pointer to the allocated memory. 
	size_t elem;	///< Size (elements) of the allocated memory in bytes.
} ntrb_SpanU8;

#endif