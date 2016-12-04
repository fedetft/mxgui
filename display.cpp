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

#include "display.h"
#include "misc_inst.h"
#include "pthread_lock.h"

#if MXGUI_SETTINGS_VERSION != 101
#error Wrong mxgui_settings.h version. You need to upgrade it.
#endif

using namespace std;

namespace mxgui {

//
// class DisplayManager
//

DisplayManager& DisplayManager::instance()
{
    static DisplayManager singleton;
    return singleton;
}

Display& DisplayManager::getDisplay(unsigned int id)
{
    PthreadLock lock(mutex);
    return *displays.at(id);
}

int DisplayManager::registerDisplay(Display *display)
{
    PthreadLock lock(mutex);
    displays.push_back(display);
    return displays.size()-1;
}

DisplayManager::DisplayManager()
{
    pthread_mutex_init(&mutex,NULL);
    registerDisplayHook(*this);
}

//
// class Display
//

Display::Display() : isDisplayOn(true), font(miscFixed)
{
    pthread_mutexattr_t temp;
    pthread_mutexattr_init(&temp);
    pthread_mutexattr_settype(&temp,PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&dispMutex,&temp);
    pthread_mutexattr_destroy(&temp);
}

void Display::turnOn()
{
    PthreadLock lock(dispMutex);
    if(isDisplayOn) return;
    doTurnOn();
    isDisplayOn=true;
}

void Display::turnOff()
{
    PthreadLock lock(dispMutex);
    if(isDisplayOn==false) return;
    doTurnOff();
    isDisplayOn=false;
}

void Display::setBrightness(int brt)
{
    PthreadLock lock(dispMutex);
    doSetBrightness(brt);
}

void Display::setTextColor(pair<Color,Color> colors)
{
    Font::generatePalette(textColor,colors.first,colors.second);
}

pair<Color,Color> Display::getTextColor() const
{
    return make_pair(textColor[3],textColor[0]);
}

void Display::setFont(const Font& font) { this->font=font; }

Font Display::getFont() const { return font; }

void Display::update() {}

Display::~Display() {}

} //namespace mxgui
