#ifndef ntrb_str_utils_h
#define ntrb_str_utils_h

#include <stdio.h>

/**
\file str_utils.h
A module providing function for reading characters from a FILE* and misc. functions related to strings.
*/

/**
An enum for functions which gets a character or characters from a FILE* object.
\todo dont do nested enums. it's bad.
*/
enum ntrb_GetCharStatus{
	ntrb_GetChar_OK,		///< wow!
	ntrb_GetChar_AllocErr,	///< Failed to allocate memory. Does not happen in ntrb_getc().
	ntrb_GetChar_EOF,       ///< Reached EOF while reading FILE*.
	ntrb_GetChar_Ferror,    ///< Recieved an ferror while reading FILE*, this is an offset of ferror. Subtract the return enum with this to get the ferror.
};

/**
Gets a character from the provided FILE* and writes it to ret.

\param[in] instream The FILE* to getc() from.
\param[out] ret A pointer to a valid char to write to. 

If the return value is not ntrb_GetChar_OK, the return pointer is not written to 
  and the FILE* will be clearerr()'ed if reached EOF or ferror.
*/
enum ntrb_GetCharStatus ntrb_getc(FILE* const instream, char* const ret);

/**
Allocates the char* which ret points to with max_strlen + 1 bytes and write the contents of a line read from instream to it.

The string written to the char* will not contain a newline character, and if the line is shorter than max_strlen or the instream reached EOF, the string is filled with 0s after the contents read.

- If the function failed to allocate space to the char*, nothing gets read and the function returns ntrb_GetChar_AllocErr.
- If an ferror is caught, the char* is deallocated, the instream gets clearerr()'ed, and ntrb_GetChar_Ferror is returned.
- If an EOF is caught, the char* contains what the stream could read before reaching it, the instream gets clearerr()'ed, and ntrb_GetChar_EOF is returned.
*/
enum ntrb_GetCharStatus ntrb_getsn(const size_t max_strlen, FILE* const instream, char** ret);

/**
Returns a separately allocated string which has no separators next to each other (only one separator).

The allocated string is allocated and 0 filled by str_len bytes.
Technically it just removes duplicate characters and only keep one. So uh ok.

Returns NULL if the function failed to allocate the space for the string.
*/
char* ntrb_trim_duplicate_separators(const char* const untrimmed_str, const size_t str_len, const char separator);

/**
Gets the filetype from the argument.

The argument is expected to be in "filename.filetype" format, 
and the function will return a separately allocated "filetype" string (without the period).

Returns a valid string if the filetype exists. Else it returns NULL.
So "filename." or "filename" is going to return NULL. ".filetype" does return "filetype" as the string.
*/
char* ntrb_get_filetype(const char* const filename);

#endif
