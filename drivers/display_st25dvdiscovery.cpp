/***************************************************************************
 *   Copyright (C) 2014 by Terraneo Federico                               *
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

#ifdef _BOARD_STM32F415VG_ST25DVDISCOVERY
#include "display_st25dvdiscovery.h"
#include "miosix.h"
#include "misc_inst.h"
#include "line.h"
#include <cstdarg>

using namespace std;
using namespace miosix;

namespace mxgui {

//Control interface
typedef Gpio<GPIOB_BASE,13> scl; //SPI SCK
typedef Gpio<GPIOC_BASE, 3> sda; //SPI MOSI
typedef Gpio<GPIOC_BASE, 0> dcx; //Data/command
typedef Gpio<GPIOB_BASE,12> csx; //SPI CS

/**
 * Send and receive a byte through SPI2
 * \param c byte to send
 * \return byte received
 */
static unsigned char spi2sendRev(unsigned char c=0)
{
    SPI2->DR=c;
    while((SPI2->SR & SPI_SR_RXNE)==0) ;
    return SPI2->DR;
}

void sendCmd(unsigned char cmd, int len, ...)
{
    // Send Command
    dcx::low();
    csx::low();
    spi2sendRev(cmd);
    dcx::high();

    // Send Arguments
    va_list arg;
    va_start(arg,len);
    for(int i=0;i<len;i++)
    {
        spi2sendRev(va_arg(arg,int));
    }
    va_end(arg);
    csx::high();
}

Transaction::Transaction(unsigned char cmd)
{
    dcx::low();
    csx::low();
    spi2sendRev(cmd);
    dcx::high();
}

void Transaction::write(unsigned char c)
{
    spi2sendRev(c);
}

Transaction::~Transaction()
{
    csx::high();
}


void registerDisplayHook(DisplayManager& dm)
{
    dm.registerDisplay(&DisplayImpl::instance());
}

DisplayImpl& DisplayImpl::instance()
{
    static DisplayImpl instance;
    return instance;
}

void DisplayImpl::doTurnOn()
{
    sendCmd(0x29,0); //LCD_DISPLAY_ON
}

void DisplayImpl::doTurnOff()
{
    sendCmd(0x28,0); //LCD_DISPLAY_ON
}

void DisplayImpl::doSetBrightness(int brt) 
{
    //TODO - Can be set by 0x51h
}

pair<short int, short int> DisplayImpl::doGetSize() const
{
    return make_pair(height,width);
}

void DisplayImpl::write(Point p, const char *text)
{
    font.draw(*this, textColor, p, text);
}

void DisplayImpl::clippedWrite(Point p, Point a,  Point b, const char *text)
{
    font.clippedDraw(*this, textColor, p, a, b, text);
}

void DisplayImpl::clear(Color color)
{
    clear(Point(0,0), Point(width-1,height-1), color);
}

void DisplayImpl::clear(Point p1, Point p2, Color color)
{
    unsigned char lsb = color & 0xFF;
    unsigned char msb = (color >> 8) & 0xFF;

    imageWindow(p1, p2);
    int numPixels = (p2.x() - p1.x() + 1) * (p2.y() - p1.y() + 1);

    Transaction t(0x2c);
    //Send data to write on GRAM
    for(int i=0; i < numPixels; i++)
    {
        t.write(msb);
        t.write(lsb);
    }
}

void DisplayImpl::beginPixel() {}

void DisplayImpl::setPixel(Point p, Color color)
{
    unsigned char lsb = color & 0xFF;
    unsigned char msb = (color >> 8) & 0xFF;
    imageWindow(p, p); // set cursor
    sendCmd(0x2c,2,msb,lsb); // RAMWR
}

void DisplayImpl::line(Point a, Point b, Color color)
{
    //Horizontal line speed optimization
    if(a.y()==b.y())
    {
        imageWindow(Point(min(a.x(),b.x()),a.y()),
                    Point(max(a.x(),b.x()),a.y()));
        Transaction ts(0x2c);
        int numPixels=abs(a.x()-b.x());
        for(int i=0;i<=numPixels;i++)
        {
            ts.write(static_cast<unsigned char>(color >> 8));
            ts.write(static_cast<unsigned char>(color & 0xFF));
        }
        return;
    }
    //Vertical line speed optimization
    if(a.x()==b.x())
    {
        textWindow(Point(a.x(),min(a.y(),b.y())),
                    Point(a.x(),max(a.y(),b.y())));
        Transaction ts(0x2c);
        int numPixels=abs(a.y()-b.y());
        for(int i=0;i<=numPixels;i++)
        {
            ts.write(static_cast<unsigned char>(color >> 8));
            ts.write(static_cast<unsigned char>(color & 0xFF));
        }
        return;
    }
    //General case, always works but it is much slower due to the display
    //not having fast random access to pixels
    Line::draw(*this, a, b, color);
}

void DisplayImpl::scanLine(Point p, const Color *colors, unsigned short length)
{
    imageWindow(p,Point(width-1,p.y()));

    unsigned char lsb = 0x00;
    unsigned char msb = 0x00;

    Transaction t(0x2c); // RAMWR
    for (int i = 0; i < length; i++)
    {
        lsb = colors[i] & 0xFF;
        msb = (colors[i] >> 8) & 0xFF;
        t.write(msb);
        t.write(lsb);
    }
}

Color* DisplayImpl::getScanLineBuffer()
{
    if(buffer == 0) buffer = new Color[getWidth()];
    return buffer;
}

void DisplayImpl::scanLineBuffer(Point p, unsigned short length)
{
    scanLine(p, buffer, length);
}

void DisplayImpl::drawImage(Point p, const ImageBase& img)
{
    short int xEnd = p.x() + img.getWidth() - 1;
    short int yEnd = p.y() + img.getHeight() - 1;
    if(xEnd >= width || yEnd >= height) { return; }

    const unsigned short *imgData = img.getData();
    if(imgData != 0)
    {
        unsigned char lsb = 0x00;
        unsigned char msb = 0x00;

        //Optimized version for memory-loaded images
        imageWindow(p, Point(xEnd, yEnd));
        int numPixels = img.getHeight() * img.getWidth();

        Transaction t(0x2c); // RAMWR
        for(int i=0; i <= numPixels; i++)
        {
            lsb = imgData[i] & 0xFF;
            msb = (imgData[i] >> 8) & 0xFF;
            t.write(msb);
            t.write(lsb);
        }
    }
    else img.draw(*this,p);
}

void DisplayImpl::clippedDrawImage(Point p, Point a, Point b, const ImageBase& img)
{
    img.clippedDraw(*this,p,a,b);
}

void DisplayImpl::drawRectangle(Point a, Point b, Color c)
{
    line(a,Point(b.x(), a.y()), c);
    line(Point(b.x(), a.y()), b, c);
    line(b,Point(a.x(), b.y()), c);
    line(Point(a.x(), b.y()), a, c);
}

DisplayImpl::pixel_iterator DisplayImpl::begin(Point p1, Point p2, IteratorDirection d)
{
    if(p1.x()<0 || p1.y()<0 || p2.x()<0 || p2.y()<0)
    {
        return pixel_iterator();
    }
    if(p1.x() >= width || p1.y() >= height || p2.x() >= width || p2.y() >= height)
    {
        return pixel_iterator();
    }
    if(p2.x() < p1.x() || p2.y() < p1.y())
    {
        return pixel_iterator();
    }

    if(d==DR) textWindow(p1,p2);
    else imageWindow(p1,p2);
    sendCmd(0x2c,0);

    unsigned int numPixels=(p2.x()-p1.x()+1)*(p2.y()-p1.y()+1);
    return pixel_iterator(numPixels);
}

DisplayImpl::DisplayImpl() : buffer(0)
{
    // SPI2 Configuration
    {
        FastInterruptDisableLock dLock;
        
        scl::mode(Mode::ALTERNATE);    scl::alternateFunction(5); //SPI5
        sda::mode(Mode::ALTERNATE);    sda::alternateFunction(5);
        csx::mode(Mode::OUTPUT);       csx::high();
        dcx::mode(Mode::OUTPUT);
        
        RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;      
        RCC_SYNC();
    }

    // The ILI9341 datasheet says the SPI interface runs up to 10MHz but
    // this controller has been overclocked up to 100MHz with no side-effects:
    // https://www.eevblog.com/forum/microcontrollers/ili9341-lcd-driver-max-spi-clock-speed/
    SPI2->CR1=SPI_CR1_SSM   //Sowtware CS
            | SPI_CR1_SSI   //Software CS high
            | SPI_CR1_SPE   //SPI enabled
            | (0<<3)        //Divide input clock by 2: 42/2=21MHz
            | SPI_CR1_MSTR; //Master mode
    Thread::sleep(1);

    // ILI9341 Configuration
    sendCmd(0xca,3,0xc3,0x08,0x50);           //undocumented command
    sendCmd(0xcf,3,0x00,0xc1,0x30);           //LCD_POWERB
    sendCmd(0xed,4,0x64,0x03,0x12,0x81);      //LCD_POWER_SEQ
    sendCmd(0xe8,3,0x85,0x00,0x78);           //LCD_DTCA
    sendCmd(0xcb,5,0x39,0x2c,0x00,0x34,0x02); //LCD_POWERA
    sendCmd(0xf7,1,0x20);                     //LCD_PRC
    sendCmd(0xea,2,0x00,0x00);                //LCD_DTCB
    sendCmd(0xb1,2,0x00,0x1b);                //LCD_FRMCTR1
    sendCmd(0xb6,2,0x0a,0xa2);                //LCD_DFC
    sendCmd(0xc0,1,0x10);                     //LCD_POWER1
    sendCmd(0xc1,1,0x10);                     //LCD_POWER2
    sendCmd(0xc5,2,0x45,0x15);                //LCD_VCOM1
    sendCmd(0xc7,1,0x90);                     //LCD_VCOM2
    sendCmd(0x36,1,0x08);                     //LCD_MAC
    sendCmd(0xf2,1,0x00);                     //LCD_3GAMMA_EN
    sendCmd(0xb6,4,0x0a,0xa7,0x27,0x04);      //LCD_DFC
    sendCmd(0x2a,4,0x00,0x00,0x00,0xef);      //LCD_COLUMN_ADDR
    sendCmd(0x2b,4,0x00,0x00,0x01,0x3f);      //LCD_PAGE_ADDR
    sendCmd(0xf6,3,0x01,0x00,0x00);           //LCD_INTERFACE
    sendCmd(0x3a,1,0x05);
    sendCmd(0x2c,0);                          //LCD_GRAM
    Thread::sleep(200);
    sendCmd(0x26,1,0x01);                     //LCD_GAMMA
    sendCmd(0xe0,15,0x0f,0x29,0x24,0x0c,0x0e,0x09,0x4e,0x78,0x3c,0x09,0x13,
            0x05,0x17,0x11,0x00);             //LCD_PGAMMA
    sendCmd(0xe1,15,0x00,0x16,0x1b,0x04,0x11,0x07,0x31,0x33,0x42,0x05,0x0c,
            0x0a,0x28,0x2f,0x0f);             //LCD_NGAMMA
    sendCmd(0x11,0);                          //LCD_SLEEP_OUT
    Thread::sleep(500);
    sendCmd(0x29,0);                          //LCD_DISPLAY_ON
    sendCmd(0x2c,0);                          //LCD_GRAM

    imageWindow(Point(0,0), Point(width-1,height-1));
    setTextColor({white, black});
    clear(black);
};

DisplayImpl::~DisplayImpl() 
{
    if (buffer) delete[] buffer;
}

}

#endif //_BOARD_STM32F415VG_ST25DVDISCOVERY
