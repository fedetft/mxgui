/***************************************************************************
 *   Copyright (C) 2011 by Yury Kuchura
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

#include "display_redbull_v2.h"
#include "miosix.h"

using namespace std;
using namespace miosix;

#ifdef _BOARD_REDBULL_V2

//Helper to set gamma register values
#define GREGVAL(byte1, byte2) (uint16_t)(((byte1) << 8) | ((byte2) & 0xFF))
//Helper for bit mask
#define b(n) (1<<(n))

namespace mxgui {

void registerDisplayHook(DisplayManager& dm)
{
    dm.registerDisplay(&DisplayImpl::instance());
}

//
// Class DisplayImpl
//

DisplayImpl& DisplayImpl::instance()
{
    static DisplayImpl instance;
    return instance;
}

void DisplayImpl::doTurnOn()
{
    disp::backlight::high(); //Backlight on
}

void DisplayImpl::doTurnOff()
{
    disp::backlight::low(); //Backlight off
}

void DisplayImpl::doSetBrightness(int brt) {}

pair<short int, short int> DisplayImpl::doGetSize() const
{
    return make_pair(height,width);
}

void DisplayImpl::write(Point p, const char *text)
{
    font.draw(*this,textColor,p,text);
}

void DisplayImpl::clippedWrite(Point p, Point a, Point b, const char *text)
{
    font.clippedDraw(*this,textColor,p,a,b,text);
}

void DisplayImpl::clear(Color color)
{
    clear(Point(0,0),Point(width-1,height-1),color);
}

void DisplayImpl::clear(Point p1, Point p2, Color color)
{
    imageWindow(p1,p2);
    writeIdx(0x22);//Write to GRAM
    int numPixels=(p2.x()-p1.x()+1)*(p2.y()-p1.y()+1);
    for(int i=0;i<numPixels;i++) writeRam(color);
}

void DisplayImpl::beginPixel()
{
    textWindow(Point(0,0),Point(width-1,height-1));//Restore default window
}

void DisplayImpl::setPixel(Point p, Color color)
{
    setCursor(p);
    writeIdx(0x22);//Write to GRAM
    writeRam(color);
}

void DisplayImpl::line(Point a, Point b, Color color)
{
    //Horizontal line speed optimization
    if(a.y()==b.y())
    {
        imageWindow(Point(min(a.x(),b.x()),a.y()),
                    Point(max(a.x(),b.x()),a.y()));
        writeIdx(0x22);//Write to GRAM
        int numPixels=abs(a.x()-b.x());
        for(int i=0;i<=numPixels;i++) DISPLAY->RAM=color;
        if(numPixels & 0x1) writeRam(color);
        return;
    }
    //Vertical line speed optimization
    if(a.x()==b.x())
    {
        textWindow(Point(a.x(),min(a.y(),b.y())),
                   Point(a.x(),max(a.y(),b.y())));
        writeIdx(0x22);//Write to GRAM
        int numPixels=abs(a.y()-b.y());
        for(int i=0;i<=numPixels;i++) DISPLAY->RAM=color;
        if(numPixels & 0x1) writeRam(color);
        return;
    }
    //General case, always works but it is much slower due to the display
    //not having fast random access to pixels
    Line::draw(*this,a,b,color);
}

void DisplayImpl::scanLine(Point p, const Color *colors, unsigned short length)
{
    imageWindow(p,Point(width-1,p.y()));
    writeIdx(0x22); //Write to GRAM
    for(int i=0;i<length;i++) writeRam(colors[i]);
}

Color *DisplayImpl::getScanLineBuffer()
{
    if(buffer==0) buffer=new Color[getWidth()];
    return buffer;
}

void DisplayImpl::scanLineBuffer(Point p, unsigned short length)
{
    scanLine(p,buffer,length);
}

void DisplayImpl::drawImage(Point p, const ImageBase& img)
{
    short int xEnd=p.x()+img.getWidth()-1;
    short int yEnd=p.y()+img.getHeight()-1;
    if(xEnd >= width || yEnd >= height) return;

    const unsigned short *imgData=img.getData();
    if(imgData!=0)
    {
        //Optimized version for memory-loaded images
        imageWindow(p,Point(xEnd,yEnd));
        writeIdx(0x22);//Write to GRAM
        int numPixels=img.getHeight()*img.getWidth();
        for(int i=0;i<=numPixels;i++)
        {
            writeRam(imgData[0]);
            imgData++;
        }
    } else img.draw(*this,p);
}

void DisplayImpl::clippedDrawImage(Point p, Point a, Point b, const ImageBase& img)
{
    img.clippedDraw(*this,p,a,b);
}

void DisplayImpl::drawRectangle(Point a, Point b, Color c)
{
    line(a,Point(b.x(),a.y()),c);
    line(Point(b.x(),a.y()),b,c);
    line(b,Point(a.x(),b.y()),c);
    line(Point(a.x(),b.y()),a,c);
}

void DisplayImpl::setTextColor(pair<Color,Color> colors)
{
    Font::generatePalette(textColor,colors.first,colors.second);
}

pair<Color,Color> DisplayImpl::getTextColor() const
{
    return make_pair(textColor[3],textColor[0]);
}

void DisplayImpl::setFont(const Font& font) { this->font=font; }

Font DisplayImpl::getFont() const { return font; }

void DisplayImpl::update() {}

DisplayImpl::pixel_iterator DisplayImpl::begin(Point p1, Point p2,
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

DisplayImpl::~DisplayImpl()
{
    if(buffer) delete[] buffer;
}

DisplayImpl::DisplayImpl(): buffer(0), textColor(), font(droid11)
{
    
    //LCD connection GPIO should have been initialized at
    //this point. bsp.cpp is responsible for this.
    RCC->AHBENR |= RCC_AHBENR_FSMCEN;

    //The way BCR and BTR are specified in stm32f10x.h sucks, trying to work
    //around it...
    volatile uint32_t& BCR4=FSMC_Bank1->BTCR[6];
    volatile uint32_t& BTR4=FSMC_Bank1->BTCR[7];
    volatile uint32_t& BWTR4=FSMC_Bank1E->BWTR[6];

    //Timings for SSD1289
    //Write burst disabled, Extended mode enabled, Wait signal disabled
    //Write enabled, Wait signal active before wait state, Wrap disabled
    //Burst disabled, Data width 16bit, Memory type SRAM, Data mux disabled
    BCR4 = FSMC_BCR4_WREN | FSMC_BCR4_MWID_0 | FSMC_BCR4_MBKEN | FSMC_BCR4_EXTMOD;
    // Write timings
    //Address setup=0, Data setup=4, Access mode=A
    BWTR4 = FSMC_BTR4_DATAST_2;
    // Read timings
    //Address setup=13, Data setup=10, Access mode=A
    BTR4 = FSMC_BTR4_DATAST_3 | FSMC_BTR4_DATAST_1 | FSMC_BTR4_ADDSET_3 |
           FSMC_BTR4_ADDSET_2 | FSMC_BTR4_ADDSET_0;

    //Power up
    delayMs(10);
    unsigned short id=readReg(0);
    //SSD1289: 0x8989
    if (0x8989 != id)
    {
        return;
    }

    //Turn on backlight
    disp::backlight::high();

    writeReg(0x0000,0x0001);
    writeReg(0x0003,0xA8A4);
    writeReg(0x000C,0x0000);
    writeReg(0x000D,0x000C);
    writeReg(0x000E,0x2B00);
    writeReg(0x001E,0x00B0);
    writeReg(0x0001,0x2B3F);
    writeReg(0x0002,0x0600);
    writeReg(0x0010,0x0000);
    writeReg(0x0011,0x6078);
    writeReg(0x0005,0x0000);
    writeReg(0x0006,0x0000);
    writeReg(0x0016,0xEF1C);
    writeReg(0x0017,0x0003);
    writeReg(0x0007,0x0133);
    writeReg(0x000B,0x0000);
    writeReg(0x000F,0x0000);
    writeReg(0x0041,0x0000);
    writeReg(0x0042,0x0000);
    writeReg(0x0048,0x0000);
    writeReg(0x0049,0x013F);
    writeReg(0x004A,0x0000);
    writeReg(0x004B,0x0000);
    writeReg(0x0044,0xEF00);
    writeReg(0x0045,0x0000);
    writeReg(0x0046,0x013F);
    writeReg(0x0030,0x0707);
    writeReg(0x0031,0x0204);
    writeReg(0x0032,0x0204);
    writeReg(0x0033,0x0502);
    writeReg(0x0034,0x0507);
    writeReg(0x0035,0x0204);
    writeReg(0x0036,0x0204);
    writeReg(0x0037,0x0502);
    writeReg(0x003A,0x0302);
    writeReg(0x003B,0x0302);
    writeReg(0x0023,0x0000);
    writeReg(0x0024,0x0000);
    writeReg(0x0025,0x8000);
    writeReg(0x004f,0);
    writeReg(0x004e,0);
    //Fill display
    setTextColor(make_pair(white, black));
    clear(black);
}

void DisplayImpl::setCursor(Point p)
{
    #ifdef MXGUI_ORIENTATION_VERTICAL
    writeReg(0x4E,p.x());
    writeReg(0x4F,p.y());
    #elif defined MXGUI_ORIENTATION_HORIZONTAL
    writeReg(0x4E,p.y());
    writeReg(0x4F,319-p.x());
    #elif defined MXGUI_ORIENTATION_VERTICAL_MIRRORED
    writeReg(0x4E,p.x());
    writeReg(0x4F,p.y());
    #else //MXGUI_ORIENTATION_HORIZONTAL_MIRRORED
    writeReg(0x4E,p.y());
    writeReg(0x4F,319-p.x());
    #endif
}


void DisplayImpl::textWindow(Point p1, Point p2)
{
    #ifdef MXGUI_ORIENTATION_VERTICAL
    writeReg(0x11, 0x6078); //ID = 11 AM = 1
    writeReg(0x44, p1.x() | (p2.x() << 8) );
    writeReg(0x45, p1.y());
    writeReg(0x46, p2.y());
    setCursor(p1);
    #elif defined MXGUI_ORIENTATION_HORIZONTAL
    writeReg(0x11,0x6050); //ID = 01 AM = 0
    writeReg(0x44, p1.y() | (p2.y() << 8) );
    writeReg(0x45, 319-p2.x());
    writeReg(0x46, 319-p1.x());
    setCursor(p1);
    #elif defined MXGUI_ORIENTATION_VERTICAL_MIRRORED
    writeReg(0x11, 0x6078);
    writeReg(0x44, p1.x() | (p2.x() << 8) );
    writeReg(0x45, p1.y());
    writeReg(0x46, p2.y());
    setCursor(p1);
    #else //MXGUI_ORIENTATION_HORIZONTAL_MIRRORED
    writeReg(0x11,0x6050); //ID = 01 AM = 0
    writeReg(0x44, p1.y() | (p2.y() << 8) );
    writeReg(0x45, 319-p2.x());
    writeReg(0x46, 319-p1.x());
    setCursor(p1);
    #endif
}

void DisplayImpl::imageWindow(Point p1, Point p2)
{
    #ifdef MXGUI_ORIENTATION_VERTICAL
    writeReg(0x11, 0x6070); //ID = 11 AM = 0 : GRAM increment left-to-right first, then up-to-down
    writeReg(0x44, p1.x() | (p2.x() << 8) );
    writeReg(0x45, p1.y());
    writeReg(0x46, p2.y());
    setCursor(p1);
    #elif defined MXGUI_ORIENTATION_HORIZONTAL
    writeReg(0x11,0x6058); //ID = 01 AM = 1
    writeReg(0x44, p1.y() | (p2.y() << 8) );
    writeReg(0x45, 319-p2.x());
    writeReg(0x46, 319-p1.x());
    setCursor(p1);
    #elif defined MXGUI_ORIENTATION_VERTICAL_MIRRORED
    writeReg(0x11, 0x6070); //ID = 11 AM = 0 : GRAM increment left-to-right first, then up-to-down
    writeReg(0x44, p1.x() | (p2.x() << 8) );
    writeReg(0x45, p1.y());
    writeReg(0x46, p2.y());
    setCursor(p1);
    #else //MXGUI_ORIENTATION_HORIZONTAL_MIRRORED
    writeReg(0x11,0x6058); //ID = 01 AM = 1
    writeReg(0x44, p1.y() | (p2.y() << 8) );
    writeReg(0x45, 319-p2.x());
    writeReg(0x46, 319-p1.x());
    setCursor(p1);
    #endif
}

DisplayImpl::DisplayMemLayout *const DisplayImpl::DISPLAY=
        reinterpret_cast<DisplayImpl::DisplayMemLayout*>(0x6C000000);

} //namespace mxgui

#endif //_BOARD_REDBULL_V2
