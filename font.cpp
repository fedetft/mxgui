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
#include <cstring>

using namespace std;

namespace mxgui {

//
// Class Font
//

Font::Font(unsigned char startChar, unsigned char endChar, unsigned char height,
        unsigned char width, unsigned char dataSize, bool antialiased,
        const void *data): startChar(startChar), endChar(endChar),
        height(height), width(width), dataSize(dataSize),
        antialiased(antialiased), widths(0), offset(0), data(data) {}

Font::Font(unsigned char startChar, unsigned char endChar, unsigned char height,
        unsigned char dataSize, bool antialiased, const unsigned char *widths,
        const unsigned short *offset, const void *data): startChar(startChar),
        endChar(endChar), height(height), width(0), dataSize(dataSize),
        antialiased(antialiased), widths(widths), offset(offset), data(data) {}

short int Font::calculateLength(const char *s) const
{
    if(isFixedWidth())
    {
        return strlen(s)*width;
    } else {
        short int result=0;
        while(*s!='\0')
        {
            const char c=*s;
            if(c<startChar || c>endChar) result+=widths[0];//Width of startchar
            else result+=widths[c-startChar];
            s++;
        }
        return result;
    }
}

void Font::generatePalette(Color out[4], Color fgcolor, Color bgcolor)
{
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
}

}  //namespace mxgui
