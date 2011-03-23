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

#include "misc_inst.h"
#include "fonts/miscfixed.h"
#include "fonts/miscfixed_bold.h"
#include "fonts/tahoma.h"
#include "fonts/droid11.h"
#include "fonts/droid11b.h"
#include "fonts/droid21.h"
#include "fonts/droid21b.h"

namespace mxgui {

#ifdef MXGUI_FONT_MISCFIXED
 const Font miscFixed(miscfixedStartChar,miscfixedEndChar,miscfixedHeight,
         miscfixedWidth,miscfixedDataSize,miscfixedIsAntialiased,miscfixedData);
#ifdef MXGUI_ENABLE_BOLD_FONTS
 const Font miscFixedBold(miscfixedBoldStartChar,miscfixedBoldEndChar,
         miscfixedBoldHeight,miscfixedBoldWidth,miscfixedBoldDataSize,
         miscfixedBoldIsAntialiased,miscfixedBoldData);
#endif //MXGUI_ENABLE_BOLD_FONTS
#endif //MXGUI_FONT_MISCFIXED

#ifdef MXGUI_FONT_DROID11
const Font droid11(droid11StartChar,droid11EndChar,droid11Height,
        droid11DataSize,droid11IsAntialiased,droid11Width,droid11Offset,
        droid11Data);
#ifdef MXGUI_ENABLE_BOLD_FONTS
 const Font droid11b(droid11bStartChar,droid11bEndChar,droid11bHeight,
         droid11bDataSize,droid11bIsAntialiased,droid11bWidth,droid11bOffset,
         droid11bData);
#endif //MXGUI_ENABLE_BOLD_FONTS
#endif //MXGUI_FONT_MISCFIXED

#ifdef MXGUI_FONT_DROID21
const Font droid21(droid21StartChar,droid21EndChar,droid21Height,
        droid21DataSize,droid21IsAntialiased,droid21Width,droid21Offset,
        droid21Data);
#ifdef MXGUI_ENABLE_BOLD_FONTS
 const Font droid21b(droid21bStartChar,droid21bEndChar,droid21bHeight,
         droid21bDataSize,droid21bIsAntialiased,droid21bWidth,droid21bOffset,
         droid21bData);
#endif //MXGUI_ENABLE_BOLD_FONTS
#endif //MXGUI_FONT_DROID21

#ifdef MXGUI_FONT_TAHOMA
const Font tahoma(tahomaStartChar,tahomaEndChar,tahomaHeight,tahomaDataSize,
         tahomaIsAntialiased,tahomaWidth,tahomaOffset,tahomaData);
#endif //MXGUI_FONT_TAHOMA

#ifdef MXGUI_COLOR_DEPTH_1_BIT
const Color white(true);
const Color black(false);
#elif defined(MXGUI_COLOR_DEPTH_8_BIT)
const Color white(0xff);
const Color black(0x00);
const Color grey(0x92);
const Color red(0xe0);
const Color green(0x1c);
const Color blue(0x03);
#elif defined(MXGUI_COLOR_DEPTH_16_BIT)
const Color white(0xffff);
const Color black(0x0000);
const Color grey(0xf7de);
const Color red(0xf800);
const Color green(0x07e0);
const Color blue(0x001f);
#endif

} // namespace mxgui
