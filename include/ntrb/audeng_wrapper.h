#ifndef ntrb_audengwrapper_h
#define ntrb_audengwrapper_h

/*
Copyright 2024 Pawikan Boonnaum

This file is part of naturau-base/ntrb.

naturau-base/ntrb is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

naturau-base/ntrb is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with naturau-base/ntrb. If not, see <https://www.gnu.org/licenses/>.
*/


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