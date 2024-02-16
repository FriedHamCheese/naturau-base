#define NATURAU_TYPE_TEST 0
#if NATURAU_TYPE_TEST == 0

#include "RuntimeCoreData.h"
#include "SlicedStrings.h"
#include "str_utils.h"
#include "aud_std_fmt.h"
#include "audeng_wrapper.h"
#include "file_wrapper.h"

#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void* user_input_loop(void* const runtime_core_data_void){
	while(true){
		printf(": ");
		fflush(stdout);
		
		char* user_input_str;
		const enum ntrb_GetCharStatus getstr_status = ntrb_getsn(128, stdin, &user_input_str);
		if(getstr_status != ntrb_GetChar_OK){
			fprintf(stderr, "[Error]: %s: %d: Error reading user input. (ntrb_GetCharStatus %d) \n", __FILE__, __LINE__, getstr_status);
			fflush(stderr);
			continue;
		}
		
		ntrb_SlicedStrings sliced_strs = ntrb_SlicedStrings_slice(user_input_str, strlen(user_input_str));
		free(user_input_str);
		if(sliced_strs.str_ptrs == NULL){
			fprintf(stderr, "[Error]: %s: %d: Error parsing user input.\n", __FILE__, __LINE__);
			fflush(stderr);	
			continue;
		}
		
		if(sliced_strs.elem < 1){
			fprintf(stderr, "[Error]: %s: %d: No input provided.\n", __FILE__, __LINE__);
			fflush(stderr);
			ntrb_SlicedStrings_free(&sliced_strs);
			continue;
		}
		
		const int strcmp_equal = 0;
		const char* const command = sliced_strs.str_ptrs[0];
		ntrb_RuntimeCoreData* const rcd = (ntrb_RuntimeCoreData*)runtime_core_data_void;
		if(strcmp(command, "p") == strcmp_equal){
			rcd->in_pause_state = !(rcd->in_pause_state);
			if(rcd->in_pause_state == true) fprintf(stdout, "Paused.\n");
			else fprintf(stdout, "Resuming...\n");
			fflush(stdout);
		}
		else if(strcmp(command, "l") == strcmp_equal){
			const bool no_filename_provided = sliced_strs.elem < 2;
			if(no_filename_provided){
				fprintf(stderr, "[Error]: %s: %d: No filename provided.\n", __FILE__, __LINE__);
				fflush(stderr);
			}else{
				char* concat_strs = ntrb_SlicedStrings_concat_strs(sliced_strs, 1, sliced_strs.elem, ' ');
				if(concat_strs == NULL){
					fprintf(stderr, "[Error]: %s: %d: Error concatenating filename.\n", __FILE__, __LINE__);
					fflush(stderr);
				}else{		
					const enum ntrb_RCD_QueueAudioReturn queueaudio_status = ntrb_RuntimeCoreData_queue_audio(rcd, concat_strs);
					if(queueaudio_status != ntrb_RCD_QueueAudio_OK){
						const enum ntrb_ReadFileResult readfile_result = queueaudio_status - ntrb_RCD_QueueAudio_ntrb_LoadStdFmtAudioResult - ntrb_LoadStdFmtAudioResult_ntrb_ReadFileResult;
						fprintf(stderr, "[Error]: %s: %d: Unable to queue audio (ntrb_RCD_QueueAudioReturn %d).\n", __FILE__, __LINE__, queueaudio_status);		
						if(readfile_result == ntrb_ReadFileResult_FileOpenError) 
							fprintf(stderr, "\tMost common cause of this error is: the file does not exist in the specified directory.\n");
						
						fflush(stderr);
					}else{
						fprintf(stdout, "Audio queued.\n");
						fflush(stdout);
					}
				}
			}
		}	
		else if(strcmp(command, "q") == strcmp_equal){
			rcd->requested_exit = true;
			ntrb_SlicedStrings_free(&sliced_strs);
			break;
		}
		else{
			fprintf(stderr, "[Error]: %s: %d: Invalid command.\n", __FILE__, __LINE__);
			fflush(stderr);			
		}
		
		ntrb_SlicedStrings_free(&sliced_strs);
	}
	
	return NULL;
}

int main(){
	ntrb_RuntimeCoreData rcd = new_ntrb_RuntimeCoreData(256);
	if(rcd.audio_tracks == NULL) return 0;
	
	pthread_t user_input_thread;
	pthread_create(&user_input_thread, NULL, user_input_loop, &rcd);
	
	pthread_t audio_engine_thread;
	pthread_create(&audio_engine_thread, NULL, ntrb_run_audio_engine, &rcd);
	
	pthread_join(user_input_thread, NULL);	
	pthread_join(audio_engine_thread, NULL);
	
	free_ntrb_RuntimeCoreData(&rcd);
	
	return 0;
}

#endif
#if NATURAU_TYPE_TEST == 1

int main(){
	return 0;
}

#endif