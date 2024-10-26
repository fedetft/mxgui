
#include <cstdio>
#include "miosix.h"
#include "mxgui/display.h"
#include "mxgui/misc_inst.h"
#include "display_depg0290b01.h"
#include "pic.h"

using namespace std;
using namespace miosix;
using namespace mxgui;

/*
 * On boards which do not have a built-in display, MXGUI requires you to
 * implement the registerDisplayHook callback to tell MXGUI which display to
 * use. If you want to adapt this example for a board that already has a
 * display, you can register a secondary display in the main with the following
 * line
 * \code
 * DisplayManager::instance().registerDisplay(new DisplayLy091wg14<sda,scl,reset>);
 * \endcode
 * And then get the display with DisplayManager::instance().getDisplay(1).
 * Note that 0 is the default display, 1 would be the secondary one.
 */
namespace mxgui {
void registerDisplayHook(DisplayManager& dm)
{
    dm.registerDisplay(new DisplayDepg0290b01);
}
} //namespace mxgui

int main()
{
    auto& display=DisplayManager::instance().getDisplay();
    {
        DrawingContext dc(display);
        dc.clear(white);
        dc.setFont(tahoma);
        dc.write(Point(0,0),"Miosix OS");
        dc.write(Point(0,tahoma.getHeight()),"MXGUI graphics library");
    }
    for(;;)
    {
        Thread::sleep(250);
        ledOn();
        Thread::sleep(250);
        ledOff();
    }
}
