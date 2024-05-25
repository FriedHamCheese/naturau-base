#ifndef test_ntrb_str_utils_h
#define test_ntrb_str_utils_h

#include "str_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>

static void test_ntrb_getc(){
	//this covers the ntrb_GetChar_OK flag, and the ntrb_GetChar_EOF flag.
	
	FILE* const test_file = fopen("test/15_bytes.txt", "r");
	assert(test_file != NULL);
	
	char first_character = 0;
	assert(ntrb_getc(test_file, &first_character) == ntrb_GetChar_OK);
	assert(first_character == '1');
	
	const size_t remaining_filesize = 15 - 1;
	for(size_t i = 0; i < remaining_filesize; i++){
		const enum ntrb_GetCharStatus getchar_status = ntrb_getc(test_file, &first_character);
		assert(getchar_status == ntrb_GetChar_OK);
	}
	
	//read all 15 characters, the 16th should trip EOF
	assert(ntrb_getc(test_file, &first_character) == ntrb_GetChar_EOF);
	fclose(test_file);
}

static void test_ntrb_getsn(){
	FILE* const test_file = fopen("test/getsn_test.txt", "r");
	assert(test_file != NULL);
	
	const char* const line_1 = "hi this is, ";
	const char* const line_2 = "a newline";
	const char* const line_3 = "wahoo!";
	
	char* read_str = 0;
	assert(ntrb_getsn(128, test_file, &read_str) == ntrb_GetChar_OK);
	assert(strcmp(read_str, line_1) == 0);
	ntrb_free(read_str);
	
	assert(ntrb_getsn(128, test_file, &read_str) == ntrb_GetChar_OK);
	assert(strcmp(read_str, line_2) == 0);
	ntrb_free(read_str);

	assert(ntrb_getsn(128, test_file, &read_str) == ntrb_GetChar_OK);
	assert(strcmp(read_str, line_3) == 0);
	ntrb_free(read_str);

	//EOF
	assert(ntrb_getsn(128, test_file, &read_str) == ntrb_GetChar_EOF);
	assert(read_str == NULL);	

	fclose(test_file);
}

static void test_ntrb_trim_duplicate_separators(){
	const char* const input_str = "Freshly//baked/cookies///at/the/bakery.";
	const char* const expected_str = "Freshly/baked/cookies/at/the/bakery.";
	
	char* const returned_str = ntrb_trim_duplicate_separators(input_str, strlen(input_str), '/');
	assert(returned_str != NULL);
	assert(strcmp(returned_str, expected_str) == 0);
	ntrb_free(returned_str);
	
	const char* const no_duplicate_str = "Coconut water";
	const size_t nodup_strlen = strlen(no_duplicate_str);
	char* const returned_str2 = ntrb_trim_duplicate_separators(no_duplicate_str, nodup_strlen, ' ');
	assert(returned_str2 != NULL);
	assert(strlen(returned_str2) == nodup_strlen);
	assert(strcmp(returned_str2, no_duplicate_str) == 0);
	ntrb_free(returned_str2);
}

static void test_ntrb_trim_whitespace(){
	char* const trimmed_front_and_back_str = ntrb_trim_whitespace("  hello world! ");
	assert(trimmed_front_and_back_str != NULL);
	assert(strcmp(trimmed_front_and_back_str, "hello world!") == 0);
	ntrb_free(trimmed_front_and_back_str);
	
	char* const trimmed_front_only_str = ntrb_trim_whitespace(" hi!");
	assert(trimmed_front_only_str != NULL);
	assert(strcmp(trimmed_front_only_str, "hi!") == 0);
	ntrb_free(trimmed_front_only_str);
	
	char* const trimmed_back_only_str = ntrb_trim_whitespace("hello2!  ");
	assert(trimmed_back_only_str != NULL);
	assert(strcmp(trimmed_back_only_str, "hello2!") == 0);
	ntrb_free(trimmed_back_only_str);
	
	char* const all_whitespace_trimmed_str = ntrb_trim_whitespace("    ");
	assert(all_whitespace_trimmed_str != NULL);
	assert(strcmp(all_whitespace_trimmed_str, "") == 0);
	ntrb_free(all_whitespace_trimmed_str);
	
	char* const no_whitespace_str = ntrb_trim_whitespace("nope");
	assert(no_whitespace_str != NULL);
	assert(strcmp(no_whitespace_str, "nope") == 0);
	ntrb_free(no_whitespace_str);	
}

static void test_ntrb_get_filetype(){
	char* const filetype_1 = ntrb_get_filetype("I.love.cookies!");
	assert(filetype_1 != NULL);
	assert(strcmp(filetype_1, "cookies!") == 0);
	ntrb_free(filetype_1);
	
	char* const filetype_2 = ntrb_get_filetype("Nofiletype.");
	assert(filetype_2 == NULL);
	
	char* const filetype_3 = ntrb_get_filetype(".makefile");
	assert(filetype_3 != NULL);	
	assert(strcmp(filetype_3, "makefile") == 0);
	ntrb_free(filetype_3);	
}

static void test_suite_ntrb_str_utils(){
	test_ntrb_getc();
	test_ntrb_getsn();
	test_ntrb_trim_duplicate_separators();
	test_ntrb_trim_whitespace();
	test_ntrb_get_filetype();
}

#endif