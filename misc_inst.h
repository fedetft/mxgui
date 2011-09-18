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

/**
 * \file misc_inst.h
 * This file contains some constants and type instantiations, such as
 * default colors such as white and black, and font types such as droid11()
 */

#ifndef MISC_INST_H
#define	MISC_INST_H

#include "color.h"
#include "font.h"

namespace mxgui {

/**
 * \addtogroup pub_iface
 * \{
 */

#ifdef MXGUI_FONT_MISCFIXED
/// \hideinitializer
/// MiscFixed, an 8x16 fixed width font
extern const Font miscFixed;
#ifdef MXGUI_ENABLE_BOLD_FONTS
/// \hideinitializer
/// MiscFixed bold, an 8x16 fixed width font
extern const Font miscFixedBold;
#endif //MXGUI_ENABLE_BOLD_FONTS
#endif //MXGUI_FONT_MISCFIXED

#ifdef MXGUI_FONT_DROID11
/// \hideinitializer
/// Droid 11, a variable width antialiased font
extern const Font droid11;
#ifdef MXGUI_ENABLE_BOLD_FONTS
/// \hideinitializer
/// Droid 11 bold, a variable width antialiased font
extern const Font droid11b;
#endif //MXGUI_ENABLE_BOLD_FONTS
#endif //MXGUI_FONT_MISCFIXED

#ifdef MXGUI_FONT_DROID21
/// \hideinitializer
/// Droid 21, a variable width antialiased font
extern const Font droid21;
#ifdef MXGUI_ENABLE_BOLD_FONTS
/// \hideinitializer
/// Droid 21 bold, a variable width antialiased font
extern const Font droid21b;
#endif //MXGUI_ENABLE_BOLD_FONTS
#endif //MXGUI_FONT_DROID21

#ifdef MXGUI_FONT_TAHOMA
/// \hideinitializer
/// Tahoma, a variable width font
extern const Font tahoma;
#endif //MXGUI_FONT_TAHOMA

#ifdef MXGUI_COLOR_DEPTH_1_BIT
const Color white(1);         ///< White color constant
const Color black(0);         ///< Black color constant
#elif defined(MXGUI_COLOR_DEPTH_8_BIT)
const Color white(0xff);      ///< White color constant
const Color black(0x00);      ///< Black color constant
const Color grey(0x92);       ///< Grey color constant
const Color red(0xe0);        ///< Red color constant
const Color green(0x1c);      ///< Green color constant
const Color blue(0x03);       ///< Blue color constant
#elif defined(MXGUI_COLOR_DEPTH_16_BIT)
const Color white(0xffff);    ///< White color constant
const Color black(0x0000);    ///< Black color constant
const Color darkGrey(0x4208); ///< Dark grey color constant
const Color grey(0x8410);     ///< Grey color constant
const Color lightGrey(0xc618);///< Light grey color constant
const Color red(0xf800);      ///< Red color constant
const Color green(0x07e0);    ///< Green color constant
const Color blue(0x001f);     ///< Blue color constant
#endif

/**
 * \}
 */

} // namespace mxgui

#endif //MISC_INST_H
