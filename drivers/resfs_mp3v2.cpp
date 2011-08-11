/***************************************************************************
 *   Copyright (C) 2011 by Terraneo Federico                               *
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

#include "resfs_mp3v2.h"
#include "miosix.h"
#include <cstdlib>

#ifdef _BOARD_MP3V2

namespace resfs {

/**
 * \param data byte to send
 * \return byte received
 */
static inline unsigned char spi2sendByte(unsigned char data=0)
{
    SPI2->DR=data;
    while((SPI2->SR & SPI_SR_RXNE)==0) ;
    return SPI2->DR;
}

/**
 * \param data word to send
 * \return word received
 */
static inline unsigned int spi2sendWord(unsigned int data=0)
{
    SPI2->DR=data>>24;
    while((SPI2->SR & SPI_SR_RXNE)==0) ;
    unsigned int result=SPI2->DR<<24;
    SPI2->DR=(data>>16) & 0xff;
    while((SPI2->SR & SPI_SR_RXNE)==0) ;
    result |= SPI2->DR<<16;
    SPI2->DR=(data>>8) & 0xff;
    while((SPI2->SR & SPI_SR_RXNE)==0) ;
    result |= SPI2->DR<<8;
    SPI2->DR=data & 0xff;
    while((SPI2->SR & SPI_SR_RXNE)==0) ;
    return result | SPI2->DR;
}

/**
 * Initialize the spi peripheral
 */
static inline void spi2init()
{
    RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
    //Master mode no hardware CS pin
    //Note: SPI2 is attached on the 36MHz APB2 bus, so the clock is set
    //to APB2/2=18MHz. This is the maximum achievable.
    SPI2->CR2=0;
    SPI2->CR1=SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_MSTR | SPI_CR1_SPE;
}

void backendInit()
{
    spi2init();
}

void backendRead(char *buf, int addr, int len)
{
    //The flash on the mp3v2 is an at45db041b
    div_t sector=div(addr,264);
    xflash::cs::low();
    spi2sendWord(0xe8000000 | sector.quot<<9 | sector.rem);
    spi2sendWord();
    while(len-->0) *buf++=spi2sendByte();
    xflash::cs::high();
}

} //namespace resfs

#endif //_BOARD_MP3V2
