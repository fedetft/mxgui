#include <cstdio>
#include "miosix.h"
#include "mxgui/display.h"
#include "mxgui/misc_inst.h"

using namespace std;
using namespace miosix;
using namespace mxgui;

int main()
{
    auto& display=DisplayManager::instance().getDisplay();
    {
        DrawingContext dc(display);
        dc.setFont(droid21);
        dc.write(Point(0,0),"Miosix OS");
        dc.setFont(tahoma);
        dc.write(Point(0,droid21.getHeight()),"MXGUI graphics library");
    }
   for(;;) Thread::sleep(100);
}
