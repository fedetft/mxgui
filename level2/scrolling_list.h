/***************************************************************************
 *   Copyright (C) 2024 by Aaron Tognoli                                   *
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

#pragma once

#include "button.h"
#include <thread>

#define innerPointTr Point(innerPointBr.x(),innerPointTl.y())
#define innerPointBl Point(innerPointTl.x(),innerPointBr.y())
#define middleTop Point(innerPointTl.x()+(innerPointTr.x()-innerPointTl.x())/2,innerPointTl.y())
#define middleBottom Point(innerPointBl.x()+(innerPointBr.x()-innerPointBl.x())/2,innerPointBl.y())
#define middleLeft Point(innerPointTl.x(),innerPointTl.y()+(innerPointBl.y()-innerPointTl.y())/2)
#define middleRight Point(innerPointTr.x(),innerPointTr.y()+(innerPointBr.y()-innerPointTr.y())/2)

#ifdef MXGUI_LEVEL_2

namespace mxgui {

/**
 * Types of ScrollButton
 */
enum class ScrollButtonType
{
    UP,
    DOWN,
    SCROLL
};

/**
 * Class for the buttons of the scrollbar of the ScrollingList
*/
class ScrollButton : public Button
{
public:
    /**
     * Constructor
     * mutableDrawArea used to allow the button to be moved
     * Updated inner points for easier drawing
     * \param w window to which this object belongs
     * \param da area on screen occupied by this object
     * \param type type of the button
     */
    ScrollButton(Window *w, DrawArea da, ScrollButtonType type) : Button(w,da)
    {
        this->type=type;
        mutableDrawArea = da;
        updateInnerPoints();
        enqueueForRedraw();
    }

    /**
     * Constructor
     * \param w window to which this object belongs
     * \param start upper left point of the button
     * \param bWidth width of the button
     * \param bHeight height of the button
     * \param type type of the button
     */
    ScrollButton(Window *w,Point start,int bWidth,int bHeight,ScrollButtonType type) : ScrollButton(w,DrawArea(start,Point(start.x()+bWidth,start.y()+bHeight)),type)
    {
    }

   /**
    * Constructor
    * Width can be omitted, in this case the button will be square using buttonHeight as width
    * \param w window to which this object belongs
    * \param start upper left point of the button
    * \param type type of the button
   */
    ScrollButton(Window *w,Point start,ScrollButtonType type,int buttonHeight) : ScrollButton(w,start,buttonHeight,buttonHeight,type)
    {
    }

    /**
     * \internal
     * Overridden this member function to draw the object.
     * \param dc drawing context used to draw the object
     */
    virtual void onDraw(DrawingContextProxy& dc)
    {
        Button::onDraw(dc);
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

    virtual void buttonDown()
    {
        Button::buttonDown();
        pressed=true;
        if(downCallback)
            downCallback();
        
    }
    
    virtual void onEvent(Event e)
    {
        if(!this->checkEventArea(e))
        {
            if(e.getEvent()==EventType::TouchUp && isPressed())
            {
                this->buttonUp();
            }
            return;
        }
        if(e.getEvent()==EventType::TouchDown )
        {
            this->buttonDown();
        } else if(e.getEvent()==EventType::TouchUp)
        {
            this->buttonUp();
        }
    } 

    virtual void resetState()
    {
        pressed=false;
        Button::resetState();
    }

    void setDownCallback(std::function<void ()> callback)
    {
        swap(this->downCallback,callback);
    }

    /**
     * Used to set the DrawArea of the button to allow it to be moved
     */
    void setDrawArea(DrawArea da)
    {
        mutableDrawArea = da;
        updateInnerPoints();
        enqueueForRedraw();
    }

    /**
     * Used to read the DrawArea of the button from outside the class
     */
    DrawArea readDrawArea()
    {
        return getDrawArea();
    }

    bool isPressed()
    {
        return pressed;
    }

protected:
    bool pressed=false;///< True if the button is pressed
    DrawArea mutableDrawArea;///< DrawArea of the button that can be changed

    /**
     * Used inside the class to properly draw the button
     * \return the mutableDrawArea of the button
     */
    DrawArea getDrawArea() const
    {   
        return mutableDrawArea;
    }
    
private:
    ScrollButtonType type;///< Type of the button
    std::function<void ()> downCallback; ///< Callback to be called on touchDown

    /**
     * Used to update the inner points of the button
     */
    void updateInnerPoints()
    {
        DrawArea da = getDrawArea();
        this->innerPointTl = Point(da.first.x()+3,da.first.y()+3);
        this->innerPointBr = Point(da.second.x()-3,da.second.y()-3);
    }
};

/**
 * Class for the ScrollingList Item Label
 */
class ItemLabel : public Label
{
public:
    /**
     * Constructor
     * \param w window to which this object belongs
     * \param da area on screen occupied by this object
     * \param text text written in the Label
     * \param itemHeight height of the item
     */
    ItemLabel(Window* w,Point start,int width,int itemHeight) : Label(w,DrawArea(start,Point(start.x()+width,start.y()+itemHeight)),"")
    {
    }

    /**
     * Used to read the draw area of the label from outside the class
     */
    DrawArea readDrawArea()
    {
        return getDrawArea();
    }
};

/**
 * Class for the ScrollingList
 */
class ScrollingList : public Drawable
{
public:
    /**
     * Constructor
     * sets the draw area of the list and creates the buttons
     * sets the button callbacks and the labels
     * The object will be immediately enqueued for redraw
     * \param w window to which this object belongs
     * \param start upper left point of the list
     * \param nItems number of items to be displayed
     * \param width width of the list
     * \param buttonHeight height of the buttons
     * \param itemHeight height of the items
     */
    ScrollingList(Window* w,Point start, int nItems,int width,int buttonHeight=10,int itemHeight=20);

    /**
     * Add an item to the list
     */
    void addItem(const std::string& item);

    /**
     * \internal
     * Overridden this member function to draw the object.
     * \param dc drawing context used to draw the object
     */
    virtual void onDraw(DrawingContextProxy& dc);

    /**
     * \internal
     * Overridden this member function to handle the events.
     * \param e event to be handled
     */
    virtual void onEvent(Event e);

    /**
     * Set the callback to be called when an item is selected
     */
    void setCallback(std::function<void ()> callback);

    /**
     * \return the selected item
     */
    std::string getSelected();

private:
    /**
     * Selects an item
     * \param item item to be selected
     */
    void selectItem(const std::string& item);

    /**
     * Checks if the event is inside the draw area
     * \param e event to be checked
     * \param da draw area to be checked
     */
    bool checkArea(Event e,DrawArea da);

    void upOne(); ///< Scroll up one item
    void keepScrollingUp(); ///< Keep scrolling up
    void keepScrollingDown(); ///< Keep scrolling down
    void downOne(); ///< Scroll down one item
    void pageDown(); ///< Scroll down a page
    void pageUp(); ///< Scroll up a page
    void updateScrollButton(); ///< Update the scroll button

    ScrollButton *up;///< Button to scroll up
    ScrollButton *down;///< Button to scroll down
    ScrollButton *scroll;///< Button to scroll
    int buttonHeight;///< Height of the buttons
    int itemHeight;///< Height of the items
    bool scrolling;///< True if the scroll button is being dragged
    std::thread *scrollingThread;///< Thread to keep scrolling
    DrawArea listArea; ///< Area of the list
    std::vector<ItemLabel*> visibleItems; ///< Labels of the visible items
    std::vector<std::string> items; ///< Items of the list
    std::string selected; ///< Selected item
    int firstVisibleIndex; ///< Index of the first visible item
    int startY; ///< Y coordinate of the start of the touchDown/move event
    std::function<void ()> callback; ///< Callback to be called when an item is selected
};

} //namesapce mxgui

#endif //MXGUI_LEVEL_2
