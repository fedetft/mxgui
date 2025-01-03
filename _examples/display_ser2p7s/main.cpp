
#include <cstdio>
#include "miosix.h"
#include "mxgui/display.h"
#include "mxgui/misc_inst.h"
#include "display_ser2p7s.h"

using namespace std;
using namespace miosix;
using namespace mxgui;

//NOTE: to try this example, you have to connect the display to the board as
//written in display_ser2p7s.cpp

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
    dm.registerDisplay(new DisplaySer2p7s);
}
} //namespace mxgui

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
    for(int i=0;;i++)
    {
        {
            DrawingContext dc(display);
            char s[16];
            sniprintf(s,15,"%02d:%02d",i/60,i%60);
            dc.write(Point(0,droid21.getHeight()+tahoma.getHeight()),s);
        }
        Thread::sleep(1000);
    }
}
