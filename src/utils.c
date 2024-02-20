#include "utils.h"

#include <math.h>
#include <stdint.h>
#include <stdbool.h>

int64_t ntrb_clamp_i64(const int64_t value, const int64_t min, const int64_t max){
	if(value < min) return min;
	else if (value > max) return max;
	else return value;
}

uint64_t ntrb_clamp_u64(const uint64_t value, const uint64_t min, const uint64_t max){
	if(value < min) return min;
	else if (value > max) return max;
	else return value;
}

float ntrb_clamp_float(const float value, const float min, const float max){
	if(value < min) return min;
	else if (value > max) return max;
	else return value;
}

bool ntrb_float_equal(const float a, const float b, const float error_margin){
	const float difference = a - b;
	return fabsf(difference) <= fabsf(error_margin);
}