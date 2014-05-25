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

#include <string>
#include <utility>
#include <config/mxgui_settings.h>
#include "display.h"
#include "image.h"

#ifdef MXGUI_LEVEL_2

#ifndef SIMPLE_IMAGE_BUTTON_H
#define	SIMPLE_IMAGE_BUTTON_H

namespace mxgui {

/**
 * Base class for simple buttons
 */
class SimpleButton
{
public:
    SimpleButton(Point a, Point b) : a(a), b(b), down(false), clicked(false) {}
    
    /**
     * Handle a touch of the screen that could possibly result in the button
     * being clicked
     * \param p touch pount
     * \return true if the button was clicked
     */
    bool handleTouchDown(Point p)
    {
        if(within(p,a,b)) { down=true; clicked=true; }
        return clicked;
    }
    
    /**
     * Handle a touch up event
     * \param p touch pount
     */
    void handleTouchUp(Point p) { down=false; clicked=false; }
    
    /**
     * \return true if the button was clicked, and resets the clicked flag
     */
    bool isClicked()
    {
        bool result=clicked;
        clicked=false;
        return result;
    }
    
    /**
     * Draw the button
     */
    virtual void draw(DrawingContext& dc)=0;
    
protected:
    Point a; //Uppper left corner of button
    Point b; //Lower right corner of button
    bool down;
    bool clicked;
};

/**
 * Simple button class showing an image
 */
class SimpleImageButton : public SimpleButton
{
public:
    /**
     * A button made of an image
     * \param imageNotClicked the image that is drawn as the button when not
     * clicked. The button size is the same as the image. Ownership of this
     * pointer remains of the caller
     * \param imageClicked the image that is drawn as the button when clicked,
     * must be of the same size of imageNotClicked
     * \param place place where the button is drawn (upper left coordinate)
     */
    SimpleImageButton(const Image *imageNotClicked, const Image *imageClicked,
            Point place);
    
    /**
     * Draw the button
     * \param dc drawing context where to draw the button
     */
    void draw(DrawingContext& dc);
    
private:
    const Image *imageNotClicked;
    const Image *imageClicked;
};

/**
 * Simple button class with a text caption and border
 */
class SimpleTextButton : public SimpleButton
{
public:
    /**
     * Constructor
     * \param a upper left corner of button
     * \param b lower right corner of button
     * \param colors first is foreground color, second is background color
     * \param caption text that appears in the button
     */
    SimpleTextButton(Point a, Point b, std::pair<Color,Color> colors,
            const std::string& caption);
    
    /**
     * Draw the button
     * \param dc drawing context where to draw the button
     */
    void draw(DrawingContext& dc);
    
private:
    std::pair<Color,Color> colors;
    std::string caption;
};

} //namespace mxgui

#endif //SIMPLE_IMAGE_BUTTON_H

#endif //MXGUI_LEVEL_2
