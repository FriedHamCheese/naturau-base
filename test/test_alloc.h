#include "alloc.h"

#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

#ifdef NTRB_MEMDEBUG
bool _test__ntrb_memdebug_element_equal(const size_t i, const void* const ptr, const size_t allocsize, const char* const filename, const int line);

void _test__ntrb_memdebug_free_all();

void test__ntrb_memdebug_add_element_to_unused_space();
void test__ntrb_memdebug_add_element();
void test__ntrb_memdebug_remove_element();
void test__ntrb_memdebug_ptr_index();
void test__ntrb_memdebug_malloc();
void test__ntrb_memdebug_calloc();

void _test__ntrb_memdebug_empty_realloc_callback(const void*, const void*);

extern int _test__ntrb_memdebug_unregistered_ptr_reallocs;
void _test__ntrb_memdebug_realloc_count_unregistered_call(const void*, const void*);

void test__ntrb_memdebug_realloc();

void* _test__ntrb_memdebug_malloc_thread(void* const int_ptr);
void* _test__ntrb_memdebug_realloc_thread(void* const int_ptr);
void* _test__ntrb_memdebug_free_thread(void* int_ptr);
void test__ntrb_memdebug_test_lock();

#endif

static void test_suite_ntrb_alloc(){
	#ifdef NTRB_MEMDEBUG
	test__ntrb_memdebug_add_element_to_unused_space();
	test__ntrb_memdebug_add_element();
	test__ntrb_memdebug_remove_element();
	test__ntrb_memdebug_ptr_index();
	
	test__ntrb_memdebug_malloc();
	test__ntrb_memdebug_calloc();
	test__ntrb_memdebug_realloc();
	
	#ifdef NTRB_MEMDEBUG_TEST_DATA_RACE
	test__ntrb_memdebug_test_lock();
	#endif
	#endif
}