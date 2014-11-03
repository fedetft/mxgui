/***************************************************************************
 *   Copyright (C) 2011 by Terraneo Federico                               *
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

#ifndef _MIOSIX

#include "event_qt.h"
#include <list>
#include <boost/thread.hpp>

using namespace std::tr1;

namespace mxgui {

static boost::mutex eqMutex; ///< Mutex to guard the event queue
static boost::condition_variable eqCond; ///< Condvar for blocking getEvent
static std::list<Event> eventQueue; ///< Queue of events from the GUI
static std::tr1::function<void ()> eventCallback;

void addEvent(Event e)
{
    {
        boost::unique_lock<boost::mutex> l(eqMutex);
        if(eventQueue.size()>=100) return;
        eventQueue.push_back(e); //Drop if queue too long
        eqCond.notify_one();
    }
    if(eventCallback) eventCallback();
}

//
// class InputHandlerImpl
//

Event InputHandlerImpl::getEvent()
{
    boost::unique_lock<boost::mutex> l(eqMutex);
    while(eventQueue.empty()) eqCond.wait(l);
    Event result=eventQueue.front();
    eventQueue.pop_front();
    return result;
}

Event InputHandlerImpl::popEvent()
{
    boost::unique_lock<boost::mutex> l(eqMutex);
    if(eventQueue.empty()) return Event(); //Default constructed event
    Event result=eventQueue.front();
    eventQueue.pop_front();
    return result;
}

function<void ()> InputHandlerImpl::registerEventCallback(function<void ()> cb)
{
    boost::unique_lock<boost::mutex> l(eqMutex);
    swap(eventCallback,cb);
    return cb;
}

} //namespace mxgui

#endif //_MIOSIX
