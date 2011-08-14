
#ifndef ENTRY_H
#define	ENTRY_H

//
// This file exists only to avoid adding a couple of #ifdef in user code to
// make it work both with Miosix and within the simulator
//

//
// Include miosix.h only if we are not compiling for the simulator
//
#ifdef _MIOSIX
#include "miosix.h"
#endif //_MIOSIX

//
// Simple macro to work around the fact that under miosix the code starts
// at main(), while within the mxgui simulator code starts at entryPoint()
//
// Use it like this:
// ENTRY()
// {
//     //Your main() code
// }
//
#ifdef _MIOSIX
#define ENTRY int main
#else //_MIOSIX
#define ENTRY int entryPoint
#endif //_MIOSIX

#endif //ENTRY_H
