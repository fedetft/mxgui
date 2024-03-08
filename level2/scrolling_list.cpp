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
#include <iostream>

using namespace std;

namespace mxgui {

    ScrollingList::ScrollingList(Window* w,Point start, int nItems,int width) : Drawable(w,start,width,nItems*ITEM_HEIGHT)
    {
        DrawArea da = getDrawArea();
        listArea = DrawArea(da.first,Point(da.second.x()-BUTTON_HEIGHT-1,da.second.y()));
        Point upButtonPoint=Point(listArea.second.x(),listArea.first.y());
        Point downButtonPoint=Point(listArea.second.x(),listArea.second.y()-BUTTON_HEIGHT);
        scrollButtonTLPoint=Point(listArea.second.x(),listArea.first.y()+BUTTON_HEIGHT);
        scrollButtonBRPoint=Point(listArea.second.x()+BUTTON_HEIGHT,listArea.second.y()-BUTTON_HEIGHT);
        
        up = new ScrollButton(w,upButtonPoint,ScrollButtonType::UP);
        up->setCallback([this](){
            this->upOne();
        });
        scroll = new ScrollButton(w,DrawArea(scrollButtonTLPoint,scrollButtonBRPoint),ScrollButtonType::SCROLL);
        down = new ScrollButton(w,downButtonPoint,ScrollButtonType::DOWN);
        down->setCallback([this](){
            this->downOne();
        });
        items = vector<string>();
        visibleItems=vector<ListItem*>();
        for(int i=0;i<nItems;i++)
        {
            ListItem *l = new ListItem(w,Point(listArea.first.x(),da.first.y()+i*ITEM_HEIGHT),listArea.second.x()-listArea.first.x()-1);
            l->setXAlignment(Alignment::LEFT);
            l->setYAlignment(Alignment::CENTER);
            visibleItems.push_back(l);
        }
        
        selected = "";
        firstVisibleIndex=0;
        enqueueForRedraw();

    }

    void ScrollingList::upOne()
    {
        if(firstVisibleIndex>0)
        {
            firstVisibleIndex--;
            updateScrollButton();
            enqueueForRedraw();
        }
    }

    void ScrollingList::downOne()
    {
        if(firstVisibleIndex+visibleItems.size()<items.size())
        {
            firstVisibleIndex++;
            updateScrollButton();
            enqueueForRedraw();
        }
    }

    void ScrollingList::addItem(const string& text)
    {
        items.push_back(text);
        updateScrollButton();
        enqueueForRedraw();
    }

    void ScrollingList::onDraw(DrawingContextProxy& dc)
    {
        DrawArea da = getDrawArea();
        dc.clear(da.first,da.second,grey);
        for(int index=0;index<visibleItems.size() ;index++)
        {
            string item; 
            Label* curr = visibleItems.at(index);
            if(index+firstVisibleIndex<items.size())
            {
                item = items.at(firstVisibleIndex+index);
            }
            else
            {
                item = "";
            }
            curr->setText(item);
            
            if(item==selected && selected!="")
            {
                curr->setColors(pair<Color,Color>(white,blue));
            }
            else
            {
                curr->setColors(pair<Color,Color>(black,white));
            }
        }
        
        up->enqueueForRedraw();
        scroll->enqueueForRedraw();
        down->enqueueForRedraw();
        
    }

    bool ScrollingList::checkArea(Event e,DrawArea da)
    {
        
        return within(e.getPoint(),da.first,da.second);
    }

    void ScrollingList::onEvent(Event e)
    {
        if(this->checkArea(e,listArea))
        {
            if(e.getEvent()==EventType::TouchUp)
            {
                for(ListItem* l : visibleItems)
                {
                    if(checkArea(e,l->readDrawArea()))
                    {
                        cout<<"Selected: "<<l->getText()<<endl;
                        selectItem(l->getText());
                    }
                }
            }
            return;
        }
        
    }
    string ScrollingList::getSelected()
    {
        return selected;
    }

    void ScrollingList::selectItem(const string& item)
    {
        if(item!="")
        {
            selected = item;
            enqueueForRedraw();
            if(callback) callback();
        }
    }

    void ScrollingList::setCallback(function<void()> callback)
    {
        swap(this->callback,callback);
    }

    void ScrollingList::updateScrollButton()
    {
        if(items.size()>visibleItems.size())
        {
            int scrollHeight = (scrollButtonBRPoint.y()-scrollButtonTLPoint.y())*visibleItems.size()/items.size();
            int scrollY = (scrollButtonBRPoint.y()-scrollButtonTLPoint.y())*firstVisibleIndex/items.size();
            
            scroll->setDrawArea(DrawArea(Point(scrollButtonTLPoint.x(),scrollButtonTLPoint.y()+scrollY),Point(scrollButtonBRPoint.x(),scrollButtonTLPoint.y()+scrollY+scrollHeight)));
            scroll->enqueueForRedraw();
            
        }
    }
    


}//namespace mxgui

#endif //MXGUI_LEVEL_2
