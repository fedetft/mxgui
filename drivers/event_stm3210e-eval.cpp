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

#include "mxgui/mxgui_settings.h"
#include "interfaces-impl/gpio_impl.h"

#if defined(_BOARD_STM3210E_EVAL) && defined(MXGUI_LEVEL_2)

#include "event_stm3210e-eval.h"
#include "miosix.h"

using namespace miosix;

namespace mxgui {

typedef Gpio<GPIOA_BASE,0>  button1;
typedef Gpio<GPIOC_BASE,13> button2;
typedef Gpio<GPIOG_BASE,8>  button3;

Queue<Event,10> eventQueue;

void callback(Event e)
{
    FastInterruptDisableLock dLock;
    eventQueue.IRQput(e);
}

void eventThread(void *)
{
    button1::mode(Mode::INPUT);
    button2::mode(Mode::INPUT);
    button3::mode(Mode::INPUT);

    bool aPrev=false;
    bool bPrev=false;
    bool cPrev=false;
    for(;;)
    {
        Thread::sleep(50); //Check for events 20 times a second
        //Check buttons
        if(button1::value()==0)
        {
            if(aPrev==false) callback(Event(EventType::ButtonA));
            aPrev=true;
        } else aPrev=false;
        if(button2::value()==0)
        {
            if(bPrev==false) callback(Event(EventType::ButtonB));
            bPrev=true;
        } else bPrev=false;
        if(button3::value()==0)
        {
            if(cPrev==false) callback(Event(EventType::ButtonC));
            cPrev=true;
        } else cPrev=false;
    }
}

//
// class InputHandlerImpl
//

InputHandlerImpl::InputHandlerImpl()
{
    //Note that this class is instantiated only once. Otherwise
    //we'd have to think a way to avoid creating multiple threads
    Thread::create(eventThread,STACK_MIN);
}

Event InputHandlerImpl::getEvent()
{
    Event result;
    eventQueue.get(result);
    return result;
}

Event InputHandlerImpl::popEvent()
{
    FastInterruptDisableLock dLock;
    Event result;
    if(eventQueue.isEmpty()==false) eventQueue.IRQget(result);
    return result;
}

} //namespace mxgui

#endif //_BOARD_STM3210E_EVAL
