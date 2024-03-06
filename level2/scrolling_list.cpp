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


#include "scrolling_list.h"
#ifdef MXGUI_LEVEL_2


#include <utility>

using namespace std;

namespace mxgui {

    ScrollingList::ScrollingList(Window* w,Point start, int nItems,int width) : Drawable(w,start,width,nItems*ITEM_HEIGHT)
    {
        DrawArea da = getDrawArea();
        Point upButtonPoint=Point(da.second.x()-BUTTON_HEIGHT,da.first.y());
        Point downButtonPoint=Point(da.second.x()-BUTTON_HEIGHT,da.second.y()-BUTTON_HEIGHT);
        Point scrollButtonTLPoint=Point(da.second.x()-BUTTON_HEIGHT,da.first.y()+BUTTON_HEIGHT);
        Point scrollButtonBRPoint=Point(da.second.x(),da.second.y()-BUTTON_HEIGHT);

        up = new ScrollButton(w,upButtonPoint,ScrollButtonType::UP);
        scroll = new ScrollButton(w,DrawArea(scrollButtonTLPoint,scrollButtonBRPoint),ScrollButtonType::SCROLL);
        down = new ScrollButton(w,downButtonPoint,ScrollButtonType::DOWN);
        items = vector<string>();
        visibleItems=vector<Label>();
        for(int i=0;i<nItems;i++)
        {
            Label l = Label(w,Point(da.first.x(),da.first.y()+i*ITEM_HEIGHT),width,ITEM_HEIGHT,to_string(i));
            l.setXAlignment(Alignment::LEFT);
            l.setYAlignment(Alignment::CENTER);
            visibleItems.push_back(l);
        }
        
        selected = nullptr;
        firstVisibleIndex=INT32_MAX;

    }

    void ScrollingList::addItem(const string& text)
    {
        //addItem to list
        enqueueForRedraw();
    }

    void ScrollingList::onDraw(DrawingContextProxy& dc)
    {
        DrawArea da = getDrawArea();
        dc.clear(da.first,da.second,white);
        for(int index=firstVisibleIndex;index<items.size();index++)
        {
            string item = items.at(index);
            //highlight selected item
            //draw visible items
        }
        up->enqueueForRedraw();
        scroll->enqueueForRedraw();
        down->enqueueForRedraw();
    }


}//namespace mxgui

#endif //MXGUI_LEVEL_2
