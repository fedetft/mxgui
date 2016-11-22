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

#ifndef DISPLAY_STRIVE_H
#define DISPLAY_STRIVE_H

#ifdef _BOARD_STRIVE_MINI

#include <config/mxgui_settings.h>
#include "display.h"
#include "point.h"
#include "color.h"
#include "font.h"
#include "image.h"
#include "iterator_direction.h"
#include "misc_inst.h"
#include "line.h"
#include <cstdio>
#include <cstring>
#include <algorithm>

namespace mxgui {

//This display is 16 bit per pixel, check that the color depth is properly
//configured
#ifndef MXGUI_COLOR_DEPTH_16_BIT
#error This IL9325 driver requires a color depth of 16bit per pixel
#endif

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
    void drawRectangle(Point a, Point b, Color c) override;;

    /**
     * Set colors used for writing text
     * \param fgcolor text color
     * \param bgcolor background color
     */
    void setTextColor(std::pair<Color,Color> colors) override;

    /**
     * \return a pair with the foreground and background colors.
     * Those colors are used to draw text on screen
     */
    std::pair<Color,Color> getTextColor() const override;

    /**
     * Set the font used for writing text
     * \param font new font
     */
    void setFont(const Font& font) override;

    /**
     * \return the current font used to draw text
     */
    Font getFont() const override;

    /**
     * Make all changes done to the display since the last call to update()
     * visible. This backends does not require it, so it is empty.
     */
    void update() override;
    
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
        pixel_iterator(): pixelLeft(0) {}

        /**
         * Set a pixel and move the pointer to the next one
         * \param color color to set the current pixel
         * \return a reference to this
         */
        pixel_iterator& operator= (Color color)
        {
            pixelLeft--;
            writeRam(color);
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
        pixel_iterator(unsigned int pixelLeft): pixelLeft(pixelLeft) {}

        unsigned int pixelLeft; ///< How many pixels are left to draw

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
    pixel_iterator end() const
    {
        //Default ctor: pixelLeft is zero.
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
    
    #if defined MXGUI_ORIENTATION_VERTICAL || \
        defined MXGUI_ORIENTATION_VERTICAL_MIRRORED
    static const short int width=240;
    static const short int height=320;
    #elif defined MXGUI_ORIENTATION_HORIZONTAL || \
          defined MXGUI_ORIENTATION_HORIZONTAL_MIRRORED
    static const short int width=320;
    static const short int height=240;
    #else
    #error No orientation defined
    #endif

    /**
     * Set cursor to desired location
     * \param point where to set cursor (0<=x<240, 0<=y<320)
     */
    static inline void setCursor(Point p)
    {
        #ifdef MXGUI_ORIENTATION_VERTICAL
        writeReg(0x20,p.x());
        writeReg(0x21,p.y());
        #elif defined MXGUI_ORIENTATION_HORIZONTAL
        writeReg(0x20,p.y());
        writeReg(0x21, 319-p.x());
        #elif defined MXGUI_ORIENTATION_VERTICAL_MIRRORED
        writeReg(0x20,239-p.x());
        writeReg(0x21,319-p.y());
        #else //MXGUI_ORIENTATION_HORIZONTAL_MIRRORED
        writeReg(0x20,239-p.y());
        writeReg(0x21,p.x());
        #endif
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
        writeReg(0x03,0x1038);//GRAM increment up-to-down first, then left-to-right
        writeReg(0x50, p1.x());
        writeReg(0x51, p2.x());
        writeReg(0x52, p1.y());
        writeReg(0x53, p2.y());
        setCursor(p1);
        #elif defined MXGUI_ORIENTATION_HORIZONTAL
        writeReg(0x03,0x1010);//GRAM increment left-to-right first, then up to down
        writeReg(0x50, p1.y());
        writeReg(0x51, p2.y());
        writeReg(0x52, 319-p2.x());
        writeReg(0x53, 319-p1.x());
        setCursor(p1);
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
        writeReg(0x03,0x1038);//GRAM increment up-to-down first, then left-to-right
        writeReg(0x50, p1.x());
        writeReg(0x51, p2.x());
        writeReg(0x52, p1.y());
        writeReg(0x53, p2.y());
        setCursor(p1);
        #elif defined MXGUI_ORIENTATION_HORIZONTAL
        writeReg(0x03,0x1010);//GRAM increment left-to-right first, then up to down
        writeReg(0x50, p1.y());
        writeReg(0x51, p2.y());
        writeReg(0x52, 319-p2.x());
        writeReg(0x53, 319-p1.x());
        setCursor(p1);
        #elif defined MXGUI_ORIENTATION_VERTICAL_MIRRORED
            #error Not implemented
        #else //MXGUI_ORIENTATION_HORIZONTAL_MIRRORED
            #error Not implemented
        #endif
    }

    /**
     * Memory layout of the display.
     * This backend is meant to connect an stm32f103ve to an OLED display with
     * an s6e63d6 controller on the mp3v2 board.
     */
    struct DisplayMemLayout
    {
        volatile unsigned short IDX;//Index, select register to write
        unsigned char padding[131070];
        union {
            //Ram, read and write from registers and GRAM
            volatile unsigned short RAM;
            // This is an hack on databus aliasing to work around a bottleneck
            // of the stm32's fsmc: writing an unsigned int is way faster than
            // writing two unsigned short, because for some reason the fsmc
            // seems to insert six cycles between two writes, but it does not
            // add those cycles between the two halfword of an unsigned int.
            // Writing an unsigned int will result  in two fast 16 bit writes
            // to two consecutive addresses, but since the display's register
            // select is wired to a16, these two adresses will be aliased to
            // the same location on the display.
            // In turn, this provides a fast way to write to the display that
            // can be used any time it is possible to write pixels two at a
            // time.
            volatile unsigned int TWOPIX_RAM;
        };
    };

    /**
     * Pointer to the memory mapped display.
     */
    static DisplayMemLayout *const DISPLAY;

    /**
     * Set the index register
     * \param reg register to select
     */
    static void writeIdx(unsigned char reg)
    {
        DISPLAY->IDX=reg;
    }

    /**
     * Write data to selected register
     * \param data data to write
     */
    static void writeRam(unsigned short data)
    {
        DISPLAY->RAM=data;
    }

    /**
     * Write data from selected register
     * \return data read from register
     */
    static unsigned short readRam()
    {
        return DISPLAY->RAM;
    }

    /**
     * Write data to a display register
     * \param reg which register?
     * \param data data to write
     */
    static void writeReg(unsigned char reg, unsigned short data)
    {
        DISPLAY->IDX=reg;
        DISPLAY->RAM=data;
    }

    /**
     * Read data from a display register
     * \param reg which register?
     * \return data read from register
     */
    static unsigned short readReg(unsigned char reg)
    {
        DISPLAY->IDX=reg;
        return DISPLAY->RAM;
    }
    
    Color *buffer; ///< For scanLineBuffer
    /// textColors[0] is the background color, textColor[3] the foreground
    /// while the other two are the intermediate colors for drawing antialiased
    /// fonts.
    Color textColor[4];
    Font font; ///< Current font selected for writing text
};

} //namespace mxgui

#endif //_BOARD_STRIVE_MINI

#endif //DISPLAY_STRIVE_H
