#include "audeng_wrapper.h"

#include "aud_std_fmt.h"

#include "AudioDatapoints.h"
#include "RuntimeCoreData.h"

#include "portaudio.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

const unsigned long ntrb_msecs_per_callback = 100;


static int stream_audio(const void *, void *output_void, unsigned long frameCount, 
						const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags, 
						void *userData)
{
	const unsigned long float_count = frameCount * ntrb_std_audchannels;
	float* const output = (float*)output_void;

	for(unsigned long i = 0; i < float_count; i++)
		output[i] = 0.0;
	
	ntrb_RuntimeCoreData* const runtime_data = (ntrb_RuntimeCoreData*)userData;
	if(!runtime_data->in_pause_state){
		pthread_rwlock_wrlock(&(runtime_data->audio_track_rwlock));

		for(uint16_t track_id = 0; track_id < runtime_data->audio_track_count; track_id++)
		{
			if(runtime_data->audio_tracks[track_id] == NULL)
				continue;
			
			for(unsigned long i = 0; i < float_count; i++)
			{
				ntrb_AudioDatapoints* current_audiotrack = (runtime_data->audio_tracks[track_id]);
				
				if(current_audiotrack->byte_pos < current_audiotrack->byte_count){	
					output[i] += *((float*)(current_audiotrack->bytes + current_audiotrack->byte_pos));
					current_audiotrack->byte_pos += sizeof(float);
				}else{
					ntrb_RuntimeCoreData_free_track(runtime_data, track_id);
					break;
				}
			}
		}
		pthread_rwlock_unlock(&(runtime_data->audio_track_rwlock));
	}
	
	if(runtime_data->requested_exit) return paComplete;
	else return paContinue;
}


static PaError get_output_stream_params(PaStreamParameters* const output_params){
	output_params->device = Pa_GetDefaultOutputDevice();
	if(output_params->device == paNoDevice) return paDeviceUnavailable;
	
	output_params->suggestedLatency = Pa_GetDeviceInfo(output_params->device)->defaultLowOutputLatency;
	output_params->sampleFormat = ntrb_std_sample_fmt;
	output_params->channelCount = ntrb_std_audchannels;
	output_params->hostApiSpecificStreamInfo = NULL;

	return paNoError;
}

void* ntrb_run_audio_engine(void* const runtime_data_void){
	ntrb_RuntimeCoreData* const runtime_data = (ntrb_RuntimeCoreData*)runtime_data_void;
	PaError pa_error = paNoError;
	PaError pa_uninit_error = paNoError;
	
	//Pa_Initialize Alloc
	pa_error = Pa_Initialize();
	if(pa_error) goto print_err;
		
	PaStreamParameters output_stream_params;
	pa_error = get_output_stream_params(&output_stream_params);
	if(pa_error) goto uninit_pa;
	
	//output_stream Alloc
	PaStream* output_stream;
	const unsigned long frames_per_buffer = ntrb_frames_for_msec(ntrb_std_samplerate) * ntrb_msecs_per_callback;
	pa_error = Pa_OpenStream(&output_stream, NULL, &output_stream_params, 
								ntrb_std_samplerate, frames_per_buffer, paClipOff,
								stream_audio, runtime_data);
	if(pa_error) goto uninit_pa;
	
	pa_error = Pa_StartStream(output_stream);
	if(pa_error) goto close_stream;
	
	while(runtime_data->requested_exit == false){
		Pa_Sleep(ntrb_msecs_per_callback);
	}
	
	Pa_StopStream(output_stream);
	//it's a lot more messier to error handle the cleanup procedures. So we only check at Pa_Terminate().
	
	close_stream:
	Pa_CloseStream(output_stream);
		
	uninit_pa:
	pa_uninit_error = Pa_Terminate();
	
	print_err:
	if(pa_error != paNoError){
		fprintf(stderr, "PaError %i caught.\n", pa_error);
		fprintf(stderr,  "(%s)\n", Pa_GetErrorText(pa_error));
	}
	if(pa_uninit_error != paNoError){
		fprintf(stderr, "Error %i caught while uninitializing audio system.\n", pa_uninit_error);
		fprintf(stderr,  "(%s)\n", Pa_GetErrorText(pa_uninit_error));
	}
	
	return NULL;
}

unsigned long ntrb_frames_for_msec(const unsigned long samplerate){
	//samplerate is samples per second, so divide samplerate by msecs to get samplerate per msec.
	return samplerate / 1000;
}