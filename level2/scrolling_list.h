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

#ifndef SCROLLINGLIST_H
#define	SCROLLINGLIST_H
#include "button.h"

#ifdef MXGUI_LEVEL_2
#define BUTTON_HEIGHT 20
#define ITEM_HEIGHT 15
namespace mxgui {
enum class ScrollButtonType
{
    UP,
    DOWN,
    SCROLL
};
class ScrollButton : public Button
{
public:

    ScrollButton(Window *w, DrawArea da, ScrollButtonType type) : Button(w,da)
    {
        this->type=type;
        enqueueForRedraw();
    }

    ScrollButton(Window *w,Point start,int bWidth,int bHeight,ScrollButtonType type) : ScrollButton(w,DrawArea(start,Point(start.x()+bWidth,start.y()+bHeight)),type)
    {
    }


    ScrollButton(Window *w,Point start,ScrollButtonType type) : ScrollButton(w,start,BUTTON_HEIGHT,BUTTON_HEIGHT,type)
    {
    }

    virtual void onDraw(DrawingContextProxy& dc)
    {
        Button::onDraw(dc);
        if(type==ScrollButtonType::UP)
        {
            //draw up
        }
        else if(type==ScrollButtonType::DOWN)
        {
            //draw down
        }
        else if(type==ScrollButtonType::SCROLL)
        {
            //draw line
        }
    }
private:
    ScrollButtonType type;
    
};

class ScrollingList : public Drawable
{
    public:
        ScrollingList(Window* w,Point start, int nItems,int width);
        
        
        void addItem(const std::string& item);

        virtual void onDraw(DrawingContextProxy& dc);
        virtual void onEvent(const Event& ev);
        string getSelected();
    private:
        ScrollButton *up;
        ScrollButton *down;
        ScrollButton *scroll;
        std::vector<Label> visibleItems;
        std::vector<std::string> items;
        string selected;
        int firstVisibleIndex;

        void selectItem(const std::string& item);
};

} //namesapce mxgui

#endif //MXGUI_LEVEL_2

#endif //SCROLLINGLIST_H
