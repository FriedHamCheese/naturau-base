#include "decode_flac.h"

#include "AudioHeader.h"
#include "bytevec.h"

#include "aud_std_fmt.h"

#include "FLAC/stream_decoder.h"
#include "portaudio.h"

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

static void read_metadata(const FLAC__StreamDecoder*, const FLAC__StreamMetadata *metadata, void *client_data){
	if(metadata->type != FLAC__METADATA_TYPE_STREAMINFO) return;
	
	ntrb_AudioDataFLAC* const aud_data = client_data;
	
	aud_data->header.SampleRate = metadata->data.stream_info.sample_rate;
	aud_data->header.NumChannels = metadata->data.stream_info.channels;
	aud_data->header.BitsPerSample = metadata->data.stream_info.bits_per_sample;
	
	//FLAC only uses signed int from 4 to 32 bits
	switch(aud_data->header.BitsPerSample){
		case 16:
			aud_data->header.AudioFormat = paInt16;
			break;
		default:
			aud_data->header.AudioFormat = paCustomFormat;
	}
}

static FLAC__StreamDecoderWriteStatus decode_buffer(const FLAC__StreamDecoder*, const FLAC__Frame *frame, const FLAC__int32 *const buffer[], void *client_data){
	ntrb_AudioDataFLAC* const aud_data = client_data;
	//check enum which may be set by error_handler, in case an error happened.
	if(aud_data->_decoder_error != ntrb_FLAC_decode_OK)
		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
	
	const uint8_t bits_per_byte = 8;
	const uint8_t bytes_per_sample = aud_data->header.BitsPerSample / bits_per_byte;
	
	const uint32_t samples_per_channel = frame->header.blocksize;
	const uint32_t next_total_bytes = aud_data->header.NumChannels * samples_per_channel * bytes_per_sample;
	
	while(aud_data->datapoints.elements + next_total_bytes >= aud_data->datapoints.capacity){
		if(!ntrb_bytevec_reserve(&(aud_data->datapoints), aud_data->datapoints.capacity)){
			aud_data->_decoder_error =ntrb_FLAC_decode_FLAC__StreamDecoderState +  FLAC__STREAM_DECODER_MEMORY_ALLOCATION_ERROR;
			return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
		}
	}
	
	//this is not so cache friendly, but should be fine
	for(uint32_t i = 0; i < samples_per_channel; i++){
		for(uint8_t channel = 0; channel < aud_data->header.NumChannels; channel++){
			*(int16_t*)(aud_data->datapoints.base_ptr + aud_data->datapoints.elements) = buffer[channel][i];
			aud_data->datapoints.elements += bytes_per_sample;
		}
	}
	
	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

static void error_handler(const FLAC__StreamDecoder *, FLAC__StreamDecoderErrorStatus status, void* client_data){
	((ntrb_AudioDataFLAC*)client_data)->_decoder_error = ntrb_FLAC_decode_FLAC__StreamDecoderErrorStatus + status;
}

enum ntrb_FLAC_decode_status ntrb_decode_FLAC_file(const char* const filename, ntrb_AudioDataFLAC* const aud_data) {
	aud_data->_decoder_error = ntrb_FLAC_decode_OK;
	//this pairs with FLAC__stream_decoder_delete();
	FLAC__StreamDecoder* decoder = FLAC__stream_decoder_new();
	if(decoder == NULL) 
		return ntrb_FLAC_decode_FLAC__StreamDecoderInitStatus + FLAC__STREAM_DECODER_INIT_STATUS_MEMORY_ALLOCATION_ERROR;
	
	FLAC__stream_decoder_set_md5_checking(decoder, true);
	
	//this pairs with FLAC__stream_decoder_finish();	
	FLAC__StreamDecoderInitStatus decoder_init_status = FLAC__stream_decoder_init_file(decoder, filename, decode_buffer, read_metadata, error_handler, aud_data);
	
	if(decoder_init_status != FLAC__STREAM_DECODER_INIT_STATUS_OK){
		FLAC__stream_decoder_delete(decoder);
		return ntrb_FLAC_decode_FLAC__StreamDecoderInitStatus + decoder_init_status;
	}
	
	if(!FLAC__stream_decoder_process_until_end_of_metadata(decoder)){
		const FLAC__StreamDecoderState decoder_state = FLAC__stream_decoder_get_state(decoder);
		FLAC__stream_decoder_finish(decoder);
		FLAC__stream_decoder_delete(decoder);
		
		return ntrb_FLAC_decode_FLAC__StreamDecoderState + decoder_state;
	}	
	
	if(aud_data->header.AudioFormat == paCustomFormat){
		FLAC__stream_decoder_finish(decoder);	
		FLAC__stream_decoder_delete(decoder);		
		return ntrb_FLAC_decode_unsupported_bps;
	}
	if(aud_data->header.NumChannels > ntrb_std_audchannels){
		FLAC__stream_decoder_finish(decoder);		
		FLAC__stream_decoder_delete(decoder);	
		return ntrb_FLAC_decode_unsupported_channelcount;
	}
	
	const size_t stdaud_for_5_secs = 5 * ntrb_std_samplerate * ntrb_std_audchannels * sizeof(int16_t);
	aud_data->datapoints = ntrb_bytevec_new(stdaud_for_5_secs);
	if(aud_data->datapoints.base_ptr == NULL)
		return ntrb_FLAC_decode_FLAC__StreamDecoderInitStatus + FLAC__STREAM_DECODER_INIT_STATUS_MEMORY_ALLOCATION_ERROR;
	
	if(!FLAC__stream_decoder_process_until_end_of_stream(decoder)){
		const FLAC__StreamDecoderState decoder_state = FLAC__stream_decoder_get_state(decoder);	
		FLAC__stream_decoder_finish(decoder);
		FLAC__stream_decoder_delete(decoder);			
		
		if(decoder_state == FLAC__STREAM_DECODER_ABORTED)
			return aud_data->_decoder_error;
		else
			return ntrb_FLAC_decode_FLAC__StreamDecoderState + decoder_state;			
	}

	//this pairs with FLAC__stream_decoder_init_file();
	const bool md5_equal = FLAC__stream_decoder_finish(decoder);
	//this pairs with FLAC__stream_decoder_new();	
	FLAC__stream_decoder_delete(decoder);
	
	//we used the internal _decode_status because if the error still made it out, report the error that caused it in the first place.
	if(aud_data->_decoder_error != ntrb_FLAC_decode_OK)
		return aud_data->_decoder_error;
	
	return md5_equal ? ntrb_FLAC_decode_OK : ntrb_FLAC_decode_MD5_not_equal;
}