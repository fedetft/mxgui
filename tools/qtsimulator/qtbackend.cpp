/***************************************************************************
 *   Copyright (C) 2010 by Terraneo Federico                               *
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
//#include "application.h"
#include "window.h"
#include <iostream>

using namespace std;
using namespace boost;

/**
 * The mxgui application is started here.
 * This is the entry point of the background thread
 * Note: the entry point for the application should be main()
 * when compiling the application for Miosix, or entryPoint()
 * when compiling for the Qt simulator.
 * an #ifdef is suggested.
 */
extern int entryPoint();

//
// class QTBackend
//

QTBackend& QTBackend::instance()
{
    static QTBackend result;
    return result;
}

void QTBackend::start(shared_ptr<UpdateSignalSender> sender)
{
    this->sender=sender;
    thread t(entryPoint);
    t.detach();
}

boost::shared_ptr<UpdateSignalSender> QTBackend::getSender() const
{
    return sender;
}
