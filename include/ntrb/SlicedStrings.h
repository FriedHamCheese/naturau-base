#ifndef ntrb_SlicedStrings_h
#define ntrb_SlicedStrings_h

#include <stddef.h>

typedef struct{
	char** str_ptrs;
	size_t elem;
} ntrb_SlicedStrings;

//both externs from ntrb_SlicedStrings.c
extern const ntrb_SlicedStrings failed_ntrb_SlicedStrings;
extern const size_t ntrb_slice_string_max_len;

ntrb_SlicedStrings ntrb_SlicedStrings_new(const size_t str_count);
void ntrb_SlicedStrings_free(ntrb_SlicedStrings* const obj);

ntrb_SlicedStrings ntrb_SlicedStrings_slice_without_trimming(const char* const str, const size_t str_len, const char separator);
ntrb_SlicedStrings ntrb_SlicedStrings_slice_sep(const char* const str, const size_t str_len, const char separator);
ntrb_SlicedStrings ntrb_SlicedStrings_slice(const char* const str, const size_t str_len);
char* ntrb_SlicedStrings_concat_strs(const ntrb_SlicedStrings slices, const size_t beg, const size_t end, const char separator);

#endif