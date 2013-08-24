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

#include "boot_message.h"
#include <mxgui/display.h>
#include <mxgui/misc_inst.h>
#include <unistd.h>

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

void bootMessage()
{
    Display& display=Display::instance();
    const char s0[]="Miosix";
    const char s1[]="We do what we must";
    const char s2[]="Because we can";
    const int s0pix=droid21.calculateLength(s0);
    const int s1pix=tahoma.calculateLength(s1);
    const int s2pix=tahoma.calculateLength(s2);
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
        dc.setFont(tahoma);
    }

    drawSlowly(display,Point((width-s1pix)/2,y),s1);
    y+=tahoma.getHeight();
    drawSlowly(display,Point((width-s2pix)/2,y),s2);
    blinkingDot(display,Point((width-s2pix)/2+s2pix,y));
    {
        DrawingContext dc(display);
        dc.clear(black);
    }
}
