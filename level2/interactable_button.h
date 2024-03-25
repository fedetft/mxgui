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

#ifndef INTBUTTON_H
#define	INTBUTTON_H

#include <config/mxgui_settings.h>
#include <functional>
#include "application.h"
#include "label.h"
#include "input.h"
#include "../misc_inst.h"

#ifdef MXGUI_LEVEL_2

namespace mxgui {

//Corner images of the button
static const unsigned short tlp[]={
        61276,46420,31661,48565,33741,46452,31661,46452,57050
    };
static const unsigned short trp[]={
        31661,46420,61276,57050,31628,48565,48499,42161,31661
    };
static const unsigned short blp[]={
        31661,57050,48499,48565,33741,42225,61276,46420,31661
    };
static const unsigned short brp[]={
        48499,42161,31661,42225,33741,48565,31661,46420,61276
    };

/**
 * Abstract base class for buttons.
 */
class InteractableButton : public Drawable
{
public:

    /**
     * Constructor
     * \param w pointer to the window containing the button
     * \param da draw area of the button
     */
    InteractableButton(Window* w, DrawArea da)
        : Drawable(w,da)
    {
        this->innerPointTl = Point(da.first.x()+3,da.first.y()+3);
        this->innerPointBr = Point(da.second.x()-2,da.second.y()-2);
    }
    /**
     * Change the text being displayed
     * \param text new text to be displayed
     */
    void setText(const std::string& text)
    {
        if(this->text)
            this->text->setText(text);
    }

   
    /**
     * Set the function to be called when the button is pressed
     * \param callback wrapper to the function to be called
     */
    void setCallback(std::function<void ()> callback)
    {
        swap(this->callback,callback);
    }
   
    /**
     * \internal
     * Overridden this member function to handle events.
     * \param e event to be handled
     */
    virtual void onEvent(Event e)
    {
        if(!this->checkEventArea(e))
        {
            this->resetState();
            return;
        }
        if(e.getEvent()==EventType::TouchDown || e.getEvent()==EventType::TouchMove)
        {
            this->buttonDown();
        }
        else if(e.getEvent()==EventType::TouchUp)
        {
            this->buttonUp();
        }
    
    } 
    
    /**
     * \internal
     * Overridden this member function to draw the object.
     * \param dc drawing context used to draw the object
     */
    virtual void onDraw(DrawingContextProxy& dc)=0;

    /**
     * Destructor
     */
    ~InteractableButton() = default;

    
private:
    
    std::function<void ()> callback; ///< Wrapper to the function to be called when the button is pressed
    

protected:

    Label *text = nullptr; ///< Text of the button
    Point innerPointTl; ///< Upper left point of the inner area of the button
    Point innerPointBr; ///< Lower right point of the inner area of the button
    //Drawing elements

    const Image tl=Image(3,3,tlp); //Button top left
    const Image tr=Image(3,3,trp); //Button top right
    const Image bl=Image(3,3,blp); //Button bottom left
    const Image br=Image(3,3,brp); //Button bottom right
    std::pair<Color,Color> colors; ///< Colors of the InteractableButton
    
    /**
     * Check if the event is within the button area
     * \param e event to be checked
     * \return true if the event is within the button area
     */
    bool checkEventArea(Event e)
    {
        DrawArea da=getDrawArea();
        return within(e.getPoint(),da.first,da.second);
    }

    /**
     * Resets the state of the button
     */
    virtual void resetState()
    {
        enqueueForRedraw();
    }

    /**
     * Set button state to pressed
     */
    virtual void buttonDown()=0;

    /**
     * Set button state to released
     * Call the callback function if it is set
     */
    virtual void buttonUp()
    {
        if(callback) callback();
    }

    /**
     * Allows subclasses to modify Button drawArea without exposing it
     * \return the draw area of the button
     * 
     */
    virtual DrawArea getDrawArea() const
    {
        return Drawable::getDrawArea();
    }
};

} //namesapce mxgui

#endif //MXGUI_LEVEL_2

#endif //INTBUTTON_H
