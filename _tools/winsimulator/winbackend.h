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
#ifndef _WINBACKEND_H_
#define _WINBACKEND_H_

#include <cstring>
#include "mxgui/mxgui_settings.h"

/**
 * Class that interfaces Windows GUI (running from main thread) and mxgui (running
 * in a background thread)
 */
class WinBackend
{
public:
    /**
     * Singleton
     * \return the only instance
     */
    static WinBackend& instance();

    /**
     * Terminate application thread at the end.
     */
    ~WinBackend();

    /**
     * Start application.
     * This starts the background thread.
     */
    void start();

    void setPixel(short x, short y, unsigned short color);
private:
    WinBackend(const WinBackend& );
    WinBackend& operator= (const WinBackend& );

    /**
     * Constructor
     */
    WinBackend(): started(false) {}

    bool started; ///< True if the background thread has already been started
};

#endif // _WINBACKEND_H_
