#ifndef ntrb_SlicedStrings_h
#define ntrb_SlicedStrings_h

/**
Copyright 2024 Pawikan Boonnaum

This file is part of naturau-base/ntrb.

naturau-base/ntrb is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

naturau-base/ntrb is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with naturau-base/ntrb. If not, see <https://www.gnu.org/licenses/>.
*/


/**
\file SlicedStrings.h
A module providing ntrb_SlicedStrings, a container for substrings; and the functions for manipulating it.
*/

#include <stddef.h>

/**
A container of substrings (sliced strings) in a string which are separately allocated from the main string.

If str_ptrs is NULL, this indicates failure from a function or failure to allocate memory.

See SlicedStrings.h for methods for manipulating the container.
*/
typedef struct{
	char** str_ptrs;	///< An array of char* which the char** is allocated and each of the char* is also separately allocated.
	size_t elem;		///< The amount of char* which the container has access to.
} ntrb_SlicedStrings;

/**
A quick value for failure returns of a function or memory allocation.

If the object you recieve indicates failure from a function or memory allocation, its str_ptrs will be NULL.

Defined in SlicedStrings.c
*/
extern const ntrb_SlicedStrings failed_ntrb_SlicedStrings;

/**
Maximum length of a substring, not including the null terminator. 

Defined in SlicedStrings.c
*/
extern const size_t ntrb_slice_string_max_len;

/**
Creates a new ntrb_SlicedStrings and allocates its char** to store (str_count) char pointers, 
and (str_count) substrings each with its own allocated ntrb_slice_string_max_len + 1 bytes of space.

Typically you don't use this directly, it's for the methods in the module. But can't see a reason why you can't.

Returns an object which the str_ptrs isn't NULL if successful. Else returns one with NULL.
*/
ntrb_SlicedStrings ntrb_SlicedStrings_new(const size_t str_count);

/**
Frees each of the char* in the object, then the char** of the object, set str_ptrs to NULL and elem to 0.

If obj is NULL, the function does nothing. If obj->str_ptrs is NULL, it does nothing.
*/
void ntrb_SlicedStrings_free(ntrb_SlicedStrings* const obj);

/**
Returns the substrings which are separated by the separator argument in the provided string.

This function does not truncate (trim) each of the substrings to its length, leaving 0 filled space after the substrings.
It also doesn't remove duplicate separators, which can lead to empty substrings. 

This function is a part of ntrb_SlicedStrings_slice_sep() and you should use that instead.

\param str the main string to reference from.
\param str_len the length of the main string.
\param separator the separator which separate the substrings each other.

Returns an object which the str_ptrs isn't NULL if successful. Else returns one with NULL.

*/
ntrb_SlicedStrings ntrb_SlicedStrings_slice_without_trimming(const char* const str, const size_t str_len, const char separator);

/**
Returns the substrings which are separated by the separator argument in the provided string.

\param str the main string to reference from.
\param str_len the length of the main string.
\param separator the separator which separate the substrings from each other.

If the separator is a null character, it returns failed_ntrb_SlicedStrings.

Returns an object which the str_ptrs isn't NULL if successful. Else returns one with NULL.
*/
ntrb_SlicedStrings ntrb_SlicedStrings_slice_sep(const char* const str, const size_t str_len, const char separator);

/**
Returns the substrings which are separated by a space character in the provided string.

\param str the main string to reference from.
\param str_len the length of the main string.

Returns an object which the str_ptrs isn't NULL if successful. Else returns one with NULL.
*/
ntrb_SlicedStrings ntrb_SlicedStrings_slice(const char* const str, const size_t str_len);

/**
Concatenates the strings in the object to a single, separately allocated string.

\param slices A valid object with valid substrings to concatenate the strings from.
\param beg An index (starts at 0) of the string in slices.sub_strs to start concatenation from.
\param end An index after the index of the last string to concatenate from, pass in slices.elem to concatenate to the last string.
\param separator A character to put in between the substrings while concatenating.

- If separator is a null character, the function joins the string without a separator in between.
- if beg > end, the function returns NULL.
- if (beg or end) > slices.elem, the function returns NULL.

Returns the concatenated string which is separately allocated, NULL if allocation error occured.
*/
char* ntrb_SlicedStrings_concat_strs(const ntrb_SlicedStrings slices, const size_t beg, const size_t end, const char separator);

#endif