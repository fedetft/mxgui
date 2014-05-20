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

static unsigned char spi5sendRev(unsigned char c=0)
{
    SPI5->DR=c;
    while((SPI5->SR & SPI_SR_RXNE)==0) ;
    return SPI5->DR;
}

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

//
// Class DisplayImpl
//

DisplayImpl::DisplayImpl()
    : framebuffer1(reinterpret_cast<unsigned short*>(0xd0600000)),
      buffer(framebuffer1+numPixels), font(droid11)
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
    
    setTextColor(Color(0xffff),Color(0x0000));
    clear(black);
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

void DisplayImpl::drawRectangle(Point a, Point b, Color c)
{
    line(a,Point(b.x(),a.y()),c);
    line(Point(b.x(),a.y()),b,c);
    line(b,Point(a.x(),b.y()),c);
    line(Point(a.x(),b.y()),a,c);
}

void DisplayImpl::turnOn()
{
    LTDC->GCR |= LTDC_GCR_LTDCEN;
    Thread::sleep(40);
    sendCmd(0x29,0); //LCD_DISPLAY_ON
}

void DisplayImpl::turnOff()
{
    sendCmd(0x28,0); //LCD_DISPLAY_OFF
    LTDC->GCR &=~ LTDC_GCR_LTDCEN;
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

Color DisplayImpl::pixel_iterator::dummy;

} //namespace mxgui

#endif //_BOARD_STM32F429ZI_STM32F4DISCOVERY
