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

#pragma once

#include <string>
#include <config/mxgui_settings.h>
#include "application.h"

#ifdef MXGUI_LEVEL_2

namespace mxgui {

/**
 * A Label is used to print non-interactive text on the screen
 */
class Label : public Drawable
{
public:
    /**
     * Constructor
     * The object will be immediately enqueued for redraw
     * \param w window to which this object belongs
     * \param da area on screen occupied by this object
     * \param text text written in the Label
     */
    Label(Window *w, DrawArea da, const std::string& text="");
    
    /**
     * Constructor
     * The object will be immediately enqueued for redraw
     * \param w window to which this object belongs
     * \param p upper left point of the text label
     * \param width width of the text label
     * \param height height of the text label
     * \param text text written in the Label
     */
    Label(Window *w, Point p, short width, short height, const std::string& text="");
    
    /**
     * \return the text being displayed 
     */
    const std::string& getText() const { return text; }
    
    /**
     * Change the text being displayed
     * \param text new text to be displayed
     */
    void setText(const std::string& text)
    {
        this->text=text;
        enqueueForRedraw();
    }
    
    /**
     * Set the text alignment along the x axis
     * \param a alignment LEFT, CENTER, RIGHT
     */
    void setXAlignment(Alignment::Alignment_ a)
    {
        if(xAlign!=a) enqueueForRedraw();
        xAlign=a;
    }
    
    /**
     * \return true if the text is aligned along the x axis
     */
    Alignment::Alignment_ getXAlignment() const { return xAlign; }
    
    /**
     * Set the text alignment along the y axis
     * \param a alignment TOP, CENTER, BOTTOM
     */
    void setYAlignment(Alignment::Alignment_ a)
    {
        if(yAlign!=a) enqueueForRedraw();
        yAlign=a;
    }
    
    /**
     * \return true if the text is aligned along the y axis
     */
    Alignment::Alignment_ getYAlignment() const { return yAlign; }
    
    /**
     * Set foreground and background colors
     * \param colors 
     */
    void setColors(std::pair<Color,Color> colors);
    
    /**
     * Return to the default behaviour of taking the colors from the window
     * default preferences
     */
    void resetForegroundColor()
    {
        if(overrideColors) enqueueForRedraw();
        overrideColors=false;
    }
   
    /**
     * \internal
     * Overridden this member function to draw the object.
     * \param dc drawing context used to draw the object
     */
    virtual void onDraw(DrawingContextProxy& dc);
    
private:
    std::string text; ///< Text of the label
    std::pair<Color,Color> colors; ///< Colors, if overridden
    Alignment::Alignment_ xAlign; ///< Text alignment along x axis
    Alignment::Alignment_ yAlign; ///< Text alignment along y axis
    bool overrideColors; ///< true: use local color, false: use window defaults
};

} //namesapce mxgui

#endif //MXGUI_LEVEL_2
