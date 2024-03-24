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

size_t ntrb_getSubchunk1Start(const ntrb_SpanU8 file_buffer, const size_t RIFFchunkEnd);
size_t ntrb_getSubchunk2Start(const ntrb_SpanU8 file_buffer, const size_t Subchunk1End);

enum ntrb_AudioHeaderFromWAVFileStatus ntrb_AudioHeader_from_WAVfile(ntrb_AudioHeader* const returnArg, size_t* const audiodataOffset, size_t* const audiodataSize, const ntrb_SpanU8 fileBuffer);

ntrb_AudioDatapoints ntrb_get_WAV_audiodata(const ntrb_SpanU8 wavfile, const size_t audiodata_size, const size_t audiodata_offset);
#endif