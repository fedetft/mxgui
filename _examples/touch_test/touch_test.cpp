/***************************************************************************
 *   Copyright (C) 2010, 2011, 2012, 2013, 2014 by Terraneo Federico       *
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

#include "mxgui/entry.h"
#include "mxgui/display.h"
#include "mxgui/misc_inst.h"
#include "mxgui/level2/input.h"
#include <cstdio>
#include <cstring>

using namespace std;
using namespace mxgui;

ENTRY()
{
    Display& display=DisplayManager::instance().getDisplay();
    unsigned short maxX = display.getWidth()-1;
    unsigned short maxY = display.getHeight()-1;
    InputHandler& backend=InputHandler::instance();
    short oldX=0,oldY=0;
    for(;;)
    {
        Event e=backend.getEvent();
        switch(e.getEvent())
        {   
            case EventType::ButtonA:
                display.turnOff();
                return 0;
            case EventType::TouchDown:
            case EventType::TouchUp:
            case EventType::TouchMove:
            {
                DrawingContext dc(display);
                dc.line(Point(0,oldY),Point(maxX,oldY),black);
                dc.line(Point(oldX,0),Point(oldX,maxY),black);
                oldX=e.getPoint().x();
                oldY=e.getPoint().y();
                dc.line(Point(0,oldY),Point(maxX,oldY),white);
                dc.line(Point(oldX,0),Point(oldX,maxY),white);
                char line[128];
                siprintf(line,"(%d, %d)          ",oldX,oldY);
                dc.write(Point(0,0),line);
                break;
            }
            default:
                break;
        }
    }
}
