# What is naturau-base?
naturau-base or ntrb, is the base (core) code of naturau, a full-scale dynamic audio mixer I'm working on; and a foundation for my future work in audio.
naturau-base provides the foundation for playing and streaming audio from files (currently WAV or FLAC) or creating a source of your own, 
as well as a few utilities to aid development of other projects using naturau-base.

This is an example of a high-level audio playback with naturau-base:
```
#include "ntrb/alloc.h"
#include "ntrb/audeng_wrapper.h"
#include "ntrb/RuntimeCoreData.h"

#include <assert.h>
#include <stdbool.h>

int main(){
	#ifdef NTRB_MEMDEBUG
	assert(ntrb_memdebug_init_with_return_value() == ntrb_memdebug_OK);
	#endif
	
	ntrb_RuntimeCoreData rcd;
	const size_t track_count = 2;
	assert(ntrb_RuntimeCoreData_new(&rcd, track_count) == ntrb_RuntimeCoreData_OK);
	
	const char* const aud_filename = "audio.wav";
	ntrb_RuntimeCoreData_queue_audio(&rcd, aud_filename);
	
	rcd.in_pause_state = false;
	
	ntrb_run_audio_engine(&rcd);
	ntrb_RuntimeCoreData_free(&rcd);
	
	#ifdef NTRB_MEMDEBUG
	assert(ntrb_memdebug_uninit(true) == ntrb_memdebug_OK);
	#endif
	return 0;
}
```
Although, naturau-base is more designed towards being extended, modified or included in your projects. See naturau.
There is truly no right or wrong way of using naturau-base. As long as you are able to make what you want, all the power to you :D

# Building ntrb
*This is a verbatim of the building documentation in the Doxygen documentation*

## Prerequisites List
- gcc C99 compiler
- make
- CMake - for libFLAC 
- PortAudio
- libFLAC from Xiph.org
- Doxygen

## Building ntrb Documentation
Navigate to ./doc directory in ntrb directory, call `doxygen ./docconf`.

## Where to put PortAudio and libFLAC?
Anywhere you'd like. Just make sure you are able to build both of them.

## Building PortAudio
You can try building PortAudio from scratch by following the guide from PortAudio,
or in MSYS2 MinGW64, you can download the package.

Tip for people manually compiling with MSYS2 MinGW64: 
Remove the -DPA_*=0 arguments when creating CMake config.
This ensures PortAudio will have multiple audio hosts for fallback.

Once that is completed, test the audio engine by compiling the examples in the PortAudio directory, 
paex_sine.c or paex_saw.c is a good one.
To compile the an example in a single gcc call, 
you need to provide the PortAudio include folder if needed with `-I`,
include the PortAudio lib file with `-lportaudio` and provide the directory of the lib file if needed
using the `-L`. My gcc call in the examples directory looks something like
```
gcc ./paex_sine.c -I../include -L../build -lportaudio
```

If you chose to install PortAudio as an MSYS package, the libPortAudio.dll file is in your environment.
But when shipping your code, make sure to include it in where the compiler is called in your project directory as well.

If you chose to manually build one yourself, the .dll file is in the build directory of PortAudio 
and must be present in the compiler's call directory.
You can either put the compiled .dll in your terminal's environment directly, 
or put it in where you would call `make` or `gcc` in your project. 
For your example compilation, you can either compile from the build directory instead or copy the .dll to the examples directory.

If the playback is correct, you're good on this one!

## Building libFLAC
Follow the instructions on the github page of libFLAC.
Download the release source code, not the platform specific release. We need to build it from scratch.

There are a few optimisations for building libFLAC in the CMakeLists.txt file in its directory.
- If you won't be using the C++ version of libFLAC, the BUILD_CXXLIBS option should be set to OFF.
- If you won't be using the command line programs in flac/src/flac and flac/src/metaflac, set BUILD_PROGRAMS to OFF.
- If you don't need the examples to be built, set BUILD_EXAMPLES to off.
- If you don't use MAN Pages or local Doxygen documentation of libFLAC, switch it off.

If you don't have OGG libary from Xiph, switch WITH_OGG to OFF. Not doing so will cause an error.

Tip for MSYS MinGW64 users: if needed, 
add `-G "MSYS Makefiles"` in `cmake /path/to/flac-source` to build with `make`.

Anything else should be left as is. But make sure BUILD_SHARED_LIBS is OFF. We are compiling it statically.

You can choose to `make install` libFLAC or not. It slightly changes how you incorporate it with ntrb,
but we got you covered for both :D

Once the libFLAC test passed, you are now clear to build ntrb.

## Building ntrb
ntrb itself doesn't have an executable, building ntrb means building the object files for other code to include.

First, rename or copy the "extern_path - template.make" to "extern_path.make".
In it, there are PORTAUDIO_INCLUDEDIR, PORTAUDIO_LIBDIR, FLAC_INCLUDEDIR, FLAC_LIBDIR.
You will need to fill in path to the include directory of PortAudio, the directory of the libPortAudio.a,
the include directory of libFLAC and the directory of libFLAC.a (in where-you-built-flac/src/libFLAC) respectively.
If you used the PortAudio package instead, the two probably don't have to be filled in.
And if you ran `make install` for libFLAC, you probably don't have to fill in the latter two.

To build ntrb, call `make` in the ntrb directory.
Aside from a few pointer warnings from the alloc module, no errors should occur, 
and the ALL CLEAR for the unit test should be there.

Congratulations! You have built ntrb! You can try to build the example in the ./example directory.
Call `make` in there and you should have a working demonstration of ntrb code.
Or you can refer to the next page for including and building with ntrb in your project.
