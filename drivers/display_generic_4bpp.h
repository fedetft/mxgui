/***************************************************************************
 *   Copyright (C) 2018 by Terraneo Federico                               *
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

#include <config/mxgui_settings.h>
#include "display.h"
#include "point.h"
#include "color.h"
#include "iterator_direction.h"
#include <cstdio>
#include <cstring>
#include <algorithm>

namespace mxgui {

/**
 * Generic driver for a 4 bit per pixel display, like the OLED displays based
 * on the SSD1322 controller.
 * To use this class for a display driver, you have to derive from this class
 * and provide:
 * - a constructor that shall initialize and turn on the display, following
 *   whatever initialization sequence is recomended by the manufacturer
 * - the implementation for the doTurnOn(), doTurnOff() and doSetBrightness()
 *   member functions
 * - the update() member function which shall copy the backbuffer which is in
 *   the microcontroller memory to the display memory
 */
class DisplayGeneric4BPP : public Display
{
public:
    /**
     * Constructor.
     * \param width display width
     * \param height display height
     */
    DisplayGeneric4BPP(short width, short height);
    
    /**
     * \return a pair with the display height and width
     */
    std::pair<short int, short int> doGetSize() const override;

    /**
     * Write text to the display. If text is too long it will be truncated
     * \param p point where the upper left corner of the text will be printed
     * \param text, text to print.
     */
    void write(Point p, const char *text) override;

    /**
     * Write part of text to the display
     * \param p point of the upper left corner where the text will be drawn.
     * Negative coordinates are allowed, as long as the clipped view has
     * positive or zero coordinates
     * \param a Upper left corner of clipping rectangle
     * \param b Lower right corner of clipping rectangle
     * \param text text to write
     */
    void clippedWrite(Point p, Point a, Point b, const char *text) override;

    /**
     * Clear the Display. The screen will be filled with the desired color
     * \param color fill color
     */
    void clear(Color color) override;

    /**
     * Clear an area of the screen
     * \param p1 upper left corner of area to clear
     * \param p2 lower right corner of area to clear
     * \param color fill color
     */
    void clear(Point p1, Point p2, Color color) override;

    /**
     * This backend does not require it, so it is a blank.
     */
    void beginPixel() override;

    /**
     * Draw a pixel with desired color. You have to call beginPixel() once
     * before calling setPixel()
     * \param p point where to draw pixel
     * \param color pixel color
     */
    void setPixel(Point p, Color color) override;

    /**
     * Draw a line between point a and point b, with color c
     * \param a first point
     * \param b second point
     * \param c line color
     */
    void line(Point a, Point b, Color color) override;

    /**
     * Draw an horizontal line on screen.
     * Instead of line(), this member function takes an array of colors to be
     * able to individually set pixel colors of a line.
     * \param p starting point of the line
     * \param colors an array of pixel colors whoase size must be b.x()-a.x()+1
     * \param length length of colors array.
     * p.x()+length must be <= display.width()
     */
    void scanLine(Point p, const Color *colors, unsigned short length) override;
    
    /**
     * \return a buffer of length equal to this->getWidth() that can be used to
     * render a scanline.
     */
    Color *getScanLineBuffer() override;
    
    /**
     * Draw the content of the last getScanLineBuffer() on an horizontal line
     * on the screen.
     * \param p starting point of the line
     * \param length length of colors array.
     * p.x()+length must be <= display.width()
     */
    void scanLineBuffer(Point p, unsigned short length) override;

    /**
     * Draw an image on the screen
     * \param p point of the upper left corner where the image will be drawn
     * \param i image to draw
     */
    void drawImage(Point p, const ImageBase& img) override;

    /**
     * Draw part of an image on the screen
     * \param p point of the upper left corner where the image will be drawn.
     * Negative coordinates are allowed, as long as the clipped view has
     * positive or zero coordinates
     * \param a Upper left corner of clipping rectangle
     * \param b Lower right corner of clipping rectangle
     * \param i Image to draw
     */
    void clippedDrawImage(Point p, Point a, Point b, const ImageBase& img) override;

    /**
     * Draw a rectangle (not filled) with the desired color
     * \param a upper left corner of the rectangle
     * \param b lower right corner of the rectangle
     * \param c color of the line
     */
    void drawRectangle(Point a, Point b, Color c) override;
    
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
        pixel_iterator() : x(0), y(0), disp(nullptr) {}

        /**
         * Set a pixel and move the pointer to the next one
         * \param color color to set the current pixel
         * \return a reference to this
         */
        pixel_iterator& operator= (Color color)
        {
            if(disp==nullptr) return *this;
            disp->doSetPixel(x,y,conv1(color));
            
            if(direction==RD)
            {
                if(++x>xe)
                {
                    x=xs;
                    y++;
                }
            } else {
                if(++y>ye)
                {
                    y=ys;
                    x++;
                }
            }
            return *this;
        }

        /**
         * Compare two pixel_iterators for equality.
         * They are equal if they point to the same location.
         */
        bool operator== (const pixel_iterator& itr)
        {
            return this->x==itr.x && this->y==itr.y;
        }

        /**
         * Compare two pixel_iterators for inequality.
         * They different if they point to different locations.
         */
        bool operator!= (const pixel_iterator& itr)
        {
            return this->x!=itr.x || this->y!=itr.y;
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
        
        /**
         * Must be called if not all pixels of the required window are going
         * to be written.
         */
        void invalidate() {}

    private:
        /**
         * Constructor
         * \param start Upper left corner of window
         * \param end Lower right corner of window
         * \param direction Iterator direction
         * \param disp Display we're associated
         */
        pixel_iterator(Point start, Point end, IteratorDirection direction,
                       DisplayGeneric4BPP *disp) : direction(direction), disp(disp)
        {
            x=xs=start.x();
            y=ys=start.y();
            xe=end.x();
            ye=end.y();
        }
        
        short x,xs,xe;
        short y,ys,ye;
        IteratorDirection direction;
        DisplayGeneric4BPP *disp;

        friend class DisplayGeneric4BPP; //Needs access to ctor
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
    
    /**
     * Destructor
     */
    ~DisplayGeneric4BPP();
    
protected:
    
    //FIXME: ignoring MXGUI_ORIENTATION
    const short int width;
    const short int height;
    
    const int fbSize; //Framebuffer is 4 bit per pixel

    unsigned char *backbuffer; ///< Display backbuffer (frontbuffer is in the display chip)

private:
    //FIXME: these displays are black and white with 4 bit per pixel, but we
    //still don't have 4 bit per pixel color format in the rest of the library,
    //so for now use 16 bit per pixel RGB565 format and take the 4 MSBs from the
    //blue channel. Images should be exported as RGB565 even if they are black
    //and white until this is fixed
    static unsigned char conv1(Color c) { return (c & 0x1f)>>1; }
    static unsigned char conv2(Color c) { unsigned char x=(c & 0x1f)>>1; return x | x<<4; }
    
    /**
     * Non bound checked no color conversion non virtual setPixel.
     */
    void doSetPixel(short x, short y, unsigned char cc)
    {
        int offset=(x+y*width)/2;
        if(x & 1)
            backbuffer[offset]=(backbuffer[offset] & 0b11110000) | cc;
        else
            backbuffer[offset]=(backbuffer[offset] & 0b00001111) | (cc<<4);
    }
    
    Color *buffer;             ///< For scanLineBuffer
    pixel_iterator last;       ///< Last iterator for end of iteration check
};

} //namespace mxgui
