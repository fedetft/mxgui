/***************************************************************************
 *   Copyright (C) 2024 by Daniele Cattaneo                                *
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

#include <entry.h>
#include <display.h>
#include <textbox.h>
#include <misc_inst.h>
#include <unistd.h>
#include <cstdlib>
#ifdef _MIOSIX
#include "miosix.h"
#endif

using namespace mxgui;

static const char loremIpsum[] = 
"** LOREM IPSUM **\n"
"Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
"tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
"quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
"consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
"cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat "
"non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.\n"
"Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
"tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
"quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
"consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
"cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat "
"non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.\n"
"Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
"tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
"quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
"consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
"cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat "
"non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.\n"
"Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
"tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
"quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
"consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
"cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat "
"non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.\n";

void randomOptionsDemo()
{
    static int loop=0;
    static const int options[] = {
        TextBox::CharWrap | TextBox::LeftAlignment | TextBox::RemovePartialLines,
        TextBox::WordWrap | TextBox::LeftAlignment | TextBox::ClipPartialLines,
        TextBox::CharWrap | TextBox::CenterAlignment | TextBox::ClipPartialLines,
        TextBox::WordWrap | TextBox::CenterAlignment | TextBox::RemovePartialLines,
        TextBox::CharWrap | TextBox::RightAlignment | TextBox::RemovePartialLines,
        TextBox::WordWrap | TextBox::RightAlignment | TextBox::ClipPartialLines};
    static const int optionsSz=sizeof(options)/sizeof(int);
    int borderTop=random()%32;
    int borderLeft=random()%32;
    int borderRight=random()%32;
    int borderBottom=random()%32;
    {
        DrawingContext dc(DisplayManager::instance().getDisplay());
        dc.clear(grey);
        dc.setFont((loop/optionsSz)%2==0 ? droid11 : droid21);
        dc.setTextColor({black,white});
        TextBox::draw(dc,
            Point(5,5),
            Point(dc.getWidth()-6,dc.getHeight()-6), 
            loremIpsum, 
            options[loop%optionsSz],
            borderTop,borderLeft,borderRight,borderBottom);
    }
    usleep(2000000);
    loop++;
}

void scrollingDemo()
{
    static int loop=0;
    static const int options[] = {
        TextBox::WordWrap | TextBox::LeftAlignment | TextBox::ClipPartialLines,
        TextBox::WordWrap | TextBox::CenterAlignment | TextBox::ClipPartialLines,
        TextBox::WordWrap | TextBox::RightAlignment | TextBox::ClipPartialLines};
    static const int optionsSz=sizeof(options)/sizeof(int);
    int dispHeight;
    {
        DrawingContext dc(DisplayManager::instance().getDisplay());
        dispHeight=dc.getHeight();
        dc.clear(grey);
    }
    bool reachedEnd=false;
    int endY=0;
    #ifdef _MIOSIX
    long long frameStartTime=miosix::getTime();
    long long frameTime=0;
    const long long targetFrameTime=16666666; // 50 FPS
    #endif
    for(int scrollY=-dispHeight-10; !reachedEnd;)
    {
        char frameTimeBuf[32];
        #ifdef _MIOSIX
        sniprintf(frameTimeBuf, 32, "%04lld.%03lld ms", frameTime/1000000, (frameTime/1000)%1000);
        #else
        snprintf(frameTimeBuf, 32, "0000.000 ms");
        #endif
        const char *stringPtr;
        {
            DrawingContext dc(DisplayManager::instance().getDisplay());
            dc.setFont(loop%2==0 ? droid11 : droid21);
            dc.setTextColor({black,white});
            stringPtr=TextBox::draw(dc,
                Point(5,5),
                Point(dc.getWidth()-6,dc.getHeight()-15), 
                loremIpsum, 
                options[loop%optionsSz],
                0,0,0,0,scrollY);
            dc.setFont(tahoma);
            dc.write(Point(5,dc.getHeight()-13),frameTimeBuf);
        }
        if(*stringPtr=='\0')
        {
            if(endY == 0) endY=scrollY;
            if(scrollY-endY>dispHeight+10) reachedEnd=true;
        }
        #ifdef _MIOSIX
        // constant speed even when lagging
        long long frameEndTime=miosix::getTime();
        frameTime=frameEndTime-frameStartTime;
        int dScrollY=static_cast<int>(frameTime/targetFrameTime)+1;
        scrollY+=dScrollY;
        frameStartTime+=dScrollY*targetFrameTime;
        miosix::Thread::nanoSleepUntil(frameStartTime);
        #else
        usleep(10000);
        scrollY++;
        #endif
    }
    loop++;
}

ENTRY()
{
    {
        DrawingContext dc(DisplayManager::instance().getDisplay());
        dc.clear(lightGrey);
    }
    for (int loop=0;; loop++)
    {
        if (loop%7==6) scrollingDemo();
        else randomOptionsDemo();
    }
}
