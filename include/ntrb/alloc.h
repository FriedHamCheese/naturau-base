#ifndef ntrb_alloc_h
#define ntrb_alloc_h

/**
\file alloc.h
A module providing memory management functions. 

You always use the conditional macros provided (not the debugging implementations) if you are working in the ntrb code itself, but for other code using ntrb, it's up to you.
Just make sure that you compile with or without -DNTRB_MEMDEBUG in the same way across all of your code and ntrb itself.
If you have a codebase which uses ntrb and it still has a mix of stdlib memory functions, it's best to not use -DNTRB_MEMDEBUG both in ntrb and your code.

To debug memory management, add -DNTRB_MEMDEBUG to CFLAGS of ntrb and other codebase using ntrb. If you choose not to, simply make sure that ntrb and the code using ntrb are not compiled with -DNTRB_MEMDEBUG enabled.

The ntrb_(func)(...) are macros which transform to the underlying debugging implementations from ntrb if NTRB_MEMDEBUG is defined. If it is not defined, it turns into (func)(...). So ntrb_realloc would transform into _ntrb_memdebug_realloc if debugging is enabled, else just realloc.

\todo For 2.0, merge the 4 bytevecs into 1 and have a struct for the information instead.
*/

/**
\def ntrb_malloc(size_bytes)
A macro which either transforms into the debugging implementation _ntrb_memdebug_malloc() or stdlib malloc() depending on if NTRB_MEMDEBUG is defined or not respectively.
*/
/**
\def ntrb_calloc(elements, typesize)
A macro which either transforms into the debugging implementation _ntrb_memdebug_calloc() or stdlib calloc() depending on if NTRB_MEMDEBUG is defined or not respectively.
*/
/**
\def ntrb_realloc(ptr, size_bytes)
A macro which either transforms into the debugging implementation _ntrb_memdebug_realloc() or stdlib realloc() depending on if NTRB_MEMDEBUG is defined or not respectively.
*/
/**
\def ntrb_free(ptr)
A macro which either transforms into the debugging implementation _ntrb_memdebug_free() or stdlib free() depending on if NTRB_MEMDEBUG is defined or not respectively.
*/


#include "_alloc_bytevec.h"

#include <stddef.h>
#include <stdbool.h>

#ifdef NTRB_MEMDEBUG
	#define ntrb_malloc(size_bytes) _ntrb_memdebug_malloc(size_bytes, __FILE__, __LINE__)
	#define ntrb_calloc(elements, typesize) _ntrb_memdebug_calloc(elements, typesize, __FILE__, __LINE__)
	#define ntrb_realloc(ptr, size_bytes) _ntrb_memdebug_realloc(ptr, size_bytes, __FILE__, __LINE__)
	#define ntrb_free(ptr) _ntrb_memdebug_free(ptr, __FILE__, __LINE__)	
	
	/**
	Older way of initialising the alloc module.

	It calls the newer ntrb_memdebug_init_with_return_value() and returns true if the underlying function returns 0.
	Else it returns false.
	*/	
	bool ntrb_memdebug_init();
	
	/**
	Initialises the alloc module by initialising the rwlock of the module and the 4 _ntrb_alloc_bytevec which keep track of memory management information and then marks the module as successfully initialised.

	Returns 0 if all went well. If it returns ENOMEM, it either means the bytevecs couldn't be initialised or the rwlock fails to initialise due to the lack of memory. Anything else would be return values of the pthread_rwlock_init().

	An assert would fail if attempting to initialise the already initialised module.
	*/	
	int ntrb_memdebug_init_with_return_value();
	
	/**
	Uninitialises the module by freeing the bytevecs and destroying the rwlock.
	
	Returns 0 if sucessful. 
	
	If an error occurs when trying to lock, unlock or destroy the rwlock, the return value is from either pthread_rwlock_wrlock(), pthread_rwlock_unlock(), or pthread_rwlock_destroy().
	
	Because it is difficult to determine if the bytevecs are freed after the rwlock errors or not, it's okay to just let it fail in most cases. Unless you are tight on memory or something, then recursively calling it until it returns 0 may be a viable option.
	
	\param print_summary prints the remaining unallocated memory pointers if set to true. Good and convenient for checking memory leaks after testing your program.
	*/
	int ntrb_memdebug_uninit(const bool print_summary);
	
	/**
	Prints the record of unfreed pointers in the record, from allocation or reallocation.
	
	Returns 0 if the rwlock of the module has no issues requesting a read lock and unlocking. Else it returns the error value from acquiring a read lock or unlocking the lock. If this function fails, probably not a big deal, it's just printing text.
	*/
	int ntrb_memdebug_view();

	/**
	An array of void* which has been given memory allocation.
	
	We add pointers and their information to the appropriate containers if:
	- We allocate new memory by using ntrb_malloc or ntrb_calloc; 
	  or allocate with ntrb_realloc by passing a NULL pointer which acts as a malloc (exact behaviour as stdlib realloc).
	
	We edit the pointer and its information if:
	- We called ntrb_realloc with a pointer which is in the record already.
	
	And we delete the pointer and its information if:
	- We ntrb_free a pointer which is in the record.
	
	To access the pointer and its information, have an index of the pointer in void* type and access each of the container with ((type_in_container*)(container.base_ptr))[index]. For example ((size_t*)(_ntrb_memdebug_size))[2];
	
	We use container.elements as the boundary for seeking through the record, as we need to make sure that the boundary guarantees no garbage value. If we mark an element unused and the element is not at the end, we don't change the .elements value. We only change when we remove the last element of the container, then we subtract sizeof(type_in_container) from container.elements in each of the containers.
	
	This is only left exposed for testing the module.
	
	Defined in alloc.c
	*/
	extern _ntrb_alloc_bytevec _ntrb_memdebug_ptr;
	
	/**
	An array of size_t containing the allocation size of each pointer.
	
	This is only left exposed for testing the module. Refer to the documentation on _ntrb_memdebug_ptr for how to access this container properly.
	
	Defined in alloc.c
	*/
	extern _ntrb_alloc_bytevec _ntrb_memdebug_size;
	
	/**
	An array of const char* containing the filename of each allocation/reallocation site. Typically it's from (double underscore)FILE(double underscore) so don't free any of the pointers in this container.
	
	This is only left exposed for testing the module. Refer to the documentation on _ntrb_memdebug_ptr for how to access this container properly.
	
	Defined in alloc.c
	*/
	extern _ntrb_alloc_bytevec _ntrb_memdebug_filename;
	
	/**
	An array of int which is the line number at each allocation/reallocation site. Typically it's from (double underscore)LINE(double underscore).
	
	This is only left exposed for testing the module. Refer to the documentation on _ntrb_memdebug_ptr for how to access this container properly.
	
	Defined in alloc.c
	*/
	extern _ntrb_alloc_bytevec _ntrb_memdebug_line;

	/**
	Allocates a new pointer with size_bytes bytes of space and keeps a record of: the pointer, allocation size, callsite filename and line.
	
	Returns the allocated pointer if the allocation is successful. If the underlying malloc fails, the function will not write to the record containers and a NULL is returned.
	
	If _ntrb_memdebug_rwlock encounters error while locking or unlocking, a message will be printed to stderr.
	And if any of the record containers failed to allocate space for keeping the record, assertions will fail.
	*/
	void* _ntrb_memdebug_malloc(const size_t size_bytes, const char* const filename, const int line);
	
	/**
	Allocates a new pointer with elements x typesize bytes of space, set every byte to 0 and keeps a record of: the pointer, allocation size, callsite filename and line.
	
	Returns the allocated pointer if the allocation is successful. If the underlying malloc fails, the function will not write to the record containers and a NULL is returned
	
	If _ntrb_memdebug_rwlock encounters error while locking or unlocking, a message will be printed to stderr and the record containers will not be accessed.
	And if any of the containers failed to allocate space for keeping the record, assertions will fail.
	*/
	void* _ntrb_memdebug_calloc(const size_t elements, const size_t typesize, const char* const filename, const int line);
	
	/**
	Either allocates, reallocates or shrinks a pointer, and add or edit the record containers.
	
	These are the possible scenarios of calling the function and the behaviour of the function are as follows:
	- Failed to allocate, reallocate or shrink the memory: returns NULL and does not access the record containers. The passed pointer is left unchanged.
	- If an error occurs while the _ntrb_memdebug_rwlock is requesting a write lock: a message will be printed in stderr and the record containers would not be changed, which causes inaccuracies of the record.
	- If an error occurs while the _ntrb_memdebug_rwlock is requesting an unlock: a message will be printed in stderr but the record containers can be mutated.
	- If the passed pointer is NULL: add the allocated pointer and its callsite information to the record.
	- If the passed pointer is not in the record when reallocating or shrinking: does the request, but does not add the reallocated or shrunk pointer to the record and warns through stdout.
	- If the passed pointer is in the record, does the request and edit its information in the record accordingly.
	*/
	void* _ntrb_memdebug_realloc(void* const ptr, const size_t size_bytes, const char* const filename, const int line);
	
	/**
	Frees the passed pointer and delete its record from the containers.
	
	Possibilities:
	- passed a NULL pointer: does nothing.
	- If an error occurs while the _ntrb_memdebug_rwlock is requesting a write lock: a message will be printed in stderr and the record containers would be unchanged, which causes inaccuracies of the record.
	- passed a pointer which is in the record: frees the pointer and remove it from the record.
	- passed a pointer which is not in the record: does not free the pointer and warns through stderr.
	- If an error occurs while the _ntrb_memdebug_rwlock is requesting an unlock: a message will be printed in stderr but the record containers can be mutated.
	*/
	void _ntrb_memdebug_free(void* const ptr, const char* const filename, const int line);

	/**
	Prints the record of unfreed pointers in the record, from allocation or reallocation. As the name suggests, it does not lock the rwlock when accessing the record containers. This is for internal use only.
	*/
	void _ntrb_memdebug_view_no_lock();

	/**
	Finds unused space between the data in the record containers and tries to add the information to it.
	
	Returns true if there was an empty space and the function added the information to it. False if there was none and the information was not added.
	
	This only checks the space in the _ntrb_memdebug_ptr but write to other containers without checking.
	It does the job if you have manipulated the containers in the correct behaviour so far.
	*/
	bool _ntrb_memdebug_add_element_to_unused_space(void* const ptr, const size_t size_bytes, const char* const filename, const int line);
	/**
	Adds the information provided to the record containers.
	
	If there is an empty space between the data in the containers, it adds the information to it (_ntrb_memdebug_add_element_to_unused_space()).
	If there isn't, try adding it at the end of the containers, and if the container fails to allocate the space for it, assertions will fail.
	
	\todo get rid of the asserts and replace it with a return boolean or some sort.
	*/
	void _ntrb_memdebug_add_element(void* const ptr, const size_t size_bytes, const char* const filename, const int line);
	
	/**
	Marks the space which i_element points to as unused.
	
	\param i_element index of the data in each of the record containers to mark as unused.
	\param element_count the elements of any of the containers which we can iterate through without reading garbage. Just pass in (container.elements) / sizeof(type_in_container).
	*/
	void _ntrb_memdebug_remove_element(const size_t i_element, const size_t element_count);
	
	/**
	Returns the element index of the pointer in _ntrb_memdebug_ptr.
	
	If the pointer is not in the container, it returns -1.
	*/
	int_least64_t _ntrb_memdebug_ptr_index(const void* const ptr);
	
	/**
	Overwrites the data in each of the container at i_element with the provided data.
	
	This does not perform bounds check.
	*/
	void _ntrb_memdebug_replace_element(const size_t i_element, void* const ptr, const size_t size_bytes, const char* const filename, const int line);	
#else
	#define ntrb_malloc(size_bytes) malloc(size_bytes)
	#define ntrb_calloc(elements, typesize) calloc(elements, typesize)
	#define ntrb_realloc(ptr, size_bytes) realloc(ptr, size_bytes)
	#define ntrb_free(ptr) free(ptr)
#endif
#endif