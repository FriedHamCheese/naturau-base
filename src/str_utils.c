#include "str_utils.h"
#include "alloc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


enum ntrb_GetCharStatus ntrb_getc(FILE* const instream, char* const ret){	
	const int getc_result = getc(instream);
	
	if(getc_result == EOF){
		const int feof_id = feof(instream);
		
		if(feof_id) return ntrb_GetChar_EOF;
		else return ntrb_GetChar_Ferror;
	}
	
	*ret = getc_result;
	return ntrb_GetChar_OK;
}

enum ntrb_GetCharStatus ntrb_getsn(const size_t max_strlen, FILE* const instream, char** const ret){
	/*
	- if an ferror is caught, we free the allocated *(char*) param and the function returns ntrb_GetChar_Ferror.
	- if an feof is caught, we return the read *(char*) param and return ntrb_GetChar_EOF.
	- if fgets reads without any errors, return the read *(char)* param and return an OK enum.
	*/
	*ret = ntrb_calloc(max_strlen+1, sizeof(char));
	if(*ret == NULL) return ntrb_GetChar_AllocErr;
	
	//this reads max_strlen characters.
	char* const fgets_ptr = fgets(*ret, max_strlen+1, instream);

	if(fgets_ptr == NULL){
		ntrb_free(*ret);
		*ret = NULL;
		
		const int feof_status = feof(instream);
		
		if(feof_status) return ntrb_GetChar_EOF;
		else return ntrb_GetChar_Ferror;
	}
	
	*ret = ntrb_realloc(*ret, strlen(*ret)+1);
	const size_t last_char_it = strlen(*ret)-1;
	if((*ret)[last_char_it] == '\n') (*ret)[last_char_it] = '\0';
	
	return ntrb_GetChar_OK;
}

char* ntrb_trim_duplicate_separators(const char* const untrimmed_str, const size_t str_len, const char separator){	
	char* const trimmed_str = ntrb_calloc(str_len+1, sizeof(char));
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

char* ntrb_trim_whitespace(const char* const untrimmed_str){
	const int untrimmed_strlen = strlen(untrimmed_str);
	int first_nonwhitespace_character_index = untrimmed_strlen;
	
	for(int i = 0; i < untrimmed_strlen; i++){
		if(untrimmed_str[i] != ' '){
			first_nonwhitespace_character_index = i;
			break;
		}
	}
	
	const bool all_whitespace = first_nonwhitespace_character_index == untrimmed_strlen;
	if(all_whitespace) return ntrb_calloc(1, sizeof(char));
	
	int last_nonwhitespace_character_index = untrimmed_strlen;
	for(int i = untrimmed_strlen-1; i >= first_nonwhitespace_character_index; i--){
		if(untrimmed_str[i] != ' '){
			first_nonwhitespace_character_index = i;
			break;
		}
	}
	
	const int trimmed_strlen = (last_nonwhitespace_character_index - first_nonwhitespace_character_index) + 1;
	char* trimmed_str = ntrb_calloc(trimmed_strlen + 1, sizeof(char));
	if(trimmed_str == NULL) return NULL;
	
	memcpy(trimmed_str, untrimmed_str + first_nonwhitespace_character_index, trimmed_strlen);
	return trimmed_str;
}

char* ntrb_get_filetype(const char* const filename){	
	const char* filetype_separator = strrchr(filename, '.');
	if(!filetype_separator) return NULL;
	
	//if the period is the last character, this would point to a null terminator, the strlen of filetype_begin would be 0.
	const char* filetype_begin = filetype_separator + 1;
	const size_t filetype_len = strlen(filetype_begin);
	if(!filetype_len) return NULL;
	
	char* filetype = ntrb_calloc(filetype_len + 1, sizeof(char));
	memcpy(filetype, filetype_begin, filetype_len);
	return filetype;
}