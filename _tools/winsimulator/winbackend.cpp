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
#include "winbackend.h"
#include <process.h>
#include <windows.h>
#include "mxgui/display.h"
#include "mxgui/misc_inst.h"

/**
 * The mxgui application is started here.
 * This is the entry point of the background thread
 * Note: the entry point for the application should be main()
 * when compiling the application for Miosix, or entryPoint()
 * when compiling for the Qt simulator.
 * an #ifdef is suggested.
 */
extern int entryPoint();

static uintptr_t threadId = 0;

void __cdecl appThread(void* pParam)
{
    entryPoint();
    _endthread();
}

//
// class WinBackend
//

WinBackend& WinBackend::instance()
{
    static WinBackend result;
    return result;
}

//Called from Windows message handler at WM_PAINT
void WinBackend::start()
{
    if(started==true) return;

    {
        mxgui::Display& display = mxgui::DisplayManager::instance().getDisplay();
        mxgui::DrawingContext dc(display);
        dc.beginPixel();
        dc.clear(mxgui::black);
    }
    threadId = _beginthread(appThread, 0, NULL);
    started=true;
}

WinBackend::~WinBackend()
{
}

extern void DrawPixel(short x, short y, unsigned short color);

//Call Windows pixel drawer
void WinBackend::setPixel(short x, short y, unsigned short color)
{
    DrawPixel(x, y, color);
}
