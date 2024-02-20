#include "AudioDatapoints.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

const ntrb_AudioDatapoints failed_ntrb_AudioDatapoints = {.bytes=NULL, .byte_count=0, .byte_pos=0};

ntrb_AudioDatapoints new_ntrb_AudioDatapoints(const size_t size_bytes){
	ntrb_AudioDatapoints o;
	if(size_bytes == 0) return failed_ntrb_AudioDatapoints;
	o.bytes = calloc(size_bytes, sizeof(uint8_t));
	o.byte_count = size_bytes;
	o.byte_pos = 0;
	
	return o;
}

ntrb_AudioDatapoints copy_ntrb_AudioDatapoints(const ntrb_AudioDatapoints orig){
	ntrb_AudioDatapoints clone = new_ntrb_AudioDatapoints(orig.byte_count);
	if(clone.bytes == NULL) return clone;
	
	memcpy(clone.bytes, orig.bytes, orig.byte_count);
	clone.byte_pos = orig.byte_pos;
	return clone;
}