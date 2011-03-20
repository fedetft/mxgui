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

#ifndef COLOR_H
#define	COLOR_H

#include "mxgui_settings.h"

namespace mxgui {

/**
 * Color class. Colors are immutable except they can be assigned with operator=
 */
template<typename T>
class basic_color
{
public:
    /**
     * Constructor, create an instance of the Color class given its value
     */
    basic_color(T value): colorValue(value) {}

    /**
     * Default constructor. Resulting color is black.
     */
    basic_color(): colorValue(0) {}

    /**
     * \return the color value
     */
    const T value() const { return colorValue; }

    //Uses default copy constructor and operator=

private:
    T colorValue;
};

///Define the Color class, depending on the COLOR_DEPTH constant
#ifdef MXGUI_COLOR_DEPTH_1_BIT
typedef basic_color<bool> Color;//Mainly for consistency
#elif defined(MXGUI_COLOR_DEPTH_8_BIT)
typedef basic_color<unsigned char> Color;
#elif defined(MXGUI_COLOR_DEPTH_16_BIT)
typedef basic_color<unsigned short> Color;
#endif

} // namespace mxgui

#endif //COLOR_H
