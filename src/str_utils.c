#include "str_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


enum ntrb_GetCharStatus ntrb_getc(FILE* const instream, char* const ret){	
	const int getc_result = getc(instream);
	
	if(getc_result == EOF){
		const int ferror_id = ferror(instream);
		const int feof_id = feof(instream);
		clearerr(instream);
		
		if(feof_id) return ntrb_GetChar_EOF;
		else return (enum ntrb_GetCharStatus)(ntrb_GetChar_Ferror + ferror_id);		
	}
	
	*ret = getc_result;
	return ntrb_GetChar_OK;
}

enum ntrb_GetCharStatus ntrb_getsn(const size_t max_strlen, FILE* const instream, char** const ret){
	/*
	if an ferror is caught, we free the allocated *(char*) param, remember the ferror value, clear the error, and return the error enum.
	if an feof is caught, we return the read *(char*) param, remember an EOF has occurred, clear the error, and return EOF enum.
	if fgets reads without any errors, return the read *(char)* param and return an OK enum.
	*/
	*ret = calloc(max_strlen+1, sizeof(char));
	if(*ret == NULL) return ntrb_GetChar_AllocErr;
	
	//this reads max_strlen characters.
	char* const fgets_ptr = fgets(*ret, max_strlen+1, instream);

	if(fgets_ptr == NULL){
		free(*ret);
		*ret = NULL;
		
		const int feof_status = feof(instream);
		const int ferror_status = ferror(instream);
		clearerr(instream);
		
		if(feof_status) return ntrb_GetChar_EOF;
		else return (ntrb_GetChar_Ferror + ferror_status);
	}
	
	*ret = realloc(*ret, strlen(*ret)+1);
	const size_t last_char_it = strlen(*ret)-1;
	if((*ret)[last_char_it] == '\n') (*ret)[last_char_it] = '\0';
	
	return ntrb_GetChar_OK;
}

char* ntrb_trim_duplicate_separators(const char* const untrimmed_str, const size_t str_len, const char separator){	
	char* const trimmed_str = calloc(str_len+1, sizeof(char));
	if(trimmed_str == NULL) return NULL;
	
	size_t i_trimmed = 0;
	bool prev_was_separator = false;
	for(size_t i_untrimmed = 0; i_untrimmed < str_len; i_untrimmed++){
		const char untrimmed_c = untrimmed_str[i_untrimmed];
		const bool is_separator = untrimmed_c == separator;
		const bool duplicating_separator = prev_was_separator && is_separator;
		if(duplicating_separator) continue;
		
		prev_was_separator = is_separator;
		trimmed_str[i_trimmed] = untrimmed_c;
		i_trimmed++;
	}
	
	return trimmed_str;
}

char* ntrb_get_filetype(const char* const filename){	
	const char* filetype_separator = strrchr(filename, '.');
	if(!filetype_separator) return NULL;
	
	//if the period is the last character, this would point to a null terminator, the strlen of filetype_begin would be 0.
	const char* filetype_begin = filetype_separator + 1;
	const size_t filetype_len = strlen(filetype_begin);
	if(!filetype_len) return NULL;
	
	char* filetype = calloc(filetype_len + 1, sizeof(char));
	memcpy(filetype, filetype_begin, filetype_len);
	return filetype;
}