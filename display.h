/***************************************************************************
 *   Copyright (C) 2011, 2012, 2013, 2014, 2015, 2016 by Terraneo Federico *
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

#include <utility>
#include <pthread.h>
#include <config/mxgui_settings.h>
#include "point.h"
#include "color.h"
#include "font.h"
#include "image.h"

namespace mxgui {

class Display;        //Forward declaration
class DrawingContext;

/**
 * \ingroup pub_iface
 * This class allows to register and retrieve one or more displays in the
 * system. Selected boards supported by miosix already have a display driver
 * provided. When working with those boards, the driver will be already
 * registered.
 * This design also allows to support multiple displays connected to the same
 * microcontroller.
 */
class DisplayManager
{
public:
    /**
     * \return a reference to the instance of the DisplayManager
     * Multiple calls return the same display instance (singleton)
     */
    static DisplayManager& instance();

    /**
     * \param id display id. This allows to select one of the registered
     * displays for boards that have multiple displays.
     * \return a reference to the display class with the given id
     * \throws exception if no display with the given id exists
     */
    Display& getDisplay(unsigned int id=0);

    /**
     * Register a display to the display manager.
     * \param display a pointer to the new display to be registered. Note that
     * there is no way to unregister a display. This is done on purpose, as
     * the applications may cache the reference returned by getDisplay(),
     * and so removing displays may cause dangling pointers. Moreover, display
     * subclasses are expected to be singletons.
     * \return the display id associated with the registered display
     */
    int registerDisplay(Display *display);

private:
    DisplayManager(const DisplayManager&)=delete;
    DisplayManager& operator=(const DisplayManager&)=delete;

    /**
     * Constructor
     */
    DisplayManager();

    pthread_mutex_t mutex;          ///< Mutex to support concurrent access
    std::vector<Display*> displays; ///< Registered displays
};

/**
 * If a board has a display driver, this function should be implemented by it
 * to register the display. An attempt to compile mxgui for a board for which
 * no display driver is provided will result in an undefined reference to this
 * function. It can be implemented by writing a custom display driver and
 * registering it.
 */
void registerDisplayHook(DisplayManager& dm);

/**
 * \ingroup pub_iface
 * Display class. This is the base class from which all display drivers should
 * derive. Contains member functions to to turn the display on or off.
 * For drawing onto the display, you need to instantiate a DrawingContext that
 * will lock the display mutex and allow thread-safe display access.
 */
class Display
{
public:
    /**
     * Constructor
     */
    Display();
    
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
    short int getHeight() const { return doGetHeight(); }

    /**
     * \return the display's width
     */
    short int getWidth() const { return doGetWidth(); }
    
    /**
     * Destructor
     */
    virtual ~Display();

private:
    Display(const Display&)=delete;
    Display& operator=(const Display&)=delete;
    
    /**
     * Turn the display On after it has been turned Off.
     * Display initial state is On.
     */
    virtual void doTurnOn()=0;

    /**
     * Turn the display Off. It can be later turned back On.
     */
    virtual void doTurnOff()=0;
    
    /**
     * Set display brightness. Depending on the underlying driver,
     * may do nothing.
     * \param brt from 0 to 100
     */
    virtual void doSetBrightness(int brt)=0;
    
    /**
     * \return the display's height
     */
    virtual short int doGetHeight() const=0;

    /**
     * \return the display's width
     */
    virtual short int doGetWidth() const=0;
    
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
     * This member function is used on some target displays to reset the
     * drawing window to its default value. You have to call beginPixel() once
     * before calling setPixel(). You can then make any number of calls to
     * setPixel() without calling beginPixel() again, as long as you don't
     * call any other member function in this class. If you call another
     * member function, for example line(), you have to call beginPixel() again
     * before calling setPixel().
     */
    virtual void beginPixel()=0;

    /**
     * Draw a pixel with desired color. You have to call beginPixel() once
     * before calling setPixel()
     * \param p point where to draw pixel
     * \param color pixel color
     */
    virtual void setPixel(Point p, Color color)=0;

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
     * Set colors used for writing text
     * \param fgcolor text color
     * \param bgcolor background color
     */
    virtual void setTextColor(Color fgcolor, Color bgcolor)=0;

    /**
     * \return the current foreground color.
     * The foreground color is used to draw text on screen
     */
    virtual Color getForeground() const=0;

    /**
     * \return the current background color.
     * The foreground color is used to draw text on screen
     */
    virtual Color getBackground() const=0;

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
     * Make all changes done to the display since the last call to update()
     * visible. This backends does not require it, so it is empty.
     */
    virtual void update()=0;

    pthread_mutex_t dispMutex; ///< To lock concurrent access to the display
    bool isDisplayOn;          ///< True if display is on

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
    DrawingContext(Display& display) : display(display)
    {
        pthread_mutex_lock(&display.dispMutex);
    }

    /**
     * Write text to the display. If text is too long it will be truncated
     * \param p point where the upper left corner of the text will be printed
     * \param text, text to print.
     */
    void write(Point p, const char *text)
    {
        display.write(p,text);
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
        display.clippedWrite(p,a,b,text);
    }

    /**
     * Clear the Display. The screen will be filled with the desired color
     * \param color fill color
     */
    void clear(Color color)
    {
        display.clear(color);
    }

    /**
     * Clear an area of the screen
     * \param p1 upper left corner of area to clear
     * \param p2 lower right corner of area to clear
     * \param color fill color
     */
    void clear(Point p1, Point p2, Color color)
    {
        display.clear(p1,p2,color);
    }

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
        display.beginPixel();
    }

    /**
     * Draw a pixel with desired color. You have to call beginPixel() once
     * before calling setPixel()
     * \param p point where to draw pixel
     * \param color pixel color
     */
    void setPixel(Point p, Color color)
    {
        display.setPixel(p,color);
    }

    /**
     * Draw a line between point a and point b, with color c
     * \param a first point
     * \param b second point
     * \param color line color
     */
    void line(Point a, Point b, Color color)
    {
        display.line(a,b,color);
    }

    /**
     * Draw an horizontal line on screen.
     * Instead of line(), this member function takes an array of colors to be
     * able to individually set pixel colors of a line.
     * \param p starting point of the line
     * \param colors an array of pixel colors whoase size must be b.x()-a.x()+1
     * \param length length of colors array.
     * p.x()+length must be <= display.width()
     */
    void scanLine(Point p, const Color *colors, unsigned short length)
    {
        display.scanLine(p,colors,length);
    }
    
    /**
     * \return a buffer of length equal to this->getWidth() that can be used to
     * render a scanline.
     */
    Color *getScanLineBuffer()
    {
        return display.getScanLineBuffer();
    }
    
    /**
     * Draw the content of the last getScanLineBuffer() on an horizontal line
     * on the screen.
     * \param p starting point of the line
     * \param length length of colors array.
     * p.x()+length must be <= display.width()
     */
    void scanLineBuffer(Point p, unsigned short length)
    {
        display.scanLineBuffer(p,length);
    }

    /**
     * Draw an image on the screen
     * \param p point of the upper left corner where the image will be drawn
     * \param img image to draw
     */
    void drawImage(Point p, const ImageBase& img)
    {
        display.drawImage(p,img);
    }

    /**
     * Draw part of an image on the screen
     * \param p point of the upper left corner where the image will be drawn.
     * Negative coordinates are allowed, as long as the clipped view has
     * positive or zero coordinates
     * \param a Upper left corner of clipping rectangle
     * \param b Lower right corner of clipping rectangle
     * \param img Image to draw
     */
    void clippedDrawImage(Point p, Point a, Point b, const ImageBase& img)
    {
        display.clippedDrawImage(p,a,b,img);
    }

    /**
     * Draw a rectangle (not filled) with the desired color
     * \param a upper left corner of the rectangle
     * \param b lower right corner of the rectangle
     * \param c color of the line
     */
    void drawRectangle(Point a, Point b, Color c)
    {
        display.drawRectangle(a,b,c);
    }

    /**
     * \return the display's height
     */
    short int getHeight() const
    {
        return display.doGetHeight();
    }

    /**
     * \return the display's width
     */
    short int getWidth() const
    {
        return display.doGetWidth();
    }

    /**
     * Set colors used for writing text
     * \param fgcolor text color
     * \param bgcolor background color
     */
    void setTextColor(Color fgcolor, Color bgcolor)
    {
        display.setTextColor(fgcolor,bgcolor);
    }
    
    /**
     * Set colors used for writing text
     * \param colors a pair where first is the foreground color, and second the
     * background one
     */
    void setTextColor(std::pair<Color,Color> colors)
    {
        display.setTextColor(colors.first,colors.second);
    }

    /**
     * \return the current foreground color.
     * The foreground color is used to draw text on screen
     */
    Color getForeground() const
    {
        return display.getForeground();
    }

    /**
     * \return the current background color.
     * The foreground color is used to draw text on screen
     */
    Color getBackground() const
    {
        return display.getBackground();
    }
    
    /**
     * \return a pari with the foreground and background color
     */
    std::pair<Color,Color> getTextColor() const
    {
        return std::make_pair(display.getForeground(),display.getBackground());
    }

    /**
     * Set the font used for writing text
     * \param font new font
     */
    void setFont(const Font& font)
    {
        display.setFont(font);
    }

    /**
     * \return the current font used to draw text
     */
    Font getFont() const
    {
        return display.getFont();
    }
    
    /**
     * Destructor
     */
    ~DrawingContext()
    {
        display.update();
        pthread_mutex_unlock(&display.dispMutex);
    }

private:
    DrawingContext(const DrawingContext&)=delete;
    DrawingContext& operator=(DrawingContext&)=delete;

    Display& display; ///< Underlying display object
};

} //namespace mxgui

#endif //DISPLAY_H
