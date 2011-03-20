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

#ifndef BACKEND_LPC2138SPI_H
#define	BACKEND_LPC2138SPI_H

#include "mxgui/mxgui_settings.h"

#ifdef MXGUI_BACKEND_LPC2138SPI

#include "libraries/gpio.h"

namespace mxgui {

typedef Gpio<GPIO0_BASE,29> reset;    //RESET   = p0.29
typedef Gpio<GPIO0_BASE,30> ncp5810;  //NCP5810 = p0.30

class spi0
{
public:
    /**
    Initializes SPI0 interface. Assumes that PCLK is 14.7456MHz
    Spi clock speed is set to 1.8432MHz
    */
    static void init()
    {
        PCONP|=(1<<8);//Enable SPI0 peripheral
        PINSEL0|=0x1500;//SPI pin connections
        S0SPCCR=8;//clock=14745600/8=1843200 (maximum possible)
        S0SPCR=0x20;//Master, interrupt disabled
    }

    /**
    Send a byte through SPI0
    \param x byte to transfer
    \return the received value through spi
    */
    static unsigned char tx_byte(unsigned char x)
    {
        S0SPDR=x;
        while(!(S0SPSR & 0x80)) ; //wait
        return S0SPDR;
    }

    /**
    Receive a byte through SPI0
    \return byte from SPI
    */
    static unsigned char rx_byte()
    {
        S0SPDR=0;//Write zero
        while(!(S0SPSR & 0x80)) ; //wait
        return S0SPDR;
    }
};

/**
 * Set the index register
 * \param reg register to select
 */
void writeIdx(unsigned char reg);

/**
 * Write data to selected register
 * \param data data to write
 */
void writeRam(unsigned short data);

/**
 * Write a value to a register of the display
 * \param reg register number
 * \param data data to write
 */
inline void writeReg(unsigned char reg, unsigned short data)
{
    writeIdx(reg);
    writeRam(data);
}

/**
 * Initializes the hardware backend
 */
void hardwareInit();

} // namespace mxgui

#endif //MXGUI_BACKEND_LPC2138SPI

#endif	/* BACKEND_LPC2138SPI_H */
