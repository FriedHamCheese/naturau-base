SRCDIR := ./src
HEADERDIR := $(SRCDIR)

OBJDIR := ./bin

SRCFILES := $(wildcard $(SRCDIR)/*.c)
OBJFILES := $(SRCFILES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

CC := cc
CFLAGS := -Wall -Wextra -g3 -I./include -I./include/ntrb -I../flac-1.4.3/include -DFLAC__NO_DLL -DNTRB_MEMDEBUG
LDFLAGS :=
LDLIBS :=  -lportaudio -L../flac-1.4.3/bin/src/libFLAC -lFLAC

.PHONY: all
all: build build_test

.PHONY: path_debug
path_debug:
	@echo
	@echo SRCDIR: $(SRCDIR)
	@echo SRCFILES: $(SRCFILES)
	@echo
	@echo OBJDIR: $(OBJDIR)
	@echo OBJFILES: $(OBJFILES)
	
	@echo
	@echo TEST_DIR: $(TEST_DIR)
	@echo TEST_SRCFILES: 
	@echo $(TEST_SRCFILES)
	@echo
	@echo TEST_OBJFILES: 
	@echo $(TEST_OBJFILES)
	

.PHONY: build
build: $(OBJFILES)

%.o: %.c
$(OBJDIR)/%.o: $(SRCDIR)/%.c Makefile | $(OBJDIR) 
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir $@
$(EXECDIR):
	mkdir $@

.PHONY:
clean:
	rm -rf $(OBJDIR)
	rm -rf $(TESTEXEC)
	rm -rf $(TEST_OBJDIR)


#Code for the /test folder
TEST_DIR := ./test
TEST_OBJDIR := $(TEST_DIR)/bin
TEST_SRCFILES := $(wildcard $(TEST_DIR)/*.c)
TEST_OBJFILES := $(TEST_SRCFILES:$(TEST_DIR)/%.c=$(TEST_OBJDIR)/%.o)

TESTEXEC := $(TEST_DIR)/test.exe

build_test: $(TESTEXEC)
	$(TESTEXEC)

$(TESTEXEC): $(TEST_OBJFILES) $(OBJFILES)
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)
	
$(TEST_OBJDIR)/%.o: $(TEST_DIR)/%.c Makefile | $(TEST_OBJDIR) 
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST_OBJDIR):
	mkdir $@
	
.PHONY: clean_test
clean_test: 
	rm -rf $(TESTEXEC)
	rm -rf $(TEST_OBJDIR)