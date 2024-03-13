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

#define scrollAreaTLPoint Point(listArea.second.x(),listArea.first.y()+buttonHeight)
#define scrollAreaBRPoint Point(listArea.second.x()+buttonHeight,listArea.second.y()-buttonHeight)
#include <utility>
#include <chrono>





using namespace std;

namespace mxgui {

    ScrollingList::ScrollingList(Window* w,Point start, int nItems,int width,int buttonHeight,int itemHeight) : Drawable(w,start,width,nItems*itemHeight)
    {
        this->buttonHeight=buttonHeight;
        this->itemHeight=itemHeight;
        scrolling=false;
        DrawArea da = getDrawArea();
        listArea = DrawArea(da.first,Point(da.second.x()-buttonHeight-1,da.second.y()));
        Point upButtonPoint=Point(listArea.second.x(),listArea.first.y());
        Point downButtonPoint=Point(listArea.second.x(),listArea.second.y()-buttonHeight);
        
        
        up = new ScrollButton(w,upButtonPoint,ScrollButtonType::UP,buttonHeight);
        up->setDownCallback([this](){
            this->upOne();
            scrollingThread = new thread( &ScrollingList::keepScrollingUp, this);
        });
        up->setCallback([this](){
            if(this->scrollingThread!=nullptr)
            {
                this->scrollingThread->join();
                delete this->scrollingThread;
                this->scrollingThread=nullptr;
            }
        });
        scroll = new ScrollButton(w,DrawArea(scrollAreaTLPoint,scrollAreaBRPoint),ScrollButtonType::SCROLL);
        down = new ScrollButton(w,downButtonPoint,ScrollButtonType::DOWN,buttonHeight);
        down->setDownCallback([this](){
            this->downOne();
            thread{ &ScrollingList::keepScrollingDown, this}.detach();  
        });
        down->setCallback([this](){
            if(this->scrollingThread!=nullptr)
            {
                this->scrollingThread->join();
                delete this->scrollingThread;
                this->scrollingThread=nullptr;
            }
        });
        items = vector<string>();
        visibleItems=vector<ItemLabel*>();
        for(int i=0;i<nItems;i++)
        {
            ItemLabel *l = new ItemLabel(w,Point(listArea.first.x(),da.first.y()+i*itemHeight),listArea.second.x()-listArea.first.x()-1,itemHeight);
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
    void ScrollingList::keepScrollingUp()
    {
        int i;
        while(i<500)
        {
            if(!up->isPressed())
            {
                return;
            }
            i++;
            this_thread::sleep_for(chrono::milliseconds(1));
        }
        
        
        while(up->isPressed())
        {
            upOne();
            i=0;
            while(i<100)
            {
                if(!up->isPressed())
                {
                    return;
                }
                i++;
                this_thread::sleep_for(chrono::milliseconds(1));
            }
            
        }
    }

    void ScrollingList::keepScrollingDown()
    {
        int i;
        while(i<500)
        {
            if(!down->isPressed())
            {
                return;
            }
            i++;
            this_thread::sleep_for(chrono::milliseconds(1));
        }
        
        
        while(down->isPressed())
        {
            downOne();
            i=0;
            while(i<100)
            {
                if(!down->isPressed())
                {
                    return;
                }
                i++;
                this_thread::sleep_for(chrono::milliseconds(1));
            }
            
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
        if(scrolling)
        {
            if(e.getEvent()==EventType::TouchUp)
            {
                scrolling=false;
            }
            else if(e.getEvent()==EventType::TouchMove)
            {
                int mindiff=(scrollAreaBRPoint.y()-scrollAreaTLPoint.y())/items.size();
                if(e.getPoint().y()-startY>=mindiff)
                {
                    startY=e.getPoint().y();
                    downOne();
                }
                else if(e.getPoint().y()-startY<=-mindiff)
                {
                    startY=e.getPoint().y();
                    upOne();
                }
            }
        }
        else if(this->checkArea(e,listArea))
        {
            if(e.getEvent()==EventType::TouchUp)
            {
                for(ItemLabel* l : visibleItems)
                {
                    if(checkArea(e,l->readDrawArea()))
                    {
                        selectItem(l->getText());
                    }
                }
            }
            return;
        }else if(this->checkArea(e,scroll->readDrawArea()))
        {
            
            if(e.getEvent()==EventType::TouchDown)
            {
                startY=e.getPoint().y();
                scrolling=true;
            }
            
            return;
        }
        else if(this->checkArea(e,DrawArea(Point(up->readDrawArea().first.x(),up->readDrawArea().second.y()),Point(scroll->readDrawArea().second.x(),scroll->readDrawArea().first.y()))))
        {
            if(e.getEvent()==EventType::TouchUp)
            {
                pageUp();
            }
            return;
        }
        else if(this->checkArea(e,DrawArea(Point(scroll->readDrawArea().first.x(),scroll->readDrawArea().second.y()),Point(down->readDrawArea().second.x(),down->readDrawArea().first.y()))))
        {
            if(e.getEvent()==EventType::TouchUp)
            {
                pageDown();
            }
            return;
        }
        
        
        
    }

    void ScrollingList::pageDown()
    {
        int temp=firstVisibleIndex+visibleItems.size();
        if(temp>items.size()-visibleItems.size())
            temp=items.size()-visibleItems.size();
            
        firstVisibleIndex=temp;
        updateScrollButton();
        enqueueForRedraw();
    }

    void ScrollingList::pageUp()
    {
        int temp=firstVisibleIndex-visibleItems.size();
        if(temp<0)
            temp=0;
        firstVisibleIndex=temp;
        updateScrollButton();
        enqueueForRedraw();
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
            int scrollHeight = (scrollAreaBRPoint.y()-scrollAreaTLPoint.y())*visibleItems.size()/items.size();
            int scrollY = (scrollAreaBRPoint.y()-scrollAreaTLPoint.y())*firstVisibleIndex/items.size();
            
            scroll->setDrawArea(DrawArea(Point(scrollAreaTLPoint.x(),scrollAreaTLPoint.y()+scrollY),Point(scrollAreaBRPoint.x(),scrollAreaTLPoint.y()+scrollY+scrollHeight)));
            scroll->enqueueForRedraw();
            
        }
    }

    


}//namespace mxgui

#endif //MXGUI_LEVEL_2
