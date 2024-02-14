#include "_test_template.h"
#include "../src/template.h"

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

static void test_template_empty(FILE*, FILE*){	
}

void test_suite_template(FILE* const outstream, FILE* const errstream){
	test_template_empty(outstream, errstream);
}
