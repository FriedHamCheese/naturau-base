#include "test_file_wrapper.h"
#include "file_wrapper.h"

#include "SpanU8.h"

#include <assert.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include <time.h>

static void test_ntrb_get_filesize_bytes(FILE* const, FILE* const){
	FILE* test_file = fopen("test/15_bytes.txt", "rb");
	assert(test_file != NULL);
	
	assert(ntrb_get_filesize_bytes(test_file) == 15);
	fclose(test_file);
}

static void test_ntrb_read_entire_file_rb(FILE* const, FILE* const){
	const char* correct_data = "1234567890ABCDE";
	
	ntrb_SpanU8 file_buffer;
	enum ntrb_ReadFileResult read_result = ntrb_read_entire_file_rb(&file_buffer, "test/15_bytes.txt");
	assert(read_result == ntrb_ReadFileResult_OK);
	assert(memcmp(correct_data, file_buffer.ptr, file_buffer.elem) == 0);
	
	free(file_buffer.ptr);
	
	ntrb_SpanU8 file_buffer_2;
	const char* invalid_file = "opekfop4ekfoprk4fop4kf.jpg";
	enum ntrb_ReadFileResult read_result_2 = ntrb_read_entire_file_rb(&file_buffer_2, invalid_file);	
	assert(read_result_2 == ntrb_ReadFileResult_FileOpenError);
}


void test_suite_ntrb_file_wrapper(FILE* const outstream, FILE* const errstream){	
	test_ntrb_get_filesize_bytes(outstream, errstream);
	test_ntrb_read_entire_file_rb(outstream, errstream);
}