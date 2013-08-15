/***************************************************************************
 *   Copyright (C) 2010, 2011 by Terraneo Federico                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   As a special exception, if other files instantiate templates or use   *
 *   macros or inline functions from this file, or you compile this file   *
 *   and link it with other works to produce a work based on this file,    *
 *   this file does not by itself cause the resulting work to be covered   *
 *   by the GNU General Public License. However the source code for this   *
 *   file must still be made available in accordance with the GNU General  *
 *   Public License. This exception does not invalidate any other reasons  *
 *   why a work based on this file might be covered by the GNU General     *
 *   Public License.                                                       *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>   *
 ***************************************************************************/

#include "benchmark.h"
#include "mxgui/misc_inst.h"
#include "mxgui/resource_image.h"
#include "checkpattern2.h"
#include "micro_qr_code_from_wikipedia.h"
#include "benchmark.h"
#include <cstdio>
#include <cstring>

using namespace std;
using namespace mxgui;
using namespace miosix;

//
// Class BenchmarkResult
//

BenchmarkResult::BenchmarkResult(const char name[20], unsigned int time)
        : time(time)
{
    strncpy(this->name,name,20);
    this->name[19]='\0';
}

void BenchmarkResult::print(DrawingContext& dc, Point p)
{
    const int minWidth=200; //Below this there's no room to print bench name
    const int numberSpace=76; //Number of pixels to write the numbers
    const int x=dc.getWidth()-1-numberSpace;
    if(dc.getWidth()>minWidth) dc.write(p,name);
    else dc.clippedWrite(p,Point(0,0),Point(x,dc.getHeight()-1),name);
    char line[64];
    int a=getFps()/100;
    int b=getFps()%100;
    sniprintf(line,63,"%d.%06d %d.%02d",time/1000000,time%1000000,a,b);
    dc.write(Point(dc.getWidth()>minWidth ? 130 : x+2,p.y()),line);
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
    index=0;
    {
        DrawingContext dc(display);
        dc.clear(black);
    }

    //Then, do benchmarks
    fixedWidthTextBenchmark();
    variableWidthTextBenchmark();
    #ifndef MXGUI_COLOR_DEPTH_1_BIT_LINEAR
    antialiasingBenchmark();
    #endif //MXGUI_COLOR_DEPTH_1_BIT_LINEAR
    horizontalLineBenchmark();
    verticalLineBenchmark();
    obliqueLineBenchmark();
    clearScreenBenchmark();
    imageBenchmark();
    #ifndef MXGUI_COLOR_DEPTH_1_BIT_LINEAR
    scanLineBenchmark();
    #endif //MXGUI_COLOR_DEPTH_1_BIT_LINEAR
    clippedDrawBenchmark();
    clippedWriteBenchmark();
    #ifdef MXGUI_ENABLE_RESOURCEFS
    resourceImageBenchmark();
    #endif //MXGUI_ENABLE_RESOURCEFS

    //Last print results
    #ifndef _BOARD_BITSBOARD
    const Color fg=white;
    const Color bg=black;
    #else //_BOARD_BITSBOARD
    const Color fg=black;
    const Color bg=white;
    #endif //_BOARD_BITSBOARD
    {
        DrawingContext dc(display);
        dc.clear(bg);
        #ifdef MXGUI_FONT_DROID11
        dc.setFont(droid11);
        #else //MXGUI_FONT_DROID11
        dc.setFont(tahoma);
        #endif //MXGUI_FONT_DROID11
        dc.setTextColor(fg,bg);
        dc.write(Point(0,0),"Benchmark name                 Time         Fps");
        dc.line(Point(0,12),Point(dc.getWidth()-1,12),fg);
        for(int i=0, j=13;i<index;i++,j+=12)
        {
             if(j+12>=dc.getHeight())
             {
                 #ifdef _BOARD_SONY_NEWMAN
                 while(POWER_BTN_PRESS_Pin::value()==0) Thread::sleep(100);
                 #else //_BOARD_SONY_NEWMAN
                 Thread::sleep(5000);
                 #endif //_BOARD_SONY_NEWMAN
                 dc.clear(bg);
                 j=13;
             }
             results[i].print(dc,Point(0,j));
        }
        Thread::sleep(500); //To avoid immediate shutdown after pressing button
    }
}

void Benchmark::fixedWidthTextBenchmark()
{
    unsigned int totalTime=0;
    char text[64];
    memset(text,0,sizeof(text));
    for(int i=0;i<min(63,display.getWidth()/8);i++) text[i]='0'+i%10;
    for(int i=0;i<4;i++)
    {
        {
            DrawingContext dc(display);
            dc.setFont(miscFixed);
            #ifndef MXGUI_COLOR_DEPTH_1_BIT_LINEAR
            dc.setTextColor(i%2==0 ? red : green,black);
            #else //MXGUI_COLOR_DEPTH_1_BIT_LINEAR
            i%2==0 ? dc.setTextColor(white,black) : dc.setTextColor(black,white);
            #endif //MXGUI_COLOR_DEPTH_1_BIT_LINEAR
            timer.start();
            for(int j=0;j<dc.getHeight();j+=16) dc.write(Point(0,j),text);
            timer.stop();
        }
        totalTime+=timer.interval()*1000000/TICK_FREQ;
        timer.clear();
        delayMs(500);
    }
    totalTime/=4;
    results[index++]=BenchmarkResult("Monospace text",totalTime);
}

void Benchmark::variableWidthTextBenchmark()
{ 
    unsigned int totalTime=0;
    char text[64];
    if(display.getWidth()==240)
    {
        //This line with tahoma font is exactly 240 pixel wide
        strcpy(text,"abcdefghijklmnopqrtstuvwxyz0123456789%$! '&/");
    } else {
        memset(text,0,sizeof(text));
        for(int i=0;i<min(63,display.getWidth()/6);i++) text[i]='0'+i%10;
    }
    for(int i=0;i<4;i++)
    {
        {
            DrawingContext dc(display);
            dc.setFont(tahoma);
            #ifndef MXGUI_COLOR_DEPTH_1_BIT_LINEAR
            dc.setTextColor(i%2==0 ? red : green,black);
            #else //MXGUI_COLOR_DEPTH_1_BIT_LINEAR
            i%2==0 ? dc.setTextColor(white,black) : dc.setTextColor(black,white);
            #endif //MXGUI_COLOR_DEPTH_1_BIT_LINEAR
            timer.start();
            for(int j=0;j<dc.getHeight();j+=12) dc.write(Point(0,j),text);
            timer.stop();
        }
        totalTime+=timer.interval()*1000000/TICK_FREQ;
        timer.clear();
        delayMs(500);
    }
    totalTime/=4;
    results[index++]=BenchmarkResult("Variable width text",totalTime);
}


#ifndef MXGUI_COLOR_DEPTH_1_BIT_LINEAR
void Benchmark::antialiasingBenchmark()
{
    unsigned int totalTime=0;
    char text[64];
    if(display.getWidth()==240)
    {
        strcpy(text,"abcdefghijklmnopqrtstuvwxyz0123456789%$! '&/");
    } else {
        memset(text,0,sizeof(text));
        for(int i=0;i<min(63,display.getWidth()/6);i++) text[i]='0'+i%10;
    }
    for(int i=0;i<4;i++)
    {
        {
            DrawingContext dc(display);
            dc.setFont(droid11);
            dc.setTextColor(i%2==0 ? red : green,black);
            timer.start();
            for(int j=0;j<dc.getHeight();j+=12) dc.write(Point(0,j),text);
            timer.stop();
        }
        totalTime+=timer.interval()*1000000/TICK_FREQ;
        timer.clear();
        delayMs(500);
    }
    totalTime/=4;
    results[index++]=BenchmarkResult("Antialiased text",totalTime);
}
#endif //MXGUI_COLOR_DEPTH_1_BIT_LINEAR

void Benchmark::horizontalLineBenchmark()
{
    unsigned int totalTime=0;
    for(int i=0;i<4;i++)
    {
        #ifndef MXGUI_COLOR_DEPTH_1_BIT_LINEAR
        Color color=i%2==0?red:green;
        #else //MXGUI_COLOR_DEPTH_1_BIT_LINEAR
        Color color=i%2==0?white:black;
        #endif //MXGUI_COLOR_DEPTH_1_BIT_LINEAR
        {
            DrawingContext dc(display);
            timer.start();
            for(int j=0;j<dc.getHeight();j++)
                dc.line(Point(0,j),Point(dc.getWidth()-1,j),color);
            timer.stop();
        }
        totalTime+=timer.interval()*1000000/TICK_FREQ;
        timer.clear();
        delayMs(500);
    }
    totalTime/=4;
    results[index++]=BenchmarkResult("Horizontal lines",totalTime);
}

void Benchmark::verticalLineBenchmark()
{
    unsigned int totalTime=0;
    for(int i=0;i<4;i++)
    {
        #ifndef MXGUI_COLOR_DEPTH_1_BIT_LINEAR
        Color color=i%2==0?red:green;
        #else //MXGUI_COLOR_DEPTH_1_BIT_LINEAR
        Color color=i%2==0?white:black;
        #endif //MXGUI_COLOR_DEPTH_1_BIT_LINEAR
        {
            DrawingContext dc(display);
            timer.start();
            for(int j=0;j<dc.getWidth();j++)
                dc.line(Point(j,0),Point(j,dc.getHeight()-1),color);
            timer.stop();
        }
        totalTime+=timer.interval()*1000000/TICK_FREQ;
        timer.clear();
        delayMs(500);
    }
    totalTime/=4;
    results[index++]=BenchmarkResult("Vertical lines",totalTime);
}

void Benchmark::obliqueLineBenchmark()
{
    unsigned int totalTime=0; 
    for(int i=0;i<4;i++)
    {
        #ifndef MXGUI_COLOR_DEPTH_1_BIT_LINEAR
        const Color darkRed(0x7800);
        const Color darkGreen(0x3e00);
        const Color darkBlue(0x000f);
        Color colorA=i%2==0?darkRed:darkGreen;
        Color colorB=i%2==0?darkGreen:darkBlue;
        Color colorC=i%2==0?darkBlue:darkRed;
        #else //MXGUI_COLOR_DEPTH_1_BIT_LINEAR
        Color colorA=i%2==0?white:black;
        Color colorB=colorA;
        Color colorC=i%2==0?black:white;
        #endif //MXGUI_COLOR_DEPTH_1_BIT_LINEAR
        {
            DrawingContext dc(display);
            timer.start();
            if(dc.getHeight()>=dc.getWidth())
            {
                for(int j=0;j<dc.getWidth();j++)
                {
                    dc.line(Point(j,0),
                        Point(dc.getWidth()-1,dc.getWidth()-1-j),colorA);
                }
                for(int j=0;j<dc.getWidth();j++)
                {
                    dc.line(Point(0,dc.getHeight()-dc.getWidth()+j),
                        Point(dc.getWidth()-1-j,dc.getHeight()-1),colorB);
                }
                for(int j=0;j<dc.getHeight()-dc.getWidth();j++)
                {
                    dc.line(Point(0,1+j),
                        Point(dc.getWidth()-1,dc.getWidth()+j),colorC);
                }
            } else {
                for(int j=0;j<dc.getHeight();j++)
                {
                    dc.line(Point(0,j),
                        Point(dc.getHeight()-1-j,dc.getHeight()-1),colorA);
                }
                for(int j=0;j<dc.getHeight();j++)
                {
                    dc.line(Point(dc.getWidth()-dc.getHeight()+j,0),
                        Point(dc.getWidth()-1,dc.getHeight()-1-j),colorB);
                }
                for(int j=0;j<dc.getWidth()-dc.getHeight();j++)
                {
                    dc.line(Point(1+j,0),
                        Point(dc.getHeight()+j,dc.getHeight()-1),colorC);
                }
            }
            timer.stop();
        }
        totalTime+=timer.interval()*1000000/TICK_FREQ;
        timer.clear();
        delayMs(500);
    }
    totalTime/=4;
    results[index++]=BenchmarkResult("Oblique lines",totalTime);
}

void Benchmark::clearScreenBenchmark()
{
    unsigned int totalTime=0;
    for(int i=0;i<4;i++)
    {
        #ifndef MXGUI_COLOR_DEPTH_1_BIT_LINEAR
        Color color=i%2==0?red:green;
        #else //MXGUI_COLOR_DEPTH_1_BIT_LINEAR
        Color color=i%2==0?white:black;
        #endif //MXGUI_COLOR_DEPTH_1_BIT_LINEAR
        {
            DrawingContext dc(display);
            timer.start();
            dc.clear(color);
            timer.stop();
        }
        totalTime+=timer.interval()*1000000/TICK_FREQ;
        timer.clear();
        delayMs(500);
    }
    totalTime/=4;
    results[index++]=BenchmarkResult("Screen clear",totalTime);
}

void Benchmark::imageBenchmark()
{
    unsigned int totalTime=0;
    for(int i=0;i<4;i++)
    {
        {
            DrawingContext dc(display);
            timer.start();
            for(int j=0;j<dc.getWidth();j+=16)
                for(int k=0;k<dc.getHeight();k+=16)
                    #ifndef MXGUI_COLOR_DEPTH_1_BIT_LINEAR
                    dc.drawImage(Point(j,k),micro_qr_code_from_wikipedia);
                    #else //MXGUI_COLOR_DEPTH_1_BIT_LINEAR
                    dc.drawImage(Point(j,k),checkpattern2);
                    #endif //MXGUI_COLOR_DEPTH_1_BIT_LINEAR
            timer.stop();
        }
        delayMs(250);
        {
            DrawingContext dc(display);
            dc.clear(black);
        }
        totalTime+=timer.interval()*1000000/TICK_FREQ;
        timer.clear();
        delayMs(250);
    }
    totalTime/=4;
    results[index++]=BenchmarkResult("Draw image",totalTime);
}

#ifndef MXGUI_COLOR_DEPTH_1_BIT_LINEAR
static const Color rainbow[]={
 63488,63520,63584,63616,63680,63744,63776,63840,
 63872,63936,64000,64032,64096,64128,64192,64256,
 64288,64352,64384,64448,64512,64544,64608,64640,
 64704,64768,64800,64864,64896,64960,65024,65056,
 65120,65152,65216,65280,65312,65376,65408,65472,
 65504,63456,63456,61408,59360,57312,55264,55264,
 53216,51168,49120,49120,47072,45024,42976,40928,
 38880,38880,36832,34784,32736,30688,30688,28640,
 26592,24544,24544,22496,20448,18400,16352,16352,
 14304,12256,10208,8160,8160,6112,4064,2016,
 2016,2017,2017,2018,2019,2020,2021,2021,
 2022,2023,2024,2025,2025,2026,2027,2028,
 2029,2029,2030,2031,2032,2033,2033,2034,
 2035,2036,2037,2037,2038,2039,2040,2041,
 2041,2042,2043,2044,2045,2045,2046,2047,
 2047,1983,1919,1887,1823,1791,1727,1663,
 1631,1567,1535,1471,1407,1375,1311,1279,
 1215,1151,1119,1055,1023,959,895,863,
 799,767,703,639,607,543,479,447,
 383,351,287,255,191,127,95,31,
 31,2079,4127,6175,6175,8223,10271,12319,
 14367,14367,16415,18463,20511,20511,22559,24607,
 26655,28703,30751,30751,32799,34847,36895,38943,
 38943,40991,43039,45087,47135,47135,49183,51231,
 53279,55327,55327,57375,59423,61471,63519,63519,
 63519,63518,63517,63516,63516,63515,63514,63513,
 63512,63512,63511,63510,63509,63508,63508,63507,
 63506,63505,63504,63504,63503,63502,63501,63500,
 63500,63499,63498,63497,63496,63496,63495,63494,
 63493,63492,63492,63491,63490,63489,63488,63488
};

void Benchmark::scanLineBenchmark()
{
    //TODO: make it work also with screen different from 240x320
    unsigned int totalTime=0;
    for(int i=0;i<4;i++)
    {
        {
            DrawingContext dc(display);
            timer.start();
            for(int k=0;k<320;k++)
                dc.scanLine(Point(0,k),rainbow,240);
            timer.stop();
            totalTime+=timer.interval()*1000000/TICK_FREQ;
            timer.clear();
            delayMs(250);
            dc.clear(black);
            timer.clear();
        }
        delayMs(250);
    }
    totalTime/=4;
    results[index++]=BenchmarkResult("ScanLine",totalTime);
}
#endif //MXGUI_COLOR_DEPTH_1_BIT_LINEAR

void Benchmark::clippedDrawBenchmark()
{
    unsigned int totalTime=0;
    for(int i=0;i<4;i++)
    {
        {
            DrawingContext dc(display);
            timer.start();
            for(int j=0;j<dc.getWidth();j+=8)
                for(int k=0;k<dc.getHeight();k+=8)
                {
                    Point p(j-8,k-8);
                    Point a(j,k);
                    Point b(j+8,k+8);
                    #ifndef MXGUI_COLOR_DEPTH_1_BIT_LINEAR
                    dc.clippedDrawImage(p,a,b,micro_qr_code_from_wikipedia);
                    #else //MXGUI_COLOR_DEPTH_1_BIT_LINEAR
                    dc.clippedDrawImage(p,a,b,checkpattern2);
                    #endif //MXGUI_COLOR_DEPTH_1_BIT_LINEAR
                }
            timer.stop();
        }
        delayMs(250);
        {
            DrawingContext dc(display);
            dc.clear(black);
        }
        totalTime+=timer.interval()*1000000/TICK_FREQ;
        timer.clear();
        delayMs(250);
    }
    totalTime/=4;
    results[index++]=BenchmarkResult("ClippedDraw",totalTime);
}

void Benchmark::clippedWriteBenchmark()
{
    unsigned int totalTime=0;
    char text[64];
    if(display.getWidth()==240)
    {
        strcpy(text,"abcdefghijklmnopqrtstuvwxyz0123456789%$! '&/");
    } else {
        memset(text,0,sizeof(text));
        for(int i=0;i<min(63,display.getWidth()/6);i++) text[i]='0'+i%10;
    }
    for(int i=0;i<4;i++)
    {
        {
            DrawingContext dc(display);
            #ifndef MXGUI_COLOR_DEPTH_1_BIT_LINEAR
            dc.setFont(droid11);
            if(i%2==0) dc.setTextColor(red,black);
            else dc.setTextColor(green,black);
            #else //MXGUI_COLOR_DEPTH_1_BIT_LINEAR
            dc.setFont(tahoma);
            if(i%2==0) dc.setTextColor(white,black);
            else dc.setTextColor(black,white);
            #endif //MXGUI_COLOR_DEPTH_1_BIT_LINEAR
            timer.start();
            for(int j=0;j<dc.getHeight();j+=6)
            {
                Point p(0,j-3);
                Point a(0,j);
                Point b(dc.getWidth()-1,j+5);
                dc.clippedWrite(p,a,b,text);
            }
            timer.stop();
        }
        totalTime+=timer.interval()*1000000/TICK_FREQ;
        timer.clear();
        delayMs(500);
    }
    totalTime/=4;
    results[index++]=BenchmarkResult("Clipped text",totalTime);
}

#ifdef MXGUI_ENABLE_RESOURCEFS
void Benchmark::resourceImageBenchmark()
{
    
    unsigned int totalTime=0;
    ResourceImage img("background");
    for(int i=0;i<4;i++)
    {
        {
            DrawingContext dc(display);
            dc.clear(black);
            timer.start();
            dc.drawImage(Point(0,0),img);
            timer.stop();
        }
        totalTime+=timer.interval()*1000000/TICK_FREQ;
        timer.clear();
        delayMs(500);
    }
    totalTime/=4;
    results[index++]=BenchmarkResult("ResourceImage",totalTime);    
}
#endif //MXGUI_ENABLE_RESOURCEFS
