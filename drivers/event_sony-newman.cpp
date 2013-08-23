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

#include "mxgui/mxgui_settings.h"
#include "kernel/sync.h"

#if defined(_BOARD_SONY_NEWMAN) && defined(MXGUI_LEVEL_2)

#include "event_sony-newman.h"
#include "miosix.h"
#include "interfaces/bsp.h"
#include "interfaces/endianness.h"
#include <algorithm>

using namespace miosix;
using namespace std;

namespace mxgui {

static const int pollPeriod=50;  ///< 50ms, check for events 20 times a second
static FastMutex touchMutex;     ///< To protect isTouchEnabled
static bool isTouchEnabled=true; ///< If true, can read touch data

//From Sony's website
struct I2C_regs_tiny
{
    unsigned char mode; //See defines above
 
    unsigned char command; //See defines above
 
    //sensor activity, one bit per sensor
    unsigned short sensorActiveMask;
 
    unsigned char bX; //X position.
 
    unsigned char bY; //Y position
};

struct I2C_regs
{
    unsigned char mode; //See defines above
 
    unsigned char command; //See defines above
 
    //sensor activity, one bit per sensor
    unsigned short sensorActiveMask;
 
    unsigned char bX; //X position.
 
    unsigned char bY; //Y position
 
    //Sensor difference counts, also used as parameter
    //values for commands to and from Master
    unsigned short wSns[9];
 
    unsigned short wSnsRaw[9];
    unsigned short wSnsBase[9];
};

/**
 * Enables and initializes the touchscreen
 */
static void initTouchscreen()
{
    touch::Touch_Reset_Pin::high();
    delayUs(50);
    touch::Touch_Reset_Pin::low();
    Thread::sleep(10); //Looks like the sensor boot takes ~7ms
}

/**
 * Disables touchscreen
 */
static void shutdownTouchscreen()
{
    //FIXME: Don't know how to turn the touchscreen off, for now I'll hold it
    //in reset state and hope it draws little current in reset.
    touch::Touch_Reset_Pin::high();
}

/**
 * Read the registers of the touchscreen controller related to the touch
 * data. Can only be called if the touchscreen is enabled
 */
static bool readTouchRegs(I2C1Driver& i2c, I2C_regs_tiny& regs)
{
    const int retryMax=10;
    for(int i=0;i<retryMax;i++)
    {
        Lock<FastMutex> l(i2cMutex);
        if(i2c.recv(TOUCH_I2C_ADDRESS,&regs,sizeof(regs))==false) continue;
        regs.sensorActiveMask=fromBigEndian16(regs.sensorActiveMask);
        return true;
    }
    return false;
}

/**
 * Called when someone is touching the screen, it returns the point of touch.
 * \return point of touch, or (-1,-1) if no one is touching the screen.
 */
static Point getTouchData()
{
    I2C_regs_tiny regs;
    {
        Lock<FastMutex> l(touchMutex);
        if(isTouchEnabled==false) return Point(-1,-1);

        I2C1Driver& i2c=I2C1Driver::instance();
        if(readTouchRegs(i2c,regs)==false) return Point(-1,-1);
    }
    
    // Taken from underverk's code
    // Tip from sony:
    // Sensor may lock up, so we need to watchdog each individual sensor and
    // re-init the controller if any of them stay "active" for too long.
    const int timeout=4000/pollPeriod; //4 seconds
    static unsigned char sensorOnCount[9];
    static bool firstTouch=true;
    bool error=false;
    for(int i=0;i<9;i++)
    {
        if(regs.sensorActiveMask & (1<<i))
        {
            if(++sensorOnCount[i]>=timeout) error=true;
        } else sensorOnCount[i]=0;
    }
    if(error)
    {
        for(int i=0;i<9;i++) sensorOnCount[i]=0;
        initTouchscreen();
        firstTouch=true;
        return Point(-1,-1);
    }
    
    if(regs.sensorActiveMask==0)
    {
        firstTouch=true;
        return Point(-1,-1);
    }
    
    if(firstTouch) //The position of the first touch seems unreliable
    {
        firstTouch=false;
        return Point(-1,-1);
    }
    return Point(127-regs.bX,127-regs.bY);
}

Queue<Event,10> eventQueue;

void callback(Event e)
{
    FastInterruptDisableLock dLock;
    eventQueue.IRQput(e);
}

void eventThread(void *)
{
    const int timeout=30*pollPeriod; //30s
    int timeoutCounter;
    bool aPrev=false;
    bool tPrev=false;
    Point pOld;
    for(;;)
    {
        Thread::sleep(pollPeriod);
        //Check buttons
        if(POWER_BTN_PRESS_Pin::value())
        {
            timeoutCounter=0;
            if(aPrev==false) callback(Event(EventType::ButtonA));
            aPrev=true;
        } else aPrev=false;
        //Check touchscreen
        Point p=getTouchData();
        if(p.x()>=0) //Is someone touching the screen?
        {
            timeoutCounter=0;
            //Ok, someone is touching the screen
            //did the touch point differ that much from the previous?
            if(abs(pOld.x()-p.x())>3 || abs(pOld.y()-p.y())>3 || !tPrev)
            {
                pOld=p;
                if(tPrev==false) callback(Event(EventType::TouchDown,pOld));
                else callback(Event(EventType::TouchMove,pOld));
            }
            tPrev=true;
        } else {
            //No, no one is touching the screen
            if(tPrev==true) callback(Event(EventType::TouchUp,pOld));
            tPrev=false;
        }
        if(++timeoutCounter==timeout) callback(Event(EventType::Timeout));
    }
}

void disableTouchscreen()
{
    Lock<FastMutex> l(touchMutex);
    isTouchEnabled=false;
    shutdownTouchscreen();
}

void enableTouchscreen()
{
    Lock<FastMutex> l(touchMutex);
    isTouchEnabled=true;
    initTouchscreen();
}

bool isTouchScreenEnabled()
{
    return isTouchEnabled;
}

//
// class InputHandlerImpl
//

InputHandlerImpl::InputHandlerImpl()
{
    initTouchscreen();
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

#endif //_BOARD_SONY_NEWMAN
