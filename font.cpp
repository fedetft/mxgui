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

unsigned int Font::computeVirtualCodepoint(char32_t codepoint) const
{	
	// traverse the ranges until the right one is found
	int i=2;
	// codepoint of the character as if we had one big contiguous range
	unsigned int virtualCodepoint=0;
	while(i<2*(numBlocks-1) && blocks[i]<=codepoint)
	{
		virtualCodepoint+=blocks[i-1];
    	i+=2;
	}

	// we end up in the first range after our character,
	// so need to go back
	char32_t rangeBase=blocks[i-2];
	unsigned short charOffset=codepoint-rangeBase;
	if(rangeBase + charOffset <= blocks[i-2]+blocks[i-1])
		virtualCodepoint+=charOffset;
	else
	    virtualCodepoint=numGlyphs-1;
	
	return virtualCodepoint;
}
	
short int Font::calculateLength(const char *s) const
{
    if(isFixedWidth())
    {
        return strlen(s)*width;
    } else {
        short int result=0;
		char32_t c;
		while((c=miosix::Unicode::nextUtf8(s))!='\0')
        {
			unsigned int vc = computeVirtualCodepoint(c);
			//if(c<startChar || c>endChar) result+=widths[0];//Width of startchar
            //else result+=widths[vc];
			result+=widths[vc];
        }
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
