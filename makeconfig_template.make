NTRB_PORTAUDIO_INCLUDE := ./dependencies/portaudio/include
NTRB_PORTAUDIO_LIBDIR := ./dependencies/portaudio/build
NTRB_FLAC_INCLUDE := ./dependencies/flac/include
NTRB_FLAC_LIBDIR := ./dependencies/flac/build/libFLAC
NTRB_LIBS := -lportaudio -lFLAC.dll

NTRB_COMPILING_SYMBOLS := -DNTRB_MEMDEBUG
NTRB_SHARED_LIBS := LINUX
