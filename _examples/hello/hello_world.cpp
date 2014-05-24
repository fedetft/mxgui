
#include "mxgui/entry.h"
#include "mxgui/display.h"

using namespace mxgui;

ENTRY()
{
	{
		DrawingContext dc(Display::instance());
		dc.write(Point(0,0),"Hello world");
	}
	for(;;) ;
}
