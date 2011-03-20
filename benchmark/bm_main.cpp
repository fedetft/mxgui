
#ifdef _MIOSIX

#include "miosix.h"
#include "benchmark.h"
#include "mxgui/display.h"

#ifdef _BOARD_MP3V2
#include "hwmapping.h"
#endif //_BOARD_MP3V2

using namespace miosix;
using namespace mxgui;

int main()
{
    Display& display=Display::instance();
    Benchmark benchmark(display);
    benchmark.start();

    #ifdef _BOARD_MP3V2
    while(button1::value()) Thread::sleep(100);
    #else //_BOARD_MP3V2
    for(;;) Thread::sleep(100);
    #endif //_BOARD_MP3V2
}

#endif //_MIOSIX
