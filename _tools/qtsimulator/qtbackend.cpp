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
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>   *
 ***************************************************************************/

#include "qtbackend.h"
#include <cstdlib>
#include <thread>

/**
 * The mxgui application is started here.
 * This is the entry point of the background thread
 * Note: the entry point for the application should be main()
 * when compiling the application for Miosix, or entryPoint()
 * when compiling for the Qt simulator.
 * an #ifdef is suggested.
 */
extern int entryPoint();

void appThread()
{
    entryPoint();
    exit(0);
}

//
// class QTBackend
//

QTBackend& QTBackend::instance()
{
    static QTBackend result;
    return result;
}

void QTBackend::start(std::shared_ptr<UpdateSignalSender> sender)
{
    if(started==true) return;
    started=true;
    this->sender=sender;
    std::thread t(appThread);
    t.detach();
}
