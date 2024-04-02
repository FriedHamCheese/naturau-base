#include "alloc.h"
#include "_alloc_bytevec.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include <pthread.h>
	
pthread_rwlock_t _ntrb_memdebug_rwlock;
	
static _Atomic int_least64_t _ntrb_memdebug_initialized_value;
//i just mashed my keypad kekew
static const int_least64_t _ntrb_memdebug_correct_initialized_value = 84984981896;
	
_ntrb_alloc_bytevec _ntrb_memdebug_ptr;
_ntrb_alloc_bytevec _ntrb_memdebug_size;
_ntrb_alloc_bytevec _ntrb_memdebug_filename;
_ntrb_alloc_bytevec _ntrb_memdebug_line;

#ifdef NTRB_MEMDEBUG
bool ntrb_memdebug_init(){
	return ntrb_memdebug_init_with_return_value() == 0;
}

int ntrb_memdebug_init_with_return_value(){
	assert(_ntrb_memdebug_initialized_value != _ntrb_memdebug_correct_initialized_value);
	
	const int rwlock_init_error = pthread_rwlock_init(&_ntrb_memdebug_rwlock, NULL);
	if(rwlock_init_error) return rwlock_init_error;
	
	_ntrb_memdebug_ptr = _ntrb_alloc_bytevec_new(sizeof(void*));
	if(_ntrb_memdebug_ptr.base_ptr == NULL) return ENOMEM;
	
	_ntrb_memdebug_size = _ntrb_alloc_bytevec_new(sizeof(size_t));
	if(_ntrb_memdebug_size.base_ptr == NULL) return ENOMEM;
	
	_ntrb_memdebug_filename = _ntrb_alloc_bytevec_new(sizeof(const char*));
	if(_ntrb_memdebug_filename.base_ptr == NULL) return ENOMEM;
	
	_ntrb_memdebug_line = _ntrb_alloc_bytevec_new(sizeof(int));
	if(_ntrb_memdebug_line.base_ptr == NULL) return ENOMEM;
	
	_ntrb_memdebug_initialized_value = _ntrb_memdebug_correct_initialized_value;	
	return 0;
}

int ntrb_memdebug_uninit(const bool print_summary){
	assert(_ntrb_memdebug_initialized_value == _ntrb_memdebug_correct_initialized_value);	
	
	const int wrlock_error = pthread_rwlock_wrlock(&_ntrb_memdebug_rwlock);
	if(wrlock_error) return wrlock_error;
		
	if(print_summary){
		printf("[Info]: ntrb_memdebug_uninit() called, the resources listed below are not freed properly.\n");
		_ntrb_memdebug_view_no_lock();
	}
	
	_ntrb_alloc_bytevec_free(&_ntrb_memdebug_ptr);
	_ntrb_alloc_bytevec_free(&_ntrb_memdebug_size);
	_ntrb_alloc_bytevec_free(&_ntrb_memdebug_filename);
	_ntrb_alloc_bytevec_free(&_ntrb_memdebug_line);	
	
	_ntrb_memdebug_initialized_value = 125;
	const int unlock_error = pthread_rwlock_unlock(&_ntrb_memdebug_rwlock);	
	if(unlock_error) return unlock_error;
	
	const int destroy_error = pthread_rwlock_destroy(&_ntrb_memdebug_rwlock);
	if(destroy_error) return destroy_error;	
	
	return 0;
}


int ntrb_memdebug_view(){
	assert(_ntrb_memdebug_initialized_value == _ntrb_memdebug_correct_initialized_value);	
	
	const int rdlock_error = pthread_rwlock_rdlock(&_ntrb_memdebug_rwlock);
	if(rdlock_error) return rdlock_error;
	
	_ntrb_memdebug_view_no_lock();
	
	const int unlock_error = pthread_rwlock_unlock(&_ntrb_memdebug_rwlock);		
	if(unlock_error) return unlock_error;
	
	return 0;
}

void _ntrb_memdebug_view_no_lock(){	
	bool printed_an_element = false;
	for(size_t i = 0; i < _ntrb_memdebug_ptr.elements/sizeof(void*); i++){
		const void* ptr = *(void**)(_ntrb_memdebug_ptr.base_ptr + i*sizeof(void*));

		if(ptr){
			const size_t size = *(size_t*)(_ntrb_memdebug_size.base_ptr + i*sizeof(size_t));
			const char* const filename = *(const char**)(_ntrb_memdebug_filename.base_ptr + i*sizeof(const char*));
			const int line = *(int*)(_ntrb_memdebug_line.base_ptr + i*sizeof(int));			
			
			printf("i: %llu | %s line %d: 0x%p %llu bytes.\n", i, filename, line, ptr, size);
			printed_an_element = true;
		}
	}
	
	if(!printed_an_element){
		printf("[Info]: ntrb_memdebug_view(): no allocated memory.\n");
	}
}


void* _ntrb_memdebug_malloc(const size_t size_bytes, const char* const filename, const int line){	
	void* const ptr = malloc(size_bytes);
	if(ptr){
		const int wrlock_error = pthread_rwlock_wrlock(&_ntrb_memdebug_rwlock);
		if(wrlock_error){
			fprintf(stderr, "[Error]: %s %d: _ntrb_memdebug_malloc(): Error requesting a write lock (%d).\nThe allocated pointer will not be added to record.\n", filename, line, wrlock_error);
		}else{
			_ntrb_memdebug_add_element(ptr, size_bytes, filename, line);
			
			const int unlock_error = pthread_rwlock_unlock(&_ntrb_memdebug_rwlock);
			if(unlock_error){
				fprintf(stderr, "[Error]: %s %d: _ntrb_memdebug_malloc(): Error unlocking a write lock (%d).\n", filename, line, wrlock_error);
			}
		}
	}
	
	return ptr;
}

void* _ntrb_memdebug_calloc(const size_t elements, const size_t typesize, const char* const filename, const int line){
	const size_t bytes = elements * typesize;
	void* ptr = _ntrb_memdebug_malloc(bytes, filename, line);
	
	if(ptr) memset(ptr, (uint8_t)(0), bytes);
	return ptr;
}

void* _ntrb_memdebug_realloc(void* const ptr, const size_t size_bytes, const char* const filename, const int line){
	void* const realloc_ptr = realloc(ptr, size_bytes);
	//failed allocation; return null, don't need to modify the record
	if(!realloc_ptr) return NULL;
	
	//the original pointer was null, standard realloc would act as malloc, so we just add the allocated pointer to the record.
	
	const int wrlock_error = pthread_rwlock_wrlock(&_ntrb_memdebug_rwlock);
	if(wrlock_error){
		fprintf(stderr, "[Error]: %s %d: _ntrb_memdebug_realloc(): Error requesting a write lock (%d).\nAccess to the memory record is unavailable\n", filename, line, wrlock_error);		
	}else{	
		if(ptr == NULL) 
			_ntrb_memdebug_add_element(realloc_ptr, size_bytes, filename, line);
		else{
			//the original pointer is either expanded, or shrunken, we cannot be certain.
			//if the original pointer is in the record, we change its record contents according to the arguments,
			//if not, we don't add the unregistered pointer to the record, rather we just warn
			const int_least64_t i_ptr = _ntrb_memdebug_ptr_index(ptr);
			const bool unregistered_original_ptr = i_ptr == -1;
			if(unregistered_original_ptr){
				printf("[Warn]: %s %d: _ntrb_memdebug_realloc(): Reallocated 0x%p from unregistered pointer 0x%p. Both pointers will not be added to the record.\n", filename, line, realloc_ptr, ptr);
			}else{
				_ntrb_memdebug_replace_element(i_ptr, realloc_ptr, size_bytes, filename, line);
			}
		}
			
		const int unlock_error = pthread_rwlock_unlock(&_ntrb_memdebug_rwlock);
		if(unlock_error){
			fprintf(stderr, "[Error]: %s %d: _ntrb_memdebug_realloc(): Error unlocking a write lock (%d).\n", filename, line, wrlock_error);
		}
	}
	
	return realloc_ptr;
}

void _ntrb_memdebug_free(void* const ptr, const char* const filename, const int line){
	assert(_ntrb_memdebug_initialized_value == _ntrb_memdebug_correct_initialized_value);
	
	const int wrlock_error = pthread_rwlock_wrlock(&_ntrb_memdebug_rwlock);
	if(wrlock_error){
		fprintf(stderr, "[Error]: %s %d: _ntrb_memdebug_free(): Error requesting a write lock (%d).\nThe pointer cannot be freed.\n", filename, line, wrlock_error);
		return;
	}
		
	const size_t ptr_count = _ntrb_memdebug_ptr.elements / sizeof(void*);
	for(size_t i = 0; i < ptr_count; i++){
		void* it_ptr = ((void**)(_ntrb_memdebug_ptr.base_ptr))[i];
		const bool ptr_in_record = it_ptr == ptr;
		if(ptr_in_record){
			_ntrb_memdebug_remove_element(i, ptr_count);
			free(ptr);
			pthread_rwlock_unlock(&_ntrb_memdebug_rwlock);				
			return;
		}
	}
	
	const int unlock_error = pthread_rwlock_unlock(&_ntrb_memdebug_rwlock);
	if(unlock_error){
		fprintf(stderr, "[Error]: %s %d: _ntrb_memdebug_free(): Error unlocking a write lock (%d).\n", filename, line, wrlock_error);
	}
	
	printf("[Warn]: %s %d: _ntrb_memdebug_free(): Prevented freeing unallocated 0x%p!\n", filename, line, ptr);
}


void _ntrb_memdebug_add_element(void* const ptr, const size_t size_bytes, const char* const filename, const int line){
	assert(_ntrb_memdebug_initialized_value == _ntrb_memdebug_correct_initialized_value);

	if(!_ntrb_memdebug_add_element_to_unused_space(ptr, size_bytes, filename, line)){
		assert(_ntrb_alloc_bytevec_append(&_ntrb_memdebug_ptr, sizeof(void*), &ptr));
		assert(_ntrb_alloc_bytevec_append(&_ntrb_memdebug_size, sizeof(size_t), &size_bytes));
		assert(_ntrb_alloc_bytevec_append(&_ntrb_memdebug_filename, sizeof(const char*), &filename));
		assert(_ntrb_alloc_bytevec_append(&_ntrb_memdebug_line, sizeof(int), &line));		
	}
}

bool _ntrb_memdebug_add_element_to_unused_space(void* const ptr, const size_t size_bytes, const char* const filename, const int line){
	const size_t ptr_count = _ntrb_memdebug_ptr.elements / sizeof(void*);
	
	for(size_t i = 0; i < ptr_count; i++){
		void* i_ptr = ((void**)(_ntrb_memdebug_ptr.base_ptr))[i];
		
		const bool ptr_slot_is_empty = !i_ptr;
		if(ptr_slot_is_empty){
			((void**)(_ntrb_memdebug_ptr.base_ptr))[i] = ptr;
			((size_t*)(_ntrb_memdebug_size.base_ptr))[i] = size_bytes;
			((const char**)(_ntrb_memdebug_filename.base_ptr))[i] = filename;
			((int*)(_ntrb_memdebug_line.base_ptr))[i] = line;
			return true;
		}
	}
	return false;
}

int_least64_t _ntrb_memdebug_ptr_index(const void* const ptr){	
	assert(_ntrb_memdebug_initialized_value == _ntrb_memdebug_correct_initialized_value);

	const size_t ptr_count = _ntrb_memdebug_ptr.elements / sizeof(void*);
	
	for(size_t i = 0; i < ptr_count; i++){
		const void* const i_ptr = ((void**)(_ntrb_memdebug_ptr.base_ptr))[i];
		if(i_ptr == ptr) return i;
	}
	
	return -1;
}

void _ntrb_memdebug_replace_element(const size_t i_element, void* const ptr, const size_t size_bytes, const char* const filename, const int line){
	assert(_ntrb_memdebug_initialized_value == _ntrb_memdebug_correct_initialized_value);

	((void**)(_ntrb_memdebug_ptr.base_ptr))[i_element] = ptr;
	((size_t*)(_ntrb_memdebug_size.base_ptr))[i_element] = size_bytes;
	((const char**)(_ntrb_memdebug_filename.base_ptr))[i_element] = filename;
	((int*)(_ntrb_memdebug_line.base_ptr))[i_element] = line;
}

void _ntrb_memdebug_remove_element(const size_t i_element, const size_t element_count){
	assert(_ntrb_memdebug_initialized_value == _ntrb_memdebug_correct_initialized_value);
	
	((void**)(_ntrb_memdebug_ptr.base_ptr))[i_element] = NULL;
	((size_t*)(_ntrb_memdebug_size.base_ptr))[i_element] = 0;
	((const char**)(_ntrb_memdebug_filename.base_ptr))[i_element] = NULL;
	((int*)(_ntrb_memdebug_line.base_ptr))[i_element] = 0;
	
	const bool is_last_element = (i_element+1 == element_count);
	if(is_last_element){
		_ntrb_memdebug_ptr.elements -= sizeof(void*);
		_ntrb_memdebug_size.elements -= sizeof(size_t);
		_ntrb_memdebug_filename.elements -= sizeof(const char*);
		_ntrb_memdebug_line.elements -= sizeof(int);
	}
}

#endif