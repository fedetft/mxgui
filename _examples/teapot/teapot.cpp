
#include "teapot_model.h"
#include "mxgui/entry.h"
#include "mxgui/display.h"
#include "mxgui/misc_inst.h"
#include "mxgui/level2/input.h"
#include "rendering_engine.h"
#include "fps_counter.h"
#include <cstdio>
#include <cstdlib>

using namespace std;
using namespace mxgui;

//
// Since mxgui level 2 isn't finished yet, but I need GUI buttons and textboxes
// NOW, the code in this file is a little lower level than what I'd like it to
// be, but will be fixed when level 2 is finished.
//

#ifndef _MIOSIX
#define siprintf sprintf //The low code size integer only sprintf
#endif //_MIOSIX

//Height and width of a text box used to write cpu and fps meters
const short textBoxH=21;
const short textBoxW=50;

//Upper and lower corner of the text box for printing fps
const Point fpsTextBoxA(30,180);
const Point fpsTextBoxB(fpsTextBoxA.x()+textBoxW-1,fpsTextBoxA.y()+textBoxH-1);

//Upper and lower corner of the text box for printing cpu usage
const Point cpuTextBoxA(150,180);
const Point cpuTextBoxB(cpuTextBoxA.x()+textBoxW-1,cpuTextBoxA.y()+textBoxH-1);

//Upper and lower corner of rendering mode button
const Point buttonModeA(120,220);
const Point buttonModeB(220,250);

//Upper and lower corner of fps capping button
const Point buttonFpsA(120,270);
const Point buttonFpsB(220,300);

/**
 * Update fps and cpu usage within the text boxes
 * \param fps current fps
 * \param cpu current cpu usage
 */
static void printFps(int fps, int cpu)
{
    DrawingContext dc(Display::instance());
    dc.setFont(droid21);
    dc.setTextColor(green,black);
    char line[16];
    siprintf(line,"%d",fps);
    int len=droid21.calculateLength(line);
    int sep=fpsTextBoxA.x()+textBoxW-len;
    dc.clear(fpsTextBoxA,Point(sep,fpsTextBoxB.y()),black);
    dc.write(Point(sep,fpsTextBoxA.y()),line);
    siprintf(line,"%d%%",cpu);
    len=droid21.calculateLength(line);
    sep=cpuTextBoxA.x()+textBoxW-len;
    dc.clear(cpuTextBoxA,Point(sep,cpuTextBoxB.y()),black);
    dc.write(Point(sep,cpuTextBoxA.y()),line);
}

/**
 * Draw a rectangle which is brighter in the bottom and right corner
 * \param dc drawing context
 * \param a upper left corner
 * \param b lower right corner
 */
static void shadowRectangle(DrawingContext& dc, Point a, Point b)
{
    dc.line(a,Point(b.x(),a.y()),darkGrey);
    dc.line(Point(b.x(),a.y()),b,lightGrey);
    dc.line(b,Point(a.x(),b.y()),lightGrey);
    dc.line(Point(a.x(),b.y()),a,darkGrey);
}

/**
 * Draw a button
 * \param dc drawing context
 * \param a upper left corner
 * \param b lower right corner
 * \param s button caption
 * \param click true if clicked
 */
void drawButton(DrawingContext& dc, Point a, Point b, const char *s, bool click)
{
    static const unsigned short tlp[]={
        61276,46420,31661,48565,33741,46452,31661,46452,57050
    };
    const Image tl(3,3,tlp); //Button top left
    static const unsigned short trp[]={
        31661,46420,61276,57050,31628,48565,48499,42161,31661
    };
    const Image tr(3,3,trp); //Button top right
    static const unsigned short blp[]={
        31661,57050,48499,48565,33741,42225,61276,46420,31661
    };
    const Image bl(3,3,blp); //Button bottom left
    static const unsigned short brp[]={
        48499,42161,31661,42225,33741,48565,31661,46420,61276
    };
    const Image br(3,3,brp); //Button bottom right
    static const unsigned short topColors[]={31628,59131,48499};
    static const unsigned short botColors[]={48499,48499,31628};
    //Draw button corners
    dc.drawImage(a,tl);
    dc.drawImage(Point(b.x()-2,a.y()),tr);
    dc.drawImage(Point(a.x(),b.y()-2),bl);
    dc.drawImage(Point(b.x()-2,b.y()-2),br);
    //Draw button surrounding lines
    for(int i=0;i<3;i++)
        dc.line(Point(a.x()+3,a.y()+i),Point(b.x()-2,a.y()+i),topColors[i]);
    for(int i=0;i<3;i++)
        dc.line(Point(a.x()+i,a.y()+3),Point(a.x()+i,b.y()-3),topColors[i]);
    for(int i=0;i<3;i++)
        dc.line(Point(b.x()-i,a.y()+3),Point(b.x()-i,b.y()-3),botColors[3-i]);
    for(int i=0;i<3;i++)
        dc.line(Point(a.x()+3,b.y()-i),Point(b.x()-2,b.y()-i),botColors[3-i]);
    Point aa(a.x()+3,a.y()+3);
    Point bb(b.x()-3,b.y()-3);
    if(click)
    {
        dc.clear(aa,bb,darkGrey);
        dc.setTextColor(black,darkGrey);
    } else {
        dc.clear(aa,bb,lightGrey);
        dc.setTextColor(black,lightGrey);
    }
    dc.setFont(droid11);
    int len=droid11.calculateLength(s);
    int dx=bb.x()-aa.x();
    int yy=aa.y()+(bb.y()-aa.y()-droid11.getHeight())/2;
    if(len<=dx) dc.write(Point(aa.x()+(dx-len)/2,yy),s);
}

/**
 * \return a solid rendering engine
 */
static RenderingEngine *configureSolid()
{
    SolidRenderingEngine *result=new SolidRenderingEngine;
    result->setModel(vertices,numVertices,polygons,numPolygons);
    result->setColors((const Color*)colors);
    result->setDrawArea(Point(1,1),Point(238,158));
    result->setTranslation(Point(120,160));
    return result;
}

/**
 * \return a wireframe rendering engine
 */
static RenderingEngine *configureWireframe()
{
    WireframeRenderingEngine *result=new WireframeRenderingEngine;
    result->setModel(vertices,numVertices,polygons,numPolygons);
    result->setDrawArea(Point(1,1),Point(238,158));
    result->setTranslation(Point(120,160));
    return result;
}

ENTRY()
{
    //Initial drawing, print fixed strings, buttons, etc.
    {
        DrawingContext dc(Display::instance());
        dc.setTextColor(black,grey);
        dc.clear(Point(0,160),Point(239,319),grey);
        shadowRectangle(dc,Point(0,0),Point(239,159));
        shadowRectangle(dc,Point(fpsTextBoxA.x()-1,fpsTextBoxA.y()-1),
                Point(fpsTextBoxB.x()+1,fpsTextBoxB.y()+1));
        dc.clear(fpsTextBoxA,fpsTextBoxB,black);
        dc.write(Point(fpsTextBoxB.x()+5,fpsTextBoxB.y()-15),"FPS");
        shadowRectangle(dc,Point(cpuTextBoxA.x()-1,cpuTextBoxA.y()-1),
                Point(cpuTextBoxB.x()+1,cpuTextBoxB.y()+1));
        dc.clear(cpuTextBoxA,cpuTextBoxB,black);
        dc.write(Point(cpuTextBoxB.x()+5,cpuTextBoxB.y()-15),"CPU");
        dc.write(Point(20,230),"Rendering mode");
        dc.write(Point(20,280),"FPS capping");
        drawButton(dc,buttonModeA,buttonModeB,"Wireframe",false);
        drawButton(dc,buttonFpsA,buttonFpsB,"Enabled",false);
    }

    //Default values: wireframe and fps capped
    RenderingEngine *engine=configureWireframe();
    FpsCounter fps;
    fps.setFpsCap(10);

    //Button state variables
    int renderingOption=0;
    const char *renderingStr[]={"Wireframe","Solid"};
    int fpsOption=0;
    const char *fpsStr[]={"Enabled","Disabled"};

    //Rendering loop
    for(float angle=0.0f;;angle-=10*M_PI/180.0f)
    {
        Matrix3f xfm=xrot(M_PI/8-M_PI/2)*zrot(angle)*scale(105+30*sin(angle/4));
        engine->setTransformMatrix(xfm);
        engine->render(Display::instance());
        printFps(fps.getFps(),fps.getCpuUsed());
        fps.sleepBetweenFrames();

        //Event handling code, the long and boring part
        Event e=InputHandler::instance().popEvent();
        switch(e.getEvent())
        {
            case EventType::Default:
            case EventType::TouchMove:
                break;
            case EventType::TouchDown:
                if(within(e.getPoint(),buttonFpsA,buttonFpsB))
                {
                    DrawingContext dc(Display::instance());
                    drawButton(dc,buttonFpsA,buttonFpsB,
                            fpsStr[fpsOption],true);
                } else if(within(e.getPoint(),buttonModeA,buttonModeB))
                {
                    DrawingContext dc(Display::instance());
                    drawButton(dc,buttonModeA,buttonModeB,
                            renderingStr[renderingOption],true);
                }
                break;
            case EventType::TouchUp:
                if(within(e.getPoint(),buttonFpsA,buttonFpsB))
                {
                    if(fpsOption==0)
                    {
                        fpsOption=1;
                        fps.setFpsCap(0);
                    } else {
                        fpsOption=0;
                        fps.setFpsCap(10);
                    }
                    {
                        DrawingContext dc(Display::instance());
                        drawButton(dc,buttonFpsA,buttonFpsB,
                                fpsStr[fpsOption],false);
                    }
                } else if(within(e.getPoint(),buttonModeA,buttonModeB))
                {
                    delete engine;
                    if(renderingOption==0)
                    {
                        renderingOption=1;
                        engine=configureSolid();
                    } else {
                        renderingOption=0;
                        engine=configureWireframe();
                    }
                    {
                        DrawingContext dc(Display::instance());
                        drawButton(dc,buttonModeA,buttonModeB,
                                renderingStr[renderingOption],false);
                    }
                }
                break;
            default:
                goto quit;
        }
    }
    quit:;
    delete engine;
    {
        DrawingContext dc(Display::instance()); //Restore defaults
        dc.setFont(droid11);
        dc.setTextColor(white,black);
    }
    return 0;
}
