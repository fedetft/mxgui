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

#ifndef MXGUI_LIBRARY
#error "This is header is private, it can be used only within mxgui."
#error "If your code depends on a private header, it IS broken."
#endif //MXGUI_LIBRARY

#ifdef _BOARD_STM32F429ZI_STM32F4DISCOVERY

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
#error The ILI9341 driver requires a color depth of 16bit per pixel
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
         * Note that since aIncr and sIncr are both zero all the writes will
         * happens to the same memory location, but we need a safe
         * /dev/null-like location where to write, which is dummy
         */
        pixel_iterator() : ctr(0), endCtr(0), aIncr(0), sIncr(0),
                dataPtr(&dummy) {}

        /**
         * Set a pixel and move the pointer to the next one
         * \param color color to set the current pixel
         * \return a reference to this
         */
        pixel_iterator& operator= (Color color)
        {
            *dataPtr=color;

            //This is to move to the adjacent pixel
            dataPtr+=aIncr;
            
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
                DisplayImpl *disp) : ctr(0), dataPtr(disp->framebuffer1)
        {
            //Compute the increment in the adjacent direction (aIncr) and in the
            //step direction (sIncr) depending on the direction
            dataPtr+=start.y()*disp->getWidth()+start.x();
            if(direction==RD)
            {
                endCtr=end.x()+1-start.x();
                aIncr=1;
                sIncr=disp->getWidth()-endCtr;
            } else {
                endCtr=end.y()+1-start.y();
                aIncr=disp->getWidth();
                sIncr=-aIncr*endCtr+1;
            }
        }

        unsigned short ctr;           ///< Counter to decide when to step
        unsigned short endCtr;        ///< When ctr==endCtr apply a step
        
        short aIncr;                  ///< Adjacent increment
        int sIncr;                    ///< Step increment           
        Color *dataPtr;               ///< Pointer to framebuffer
        
        static Color dummy;           ///< Invalid iterators write here

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

    /**
     * Pointer to the memory mapped display.
     */
    Color * const framebuffer1;
    Color *buffer; ///< For scanLineBuffer
    pixel_iterator last; ///< Last iterator for end of iteration check
    static const unsigned int bpp=sizeof(Color); ///< Bytes per pixel
    static const int numPixels=width*height; ///< Number of pixels of the display
};

} //namespace mxgui

#endif //_BOARD_STM32F429ZI_STM32F4DISCOVERY

#ifdef _BOARD_STM32F469NI_STM32F469I_DISCO

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
#error The OTM8009A driver requires a color depth of 16bit per pixel
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
     * Make all changes done to the display since the last call to update()
     * visible. This backend requires it.
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
         * Note that since aIncr and sIncr are both zero all the writes will
         * happens to the same memory location, but we need a safe
         * /dev/null-like location where to write, which is dummy
         */
        pixel_iterator() : ctr(0), endCtr(0), aIncr(0), sIncr(0),
                dataPtr(&dummy) {}

        /**
         * Set a pixel and move the pointer to the next one
         * \param color color to set the current pixel
         * \return a reference to this
         */
        pixel_iterator& operator= (Color color)
        {
            *dataPtr=color;

            //This is to move to the adjacent pixel
            dataPtr+=aIncr;
            
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
                DisplayImpl *disp) : ctr(0), dataPtr(disp->framebuffer1)
        {
            //Compute the increment in the adjacent direction (aIncr) and in the
            //step direction (sIncr) depending on the direction
            dataPtr+=start.y()*disp->getWidth()+start.x();
            if(direction==RD)
            {
                endCtr=end.x()+1-start.x();
                aIncr=1;
                sIncr=disp->getWidth()-endCtr;
            } else {
                endCtr=end.y()+1-start.y();
                aIncr=disp->getWidth();
                sIncr=-aIncr*endCtr+1;
            }
        }

        unsigned short ctr;           ///< Counter to decide when to step
        unsigned short endCtr;        ///< When ctr==endCtr apply a step
        
        short aIncr;                  ///< Adjacent increment
        int sIncr;                    ///< Step increment           
        Color *dataPtr;               ///< Pointer to framebuffer
        
        static Color dummy;           ///< Invalid iterators write here

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
    static const short int width=480;
    static const short int height=800;
    #elif defined MXGUI_ORIENTATION_HORIZONTAL
    static const short int width=800;
    static const short int height=480;
    #elif defined MXGUI_ORIENTATION_VERTICAL_MIRRORED || \
          defined MXGUI_ORIENTATION_HORIZONTAL_MIRRORED
    #error unsupported orientation
    #else
    #error No orientation defined
    #endif

    /**
     * Pointer to the memory mapped display.
     */
    Color * const framebuffer1;
    Color *buffer; ///< For scanLineBuffer
    pixel_iterator last; ///< Last iterator for end of iteration check
    static const unsigned int bpp=sizeof(Color); ///< Bytes per pixel
    static const int numPixels=width*height; ///< Number of pixels of the display
};

} //namespace mxgui

#endif //_BOARD_STM32F469NI_STM32F469I_DISCO
