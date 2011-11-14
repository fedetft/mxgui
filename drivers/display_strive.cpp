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

#include "display_strive.h"
#include "miosix.h"

using namespace miosix;

#ifdef _BOARD_STRIVE_MINI

namespace mxgui {

//
// Class DisplayImpl
//

DisplayImpl::DisplayImpl(): textColor(), font(droid11)
{
	//Enable clocks to all ports
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN |
					RCC_APB2ENR_IOPCEN | RCC_APB2ENR_IOPDEN |
					RCC_APB2ENR_IOPEEN;
	disp::reset::mode(Mode::OUTPUT);
	disp::ncpEn::mode(Mode::OUTPUT);

	disp::d0::mode(Mode::ALTERNATE);
	disp::d1::mode(Mode::ALTERNATE);
	disp::d2::mode(Mode::ALTERNATE);
	disp::d3::mode(Mode::ALTERNATE);
	disp::d4::mode(Mode::ALTERNATE);
	disp::d5::mode(Mode::ALTERNATE);
	disp::d6::mode(Mode::ALTERNATE);
	disp::d7::mode(Mode::ALTERNATE);
	disp::d8::mode(Mode::ALTERNATE);
	disp::d9::mode(Mode::ALTERNATE);
	disp::d10::mode(Mode::ALTERNATE);
	disp::d11::mode(Mode::ALTERNATE);
	disp::d12::mode(Mode::ALTERNATE);
	disp::d13::mode(Mode::ALTERNATE);
	disp::d14::mode(Mode::ALTERNATE);
	disp::d15::mode(Mode::ALTERNATE);
	disp::rd::mode(Mode::ALTERNATE);
	disp::wr::mode(Mode::ALTERNATE);
	disp::cs::mode(Mode::ALTERNATE);
	disp::rs::mode(Mode::ALTERNATE);

	//FIXME: This assumes xram is already initialized an so D0..D15, A0, NOE,
    //NWE are correctly initialized
    RCC->AHBENR |= RCC_AHBENR_FSMCEN;

    volatile uint32_t& BCR1=FSMC_Bank1->BTCR[0];
    BCR1 = FSMC_BCR1_MBKEN  | //memory bank enable
           FSMC_BCR1_MTYP_1 | //memory type: NOR
           FSMC_BCR1_MWID_0 | //memory data width: 16
           FSMC_BCR1_WREN |   //Write Operation Enable
           FSMC_BCR1_FACCEN;// | //Flash access enable
           //FSMC_BCR1_EXTMOD;

    //volatile uint32_t& BWTR1=FSMC_Bank1E->BWTR[0];
    //BWTR1 = FSMC_BWTR1_DATAST_1 | FSMC_BWTR1_DATAST_0;

    volatile uint32_t& BTR1=FSMC_Bank1->BTCR[1];
    BTR1 = 2 << 0 |  // Address setup phase duration
           0 << 4 |  // Address-hold phase duration
           5 << 8 |  // Data-phase duration
           0 << 16 | // Bus turn-around duration
           0 << 20 | // Clock divide ratio
           0 << 24 | // Data latency
           1 << 28;  // Access mode
    
    //
    //Power up sequence -- begin
    //
    delayMs(500);//Wait at least 500ms from +3.3 stabilized
    disp::reset::low();
    delayMs(1);
    disp::reset::high();
    delayMs(10);//Wait 10ms before waking up the display

    //Turn on backlight
    disp::ncpEn::high();

    writeReg(0x00E3, 0x3008); // Set internal timing
    writeReg(0x00E7, 0x0012); // Set internal timing
    writeReg(0x00EF, 0x1231); // Set internal timing
    writeReg(0x0000, 0x0001); // Start Oscillation
    writeReg(0x0001, 0x0100); // set SS and SM bit
    writeReg(0x0002, 0x0700); // set 1 line inversion

    writeReg(0x0003, 0x1038); // set GRAM write direction and BGR=0,262K colors,1 transfers/pixel.
    writeReg(0x0004, 0x0000); // Resize register
    writeReg(0x0008, 0x0202); // set the back porch and front porch
    writeReg(0x0009, 0x0000); // set non-display area refresh cycle ISC[3:0]
    writeReg(0x000A, 0x0000); // FMARK function
    writeReg(0x000C, 0x0000); // RGB interface setting
    writeReg(0x000D, 0x0000); // Frame marker Position
    writeReg(0x000F, 0x0000); // RGB interface polarity
        //Power On sequence
    writeReg(0x0010, 0x0000); // SAP, BT[3:0], AP, DSTB, SLP, STB
    writeReg(0x0011, 0x0007); // DC1[2:0], DC0[2:0], VC[2:0]
    writeReg(0x0012, 0x0000); // VREG1OUT voltage
    writeReg(0x0013, 0x0000); // VDV[4:0] for VCOM amplitude
    delayMs(200); // Dis-charge capacitor power voltage
	writeReg(0x0010, 0x1690); // SAP, BT[3:0], AP, DSTB, SLP, STB
	writeReg(0x0011, 0x0227); // R11h=0x0221 at VCI=3.3V, DC1[2:0], DC0[2:0], VC[2:0]
	delayMs(50); // Delay 50ms
	writeReg(0x0012, 0x001C); // External reference voltage= Vci;
	delayMs(50); // Delay 50ms
	writeReg(0x0013, 0x1800); // R13=1200 when R12=009D;VDV[4:0] for VCOM amplitude
	writeReg(0x0029, 0x001C); // R29=000C when R12=009D;VCM[5:0] for VCOMH
	writeReg(0x002B, 0x000D); // Frame Rate = 91Hz
	delayMs(50); // Delay 50ms
	writeReg(0x0020, 0x0000); // GRAM horizontal Address
	writeReg(0x0021, 0x0000); // GRAM Vertical Address
// ----------- Adjust the Gamma Curve ----------//
	writeReg(0x0030, 0x0007);
	writeReg(0x0031, 0x0707);
	writeReg(0x0032, 0x0006);
	writeReg(0x0035, 0x0704);
	writeReg(0x0036, 0x1F04);
	writeReg(0x0037, 0x0004);
	writeReg(0x0038, 0x0000);
	writeReg(0x0039, 0x0706);
	writeReg(0x003C, 0x0701);
	writeReg(0x003D, 0x000F);
//------------------ Set GRAM area ---------------//
	writeReg(0x0050, 0x0000); // Horizontal GRAM Start Address
	writeReg(0x0051, 0x00EF); // Horizontal GRAM End Address
	writeReg(0x0052, 0x0000); // Vertical GRAM Start Address
	writeReg(0x0053, 0x013F); // Vertical GRAM Start Address
	writeReg(0x0060, 0xA700); // Gate Scan Line
	writeReg(0x0061, 0x0001); // NDL,VLE, REV
	writeReg(0x006A, 0x0000); // set scrolling line
//-------------- Partial Display Control ---------//
	writeReg(0x0080, 0x0000);
	writeReg(0x0081, 0x0000);
	writeReg(0x0082, 0x0000);
	writeReg(0x0083, 0x0000);
	writeReg(0x0084, 0x0000);
	writeReg(0x0085, 0x0000);
//-------------- Panel Control -------------------//
	writeReg(0x0090, 0x0010);
	writeReg(0x0092, 0x0000);
	writeReg(0x0093, 0x0003);
	writeReg(0x0095, 0x0110);
	writeReg(0x0097, 0x0000);
	writeReg(0x0098, 0x0000);
	writeReg(0x0007, 0x0133); // 262K color and display ON

    setTextColor(black, white);
    clear(white);
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

void DisplayImpl::drawRectangle(Point a, Point b, Color c)
{
    line(a,Point(b.x(),a.y()),c);
    line(Point(b.x(),a.y()),b,c);
    line(b,Point(a.x(),b.y()),c);
    line(Point(a.x(),b.y()),a,c);
}

void DisplayImpl::turnOn()
{
    writeReg(0x10,0x0000);//STB = 0 (out of standby)
    delayMs(100);//Let internal voltages stabilize
    disp::ncpEn::high();
    delayMs(32);//Let AR_VDD and AR_VSS stabilize
    //writeReg(0x05,0x0001);//DISP_ON = 1 (display active)
}

void DisplayImpl::turnOff()
{
    //writeReg(0x05,0x0000);//DISP_ON = 0 (display blank)
    delayMs(32);
    disp::ncpEn::low();
    delayMs(32);
    writeReg(0x10,0x0001);//STB = 1 (standby mode)
    delayMs(500);
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

DisplayImpl::DisplayMemLayout *const DisplayImpl::DISPLAY=
        reinterpret_cast<DisplayImpl::DisplayMemLayout*>(0x60000000);

} //namespace mxgui

#endif //_BOARD_STRIVE_MINI
