This directory contains two tools, fontrendering and pngconverter that
are part of mxgui.
-------------------------------------------------------------------------------
fontrendering: takes a font in .ttf or .bdf format and produces a set of
C++ look up tables suitable to be stored in the FLASH memory of a
microcontroller. In doing so it has to render truetype font to bitmap, while
bdf fonts are already bitmaps.

pngconverter: takes a .png image and produces a set of C++ look up tables
suitable to be stored in the FLASH memory of a microcontroller.

-------------------------------------------------------------------------------
Notes:
The quality of rendered fonts depends heavily on the version of freetype used.
Always check the fonts against the reference version in mxgui/fonts for
regressions.

The recomended version is freetype 2.4.3, with bytecode hinter ENABLED,
since it no longer patented.

For this reason, by default fontrendering is built with that specific
version of freetype. To disable this and use the system's freetype library,
modify CMakeLists.txt
