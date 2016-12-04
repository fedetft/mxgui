/***************************************************************************
 *   Copyright (C) 2011 by Yury Kuchura                                    *
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
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>   *
 ***************************************************************************/
#include "window.h"
#include "winbackend.h"
#include "mxgui/drivers/event_win.h"
#include "mxgui/display.h"

using namespace mxgui;

//
// class Window
//

Window::Window()
{
}

void Window::mouseMoveEvent(int x, int y)
{
    if(x<0 || x>=DisplayManager::instance().getDisplay().getWidth()) return;
    if(y<0 || y>=DisplayManager::instance().getDisplay().getHeight()) return;
    addEvent(mxgui::Event(EventType::TouchMove,Point(x,y),EventDirection::DOWN));
}

void Window::mousePressEvent(int x, int y)
{
    if(x<0 || x>=DisplayManager::instance().getDisplay().getWidth()) return;
    if(y<0 || y>=DisplayManager::instance().getDisplay().getHeight()) return;
    addEvent(mxgui::Event(EventType::TouchDown,Point(x,y),EventDirection::DOWN));
}

void Window::mouseReleaseEvent(int x, int y)
{
    if(x<0 || x>=DisplayManager::instance().getDisplay().getWidth()) return;
    if(y<0 || y>=DisplayManager::instance().getDisplay().getHeight()) return;
    addEvent(mxgui::Event(EventType::TouchUp,Point(x,y),EventDirection::UP));
}

void Window::ButtonAEvent()
{
    addEvent(mxgui::Event(EventType::ButtonA,EventDirection::DOWN));
    //FIXME: we should also generate a button up event, but how?
}

void Window::ButtonBEvent()
{
    addEvent(mxgui::Event(EventType::ButtonB,EventDirection::DOWN));
    //FIXME: we should also generate a button up event, but how?
}
