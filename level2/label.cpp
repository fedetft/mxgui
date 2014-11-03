/***************************************************************************
 *   Copyright (C) 2014 by Terraneo Federico                               *
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

#include "label.h"
#include <utility>

using namespace std;

namespace mxgui {

Label::Label(Window* w, DrawArea da, const string& text)
    : Drawable(w,da), text(text), xAlign(Alignment::LEFT),
      yAlign(Alignment::CENTER), overrideColors(false)
{
    enqueueForRedraw();
}

Label::Label(Window *w, Point p, short width, short height, const string& text)
    : Drawable(w,p,width,height), text(text), xAlign(Alignment::LEFT),
      yAlign(Alignment::CENTER), overrideColors(false)
{
    enqueueForRedraw();
}

void Label::setColors(std::pair<Color,Color> colors)
{
    overrideColors=true;
    this->colors=colors;
    enqueueForRedraw();
}

void Label::onDraw(DrawingContextProxy& dc)
{
    DrawArea da=getDrawArea();
    Font font=dc.getFont();
    pair<Color,Color> c;
    if(overrideColors) c=colors;
    else c=dc.getTextColor();
    
    //Guiding lines for overdraw check -- begin
//    dc.drawRectangle(Point(da.first.x()-1,da.first.y()-1),
//                     Point(da.second.x()+1,da.second.y()+1),31<<11);
    //Guiding lines for overdraw check -- end
    
    short x=da.first.x();
    short y=da.first.y();
    short dx=da.second.x()-da.first.x()+1;
    short dy=da.second.y()-da.first.y()+1;
    short w=font.calculateLength(text.c_str());
    short h=font.getHeight();
    switch(xAlign)
    {
        case Alignment::RIGHT:
            x+=dx-w; //x can become less than da.first.x(), or even <0
            break;
        case Alignment::CENTER:
            if(dx>w) x+=(dx-w)/2;
            break;
        default: //LEFT
            break;
    }
    switch(yAlign)
    {
        case Alignment::BOTTOM:
            y+=dy-h; //y can become less than da.first.y(), or even <0
            break;
        case Alignment::CENTER:
            if(dy>h) y+=(dy-h)/2;
            break;
        default: //TOP
            break;
    }
    
    pair<Color,Color> temp;
    if(overrideColors)
    {
        temp=dc.getTextColor();
        dc.setTextColor(c);
    }
    dc.clippedWrite(Point(x,y),da.first,da.second,text.c_str());
    if(overrideColors) dc.setTextColor(temp);
    //Fill the contour of the label with the background color
    if(x>da.first.x())
        dc.clear(Point(da.first.x(),y),
                 Point(x-1,min<short>(y+h-1,da.second.y())),
                 c.second);
    if(x+w<=da.second.x())
        dc.clear(Point(x+w,y),
                 Point(da.second.x(),min<short>(y+h-1,da.second.y())),
                 c.second);
    if(y>da.first.y())
        dc.clear(da.first,
                 Point(da.second.x(),y-1),
                 c.second);
    if(y+h<=da.second.y())
        dc.clear(Point(da.first.x(),y+h),
                 Point(da.second.x(),da.second.y()),
                 c.second);
}

} //namespace mxgui
