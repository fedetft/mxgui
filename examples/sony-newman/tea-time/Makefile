##
## Makefile for Miosix np embedded OS
## TFT:Terraneo Federico Technlogies
##
MAKEFILE_VERSION := 1.02
include miosix/config/Makefile.inc

##
## List here subdirectories which contains makefiles
##
SUBDIRS := miosix mxgui

##
## List here your source files (both .s, .c and .cpp)
##
SRC := \
main.cpp \
boot_message.cpp \
time_config.cpp \
power_manager.cpp \
teapot_app.cpp \
mxgui/examples/teapot/bresenham_fsm.cpp \
mxgui/examples/teapot/fps_counter.cpp \
mxgui/examples/teapot/rendering_engine.cpp \
mxgui/examples/teapot/triangle_fsm.cpp \

IMG :=  \
images/batt0.png \
images/batt25.png \
images/batt50.png \
images/batt75.png \
images/batt100.png \
images/batt0c.png \
images/batt25c.png \
images/batt50c.png \
images/batt75c.png \
images/batt100c.png \
images/up.png \
images/upClicked.png \
images/dw.png \
images/dwClicked.png

SRC2 := $(IMG:.png=.cpp)
# Images should be compiled first to prevent missing includes
SRC := $(SRC2) $(SRC)
%.cpp : %.png
	./mxgui/tools/code_generators/build/pngconverter --in $< --depth 16

# This prevents make from deleting the intermediate .cpp files
.PRECIOUS: $(SRC2)

##
## List here additional static libraries with relative path
##
LIBS := mxgui/libmxgui.a

##
## List here additional include directories (in the form -Iinclude_dir)
##
INCLUDE_DIRS :=

##############################################################################
## You should not need to modify anything below                             ##
##############################################################################

## Replaces both "foo.cpp"-->"foo.o" and "foo.c"-->"foo.o"
OBJ := $(addsuffix .o, $(basename $(SRC)))

## Includes the miosix base directory for C/C++
CXXFLAGS  := $(CXXFLAGS_BASE) -I. -Imiosix -Imiosix/arch/common \
             -Imiosix/$(ARCH_INC) -Imiosix/$(BOARD_INC) $(INCLUDE_DIRS)
CFLAGS    := $(CFLAGS_BASE)   -I. -Imiosix -Imiosix/arch/common \
             -Imiosix/$(ARCH_INC) -Imiosix/$(BOARD_INC) $(INCLUDE_DIRS)
AFLAGS    := $(AFLAGS_BASE)
LFLAGS    := $(LFLAGS_BASE)
DFLAGS    := -MMD -MP

LINK_LIBS := $(LIBS) -L./miosix -Wl,--start-group -lmiosix -lstdc++ -lc -lm \
    -lgcc -Wl,--end-group

all: all-recursive main

clean: clean-recursive clean-topdir

program:
	$(PROGRAM_CMDLINE)

all-recursive:
	$(foreach i,$(SUBDIRS),$(MAKE) -C $(i) FOLDER="$(FOLDER) $(i)/" || exit 1;)

clean-recursive:
	$(foreach i,$(SUBDIRS),$(MAKE) -C $(i) FOLDER="$(FOLDER) $(i)/" clean || exit 1;)

clean-topdir:
	-rm -f $(OBJ) main.elf main.hex main.bin main.map $(OBJ:.o=.d)

main: main.elf
	$(CP) -O ihex   main.elf main.hex
	$(CP) -O binary main.elf main.bin
	$(SZ) main.elf

main.elf: $(OBJ) all-recursive
	@ echo "linking"
	$(CXX) $(LFLAGS) -o main.elf $(OBJ) miosix/$(BOOT_FILE) $(LINK_LIBS)

%.o: %.s
	$(AS) $(AFLAGS) $< -o $@

%.o : %.c
	$(CC) $(DFLAGS) $(CFLAGS) $< -o $@

%.o : %.cpp
	$(CXX) $(DFLAGS) $(CXXFLAGS) $< -o $@

#pull in dependecy info for existing .o files
-include $(OBJ:.o=.d)
