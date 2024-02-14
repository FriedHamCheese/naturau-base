#include "test_str_utils.h"
#include "../src/str_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <assert.h>

static void test_ntrb_getc(FILE*, FILE*){
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

static void test_ntrb_getsn(FILE*, FILE*){
	FILE* const test_file = fopen("test/getsn_test.txt", "r");
	assert(test_file != NULL);
	
	const char* const line_1 = "hi this is, ";
	const char* const line_2 = "a newline";
	const char* const line_3 = "wahoo!";
	
	char* read_str = 0;
	assert(ntrb_getsn(128, test_file, &read_str) == ntrb_GetChar_OK);
	assert(strcmp(read_str, line_1) == 0);
	free(read_str);
	
	assert(ntrb_getsn(128, test_file, &read_str) == ntrb_GetChar_OK);
	assert(strcmp(read_str, line_2) == 0);
	free(read_str);

	assert(ntrb_getsn(128, test_file, &read_str) == ntrb_GetChar_OK);
	assert(strcmp(read_str, line_3) == 0);
	free(read_str);

	//EOF
	assert(ntrb_getsn(128, test_file, &read_str) == ntrb_GetChar_EOF);
	assert(read_str == NULL);	

	fclose(test_file);
}

static void test_ntrb_trim_duplicate_separators(FILE*, FILE*){
	const char* const input_str = "Freshly//baked/cookies///at/the/bakery.";
	const char* const expected_str = "Freshly/baked/cookies/at/the/bakery.";
	
	char* const returned_str = ntrb_trim_duplicate_separators(input_str, strlen(input_str), '/');
	assert(returned_str != NULL);
	assert(strcmp(returned_str, expected_str) == 0);
	free(returned_str);
	
	const char* const no_duplicate_str = "Coconut water";
	const size_t nodup_strlen = strlen(no_duplicate_str);
	char* const returned_str2 = ntrb_trim_duplicate_separators(no_duplicate_str, nodup_strlen, ' ');
	assert(returned_str2 != NULL);
	assert(strlen(returned_str2) == nodup_strlen);
	assert(strcmp(returned_str2, no_duplicate_str) == 0);
}

void test_suite_ntrb_str_utils(FILE* const outstream, FILE* const errstream){
	test_ntrb_getc(outstream, errstream);
	test_ntrb_getsn(outstream, errstream);
	test_ntrb_trim_duplicate_separators(outstream, errstream);
}