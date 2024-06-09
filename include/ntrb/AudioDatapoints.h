#ifndef ntrb_AudioDatapoints_h
#define ntrb_AudioDatapoints_h

/**
Copyright 2024 Pawikan Boonnaum

This file is part of naturau-base/ntrb.

naturau-base/ntrb is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

naturau-base/ntrb is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with naturau-base/ntrb. If not, see <https://www.gnu.org/licenses/>.
*/


/**
\file AudioDatapoints.h
A module providing the ntrb_AudioDatapoints, a struct containing a fixed sized array, its size, and the offset from the pointer;
and functions for manipulating it.
*/

#include <stddef.h>
#include <stdint.h>

/**
A struct containing a fixed sized array, its size, and the offset from the pointer. Used in most of the audio related parts of ntrb.

If bytes is NULL, this typically indicates a failure has occured from a function or memory allocation.

See the functions for interacting with it in AudioDatapoints.h.
*/
typedef struct{
	uint8_t* bytes;		///< The allocated pointer.
	size_t byte_count;	///< How many bytes the pointer has access to.
	size_t byte_pos;	///< Offset from the pointer, used in audio playback.
} ntrb_AudioDatapoints;

/**
A value for failure return of a function or memory allocation.

*bytes is equal to NULL to indicate failure of a function or memory allocation.

Defined in AudioDatapoints.c
*/
extern const ntrb_AudioDatapoints failed_ntrb_AudioDatapoints;

/**
Allocates a fixed size array of size_bytes bytes, 0 set the array, set byte_count to size_bytes and byte_pos to 0.

If the allocation failed, the function returns failed_ntrb_AudioDatapoints. Else it returns a valid object.
*/
ntrb_AudioDatapoints ntrb_AudioDatapoints_new(const size_t size_bytes);

/**
Returns a separately allocated clone of orig. 

The *bytes of the clone is a separately allocated memory and has the same byte contents with orig.bytes.
The byte_count and byte_pos of the cloned object is also identical to that of the orig.

Returns failed_ntrb_AudioDatapoints if the function could not allocate the space for the new object.
*/
ntrb_AudioDatapoints ntrb_AudioDatapoints_copy(const ntrb_AudioDatapoints orig);

/**
Frees obj->bytes, sets it to NULL, and set byte_count and byte_pos to 0.
*/
void ntrb_AudioDatapoints_free(ntrb_AudioDatapoints* const obj);

#endif