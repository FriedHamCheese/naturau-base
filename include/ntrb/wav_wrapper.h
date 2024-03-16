#ifndef ntrb_wav_wrapper_h
#define ntrb_wav_wrapper_h

#include "SpanU8.h"
#include "AudioHeader.h"
#include "AudioDatapoints.h"

#include <stdio.h>

PaSampleFormat ntrb_WAV_PaSampleFormat(const uint16_t WAV_audioFormat, const uint16_t BitsPerSample);

enum ntrb_AudioHeaderFromWAVFileStatus{
	ntrb_AudioHeaderFromWAVFile_ok,
	ntrb_AudioHeaderFromWAVFile_buffer_too_small,
	ntrb_AudioHeaderFromWAVFile_invalid_RIFF_ID,
	ntrb_AudioHeaderFromWAVFile_invalid_WAVE_ID,
	ntrb_AudioHeaderFromWAVFile_invalid_fmt_ID,
	ntrb_AudioHeaderFromWAVFile_invalid_data_ID,
	ntrb_AudioHeaderFromWAVFile_invalid_chunk_size,
	ntrb_AudioHeaderFromWAVFile_invalid_Subchunk1Size,
	ntrb_AudioHeaderFromWAVFile_invalid_Subchunk2Size,	
};


enum ntrb_AudioHeaderFromWAVFileStatus WAVfile_to_ntrb_AudioHeader(ntrb_AudioHeader* const return_arg, size_t* const audiodata_offset, size_t* const audiodata_bytes, const ntrb_SpanU8 file_buffer);

ntrb_AudioDatapoints ntrb_get_WAV_audiodata(const ntrb_SpanU8 wavfile, const size_t audiodata_size, const size_t audiodata_offset);
#endif