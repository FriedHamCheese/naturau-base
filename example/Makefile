EXECFILE := ./build.exe
EXTERN_OBJFILES := $(wildcard ../bin/*.o)

CC := gcc
CFLAGS := -Wall -Wextra -g3 -I../include/ntrb
LDFLAGS :=
LDLIBS := -lportaudio

$(EXECFILE): main.o $(EXTERN_OBJFILES)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@ 
main.o: main.c Makefile