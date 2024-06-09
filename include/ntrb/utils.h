#ifndef ntrb_utils_h
#define ntrb_utils_h

/*
Copyright 2024 Pawikan Boonnaum

This file is part of naturau-base/ntrb.

naturau-base/ntrb is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

naturau-base/ntrb is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with naturau-base/ntrb. If not, see <https://www.gnu.org/licenses/>.
*/

/**
\file utils.h
A module providing mostly clamping functions for different types of numbers.
*/

#include <stdint.h>
#include <stdbool.h>

///Clamps the value to be within [min, max].
int64_t ntrb_clamp_i64(const int64_t value, const int64_t min, const int64_t max);
///Clamps the value to be within [min, max].
uint64_t ntrb_clamp_u64(const uint64_t value, const uint64_t min, const uint64_t max);
///Clamps the value to be within [min, max].
float ntrb_clamp_float(const float value, const float min, const float max);

///Used for comparing equality between the floats. 
///Comparing two visually exact floats might not result in logical equality,
///so we provide the equality comparision within error_margin, as floating points aren't always exact.
bool ntrb_float_equal(const float a, const float b, const float error_margin);

#endif