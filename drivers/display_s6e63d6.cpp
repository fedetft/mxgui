/***************************************************************************
 *   Copyright (C) 2010 by Terraneo Federico                               *
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

#include "display_s6e63d6.h"

#ifdef MXGUI_DISPLAY_TYPE_S6E63D6

#include "mxgui/misc_inst.h"
#include "miosix.h"
#include <cstdio>
#include <cstring>
#include <algorithm>

using namespace std;
using namespace miosix;

namespace mxgui {

//
// Debug mode printf
//

//#define DBG iprintf
#define DBG (void)

//
// Class DisplayS6E63D6
//

DisplayS6E63D6::DisplayS6E63D6(): textColor(), font(droid11)
{
    hardwareInit();
    //
    //Power up sequence -- begin
    //
    delayMs(500);//Wait at least 500ms from +3.3 stabilized
    disp::reset::low();
    delayMs(1);
    disp::reset::high();
    delayMs(10);//Wait 10ms before waking up the display
    #ifdef BACKEND_STM32FSMC
    writeIdx(0x23); //These additional instrutions set up 16bit parallel iface
    writeReg(0x02,0x0000);
    #endif //BACKEND_STM32FSMC
    writeReg(0x10,0x0000);//STB = 0 (out of standby)
    delayMs(100);//Let internal voltages stabilize
    //misc settings
    writeReg(0x03,0x0130);
    writeReg(0x18,0x0028);
    writeReg(0xf8,0x000f);//register f8 is undocumented in the datasheet??
    writeReg(0xf9,0x000f);//register f9 is undocumented in the datasheet??
    //Gamma settings
    writeReg(0x70,0x2580);
    writeReg(0x71,0x2780);
    writeReg(0x72,0x3380);
    writeReg(0x73,0x1d18);
    writeReg(0x74,0x1f11);
    writeReg(0x75,0x2419);
    writeReg(0x76,0x1a14);
    writeReg(0x77,0x211a);
    writeReg(0x78,0x2013);

    clear(Color(0x0000));//Clear screen before activating display
    disp::ncpEn::high();
    delayMs(32);//Let AR_VDD and AR_VSS stabilize
    writeReg(0x05,0x0001);//DISP_ON = 1 (display active)
    //
    //Power up sequence -- end
    //
    
    setTextColor(Color(0xffff),Color(0x0000));
    clear(black);
}

void DisplayS6E63D6::write(Point p, const char *text)
{
    font.draw(*this,textColor,p,text);
}

void DisplayS6E63D6::clear(Color color)
{
    clear(Point(0,0),Point(width-1,height-1),color);
}

void DisplayS6E63D6::clear(Point p1, Point p2, Color color)
{
    imageWindow(p1,p2);
    writeIdx(0x22);//Write to GRAM
    int numPixels=(p2.x()-p1.x()+1)*(p2.y()-p1.y()+1)/2;
    volatile unsigned int *dirtyTrick;
    dirtyTrick=reinterpret_cast<volatile unsigned int*>(0x60020000);
    unsigned int twoPixColor=color.value() | color.value()<<16;
    for(int i=0;i<numPixels;i++)
    {
        *dirtyTrick=twoPixColor;
    }
//    No speed advantage
//    int numPixels=(p2.x()-p1.x()+1)*(p2.y()-p1.y()+1)/8;
//    asm volatile("           cmp %1, #0              \n"
//                 "           beq __clear_2           \n"
//                 "           mov r0, #0              \n"
//                 "           mov r7, %0              \n"
//                 "__clear_1: stmia %2, {%0,r7}       \n"
//                 "           add r0, r0, #1          \n"
//                 "           cmp r0, %1              \n"
//                 "           stmia %2, {%0,r7}       \n"
//                 "           bne __clear_1           \n"
//                 "__clear_2:                         \n"
//                 ::"r"(twoPixColor),"r"(numPixels),"r"(dirtyTrick)
//                 :"r0","r7");
    textWindow(Point(0,0),Point(width-1,height-1));//Restore default window
}

void DisplayS6E63D6::setPixel(Point p, Color color)
{
    setCursor(p);
    writeIdx(0x22);//Write to GRAM
    writeRam(color.value());
}

void DisplayS6E63D6::line(Point a, Point b, Color color)
{
    //Horizontal line speed optimization
    //The height-8 and width-8 condition is because from the spfd5408 datasheet
    //a window has minimum size constraints
    if(a.y()==b.y() && a.y()<height-8 && min(a.x(),b.x())<width-8)
    {
        imageWindow(Point(min(a.x(),b.x()),a.y()),Point(width-1,a.y()+8));
        writeIdx(0x22);//Write to GRAM
        int numPixels=abs(a.x()-b.x());
        //Loop not unrolled because when running from flash is slower
        for(int i=0;i<=numPixels;i++) writeRam(color.value());
        textWindow(Point(0,0),Point(width-1,height-1));//Restore default window
        return;
    }
    //Vertical line speed optimization
    //The height-8 and width-8 condition is because from the spfd5408 datasheet
    //a window has minimum size constraints
    if(a.x()==b.x() && min(a.y(),b.y())<height-8 && a.x()<width-8)
    {
        textWindow(Point(a.x(),min(a.y(),b.y())),Point(a.x()+8,height-1));
        writeIdx(0x22);//Write to GRAM
        int numPixels=abs(a.y()-b.y());
        //Loop not unrolled because when running from flash is slower
        for(int i=0;i<=numPixels;i++) writeRam(color.value());
        textWindow(Point(0,0),Point(width-1,height-1));//Restore default window
        return;
    }
    //General case, always works but it is a bit slower
    const short int dx=b.x()-a.x();
    const short int dy=b.y()-a.y();
    if(dx==0 && dy==0)
    {
        setPixel(a,color);
        return;
    }
    if(abs(dx)>=abs(dy))
    {
        int m=(dy*width)/dx;
        short int x;
        if(dx>0)
        {
            for(x=a.x();x<b.x();x++)
                setPixel(Point(x,a.y()+((m*(x-a.x()))/width)),color.value());
            setPixel(b,color.value());
        } else {
            for(x=b.x();x<a.x();x++)
                setPixel(Point(x,b.y()+((m*(x-b.x()))/width)),color.value());
            setPixel(a,color.value());
        }
    } else {
        int m=(dx*height)/dy;
        short int y;
        if(dy>0)
        {
            for(y=a.y();y<b.y();y++)
                setPixel(Point(a.x()+((m*(y-a.y()))/height),y),color.value());
            setPixel(b,color.value());
        } else {
            for(y=b.y();y<a.y();y++)
                setPixel(Point(b.x()+((m*(y-b.y()))/height),y),color.value());
            setPixel(a,color.value());
        }
    }
}

void DisplayS6E63D6::drawImage(Point p, Image img)
{
    short int xEnd=p.x()+img.getWidth()-1;
    short int yEnd=p.y()+img.getHeight()-1;

    if(xEnd > width || yEnd > height) return;
    if(img.imageDepth()!=ImageDepth::DEPTH_16_BIT) return;

    imageWindow(p,Point(xEnd,yEnd));
    writeIdx(0x22);//Write to GRAM
    const unsigned char *imgData=img.getData();

    int numPixels=img.getHeight()*img.getWidth();
    //Loop unrolled 8 times for speed
    int remaining=numPixels % 8;
    numPixels/=8;
    for(int i=0;i<=remaining;i++)
    {
        writeRam(((*imgData)<<8) | *(imgData+1));
        imgData+=2;
    }
    for(int i=0;i<=numPixels;i++)
    {
        writeRam(((*imgData)<<8) | *(imgData+1));
        writeRam(((*(imgData+2))<<8) | *(imgData+3));
        writeRam(((*(imgData+4))<<8) | *(imgData+5));
        writeRam(((*(imgData+6))<<8) | *(imgData+7));
        writeRam(((*(imgData+8))<<8) | *(imgData+9));
        writeRam(((*(imgData+10))<<8) | *(imgData+11));
        writeRam(((*(imgData+12))<<8) | *(imgData+13));
        writeRam(((*(imgData+14))<<8) | *(imgData+15));
        imgData+=16;
    }
}

void DisplayS6E63D6::drawRectangle(Point a, Point b, Color c)
{
    line(a,Point(b.x(),a.y()),c);
    line(Point(b.x(),a.y()),b,c);
    line(b,Point(a.x(),b.y()),c);
    line(Point(a.x(),b.y()),a,c);
}

void DisplayS6E63D6::turnOn()
{
    writeReg(0x10,0x0000);//STB = 0 (out of standby)
    delayMs(100);//Let internal voltages stabilize
    disp::ncpEn::high();
    delayMs(32);//Let AR_VDD and AR_VSS stabilize
    writeReg(0x05,0x0001);//DISP_ON = 1 (display active)
}

void DisplayS6E63D6::turnOff()
{
    writeReg(0x05,0x0000);//DISP_ON = 0 (display blank)
    delayMs(32);
    disp::ncpEn::low();
    delayMs(32);
    writeReg(0x10,0x0001);//STB = 1 (standby mode)
    delayMs(500);
}

void DisplayS6E63D6::setTextColor(Color fgcolor, Color bgcolor)
{
    unsigned short fgR=fgcolor.value(); //& 0xf800; Optimization, & not required
    unsigned short bgR=bgcolor.value(); //& 0xf800; Optimization, & not required
    unsigned short fgG=fgcolor.value() & 0x7e0;
    unsigned short bgG=bgcolor.value() & 0x7e0;
    unsigned short fgB=fgcolor.value() & 0x1f;
    unsigned short bgB=bgcolor.value() & 0x1f;
    unsigned short deltaR=((fgR-bgR)/3) & 0xf800;
    unsigned short deltaG=((fgG-bgG)/3) & 0x7e0;
    unsigned short deltaB=((fgB-bgB)/3) & 0x1f;
    unsigned short delta=deltaR | deltaG | deltaB;
    textColor[3]=fgcolor;
    textColor[2]=Color(bgcolor.value()+2*delta);
    textColor[1]=Color(bgcolor.value()+delta);
    textColor[0]=bgcolor;
    //cout<<hex<<"<"<<textColor[0].value()<<","<<textColor[1].value()<<","<<
    //        textColor[2].value()<<","<<textColor[3].value()<<">"<<endl;
}

void DisplayS6E63D6::setFont(const Font& font)
{
    this->font=font;
}

DisplayS6E63D6::pixel_iterator DisplayS6E63D6::begin(Point p1, Point p2,
        IteratorDirection d)
{
    if(p1.x()<0 || p1.y()<0 || p2.x()<0 || p2.y()<0) return pixel_iterator();
    if(p1.x()>=width || p1.y()>=height || p2.x()>=width || p2.y()>=height)
        return pixel_iterator();
    if(p2.x()<p1.x() || p2.y()<p1.y()) return pixel_iterator();

    if(d==DR) textWindow(p1,p2);
    else imageWindow(p1,p2);
    writeIdx(0x22);//Write to GRAM

    unsigned int numPixels=(p2.x()-p1.x()+1)*(p2.y()-p1.y()+1);
    return pixel_iterator(numPixels);
}

} //namespace mxgui

#endif //MXGUI_DISPLAY_TYPE_S6E63D6
