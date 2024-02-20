#ifndef ntrb_utils_h
#define ntrb_utils_h

#include <stdint.h>
#include <stdbool.h>

int64_t ntrb_clamp_i64(const int64_t value, const int64_t min, const int64_t max);
uint64_t ntrb_clamp_u64(const uint64_t value, const uint64_t min, const uint64_t max);
float ntrb_clamp_float(const float value, const float min, const float max);
bool ntrb_float_equal(const float a, const float b, const float error_margin);

#endif