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
//#define MXGUI_PIXEL_FORMAT_GRAY1
//#define MXGUI_PIXEL_FORMAT_GRAY4
//#define MXGUI_PIXEL_FORMAT_RGB332 //Untested
#define MXGUI_PIXEL_FORMAT_RGB565

///
/// 1BPP color threshold.
/// The threshold at which a color is considered white in 1BPP mode.
/// The value is the sum of the red, green and blue components (0-255 each).
/// Default is 384 (128*3, or 50% gray).
///
#define MXGUI_1BPP_THRESHOLD 384

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
// Choose pixel format.
//
// #define MXGUI_PIXEL_FORMAT_GRAY1
// #define MXGUI_PIXEL_FORMAT_GRAY4
// #define MXGUI_PIXEL_FORMAT_RGB332
#define MXGUI_PIXEL_FORMAT_RGB565

///
/// 1BPP color threshold.
/// The threshold at which a color is considered white in 1BPP mode.
/// The value is the sum of the red, green and blue components (0-255 each).
/// Default is 384 (128*3, or 50% gray).
///
#define MXGUI_1BPP_THRESHOLD 384

static const unsigned int SIMULATOR_DISP_HEIGHT=320;
static const unsigned int SIMULATOR_DISP_WIDTH=240;
static const unsigned char SIMULATOR_FG_R=255;
static const unsigned char SIMULATOR_FG_G=255;
static const unsigned char SIMULATOR_FG_B=255;
static const unsigned char SIMULATOR_BG_R=0;
static const unsigned char SIMULATOR_BG_G=0;
static const unsigned char SIMULATOR_BG_B=0;

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
