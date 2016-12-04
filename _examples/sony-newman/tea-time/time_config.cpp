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

#include "time_config.h"
#include "simple_button.h"
#include <stdio.h>
#include <mxgui/display.h>
#include <mxgui/misc_inst.h>
#include <mxgui/level2/input.h>
#ifdef _MIOSIX
#include <interfaces/bsp.h>
#endif //_MIOSIX

#include "images/up.h"
#include "images/upClicked.h"
#include "images/dw.h"
#include "images/dwClicked.h"

using namespace std;
using namespace mxgui;

const char *days[]={"Sunday","Monday","Tuesday","Wednesday",
        "Thusday","Friday","Saturday"};

/**
 * A number class with user defined range checks
 */
class NumberWithRange
{
public:
    /**
     * Constructor
     * \param defaultValue the value of this number after construction
     * \param minValue the minimum value it can assume (included)
     * \param maxValue the maximum value it can assume (included)
     */
    NumberWithRange(int defaultValue, int minValue, int maxValue)
            : value(max(minValue,min(maxValue,defaultValue))),
              minValue(minValue), maxValue(maxValue) {}
    
    /**
     * Increment with rollover
     */
    void increment() { if(++value>maxValue) value=minValue; }
    
    /**
     * Decrement with rollover
     */
    void decrement() { if(--value<minValue) value=maxValue; }
    
    /**
     * \return the current value 
     */
    int get() const { return value; }
    
private:
    int value;
    const int minValue;
    const int maxValue;
};

static const int yStart=12;
static const int yUpButtons=yStart+12;
static const int yNumbers=yUpButtons+20;
static const int yDwButtons=yNumbers+21;
static const int numSpace=42;
static const int buttonSpace=50;

static void inputNumbers(Display& display, InputHandler& input, const char *name,
    const char *sep, NumberWithRange *numbers[3])
{
    {
        DrawingContext dc(display);
        dc.clear(Point(0,yStart),Point(dc.getWidth()-1,dc.getHeight()-1),black);
        dc.setFont(tahoma);
        dc.write(Point(0,yStart),name);
        dc.setFont(droid21);
        dc.write(Point(numSpace-8,yNumbers),sep);
        dc.write(Point(2*numSpace-8,yNumbers),sep);
    }
    SimpleImageButton up1(&up,&upClicked,Point(2,yUpButtons));
    SimpleImageButton up2(&up,&upClicked,Point(2+buttonSpace,yUpButtons));
    SimpleImageButton up3(&up,&upClicked,Point(2+2*buttonSpace,yUpButtons));
    SimpleImageButton dw1(&dw,&dwClicked,Point(2,yDwButtons));
    SimpleImageButton dw2(&dw,&dwClicked,Point(2+buttonSpace,yDwButtons));
    SimpleImageButton dw3(&dw,&dwClicked,Point(2+2*buttonSpace,yDwButtons));
    SimpleTextButton  ok(Point(0,108),
        Point(display.getWidth()-1,display.getHeight()-1),make_pair(64512,0),"Ok");
    SimpleButton *buttons[]={&up1,&up2,&up3,&dw1,&dw2,&dw3,&ok};
    const int numButtons=sizeof(buttons)/sizeof(buttons[0]);
    
    for(;;)
    {
        {
            DrawingContext dc(display);
            for(int i=0;i<numButtons;i++) buttons[i]->draw(dc);
            for(int i=0;i<3;i++)
            {
                char line[16];
                sprintf(line,"%02d",numbers[i]->get());
                dc.write(Point(numSpace*i,yNumbers),line);
            }
        }
        Event e=input.getEvent();
        switch(e.getEvent())
        {
            case EventType::TouchDown:
            case EventType::TouchMove:
                for(int i=0;i<numButtons;i++)
                    buttons[i]->handleTouchDown(e.getPoint());
                break;
            case EventType::TouchUp:
                for(int i=0;i<numButtons;i++)
                    buttons[i]->handleTouchUp(e.getPoint());
                break;
            default:
                break;
        }
        for(int i=0;i<3;i++) if(buttons[i]->isClicked()) numbers[i]->increment();
        for(int i=0;i<3;i++) if(buttons[3+i]->isClicked()) numbers[i]->decrement();
        if(ok.isClicked()) return;
    }
}

int inputWeekDay(Display& display, InputHandler& input)
{
    {
        DrawingContext dc(display);
        dc.clear(Point(0,yStart),Point(dc.getWidth()-1,dc.getHeight()-1),black);
        dc.setFont(tahoma);
        dc.write(Point(0,yStart),"Set weekday");
        dc.setFont(droid21);
    }
    SimpleImageButton up2(&up,&upClicked,Point(2+buttonSpace,yUpButtons));
    SimpleImageButton dw2(&dw,&dwClicked,Point(2+buttonSpace,yDwButtons));
    SimpleTextButton  ok(Point(0,108),
        Point(display.getWidth()-1,display.getHeight()-1),make_pair(64512,0),"Ok");
    SimpleButton *buttons[]={&up2,&dw2,&ok};
    const int numButtons=sizeof(buttons)/sizeof(buttons[0]);
    
    int result=0;
    for(;;)
    {
        {
            DrawingContext dc(display);
            for(int i=0;i<numButtons;i++) buttons[i]->draw(dc);
            for(int i=0;i<3;i++)
            {
                dc.write(Point(0,yNumbers),days[result]);
                short l=dc.getFont().calculateLength(days[result]);
                dc.clear(Point(l,yNumbers),Point(dc.getWidth()-1,yNumbers+
                    dc.getFont().getHeight()),black);
            }
        }
        Event e=input.getEvent();
        switch(e.getEvent())
        {
            case EventType::TouchDown:
            case EventType::TouchMove:
                for(int i=0;i<numButtons;i++)
                    buttons[i]->handleTouchDown(e.getPoint());
                break;
            case EventType::TouchUp:
                for(int i=0;i<numButtons;i++)
                    buttons[i]->handleTouchUp(e.getPoint());
                break;
            default:
                break;
        }
        if(up2.isClicked()) if(++result>6) result=0;
        if(dw2.isClicked()) if(--result<0) result=6;
        if(ok.isClicked()) return result;
    }
}

void configureTime()
{
    Display& display=DisplayManager::instance().getDisplay();
    InputHandler& input=InputHandler::instance();
    
    NumberWithRange day(1,1,31);
    NumberWithRange month(1,1,12);
    NumberWithRange year(2013,2013,2100);
    NumberWithRange *numbers2[]={&day,&month,&year};
    inputNumbers(display,input,"Set date","/",numbers2);
    
    NumberWithRange hour(0,0,23);
    NumberWithRange minutes(0,0,59);
    NumberWithRange seconds(0,0,59);
    NumberWithRange *numbers1[]={&hour,&minutes,&seconds};
    inputNumbers(display,input,"Set time",":",numbers1);
    
    struct tm t;
    t.tm_hour=hour.get(); t.tm_min=minutes.get(); t.tm_sec=seconds.get();
    t.tm_mday=day.get(); t.tm_mon=month.get()-1; t.tm_year=year.get()-1900;
    t.tm_wday=inputWeekDay(display,input);
    #ifdef _MIOSIX
    miosix::Rtc::instance().setTime(t);
    #endif //_MIOSIX
}
