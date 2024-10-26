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

#include "display_depg0290b01.h"
#include "mxgui/misc_inst.h"
#include <miosix.h>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <array>

using namespace std;
using namespace miosix;

#ifndef _BOARD_STM32F411CE_BLACKPILL
#warning "The SPI driver has only been tested on an STM32F411CE_BLACKPILL"
#endif

//Display connection

using mosi = Gpio<GPIOB_BASE,3>; //HW SPI1
using sck  = Gpio<GPIOB_BASE,5>; //HW SPI1
using cs   = Gpio<GPIOB_BASE,6>;
using dc   = Gpio<GPIOB_BASE,7>;
using busy = Gpio<GPIOB_BASE,8>;
using res  = Gpio<GPIOB_BASE,9>;

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

static void sendCmd(unsigned char cmd, int len=0, ...)
{
    dc::low();
    cs::low();
    spi1sendRecv(cmd);
    dc::high();
    va_list arg;
    va_start(arg,len);
    for(int i=0;i<len;i++) spi1sendRecv(va_arg(arg,int));
    va_end(arg);
    cs::high();
    delayUs(1);
}

static void sendArray(unsigned char cmd, int len=0, const void *data=nullptr)
{
    SPI1->CR1 |= SPI_CR1_LSBFIRST; //Quirk: framebuffer layout is bitswapped, WTF?
    auto ptr=reinterpret_cast<const unsigned char *>(data);
    dc::low();
    cs::low();
    spi1sendRecv(cmd);
    dc::high();
    for(int i=0;i<len;i++) spi1sendOnly(ptr[i]);
    spi1waitCompletion();
    SPI1->CR1 &= ~SPI_CR1_LSBFIRST; //Undo quirk
    cs::high();
    delayUs(1);
}

static void waitBusy(bool longWait=false)
{
    // auto b=getTime();
    while(busy::value()==1) if(longWait) Thread::sleep(20);
    // auto e=getTime();
    // iprintf("busy wait %lldns\n",e-b);
}

//
// class DisplayErOledm024
//

namespace mxgui {

DisplayDepg0290b01::DisplayDepg0290b01() : DisplayGeneric1BPP(296,128)
{
    {
        FastInterruptDisableLock dLock;
        mosi::mode(Mode::ALTERNATE); mosi::alternateFunction(5);
        sck::mode(Mode::ALTERNATE);  sck::alternateFunction(5);
        cs::mode(Mode::OUTPUT);      cs::high();
        dc::mode(Mode::OUTPUT);
        busy::mode(Mode::INPUT);
        res::mode(Mode::OUTPUT);     res::low();
        RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
        RCC_SYNC();
    }
    SPI1->CR1=SPI_CR1_SSM   //No HW cs
            | SPI_CR1_SSI
            | SPI_CR1_SPE   //SPI enabled
            | SPI_CR1_BR_0  //SPI clock 50/4=12.5 MHz (Fmax=20MHz)
            | SPI_CR1_MSTR; //Master mode

    setTextColor({black,white});
}

void DisplayDepg0290b01::doTurnOn() {}

void DisplayDepg0290b01::doTurnOff() {}

void DisplayDepg0290b01::doSetBrightness(int brt) {}

void DisplayDepg0290b01::update()
{
    res::low();
    Thread::nanoSleep(200000);
    res::high();
    Thread::nanoSleep(200000);
    waitBusy();                          //busy wait ~1us
    sendCmd(0x12);                       //Software reset
    waitBusy();                          //busy wait ~1.5ms
    sendCmd(0x74,1,0x54);                //Set Analog Block Control
    sendCmd(0x7e,1,0x3b);                //Set Digital Block Control
    sendCmd(0x01,3,0x27,0x01,0x00);      //Set display size and driver output control
    sendCmd(0x11,1,0x05);                //Ram data entry mode
    sendCmd(0x44,2,0x00,0x0f);           //Set Ram X address
    sendCmd(0x45,4,0x27,0x01,0x00,0x00); //Set Ram Y address
    sendCmd(0x3c,1,0x01);                //Set border
    sendCmd(0x2c,1,0x26);                //Set VCOM value
    sendCmd(0x03,1,0x17);                //Gate voltage setting
    sendCmd(0x04,3,0x41,0x00,0x32);      //Source voltage setting
    sendCmd(0x3a,1,0x30);                //Frame setting 50hz
    sendCmd(0x3b,1,0x0a);                //???
    sendCmd(0x32,70,                     //LUT (https://badge.team/docs/badges/sha2017)
    0xA0,0x90,0x50,0x0,0x0,0x0,0x0,
    0x50,0x90,0xA0,0x0,0x0,0x0,0x0,
    0xA0,0x90,0x50,0x0,0x0,0x0,0x0,
    0x50,0x90,0xA0,0x0,0x0,0x0,0x0,
    0x00,0x00,0x00,0x0,0x0,0x0,0x0,
    0xF,0xF,0x0,0x0,0x0,
    0xF,0xF,0x0,0x0,0x2,
    0xF,0xF,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0
    );
    sendCmd(0x4e,1,0x00);                //Set Ram X address counter
    sendCmd(0x4f,2,0x27,0x01);           //Set Ram Y address counter
    // auto b=getTime();
    sendArray(0x24,fbSize,backbuffer);   //Load image (3056400ns)
    // auto e=getTime();
    // iprintf("Sending framebuffer took %lldns\n",e-b);
    sendCmd(0x22,1,0xc7);                //Image update
    sendCmd(0x20);                       //???
    waitBusy(true);                      //busy wait ~3.2s
    sendCmd(0x10,1,0x01);                //Enter deep sleep mode
}

} //namespace mxgui
