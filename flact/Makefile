EXECFILE := ./build.exe
EXTERN_OBJFILES := $(wildcard ../bin/*.o)

CC := gcc
CFLAGS := -Wall -Wextra -g3 -I../include/ntrb -I../../flac-1.4.3/include -DFLAC__NO_DLL
LDFLAGS :=
LDLIBS := -lportaudio -L../../flac-1.4.3/bin/src/libFLAC -lFLAC

$(EXECFILE): main.o $(EXTERN_OBJFILES)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@ 
main.o: main.c Makefile