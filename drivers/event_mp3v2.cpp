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

#ifdef _BOARD_MP3V2

//This includes the definitions of button1, button2, yp, ym, xp, xm GPIOs
#include "hwmapping.h"

#include "event_mp3v2.h"
#include "miosix.h"
#include <algorithm>

using namespace miosix;
using namespace std;

namespace mxgui {

///The callback to call when an event is received
static EventCallback callback;

/**
 * Initialize ADC2.
 */
static void adcInit()
{
    InterruptDisableLock dLock;
	//Gpio to adc mapping. Note: using adc2 to read all pins to save power
	//pwrmgmt::vbat c5 adc12_in15
	//disp::yp      b0 adc12_in8
	//disp::ym      b1 adc12_in9
	//disp::xp      c3 adc123_in13
	//disp::xm      c4 adc12_in14
	//accel::x      c0 adc123_in10
	//accel::y      c1 adc123_in11
	//accel::z      c2 adc123_in12
	RCC->CFGR |= RCC_CFGR_ADCPRE_1; //ADC prescaler 72MHz/6=12MHz
	RCC->APB2ENR |= RCC_APB2ENR_ADC2EN;
	ADC2->CR1=0;
	ADC2->CR2=ADC_CR2_ADON; //The first assignment sets the bit
	//Calibrate ADC once at powerup
	ADC2->CR2=ADC_CR2_ADON | ADC_CR2_CAL;
	while(ADC2->CR2 & ADC_CR2_CAL) ;
	ADC2->SQR1=0; //Do only one conversion
	ADC2->SQR2=0;
	ADC2->SQR3=0;
	ADC2->SMPR1=1<<15; //7.5 cycles of sample period
}

/**
 * Read an ADC channel
 * \param input input channel, 0 to 15
 * \return ADC result
 */
static unsigned short adcRead(unsigned char input)
{
    ADC2->SQR3=input;
	ADC2->CR2=ADC_CR2_ADON;//Setting the bit while already @ 1 starts conversion
	while((ADC2->SR & ADC_SR_EOC)==0) ;
	return ADC2->DR;
}

/**
 * Called when someone is touching the screen, it returns the point of touch.
 * It is unspecified what it returns when no one is touching the screen.
 * Also, to avoid the race condition of calling this function just before
 * the finger is raised from the screen, you should check that someone is still
 * touching the screen after having called the function, and if not, discard
 * the returned point.
 * \return point of touch
 */
Point getTouchData()
{
    int x,y;
    {
        InterruptDisableLock dLock;
        disp::ym::high(); //Raising ym instead of yp because y is flipped
        disp::xp::mode(Mode::INPUT_ANALOG);
        disp::xm::mode(Mode::INPUT_ANALOG);
        delayUs(1);
        y=(adcRead(13)+adcRead(14)+adcRead(13)+adcRead(14))/4;

        disp::xp::mode(Mode::OUTPUT);
        disp::xm::mode(Mode::OUTPUT);
        disp::xp::high();
        disp::xm::low();
        disp::yp::mode(Mode::INPUT_ANALOG);
        disp::ym::mode(Mode::INPUT_ANALOG);
        delayUs(1);
        x=(adcRead(8)+adcRead(9)+adcRead(8)+adcRead(9))/4;
        //Leave the GPIOs in their default state for next time
        disp::yp::mode(Mode::OUTPUT);
        disp::ym::mode(Mode::OUTPUT);
        disp::yp::low();
        disp::ym::low();
        disp::xp::mode(Mode::INPUT_PULL_UP_DOWN);
        disp::xm::mode(Mode::INPUT_PULL_UP_DOWN);
        disp::xp::pullup();
        disp::xm::pullup();
    }
    //Calibration values. May vary from unit to unit
    const int xMin=230;
    const int xMax=1950;
    const int yMin=370;
    const int yMax=3780;
    
    x=((x-xMin)*240)/(xMax-xMin);
    y=((y-yMin)*320)/(yMax-yMin);
    x=min(239,max(0,x));
    y=min(319,max(0,y));
    return Point(x,y);
}

/**
 * Thread to check for events
 */
void eventThread(void *)
{
    disp::xp::mode(Mode::INPUT_PULL_UP_DOWN);
    disp::xp::pullup();
    disp::xm::mode(Mode::INPUT_PULL_UP_DOWN);
    disp::xm::pullup();

    bool aPrev=false;
    bool bPrev=false;
    bool tPrev=false;
    Point pOld;
    for(;;)
    {
        Thread::sleep(50); //Check for events 20 times a second
        //Check buttons
        if(button1::value()==0)
        {
            if(aPrev==false) callback(Event(EventType::ButtonB));
            aPrev=true;
        } else aPrev=false;
        if(button2::value()==0)
        {
            if(bPrev==false) callback(Event(EventType::ButtonA));
            bPrev=true;
        } else bPrev=false;
        //Check touchscreen
        if(disp::xp::value()==0)
        {
            Point p=getTouchData();
            //Check also after to avoid race condition
            if(disp::xp::value()==0)
            {
                if(tPrev==false) callback(Event(EventType::TouchDown,p));
                else callback(Event(EventType::TouchMove,p));
                pOld=p;
                tPrev=true;
            } else {
                if(tPrev==true) callback(Event(EventType::TouchUp,pOld));
                tPrev=false;
            }
        } else {
            if(tPrev==true) callback(Event(EventType::TouchUp,pOld));
            tPrev=false;
        }
    }
}

void initEventSystem(EventCallback cb)
{
    adcInit();
    callback=cb;
    //Note that this function is called only once. Otherwise
    //we'd have to think a way to avoid creating multiple threads
    Thread::create(eventThread,STACK_MIN);
}

} //namespace mxgui

#endif //_BOARD_MP3V2
