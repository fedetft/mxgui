/***************************************************************************
 *   Copyright (C) 2011, 2012, 2013 by Terraneo Federico                   *
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

#include "display.h"
#include "drivers/display_stm3210e-eval.h"
#include "drivers/display_mp3v2.h"
#include "drivers/display_qt.h"
#include "drivers/display_win.h"
#include "drivers/display_strive.h"
#include "drivers/display_redbull_v2.h"
#include "drivers/display_bitsboard.h"
#include "drivers/display_sony-newman.h"
#include "drivers/display_stm32f4discovery.h"
#include "drivers/display_oledboard2.h"
#include "pthread_lock.h"

#if MXGUI_SETTINGS_VERSION != 100
#error Wrong mxgui_settings.h version. You need to upgrade it.
#endif

namespace mxgui {

//
// class Display
//

Display& Display::instance()
{
    static DisplayImpl implementation;
    static Display singleton(&implementation);
    return singleton;
}

void Display::turnOn()
{
    PthreadLock lock(dispMutex);
    if(isDisplayOn) return;
    isDisplayOn=true;
    pImpl->turnOn();
}

void Display::turnOff()
{
    PthreadLock lock(dispMutex);
    if(isDisplayOn==false) return;
    isDisplayOn=false;
    pImpl->turnOff();
}

void Display::setBrightness(int brt)
{
    PthreadLock lock(dispMutex);
    pImpl->setBrightness(brt);
}

short int Display::getHeight() const
{
    return pImpl->getHeight();
}

short int Display::getWidth() const
{
    return pImpl->getWidth();
}

Display::Display(DisplayImpl *impl) : pImpl(impl), isDisplayOn(true)
{
    pthread_mutexattr_t temp;
    pthread_mutexattr_init(&temp);
    pthread_mutexattr_settype(&temp,PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&dispMutex,&temp);
    pthread_mutexattr_destroy(&temp);
}

//
// class DrawingContext
//

DrawingContext::DrawingContext(Display& display) : display(display)
{
    pthread_mutex_lock(&display.dispMutex);
}

void DrawingContext::write(Point p, const char* text)
{
    display.pImpl->write(p,text);
}

void DrawingContext::clippedWrite(Point p, Point a, Point b, const char* text)
{
    display.pImpl->clippedWrite(p,a,b,text);
}

void DrawingContext::clear(Color color)
{
    display.pImpl->clear(color);
}

void DrawingContext::clear(Point p1, Point p2, Color color)
{
    display.pImpl->clear(p1,p2,color);
}

void DrawingContext::beginPixel()
{
    display.pImpl->beginPixel();
}

void DrawingContext::setPixel(Point p, Color color)
{
    display.pImpl->setPixel(p,color);
}

void DrawingContext::line(Point a, Point b, Color color)
{
    display.pImpl->line(a,b,color);
}

void DrawingContext::scanLine(Point p, const Color* colors, unsigned short length)
{
    display.pImpl->scanLine(p,colors,length);
}

Color *DrawingContext::getScanLineBuffer()
{
    return display.pImpl->getScanLineBuffer();
}

void DrawingContext::scanLineBuffer(Point p, unsigned short length)
{
    display.pImpl->scanLineBuffer(p,length);
}

void DrawingContext::drawImage(Point p, const ImageBase& img)
{
    display.pImpl->drawImage(p,img);
}

void DrawingContext::clippedDrawImage(Point p, Point a, Point b, const ImageBase& img)
{
    display.pImpl->clippedDrawImage(p,a,b,img);
}

void DrawingContext::drawRectangle(Point a, Point b, Color c)
{
    display.pImpl->drawRectangle(a,b,c);
}

short int DrawingContext::getHeight() const
{
    return display.pImpl->getHeight();
}

short int DrawingContext::getWidth() const
{
    return display.pImpl->getWidth();
}

void DrawingContext::setTextColor(Color fgcolor, Color bgcolor)
{
    display.pImpl->setTextColor(fgcolor,bgcolor);
}

void DrawingContext::setTextColor(std::pair<Color,Color> colors)
{
    display.pImpl->setTextColor(colors.first,colors.second);
}

Color DrawingContext::getForeground() const
{
    return display.pImpl->getForeground();
}

Color DrawingContext::getBackground() const
{
    return display.pImpl->getBackground();
}

std::pair<Color,Color> DrawingContext::getTextColor() const
{
    return std::make_pair(display.pImpl->getForeground(),
        display.pImpl->getBackground());
}

void DrawingContext::setFont(const Font& font)
{
    display.pImpl->setFont(font);
}

Font DrawingContext::getFont() const
{
    return display.pImpl->getFont();
}

DrawingContext::~DrawingContext()
{
    display.pImpl->update();
    pthread_mutex_unlock(&display.dispMutex);
}

} //namespace mxgui
