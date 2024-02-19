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
#include "label.h"
#ifdef MXGUI_LEVEL_2

#include <utility>

using namespace std;

namespace mxgui {

Button::Button(Window* w, DrawArea da, const string& text)
    : Drawable(w,da)
{
    this->innerPointTl = Point(da.first.x()+3,da.first.y()+3);
    this->innerPointBr = Point(da.second.x()-3,da.second.y()-3);
    this->text=new Label(w,DrawArea(innerPointTl,innerPointBr),text);
    enqueueForRedraw();
}

Button::Button(Window *w, Point p, short width, short height, const string& text)
    : Button(w,DrawArea(p,Point(p.x()+width-1,p.y()+height-1)),text)
{
}

void Button::setText(const string& text)
{
    this->text->setText(text);
    enqueueForRedraw();
}

void Button::buttonDown()
{
    //TODO: set the button in the "pressed" state
    clicked=true;
}
void Button::buttonUp()
{
    //TODO: set the button in the "released" state
    clicked=false;
    if(callback) callback(1);
}

void Button::onDraw(DrawingContextProxy& dc)
{
    DrawArea da=getDrawArea();
    //TODO: draw the button
    
}

} //namespace mxgui

#endif //MXGUI_LEVEL_2