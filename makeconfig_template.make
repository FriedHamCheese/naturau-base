NTRB_PORTAUDIO_INCLUDE := 
NTRB_PORTAUDIO_LINKFLAGS :=
NTRB_FLAC_INCLUDE :=
NTRB_FLAC_LINKFLAGS :=

NTRB_DEPENDENCY_INCLUDES := $(NTRB_PORTAUDIO_INCLUDE) $(NTRB_FLAC_INCLUDE)

NTRB_LINKING_DEPENDENCIES := $(NTRB_PORTAUDIO_LINKFLAGS) $(NTRB_FLAC_LINKFLAGS)
NTRB_COMPILING_SYMBOLS := -DFLAC__NO_DLL -DNTRB_MEMDEBUG
