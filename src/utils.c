#include "utils.h"

#include <stdint.h>

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