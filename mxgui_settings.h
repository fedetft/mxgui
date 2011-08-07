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

#ifndef MXGUI_SETTINGS_H
#define	MXGUI_SETTINGS_H

namespace mxgui {

#ifdef _MIOSIX

//
// Choose color depth.
//
//#define MXGUI_COLOR_DEPTH_1_BIT //Untested
//#define MXGUI_COLOR_DEPTH_8_BIT //Untested
#define MXGUI_COLOR_DEPTH_16_BIT

//
// Display orientation settings, choose ONE of these. Their meaninig depends
// on the chosen display type
//
#define MXGUI_ORIENTATION_VERTICAL
//#define MXGUI_ORIENTATION_HORIZONTAL
//#define MXGUI_ORIENTATION_VERTICAL_MIRRORED
//#define MXGUI_ORIENTATION_HORIZONTAL_MIRRORED

//
// Select which fonts are required. Choose one or more
//
#define MXGUI_FONT_DROID11
//#define MXGUI_FONT_DROID21
#define MXGUI_FONT_MISCFIXED
#define MXGUI_FONT_TAHOMA
//#define MXGUI_ENABLE_BOLD_FONTS

#else //_MIOSIX

//
// Choose color depth.
//
//#define MXGUI_COLOR_DEPTH_1_BIT //Untested
//#define MXGUI_COLOR_DEPTH_8_BIT //Untested
#define MXGUI_COLOR_DEPTH_16_BIT

static const unsigned int SIMULATOR_DISP_HEIGHT=320;
static const unsigned int SIMULATOR_DISP_WIDTH=240;

//
// Display orientation settings, choose ONE of these. Their meaninig depends
// on the chosen display type
//
#define MXGUI_ORIENTATION_VERTICAL
//#define MXGUI_ORIENTATION_HORIZONTAL
//#define MXGUI_ORIENTATION_VERTICAL_MIRRORED
//#define MXGUI_ORIENTATION_HORIZONTAL_MIRRORED

//
// Select which fonts are required. Choose one or more
//
#define MXGUI_FONT_DROID11
#define MXGUI_FONT_DROID21
#define MXGUI_FONT_MISCFIXED
#define MXGUI_FONT_TAHOMA
#define MXGUI_ENABLE_BOLD_FONTS

#endif //_MIOSIX

} //namespace mxgui

#endif //MXGUI_SETTINGS_H
