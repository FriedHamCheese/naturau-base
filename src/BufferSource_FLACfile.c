/*
Copyright 2024 Pawikan Boonnaum

This file is part of naturau-base/ntrb.

naturau-base/ntrb is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

naturau-base/ntrb is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with naturau-base/ntrb. If not, see <https://www.gnu.org/licenses/>.
*/

#include "BufferSource_FLACfile.h"

#include "alloc.h"
#include "aud_std_fmt.h"
#include "AudioBuffer.h"
#include "AudioDatapoints.h"

#include "FLAC/stream_decoder.h"

#include <math.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>

static FLAC__StreamDecoderWriteStatus read_FLAC_frame(const FLAC__StreamDecoder*, const FLAC__Frame *frame, const FLAC__int32 *const buffer[], void *void_ntrb_AudioBuffer){
	/*
	This function iterates over each of the audio frames in an FLAC frame.
	Every iteration first checks if the buffer in ntrb_BufferSource_FLACfile is full.
	If it is: exit out of the function. ntrb_BufferSource_FLACfile_load_buffer() will then process the buffer of the ntrb_BufferSource_FLACfile and writes it to ntrb_AudioBuffer.
	If it isn't: keep filling the buffer of ntrb_BufferSource_FLACfile until it is full, as well as keep track of the audio frame of the FLAC file.
	  ntrb_BufferSource_FLACfile_load_buffer() needs the audio frame value to jump back to the unfinished frame.
	*/
	
	ntrb_AudioBuffer* const aud = void_ntrb_AudioBuffer;

	for(size_t i_monochannel_sample = 0; i_monochannel_sample < frame->header.blocksize; i_monochannel_sample++){
		if(aud->source.flac_file.bytes_in_buffer >= aud->source.flac_file.buffersize_bytes)
			return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
		
		for(size_t i_channel = 0; i_channel < frame->header.channels; i_channel++){
			const size_t write_at_sample = (aud->source.flac_file.bytes_in_buffer / sizeof(uint16_t)) + i_channel;
			((uint16_t*)(aud->source.flac_file.read_bytes))[write_at_sample] = buffer[i_channel][i_monochannel_sample];
		}
		
		aud->source.flac_file.current_frame++;
		aud->source.flac_file.bytes_in_buffer += sizeof(uint16_t) * frame->header.channels;
	}
	
	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

static void read_metadata(const FLAC__StreamDecoder*, const FLAC__StreamMetadata * metadata, void * void_ntrb_AudioBuffer){
	if(metadata->type != FLAC__METADATA_TYPE_STREAMINFO) return;
	
	ntrb_BufferSource_FLACfile* const aud_data = &(((ntrb_AudioBuffer*)(void_ntrb_AudioBuffer))->source.flac_file);
	
	aud_data->aud_header.SampleRate = metadata->data.stream_info.sample_rate;
	aud_data->aud_header.NumChannels = metadata->data.stream_info.channels;
	aud_data->aud_header.BitsPerSample = metadata->data.stream_info.bits_per_sample;
	
	//FLAC only uses signed int from 4 to 32 bits
	switch(aud_data->aud_header.BitsPerSample){
		case 16:
			aud_data->aud_header.AudioFormat = paInt16;
			break;
		default:
			aud_data->aud_header.AudioFormat = paCustomFormat;
	}
}

static void error_callback(const FLAC__StreamDecoder*, FLAC__StreamDecoderErrorStatus, void *void_ntrb_AudioBuffer){
	//4/5 of the errors are related to bad data, so we'll just generalise it.
	((ntrb_AudioBuffer*)void_ntrb_AudioBuffer)->load_err = ntrb_AudioBufferLoad_FLACBadData;
}

int ntrb_BufferSource_FLACfile_new(void* const void_ntrb_AudioBuffer, const char* const filename, const size_t frame_count){
	ntrb_AudioBuffer* const aud = void_ntrb_AudioBuffer;
	//So we can use ntrb_BufferSource_FLACfile_free() without freeing an invalid memory address.
	aud->source.flac_file.read_bytes = NULL;
	
	aud->source.flac_file.decoder = FLAC__stream_decoder_new();
	if(aud->source.flac_file.decoder == NULL) return ntrb_AudioBufferNew_AllocError;
		
	const FLAC__StreamDecoderInitStatus decoder_init_error = FLAC__stream_decoder_init_file(aud->source.flac_file.decoder, filename, read_FLAC_frame, read_metadata, error_callback, void_ntrb_AudioBuffer);
	
	if(decoder_init_error){
		FLAC__stream_decoder_delete(aud->source.flac_file.decoder);
		
		switch(decoder_init_error){
			case FLAC__STREAM_DECODER_INIT_STATUS_UNSUPPORTED_CONTAINER:
			return ntrb_AudioBufferNew_FLACcontainerError;
			case FLAC__STREAM_DECODER_INIT_STATUS_MEMORY_ALLOCATION_ERROR: 
			return ntrb_AudioBufferNew_AllocError;
			case FLAC__STREAM_DECODER_INIT_STATUS_ERROR_OPENING_FILE:
			return ntrb_AudioBufferNew_FileOpenError;
			default:
			return ntrb_AudioBufferNew_UnknownError; 
		}
	}
	
	const bool metadata_read_error = !FLAC__stream_decoder_process_until_end_of_metadata(aud->source.flac_file.decoder);
	if(metadata_read_error) {
		ntrb_BufferSource_FLACfile_free(&(aud->source.flac_file));
		return ntrb_AudioBufferNew_UnknownError;
	}
	
	if(aud->source.flac_file.aud_header.AudioFormat == paCustomFormat || aud->source.flac_file.aud_header.NumChannels > 2){
		ntrb_BufferSource_FLACfile_free(&(aud->source.flac_file));
		return ntrb_AudioBufferNew_InvalidAudFormat;
	}
	
	const float unprocessed_samplerate_over_stdaud_samplerate = (float)(aud->source.flac_file.aud_header.SampleRate) / (float)ntrb_std_samplerate;
	
	const float samples_to_read = unprocessed_samplerate_over_stdaud_samplerate * (float)frame_count * (float)aud->source.flac_file.aud_header.NumChannels;	
	
	const size_t buffersize_bytes = ceil(samples_to_read) * (aud->source.flac_file.aud_header.BitsPerSample / 8);
	
	aud->source.flac_file.read_bytes = ntrb_calloc(buffersize_bytes, sizeof(uint8_t));
	if(aud->source.flac_file.read_bytes == NULL){
		ntrb_BufferSource_FLACfile_free(&(aud->source.flac_file));		
		return ntrb_AudioBufferNew_AllocError;
	}
	
	aud->source.flac_file.current_frame = 0;
	aud->source.flac_file.buffersize_bytes = buffersize_bytes;
	aud->source.flac_file.bytes_in_buffer = 0;
	
	return ntrb_AudioBufferNew_OK;
}

void ntrb_BufferSource_FLACfile_free(ntrb_BufferSource_FLACfile* const ret){
	ntrb_free(ret->read_bytes);
	FLAC__stream_decoder_finish(ret->decoder);
	FLAC__stream_decoder_delete(ret->decoder);
}

void* ntrb_BufferSource_FLACfile_load_buffer(void* const void_ntrb_AudioBuffer){
	ntrb_AudioBuffer* const aud = void_ntrb_AudioBuffer;

	const int acq_writelock_error = pthread_rwlock_wrlock(&(aud->buffer_access));
	if(acq_writelock_error){
		aud->load_err = ntrb_AudioBufferLoad_AcqWritelockError;
		return NULL;
	}
	
	memset(aud->datapoints, 0, aud->monochannel_samples * ntrb_std_audchannels * sizeof(float));
	
	//This actually calls read_FLAC_frame().
	//We seek the decoder to the ntrb_BufferSource_FLACfile.current_frame, which will call the callback which fills the buffer.
	FLAC__stream_decoder_seek_absolute(aud->source.flac_file.decoder, aud->source.flac_file.current_frame);
	aud->load_err = FLAC__StreamDecoderState_to_ntrb_AudioBufferLoad_Error(FLAC__stream_decoder_get_state(aud->source.flac_file.decoder));

	//And we keep calling the callback until the ntrb_BufferSource_FLACfile.read_bytes is fully filled.
	while(aud->source.flac_file.bytes_in_buffer < aud->source.flac_file.buffersize_bytes && !(aud->load_err)){
		FLAC__stream_decoder_process_single(aud->source.flac_file.decoder);
		aud->load_err = FLAC__StreamDecoderState_to_ntrb_AudioBufferLoad_Error(FLAC__stream_decoder_get_state(aud->source.flac_file.decoder));
	}

	ntrb_AudioDatapoints stdaud;
	const ntrb_AudioDatapoints unprocessed_aud = {.bytes = aud->source.flac_file.read_bytes, .byte_pos = 0, .byte_count = aud->source.flac_file.bytes_in_buffer};
	const enum ntrb_StdAudFmtConversionResult stdaud_conversion_error = ntrb_to_standard_format(&stdaud, unprocessed_aud, &(aud->source.flac_file.aud_header));
	if(stdaud_conversion_error){
		aud->load_err = ntrb_AudioBufferLoad_StdaudConversionError;
		pthread_rwlock_unlock(&(aud->buffer_access));		
		return NULL;
	}
	
	memcpy(aud->datapoints, stdaud.bytes, stdaud.byte_count);
	ntrb_AudioDatapoints_free(&stdaud);	

	//Once the buffer is guaranteed to be filled, we set the bytes_in_buffer to 0,
	//indicating the buffer is ready to be filled at its start for the next call of this function.
	//While the decoder's output may not be fully read, we keep track of the last decoded monochannel sample we've read in ->current_frame.
	//Next time, even if we may or may not have fully read the output, we seek the decoder to the ->current_frame.
	aud->source.flac_file.bytes_in_buffer = 0;
	
	const int unlock_rwlock_error = pthread_rwlock_unlock(&(aud->buffer_access));	
	if(unlock_rwlock_error){
		aud->load_err = ntrb_AudioBufferLoad_RwlockUnlockError;
		return NULL;
	}

	aud->load_err = ntrb_AudioBufferLoad_OK;	
	return NULL;
}

enum ntrb_AudioBufferLoad_Error FLAC__StreamDecoderState_to_ntrb_AudioBufferLoad_Error(const FLAC__StreamDecoderState arg){
	switch(arg){
		case FLAC__STREAM_DECODER_END_OF_STREAM:	return ntrb_AudioBufferLoad_EOF;
		case FLAC__STREAM_DECODER_OGG_ERROR:		return ntrb_AudioBufferLoad_OGGError;
		case FLAC__STREAM_DECODER_SEEK_ERROR:		return ntrb_AudioBufferLoad_SeekError;
		case FLAC__STREAM_DECODER_ABORTED:			return ntrb_AudioBufferLoad_Aborted;
		case FLAC__STREAM_DECODER_MEMORY_ALLOCATION_ERROR:
			return ntrb_AudioBufferLoad_AllocError;
		default:
			return ntrb_AudioBufferLoad_OK;
	}
}