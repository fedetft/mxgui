##
## Makefile for mxgui
## TFT:Terraneo Federico Technlogies
## This makefile builds libmxgui.a
##
include ../miosix/config/Makefile.inc

## List of all mxgui source files (both .c and .cpp)
## These files will end up in libmxgui.a
SRC :=                         \
font.cpp                       \
misc_inst.cpp                  \
drivers/display_spfd5408.cpp   \
drivers/display_s6e63d6.cpp    \
drivers/backend_stm32fsmc.cpp  \
drivers/backend_lpc2138spi.cpp

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
