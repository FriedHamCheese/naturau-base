#ifndef ntrb_filewrapper_h
#define ntrb_filewrapper_h

/**
\file file_wrapper.h
A module providing functions abstracting file reading.
*/

#include "SpanU8.h"
#include <stdio.h>

/**
Enum for error report from ntrb_read_entire_file_rb().
*/
enum ntrb_ReadFileResult{
	ntrb_ReadFileResult_OK,				///< Congratulations!
	ntrb_ReadFileResult_FileOpenError,	///< Error from opening file, usually because the file doesn't exist in the path.
	ntrb_ReadFileResult_FilesizeError,	///< Error from getting filesize.
	ntrb_ReadFileResult_CallocError,	///< Error from allocating memory to contain the file contents.
	ntrb_ReadFileResult_FileReadError,	///< Error from reading file contents.
};

///Returns the filesize in bytes. Returns -1 if an error occured.
long int ntrb_get_filesize_bytes(FILE* const file);

/**
Reads the file contents from a file from the provided filename which the filename may be a relative path from the program callsite or an absolute filepath. Then allocates memory to the provided uninitialised ntrb_SpanU8 and writes the file contents to it. buffer->elem is set to the filesize of the file.

All errors guarantee the provided buffer is deallocated if the function allocates it.
*/
enum ntrb_ReadFileResult ntrb_read_entire_file_rb(ntrb_SpanU8* const buffer, const char* filename);
enum ntrb_ReadFileResult ntrb_read_entire_file_rb_ptr(ntrb_SpanU8* const buffer, FILE* const file);
enum ntrb_ReadFileResult ntrb_readsome_from_file_rb(ntrb_SpanU8* const buffer, FILE* const file, const size_t bytes_to_read);

#endif