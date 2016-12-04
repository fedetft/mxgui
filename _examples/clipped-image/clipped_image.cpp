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

#include "mxgui/entry.h"
#include "mxgui/display.h"
#include "mxgui/misc_inst.h"
#include "mxgui/tga_image.h"
#include "mxgui/resource_image.h"
#include "mxgui/level2/input.h"
#include <cstdio>
#include <cstring>
#include <unistd.h>

using namespace std;
using namespace mxgui;

static void drawSlowly(Display& display, Point p, const char *line)
{
    char cur[2]={0};
    while(*line!='\0')
    {
        *cur=*line++;
        {
            DrawingContext dc(display);
            dc.write(p,cur);
            p=Point(p.x()+dc.getFont().calculateLength(cur),p.y());
        }
        usleep(30000);
    }
}

static void blinkingDot(Display& display, Point p)
{
    for(int i=0;i<12;i++)
    {
        {
            DrawingContext dc(display);
            dc.write(p,i & 1 ? " " : ".");
        }
        usleep(200000);
    }
}

void bootMessage(Display& display)
{
    const char s0[]="Miosix";
    const char s1[]="We do what we must";
    const char s2[]="Because we can";
    const int s0pix=droid21.calculateLength(s0);
    const int s1pix=droid11.calculateLength(s1);
    const int s2pix=droid11.calculateLength(s2);
    int y=10;
    int width;
    {
        DrawingContext dc(display);
        dc.setFont(droid21);
        width=dc.getWidth();
        dc.write(Point((width-s0pix)/2,y),s0);
        y+=dc.getFont().getHeight();
        dc.line(Point((width-s1pix)/2,y),Point((width-s1pix)/2+s1pix,y),white);
        y+=4;
        dc.setFont(droid11);
    }

    drawSlowly(display,Point((width-s1pix)/2,y),s1);
    y+=droid11.getHeight();
    drawSlowly(display,Point((width-s2pix)/2,y),s2);
    blinkingDot(display,Point((width-s2pix)/2+s2pix,y));
}

ENTRY()
{
    Display& display=DisplayManager::instance().getDisplay();
    bootMessage(display);

    TgaImage img("dis.tga");
    InputHandler& backend=InputHandler::instance();
    short oldX=0,oldY=0;
    bool needClear=true;
    for(;;)
    {
        Event e=backend.getEvent();
        switch(e.getEvent())
        {
            case EventType::ButtonA:
            {
                DrawingContext dc(display);
                dc.drawImage(Point(0,0),img);
                needClear=true;
                break;
            }
                
            case EventType::ButtonB:
                display.turnOff();
                return 0;
            case EventType::TouchDown:
            case EventType::TouchUp:
            case EventType::TouchMove:
            {
                DrawingContext dc(display);
                if(e.hasValidPoint()==false) break;
                if(needClear)
                {
                    needClear=false;
                    dc.clear(black);
                    break;
                }
                const int k=80;
                int x=max(0,e.getPoint().x()-k/2);
                int y=max(0,e.getPoint().y()-k/2);
                Point a(min(x,239-k),min(y,319-k));
                Point b(a.x()+k,a.y()+k);

                if(a.x()!=oldX)
                {
                    short yMin=min(a.y(),oldY);
                    short yMax=max(a.y(),oldY)+k;
                    if(a.x()<oldX)
                        dc.clear(Point(a.x()+k,yMin),Point(oldX+k,yMax),black);
                    else
                        dc.clear(Point(oldX,yMin),Point(a.x(),yMax),black);
                }
                if(a.y()!=oldY)
                {
                    short xMin=min(a.x(),oldX);
                    short xMax=max(a.x(),oldX)+k;
                    if(a.y()<oldY)
                        dc.clear(Point(xMin,a.y()+k),Point(xMax,oldY+k),black);
                    else
                        dc.clear(Point(xMin,oldY),Point(xMax,a.y()),black);
                }
                dc.clippedDrawImage(Point(0,0),a,b,img);
                oldX=a.x();
                oldY=a.y();

//                dc.line(Point(0,oldY),Point(239,oldY),black);
//                dc.line(Point(oldX,0),Point(oldX,319),black);
//                oldX=e.getPoint().x();
//                oldY=e.getPoint().y();
//                dc.line(Point(0,oldY),Point(239,oldY),white);
//                dc.line(Point(oldX,0),Point(oldX,319),white);
                break;
            }
            default:
                break;
        }
    }
}
