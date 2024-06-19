#ifndef ntrb_str_utils_h
#define ntrb_str_utils_h

/*
Copyright 2024 Pawikan Boonnaum

This file is part of naturau-base/ntrb.

naturau-base/ntrb is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

naturau-base/ntrb is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with naturau-base/ntrb. If not, see <https://www.gnu.org/licenses/>.
*/


#include <stdio.h>

/**
\file str_utils.h
A module providing function for reading characters from a FILE* and misc. functions related to strings.
*/

/**
An enum for functions which gets a character or characters from a FILE* object.
*/
enum ntrb_GetCharStatus{
	ntrb_GetChar_OK,		///< wow!
	ntrb_GetChar_AllocErr,	///< Failed to allocate memory. Does not happen in ntrb_getc().
	ntrb_GetChar_EOF,       ///< Reached EOF while reading FILE*.
	ntrb_GetChar_Ferror,    ///< Recieved an ferror while reading FILE*
};

#ifdef __cplusplus
extern "C"{	
#endif

/**
Gets a character from the provided FILE* and writes it to ret.

\param[in] instream The FILE* to getc() from.
\param[out] ret A pointer to a valid char to write to. 

If the return value is not ntrb_GetChar_OK and the return pointer is not written to if reached EOF or ferror.
*/
enum ntrb_GetCharStatus ntrb_getc(FILE* const instream, char* const ret);

/**
Allocates the char* which ret points to with max_strlen + 1 bytes and write the contents of a line read from instream to it.

The string written to the char* will not contain a newline character, and if the line is shorter than max_strlen or the instream reached EOF, the string is filled with 0s after the contents read.

- If the function failed to allocate space to the char*, nothing gets read and the function returns ntrb_GetChar_AllocErr.
- If an ferror is caught, the char* which was allocated by the function is deallocated and ntrb_GetChar_Ferror is returned from the function.
- If an EOF is caught, the char* contains what the stream could read before reaching it and ntrb_GetChar_EOF is returned from the function.
*/
enum ntrb_GetCharStatus ntrb_getsn(const size_t max_strlen, FILE* const instream, char** ret);

/**
Returns a separately allocated string which is a variant of untrimmed_str but has no separators next to each other.

Technically it just removes duplicate characters and only keep one. So uh ok.

Returns NULL if the function failed to allocate the space for the string.
*/
char* ntrb_trim_duplicate_separators(const char* const untrimmed_str, const size_t str_len, const char separator);

char* ntrb_trim_whitespace(const char* const untrimmed_str);

/**
Gets the filetype from the argument.

The argument is expected to be in "filename.filetype" format, 
and the function will return a separately allocated "filetype" string (without the period).

Returns a valid string if the filetype exists. Else it returns NULL.
So "filename." or "filename" is going to return NULL. ".filetype" does return "filetype" as the string.
*/
char* ntrb_get_filetype(const char* const filename);

#ifdef __cplusplus
};
#endif

#endif
