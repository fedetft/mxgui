/***************************************************************************
 *   Copyright (C) 2025 by Terraneo Federico, Bontempi Andrea              *
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

#include "display_gc9a01.h"
#include <miosix.h>
#include <kernel/scheduler/scheduler.h>
#include <interfaces/endianness.h>
#include <algorithm>
#include <line.h>
#include <thread>

using namespace std;
using namespace miosix;
using namespace mxgui;

#ifndef BOARD_RP2040_RASPBERRY_PI_PICO
#warning "The SPI driver has only been tested on an BOARD_RP2040_RASPBERRY_PI_PICO"
#endif

//Display connection
using spi0_mosi = Gpio<GPIO0_BASE, 11>;
using spi0_sck = Gpio<GPIO0_BASE, 10>;
using dc = Gpio<GPIO0_BASE, 8>;
using cs = Gpio<GPIO0_BASE, 9>;
using reset = Gpio<GPIO0_BASE, 12>;
using back = Gpio<GPIO0_BASE, 25>;

static void spiInit()
{
    spi0_mosi::mode(Mode::OUTPUT);
    spi0_mosi::function(Function::SPI);
    //spi0_miso::mode(Mode::INPUT);
    //spi0_miso::function(Function::SPI);
    spi0_sck::mode(Mode::OUTPUT);
    spi0_sck::function(Function::SPI);
    cs::mode(Mode::OUTPUT);
    cs::low();
    dc::mode(Mode::OUTPUT);
    reset::mode(Mode::OUTPUT);
    back::mode(Mode::OUTPUT);
    back::high();
    unreset_block_wait(RESETS_RESET_SPI1_BITS);

    spi1_hw->cr0 = 0 << 8 | 0b111; // Speed 62.5 MHz
    spi1_hw->cr1 = 0;
    spi1_hw->cpsr = 2; // Speed 62.5 MHz
    spi1_hw->cr1 |= 1 << 1;
}

/**
 * Send and receive a byte, thus returning only after transmission is complete
 * \param x byte to send
 * \return the received byte
 */
static unsigned char spiSendRecv(unsigned char x)
{
    spi1_hw->dr = x;
    while((spi1_hw->sr & (1 << 2)) == 0) ;
    return spi1_hw->dr;
}

/**
 * Send a command to the display
 * \param c command
 */
static void cmd(unsigned char c)
{
    dc::low();
    cs::low();
    spiSendRecv(c);
    cs::high();
    delayUs(1);
}

/**
 * Send data to the display
 * \param d data
 */
static void dat(unsigned char d)
{
    dc::high();
    cs::low();
    spiSendRecv(d);
    cs::high();
    delayUs(1);
}

//
// class DisplayGc9a01
//

namespace mxgui {

DisplayGc9a01::DisplayGc9a01() : buffer(nullptr)
{

    spiInit();
    reset::high();
    this_thread::sleep_for(100ms);
    reset::low();
    this_thread::sleep_for(100ms);
    reset::high();
	cs::low();
    this_thread::sleep_for(100ms);
    
    cmd(0xef); // Inner register enable 2
	cmd(0xeb); dat(0x14); // ?
    cmd(0xfe); // Inner register enable 1
	cmd(0xef); // Inner register enable 2
	cmd(0xeb); dat(0x14); // ?
	cmd(0x84); dat(0x40); // ?
	cmd(0x85); dat(0xff); // ?
	cmd(0x86); dat(0xff); // ?
	cmd(0x87); dat(0xff); // ?
    cmd(0x88); dat(0x0a); // ?
	cmd(0x89); dat(0x21); // ?
	cmd(0x8a); dat(0x00); // ?
	cmd(0x8b); dat(0x80); // ?
	cmd(0x8c); dat(0x01); // ?
	cmd(0x8d); dat(0x01); // ?
    cmd(0x8E); dat(0xff); // ?
    cmd(0x8f); dat(0xff); // ?
    cmd(0xb6); dat(0x00); dat(0x20); // Display Function Control
    cmd(0x36); dat(0x08); // Memory Access Control
    cmd(0x3a); dat(0x05); // Pixel Format Set
    cmd(0x90); dat(0x08); dat(0x08); dat(0x08); dat(0x08); // ?
    cmd(0xbd); dat(0x06); // ?
    cmd(0xbc); dat(0x00); // ?
    cmd(0xff); dat(0x60); dat(0x01); dat(0x04); // ?
    cmd(0xc3); dat(0x13); // Vreg1a voltage Control
    cmd(0xc4); dat(0x13); // Vreg1b voltage Control
    cmd(0xc9); dat(0x22); // Vreg2a voltage Control
    cmd(0xbe); dat(0x11); // ?
    cmd(0xe1); dat(0x10); dat(0x0E); // ?
    cmd(0xdf); dat(0x21); dat(0x0c); dat(0x02); // ?
    cmd(0xf0); dat(0x45); dat(0x09); dat(0x08); dat(0x08); dat(0x26); dat(0x2a); // SET_GAMMA1
    cmd(0xf1); dat(0x43); dat(0x70); dat(0x72); dat(0x36); dat(0x37); dat(0x6f); // SET_GAMMA2
    cmd(0xf2); dat(0x45); dat(0x09); dat(0x08); dat(0x08); dat(0x26); dat(0x2a); // SET_GAMMA3
    cmd(0xf3); dat(0x43); dat(0x70); dat(0x72); dat(0x36); dat(0x37); dat(0x6f); // SET_GAMMA4
    cmd(0xed); dat(0x1b); dat(0x0b); // ?
    cmd(0xae); dat(0x77); // ?
    cmd(0xcd); dat(0x63); // ?
    //cmd(0x70); dat(0x07); dat(0x07); dat(0x04); dat(0x0e); dat(0x0f); dat(0x09); dat(0x07); dat(0x08); dat(0x03); // ?
    cmd(0xe8); dat(0x34); // Frame Rate
    cmd(0x62); dat(0x18); dat(0x0d); dat(0x71); dat(0xed); dat(0x70); dat(0x70); dat(0x18); dat(0x0f); dat(0x71); dat(0xef); dat(0x70); dat(0x70); // ?
    cmd(0x63); dat(0x18); dat(0x11); dat(0x71); dat(0xf1); dat(0x70); dat(0x70); dat(0x18); dat(0x13); dat(0x71); dat(0xf3); dat(0x70); dat(0x70); // ?
    cmd(0x64); dat(0x28); dat(0x29); dat(0xf1); dat(0x01); dat(0xf1); dat(0x00); dat(0x07); // ?
    cmd(0x66); dat(0x3c); dat(0x00); dat(0xcd); dat(0x67); dat(0x45); dat(0x45); dat(0x10); dat(0x00); dat(0x00); dat(0x00); // ?
    cmd(0x67); dat(0x00); dat(0x3C); dat(0x00); dat(0x00); dat(0x00); dat(0x01); dat(0x54); dat(0x10); dat(0x32); dat(0x98); // ?
    cmd(0x74); dat(0x10); dat(0x85); dat(0x80); dat(0x00); dat(0x00); dat(0x4e); dat(0x00); // ?
    cmd(0x98); dat(0x3e); dat(0x07); // ?
    cmd(0x35); // Tearing Effect Line ON
    cmd(0x21); // Display Inversion ON
    cmd(0x11); // Sleep OUT
    this_thread::sleep_for(120ms);
    clear(0x00);
    cmd(0x29); // Display ON
    this_thread::sleep_for(20ms);
    setTextColor(make_pair(Color(0xffff),Color(0x0)));
}

void DisplayGc9a01::doTurnOn()
{
    cmd(0x29);
}

void DisplayGc9a01::doTurnOff()
{
    cmd(0x28);
}

void DisplayGc9a01::doSetBrightness(int brt)
{
    //TODO
}

pair<short int, short int> DisplayGc9a01::doGetSize() const
{
    return make_pair(height,width);
}

void DisplayGc9a01::write(Point p, const char *text)
{
    font.draw<DisplayGc9a01, true>(*this,textColor,p,text);
}

void DisplayGc9a01::clippedWrite(Point p, Point a, Point b, const char *text)
{
    font.clippedDraw<DisplayGc9a01, true>(*this,textColor,p,a,b,text);
}

void DisplayGc9a01::clear(Color color)
{
    clear(Point(0,0),Point(width-1,height-1),color);
}

void DisplayGc9a01::clear(Point p1, Point p2, Color color)
{
    imageWindow(p1,p2);
    doBeginPixelWrite();
    int numPixels=(p2.x()-p1.x()+1)*(p2.y()-p1.y()+1);
    for(int i=0;i<numPixels;i++) doWritePixel(color);
    doEndPixelWrite();
}

void DisplayGc9a01::beginPixel() {}

void DisplayGc9a01::setPixel(Point p, Color color)
{
    //Can't move boilerplate to beginPixel, as can't do setCursor in between
    setCursor(p);
    doBeginPixelWrite();
    doWritePixel(color);
    doEndPixelWrite();
}

void DisplayGc9a01::line(Point a, Point b, Color color)
{
    //Horizontal line speed optimization
    if(a.y()==b.y())
    {
        imageWindow(Point(min(a.x(),b.x()),a.y()),
                    Point(max(a.x(),b.x()),a.y()));
        doBeginPixelWrite();
        int numPixels=abs(a.x()-b.x());
        for(int i=0;i<=numPixels;i++) doWritePixel(color);
        doEndPixelWrite();
        return;
    }
    //Vertical line speed optimization
    if(a.x()==b.x())
    {
        textWindow(Point(a.x(),min(a.y(),b.y())),
                    Point(a.x(),max(a.y(),b.y())));
        doBeginPixelWrite();
        int numPixels=abs(a.y()-b.y());
        for(int i=0;i<=numPixels;i++) doWritePixel(color);
        doEndPixelWrite();
        return;
    }
    //General case
    Line::draw(*this,a,b,color);
}

void DisplayGc9a01::scanLine(Point p, const Color *colors, unsigned short length)
{
    length=min<unsigned short>(length,width-p.x());
    imageWindow(p,Point(length-1,p.y()));
    cmd(0x2c); // Memory write
    dc::high();
    cs::low();
    for(int i=0;i<length;i++) {
        spiSendRecv(colors[i] >> 8);
        spiSendRecv(colors[i]);
    }
    cs::high();
    delayUs(1);
}

Color *DisplayGc9a01::getScanLineBuffer()
{
    //getWidth() would be enough as size, but we reuse the buffer for DMA
    if(buffer==nullptr) buffer=new Color[getWidth()];
    return buffer;
}

void DisplayGc9a01::scanLineBuffer(Point p, unsigned short length)
{
    scanLine(p,buffer,length);
}

void DisplayGc9a01::drawImage(Point p, const ImageBase& img)
{
    img.draw(*this,p);
}

void DisplayGc9a01::clippedDrawImage(Point p, Point a, Point b, const ImageBase& img)
{
    img.clippedDraw(*this,p,a,b);
}

void DisplayGc9a01::drawRectangle(Point a, Point b, Color c)
{
    line(a,Point(b.x(),a.y()),c);
    line(Point(b.x(),a.y()),b,c);
    line(b,Point(a.x(),b.y()),c);
    line(Point(a.x(),b.y()),a,c);
}

DisplayGc9a01::pixel_iterator DisplayGc9a01::begin(Point p1, Point p2,
        IteratorDirection d)
{
    if(p1.x()<0 || p1.y()<0 || p2.x()<0 || p2.y()<0) return pixel_iterator();
    if(p1.x()>=width || p1.y()>=height || p2.x()>=width || p2.y()>=height)
        return pixel_iterator();
    if(p2.x()<p1.x() || p2.y()<p1.y()) return pixel_iterator();
 
    if(d==DR) textWindow(p1,p2);
    else imageWindow(p1,p2);
    doBeginPixelWrite();

    unsigned int numPixels=(p2.x()-p1.x()+1)*(p2.y()-p1.y()+1);
    return pixel_iterator(numPixels);
}

DisplayGc9a01::~DisplayGc9a01() {}

void DisplayGc9a01::doBeginPixelWrite()
{
    cmd(0x2c); // Memory write
    dc::high();
    cs::low();
}
    
void DisplayGc9a01::doWritePixel(Color c)
{
    spiSendRecv(c>>8);
    spiSendRecv(c);
}
    
void DisplayGc9a01::doEndPixelWrite()
{
    cs::high();
    delayUs(1);
}

/**
 * Set cursor to desired location
 * \param point where to set cursor (0<=x<128, 0<=y<128)
 */
inline void DisplayGc9a01::setCursor(Point p)
{
    cmd(0x2a); dat(0); dat(p.x()); dat(0); dat(p.x()); // Set column address
    cmd(0x2b); dat(0); dat(p.y()); dat(0); dat(p.y()); // Set row address
}

/**
 * Set a hardware window on the screen, optimized for writing text.
 * The GRAM increment will be set to up-to-down first, then left-to-right which
 * is the correct increment to draw fonts
 * \param p1 upper left corner of the window
 * \param p2 lower right corner of the window
 */
void DisplayGc9a01::textWindow(Point p1, Point p2)
{
    cmd(0x2a); dat(0); dat(p1.y()); dat(0); dat(p2.y()); // Set row address
    cmd(0x2b); dat(0); dat(p1.x()); dat(0); dat(p2.x()); // Set column address
    #if defined(MXGUI_ORIENTATION_VERTICAL)
    cmd(0x36); dat(MADCTL_BGR | MADCTL_MV);
    #elif defined(MXGUI_ORIENTATION_VERTICAL_MIRRORED)
    cmd(0x36); dat(MADCTL_BGR | MADCTL_MV | MADCTL_MX | MADCTL_MY);
    #elif defined(MXGUI_ORIENTATION_HORIZONTAL)
    cmd(0x36); dat(MADCTL_BGR | MADCTL_MY);
    #elif defined(MXGUI_ORIENTATION_HORIZONTAL_MIRRORED)
    cmd(0x36); dat(MADCTL_BGR | MADCTL_MX);
    #endif
}

/**
 * Set a hardware window on the screen, optimized for drawing images.
 * The GRAM increment will be set to left-to-right first, then up-to-down which
 * is the correct increment to draw images
 * \param p1 upper left corner of the window
 * \param p2 lower right corner of the window
 */
void DisplayGc9a01::imageWindow(Point p1, Point p2)
{
    cmd(0x2a); dat(0); dat(p1.x()); dat(0); dat(p2.x()); // Set column address
    cmd(0x2b); dat(0); dat(p1.y()); dat(0); dat(p2.y()); // Set row address
    #if defined(MXGUI_ORIENTATION_VERTICAL)
    cmd(0x36); dat(MADCTL_BGR);
    #elif defined(MXGUI_ORIENTATION_VERTICAL_MIRRORED)
    cmd(0x36); dat(MADCTL_BGR | MADCTL_MX | MADCTL_MY);
    #elif defined(MXGUI_ORIENTATION_HORIZONTAL)
    cmd(0x36); dat(MADCTL_BGR | MADCTL_MV | MADCTL_MY);
    #elif defined(MXGUI_ORIENTATION_HORIZONTAL_MIRRORED)
    cmd(0x36); dat(MADCTL_BGR | MADCTL_MV | MADCTL_MX);
    #endif
}

} //namespace mxgui
