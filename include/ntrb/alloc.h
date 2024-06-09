#ifndef ntrb_alloc_h
#define ntrb_alloc_h

#include "_alloc_bytevec.h"

#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>

/*
Copyright 2024 Pawikan Boonnaum

This file is part of naturau-base/ntrb.

naturau-base/ntrb is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

naturau-base/ntrb is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with naturau-base/ntrb. If not, see <https://www.gnu.org/licenses/>.
*/


/**
\file alloc.h
A module providing memory management functions. 

You always use the conditional macros provided (not the debugging implementations) if you are working in the ntrb code itself, but for other code using ntrb, it's up to you.
Just make sure that you compile with or without -DNTRB_MEMDEBUG in the same way across all of your code and ntrb itself.
If you have a codebase which uses ntrb and it still has a mix of stdlib memory functions, it's best to not use -DNTRB_MEMDEBUG both in ntrb and your code.

To debug memory management, add -DNTRB_MEMDEBUG to CFLAGS of ntrb and other codebase using ntrb. If you choose not to, simply make sure that ntrb and the code using ntrb are not compiled with -DNTRB_MEMDEBUG enabled.

In your int main() of your codebase(s), simply have ntrb_memdebug_init_with_return_value() at the very top,
and ntrb_memdebug_uninit(true) at the bottom, each guarded with an #ifdef NTRB_MEMDEBUG and an #endif. 
So your codebase could both operate with or without memory debugging.

Typically you would give ntrb_memdebug_uninit() a true, which makes it to display any unfreed pointers which are recorded by the debugging implementations before deinitialising the module.

Go through your code and replace malloc, calloc, realloc and free; with ntrb_* macro equivalents. And that's pretty much it :D The parameters for the macros are identical to that of the stdlib counterparts, guaranteeing ease of change.

The ntrb_(func)(...) are macros which transform to the underlying debugging implementations from ntrb if NTRB_MEMDEBUG is defined. If it is not defined, it turns into (func)(...). So ntrb_realloc would transform into _ntrb_memdebug_realloc if debugging is enabled, else just realloc.
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

#ifdef NTRB_MEMDEBUG
	#define ntrb_malloc(size_bytes) _ntrb_memdebug_malloc(size_bytes, __FILE__, __LINE__)
	#define ntrb_calloc(elements, typesize) _ntrb_memdebug_calloc(elements, typesize, __FILE__, __LINE__)
	#define ntrb_realloc(ptr, size_bytes) _ntrb_memdebug_realloc(ptr, size_bytes, __FILE__, __LINE__, _ntrb_memdebug_unresgistered_realloc_ptr_callback)
	#define ntrb_free(ptr) _ntrb_memdebug_free(ptr, __FILE__, __LINE__)	
	
	/**
	A struct containing information on an allocation or a reallocation from functions in alloc.c or alloc.h.
	*/
	typedef struct{
		void* ptr;
		size_t allocsize_bytes;
		const char* callsite_filename;
		int callsite_line;
	} _ntrb_memdebug_AllocData;
	
	/**
	Generic enum representing errors in alloc.c or alloc.h.
	*/
	enum ntrb_memdebug_Error{
		///Indicates a successful operation from the function.
		ntrb_memdebug_OK,
		
		/**
		Attempted to initialise the already initialised module.
	
		This only gets returned from functions which initialises the module.
		*/
		ntrb_memdebug_AlreadyInit,
		
		///The module is not yet initialised.
		ntrb_memdebug_NotInit,
		
		///Failed to allocate memory for the record container (_ntrb_memdebug_alloc_data).
		ntrb_memdebug_AllocError,
		
		/**
		Failed to initialise the rwlock of the module.

		This only gets returned from functions which initialises the module.		
		*/
		ntrb_memdebug_RwlockInitError,
		///Failed to acquire either the read lock or write lock.
		ntrb_memdebug_RwlockAcqError,
		///Failed to unlock the rwlock of the module.
		ntrb_memdebug_RwlockUnlockError,
		/**
		Failed to destroy the rwlock of the module.
		
		This only gets returned from _ntrb_memdebug_uninit().
		*/
		ntrb_memdebug_RwlockDestroyError,
		
	};
	
	/**
	The "record" or "record container" which contains information on allocations or reallocations made by functions of the module.
	
	It contains multiple _ntrb_memdebug_AllocData, each representing a single allocation or reallocation.
	
	If an element of _ntrb_memdebug_AllocData in the container has its .ptr as NULL, this indicates an empty space.
	For performance and efficiency reasons, you would try to write to this first, as the _ntrb_memdebug_add_element_to_unused_space() in _ntrb_memdebug_add_element() does.
	*/
	extern  _ntrb_alloc_bytevec _ntrb_memdebug_alloc_data;
	
	/**
	Older way of initialising the alloc module.

	It calls the newer ntrb_memdebug_init_with_return_value() and returns true if the underlying function returns 0.
	Else it returns false.
	*/	
	bool ntrb_memdebug_init();
	
	/**
	Initialises the alloc module by initialising the rwlock of the module and the record which keep track of memory management information, and then marks the module as successfully initialised.
	
	- If the module is already initialised, it returns ntrb_memdebug_AlreadyInit.
	- If the rwlock initialisation fails, it returns ntrb_memdebug_RwlockInitError.
	- If the allocation of the record, _ntrb_memdebug_alloc_data, fails; it returns ntrb_memdebug_AllocError.
	- Else it returns ntrb_memdebug_OK, if any of the mentioned has not occurred.
	*/	
	enum ntrb_memdebug_Error ntrb_memdebug_init_with_return_value();
	
	/**
	Uninitialises the module by freeing the record container and destroying the rwlock.
	
	- If the module is not initialised, it returns ntrb_memdebug_NotInit.
	- If there was an error acquiring the write lock of the module rwlock, it returns ntrb_memdebug_RwlockAcqError.
	- If there was an error releasing the write lock of the module, the record container is now freed and the module is identified as uninitialised.
	- If there was an error destroying the rwlock of the module, the record container is now freed and the module is identified as uninitialised.
	- Else it returns ntrb_memdebug_OK, if any of the mentioned has not occurred.
	
	\param print_summary prints the remaining unallocated memory pointers if set to true. Good and convenient for checking memory leaks after testing your program.
	*/
	enum ntrb_memdebug_Error ntrb_memdebug_uninit(const bool print_summary);
	
	/**
	Prints the record of unfreed pointers in the record, from allocation or reallocation.
		
	- If the module is not initialised, it returns ntrb_memdebug_NotInit.
	- If there was an error acquiring the read lock of the module rwlock, it returns ntrb_memdebug_RwlockAcqError.
	- If there was an error releasing the lock of the module, the function will have printed the record and will return ntrb_memdebug_RwlockUnlockError.
	- Else it returns ntrb_memdebug_OK, if any of the mentioned has not occurred.
	*/
	enum ntrb_memdebug_Error ntrb_memdebug_view();

	/**
	Allocates a new pointer with size_bytes bytes of space and keeps a record of the pointer, allocation size, callsite filename and line.
	
	Returns the allocated pointer if the allocation is successful. If the underlying malloc fails, the function will not write to the record container and a NULL is returned.
	
	- If the rwlock of the module fails to acquire a write lock;
	  the allocated memory is provided in the return value, but not logged to the record and a message will be printed to stderr regarding the error.
	- If there isn't sufficient memory for the record container;
	  the allocated memory is provided in the return value, but not logged to the record and a message will be printed to stderr regarding the error.
	- If the rwlock of the module fails to release the write lock; 
	  the allocated memory is provided in the return value, the allocation is logged to the record and a message is printed to stderr regarding the error.
	- Else, the allocation is logged and no message is printed to stderr.
	*/
	void* _ntrb_memdebug_malloc(const size_t size_bytes, const char* const filename, const int line);
	
	/**
	Allocates a new pointer with elements x typesize bytes of space, set every byte to 0 and keeps a record of the pointer, allocation size, callsite filename and line.
	
	Returns the allocated pointer if the allocation is successful. If the underlying malloc fails, the function will not write to the record container and a NULL is returned.
	
	- If the rwlock of the module fails to acquire a write lock;
	  the allocated memory is provided in the return value, but not logged to the record and a message will be printed to stderr regarding the error.
	- If there isn't sufficient memory for the record container;
	  the allocated memory is provided in the return value, but not logged to the record and a message will be printed to stderr regarding the error.
	- If the rwlock of the module fails to release the write lock; 
	  the allocated memory is provided in the return value, the allocation is logged to the record and a message is printed to stderr regarding the error.
	- Else, the allocation is logged and no message is printed to stderr.
	*/
	void* _ntrb_memdebug_calloc(const size_t elements, const size_t typesize, const char* const filename, const int line);
	
	/**
	The callback for _ntrb_memdebug_realloc() when a reallocating or shrinking of a pointer not in the record.
	
	This prints a message to stderr.
	*/
	void _ntrb_memdebug_unresgistered_realloc_ptr_callback(const void* const realloced_ptr, const void* const requested_ptr);

	/**
	Either allocates, reallocates or shrinks a pointer, and add or edit the record containers.
	
	These are the possible scenarios of calling the function:
	- Failed to allocate, reallocate or shrink the memory: returns NULL and does not access the record container. The passed pointer and its contents are left unchanged.
	- If an error occurs while the _ntrb_memdebug_rwlock is requesting a write lock: a message will be printed in stderr and the record container would not be accessed, causing inaccuracies of the record.
	- If an error occurs while the _ntrb_memdebug_rwlock is requesting an unlock: a message will be printed in stderr but the record containers will be accessed.
	- If the passed pointer is NULL: add the allocated pointer and its callsite information to the record.
	- If the passed pointer is not in the record when reallocating or shrinking: does the request, but does not add the reallocated or shrunk pointer to the record and warns through stdout.
	- If the passed pointer is in the record, does the request and edit its information in the record accordingly.
	
	\param unregistered_ptr_callback A callback for when the function is requested to reallocate or shrink a pointer which is not in the record.
	*/
	void* _ntrb_memdebug_realloc(void* const ptr, const size_t size_bytes, const char* const filename, const int line, void (*unregistered_ptr_callback)(const void*, const void*));
	
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
	Finds unused space between the data in the record container and tries to add the information to it.
	
	Returns true if there was an empty space and the function added the information to it. False if there was none and the information was not added.
	*/
	bool _ntrb_memdebug_add_element_to_unused_space(void* const ptr, const size_t size_bytes, const char* const filename, const int line);
	
	/**
	Adds the information provided to the record container.
	
	If there is an empty space between the data in the container, it adds the information to it (_ntrb_memdebug_add_element_to_unused_space()).
	If there isn't, it tries adding it at the end of the container. And if the container fails to allocate memory for adding the information, the function returns false.
	If the function successfully added the information to the record, whether it is added to the unused space or not, the function returns true.
	*/
	bool _ntrb_memdebug_add_element(void* const ptr, const size_t size_bytes, const char* const filename, const int line);
	
	/**
	Marks the space which i_element points to as unused.

	i_element and element_count are interpreted as n element of _ntrb_memdebug_AllocData, not in bytes.
	
	\param i_element Index of the data in the record container to mark as unused.
	\param element_count The elements of _ntrb_memdebug_AllocData in _ntrb_memdebug_alloc_data.
	*/
	void _ntrb_memdebug_remove_element(const size_t i_element, const size_t element_count);
	
	/**
	Returns the element index of the pointer in _ntrb_memdebug_alloc_data.
	
	i_element is interpreted as element of _ntrb_memdebug_AllocData, not in bytes.	
	
	If the pointer is not in the container, it returns -1.
	*/
	int_least64_t _ntrb_memdebug_ptr_index(const void* const ptr);
	
	/**
	Overwrites the data in the record container at i_element with the provided data.
	
	i_element is interpreted as element of _ntrb_memdebug_AllocData, not in bytes.
		
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