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
#include <iostream>

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
        mutableDrawArea = da;
        updateInnerPoints();
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
        DrawArea da = getDrawArea();
        switch(type)
        {
            case ScrollButtonType::UP:
                dc.line(innerPointBl,middleTop,colors.first);
                dc.line(middleTop,innerPointBr,colors.first);
                break;
            case ScrollButtonType::DOWN:
                dc.line(innerPointTl,middleBottom,colors.first);
                dc.line(middleBottom,innerPointTr,colors.first);
                break;
            case ScrollButtonType::SCROLL:
                dc.line(middleRight,middleLeft,colors.first);
                break;
        }
    }
    void setDrawArea(DrawArea da)
    {
        mutableDrawArea = da;
        updateInnerPoints();
        enqueueForRedraw();
    }
    DrawArea readDrawArea()
    {
        return getDrawArea();
    }

protected:
    Point innerPointTr; 
    Point innerPointBl; 
    Point middleTop; 
    Point middleBottom; 
    Point middleLeft; 
    Point middleRight; 
    DrawArea mutableDrawArea;
    DrawArea getDrawArea() const
    {   
        return mutableDrawArea;
    }
    
private:
    ScrollButtonType type;
    
    void updateInnerPoints()
    {
        DrawArea da = getDrawArea();
        this->innerPointTl = Point(da.first.x()+3,da.first.y()+3);
        this->innerPointBr = Point(da.second.x()-3,da.second.y()-3);
        this->innerPointTr = Point(innerPointBr.x(),innerPointTl.y());
        this->innerPointBl = Point(innerPointTl.x(),innerPointBr.y());
        this->middleTop = Point(innerPointTl.x()+(innerPointTr.x()-innerPointTl.x())/2,innerPointTl.y());
        this->middleBottom = Point(innerPointBl.x()+(innerPointBr.x()-innerPointBl.x())/2,innerPointBl.y());
        this->middleLeft = Point(innerPointTl.x(),innerPointTl.y()+(innerPointBl.y()-innerPointTl.y())/2);
        this->middleRight = Point(innerPointTr.x(),innerPointTr.y()+(innerPointBr.y()-innerPointTr.y())/2);
    }
    
};

class ListItem : public Label
{
public:
    ListItem(Window* w,Point start,int width) : Label(w,DrawArea(start,Point(start.x()+width,start.y()+ITEM_HEIGHT)),"")
    {
    }

    DrawArea readDrawArea()
    {
        return getDrawArea();
    }
};



class ScrollingList : public Drawable
{
    public:
        ScrollingList(Window* w,Point start, int nItems,int width);
        
        
        void addItem(const std::string& item);

        virtual void onDraw(DrawingContextProxy& dc);
        virtual void onEvent(Event e);
        void setCallback(std::function<void ()> callback);
    
        std::string getSelected();
    private:
        ScrollButton *up;
        ScrollButton *down;
        ScrollButton *scroll;

        Point scrollButtonTLPoint;
        Point scrollButtonBRPoint;
        DrawArea listArea;
        std::vector<ListItem*> visibleItems;
        std::vector<std::string> items;
        std::string selected;
        int firstVisibleIndex;
        int startY;
        std::function<void ()> callback;

        void selectItem(const std::string& item);
        bool checkArea(Event e,DrawArea da);

        void upOne();
        void downOne();
        void pageDown();
        void pageUp();
        void updateScrollButton();
};

} //namesapce mxgui

#endif //MXGUI_LEVEL_2

#endif //SCROLLINGLIST_H
