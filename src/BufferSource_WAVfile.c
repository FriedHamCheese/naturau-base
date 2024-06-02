#include "BufferSource_WAVfile.h"

#include "SpanU8.h"
#include "AudioBuffer.h"
#include "AudioDatapoints.h"

#include "alloc.h"
#include "utils.h"
#include "aud_std_fmt.h"
#include "wav_wrapper.h"
#include "file_wrapper.h"

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

int ntrb_BufferSource_WAVfile_new(ntrb_BufferSource_WAVfile* const ret, const char* const filename, const size_t frame_count){
	//Alloc ret->aud_file, don't free with fclose() unless error occurred in this scope.
	ret->aud_file = fopen(filename, "rb");
	if(ret->aud_file == NULL) return ntrb_AudioBufferNew_FileOpenError;
	
	const enum ntrb_AudioBufferNew_Error audheader_load_status = ntrb_BufferSource_WAVfile_load_header(ret);
	if(audheader_load_status != ntrb_AudioBufferNew_OK){
		fclose(ret->aud_file);
		return audheader_load_status;
	}
	
	const float unprocessed_samplerate_over_stdaud_samplerate = (float)(ret->aud_header.SampleRate) / (float)ntrb_std_samplerate;
	const float samples_to_read = unprocessed_samplerate_over_stdaud_samplerate * (float)frame_count * (float)(ret->aud_header.NumChannels);
	
	const size_t bytes_to_read = floor(samples_to_read) * (ret->aud_header.BitsPerSample / 8);
	ret->bytes_to_read = bytes_to_read;
	
	ret->read_bytes = ntrb_calloc(bytes_to_read, sizeof(uint8_t));
	
	if(ret->read_bytes == NULL){
		fclose(ret->aud_file);
		return ntrb_AudioBufferNew_AllocError;
	}
	
	return ntrb_AudioBufferNew_OK;
}

int ntrb_BufferSource_WAVfile_free(ntrb_BufferSource_WAVfile* const ret){
	ntrb_free(ret->read_bytes);
	return fclose(ret->aud_file);
}

int ntrb_BufferSource_WAVfile_load_header(ntrb_BufferSource_WAVfile* const ret){
	ntrb_SpanU8 file_buffer;
	
	const size_t max_header_boundary_bytes = 65535;
	const enum ntrb_ReadFileResult read_file_result = ntrb_readsome_from_file_rb(&file_buffer, ret->aud_file, max_header_boundary_bytes);
	if(read_file_result != ntrb_ReadFileResult_OK) return ntrb_AudioBufferNew_FileReadError;
	
	size_t audiodataOffset = 0;
	const enum ntrb_AudioHeaderFromWAVFileStatus wav_header_status = ntrb_AudioHeader_from_WAVfile_2(&(ret->aud_header), &(audiodataOffset), &(ret->audiodataSize), file_buffer);
	ntrb_free(file_buffer.ptr);
	
	if(fseek(ret->aud_file, audiodataOffset, SEEK_SET) != 0){
		return ntrb_AudioBufferNew_FileReadError;
	}
	
	if(wav_header_status != ntrb_AudioHeaderFromWAVFile_ok)
		return ntrb_AudioBufferNew_WAVheaderError;	
	
	return ntrb_AudioBufferNew_OK;	
}

void* ntrb_BufferSource_WAVfile_load_buffer(void* const void_ntrb_AudioBuffer){
	ntrb_AudioBuffer* const audbuf = void_ntrb_AudioBuffer;
	ntrb_BufferSource_WAVfile* const wav_source = &(audbuf->source.wav_file);
	
	const int acq_writelock_error = pthread_rwlock_wrlock(&(audbuf->buffer_access));
	if(acq_writelock_error){
		audbuf->load_err = ntrb_AudioBufferLoad_AcqWritelockError;
		return NULL;
	}

	memset(audbuf->datapoints, 0, audbuf->monochannel_samples * ntrb_std_audchannels * sizeof(float));
	
	if(wav_source->audiodataSize == 0){
		audbuf->load_err = ntrb_AudioBufferLoad_EOF;
		pthread_rwlock_unlock(&(audbuf->buffer_access));	
		return NULL;
	}
	
	const size_t clamped_bytes_to_read = ntrb_clamp_u64(wav_source->bytes_to_read, 0, wav_source->audiodataSize);
	
	const size_t bytes_read = fread(wav_source->read_bytes, sizeof(uint8_t), clamped_bytes_to_read, wav_source->aud_file);
	if(bytes_read != clamped_bytes_to_read){
		audbuf->load_err = ntrb_AudioBufferLoad_UnequalRead;
		pthread_rwlock_unlock(&(audbuf->buffer_access));
		return NULL;
	}
		
	//No overflows from subtraction, the clamp is always lesser or equal to audiodataSize.
	wav_source->audiodataSize -= clamped_bytes_to_read;
	
	ntrb_AudioDatapoints stdaud;
	const ntrb_AudioDatapoints unprocessed_aud = {.bytes = wav_source->read_bytes, .byte_pos = 0, .byte_count = bytes_read};
	const enum ntrb_StdAudFmtConversionResult stdaud_conversion_error = ntrb_to_standard_format(&stdaud, unprocessed_aud, &(wav_source->aud_header));
	if(stdaud_conversion_error){
		audbuf->load_err = ntrb_AudioBufferLoad_StdaudConversionError;
		return NULL;
	}
		
	memcpy(audbuf->datapoints, stdaud.bytes, stdaud.byte_count);
	ntrb_AudioDatapoints_free(&stdaud);
	
	const int unlock_rwlock_error = pthread_rwlock_unlock(&(audbuf->buffer_access));
	if(unlock_rwlock_error){
		audbuf->load_err = ntrb_AudioBufferLoad_RwlockUnlockError;
		return NULL;
	}
	
	audbuf->load_err = ntrb_AudioBufferLoad_OK;
	return NULL;	
}