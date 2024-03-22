#ifndef test_ntrb_audeng_wrapper_h
#define test_ntrb_audeng_wrapper_h

#include "audeng_wrapper.h"

#include <assert.h>

static void test_ntrb_frames_for_msec(){	
	assert(ntrb_frames_for_msec(96000) == 96.0f);
}

static void test_suite_ntrb_audeng_wrapper(){
	test_ntrb_frames_for_msec();
}

#endif