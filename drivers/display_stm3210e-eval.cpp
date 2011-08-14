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

#ifdef _BOARD_STM3210E_EVAL

#include "display_stm3210e-eval.h"
#include "miosix.h"
#include <cstdio>

using namespace miosix;

namespace mxgui {

//
// Debug mode printf
//

//#define DBG iprintf
#define DBG (void)

//
// Class DisplayStm32e_eval
//

DisplayImpl::DisplayImpl(): displayType(UNKNOWN), textColor(),
        font(droid11)
{
    //FIXME: This assumes xram is already initialized an so D0..D15, A0, NOE,
    //NWE are correctly initialized

    //Set portG12 (Display CS) as alternate function push pull 50MHz
    Gpio<GPIOG_BASE,12>::mode(Mode::ALTERNATE);

    //The way BCR and BTR are specified in stm32f10x.h sucks, trying to work
    //around it...
    volatile uint32_t& BCR4=FSMC_Bank1->BTCR[6];
    volatile uint32_t& BTR4=FSMC_Bank1->BTCR[7];
    volatile uint32_t& BWTR4=FSMC_Bank1E->BWTR[6];

    //Timings for spfd5408 and ili9320

    //Write burst disabled, Extended mode enabled, Wait signal disabled
    //Write enabled, Wait signal active before wait state, Wrap disabled
    //Burst disabled, Data width 16bit, Memory type SRAM, Data mux disabled
    BCR4 = FSMC_BCR4_WREN | FSMC_BCR4_MWID_0 | FSMC_BCR4_MBKEN | FSMC_BCR4_EXTMOD;
    // Write timings
    //Address setup=0, Data setup=4, Access mode=A
    BWTR4 = FSMC_BTR4_DATAST_2;
    // Read timings
    //Address setup=13, Data setup=10, Access mode=A
    BTR4 = FSMC_BTR4_DATAST_3 | FSMC_BTR4_DATAST_1 | FSMC_BTR4_ADDSET_3 |
           FSMC_BTR4_ADDSET_2 | FSMC_BTR4_ADDSET_0;
    
    //Detect what kind of display controller we are interfacing with and init it
    delayMs(10);
    unsigned short id=readReg(0);
    DBG("Display ID is 0x%x: ",id);
    switch(id)
    {
        case 0x5408:
            DBG("SPFD5408\n");
            initSPFD5408();
            displayType=SPFD5408;
            break;
        case 0x9320:
            DBG("ILI9320 (untested, use at your own risk)\n");
            initILI9320();
            displayType=ILI9320;
            break;
        default:
            DBG("uh oh, unknown device\n");
            displayType=UNKNOWN;
            break;
    }

    setTextColor(Color(0xffff),Color(0x0000));
    clear(black);
}

void DisplayImpl::clear(Point p1, Point p2, Color color)
{
    imageWindow(p1,p2);
    writeIdx(0x22);//Write to GRAM
    int numPixels=(p2.x()-p1.x()+1)*(p2.y()-p1.y()+1);
    for(int i=0;i<numPixels;i++) writeRam(color);
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
    switch(displayType)
    {
        case SPFD5408:
            writeReg(0x10,0x0);//Out of deep standby
            initSPFD5408();//Everything is lost, so initialze again
            break;
        case ILI9320:
            //Unimplemented as I don't have this kind of display
            break;
        default:
            break;
    }
}

void DisplayImpl::turnOff()
{
    switch(displayType)
    {
        case SPFD5408:
            delayMs(30);
            writeReg(0x10,0x280);//SAP & AP[1:0] @ 0
            delayMs(15);
            writeReg(0x10,0x200);//APE @ 0
            writeReg(0x10,0x4);//Deep standby
            break;
        case ILI9320:
            //Unimplemented as I don't have this kind of display
            break;
        default:
            break;
    }
}

DisplayImpl::pixel_iterator DisplayImpl::begin(Point p1,
        Point p2, IteratorDirection d)
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

void DisplayImpl::initSPFD5408()
{
    //Start Initial Sequence
    writeReg(1, 0x0100);  //Set SS bit
    writeReg(2, 0x0700);  //Set 1 line inversion
    writeReg(3, 0x1030);  //Set GRAM write direction and BGR=1
    writeReg(4, 0x0000);  //Resize register
    writeReg(8, 0x0202);  //Set the back porch and front porch
    writeReg(9, 0x0000);  //Set non-display area refresh cycle ISC[3:0]
    writeReg(10, 0x0000); //FMARK function
    writeReg(12, 0x0000); //RGB 18-bit System interface setting
    writeReg(13, 0x0000); //Frame marker Position
    writeReg(15, 0x0000); //RGB interface polarity, no impact
    //Power On sequence
    writeReg(16, 0x0000); //SAP, BT[3:0], AP, DSTB, SLP, STB
    writeReg(17, 0x0000); //DC1[2:0], DC0[2:0], VC[2:0]
    writeReg(18, 0x0000); //VREG1OUT voltage
    writeReg(19, 0x0000); //VDV[4:0] for VCOM amplitude
    delayMs(200);//Discharge capacitors (200ms)
    writeReg(17, 0x0007);	//DC1[2:0], DC0[2:0], VC[2:0]
    delayMs(50);
    writeReg(16, 0x12B0);	//SAP, BT[3:0], AP, DSTB, SLP, STB
    delayMs(50);
    writeReg(18, 0x01BD);  //External reference voltage= Vci
    delayMs(50);
    writeReg(19, 0x1400);  //VDV[4:0] for VCOM amplitude
    writeReg(41, 0x000E);  //VCM[4:0] for VCOMH
    delayMs(50);
    writeReg(32, 0x0000); //GRAM horizontal Address
    writeReg(33, 0x013F); //GRAM Vertical Address
    //Adjust the Gamma Curve (SPFD5408B)
    writeReg(48, 0x0b0d);
    writeReg(49, 0x1923);
    writeReg(50, 0x1c26);
    writeReg(51, 0x261c);
    writeReg(52, 0x2419);
    writeReg(53, 0x0d0b);
    writeReg(54, 0x1006);
    writeReg(55, 0x0610);
    writeReg(56, 0x0706);
    writeReg(57, 0x0304);
    writeReg(58, 0x0e05);
    writeReg(59, 0x0e01);
    writeReg(60, 0x010e);
    writeReg(61, 0x050e);
    writeReg(62, 0x0403);
    writeReg(63, 0x0607);
    //Set GRAM area
    writeReg(80, 0x0000); //Horizontal GRAM Start Address
    writeReg(81, 0x00EF); //Horizontal GRAM End Address
    writeReg(82, 0x0000); //Vertical GRAM Start Address
    writeReg(83, 0x013F); //Vertical GRAM End Address
    writeReg(96,  0xA700); //Gate Scan Line
    writeReg(97,  0x0001); //NDL, VLE, REV
    writeReg(106, 0x0000); //set scrolling line
    //Partial Display Control
    writeReg(128, 0x0000);
    writeReg(129, 0x0000);
    writeReg(130, 0x0000);
    writeReg(131, 0x0000);
    writeReg(132, 0x0000);
    writeReg(133, 0x0000);
    //Panel Control
    writeReg(144, 0x0010);
    writeReg(146, 0x0000);
    writeReg(147, 0x0003);
    writeReg(149, 0x0110);
    writeReg(151, 0x0000);
    writeReg(152, 0x0000);
    //Set GRAM write direction and BGR=1
    //I/D=01 (Horizontal : increment, Vertical : decrement)
    //AM=1 (address is updated in vertical writing direction)
    writeReg(3, 0x1018);
    writeReg(7, 0x0112); //262K color and display ON
}

void DisplayImpl::initILI9320()
{
    //Start Initial Sequence
    writeReg(229,0x8000); //Set the internal vcore voltage
    writeReg(0,  0x0001); //Start internal OSC.
    writeReg(1,  0x0100); //set SS and SM bit
    writeReg(2,  0x0700); //set 1 line inversion
    writeReg(3,  0x1030); //set GRAM write direction and BGR=1.
    writeReg(4,  0x0000); //Resize register
    writeReg(8,  0x0202); //set the back porch and front porch
    writeReg(9,  0x0000); //set non-display area refresh cycle ISC[3:0]
    writeReg(10, 0x0000); //FMARK function
    writeReg(12, 0x0000); //RGB interface setting
    writeReg(13, 0x0000); //Frame marker Position
    writeReg(15, 0x0000); //RGB interface polarity
    //Power On sequence
    writeReg(16, 0x0000); //SAP, BT[3:0], AP, DSTB, SLP, STB
    writeReg(17, 0x0000); //DC1[2:0], DC0[2:0], VC[2:0]
    writeReg(18, 0x0000); //VREG1OUT voltage
    writeReg(19, 0x0000); //VDV[4:0] for VCOM amplitude
    delayMs(200);//Discharge capacitors (200ms)
    writeReg(16, 0x17B0); //SAP, BT[3:0], AP, DSTB, SLP, STB
    writeReg(17, 0x0137); //DC1[2:0], DC0[2:0], VC[2:0]
    delayMs(50);
    writeReg(18, 0x0139); //VREG1OUT voltage
    delayMs(50);
    writeReg(19, 0x1d00); //VDV[4:0] for VCOM amplitude
    writeReg(41, 0x0013); //VCM[4:0] for VCOMH
    delayMs(50);
    writeReg(32, 0x0000); //GRAM horizontal Address
    writeReg(33, 0x0000); //GRAM Vertical Address
    //Adjust the Gamma Curve
    writeReg(48, 0x0006);
    writeReg(49, 0x0101);
    writeReg(50, 0x0003);
    writeReg(53, 0x0106);
    writeReg(54, 0x0b02);
    writeReg(55, 0x0302);
    writeReg(56, 0x0707);
    writeReg(57, 0x0007);
    writeReg(60, 0x0600);
    writeReg(61, 0x020b);
    //Set GRAM area
    writeReg(80, 0x0000); //Horizontal GRAM Start Address
    writeReg(81, 0x00EF); //Horizontal GRAM End Address
    writeReg(82, 0x0000); //Vertical GRAM Start Address
    writeReg(83, 0x013F); //Vertical GRAM End Address
    writeReg(96,  0x2700); //Gate Scan Line
    writeReg(97,  0x0001); //NDL,VLE, REV
    writeReg(106, 0x0000); //set scrolling line
    //Partial Display Control
    writeReg(128, 0x0000);
    writeReg(129, 0x0000);
    writeReg(130, 0x0000);
    writeReg(131, 0x0000);
    writeReg(132, 0x0000);
    writeReg(133, 0x0000);
    //Panel Control
    writeReg(144, 0x0010);
    writeReg(146, 0x0000);
    writeReg(147, 0x0003);
    writeReg(149, 0x0110);
    writeReg(151, 0x0000);
    writeReg(152, 0x0000);
    //Set GRAM write direction and BGR = 1
    //I/D=01 (Horizontal : increment, Vertical : decrement)
    //AM=1 (address is updated in vertical writing direction)
    writeReg(3, 0x1018);
    writeReg(7, 0x0173); //262K color and display ON
}

DisplayImpl::DisplayMemLayout *const DisplayImpl::DISPLAY=
        reinterpret_cast<DisplayImpl::DisplayMemLayout*>(0x6c000000);

} //namespace mxgui

#endif //_BOARD_STM3210E_EVAL
