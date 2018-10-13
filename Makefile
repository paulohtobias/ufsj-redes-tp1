#Main Makefile
CC := gcc
CFLAGS := -Wall -MMD

#Binary
ifeq ($(OS),Windows_NT)
	BIN := main.exe
	DLE := dll
else
	BIN := main.out
	DLE := so
endif

#Directories
IDIR := ./include
SDIR := ./src

ODIR := ./obj/release

#Files
SOURCE := .c

#Paths
INCLUDE_PATHS := -I$(IDIR)

#Libraries
LIBS := gtk+-3.0
CFLAGS += `pkg-config --cflags $(LIBS)`
LOADLIBES := `pkg-config --libs $(LIBS)`

#Compilation line
COMPILE = $(CC) $(CFLAGS) $(INCLUDE_PATHS)

#FILEs
#---------------Source----------------#
SRCS := $(wildcard $(SDIR)/*$(SOURCE)) $(wildcard $(SDIR)/*/*$(SOURCE))

#---------------Object----------------#
OBJS := $(SRCS:$(SDIR)/%$(SOURCE)=$(ODIR)/%.o)
#-------------Dependency--------------#
DEPS := $(SRCS:$(SDIR)/%$(SOURCE)=$(ODIR)/%.d)

# Build main application
all: $(OBJS)
	$(COMPILE) $(OBJS) main$(SOURCE) -o $(BIN) $(LOADLIBES)

# Build main application
debug: CFLAGS += -g -DDEBUG
debug: $(OBJS)
	$(COMPILE) $(OBJS) main$(SOURCE) -o $(BIN) $(LOADLIBES)

# Build shared library
dll: LOADLIBES += -lm -fPIC
dll: LIB_NAME :=
dll: $(OBJS)
	$(COMPILE) -shared -o lib$(LIB_NAME).$(DLE) $(OBJS) $(LOADLIBES)

# Include all .d files
-include $(DEPS)

$(ODIR)/%.o: $(SDIR)/%$(SOURCE)
	$(COMPILE) -c $< -o $@ $(LOADLIBES)

.PHONY : clean
clean :
	-rm $(BIN) $(OBJS) $(DEPS)

init:
	mkdir -p include
	mkdir -p src
	mkdir -p obj
	mkdir -p obj/windows
	mkdir -p obj/linux

run:
	./$(BIN)
