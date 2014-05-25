Using QT and BOOST to build Windows MXGUI simulator is a big overhead.
My intention was to create simulation environment that uses freely
available bare Microsoft Visual Studio Express 2010 and
no need to install anything else.

For now, "pthread.h" is implemented very basically. Actually, only
mutex is supported. But it is more than enough currently.
Nevertheles, in the future I plan to use well-implemented
pthread-win32 library from http://sourceware.org/pthreads-win32/.

I hope this simple simulator will help in designing MXGUI
applications without the need to use real boards.

The limitations:
* Currently only MXGUI_COLOR_DEPTH_16_BIT is supported.
* MXGUI_ORIENTATION_VERTICAL and MXGUI_ORIENTATION_HORIZONTAL
  are supported, of course, if application supports them.
  But you should take care of properly setting 
  SIMULATOR_DISP_HEIGHT and SIMULATOR_DISP_WIDTH in mxgui_settings.h
* Only Debug configuration is supported.
* __attribute__((packed)) is not understood by MSVC compiler.
  Therefore it is undefined, and member alignment for all structures
  is explicitly set to 1 for the whole project.

To build example MXGUI application, open solution (.sln) in Visual Studio.
In the Solution Explorer open filter "mxgui/Examples/<example_name>",
select all files, right-click the selection, hit "Properties",
unfold  "Configuration Properties", "General", and select "No"
for "Excluded from build" parameter. Take into account that only
one example can be built, so you must exclude all unused examples
using similar steps.

Then build solution and run right from Visual Studio.

NB: To run ClippedImage example copy "dis.tga" file to
    mxgui/_tools/winsimulator

-- 
Yury Kuchura kuchura@gmail.com
December 8, 2011
