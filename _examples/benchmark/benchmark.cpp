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
#include "checkpattern2.h"
#include "micro_qr_code_from_wikipedia.h"
#include "benchmark.h"
#include <cstdio>
#include <cstring>
#include <chrono>

using namespace std;
using namespace std::chrono;
using namespace mxgui;
using namespace miosix;

//
// Class BenchmarkResult
//

BenchmarkResult::BenchmarkResult(const char name[20], unsigned int time)
        : time(time)
{
    strncpy(this->name,name,19);
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
    #ifndef MXGUI_PIXEL_FORMAT_GRAY1
    antialiasingBenchmark();
    #endif //MXGUI_PIXEL_FORMAT_GRAY1
    horizontalLineBenchmark();
    verticalLineBenchmark();
    obliqueLineBenchmark();
    clearScreenBenchmark();
    imageBenchmark();
    #ifndef MXGUI_PIXEL_FORMAT_GRAY1
    scanLineBenchmark();
    #endif //MXGUI_PIXEL_FORMAT_GRAY1
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
            #ifndef MXGUI_PIXEL_FORMAT_GRAY1
            dc.setTextColor(i%2==0 ? red : green,black);
            #else //MXGUI_PIXEL_FORMAT_GRAY1
            i%2==0 ? dc.setTextColor(white,black) : dc.setTextColor(black,white);
            #endif //MXGUI_PIXEL_FORMAT_GRAY1
        }
        auto t=system_clock::now();
        {
            DrawingContext dc(display);
            for(int j=0;j<dc.getHeight();j+=16) dc.write(Point(0,j),text);
        }
        auto d=system_clock::now()-t;
        totalTime+=duration_cast<microseconds>(d).count();
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
            #ifndef MXGUI_PIXEL_FORMAT_GRAY1
            dc.setTextColor(i%2==0 ? red : green,black);
            #else //MXGUI_PIXEL_FORMAT_GRAY1
            i%2==0 ? dc.setTextColor(white,black) : dc.setTextColor(black,white);
            #endif //MXGUI_PIXEL_FORMAT_GRAY1
        }
        auto t=system_clock::now();
        {
            DrawingContext dc(display);
            for(int j=0;j<dc.getHeight();j+=12) dc.write(Point(0,j),text);
        }
        auto d=system_clock::now()-t;
        totalTime+=duration_cast<microseconds>(d).count();
        delayMs(500);
    }
    totalTime/=4;
    results[index++]=BenchmarkResult("Variable width text",totalTime);
}


#ifndef MXGUI_PIXEL_FORMAT_GRAY1
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
        }
        auto t=system_clock::now();
        {
            DrawingContext dc(display);
            for(int j=0;j<dc.getHeight();j+=12) dc.write(Point(0,j),text);
        }
        auto d=system_clock::now()-t;
        totalTime+=duration_cast<microseconds>(d).count();
        delayMs(500);
    }
    totalTime/=4;
    results[index++]=BenchmarkResult("Antialiased text",totalTime);
}
#endif //MXGUI_PIXEL_FORMAT_GRAY1

void Benchmark::horizontalLineBenchmark()
{
    unsigned int totalTime=0;
    for(int i=0;i<4;i++)
    {
        #ifndef MXGUI_PIXEL_FORMAT_GRAY1
        Color color=i%2==0?red:green;
        #else //MXGUI_PIXEL_FORMAT_GRAY1
        Color color=i%2==0?white:black;
        #endif //MXGUI_PIXEL_FORMAT_GRAY1
        auto t=system_clock::now();
        {
            DrawingContext dc(display);
            for(int j=0;j<dc.getHeight();j++)
                dc.line(Point(0,j),Point(dc.getWidth()-1,j),color);
        }
        auto d=system_clock::now()-t;
        totalTime+=duration_cast<microseconds>(d).count();
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
        #ifndef MXGUI_PIXEL_FORMAT_GRAY1
        Color color=i%2==0?red:green;
        #else //MXGUI_PIXEL_FORMAT_GRAY1
        Color color=i%2==0?white:black;
        #endif //MXGUI_PIXEL_FORMAT_GRAY1
        auto t=system_clock::now();
        {
            DrawingContext dc(display);
            for(int j=0;j<dc.getWidth();j++)
                dc.line(Point(j,0),Point(j,dc.getHeight()-1),color);
        }
        auto d=system_clock::now()-t;
        totalTime+=duration_cast<microseconds>(d).count();
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
        #ifndef MXGUI_PIXEL_FORMAT_GRAY1
        const Color darkRed = Color::fromRGB565(0x7800);
        const Color darkGreen = Color::fromRGB565(0x3e00);
        const Color darkBlue = Color::fromRGB565(0x000f);
        Color colorA=i%2==0?darkRed:darkGreen;
        Color colorB=i%2==0?darkGreen:darkBlue;
        Color colorC=i%2==0?darkBlue:darkRed;
        #else //MXGUI_PIXEL_FORMAT_GRAY1
        Color colorA=i%2==0?white:black;
        Color colorB=colorA;
        Color colorC=i%2==0?black:white;
        #endif //MXGUI_PIXEL_FORMAT_GRAY1
        auto t=system_clock::now();
        {
            DrawingContext dc(display);
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
        }
        auto d=system_clock::now()-t;
        totalTime+=duration_cast<microseconds>(d).count();
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
        #ifndef MXGUI_PIXEL_FORMAT_GRAY1
        Color color=i%2==0?red:green;
        #else //MXGUI_PIXEL_FORMAT_GRAY1
        Color color=i%2==0?white:black;
        #endif //MXGUI_PIXEL_FORMAT_GRAY1
        auto t=system_clock::now();
        {
            DrawingContext dc(display);
            dc.clear(color);
        }
        auto d=system_clock::now()-t;
        totalTime+=duration_cast<microseconds>(d).count();
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
        auto t=system_clock::now();
        {
            DrawingContext dc(display);
            for(int j=0;j<dc.getWidth();j+=16)
                for(int k=0;k<dc.getHeight();k+=16)
                    #ifndef MXGUI_PIXEL_FORMAT_GRAY1
                    dc.drawImage(Point(j,k),micro_qr_code_from_wikipedia);
                    #else //MXGUI_PIXEL_FORMAT_GRAY1
                    dc.drawImage(Point(j,k),checkpattern2);
                    #endif //MXGUI_PIXEL_FORMAT_GRAY1
        }
        auto d=system_clock::now()-t;
        delayMs(250);
        {
            DrawingContext dc(display);
            dc.clear(black);
        }
        totalTime+=duration_cast<microseconds>(d).count();
        delayMs(250);
    }
    totalTime/=4;
    results[index++]=BenchmarkResult("Draw image",totalTime);
}

#ifndef MXGUI_PIXEL_FORMAT_GRAY1
static const Color rainbow[]={
 Color::fromRGB565(63488),
 Color::fromRGB565(63520),
 Color::fromRGB565(63584),
 Color::fromRGB565(63616),
 Color::fromRGB565(63680),
 Color::fromRGB565(63744),
 Color::fromRGB565(63776),
 Color::fromRGB565(63840),
 Color::fromRGB565(63872),
 Color::fromRGB565(63936),
 Color::fromRGB565(64000),
 Color::fromRGB565(64032),
 Color::fromRGB565(64096),
 Color::fromRGB565(64128),
 Color::fromRGB565(64192),
 Color::fromRGB565(64256),
 Color::fromRGB565(64288),
 Color::fromRGB565(64352),
 Color::fromRGB565(64384),
 Color::fromRGB565(64448),
 Color::fromRGB565(64512),
 Color::fromRGB565(64544),
 Color::fromRGB565(64608),
 Color::fromRGB565(64640),
 Color::fromRGB565(64704),
 Color::fromRGB565(64768),
 Color::fromRGB565(64800),
 Color::fromRGB565(64864),
 Color::fromRGB565(64896),
 Color::fromRGB565(64960),
 Color::fromRGB565(65024),
 Color::fromRGB565(65056),
 Color::fromRGB565(65120),
 Color::fromRGB565(65152),
 Color::fromRGB565(65216),
 Color::fromRGB565(65280),
 Color::fromRGB565(65312),
 Color::fromRGB565(65376),
 Color::fromRGB565(65408),
 Color::fromRGB565(65472),
 Color::fromRGB565(65504),
 Color::fromRGB565(63456),
 Color::fromRGB565(63456),
 Color::fromRGB565(61408),
 Color::fromRGB565(59360),
 Color::fromRGB565(57312),
 Color::fromRGB565(55264),
 Color::fromRGB565(55264),
 Color::fromRGB565(53216),
 Color::fromRGB565(51168),
 Color::fromRGB565(49120),
 Color::fromRGB565(49120),
 Color::fromRGB565(47072),
 Color::fromRGB565(45024),
 Color::fromRGB565(42976),
 Color::fromRGB565(40928),
 Color::fromRGB565(38880),
 Color::fromRGB565(38880),
 Color::fromRGB565(36832),
 Color::fromRGB565(34784),
 Color::fromRGB565(32736),
 Color::fromRGB565(30688),
 Color::fromRGB565(30688),
 Color::fromRGB565(28640),
 Color::fromRGB565(26592),
 Color::fromRGB565(24544),
 Color::fromRGB565(24544),
 Color::fromRGB565(22496),
 Color::fromRGB565(20448),
 Color::fromRGB565(18400),
 Color::fromRGB565(16352),
 Color::fromRGB565(16352),
 Color::fromRGB565(14304),
 Color::fromRGB565(12256),
 Color::fromRGB565(10208),
 Color::fromRGB565(8160),
 Color::fromRGB565(8160),
 Color::fromRGB565(6112),
 Color::fromRGB565(4064),
 Color::fromRGB565(2016),
 Color::fromRGB565(2016),
 Color::fromRGB565(2017),
 Color::fromRGB565(2017),
 Color::fromRGB565(2018),
 Color::fromRGB565(2019),
 Color::fromRGB565(2020),
 Color::fromRGB565(2021),
 Color::fromRGB565(2021),
 Color::fromRGB565(2022),
 Color::fromRGB565(2023),
 Color::fromRGB565(2024),
 Color::fromRGB565(2025),
 Color::fromRGB565(2025),
 Color::fromRGB565(2026),
 Color::fromRGB565(2027),
 Color::fromRGB565(2028),
 Color::fromRGB565(2029),
 Color::fromRGB565(2029),
 Color::fromRGB565(2030),
 Color::fromRGB565(2031),
 Color::fromRGB565(2032),
 Color::fromRGB565(2033),
 Color::fromRGB565(2033),
 Color::fromRGB565(2034),
 Color::fromRGB565(2035),
 Color::fromRGB565(2036),
 Color::fromRGB565(2037),
 Color::fromRGB565(2037),
 Color::fromRGB565(2038),
 Color::fromRGB565(2039),
 Color::fromRGB565(2040),
 Color::fromRGB565(2041),
 Color::fromRGB565(2041),
 Color::fromRGB565(2042),
 Color::fromRGB565(2043),
 Color::fromRGB565(2044),
 Color::fromRGB565(2045),
 Color::fromRGB565(2045),
 Color::fromRGB565(2046),
 Color::fromRGB565(2047),
 Color::fromRGB565(2047),
 Color::fromRGB565(1983),
 Color::fromRGB565(1919),
 Color::fromRGB565(1887),
 Color::fromRGB565(1823),
 Color::fromRGB565(1791),
 Color::fromRGB565(1727),
 Color::fromRGB565(1663),
 Color::fromRGB565(1631),
 Color::fromRGB565(1567),
 Color::fromRGB565(1535),
 Color::fromRGB565(1471),
 Color::fromRGB565(1407),
 Color::fromRGB565(1375),
 Color::fromRGB565(1311),
 Color::fromRGB565(1279),
 Color::fromRGB565(1215),
 Color::fromRGB565(1151),
 Color::fromRGB565(1119),
 Color::fromRGB565(1055),
 Color::fromRGB565(1023),
 Color::fromRGB565(959),
 Color::fromRGB565(895),
 Color::fromRGB565(863),
 Color::fromRGB565(799),
 Color::fromRGB565(767),
 Color::fromRGB565(703),
 Color::fromRGB565(639),
 Color::fromRGB565(607),
 Color::fromRGB565(543),
 Color::fromRGB565(479),
 Color::fromRGB565(447),
 Color::fromRGB565(383),
 Color::fromRGB565(351),
 Color::fromRGB565(287),
 Color::fromRGB565(255),
 Color::fromRGB565(191),
 Color::fromRGB565(127),
 Color::fromRGB565(95),
 Color::fromRGB565(31),
 Color::fromRGB565(31),
 Color::fromRGB565(2079),
 Color::fromRGB565(4127),
 Color::fromRGB565(6175),
 Color::fromRGB565(6175),
 Color::fromRGB565(8223),
 Color::fromRGB565(10271),
 Color::fromRGB565(12319),
 Color::fromRGB565(14367),
 Color::fromRGB565(14367),
 Color::fromRGB565(16415),
 Color::fromRGB565(18463),
 Color::fromRGB565(20511),
 Color::fromRGB565(20511),
 Color::fromRGB565(22559),
 Color::fromRGB565(24607),
 Color::fromRGB565(26655),
 Color::fromRGB565(28703),
 Color::fromRGB565(30751),
 Color::fromRGB565(30751),
 Color::fromRGB565(32799),
 Color::fromRGB565(34847),
 Color::fromRGB565(36895),
 Color::fromRGB565(38943),
 Color::fromRGB565(38943),
 Color::fromRGB565(40991),
 Color::fromRGB565(43039),
 Color::fromRGB565(45087),
 Color::fromRGB565(47135),
 Color::fromRGB565(47135),
 Color::fromRGB565(49183),
 Color::fromRGB565(51231),
 Color::fromRGB565(53279),
 Color::fromRGB565(55327),
 Color::fromRGB565(55327),
 Color::fromRGB565(57375),
 Color::fromRGB565(59423),
 Color::fromRGB565(61471),
 Color::fromRGB565(63519),
 Color::fromRGB565(63519),
 Color::fromRGB565(63519),
 Color::fromRGB565(63518),
 Color::fromRGB565(63517),
 Color::fromRGB565(63516),
 Color::fromRGB565(63516),
 Color::fromRGB565(63515),
 Color::fromRGB565(63514),
 Color::fromRGB565(63513),
 Color::fromRGB565(63512),
 Color::fromRGB565(63512),
 Color::fromRGB565(63511),
 Color::fromRGB565(63510),
 Color::fromRGB565(63509),
 Color::fromRGB565(63508),
 Color::fromRGB565(63508),
 Color::fromRGB565(63507),
 Color::fromRGB565(63506),
 Color::fromRGB565(63505),
 Color::fromRGB565(63504),
 Color::fromRGB565(63504),
 Color::fromRGB565(63503),
 Color::fromRGB565(63502),
 Color::fromRGB565(63501),
 Color::fromRGB565(63500),
 Color::fromRGB565(63500),
 Color::fromRGB565(63499),
 Color::fromRGB565(63498),
 Color::fromRGB565(63497),
 Color::fromRGB565(63496),
 Color::fromRGB565(63496),
 Color::fromRGB565(63495),
 Color::fromRGB565(63494),
 Color::fromRGB565(63493),
 Color::fromRGB565(63492),
 Color::fromRGB565(63492),
 Color::fromRGB565(63491),
 Color::fromRGB565(63490),
 Color::fromRGB565(63489),
 Color::fromRGB565(63488),
 Color::fromRGB565(63488)
};

void Benchmark::scanLineBenchmark()
{
    unsigned int totalTime=0;
    for(int i=0;i<4;i++)
    {
        auto t=system_clock::now();
        {
            DrawingContext dc(display);
            //TODO: does not work well for displays with width > 240
            for(int k=0;k<dc.getHeight();k++)
                dc.scanLine(Point(0,k),rainbow,min<int>(240,dc.getWidth()));
        }
        auto d=system_clock::now()-t;
        totalTime+=duration_cast<microseconds>(d).count();
        delayMs(250);
        {
            DrawingContext dc(display);
            dc.clear(black);
        }
        delayMs(250);
    }
    totalTime/=4;
    results[index++]=BenchmarkResult("ScanLine",totalTime);
}
#endif //MXGUI_PIXEL_FORMAT_GRAY1

void Benchmark::clippedDrawBenchmark()
{
    unsigned int totalTime=0;
    for(int i=0;i<4;i++)
    {
        auto t=system_clock::now();
        {
            DrawingContext dc(display);
            for(int j=0;j<dc.getWidth();j+=8)
                for(int k=0;k<dc.getHeight();k+=8)
                {
                    Point p(j-8,k-8);
                    Point a(j,k);
                    Point b(j+8,k+8);
                    #ifndef MXGUI_PIXEL_FORMAT_GRAY1
                    dc.clippedDrawImage(p,a,b,micro_qr_code_from_wikipedia);
                    #else //MXGUI_PIXEL_FORMAT_GRAY1
                    dc.clippedDrawImage(p,a,b,checkpattern2);
                    #endif //MXGUI_PIXEL_FORMAT_GRAY1
                }
        }
        auto d=system_clock::now()-t;
        delayMs(250);
        {
            DrawingContext dc(display);
            dc.clear(black);
        }
        totalTime+=duration_cast<microseconds>(d).count();
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
            #ifndef MXGUI_PIXEL_FORMAT_GRAY1
            dc.setFont(droid11);
            if(i%2==0) dc.setTextColor(red,black);
            else dc.setTextColor(green,black);
            #else //MXGUI_PIXEL_FORMAT_GRAY1
            dc.setFont(tahoma);
            if(i%2==0) dc.setTextColor(white,black);
            else dc.setTextColor(black,white);
            #endif //MXGUI_PIXEL_FORMAT_GRAY1
        }
        auto t=system_clock::now();
        {
            DrawingContext dc(display);
            for(int j=0;j<dc.getHeight();j+=6)
            {
                Point p(0,j-3);
                Point a(0,j);
                Point b(dc.getWidth()-1,j+5);
                dc.clippedWrite(p,a,b,text);
            }
        }
        auto d=system_clock::now()-t;
        totalTime+=duration_cast<microseconds>(d).count();
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
        }
        auto t=system_clock::now();
        {
            DrawingContext dc(display);
            dc.drawImage(Point(0,0),img);
        }
        auto d=system_clock::now()-t;
        totalTime+=duration_cast<microseconds>(d).count();
        delayMs(500);
    }
    totalTime/=4;
    results[index++]=BenchmarkResult("ResourceImage",totalTime);    
}
#endif //MXGUI_ENABLE_RESOURCEFS
