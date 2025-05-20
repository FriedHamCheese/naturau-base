EXECFILE := ./build.exe
EXTERN_OBJFILES := $(wildcard ../bin/*.o)

include ../makeconfig.make

CC := gcc
CFLAGS := -Wall -Wextra -g3 -I../$(NTRB_PORTAUDIO_INCLUDE) -I../$(NTRB_FLAC_INCLUDE) -I../include/ntrb $(NTRB_COMPILING_SYMBOLS)
LDFLAGS :=
LDLIBS := -L../$(NTRB_PORTAUDIO_LIBDIR) -L../$(NTRB_FLAC_LIBDIR) $(NTRB_LIBS)

$(EXECFILE): main.o $(EXTERN_OBJFILES)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@ 
main.o: main.c Makefile

.PHONY: clean
clean:
	rm build.exe
	rm main.o