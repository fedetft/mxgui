/***************************************************************************
 *   Copyright (C) 2010 by Terraneo Federico                               *
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

#include "backend_lpc2138spi.h"

#ifdef MXGUI_BACKEND_LPC2138SPI

namespace mxgui {

//typedef Gpio<GPIO0_BASE,4>  sck;      //SCK     = p0.4 (handled by hardware)
//typedef Gpio<GPIO0_BASE,5>  miso;     //MISO    = p0.5 (handled by hardware)
//typedef Gpio<GPIO0_BASE,6>  mosi;     //MOSI    = p0.6 (handled by hardware)
typedef Gpio<GPIO0_BASE,7>  cs;       //CS      = p0.7

void writeIdx(unsigned char reg)
{
    cs::low();
    //first 5 bit=01110, then 0=ID, 0=rs (command) 0=write
    spi0::tx_byte(0x70);//0111 0000
    spi0::tx_byte(0);
    spi0::tx_byte(reg);
    cs::high();
}

void writeRam(unsigned short data)
{
    cs::low();
    //first 5 bit=01110, then 0=ID, 1=rs (data) 0=write
    spi0::tx_byte(0x72);//0111 0010
    spi0::tx_byte(data>>8);
    spi0::tx_byte(data & 0xff);
    cs::high();
}

void hardwareInit()
{
    IOCLR0=0xffffffff;
    IOSET0=(1<<7) | (1<<29); //All @ 0 except p0.7 (cs) and p0.29 (reset)
    IODIR0=0xffffbfdf;//All out except p0.5 (sdo) and p0.14
    spi0::init();
}

} // namespace mxgui

#endif //MXGUI_BACKEND_LPC2138SPI
