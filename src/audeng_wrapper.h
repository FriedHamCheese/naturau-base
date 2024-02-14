#ifndef ntrb_audengwrapper_h
#define ntrb_audengwrapper_h

//in audeng_wrapper.c
extern const unsigned long ntrb_msecs_per_callback;

void* ntrb_run_audio_engine(void* const runtime_data_void);
unsigned long ntrb_frames_for_msec(const unsigned long samplerate);

#endif