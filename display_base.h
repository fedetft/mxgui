/***************************************************************************
 *   Copyright (C) 2010, 2011 by Terraneo Federico                         *
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

#include "point.h"
#include "color.h"
#include "font.h"
#include "image.h"
#include "iterator_direction.h"

#ifndef DISPLAY_BASE_H
#define	DISPLAY_BASE_H

namespace mxgui {

/**
 * Display class. This is an interface that all display drivers must implement
 * It represents the first usable abstraction over a display, with member
 * functions to draw lines, text, images...
 * Dispatching of the calls to the implementation is done using templates
 * instead of inheritance and virtual functions beacause the display
 * implementation is chosen at compile time.
 */
template<typename T>
class basic_display
{
public:
    /**
     * \return a reference to the only instance of the Display class (singleton)
     * The actual type of the Display class can be statically customized via
     * a #define to match the display type available
     */
    static basic_display& instance();

    /**
     * Write text to the display. If text is too long it will be truncated
     * \param p point where the upper left corner of the text will be printed
     * \param text, text to print.
     */
    void write(Point p, const char *text)
    {
        subclass.write(p,text);
    }

    /**
     *  Write part of text to the display
     * \param p point of the upper left corner where the text will be drawn.
     * Negative coordinates are allowed, as long as the clipped view has
     * positive or zero coordinates
     * \param a Upper left corner of clipping rectangle
     * \param b Lower right corner of clipping rectangle
     * \param text text to write
     */
    void clippedWrite(Point p, Point a, Point b, const char *text)
    {
        subclass.clippedWrite(p,a,b,text);
    }

    /**
     * Clear the Display. The screen will be filled with the desired color
     * \param color fill color
     */
    void clear(Color color)
    {
        subclass.clear(color);
    }

    /**
     * Clear an area of the screen
     * \param p1 upper left corner of area to clear
     * \param p2 lower right corner of area to clear
     * \param color fill color
     */
    void clear(Point p1, Point p2, Color color)
    {
        subclass.clear(p1,p2,color);
    }

    /**
     * This member function is used on some target displays to reset the
     * drawing window to its default value. You have to call beginPixel() once
     * before calling setPixel(). Yo can then make any number of calls to
     * setPixel() without calling beginPixel() again, as long as you don't
     * call any other member function in this class. If you call another
     * member function, for example line(), you have to call beginPixel() again
     * before calling setPixel().
     */
    void beginPixel()
    {
        subclass.beginPixel();
    }

    /**
     * Draw a pixel with desired color. You have to call beginPixel() once
     * before calling setPixel()
     * \param p point where to draw pixel
     * \param color pixel color
     */
    void setPixel(Point p, Color color)
    {
        subclass.setPixel(p,color);
    }

    /**
     * Draw a line between point a and point b, with color c
     * \param a first point
     * \param b second point
     * \param c line color
     */
    void line(Point a, Point b, Color color)
    {
        subclass.line(a,b,color);
    }

    /**
     * Draw an horizontal line on screen.
     * Instead of line(), this member function takes an array of colors to be
     * able to individually set pixel colors of a line.
     * Note that the two points must satisfy the following constraints:
     * a.y() == b.y() (horizontal line)
     * a.x() <= b.x() (a must be to the left of b))
     * \param a first point
     * \param b second point
     * \param colors an array of pixel colors whoase size must be b.x()-a.x()+1
     */
    void scanLine(Point a, Point b, const Color *colors)
    {
        subclass.scanLine(a,b,colors);
    }

    /**
     * Draw an image on the screen
     * \param p point of the upper left corner where the image will be drawn
     * \param i image to draw
     */
    void drawImage(Point p, Image img)
    {
        subclass.drawImage(p,img);
    }
    
    /**
     * Draw part of an image on the screen
     * \param p point of the upper left corner where the image will be drawn.
     * Negative coordinates are allowed, as long as the clipped view has
     * positive or zero coordinates
     * \param a Upper left corner of clipping rectangle
     * \param b Lower right corner of clipping rectangle
     * \param i Image to draw
     */
    void clippedDrawImage(Point p, Point a, Point b, Image img)
    {
        subclass.clippedDrawImage(p,a,b,img);
    }

    /**
     * Draw a rectangle (not filled) with the desired color
     * \param a upper left corner of the rectangle
     * \param b lower right corner of the rectangle
     * \param c color of the line
     */
    void drawRectangle(Point a, Point b, Color c)
    {
        subclass.drawRectangle(a,b,c);
    }

    /**
     * \return the display's height
     */
    short int getHeight() const
    {
        return subclass.getHeight();
    }

    /**
     * \return the display's width
     */
    short int getWidth() const
    {
        return subclass.getWidth();
    }

    /**
     * Turn the display On after it has been turned Off.
     * Display initial state is On.
     */
    void turnOn()
    {
        subclass.turnOn();
    }

    /**
     * Turn the display Off. It can be later turned back On.
     */
    void turnOff()
    {
        subclass.turnOff();
    }

    /**
     * Set colors used for writing text
     * \param fgcolor text color
     * \param bgcolor background color
     */
    void setTextColor(Color fgcolor, Color bgcolor)
    {
        subclass.setTextColor(fgcolor,bgcolor);
    }

    /**
     * \return the current foreground color.
     * The foreground color is used to draw text on screen
     */
    Color getForeground() const
    {
        return subclass.getForeground();
    }

    /**
     * \return the current background color.
     * The foreground color is used to draw text on screen
     */
    Color getBackground() const
    {
        return subclass.getBackground();
    }

    /**
     * Set the font used for writing text
     * \param font new font
     */
    void setFont(const Font& font)
    {
        subclass.setFont(font);
    }

    /**
     * \return the current font used to draw text
     */
    Font getFont() const
    {
        return subclass.getFont();
    }

    /**
     * Make all changes done to the display since the last call to update()
     * visible. Not all backends require it, so on some backend what you do
     * will be immediately visible, while on others a call to update() is needed
     */
    void update()
    {
        subclass.update();
    }

    /// pixel iterator.
    /// A pixel iterator is an output iterator that allows to define a window on
    /// the display and write to its pixels.
    /// For performance reasons, they have the following limitations:
    /// - They are output iterators only, so they can't be used for reading
    /// - There can be only ONE active iterator per display object, calling
    ///   begin() will invalidate any previous iterator (watch out for
    ///   multithreading use too). This because they are meant to be implemented
    ///   using hardware accelerated window drawing.
    typedef typename T::pixel_iterator pixel_iterator;

    /**
     * Specify a window on screen and return an object that allows to write
     * its pixels.
     * Note: a call to begin() will invalidate any previous iterator.
     * \param p1 upper left corner of window
     * \param p2 lower right corner (included)
     * \param d increment direction
     * \return a pixel iterator
     */
    pixel_iterator begin(Point p1, Point p2, IteratorDirection d=RD)
    {
        return subclass.begin(p1,p2,d);
    }

    /**
     * \return an iterator which is one past the last pixel in the pixel
     * specified by begin. Behaviour is undefined if called before calling
     * begin()
     */
    pixel_iterator end() const
    {
        return subclass.end();
    }

protected:
    /**
     * Constructor
     */
    basic_display() {}
    
private:
    /**
     * Class cannot be copied
     */
    basic_display(const basic_display&);
    basic_display& operator=(const basic_display&);

    T subclass; ///< The subclass, that implements this "interface"
};

template<typename T>
basic_display<T>& basic_display<T>::instance()
{
    //FIXME: thread unsafe...
    static basic_display<T> *result=0;
    if(result==0) result=new basic_display<T>;
    return *result;
}

} //namespace mxgui

#endif //DISPLAY_BASE_H
