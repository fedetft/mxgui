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

/**
 * \file mxgui_settings.h
 * This file contains configuration parameters for the mxusb library.
 */

#pragma once

#define MXGUI_SETTINGS_VERSION 102

// Before you can compile mxgui you have to configure it by editing this
// file. After that, comment out this line to disable the reminder error.
#error This error is a reminder that you have not edited mxgui_settings.h yet.

namespace mxgui {

#ifdef _MIOSIX

///
/// Enable or disable level 2.
/// The mxgui library is divided in two levels:
/// - level 1: Basic display abstraction layer (Display class)
/// - level 2: Touchscreen support, display arbitration (multiple applications
///   contending one display), higher level widgets
///
#define MXGUI_LEVEL_2

///
/// Maximum number of appications that can run simultaneously
/// (valid only if MXGUI_LEVEL_2 is defined)
///
static const unsigned int level2MaxNumApps=4;

///
/// Enable or disable ResourceFs, only some targets support it
///
//#define MXGUI_ENABLE_RESOURCEFS

///
/// Choose color depth. Three options are provided for 1, 8 or 16 bit per pixel
///
//#define MXGUI_COLOR_DEPTH_1_BIT_LINEAR
//#define MXGUI_COLOR_DEPTH_8_BIT //Untested
#define MXGUI_COLOR_DEPTH_16_BIT

///
/// Display orientation settings. Four options are provided for HORIZONTAL or
/// VERTICAL display orientation, and a mirrored options for both.
/// Their meaninig depends on how the display backend is implemented.
///
#define MXGUI_ORIENTATION_VERTICAL
//#define MXGUI_ORIENTATION_HORIZONTAL
//#define MXGUI_ORIENTATION_VERTICAL_MIRRORED
//#define MXGUI_ORIENTATION_HORIZONTAL_MIRRORED

///
/// Invert touch screen Y coordinate.
/// Required on some boards as the touchscreen sensor is mounted with an
/// inverted vertical coordinate with respect to the display's one.
///
//#define MXGUI_TOUCH_INVERT_Y

///
/// Select which fonts are required. Choose one or more
///
#define MXGUI_FONT_DROID11
#define MXGUI_FONT_DROID21
#define MXGUI_FONT_MISCFIXED
#define MXGUI_FONT_TAHOMA
#define MXGUI_ENABLE_BOLD_FONTS

//Default font
#define defaultFont droid11

#else //_MIOSIX

// Enable or disable level 2.
// The mxgui library is divided in two levels:
// - level 1: Basic display abstraction layer (Display class)
// - level 2: Touchscreen support, display arbitration (multiple applications
//   contending one display), higher level widgets
//
#define MXGUI_LEVEL_2

///
/// Maximum number of appications that can run simultaneously
/// (valid only if MXGUI_LEVEL_2 is defined)
///
static const int level2MaxNumApps=4;

//
// Enable or disable ResourceFs, only some targets support it
//
//#define MXGUI_ENABLE_RESOURCEFS

//
// Choose color depth.
//
//#define MXGUI_COLOR_DEPTH_1_BIT_LINEAR
//#define MXGUI_COLOR_DEPTH_8_BIT //Untested
#define MXGUI_COLOR_DEPTH_16_BIT

static const unsigned int SIMULATOR_DISP_HEIGHT=320;
static const unsigned int SIMULATOR_DISP_WIDTH=240;
static const unsigned int SIMULATOR_FGCOLOR=0xffff;
static const unsigned int SIMULATOR_BGCOLOR=0;

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

//Default font
#define defaultFont droid11

#endif //_MIOSIX

} //namespace mxgui
