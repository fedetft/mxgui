# mxgui: the Miosix GUI library

Mxgui is a lightweight and configurable GUI library for 32bit microcontrollers.
It can drive monochrome and color displays using microcontrollers with limited
amounts of RAM memory and that cannot in general perform double buffering
because of that. Its target are microcontroller down to 32KB of FLASH and 8KB
of RAM memory.

It is designed to work together with the Miosix kernel, which provide a
threadsafe C and C++ standard library as well as posix threads.

## Getting started

Create a Miosix application project (in tree or out-of-tree), then clone this
repository alongside the `miosix' directory.
You can also use a submodule if you wish.
Then, modify the Makefile to add mxgui as a library to your project by setting
the following variables:

```
INCLUDE_DIRS := -Imxgui
LIBS := mxgui/libmxgui.a
SUBDIRS += mxgui
```

Now, edit mxgui/config/mxgui_settings.h to customize mxgui to your liking.
For most boards that don't have a touchscreen, you will have to disable
level 2 by commenting out the line:

```
#define MXGUI_LEVEL_2
```

For most boards with a screen supported by Miosix, mxgui autoconfigures the
display context with the appropriate driver. Otherwise you have to do so
yourself by providing the following function:

```
namespace mxgui {
void registerDisplayHook(DisplayManager& dm)
{
    dm.registerDisplay(new ...);
}
} //namespace mxgui
```

where you shall pass a new instance of the appropriate display driver to the
registerDisplay method. You can find drivers for common OLED displays found on
the market in the _examples subdirectory.

For more information, refer to the
(Miosix wiki)[https://miosix.org/wiki/index.php] and the documentation of
the mxgui project.
