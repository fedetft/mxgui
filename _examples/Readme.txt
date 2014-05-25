
To run the examples,
copy the content of this directory in the top-level directory:

[ ] //Top level directory
 |
 +-- <the example you chose>
 |
 +-- Makefile
 |
 +--[ ] miosix //Miosix kernel folder, downloaded from http://gitorious.org/miosix/kernel
 |
 +--[ ] mxgui  //Mxgui folder

And modify the makefile so that "SRC" contains the name of the example source file.
Also modify FOO_SRCS in mxgui/_tools/qtsimulator/CMakeFiles.txt if targeting the simulator.
Some examples depend on loading images, these images must be copied to a
microSD or other device that Miosix can mount when testing on Miosix, or be
copied to the mxgui/_tools/qtsimulator/build directory when testing the example
on the simulator
