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

#include "teapot_app.h"
#include "time_config.h"
#include "power_manager.h"
#include "simple_button.h"
#include <mxgui/display.h>
#include <mxgui/misc_inst.h>
#include <mxgui/level2/input.h>
#include <mxgui/_examples/teapot/rendering_engine.h>
#include <mxgui/_examples/teapot/fps_counter.h>
#include <mxgui/_examples/teapot/teapot_model.h>
#include <cstdio>
#include <cstdlib>
#ifdef _MIOSIX
#include <interfaces/bsp.h>
#endif //_MIOSIX

using namespace std;
using namespace mxgui;

const Point teapotAreaA(1,51);
const Point teapotAreaB(126,126);

//static void printCommon(DrawingContext& dc)
//{
//    Point end(dc.getWidth()-1,dc.getHeight()-1);
//    dc.drawRectangle(Point(0,12),end,green);
//    dc.drawRectangle(Point(teapotAreaA.x(),teapotAreaA.y()-1),end,green);
//}

static void printFps(DrawingContext& dc, int fps, int cpu)
{
    dc.setFont(tahoma);
    dc.setTextColor(green,black);
    char line[16];
    sprintf(line,"%02dfps",fps);
    dc.write(Point(25,0),line);
}

static void printTime(DrawingContext& dc)
{
    #ifdef _MIOSIX
    struct tm t=miosix::Rtc::instance().getTime();
    #else //_MIOSIX
    struct tm t;
    time_t tt=time(NULL);
    localtime_r(&tt,&t);
    #endif //_MIOSIX
    char timeStr[64];
    char dateStr[64];
    sprintf(timeStr,"%02d:%02d:%02d",t.tm_hour,t.tm_min,t.tm_sec);
    sprintf(dateStr,"%s %02d/%02d/%04d",days[t.tm_wday],
        t.tm_mday,t.tm_mon+1,t.tm_year+1900);
    short timeStrLen=droid21b.calculateLength(timeStr);
    short dateStrLen=tahoma.calculateLength(dateStr);
    short timeX=(dc.getWidth()-timeStrLen)/2;
    short dateX=(dc.getWidth()-dateStrLen)/2;
    const int dateY=14;
    const int timeY=29;
    dc.setFont(tahoma);
    dc.setTextColor(white,black);
    dc.clear(Point(1,dateY),Point(dateX-1,dateY+tahoma.getHeight()),black);
    dc.clear(Point(dateX+dateStrLen,dateY),Point(dc.getWidth()-2,
        dateY+tahoma.getHeight()),black);
    dc.write(Point(dateX,dateY),dateStr);
    dc.setFont(droid21b);
    dc.write(Point(timeX,timeY),timeStr);
}

enum ShutdownAnswer {answerYes, answerNo, answerNoAndSleep};

ShutdownAnswer shutdownQuestion(Display& display, InputHandler& input)
{
    {
        DrawingContext dc(display);
        dc.clear(Point(0,12),Point(dc.getWidth()-1,dc.getHeight()-1),black);
        dc.setFont(tahoma);
        dc.write(Point(0,12),"Really quit?");
    }
    SimpleTextButton yes(Point(0,64),Point(63,96),make_pair(white,black),"Yes");
    SimpleTextButton no(Point(64,64),Point(127,96),make_pair(white,black),"No");
    SimpleButton *buttons[]={&yes,&no};
    const int numButtons=sizeof(buttons)/sizeof(buttons[0]);
    for(;;)
    {
        {
            DrawingContext dc(display);
            for(int i=0;i<numButtons;i++) buttons[i]->draw(dc);
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
            #ifdef _MIOSIX
            case EventType::Timeout:
                return answerNoAndSleep;
            #endif //_MIOSIX
            default:
                break;
        }
        if(yes.isClicked()) return answerYes;
        if(no.isClicked())
        {
            DrawingContext dc(display);
            dc.clear(Point(0,12),Point(dc.getWidth()-1,dc.getHeight()-1),black);
            return answerNo;
        }
    }
}

/**
 * \return a solid rendering engine
 */
static RenderingEngine *configureSolid()
{
    SolidRenderingEngine *result=new SolidRenderingEngine;
    result->setModel(vertices,numVertices,polygons,numPolygons);
    result->setColors((const Color*)colors);
    result->setDrawArea(teapotAreaA,teapotAreaB);
    result->setTranslation(Point(64,130));
    return result;
}

/**
 * \return a wireframe rendering engine
 */
static RenderingEngine *configureWireframe()
{
    WireframeRenderingEngine *result=new WireframeRenderingEngine;
    result->setModel(vertices,numVertices,polygons,numPolygons);
    result->setDrawArea(teapotAreaA,teapotAreaB);
    result->setTranslation(Point(64,130));
    return result;
}

void teapotApp()
{
    Display& display=Display::instance();
    InputHandler& input=InputHandler::instance();
    PowerManager& power=PowerManager::instance();
    power.showBatterIcon(true);
    RenderingEngine *engine=configureSolid();
    enum { solid, wireframe } type;
    type=solid;
    FpsCounter fps;
    fps.setFpsCap(30); //Capped in software to 30fps

    //Rendering loop
    for(float angle=0.0f;;angle-=10*M_PI/180.0f)
    {
        Matrix3f xfm=xrot(M_PI/8-M_PI/2)*zrot(angle)*scale(55+15*sin(angle/4));
        engine->setTransformMatrix(xfm);
        engine->render(display);
        {
            DrawingContext dc(display);
            printFps(dc,fps.getFps(),fps.getCpuUsed());
            printTime(dc);
        }
        fps.sleepBetweenFrames();
        Event e=input.popEvent();
        switch(e.getEvent())
        {
            case EventType::TouchDown:
                if(within(e.getPoint(),teapotAreaA,teapotAreaB))
                {
                    delete engine;
                    if(type==solid)
                    {
                        type=wireframe;
                        engine=configureWireframe();
                    } else {
                        type=solid;
                        engine=configureSolid();
                    }
                } else if(within(e.getPoint(),Point(0,0),Point(127,12))) {
                    switch(shutdownQuestion(display,input))
                    {
                        case answerYes:
                            delete engine;
                            return;
                        case answerNo:
                            break;
                        case answerNoAndSleep:
                            power.powerSave();
                            power.showBatterIcon(true);
                            break;
                    }
                }
                break;
            #ifdef _MIOSIX
            case EventType::Timeout:
            #endif //_MIOSIX
            case EventType::ButtonA:
                power.powerSave();
                power.showBatterIcon(true);
                break;
            default:
                break;
        }
    }
}
