
#include "benchmark.h"
#include "mxgui/misc_inst.h"
#include "checkpattern.h"
#include "micro_qr_code_from_wikipedia.h"
#include <cstdio>
#include <cstring>

using namespace std;
using namespace mxgui;
using namespace miosix;

//
// Class BenchmarkResult
//

BenchmarkResult::BenchmarkResult(const char name[25], unsigned int time,
        double fps): time(time), fps(fps)
{
    strcpy(this->name,name);
}

const char *BenchmarkResult::getName() const
{
    return name;
}

unsigned int BenchmarkResult::getTime() const
{
    return time;
}

unsigned int BenchmarkResult::getFps() const
{
    return fps;
}

void BenchmarkResult::print(mxgui::Display& d, mxgui::Point p)
{
    d.write(p,name);
    char line[64];
    int a=fps;
    int b=static_cast<int>(fps*100.0f)%100;
    sniprintf(line,63,"%d.%06d %d.%02d",time/1000000,time%1000000,a,b);
    d.write(Point(130,p.y()),line);
    iprintf("%s",name);
    for(unsigned int i=0;i<24-strlen(name);i++) putchar(' ');
    iprintf("%s\n",line);
}

//
// class Benchmark
//

Benchmark::Benchmark(mxgui::Display& display): display(display) {}

void Benchmark::start()
{
    //First, setup
    if(display.getWidth()!=240 || display.getHeight()!=320)
    {
        iprintf("Error: benchmark is designed for a display with a width\n"
                "of 240 pixels and a height of 320 pixels\n");
        return;
    }
    display.clear(black);

    //Then, do benchmarks
    fixedWidthTextBenchmark();
    variableWidthTextBenchmark();
    antialiasingBenchmark();
    horizontalLineBenchmark();
    verticalLineBenchmark();
    obliqueLineBenchmark();
    clearScreenBenchmark();
    imageBenchmark();

    //Last print results
    display.clear(black);
    display.setFont(tahoma);
    display.setTextColor(white,black);
    display.write(Point(0,0),"Benchamrk name                 Time         Fps");
    display.line(Point(0,12),Point(240,12),white);
    for(unsigned int i=0, j=13;i<results.size();i++,j+=12)
        results[i].print(display,Point(0,j));
}

void Benchmark::fixedWidthTextBenchmark()
{
    unsigned int totalTime=0;
    const char text[]="012345678901234567890123456789";
    display.setFont(miscFixed);
    for(int i=0;i<4;i++)
    {
        if(i%2==0) display.setTextColor(red,black);
        else display.setTextColor(green,black);
        timer.start();
        for(int j=0;j<320;j+=16)
            display.write(Point(0,j),text);
        timer.stop();
        totalTime+=timer.interval()*1000000/TICK_FREQ;
        timer.clear();
        delayMs(500);
    }
    totalTime/=4;
    results.push_back(BenchmarkResult("Monospace text",totalTime,
            1.0/(static_cast<double>(totalTime)/1000000)));
}

void Benchmark::variableWidthTextBenchmark()
{
    display.clear(black);
    display.setFont(tahoma);
    unsigned int totalTime=0;
    //This line with tahoma font is exactly 240 pixel wide
    const char text[]="abcdefghijklmnopqrtstuvwxyz0123456789%$! '&/";
    if(tahoma.calculateLength(text)!=240)
    {
        iprintf("Warning: line length is not 240, it is %d\n",
                tahoma.calculateLength(text));
    }
    for(int i=0;i<4;i++)
    {
        if(i%2==0) display.setTextColor(red,black);
        else display.setTextColor(green,black);
        timer.start();
        for(int j=0;j<320;j+=12) display.write(Point(0,j),text);
        timer.stop();
        totalTime+=timer.interval()*1000000/TICK_FREQ;
        timer.clear();
        delayMs(500);
    }
    totalTime/=4;
    results.push_back(BenchmarkResult("Variable width text",totalTime,
            1.0/(static_cast<double>(totalTime)/1000000)));
}

void Benchmark::antialiasingBenchmark()
{
    display.clear(black);
    display.setFont(droid11);
    unsigned int totalTime=0;
    const char text[]="abcdefghijklmnopqrtstuvwxyz0123456789%$! '&/";
    for(int i=0;i<4;i++)
    {
        if(i%2==0) display.setTextColor(red,black);
        else display.setTextColor(green,black);
        timer.start();
        for(int j=0;j<320;j+=12) display.write(Point(0,j),text);
        timer.stop();
        totalTime+=timer.interval()*1000000/TICK_FREQ;
        timer.clear();
        delayMs(500);
    }
    totalTime/=4;
    results.push_back(BenchmarkResult("Antialiased text",totalTime,
            1.0/(static_cast<double>(totalTime)/1000000)));
}

void Benchmark::horizontalLineBenchmark()
{
    unsigned int totalTime=0;
    for(int i=0;i<4;i++)
    {
        Color color=i%2==0?red:green;
        timer.start();
        for(int j=0;j<320;j++) display.line(Point(0,j),Point(239,j),color);
        timer.stop();
        totalTime+=timer.interval()*1000000/TICK_FREQ;
        timer.clear();
        delayMs(500);
    }
    totalTime/=4;
    results.push_back(BenchmarkResult("Horizontal lines",totalTime,
            1.0/(static_cast<double>(totalTime)/1000000)));
}

void Benchmark::verticalLineBenchmark()
{
    unsigned int totalTime=0;
    for(int i=0;i<4;i++)
    {
        Color color=i%2==0?red:green;
        timer.start();
        for(int j=0;j<240;j++) display.line(Point(j,0),Point(j,319),color);
        timer.stop();
        totalTime+=timer.interval()*1000000/TICK_FREQ;
        timer.clear();
        delayMs(500);
    }
    totalTime/=4;
    results.push_back(BenchmarkResult("Vertical lines",totalTime,
            1.0/(static_cast<double>(totalTime)/1000000)));
}

void Benchmark::obliqueLineBenchmark()
{
    unsigned int totalTime=0;
    const Color darkRed(0x7800);
    const Color darkGreen(0x3e00);
    const Color darkBlue(0x000f);
    for(int i=0;i<4;i++)
    {
        Color colorA=i%2==0?darkRed:darkGreen;
        Color colorB=i%2==0?darkGreen:darkBlue;
        Color colorC=i%2==0?darkBlue:darkRed;
        timer.start();
        for(int j=0;j<240;j++)
        {
            display.line(Point(j,0),Point(239,239-j),colorA);
        }
        for(int j=0;j<240;j++)
        {
            display.line(Point(0,320-240+j),Point(239-j,319),colorB);
        }
        for(int j=0;j<320-240;j++)
        {
            display.line(Point(0,1+j),Point(239,240+j),colorC);
        }
        timer.stop();
        totalTime+=timer.interval()*1000000/TICK_FREQ;
        timer.clear();
        delayMs(500);
    }
    totalTime/=4;
    results.push_back(BenchmarkResult("Oblique lines",totalTime,
            1.0/(static_cast<double>(totalTime)/1000000)));
}

void Benchmark::clearScreenBenchmark()
{
    unsigned int totalTime=0;
    for(int i=0;i<4;i++)
    {
        Color color=i%2==0?red:green;
        timer.start();
        display.clear(color);
        timer.stop();
        totalTime+=timer.interval()*1000000/TICK_FREQ;
        timer.clear();
        delayMs(500);
    }
    totalTime/=4;
    results.push_back(BenchmarkResult("Screen clear",totalTime,
            1.0/(static_cast<double>(totalTime)/1000000)));
}

void Benchmark::imageBenchmark()
{
    unsigned int totalTime=0;
    for(int i=0;i<2;i++)
    {
        timer.start();
        for(int j=0;j<240;j+=16)
            for(int k=0;k<320;k+=16)
                display.drawImage(Point(j,k),micro_qr_code_from_wikipedia);
        timer.stop();
        totalTime+=timer.interval()*1000000/TICK_FREQ;
        timer.clear();
        delayMs(250);
        timer.start();
        for(int j=0;j<240;j+=16)
            for(int k=0;k<320;k+=16)
                display.drawImage(Point(j,k),checkpattern);
        timer.stop();
        totalTime+=timer.interval()*1000000/TICK_FREQ;
        timer.clear();
        delayMs(250);
    }
    totalTime/=4;
    results.push_back(BenchmarkResult("Draw image",totalTime,
            1.0/(static_cast<double>(totalTime)/1000000)));
}
