/***************************************************************************
 *   Copyright (C) 2022,2024 by Daniele Cattaneo                           *
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

#include "textbox.h"
#include "misc_inst.h"
#include <utility>
#include <cctype>

using namespace mxgui;

typedef short (*GlyphWidthFP)(const Font& f, char c);

static short getFWFGlyphWidth(const Font& f, char c)
{
    return f.getWidth();
}

static short getVWFGlyphWidth(const Font& f, char c)
{
    if(c<f.getStartChar() || c>f.getEndChar()) c=' ';
    return f.getWidths()[c-f.getStartChar()];
}

static inline std::pair<int, short> computeLineEnd_charWrap(const Font& font, GlyphWidthFP getGlyphWidth, const char *p, short maxWidth)
{
    short lineWidth=0;
    int i;
    for(i=0; p[i]!='\0'; i++)
    {
        if(p[i]=='\n') { i++; break; }
        short width=getGlyphWidth(font, p[i]);
        if (lineWidth+width>maxWidth) break;
        lineWidth+=width;
    }
    return std::make_pair(i, lineWidth);
}

static inline std::pair<int, short> computeLineEnd_wordWrap(const Font& font, GlyphWidthFP getGlyphWidth, const char *p, short maxWidth)
{
    const short spaceWidth=getGlyphWidth(font, ' ');
    short lineWidth=0;
    int i=0;
    bool firstWord=true;
    short lastTrailingSpaceWidth=0;
    while(p[i]!='\0')
    {
        if(p[i]=='\n') { i++; break; }
        short wordWidth=0;
        int j;
        for(j=i; p[j]!='\0' && p[j]!=' ' && p[j]!='\n'; j++)
        {
            wordWidth+=getGlyphWidth(font, p[j]);
        }
        if(lineWidth+lastTrailingSpaceWidth+wordWidth>maxWidth) break;
        firstWord=false;
        i=j;
        lineWidth+=wordWidth+lastTrailingSpaceWidth;
        lastTrailingSpaceWidth=0;
        while(p[i]==' ')
        {
            lastTrailingSpaceWidth+=spaceWidth;
            i++;
        }
        if(lineWidth+lastTrailingSpaceWidth>maxWidth) break;
    }
    if(firstWord && i==0) return computeLineEnd_charWrap(font, getGlyphWidth, p, maxWidth);
    return std::make_pair(i, lineWidth);
}

int TextBox::draw(DrawingContext& dc, Point p0, Point p1,
    const char *str, unsigned int options, 
    unsigned short topMargin, unsigned short leftMargin,
    unsigned short rightMargin, unsigned short bottomMargin,
    int scrollY)
{
    unsigned short left=p0.x(), top=p0.y();
    unsigned short right=p1.x(), btm=p1.y();
    const Color bgColor = dc.getBackground();
    if (topMargin>0) dc.clear(Point(left,top),Point(right,top+topMargin-1), bgColor);
    top+=topMargin;
    if (leftMargin>0) dc.clear(Point(left,top), Point(left+leftMargin-1,btm), bgColor);
    left+=leftMargin;
    if (rightMargin>0) dc.clear(Point(right-rightMargin+1,top), Point(right,btm), bgColor);
    right-=rightMargin;
    if (bottomMargin>0) dc.clear(Point(left,btm-bottomMargin+1), Point(right,btm), bgColor);
    btm-=bottomMargin;
    return TextBox::draw(dc, Point(left,top), Point(right,btm), str, options, scrollY);
}

int TextBox::draw(DrawingContext& dc, Point p0, Point p1,
    const char *str, unsigned int options, int scrollY)
{
    int left=p0.x(), top=p0.y(), right=p1.x(), btm=p1.y();
    const Font font = dc.getFont();
    const Color bgColor = dc.getBackground();
    const int lineHeight = font.getHeight();
    const bool withBG = (options & BackgroundMask)==BoxBackground;
    const bool withClip = (options & PartialLinesMask)==ClipPartialLines;

    GlyphWidthFP getGlyphWidth;
    if (font.isFixedWidth()) getGlyphWidth=getFWFGlyphWidth;
    else getGlyphWidth=getVWFGlyphWidth;

    int lineTop=top-scrollY;
    if (withBG && lineTop>top) dc.clear(Point(left,top), Point(right,std::min(lineTop-1,btm)), bgColor);
    const char *p=str;
    int stopY = withClip ? btm : btm-lineHeight;
    while(*p!='\0' && lineTop<=stopY)
    {
        std::pair<int, short> end;
        if ((options&WrapMask)==WordWrap) end=computeLineEnd_wordWrap(font, getGlyphWidth, p, right-left+1);
        else end = computeLineEnd_charWrap(font, getGlyphWidth, p, right-left+1);
        //printf("nchar=%d lineWidth=%d\n", end.first, end.second);

        const int lineBottom=std::min(lineTop+lineHeight, btm+1);
        if(!withClip && lineTop<top && lineBottom>top)
        {
            if (withBG) dc.clear(Point(left,top), Point(right,lineBottom-1), bgColor);
        }
        else if(lineBottom>lineTop && lineBottom>top)
        {
            int realLineTop = std::max(lineTop,top);
            if ((options&AlignmentMask) == LeftAlignment)
            {
                const short textRight=left+end.second-1;
                dc.clippedWrite(Point(left,lineTop), Point(left,realLineTop), Point(textRight,lineBottom-1), p);
                if (withBG && textRight+1<=right) dc.clear(Point(textRight+1,realLineTop), Point(right,lineBottom-1), bgColor);
            }
            if ((options&AlignmentMask) == CenterAlignment)
            {
                const short leftPadding=((right-left)-end.second)/2;
                const short textLeft=left+leftPadding;
                const short rightBoxLeft=textLeft+end.second;
                if (withBG && leftPadding>0) dc.clear(Point(left,realLineTop), Point(textLeft-1,lineBottom-1), bgColor);
                dc.clippedWrite(Point(textLeft,lineTop), Point(textLeft,realLineTop), Point(rightBoxLeft-1,lineBottom-1), p);
                if (withBG && rightBoxLeft<=right) dc.clear(Point(rightBoxLeft,realLineTop), Point(right,lineBottom-1), bgColor);
            }
            if ((options&AlignmentMask) == RightAlignment)
            {
                const short textLeft=right-end.second+1;
                if (withBG && textLeft>left) dc.clear(Point(left,realLineTop), Point(textLeft-1,lineBottom-1), bgColor);
                dc.clippedWrite(Point(textLeft,lineTop), Point(textLeft,realLineTop), Point(right,lineBottom-1), p);
            }
        }
        lineTop=lineBottom;
        p+=end.first;
    }
    if (withBG && lineTop<=btm) dc.clear(Point(left,std::max(top,lineTop)), Point(right,btm), bgColor);
    return p-str;
}
