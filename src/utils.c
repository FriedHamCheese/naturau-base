/**
Copyright 2024 Pawikan Boonnaum

This file is part of naturau-base/ntrb.

naturau-base/ntrb is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

naturau-base/ntrb is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with naturau-base/ntrb. If not, see <https://www.gnu.org/licenses/>.
*/

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