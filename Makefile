#Main Makefile
CC := gcc
CFLAGS := -Wall -MMD

#Directories
IDIR := ./include
SDIR := ./src
ODIR := ./obj/release

#Paths
INCLUDE_PATHS := -I$(IDIR)

#Libraries
LIBS := gtk+-3.0
CFLAGS += `pkg-config --cflags $(LIBS)`
LOADLIBES := -Bstatic `pkg-config --libs $(LIBS)`

#Compilation line
COMPILE = $(CC) $(CFLAGS) $(INCLUDE_PATHS)

#FILEs
#---------------Source----------------#
SRCS := $(wildcard $(SDIR)/*.c)

#---------------Object----------------#
OBJS = $(SRCS:$(SDIR)/%.c=$(ODIR)/%.o)
#-------------Dependency--------------#
DEPS = $(SRCS:$(SDIR)/%.c=$(ODIR)/%.d)

# Build main application
all: servidor cliente

servidor: $(OBJS)
	$(COMPILE) $(OBJS) truco_$@.c -o $@ $(LOADLIBES)

cliente: $(OBJS)
	$(COMPILE) $(OBJS) truco_$@.c -o $@ $(LOADLIBES)

# Build main application for debug
debug: CFLAGS += -g -DDEBUG
debug: all


# Include all .d files
-include $(DEPS)

$(ODIR)/%.o: $(SDIR)/%.c
	$(COMPILE) -c $< -o $@ $(LOADLIBES)

.PHONY : clean
clean :
	-rm -f obj/debug/*
	-rm -f obj/release/*

init:
	mkdir -p include
	mkdir -p src
	mkdir -p obj
	mkdir -p obj/windows
	mkdir -p obj/linux

run:
	./$(BIN)
