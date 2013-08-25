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

#include "power_manager.h"
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <mxgui/level2/input.h>
#include <mxgui/misc_inst.h>

//Icons
#include "images/batt0.h"
#include "images/batt25.h"
#include "images/batt50.h"
#include "images/batt75.h"
#include "images/batt100.h"
#include "images/batt0c.h"
#include "images/batt25c.h"
#include "images/batt50c.h"
#include "images/batt75c.h"
#include "images/batt100c.h"

using namespace std;
using namespace mxgui;
#ifdef _MIOSIX
using namespace miosix;
#endif //_MIOSIX

//
// class PowerManager
//

PowerManager& PowerManager::instance()
{
    static PowerManager singleton;
    return singleton;
}

void PowerManager::showBatterIcon(bool show)
{
    pthread_mutex_lock(&mutex);
    showIconFlag=show;
    drawBatteryIcon(); //Don't wait for the other thread, show it immediately
    pthread_mutex_unlock(&mutex);
}

void PowerManager::showTime(bool show)
{
    pthread_mutex_lock(&mutex);
    showTimeFlag=show;
    printTime(); //Don't wait for the other thread, show it immediately
    pthread_mutex_unlock(&mutex);
}

void PowerManager::powerSave()
{
#ifdef _MIOSIX
    display.turnOff();
    disableTouchscreen();
    PowerManagement::CoreFrequency cf=pmu.getCoreFrequency();
    pmu.setCoreFrequency(PowerManagement::FREQ_26MHz);
    
    //Wait for button released
    while(POWER_BTN_PRESS_Pin::value()) Thread::sleep(50);
    //Now go in deep sleep waiting for button pressed
    while(POWER_BTN_PRESS_Pin::value()==0)
    {
        pmu.setWakeOnButton(true);
        pmu.goDeepSleep(1000);
    }
    
    pmu.setCoreFrequency(cf);
    enableTouchscreen();
    display.turnOn();
    //Brightness may have changed since last time it was updated
    pthread_mutex_lock(&mutex);
    adjustBrightness();
    pthread_mutex_unlock(&mutex);
    
    //Now we have to make sure the input handler doesn't send out a button
    //pressed event. For this we wait a bit, and then flush all events
    Thread::sleep(50);
    InputHandler& input=InputHandler::instance();
    while(input.popEvent().getEvent()!=EventType::Default) ;
    
#else //_MIOSIX
    showBatterIcon(false);
    showTime(false);
    {
        DrawingContext dc(display);
        dc.clear(black);
    }
    display.turnOff();
    while(InputHandler::instance().getEvent().getEvent()!=EventType::ButtonA) ;
    display.turnOn();
#endif //_MIOSIX
}

void PowerManager::mainLoop()
{
    for(;;)
    {
        pthread_mutex_lock(&mutex);
        if(++count>=3) //Update battery status only every 3 seconds
        {
            count=0;
            if(showIconFlag) drawBatteryIcon();
            adjustBrightness();
        }
        if(showTimeFlag) printTime();
        pthread_mutex_unlock(&mutex);
        sleep(1);
    }
}

void PowerManager::drawBatteryIcon()
{
    #ifdef _MIOSIX
    int batteryVoltage=pmu.getBatteryStatus();
    bool charging=pmu.isCharging();
    #else //_MIOSIX
    int batteryVoltage=100; //No real battery monitor in the simulator
    bool charging=true;
    #endif //_MIOSIX
    {
        DrawingContext dc(display);
        if(batteryVoltage<20)
        {
            dc.drawImage(Point(0,0),charging ? batt0c : batt0);
        } else if(batteryVoltage<40) {
            dc.drawImage(Point(0,0),charging ? batt25c : batt25);
        } else if(batteryVoltage<60) {
            dc.drawImage(Point(0,0),charging ? batt50c : batt50);
        } else if(batteryVoltage<80) {
            dc.drawImage(Point(0,0),charging ? batt75c : batt75);
        } else {
            dc.drawImage(Point(0,0),charging ? batt100c : batt100);
        }
    }
}

void PowerManager::printTime()
{
    #ifdef _MIOSIX
    struct tm t=rtc.getTime();
    #else //_MIOSIX
    struct tm t;
    time_t tt=time(NULL);
    localtime_r(&tt,&t);
    #endif //_MIOSIX
    char timestring[16];
    sprintf(timestring,"%02d:%02d:%02d",t.tm_hour,t.tm_min,t.tm_sec);
    if(timeStrPixels==0) timeStrPixels=tahoma.calculateLength(timestring);
    DrawingContext dc(display);
    Font f=dc.getFont();
    pair<Color,Color> c=dc.getTextColor();
    dc.setFont(tahoma);
    dc.setTextColor(white,black);
    dc.write(Point(dc.getWidth()-1-timeStrPixels,0),timestring);
    dc.setFont(f);
    dc.setTextColor(c);
}

void PowerManager::adjustBrightness()
{
    #ifdef _MIOSIX
    DrawingContext dc(display); //Only for locking
    if(display.isOn())
    {
        int lightLevel=light.read();
        int brightness=15+min(85,lightLevel/2);
        if(abs(brightness-oldBrightness)>5)
        {
            oldBrightness=brightness;
            display.setBrightness(brightness);
        }
//        char str[32];
//        siprintf(str,"%03d %03d",lightLevel,oldBrightness);
//        dc.write(Point(0,12),str);
    }
    #endif //_MIOSIX
}

PowerManager::PowerManager() : display(Display::instance()),
#ifdef _MIOSIX
        pmu(PowerManagement::instance()), light(LightSensor::instance()),
        rtc(Rtc::instance()),
#endif //_MIOSIX
        showIconFlag(false), showTimeFlag(false), count(0), timeStrPixels(0),
        oldBrightness(40)
{
    pthread_mutex_init(&mutex,NULL);
    pthread_t thread;
    pthread_create(&thread,NULL,&threadLauncher,reinterpret_cast<void*>(this));
}
