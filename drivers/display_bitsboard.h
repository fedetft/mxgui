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

#ifndef MXGUI_LIBRARY
#error "This is header is private, it can be used only within mxgui."
#error "If your code depends on a private header, it IS broken."
#endif //MXGUI_LIBRARY

#ifndef DISPLAY_BITSBOARD_H
#define	DISPLAY_BITSBOARD_H

#ifdef _BOARD_BITSBOARD

#include "mxgui/mxgui_settings.h"
#include "mxgui/point.h"
#include "mxgui/color.h"
#include "mxgui/font.h"
#include "mxgui/image.h"
#include "mxgui/iterator_direction.h"
#include <stdexcept>
#include <limits>

//This display is 1 bit per pixel, check that the color depth is properly
//configured
#ifndef MXGUI_COLOR_DEPTH_1_BIT_LINEAR
#error The bitsboard driver requires a color depth of 1bit per pixel
#endif

namespace mxgui {

class DisplayImpl
{
public:
    /**
     * Constructor.
     * Do not instantiate objects of this type directly from application code,
     * use Display::instance() instead.
     */
    DisplayImpl();
    
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
     * before calling setPixel(). Yo can then make any number of calls to
     * setPixel() without calling beginPixel() again, as long as you don't
     * call any other member function in this class. If you call another
     * member function, for example line(), you have to call beginPixel() again
     * before calling setPixel().
     */
    void beginPixel() {}

    /**
     * Draw a pixel with desired color. You have to call beginPixel() once
     * before calling setPixel()
     * \param p point where to draw pixel
     * \param color pixel color
     */
    void setPixel(Point p, Color color)
    {
        //if(p.x()<0 || p.y()<0 || p.x()>=width || p.y()>=height) return;
        unsigned short x=p.x();
        unsigned short y=p.y();
        if(y>=64)
        {
            y-=64;
            x+=256;
        }
        framebufferBitBandAlias[512*y+x]= color ? 0 : 1;
    }

    /**
     * Draw a line between point a and point b, with color c
     * \param a first point
     * \param b second point
     * \param c line color
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
     * Draw an image on the screen
     * \param p point of the upper left corner where the image will be drawn
     * \param i image to draw
     */
    void drawImage(Point p, const ImageBase& img);

    /**
     * Draw part of an image on the screen
     * \param p point of the upper left corner where the image will be drawn.
     * Negative coordinates are allowed, as long as the clipped view has
     * positive or zero coordinates
     * \param a Upper left corner of clipping rectangle
     * \param b Lower right corner of clipping rectangle
     * \param i Image to draw
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
    short int getHeight() const { return height; }

    /**
     * \return the display's width
     */
    short int getWidth() const { return width; }

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
     * Set colors used for writing text
     * \param fgcolor text color
     * \param bgcolor background color
     */
    void setTextColor(Color fgcolor, Color bgcolor);

    /**
     * \return the current foreground color.
     * The foreground color is used to draw text on screen
     */
    Color getForeground() const { return textColor[3]; }

    /**
     * \return the current background color.
     * The foreground color is used to draw text on screen
     */
    Color getBackground() const { return textColor[0]; }

    /**
     * Set the font used for writing text
     * \param font new font
     */
    void setFont(const Font& font);

    /**
     * \return the current font used to draw text
     */
    Font getFont() const { return font; }

    /**
     * Make all changes done to the display since the last call to update()
     * visible. This backends require it.
     */
    void update() {}

    /**
     * Pixel iterator. A pixel iterator is an output iterator that allows to
     * define a window on the display and write to its pixels.
     */
    class pixel_iterator
    {
    public:
        /**
         * Default constructor, results in an invalid iterator.
         */
        pixel_iterator(): dataPtr(0) {}

        /**
         * Set a pixel and move the pointer to the next one
         * \param color color to set the current pixel
         * \return a reference to this
         */
        pixel_iterator& operator= (Color color)
        {
            *dataPtr= color ? 0 : 1;

            //This is to move to the adjacent pixel
            dataPtr+=aIncr;

            //This is in case the 64th vertical line is crossed, and is because
            //the display framebuffer is logically 256x128 but physically 512x64
            if(--quirkCtr<=0)
            {
                quirkCtr=quirkReload[quirkFlag];
                dataPtr+=qIncr[quirkFlag];
                quirkFlag=1-quirkFlag;
            }
            
            //This is the step move to the next horizontal/vertical line
            if(++ctr>=endCtr)
            {
                ctr=0;
                dataPtr+=sIncr;
            }
            return *this;
        }

        /**
         * Compare two pixel_iterators for equality.
         * They are equal if they point to the same location.
         */
        bool operator== (const pixel_iterator& itr)
        {
            return this->dataPtr==itr.dataPtr;
        }

        /**
         * Compare two pixel_iterators for inequality.
         * They different if they point to different locations.
         */
        bool operator!= (const pixel_iterator& itr)
        {
            return this->dataPtr!=itr.dataPtr;
        }

        /**
         * \return a reference to this.
         */
        pixel_iterator& operator* () { return *this; }

        /**
         * \return a reference to this. Does not increment pixel pointer.
         */
        pixel_iterator& operator++ ()  { return *this; }

        /**
         * \return a reference to this. Does not increment pixel pointer.
         */
        pixel_iterator& operator++ (int)  { return *this; }

    private:
        /**
         * Constructor
         * \param start Upper left corner of window
         * \param end Lower right corner of window
         * \param direction Iterator direction
         * \param disp Display we're associated
         */
        pixel_iterator(Point start, Point end, IteratorDirection direction,
                DisplayImpl *disp) : ctr(0),
                quirkCtr(std::numeric_limits<int>::max()), quirkFlag(0),
                dataPtr(disp->framebufferBitBandAlias)
        {
            //Handle the framebuffer quirk if the start is in the bottom half
            short ys=start.y();
            short half=disp->getHeight()/2;
            if(ys>=half)
            {
                ys-=half;
                dataPtr+=disp->getWidth();
            }
            
            //Compite the increment in the adjacent direction (aIncr) and in the
            //step direction (sIncr) depending on the direction
            dataPtr+=2*ys*disp->getWidth()+start.x();
            if(direction==RD)
            {
                endCtr=end.x()+1-start.x();
                aIncr=1;
                sIncr=start.x()+2*disp->getWidth()-1-end.x();
            } else {
                endCtr=end.y()+1-start.y();
                aIncr=2*disp->getWidth();
                sIncr=-aIncr*endCtr+1;
            }
            
            //Handle the framebuffer quirk if the window crosses the screen half
            if(start.y()<half && end.y()>=half)
            {
                if(direction==RD)
                {
                    //In this case the 64th line is crossed only once
                    quirkCtr=endCtr*(half-start.y());
                    quirkReload[0]=std::numeric_limits<int>::max();
                    qIncr[0]=-(disp->getHeight()-1)*disp->getWidth();
                } else {
                    //In this case the 64th line is crossed many times
                    quirkReload[0]=end.y()+1-half;
                    quirkReload[1]=quirkCtr=half-start.y();
                    qIncr[0]=-(disp->getHeight()-1)*disp->getWidth();
                    qIncr[1]=(disp->getHeight()-1)*disp->getWidth();
                }
            }
        }

        unsigned short ctr;           ///< Counter to decide when to step
        unsigned short endCtr;        ///< When ctr==endCtr apply a step
        
        int quirkCtr;                 ///< Quirk increment is done if reaches zero
        unsigned short quirkReload[2];///< Value reloaded into quirkCtr
        int qIncr[2];                 ///< Quirk increments
        short quirkFlag;              ///< Used as index in the previous arrays
        
        short aIncr;                  ///< Adjacent increment
        int sIncr;                    ///< Step increment           
        unsigned int *dataPtr;        ///< Pointer to bit band area

        friend class DisplayImpl; //Needs access to ctor
    };

    /**
     * Specify a window on screen and return an object that allows to write
     * its pixels.
     * Note: a call to begin() will invalidate any previous iterator.
     * \param p1 upper left corner of window
     * \param p2 lower right corner (included)
     * \param d increment direction
     * \return a pixel iterator
     */
    pixel_iterator begin(Point p1, Point p2, IteratorDirection d);

    /**
     * \return an iterator which is one past the last pixel in the pixel
     * specified by begin. Behaviour is undefined if called before calling
     * begin()
     */
    pixel_iterator end() const { return last; }

private:
    #if defined MXGUI_ORIENTATION_VERTICAL || \
        defined MXGUI_ORIENTATION_VERTICAL_MIRRORED
    static const short int width=128;
    static const short int height=256;
    #elif defined MXGUI_ORIENTATION_HORIZONTAL || \
          defined MXGUI_ORIENTATION_HORIZONTAL_MIRRORED
    static const short int width=256;
    static const short int height=128;
    #else
    #error No orientation defined
    #endif

    /// textColors[0] is the background color, textColor[3] the foreground
    /// while the other two are the intermediate colors for drawing antialiased
    /// fonts. They remain just for compatibilty, as this screen in monochrome
    Color textColor[4];
    Font font; ///< Current font selected for writing text
    pixel_iterator last; ///< Last iterator for end of iteration check
    unsigned int *framebufferBitBandAlias; ///< For fast pixel_iterator
};

} //namespace mxgui

#endif //_BOARD_BITSBOARD

#endif //DISPLAY_BITSBOARD_H
