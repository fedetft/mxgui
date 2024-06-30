##
## Makefile for Miosix embedded OS
##

## Path to kernel/config directories (edited by init_project_out_of_git_repo.pl)
KPATH := miosix
CONFPATH := $(KPATH)
MAKEFILE_VERSION := 1.15
include $(KPATH)/Makefile.kcommon

##
## List here your source files (both .s, .c and .cpp)
##
SRC := \
main.cpp \
boot_message.cpp \
time_config.cpp \
power_manager.cpp \
teapot_app.cpp \
simple_button.cpp \
mxgui/_examples/teapot/bresenham_fsm.cpp \
mxgui/_examples/teapot/fps_counter.cpp \
mxgui/_examples/teapot/rendering_engine.cpp \
mxgui/_examples/teapot/triangle_fsm.cpp

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
	./mxgui/_tools/code_generators/build/pngconverter --in $< --depth 16

# This prevents make from deleting the intermediate .cpp files
.PRECIOUS: $(SRC2)

##
## List here additional include directories (in the form -Iinclude_dir)
##
INCLUDE_DIRS := -Imxgui

##
## List here additional static libraries with relative path
##
LIBS := mxgui/libmxgui.a

##
## List here subdirectories which contains makefiles
##
SUBDIRS += mxgui

##
## Attach a romfs filesystem image after the kernel
##
ROMFS_DIR :=

all: $(if $(ROMFS_DIR), image, main)

main: $(OBJ) all-recursive
	$(ECHO) "[LD  ] main.elf"
	$(Q)$(CXX) $(LFLAGS) -o main.elf $(OBJ) $(LINK_LIBS)
	$(ECHO) "[CP  ] main.hex"
	$(Q)$(CP) -O ihex   main.elf main.hex
	$(ECHO) "[CP  ] main.bin"
	$(Q)$(CP) -O binary main.elf main.bin
	$(Q)$(SZ) main.elf

clean: clean-recursive
	$(Q)rm -f $(OBJ) $(OBJ:.o=.d) main.elf main.hex main.bin main.map

-include $(OBJ:.o=.d)
