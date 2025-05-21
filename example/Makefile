EXECFILE := ./build.exe

include ../makeconfig.make

CC := gcc
CFLAGS := -Wall -Wextra -g3 -I../$(NTRB_PORTAUDIO_INCLUDE) -I../$(NTRB_FLAC_INCLUDE) -I../include/ntrb $(NTRB_COMPILING_SYMBOLS)
LDFLAGS :=
LDLIBS := -L../$(NTRB_PORTAUDIO_LIBDIR) -L../$(NTRB_FLAC_LIBDIR) -L../bin -lntrb $(NTRB_LIBS) -DNTRB_DLL_IMPORT

$(EXECFILE): main.c
	$(CC) $(CFLAGS) main.c -o main.exe $(LDFLAGS) $(LDLIBS) 

.PHONY: clean
clean:
	rm build.exe
	rm main.o