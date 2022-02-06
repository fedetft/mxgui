/***************************************************************************
 *   Copyright (C) 2021 by Terraneo Federico                               *
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

#include "display_er_oledm024.h"
#include <miosix.h>
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

//
// class DisplayErOledm024
//

namespace mxgui {

DisplayErOledm024::DisplayErOledm024() : DisplayGeneric1BPP(128,64)
{
    {
        FastInterruptDisableLock dLock;
        cs::mode(Mode::OUTPUT);      cs::high();
        sck::mode(Mode::ALTERNATE);  sck::alternateFunction(5);
        mosi::mode(Mode::ALTERNATE); mosi::alternateFunction(5);
        dc::mode(Mode::OUTPUT);
        res::mode(Mode::OUTPUT);

        RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
        RCC_SYNC();
    }

    SPI1->CR1=SPI_CR1_SSM  //No HW cs
            | SPI_CR1_SSI
            | SPI_CR1_SPE  //SPI enabled
            | SPI_CR1_BR_1 //SPI clock 50/8=6.25 MHz (Fmax=10MHz)
            | SPI_CR1_MSTR;//Master mode

    res::high();
    Thread::sleep(1);
    res::low();
    delayUs(100);
    res::high();
    delayUs(100);

    //VCC=13.4V VCOMH=10.4V IREF~11uA SPI_fmax=10MHz
    /*
     * Power save mode reduces frame rate and precharge time to reduce current
     * consumption from the VCC side. Power consumption from the VDD side is
     * < 100uA and thus negligible.
     * Measured current [mA] as a function of #pixel active
     * pixel_on   default_current  power_save_current
     *        0       0.70           0.49 (off current)
     *      128       1.23           0.85
     *      ...
     *   15*128       6.88           4.98
     * 
     * Further tried to reduce scanlines to 16 with
     * cmd(0xd5); cmd(0x03);     // Oscillator 0x0, /4 (75Hz)
     * cmd(0xa8); cmd(15);       // Number of rows 16
     * and got off current down to 0.15mA but per pixel current significantly
     * increased and bug in cmd(0x81) set brightness did not allow to correct
     */
    #define POWER_SAVE //Active by default

    #ifndef POWER_SAVE
    const unsigned char oscCfg=0xa0; // 120Hz refresh rate
    const unsigned char prcgCfg=0x25;// Precharge phase2=2 phase1=5
    #else //POWER_SAVE
    const unsigned char oscCfg=0x00; // 78Hz -30% off current 0.7 -> 0.48mA
    const unsigned char prcgCfg=0x13;// 1,3  -22% on  current 3.1 -> 2.4uA/pix
    #endif //POWER_SAVE
    
    cmd(0xfd); cmd(0x12);     // Disable command lock
    cmd(0xae);                // Display OFF
    cmd(0xd5); cmd(oscCfg);   // Oscillator
    cmd(0xa8); cmd(height-1); // Number of rows 64
    cmd(0xd3); cmd(0x00);     // Display offset 0
    cmd(0x40);                // Display start line 0
    cmd(0x20); cmd(0x00);     // Memory addrressing horizontal
    cmd(0xa1);
    cmd(0xc8);                // Scan direction 64 to 1
    cmd(0xda); cmd(0x12);     // Alternate COM, no COM left/right
    cmd(0x81); cmd(0x32);     // Brightness 1.1uA/pixel
    cmd(0xd9); cmd(prcgCfg);  // Precharge phase2=2 phase1=5
    cmd(0xdb); cmd(0x34);     // VCOMH=.78*VCC 
    cmd(0xa6);                // Normal display mode
    cmd(0xa4);                // Disable test mode
    clear(0);
    update();
    cmd(0xaf);                // Display ON
    setTextColor(std::make_pair(Color(0xf),Color(0x0)));
}

void DisplayErOledm024::doTurnOn()
{
    cmd(0xaf);
}

void DisplayErOledm024::doTurnOff()
{
    cmd(0xae);
}

void DisplayErOledm024::doSetBrightness(int brt)
{
    int brightness=max(0,min(50,brt/2));
    cmd(0x81); cmd(brightness);
}

void DisplayErOledm024::update()
{
    cmd(0x21); cmd(0); cmd(127);
    cmd(0x22); cmd(0); cmd(7);
    dc::high();
    cs::low();
    for(int i=0;i<fbSize;i++) spi1sendOnly(backbuffer[i]);
    spi1waitCompletion();
    cs::high();
    delayUs(1);
}

} //namespace mxgui
