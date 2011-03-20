/***************************************************************************
 *   Copyright (C) 2010 by Terraneo Federico                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   As a special exception, if other files instantiate templates or use   *
 *   macros or inline functions from this file, or you compile this file   *
 *   and link it with other works to produce a work based on this file,    *
 *   this file does not by itself cause the resulting work to be covered   *
 *   by the GNU General Public License. However the source code for this   *
 *   file must still be made available in accordance with the GNU General  *
 *   Public License. This exception does not invalidate any other reasons  *
 *   why a work based on this file might be covered by the GNU General     *
 *   Public License.                                                       *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>   *
 ***************************************************************************/

#ifndef BACKEND_STM32FSMC
#define	BACKEND_STM32FSMC

#include "mxgui/mxgui_settings.h"

#ifdef MXGUI_BACKEND_STM32FSMC

#include "miosix.h"
#include "hwmapping.h"

namespace mxgui {

#ifdef MXGUI_DISPLAY_TYPE_S6E63D6

// This backend is meant to connect an stm32f103ve to an OLED display with
// an s6e63d6 controller on the mp3v2 board.

/**
 * Memory layout of the display
 */
struct DisplayMemLayout
{
    volatile unsigned short IDX;//Index, select register to write
    unsigned char padding[131070];
    volatile unsigned short RAM;//Ram, read and write from registers and GRAM
};

/**
 * Pointer to the memory mapped display.
 */
DisplayMemLayout *const DISPLAY=reinterpret_cast<DisplayMemLayout*>(0x60000000);

#elif defined MXGUI_DISPLAY_TYPE_SPFD5408

// This backend is meant to connect an stm32f103re to an LCD display with
// an spfd5408 controller on the stm3210e_eval board.

/**
 * Memory layout of the display
 */
struct DisplayMemLayout
{
    volatile unsigned short IDX;//Index, select register to write
    volatile unsigned short RAM;//Ram, read and write from registers and GRAM
};

/**
 * Pointer to the memory mapped display.
 */
DisplayMemLayout *const DISPLAY=reinterpret_cast<DisplayMemLayout*>(0x6c000000);

#else
#error this backend does not support this display type
#endif

/**
 * Set the index register
 * \param reg register to select
 */
inline void writeIdx(unsigned char reg)
{
    DISPLAY->IDX=reg;
}

/**
 * Write data to selected register
 * \param data data to write
 */
inline void writeRam(unsigned short data)
{
    DISPLAY->RAM=data;
}

/**
 * Write data from selected register
 * \return data read from register
 */
inline unsigned short readRam()
{
    return DISPLAY->RAM;
}

/**
 * Write data to a display register
 * \param reg which register?
 * \param data data to write
 */
inline void writeReg(unsigned char reg, unsigned short data)
{
    DISPLAY->IDX=reg;
    DISPLAY->RAM=data;
}

/**
 * Read data from a display register
 * \param reg which register?
 * \return data read from register
 */
inline unsigned short readReg(unsigned char reg)
{
    DISPLAY->IDX=reg;
    return DISPLAY->RAM;
}

/**
 * Initializes the hardware backend
 */
void hardwareInit();

} //namespace mxgui

#endif //MXGUI_BACKEND_STM32FSMC

#endif	/* BACKEND_STM32FSMC */

