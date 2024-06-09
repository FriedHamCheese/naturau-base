#ifndef ntrb_filewrapper_h
#define ntrb_filewrapper_h

/**
Copyright 2024 Pawikan Boonnaum

This file is part of naturau-base/ntrb.

naturau-base/ntrb is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

naturau-base/ntrb is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with naturau-base/ntrb. If not, see <https://www.gnu.org/licenses/>.
*/


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
	ntrb_ReadFileResult_EOF				///< An end of file has been reached.
};

///Returns the filesize in bytes. Returns -1 if an error occured.
long int ntrb_get_filesize_bytes(FILE* const file);

/**
Reads the file contents from a file from the provided filename which the filename may be a relative path from the program callsite or an absolute filepath. Then allocates memory to the provided uninitialised ntrb_SpanU8 and writes the file contents to it. buffer->elem is set to the filesize of the file.

buffer must be an uninitialised ntrb_SpanU8.

All errors guarantee the provided buffer is deallocated if the function allocates it.
*/
enum ntrb_ReadFileResult ntrb_read_entire_file_rb(ntrb_SpanU8* const buffer, const char* filename);

/**
Reads [remaining_filesize, bytes_to_read] bytes from *file.

\param[out] buffer An uninitialised ntrb_SpanU8.
\param[in]  file A FILE* of an initialised file descriptor.
\param[in]  bytes_to_read The amount of bytes to try to read.

If any errors occurred, the buffer contents will be freed.

\return ntrb_ReadFileResult_FilesizeError if any errors occurred while detemining the remaining filesize.
  If an filesize related error occurs, the function will seek the file position back to where it was.
\return ntrb_ReadFileResult_FileReadError if an error occurred while seeking the file position back
  after successfully determining the remaining filesize, or an ferror occurs.
\return ntrb_ReadFileResult_CallocError if an error occurred when attempting to allocate buffer.
*/
enum ntrb_ReadFileResult ntrb_readsome_from_file_rb(ntrb_SpanU8* const buffer, FILE* const file, const size_t bytes_to_read);

#endif