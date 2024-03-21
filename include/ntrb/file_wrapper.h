#ifndef ntrb_filewrapper_h
#define ntrb_filewrapper_h

#include "SpanU8.h"
#include <stdio.h>

//from file_wrapper.c
extern const int ntrb_get_filesize_bytes_seekend_error;
extern const int ntrb_get_filesize_bytes_ftell_error;
extern const int ntrb_get_filesize_bytes_seekset_error;

enum ntrb_ReadFileResult{
	ntrb_ReadFileResult_OK,
	ntrb_ReadFileResult_FileOpenError,
	ntrb_ReadFileResult_FilesizeError,
	ntrb_ReadFileResult_CallocError,
	ntrb_ReadFileResult_FileReadError,
};

long int ntrb_get_filesize_bytes(FILE* const file);
enum ntrb_ReadFileResult ntrb_read_entire_file_rb(ntrb_SpanU8* const buffer, const char* filename);

#endif