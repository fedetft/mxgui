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

#include "display_mp3v2.h"
#include "miosix.h"

using namespace miosix;

#ifdef _BOARD_MP3V2

namespace mxgui {

//
// Class DisplayImpl
//

DisplayImpl::DisplayImpl(): textColor(), font(droid11)
{
    //FIXME: This assumes xram is already initialized an so D0..D15, A0, NOE,
    //NWE are correctly initialized
    RCC->AHBENR |= RCC_AHBENR_FSMCEN;

    //The way BCR and BTR are specified in stm32f10x.h sucks, trying to work
    //around it...
    volatile uint32_t& BCR1=FSMC_Bank1->BTCR[0];
    volatile uint32_t& BTR1=FSMC_Bank1->BTCR[1];
    volatile uint32_t& BWTR1=FSMC_Bank1E->BWTR[0];

    //Timings for s6e63d6

    //Write burst disabled, Extended mode enabled, Wait signal disabled
    //Write enabled, Wait signal active before wait state, Wrap disabled
    //Burst disabled, Data width 16bit, Memory type SRAM, Data mux disabled
    BCR1 = FSMC_BCR1_WREN | FSMC_BCR1_MWID_0 | FSMC_BCR1_MBKEN | FSMC_BCR1_EXTMOD;

    //--Write timings--
    //Address setup=0 (+1), Data setup=3 (+1), Access mode=A
    //NWE low    3 cycle 42ns: pass tWLW80(27.5ns)
    //Data setup 3 cycle 42ns: pass tWDS80(40ns)
    //NWE high   2 cycle 28ns: pass tWHW80(27.5ns)
    //Data hold  1 cycle 14ns: nearly pass tWDH80(15ns)
    //Cycle time 5 cycle 70ns: fail tCYCW80(85ns)
    //The fail is done on purpose to gain speed. Can be fixed if *really* needed
    //with an address setup of 1 instead of zero.
    //Maximum theoretical framerate is 187.5fps
    BWTR1 = FSMC_BWTR1_DATAST_1 | FSMC_BWTR1_DATAST_0;
    //--Read timings--
    //Address setup=15 (+1), Data setup=15 (+3), Access mode=A
    //NOE low    18 cycle 252ns: pass tWLR80(250ns)
    //Data setup 15 cycle 210ns: pass tRDD80(200ns)
    //NOE high   16 cycle 224ns: fail tWHR80(250ns)
    //Cycle time 34 cycle 476ns: fail tCYCR80(500ns)
    //The failures are the result of the fact that the maximum value of DATAST
    //and ADDSET is 15 so there's no simple way to fix them
    //Maximum theoretical framerate is 27.6fps
    BTR1 = FSMC_BTR1_DATAST_3 | FSMC_BTR1_DATAST_2 | FSMC_BTR1_DATAST_1 |
           FSMC_BTR1_DATAST_0 | FSMC_BTR1_ADDSET_3 | FSMC_BTR1_ADDSET_2 |
           FSMC_BTR1_ADDSET_1 | FSMC_BTR1_ADDSET_0;
    
    //
    //Power up sequence -- begin
    //
    delayMs(500);//Wait at least 500ms from +3.3 stabilized
    disp::reset::low();
    delayMs(1);
    disp::reset::high();
    delayMs(10);//Wait 10ms before waking up the display
    writeIdx(0x23); //These additional instrutions set up 16bit parallel iface
    writeReg(0x02,0x0000);
    writeReg(0x10,0x0000);//STB = 0 (out of standby)
    delayMs(100);//Let internal voltages stabilize
    //misc settings
    writeReg(0x03,0x0130);
    writeReg(0x18,0x0028);
    writeReg(0xf8,0x000f);//register f8 is undocumented in the datasheet??
    writeReg(0xf9,0x000f);//register f9 is undocumented in the datasheet??
    //Gamma settings
    writeReg(0x70,0x2580);
    writeReg(0x71,0x2780);
    writeReg(0x72,0x3380);
    writeReg(0x73,0x1d18);
    writeReg(0x74,0x1f11);
    writeReg(0x75,0x2419);
    writeReg(0x76,0x1a14);
    writeReg(0x77,0x211a);
    writeReg(0x78,0x2013);

    clear(Color(0x0000));//Clear screen before activating display
    disp::ncpEn::high();
    delayMs(32);//Let AR_VDD and AR_VSS stabilize
    writeReg(0x05,0x0001);//DISP_ON = 1 (display active)
    //
    //Power up sequence -- end
    //
    
    setTextColor(Color(0xffff),Color(0x0000));
    clear(black);
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
    writeReg(0x05,0x0001);//DISP_ON = 1 (display active)
}

void DisplayImpl::turnOff()
{
    writeReg(0x05,0x0000);//DISP_ON = 0 (display blank)
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

#endif //_BOARD_MP3V2
