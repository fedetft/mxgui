/***************************************************************************
 *   Copyright (C) 2013 by Terraneo Federico                               *
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

#include "simple_button.h"

#ifdef MXGUI_LEVEL_2

using namespace std;
using namespace mxgui;

//
// class SimpleImageButton
//

SimpleImageButton::SimpleImageButton(const Image *imageNotClicked,
            const Image *imageClicked, Point place)
            : SimpleButton(place,Point(place.x()+imageNotClicked->getWidth(),
              place.y()+imageNotClicked->getHeight())),
              imageNotClicked(imageNotClicked), imageClicked(imageClicked) {}

void SimpleImageButton::draw(DrawingContext& dc)
{
    dc.drawImage(a,down ? *imageClicked : *imageNotClicked);
}

//
// class SimpleTextButton
//

SimpleTextButton::SimpleTextButton(Point a, Point b,
            std::pair<Color,Color> colors, const std::string& caption)
            : SimpleButton(a,b), colors(colors), caption(caption) {}

void SimpleTextButton::draw(DrawingContext& dc)
{
    //TODO: improve this
    dc.drawRectangle(a,b,colors.first);
    short x=(b.x()-a.x()-dc.getFont().calculateLength(caption.c_str()))/2;
    x=std::max(a.x()+2,a.x()+x);
    short y=(b.y()-a.y()-dc.getFont().getHeight())/2;
    y=std::max(a.y()+2,a.y()+y);
    Point upper(x,y);
    Point lower(b.x()-1,b.y()-1);
    pair<Color,Color> temp=dc.getTextColor();
    dc.setTextColor(colors);
    dc.clippedWrite(upper,upper,lower,caption.c_str());
    dc.setTextColor(temp);
}

#endif //MXGUI_LEVEL_2
