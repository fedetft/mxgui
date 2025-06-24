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

#pragma once

#include "display_generic_1bpp.h"
#include <util/software_i2c.h>
#include <algorithm>

namespace mxgui {

/**
 * A 1.5", 128x128 pixel monochrome inexpensive and not particularly
 * fast (I2C interface) OLED display that can be easily found on ebay.
 * Based on the SH1107 controller.
 * 
 * This driver uses software I2C
 * 
 * \tparam SDA I2C SDA Gpio connected to the display (requires 4k7 pullup)
 * \tparam SCL I2C SCL Gpio connected to the display (requires 4k7 pullup)
 * \tparam RESET RESET Gpio connected to the display
 */
template<typename SDA, typename SCL, typename RESET>
class DisplayGME128128 : public DisplayGeneric1BPP
{
public:
    /*
     * Constructor
     */
    DisplayGME128128();

    /**
     * Turn the display On after it has been turned Off.
     * Display initial state is On.
     */
    void doTurnOn() override { cmd(0xaf); }

    /**
     * Turn the display Off. It can be later turned back On.
     */
    void doTurnOff() override { cmd(0xae); }
    
    /**
     * Set display brightness. Depending on the underlying driver,
     * may do nothing.
     * \param brt from 0 to 100
     */
    void doSetBrightness(int brt) override
    {
        int brightness=std::max(0,std::min(255,5+brt*2+brt/2)); //5+brt*2.5
        cmd(0x81); cmd(brightness);
    }
    
    /**
     * Make all changes done to the display since the last call to update()
     * visible. Backends that require it may override this.
     */
    void update() override
    {
        cmd(0x00); cmd(0x10); // Set column address
        int i=0;
        for(int page=0;page<16;page++)
        {
            cmd(0xb0+page); // Set page address
            i2c::sendStart();
            if(i2c::send(ADDR)==0) { i2c::sendStop(); return; }
            i2c::send(GDATA);
            for(int j=0;j<128;j++) i2c::send(backbuffer[i++]);
            i2c::sendStop();
        }
    }
    
private:
    using i2c=miosix::SoftwareI2C<SDA,SCL,0,true>;

    static const unsigned char ADDR    = 0b01111000;
    static const unsigned char COMMAND = 0b10000000;
    static const unsigned char GDATA   = 0b01000000;

    static void cmd(unsigned char c);
};

template<typename SDA, typename SCL, typename RESET>
DisplayGME128128<SDA,SCL,RESET>::DisplayGME128128() : DisplayGeneric1BPP(128,128)
{
    using namespace miosix;

    RESET::mode(Mode::OUTPUT);
    i2c::init();

    RESET::high();
    Thread::sleep(1);
    RESET::low();
    delayUs(100);
    RESET::high();
    delayUs(100);
    
    cmd(0xae);                         // Display off
    cmd(0xd5); cmd(0x80);              // Oscillator settings 0x8, divide by 1
    cmd(0xa8); cmd(height-1);          // Mux ratio
    cmd(0xd3); cmd(0x00);              // Display offset 0
    cmd(0x40);                         // Display start line 0
    cmd(0x20);                         // Memory addressing horizontal
    #if defined(MXGUI_ORIENTATION_VERTICAL)
    cmd(0xa1);                         // Remap: col127=seg0
    cmd(0xc8);                         // COM scan direction decreasing
    #elif defined(MXGUI_ORIENTATION_HORIZONTAL)
    cmd(0xa1);                         // Remap: col127=seg0
    cmd(0xc0);                         // COM scan direction increasing
    #elif defined(MXGUI_ORIENTATION_VERTICAL_MIRRORED)
    cmd(0xa0);                         // No remap
    cmd(0xc0);                         // COM scan direction increasing
    #else // defined(MXGUI_ORIENTATION_HORIZONTAL_MIRRORED)
    cmd(0xa0);                         // No remap
    cmd(0xc8);                         // COM scan direction decreasing
    #endif
    cmd(0x81); cmd(0x80);              // Default brightness
    cmd(0xd9); cmd(0xf1);              // Precharge periods
    cmd(0xdb); cmd(0x40);              // VCOMH
    cmd(0xa6);                         // Normal display mode
    clear(0);
    update();
    cmd(0xaf);                         // Display on
    setTextColor(std::make_pair(Color(0xf),Color(0x0)));
}

template<typename SDA, typename SCL, typename RESET>
void DisplayGME128128<SDA,SCL,RESET>::cmd(unsigned char c)
{
    i2c::sendStart();
    if(i2c::send(ADDR)==0) { i2c::sendStop(); return; }
    i2c::send(COMMAND);
    i2c::send(c);
    i2c::sendStop();
}

} //namespace mxgui
