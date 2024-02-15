SRCDIR := ./src
HEADERDIR := $(SRCDIR)

OBJDIR := ./bin

SRCFILES := $(wildcard $(SRCDIR)/*.c)
OBJFILES := $(SRCFILES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

CC := gcc
GCC_FLAGS := -Wall -Wextra -g3 -I./include -I./include/ntrb
LD_FLAGS := -lportaudio


.PHONY: all
all: build

.PHONY: path_debug
path_debug:
	@echo
	@echo SRCDIR: $(SRCDIR)
	@echo SRCFILES: $(SRCFILES)
	@echo
	@echo OBJDIR: $(OBJDIR)
	@echo OBJFILES: $(OBJFILES)


.PHONY: build
build: $(OBJFILES)

%.o: %.c
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR) Makefile
	$(CC) -c $< $(GCC_FLAGS) -o $@


$(OBJDIR):
	mkdir $@
$(EXECDIR):
	mkdir $@
	
	
.PHONY:
clean:
	rm -rf $(OBJDIR)
