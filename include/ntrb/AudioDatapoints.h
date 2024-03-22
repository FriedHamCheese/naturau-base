#ifndef ntrb_AudioDatapoints_h
#define ntrb_AudioDatapoints_h

#include <stddef.h>
#include <stdint.h>

typedef struct{
	uint8_t* bytes;
	size_t byte_count;
	size_t byte_pos;
} ntrb_AudioDatapoints;

extern const ntrb_AudioDatapoints failed_ntrb_AudioDatapoints;

ntrb_AudioDatapoints ntrb_AudioDatapoints_new(const size_t size_bytes);
ntrb_AudioDatapoints ntrb_AudioDatapoints_copy(const ntrb_AudioDatapoints orig);
void ntrb_AudioDatapoints_free(ntrb_AudioDatapoints* const obj);

#endif