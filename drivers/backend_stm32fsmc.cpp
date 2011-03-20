/***************************************************************************
 *   Copyright (C) 2010 by Terraneo Federico                               *
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

#include "backend_stm32fsmc.h"

#ifdef MXGUI_BACKEND_STM32FSMC

namespace mxgui {

#ifdef MXGUI_DISPLAY_TYPE_S6E63D6

void hardwareInit()
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
}

#elif defined MXGUI_DISPLAY_TYPE_SPFD5408

void hardwareInit()
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
}

#else
#error this backend does not support this display type
#endif

} // namespace mxgui

#endif //MXGUI_BACKEND_STM32FSMC
