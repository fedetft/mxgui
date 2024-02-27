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


#include "button.h"
#ifdef MXGUI_LEVEL_2


#include <utility>
#include <iostream>

using namespace std;

namespace mxgui {

Button::Button(Window* w, DrawArea da, const string& text)
    : InteractableButton(w,da)
{
    this->innerPointTl = Point(da.first.x()+3,da.first.y()+3);
    this->innerPointBr = Point(da.second.x()-3,da.second.y()-3);
    
    this->text=new Label(w,DrawArea(innerPointTl,innerPointBr),text);
    this->text->setXAlignment(Alignment::CENTER);
    this->text->setYAlignment(Alignment::CENTER);
    resetState();
    enqueueForRedraw();
}

Button::Button(Window *w, Point p, short width, short height, const string& text)
    : Button(w,DrawArea(p,Point(p.x()+width,p.y()+height)),text)
{}

void Button::resetState()
{
    colors=make_pair(black,lightGrey);
    text->setColors(colors);
    InteractableButton::resetState();
}
void Button::buttonDown()
{
    colors=make_pair(white,darkGrey);
    text->setColors(colors);    
    enqueueForRedraw();
}

void Button::buttonUp()
{
    resetState();
    InteractableButton::buttonUp();
}

void Button::onDraw(DrawingContextProxy& dc)
{
    cout<<"Button::onDraw "<<colors.first<<", "<<colors.second<<endl;
    DrawArea da=getDrawArea();
    dc.clear(da.first,da.second,colors.second);
    dc.drawImage(da.first,tl);
    dc.drawImage(Point(da.second.x()-2,da.first.y()),tr);
    dc.drawImage(Point(da.first.x(),da.second.y()-2),bl);
    dc.drawImage(innerPointBr,br);

}

void Button::onEvent(Event e)
{
    if(!this->checkEventArea(e))
    {
        colors=make_pair(black,lightGrey);
        text->setColors(colors);
        enqueueForRedraw();
        return;
    }
    InteractableButton::onEvent(e);
}


}//namespace mxgui

#endif //MXGUI_LEVEL_2
