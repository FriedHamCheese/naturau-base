#ifndef ntrb_SpanU8_h
#define ntrb_SpanU8_h

/**
Copyright 2024 Pawikan Boonnaum

This file is part of naturau-base/ntrb.

naturau-base/ntrb is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

naturau-base/ntrb is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with naturau-base/ntrb. If not, see <https://www.gnu.org/licenses/>.
*/


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