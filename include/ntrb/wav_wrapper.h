#ifndef ntrb_wav_wrapper_h
#define ntrb_wav_wrapper_h

/*
Copyright 2024 Pawikan Boonnaum

This file is part of naturau-base/ntrb.

naturau-base/ntrb is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

naturau-base/ntrb is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with naturau-base/ntrb. If not, see <https://www.gnu.org/licenses/>.
*/


/**
\file wav_wrapper.h
A module for reading WAV files.
*/

#include "export.h"
#include "SpanU8.h"
#include "AudioHeader.h"
#include "AudioDatapoints.h"

#include <stdio.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"{	
#endif

/**
Returns the PaSampleFormat equivalent from the two provided arguments.
If there is no equivalent, it returns paCustomFormat, indicating an unknown format.

Currently it supports paInt16 (1, 16) and paFloat32 (3, 32): the most common formats used in WAV files.

\param WAV_audioFormat A 2 byte value after 8 bytes from the 'f' in "fmt " which indicates the format type of the sample.
\param BitsPerSample A 2 byte value after 22 bytes from the 'f' in "fmt ", indicates how many bits are in a sample.
*/
NTRB_DLL_VISIBILITY PaSampleFormat ntrb_WAV_PaSampleFormat(const uint16_t WAV_audioFormat, const uint16_t BitsPerSample);

/**
Iterates over each byte of file_buffer starting from RIFFchunkEnd to file_buffer.elem - 1
and returns the offset of Subchunk1 (indicated by the "fmt ") from file_buffer.ptr.

If no "fmt " is found in file_buffer, it returns 0, which is impossible for a valid WAV file to have it at that index.

RIFFchunkEnd is always 12 bytes from file_buffer.ptr, so you'd just pass that in.

This is used in ntrb_AudioHeader_from_WAVfile().
*/
NTRB_DLL_VISIBILITY size_t ntrb_getSubchunk1Start(const ntrb_SpanU8 file_buffer, const size_t RIFFchunkEnd);

/**
Iterates over each byte of file_buffer starting from Subchunk1End to file_buffer.elem - 1
and returns the offset of subchunk2 (indicated by the "data") from file_buffer.ptr.

If no "data" is found in file_buffer, it returns 0, which is impossible for a valid WAV file to have it at that index.

Pass the offset of 'f' in "fmt " added with 24 (bytes), that is the minimum offset of Subchunk2Start from Subchunk1.
There may be extra data after 24 bytes which will be skipped, but these aren't important to us. We only need the "data" indicator.

This is used in ntrb_AudioHeader_from_WAVfile().
*/
NTRB_DLL_VISIBILITY size_t ntrb_getSubchunk2Start(const ntrb_SpanU8 file_buffer, const size_t Subchunk1End);

/**
An enum for possible errors while reading the WAV header file and converting to ntrb_AudioHeader.
*/
enum ntrb_AudioHeaderFromWAVFileStatus{
	ntrb_AudioHeaderFromWAVFile_ok,						///< nice!
	ntrb_AudioHeaderFromWAVFile_buffer_too_small,		///< The size of the file buffer is too small for any valid WAV header to be in.
	ntrb_AudioHeaderFromWAVFile_invalid_RIFF_ID,		///< Couldn't find the "RIFF" text.
	ntrb_AudioHeaderFromWAVFile_invalid_WAVE_ID,		///< Couldn't find the "WAVE" text.
	ntrb_AudioHeaderFromWAVFile_invalid_fmt_ID,			///< Couldn't find the "fmt " text which indicates Subchunk1.
	ntrb_AudioHeaderFromWAVFile_invalid_data_ID,		///< Couldn't find the "data" text which indicates Subchunk2.
	ntrb_AudioHeaderFromWAVFile_invalid_chunk_size,		///< Chunk size in the header exceeds the buffer size.
	ntrb_AudioHeaderFromWAVFile_invalid_Subchunk1Size,	///< Subchunk1 size is somehow larger than chunk size.
	ntrb_AudioHeaderFromWAVFile_invalid_Subchunk2Size,	///< Subchunk2 offset added with its size exceeds the buffer size somehow.
};

/**
Attempts to fetch and convert the WAV header data to ntrb_AudioHeader, get the offset of the audio from fileBuffer.elem and the size of the audio data.

\param[in] fileBuffer the buffer containing the entire contents of the WAV file.
\param[out] returnArg the ntrb_AudioHeader with data written to.
\param[out] audiodataOffset the offset of the first audio data byte from fileBuffer.elem.
\param[out] audiodataSize the size of the audio data.
*/
NTRB_DLL_VISIBILITY enum ntrb_AudioHeaderFromWAVFileStatus ntrb_AudioHeader_from_WAVfile(ntrb_AudioHeader* const returnArg, size_t* const audiodataOffset, size_t* const audiodataSize, const ntrb_SpanU8 fileBuffer);

NTRB_DLL_VISIBILITY enum ntrb_AudioHeaderFromWAVFileStatus ntrb_AudioHeader_from_WAVfile_2(ntrb_AudioHeader* const returnArg, size_t* const audiodataOffset, size_t* const audiodataSize, const ntrb_SpanU8 fileBuffer);

NTRB_DLL_VISIBILITY bool verify_WAV_out_of_bounds(const size_t audiodataOffset, const size_t audiodataSize, const size_t filesize);

/**
Returns a separately allocated buffer containing the audio data from wavfile.

\param wavfile the buffer containing the entire file contents of the WAV file.
\param audiodata_size how many bytes of audio data to copy from wavfile.elem + audiodata_offset. This is not wavfile.elem - audiodata_offset.
\param audiodata_offset the offset of the first audio data byte.

You get two latter parameters from ntrb_AudioHeader_from_WAVfile().
*/
NTRB_DLL_VISIBILITY ntrb_AudioDatapoints ntrb_get_WAV_audiodata(const ntrb_SpanU8 wavfile, const size_t audiodata_size, const size_t audiodata_offset);

#ifdef __cplusplus
};
#endif

#endif