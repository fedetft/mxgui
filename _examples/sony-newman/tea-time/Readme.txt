To compile the firmware for the watch:
======================================

1) Make sure you have downloaded Miosix and mxgui, and installed the ARM compiler.
You can find info here:

miosix.org

Also follow the section 'Downloading additional libraries' to download and configure
the mxgui gui library.

2) The kernel works with many boards, so you have to configure it for the sony smartwatch
Uncomment the line

OPT_BOARD := stm32f205rg_sony-newman

in miosix/config/Makefile.inc, and comment any other line defining OPT_BOARD.
Also, disable filesystem support by commenting out the line

#define WITH_FILESYSTEM

in miosix/config/miosix_settings.h

3) Open a shell in the

mxgui/_tools/code_generators

and type

mkdir build
cd build
cmake ..
make

This compiles the tools required to embed images and fonts into the firmware.
For this to work you need to have CMake, boost and libpng (including headers) installed
on your machine.

4) Copy the content of this directory (source files, images directory and makefile) in
the top level directory, the one with the miosix and mxgui directories.

5) Open a shell in the top level directory, and type 'make'

6) Follow the instructions here

developer.sonymobile.com/services/open-smartwatch-project/how-to-flash-alternative-firmware-to-smartwatch

to put the watch in dfu mode, and type 'make program'

To simulate the code:
=====================

1) edit mxgui/config/mxgui_settings.h to configure the display rsolution:

static const unsigned int SIMULATOR_DISP_HEIGHT=128;
static const unsigned int SIMULATOR_DISP_WIDTH=128;

replace mxgui/_tools/qtsimulator/CMakeLists.txt with the one found in this directory

2) Open a shell in the directory

mxgui/_tools/qtsimulator

the, type:

mkdir build; cd build
cmake ..
make
./qtsimulator

For this to work you need to have CMake and Qt (including header files) installed.
