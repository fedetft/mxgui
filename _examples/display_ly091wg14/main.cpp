
#include <cstdio>
#include "miosix.h"
#include "mxgui/display.h"
#include "mxgui/misc_inst.h"
#include "mxgui/drivers/display_ly091wg14.h"

using namespace std;
using namespace miosix;
using namespace mxgui;

// Configure these GPIOs to match how you connected the display
typedef Gpio<GPIOA_BASE,1> reset;
typedef Gpio<GPIOA_BASE,2> scl;
typedef Gpio<GPIOA_BASE,3> sda;

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
    dm.registerDisplay(new DisplayLy091wg14<sda,scl,reset>);
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
        dc.write(Point(0,32-tahoma.getHeight()),"MXGUI graphics library");
    }
    for(;;) Thread::sleep(100);
}
