#ifndef ntrb_wav_wrapper_h
#define ntrb_wav_wrapper_h

#include "SpanU8.h"
#include "WAVheader.h"
#include "AudioHeader.h"
#include "AudioDatapoints.h"

#include <stdio.h>

enum ntrb_GetWAVheaderStatus{
	ntrb_GetWAVheader_ok,
	ntrb_GetWAVheader_buffer_too_small,
	ntrb_GetWAVheader_invalid_RIFF_ID,
	ntrb_GetWAVheader_invalid_WAVE_ID,
	ntrb_GetWAVheader_invalid_fmt_ID,
	ntrb_GetWAVheader_invalid_data_ID,
	ntrb_GetWAVheader_invalid_chunk_size,
	ntrb_GetWAVheader_invalid_Subchunk1Size,
	ntrb_GetWAVheader_invalid_Subchunk2Size,	
};

enum ntrb_GetWAVheaderStatus get_ntrb_WAVheader(ntrb_WAVheader* const return_arg, size_t* const audiodata_offset, const ntrb_SpanU8 file_buffer);
void print_ntrb_WAVheader(const ntrb_WAVheader* const header, FILE* const printstream);

ntrb_AudioDatapoints ntrb_get_WAV_audiodata(const ntrb_WAVheader* const header, const ntrb_SpanU8 wavfile, const size_t audiodata_offset);

float ntrb_WAVheader_audio_msec(const ntrb_WAVheader* const header);

ntrb_AudioHeader ntrb_WAVheader_to_ntrb_AudioHeader(const ntrb_WAVheader* const wavheader);
enum ntrb_GetWAVheaderStatus WAVfile_to_ntrb_AudioHeader(ntrb_AudioHeader* const ret, size_t* const audiodata_offset, const ntrb_SpanU8 file_buffer);
ntrb_AudioDatapoints ntrb_get_WAV_audiodata_ntrb_AudioHeader(const ntrb_AudioHeader header, const ntrb_SpanU8 wavfile, const size_t audiodata_offset);
PaSampleFormat ntrb_WAV_PaSampleFormat(const ntrb_WAVheader* const header);
#endif