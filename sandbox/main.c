#include "AudioBuffer.h"
#include "audeng_wrapper.h"
#include "RuntimeCoreData.h"

#include "alloc.h"

#include <assert.h>
#include <stdbool.h>

int main(){
	#ifdef NTRB_MEMDEBUG
	ntrb_memdebug_init_with_return_value();
	#endif
	
	ntrb_RuntimeCoreData rcd;
	const int rcd_new_status = ntrb_RuntimeCoreData_new(&rcd, 256);
	if(rcd_new_status != 0){
		fprintf(stderr, "[Error]: %s: %d: Error initialising ntrb_RuntimeCoreData (%d).\n", __FILE__, __LINE__, rcd_new_status);		
		return -1;
	}
	
	assert(ntrb_RuntimeCoreData_queue_audio(&rcd, "../../ntr-aud/my hair.wav") == 0);
	rcd.in_pause_state = false;
	
	pthread_t audio_engine_thread;
	pthread_create(&audio_engine_thread, NULL, ntrb_run_audio_engine, &rcd);
	
	pthread_join(audio_engine_thread, NULL);
	
	ntrb_RuntimeCoreData_free(&rcd);

	#ifdef NTRB_MEMDEBUG
	ntrb_memdebug_uninit(true);
	#endif
	
	return 0;
}