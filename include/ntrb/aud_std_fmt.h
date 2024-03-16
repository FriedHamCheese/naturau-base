#ifndef ntrb_aud_std_fmt_h
#define ntrb_aud_std_fmt_h

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

ntrb_AudioDatapoints ntrb_AudioDatapoints_i16_to_f32(const ntrb_AudioDatapoints int16);

ntrb_AudioDatapoints ntrb_split_as_mono(const ntrb_AudioDatapoints multichannel_aud, const size_t channels, const size_t offset);
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
	ntrb_LoadStdFmtAudioResult_ntrb_AudioHeaderFromWAVFileStatus = ntrb_LoadStdFmtAudioResult_ntrb_ReadFileResult + 15,
	ntrb_LoadStdFmtAudioResult_ntrb_FLAC_decode_status = ntrb_LoadStdFmtAudioResult_ntrb_AudioHeaderFromWAVFileStatus + 15,
	
	ntrb_LoadStdFmtAudioResult_ntrb_StdAudFmtConversionResult = ntrb_LoadStdFmtAudioResult_ntrb_FLAC_decode_status + ntrb_FLAC_decode_FLAC__StreamDecoderErrorStatus + 10,
};

enum ntrb_LoadStdFmtAudioResult ntrb_load_wav(ntrb_AudioHeader* const header, ntrb_AudioDatapoints* const datapoints, const char* const filename);
enum ntrb_LoadStdFmtAudioResult ntrb_load_flac(ntrb_AudioHeader* const header, ntrb_AudioDatapoints* const datapoints, const char* const filename);

enum ntrb_LoadStdFmtAudioResult ntrb_load_std_fmt_audio(ntrb_AudioDatapoints* const ret, const char* const filename);

#endif