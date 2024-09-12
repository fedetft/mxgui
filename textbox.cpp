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

struct LineEndPos
{
    const char *nextChar;
    short lineWidth;
};

static inline LineEndPos computeLineEnd_charWrap(const Font& font, const char *first, short maxWidth)
{
    short lineWidth=0;
    char32_t c;
    const char *next=first, *cur=first;
    while((c=miosix::Unicode::nextUtf8(next))!='\0')
    {
        // If newline character exit.
        // The first character of the next line is the one after the newline.
        if(c=='\n') return LineEndPos{next, lineWidth};
        // If this character exceeds line width, exit.
        short width=font.calculateLength(c);
        if (lineWidth+width>maxWidth)
        {
            // The first character doesn't fit, make it fit forcefully.
            if(lineWidth==0) return LineEndPos{next, maxWidth};
            // The first character of the next line is the current one.
            return LineEndPos{cur, lineWidth};
        }
        // Not at the end of the line yet
        lineWidth+=width;
        cur=next;
    }
    // End of the string
    return LineEndPos{cur, lineWidth};
}

static inline LineEndPos computeLineEnd_wordWrap(const Font& font, const char *first, short maxWidth)
{
    // Cache the width of a single space for speed
    const short spaceWidth=font.calculateLength(' ');
    short lineWidth=0;
    short lastInterwordSpacing=0;
    bool firstWord=true;
    const char *next=first, *cur=next;
    char32_t c=miosix::Unicode::nextUtf8(next);
    while(c!='\0')
    {
        // If newline character exit, next character is already in p
        if(c=='\n') return LineEndPos{next, lineWidth};
        
        // Compute the length of the next word
        const char *wordStart=cur;
        short wordWidth=0;
        while(c!='\0' && c!=' ' && c!='\n')
        {
            wordWidth+=font.calculateLength(c);
            cur=next;
            c=miosix::Unicode::nextUtf8(next);
        }
        // If this word makes us exceed the end of the line, exit
        if(lineWidth+lastInterwordSpacing+wordWidth>maxWidth) { cur=wordStart; break; }
        lineWidth+=lastInterwordSpacing+wordWidth;
        firstWord=false;
        lastInterwordSpacing=0;

        // Perform char wrap for spaces.
        while(c==' ')
        {
            // When a space exceeds the width of a line, transform it into a
            // newline
            if(lineWidth+lastInterwordSpacing+spaceWidth>maxWidth)
                return LineEndPos{next, (short)(lineWidth+lastInterwordSpacing)};
            lastInterwordSpacing+=spaceWidth;
            cur=next;
            c=miosix::Unicode::nextUtf8(next);
        }
    }

    if(firstWord && cur==first) return computeLineEnd_charWrap(font, first, maxWidth);
    return LineEndPos{cur, lineWidth};
}

const char *TextBox::draw(DrawingContext& dc, Point p0, Point p1,
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

const char *TextBox::draw(DrawingContext& dc, Point p0, Point p1,
    const char *str, unsigned int options, int scrollY)
{
    int left=p0.x(), top=p0.y(), right=p1.x(), btm=p1.y();
    const Font font = dc.getFont();
    const Color bgColor = dc.getBackground();
    const int lineHeight = font.getHeight();
    const bool withBG = (options & BackgroundMask)==BoxBackground;
    const bool withClip = (options & PartialLinesMask)==ClipPartialLines;

    int lineTop=top-scrollY;
    if (withBG && lineTop>top) dc.clear(Point(left,top), Point(right,std::min(lineTop-1,btm)), bgColor);
    const char *cur=str;
    int stopY = withClip ? btm : btm-lineHeight;
    while(*cur!='\0' && lineTop<=stopY)
    {
        LineEndPos end;
        if ((options&WrapMask)==WordWrap) end=computeLineEnd_wordWrap(font, cur, right-left+1);
        else end=computeLineEnd_charWrap(font, cur, right-left+1);
        
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
                const short textRight=left+end.lineWidth-1;
                dc.clippedWrite(Point(left,lineTop), Point(left,realLineTop), Point(textRight,lineBottom-1), cur);
                if (withBG && textRight+1<=right) dc.clear(Point(textRight+1,realLineTop), Point(right,lineBottom-1), bgColor);
            }
            if ((options&AlignmentMask) == CenterAlignment)
            {
                const short leftPadding=((right-left)-end.lineWidth)/2;
                const short textLeft=left+leftPadding;
                const short rightBoxLeft=textLeft+end.lineWidth;
                if (withBG && leftPadding>0) dc.clear(Point(left,realLineTop), Point(textLeft-1,lineBottom-1), bgColor);
                dc.clippedWrite(Point(textLeft,lineTop), Point(textLeft,realLineTop), Point(rightBoxLeft-1,lineBottom-1), cur);
                if (withBG && rightBoxLeft<=right) dc.clear(Point(rightBoxLeft,realLineTop), Point(right,lineBottom-1), bgColor);
            }
            if ((options&AlignmentMask) == RightAlignment)
            {
                const short textLeft=right-end.lineWidth+1;
                if (withBG && textLeft>left) dc.clear(Point(left,realLineTop), Point(textLeft-1,lineBottom-1), bgColor);
                dc.clippedWrite(Point(textLeft,lineTop), Point(textLeft,realLineTop), Point(right,lineBottom-1), cur);
            }
        }
        lineTop=lineBottom;
        cur=end.nextChar;
    }
    if (withBG && lineTop<=btm) dc.clear(Point(left,std::max(top,lineTop)), Point(right,btm), bgColor);
    return cur;
}
