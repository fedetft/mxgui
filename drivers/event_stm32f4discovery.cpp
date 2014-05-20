/***************************************************************************
 *   Copyright (C) 2014 by Terraneo Federico                               *
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

#if defined(_BOARD_STM32F429ZI_STM32F4DISCOVERY) && defined(MXGUI_LEVEL_2)

#include "event_stm32f4discovery.h"
#include "miosix.h"
#include "util/software_i2c.h"
#include <algorithm>

using namespace miosix;
using namespace std;

namespace mxgui {

typedef Gpio<GPIOA_BASE,0> buttonA;
typedef Gpio<GPIOA_BASE,8> scl; //I2C3 SCL
typedef Gpio<GPIOC_BASE,9> sda; //I2C3 SDA
typedef Gpio<GPIOA_BASE,15> interrupt;

typedef SoftwareI2C<sda,scl> stmpe811;

enum stmpe811regs
{
    SYS_CTRL1=0x03,
    SYS_CTRL2=0x04,
    INT_CTRL=0x09,
    INT_EN=0x0a,
    TSC_CTRL=0x40,
    TSC_CFG=0x41,
    FIFO_TH=0x4a,
    FIFO_STA=0x4b,
    TSC_DATA=0xd7
};

static void stmpe811writeReg(unsigned char reg, unsigned char val)
{
    stmpe811::sendStart();
    stmpe811::send(0x82);
    stmpe811::send(reg);
    stmpe811::send(val);
    stmpe811::sendStop();
}

static void stmpe811readReg(unsigned char reg, int n, unsigned char *result)
{
    if(n<=0) return;
    stmpe811::sendStart();
    stmpe811::send(0x82);
    stmpe811::send(reg);
    stmpe811::sendStop();
    stmpe811::sendStart();
    stmpe811::send(0x82 | 1);
    for(int i=0;i<n-1;i++) result[i]=stmpe811::recvWithAck();
    result[n-1]=stmpe811::recvWithNack();
    stmpe811::sendStop();
}

static void touchFifoClear()
{
    stmpe811writeReg(FIFO_STA,0x01); //RESET FIFO
    stmpe811writeReg(FIFO_STA,0x00); //RESET FIFO
}

static Point getTouchData()
{
    unsigned char ctrl;
    stmpe811readReg(TSC_CTRL,1,&ctrl);
    if((ctrl & 0x80)==0) return Point(-1,-1);
    unsigned char tsData[3];
    stmpe811readReg(TSC_DATA,3,tsData);
    stmpe811writeReg(FIFO_STA,0x01); //RESET FIFO
    stmpe811writeReg(FIFO_STA,0x00); //RESET FIFO
    int x=static_cast<int>(tsData[0])<<4 | tsData[1]>>4;
    int y=((static_cast<int>(tsData[1]) & 0xf)<<8) | tsData[2];
    x=4095-x; //X is swapped
    //Calibration values. May vary from unit to unit
    const int xMin=300;
    const int xMax=3770;
    const int yMin=300;
    const int yMax=3880;

    x=((x-xMin)*240)/(xMax-xMin);
    y=((y-yMin)*320)/(yMax-yMin);
    x=min(239,max(0,x));
    y=min(319,max(0,y));
    return Point(x,y);
}

Queue<Event,10> eventQueue;

void callback(Event e)
{
    FastInterruptDisableLock dLock;
    eventQueue.IRQput(e);
}

void eventThread(void *)
{
    bool aPrev=false;
    bool tPrev=false;
    Point pOld;
    for(;;)
    {
        //TODO: use interrupts to avoid polling
        Thread::sleep(50); //Check for events 20 times a second
        //Check buttons
        if(buttonA::value()==1)
        {
            if(aPrev==false) callback(Event(EventType::ButtonA));
            aPrev=true;
        } else aPrev=false;
        //Check touchscreen
        Point p=getTouchData();
        if(p.x()>=0) //Is someone touching the screen?
        {
            //Ok, someone is touching the screen
            //Did the touch point differ that much from the previous?
            if(abs(pOld.x()-p.x())>3 || abs(pOld.y()-p.y())>3 || !tPrev)
            {
                pOld=p;
                if(tPrev==false) callback(Event(EventType::TouchDown,pOld));
                else callback(Event(EventType::TouchMove,pOld));
            }
            tPrev=true;
        } else {
            //No, no one is touching the screen
            if(tPrev==true)
            {
                touchFifoClear();
                callback(Event(EventType::TouchUp,pOld));
            }
            tPrev=false;
        }
    }
}

//
// class InputHandlerImpl
//

InputHandlerImpl::InputHandlerImpl()
{
    {
        FastInterruptDisableLock dLock;
        buttonA::mode(Mode::INPUT_PULL_DOWN);
        interrupt::mode(Mode::INPUT);
        stmpe811::init();
    }
    
    //To let the I2C voltages settle
    Thread::sleep(5);
    
    stmpe811writeReg(SYS_CTRL1,0x02); //SOFT_RESET=1
    Thread::sleep(10);
    stmpe811writeReg(SYS_CTRL1,0x00); //SOFT_RESET=0
    Thread::sleep(2);
    stmpe811writeReg(SYS_CTRL2,0x08); // !GPIO_OFF !TSC_OFF !ADC_OFF
    
    //Total time to read the touchscreen is
    //TDD*2+SETTLING*3+AVE*17.2us*3= ~ 17.5ms
    stmpe811writeReg(TSC_CFG,0xe4); //TSC_CFG= AVE=8, TDD=1ms, SETTLING=5ms
    stmpe811writeReg(FIFO_TH,0x01); //FIFO_TH= 1
    stmpe811writeReg(FIFO_STA,0x01); //RESET FIFO
    stmpe811writeReg(FIFO_STA,0x00); //RESET FIFO
    
    //This may allow the chip to go out of hibernate once touch detected
    stmpe811writeReg(INT_CTRL,0x01);
    stmpe811writeReg(INT_EN,0x01);

    stmpe811writeReg(TSC_CTRL,0x03); //TSC_CTRL=No window track, XY, Enabled
    
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

#endif //_BOARD_STM32F429ZI_STM32F4DISCOVERY
