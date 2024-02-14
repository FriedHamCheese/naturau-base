#ifndef ntrb_SpanU8_h
#define ntrb_SpanU8_h

#include <stdint.h>
#include <stddef.h>

typedef struct{
	uint8_t* ptr;
	size_t elem;
} ntrb_SpanU8;

#endif