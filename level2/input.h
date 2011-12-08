/***************************************************************************
 *   Copyright (C) 2011 by Terraneo Federico                               *
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

#include "mxgui/mxgui_settings.h"
#include "mxgui/point.h"
#include "mxgui/drivers/event_types_qt.h"
#include "mxgui/drivers/event_types_win.h"
#include "mxgui/drivers/event_types_mp3v2.h"
#include "mxgui/drivers/event_types_strive.h"
#include "mxgui/drivers/event_types_stm3210e-eval.h"
#include "mxgui/drivers/event_types_redbull_v2.h"

#ifdef MXGUI_LEVEL_2

#ifndef INPUT_H
#define	INPUT_H

namespace mxgui {

/**
 * \ingroup pub_iface_2
 * Generic event class. Events are the object type used to dispatch events
 * such as button presses or touchscreen taping to applications.
 * An Event has an associated type, which is implementation-defined depending
 * on the board on which mxgui is ported (different boards have different
 * number of buttons), and a point used to represent touchscreen point of touch
 * for boards that have a touchscreen.
 */
class Event
{
public:
    /**
     * Default constructor
     */
    Event(): e(EventType::Default), p(-1,-1) {}

    /**
     * Constructor for events without a position information
     * \param e event type
     */
    explicit Event(EventType::E e): e(e), p(-1,-1) {}


    /**
     * Constructor for events that also carry a position information
     * \param e event type
     * \param p point
     */
    Event(EventType::E e, Point p): e(e), p(p) {}

    /**
     * \return the event information
     */
    EventType::E getEvent() const { return e; }

    /**
     * \return true if the event has a valid point associated with it
     */
    bool hasValidPoint() const { return p.x()>=0; }

    /**
     * \return the point information
     */
    Point getPoint() const { return p; }

private:
    EventType::E e;
    Point p;
};

class InputHandlerImpl; //Forward declaration

/**
 * \ingroup pub_iface_2
 * This class contains member function to retrieve events from the system.
 */
class InputHandler
{
public:
    /**
     * \return an instance of this class (singleton)
     */
    static InputHandler& instance();

    /**
     * \return A valid event. Blocking.
     */
    Event getEvent();

    /**
     * \return A valid event or a default-constructed event if no events
     * available. Nonblocking.
     */
    Event popEvent();

private:
    /**
     * Class cannot be copied
     */
    InputHandler(const InputHandler&);
    InputHandler& operator= (const InputHandler&);

    InputHandler(InputHandlerImpl *impl);
    
    InputHandlerImpl *pImpl; //Implementation detal
};

} //namespace mxgui

#endif //INPUT_H

#endif //MXGUI_LEVEL_2
