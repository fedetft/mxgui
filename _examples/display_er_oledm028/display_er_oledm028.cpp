/***************************************************************************
 *   Copyright (C) 2018 by Terraneo Federico                               *
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

#include "display_er_oledm028.h"
#include "miosix.h"
#include <algorithm>

using namespace std;
using namespace miosix;

#ifndef _BOARD_STM32F4DISCOVERY
#warning "The SPI driver has only been tested on an STM32F4DISCOVERY"
#endif

//Display connection

typedef Gpio<GPIOB_BASE,3> sck;
typedef Gpio<GPIOB_BASE,4> miso; //Not used
typedef Gpio<GPIOB_BASE,5> mosi;
typedef Gpio<GPIOB_BASE,7> cs;

typedef Gpio<GPIOB_BASE,8> dc;
typedef Gpio<GPIOB_BASE,15> reset;

static void spiInit()
{
    sck::mode(Mode::ALTERNATE);
    sck::alternateFunction(6);
    mosi::mode(Mode::ALTERNATE);
    mosi::alternateFunction(6);
    cs::mode(Mode::OUTPUT);
    cs::high();

    {
        FastInterruptDisableLock dLock;
        RCC->APB1ENR |= RCC_APB1ENR_SPI3EN;
        RCC_SYNC();
    }
    //Master mode no hardware CS pin
    //Note: SPI3 is attached on the 42MHz APB2 bus, so the clock is set
    //to APB2/2/2=10.5MHz. This overclocking the SSD1332 by 500KHz
    SPI3->CR1=SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_MSTR | SPI_CR1_BR_0;
    SPI3->CR2=0;
    SPI3->CR1 |= SPI_CR1_SPE; //Set enable bit
}

static unsigned char spiSendRecv(unsigned char data)
{
    SPI3->DR=data;
    while((SPI3->SR & SPI_SR_RXNE)==0) ;
    return SPI3->DR;
}

static void cmd(unsigned char c)
{
    dc::low();

    cs::low();
    delayUs(1);
    spiSendRecv(c);
    cs::high();
    delayUs(1);
}

static void data(unsigned char d)
{
    dc::high();

    cs::low();
    delayUs(1);
    spiSendRecv(d);
    cs::high();
    delayUs(1);
}

//
// class DisplayErOledm028
//

namespace mxgui {

DisplayErOledm028::DisplayErOledm028() : DisplayGeneric4BPP(256,64)
{
    spiInit();
    dc::mode(Mode::OUTPUT);
    reset::mode(Mode::OUTPUT);

    reset::high();
    Thread::sleep(1);
    reset::low();
    delayUs(100);
    reset::high();
    delayUs(100);

    cmd(0xfd); data(0x12);             // Disable command lock
    cmd(0xae);                         // Display off
    cmd(0xb3); data(0x91);             // Oscillator settings 0x9, divide by 2
    cmd(0xca); data(0x3f);             // Mux ratio 64
    cmd(0xa2); data(0x00);             // Display offset 0
    cmd(0xa1); data(0x00);             // Display start line 0
    cmd(0xa0); data(0x14); data(0x11); // Remap: dual com enabled, nibble remap
    cmd(0xab); data(0x01);             // Select internal VDD
    cmd(0xb4); data(0xa0); data(0xfd); // ?
    cmd(0xc1); data(0x80);             // Contrast current 0x80
    cmd(0xc7); data(0x0f);             // Maximum brightness
    cmd(0xb1); data(0xe2);             // Phase length
    cmd(0xd1); data(0x82); data(0x20); // ?
    cmd(0xbb); data(0x1f);             // Precharge voltage
    cmd(0xb6); data(0x08);             // Second precharge period
    cmd(0xbe); data(0x07);             // VCOMH
    cmd(0xa6);                         // Normal display mode
    clear(0);
    update();
    cmd(0xaf);                         // Display on
}

void DisplayErOledm028::doTurnOn()
{
    cmd(0xaf);
}

void DisplayErOledm028::doTurnOff()
{
    cmd(0xae);
}

void DisplayErOledm028::doSetBrightness(int brt)
{
    int brightness=max(0,min(15,brt/6));
    cmd(0xc7); data(brightness);
}

void DisplayErOledm028::update()
{
    static const unsigned char xStart=28;
    static const unsigned char xEnd=91;
    static const unsigned char yStart=0;
    static const unsigned char yEnd=63;
    
    cmd(0x15); data(xStart); data(xEnd);
    cmd(0x75); data(yStart); data(yEnd);
    cmd(0x5c);

    dc::high();
    cs::low();
    delayUs(1);
    for(int i=0;i<fbSize;i++) spiSendRecv(backbuffer[i]);
    cs::high();
    delayUs(1);
}

} //namespace mxgui
