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

#include "checkbox.h"
#ifdef MXGUI_LEVEL_2

#include <utility>

using namespace std;

namespace mxgui {

CheckBox::CheckBox(Window *w, Point p, short dimension, const string& text, bool checked)
    : InteractableButton(w,DrawArea(p,Point(p.x()+dimension,p.y()+dimension)))
{
    int textLen =w->getPreferences().font.calculateLength(text.c_str());
    this->checked=checked;
    this->colors=make_pair(black,lightGrey);
    this->labelStartingPoint = Point(p.x()+dimension+4,p.y());
    this->text=new Label(w,this->labelStartingPoint,textLen,dimension,text);
    this->text->setXAlignment(Alignment::LEFT);
    this->text->setYAlignment(Alignment::CENTER);
    enqueueForRedraw();
}

void CheckBox::resetState()
{
    if(colors!=make_pair(black,lightGrey))
    {
        colors=make_pair(black,lightGrey);
        InteractableButton::resetState();
    }
    
}

void CheckBox::buttonDown()
{
    if(colors!=make_pair(white,darkGrey))
    {
        colors=make_pair(white,darkGrey);
        enqueueForRedraw();
    }
}

void CheckBox::buttonUp()
{
    this->check();
    resetState();
    InteractableButton::buttonUp();
}

void CheckBox::check()
{
    checked=!checked;
}

bool CheckBox::isChecked()
{
    return this->checked;
}

void CheckBox::onDraw(DrawingContextProxy& dc)
{
    DrawArea da=getDrawArea();
    dc.clear(da.first,da.second,colors.second);
    dc.drawImage(da.first,tl);
    dc.drawImage(Point(da.second.x()-2,da.first.y()),tr);
    dc.drawImage(Point(da.first.x(),da.second.y()-2),bl);
    dc.drawImage(innerPointBr,br);
    if(isChecked())
    {
        dc.line(innerPointTl,innerPointBr,black);
        dc.line(Point(innerPointTl.x(),innerPointBr.y()),Point(innerPointBr.x(),innerPointTl.y()),black);
    }
}

} //namespace mxgui

#endif //MXGUI_LEVEL_2
