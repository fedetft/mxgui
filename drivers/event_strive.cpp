/***************************************************************************
 *   Copyright (C) 2011 by Yury Kuchura                               *
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

#if defined(_BOARD_STRIVE_MINI) && defined(MXGUI_LEVEL_2)

#include "event_strive.h"
#include "miosix.h"

using namespace miosix;
using namespace std;

namespace mxgui {

//Calibration values for raw TSC2046 data. May be specific for particular display.
static const uint32_t XMAX = 3800;
static const uint32_t XMIN = 400;
static const uint32_t XBW = XMAX - XMIN; //x bandwidth = 0...3400
static const uint32_t YMAX = 3750;
static const uint32_t YMIN = 255;
static const uint32_t YBW = YMAX - YMIN; //y bandwidth = 0...3495

/**
 Exchange byte over SPI interface.
 @note SPI is implemented by bitbanging because with APB1 clock of 72 MHz
 it is impossible to use hardware SPI1 on speeds below 285 kb/s. TSC2046
 requires speed of 125 kb/s or less.
 @param [in] byteTx Byte to transmit
 @return received byte
*/
static uint8_t SpiExchangeByte(uint8_t byteTx)
{
    uint8_t mask = 0x80;
    uint8_t rxByte = 0;
    while (mask)
    {
        (byteTx & mask) ? spi1::mosi::high() : spi1::mosi::low();
        miosix::delayUs(5);
        if (spi1::miso::value())
            rxByte |= mask;
        spi1::sck::high();
        miosix::delayUs(5);
        spi1::sck::low();
        mask >>= 1;
    }
    spi1::mosi::low();
    return rxByte;
}

/** Poll touchscreen controller once. \n
    @note This function returns raw data measured by ADC. It is an overhead
    to convert it to coordinates here. It's better to do this after filtering
    several measurements.
    @param [in] coord the structure that will be filled with raw ADC data
    @return true if successful, false if touch is released.
*/
static bool Poll2046Once(Point& coord)
{
    if (spi1::touchint::value())
        return false;

    //Disable context switching. It is enough.
    PauseKernelLock kLock;

    //Chip select
    spi1::ntouchss::low();
    delayUs(5);

    //16-bits, DFR mode, measure Y, X.
    static const uint8_t txBytes[9] = {0x90, 0x00, 0xD0, 0x00, 0x00};
    uint8_t rxBytes[9];
    uint32_t i;

    //Exchange bytes
    for (i = 0; i < 5; ++i)
    {
        rxBytes[i] = SpiExchangeByte(txBytes[i]);
    }
    
    //Deselect
    spi1::ntouchss::high();
    delayUs(2);

    //Touch has been released before ADC conversion is finished:
    //the result can be invalid.
    if (spi1::touchint::value())
        return false;

    //Now extract raw ADC values
    uint16_t rawY =  (uint16_t(rxBytes[1]) << 5) | (uint16_t(rxBytes[2]) >> 3);
    uint16_t rawX =  (uint16_t(rxBytes[3]) << 5) | (uint16_t(rxBytes[4]) >> 3);
    coord = Point(rawX, rawY);
    return true;
}

/** Poll touchscreen controller, filter data and calculate real coordinates.
    @param [in] coord the structure that will be filled with raw ADC data
    @return true if successful, false if touch is released.
*/
static bool Poll2046(Point& coord)
{
    Point polls[5];

    //Configure SPI pins for bitbanging
    spi1::sck::low();
    spi1::sck::mode(Mode::OUTPUT);
    spi1::miso::pullup();
    spi1::miso::mode(Mode::INPUT_PULL_UP_DOWN);
    spi1::mosi::mode(Mode::OUTPUT);
    spi1::ntouchss::high();
    spi1::ntouchss::mode(Mode::OUTPUT);
    spi1::touchint::pullup();
    spi1::touchint::mode(Mode::INPUT_PULL_UP_DOWN);
    delayUs(5);

    //Poll controller 5 times
    int i;
    for (i = 0; i < 5; ++i)
    {
        if (false == Poll2046Once(polls[i]))
            return false; //touch has been released
    }

    uint16_t max = 0;
    uint16_t min = 4096;
    int imin=0, imax=0;
    uint16_t x, y;

    //Now find max and min x to exclude
    for (i = 0; i < 5; ++i)
    {
        if (polls[i].x() < min) { min = polls[i].x(); imin = i; }
        if (polls[i].x() > max) { max = polls[i].x(); imax = i; }
    }
    if (imin == imax)
    {//no need to calculate median
        x = polls[imin].x();
    }
    else
    {
        x = 0;
        for (i = 0; i < 5; ++i)
        {
            if (i == imin || i == imax) continue; //Skip
            x += polls[i].x();
        }
        x /= 3; //Median value of x
    }

    //Repeat the same for y
    max = 0;
    min = 4096;
    for (i = 0; i < 5; ++i)
    {
        if (polls[i].y() < min) { min = polls[i].y(); imin = i; }
        if (polls[i].y() > max) { max = polls[i].y(); imax = i; }
    }

    if (imin == imax)
    {//no need to calculate median
        y = polls[imin].y();
    }
    else
    {
        y = 0;
        for (i = 0; i < 5; ++i)
        {
            if (i == imin || i == imax) continue;
            y += polls[i].y();
        }
        y /= 3; //Median value of y
    }

    if (x > XMAX) x = XMAX;
    else if (x < XMIN) x = XMIN;
    x -= XMIN;
    if (y > YMAX) y = YMAX;
    else if (y < YMIN) y = YMIN;
    y -= YMIN;

    //Now convert to real coordinates on screen
#if defined(MXGUI_ORIENTATION_VERTICAL)
    uint16_t resX = 240 - (uint32_t(y) * 240) / YBW;
    uint16_t resY = 320 - (uint32_t(x) * 320) / XBW;
#elif defined(MXGUI_ORIENTATION_HORIZONTAL)
    uint16_t resX = (uint32_t(x) * 320) / XBW;
    uint16_t resY = 240 - (uint32_t(y) * 240) / YBW;
#else
    #error Not implemented
#endif
    coord = Point(resX, resY);
    return true;
}

static Queue<Event,50> eventQueue;

static void callback(Event e)
{
    FastInterruptDisableLock dLock;
    eventQueue.IRQput(e);
}

static void eventThread(void*)
{
    bool tPrev=false;
    Point pOld;
    Point p;

    for(;;)
    {
        Thread::sleep(20); //Check for events 50 times a second
        //Check touchscreen
        if( Poll2046(p) )
        {//Someone is touching the screen
            //Did the touch point differ that much from the previous?
            //if(abs(pOld.x()-p.x()) > 3 || abs(pOld.y()-p.y()) > 3 || !tPrev)
            if( !tPrev || pOld != p)
            {
                pOld = p;
                if(tPrev == false) callback(Event(EventType::TouchDown, pOld));
                else callback(Event(EventType::TouchMove, pOld));
            }
            tPrev=true;
        }
        else
        {
            //No, no one is touching the screen
            if(tPrev==true) callback(Event(EventType::TouchUp, pOld));
            tPrev=false;
        }
    }// for(;;)
}//static void eventThread(void*)

//
// class InputHandlerImpl
//

InputHandlerImpl::InputHandlerImpl()
{
    //Note that this class is instantiated only once. Otherwise
    //we'd have to think a way to avoid creating multiple threads
    Thread::create(eventThread, STACK_MIN);
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

#endif //_BOARD_STRIVE_MINI
