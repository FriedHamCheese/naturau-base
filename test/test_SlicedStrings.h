/*
Copyright 2024 Pawikan Boonnaum

This file is part of naturau-base/ntrb.

naturau-base/ntrb is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

naturau-base/ntrb is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with naturau-base/ntrb. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef test_ntrb_SlicedStrings_h
#define test_ntrb_SlicedStrings_h

#include "SlicedStrings.h"

#include <stddef.h>
#include <string.h>
#include <assert.h>

static void test_ntrb_SlicedStrings_new(){
	assert(ntrb_SlicedStrings_new(0).str_ptrs == NULL);
	
	const size_t str_count = 16;
	ntrb_SlicedStrings sliced_strs = ntrb_SlicedStrings_new(str_count);
	assert(sliced_strs.elem == str_count);
	assert(sliced_strs.str_ptrs != NULL);

	const size_t str_alloc_size = ntrb_slice_string_max_len+1;
	char* expected_str_alloc = ntrb_calloc(str_alloc_size, sizeof(char));
	
	for(size_t i = 0; i < sliced_strs.elem; i++){
		assert(memcmp(expected_str_alloc, sliced_strs.str_ptrs[i], str_alloc_size) == 0);
	}
	
	ntrb_SlicedStrings_free(&sliced_strs);
	ntrb_free(expected_str_alloc);
}

static void test_ntrb_SlicedStrings_free(){
	ntrb_SlicedStrings_free(NULL);
	
	ntrb_SlicedStrings typical_alloc = ntrb_SlicedStrings_new(16);
	assert(typical_alloc.str_ptrs != NULL);
	
	ntrb_SlicedStrings_free(&typical_alloc);
	assert(typical_alloc.str_ptrs == NULL);
	assert(typical_alloc.elem == 0);
	
	//the test should not crash.
	ntrb_SlicedStrings null_sliced_str = {.str_ptrs=NULL, .elem=0};
	ntrb_SlicedStrings_free(&null_sliced_str);
}

static void test_ntrb_SlicedStrings_slice_without_trimming(){
	const char* const input_str = "Freshly,toasted,cheeseburger,buns";
	const char* const expected_sliced_strs[] = {"Freshly", "toasted", "cheeseburger", "buns"};
	
	ntrb_SlicedStrings sliced_str = ntrb_SlicedStrings_slice_without_trimming(input_str, strlen(input_str), ',');
	assert(sliced_str.str_ptrs != NULL);
	assert(sliced_str.elem == 4);
	
	for(size_t i = 0; i < sliced_str.elem; i++){
		assert(strcmp(sliced_str.str_ptrs[i], expected_sliced_strs[i]) == 0);
	}
	
	ntrb_SlicedStrings_free(&sliced_str);
	
	const char* const input_str2 = " french fries  ";
	const char* const expected_sliced_strs2[] = {"", "french", "fries", "", ""};
	
	ntrb_SlicedStrings sliced_str2 = ntrb_SlicedStrings_slice_without_trimming(input_str2, strlen(input_str2), ' ');
	assert(sliced_str2.str_ptrs != NULL);
	assert(sliced_str2.elem == 5);
	
	for(size_t i = 0; i < sliced_str2.elem; i++){
		assert(strcmp(sliced_str2.str_ptrs[i], expected_sliced_strs2[i]) == 0);
	}	
	
	ntrb_SlicedStrings_free(&sliced_str2);
	
	ntrb_SlicedStrings zero_length = ntrb_SlicedStrings_slice_without_trimming(input_str2, 0, ' ');
	assert(zero_length.str_ptrs != NULL);
	assert(zero_length.elem == 1);
	assert(strcmp(zero_length.str_ptrs[0], "") == 0);
	
	ntrb_SlicedStrings_free(&zero_length);
}

static void test_ntrb_SlicedStrings_slice_sep(){
	const char* const input_str = ",,Freshly,,toasted,cheeseburger,,,buns,";
	const char* const expected_sliced_strs[] = {"", "Freshly", "toasted", "cheeseburger", "buns", ""};
	const size_t expected_slice_count = 6;
	
	ntrb_SlicedStrings sliced_str = ntrb_SlicedStrings_slice_sep(input_str, strlen(input_str), ',');
	assert(sliced_str.str_ptrs != NULL);
	assert(sliced_str.elem == expected_slice_count);
		
	for(size_t i = 0; i < sliced_str.elem; i++){
		assert(strcmp(sliced_str.str_ptrs[i], expected_sliced_strs[i]) == 0);
	}	
	
	ntrb_SlicedStrings_free(&sliced_str);
}

static void test_ntrb_SlicedStrings_concat_strs(){
	const char* const unsliced_str = "Homemade american cheese";
	ntrb_SlicedStrings sliced_strs = ntrb_SlicedStrings_slice_sep(unsliced_str, strlen(unsliced_str), ' ');
	assert(sliced_strs.str_ptrs != NULL);
		
	assert(ntrb_SlicedStrings_concat_strs(sliced_strs, 26, 5, ' ') == NULL);
	assert(ntrb_SlicedStrings_concat_strs(sliced_strs, 0, sliced_strs.elem +1, ' ') == NULL);
	assert(ntrb_SlicedStrings_concat_strs(sliced_strs, 26, sliced_strs.elem +1, ' ') == NULL);
	
	const char* const expected_slash_sep_str = "Homemade/american/cheese";
	char* const slash_sep_str = ntrb_SlicedStrings_concat_strs(sliced_strs, 0, sliced_strs.elem, '/');
	assert(slash_sep_str != NULL);
	assert(strcmp(slash_sep_str, expected_slash_sep_str) == 0);
	ntrb_free(slash_sep_str);
	
	const char* const expected_nosep_str = "americancheese";
	char* const nosep_str = ntrb_SlicedStrings_concat_strs(sliced_strs, 1, sliced_strs.elem, '\0');
	assert(nosep_str != NULL);
	assert(strcmp(nosep_str, expected_nosep_str) == 0);	
	ntrb_free(nosep_str);
	
	ntrb_SlicedStrings_free(&sliced_strs);
}


void test_suite_ntrb_SlicedStrings(){
	test_ntrb_SlicedStrings_new();
	test_ntrb_SlicedStrings_free();
	test_ntrb_SlicedStrings_slice_without_trimming();
	test_ntrb_SlicedStrings_slice_sep();
	test_ntrb_SlicedStrings_concat_strs();
}

#endif