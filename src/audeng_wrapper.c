#include "audeng_wrapper.h"

#include "aud_std_fmt.h"

#include "AudioDatapoints.h"
#include "RuntimeCoreData.h"

#include "portaudio.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

const unsigned long ntrb_msecs_in_sec = 1000;
const unsigned long ntrb_msecs_per_callback = 100;
//const unsigned long ntrb_std_monochannel_samples = (ntrb_std_samplerate * ntrb_msecs_per_callback) / ntrb_msecs_in_sec;
const unsigned long ntrb_std_monochannel_samples = 4800;

static int stream_audio(const void *, void *output_void, unsigned long frameCount, 
						const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags, 
						void *userData)
{
	const unsigned long float_count = frameCount * ntrb_std_audchannels;
	float* const output = (float*)output_void;

	for(unsigned long i = 0; i < float_count; i++)
		output[i] = 0.0;
	
	//If the loop is in pause state, do nothing. The output is 0 filled, so no sound is produced.
	//Else, mix the audio buffers and free them if an error occurred or it reaches its end.
	ntrb_RuntimeCoreData* const runtime_data = (ntrb_RuntimeCoreData*)userData;
	if(!runtime_data->in_pause_state)
	{
		//Alloc runtime_data->audio_track_rwlock, free with pthread_rwlock_unlock().
		//
		//We need a write lock just in case the audio buffer reaches its end or has an error while loading,
		//then we can tell the ntrb_RuntimeCoreData to free the audio buffer and mark its audio track as empty.
		pthread_rwlock_wrlock(&(runtime_data->audio_track_rwlock));

		for(uint16_t track_id = 0; track_id < runtime_data->audio_track_count; track_id++)
		{
			ntrb_AudioBuffer* const current_audiotrack = (runtime_data->audio_tracks[track_id]);
			if(current_audiotrack == NULL) continue;
			
			//Alloc current_audiotrack->buffer_access, free with pthread_rwlock_unlock().
			//
			//Try to acquire a read lock without blocking the loop.
			//If the audio buffer is busy or a lock error occurred, we skip this track.
			const int audiotrack_acq_rdlock_error = pthread_rwlock_tryrdlock(&(current_audiotrack->buffer_access));
			if(audiotrack_acq_rdlock_error){
				fprintf(stderr, "Track %hu missed.\n", track_id);
				continue;
			}
			
			//If an audio loading error from previous call occurred, we free the buffer.
			//This mostly will be from the audio loader indicating an EOF of the audio file. 
			//But we also free it if there was an actual error.
			if(current_audiotrack->load_err){
				//ntrb_RuntimeCoreData_free_track() calls ntrb_AudioBuffer_free(), which will block execution until
				//it can acquire a write lock, so we need to unlock our acquired read lock.
				pthread_rwlock_unlock(&(current_audiotrack->buffer_access));
				ntrb_RuntimeCoreData_free_track(runtime_data, track_id);
				continue;
			}
			
			for(unsigned long i = 0; i < float_count; i++){
				output[i] += (current_audiotrack->datapoints)[i];
			}
			
			//Free current_audiotrack->buffer_access.
			pthread_rwlock_unlock(&(current_audiotrack->buffer_access));
	
			//Dispatch an audio file loading thread and let loose of it.
			//We cannot use pthread_join(), as it would block the execution.
			//We can know if the buffer has finished being loaded or not by trying to acquire the buffer_access lock.
			pthread_t thread;
			const bool thread_create_error = pthread_create(&thread, NULL, current_audiotrack->load_buffer_callback, current_audiotrack) != 0;			
			if(!thread_create_error){
				//Detach tells the OS to free the thread immediately after it is finished.
				//If it fails, the OS will take some time to free the thread.
				//Probably not worth the trouble of return value.
				pthread_detach(thread);
			}
		}
		//Free runtime_data->audio_track_rwlock.
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
								ntrb_std_samplerate, frames_per_buffer, paNoFlag,
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