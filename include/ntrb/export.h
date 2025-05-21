#ifndef EXPORT_H
#define EXPORT_H

#ifdef NTRB_DLL_EXPORT
	#define NTRB_DLL_VISIBILITY __declspec(dllexport)
#else
	#ifdef NTRB_DLL_IMPORT
		#define NTRB_DLL_VISIBILITY __declspec(dllimport)
	#else
		#define NTRB_DLL_VISIBILITY
	#endif
#endif

#endif