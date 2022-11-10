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

#include "display_er_oledm015.h"
#include <miosix.h>
#include <kernel/scheduler/scheduler.h>
#include <interfaces/endianness.h>
#include <algorithm>
#include <line.h>

using namespace std;
using namespace miosix;
using namespace mxgui;

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
 * DMA TX end of transfer
 * NOTE: conflicts with SDIO driver but this board does not have an SD card
 */
void __attribute__((naked)) DMA2_Stream3_IRQHandler()
{
    saveContext();
    asm volatile("bl _Z20SPI1txDmaHandlerImplv");
    restoreContext();
}

static Thread *waiting=nullptr;
static bool error;

void __attribute__((used)) SPI1txDmaHandlerImpl()
{
    if(DMA2->LISR & (DMA_LISR_TEIF3 | DMA_LISR_DMEIF3 | DMA_LIFCR_CFEIF3))
        error=true;
    DMA2->LIFCR=DMA_LIFCR_CTCIF3
              | DMA_LIFCR_CTEIF3
              | DMA_LIFCR_CDMEIF3
              | DMA_LIFCR_CFEIF3;
    waiting->IRQwakeup();
    if(waiting->IRQgetPriority()>Thread::IRQgetCurrentThread()->IRQgetPriority())
        Scheduler::IRQfindNextThread();
    waiting=nullptr;
}

static void spi1SendDMA(const Color *data, int size)
{
    error=false;
    unsigned short tempCr1=SPI1->CR1;
    SPI1->CR1=0;
    SPI1->CR2=SPI_CR2_TXDMAEN;
    SPI1->CR1=tempCr1;
    
    waiting=Thread::getCurrentThread();
    NVIC_ClearPendingIRQ(DMA2_Stream3_IRQn);
    NVIC_SetPriority(DMA2_Stream3_IRQn,10);//Low priority for DMA
    NVIC_EnableIRQ(DMA2_Stream3_IRQn);

    DMA2_Stream3->CR=0;
    DMA2_Stream3->PAR=reinterpret_cast<unsigned int>(&SPI1->DR);
    DMA2_Stream3->M0AR=reinterpret_cast<unsigned int>(data);
    DMA2_Stream3->NDTR=2*size; //Size is at the peripheral side (8bit)
    DMA2_Stream3->FCR=DMA_SxFCR_FEIE
                    | DMA_SxFCR_DMDIS;
    DMA2_Stream3->CR=DMA_SxCR_CHSEL_0 //Channel 3 SPI1
                   | DMA_SxCR_CHSEL_1
                   //| DMA_SxCR_MSIZE_0 //Memory size 16 bit
                   | DMA_SxCR_MINC    //Increment memory pointer
                   | DMA_SxCR_DIR_0   //Memory to peripheral
                   | DMA_SxCR_TCIE    //Interrupt on transfer complete
                   | DMA_SxCR_TEIE    //Interrupt on transfer error
                   | DMA_SxCR_DMEIE   //Interrupt on direct mode error
                   | DMA_SxCR_EN;     //Start DMA
    
    {
        FastInterruptDisableLock dLock;
        while(waiting!=nullptr)
        {
            waiting->IRQwait();
            {
                FastInterruptEnableLock eLock(dLock);
                Thread::yield();
            }
        }
    }
                
    NVIC_DisableIRQ(DMA2_Stream3_IRQn);
    spi1waitCompletion();
    SPI1->CR1=0;
    SPI1->CR2=0;
    SPI1->CR1=tempCr1;
    //if(error) puts("SPI1 DMA tx failed"); //TODO: look into why this fails
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

/**
 * Send data to the display
 * \param d data
 */
static void dat(unsigned char d)
{
    dc::high();
    cs::low();
    spi1sendRecv(d);
    cs::high();
    delayUs(1);
}

static const int width=128, height=128; ///< Display size

/**
 * Set cursor to desired location
 * \param point where to set cursor (0<=x<128, 0<=y<128)
 */
static void setCursor(Point p)
{
    #ifdef MXGUI_ORIENTATION_VERTICAL
    cmd(0x15); dat(p.x()); dat(0x7f); // Set column address
    cmd(0x75); dat(p.y()); dat(0x7f); // Set row address
    #else //MXGUI_ORIENTATION_HORIZONTAL
    cmd(0x15); dat(p.y()); dat(0x7f); // Set column address
    cmd(0x75); dat(p.x()); dat(0x7f); // Set row address
    #endif //Hardware doesn't seem to support mirroring
}

/**
 * Set a hardware window on the screen, optimized for writing text.
 * The GRAM increment will be set to up-to-down first, then left-to-right which
 * is the correct increment to draw fonts
 * \param p1 upper left corner of the window
 * \param p2 lower right corner of the window
 */
static void textWindow(Point p1, Point p2)
{
    #ifdef MXGUI_ORIENTATION_VERTICAL
    cmd(0x15); dat(p1.x()); dat(p2.x()); // Set column address
    cmd(0x75); dat(p1.y()); dat(p2.y()); // Set row address
    cmd(0xa0); dat(0x67);
    #else //MXGUI_ORIENTATION_HORIZONTAL
    cmd(0x15); dat(p1.y()); dat(p2.y()); // Set column address
    cmd(0x75); dat(p1.x()); dat(p2.x()); // Set row address
    cmd(0xa0); dat(0x64);
    #endif //Hardware doesn't seem to support mirroring
}

/**
 * Set a hardware window on the screen, optimized for drawing images.
 * The GRAM increment will be set to left-to-right first, then up-to-down which
 * is the correct increment to draw images
 * \param p1 upper left corner of the window
 * \param p2 lower right corner of the window
 */
static inline void imageWindow(Point p1, Point p2)
{
    #ifdef MXGUI_ORIENTATION_VERTICAL
    cmd(0x15); dat(p1.x()); dat(p2.x()); // Set column address
    cmd(0x75); dat(p1.y()); dat(p2.y()); // Set row address
    cmd(0xa0); dat(0x66);
    #else //MXGUI_ORIENTATION_HORIZONTAL
    cmd(0x15); dat(p1.y()); dat(p2.y()); // Set column address
    cmd(0x75); dat(p1.x()); dat(p2.x()); // Set row address
    cmd(0xa0); dat(0x65);
    #endif //Hardware doesn't seem to support mirroring
}

//
// class DisplayErOledm015
//

namespace mxgui {

DisplayErOledm015::DisplayErOledm015() : buffer(nullptr), buffer2(nullptr)
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
            | SPI_CR1_BR_0 //SPI clock 50/4=12.5 MHz (Fmax=20MHz)
            | SPI_CR1_MSTR;//Master mode

    res::high();
    Thread::sleep(1);
    res::low();
    delayUs(100);
    res::high();
    delayUs(100);
    
    static const unsigned char grayTable[]=
    {
          0,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16,
         17, 18, 19, 21, 23, 25, 27, 29, 31, 33, 35, 37, 39, 42, 45, 48,
         51, 54, 57, 60, 63, 66, 69, 72, 76, 80, 84, 88, 92, 96,100,104,
        108,112,116,120,125,130,135,140,145,150,155,160,165,170,175,180
    };
    
    cmd(0xfd); dat(0x12);                       // Disable command lock
    cmd(0xfd); dat(0xb1);                       // Enable all commands
    cmd(0xae);                                  // Display OFF
    cmd(0xa1); dat(0x00);                       // Set display start line
    cmd(0xa2); dat(0x00);                       // Set display offset
    cmd(0xa6);                                  // Normal display mode
    cmd(0xab); dat(0x01);                       // 8bit iface, 2.5V regulator ON
    cmd(0xb1); dat(0x32);                       // Precharge phase2=3 phase1=5
    cmd(0xb3); dat(0xf1);                       // Oscillator 0xf divide by 2
    cmd(0xb4); dat(0xa0); dat(0xb5); dat(0x55); // External VSL
    cmd(0xb6); dat(0x01);                       // Second precharge 1 DCLKS
    cmd(0xb8);                                  // Set gray table
    for(unsigned int i=0;i<sizeof(grayTable);i++) dat(grayTable[i]);
    cmd(0xbb); dat(0x17);                       // Precharge voltage ~0.5VCC
    cmd(0xbe); dat(0x05);                       // VCOMH
    cmd(0xc1); dat(0x88); dat(0x70); dat(0x88); // A B C brightness
    cmd(0xc7); dat(0x0f);                       // Master brightness
    cmd(0xca); dat(0x7f);                       // Duty 1:128
    clear(0);
    cmd(0xaf);                                  // Display ON

    setTextColor(make_pair(Color(0xffff),Color(0x0)));
}

void DisplayErOledm015::doTurnOn()
{
    cmd(0xaf);
}

void DisplayErOledm015::doTurnOff()
{
    cmd(0xae);
}

void DisplayErOledm015::doSetBrightness(int brt)
{
    cmd(0xc7); dat(max(0,min(15,brt/6)));
}

pair<short int, short int> DisplayErOledm015::doGetSize() const
{
    return make_pair(height,width);
}

void DisplayErOledm015::write(Point p, const char *text)
{
    font.draw(*this,textColor,p,text);
}

void DisplayErOledm015::clippedWrite(Point p, Point a, Point b, const char *text)
{
    font.clippedDraw(*this,textColor,p,a,b,text);
}

void DisplayErOledm015::clear(Color color)
{
    clear(Point(0,0),Point(width-1,height-1),color);
}

void DisplayErOledm015::clear(Point p1, Point p2, Color color)
{
    imageWindow(p1,p2);
    doBeginPixelWrite();
    int numPixels=(p2.x()-p1.x()+1)*(p2.y()-p1.y()+1);
    for(int i=0;i<numPixels;i++) doWritePixel(color);
    doEndPixelWrite();
}

void DisplayErOledm015::beginPixel() {}

void DisplayErOledm015::setPixel(Point p, Color color)
{
    //Can't move boilerplate to beginPixel, as can't do setCursor in between
    setCursor(p);
    doBeginPixelWrite();
    doWritePixel(color);
    doEndPixelWrite();
}

void DisplayErOledm015::line(Point a, Point b, Color color)
{
    //Horizontal line speed optimization
    if(a.y()==b.y())
    {
        imageWindow(Point(min(a.x(),b.x()),a.y()),
                    Point(max(a.x(),b.x()),a.y()));
        doBeginPixelWrite();
        int numPixels=abs(a.x()-b.x());
        for(int i=0;i<=numPixels;i++) doWritePixel(color);
        doEndPixelWrite();
        return;
    }
    //Vertical line speed optimization
    if(a.x()==b.x())
    {
        textWindow(Point(a.x(),min(a.y(),b.y())),
                    Point(a.x(),max(a.y(),b.y())));
        doBeginPixelWrite();
        int numPixels=abs(a.y()-b.y());
        for(int i=0;i<=numPixels;i++) doWritePixel(color);
        doEndPixelWrite();
        return;
    }
    //General case
    Line::draw(*this,a,b,color);
}

void DisplayErOledm015::scanLine(Point p, const Color *colors, unsigned short length)
{
    if(buffer2==nullptr) buffer2=new Color[buffer2Size];
    length=min<unsigned short>(length,width-p.x());
    imageWindow(p,Point(length-1,p.y()));
    cmd(0x5c);
    dc::high();
    cs::low();
    for(int i=0;i<length;i++) buffer2[i]=toBigEndian16(colors[i]);
    spi1SendDMA(buffer2,length);
    cs::high();
    delayUs(1);
}

Color *DisplayErOledm015::getScanLineBuffer()
{
    //getWidth() would be enough as size, but we reuse the buffer for DMA
    if(buffer==nullptr) buffer=new Color[getWidth()];
    return buffer;
}

void DisplayErOledm015::scanLineBuffer(Point p, unsigned short length)
{
    scanLine(p,buffer,length);
}

void DisplayErOledm015::drawImage(Point p, const ImageBase& img)
{
    const Color *imgData=img.getData();
    if(imgData!=0)
    {
        if(buffer2==nullptr) buffer2=new Color[buffer2Size];
        short int xEnd=p.x()+img.getWidth()-1;
        short int yEnd=p.y()+img.getHeight()-1;
        imageWindow(p,Point(xEnd,yEnd));
        cmd(0x5c);
        dc::high();
        cs::low();
        //Unfortunately the DMA requires the endianness to be swapped, the
        //pointer we get is read-only (can be in flash), and we may not have
        //enough memory to allocate a large enough buffer to hold the entire
        //image, so we'll have to split it in chunks
        int imgSize=img.getHeight()*img.getWidth();
        while(imgSize>0)
        {
            int chunkSize=min(imgSize,buffer2Size);
            for(int i=0;i<chunkSize;i++) buffer2[i]=toBigEndian16(imgData[i]);
            spi1SendDMA(buffer2,chunkSize);
            imgSize-=chunkSize;
            imgData+=chunkSize;
        }
        cs::high();
        delayUs(1);
    } else img.draw(*this,p);
}

void DisplayErOledm015::clippedDrawImage(Point p, Point a, Point b, const ImageBase& img)
{
    img.clippedDraw(*this,p,a,b);
}

void DisplayErOledm015::drawRectangle(Point a, Point b, Color c)
{
    line(a,Point(b.x(),a.y()),c);
    line(Point(b.x(),a.y()),b,c);
    line(b,Point(a.x(),b.y()),c);
    line(Point(a.x(),b.y()),a,c);
}

DisplayErOledm015::pixel_iterator DisplayErOledm015::begin(Point p1, Point p2,
        IteratorDirection d)
{
    if(p1.x()<0 || p1.y()<0 || p2.x()<0 || p2.y()<0) return pixel_iterator();
    if(p1.x()>=width || p1.y()>=height || p2.x()>=width || p2.y()>=height)
        return pixel_iterator();
    if(p2.x()<p1.x() || p2.y()<p1.y()) return pixel_iterator();
 
    if(d==DR) textWindow(p1,p2);
    else imageWindow(p1,p2);
    doBeginPixelWrite();

    unsigned int numPixels=(p2.x()-p1.x()+1)*(p2.y()-p1.y()+1);
    return pixel_iterator(numPixels);
}

DisplayErOledm015::~DisplayErOledm015() {}

void DisplayErOledm015::doBeginPixelWrite()
{
    cmd(0x5c);
    dc::high();
    cs::low();
}
    
void DisplayErOledm015::doWritePixel(Color c)
{
    spi1sendOnly(c>>8);
    spi1sendOnly(c);
}
    
void DisplayErOledm015::doEndPixelWrite()
{
    spi1waitCompletion();
    cs::high();
    delayUs(1);
}

} //namespace mxgui
