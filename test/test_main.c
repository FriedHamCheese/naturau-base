#include "test__alloc_bytevec.h"
#include "test_alloc.h"

#ifndef NTRB_TEST_MEMDEBUG_LEAK_ONLY
#include "test_aud_std_fmt.h"
#include "test_audeng_wrapper.h"
#include "test_AudioDatapoints.h"
#include "test_bytevec.h"
#include "test_file_wrapper.h"
//#include "test_RuntimeCoreData.h"
#include "test_SlicedStrings.h"
#include "test_str_utils.h"
#include "test_utils.h"
#include "test_wav_wrapper.h"
#endif

#include <stdio.h>

int main(){
	printf("Initiating test routine...\n");	
	printf("If an all clear message is not shown after this, the test has failed.\n");
	fflush(stdout);
	
	test_suite__ntrb_alloc_bytevec();
	test_suite_ntrb_alloc();
	
	#ifndef NTRB_TEST_MEMDEBUG_LEAK_ONLY
	#ifdef NTRB_MEMDEBUG
	assert(ntrb_memdebug_init());
	#endif
	
	test_suite_ntrb_utils();	
	test_suite_ntrb_bytevec();
	test_suite_ntrb_AudioDatapoints();
	test_suite_ntrb_SlicedStrings();	
	test_suite_ntrb_str_utils();	

	test_suite_ntrb_file_wrapper();
	test_suite_ntrb_wav_wrapper();
	
	test_suite_ntrb_aud_std_fmt();
	test_suite_ntrb_audeng_wrapper();
	//test_suite_ntrb_RuntimeCoreData();
	
	#ifdef NTRB_MEMDEBUG
	ntrb_memdebug_uninit(true);
	#endif
	#endif
	
	printf("\n[ALL CLEAR]: No errors from test routine.\n");
	fflush(stdout);
	
	return 0;
}