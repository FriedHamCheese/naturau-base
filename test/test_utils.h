/*
Copyright 2024 Pawikan Boonnaum

This file is part of naturau-base/ntrb.

naturau-base/ntrb is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

naturau-base/ntrb is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with naturau-base/ntrb. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef test_ntrb_utils_h
#define test_ntrb_utils_h

#include "utils.h"

#include <float.h>
#include <stdint.h>
#include <stdbool.h>

#include <assert.h>

static void test_ntrb_clamp_i64(){
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

static void test_ntrb_clamp_u64(){
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

static void test_ntrb_clamp_float(){
	const float allowable_error = FLT_MIN;
	assert(ntrb_float_equal(ntrb_clamp_float(28.9, 28.8, 30), 28.9, allowable_error));
	assert(ntrb_float_equal(ntrb_clamp_float(30.15, 30.16, 31), 30.16, allowable_error));
	assert(ntrb_float_equal(ntrb_clamp_float(495.98, 495, 495.97), 495.97, allowable_error));
}

static void test_ntrb_float_equal(){
	const float allowable_error = 0.000001;
	assert(ntrb_float_equal(2.0, 2.0, allowable_error) == true);
	assert(ntrb_float_equal(2.0 - allowable_error, 2.0, allowable_error) == true);
	assert(ntrb_float_equal(2.0 + allowable_error, 2.0, allowable_error) == true);
	assert(ntrb_float_equal(2.0 + allowable_error*2, 2.0, allowable_error) == false);	
}

void test_suite_ntrb_utils(){
	test_ntrb_clamp_i64();
	test_ntrb_clamp_u64();
	test_ntrb_clamp_float();
	test_ntrb_float_equal();
}


#endif