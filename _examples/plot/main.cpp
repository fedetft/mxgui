
#include <entry.h>
#include <display.h>
#include <level2/simple_plot.h>
#include <unistd.h>
#include <cmath>

using namespace std;
using namespace mxgui;

ENTRY()
{
    int i=0;
    vector<float> data1; 
    vector<float> data2;
    
    vector<Dataset> dataset;
    dataset.push_back(Dataset(data1,red));
    dataset.push_back(Dataset(data2,green));
    
    Display& display=DisplayManager::instance().getDisplay();
    SimplePlot plot1(Point(0,0),Point(239,200));
    
    for(;;i+=2)
    {
        {
            DrawingContext dc(display);
            plot1.draw(dc,dataset);
        }
        data1.push_back(20*(1+0.05*i)*sin(0.1*i));
        data2.push_back(20*(1+0.05*i));
        usleep(100000);
    }
}
