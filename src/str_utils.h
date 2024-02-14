#ifndef ntrb_str_utils_h
#define ntrb_str_utils_h

#include <stdio.h>

enum ntrb_GetCharStatus{
	ntrb_GetChar_OK,
	ntrb_GetChar_AllocErr,
	ntrb_GetChar_EOF,
	ntrb_GetChar_Ferror,
};

enum ntrb_GetCharStatus ntrb_getc(FILE* const instream, char* const ret);
enum ntrb_GetCharStatus ntrb_getsn(const size_t max_strlen, FILE* const instream, char** ret);

char* ntrb_trim_duplicate_separators(const char* const untrimmed_str, const size_t str_len, const char separator);

#endif
