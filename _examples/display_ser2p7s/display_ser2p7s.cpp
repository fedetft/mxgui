/***************************************************************************
 *   Copyright (C) 2024 by Terraneo Federico                               *
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

#include "display_ser2p7s.h"
#include "miosix.h"
#include <algorithm>

using namespace std;
using namespace miosix;

#ifndef _BOARD_STM32F411CE_BLACKPILL
#warning "The SPI driver has only been tested on an STM32F411CE_BLACKPILL"
#endif

//Display connection

using cs   = Gpio<GPIOB_BASE,7>;
using sck  = Gpio<GPIOB_BASE,3>; //Used as HW SPI
using mosi = Gpio<GPIOB_BASE,5>; //Used as HW SPI
using dc   = Gpio<GPIOB_BASE,6>;
using res  = Gpio<GPIOB_BASE,4>;

/**
 * Send and receive a byte, thus returning only after transmission is complete
 * \param x byte to send
 * \return the received byte
 */
static unsigned char spi1sendRecv(unsigned char x=0)
{
    SPI1->DR=x;
    while((SPI1->SR & SPI_SR_RXNE)==0) ;
    return SPI1->DR;
}

/**
 * Send a byte only.
 * NOTE: this function requires special care to use as
 * - it returns before the byte has been transmitted, and if this is the last
 *   byte, you have to wait with spi1waitCompletion() before deasserting cs
 * - as the received byte is ignored, the overrun flag gets set and it must be
 *   cleared (spi1waitCompletion() does that as well)
 */
static void spi1sendOnly(unsigned char x)
{
    //NOTE: data is sent after the function returns, watch out!
    while((SPI1->SR & SPI_SR_TXE)==0) ;
    SPI1->DR=x;
}

/**
 * Must be called after using spi1sendOnly(), complete the last byte transmission
 */
static void spi1waitCompletion()
{
    while(SPI1->SR & SPI_SR_BSY) ;
    //Reading DR and then SR clears overrun flag
    [[gnu::unused]] volatile int unused;
    unused=SPI1->DR;
    unused=SPI1->SR;
}


/**
 * Send a command to the display
 * \param c command
 */
static void cmd(unsigned char c)
{
    dc::low();
    cs::low();
    spi1sendRecv(c);
    cs::high();
    delayUs(1);
}

/**
 * Send data to the display
 * \param d data
 */
static void data(unsigned char d)
{
    dc::high();
    cs::low();
    spi1sendRecv(d);
    cs::high();
    delayUs(1);
}

//
// class DisplaySer2p7s
//

namespace mxgui {

DisplaySer2p7s::DisplaySer2p7s() : DisplayGeneric4BPP(256,128)
{
    {
        FastInterruptDisableLock dLock;
        cs::mode(Mode::OUTPUT);      cs::high();
        sck::mode(Mode::ALTERNATE);  sck::alternateFunction(5);
        mosi::mode(Mode::ALTERNATE); mosi::alternateFunction(5);
        cs::mode(Mode::OUTPUT);      cs::high();
        dc::mode(Mode::OUTPUT);
        res::mode(Mode::OUTPUT);
        RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
        RCC_SYNC();
    }
    SPI1->CR1=SPI_CR1_SSM   //No HW cs
            | SPI_CR1_SSI
            | SPI_CR1_SPE   //SPI enabled
            | SPI_CR1_BR_0  //SPI clock 50/4=12.5 MHz (Fmax=20MHz)
            | SPI_CR1_MSTR; //Master mode

    res::high();
    Thread::nanoSleep(1000000);
    res::low();
    Thread::nanoSleep(100000);
    res::high();
    Thread::nanoSleep(100000);

    cmd(0xfd); data(0x12);             // Disable command lock
    cmd(0xae);                         // Display off
    cmd(0xb3); data(0x30);             // Oscillator 0x3 div 1 (~100Hz framerate)
    cmd(0xca); data(0x7f);             // Mux ratio 128
    cmd(0xa1); data(0x00);             // Display start line 0
    #if defined(MXGUI_ORIENTATION_HORIZONTAL)
    cmd(0xa2); data(0x20);             // Display offset 32
    cmd(0xa0); data(0x32); data(0x00); // Remap: COM split, flip COM/column
    #elif defined(MXGUI_ORIENTATION_HORIZONTAL_MIRRORED)
    cmd(0xa2); data(0x80);             // Display offset 160-32
    cmd(0xa0); data(0x20); data(0x00); // Remap: COM split
    #else
    #error "Unsupported orintation"
    #endif
    cmd(0xb4); data(0x32); data(0x0c); // ?
    cmd(0xc1); data(0x80);             // Contrast current 0x80
    cmd(0xba); data(0x03);             // Vp connected to capacitor
    cmd(0xad); data(0x90);             // Select internal Iref
    cmd(0xb1); data(0x74);             // Phase length
    cmd(0xbb); data(0x0c);             // Precharge voltage
    cmd(0xb6); data(0xc8);             // Second precharge period
    cmd(0xbe); data(0x04);             // VCOMH
    cmd(0xb9);                         // Linear gamma table
    cmd(0xa6);                         // Normal display mode
    clear(0);
    update();
    cmd(0xaf);                         // Display on
}

void DisplaySer2p7s::doTurnOn()
{
    cmd(0xaf);
}

void DisplaySer2p7s::doTurnOff()
{
    cmd(0xae);
}

void DisplaySer2p7s::doSetBrightness(int brt)
{
    const int maxSetting=0x80;
    const int minSetting=0x14;
    const int range=maxSetting-minSetting;
    int brightness=max(0,min(range,range*brt/100));
    cmd(0xc1); data(minSetting+brightness);
}

void DisplaySer2p7s::update()
{
    static const unsigned char xStart=0+8;
    static const unsigned char xEnd=63+8;
    static const unsigned char yStart=0;
    static const unsigned char yEnd=127;

    cmd(0x15); data(xStart); data(xEnd);
    cmd(0x75); data(yStart); data(yEnd);
    cmd(0x5c);

    dc::high();
    cs::low();
    for(int i=0;i<fbSize;i++) spi1sendOnly(backbuffer[i]); //TODO: DMA
    spi1waitCompletion();
    cs::high();
    delayUs(1);
}

} //namespace mxgui
