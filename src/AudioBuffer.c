#include "AudioBuffer.h"

#include "alloc.h"
#include "utils.h"
#include "aud_std_fmt.h"
#include "str_utils.h"
#include "wav_wrapper.h"
#include "file_wrapper.h"

#include "SpanU8.h"
#include "AudioHeader.h"

#include <pthread.h>

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

const ntrb_AudioBuffer failed_ntrb_AudioBuffer = {
	.datapoints = NULL,
};

enum ntrb_AudioBufferNew_Error ntrb_AudioBuffer_new(ntrb_AudioBuffer* const ret, const char* const filename, const size_t monochannel_samples){
	ret->load_err = ntrb_AudioBufferLoad_OK;
	
	//Alloc ret->buffer_access, don't free with pthread_rwlock_destroy() unless an error occurred in this scope.
	const int pthread_rwlock_init_error = pthread_rwlock_init(&(ret->buffer_access), NULL);
	if(pthread_rwlock_init_error) return ntrb_AudioBufferNew_RwlockInitError;
	
	//Alloc filetype, free with ntrb_free()
	char* const filetype = ntrb_get_filetype(filename);
	if(filetype == NULL)
		return ntrb_AudioBufferNew_InvalidAudFiletype;
	
	//Alloc ret->aud_file, don't free with fclose() unless error occurred in this scope.
	ret->aud_file = fopen(filename, "rb");
	if(ret->aud_file == NULL){
		ntrb_free(filetype);
		return ntrb_AudioBufferNew_FileOpenError;
	}

	enum ntrb_LoadAudheader_status audheader_load_status = ntrb_LoadAudheader_OK;
	if(strcmp(filetype, "wav") == 0){
		ret->load_buffer_callback = load_wav_buffer;
		audheader_load_status = load_wav_header(ret);
	}
	else if(strcmp(filetype, "flac") == 0){
		ret->load_buffer_callback = load_flac_buffer;		
	}
	else{
		fclose(ret->aud_file);
		ntrb_free(filetype);
		return ntrb_AudioBufferNew_InvalidAudFiletype;
	}
	
	//Free filetype
	ntrb_free(filetype);
	
	if(audheader_load_status != ntrb_LoadAudheader_OK){
		fclose(ret->aud_file);
		
		switch(audheader_load_status){
			case ntrb_LoadAudheader_FileError:
			return ntrb_AudioBufferNew_FileReadError;
			
			case ntrb_LoadAudheader_WAVHeaderConversionError:
			return ntrb_AudioBufferNew_WAVheaderError;
			
			default:
			break;
		}
	}
	
	ret->monochannel_samples = monochannel_samples;
	const size_t stereo_samples = monochannel_samples * 2;
	ret->datapoints = ntrb_calloc(stereo_samples, sizeof(float));
	if(ret->datapoints == NULL){
		fclose(ret->aud_file);
		return ntrb_AudioBufferNew_AllocError;
	}

	const float unprocessed_samplerate_over_stdaud_samplerate = (float)(ret->aud_header.SampleRate) / (float)ntrb_std_samplerate;
	
	const size_t unprocessed_samples = unprocessed_samplerate_over_stdaud_samplerate * (float)monochannel_samples * (float)(ret->aud_header.NumChannels);
	ret->unprocessed_datapoints = ntrb_calloc(unprocessed_samples, (ret->aud_header.BitsPerSample / 8));
	
	if(ret->unprocessed_datapoints == NULL){
		ntrb_free(ret->datapoints);
		fclose(ret->aud_file);
		return ntrb_AudioBufferNew_AllocError;
	}	
	
	return ntrb_AudioBufferNew_OK;
}

enum ntrb_AudioBufferFree_Error ntrb_AudioBuffer_free(ntrb_AudioBuffer* const obj){
	const int acq_writelock_error = pthread_rwlock_wrlock(&(obj->buffer_access));
	if(acq_writelock_error) return ntrb_AudioBufferFree_AcqWritelockError;
		
	ntrb_free(obj->datapoints);
	ntrb_free(obj->unprocessed_datapoints);
	
	fclose(obj->aud_file);
	obj->monochannel_samples = 0;
	
	const int unlock_rwlock_error = pthread_rwlock_unlock(&(obj->buffer_access));
	if(unlock_rwlock_error) return ntrb_AudioBufferFree_RwlockUnlockError;
		
	const int destroy_rwlock_error = pthread_rwlock_destroy(&(obj->buffer_access));
	if(destroy_rwlock_error) return ntrb_AudioBufferFree_RwlockDestroyError;
	
	return ntrb_AudioBufferFree_OK;
}

enum ntrb_LoadAudheader_status load_wav_header(void* const void_ntrb_AudioBuffer){
	ntrb_AudioBuffer* const aud = (ntrb_AudioBuffer*)void_ntrb_AudioBuffer;
	ntrb_SpanU8 file_buffer;
	
	const size_t max_header_boundary_bytes = 65535;
	const enum ntrb_ReadFileResult read_file_result = ntrb_readsome_from_file_rb(&file_buffer, aud->aud_file, max_header_boundary_bytes);	
	if(read_file_result != ntrb_ReadFileResult_OK) return ntrb_LoadAudheader_FileError;
		
	const enum ntrb_AudioHeaderFromWAVFileStatus wav_header_status = ntrb_AudioHeader_from_WAVfile_2(&(aud->aud_header), &(aud->aud_header.audiodataOffset), &(aud->aud_header.audiodataSize), file_buffer);
	ntrb_free(file_buffer.ptr);
	
	if(fseek(aud->aud_file, aud->aud_header.audiodataOffset, SEEK_SET) != 0){
		return ntrb_LoadAudheader_FileError;
	}	
	
	if(wav_header_status != ntrb_AudioHeaderFromWAVFile_ok)
		return ntrb_LoadAudheader_WAVHeaderConversionError;	
	return ntrb_LoadAudheader_OK;
}

void load_flac_header(void* const void_ntrb_AudioBuffer){
}

void* load_wav_buffer(void* const void_ntrb_AudioBuffer){
	ntrb_AudioBuffer* const audbuf = void_ntrb_AudioBuffer;
	const int acq_writelock_error = pthread_rwlock_wrlock(&(audbuf->buffer_access));
	if(acq_writelock_error){
		audbuf->load_err = ntrb_AudioBufferLoad_AcqWritelockError;
		return NULL;
	}
		
	const float unprocessed_samplerate_over_stdaud_samplerate = (float)(audbuf->aud_header.SampleRate) / (float)ntrb_std_samplerate;
	const float unprocessed_aud_bytes_to_read_float = (float)(audbuf->monochannel_samples) * unprocessed_samplerate_over_stdaud_samplerate * (float)(audbuf->aud_header.NumChannels) * (float)(audbuf->aud_header.BitsPerSample / 8);
	const size_t unprocessed_aud_bytes_to_read = floor(unprocessed_aud_bytes_to_read_float);
	memset(audbuf->unprocessed_datapoints, 0, unprocessed_aud_bytes_to_read);
	
	const size_t clamped_bytes_to_read = ntrb_clamp_u64(unprocessed_aud_bytes_to_read, 0, audbuf->aud_header.audiodataSize);
	
	const size_t bytes_read = fread(audbuf->unprocessed_datapoints, sizeof(uint8_t), clamped_bytes_to_read, audbuf->aud_file);
	if(bytes_read != clamped_bytes_to_read){
		audbuf->load_err = ntrb_AudioBufferLoad_UnequalRead;
		pthread_rwlock_unlock(&(audbuf->buffer_access));
		return NULL;
	}
		
	//No overflows from subtraction, the clamp is always lesser or equal to audiodataSize.
	audbuf->aud_header.audiodataSize -= clamped_bytes_to_read;
	if(audbuf->aud_header.audiodataSize == 0){
		audbuf->load_err = ntrb_AudioBufferLoad_EOF;
		pthread_rwlock_unlock(&(audbuf->buffer_access));		
		return NULL;
	}
	
	ntrb_AudioDatapoints stdaud;
	const ntrb_AudioDatapoints unprocessed_aud = {.bytes = audbuf->unprocessed_datapoints, .byte_pos = 0, .byte_count = bytes_read};
	const enum ntrb_StdAudFmtConversionResult stdaud_conversion_error = ntrb_to_standard_format(&stdaud, unprocessed_aud, &(audbuf->aud_header));
	if(stdaud_conversion_error){
		audbuf->load_err = ntrb_AudioBufferLoad_StdaudConversionError;
		return NULL;
	}
		
	memcpy(audbuf->datapoints, stdaud.bytes, stdaud.byte_count);
	ntrb_AudioDatapoints_free(&stdaud);
	
	const int unlock_rwlock_error = pthread_rwlock_unlock(&(audbuf->buffer_access));
	if(unlock_rwlock_error){
		audbuf->load_err = ntrb_AudioBufferLoad_EOF;
		return NULL;
	}
	
	audbuf->load_err = ntrb_AudioBufferLoad_OK;
	return NULL;
}

void* load_flac_buffer(void* const void_ntrb_AudioBuffer){
	return NULL;
}
