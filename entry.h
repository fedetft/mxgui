
#ifndef ENTRY_H
#define	ENTRY_H

/**
 * \file entry.h
 * This file exists only to avoid adding a couple of #ifdef in user code to
 * make it work both with Miosix and within the simulator. When compiling for
 * Miosix it includes miosix.h
 */

//
// Include miosix.h only if we are not compiling for the simulator
//
#ifdef _MIOSIX
#include "miosix.h"
#endif //_MIOSIX

/**
 * \ingroup pub_iface
 * Simple macro to work around the fact that under miosix the code starts
 * at main(), while within the mxgui simulator code starts at entryPoint()
 *
 * Use it like this:
 * \code
 * ENTRY()
 * {
 *     //Your main() code
 * }
 * \endcode
 */
#ifdef _MIOSIX
#define ENTRY int main
#else //_MIOSIX
#define ENTRY int entryPoint
#endif //_MIOSIX

#endif //ENTRY_H
