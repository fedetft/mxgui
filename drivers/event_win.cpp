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
#include "event_win.h"
#include "pthread.h"
#include <list>
#include <windows.h>

using namespace std::tr1;

namespace mxgui {

static pthread_mutex_t eqMutex; ///< Mutex to guard the event queue
static HANDLE hEvnt;
static std::list<Event> eventQueue; ///< Queue of events from the GUI
static std::tr1::function<void ()> eventCallback;

struct EventWrapper
{
    EventWrapper()
    {
        hEvnt = CreateEvent(NULL, FALSE, FALSE, NULL);
    }
    ~EventWrapper()
    {
        CloseHandle(hEvnt);
    }
};

static EventWrapper __ev;

void addEvent(Event e)
{
    eqMutex.lock();
    if(eventQueue.size()>=100)
    {
        eqMutex.unlock();
        return;
    }    
    eventQueue.push_back(e); //Drop if queue too long
    SetEvent(hEvnt);
    eqMutex.unlock();
    if(eventCallback) eventCallback();
}

//
// class InputHandlerImpl
//

Event InputHandlerImpl::getEvent()
{
    eqMutex.lock();
    while(eventQueue.empty())
        WaitForSingleObject(hEvnt, 100);
    Event result=eventQueue.front();
    eventQueue.pop_front();
    eqMutex.unlock();
    return result;
}

Event InputHandlerImpl::popEvent()
{
    eqMutex.lock();
    if(eventQueue.empty())
    {
        eqMutex.unlock();
        return Event(); //Default constructed event
    }
    Event result=eventQueue.front();
    eventQueue.pop_front();
    eqMutex.unlock();
    return result;
}

function<void ()> InputHandlerImpl::registerEventCallback(function<void ()> cb)
{
    swap(eventCallback,cb);
    return cb;
}

} //namespace mxgui

#endif //_MIOSIX
