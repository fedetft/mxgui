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
#include <algorithm>

using namespace std;

#ifdef _BOARD_MP3V2

namespace resfs {

/**
 * Used to swap bytes of an unsigned short, since for no apparent reason the
 * SPI peripheral when operated in 16 bit mode returns an unsigned short with
 * its bytes swapped.
 * \param x data to swap
 * \return data swapped
 */
static inline unsigned short swapBytes(unsigned short x)
{
    union {
        unsigned short a;
        unsigned char b[2];
    } result;
    result.a=x;
    swap(result.b[0],result.b[1]);
    return result.a;
}

/**
 * \param data byte to send
 * \return byte received
 * Note: the spi device must be in 8 bit mode
 */
static inline unsigned char spi2sendByte(unsigned char data=0)
{
    SPI2->DR=data;
    while((SPI2->SR & SPI_SR_RXNE)==0) ;
    return SPI2->DR;
}

/**
 * \param data halfword to send
 * \return halfword received
 * Note: the spi device must be in 16 bit mode
 */
static inline unsigned short spi2sendHalfword(unsigned short data=0)
{
    SPI2->DR=data;
    while((SPI2->SR & SPI_SR_RXNE)==0) ;
    return swapBytes(SPI2->DR);
}

/**
 * \param data word to send
 * \return word received
 * Note: the spi device must be in 16 bit mode
 */
static inline unsigned int spi2sendWord(unsigned int data=0)
{
    SPI2->DR=data>>16;
    while((SPI2->SR & SPI_SR_RXNE)==0) ;
    unsigned int result=swapBytes(SPI2->DR)<<16;
    SPI2->DR=data & 0xffff;
    while((SPI2->SR & SPI_SR_RXNE)==0) ;
    return result | swapBytes(SPI2->DR);
}

/**
 * Set the spi device to 8 bit mode
 */
static inline void mode8bit()
{
    SPI2->CR1=SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_MSTR | SPI_CR1_SPE;
}

/**
 * Set the spi device to 16 bit mode
 */
static inline void mode16bit()
{
    SPI2->CR1=SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_MSTR | SPI_CR1_DFF |
            SPI_CR1_SPE;
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
    mode16bit();
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
    if(len<=0) return;
    if(reinterpret_cast<int>(buf) & 1)
    {
        mode8bit();
        len--;
        *buf++=spi2sendByte();
        mode16bit();
    }
    //Now buf is surely halfword aligned
    short *aligned=reinterpret_cast<short*>(buf);
    int count=len/4;
    while(count-->0)
    {
        *aligned++=spi2sendHalfword();
        *aligned++=spi2sendHalfword();
    }
    int remaining=len & 3;
    if(remaining)
    {
        buf=reinterpret_cast<char*>(aligned);
        mode8bit();
        while(remaining-->0) *buf++=spi2sendByte();
        mode16bit();
    }
    xflash::cs::high();
}

} //namespace resfs

#endif //_BOARD_MP3V2
