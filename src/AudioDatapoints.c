/*
Copyright 2024 Pawikan Boonnaum

This file is part of naturau-base/ntrb.

naturau-base/ntrb is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

naturau-base/ntrb is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with naturau-base/ntrb. If not, see <https://www.gnu.org/licenses/>.
*/

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