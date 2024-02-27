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


#include "radio_button.h"
#ifdef MXGUI_LEVEL_2


#include <utility>

using namespace std;

namespace mxgui {
//RadioGroup

RadioGroup::RadioGroup()
{
    radioButtons = list<RadioButton*>();
    checked=nullptr;
}


void RadioGroup::addRadioButton(RadioButton *rb)
{
    if(std::find(radioButtons.begin(), radioButtons.end(), rb) == radioButtons.end())
        radioButtons.push_back(rb);
}

void RadioGroup::setChecked(RadioButton *rb)
{
    for(auto it : radioButtons)
    {
        
        if(it!=rb)
        {
            if(it->isChecked())
            {
                it->setChecked(false);
                it->enqueueForRedraw();
            }
            
        }
        else
        {
            
            if(!it->isChecked())
            {

                checked=rb;
                it->setChecked(true);
                it->enqueueForRedraw();
            }
        }
    }
}
RadioButton* RadioGroup::getChecked()
{
    return checked;
}
//RadioButton

RadioButton::RadioButton(Window *w,RadioGroup *group, Point p, short dimension, const string& text)
    : CheckBox(w,p,dimension,text,false)
{
    this->group=group;
    this->group->addRadioButton(this);
    enqueueForRedraw();
}




void RadioButton::check()
{
    group->setChecked(this);
    enqueueForRedraw();
}
void RadioButton::setChecked(bool checked)
{
    this->checked=checked;
}

string RadioButton::getLabel()
{
    return text->getText();
}

void RadioButton::onDraw(DrawingContextProxy& dc)
{
    DrawArea da=getDrawArea();
    dc.clear(da.first,da.second,colors.second);
    dc.drawImage(da.first,tl);
    dc.drawImage(Point(da.second.x()-2,da.first.y()),tr);
    dc.drawImage(Point(da.first.x(),da.second.y()-2),bl);
    dc.drawImage(innerPointBr,br);
    dc.drawRectangle(innerPointTl,innerPointBr,black);
    if(isChecked())
    {
        dc.clear(innerPointTl,innerPointBr,black);
    }

}



}//namespace mxgui

#endif //MXGUI_LEVEL_2
