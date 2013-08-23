/***************************************************************************
 *   Copyright (C) 2010, 2011, 2012, 2013 by Terraneo Federico             *
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

#ifndef DISPLAY_H
#define	DISPLAY_H

#include <pthread.h>
#include "mxgui_settings.h"
#include "point.h"
#include "color.h"
#include "font.h"
#include "image.h"

namespace mxgui {

class DisplayImpl;    //Forward declaration
class DrawingContext;

/**
 * \ingroup pub_iface
 * Display class.
 * Contains member functions to retrieve a display instance, and to turn it on
 * or off. For drawing onto the display, you need to instantiate a
 * DrawingContext.
 */
class Display
{
public:
    /**
     * \return a reference to the instance of the Display.
     * Multiple calls return the same display instance (singleton)
     */
    static Display& instance();

    /**
     * Turn the display On after it has been turned Off.
     * Display initial state is On.
     */
    void turnOn();

    /**
     * Turn the display Off. It can be later turned back On.
     */
    void turnOff();
    
    /**
     * Set display brightness. Depending on the underlying driver,
     * may do nothing.
     * \param brt from 0 to 100
     */
    void setBrightness(int brt);
    
    /**
     * \return true if the display is on
     */
    bool isOn() const { return isDisplayOn; }

    /**
     * \return the display's height
     */
    short int getHeight() const;

    /**
     * \return the display's width
     */
    short int getWidth() const;

private:
    /**
     * Constructor
     */
    Display(DisplayImpl *impl);

    /*
     * Class cannot be copied
     */
    Display(const Display&);
    Display& operator=(const Display&);

    DisplayImpl *pImpl; //Implementation detal
    pthread_mutex_t dispMutex; //To lock concurrent access to the display
    bool isDisplayOn;

    friend class DrawingContext;
};

/**
 * \ingroup pub_iface
 * A drawing context is a class that is instantiated whenever there is the
 * need to draw something on a display. Its primary purpose is to lock a mutex
 * allowing safe concurrent access to a display from multiple threads, but
 * avoiding the overhead of locking a mutex for each single graphic primitive
 * call.
 */
class DrawingContext
{
public:
    /**
     * Constructor
     * \param display the display on which you want to draw
     */
    DrawingContext(Display& display);

    /**
     * Write text to the display. If text is too long it will be truncated
     * \param p point where the upper left corner of the text will be printed
     * \param text, text to print.
     */
    void write(Point p, const char *text);

    /**
     *  Write part of text to the display
     * \param p point of the upper left corner where the text will be drawn.
     * Negative coordinates are allowed, as long as the clipped view has
     * positive or zero coordinates
     * \param a Upper left corner of clipping rectangle
     * \param b Lower right corner of clipping rectangle
     * \param text text to write
     */
    void clippedWrite(Point p, Point a, Point b, const char *text);

    /**
     * Clear the Display. The screen will be filled with the desired color
     * \param color fill color
     */
    void clear(Color color);

    /**
     * Clear an area of the screen
     * \param p1 upper left corner of area to clear
     * \param p2 lower right corner of area to clear
     * \param color fill color
     */
    void clear(Point p1, Point p2, Color color);

    /**
     * This member function is used on some target displays to reset the
     * drawing window to its default value. You have to call beginPixel() once
     * before calling setPixel(). You can then make any number of calls to
     * setPixel() without calling beginPixel() again, as long as you don't
     * call any other member function in this class. If you call another
     * member function, for example line(), you have to call beginPixel() again
     * before calling setPixel().
     */
    void beginPixel();

    /**
     * Draw a pixel with desired color. You have to call beginPixel() once
     * before calling setPixel()
     * \param p point where to draw pixel
     * \param color pixel color
     */
    void setPixel(Point p, Color color);

    /**
     * Draw a line between point a and point b, with color c
     * \param a first point
     * \param b second point
     * \param color line color
     */
    void line(Point a, Point b, Color color);

    /**
     * Draw an horizontal line on screen.
     * Instead of line(), this member function takes an array of colors to be
     * able to individually set pixel colors of a line.
     * \param p starting point of the line
     * \param colors an array of pixel colors whoase size must be b.x()-a.x()+1
     * \param length length of colors array.
     * p.x()+length must be <= display.width()
     */
    void scanLine(Point p, const Color *colors, unsigned short length);
    
    /**
     * \return a buffer of length equal to this->getWidth() that can be used to
     * render a scanline.
     */
    Color *getScanLineBuffer();
    
    /**
     * Draw the content of the last getScanLineBuffer() on an horizontal line
     * on the screen.
     * \param p starting point of the line
     * \param length length of colors array.
     * p.x()+length must be <= display.width()
     */
    void scanLineBuffer(Point p, unsigned short length);

    /**
     * Draw an image on the screen
     * \param p point of the upper left corner where the image will be drawn
     * \param img image to draw
     */
    void drawImage(Point p, const ImageBase& img);

    /**
     * Draw part of an image on the screen
     * \param p point of the upper left corner where the image will be drawn.
     * Negative coordinates are allowed, as long as the clipped view has
     * positive or zero coordinates
     * \param a Upper left corner of clipping rectangle
     * \param b Lower right corner of clipping rectangle
     * \param img Image to draw
     */
    void clippedDrawImage(Point p, Point a, Point b, const ImageBase& img);

    /**
     * Draw a rectangle (not filled) with the desired color
     * \param a upper left corner of the rectangle
     * \param b lower right corner of the rectangle
     * \param c color of the line
     */
    void drawRectangle(Point a, Point b, Color c);

    /**
     * \return the display's height
     */
    short int getHeight() const;

    /**
     * \return the display's width
     */
    short int getWidth() const;

    /**
     * Set colors used for writing text
     * \param fgcolor text color
     * \param bgcolor background color
     */
    void setTextColor(Color fgcolor, Color bgcolor);

    /**
     * \return the current foreground color.
     * The foreground color is used to draw text on screen
     */
    Color getForeground() const;

    /**
     * \return the current background color.
     * The foreground color is used to draw text on screen
     */
    Color getBackground() const;

    /**
     * Set the font used for writing text
     * \param font new font
     */
    void setFont(const Font& font);

    /**
     * \return the current font used to draw text
     */
    Font getFont() const;
    
    /**
     * Destructor
     */
    ~DrawingContext();

private:
    /*
     * Class cannot be copied
     */
    DrawingContext(const DrawingContext&);
    DrawingContext& operator=(DrawingContext&);

    Display& display; //Underlying display object
};

} //namespace mxgui

#endif //DISPLAY_H
