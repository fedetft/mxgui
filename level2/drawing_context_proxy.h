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

#include <utility>
#include <config/mxgui_settings.h>
#include "display.h"
#include "point.h"
#include "color.h"
#include "font.h"
#include "image.h"
#include "draw_area.h"

#ifdef MXGUI_LEVEL_2

namespace mxgui {

/**
 * \ingroup pub_iface_2
 * Base class used by Level2 applications to draw
 */
class DrawingContextProxy
{
public:
    /**
     * Constructor
     */
    DrawingContextProxy() {}
    
    /**
     * Write text to the display. If text is too long it will be truncated
     * \param p point where the upper left corner of the text will be printed
     * \param text, text to print.
     */
    virtual void write(Point p, const char *text)=0;

    /**
     *  Write part of text to the display
     * \param p point of the upper left corner where the text will be drawn.
     * Negative coordinates are allowed, as long as the clipped view has
     * positive or zero coordinates
     * \param a Upper left corner of clipping rectangle
     * \param b Lower right corner of clipping rectangle
     * \param text text to write
     */
    virtual void clippedWrite(Point p, Point a, Point b, const char *text)=0;

    /**
     * Clear the Display. The screen will be filled with the desired color
     * \param color fill color
     */
    virtual void clear(Color color)=0;

    /**
     * Clear an area of the screen
     * \param p1 upper left corner of area to clear
     * \param p2 lower right corner of area to clear
     * \param color fill color
     */
    virtual void clear(Point p1, Point p2, Color color)=0;

    /**
     * Draw a line between point a and point b, with color c
     * \param a first point
     * \param b second point
     * \param color line color
     */
    virtual void line(Point a, Point b, Color color)=0;

    /**
     * Draw an horizontal line on screen.
     * Instead of line(), this member function takes an array of colors to be
     * able to individually set pixel colors of a line.
     * \param p starting point of the line
     * \param colors an array of pixel colors whoase size must be b.x()-a.x()+1
     * \param length length of colors array.
     * p.x()+length must be <= display.width()
     */
    virtual void scanLine(Point p, const Color *colors, unsigned short length)=0;
    
    /**
     * \return a buffer of length equal to this->getWidth() that can be used to
     * render a scanline.
     */
    virtual Color *getScanLineBuffer()=0;
    
    /**
     * Draw the content of the last getScanLineBuffer() on an horizontal line
     * on the screen.
     * \param p starting point of the line
     * \param length length of colors array.
     * p.x()+length must be <= display.width()
     */
    virtual void scanLineBuffer(Point p, unsigned short length)=0;

    /**
     * Draw an image on the screen
     * \param p point of the upper left corner where the image will be drawn
     * \param img image to draw
     */
    virtual void drawImage(Point p, const ImageBase& img)=0;

    /**
     * Draw part of an image on the screen
     * \param p point of the upper left corner where the image will be drawn.
     * Negative coordinates are allowed, as long as the clipped view has
     * positive or zero coordinates
     * \param a Upper left corner of clipping rectangle
     * \param b Lower right corner of clipping rectangle
     * \param img Image to draw
     */
    virtual void clippedDrawImage(Point p, Point a, Point b, const ImageBase& img)=0;

    /**
     * Draw a rectangle (not filled) with the desired color
     * \param a upper left corner of the rectangle
     * \param b lower right corner of the rectangle
     * \param c color of the line
     */
    virtual void drawRectangle(Point a, Point b, Color c)=0;

    /**
     * \return the display's height
     */
    virtual short int getHeight() const=0;

    /**
     * \return the display's width
     */
    virtual short int getWidth() const=0;
    
    /**
     * Set colors used for writing text
     * \param colors a pair where first is the foreground color, and second the
     * background one
     */
    virtual void setTextColor(std::pair<Color,Color> colors)=0;
    
    /**
     * \return a pari with the foreground and background color
     */
    virtual std::pair<Color,Color> getTextColor() const=0;

    /**
     * Set the font used for writing text
     * \param font new font
     */
    virtual void setFont(const Font& font)=0;

    /**
     * \return the current font used to draw text
     */
    virtual Font getFont() const=0;
    
    /**
     * Destructor
     */
    virtual ~DrawingContextProxy();
    
private:
    DrawingContextProxy(const DrawingContextProxy&);
    DrawingContextProxy& operator=(const DrawingContextProxy&);
};

/**
 * \ingroup pub_iface_2
 * This drawing context is used for applications that are fullscreen without
 * a popup window in foreground. It simply forwards all calls to the display's
 * DrawingContext
 */
class FullScreenDrawingContextProxy : public DrawingContextProxy
{
public:
    /**
     * Constructor
     * \param display the display on which you want to draw
     */
    FullScreenDrawingContextProxy(Display& display);
    
    /**
     * Write text to the display. If text is too long it will be truncated
     * \param p point where the upper left corner of the text will be printed
     * \param text, text to print.
     */
    virtual void write(Point p, const char *text);

    /**
     *  Write part of text to the display
     * \param p point of the upper left corner where the text will be drawn.
     * Negative coordinates are allowed, as long as the clipped view has
     * positive or zero coordinates
     * \param a Upper left corner of clipping rectangle
     * \param b Lower right corner of clipping rectangle
     * \param text text to write
     */
    virtual void clippedWrite(Point p, Point a, Point b, const char *text);

    /**
     * Clear the Display. The screen will be filled with the desired color
     * \param color fill color
     */
    virtual void clear(Color color);

    /**
     * Clear an area of the screen
     * \param p1 upper left corner of area to clear
     * \param p2 lower right corner of area to clear
     * \param color fill color
     */
    virtual void clear(Point p1, Point p2, Color color);

    /**
     * Draw a line between point a and point b, with color c
     * \param a first point
     * \param b second point
     * \param color line color
     */
    virtual void line(Point a, Point b, Color color);

    /**
     * Draw an horizontal line on screen.
     * Instead of line(), this member function takes an array of colors to be
     * able to individually set pixel colors of a line.
     * \param p starting point of the line
     * \param colors an array of pixel colors whoase size must be b.x()-a.x()+1
     * \param length length of colors array.
     * p.x()+length must be <= display.width()
     */
    virtual void scanLine(Point p, const Color *colors, unsigned short length);
    
    /**
     * \return a buffer of length equal to this->getWidth() that can be used to
     * render a scanline.
     */
    virtual Color *getScanLineBuffer();
    
    /**
     * Draw the content of the last getScanLineBuffer() on an horizontal line
     * on the screen.
     * \param p starting point of the line
     * \param length length of colors array.
     * p.x()+length must be <= display.width()
     */
    virtual void scanLineBuffer(Point p, unsigned short length);

    /**
     * Draw an image on the screen
     * \param p point of the upper left corner where the image will be drawn
     * \param img image to draw
     */
    virtual void drawImage(Point p, const ImageBase& img);

    /**
     * Draw part of an image on the screen
     * \param p point of the upper left corner where the image will be drawn.
     * Negative coordinates are allowed, as long as the clipped view has
     * positive or zero coordinates
     * \param a Upper left corner of clipping rectangle
     * \param b Lower right corner of clipping rectangle
     * \param img Image to draw
     */
    virtual void clippedDrawImage(Point p, Point a, Point b, const ImageBase& img);

    /**
     * Draw a rectangle (not filled) with the desired color
     * \param a upper left corner of the rectangle
     * \param b lower right corner of the rectangle
     * \param c color of the line
     */
    virtual void drawRectangle(Point a, Point b, Color c);

    /**
     * \return the display's height
     */
    virtual short int getHeight() const;

    /**
     * \return the display's width
     */
    virtual short int getWidth() const;
    
    /**
     * Set colors used for writing text
     * \param colors a pair where first is the foreground color, and second the
     * background one
     */
    virtual void setTextColor(std::pair<Color,Color> colors);
    
    /**
     * \return a pari with the foreground and background color
     */
    virtual std::pair<Color,Color> getTextColor() const;

    /**
     * Set the font used for writing text
     * \param font new font
     */
    virtual void setFont(const Font& font);

    /**
     * \return the current font used to draw text
     */
    virtual Font getFont() const;
    
private:
    DrawingContext dc;
};

/**
 * \ingroup pub_iface_2
 * This drawing context is used for applications that have exactly one popup
 * window in foreground. It avoids drawing over the area where the popup is
 * placed. Support for more than one rectangular foreground window is not
 * imlemented yet.
 */
class BackgroudDrawingContextProxy : public DrawingContextProxy
{
public:
    /**
     * Constructor
     * \param display the display on which you want to draw
     * \param da drawing area of popup
     */
    BackgroudDrawingContextProxy(Display& display, DrawArea da);
    
    //TODO
};

/**
 * \ingroup pub_iface_2
 * This drawing context is used for a popup window in foreground. It avoids
 * drawing outside the area where the popup is placed.
 */
class ForegroundDrawingContextProxy : public DrawingContextProxy
{
public:
    /**
     * Constructor
     * \param display the display on which you want to draw
     * \param da drawing area of popup
     */
    ForegroundDrawingContextProxy(Display& display, DrawArea da);
    
    //TODO
};

} //namespace mxgui

#endif //MXGUI_LEVEL_2
