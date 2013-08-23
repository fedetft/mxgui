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

#ifndef MXGUI_LIBRARY
#error "This is header is private, it can be used only within mxgui."
#error "If your code depends on a private header, it IS broken."
#endif //MXGUI_LIBRARY

#ifndef DISPLAY_SONY_NEWMAN_H
#define	DISPLAY_SONY_NEWMAN_H

#ifdef _BOARD_SONY_NEWMAN

#include "mxgui/mxgui_settings.h"
#include "mxgui/point.h"
#include "mxgui/color.h"
#include "mxgui/font.h"
#include "mxgui/image.h"
#include "mxgui/iterator_direction.h"
#include "mxgui/misc_inst.h"
#include "mxgui/line.h"
#include "miosix.h"
#include <algorithm>

namespace mxgui {

//This display is 16 bit per pixel, chech that the color depth is properly
//configured
#ifndef MXGUI_COLOR_DEPTH_16_BIT
#error The LD7131 driver requires a color depth of 16bit per pixel
#endif

#ifndef MXGUI_ORIENTATION_VERTICAL
#error Unsupported orientation
#endif

class SPITransaction
{
public:
    SPITransaction()  { miosix::oled::OLED_nSS_Pin::low();  }
    ~SPITransaction() { miosix::oled::OLED_nSS_Pin::high(); }
};

class CommandTransaction
{
public:
    CommandTransaction()  { miosix::oled::OLED_A0_Pin::low();  }
    ~CommandTransaction() { miosix::oled::OLED_A0_Pin::high(); }
};

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
    void write(Point p, const char *text)
    {
        waitDmaCompletion();
        font.draw(*this,textColor,p,text);
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
        waitDmaCompletion();
        font.clippedDraw(*this,textColor,p,a,b,text);
    }

    /**
     * Clear the Display. The screen will be filled with the desired color
     * \param color fill color
     */
    void clear(Color color)
    {
        clear(Point(0,0),Point(width-1,height-1),color);
    }

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
    void beginPixel()
    {
        waitDmaCompletion();
        //TODO: uncomment this if we ever get access to the datasheet and find
        //a way to implement setCursor() in a proper way
        //imageWindow(Point(0,0),Point(width-1,height-1));//Restore default window
    }

    /**
     * Draw a pixel with desired color. You have to call beginPixel() once
     * before calling setPixel()
     * \param p point where to draw pixel
     * \param color pixel color
     */
    void setPixel(Point p, Color color)
    {
        //Very slow, but that's all we can do
        setCursor(p);
        SPITransaction t;
        writeRamBegin();
        writeRam(color);
        writeRamEnd();
    }

    /**
     * Draw a line between point a and point b, with color c
     * \param a first point
     * \param b second point
     * \param c line color
     */
    void line(Point a, Point b, Color color)
    {
        using namespace std;
        waitDmaCompletion();
        //Horizontal line speed optimization
        if(a.y()==b.y())
        {
            imageWindow(Point(min(a.x(),b.x()),a.y()),
                        Point(max(a.x(),b.x()),a.y()));
            int numPixels=abs(a.x()-b.x());
            pixel=color;
            startDmaTransfer(&pixel,numPixels+1,false);
            return;
        }
        //Vertical line speed optimization
        if(a.x()==b.x())
        {
            textWindow(Point(a.x(),min(a.y(),b.y())),
                        Point(a.x(),max(a.y(),b.y())));
            int numPixels=abs(a.y()-b.y());
            pixel=color;
            startDmaTransfer(&pixel,numPixels+1,false);
            return;
        }
        //General case, always works but it is much slower due to the display
        //not having fast random access to pixels
        Line::draw(*this,a,b,color);
    }

    /**
     * Draw an horizontal line on screen.
     * Instead of line(), this member function takes an array of colors to be
     * able to individually set pixel colors of a line.
     * \param p starting point of the line
     * \param colors an array of pixel colors whose size must be b.x()-a.x()+1
     * \param length length of colors array.
     * p.x()+length must be <= display.width()
     */
    void scanLine(Point p, const Color *colors, unsigned short length)
    {
        waitDmaCompletion();
        imageWindow(p,Point(width-1,p.y()));
        startDmaTransfer(colors,length,true);
        waitDmaCompletion();
    }
    
    /**
     * \return a buffer of length equal to this->getWidth() that can be used to
     * render a scanline.
     */
    Color *getScanLineBuffer() { return buffers[which]; }
    
    /**
     * Draw the content of the last getScanLineBuffer() on an horizontal line
     * on the screen.
     * \param p starting point of the line
     * \param length length of colors array.
     * p.x()+length must be <= display.width()
     */
    void scanLineBuffer(Point p, unsigned short length)
    {
        waitDmaCompletion();
        imageWindow(p,Point(width-1,p.y()));
        startDmaTransfer(buffers[which],length,true);
        which= (which==0 ? 1 : 0);
    }

    /**
     * Draw an image on the screen
     * \param p point of the upper left corner where the image will be drawn
     * \param i image to draw
     */
    void drawImage(Point p, const ImageBase& img)
    {
        short int xEnd=p.x()+img.getWidth()-1;
        short int yEnd=p.y()+img.getHeight()-1;
        if(xEnd >= width || yEnd >= height) return;
        
        waitDmaCompletion();

        const unsigned short *imgData=img.getData();
        if(imgData!=0)
        {
            //Optimized version for memory-loaded images
            imageWindow(p,Point(xEnd,yEnd));
            int numPixels=img.getHeight()*img.getWidth();
            startDmaTransfer(imgData,numPixels,true);
            //If the image is in RAM don't overlap I/O, as the caller could
            //deallocate it. If it is in FLASH it's guaranteed to be const
            if(reinterpret_cast<unsigned int>(imgData)>=0x20000000)
                waitDmaCompletion();
        } else img.draw(*this,p);
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
    void clippedDrawImage(Point p, Point a, Point b, const ImageBase& img)
    {
        waitDmaCompletion();
        img.clippedDraw(*this,p,a,b);
    }

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
     * Set display brightness
     * \param brt from 0 to 100
     */
    void setBrightness(int brt);

    /**
     * Set colors used for writing text
     * \param fgcolor text color
     * \param bgcolor background color
     */
    void setTextColor(Color fgcolor, Color bgcolor)
    {
        Font::generatePalette(textColor,fgcolor,bgcolor);
    }

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
    void setFont(const Font& font) { this->font=font; }

    /**
     * \return the current font used to draw text
     */
    Font getFont() const { return font; }

    /**
     * Make all changes done to the display since the last call to update()
     * visible. This backends does not require it, so it is empty.
     */
    void update() { waitDmaCompletion(); }

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
            writeRam(color);
            if(--pixelLeft==0) invalidate();
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
        void invalidate()
        {
            miosix::oled::OLED_nSS_Pin::high();
            writeRamEnd();
        }

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

private:
    static const short int width=128;
    static const short int height=128;

    /**
     * Set cursor to desired location
     * \param point where to set cursor (0<=x<127, 0<=y<127)
     */
    static inline void setCursor(Point p)
    {
        /*
         * This is not the most efficent way to set the cursor, as the window
         * command requires to send 9 bytes through the SPI. Each display
         * controller usually has a faster set cursor command, but we don't
         * have the datasheet...
         */
        window(p,p);
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
        writeReg(0x05,0x0c); //See the comment in the .cpp regarding reg 0x05
        window(p1,p2);
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
        writeReg(0x05,0x08); //See the comment in the .cpp regarding reg 0x05
        window(p1,p2);
    }
    
    /**
     * Common part of all window commands
     */
    static void window(Point p1, Point p2);
    
    /**
     * Sends command 0xc which seems to be the one to start sending pixels.
     * Also, change SPI interface to 16 bit mode
     */
    static void writeRamBegin()
    {
        CommandTransaction c;
        writeRam(0xc);
        //Change SPI interface to 16 bit mode, for faster pixel transfer
        SPI1->CR1=0;
        SPI1->CR1=SPI_CR1_SSM
            | SPI_CR1_SSI
            | SPI_CR1_DFF
            | SPI_CR1_MSTR
            | SPI_CR1_SPE;
    }
    
    /**
     * Used to send pixel data to the display's RAM, and also to send commands.
     * The SPI chip select must be low before calling this member function
     * \param data data to write
     */
    static unsigned short writeRam(unsigned short data)
    {
        SPI1->DR=data;
        while((SPI1->SR & SPI_SR_RXNE)==0) ;
        return SPI1->DR; //Note: reading back SPI1->DR is necessary.
    }
    
    /**
     * Ends a pixel transfer to the display
     */
    static void writeRamEnd()
    {
        //Put SPI back into 8 bit mode
        SPI1->CR1=0;
        SPI1->CR1=SPI_CR1_SSM
            | SPI_CR1_SSI
            | SPI_CR1_MSTR
            | SPI_CR1_SPE;
    }
    
    /**
     * Write data to a display register
     * \param reg which register?
     * \param data data to write
     */
    static void writeReg(unsigned char reg, unsigned char data);
    
    /**
     * Write data to a display register
     * \param reg which register?
     * \param data data to write, if null only reg will be written (zero arg cmd)
     * \param len length of data, number of argument bytes
     */
    static void writeReg(unsigned char reg, const unsigned char *data=0, int len=1);
    
    /**
     * Start a DMA transfer to the display
     * \param data pointer to the data to be written
     * \param length number of 16bit transfers to make to the display
     * \param increm if true, the DMA will increment the pointer between
     * successive transfers, otherwise the first 16bits pointed to will be
     * repeatedly read length times
     */
    void startDmaTransfer(const unsigned short *data, int length, bool increm);
    
    /**
     * Check if a pending DMA transfer is in progress, and wait for
     * its completion
     */
    void waitDmaCompletion();

    Color pixel; ///< Buffer of one pixel, for overlapped I/O
    Color buffers[2][128]; ///< Line buffers for scanline overlapped I/O
    int which; ///< Currently empty buffer
    /// textColors[0] is the background color, textColor[3] the foreground
    /// while the other two are the intermediate colors for drawing antialiased
    /// fonts.
    Color textColor[4];
    Font font; ///< Current font selected for writing text
};

} //namespace mxgui

#endif //_BOARD_SONY_NEWMAN

#endif //DISPLAY_SONY_NEWMAN_H
