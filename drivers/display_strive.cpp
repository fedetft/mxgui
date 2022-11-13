/***************************************************************************
 *   Copyright (C) 2011 by Yury Kuchura
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

#include "display_strive.h"
#include "miosix.h"

using namespace std;
using namespace miosix;

#ifdef _BOARD_STRIVE_MINI

//Helper to set gamma register values
#define GREGVAL(byte1, byte2) (uint16_t)(((byte1) << 8) | ((byte2) & 0xFF))
//Helper for bit mask
#define b(n) (1<<(n))

namespace mxgui {

//IL9325 bits
enum
{
    //Reg 01h Driver Output control
    SS=b(8), SM=b(10),
    //Reg 02h LCD Driving control
    EOR=b(8), BC0=b(9),
    //Reg 03h Entry mode
    AM=b(3), ID0=b(4), ID1=b(5), ORG=b(7), HWM=b(9), BGR=b(12),
    DFM=b(14), TRI=b(15),
    //Reg 04h Resize control
    RSZ0=b(0), RSZ1=b(1), RCH0=b(4), RCH1=b(5), RCV0=b(8), RCV1=b(9),
    //Reg 07h Display control 1
    D0=b(0), D1=b(1), CL=b(3), DTE=b(4), GON=b(5), BASEE=b(8),
    PTDE0=b(12), PTDE1=b(13),
    //Reg 08h Display Control 2
    BP0=b(0), BP1=b(1), BP2=b(2), BP3=b(3),
    FP0=b(8), FP1=b(9), FP2=b(10), FP3=b(11),
    //Reg 09h Display Control 3
    ISC0=b(0), ISC1=b(1), ISC2=b(2), ISC3=b(3),
    PTG0=b(4), PTG1=b(5),
    PTS0=b(8), PTS1=b(9), PTS2=b(10),
    //Reg 0Ah Display Control 4
    FMI0=b(0), FMI1=b(1), FMI2=b(3), FMARKOE=b(3),
    //Reg 0Ch RGB Display Interface Control 1
    ENC2=b(14), ENC1=b(13), ENC0=b(12), RM=b(8), DM1=b(5), DM0=b(4),
    RIM1=b(1), RIM0=b(0),
    //Reg 0Dh Frame Maker Position
    FMP8=b(8), FMP7=b(7), FMP6=b(6), FMP5=b(5), FMP4=b(4), FMP3=b(3),
    FMP2=b(2), FMP1=b(1), FMP0=b(0),
    //Reg 0Fh RGB Display Interface Control 2
    EPL=b(0), DPL=b(1), HSPL=b(3), VSPL=b(4),
    //Reg 10h Power Control 1
    SAP=b(12), BT2=b(10), BT1=b(9), BT0=b(8), APE=b(7),
    AP2=b(6), AP1=b(5), AP0=b(4), DSTB=b(2), SLP=b(1), STB=b(0),
    //Reg 11h Power Control 2
    DC12=b(10), DC11=b(9), DC10=b(8), DC02=b(6), DC01=b(5), DC00=b(4),
    VC2=b(2), VC1=b(1), VC0=b(0),
    //Reg 12h Power Control 3
    VCIRE=b(7), PON=b(4), VRH3=b(3), VRH2=b(2), VRH1=b(1), VRH0=b(0),
    //Reg 13h Power Control 4
    VDV4=b(12), VDV3=b(11), VDV2=b(10), VDV1=b(9), VDV0=b(8),
    //Reg 20h Horizontal GRAM Address Set (0..239)
    //Reg 21h Vertical GRAM Address Set (0...319)
    //Reg 22h Write Data to GRAM
    //Reg 29h Power Control 7
    VCM5=b(5), VCM4=b(4), VCM3=b(3), VCM2=b(2), VCM1=b(1), VCM0=b(0),
    //Reg 2Bh Frame Rate and Color Control
    FRS3=b(3), FRS2=b(2), FRS1=b(1), FRS0=b(0),
    //Reg 30h Gamma Control 1:  GREGVAL(KP1, KP0)
    //Reg 31h Gamma Control 2:  GREGVAL(KP3, KP2)
    //Reg 32h Gamma Control 3:  GREGVAL(KP5, KP4)
    //Reg 35h Gamma Control 4:  GREGVAL(RP1, RP0)
    //Reg 36h Gamma Control 5:  GREGVAL(VRP1, VRP0)
    //Reg 37h Gamma Control 6:  GREGVAL(KN1, KN0)
    //Reg 38h Gamma Control 7:  GREGVAL(KN3, KN2)
    //Reg 39h Gamma Control 8:  GREGVAL(KN5, KN4)
    //Reg 3Ch Gamma Control 9:  GREGVAL(RN1, RN0)
    //Reg 3Dh Gamma Control 10: GREGVAL(VRN1, VRN0)
    //Reg 50h Horizontal Address Start Position (0...239)
    //Reg 51h Horizontal Address End Position (0...239)
    //Reg 52h Vertical Address Start Position (0...319)
    //Reg 53h Vertical Address End Position (0...319)
    //Reg 60h Driver Output Control 2
    GS=b(15), NL5=b(13), NL4=b(12), NL3=b(11), NL2=b(10), NL1=b(9), NL0=b(8),
    SCN5=b(5), SCN4=b(4), SCN3=b(3), SCN2=b(2), SCN1=b(1), SCN0=b(0),
    //Reg 61h Base Image Display Control
    NDL=b(2), VLE=b(1), REV=b(0),
    //Reg 6Ah Vertical Scroll Control (0...319)
    //Reg 80h Partial Image 1 Display Position  (9 bits) PTDP00..08
    //Reg 81h Partial Image 1 Area (Start Line) (9 bits) PTSA00..08
    //Reg 82h Partial Image 1 Area (End Line) (9 bits) PTEA00..08
    //Reg 83h Partial Image 2 Display Position (9 bits) PTDP10..18
    //Reg 84h Partial Image 2 Area (Start Line) (9 bits) PTSA10..18
    //Reg 85h Partial Image 2 Area (End Line) (9 bits) PTEA10..18
    //Reg 90h Panel Interface Control 1
    DIVI1=b(9), DIVI00=b(8), RTNI3=b(3), RTNI2=b(2), RTNI1=b(1), RTNI0=b(0),
    //Reg 92h Panel Interface Control 2
    NOWI2=b(10), NOWI1=b(9), NOWI0=b(8),
    //Reg 95h Panel Interface Control 4
    DIVE1=b(9), DIVE0=b(8),
    RTNE5=b(5), RTNE4=b(4), RTNE3=b(3), RTNE2=b(2), RTNE1=b(1), RTNE0=b(0),
    //Reg A1h OTP VCM Programming Control OTP_PGM_EN + 6 bits (VCM_OTP0..5)
    OTP_PGM_EN=b(11),
    //Reg A2h OTP VCM Status and Enable
    PGM_CNT1=b(15), PGM_CNT0=b(14),
    VCM_D5=b(13), VCM_D4=b(12), VCM_D3=b(11), VCM_D2=b(10),
    VCM_D1=b(9), VCM_D0=b(8),
    VCM_EN=b(0),
    //Reg A5h OTP Programming ID Key (16 bits)
}; //enum

void registerDisplayHook(DisplayManager& dm)
{
    dm.registerDisplay(&DisplayImpl::instance());
}

//
// Class DisplayImpl
//
const short int DisplayImpl::width;
const short int DisplayImpl::height;

DisplayImpl& DisplayImpl::instance()
{
    static DisplayImpl instance;
    return instance;
}

void DisplayImpl::doTurnOn()
{
    writeReg(0x10, 0); //Exit standby mode
    writeReg(0x10, SAP | BT2 | BT1 | APE | AP0); //Enable power supply circuits
    disp::ncpEn::high(); //Backlight on
}

void DisplayImpl::doTurnOff()
{
    disp::ncpEn::low(); //Backlight off
    writeReg(0x10, STB);//Standby (Only GRAM continues operation).
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
    imageWindow(p1,p2);
    writeIdx(0x22);//Write to GRAM
    int numPixels=(p2.x()-p1.x()+1)*(p2.y()-p1.y()+1);
    int fastPixels=numPixels/2;
    unsigned int twoPixColor=color | color<<16;
    for(int i=0;i<fastPixels;i++) DISPLAY->TWOPIX_RAM=twoPixColor;
    if(numPixels & 0x1) writeRam(color);
}

void DisplayImpl::beginPixel() {}

void DisplayImpl::setPixel(Point p, Color color)
{
    setCursor(p);
    writeIdx(0x22);//Write to GRAM
    writeRam(color);
}

void DisplayImpl::line(Point a, Point b, Color color)
{
    //Horizontal line speed optimization
    if(a.y()==b.y())
    {
        imageWindow(Point(min(a.x(),b.x()),a.y()),
                    Point(max(a.x(),b.x()),a.y()));
        writeIdx(0x22);//Write to GRAM
        int numPixels=abs(a.x()-b.x());
        int fastPixels=numPixels/2;
        unsigned int twoPixColor=color | color<<16;
        for(int i=0;i<=fastPixels;i++) DISPLAY->TWOPIX_RAM=twoPixColor;
        if(numPixels & 0x1) writeRam(color);
        return;
    }
    //Vertical line speed optimization
    if(a.x()==b.x())
    {
        textWindow(Point(a.x(),min(a.y(),b.y())),
                    Point(a.x(),max(a.y(),b.y())));
        writeIdx(0x22);//Write to GRAM
        int numPixels=abs(a.y()-b.y());
        int fastPixels=numPixels/2;
        unsigned int twoPixColor=color | color<<16;
        for(int i=0;i<=fastPixels;i++) DISPLAY->TWOPIX_RAM=twoPixColor;
        if(numPixels & 0x1) writeRam(color);
        return;
    }
    //General case, always works but it is much slower due to the display
    //not having fast random access to pixels
    Line::draw(*this,a,b,color);
}

void DisplayImpl::scanLine(Point p, const Color *colors, unsigned short length)
{
    imageWindow(p,Point(width-1,p.y()));
    writeIdx(0x22); //Write to GRAM
    int fastPixels=length/2;
    for(int i=0;i<fastPixels;i++)
    {
        unsigned int twoPix=colors[0] | colors[1]<<16;
        DISPLAY->TWOPIX_RAM=twoPix;
        colors+=2;
    }
    if(length & 0x1) writeRam(colors[0]);
}

Color *DisplayImpl::getScanLineBuffer()
{
    if(buffer==0) buffer=new Color[getWidth()];
    return buffer;
}

void DisplayImpl::scanLineBuffer(Point p, unsigned short length)
{
    scanLine(p,buffer,length);
}

void DisplayImpl::drawImage(Point p, const ImageBase& img)
{
    short int xEnd=p.x()+img.getWidth()-1;
    short int yEnd=p.y()+img.getHeight()-1;
    if(xEnd >= width || yEnd >= height) return;

    const unsigned short *imgData=img.getData();
    if(imgData!=0)
    {
        //Optimized version for memory-loaded images
        imageWindow(p,Point(xEnd,yEnd));
        writeIdx(0x22);//Write to GRAM
        int numPixels=img.getHeight()*img.getWidth();
        int fastPixels=numPixels/2;
        for(int i=0;i<fastPixels;i++)
        {
            unsigned int twoPix=imgData[0] | imgData[1]<<16; //Pack two pixel
            DISPLAY->TWOPIX_RAM=twoPix;
            imgData+=2;
        }
        if(numPixels & 0x1) writeRam(imgData[0]);

    } else img.draw(*this,p);
}

void DisplayImpl::clippedDrawImage(Point p, Point a, Point b, const ImageBase& img)
{
    using namespace std;
    if(img.getData()==0)
    {
        img.clippedDraw(*this,p,a,b);
        return;
    } //else optimized version for memory-loaded images

    //Find rectangle wich is the non-empty intersection of the image rectangle
    //with the clip rectangle
    short xa=max(p.x(),a.x());
    short xb=min<short>(p.x()+img.getWidth()-1,b.x());
    if(xa>xb) return; //Empty intersection

    short ya=max(p.y(),a.y());
    short yb=min<short>(p.y()+img.getHeight()-1,b.y());
    if(ya>yb) return; //Empty intersection

    //Draw image
    imageWindow(Point(xa,ya),Point(xb,yb));
    writeIdx(0x22);//Write to GRAM
    short nx=xb-xa+1;
    short ny=yb-ya+1;
    int skipStart=(ya-p.y())*img.getWidth()+(xa-p.x());
    const unsigned short *pix=img.getData()+skipStart;
    int toSkip=(xa-p.x())+((p.x()+img.getWidth()-1)-xb);
    short fastNx=nx/2;
    if((nx & 0x1)==0) //Scanline has odd number of pixels
    {
        for(short i=0;i<ny;i++)
        {
            for(short j=0;j<fastNx;j++)
            {
                unsigned int twoPix=pix[0] | pix[1]<<16; //Pack two pixel
                DISPLAY->TWOPIX_RAM=twoPix;
                pix+=2;
            }
            pix+=toSkip;
        }
    } else {
        for(short i=0;i<ny;i++)
        {
            for(short j=0;j<fastNx;j++)
            {
                unsigned int twoPix=pix[0] | pix[1]<<16; //Pack two pixel
                DISPLAY->TWOPIX_RAM=twoPix;
                pix+=2;
            }
            writeRam(pix[0]);
            pix+=toSkip+1;
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

DisplayImpl::pixel_iterator DisplayImpl::begin(Point p1, Point p2,
        IteratorDirection d)
{
    if(p1.x()<0 || p1.y()<0 || p2.x()<0 || p2.y()<0) return pixel_iterator();
    if(p1.x()>=width || p1.y()>=height || p2.x()>=width || p2.y()>=height)
        return pixel_iterator();
    if(p2.x()<p1.x() || p2.y()<p1.y()) return pixel_iterator();

    if(d==DR) textWindow(p1,p2);
    else imageWindow(p1,p2);
    writeIdx(0x22);//Write to GRAM

    unsigned int numPixels=(p2.x()-p1.x()+1)*(p2.y()-p1.y()+1);
    return pixel_iterator(numPixels);
}

DisplayImpl::~DisplayImpl()
{
    if(buffer) delete[] buffer;
}

DisplayImpl::DisplayImpl(): buffer(0)
{
    
    //LCD connection GPIO should have been initialized at
    //this point. bsp.cpp is responsible for this.
    RCC->AHBENR |= RCC_AHBENR_FSMCEN;

    volatile uint32_t& BCR1=FSMC_Bank1->BTCR[0];
    BCR1 = FSMC_BCR1_MBKEN  | //memory bank enable
           FSMC_BCR1_MTYP_1 | //memory type: NOR
           FSMC_BCR1_MWID_0 | //memory data width: 16
           FSMC_BCR1_WREN   | //Write Operation Enable
           FSMC_BCR1_FACCEN;  //Flash access enable

    volatile uint32_t& BTR1=FSMC_Bank1->BTCR[1];
    BTR1 = 2 << 0 |  // Address setup phase duration
           0 << 4 |  // Address-hold phase duration
           5 << 8 |  // Data-phase duration
           0 << 16 | // Bus turn-around duration
           0 << 20 | // Clock divide ratio
           0 << 24 | // Data latency
           1 << 28;  // Access mode
    
    //Power up
    delayMs(100);
    disp::reset::low();
    delayMs(1);
    disp::reset::high();
    delayMs(10);

    //Turn on backlight
    disp::ncpEn::high();

    //=======================================================================
    // The voodoo begins here. Don't touch the code unless you fully realize
    // what you are doing: the display may be seriously damaged.
    //=======================================================================
    writeReg(0x00, 0x0001); // Start Oscillation
    writeReg(0x01, SS); // SS=1, SM=0
    writeReg(0x02, BC0 | EOR); // BC0=1 EOR=1

    writeReg(0x03, BGR | ID1 | ID0 | AM); //Entry mode
    writeReg(0x04, 0x0000); // Resize register
    writeReg(0x08, 0x0202); // Front and Back Porch (must be >= 2 lines)
    writeReg(0x09, 0x0000); // Normal scan, Scan cycle=0
    writeReg(0x0A, 0x0000); // Output of FMARK signal is disabled
    writeReg(0x0C, RIM0); // System interface, 16 bits, internal clock
    writeReg(0x0D, 0x0000); // Frame marker Position
    writeReg(0x0F, 0x0000); // System interface polarity (CLK, EN)

    //Power-up sequence
    writeReg(0x10, 0x0000);
    writeReg(0x11, VC0|VC1|VC2); //Vci1 = 1.0 * Vci
    writeReg(0x12, 0x0000);      //External reference
    writeReg(0x13, 0x0000);      //VCOM = 0.70 * VREG1OUT
    delayMs(200);                  //Let the voltage stabilize
    writeReg(0x10, SAP | BT2 | BT1 | APE | AP0); //Enable power supply circuits
    writeReg(0x11, VC0 | DC01 | DC11); //Step-up circuits parameters
    delayMs(50);                   // Let it stabilize
    writeReg(0x12, PON | VRH3 | VRH2); //Set grayscale level
    delayMs(50);                   // Let it stabilize
    writeReg(0x13, VDV4 | VDV1 | VDV0); //Set Vcom voltage
    writeReg(0x29, VCM4 | VCM3 | VCM2); // Set VcomH voltage
    writeReg(0x2B, 0x000D); //Set frame rate 93Hz

    //Gamma
    //Setting all values for the linear curve, except RN1 (to increase contrast
    //in the darkest area)
    writeReg(0x30, GREGVAL(3,  3));  //KP1[3] KP0[3]   Fine adjustment positive: grayscale 8, grayscale 1
    writeReg(0x31, GREGVAL(3,  3));  //KP3[3] KP2[3]   Fine adjustment positive: grayscale 43, grayscale 20
    writeReg(0x32, GREGVAL(3,  3));  //KP5[3] KP4[3]   Fine adjustment positive: grayscale 62, grayscale 55
    writeReg(0x35, GREGVAL(1,  1));  //RP1[3] RP0[3]   Gradient adjustment variable resistors, positive
    writeReg(0x36, GREGVAL(15, 15)); //VRP1[5] VRP0[5] Amplitude adjustment variable resistors, positive
    writeReg(0x37, GREGVAL(3,  3));  //KN1[3] KN0[3]   Fine adjustment negative: grayscale 8, grayscale 1
    writeReg(0x38, GREGVAL(3,  3));  //KN3[3] KN2[3]   Fine adjustment negative: grayscale 43, grayscale 20
    writeReg(0x39, GREGVAL(3,  3));  //KN5[3] KN4[3]   Fine adjustment negative: grayscale 62, grayscale 55
    writeReg(0x3C, GREGVAL(7,  1));  //RN1[3] RN0[3]   Gradient adjustment variable resistors, negative
    writeReg(0x3D, GREGVAL(15, 15)); //VRN1[5] VRN0[5] Amplitude adjustment variable resistors, negative

    //GRAM
    writeReg(0x50, 0x0000); // Horizontal GRAM Start Address
    writeReg(0x51, 0x00EF); // Horizontal GRAM End Address
    writeReg(0x52, 0x0000); // Vertical GRAM Start Address
    writeReg(0x53, 0x013F); // Vertical GRAM Start Address
    writeReg(0x60, 0xA700); // Gate Scan Line
    writeReg(0x61, 0x0001); // NDL,VLE, REV
    writeReg(0x6A, 0x0000); // Scrolling line

    //Partial Display
    writeReg(0x80, 0x0000);
    writeReg(0x81, 0x0000);
    writeReg(0x82, 0x0000);
    writeReg(0x83, 0x0000);
    writeReg(0x84, 0x0000);
    writeReg(0x85, 0x0000);

    //Panel Control
    writeReg(0x90, 0x0010);
    writeReg(0x92, 0x0000);
    writeReg(0x93, 0x0003);
    writeReg(0x95, 0x0110);
    writeReg(0x97, 0x0000);
    writeReg(0x98, 0x0000);

    // Display ON
    writeReg(0x07, D0 | D1 |DTE | GON | BASEE);

    //Fill display
    setTextColor(make_pair(white, black));
    clear(black);
}

DisplayImpl::DisplayMemLayout *const DisplayImpl::DISPLAY=
        reinterpret_cast<DisplayImpl::DisplayMemLayout*>(0x60000000);

} //namespace mxgui

#endif //_BOARD_STRIVE_MINI
