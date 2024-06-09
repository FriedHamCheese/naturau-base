/**
Copyright 2024 Pawikan Boonnaum

This file is part of naturau-base/ntrb.

naturau-base/ntrb is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

naturau-base/ntrb is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with naturau-base/ntrb. If not, see <https://www.gnu.org/licenses/>.
*/

#include "alloc.h"

#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

#ifdef NTRB_MEMDEBUG
bool _test__ntrb_memdebug_element_equal(const size_t i, const void* const ptr, const size_t allocsize, const char* const filename, const int line){
	const _ntrb_memdebug_AllocData data = ((_ntrb_memdebug_AllocData*)(_ntrb_memdebug_alloc_data.base_ptr))[i];
	
	return data.ptr == ptr && data.allocsize_bytes == allocsize && data.callsite_filename == filename && data.callsite_line == line;
}

void _test__ntrb_memdebug_free_all(){
	const size_t ptr_count = _ntrb_memdebug_alloc_data.elements / sizeof(_ntrb_memdebug_AllocData);
	
	for(size_t i = 0; i < ptr_count; i++){
		_ntrb_memdebug_free(((_ntrb_memdebug_AllocData*)(_ntrb_memdebug_alloc_data.base_ptr))[i].ptr, __FILE__, __LINE__);
	}	
}


void test__ntrb_memdebug_add_element_to_unused_space(){
	assert(ntrb_memdebug_init());
	
	int some_int_to_have_its_pointer_used = 35;
	
	const size_t ptr_count = 5;
	assert(_ntrb_alloc_bytevec_reserve(&_ntrb_memdebug_alloc_data, ptr_count * sizeof(_ntrb_memdebug_AllocData)));
	for(size_t i = 0; i < ptr_count; i++){
		//The record contain pointers pointing to stack. Don't free it.
		const _ntrb_memdebug_AllocData alloc_data = {&some_int_to_have_its_pointer_used, 0, 0, 0};
		_ntrb_alloc_bytevec_append(&_ntrb_memdebug_alloc_data, sizeof(_ntrb_memdebug_AllocData), &alloc_data);
	}

	const size_t i_testing_ptr = 3;
	((_ntrb_memdebug_AllocData*)(_ntrb_memdebug_alloc_data.base_ptr))[i_testing_ptr].ptr = NULL;
	
	const size_t alloc_size = 27;
	const int mock_line = 9;
	void* const ptr = &some_int_to_have_its_pointer_used;

	assert(_ntrb_memdebug_add_element_to_unused_space(ptr, alloc_size, __FILE__, mock_line));
	assert(_test__ntrb_memdebug_element_equal(i_testing_ptr, ptr, alloc_size, __FILE__, mock_line));
	
	//The pointers in the record contain stack addresses which we shouldn't free.	
	ntrb_memdebug_uninit(false);
}

void test__ntrb_memdebug_add_element(){
	assert(ntrb_memdebug_init());
	
	int some_int_to_have_its_pointer_used = 35;
	
	//test appending elements
	const size_t ptr_count = 7;
	for(size_t i = 0; i < ptr_count; i++){
		const size_t allocsize = 30;
		const int line = 100;
		void* const ptr = &some_int_to_have_its_pointer_used;

		assert(_ntrb_memdebug_add_element(ptr, allocsize, __FILE__, line));
		assert( _ntrb_memdebug_alloc_data.elements == sizeof(_ntrb_memdebug_AllocData)*(i+1) );
		assert(_test__ntrb_memdebug_element_equal(i, ptr, allocsize, __FILE__, line));
	}
	
	//test adding element to unused space
	const size_t i_testing_ptr = 4;
	assert(i_testing_ptr < ptr_count);
	
	((_ntrb_memdebug_AllocData*)(_ntrb_memdebug_alloc_data.base_ptr))[i_testing_ptr].ptr = NULL;
	
	const size_t allocsize = 50;
	const int line = 125;
	void* const ptr = &some_int_to_have_its_pointer_used;	
	_ntrb_memdebug_add_element(ptr, allocsize, __FILE__, line);
	
	assert( _ntrb_memdebug_alloc_data.elements == sizeof(_ntrb_memdebug_AllocData)*ptr_count );
	assert(_test__ntrb_memdebug_element_equal(i_testing_ptr, ptr, allocsize, __FILE__, line));
	
	//The pointers in the record contain stack addresses which we shouldn't free.
	ntrb_memdebug_uninit(false);	
}

void test__ntrb_memdebug_remove_element(){
	assert(ntrb_memdebug_init());
	
	int some_int_to_have_its_pointer_used = 35;	
	
	const size_t ptr_count = 11;
	for(size_t i = 0; i < ptr_count; i++){
		assert(_ntrb_memdebug_add_element(&some_int_to_have_its_pointer_used, 1, __FILE__, __LINE__));
	}

	//test removing an element	
	const size_t i_testing_ptr = 3;
	_ntrb_memdebug_remove_element(i_testing_ptr, ptr_count);
	assert( _ntrb_memdebug_alloc_data.elements == sizeof(_ntrb_memdebug_AllocData)*ptr_count );
	assert(_test__ntrb_memdebug_element_equal(i_testing_ptr, NULL, 0, NULL, 0));
	
	//testing removing the last element
	const size_t last_i = ptr_count - 1;
	_ntrb_memdebug_remove_element(last_i, ptr_count);
	assert( _ntrb_memdebug_alloc_data.elements == sizeof(_ntrb_memdebug_AllocData) * (ptr_count-1));
	assert(_test__ntrb_memdebug_element_equal(last_i, NULL, 0, NULL, 0));

	//The pointers in the record contain stack addresses which we shouldn't free.
	ntrb_memdebug_uninit(false);
}

void test__ntrb_memdebug_ptr_index(){
	assert(ntrb_memdebug_init());

	int some_int_to_have_its_pointer_used = 35;	
	int other_int_to_have_its_pointer_used = 89;
	
	assert(_ntrb_memdebug_add_element(&other_int_to_have_its_pointer_used, 36, __FILE__, __LINE__));
	assert(_ntrb_memdebug_add_element(&other_int_to_have_its_pointer_used, 36, __FILE__, __LINE__));
	assert(_ntrb_memdebug_add_element(&some_int_to_have_its_pointer_used, 36, __FILE__, __LINE__));	
	assert(_ntrb_memdebug_add_element(&other_int_to_have_its_pointer_used, 36, __FILE__, __LINE__));
	assert(_ntrb_memdebug_add_element(&other_int_to_have_its_pointer_used, 36, __FILE__, __LINE__));

	assert(_ntrb_memdebug_ptr_index(&some_int_to_have_its_pointer_used) == 2);
	
	const size_t ptr_count = _ntrb_memdebug_alloc_data.elements / sizeof(_ntrb_memdebug_AllocData);
	for(size_t i = 0; i < ptr_count; i++){
		((_ntrb_memdebug_AllocData*)(_ntrb_memdebug_alloc_data.base_ptr))[i].ptr = NULL;
	}

	assert(_ntrb_memdebug_ptr_index(&some_int_to_have_its_pointer_used) == -1);
	
	//The pointers in the record contain stack addresses which we shouldn't free.	
	ntrb_memdebug_uninit(false);
}


void test__ntrb_memdebug_malloc(){
	assert(ntrb_memdebug_init());
	
	const size_t allocsize = 26;
	const int mock_line = 9;
	void* const ptr = _ntrb_memdebug_malloc(allocsize, __FILE__, mock_line);
	assert(ptr);
	
	assert(_ntrb_memdebug_alloc_data.elements == sizeof(_ntrb_memdebug_AllocData));
	assert(_test__ntrb_memdebug_element_equal(0, ptr, allocsize, __FILE__, mock_line));
	
	_ntrb_memdebug_free(ptr, __FILE__, __LINE__);
	ntrb_memdebug_uninit(false);
}

void test__ntrb_memdebug_calloc(){
	assert(ntrb_memdebug_init());
	
	const size_t elements = 5;
	const int mock_line = 9;	
	const size_t typesize = sizeof(uint32_t);
	void* const ptr = _ntrb_memdebug_calloc(elements, typesize, __FILE__, mock_line);
	assert(ptr);
	
	const size_t bytes = elements * typesize;
	for(size_t i = 0; i < bytes; i++){
		assert(((uint8_t*)ptr)[i] == 0);
	}
	
	assert(_ntrb_memdebug_alloc_data.elements == sizeof(_ntrb_memdebug_AllocData));
	assert(_test__ntrb_memdebug_element_equal(0, ptr, bytes, __FILE__, mock_line));
	
	_ntrb_memdebug_free(ptr, __FILE__, __LINE__);
	ntrb_memdebug_uninit(false);
}

void test__ntrb_memdebug_empty_realloc_callback(const void*, const void*){
}

static int test__ntrb_memdebug_unregistered_ptr_reallocs = 0;
void test__ntrb_memdebug_realloc_count_unregistered_call(const void*, const void*){
	test__ntrb_memdebug_unregistered_ptr_reallocs++;
}

void test__ntrb_memdebug_realloc(){
	assert(ntrb_memdebug_init());
	
	//Element 0: test expanding and shrinking a registered pointer
	const int mock_line = 59;
	void* ptr_0 = _ntrb_memdebug_malloc(59, __FILE__, mock_line);
	assert(ptr_0);
	
	ptr_0 = _ntrb_memdebug_realloc(ptr_0, 72, __FILE__, mock_line, test__ntrb_memdebug_realloc_count_unregistered_call);
	assert(ptr_0);	
	assert(_test__ntrb_memdebug_element_equal(0, ptr_0, 72, __FILE__, mock_line));
	
	ptr_0 = _ntrb_memdebug_realloc(ptr_0, 29, __FILE__, mock_line, test__ntrb_memdebug_realloc_count_unregistered_call);
	assert(_test__ntrb_memdebug_element_equal(0, ptr_0, 29, __FILE__, mock_line));
	
	assert(test__ntrb_memdebug_unregistered_ptr_reallocs == 0);
	
	//Element 1: test reallocting and shrinking a null pointer, this should behave as malloc
	//you can't shrink a null pointer, as that would still act as reallocation
	void* ptr_1 = _ntrb_memdebug_realloc(NULL, 48, __FILE__, mock_line, test__ntrb_memdebug_realloc_count_unregistered_call);
	assert(ptr_1);
	assert(_test__ntrb_memdebug_element_equal(1, ptr_1, 48, __FILE__, mock_line));
	
	ptr_1 = _ntrb_memdebug_realloc(ptr_1, 16, __FILE__, mock_line, test__ntrb_memdebug_realloc_count_unregistered_call);
	assert(_test__ntrb_memdebug_element_equal(1, ptr_1, 16, __FILE__, mock_line));	
	
	assert(test__ntrb_memdebug_unregistered_ptr_reallocs == 0);
	
	//Element 2: test shirnking and then expanding a registed pointer
	void* ptr_2 = _ntrb_memdebug_malloc(59, __FILE__, mock_line);
	assert(ptr_2);
	ptr_2 = _ntrb_memdebug_realloc(ptr_2, 26, __FILE__, mock_line, test__ntrb_memdebug_realloc_count_unregistered_call);
	assert(_test__ntrb_memdebug_element_equal(2, ptr_2, 26, __FILE__, mock_line));

	ptr_2 = _ntrb_memdebug_realloc(ptr_2, 105, __FILE__, mock_line, test__ntrb_memdebug_realloc_count_unregistered_call);
	assert(ptr_2);
	assert(_test__ntrb_memdebug_element_equal(2, ptr_2, 105, __FILE__, mock_line));	
	
	assert(test__ntrb_memdebug_unregistered_ptr_reallocs == 0);
	
	//Element 3: test shrink and expanding an unregistered pointer
	void* ptr_3 = malloc(79);
	assert(ptr_3);

	ptr_3 = _ntrb_memdebug_realloc(ptr_3, 35, __FILE__, mock_line, test__ntrb_memdebug_realloc_count_unregistered_call);
	assert(ptr_3);
	
	ptr_3 = _ntrb_memdebug_realloc(ptr_3, 126, __FILE__, mock_line, test__ntrb_memdebug_realloc_count_unregistered_call);
	assert(ptr_3);
	free(ptr_3);
	
	assert(test__ntrb_memdebug_unregistered_ptr_reallocs == 2);

	const size_t ptr_count = _ntrb_memdebug_alloc_data.elements / sizeof(_ntrb_memdebug_AllocData);
	assert(ptr_count == 3);	//element 3 is unregisted, it should not be added to the record.
	
	_test__ntrb_memdebug_free_all();
	ntrb_memdebug_uninit(false);	
}

void* _test__ntrb_memdebug_malloc_thread(void* const int_ptr){
	void* ptr = _ntrb_memdebug_malloc(*((int*)int_ptr), __FILE__, *((int*)int_ptr));
	assert(ptr);
	return ptr;
}

#define _test__ntrb_memdebug_thread_count 10000
void* ptrs[_test__ntrb_memdebug_thread_count];

static size_t size_increment = 3;
void* _test__ntrb_memdebug_realloc_thread(void* const int_ptr){
	const int i = *((int*)int_ptr);
	void* ptr = _ntrb_memdebug_realloc(ptrs[i], i + size_increment, __FILE__, i + size_increment, test__ntrb_memdebug_empty_realloc_callback);
	assert(ptr);
	return ptr;
}

void* _test__ntrb_memdebug_free_thread(void* int_ptr){
	const int i = *((int*)int_ptr);
	_ntrb_memdebug_free(ptrs[i], __FILE__, __LINE__);
	return NULL;	
}

void test__ntrb_memdebug_test_lock(){
	assert(ntrb_memdebug_init());
	
	pthread_t threads[_test__ntrb_memdebug_thread_count];
	
	//testing and checking _ntrb_memdebug_malloc
	printf("[Info]: Testing data race for _ntrb_memdebug_malloc() in %s:%d with %d threads.\n", __FILE__, __LINE__, _test__ntrb_memdebug_thread_count);
	for(int i = 0; i < _test__ntrb_memdebug_thread_count; i++){
		assert(pthread_create(threads + i, NULL, _test__ntrb_memdebug_malloc_thread, &i) == 0);
		assert(pthread_join(threads[i], ptrs + i) == 0);
	}
	
	for(int i = 0; i < _test__ntrb_memdebug_thread_count; i++){
		assert(_test__ntrb_memdebug_element_equal(i, ptrs[i], i, __FILE__, i));
	}
	
	//testing and checking _ntrb_memdebug_realloc
	printf("[Info]: Testing data race for _ntrb_memdebug_realloc() in %s:%d with %d threads.\n", __FILE__, __LINE__, _test__ntrb_memdebug_thread_count);	
	for(int i = 0; i < _test__ntrb_memdebug_thread_count; i++){
		assert(pthread_create(threads + i, NULL, _test__ntrb_memdebug_realloc_thread, &i) == 0);
		assert(pthread_join(threads[i], ptrs + i) == 0);
	}	
	
	for(int i = 0; i < _test__ntrb_memdebug_thread_count; i++){
		assert(_test__ntrb_memdebug_element_equal(i, ptrs[i], i + size_increment, __FILE__, i + size_increment));
	}
	
	//testing and checking _ntrb_memdebug_free
	printf("[Info]: Testing data race for _ntrb_memdebug_free() in %s:%d with %d threads.\n", __FILE__, __LINE__, _test__ntrb_memdebug_thread_count);	
	
	for(int i = 0; i < _test__ntrb_memdebug_thread_count; i++){
		assert(pthread_create(threads + i, NULL, _test__ntrb_memdebug_free_thread, &i) == 0);
		assert(pthread_join(threads[i], NULL) == 0);
	}	
	
	for(int i = 0; i < _test__ntrb_memdebug_thread_count; i++){
		assert(_test__ntrb_memdebug_element_equal(i, NULL,0, NULL, 0));
	}
	
	assert(ntrb_memdebug_uninit(false) == 0);
}
#endif