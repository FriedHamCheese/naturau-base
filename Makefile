#Version: 2024-02-06

SRCDIR := ./src
HEADERDIR := $(SRCDIR)

OBJDIR := ./bin
EXECDIR := .
EXEC := $(EXECDIR)/run.exe

SRCFILE_PATHS := $(wildcard $(SRCDIR)/*.c) $(NATURAU_SRCFILE_PATHS)
HEADERFILE_PATHS := $(wildcard $(HEADERDIR)/*.h) $(NATURAU_HEADERFILE_PATHS)
OBJFILE_PATHS := $(SRCFILE_PATHS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

GCC_FLAGS := -Wall -Wextra -g3
LD_FLAGS := -lportaudio


.PHONY: all
all: build build_test

.PHONY: path_debug
path_debug:
	@echo
	@echo build paths:
	@echo SRCFILE_PATHS: $(SRCFILE_PATHS)
	@echo HEADERFILE_PATHS: $(HEADERFILE_PATHS)
	@echo OBJFILE_PATHS: $(OBJFILE_PATHS)
	
	@echo
	@echo test paths
	@echo TEST_SRC_PATHS: $(TEST_SRC_PATHS)
	@echo TEST_HEADER_PATHS: $(TEST_HEADER_PATHS)
	@echo TEST_OBJ_PATHS: $(TEST_OBJ_PATHS)	
	
	@echo
	@echo test with build paths
	@echo SRCFILE_PATHS_FOR_TESTING: $(SRCFILE_PATHS_FOR_TESTING)
	@echo HEADERFILE_PATHS_FOR_TESTING: $(HEADERFILE_PATHS_FOR_TESTING)
	@echo OBJFILE_PATHS_FOR_TESTING: $(OBJFILE_PATHS_FOR_TESTING)


.PHONY: build
build: $(EXEC) | $(EXECDIR)

%.o: %.c

$(EXEC): $(OBJFILE_PATHS) $(HEADERFILE_PATHS) | $(EXECDIR) Makefile
	gcc $(OBJFILE_PATHS) -o $@ $(LD_FLAGS)
	
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR) $(NATURAU_OBJDIR) Makefile
	gcc -c $< $(GCC_FLAGS) -o $@


$(OBJDIR):
	mkdir $@
$(EXECDIR):
	mkdir $@


#code for build testing
#in a nutshell, compile the original build files and additional build test files
MAIN_SRC := $(SRCDIR)/main.c
#should be same code with OBJFILE_PATHS
MAIN_OBJ := $(MAIN_SRC:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

TESTDIR := ./test
TEST_SRCDIR := $(TESTDIR)
TEST_HEADERDIR := $(TESTDIR)
TEST_OBJDIR := $(TESTDIR)

TEST_EXECDIR := $(TESTDIR)
TEST_EXEC := $(TEST_EXECDIR)/test.exe

TEST_SRC_PATHS := $(wildcard $(TEST_SRCDIR)/*.c)
TEST_HEADER_PATHS := $(wildcard $(TEST_HEADERDIR)/*.h) $(wildcard $(NATURAU_TESTDIR)/*.h)
TEST_OBJ_PATHS := $(TEST_SRC_PATHS:$(TEST_SRCDIR)/%.c=$(TEST_OBJDIR)/%.o)

SRCFILE_PATHS_FOR_TESTING := $(SRCFILE_PATHS) $(TEST_SRC_PATHS)
HEADERFILE_PATHS_FOR_TESTING := $(HEADERFILE_PATHS) $(TEST_HEADER_PATHS)
#we remove MAIN_OBJ from OBJFILE_PATHS_FOR_TESTING, because the TESTDIR will have a file also containing int main
#but for simplicity, we actually compile both MAIN_OBJ and ./test/test_main.o, we just don't use the object file for the former for the test executable.
OBJFILE_PATHS_FOR_TESTING := $(filter-out $(MAIN_OBJ),$(OBJFILE_PATHS)) $(TEST_OBJ_PATHS)


.PHONY: build_test
build_test: $(TEST_EXEC) | $(TEST_EXECDIR)
	$(TEST_EXEC)

$(TEST_EXEC): $(OBJFILE_PATHS_FOR_TESTING) $(HEADERFILE_PATHS_FOR_TESTING)
	gcc $(OBJFILE_PATHS_FOR_TESTING) -o $@ $(LD_FLAGS)

$(TEST_OBJDIR)/%.o: $(TESTDIR)/%.c
	gcc -c $< $(GCC_FLAGS) -o $@
	
$(TEST_OBJDIR):
	mkdir $@

#removed due to TEST_OBJDIR being the same as TEST_EXECDIR, causing an overide warning.
#$(TEST_EXECDIR):
#	mkdir $@


.PHONY:
clean_all: clean clean_test
	
.PHONY:
clean:
	rm -rf $(OBJDIR)
	rm -rf $(EXEC)
	
.PHONY:
clean_test:
	rm $(TEST_OBJ_PATHS)
	rm $(TEST_EXEC)