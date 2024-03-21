#include "file_wrapper.h"
#include "SpanU8.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

const int ntrb_get_filesize_bytes_seekend_error = -1;
const int ntrb_get_filesize_bytes_ftell_error = -2;
const int ntrb_get_filesize_bytes_seekset_error = -3;

long int ntrb_get_filesize_bytes(FILE* const file){
	const int fseek_ok = 0;
	
	int seekend_result = fseek(file, 0, SEEK_END);
	if(seekend_result != fseek_ok) return -1;
	
	const long int ftell_err  =-1;
	const long int filesize = ftell(file);
	if(filesize == ftell_err) return -2;	
	
	int seekset_result = fseek(file, 0, SEEK_SET);
	if (seekset_result != fseek_ok) return -3;
	
	return filesize;
}

enum ntrb_ReadFileResult ntrb_read_entire_file_rb(ntrb_SpanU8* const buffer, const char* filename){
	enum ntrb_ReadFileResult result = ntrb_ReadFileResult_OK;
	
	//audio_file Alloc, free using fclose()
	FILE* audio_file = fopen(filename, "rb");
	if(audio_file == NULL) return ntrb_ReadFileResult_FileOpenError;
	
	//using int64_t instead of long int because a long int may be 32 bits,
	//which may cause an error when comparing with max of size_t which may also be unsigned long int, 32 bits wide.
	//and since a filesize of 2^63-1 is highly unlikely, this is probably more ideal to prevent signed/unsigned comparsion.
	const int64_t minimum_normal_filesize_return = 0;
	const int64_t filesize_bytes = ntrb_get_filesize_bytes(audio_file);
	if(filesize_bytes < minimum_normal_filesize_return){
		result = ntrb_ReadFileResult_FilesizeError;
		goto close_file;
	}
	
	//buffer Alloc, this ptr is the return value
	buffer->ptr = (uint8_t*)calloc(filesize_bytes, sizeof(uint8_t));
	if(buffer->ptr == NULL){
		result = ntrb_ReadFileResult_CallocError;
		goto close_file;
	}
	
	//casting from size_t to int64_t because of signed/unsigned compare with filesize_bytes,
	//and the probability of a file having the size of 2^63-1 is highly unlikely,
	//meaning that the cast should be safe for most scenarios.
	const int64_t bytes_read = fread(buffer->ptr, sizeof(uint8_t), filesize_bytes, audio_file);
	buffer->elem = bytes_read;
	
	if(bytes_read != filesize_bytes){
		//eof would never happen, because read binary reads all the characters/bytes
		
		//ferror happened, we choose to not return buffer which may contain errors.
		free(buffer->ptr);
		buffer->ptr = NULL;
		result = ntrb_ReadFileResult_FileReadError;
	}
	
	close_file:
	//audio_file Free
	fclose(audio_file);
	
	return result;
}