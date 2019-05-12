/***************************************************************************
 *   Copyright (C) 2014 by Terraneo Federico                               *
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

#include "display_stm32f4discovery.h"
#include "miosix.h"
#include <cstdarg>

using namespace std;
using namespace miosix;

#ifdef _BOARD_STM32F429ZI_STM32F4DISCOVERY

namespace mxgui {

//Control interface
typedef Gpio<GPIOF_BASE, 7> scl; //SPI SCK
typedef Gpio<GPIOF_BASE, 9> sda; //SPI MOSI
typedef Gpio<GPIOC_BASE, 2> csx; //SPI CS
typedef Gpio<GPIOD_BASE,13> dcx; //Data/command
typedef Gpio<GPIOD_BASE,12> rdx; //Used only un parallel mode
typedef Gpio<GPIOD_BASE,11> te;  //Tearing effect output from display, unused
//Pixel sync interface
typedef Gpio<GPIOF_BASE,10> en;
typedef Gpio<GPIOG_BASE, 7> dotclk;
typedef Gpio<GPIOA_BASE, 4> vsync;
typedef Gpio<GPIOC_BASE, 6> hsync;
//Pixel data bus
typedef Gpio<GPIOC_BASE,10> r0; //r2
typedef Gpio<GPIOB_BASE, 0> r1; //r3
typedef Gpio<GPIOA_BASE,11> r2; //r4
typedef Gpio<GPIOA_BASE,12> r3; //r5
typedef Gpio<GPIOB_BASE, 1> r4; //r6
typedef Gpio<GPIOG_BASE, 6> r5; //r7
typedef Gpio<GPIOA_BASE, 6> g0; //g2
typedef Gpio<GPIOG_BASE,10> g1; //g3
typedef Gpio<GPIOB_BASE,10> g2; //g4
typedef Gpio<GPIOB_BASE,11> g3; //g5
typedef Gpio<GPIOC_BASE, 7> g4; //g6
typedef Gpio<GPIOD_BASE, 3> g5; //g7
typedef Gpio<GPIOD_BASE, 6> b0; //b2
typedef Gpio<GPIOG_BASE,11> b1; //b3
typedef Gpio<GPIOG_BASE,12> b2; //b4
typedef Gpio<GPIOA_BASE, 3> b3; //b5
typedef Gpio<GPIOB_BASE, 8> b4; //b6
typedef Gpio<GPIOB_BASE, 9> b5; //b7

/**
 * Send and receive a byte through SPI5
 * \param c byte to send
 * \return byte received
 */
static unsigned char spi5sendRev(unsigned char c=0)
{
    SPI5->DR=c;
    while((SPI5->SR & SPI_SR_RXNE)==0) ;
    return SPI5->DR;
}

/**
 * Send a command to the ILI9341 display controller
 * \param cmd command
 * \param len length of the (optional) argument, or 0 for commands without
 * arguments.
 */
static void sendCmd(unsigned char cmd, int len, ...)
{
    dcx::low();
    csx::low();
    spi5sendRev(cmd);
    csx::high();
    delayUs(1);
    dcx::high();
    va_list arg;
    va_start(arg,len);
    for(int i=0;i<len;i++)
    {   
        csx::low();
        spi5sendRev(va_arg(arg,int));
        csx::high();
        delayUs(1);
    }
    va_end(arg);
}

void registerDisplayHook(DisplayManager& dm)
{
    dm.registerDisplay(&DisplayImpl::instance());
}

//
// class DisplayImpl
//

DisplayImpl& DisplayImpl::instance()
{
    static DisplayImpl instance;
    return instance;
}

void DisplayImpl::doTurnOn()
{
    LTDC->GCR |= LTDC_GCR_LTDCEN;
    Thread::sleep(40);
    sendCmd(0x29,0); //LCD_DISPLAY_ON
}

void DisplayImpl::doTurnOff()
{
    sendCmd(0x28,0); //LCD_DISPLAY_OFF
    LTDC->GCR &=~ LTDC_GCR_LTDCEN;
}

void DisplayImpl::doSetBrightness(int brt) {}

pair<short int, short int> DisplayImpl::doGetSize() const
{
    return make_pair(height,width);
}

void DisplayImpl::write(Point p, const char *text)
{
    font.draw(*this,textColor,p,text);
}

void DisplayImpl::clippedWrite(Point p, Point a, Point b, const char *text)
{
    font.clippedDraw(*this,textColor,p,a,b,text);
}

void DisplayImpl::clear(Color color)
{
    clear(Point(0,0),Point(width-1,height-1),color);
}

void DisplayImpl::clear(Point p1, Point p2, Color color)
{
    if(p1.x()<0 || p2.x()<p1.x() || p2.x()>=width
     ||p1.y()<0 || p2.y()<p1.y() || p2.y()>=height) return;
    if((color & 0xff)==(color>>8))
    {
        //Can use memset
        if(p1.x()==0 && p2.x()==width-1)
        {
            //Can merge lines
            memset(framebuffer1+p1.y()*width,color,(p2.y()-p1.y()+1)*width*bpp);
        } else {
            //Can't merge lines
            Color *ptr=framebuffer1+p1.x()+width*p1.y();
            short len=p2.x()-p1.x()+1;
            for(short i=p1.y();i<=p2.y();i++)
            {
                memset(ptr,color,len*bpp);
                ptr+=width;
            }
        }
    } else {
        //Can't use memset
        if(p1.x()==0 && p2.x()==width-1)
        {
            //Can merge lines
            Color *ptr=framebuffer1+p1.y()*width;
            int numPixels=(p2.y()-p1.y()+1)*width;
            //This loop is worth unrolling
            for(int i=0;i<numPixels/4;i++)
            {
                *ptr++=color;
                *ptr++=color;
                *ptr++=color;
                *ptr++=color;
            }
            for(int i=0;i<(numPixels & 3);i++) *ptr++=color;
        } else {
            //Can't merge lines
            Color *ptr=framebuffer1+p1.x()+width*p1.y();
            short len=p2.x()-p1.x()+1;
            for(short i=p1.y();i<=p2.y();i++)
            {
                for(short j=0;j<len;j++) *ptr++=color;
                ptr+=width-len;
            }
        }
    }
}

void DisplayImpl::beginPixel() {}

void DisplayImpl::setPixel(Point p, Color color)
{
    int offset=p.x()+p.y()*width;
    if(offset<0 || offset>=numPixels) return;
    *(framebuffer1+offset)=color;
}

void DisplayImpl::line(Point a, Point b, Color color)
{
    //Horizontal line speed optimization
    if(a.y()==b.y())
    {
        short minx=min(a.x(),b.x());
        short maxx=max(a.x(),b.x());
        if(minx<0 || maxx>=width || a.y()<0 || a.y()>=height) return;
        Color *ptr=framebuffer1+minx+width*a.y();
        for(short i=minx;i<=maxx;i++) *ptr++=color;
        return;
    }
    //Vertical line speed optimization
    if(a.x()==b.x())
    {
        short miny=min(a.y(),b.y());
        short maxy=max(a.y(),b.y());
        if(a.x()<0 || a.x()>=width || miny<0 || maxy>=height) return;
        Color *ptr=framebuffer1+a.x()+width*miny;
        for(short i=miny;i<=maxy;i++)
        {
            *ptr=color;
            ptr+=width;
        }
        return;
    }
    //General case
    Line::draw(*this,a,b,color);
}

void DisplayImpl::scanLine(Point p, const Color *colors, unsigned short length)
{
    if(p.x()<0 || static_cast<int>(p.x())+static_cast<int>(length)>width
        ||p.y()<0 || p.y()>=height) return;
    Color *ptr=framebuffer1+p.x()+p.y()*width;
    memcpy(ptr,colors,length*bpp);
}

Color *DisplayImpl::getScanLineBuffer()
{
    return buffer;
}

void DisplayImpl::scanLineBuffer(Point p, unsigned short length)
{
    int offset=p.x()+p.y()*width;
    if(offset<0 || offset>=numPixels) return;
    memcpy(framebuffer1+offset,buffer,length*bpp);
}

void DisplayImpl::drawImage(Point p, const ImageBase& img)
{
    short int xEnd=p.x()+img.getWidth()-1;
    short int yEnd=p.y()+img.getHeight()-1;
    if(p.x()<0 || p.y()<0 || xEnd<p.x() || yEnd<p.y()
        ||xEnd >= width || yEnd >= height) return;

//    const unsigned short *imgData=img.getData();
//    if(imgData!=0)
//    {
//        //TODO Optimized version for in-memory images
//    } else
    img.draw(*this,p);
}

void DisplayImpl::clippedDrawImage(Point p, Point a, Point b, const ImageBase& img)
{
//    if(img.getData()==0)
//    {
    img.clippedDraw(*this,p,a,b);
    return;
//    } //else optimized version for memory-loaded images
//        //TODO: optimize
//    }
}


void DisplayImpl::drawRectangle(Point a, Point b, Color c)
{
    line(a,Point(b.x(),a.y()),c);
    line(Point(b.x(),a.y()),b,c);
    line(b,Point(a.x(),b.y()),c);
    line(Point(a.x(),b.y()),a,c);
}

DisplayImpl::pixel_iterator DisplayImpl::begin(Point p1, Point p2,
        IteratorDirection d)
{
    bool fail=false;
    if(p1.x()<0 || p1.y()<0 || p2.x()<0 || p2.y()<0) fail=true;
    if(p1.x()>=width || p1.y()>=height || p2.x()>=width || p2.y()>=height) fail=true;
    if(p2.x()<p1.x() || p2.y()<p1.y()) fail=true;
    if(fail)
    {
        //Return invalid (dummy) iterators
        this->last=pixel_iterator();
        return this->last;
    }

    //Set the last iterator to a suitable one-past-the last value
    if(d==DR) this->last=pixel_iterator(Point(p2.x()+1,p1.y()),p2,d,this);
    else this->last=pixel_iterator(Point(p1.x(),p2.y()+1),p2,d,this);

    return pixel_iterator(p1,p2,d,this);
}

DisplayImpl::~DisplayImpl() {}

DisplayImpl::DisplayImpl()
    : framebuffer1(reinterpret_cast<unsigned short*>(0xd0600000)),
      buffer(framebuffer1+numPixels)
{
    {
        FastInterruptDisableLock dLock;
        //PLLSAI runs @ 192MHz, both Q and R outputs are divided by 4 so 48MHz
        RCC->PLLSAICFGR=4<<28 | 4<<24 | 192<<6;
        //PLLSAI R output divided by 8 resulting in a 6MHz LTDC clock
        RCC->DCKCFGR=2<<16;
        RCC->CR |= RCC_CR_PLLSAION;
    }
    
    while((RCC->CR & RCC_CR_PLLSAIRDY)==0) ;
    
    {
        FastInterruptDisableLock dLock;
        
        scl::mode(Mode::ALTERNATE);    scl::alternateFunction(5); //SPI5
        sda::mode(Mode::ALTERNATE);    sda::alternateFunction(5);
        csx::mode(Mode::OUTPUT);       csx::high();
        dcx::mode(Mode::OUTPUT);
        rdx::mode(Mode::OUTPUT);       rdx::low(); //Original fw seems to leave it low

        en::mode(Mode::ALTERNATE);     en::alternateFunction(14);
        dotclk::mode(Mode::ALTERNATE); dotclk::alternateFunction(14);
        vsync::mode(Mode::ALTERNATE);  vsync::alternateFunction(14);
        hsync::mode(Mode::ALTERNATE);  hsync::alternateFunction(14);
        r0::mode(Mode::ALTERNATE);     r0::alternateFunction(14);
        r1::mode(Mode::ALTERNATE);     r1::alternateFunction(14);
        r2::mode(Mode::ALTERNATE);     r2::alternateFunction(14);
        r3::mode(Mode::ALTERNATE);     r3::alternateFunction(14);
        r4::mode(Mode::ALTERNATE);     r4::alternateFunction(14);
        r5::mode(Mode::ALTERNATE);     r5::alternateFunction(14);
        g0::mode(Mode::ALTERNATE);     g0::alternateFunction(14);
        g1::mode(Mode::ALTERNATE);     g1::alternateFunction(14);
        g2::mode(Mode::ALTERNATE);     g2::alternateFunction(14);
        g3::mode(Mode::ALTERNATE);     g3::alternateFunction(14);
        g4::mode(Mode::ALTERNATE);     g4::alternateFunction(14);
        g5::mode(Mode::ALTERNATE);     g5::alternateFunction(14);
        b0::mode(Mode::ALTERNATE);     b0::alternateFunction(14);
        b1::mode(Mode::ALTERNATE);     b1::alternateFunction(14);
        b2::mode(Mode::ALTERNATE);     b2::alternateFunction(14);
        b3::mode(Mode::ALTERNATE);     b3::alternateFunction(14);
        b4::mode(Mode::ALTERNATE);     b4::alternateFunction(14);
        b5::mode(Mode::ALTERNATE);     b5::alternateFunction(14);
        
        RCC->APB2ENR |= RCC_APB2ENR_LTDCEN | RCC_APB2ENR_SPI5EN;      
        RCC_SYNC();
    }
    
    SPI5->CR1=SPI_CR1_SSM   //Sowtware CS
            | SPI_CR1_SSI   //Software CS high
            | SPI_CR1_SPE   //SPI enabled
            | (3<<3)        //Divide input clock by 16: 84/16=5.25MHz
            | SPI_CR1_MSTR; //Master mode
    Thread::sleep(1);
    
    //
    // ILI9341 power up sequence -- begin
    //
    sendCmd(0xca,3,0xc3,0x08,0x50);           //undocumented command
    sendCmd(0xcf,3,0x00,0xc1,0x30);           //LCD_POWERB
    sendCmd(0xed,4,0x64,0x03,0x12,0x81);      //LCD_POWER_SEQ
    sendCmd(0xe8,3,0x85,0x00,0x78);           //LCD_DTCA
    sendCmd(0xcb,5,0x39,0x2c,0x00,0x34,0x02); //LCD_POWERA
    sendCmd(0xf7,1,0x20);                     //LCD_PRC
    sendCmd(0xea,2,0x00,0x00);                //LCD_DTCB
    sendCmd(0xb1,2,0x00,0x1b);                //LCD_FRMCTR1
    sendCmd(0xb6,2,0x0a,0xa2);                //LCD_DFC
    sendCmd(0xc0,1,0x10);                     //LCD_POWER1
    sendCmd(0xc1,1,0x10);                     //LCD_POWER2
    sendCmd(0xc5,2,0x45,0x15);                //LCD_VCOM1
    sendCmd(0xc7,1,0x90);                     //LCD_VCOM2
    sendCmd(0x36,1,0xc8);                     //LCD_MAC
    sendCmd(0xf2,1,0x00);                     //LCD_3GAMMA_EN
    sendCmd(0xb0,1,0xc2);                     //LCD_RGB_INTERFACE
    sendCmd(0xb6,4,0x0a,0xa7,0x27,0x04);      //LCD_DFC
    sendCmd(0x2a,4,0x00,0x00,0x00,0xef);      //LCD_COLUMN_ADDR
    sendCmd(0x2b,4,0x00,0x00,0x01,0x3f);      //LCD_PAGE_ADDR
    sendCmd(0xf6,3,0x01,0x00,0x06);           //LCD_INTERFACE
    sendCmd(0x2c,0);                          //LCD_GRAM
    Thread::sleep(200);
    sendCmd(0x26,1,0x01);                     //LCD_GAMMA
    sendCmd(0xe0,15,0x0f,0x29,0x24,0x0c,0x0e,0x09,0x4e,0x78,0x3c,0x09,0x13,
            0x05,0x17,0x11,0x00);             //LCD_PGAMMA
    sendCmd(0xe1,15,0x00,0x16,0x1b,0x04,0x11,0x07,0x31,0x33,0x42,0x05,0x0c,
            0x0a,0x28,0x2f,0x0f);             //LCD_NGAMMA
    sendCmd(0x11,0);                          //LCD_SLEEP_OUT
    Thread::sleep(200);
    sendCmd(0x29,0);                          //LCD_DISPLAY_ON
    sendCmd(0x2c,0);                          //LCD_GRAM
    //
    // ILI9341 power up sequence -- end
    //
    
    memset(framebuffer1,0,height*width*bpp);
    
    const unsigned int hsync=10;   //hsync timing
    const unsigned int vsync=2;    //vsync timing
    const unsigned int hbp=20;     //horizontal back porch
    const unsigned int vbp=2;      //vertical back porch
    const unsigned int hfp=10;     //horizontal front porch
    const unsigned int vfp=4;      //vertical front porch
    enum {
        ARGB8888=0,
        RGB888=1,
        RGB565=2,
        ARGB1555=3,
        ARGB4444=4,
        L8=5,
        AL44=6,
        AL88=7
    };
    //Configure timings
    LTDC->SSCR=(hsync-1)<<16               | (vsync-1);
    LTDC->BPCR=(hsync+hbp-1)<<16           | (vsync+vbp-1);
    LTDC->AWCR=(hsync+hbp+width-1)<<16     | (vsync+vbp+height-1);
    LTDC->TWCR=(hfp+hsync+hbp+width-1)<<16 | (vfp+vsync+vbp+height-1);
    //Configre background color (black))
    LTDC->BCCR=0;
    //Enable layer 2
    LTDC_Layer2->CR=0              //Disable palette mode
                  | 0              //Disable color keying
                  | LTDC_LxCR_LEN; //Enable layer
    LTDC_Layer2->WHPCR=(hsync+hbp+width-1)<<16  | (hsync+hbp);
    LTDC_Layer2->WVPCR=(vsync+vbp+height-1)<<16 | (vsync+vbp);
    LTDC_Layer2->CKCR=0;
    LTDC_Layer2->PFCR=RGB565;
    LTDC_Layer2->CACR=0xff; //Alpha=1
    LTDC_Layer2->DCCR=0;
    LTDC_Layer2->CFBAR=reinterpret_cast<unsigned int>(framebuffer1);
    LTDC_Layer2->CFBLR=(width*bpp)<<16 | (width*bpp+3); //Packed lines
    LTDC_Layer2->CFBLNR=height;
    //Write to shadow registers
    LTDC->SRCR=LTDC_SRCR_IMR;
    //Finally enable the display
    LTDC->GCR=0                //hsync active low
            | 0                //vsync active low
            | 0                //enable active low
            | 0                //input pixel clock
            | 0                //no dithering
            | LTDC_GCR_LTDCEN; //Display enabled
    
    setFont(droid11);
    setTextColor(make_pair(Color(0xffff),Color(0x0000)));
    clear(black);
}

Color DisplayImpl::pixel_iterator::dummy;

} //namespace mxgui

#endif //_BOARD_STM32F429ZI_STM32F4DISCOVERY

#ifdef _BOARD_STM32F469NI_STM32F469I_DISCO

namespace mxgui {

//Control interface

//Pixel sync interface
typedef Gpio<GPIOF_BASE,10> en;
typedef Gpio<GPIOG_BASE, 7> dotclk;
typedef Gpio<GPIOA_BASE, 4> vsync;
typedef Gpio<GPIOC_BASE, 6> hsync;
//Pixel data bus
typedef Gpio<GPIOC_BASE,10> r0; //r2
typedef Gpio<GPIOJ_BASE, 2> r1; //r3
typedef Gpio<GPIOA_BASE,11> r2; //r4
typedef Gpio<GPIOA_BASE,12> r3; //r5
typedef Gpio<GPIOJ_BASE, 5> r4; //r6
typedef Gpio<GPIOA_BASE, 6> g0; //g2
typedef Gpio<GPIOG_BASE,10> g1; //g3 AF9
typedef Gpio<GPIOJ_BASE,13> g2; //g4 AF9
typedef Gpio<GPIOH_BASE, 4> g3; //g5 AF9
typedef Gpio<GPIOC_BASE, 7> g4; //g6
typedef Gpio<GPIOD_BASE, 3> g5; //g7
typedef Gpio<GPIOD_BASE, 6> b0; //b2
typedef Gpio<GPIOG_BASE,11> b1; //b3
typedef Gpio<GPIOG_BASE,12> b2; //b4 AF9
typedef Gpio<GPIOA_BASE, 3> b3; //b5
typedef Gpio<GPIOB_BASE, 8> b4; //b6

void shortWrite(uint8_t param0, uint8_t param1) {
    // Command FIFO Empty
    while ((DSI->GPSR & DSI_GPSR_CMDFE) == 0);
    
    DSI->GHCR = (0x15 |        // DSI_DCS_SHORT_PKT_WRITE_P1
                (0 << 6) |     // Virtual Channel ID
                (param0 << 8) | \
                (param1 << 16));
}

void longWrite(uint32_t numParams, uint32_t param0, uint8_t *pParams) {
    // Command FIFO Empty
    while ((DSI->GPSR & DSI_GPSR_CMDFE) == 0);
    
    uint32_t uicounter = 0;
    while (uicounter < numParams) {
        if (uicounter == 0x00) {
            DSI->GPDR = (param0 | \
                        ((*(pParams+uicounter)) << 8) | \
                        ((*(pParams+uicounter+1)) << 16) | \
                        ((*(pParams+uicounter+2)) << 24));
            uicounter += 3;
        } else {
            DSI->GPDR = ((*(pParams+uicounter)) | \
                        ((*(pParams+uicounter+1)) << 8) | \
                        ((*(pParams+uicounter+2)) << 16) | \
                        ((*(pParams+uicounter+3)) << 24));
            uicounter += 4;
        }
    }
    
    DSI->GHCR = (0x39 |        // DSI_DCS_LONG_PKT_WRITE
                (0 << 6) |     // Virtual Channel Id
                (((numParams+1)&0x00FF) << 8) | \
                ((((numParams+1)&0xFF00) >> 8) << 16));
}

void sendCmd(uint32_t numParams, uint8_t *pParams) {
    if (numParams <= 1) {
        shortWrite(pParams[0], pParams[1]);
    } else {
        longWrite(numParams, pParams[numParams], pParams);
    }
}

void registerDisplayHook(DisplayManager& dm)
{
    dm.registerDisplay(&DisplayImpl::instance());
}

//
// class DisplayImpl
//

DisplayImpl& DisplayImpl::instance()
{
    static DisplayImpl instance;
    return instance;
}

void DisplayImpl::doTurnOn()
{
    LTDC->GCR |= LTDC_GCR_LTDCEN;
    Thread::sleep(40);
    uint8_t set_display_on[] = {0x29, 0x00};
    sendCmd(0, set_display_on);
}

void DisplayImpl::doTurnOff()
{
    uint8_t set_display_off[] = {0x28, 0x00};
    sendCmd(0, set_display_off);
    LTDC->GCR &= ~LTDC_GCR_LTDCEN;
}

void DisplayImpl::doSetBrightness(int brt) {}

pair<short int, short int> DisplayImpl::doGetSize() const
{
    return make_pair(height,width);
}

void DisplayImpl::write(Point p, const char *text)
{
    font.draw(*this,textColor,p,text);
}

void DisplayImpl::clippedWrite(Point p, Point a, Point b, const char *text)
{
    font.clippedDraw(*this,textColor,p,a,b,text);
}

void DisplayImpl::clear(Color color)
{
    clear(Point(0,0),Point(width-1,height-1),color);
}

void DisplayImpl::clear(Point p1, Point p2, Color color)
{
    if(p1.x()<0 || p2.x()<p1.x() || p2.x()>=width
     ||p1.y()<0 || p2.y()<p1.y() || p2.y()>=height) return;
    if((color & 0xff)==(color>>8))
    {
        //Can use memset
        if(p1.x()==0 && p2.x()==width-1)
        {
            //Can merge lines
            memset(framebuffer1+p1.y()*width,color,(p2.y()-p1.y()+1)*width*bpp);
        } else {
            //Can't merge lines
            Color *ptr=framebuffer1+p1.x()+width*p1.y();
            short len=p2.x()-p1.x()+1;
            for(short i=p1.y();i<=p2.y();i++)
            {
                memset(ptr,color,len*bpp);
                ptr+=width;
            }
        }
    } else {
        //Can't use memset
        if(p1.x()==0 && p2.x()==width-1)
        {
            //Can merge lines
            Color *ptr=framebuffer1+p1.y()*width;
            int numPixels=(p2.y()-p1.y()+1)*width;
            //This loop is worth unrolling
            for(int i=0;i<numPixels/4;i++)
            {
                *ptr++=color;
                *ptr++=color;
                *ptr++=color;
                *ptr++=color;
            }
            for(int i=0;i<(numPixels & 3);i++) *ptr++=color;
        } else {
            //Can't merge lines
            Color *ptr=framebuffer1+p1.x()+width*p1.y();
            short len=p2.x()-p1.x()+1;
            for(short i=p1.y();i<=p2.y();i++)
            {
                for(short j=0;j<len;j++) *ptr++=color;
                ptr+=width-len;
            }
        }
    }
}

void DisplayImpl::beginPixel() {}

void DisplayImpl::setPixel(Point p, Color color)
{
    int offset=p.x()+p.y()*width;
    if(offset<0 || offset>=numPixels) return;
    *(framebuffer1+offset)=color;
}

void DisplayImpl::line(Point a, Point b, Color color)
{
    //Horizontal line speed optimization
    if(a.y()==b.y())
    {
        short minx=min(a.x(),b.x());
        short maxx=max(a.x(),b.x());
        if(minx<0 || maxx>=width || a.y()<0 || a.y()>=height) return;
        Color *ptr=framebuffer1+minx+width*a.y();
        for(short i=minx;i<=maxx;i++) *ptr++=color;
        return;
    }
    //Vertical line speed optimization
    if(a.x()==b.x())
    {
        short miny=min(a.y(),b.y());
        short maxy=max(a.y(),b.y());
        if(a.x()<0 || a.x()>=width || miny<0 || maxy>=height) return;
        Color *ptr=framebuffer1+a.x()+width*miny;
        for(short i=miny;i<=maxy;i++)
        {
            *ptr=color;
            ptr+=width;
        }
        return;
    }
    //General case
    Line::draw(*this,a,b,color);
}

void DisplayImpl::scanLine(Point p, const Color *colors, unsigned short length)
{
    if(p.x()<0 || static_cast<int>(p.x())+static_cast<int>(length)>width
        ||p.y()<0 || p.y()>=height) return;
    Color *ptr=framebuffer1+p.x()+p.y()*width;
    memcpy(ptr,colors,length*bpp);
}

Color *DisplayImpl::getScanLineBuffer()
{
    return buffer;
}

void DisplayImpl::scanLineBuffer(Point p, unsigned short length)
{
    int offset=p.x()+p.y()*width;
    if(offset<0 || offset>=numPixels) return;
    memcpy(framebuffer1+offset,buffer,length*bpp);
}

void DisplayImpl::drawImage(Point p, const ImageBase& img)
{
    short int xEnd=p.x()+img.getWidth()-1;
    short int yEnd=p.y()+img.getHeight()-1;
    if(p.x()<0 || p.y()<0 || xEnd<p.x() || yEnd<p.y()
        ||xEnd >= width || yEnd >= height) return;

//    const unsigned short *imgData=img.getData();
//    if(imgData!=0)
//    {
//        //TODO Optimized version for in-memory images
//    } else
    img.draw(*this,p);
}

void DisplayImpl::clippedDrawImage(Point p, Point a, Point b, const ImageBase& img)
{
//    if(img.getData()==0)
//    {
    img.clippedDraw(*this,p,a,b);
    return;
//    } //else optimized version for memory-loaded images
//        //TODO: optimize
//    }
}

void DisplayImpl::drawRectangle(Point a, Point b, Color c)
{
    line(a,Point(b.x(),a.y()),c);
    line(Point(b.x(),a.y()),b,c);
    line(b,Point(a.x(),b.y()),c);
    line(Point(a.x(),b.y()),a,c);
}

void DisplayImpl::update()
{
    DSI->WCR |= DSI_WCR_LTDCEN;
}

DisplayImpl::pixel_iterator DisplayImpl::begin(Point p1, Point p2,
        IteratorDirection d)
{
    bool fail=false;
    if(p1.x()<0 || p1.y()<0 || p2.x()<0 || p2.y()<0) fail=true;
    if(p1.x()>=width || p1.y()>=height || p2.x()>=width || p2.y()>=height) fail=true;
    if(p2.x()<p1.x() || p2.y()<p1.y()) fail=true;
    if(fail)
    {
        //Return invalid (dummy) iterators
        this->last=pixel_iterator();
        return this->last;
    }

    //Set the last iterator to a suitable one-past-the last value
    if(d==DR) this->last=pixel_iterator(Point(p2.x()+1,p1.y()),p2,d,this);
    else this->last=pixel_iterator(Point(p1.x(),p2.y()+1),p2,d,this);

    return pixel_iterator(p1,p2,d,this);
}

DisplayImpl::~DisplayImpl() {}

DisplayImpl::DisplayImpl()
    : framebuffer1(reinterpret_cast<unsigned short *>(0xc0000000)),
      buffer(framebuffer1 + numPixels)
{
    /* This driver uses DSI interface in command mode, but it was firstly programmed in video mode.
     * For this reason some instructions are still there but they don't actually affect the driver.
     * For example these timing parameters are important in video mode, but in command mode they can
     * take any value and the display still works.
     */
    const unsigned int hsync = 12;   // hsync timing
    const unsigned int vsync = 120;  // vsync timing
    const unsigned int hbp = 12;     // horizontal back porch
    const unsigned int vbp = 120;    // vertical back porch
    const unsigned int hfp = 12;     // horizontal front porch
    const unsigned int vfp = 120;    // vertical front porch
    enum {
        ARGB8888 = 0,
        RGB888 = 1,
        RGB565 = 2,
        ARGB1555 = 3,
        ARGB4444 = 4,
        L8 = 5,
        AL44 = 6,
        AL88 = 7
    };
    
    // Parameters for DSI PLL
    // These values assume HSE oscillator is 8 MHz and system clock is 168 MHz
    #if HSE_VALUE != 8000000
    #error The display driver requires an HSE oscillator running at 8 MHz
    #endif
    const unsigned int IDF = 4;     // must be in the range 1..7
    const unsigned int ODF = 1;     // must be in the set {1, 2, 4, 8}
    const unsigned int NDIV = 125;  // must be in the range 10..125
    const unsigned int F_VCO = (HSE_VALUE/IDF)*2*NDIV;          // 500 MHz - must be between 500 and 1000 MHz
    const unsigned int F_PHY_MHz = (F_VCO/(2*ODF))/1000000;     // 250 MHz - HS clock for D-PHY must be between 80 and 500 MHz
    const unsigned int lane_byte_clk = F_VCO/(2*ODF*8);         // 31,25 MHz - must be no more than 62,5 MHz
    const unsigned int TXECLKDIV = 2;                           // must be at least 2 and ensure lane_byte_clk/TXECLKDIV <= 20 MHz
    const unsigned int pixel_clock = F_VCO/bpp;                 // 31,25 MHz
    const unsigned int clock_ratio = lane_byte_clk/pixel_clock;

    memset(framebuffer1, 0, height*width*bpp);
    
    // Reset of screen by active low on GPIO PH7
    typedef Gpio<GPIOH_BASE, 7> reset;
    reset::mode(Mode::OUTPUT);
    reset::speed(Speed::_100MHz);
    reset::low();
    Thread::sleep(20);
    reset::high();
    Thread::sleep(10);
    
    // Enable clock for DSI and LTDC then force their reset
    {
        FastInterruptDisableLock dLock;

        en::mode(Mode::ALTERNATE);     en::alternateFunction(14);       en::speed(Speed::_100MHz);
        dotclk::mode(Mode::ALTERNATE); dotclk::alternateFunction(14);   dotclk::speed(Speed::_100MHz);
        vsync::mode(Mode::ALTERNATE);  vsync::alternateFunction(14);    vsync::speed(Speed::_100MHz);
        hsync::mode(Mode::ALTERNATE);  hsync::alternateFunction(14);    hsync::speed(Speed::_100MHz);
        r0::mode(Mode::ALTERNATE);     r0::alternateFunction(14);       r0::speed(Speed::_100MHz);
        r1::mode(Mode::ALTERNATE);     r1::alternateFunction(14);       r1::speed(Speed::_100MHz);
        r2::mode(Mode::ALTERNATE);     r2::alternateFunction(14);       r2::speed(Speed::_100MHz);
        r3::mode(Mode::ALTERNATE);     r3::alternateFunction(14);       r3::speed(Speed::_100MHz);
        r4::mode(Mode::ALTERNATE);     r4::alternateFunction(14);       r4::speed(Speed::_100MHz);
        g0::mode(Mode::ALTERNATE);     g0::alternateFunction(14);       g0::speed(Speed::_100MHz);
        g1::mode(Mode::ALTERNATE);     g1::alternateFunction(9);        g1::speed(Speed::_100MHz);
        g2::mode(Mode::ALTERNATE);     g2::alternateFunction(9);        g2::speed(Speed::_100MHz);
        g3::mode(Mode::ALTERNATE);     g3::alternateFunction(9);        g3::speed(Speed::_100MHz);
        g4::mode(Mode::ALTERNATE);     g4::alternateFunction(14);       g4::speed(Speed::_100MHz);
        g5::mode(Mode::ALTERNATE);     g5::alternateFunction(14);       g5::speed(Speed::_100MHz);
        b0::mode(Mode::ALTERNATE);     b0::alternateFunction(14);       b0::speed(Speed::_100MHz);
        b1::mode(Mode::ALTERNATE);     b1::alternateFunction(14);       b1::speed(Speed::_100MHz);
        b2::mode(Mode::ALTERNATE);     b2::alternateFunction(9);        b2::speed(Speed::_100MHz);
        b3::mode(Mode::ALTERNATE);     b3::alternateFunction(14);       b3::speed(Speed::_100MHz);
        b4::mode(Mode::ALTERNATE);     b4::alternateFunction(14);       b4::speed(Speed::_100MHz);
        
        RCC->APB2ENR |= RCC_APB2ENR_LTDCEN;
        RCC_SYNC();
        RCC->APB2ENR |= RCC_APB2ENR_DSIEN;
        RCC_SYNC();
        
        RCC->APB2RSTR |= RCC_APB2RSTR_LTDCRST;
        RCC->APB2RSTR &= ~RCC_APB2RSTR_LTDCRST;
        
        RCC->APB2RSTR |= RCC_APB2RSTR_DSIRST;
        RCC->APB2RSTR &= ~RCC_APB2RSTR_DSIRST;
    }

    // Configure PLLSAI for LTDC, turn it ON and wait for its lock
    {
        FastInterruptDisableLock dLock;
        
        // LTDC clock depends on PLL_M which is fixed at boot with value 8
        // It also depends on PLLSAI which can be freely configured
        const unsigned int PLLSAI_N = 384;
        const unsigned int PLLSAI_R = 7;
        //const unsigned int PLLSAI_DIVR = 0;
        
        // Input VCO Frequency = HSE_VALUE/PPL_M must be between 1 and 2 MHz, so 8/8 = 1 MHz
        // N must be in the range 50..432 and ensure a frequency between 100 and 432 MHz
        // if N = 384 then 1 MHz * 384 = 384 MHz
        // R must be in the range 2..7, we choose R = 7 so 384/7 = 54,857 MHz
        // and then we divide it by 2 (setting DIVR to 0) to obtain 27,428 Mhz
        
        // Read PLLSAI_P and PLLSAI_Q values from PLLSAICFGR register
        uint32_t PLLSAI_P = ((RCC->PLLSAICFGR & RCC_PLLSAICFGR_PLLSAIP) >> 16);
        uint32_t PLLSAI_Q = ((RCC->PLLSAICFGR & RCC_PLLSAICFGR_PLLSAIQ) >> 24);     
        // PLLSAI_VCO_Input  = PLL_SOURCE/PLL_M
        // PLLSAI_VCO_Output = PLLSAI_VCO_Input * PLLSAI_N
        // LTDC_CLK(first level) = PLLSAI_VCO_Output/PLLSAI_R
        RCC->PLLSAICFGR = (PLLSAI_N <<  6) | \
                          (PLLSAI_P << 16) | \
                          (PLLSAI_Q << 24) | \
                          (PLLSAI_R << 28);
        // LTDC_CLK = LTDC_CLK(first level)/PLLSAI_DIVR
        RCC->DCKCFGR = RCC->DCKCFGR & ~RCC_DCKCFGR_PLLSAIDIVR;
      
        RCC->CR |= RCC_CR_PLLSAION;
    }
    while ((RCC->CR & RCC_CR_PLLSAIRDY) == 0);
    
    /* Start LTDC configuration */
    // Configure timings
    LTDC->SSCR = (hsync-1) << 16               | (vsync-1);
    LTDC->BPCR = (hsync+hbp-1) << 16           | (vsync+vbp-1);
    LTDC->AWCR = (hsync+hbp+width-1) << 16     | (vsync+vbp+height-1);
    LTDC->TWCR = (hsync+hbp+width+hfp-1) << 16 | (vsync+vbp+height+vfp-1);
    
    // Configure polarities (everything active high except data enabled)
    LTDC->GCR |= (LTDC_GCR_HSPOL | LTDC_GCR_VSPOL | LTDC_GCR_DEPOL | LTDC_GCR_PCPOL);
    LTDC->GCR &= ~LTDC_GCR_DEPOL;

    // Configure background color
    LTDC->BCCR = 0;

    // Configure the layer
        // Horizontal start and stop position
    LTDC_Layer1->WHPCR &= ~(LTDC_LxWHPCR_WHSTPOS | LTDC_LxWHPCR_WHSPPOS);
    LTDC_Layer1->WHPCR = ((hsync+hbp+width-1) << 16) | (hsync+hbp);
        // Vertical start and stop position 
    LTDC_Layer1->WVPCR &= ~(LTDC_LxWVPCR_WVSTPOS | LTDC_LxWVPCR_WVSPPOS);
    LTDC_Layer1->WVPCR = ((vsync+vbp+height-1) << 16) | (vsync+vbp);
        // Pixel format
    LTDC_Layer1->PFCR &= ~(LTDC_LxPFCR_PF);
    LTDC_Layer1->PFCR |= RGB565;
        // Color frame buffer start address
    LTDC_Layer1->CFBAR &= ~(LTDC_LxCFBAR_CFBADD);
    LTDC_Layer1->CFBAR = reinterpret_cast<unsigned int>(framebuffer1);
        // Color frame buffer pitch in byte (multiply by 2 for RGB565)
    LTDC_Layer1->CFBLR &= ~(LTDC_LxCFBLR_CFBP | LTDC_LxCFBLR_CFBLL);
    LTDC_Layer1->CFBLR = (width*2) << 16 | (width*2+3);
        // Frame buffer line number
    LTDC_Layer1->CFBLNR &= ~(LTDC_LxCFBLNR_CFBLNBR);
    LTDC_Layer1->CFBLNR = height;
        // Default color values (black with no alpha)
    LTDC_Layer1->DCCR &= ~(LTDC_LxDCCR_DCBLUE | LTDC_LxDCCR_DCGREEN | LTDC_LxDCCR_DCRED | LTDC_LxDCCR_DCALPHA);
    LTDC_Layer1->DCCR = 0;
        // Specifies the constant alpha value
    LTDC_Layer1->CACR &= ~(LTDC_LxCACR_CONSTA);
    LTDC_Layer1->CACR = 255;
        // Blending factors (constant alpha)
    LTDC_Layer1->BFCR &= ~(LTDC_LxBFCR_BF2 | LTDC_LxBFCR_BF1);
    LTDC_Layer1->BFCR = (0x400 | 0x5);
        // Enable layer
    LTDC_Layer1->CR |= LTDC_LxCR_LEN;
    
    // If needed enable dithering and color keying 
    LTDC_Layer1->CKCR = 0;
   
    // Reload shadow registers
    LTDC->SRCR |= LTDC_SRCR_IMR;
    
    // Finally enable the display
    LTDC->GCR |= LTDC_GCR_LTDCEN;
    /* End LTDC configuration */
    
    /* Start DSI configuration */
    // Turn on the DSI regulator and wait for the regulator ready
    DSI->WRPCR |= DSI_WRPCR_REGEN;
    while ((DSI->WISR & DSI_WISR_RRS) == 0);
    
    // Configure the DSI PLL, turn it ON and wait for its lock 
    // F_VCO = (HSE_VALUE / IDF) * 2 * NDIV
    // Lane_Byte_CLK = F_VCO / (2 * ODF * 8)
    // F_VCO must be in the range from 500 MHz to 1 GHz
    // To obtain 500 Mbit/s rate, Lane_Byte_CLK must be 31,25 MHz
    // Since HSE_VALUE = 8 MHz this is possible with NDIV = 125, IDF = 4, ODF = 1
    DSI->WRPCR &= ~(DSI_WRPCR_PLL_NDIV | DSI_WRPCR_PLL_IDF | DSI_WRPCR_PLL_ODF);
    DSI->WRPCR |= ((NDIV << 2) | (IDF << 11) | (ODF << 16));
    DSI->WRPCR |= DSI_WRPCR_PLLEN;
    while ((DSI->WISR & DSI_WISR_PLLLS) == 0);
    
    // Configure the D-PHY parameters
    // Calculate the bit period in high-speed mode in unit of 0.25 ns (UIX4)
    // The equation is: UIX4 = IntegerPart( (1000/F_PHY_MHz) * 4 )
    // Where: F_PHY_MHz = (NDIV * HSE_MHz) / (IDF * ODF)
    DSI->WPCR[0] &= ~DSI_WPCR0_UIX4;
    DSI->WPCR[0] |= (1000/F_PHY_MHz)*4;
    // Disable all error interrupts and reset the Error Mask
    DSI->IER[0] = 0;
    DSI->IER[1] = 0;
    // Configure the number of active data lanes (just one out of two for 16 bpp)
    DSI->PCONFR &= ~DSI_PCONFR_NL;
    DSI->PCONFR |= 1;
    // Set automatic clock lane control
    DSI->CLCR |= (DSI_CLCR_DPCC | DSI_CLCR_ACR);
    // Time for LP/HS and HS/LP transitions for both clock lane and data lanes
    DSI->CLTCR |= (40 << 16)   // HS to LP
                | (40 <<  0);  // LP to HS
    DSI->DLTCR |= (20 << 24)   // HS to LP
                | (20 << 16);  // HS to LP
    // Stop wait time (don't know how much should it be, random high number in 8 bit)
    DSI->PCONFR &= ~DSI_PCONFR_SW_TIME;
    DSI->PCONFR |= (100 << 8);

    // Configure the DSI Host timing
    //DSI->CCR |= (... << 8); // timeout clock configuration non dice nulla...
    // Configure clock speed for low-power mode
    DSI->CCR &= ~DSI_CCR_TXECKDIV;
    DSI->CCR |= TXECLKDIV;
    
    // Configure the DSI Host flow control and DBI interface
    DSI->PCR &= ~(DSI_PCR_CRCRXE | DSI_PCR_ECCRXE | DSI_PCR_BTAE | DSI_PCR_ETRXE | DSI_PCR_ETTXE);
    DSI->GVCIDR &= ~DSI_GVCIDR_VCID; // set Virtual Channel ID = 0 for the display
    
    // Configure the DSI Host LTDC interface
    DSI->LVCIDR &= ~3;  // Virtual channel ID for LTDC interface traffic
    DSI->LCOLCR &= ~DSI_LCOLCR_COLC;
    DSI->LCOLCR |= RGB565;  // Color coding for the host
    DSI->WCFGR &= ~DSI_WCFGR_COLMUX;
    DSI->WCFGR |= RGB565 << 1;  // Color coding for the wrapper
    DSI->LPCR &= ~(DSI_LPCR_DEP | DSI_LPCR_VSP | DSI_LPCR_HSP);
    DSI->LPCR |= (DSI_LPCR_DEP | 0 | 0);  // Polarity of control signals: same of LTDC except for DE
    DSI->WCFGR |= DSI_WCFGR_VSPOL;  // LTDC halts at VSYNC rising edge
    
    // Configure the DSI Host for command mode
        // Select command mode by setting CMDM and DSIM bits 
    DSI->MCR |= DSI_MCR_CMDM;
    DSI->WCFGR |= DSI_WCFGR_DSIM;
        // Configure the maximum allowed size for write memory command
    DSI->LCCR &= ~DSI_LCCR_CMDSIZE;
    DSI->LCCR |= width;
    
    DSI->VMCR |= 0x3f << 8;  // LP allowed in all video periods
    DSI->VMCR &= ~DSI_VMCR_FBTAAE;  // Do not request acknowledge at end of frame (at this time)
    DSI->VMCR |= DSI_VMCR_LPCE;  // Allow commands in LP
    DSI->VPCR |= width;  // Video packet size
    DSI->VCCR = 0;  // Chunks number to be transmitted through the DSI link
    DSI->VNPCR |= 0xFFF;  // Size of the null packet
    // Timings in lane byte clock cycles
    DSI->VLCR |= (hsync + hbp + width + hfp)*clock_ratio;
    DSI->VHSACR |= hsync*clock_ratio;
    DSI->VHBPCR |= hbp*clock_ratio;
    DSI->VVSACR |= vsync;
    DSI->VVBPCR |= vbp;
    DSI->VVFPCR |= vfp;
    DSI->VVACR |= height;
    DSI->LPMCR |= (64 << 16);  // Low power largest packet size
    DSI->LPMCR |= 64;  // Low power VACT largest packet size
    // Command trasmission only in low power mode
    DSI->CMCR |=  (DSI_CMCR_GSW0TX | \
                   DSI_CMCR_GSW1TX | \
                   DSI_CMCR_GSW2TX | \
                   DSI_CMCR_GSR0TX | \
                   DSI_CMCR_GSR1TX | \
                   DSI_CMCR_GSR2TX | \
                   DSI_CMCR_GLWTX  | \
                   DSI_CMCR_DSW0TX | \
                   DSI_CMCR_DSW1TX | \
                   DSI_CMCR_DSR0TX | \
                   DSI_CMCR_DLWTX  | \
                   DSI_CMCR_MRDPS);   
    
    // Configure the acknowledge request after each packet transmission
    DSI->CMCR |= DSI_CMCR_ARE;
    
    // Enable the D-PHY data lane
    DSI->PCTLR |= DSI_PCTLR_DEN;
    
    // Enable the D-PHY clock lane
    DSI->PCTLR |= DSI_PCTLR_CKE;
    
    // Enable the DSI Host
    DSI->CR |= DSI_CR_EN;
    
    // Enable the DSI wrapper
    DSI->WCR |= DSI_WCR_DSIEN;   
    /* End DSI configuration */
    
    // Send DCS commands through the APB generic interface to configure the display
    /* OTM8009A power up sequence */
    const uint8_t lcdRegData1[]  = {0x80,0x09,0x01,0xFF};
    const uint8_t lcdRegData2[]  = {0x80,0x09,0xFF};
    const uint8_t lcdRegData3[]  = {0x00,0x09,0x0F,0x0E,0x07,0x10,0x0B,0x0A,0x04,0x07,0x0B,0x08,0x0F,0x10,0x0A,0x01,0xE1};
    const uint8_t lcdRegData4[]  = {0x00,0x09,0x0F,0x0E,0x07,0x10,0x0B,0x0A,0x04,0x07,0x0B,0x08,0x0F,0x10,0x0A,0x01,0xE2};
    const uint8_t lcdRegData5[]  = {0x79,0x79,0xD8};
    const uint8_t lcdRegData6[]  = {0x00,0x01,0xB3};
    const uint8_t lcdRegData7[]  = {0x85,0x01,0x00,0x84,0x01,0x00,0xCE};
    const uint8_t lcdRegData8[]  = {0x18,0x04,0x03,0x39,0x00,0x00,0x00,0x18,0x03,0x03,0x3A,0x00,0x00,0x00,0xCE};
    const uint8_t lcdRegData9[]  = {0x18,0x02,0x03,0x3B,0x00,0x00,0x00,0x18,0x01,0x03,0x3C,0x00,0x00,0x00,0xCE};
    const uint8_t lcdRegData10[] = {0x01,0x01,0x20,0x20,0x00,0x00,0x01,0x02,0x00,0x00,0xCF};
    const uint8_t lcdRegData11[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xCB};
    const uint8_t lcdRegData12[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xCB};
    const uint8_t lcdRegData13[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xCB};
    const uint8_t lcdRegData14[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xCB};
    const uint8_t lcdRegData15[] = {0x00,0x04,0x04,0x04,0x04,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xCB};
    const uint8_t lcdRegData16[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x04,0x04,0x04,0x04,0x00,0x00,0x00,0x00,0xCB};
    const uint8_t lcdRegData17[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xCB};
    const uint8_t lcdRegData18[] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xCB};
    const uint8_t lcdRegData19[] = {0x00,0x26,0x09,0x0B,0x01,0x25,0x00,0x00,0x00,0x00,0xCC};
    const uint8_t lcdRegData20[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x26,0x0A,0x0C,0x02,0xCC};
    const uint8_t lcdRegData21[] = {0x25,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xCC};
    const uint8_t lcdRegData22[] = {0x00,0x25,0x0C,0x0A,0x02,0x26,0x00,0x00,0x00,0x00,0xCC};
    const uint8_t lcdRegData23[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x25,0x0B,0x09,0x01,0xCC};
    const uint8_t lcdRegData24[] = {0x26,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xCC};
    const uint8_t lcdRegData25[] = {0xFF,0xFF,0xFF,0xFF};    
    
    /*
     * CASET value (Column Address Set): X direction LCD GRAM boundaries depending on LCD orientation mode
     * PASET value (Page Address Set): Y direction LCD GRAM boundaries depending on LCD orientation mode
     *
     * XS[15:0] = 0x000 = 0, XE[15:0] = 0x31F = 799 for landscape mode: apply to CASET
     * YS[15:0] = 0x000 = 0, YE[15:0] = 0x31F = 799 for portrait mode: apply to PASET
     *
     * XS[15:0] = 0x000 = 0, XE[15:0] = 0x1DF = 479 for portrait mode: apply to CASET
     * YS[15:0] = 0x000 = 0, YE[15:0] = 0x1DF = 479 for landscape mode: apply to PASET
     */    
    
//     #if defined MXGUI_ORIENTATION_VERTICAL
//     const uint8_t lcdRegData27[] = {0x00, 0x00, 0x03, 0x1F, 0x2B};
//     const uint8_t lcdRegData28[] = {0x00, 0x00, 0x01, 0xDF, 0x2A};
//     #elif defined MXGUI_ORIENTATION_HORIZONTAL
//     const uint8_t lcdRegData27[] = {0x00, 0x00, 0x03, 0x1F, 0x2A};
//     const uint8_t lcdRegData28[] = {0x00, 0x00, 0x01, 0xDF, 0x2B};
//     #endif

    const uint8_t ShortRegData1[]  = {0x00, 0x00};
    const uint8_t ShortRegData2[]  = {0x00, 0x80};
    const uint8_t ShortRegData3[]  = {0xC4, 0x30};
    const uint8_t ShortRegData4[]  = {0x00, 0x8A};
    const uint8_t ShortRegData5[]  = {0xC4, 0x40};
    const uint8_t ShortRegData6[]  = {0x00, 0xB1};
    const uint8_t ShortRegData7[]  = {0xC5, 0xA9};
    const uint8_t ShortRegData8[]  = {0x00, 0x91};
    const uint8_t ShortRegData9[]  = {0xC5, 0x34};
    const uint8_t ShortRegData10[] = {0x00, 0xB4};
    const uint8_t ShortRegData11[] = {0xC0, 0x50};
    const uint8_t ShortRegData12[] = {0xD9, 0x4E};
    const uint8_t ShortRegData13[] = {0x00, 0x81};
    const uint8_t ShortRegData14[] = {0xC1, 0x66};
    const uint8_t ShortRegData15[] = {0x00, 0xA1};
    const uint8_t ShortRegData16[] = {0xC1, 0x08};
    const uint8_t ShortRegData17[] = {0x00, 0x92};
    const uint8_t ShortRegData18[] = {0xC5, 0x01};
    const uint8_t ShortRegData19[] = {0x00, 0x95};
    const uint8_t ShortRegData20[] = {0x00, 0x94};
    const uint8_t ShortRegData21[] = {0xC5, 0x33};
    const uint8_t ShortRegData22[] = {0x00, 0xA3};
    const uint8_t ShortRegData23[] = {0xC0, 0x1B};
    const uint8_t ShortRegData24[] = {0x00, 0x82};
    const uint8_t ShortRegData25[] = {0xC5, 0x83};
    const uint8_t ShortRegData26[] = {0xC4, 0x83};
    const uint8_t ShortRegData27[] = {0xC1, 0x0E};
    const uint8_t ShortRegData28[] = {0x00, 0xA6};
    const uint8_t ShortRegData29[] = {0x00, 0xA0};
    const uint8_t ShortRegData30[] = {0x00, 0xB0};
    const uint8_t ShortRegData31[] = {0x00, 0xC0};
    const uint8_t ShortRegData32[] = {0x00, 0xD0};
    const uint8_t ShortRegData33[] = {0x00, 0x90};
    const uint8_t ShortRegData34[] = {0x00, 0xE0};
    const uint8_t ShortRegData35[] = {0x00, 0xF0};
    const uint8_t ShortRegData36[] = {0x11, 0x00};
    const uint8_t ShortRegData37[] = {0x3A, 0x55};
//     const uint8_t ShortRegData38[] = {0x3A, 0x77};
    
//     #if defined MXGUI_ORIENTATION_VERTICAL
//     const uint8_t ShortRegData39[] = {0x36, 0x00};
//     #elif defined MXGUI_ORIENTATION_HORIZONTAL
//     const uint8_t ShortRegData39[] = {0x36, 0x60};
//     #endif
    
    const uint8_t ShortRegData40[] = {0x51, 0xFF};  /* Draupner: Brightness changed from 0x7F */
    const uint8_t ShortRegData41[] = {0x53, 0x2C};
    const uint8_t ShortRegData42[] = {0x55, 0x02};
    const uint8_t ShortRegData43[] = {0x5E, 0xFF};
    const uint8_t ShortRegData44[] = {0x29, 0x00};
    const uint8_t ShortRegData45[] = {0x2C, 0x00};
    const uint8_t ShortRegData46[] = {0xCF, 0x00};
    const uint8_t ShortRegData47[] = {0xC5, 0x66};
    const uint8_t ShortRegData48[] = {0x00, 0xB6};
    const uint8_t ShortRegData49[] = {0xF5, 0x06};
    
    sendCmd(0, (uint8_t *)ShortRegData1);
    sendCmd(3, (uint8_t *)lcdRegData1);
    sendCmd(0, (uint8_t *)ShortRegData2);
    sendCmd(2, (uint8_t *)lcdRegData2);
    sendCmd(0, (uint8_t *)ShortRegData2);
    sendCmd(0, (uint8_t *)ShortRegData3);
    Thread::sleep(10);
    sendCmd(0, (uint8_t *)ShortRegData4);
    sendCmd(0, (uint8_t *)ShortRegData5);
    Thread::sleep(10);
    sendCmd(0, (uint8_t *)ShortRegData6);
    sendCmd(0, (uint8_t *)ShortRegData7);
    sendCmd(0, (uint8_t *)ShortRegData8);
    sendCmd(0, (uint8_t *)ShortRegData9);
    sendCmd(0, (uint8_t *)ShortRegData10);
    sendCmd(0, (uint8_t *)ShortRegData11);
    sendCmd(0, (uint8_t *)ShortRegData1);
    sendCmd(0, (uint8_t *)ShortRegData12);
    sendCmd(0, (uint8_t *)ShortRegData13);
    sendCmd(0, (uint8_t *)ShortRegData14);
    sendCmd(0, (uint8_t *)ShortRegData15);
    sendCmd(0, (uint8_t *)ShortRegData16);
    sendCmd(0, (uint8_t *)ShortRegData17);
    sendCmd(0, (uint8_t *)ShortRegData18);
    sendCmd(0, (uint8_t *)ShortRegData19);
    sendCmd(0, (uint8_t *)ShortRegData9);
    sendCmd(0, (uint8_t *)ShortRegData1);
    sendCmd(2, (uint8_t *)lcdRegData5);
    sendCmd(0, (uint8_t *)ShortRegData20);
    sendCmd(0, (uint8_t *)ShortRegData21);
    sendCmd(0, (uint8_t *)ShortRegData22);
    sendCmd(0, (uint8_t *)ShortRegData23);
    sendCmd(0, (uint8_t *)ShortRegData24);
    sendCmd(0, (uint8_t *)ShortRegData25);
    sendCmd(0, (uint8_t *)ShortRegData13);
    sendCmd(0, (uint8_t *)ShortRegData26);
    sendCmd(0, (uint8_t *)ShortRegData15);
    sendCmd(0, (uint8_t *)ShortRegData27);
    sendCmd(0, (uint8_t *)ShortRegData28);
    sendCmd(2, (uint8_t *)lcdRegData6);
    sendCmd(0, (uint8_t *)ShortRegData2);
    sendCmd(6, (uint8_t *)lcdRegData7);
    sendCmd(0, (uint8_t *)ShortRegData29);
    sendCmd(14, (uint8_t *)lcdRegData8);
    sendCmd(0, (uint8_t *)ShortRegData30);
    sendCmd(14, (uint8_t *)lcdRegData9);
    sendCmd(0, (uint8_t *)ShortRegData31);
    sendCmd(10, (uint8_t *)lcdRegData10);
    sendCmd(0, (uint8_t *)ShortRegData32);
    sendCmd(0, (uint8_t *)ShortRegData46);
    sendCmd(0, (uint8_t *)ShortRegData2);
    sendCmd(10, (uint8_t *)lcdRegData11);
    sendCmd(0, (uint8_t *)ShortRegData33);
    sendCmd(15, (uint8_t *)lcdRegData12);
    sendCmd(0, (uint8_t *)ShortRegData29);
    sendCmd(15, (uint8_t *)lcdRegData13);
    sendCmd(0, (uint8_t *)ShortRegData30);
    sendCmd(10, (uint8_t *)lcdRegData14);
    sendCmd(0, (uint8_t *)ShortRegData31);
    sendCmd(15, (uint8_t *)lcdRegData15);
    sendCmd(0, (uint8_t *)ShortRegData32);
    sendCmd(15, (uint8_t *)lcdRegData16);
    sendCmd(0, (uint8_t *)ShortRegData34);
    sendCmd(10, (uint8_t *)lcdRegData17);
    sendCmd(0, (uint8_t *)ShortRegData35);
    sendCmd(10, (uint8_t *)lcdRegData18);
    sendCmd(0, (uint8_t *)ShortRegData2);
    sendCmd(10, (uint8_t *)lcdRegData19);
    sendCmd(0, (uint8_t *)ShortRegData33);
    sendCmd(15, (uint8_t *)lcdRegData20);
    sendCmd(0, (uint8_t *)ShortRegData29);
    sendCmd(15, (uint8_t *)lcdRegData21);
    sendCmd(0, (uint8_t *)ShortRegData30);
    sendCmd(10, (uint8_t *)lcdRegData22);
    sendCmd(0, (uint8_t *)ShortRegData31);
    sendCmd(15, (uint8_t *)lcdRegData23);
    sendCmd(0, (uint8_t *)ShortRegData32);
    sendCmd(15, (uint8_t *)lcdRegData24);
    sendCmd(0, (uint8_t *)ShortRegData13);
    sendCmd(0, (uint8_t *)ShortRegData47);
    sendCmd(0, (uint8_t *)ShortRegData48);
    sendCmd(0, (uint8_t *)ShortRegData49);
    sendCmd(0, (uint8_t *)ShortRegData1);
    sendCmd(3, (uint8_t *)lcdRegData25);
    sendCmd(0, (uint8_t *)ShortRegData1);      
    sendCmd(0, (uint8_t *)ShortRegData1);
    sendCmd(16, (uint8_t *)lcdRegData3);
    sendCmd(0, (uint8_t *)ShortRegData1);
    sendCmd(16, (uint8_t *)lcdRegData4);
    sendCmd(0, (uint8_t *)ShortRegData36);
    Thread::sleep(120);
    
    /* Set Pixel color format to RGB565 */
    sendCmd(0, (uint8_t *)ShortRegData37);
    /* Set Pixel color format to RGB888 */
    //sendCmd(0, (uint8_t *)ShortRegData38);
    
    #if defined MXGUI_ORIENTATION_HORIZONTAL
    /* Send command to configure display in landscape orientation mode. 
       By default the orientation mode is portrait. */
    sendCmd(0, (uint8_t *)ShortRegData39);
    sendCmd(4, (uint8_t *)lcdRegData27);
    sendCmd(4, (uint8_t *)lcdRegData28);
    #endif
    
    sendCmd(0, (uint8_t *)ShortRegData40);
    sendCmd(0, (uint8_t *)ShortRegData41);
    sendCmd(0, (uint8_t *)ShortRegData42);
    sendCmd(0, (uint8_t *)ShortRegData43);
    sendCmd(0, (uint8_t *)ShortRegData44);
    sendCmd(0, (uint8_t *)ShortRegData1);
    sendCmd(0, (uint8_t *)ShortRegData45);
    /* End OTM8009A power up sequence */
    
    // Disable command trasmission only in low power mode
    DSI->CMCR &= ~(DSI_CMCR_GSW0TX | \
                   DSI_CMCR_GSW1TX | \
                   DSI_CMCR_GSW2TX | \
                   DSI_CMCR_GSR0TX | \
                   DSI_CMCR_GSR1TX | \
                   DSI_CMCR_GSR2TX | \
                   DSI_CMCR_GLWTX  | \
                   DSI_CMCR_DSW0TX | \
                   DSI_CMCR_DSW1TX | \
                   DSI_CMCR_DSR0TX | \
                   DSI_CMCR_DLWTX  | \
                   DSI_CMCR_MRDPS);
    
    DSI->PCR &= ~(DSI_PCR_CRCRXE | DSI_PCR_ECCRXE | \
                  DSI_PCR_BTAE | DSI_PCR_ETRXE | \
                  DSI_PCR_ETTXE);
    DSI->PCR |= DSI_PCR_BTAE;
    
    // Enable the LTDC
    LTDC->GCR |= LTDC_GCR_LTDCEN;

    // Start the LTDC flow through the DSI wrapper (CR.LTDCEN = 1).
    // In video mode, the data streaming starts as soon as the LTDC is enabled.
    // In adapted command mode, the frame buffer update is launched as soon as the CR.LTDCEN bit is set.
    DSI->CR |= DSI_CR_EN;
    
    // Update the display
    DSI->WCR |= DSI_WCR_LTDCEN;

    setFont(droid11);
    setTextColor(make_pair(Color(0xffff), Color(0x0000)));
    clear(black);
}

Color DisplayImpl::pixel_iterator::dummy;

} //namespace mxgui

#endif //_BOARD_STM32F469NI_STM32F469I_DISCO
