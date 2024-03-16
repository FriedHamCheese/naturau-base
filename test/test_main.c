#include "test_aud_std_fmt.h"
#include "test_audeng_wrapper.h"
#include "test_AudioDatapoints.h"
#include "test_file_wrapper.h"
#include "test_RuntimeCoreData.h"
#include "test_SlicedStrings.h"
#include "test_str_utils.h"
#include "test_utils.h"
#include "test_wav_wrapper.h"

#include <stdio.h>

int main(){
	FILE* const outstream = stdout;
	FILE* const errstream = stderr;
	
	fprintf(outstream, "Initiating test routine...\n");	
	fprintf(outstream, "If an all clear message is not shown after this, the test has failed.\n");
	fflush(outstream);
	
	test_suite_ntrb_AudioDatapoints(outstream, errstream);
	test_suite_ntrb_utils(outstream, errstream);		
	test_suite_ntrb_str_utils(outstream, errstream);	

	test_suite_ntrb_file_wrapper(outstream, errstream);
	test_suite_ntrb_SlicedStrings(outstream, errstream);
	test_suite_ntrb_wav_wrapper(outstream, errstream);

	test_suite_ntrb_aud_std_fmt(outstream, errstream);
	test_suite_ntrb_RuntimeCoreData(outstream, errstream);	
	test_suite_ntrb_audeng_wrapper(outstream, errstream);
	
	fprintf(outstream, "\n[ALL CLEAR]: No errors from test routine.\n");
	fflush(outstream);
	
	return 0;
}