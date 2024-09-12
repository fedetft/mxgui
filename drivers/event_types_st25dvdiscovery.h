/***************************************************************************
 *   Copyright (C) 2014 by Terraneo Federico                               *
 *   Copyright (C) 2024 by Daniele Cattaneo                                *
 *   Copyright (C) 2024 by Ignazio Neto dell'Acqua                         *
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

#ifndef EVENT_TYPES_ST25DVDISCOVERY_H
#define	EVENT_TYPES_ST25DVDISCOVERY_H

#ifdef _BOARD_STM32F415VG_ST25DVDISCOVERY

class EventType
{
public:
    enum E
    {
        // These are a must on all backends -- begin
        Default=0,           // This actually means 'no event'
        WindowPartialRedraw=17, // At least one drawable has requested redraw
        WindowForeground=18,    // Window manager moved this window to foreground
        WindowBackground=19,    // Window manager moved this window to background
        WindowQuit=20,          // Window manager requested the window to close
        // These are a must on all backends -- end
        
        TouchDown=1,
        TouchUp=2,
        TouchMove=3,
        ButtonA=4,      // The "Key" or "User" button
        ButtonJoy=5,    // Center button on the joystick
        ButtonUp=6,     // Up button on the joystick
        ButtonDown=7,   // Down button on the joystick
        ButtonLeft=8,   // Left button on the joystick
        ButtonRight=9,  // Right button on the joystick
        None = 10,
        ButtonADown=11,      // The "Key" or "User" button become pressed
        ButtonJoyDown=12,    // Center button on the joystick become pressed
        ButtonUpDown=13,     // Up button on the joystick become pressed
        ButtonDownDown=14,   // Down button on the joystick become pressed
        ButtonLeftDown=15,   // Left button on the joystick become pressed
        ButtonRightDown=16,  // Right button on the joystick become pressed
    };
private:
    EventType();
};

#endif //_BOARD_STM32F415VG_ST25DVDISCOVERY

#endif //EVENT_TYPES_ST25DVDISCOVERY_H
