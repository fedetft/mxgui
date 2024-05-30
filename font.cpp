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

#include "font.h"
#include "misc_inst.h"
#include <cstring>

using namespace std;

namespace mxgui {

//
// Class Font
//

bool Font::isInRange(char32_t c) const
{
    const int lastBlock=2*(numBlocks-1);
    for(int block=0;block<lastBlock;block+=2)
        if(c>=blocks[block] && c<blocks[block]+blocks[block+1]) return true;
    return false;
}

unsigned int Font::getVirtualCodepoint(char32_t codepoint) const
{
    unsigned int virtualCodepoint=0;
    const int lastBlock=2*(numBlocks-1);
    for(int block=0;block<lastBlock;block+=2)
    {
        if(codepoint>=blocks[block] && codepoint<(blocks[block]+blocks[block+1]))
            return virtualCodepoint+codepoint-blocks[block];
        else virtualCodepoint+=blocks[block+1];
    }
    //Last block always only contains the missing codepoint glyph
    return virtualCodepoint;
}

short int Font::calculateLength(const char *s) const
{
    if(isFixedWidth()) return miosix::Unicode::countCodePoints(s)*width;
    else {
        short int result=0;
        while(char32_t c=miosix::Unicode::nextUtf8(s))
            result+=variableWidthGetWidth(getVirtualCodepoint(c));
        return result;
    }
}

void Font::generatePalette(Color out[4], Color fgcolor, Color bgcolor)
{
    #ifdef MXGUI_COLOR_DEPTH_16_BIT
    unsigned short fgR=fgcolor; //& 0xf800; Optimization, & not required
    unsigned short bgR=bgcolor; //& 0xf800; Optimization, & not required
    unsigned short fgG=fgcolor & 0x7e0;
    unsigned short bgG=bgcolor & 0x7e0;
    unsigned short fgB=fgcolor & 0x1f;
    unsigned short bgB=bgcolor & 0x1f;
    unsigned short deltaR=((fgR-bgR)/3) & 0xf800;
    unsigned short deltaG=((fgG-bgG)/3) & 0x7e0;
    unsigned short deltaB=((fgB-bgB)/3) & 0x1f;
    unsigned short delta=deltaR | deltaG | deltaB;
    out[3]=fgcolor;
    out[2]=Color(bgcolor+2*delta);
    out[1]=Color(bgcolor+delta);
    out[0]=bgcolor;
    #elif defined(MXGUI_COLOR_DEPTH_8_BIT)
    #error TODO
    #elif defined(MXGUI_COLOR_DEPTH_1_BIT_LINEAR)
    out[0]=out[1]=bgcolor ? white : black;
    out[2]=out[3]=fgcolor ? white : black;
    #else
    #error unsupported color depth
    #endif
}

}  //namespace mxgui
