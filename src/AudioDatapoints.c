#include "AudioDatapoints.h"
#include "alloc.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

const ntrb_AudioDatapoints failed_ntrb_AudioDatapoints = {.bytes=NULL, .byte_count=0, .byte_pos=0};

ntrb_AudioDatapoints ntrb_AudioDatapoints_new(const size_t size_bytes){
	ntrb_AudioDatapoints o;
	if(size_bytes == 0) return failed_ntrb_AudioDatapoints;
	o.bytes = ntrb_calloc(size_bytes, sizeof(uint8_t));
	o.byte_count = size_bytes;
	o.byte_pos = 0;
	
	return o;
}

ntrb_AudioDatapoints ntrb_AudioDatapoints_copy(const ntrb_AudioDatapoints orig){
	ntrb_AudioDatapoints clone = ntrb_AudioDatapoints_new(orig.byte_count);
	if(clone.bytes == NULL) return clone;
	
	memcpy(clone.bytes, orig.bytes, orig.byte_count);
	clone.byte_pos = orig.byte_pos;
	return clone;
}

void ntrb_AudioDatapoints_free(ntrb_AudioDatapoints* const obj){
	ntrb_free(obj->bytes);
	obj->bytes = NULL;
	obj->byte_count = 0;
	obj->byte_pos = 0;
}