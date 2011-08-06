##
## Makefile for mxgui
## TFT:Terraneo Federico Technlogies
## This makefile builds libmxgui.a
##
include ../miosix/config/Makefile.inc

## List of all mxgui source files (both .c and .cpp)
## These files will end up in libmxgui.a
SRC :=                              \
font.cpp                            \
misc_inst.cpp                       \
input.cpp                           \
drivers/display_stm3210e-eval.cpp   \
drivers/display_mp3v2.cpp           \
drivers/event_mp3v2.cpp

## Replaces both "foo.cpp"-->"foo.o" and "foo.c"-->"foo.o"
OBJ := $(addsuffix .o, $(basename $(SRC)))

## Includes the miosix base directory for C/C++
CXXFLAGS := $(CXXFLAGS_BASE) -I.. -I../miosix -I../miosix/$(ARCH_INC) \
    -I../miosix/$(BOARD_INC) -DMXGUI_LIBRARY
CFLAGS   := $(CFLAGS_BASE)   -I.. -I../miosix -I../miosix/$(ARCH_INC) \
    -I../miosix/$(BOARD_INC) -DMXGUI_LIBRARY
AFLAGS   := $(AFLAGS_BASE)

## Build libmxgui.a
all: $(OBJ)
	$(AR) rcs libmxgui.a $(OBJ)

clean:
	rm $(OBJ) libmxgui.a

%.o: %.s
	$(AS) $(AFLAGS) $< -o $@

%.o : %.c
	$(CC) $(CFLAGS) $< -o $@

%.o : %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@
