##
## Makefile for mxgui
## This makefile builds libmxgui.a
##
## KPATH and CONFPATH are forwarded by the parent Makefile
MAKEFILE_VERSION := 1.13
include $(KPATH)/Makefile.kcommon

## List of all mxgui source files (both .c and .cpp)
## These files will end up in libmxgui.a
SRC :=                                 \
display.cpp                            \
font.cpp                               \
misc_inst.cpp                          \
tga_image.cpp                          \
resourcefs.cpp                         \
resource_image.cpp                     \
textbox.cpp                            \
_examples/hello/hello_world.cpp        \
level2/input.cpp                       \
level2/application.cpp                 \
level2/drawing_context_proxy.cpp       \
level2/label.cpp                       \
level2/button.cpp                      \
level2/checkbox.cpp                    \
level2/radio_button.cpp                \
level2/scrolling_list.cpp              \
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
drivers/display_stm3220g-eval.cpp      \
drivers/event_stm3220g-eval.cpp

CFLAGS   += -DMXGUI_LIBRARY
CXXFLAGS += -DMXGUI_LIBRARY

all: $(OBJ)
	$(ECHO) "[AR  ] libmxgui.a"
	$(Q)$(AR) rcs libmxgui.a $(OBJ)

clean:
	rm -f $(OBJ) $(OBJ:.o=.d) libmxgui.a

-include $(OBJ:.o=.d)
