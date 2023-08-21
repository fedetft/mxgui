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

#ifndef MXGUI_LIBRARY
#error "This is header is private, it can be used only within mxgui."
#error "If your code depends on a private header, it IS broken."
#endif //MXGUI_LIBRARY

#ifndef DISPLAY_ST25DVDISCOVERY_H
#define DISPLAY_ST25DVDISCOVERY_H

#ifdef _BOARD_STM32F415VG_ST25DVDISCOVERY
#include "display.h"
#include "color.h"
#include "point.h"

namespace mxgui {

//This display is 16 bit per pixel, check that the color depth is properly
//configured
#ifndef MXGUI_COLOR_DEPTH_16_BIT
#error The ILI9341 driver requires a color depth of 16bit per pixel
#endif

/**
 * Send a command to the ILI9341 display controller
 * \param cmd command
 * \param len length of the (optional) argument, or 0 for commands without
 * arguments.
 */
void sendCmd(unsigned char cmd, int len, ...);

/**
 * Simply another flavour of sending data to Display
 * Useful for sending dynamically sized data
*/
class Transaction
{
public:
    Transaction(unsigned char cmd);
    void write(unsigned char c);
    ~Transaction();
};

class DisplayImpl : public Display
{

public:
    /**
     * \return an instance to this class (singleton)
     */
    static DisplayImpl& instance();

    /**
     * Turn the display On after it has been turned Off.
     * Display initial state is On.
     */
    void doTurnOn() override;

    /**
     * Turn the display Off. It can be later turned back On.
     */
    void doTurnOff() override;

    /**
     * Set display brightness. Depending on the underlying driver,
     * may do nothing.
     * \param brt from 0 to 100
     */
    void doSetBrightness(int brt) override;

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
     *  Write part of text to the display
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
     * This member function is used on some target displays to reset the
     * drawing window to its default value. You have to call beginPixel() once
     * before calling setPixel(). You can then make any number of calls to
     * setPixel() without calling beginPixel() again, as long as you don't
     * call any other member function in this class. If you call another
     * member function, for example line(), you have to call beginPixel() again
     * before calling setPixel().
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

    class pixel_iterator
    {
    public:

        /**
         * Default constructor, results in an invalid iterator.
         */
        pixel_iterator(): pixelLeft(0) {}

        /**
         * Set a pixel and move the pointer to the next one
         * \param color color to set the current pixel
         * \return a reference to this
         */
        pixel_iterator& operator= (Color color)
        {
            pixelLeft--;
            
            unsigned char lsb = color & 0xFF;
            unsigned char msb = (color >> 8) & 0xFF;

            wr->write(msb);
            wr->write(lsb);

            return *this;
        }

        /**
         * Compare two pixel_iterators for equality.
         * They are equal if they point to the same location.
         */
        bool operator== (const pixel_iterator& itr)
        {
            return this->pixelLeft==itr.pixelLeft;
        }

        /**
         * Compare two pixel_iterators for inequality.
         * They different if they point to different locations.
         */
        bool operator!= (const pixel_iterator& itr)
        {
            return this->pixelLeft!=itr.pixelLeft;
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
         * \param pixelLeft number of remaining pixels
         */
        pixel_iterator(unsigned int pixelLeft): pixelLeft(pixelLeft) 
        {
            wr = new Transaction(0x2c);
        }

        unsigned int pixelLeft; ///< How many pixels are left to draw

        Transaction* wr;

        friend class DisplayImpl;
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
    pixel_iterator end() const
    {
        // Default ctor: pixelLeft is zero
        return pixel_iterator();
    }

    /**
     * Destructor
     */
    ~DisplayImpl() override;

private:

    /**
     * Constructor.
     * Do not instantiate objects of this type directly from application code.
     */
    DisplayImpl();
    
    #if defined MXGUI_ORIENTATION_VERTICAL
    static const short int width=240;
    static const short int height=320;
    #elif defined MXGUI_ORIENTATION_HORIZONTAL || \
          defined MXGUI_ORIENTATION_VERTICAL_MIRRORED || \
          defined MXGUI_ORIENTATION_HORIZONTAL_MIRRORED
    #error unsupported orientation
    #else
    #error No orientation defined
    #endif

    Color* buffer; ///< For scanLineBuffer

    /**
     * This member function is used to set the cursor stored in memory, as well as
     * the SC,SP,EC,EP counters of display driver.
     * \param p1 top-left point of the rectangle
     * \param p2 bottom-right point of the rectangle
     */
    static inline void window(Point p1, Point p2)
    {
        int SC[] = {p1.x() & 0xff, (p1.x() >> 8) & 0xff};
        int EC[] = {p2.x() & 0xff, (p2.x() >> 8) & 0xff};
        int SP[] = {p1.y() & 0xff, (p1.y() >> 8) & 0xff};
        int EP[] = {p2.y() & 0xff, (p2.y() >> 8) & 0xff};

        sendCmd(0x2a,4,SC[1],SC[0],EC[1],EC[0]); //LCD_COLUMN_ADDR
        sendCmd(0x2b,4,SP[1],SP[0],EP[1],EP[0]); //LCD_PAGE_ADDR
        sendCmd(0x2c,0); //LCD_RAMWR
    }

    /**
     * Set a hardware window on the screen, optimized for writing text.
     * The GRAM increment will be set to up-to-down first, then left-to-right which
     * is the correct increment to draw fonts
     * \param p1 upper left corner of the window
     * \param p2 lower right corner of the window
     */
    static inline void textWindow(Point p1, Point p2)
    {
        #ifdef MXGUI_ORIENTATION_VERTICAL
        // p3 is p2 transposed relative to p1. So that the column and page addresses exchanges
        Point p3 = Point(p1.x()+p2.y()-p1.y(),p1.y()+p2.x()-p1.x()); 
        window(p1,p3);
        sendCmd(0x36,1,0x28); //LCD_MAC
        #elif defined MXGUI_ORIENTATION_HORIZONTAL
            #error Not implemented
        #elif defined MXGUI_ORIENTATION_VERTICAL_MIRRORED
            #error Not implemented
        #else //MXGUI_ORIENTATION_HORIZONTAL_MIRRORED
            #error Not implemented
        #endif
    }

    /**
     * Set a hardware window on the screen, optimized for drawing images.
     * The GRAM increment will be set to left-to-right first, then up-to-down which
     * is the correct increment to draw images
     * \param p1 upper left corner of the window
     * \param p2 lower right corner of the window
     */
    static inline void imageWindow(Point p1, Point p2)
    {
        #ifdef MXGUI_ORIENTATION_VERTICAL
        window(p1,p2);
        sendCmd(0x36,1,0x08); //LCD_MAC
        #elif defined MXGUI_ORIENTATION_HORIZONTAL
            #error Not implemented
        #elif defined MXGUI_ORIENTATION_VERTICAL_MIRRORED
            #error Not implemented
        #else //MXGUI_ORIENTATION_HORIZONTAL_MIRRORED
            #error Not implemented
        #endif
    }
};


} //namespace mxgui
#endif //_BOARD_STM32F415VG_ST25DVDISCOVERY

#endif //DISPLAY_ST25DVDISCOVERY_H