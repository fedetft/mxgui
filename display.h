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

#ifndef DISPLAY_H
#define	DISPLAY_H

#include "mxgui_settings.h"
#include "display_base.h"
#include "misc_inst.h"
#include "drivers/display_stm3210e-eval.h"
#include "drivers/display_mp3v2.h"
#include "drivers/display_qt.h"

namespace mxgui {

//
// Select display type based on board name. The _BOARD macros are defined
// in Miosix's Makefile.inc depending on board selected.
// If no board selected, we assume we are building for the Qt simulator to test
// the GUI on a PC.
//
#if defined(_BOARD_STM3210E_EVAL)
typedef basic_display<DisplayStm3210e_eval> Display;
#elif defined(_BOARD_MP3V2)
typedef basic_display<DisplayMP3V2> Display;
#else
typedef basic_display<DisplayQt> Display;
#endif

} //namespace mxgui

#endif //DISPLAY_H
