/*
Copyright 2024 Pawikan Boonnaum

This file is part of naturau-base/ntrb.

naturau-base/ntrb is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

naturau-base/ntrb is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with naturau-base/ntrb. If not, see <https://www.gnu.org/licenses/>.
*/

#include "file_wrapper.h"
#include "SpanU8.h"

#include "alloc.h"
#include "utils.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

long int ntrb_get_filesize_bytes(FILE* const file){
	const int fseek_ok = 0;
	
	int seekend_result = fseek(file, 0, SEEK_END);
	if(seekend_result != fseek_ok) return -1;
	
	const long int ftell_err  =-1;
	const long int filesize = ftell(file);
	if(filesize == ftell_err) return -1;	
	
	int seekset_result = fseek(file, 0, SEEK_SET);
	if (seekset_result != fseek_ok) return -1;
	
	return filesize;
}

enum ntrb_ReadFileResult ntrb_read_entire_file_rb(ntrb_SpanU8* const buffer, const char* filename){
	enum ntrb_ReadFileResult result = ntrb_ReadFileResult_OK;
	
	//audio_file Alloc, free using fclose()
	FILE* audio_file = fopen(filename, "rb");
	if(audio_file == NULL) return ntrb_ReadFileResult_FileOpenError;
	
	//Filesize should not be a negative number, otherwise the filesize function failed to get the filesize.
	const long int minimum_normal_filesize_return = 0;
	const long int filesize_bytes = ntrb_get_filesize_bytes(audio_file);
	if(filesize_bytes < minimum_normal_filesize_return){
		result = ntrb_ReadFileResult_FilesizeError;
		goto close_file;
	}
	
	//buffer Alloc, this ptr is the return value
	buffer->ptr = (uint8_t*)ntrb_calloc(filesize_bytes, sizeof(uint8_t));
	if(buffer->ptr == NULL){
		result = ntrb_ReadFileResult_CallocError;
		goto close_file;
	}
	
	const size_t bytes_read = fread(buffer->ptr, sizeof(uint8_t), filesize_bytes, audio_file);
	buffer->elem = bytes_read;
	
	//This cast is fine. 
	//It only causes issues when filesize_bytes has a signed bit, which is checked before reaching here.
	if(bytes_read != (size_t)filesize_bytes){
		//eof would never happen, because read binary reads all the characters/bytes
		
		//ferror happened, we choose to not return buffer which may contain errors.
		ntrb_free(buffer->ptr);
		buffer->ptr = NULL;
		result = ntrb_ReadFileResult_FileReadError;
	}
	
	close_file:
	//audio_file Free
	fclose(audio_file);
	
	return result;
}

enum ntrb_ReadFileResult ntrb_readsome_from_file_rb(ntrb_SpanU8* const buffer, FILE* const file, const size_t bytes_to_read){
	const long current_filepos = ftell(file);
	if(current_filepos == -1) return ntrb_ReadFileResult_FilesizeError;
	if(fseek(file, 0, SEEK_END) != 0) return ntrb_ReadFileResult_FilesizeError;
	
	const long end_filepos = ftell(file);
	if(end_filepos == -1){
		fseek(file, current_filepos, SEEK_SET);
		return ntrb_ReadFileResult_FilesizeError;
	}
	const long filesize_left = end_filepos - current_filepos;
	if(fseek(file, current_filepos, SEEK_SET) != 0) return ntrb_ReadFileResult_FileReadError;
	if(filesize_left == 0) return ntrb_ReadFileResult_EOF;
	
	const size_t clamped_bytes_to_read = ntrb_clamp_u64(bytes_to_read, 0, filesize_left);
	
	buffer->ptr = ntrb_calloc(clamped_bytes_to_read, sizeof(uint8_t));
	if(buffer->ptr == NULL) return ntrb_ReadFileResult_CallocError;
	
	const size_t bytes_read = fread(buffer->ptr, sizeof(uint8_t), clamped_bytes_to_read, file);
	buffer->elem = bytes_read;
	
	if(bytes_read != clamped_bytes_to_read){
		//ferror happened, we choose to not return buffer which may contain errors.
		ntrb_free(buffer->ptr);
		buffer->ptr = NULL;
		return ntrb_ReadFileResult_FileReadError;
	}
	
	return ntrb_ReadFileResult_OK;
}