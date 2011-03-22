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

#ifndef MISC_INST_H
#define	MISC_INST_H

#include "color.h"
#include "font.h"

namespace mxgui {

#ifdef MXGUI_FONT_MISCFIXED
extern const Font miscFixed;
#ifdef MXGUI_ENABLE_BOLD_FONTS
extern const Font miscFixedBold;
#endif //MXGUI_ENABLE_BOLD_FONTS
#endif //MXGUI_FONT_MISCFIXED

#ifdef MXGUI_FONT_DROID11
extern const Font droid11;
#ifdef MXGUI_ENABLE_BOLD_FONTS
extern const Font droid11b;
#endif //MXGUI_ENABLE_BOLD_FONTS
#endif //MXGUI_FONT_MISCFIXED

#ifdef MXGUI_FONT_DROID21
extern const Font droid21;
#ifdef MXGUI_ENABLE_BOLD_FONTS
extern const Font droid21b;
#endif //MXGUI_ENABLE_BOLD_FONTS
#endif //MXGUI_FONT_DROID21

#ifdef MXGUI_FONT_TAHOMA
extern const Font tahoma;
#endif //MXGUI_FONT_TAHOMA

#ifndef MXGUI_COLOR_DEPTH_1_BIT
extern const Color white;
extern const Color black;
extern const Color grey;
extern const Color red;
extern const Color green;
extern const Color blue;
#else //MXGUI_COLOR_DEPTH_1_BIT
extern const Color white;
extern const Color black;
#endif //MXGUI_COLOR_DEPTH_1_BIT

} // namespace mxgui

#endif //MISC_INST_H
