#ifndef ntrb_audengwrapper_h
#define ntrb_audengwrapper_h

/**
\file audeng_wrapper.h
A module providing the audio engine loop.
*/

/**
The duration in milliseconds which the audio engine plays back the audio channels before reading the audio again.

Defined in audeng_wrapper.c
*/
extern const unsigned long ntrb_msecs_per_callback;
extern const unsigned long ntrb_std_frame_count;


/**
The function for the audio engine thread.

If any errors occur while initialising or uninitialising the audio engine, the error would be printed to stderr and the function would exit early.

The function only returns NULL. You shouldn't check the return value, it's for conforming with pthread.
*/
void* ntrb_run_audio_engine(void* const runtime_data_void);

/**
A function returning the amount of frames in a single millisecond of the argument
*/
unsigned long ntrb_frames_for_msec(const unsigned long samplerate);

#endif