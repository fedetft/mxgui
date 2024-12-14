/***************************************************************************
 *   Copyright (C) 2010, 2011 by Terraneo Federico                         *
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

#pragma once

#include <config/mxgui_settings.h>

namespace mxgui {

/**
 * This class is designed to behave just like a regular unsigned char, take the same
 * amount of storage and, thanks to inlining, be as fast as dealing with an unsigned char.
 * The reason why it exists is to avoid  Color being a typedef for an unsigned char
 * both when MXGUI_COLOR_DEPTH_1_BIT_LINEAR and MXGUI_COLOR_DEPTH_8_BIT, which would
 * prevent template specialization to handle the 1 bit per pixel color depth
 */
class Color1bitlinear
{
public:
        Color1bitlinear() {} //Uninitialized just like a regular unsigned char
        Color1bitlinear(unsigned char c) : color(c) {} //Not explicit by design
        operator unsigned char() const { return color; }
        Color1bitlinear& operator=(unsigned char c) { color=c; return *this; }
private:
        unsigned char color;
};

///\ingroup pub_iface
///Define the Color type, depending on the COLOR_DEPTH constant
#ifdef MXGUI_COLOR_DEPTH_1_BIT_LINEAR
typedef Color1bitlinear Color; //Only 0 and 1 allowed
#elif defined(MXGUI_COLOR_DEPTH_8_BIT)
typedef unsigned char Color;
#elif defined(MXGUI_COLOR_DEPTH_16_BIT)
typedef unsigned short Color;
#endif

} // namespace mxgui
