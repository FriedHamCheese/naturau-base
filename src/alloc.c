#include "alloc.h"
#include "_alloc_bytevec.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include <pthread.h>

#ifdef NTRB_MEMDEBUG
pthread_rwlock_t _ntrb_memdebug_rwlock;
	
static _Atomic int_least64_t _ntrb_memdebug_initialized_value;
//i just mashed my keypad kekew
static const int_least64_t _ntrb_memdebug_correct_initialized_value = 84984981896;
	
_ntrb_alloc_bytevec _ntrb_memdebug_alloc_data;

bool ntrb_memdebug_init(){
	return ntrb_memdebug_init_with_return_value() == 0;
}

enum ntrb_memdebug_Error ntrb_memdebug_init_with_return_value(){
	if(_ntrb_memdebug_initialized_value == _ntrb_memdebug_correct_initialized_value)
		return ntrb_memdebug_AlreadyInit;
	
	const int rwlock_init_error = pthread_rwlock_init(&_ntrb_memdebug_rwlock, NULL);
	if(rwlock_init_error) return ntrb_memdebug_RwlockInitError;
	
	_ntrb_memdebug_alloc_data = _ntrb_alloc_bytevec_new(sizeof(_ntrb_memdebug_AllocData));
	if(_ntrb_memdebug_alloc_data.base_ptr == NULL){
		pthread_rwlock_destroy(&_ntrb_memdebug_rwlock);
		return ntrb_memdebug_AllocError;	
	}
	
	_ntrb_memdebug_initialized_value = _ntrb_memdebug_correct_initialized_value;	
	return ntrb_memdebug_OK;
}

enum ntrb_memdebug_Error ntrb_memdebug_uninit(const bool print_summary){
	if(_ntrb_memdebug_initialized_value != _ntrb_memdebug_correct_initialized_value)
		return ntrb_memdebug_NotInit;
	
	const int wrlock_error = pthread_rwlock_wrlock(&_ntrb_memdebug_rwlock);
	if(wrlock_error) return ntrb_memdebug_RwlockAcqError;
		
	if(print_summary){
		printf("[Info]: ntrb_memdebug_uninit() called, the resources listed below are not freed properly.\n");
		_ntrb_memdebug_view_no_lock();
	}
	
	_ntrb_alloc_bytevec_free(&_ntrb_memdebug_alloc_data);
	
	//any number that isn't the initialised value
	_ntrb_memdebug_initialized_value = 125;
	const int unlock_error = pthread_rwlock_unlock(&_ntrb_memdebug_rwlock);	
	if(unlock_error) return ntrb_memdebug_RwlockUnlockError;
	
	const int destroy_error = pthread_rwlock_destroy(&_ntrb_memdebug_rwlock);
	if(destroy_error) return ntrb_memdebug_RwlockDestroyError;	
	
	return ntrb_memdebug_OK;
}


enum ntrb_memdebug_Error ntrb_memdebug_view(){
	if(_ntrb_memdebug_initialized_value != _ntrb_memdebug_correct_initialized_value)
		return ntrb_memdebug_NotInit;
	
	const int rdlock_error = pthread_rwlock_rdlock(&_ntrb_memdebug_rwlock);
	if(rdlock_error) return ntrb_memdebug_RwlockAcqError;
	
	_ntrb_memdebug_view_no_lock();
	
	const int unlock_error = pthread_rwlock_unlock(&_ntrb_memdebug_rwlock);		
	if(unlock_error) return ntrb_memdebug_RwlockUnlockError;
	
	return ntrb_memdebug_OK;
}


void* _ntrb_memdebug_malloc(const size_t size_bytes, const char* const filename, const int line){	
	void* const ptr = malloc(size_bytes);
	if(ptr){
		assert(_ntrb_memdebug_initialized_value == _ntrb_memdebug_correct_initialized_value);
		
		const int wrlock_error = pthread_rwlock_wrlock(&_ntrb_memdebug_rwlock);
		if(wrlock_error){
			fprintf(stderr, "[Error]: %s %d: _ntrb_memdebug_malloc(): Error requesting a write lock (%d).\nThe allocated pointer will not be added to record.\n", filename, line, wrlock_error);
		}else{
			if(!_ntrb_memdebug_add_element(ptr, size_bytes, filename, line)){
				fprintf(stderr, "[Error]: %s %d: _ntrb_memdebug_malloc(): Couldn't allocate memory to add the allocation to record.\n", filename, line);				
			}
			
			const int unlock_error = pthread_rwlock_unlock(&_ntrb_memdebug_rwlock);
			if(unlock_error){
				fprintf(stderr, "[Warn]: %s %d: _ntrb_memdebug_malloc(): Error unlocking a write lock (%d).\n", filename, line, wrlock_error);
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

void _ntrb_memdebug_unresgistered_realloc_ptr_callback(const void* const realloced_ptr, const void* const requested_ptr){
	printf("[Warn]: _ntrb_memdebug_realloc(): Reallocated 0x%p from unregistered pointer 0x%p. Both pointers will not be added to the record.\n", realloced_ptr, requested_ptr);
}

void* _ntrb_memdebug_realloc(void* const ptr, const size_t size_bytes, const char* const filename, const int line, void (*unregistered_ptr_callback)(const void*, const void*)){
	void* const realloc_ptr = realloc(ptr, size_bytes);
	//failed allocation; return null, don't need to modify the record
	if(!realloc_ptr) return NULL;
		
	assert(_ntrb_memdebug_initialized_value == _ntrb_memdebug_correct_initialized_value);
	
	const int wrlock_error = pthread_rwlock_wrlock(&_ntrb_memdebug_rwlock);
	if(wrlock_error){
		fprintf(stderr, "[Error]: %s %d: _ntrb_memdebug_realloc(): Error requesting a write lock (%d).\nAccess to the memory record is unavailable\n", filename, line, wrlock_error);	
		return realloc_ptr;
	}
	
	//the original pointer was null, standard realloc would act as malloc, so we just add the allocated pointer to the record.
	if(ptr == NULL) 
		_ntrb_memdebug_add_element(realloc_ptr, size_bytes, filename, line);
	else{
		//the original pointer is either expanded, or shrunk, we cannot be certain.
		//if the original pointer is in the record, we change its record contents according to the arguments,
		//if not, we don't add the unregistered pointer to the record, rather we just warn
		const int_least64_t i_ptr = _ntrb_memdebug_ptr_index(ptr);
		const bool unregistered_original_ptr = i_ptr == -1;
		if(unregistered_original_ptr)
			unregistered_ptr_callback(realloc_ptr, ptr);
		else
			_ntrb_memdebug_replace_element(i_ptr, realloc_ptr, size_bytes, filename, line);
	}
		
	const int unlock_error = pthread_rwlock_unlock(&_ntrb_memdebug_rwlock);
	if(unlock_error){
		fprintf(stderr, "[Warn]: %s %d: _ntrb_memdebug_realloc(): Error unlocking a write lock (%d).\n", filename, line, wrlock_error);
	}
	
	return realloc_ptr;
}

void _ntrb_memdebug_free(void* const ptr, const char* const filename, const int line){
	assert(_ntrb_memdebug_initialized_value == _ntrb_memdebug_correct_initialized_value);
	
	if(ptr == NULL) return;
	
	const int wrlock_error = pthread_rwlock_wrlock(&_ntrb_memdebug_rwlock);
	if(wrlock_error){
		fprintf(stderr, "[Error]: %s %d: _ntrb_memdebug_free(): Error requesting a write lock (%d).\nThe pointer cannot be freed.\n", filename, line, wrlock_error);
		return;
	}
		
	bool ptr_not_in_record = true;
	const size_t data_count = _ntrb_memdebug_alloc_data.elements / sizeof(_ntrb_memdebug_AllocData);
	
	for(size_t i = 0; i < data_count; i++){
		const void* const it_ptr = ((_ntrb_memdebug_AllocData*)(_ntrb_memdebug_alloc_data.base_ptr))[i].ptr;
		
		const bool ptr_in_record = it_ptr == ptr;
		if(ptr_in_record){
			ptr_not_in_record = false;
			_ntrb_memdebug_remove_element(i, data_count);
			free(ptr);
			break;
		}
	}
	
	if(ptr_not_in_record)
		printf("[Warn]: %s %d: _ntrb_memdebug_free(): Prevented freeing unallocated 0x%p!\n", filename, line, ptr);

	const int unlock_error = pthread_rwlock_unlock(&_ntrb_memdebug_rwlock);
	if(unlock_error){
		fprintf(stderr, "[Warn]: %s %d: _ntrb_memdebug_free(): Error unlocking a write lock (%d).\n", filename, line, wrlock_error);
	}
	
}


void _ntrb_memdebug_view_no_lock(){
	bool printed_an_element = false;
	const size_t data_count = _ntrb_memdebug_alloc_data.elements / sizeof(_ntrb_memdebug_AllocData);
	
	for(size_t i = 0; i < data_count; i++){
		const _ntrb_memdebug_AllocData* const alloc_data = &((_ntrb_memdebug_AllocData*)(_ntrb_memdebug_alloc_data.base_ptr))[i];

		if(alloc_data->ptr){
			const size_t size = alloc_data->allocsize_bytes;
			const char* const filename = alloc_data->callsite_filename;
			const int line = alloc_data->callsite_line;			
			
			printf("i: %llu | %s line %d: 0x%p %llu bytes.\n", i, filename, line, alloc_data->ptr, size);
			printed_an_element = true;
		}
	}
	
	if(!printed_an_element)
		printf("[Info]: ntrb_memdebug_view(): no allocated memory.\n");
}


bool _ntrb_memdebug_add_element_to_unused_space(void* const ptr, const size_t size_bytes, const char* const filename, const int line){
	const size_t data_count = _ntrb_memdebug_alloc_data.elements / sizeof(_ntrb_memdebug_AllocData);
	
	for(size_t i = 0; i < data_count; i++){
		_ntrb_memdebug_AllocData* const i_alloc_data = &((_ntrb_memdebug_AllocData*)(_ntrb_memdebug_alloc_data.base_ptr))[i];
		
		const bool ptr_slot_is_empty = i_alloc_data->ptr == NULL;
		if(ptr_slot_is_empty){
			i_alloc_data->ptr = ptr;
			i_alloc_data->allocsize_bytes = size_bytes;
			i_alloc_data->callsite_filename = filename;
			i_alloc_data->callsite_line = line;
			return true;
		}
	}
	return false;
}

bool _ntrb_memdebug_add_element(void* const ptr, const size_t size_bytes, const char* const filename, const int line){
	if(!_ntrb_memdebug_add_element_to_unused_space(ptr, size_bytes, filename, line)){
		const _ntrb_memdebug_AllocData alloc_data = {.ptr = ptr, .allocsize_bytes = size_bytes, .callsite_filename = filename, .callsite_line = line};
		
		if(!_ntrb_alloc_bytevec_append(&_ntrb_memdebug_alloc_data, sizeof(_ntrb_memdebug_AllocData), &alloc_data))
			return false;
	}
	return true;
}

void _ntrb_memdebug_remove_element(const size_t i_element, const size_t element_count){
	const _ntrb_memdebug_AllocData unused_element = {
		.ptr = NULL,
		.allocsize_bytes = 0,
		.callsite_filename = NULL,
		.callsite_line = 0
	};
	
	((_ntrb_memdebug_AllocData*)(_ntrb_memdebug_alloc_data.base_ptr))[i_element] = unused_element;
	
	const bool is_last_element = (i_element+1 == element_count);
	if(is_last_element)
		_ntrb_memdebug_alloc_data.elements -= sizeof(_ntrb_memdebug_AllocData);	
}

int_least64_t _ntrb_memdebug_ptr_index(const void* const ptr){	
	const size_t data_count = _ntrb_memdebug_alloc_data.elements / sizeof(_ntrb_memdebug_AllocData);
	
	for(size_t i = 0; i < data_count; i++){
		if(((_ntrb_memdebug_AllocData*)(_ntrb_memdebug_alloc_data.base_ptr))[i].ptr == ptr) 
			return i;
	}
	
	return -1;
}


void _ntrb_memdebug_replace_element(const size_t i_element, void* const ptr, const size_t size_bytes, const char* const filename, const int line){	
	const _ntrb_memdebug_AllocData data = {
		.ptr = ptr,
		.allocsize_bytes = size_bytes,
		.callsite_filename = filename,
		.callsite_line = line
	};
	
	((_ntrb_memdebug_AllocData*)(_ntrb_memdebug_alloc_data.base_ptr))[i_element] = data;
}

#endif