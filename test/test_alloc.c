#include "alloc.h"

#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

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
	
	const size_t ptr_count = 5;
	for(size_t i = 0; i < ptr_count; i++){
		assert(_ntrb_memdebug_malloc(i, __FILE__, __LINE__));
	}
		
	const size_t i_testing_ptr = 3;
	_ntrb_memdebug_free(((_ntrb_memdebug_AllocData*)(_ntrb_memdebug_alloc_data.base_ptr))[i_testing_ptr].ptr, __FILE__, __LINE__);
	
	const size_t alloc_size = 27;
	const int mock_line = 9;
	void* const ptr = malloc(alloc_size);
	assert(ptr);

	assert(_ntrb_memdebug_add_element_to_unused_space(ptr, alloc_size, __FILE__, mock_line));
	assert(_test__ntrb_memdebug_element_equal(i_testing_ptr, ptr, alloc_size, __FILE__, mock_line));
	
	_test__ntrb_memdebug_free_all();
	ntrb_memdebug_uninit(false);
}

void test__ntrb_memdebug_add_element(){
	assert(ntrb_memdebug_init());
	
	//test appending elements
	const size_t ptr_count = 7;
	for(size_t i = 0; i < ptr_count; i++){
		const size_t allocsize = 30;
		const int line = 100;
		void* const ptr = malloc(allocsize);

		assert(_ntrb_memdebug_add_element(ptr, allocsize, __FILE__, line));
		assert( _ntrb_memdebug_alloc_data.elements == sizeof(_ntrb_memdebug_AllocData)*(i+1) );
		assert(_test__ntrb_memdebug_element_equal(i, ptr, allocsize, __FILE__, line));
	}
	
	//test adding element to unused space
	const size_t i_testing_ptr = 4;
	assert(i_testing_ptr < ptr_count);
	
	_ntrb_memdebug_free(((_ntrb_memdebug_AllocData*)(_ntrb_memdebug_alloc_data.base_ptr))[i_testing_ptr].ptr, __FILE__, __LINE__);
	
	const size_t allocsize = 50;
	const int line = 125;
	void* const ptr = malloc(allocsize);	
	_ntrb_memdebug_add_element(ptr, allocsize, __FILE__, line);
	
	assert( _ntrb_memdebug_alloc_data.elements == sizeof(_ntrb_memdebug_AllocData)*ptr_count );
	assert(_test__ntrb_memdebug_element_equal(i_testing_ptr, ptr, allocsize, __FILE__, line));
	
	_test__ntrb_memdebug_free_all();
	ntrb_memdebug_uninit(false);	
}

void test__ntrb_memdebug_remove_element(){
	assert(ntrb_memdebug_init());
	
	const size_t ptr_count = 11;
	for(size_t i = 0; i < ptr_count; i++){
		assert(_ntrb_memdebug_malloc(27, __FILE__, __LINE__));
	}
	
	const size_t i_testing_ptr = 3;
	
	//test removing an element
	void* const ptr_1 = ((_ntrb_memdebug_AllocData*)(_ntrb_memdebug_alloc_data.base_ptr))[i_testing_ptr].ptr;
	free(ptr_1);
	_ntrb_memdebug_remove_element(i_testing_ptr, ptr_count);
	assert( _ntrb_memdebug_alloc_data.elements == sizeof(_ntrb_memdebug_AllocData)*ptr_count );
	assert(_test__ntrb_memdebug_element_equal(i_testing_ptr, NULL, 0, NULL, 0));
	
	//testing removing the last element
	const size_t last_i = ptr_count - 1;
	void* const ptr_2 = ((_ntrb_memdebug_AllocData*)(_ntrb_memdebug_alloc_data.base_ptr))[last_i].ptr;
	free(ptr_2);
	_ntrb_memdebug_remove_element(last_i, ptr_count);
	assert( _ntrb_memdebug_alloc_data.elements == sizeof(_ntrb_memdebug_AllocData) * (ptr_count-1));
	assert(_test__ntrb_memdebug_element_equal(last_i, NULL, 0, NULL, 0));

	_test__ntrb_memdebug_free_all();
	ntrb_memdebug_uninit(false);
}

void test__ntrb_memdebug_ptr_index(){
	assert(ntrb_memdebug_init());
	
	assert(_ntrb_memdebug_malloc(27, __FILE__, __LINE__));
	void* const ptr = _ntrb_memdebug_malloc(36, __FILE__, __LINE__);
	assert(ptr);
	assert(_ntrb_memdebug_malloc(31, __FILE__, __LINE__));
	assert(_ntrb_memdebug_malloc(82, __FILE__, __LINE__));
	assert(_ntrb_memdebug_malloc(79, __FILE__, __LINE__));
	
	assert(_ntrb_memdebug_ptr_index(ptr) == 1);

	_test__ntrb_memdebug_free_all();
	assert(_ntrb_memdebug_ptr_index(ptr) == -1);
	ntrb_memdebug_uninit(false);
}


void test__ntrb_memdebug_malloc(){
	assert(ntrb_memdebug_init());
	
	const size_t allocsize = 26;
	void* const ptr = _ntrb_memdebug_malloc(allocsize, __FILE__, __LINE__);
	assert(ptr);
	
	assert( _ntrb_memdebug_alloc_data.elements == sizeof(_ntrb_memdebug_AllocData) );
	assert( ((_ntrb_memdebug_AllocData*)(_ntrb_memdebug_alloc_data.base_ptr))[0].ptr == ptr );
	assert( ((_ntrb_memdebug_AllocData*)(_ntrb_memdebug_alloc_data.base_ptr))[0].allocsize_bytes == allocsize );
	
	_ntrb_memdebug_free(ptr, __FILE__, __LINE__);
	ntrb_memdebug_uninit(false);	
}

void test__ntrb_memdebug_calloc(){
	assert(ntrb_memdebug_init());
	
	const size_t elements = 5;
	const size_t typesize = sizeof(uint32_t);
	void* const ptr = _ntrb_memdebug_calloc(elements, typesize, __FILE__, __LINE__);
	assert(ptr);
	
	const size_t bytes = elements * typesize;
	for(size_t i = 0; i < bytes; i++){
		assert(((uint8_t*)ptr)[i] == 0);
	}
	
	assert( _ntrb_memdebug_alloc_data.elements == sizeof(_ntrb_memdebug_AllocData) );
	assert( ((_ntrb_memdebug_AllocData*)(_ntrb_memdebug_alloc_data.base_ptr))[0].ptr == ptr );
	assert( ((_ntrb_memdebug_AllocData*)(_ntrb_memdebug_alloc_data.base_ptr))[0].allocsize_bytes == bytes );
	
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