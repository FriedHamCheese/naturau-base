#ifndef ntrb_cpp_include_compat_h
#define ntrb_cpp_include_compat_h

#ifdef __cplusplus
	#include <atomic>
	typedef std::atomic_bool atomic_bool;
#else
	#include <stdbool.h>
	#include <stdatomic.h>
#endif

#endif