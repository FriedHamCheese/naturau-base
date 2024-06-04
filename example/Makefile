EXECFILE := ./build.exe
EXTERN_OBJFILES := $(wildcard ../bin/*.o)

include ../extern_path.make

CC := gcc
CFLAGS := -Wall -Wextra -g3 -I../$(PORTAUDIO_INCLUDEDIR) -I../$(FLAC_INCLUDEDIR) -I../include/ntrb -DFLAC__NO_DLL -DNTRB_MEMDEBUG
LDFLAGS :=
LDLIBS := -L../$(PORTAUDIO_LIBDIR) -L../$(FLAC_LIBDIR) -lportaudio -lFLAC

$(EXECFILE): main.o $(EXTERN_OBJFILES)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@ 
main.o: main.c Makefile

.PHONY: clean
clean:
	rm build.exe
	rm main.o