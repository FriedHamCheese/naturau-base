#include "test_utils.h"
#include "utils.h"

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

static void test_ntrb_clamp_i64(FILE*, FILE*){
	//test a value within range
	const int64_t max_1 = 495;
	const int64_t min_1 = 32;
	const int64_t val_1 = 397;
	assert(ntrb_clamp_i64(val_1, min_1, max_1) == val_1);
	
	//test a value exceeding max
	const int64_t max_2 = 96639;
	const int64_t min_2 = 45;
	const int64_t val_2 = 96641;
	assert(ntrb_clamp_i64(val_2, min_2, max_2) == max_2);	
	
	//test a value below minimum
	const int64_t max_3 = 65;
	const int64_t min_3 = 5;
	const int64_t val_3 = -27;
	assert(ntrb_clamp_i64(val_3, min_3, max_3) == min_3);	
}

static void test_ntrb_clamp_u64(FILE*, FILE*){
	//test a value within range
	const uint64_t max_1 = 495;
	const uint64_t min_1 = 32;
	const uint64_t val_1 = 397;
	assert(ntrb_clamp_u64(val_1, min_1, max_1) == val_1);
	
	//test a value exceeding max
	const uint64_t max_2 = 96639;
	const uint64_t min_2 = 45;
	const uint64_t val_2 = 96641;
	assert(ntrb_clamp_u64(val_2, min_2, max_2) == max_2);	
	
	//test a value below minimum
	const uint64_t max_3 = 65;
	const uint64_t min_3 = 47;
	const uint64_t val_3 = 35;
	assert(ntrb_clamp_u64(val_3, min_3, max_3) == min_3);	
}

void test_suite_ntrb_utils(FILE* const outstream, FILE* const errstream){
	test_ntrb_clamp_i64(outstream, errstream);
	test_ntrb_clamp_u64(outstream, errstream);
}
