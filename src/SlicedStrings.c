#include "SlicedStrings.h"
#include "alloc.h"

#include "str_utils.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

const size_t ntrb_slice_string_max_len = 256;
const ntrb_SlicedStrings failed_ntrb_SlicedStrings = {
	.str_ptrs = NULL,
	.elem =  0
};

ntrb_SlicedStrings ntrb_SlicedStrings_new(const size_t str_count){
	ntrb_SlicedStrings slices;
	
	slices.elem = str_count;	
	if(str_count == 0) return failed_ntrb_SlicedStrings;
	
	slices.str_ptrs = ntrb_calloc(str_count, sizeof(char*));	//implicit NULL initialised char*.
	if(slices.str_ptrs == NULL) return failed_ntrb_SlicedStrings;	
	
	for(size_t i = 0; i < slices.elem; i++){
		char* const ptr = ntrb_calloc(ntrb_slice_string_max_len + 1, sizeof(char));	//implicit all \0 initialised char[]
		if(ptr == NULL){
			ntrb_SlicedStrings_free(&slices);
			return failed_ntrb_SlicedStrings;
		}else slices.str_ptrs[i] = ptr;
	}
	
	return slices;
}

void ntrb_SlicedStrings_free(ntrb_SlicedStrings* const obj){
	if(obj == NULL) return;
	if(obj->str_ptrs == NULL) return;
	
	for(size_t i = 0; i < obj->elem; i++){
		ntrb_free(obj->str_ptrs[i]);
	}
	ntrb_free(obj->str_ptrs);
	obj->str_ptrs = NULL;
	obj->elem = 0;
}


ntrb_SlicedStrings ntrb_SlicedStrings_slice_without_trimming(const char* const str, const size_t str_len, const char separator){
	size_t separator_count = 0;
	
	for(size_t i = 0; i < str_len; i++){
		if(str[i] == separator) separator_count++;
	}
	
	const size_t string_count = separator_count + 1;
	ntrb_SlicedStrings slices = ntrb_SlicedStrings_new(string_count);
	
	size_t str_ptr_pos = 0;
	size_t str_pos = 0;
	for(size_t i = 0; i < str_len; i++){		
		if(str[i] != separator && str_pos < ntrb_slice_string_max_len){
			slices.str_ptrs[str_ptr_pos][str_pos] = str[i];
			str_pos++;
		}else{
			str_ptr_pos++;
			str_pos = 0;
		}
	}
	
	return slices;
}

ntrb_SlicedStrings ntrb_SlicedStrings_slice_sep(const char* const str, const size_t str_len, const char separator){
	const bool no_separator = separator == '\0';
	if(no_separator) return failed_ntrb_SlicedStrings;
	
	char* const no_duplicate_sep_str = ntrb_trim_duplicate_separators(str, str_len, separator);
	if(no_duplicate_sep_str == NULL) return failed_ntrb_SlicedStrings;
	
	ntrb_SlicedStrings slices = ntrb_SlicedStrings_slice_without_trimming(no_duplicate_sep_str, strlen(no_duplicate_sep_str), separator);
	ntrb_free(no_duplicate_sep_str);
	if(slices.str_ptrs == NULL) return failed_ntrb_SlicedStrings;
	
	for(size_t i = 0; i < slices.elem; i++){
		const size_t slice_str_len = strlen(slices.str_ptrs[i]);
		if(slice_str_len < ntrb_slice_string_max_len){
			slices.str_ptrs[i] = ntrb_realloc(slices.str_ptrs[i], slice_str_len + 1);
			slices.str_ptrs[i][slice_str_len] = '\0';
		}
	}
	
	return slices;
}

ntrb_SlicedStrings ntrb_SlicedStrings_slice(const char* const str, const size_t str_len){
	return ntrb_SlicedStrings_slice_sep(str, str_len, ' ');
}

char* ntrb_SlicedStrings_concat_strs(const ntrb_SlicedStrings slices, const size_t beg, const size_t end, const char separator){
	if(beg > end) return NULL;
	if(beg > slices.elem || end > slices.elem) return NULL;
	
	size_t separator_size = sizeof(char);
	const bool no_separator = separator == '\0';
	if(no_separator) separator_size = 0;
	
	size_t total_strlen = 0;
	for(size_t i = beg; i < end; i++){
		total_strlen += strlen(slices.str_ptrs[i]) + separator_size;
	}
	//if a separator is used, the strlen would include a redundant byte for a trailing separator, so we remove it.
	if(!no_separator) total_strlen--;
	
	const size_t null_term_size = sizeof(char);
	char* str = ntrb_calloc(total_strlen + null_term_size, sizeof(char));
	if(str == NULL) return NULL;
		
	if(no_separator){
		size_t i_str = 0;
		
		for(size_t i_slices = beg; i_slices < end; i_slices++){
			const size_t slice_len = strlen(slices.str_ptrs[i_slices]);
			memcpy(str + i_str, slices.str_ptrs[i_slices], slice_len);
			i_str += slice_len;
		}
	}else{
		size_t i_str = 0;
	
		for(size_t i_slices = beg; i_slices < end; i_slices++){
			const size_t slice_len = strlen(slices.str_ptrs[i_slices]);
			memcpy(str + i_str, slices.str_ptrs[i_slices], slice_len);
			i_str += slice_len;
			
			const bool not_last_slice = i_slices + 1 < end;
			if(not_last_slice){
				str[i_str] = separator;
				i_str++;
			}
			//else don't add the separator.
		}
	}
	
	return str;
}