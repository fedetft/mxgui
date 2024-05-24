
#include "mxgui/entry.h"
#include "mxgui/display.h"

using namespace mxgui;

ENTRY()
{
    {
        DrawingContext dc(DisplayManager::instance().getDisplay());
        dc.write(Point(0,1),u8"Hello World!");
    }
    for(;;) ;
}
