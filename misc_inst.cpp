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
#include "fonts/tahoma.h"
#include "fonts/droid11.h"
#include "fonts/droid11b.h"
#include "fonts/droid21.h"
#include "fonts/droid21b.h"
#include "fonts/miscfixed.h"
#include "fonts/miscfixed_bold.h"

namespace mxgui {

#ifdef MXGUI_FONT_MISCFIXED
const Font miscFixed(miscfixedBlocks,miscfixedNumBlocks,miscfixedHeight,
         miscfixedWidth,miscfixedIsAntialiased,miscfixedDataSize,
         miscfixedData);
#ifdef MXGUI_ENABLE_BOLD_FONTS
const Font miscFixedBold(miscfixedBoldBlocks,miscfixedBoldNumBlocks,
         miscfixedBoldHeight,miscfixedBoldWidth,miscfixedBoldIsAntialiased,
         miscfixedBoldDataSize,miscfixedBoldData);
#endif //MXGUI_ENABLE_BOLD_FONTS
#endif //MXGUI_FONT_MISCFIXED

#ifdef MXGUI_FONT_DROID11
const Font droid11(droid11Blocks,droid11NumBlocks,droid11Height,droid11Offset,
        droid11IsAntialiased,droid11DataSize,droid11Data);
#ifdef MXGUI_ENABLE_BOLD_FONTS
const Font droid11b(droid11bBlocks,droid11bNumBlocks,droid11bHeight,droid11bOffset,
        droid11bIsAntialiased,droid11bDataSize,droid11bData);
#endif //MXGUI_ENABLE_BOLD_FONTS
#endif //MXGUI_FONT_MISCFIXED

#ifdef MXGUI_FONT_DROID21
const Font droid21(droid21Blocks,droid21NumBlocks,droid21Height,droid21Offset,
        droid21IsAntialiased,droid21DataSize,droid21Data);
#ifdef MXGUI_ENABLE_BOLD_FONTS
const Font droid21b(droid21bBlocks,droid21bNumBlocks,droid21bHeight,droid21bOffset,
        droid21bIsAntialiased,droid21bDataSize,droid21bData);
#endif //MXGUI_ENABLE_BOLD_FONTS
#endif //MXGUI_FONT_DROID21

#ifdef MXGUI_FONT_TAHOMA
const Font tahoma(tahomaBlocks,tahomaNumBlocks,tahomaHeight,tahomaOffset,
        tahomaIsAntialiased,tahomaDataSize,tahomaData);
#endif //MXGUI_FONT_TAHOMA

} // namespace mxgui
