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

ntrb_AudioDatapoints new_ntrb_AudioDatapoints(const size_t size_bytes);
ntrb_AudioDatapoints copy_ntrb_AudioDatapoints(const ntrb_AudioDatapoints orig);
#endif