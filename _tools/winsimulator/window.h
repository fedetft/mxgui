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
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>   *
 ***************************************************************************/

#ifndef _WIN_WINDOW_H
#define _WIN_WINDOW_H

#include "mxgui/drivers/event_win.h"

/**
 * Main window
 */
class Window
{
public:
    /**
     * Constructor.
     * \param parent parent widget
     */
    explicit Window();

    /**
     * Overridden to emulate touchscreen
     * \param event mouse event
     */
    void mouseMoveEvent(int x, int y);

    /**
     * Overridden to emulate touchscreen
     * \param event mouse event
     */
    void mousePressEvent(int x, int y);

    /**
     * Overridden to emulate touchscreen
     * \param event mouse event
     */
    void mouseReleaseEvent(int x, int y);

    /**
     * Overridden to emulate Button A
     */
    void ButtonAEvent();

    /**
     * Overridden to emulate Button B
     */
    void ButtonBEvent();
};

#endif //WINDOW_H
