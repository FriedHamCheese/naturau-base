#ifndef ntrb_aud_std_fmt_h
#define ntrb_aud_std_fmt_h

/**
\file aud_std_fmt.h
A module providing functions converting unprocessed audio to the standard audio format used in ntrb's audio loop.
*/

#include "AudioDatapoints.h"
#include "AudioHeader.h"
#include "decode_flac.h"

#include "portaudio.h"

#include <stdio.h>
#include <stdint.h>

extern const uint32_t ntrb_std_samplerate;
extern const uint8_t ntrb_std_audchannels;
extern const PaSampleFormat ntrb_std_sample_fmt;


enum ntrb_StdAudFmtConversionResult{
	ntrb_StdAudFmtConversion_OK,
	ntrb_StdAudFmtConversion_AllocFailure,
	ntrb_StdAudFmtConversion_UnknownSampleFormat,
	ntrb_StdAudFmtConversion_UnsupportedChannels,
};

/**
Separately allocates the float32 conversion from the provided int16 ntrb_AudioDatapoints.

The size of the output would be 2x of the int16.

Each value of the float32 values is int16/INT16_MAX.

If the function failed to allocate memory for ntrb_AudioDatapoints, it returns failed_ntrb_AudioDatapoints.
*/
ntrb_AudioDatapoints ntrb_AudioDatapoints_i16_to_f32(const ntrb_AudioDatapoints int16);

/**
Copies the (offset) channel of the datapoints, and separately allocate ntrb_AudioDatapoints for containing it.

The multichannel_aud must be encoded in float32 and has at least 2 channels.

\param multichannel_aud An ntrb_AudioDatapoints which contains multiple audio channels interleaved in a frame.
  For example, frame 0: 0ch 1ch 2ch, frame 1: 0ch 1ch 2ch, etc...
\param channels The amount of audio channels which multichannel_aud contains.
\param offset The channel which you choose to split off of multichannel_aud.
  Must be a value between [0, channels - 1]. For stereo, 0 means left and 1 means right.
  
If the function failed to allocate memory for ntrb_AudioDatapoints, it returns failed_ntrb_AudioDatapoints.
*/
ntrb_AudioDatapoints ntrb_split_as_mono(const ntrb_AudioDatapoints multichannel_aud, const size_t channels, const size_t offset);

/**
Converts a mono channeled ntrb_AudioDatapoints to one with (dest_channels) and allocates the return ntrb_AudioDatapoints separately for containing it.

orig must be encoded in float32 format and has only 1 channel.

It copies each of the frame in orig next to each other by (dest_channels - 1) copy.
So the frame of the output has 3x of the same datapoint.

For example, if dest_channels is 3:

orig frame 0: 0ch, orig frame 1: 0ch, orig frame 2: 0ch -> dest frame 0: 0ch 1ch 2ch, dest frame 1: 0ch 1ch 2ch, dest frame 20: 0ch 1ch 2ch.

Where 0ch 1ch and 2ch of each frame is the same value.

If the function failed to allocate memory for ntrb_AudioDatapoints, it returns failed_ntrb_AudioDatapoints.
*/
ntrb_AudioDatapoints ntrb_mono_to_xchannels(const ntrb_AudioDatapoints orig, const size_t dest_channels);
ntrb_AudioDatapoints ntrb_merge_to_stereo(const ntrb_AudioDatapoints l_ch, const ntrb_AudioDatapoints r_ch);

ntrb_AudioDatapoints ntrb_to_samplerate_mono(const ntrb_AudioDatapoints orig, const double orig_samplerate, const double dest_samplerate);
ntrb_AudioDatapoints ntrb_to_samplerate(const ntrb_AudioDatapoints orig, const uint32_t orig_samplerate, const uint32_t dest_samplerate);

enum ntrb_StdAudFmtConversionResult ntrb_to_std_sample_fmt(ntrb_AudioDatapoints* const ret_aud, const ntrb_AudioDatapoints orig, const PaSampleFormat originalFormat);
enum ntrb_StdAudFmtConversionResult ntrb_to_std_aud_channels(ntrb_AudioDatapoints* const ret_aud, const ntrb_AudioDatapoints orig, const uint8_t audchannels);
enum ntrb_StdAudFmtConversionResult ntrb_to_std_samplerate(ntrb_AudioDatapoints* const ret_aud, const ntrb_AudioDatapoints orig, const uint32_t samplerate);
enum ntrb_StdAudFmtConversionResult ntrb_to_standard_format(ntrb_AudioDatapoints* const ret_aud, const ntrb_AudioDatapoints orig, const ntrb_AudioHeader* const orig_header);

enum ntrb_LoadStdFmtAudioResult{
	ntrb_LoadStdFmtAudioResult_OK,
	ntrb_LoadStdFmtAudioResult_AllocError,
	ntrb_LoadStdFmtAudioResult_FiletypeError,
	ntrb_LoadStdFmtAudioResult_ntrb_ReadFileResult = 15,
	//30
	ntrb_LoadStdFmtAudioResult_ntrb_AudioHeaderFromWAVFileStatus = ntrb_LoadStdFmtAudioResult_ntrb_ReadFileResult + 15,
	//45
	ntrb_LoadStdFmtAudioResult_ntrb_FLAC_decode_status = ntrb_LoadStdFmtAudioResult_ntrb_AudioHeaderFromWAVFileStatus + 15,
	//55
	ntrb_LoadStdFmtAudioResult_ntrb_StdAudFmtConversionResult = ntrb_LoadStdFmtAudioResult_ntrb_FLAC_decode_status + ntrb_FLAC_decode_FLAC__StreamDecoderErrorStatus + 10,
};

enum ntrb_LoadStdFmtAudioResult ntrb_load_wav(ntrb_AudioHeader* const header, ntrb_AudioDatapoints* const datapoints, const char* const filename);
enum ntrb_LoadStdFmtAudioResult ntrb_load_flac(ntrb_AudioHeader* const header, ntrb_AudioDatapoints* const datapoints, const char* const filename);

enum ntrb_LoadStdFmtAudioResult ntrb_load_std_fmt_audio(ntrb_AudioDatapoints* const ret, const char* const filename);

#endif