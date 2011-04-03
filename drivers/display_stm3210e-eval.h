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

#ifndef DISPLAY_STM3210E_EVAL_H
#define	DISPLAY_STM3210E_EVAL_H

#ifdef _BOARD_STM3210E_EVAL

#include "mxgui/mxgui_settings.h"
#include "mxgui/point.h"
#include "mxgui/color.h"
#include "mxgui/font.h"
#include "mxgui/image.h"
#include "mxgui/iterator_direction.h"

namespace mxgui {

//This display is 16 bit per pixel, chech that the color depth is properly
//configured
#ifndef MXGUI_COLOR_DEPTH_16_BIT
#error The SPFD5408 driver requires a color depth of 16bit per pixel
#endif

class DisplayStm3210e_eval
{
public:
    /**
     * Constructor.
     * Do not instantiate objects of this type directly from application code,
     * use Display::instance() instead.
     */
    DisplayStm3210e_eval();

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
     * visible. This backends does not require it, so it is empty.
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
        pixel_iterator(): pixelLeft(0) {}

        /**
         * Set a pixel and move the pointer to the next one
         * \param color color to set the current pixel
         * \return a reference to this
         */
        pixel_iterator& operator= (Color color)
        {
            pixelLeft--;
            writeRam(color.value());
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

    private:
        /**
         * Constructor
         * \param pixelLeft number of remaining pixels
         */
        pixel_iterator(unsigned int pixelLeft): pixelLeft(pixelLeft) {}

        unsigned int pixelLeft; ///< How many pixels are left to draw

        friend class DisplayStm3210e_eval; //Needs access to ctor
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

private:
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
     * Contains the types of the display controllers supported
     */
    enum DisplayType {
        UNKNOWN=0,
        SPFD5408=1,
        ILI9320=2
    };

    /**
     * Detects the display controller at runtime, and initializes it
     */
    void displayDetectAndInit();

    /**
     * Initializes the ILI9320 LCD controller
     */
    void initSPFD5408();

    /**
     * Initializes the ILI9320 LCD controller
     */
    void initILI9320();

    /**
     * Set cursor to desired location
     * \param point where to set cursor (0<=x<240, 0<=y<320)
     */
    static inline void setCursor(Point p)
    {
        #ifdef MXGUI_ORIENTATION_VERTICAL
        writeReg(32,p.x());
        writeReg(33,p.y());
        #elif defined MXGUI_ORIENTATION_HORIZONTAL
        writeReg(32,p.y());
        writeReg(33,319-p.x());
        #elif defined MXGUI_ORIENTATION_VERTICAL_MIRRORED
        writeReg(32,239-p.x());
        writeReg(33,319-p.y());
        #else //MXGUI_ORIENTATION_HORIZONTAL_MIRRORED
        writeReg(32,239-p.y());
        writeReg(33,p.x());
        #endif
    }

    /**
     * Set a hardware window on the screen, optimized for writing text.
     * The GRAM increment will be set to up-to-down first, then left-to-right
     * which is the correct increment to draw fonts
     * \param p1 upper left corner of the window
     * \param p2 lower right corner of the window
     */
    static inline void textWindow(Point p1, Point p2)
    {
        #ifdef MXGUI_ORIENTATION_VERTICAL
        writeReg(0x03,0x1038);
        writeReg(0x50,p1.x());
        writeReg(0x51,p2.x());
        writeReg(0x52,p1.y());
        writeReg(0x53,p2.y());
        setCursor(p1);
        #elif defined MXGUI_ORIENTATION_HORIZONTAL
        writeReg(0x03,0x1010);
        writeReg(0x50,p1.y());
        writeReg(0x51,p2.y());
        writeReg(0x52,319-p2.x());
        writeReg(0x53,319-p1.x());
        setCursor(p1);
        #elif defined MXGUI_ORIENTATION_VERTICAL_MIRRORED
        writeReg(0x03,0x1008);
        writeReg(0x50,239-p2.x());
        writeReg(0x51,239-p1.x());
        writeReg(0x52,319-p2.y());
        writeReg(0x53,319-p1.y());
        setCursor(p1);
        #else //MXGUI_ORIENTATION_HORIZONTAL_MIRRORED
        writeReg(0x03,0x1020);
        writeReg(0x50,239-p2.y());
        writeReg(0x51,239-p1.y());
        writeReg(0x52,p1.x());
        writeReg(0x53,p2.x());
        setCursor(p1);
        #endif
    }

    /**
     * Set a hardware window on the screen, optimized for drawing images.
     * The GRAM increment will be set to left-to-right first, then up-to-down
     * which is the correct increment to draw images
     * \param p1 upper left corner of the window
     * \param p2 lower right corner of the window
     */
    static inline void imageWindow(Point p1, Point p2)
    {
        #ifdef MXGUI_ORIENTATION_VERTICAL
        writeReg(0x03,0x1030);
        writeReg(0x50,p1.x());
        writeReg(0x51,p2.x());
        writeReg(0x52,p1.y());
        writeReg(0x53,p2.y());
        setCursor(p1);
        #elif defined MXGUI_ORIENTATION_HORIZONTAL
        writeReg(0x03,0x1018);
        writeReg(0x50,p1.y());
        writeReg(0x51,p2.y());
        writeReg(0x52,319-p2.x());
        writeReg(0x53,319-p1.x());
        setCursor(p1);
        #elif defined MXGUI_ORIENTATION_VERTICAL_MIRRORED
        writeReg(0x03,0x1000);
        writeReg(0x50,239-p2.x());
        writeReg(0x51,239-p1.x());
        writeReg(0x52,319-p2.y());
        writeReg(0x53,319-p1.y());
        setCursor(p1);
        #else //MXGUI_ORIENTATION_HORIZONTAL_MIRRORED
        writeReg(0x03,0x1028);
        writeReg(0x50,239-p2.y());
        writeReg(0x51,239-p1.y());
        writeReg(0x52,p1.x());
        writeReg(0x53,p2.x());
        setCursor(p1);
        #endif
    }

    /**
     * Memory layout of the display.
     * This backend is meant to connect an stm32f103re to an LCD display with
     * an spfd5408 controller on the stm3210e_eval board.
     */
    struct DisplayMemLayout
    {
        volatile unsigned short IDX;//Index, select register to write
        volatile unsigned short RAM;//Ram, read and write from registers and GRAM
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

    /**
     * Initializes the hardware backend
     */
    void hardwareInit();
    
    DisplayType displayType;//Contains the display controller ID

    /// textColors[0] is the background color, textColor[3] the foreground
    /// while the other two are the intermediate colors for drawing antialiased
    /// fonts.
    Color textColor[4];
    Font font; ///< Current font selected for writing text
};

} //namespace mxgui

#endif //_BOARD_STM3210E_EVAL

#endif //DISPLAY_STM3210E_EVAL_H
