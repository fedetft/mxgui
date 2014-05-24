
#include <cstdio>
#include <miosix.h>
#include <mxgui/display.h>
#include <mxgui/misc_inst.h>
#include <mxgui/level2/input.h>

using namespace miosix;
using namespace mxgui;

int main()
{
    Rtc& rtc=Rtc::instance();
    InputHandler& input=InputHandler::instance();
    Display& display=Display::instance();
    struct tm start;
    start.tm_sec=30;
    start.tm_min=7;
    start.tm_hour=12;
    start.tm_mday=21;
    start.tm_mon=8;
    start.tm_year=2013;
    start.tm_wday=2;
    rtc.setTime(start);
    int mx=0;
    for(int i=0;;i++)
    {
		PowerManagement& pm=PowerManagement::instance();
        if(i%2==0) pm.setCoreFrequency(PowerManagement::FREQ_120MHz);
        else pm.setCoreFrequency(PowerManagement::FREQ_26MHz);
		LightSensor& ls=LightSensor::instance();
		bool usb=pm.isUsbConnected();
		bool chg=pm.isCharging();
        struct tm t=rtc.getTime();
		char s[64];
		char s2[64];
        char ti[64];
        char k[32];
		siprintf(s,"V=%d S=%d%%",pm.getBatteryVoltage(),pm.getBatteryStatus());
		mx=std::max(mx,ls.read());
        siprintf(s2,"L=%d\n",mx);
        siprintf(ti,"%d/%d/%d %d:%02d:%02d",t.tm_mday,t.tm_mon,t.tm_year,
                                            t.tm_hour,t.tm_min,t.tm_sec);
        siprintf(k,"%#x",rtc.notSetYet());
		{
			DrawingContext dc(Display::instance());
			dc.clear(black);
			dc.write(Point(0,0),usb?"USB connected":"USB disconnected");
			dc.write(Point(0,12),chg?"Charging":"Not charging");
			dc.write(Point(0,24),s);
			dc.write(Point(0,36),s2);
            dc.write(Point(0,48),ti);
            dc.write(Point(0,60),k);
		}
        
        //This puts the CPU in deep sleep
        //Actually, for best power saving you should also turn off the
        //display (display.turnOff();) and disable the touchscreen
        //(disableTouchscreen();)
		pm.goDeepSleep(500);
        
        if(input.popEvent().getEvent()==EventType::ButtonA)
        {
            display.turnOff();
            break;
        }
    }
}

