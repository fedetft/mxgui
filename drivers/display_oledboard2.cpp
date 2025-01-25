/***************************************************************************
 *   Copyright (C) 2015 by Terraneo Federico                               *
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

#include "display_oledboard2.h"
#include "miosix.h"
#include <algorithm>

using namespace std;
using namespace miosix;

#ifdef _BOARD_STM32F429ZI_OLEDBOARD2

#include "interfaces-impl/hwmapping.h" //This file may not exist in other boards

namespace mxgui {

/**
 * Send a byte through SPI6
 * \param c byte to send
 */
static void spi6send(unsigned char c)
{
    SPI6->DR=c;
    while((SPI6->SR & SPI_SR_RXNE)==0) ;
    //Without reading back DR, RXNE is never reset, so SPI sending fails
    volatile unsigned char discard=SPI6->DR;
    (void)discard;
}

/**
 * Write into an 8 bit register of the AMS369FG03-0 display controller
 * \param reg register number
 * \param val value to write
 */
static void sendCommand8(unsigned char reg, unsigned char val)
{
    display::cs::low();
    spi6send(0x70);
    spi6send(reg);
    display::cs::high();
    delayUs(1);
    display::cs::low();
    spi6send(0x72);
    spi6send(val);
    display::cs::high();
    delayUs(1);
}

/**
 * Write into a 16 bit register of the AMS369FG03-0 display controller
 * \param reg register number
 * \param val value to write
 */
static void sendCommand16(unsigned char reg, unsigned short val)
{
    sendCommand8(reg,val>>8);
    display::cs::low();
    spi6send(0x72);
    spi6send(val & 0xff);
    display::cs::high();
    delayUs(1);
}

void registerDisplayHook(DisplayManager& dm)
{
    dm.registerDisplay(&DisplayImpl::instance());
}

//
// Class DisplayImpl
//
const short int DisplayImpl::width;
const short int DisplayImpl::height;

DisplayImpl& DisplayImpl::instance()
{
    static DisplayImpl instance;
    return instance;
}

void DisplayImpl::doTurnOn()
{
    LTDC->GCR |= LTDC_GCR_LTDCEN;
    sendCommand8(0x1d,0xa0);
    Thread::sleep(200);
    //If display supply is provided too early an intense white flash appears
    //Total delay should be 250ms, display power is added after the first 200ms
    display::vregEn::high();
    Thread::sleep(50);
    sendCommand8(0x14,0x03);
}

void DisplayImpl::doTurnOff()
{
    sendCommand8(0x14,0x00);
    Thread::sleep(35);
    display::vregEn::low();
    Thread::sleep(15);
    sendCommand8(0x1d,0xa1);
    LTDC->GCR &=~ LTDC_GCR_LTDCEN;
}

void DisplayImpl::doSetBrightness(int brt)
{
    int brightness=max(0,min(4,brt/24));
    sendCommand8(0x39,brightness<<4);
}

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
    if(p1.x()<0 || p2.x()<p1.x() || p2.x()>=width
     ||p1.y()<0 || p2.y()<p1.y() || p2.y()>=height) return;
    if((color & 0xff)==(color>>8))
    {
        //Can use memset
        if(p1.x()==0 && p2.x()==width-1)
        {
            //Can merge lines
            memset(framebuffer1+p1.y()*width,color,(p2.y()-p1.y()+1)*width*bpp);
        } else {
            //Can't merge lines
            Color *ptr=framebuffer1+p1.x()+width*p1.y();
            short len=p2.x()-p1.x()+1;
            for(short i=p1.y();i<=p2.y();i++)
            {
                memset(ptr,color,len*bpp);
                ptr+=width;
            }
        }
    } else {
        //Can't use memset
        if(p1.x()==0 && p2.x()==width-1)
        {
            //Can merge lines
            Color *ptr=framebuffer1+p1.y()*width;
            int numPixels=(p2.y()-p1.y()+1)*width;
            //This loop is worth unrolling
            for(int i=0;i<numPixels/4;i++)
            {
                *ptr++=color;
                *ptr++=color;
                *ptr++=color;
                *ptr++=color;
            }
            for(int i=0;i<(numPixels & 3);i++) *ptr++=color;
        } else {
            //Can't merge lines
            Color *ptr=framebuffer1+p1.x()+width*p1.y();
            short len=p2.x()-p1.x()+1;
            for(short i=p1.y();i<=p2.y();i++)
            {
                for(short j=0;j<len;j++) *ptr++=color;
                ptr+=width-len;
            }
        }
    }
}

void DisplayImpl::beginPixel() {}

void DisplayImpl::setPixel(Point p, Color color)
{
    int offset=p.x()+p.y()*width;
    if(offset<0 || offset>=numPixels) return;
    *(framebuffer1+offset)=color;
}

void DisplayImpl::line(Point a, Point b, Color color)
{
    //Horizontal line speed optimization
    if(a.y()==b.y())
    {
        short minx=min(a.x(),b.x());
        short maxx=max(a.x(),b.x());
        if(minx<0 || maxx>=width || a.y()<0 || a.y()>=height) return;
        Color *ptr=framebuffer1+minx+width*a.y();
        for(short i=minx;i<=maxx;i++) *ptr++=color;
        return;
    }
    //Vertical line speed optimization
    if(a.x()==b.x())
    {
        short miny=min(a.y(),b.y());
        short maxy=max(a.y(),b.y());
        if(a.x()<0 || a.x()>=width || miny<0 || maxy>=height) return;
        Color *ptr=framebuffer1+a.x()+width*miny;
        for(short i=miny;i<=maxy;i++)
        {
            *ptr=color;
            ptr+=width;
        }
        return;
    }
    //General case
    Line::draw(*this,a,b,color);
}

void DisplayImpl::scanLine(Point p, const Color *colors, unsigned short length)
{
    if(p.x()<0 || static_cast<int>(p.x())+static_cast<int>(length)>width
        ||p.y()<0 || p.y()>=height) return;
    Color *ptr=framebuffer1+p.x()+p.y()*width;
    memcpy(ptr,colors,length*bpp);
}

Color *DisplayImpl::getScanLineBuffer()
{
    return buffer;
}

void DisplayImpl::scanLineBuffer(Point p, unsigned short length)
{
    int offset=p.x()+p.y()*width;
    if(offset<0 || offset>=numPixels) return;
    memcpy(framebuffer1+offset,buffer,length*bpp);
}

void DisplayImpl::drawImage(Point p, const ImageBase& img)
{
    short int xEnd=p.x()+img.getWidth()-1;
    short int yEnd=p.y()+img.getHeight()-1;
    if(p.x()<0 || p.y()<0 || xEnd<p.x() || yEnd<p.y()
        ||xEnd >= width || yEnd >= height) return;

//    const unsigned short *imgData=img.getData();
//    if(imgData!=0)
//    {
//        //TODO Optimized version for in-memory images
//    } else
    img.draw(*this,p);
}

void DisplayImpl::clippedDrawImage(Point p, Point a, Point b, const ImageBase& img)
{
//    if(img.getData()==0)
//    {
    img.clippedDraw(*this,p,a,b);
    return;
//    } //else optimized version for memory-loaded images
//        //TODO: optimize
//    }
}

void DisplayImpl::drawRectangle(Point a, Point b, Color c)
{
    line(a,Point(b.x(),a.y()),c);
    line(Point(b.x(),a.y()),b,c);
    line(b,Point(a.x(),b.y()),c);
    line(Point(a.x(),b.y()),a,c);
}

DisplayImpl::pixel_iterator DisplayImpl::begin(Point p1, Point p2,
        IteratorDirection d)
{
    bool fail=false;
    if(p1.x()<0 || p1.y()<0 || p2.x()<0 || p2.y()<0) fail=true;
    if(p1.x()>=width || p1.y()>=height || p2.x()>=width || p2.y()>=height) fail=true;
    if(p2.x()<p1.x() || p2.y()<p1.y()) fail=true;
    if(fail)
    {
        //Return invalid (dummy) iterators
        this->last=pixel_iterator();
        return this->last;
    }

    //Set the last iterator to a suitable one-past-the last value
    if(d==DR) this->last=pixel_iterator(Point(p2.x()+1,p1.y()),p2,d,this);
    else this->last=pixel_iterator(Point(p1.x(),p2.y()+1),p2,d,this);

    return pixel_iterator(p1,p2,d,this);
}

DisplayImpl::~DisplayImpl() {}

DisplayImpl::DisplayImpl()
    : framebuffer1(reinterpret_cast<unsigned short*>(0xd0600000)),
      buffer(framebuffer1+numPixels)
{
    /*
     * Display refresh rate is critical, as a high rate takes up a significant
     * portion of the SDRAM bandwidth. The display works well down to a 30Hz
     * refresh rate with no flickering, but there is a dependence of the display
     * brightness on the refresh rate. The refresh rate can be computed from the
     * dotclock as REFRESH=DOTCLOCK/((480+16)*(800+16)). This table contains
     * possible PLL values that give different refresh rates. It also lists the
     * current consumption measured as the STOD13AS power supply (the chip used
     * to generate ELVDD and ELVSS) when powered at 4.3V and with a fully white
     * display and the gamma settings that produce the maximum brightness.
     * As I don't have a way to measure the display brightness in cd/m^2, the
     * brightness data is measured connecting a SFH216 photodiode to an
     * oscilloscope and measuring the voltage produced in mV when facing the
     * display.
     */
    static const unsigned char pllFreqTable[]=
    {        //DOTCLOCK | REFRESH | current | brightness
        96,  //12MHz    | 29.65Hz | 170mA   | 221mV
        128, //16MHz    | 39.53Hz | 200mA   | 235mV
        144, //18MHz    | 44.47Hz | 210mA   | 241mV
        160, //20MHz    | 49.41Hz | 220mA   | 246mV
        192  //24MHz    | 59.30Hz | 240mA   | 252mV
    };
    static const unsigned int pllFreqChoice=1; //Chosen refresh rate: ~40Hz
    static const unsigned int pllFreq=pllFreqTable[pllFreqChoice];
    
    //
    // First of all, enable peripherals and LDTC PLL
    //
    {
        FastInterruptDisableLock dLock;
        //PLLSAI runs @ <pllfreq>MHz, both Q and R outputs are divided by 4
        RCC->PLLSAICFGR=4<<28 | 4<<24 | pllFreq<<6;
        //The display dotclock is additionally divided by 2
        RCC->DCKCFGR=0<<16;
        RCC->CR |= RCC_CR_PLLSAION;
    }
    while((RCC->CR & RCC_CR_PLLSAIRDY)==0) ;
    {
        FastInterruptDisableLock dLock;
        RCC->APB2ENR |= RCC_APB2ENR_LTDCEN | RCC_APB2ENR_SPI6EN;
        RCC_SYNC();
    }
    SPI6->CR1=SPI_CR1_SSM   //Sowtware CS
            | SPI_CR1_SSI   //Software CS high
            | SPI_CR1_SPE   //SPI enabled
            | (3<<3)        //Divide input clock by 16: 84/16=5.25MHz
            | SPI_CR1_MSTR  //Master mode
            | SPI_CR1_CPOL  //Clock default high
            | SPI_CR1_CPHA; //Rising edge is active edge
    
    display::reset::low();
    delayUs(10);
    display::reset::high();
    
    //
    // Configure LDTC so as to output RGB data
    //
    //Note: hbp is defined as 8-hsync as the display considers the hsync and
    //vsync time as part of the back porch, while the stm32 does not. For some
    //unknown reason not taking this into account causes a weird behaviour:
    //the RGB data being sent to the display gets interpreted as BGR.
    const unsigned int hsync=4;    //hsync timing
    const unsigned int vsync=1;    //vsync timing
    const unsigned int hbp=8-hsync;//horizontal back porch
    const unsigned int vbp=8-vsync;//vertical back porch
    const unsigned int hfp=8;      //horizontal front porch
    const unsigned int vfp=8;      //vertical front porch
    enum {
        ARGB8888=0,
        RGB888=1,
        RGB565=2,
        ARGB1555=3,
        ARGB4444=4,
        L8=5,
        AL44=6,
        AL88=7
    };
    //Configure timings
    LTDC->SSCR=(hsync-1)<<16               | (vsync-1);
    LTDC->BPCR=(hsync+hbp-1)<<16           | (vsync+vbp-1);
    LTDC->AWCR=(hsync+hbp+width-1)<<16     | (vsync+vbp+height-1);
    LTDC->TWCR=(hfp+hsync+hbp+width-1)<<16 | (vfp+vsync+vbp+height-1);
    //Configre background color (black))
    LTDC->BCCR=0;
    //Enable layer 2
    LTDC_Layer2->CR=0              //Disable palette mode
                  | 0              //Disable color keying
                  | LTDC_LxCR_LEN; //Enable layer
    LTDC_Layer2->WHPCR=(hsync+hbp+width-1)<<16  | (hsync+hbp);
    LTDC_Layer2->WVPCR=(vsync+vbp+height-1)<<16 | (vsync+vbp);
    LTDC_Layer2->CKCR=0;
    LTDC_Layer2->PFCR=RGB565;
    LTDC_Layer2->CACR=0xff; //Alpha=1
    LTDC_Layer2->DCCR=0;
    LTDC_Layer2->CFBAR=reinterpret_cast<unsigned int>(framebuffer1);
    LTDC_Layer2->CFBLR=(width*bpp)<<16 | (width*bpp+3); //Packed lines
    LTDC_Layer2->CFBLNR=height;
    //Write to shadow registers
    LTDC->SRCR=LTDC_SRCR_IMR;
    //Finally enable the display
    LTDC->GCR=0                //hsync active low
            | 0                //vsync active low
            | LTDC_GCR_DEPOL   //enable active low
            | LTDC_GCR_PCPOL   //input pixel clock
            | 0                //no dithering
            | LTDC_GCR_LTDCEN; //Display enabled
    
    //
    // Last, wait at least one frame and configure and turn on display
    //
    Thread::sleep(35);
    sendCommand8(0x31,0x08);
    sendCommand8(0x32,0x14);
    sendCommand8(0x30,0x02);
    sendCommand8(0x27,0x01);
    sendCommand8(0x12,0x08);
    sendCommand8(0x13,0x08);
    sendCommand8(0x15,0x00);
    sendCommand8(0x16,0x01); //STM32 converts RGB565 to RGB666
    sendCommand16(0xef,0xd0e8);
    //LUT-based 5 brightness levels with gamma correction
    static const unsigned char r[]=
    {
        //Note: power consumption data assuming 40Hz refresh rate, white screen
        0x00,0x3f,0x3f,0x35,0x30,0x2c,0x13,// 10cd/m^2  55mW
        0x00,0x3f,0x35,0x2c,0x2b,0x26,0x29,// 70cd/m^2 250mW
        0x00,0x3f,0x2e,0x29,0x2a,0x23,0x34,//130cd/m^2 420mW
        0x00,0x3f,0x29,0x29,0x27,0x22,0x3c,//190cd/m^2 590mW
        0x00,0x3f,0x2a,0x27,0x27,0x1f,0x44 //250cd/m^2 840mW
    };
    static const unsigned char g[]=
    {
        0x00,0x00,0x00,0x00,0x27,0x2b,0x12,
        0x00,0x00,0x00,0x25,0x29,0x26,0x28,
        0x00,0x00,0x0a,0x25,0x28,0x23,0x33,
        0x00,0x00,0x10,0x26,0x26,0x22,0x3b,
        0x00,0x00,0x17,0x24,0x26,0x1f,0x43
    };
    static const unsigned char b[]=
    {
        0x00,0x3f,0x3f,0x34,0x2f,0x2b,0x1b,
        0x00,0x3f,0x34,0x2b,0x2a,0x23,0x37,
        0x00,0x3f,0x2d,0x28,0x27,0x20,0x46,
        0x00,0x3f,0x28,0x28,0x24,0x1f,0x50,
        0x00,0x3f,0x2a,0x25,0x24,0x1b,0x5c
    };
    const int numBrigtnessLevels=5;
    const int numGammaRegisters=7;
    for(int i=0;i<numBrigtnessLevels;i++)
    {
        sendCommand8(0x39,0x40 | i); //Default is max brightness
        for(int j=0;j<numGammaRegisters;j++)
        {
            sendCommand8(0x40+j,r[j+i*numGammaRegisters]);
            sendCommand8(0x50+j,g[j+i*numGammaRegisters]);
            sendCommand8(0x60+j,b[j+i*numGammaRegisters]);
        }
    }
    sendCommand8(0x17,0x22);
    sendCommand8(0x18,0x33);
    sendCommand8(0x19,0x03);
    sendCommand8(0x1a,0x01);
    sendCommand8(0x22,0xa2); //Changed to adapt to a display voltage of 3.3V
    sendCommand8(0x23,0x00);
    sendCommand8(0x26,0xa0);
    
    setTextColor(make_pair(Color(0xffff),Color(0x0000)));
    clear(black);
    doTurnOn();
}

Color DisplayImpl::pixel_iterator::dummy;

} //namespace mxgui

#endif //_BOARD_STM32F429ZI_OLEDBOARD2
