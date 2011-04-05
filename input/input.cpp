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

#include "input.h"
#include "mxgui/drivers/event_qt.h"
#include "mxgui/drivers/event_mp3v2.h"

#ifdef _MIOSIX
#include "miosix.h"
#else //_MIOSIX
#include <boost/thread.hpp>
#endif //_MIOSIX

namespace mxgui {

#ifdef _MIOSIX

miosix::Queue<Event,10> eventQueue;

void pushEvent(Event e)
{
    miosix::InterruptDisableLock dLock;
    eventQueue.IRQput(e);
}

//
// class InputHandler
//

static InputHandler *theInstance=0;

InputHandler& InputHandler::instance()
{
    if(theInstance==0) theInstance=new InputHandler; //FIXME: thread unsafe
    return *theInstance;
}

Event InputHandler::getEvent()
{
    Event result;
    eventQueue.get(result);
    return result;
}

Event InputHandler::popEvent()
{
    miosix::InterruptDisableLock dLock;
    Event result;
    if(eventQueue.isEmpty()==false) eventQueue.IRQget(result);
    return result;
}

InputHandler::InputHandler()
{
    initEventSystem(pushEvent);
}

#else //_MIOSIX

boost::mutex eqMutex; ///< Mutex to guard the event queue
boost::condition_variable eqCond; ///< Condvar for blocking getEvent
std::list<Event> eventQueue; ///< Queue of events from the GUI

void pushEvent(Event e)
{
    boost::unique_lock<boost::mutex> l(eqMutex);
    if(eventQueue.size()<100) eventQueue.push_back(e); //Drop if queue too long
    eqCond.notify_one();
}

//
// class InputHandler
//

InputHandler& InputHandler::instance()
{
    static InputHandler result;
    return result;
}

Event InputHandler::getEvent()
{
    boost::unique_lock<boost::mutex> l(eqMutex);
    while(eventQueue.empty()) eqCond.wait(l);
    Event result=eventQueue.front();
    eventQueue.pop_front();
    return result;
}

Event InputHandler::popEvent()
{
    boost::unique_lock<boost::mutex> l(eqMutex);
    if(eventQueue.empty()) return Event(); //Default constructed event
    Event result=eventQueue.front();
    eventQueue.pop_front();
    return result;
}

InputHandler::InputHandler()
{
    initEventSystem(pushEvent);
}

#endif //_MIOSIX

} //namespace mxgui
