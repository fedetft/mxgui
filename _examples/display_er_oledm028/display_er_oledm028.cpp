/***************************************************************************
 *   Copyright (C) 2018 by Terraneo Federico                               *
 *   Copyright (C) 2025 by Daniele Cattaneo                                *
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

#ifndef MXGUI_ORIENTATION_HORIZONTAL
#error Unsupported orientation
#endif

//Display connection
//Display pin 2 is Vbat (3.3..5V), while display pins 1,7,8,9,10,11,12,13 are GND
#if defined(_BOARD_STM32F4DISCOVERY)
using cs   = Gpio<PB, 7>; //Display pin 16
using sck  = Gpio<PB, 3>; //Display pin  4, used as HW SPI
using mosi = Gpio<PB, 5>; //Display pin  5, used as HW SPI
using dc   = Gpio<PB, 8>; //Display pin 14
using res  = Gpio<PB,15>; //Display pin 15
SPI_TypeDef * const spi=SPI3;
#elif defined(_BOARD_STM32F765II_MARCO_RAM_BOARD)
using cs   = Gpio<PA,4>; //Display pin 16 
using sck  = Gpio<PA,5>; //Display pin  4, used as HW SPI
using mosi = Gpio<PD,7>; //Display pin  5, used as HW SPI
using dc   = Gpio<PA,6>; //Display pin 14
using res  = Gpio<PD,6>; //Display pin 15
SPI_TypeDef * const spi=SPI1;
#else
#warning This SPI display driver is not supported on this board
#endif

/**
 * Send and receive a byte, thus returning only after transmission is complete
 * \param x byte to send
 * \return the received byte
 */
static unsigned char spi3sendRecv(unsigned char x=0)
{
    // Accessing the data register with 8 bit accesses to defeat packed mode,
    // an amazing feature where if you accidentally set the word size to 8 bit
    // by mistake, ST will correct that to 16 bit for you. (Only For STM32F7)
    // Me: Hey ST maybe if I wanted 16 bit words in my SPI I would have set the
    //     word size register to 16 bit, right?
    // ST: No no we're helping, see, it's not 16 bit words, it's two 8-bit words
    //     one after the other! There's a difference!
    // Me: *looks at the oscilloscope trace which is identical to a single 16
    //     bit transfer* <facepalm> Fuck off ST you're clearly drunk
    *((volatile unsigned char *)&(spi->DR))=x;
    while((spi->SR & SPI_SR_RXNE)==0) ;
    return *((volatile unsigned char *)&(spi->DR));
}

/**
 * Send a byte only.
 * NOTE: this function requires special care to use as
 * - it returns before the byte has been transmitted, and if this is the last
 *   byte, you have to wait with spi3waitCompletion() before deasserting cs
 * - as the received byte is ignored, the overrun flag gets set and it must be
 *   cleared (spi1waitCompletion() does that as well)
 */
static void spi3sendOnly(unsigned char x)
{
    //NOTE: data is sent after the function returns, watch out!
    while((spi->SR & SPI_SR_TXE)==0) ;
    *((volatile unsigned char *)&(spi->DR))=x;
}

/**
 * Must be called after using spi3sendOnly(), complete the last byte transmission
 */
static void spi3waitCompletion()
{
    while(spi->SR & SPI_SR_BSY) ;
    //clears overrun flags
    while(spi->SR & SPI_SR_RXNE) (void)*((volatile unsigned char *)&(spi->DR));
}

/**
 * Send a command to the display
 * \param c command
 */
static void cmd(unsigned char c)
{
    dc::low();
    cs::low();
    spi3sendRecv(c);
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
    spi3sendRecv(d);
    cs::high();
    delayUs(1);
}

//
// class DisplayErOledm028
//

namespace mxgui {

DisplayErOledm028::DisplayErOledm028() : DisplayGeneric4BPP(256,64)
{
    {
        GlobalIrqLock dLock;
        #if defined(_BOARD_STM32F4DISCOVERY)
        int af=6;
        #elif defined(_BOARD_STM32F765II_MARCO_RAM_BOARD)
        int af=5;
        #endif
        cs::mode(Mode::OUTPUT);      cs::high();
        sck::mode(Mode::ALTERNATE);  sck::alternateFunction(af);
        mosi::mode(Mode::ALTERNATE); mosi::alternateFunction(af);
        dc::mode(Mode::OUTPUT);
        res::mode(Mode::OUTPUT);
        
        #if defined(_BOARD_STM32F4DISCOVERY)
        RCC->APB1ENR |= RCC_APB1ENR_SPI3EN;
        #elif defined(_BOARD_STM32F765II_MARCO_RAM_BOARD)
        RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
        #endif
        RCC_SYNC();
    }

    #if defined(_BOARD_STM32F4DISCOVERY)
    spi->CR1=SPI_CR1_SSM  //No HW cs
            | SPI_CR1_SSI
            | SPI_CR1_SPE  //SPI enabled
            | SPI_CR1_BR_1 //SPI clock 42/8=5.25MHz (Fmax=10MHz)
            | SPI_CR1_MSTR;//Master mode
    #elif defined(_BOARD_STM32F765II_MARCO_RAM_BOARD)
    spi->CR1=SPI_CR1_SSM  //No HW cs
            | SPI_CR1_SSI
            | SPI_CR1_SPE  //SPI enabled
            | SPI_CR1_BR_1 //SPI clock 216/8=27MHz (works fine!)
            | SPI_CR1_MSTR;//Master mode
    spi->CR2=7<<SPI_CR2_DS_Pos // 8 bit per word (warning, this field is an illusion, see the rant above on packed mode)
            | SPI_CR2_FRXTH; // RXNE on 8 bit in fifo
    #endif

    res::high();
    Thread::sleep(1);
    res::low();
    delayUs(100);
    res::high();
    delayUs(100);

    cmd(0xfd); data(0x12);             // Disable command lock
    cmd(0xae);                         // Display off
    cmd(0xb3); data(0x91);             // Oscillator settings 0x9, divide by 2
    cmd(0xca); data(0x3f);             // Mux ratio 64
    cmd(0xa1); data(0x00);             // Display start line 0
    cmd(0xa2); data(0x00);             // Display offset 0
    cmd(0xa0); data(0x14); data(0x11); // Remap: dual com enabled, nibble remap
    cmd(0xab); data(0x01);             // Select internal VDD
    cmd(0xb4); data(0xa0); data(0xfd); // Display enhancement A
    cmd(0xc1); data(0x80);             // Contrast current 0x80
    cmd(0xc7); data(0x0f);             // Maximum brightness
    cmd(0xb1); data(0xe2);             // Phase length
    cmd(0xd1); data(0x82); data(0x20); // Display enhancement B
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
    for(int i=0;i<fbSize;i++) spi3sendOnly(backbuffer[i]); //TODO: DMA
    spi3waitCompletion();
    cs::high();
    delayUs(1);
}

} //namespace mxgui
