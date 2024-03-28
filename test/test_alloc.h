#include "alloc.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

#ifdef NTRB_MEMDEBUG
static bool _test__ntrb_memdebug_ptr_equal(const size_t i, const void* const ptr){
	return ((void**)(_ntrb_memdebug_ptr.base_ptr))[i] == ptr;
}
static bool _test__ntrb_memdebug_size_equal(const size_t i, const size_t alloc_size){
	return ((size_t*)(_ntrb_memdebug_size.base_ptr))[i] == alloc_size;
}
static bool _test__ntrb_memdebug_file_equal(const size_t i, const char* const filename){
	const char* const element_filename = ((const char**)(_ntrb_memdebug_filename.base_ptr))[i];
	if(filename == element_filename) return true;
	if(filename != NULL && element_filename != NULL) return strcmp(element_filename, filename) == 0;
	else return false;
}
static bool _test__ntrb_memdebug_line_equal(const size_t i, const int line){
	return ((int*)(_ntrb_memdebug_line.base_ptr))[i] == line;
}


static bool _test__ntrb_memdebug_element_equal(const size_t i, const void* const ptr, const size_t allocsize, const char* const filename, const int line){
	const bool ptr_equal = _test__ntrb_memdebug_ptr_equal(i, ptr);
	const bool allocsize_equal = _test__ntrb_memdebug_size_equal(i, allocsize);
	const bool file_equal = _test__ntrb_memdebug_file_equal(i, filename);
	const bool line_equal = _test__ntrb_memdebug_line_equal(i, line);
	
	return ptr_equal && allocsize_equal && file_equal && line_equal;
}


static void test__ntrb_memdebug_add_element_to_unused_space(){
	assert(ntrb_memdebug_init());
	
	const size_t ptr_count = 5;
	for(size_t i = 0; i < ptr_count; i++){
		assert(_ntrb_memdebug_malloc(i, __FILE__, __LINE__));
	}
		
	const size_t i_testing_ptr = 3;
	assert(i_testing_ptr < ptr_count);
	_ntrb_memdebug_free(((void**)(_ntrb_memdebug_ptr.base_ptr))[i_testing_ptr], __FILE__, __LINE__);
	
	const size_t alloc_size = 27;
	const int mock_line = 9;
	void* const ptr = malloc(alloc_size);
	assert(ptr);

	assert(_ntrb_memdebug_add_element_to_unused_space(ptr, alloc_size, __FILE__, mock_line));
	assert(_test__ntrb_memdebug_element_equal(i_testing_ptr, ptr, alloc_size, __FILE__, mock_line));
	
	for(size_t i = 0; i < ptr_count; i++){
		_ntrb_memdebug_free(((void**)(_ntrb_memdebug_ptr.base_ptr))[i], __FILE__, __LINE__);
	}

	ntrb_memdebug_uninit(false);
}

static void test__ntrb_memdebug_add_element(){
	assert(ntrb_memdebug_init());
	
	//test appending elements
	const size_t ptr_count = 7;
	for(size_t i = 0; i < ptr_count; i++){
		const size_t allocsize = 30;
		const int line = 100;
		void* const ptr = malloc(allocsize);
		_ntrb_memdebug_add_element(ptr, allocsize, __FILE__, line);
		
		assert( _ntrb_memdebug_ptr.elements == sizeof(void*)*(i+1) );
		assert(_test__ntrb_memdebug_element_equal(i, ptr, allocsize, __FILE__, line));
	}
	
	//test adding element to unused space
	const size_t i_testing_ptr = 4;
	assert(i_testing_ptr < ptr_count);
	
	_ntrb_memdebug_free(((void**)(_ntrb_memdebug_ptr.base_ptr))[i_testing_ptr], __FILE__, __LINE__);
	
	const size_t allocsize = 50;
	const int line = 125;
	void* const ptr = malloc(allocsize);	
	_ntrb_memdebug_add_element(ptr, allocsize, __FILE__, line);
	
	assert( _ntrb_memdebug_ptr.elements == sizeof(void*)*ptr_count );
	assert(_test__ntrb_memdebug_element_equal(i_testing_ptr, ptr, allocsize, __FILE__, line));
	
	for(size_t i = 0; i < ptr_count; i++){
		_ntrb_memdebug_free(((void**)(_ntrb_memdebug_ptr.base_ptr))[i], __FILE__, __LINE__);
	}

	ntrb_memdebug_uninit(false);	
}

static void test__ntrb_memdebug_remove_element(){
	assert(ntrb_memdebug_init());
	
	const size_t ptr_count = 11;
	for(size_t i = 0; i < ptr_count; i++){
		assert(_ntrb_memdebug_malloc(27, __FILE__, __LINE__));
	}
	
	const size_t i_testing_ptr = 3;
	assert(i_testing_ptr < ptr_count);
	
	//test removing an element
	void* const ptr_1 = ((void**)(_ntrb_memdebug_ptr.base_ptr))[i_testing_ptr];
	free(ptr_1);
	_ntrb_memdebug_remove_element(i_testing_ptr, ptr_count);
	assert( _ntrb_memdebug_ptr.elements == sizeof(void*)*ptr_count );
	assert(_test__ntrb_memdebug_element_equal(i_testing_ptr, NULL, 0, NULL, 0));
	
	//testing removing the last element
	const size_t last_i = ptr_count - 1;
	void* const ptr_2 = ((void**)(_ntrb_memdebug_ptr.base_ptr))[last_i];
	free(ptr_2);
	_ntrb_memdebug_remove_element(last_i, ptr_count);
	assert( _ntrb_memdebug_ptr.elements == sizeof(void*) * (ptr_count-1));
	assert(_test__ntrb_memdebug_element_equal(last_i, NULL, 0, NULL, 0));

	for(size_t i = 0; i < ptr_count; i++){
		_ntrb_memdebug_free(((void**)(_ntrb_memdebug_ptr.base_ptr))[i], __FILE__, __LINE__);
	}
	
	ntrb_memdebug_uninit(false);
}

static void test__ntrb_memdebug_ptr_index(){
	assert(ntrb_memdebug_init());
	
	assert(_ntrb_memdebug_malloc(27, __FILE__, __LINE__));
	void* const ptr = _ntrb_memdebug_malloc(36, __FILE__, __LINE__);
	assert(ptr);
	assert(_ntrb_memdebug_malloc(31, __FILE__, __LINE__));
	assert(_ntrb_memdebug_malloc(82, __FILE__, __LINE__));
	assert(_ntrb_memdebug_malloc(79, __FILE__, __LINE__));
	
	assert(_ntrb_memdebug_ptr_index(ptr) == 1);

	for(size_t i = 0; i < _ntrb_memdebug_ptr.elements / sizeof(void*); i++){
		_ntrb_memdebug_free(((void**)(_ntrb_memdebug_ptr.base_ptr))[i], __FILE__, __LINE__);
	}
	
	assert(_ntrb_memdebug_ptr_index(ptr) == -1);
	
	ntrb_memdebug_uninit(false);
}


static void test__ntrb_memdebug_malloc(){
	assert(ntrb_memdebug_init());
	
	const size_t allocsize = 26;
	void* const ptr = _ntrb_memdebug_malloc(allocsize, __FILE__, __LINE__);
	assert(ptr);
	
	assert( _ntrb_memdebug_ptr.elements == sizeof(void*) );
	assert( ((void**)(_ntrb_memdebug_ptr.base_ptr))[0] == ptr );
	assert( ((size_t*)(_ntrb_memdebug_size.base_ptr))[0] == allocsize );
	
	_ntrb_memdebug_free(ptr, __FILE__, __LINE__);
	ntrb_memdebug_uninit(false);	
}

static void test__ntrb_memdebug_calloc(){
	assert(ntrb_memdebug_init());
	
	const size_t elements = 5;
	const size_t typesize = sizeof(uint32_t);
	void* const ptr = _ntrb_memdebug_calloc(elements, typesize, __FILE__, __LINE__);
	assert(ptr);
	
	const size_t bytes = elements * typesize;
	for(size_t i = 0; i < bytes; i++){
		assert(((uint8_t*)ptr)[i] == 0);
	}

	assert( _ntrb_memdebug_ptr.elements == sizeof(void*) );
	assert( ((void**)(_ntrb_memdebug_ptr.base_ptr))[0] == ptr );
	assert( ((size_t*)(_ntrb_memdebug_size.base_ptr))[0] == bytes );
	
	_ntrb_memdebug_free(ptr, __FILE__, __LINE__);
	ntrb_memdebug_uninit(false);
}

static void test__ntrb_memdebug_realloc(){
	assert(ntrb_memdebug_init());
	
	//Element 0: test expanding and shrinking a registered pointer
	const int mock_line = 59;
	void* ptr_0 = _ntrb_memdebug_malloc(59, __FILE__, mock_line);
	assert(ptr_0);
	
	ptr_0 = _ntrb_memdebug_realloc(ptr_0, 72, __FILE__, mock_line);
	assert(ptr_0);	
	assert(_test__ntrb_memdebug_element_equal(0, ptr_0, 72, __FILE__, mock_line));
	
	ptr_0 = _ntrb_memdebug_realloc(ptr_0, 29, __FILE__, mock_line);
	assert(_test__ntrb_memdebug_element_equal(0, ptr_0, 29, __FILE__, mock_line));
	
	
	//Element 1: test reallocting and shrinking a null pointer, this should behave as malloc
	//you can't shrink a null pointer, as that would still act as reallocation
	void* ptr_1 = _ntrb_memdebug_realloc(NULL, 48, __FILE__, mock_line);
	assert(ptr_1);
	assert(_test__ntrb_memdebug_element_equal(1, ptr_1, 48, __FILE__, mock_line));
	
	ptr_1 = _ntrb_memdebug_realloc(ptr_1, 16, __FILE__, mock_line);
	assert(_test__ntrb_memdebug_element_equal(1, ptr_1, 16, __FILE__, mock_line));	
	
	
	//Element 2: test shirnking and then expanding a registed pointer
	void* ptr_2 = _ntrb_memdebug_malloc(59, __FILE__, mock_line);
	assert(ptr_2);
	ptr_2 = _ntrb_memdebug_realloc(ptr_2, 26, __FILE__, mock_line);
	assert(_test__ntrb_memdebug_element_equal(2, ptr_2, 26, __FILE__, mock_line));

	ptr_2 = _ntrb_memdebug_realloc(ptr_2, 105, __FILE__, mock_line);
	assert(ptr_2);
	assert(_test__ntrb_memdebug_element_equal(2, ptr_2, 105, __FILE__, mock_line));	
	
	
	//Element 3: test shrink and expanding an unregistered pointer
	printf("\n[Test]: %s %d: Testing shrinking and expanding unregisted pointer. _ntrb_memdebug_realloc() should warn reallocation of an unregisted pointer twice.\n", __FILE__, mock_line);	
	void* ptr_3 = malloc(79);
	assert(ptr_3);

	ptr_3 = _ntrb_memdebug_realloc(ptr_3, 35, __FILE__, mock_line);
	assert(ptr_3);
	
	ptr_3 = _ntrb_memdebug_realloc(ptr_3, 126, __FILE__, mock_line);
	assert(ptr_3);
	free(ptr_3);
	printf("[Test]: %s %d: No further warnings should appear after this, unless specified.\n\n", __FILE__, mock_line);
	
	const size_t ptr_count = _ntrb_memdebug_ptr.elements / sizeof(void*);
	assert(ptr_count == 3);	//element 3 is unregisted, it should not be added to the record.
	for(size_t i = 0; i < ptr_count; i++){
		_ntrb_memdebug_free(((void**)(_ntrb_memdebug_ptr.base_ptr))[i], __FILE__, __LINE__);
	}

	ntrb_memdebug_uninit(false);	
}

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
	#endif
}