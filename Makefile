SRCDIR := ./src
HEADERDIR := ./include/ntrb

OBJDIR := ./bin

SRCFILES := $(wildcard $(SRCDIR)/*.c)
HEADERFILES := $(wildcard $(HEADERDIR)/*.h)
OBJFILES := $(SRCFILES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

include makeconfig.make

ifneq ($(NTRB_SHARED_LIBS),)
	ifeq ($(NTRB_SHARED_LIBS), WINDOWS)
		C_SHARED_FLAGS := -DNTRB_DLL_EXPORT
		LD_SHARED_FLAGS := -shared -Wl,--out-implib,./bin/libntrb.a
		LD_SHARED_OUTPUT := ./bin/libntrb.dll
	else
		LD_SHARED_FLAGS := -fPIC -shared
		LD_SHARED_OUTPUT := ./bin/libntrb.so
	endif
	LD_OUTPUT := $(LD_SHARED_OUTPUT)
else
	LD_OUTPUT := ./bin/libntrb.a
endif

CC := cc
CFLAGS := -Wall -Wextra -g3 -I$(NTRB_PORTAUDIO_INCLUDE) -I$(NTRB_FLAC_INCLUDE) -I./include -I./include/ntrb $(NTRB_COMPILING_SYMBOLS) $(C_SHARED_FLAGS)
LDFLAGS := 
LDLIBS := -L$(NTRB_PORTAUDIO_LIBDIR) -L$(NTRB_FLAC_LIBDIR) $(NTRB_LIBS)

.PHONY: build
build: $(LD_OUTPUT)

./bin/libntrb.a: $(OBJFILES) Makefile | $(OBJDIR)
	@echo
	$(AR) r $@  $(OBJFILES)

$(LD_SHARED_OUTPUT): $(OBJFILES) Makefile | $(OBJDIR)
	@echo
	$(CC) $(LD_SHARED_FLAGS) -o $@  $(OBJFILES) $(LDLIBS)

%.o: %.c
$(OBJDIR)/%.o: $(SRCDIR)/%.c $(HEADERFILES) Makefile | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir $@
$(EXECDIR):
	mkdir $@

.PHONY:
clean:
	rm -rf $(OBJDIR)
	#rm -rf $(TESTEXEC)
	#rm -rf $(TEST_OBJDIR)


#Code for the /test folder
#TEST_DIR := ./test
#TEST_OBJDIR := $(TEST_DIR)/bin
#TEST_SRCFILES := $(wildcard $(TEST_DIR)/*.c)
#TEST_OBJFILES := $(TEST_SRCFILES:$(TEST_DIR)/%.c=$(TEST_OBJDIR)/%.o)
#
#TESTEXEC := $(TEST_DIR)/test.exe
#
#build_test: $(TESTEXEC)
#	$(TESTEXEC)
#
#$(TESTEXEC): $(TEST_OBJFILES) $(OBJFILES)
#	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)
#	
#$(TEST_OBJDIR)/%.o: $(TEST_DIR)/%.c $(SRCFILES) $(HEADERFILES) Makefile | $(TEST_OBJDIR) 
#	$(CC) $(CFLAGS) -c $< -o $@
#
#$(TEST_OBJDIR):
#	mkdir $@
#	
#.PHONY: clean_test
#clean_test: 
#	rm -rf $(TESTEXEC)
#	rm -rf $(TEST_OBJDIR)
