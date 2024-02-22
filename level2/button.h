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

#ifndef BUTTON_H
#define	BUTTON_H

#include <config/mxgui_settings.h>

#include "application.h"
#include "label.h"
#include "../misc_inst.h"
#include "../display.h"

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
 * A basic interactive Button.
 */
class Button : public Drawable
{
public:
    /**
     * Constructor
     * The object will be immediately enqueued for redraw
     * \param w window to which this object belongs
     * \param da area on screen occupied by this object
     * \param text text written in the Button
     */
    Button(Window *w, DrawArea da, const std::string& text="");
    
    /**
     * Constructor
     * The object will be immediately enqueued for redraw
     * \param w window to which this object belongs
     * \param p upper left point of the button
     * \param width width of the button
     * \param height height of the button
     * \param text text written in the button
     */
    Button(Window *w, Point p, short width, short height, const std::string& text="");
    
    
    /**
     * Change the text being displayed
     * \param text new text to be displayed
     */
    void setText(const std::string& text);
   
    /**
     * Set the function to be called when the button is pressed
     * \param callback pointer to the function to be called
     */
    void setCallback(void (*callback)(int));
   
   /**
    * Checks Event type and calls the appropriate function
    * \param e event to be managed
   */
    void manageEvent(Event e);
    
    /**
     * \internal
     * Overridden this member function to draw the object.
     * \param dc drawing context used to draw the object
     */
    virtual void onDraw(DrawingContextProxy& dc);
    
private:
    Label *text; ///< Text of the button
    void (*callback)(int)=NULL; ///< Pointer to the function to be called when the button is pressed
    bool clicked=false; ///< True if the button is clicked
    Point innerPointTl; ///< Upper left point of the inner area of the button
    Point innerPointBr; ///< Lower right point of the inner area of the button

    //Drawing elements
    std::pair<Color,Color> colors; ///< Colors of the button

    
    const Image tl=Image(3,3,tlp); //Button top left

    
    const Image tr=Image(3,3,trp); //Button top right

    
    const Image bl=Image(3,3,blp); //Button bottom left

    
    const Image br=Image(3,3,brp); //Button bottom right

    bool checkEventArea(Event e)
    {
        DrawArea da=getDrawArea();
        return within(e.getPoint(),da.first,da.second);
    }

    void buttonDown();

    
    void buttonUp();
};

} //namesapce mxgui

#endif //MXGUI_LEVEL_2

#endif //BUTTON_H
