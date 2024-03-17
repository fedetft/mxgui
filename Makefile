##
## Makefile for mxgui
## This makefile builds libmxgui.a
##
MAKEFILE_VERSION := 1.10
GCCMAJOR := $(shell arm-miosix-eabi-gcc --version | \
                    perl -e '$$_=<>;/\(GCC\) (\d+)/;print "$$1"')
## KPATH and CONFPATH are forwarded by the parent Makefile
include $(CONFPATH)/config/Makefile.inc

## List of all mxgui source files (both .c and .cpp)
## These files will end up in libmxgui.a
SRC :=                                 \
display.cpp                            \
font.cpp                               \
misc_inst.cpp                          \
tga_image.cpp                          \
resourcefs.cpp                         \
resource_image.cpp                     \
level2/input.cpp                       \
level2/application.cpp                 \
level2/drawing_context_proxy.cpp       \
level2/label.cpp                       \
level2/simple_plot.cpp                 \
drivers/display_stm3210e-eval.cpp      \
drivers/event_stm3210e-eval.cpp        \
drivers/display_mp3v2.cpp              \
drivers/event_mp3v2.cpp                \
drivers/resfs_mp3v2.cpp                \
drivers/display_strive.cpp             \
drivers/display_st7735.cpp             \
drivers/display_oledboard2.cpp         \
drivers/event_strive.cpp               \
drivers/display_redbull_v2.cpp         \
drivers/event_redbull_v2.cpp           \
drivers/display_bitsboard.cpp          \
drivers/display_sony-newman.cpp        \
drivers/event_sony-newman.cpp          \
drivers/display_stm32f4discovery.cpp   \
drivers/event_stm32f4discovery.cpp     \
drivers/display_generic_1bpp.cpp       \
drivers/display_generic_4bpp.cpp       \
drivers/display_st7735.cpp             \
drivers/display_st25dvdiscovery.cpp    \
drivers/display_stm3220g-eval.cpp

ifeq ("$(VERBOSE)","1")
Q := 
ECHO := @true
else
Q := @
ECHO := @echo
endif

## Replaces both "foo.cpp"-->"foo.o" and "foo.c"-->"foo.o"
OBJ := $(addsuffix .o, $(basename $(SRC)))

## Includes the miosix base directory for C/C++
CXXFLAGS := $(CXXFLAGS_BASE) -I$(CONFPATH) -I$(CONFPATH)/config/$(BOARD_INC) \
            -I. -I$(KPATH) -I$(KPATH)/arch/common -I$(KPATH)/$(ARCH_INC)     \
            -I$(KPATH)/$(BOARD_INC) -DMXGUI_LIBRARY
CFLAGS   := $(CFLAGS_BASE)   -I$(CONFPATH) -I$(CONFPATH)/config/$(BOARD_INC) \
            -I. -I$(KPATH) -I$(KPATH)/arch/common -I$(KPATH)/$(ARCH_INC)     \
            -I$(KPATH)/$(BOARD_INC) -DMXGUI_LIBRARY
AFLAGS   := $(AFLAGS_BASE)
DFLAGS   := -MMD -MP

## Build libmxgui.a
all: $(OBJ)
	$(ECHO) "[AR  ] libmxgui.a"
	$(Q)$(AR) rcs libmxgui.a $(OBJ)

clean:
	rm -f $(OBJ) libmxgui.a $(OBJ:.o=.d)

%.o: %.s
	$(ECHO) "[AS  ] $<"
	$(Q)$(AS)  $(AFLAGS) $< -o $@

%.o : %.c
	$(ECHO) "[CC  ] $<"
	$(Q)$(CC)  $(DFLAGS) $(CFLAGS) $< -o $@

%.o : %.cpp
	$(ECHO) "[CXX ] $<"
	$(Q)$(CXX) $(DFLAGS) $(CXXFLAGS) $< -o $@

#pull in dependecy info for existing .o files
-include $(OBJ:.o=.d)
