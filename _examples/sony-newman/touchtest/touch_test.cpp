
#include <cstdio>
#include <mxgui/display.h>
#include <mxgui/misc_inst.h>
#include <mxgui/level2/input.h>

using namespace mxgui;

int main()
{
    Display& display=DisplayManager::instance().getDisplay();
    const short w=display.getWidth();
    const short h=display.getHeight();
    InputHandler& input=InputHandler::instance();
    Point old;
    bool first=true;
    {
        DrawingContext dc(display);
        dc.setFont(droid21);
        dc.write(Point(0,53),"Touch me!");
        dc.setFont(tahoma);
    }
    for(int i=0;;i++)
    {
        Event e=input.getEvent();
        DrawingContext dc(display);
        if(first)
        {
            first=false;
            dc.clear(black);
        }
        char str[16];
        switch(e.getEvent())
        {
            case EventType::TouchDown:
            case EventType::TouchMove:
                dc.line(Point(0,old.y()),Point(w-1,old.y()),black);
                dc.line(Point(old.x(),0),Point(old.x(),h-1),black);
                old=e.getPoint();
                dc.line(Point(0,old.y()),Point(w-1,old.y()),white);
                dc.line(Point(old.x(),0),Point(old.x(),h-1),white);
                siprintf(str,"x=%03d y=%03d",old.x(),old.y());
                dc.write(Point(0,0),str);
                break;
            case EventType::TouchUp:
                dc.line(Point(0,old.y()),Point(w-1,old.y()),black);
                dc.line(Point(old.x(),0),Point(old.x(),h-1),black);
                break;
            case EventType::ButtonA:
                display.turnOff();
                return 0;
            default:
                break;
        }
    }
}
