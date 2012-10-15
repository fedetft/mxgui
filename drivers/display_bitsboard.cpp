/***************************************************************************
 *   Copyright (C) 2010, 2011 by Terraneo Federico                         *
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

#ifdef _BOARD_BITSBOARD

#include <cstring>
#include <miosix.h>
#include "mxgui/misc_inst.h"
#include "mxgui/line.h"
#include "display_bitsboard.h"

using namespace std;
using namespace miosix;

// Display
typedef Gpio<GPIOB_BASE,13> sck;    //Connected to clock of 4094 and 4040
typedef Gpio<GPIOB_BASE,15> mosi;   //Connected to data of 4094
typedef Gpio<GPIOB_BASE,7>  dres;   //Connected to reset of 4040
typedef Gpio<GPIOB_BASE,12> nflm;   //Negated of FLM signal to display
typedef Gpio<GPIOB_BASE,14> nm;     //Negated of M signal to display
typedef Gpio<GPIOB_BASE,8>  dispoff;//DISPOFF signal to display

static unsigned short *framebuffer;
static volatile char sequence=0; //Used for pulse generation

static void dmaRefill()
{
	DMA1_Stream4->CR=0;
	DMA1_Stream4->PAR=reinterpret_cast<unsigned int>(&SPI2->DR);
	DMA1_Stream4->M0AR=reinterpret_cast<unsigned int>(framebuffer);
	DMA1_Stream4->NDTR=2048;
	DMA1_Stream4->CR=DMA_SxCR_PL_1    | //High priority DMA stream
					 DMA_SxCR_MSIZE_0 | //Read  16bit at a time from RAM
					 DMA_SxCR_PSIZE_0 | //Write 16bit at a time to SPI
					 DMA_SxCR_MINC    | //Increment RAM pointer
					 DMA_SxCR_DIR_0   | //Memory to peripheral direction
					 DMA_SxCR_TEIE    | //Interrupt on error
					 DMA_SxCR_TCIE    | //Interrupt on completion
					 DMA_SxCR_EN;       //Start the DMA
}

void DMA1_Stream4_IRQHandler()
{
	DMA1->HIFCR=DMA_HIFCR_CTCIF4  |
                DMA_HIFCR_CTEIF4  |
                DMA_HIFCR_CDMEIF4 |
                DMA_HIFCR_CFEIF4;
	dmaRefill();

	//The following sequence is to generate a pulse of FLM that can "catch" an
	//LP pulse (www.webalice.it/fede.tft/spi_as_lcd_controller/waveform_lp_flm.png)
	//and since the frequency of LP depends on the SPI clock, these delays need
	//to be changed accordingly if the SPI clock changes.
	sequence=0;
	TIM7->ARR=100; //100us delay
	TIM7->CR1 |= TIM_CR1_CEN;
}

void TIM7_IRQHandler()
{
	TIM7->SR=0;
	switch(sequence)
	{
		case 0:
			sequence=1;
			TIM7->ARR=100; //100us delay
			TIM7->CR1 |= TIM_CR1_CEN;
			nflm::low();
			break;
		case 1:
			sequence=2;
			TIM7->ARR=100; //100us delay
			TIM7->CR1 |= TIM_CR1_CEN;
			if(nm::value()) nm::low(); else nm::high(); //Toggle M
			break;
		case 2:
			nflm::high();
			break;
	}
}

namespace mxgui {

//
// class DisplayImpl
//

DisplayImpl::DisplayImpl(): textColor(), font(miscFixed), last()
{
    setTextColor(Color(black),Color(white));
    {
        FastInterruptDisableLock dLock;
        RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
        RCC->APB1ENR |= RCC_APB1ENR_SPI2EN | RCC_APB1ENR_TIM7EN;
        sck::mode(Mode::ALTERNATE_OD);
        sck::alternateFunction(5);
        mosi::mode(Mode::ALTERNATE_OD);
        mosi::alternateFunction(5);
        dres::mode(Mode::OPEN_DRAIN);
        nflm::mode(Mode::OPEN_DRAIN);
        nm::mode(Mode::OPEN_DRAIN);
        dispoff::mode(Mode::OPEN_DRAIN);
    }

    nflm::high();
    nm::high();
    dispoff::high();

    dres::high();
    delayUs(10);
    dres::low();
    delayUs(10);

    framebuffer=new unsigned short[2048]; //256*128/16=2048
    unsigned int bba=(reinterpret_cast<unsigned>(framebuffer)-0x20000000)*32;
    framebufferBitBandAlias=reinterpret_cast<unsigned int*>(0x22000000+bba);
    memset(framebuffer,0,4096);

    TIM7->CR1=TIM_CR1_OPM;
    TIM7->DIER=TIM_DIER_UIE;
    TIM7->PSC=84-1; //84MHz/84=1MHz (1us resolution)
    TIM7->CNT=0;

    dmaRefill();
    NVIC_SetPriority(DMA1_Stream4_IRQn,2);//High priority for DMA
    NVIC_EnableIRQ(DMA1_Stream4_IRQn);
    NVIC_SetPriority(TIM7_IRQn,3);//High priority for TIM7
    NVIC_EnableIRQ(TIM7_IRQn);

    SPI2->CR2=SPI_CR2_TXDMAEN;
    SPI2->CR1=SPI_CR1_DFF      | //16bit mode
              SPI_CR1_SSM      | //SS pin not connected to SPI
              SPI_CR1_SSI      | //Internal SS signal pulled high
              SPI_CR1_LSBFIRST | //Send LSB first
              SPI_CR1_MSTR     | //Master mode
              SPI_CR1_SPE      | //SPI enabled, master mode
              SPI_CR1_BR_0     |
              SPI_CR1_BR_1;      //42MHz/16=2.625MHz (/4 by the 4094)=0.66MHz

    Thread::sleep(500);
    dispoff::low();
}

void DisplayImpl::write(Point p, const char *text)
{
    if(p.x()<0 || p.y()<0 || p.x()>=width || p.y()>=height) return;

    font.draw(*this,textColor,p,text);
}

void DisplayImpl::clippedWrite(Point p, Point a, Point b, const char *text)
{
    if(a.x()<0 || a.y()<0 || b.x()<0 || b.y()<0) return;
    if(a.x()>=width || a.y()>=height || b.x()>=width || b.y()>=height) return;

    font.clippedDraw(*this,textColor,p,a,b,text);
}

void DisplayImpl::clear(Color color)
{
    memset(framebuffer,color ? 0x00 : 0xff,256*128/8);
}

void DisplayImpl::clear(Point p1, Point p2, Color color)
{
    if(p1.x()<0 || p1.y()<0 || p2.x()<0 || p2.y()<0) return;
    if(p1.x()>=width || p1.y()>=height || p2.x()>=width || p2.y()>=height) return;

    //TODO: can be optimized further
    pixel_iterator it=begin(p1,p2,RD);
    while(it!=end()) *it=color;
}

void DisplayImpl::line(Point a, Point b, Color color)
{
    if(a.x()<0 || a.y()<0 || b.x()<0 || b.y()<0) return;
    if(a.x()>=width || a.y()>=height || b.x()>=width || b.y()>=height) return;
    
    //TODO: can be optimized for vertical or horizontal lines
    Line::draw(*this,a,b,color);
}

void DisplayImpl::scanLine(Point p, const Color *colors, unsigned short length)
{
    if(p.x()<0 || p.y()<0 || p.x()>=width || p.y()>=height) return;
    if(p.x()+length>width) return;
    pixel_iterator it=begin(p,Point(p.x()+length-1,p.y()),RD);
    for(int i=0;i<length;i++) *it=colors[i];
}

void DisplayImpl::drawImage(Point p, const ImageBase& img)
{
    short int xEnd=p.x()+img.getWidth()-1;
    short int yEnd=p.y()+img.getHeight()-1;

    if(xEnd >= width || yEnd >= height) return;

    //TODO: can be optimized if image and point are 8-bit aligned
    img.draw(*this,p);
}

void DisplayImpl::clippedDrawImage(Point p, Point a, Point b,
        const ImageBase& img)
{
    if(a.x()<0 || a.y()<0 || b.x()<0 || b.y()<0) return;
    if(a.x()>=width || a.y()>=height || b.x()>=width || b.y()>=height) return;

    //TODO: can be optimized if image and point are 8-bit aligned
    img.clippedDraw(*this,p,a,b);
}

void DisplayImpl::drawRectangle(Point a, Point b, Color c)
{
    line(a,Point(b.x(),a.y()),c);
    line(Point(b.x(),a.y()),b,c);
    line(b,Point(a.x(),b.y()),c);
    line(Point(a.x(),b.y()),a,c);
}

void DisplayImpl::turnOn()
{
    dispoff::low();
}

void DisplayImpl::turnOff()
{
    dispoff::high();
}

void DisplayImpl::setTextColor(Color fgcolor, Color bgcolor)
{
    Font::generatePalette(textColor,fgcolor,bgcolor);
}

void DisplayImpl::setFont(const Font& font)
{
    this->font=font;
}

DisplayImpl::pixel_iterator DisplayImpl::begin(Point p1, Point p2, IteratorDirection d)
{
    bool fail=false;
    if(p1.x()<0 || p1.y()<0 || p2.x()<0 || p2.y()<0) fail=true;
    if(p1.x()>=width || p1.y()>=height || p2.x()>=width || p2.y()>=height) fail=true;
    if(p2.x()<p1.x() || p2.y()<p1.y()) fail=true;
    if(fail)
    {
        //Failsafe values
        this->last=pixel_iterator(Point(1,0),Point(1,0),RD,this);
        return pixel_iterator(Point(0,0),Point(getWidth()-1,getHeight()-1),RD,this);
    }

    //Set the last iterator to a suitable one-past-the last value
    if(d==DR) this->last=pixel_iterator(Point(p2.x()+1,p1.y()),p2,d,this);
    else this->last=pixel_iterator(Point(p1.x(),p2.y()+1),p2,d,this);

    return pixel_iterator(p1,p2,d,this);
}

} //namespace mxgui

#endif //_BOARD_BITSBOARD
