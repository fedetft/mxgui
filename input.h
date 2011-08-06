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

#include "mxgui/drivers/event_types_qt.h"
#include "mxgui/drivers/event_types_mp3v2.h"
#include "mxgui/point.h"

#ifndef INPUT_H
#define	INPUT_H

namespace mxgui {

/**
 * Generic event class
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

/**
 * Class to handle events
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
    InputHandler();
};

} //namespace mxgui

#endif //INPUT_H
