#ifndef ntrb_alloc_h
#define ntrb_alloc_h

#include "_alloc_bytevec.h"

#include <stddef.h>
#include <stdbool.h>

#ifdef NTRB_MEMDEBUG
	#define ntrb_malloc(size_bytes) _ntrb_memdebug_malloc(size_bytes, __FILE__, __LINE__)
	#define ntrb_calloc(elements, typesize) _ntrb_memdebug_calloc(elements, typesize, __FILE__, __LINE__)
	#define ntrb_realloc(ptr, size_bytes) _ntrb_memdebug_realloc(ptr, size_bytes, __FILE__, __LINE__)
	#define ntrb_free(ptr) _ntrb_memdebug_free(ptr, __FILE__, __LINE__)	
	
	//preventing unintended initialisation of memdebug
	bool ntrb_memdebug_init();
	int ntrb_memdebug_init_with_return_value();
	int ntrb_memdebug_uninit(const bool print_summary);
	int ntrb_memdebug_view();

	//from alloc.c
	extern _ntrb_alloc_bytevec _ntrb_memdebug_ptr;
	extern _ntrb_alloc_bytevec _ntrb_memdebug_size;
	extern _ntrb_alloc_bytevec _ntrb_memdebug_filename;
	extern _ntrb_alloc_bytevec _ntrb_memdebug_line;

	void* _ntrb_memdebug_malloc(const size_t size_bytes, const char* const filename, const int line);
	void* _ntrb_memdebug_calloc(const size_t elements, const size_t typesize, const char* const filename, const int line);
	void* _ntrb_memdebug_realloc(void* const ptr, const size_t size_bytes, const char* const filename, const int line);
	void _ntrb_memdebug_free(void* const ptr, const char* const filename, const int line);

	void _ntrb_memdebug_view_no_lock();

	bool _ntrb_memdebug_add_element_to_unused_space(void* const ptr, const size_t size_bytes, const char* const filename, const int line);
	void _ntrb_memdebug_add_element(void* const ptr, const size_t size_bytes, const char* const filename, const int line);
	void _ntrb_memdebug_remove_element(const size_t i_element, const size_t element_count);
	int_least64_t _ntrb_memdebug_ptr_index(const void* const ptr);
	void _ntrb_memdebug_replace_element(const size_t i_element, void* const ptr, const size_t size_bytes, const char* const filename, const int line);	
#else
	#define ntrb_malloc(size_bytes) malloc(size_bytes)
	#define ntrb_calloc(elements, typesize) calloc(elements, typesize)
	#define ntrb_realloc(ptr, size_bytes) realloc(ptr, size_bytes)
	#define ntrb_free(ptr) free(ptr)
#endif
#endif