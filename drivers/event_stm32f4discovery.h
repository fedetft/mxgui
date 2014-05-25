/***************************************************************************
 *   Copyright (C) 2014 by Terraneo Federico                               *
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

#ifndef MXGUI_LIBRARY
#error "This is header is private, it can be used only within mxgui."
#error "If your code depends on a private header, it IS broken."
#endif //MXGUI_LIBRARY

#include "level2/input.h"

#ifndef EVENT_STM32F4DISCOVERY_H
#define	EVENT_STM32F4DISCOVERY_H

#ifdef _BOARD_STM32F429ZI_STM32F4DISCOVERY

namespace mxgui {

/**
 * Implementation class to handle events in the Mp3v2 backend
 */
class InputHandlerImpl
{
public:
    InputHandlerImpl();

    /**
     * \return an event, blocking
     */
    Event getEvent();

    /**
     * \return an event, nonblocking. A default constructed event is returned
     * if there are no events.
     */
    Event popEvent();
};

} //namespace mxgui

#endif //_BOARD_STM32F429ZI_STM32F4DISCOVERY

#endif //EVENT_STM32F4DISCOVERY_H
