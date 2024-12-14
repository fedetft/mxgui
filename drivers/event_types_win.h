/***************************************************************************
 *   Copyright (C) 2011 by Yury Kuchura                                    *
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

#pragma once

#if !defined(_MIOSIX) && defined(_WINDOWS)

class EventType
{
public:
    enum E
    {
        // These are a must on all backends -- begin
        Default=0,           // This actually means 'no event'
        WindowPartialRedraw, // At least one drawable has requested redraw
        WindowForeground,    // Window manager moved this window to foreground
        WindowBackground,    // Window manager moved this window to background
        WindowQuit,          // Window manager requested the window to close
        // These are a must on all backends -- end
        
        TouchDown=1,
        TouchUp=2,
        TouchMove=3,
        ButtonA=4,
        ButtonB=5
    };
private:
    EventType();
};

#endif //!defined(_MIOSIX) && defined(_WINDOWS)
