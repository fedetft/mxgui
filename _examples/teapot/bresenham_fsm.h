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

#include <algorithm>
#include "mxgui/point.h"
#include "mxgui/color.h"

#ifndef BRESENHAM_FSM_H
#define	BRESENHAM_FSM_H

/**
 * The bresenham line algorithm turned into an FSM, to be able to draw only the
 * part of the line that intersects the current scanline.
 * Code is optimized for speed and to minimize the class' size.
 */
class BresenhamFSM
{
public:
    BresenhamFSM();

    /**
     * Constructor.
     * \param a one of the two endpoints of the line to draw
     * \param b one of the two endpoints of the line to draw
     */
    BresenhamFSM(mxgui::Point a, mxgui::Point b);

    /**
     * Draw part of the line that intersects the current scanline and update
     * the FSM so that the next time this member function is called, it will
     * handle the next scanline
     * \param scanLine an array of pixels. Make sure its size is at least the
     * one of the maximum x value of the points passed in the construtor, or
     * buffer overflow will occur
     * \param lineColor color used to draw the line
     * \return false if the line is finished and therefore there is no
     * intersection between the line and the current scanline, in this case
     * the scanLine parameter is not modified
     */
    bool drawScanLine(mxgui::Color scanLine[], mxgui::Color lineColor);

    /**
     * \return the leftmost x coordinate of the line that intersects the
     * current scanline and update the FSM so that the next time this member
     * function is called, it will handle the next scanline.
     * Or returns -1 if the line is finished and therefore there is no
     * intersection between the line and the current scanline
     */
    short getLeftmost() { return getLinePoints().first; }

    /**
     * \return the rightmost x coordinate of the line that intersects the
     * current scanline and update the FSM so that the next time this member
     * function is called, it will handle the next scanline.
     * Or returns -1 if the line is finished and therefore there is no
     * intersection between the line and the current scanline
     */
    short getRightmost() { return getLinePoints().second; }

    /**
     * \return the leftmost and rightmost x coordinate of the line that
     * intersects the current scanline and update the FSM so that the next time
     * this member function is called, it will handle the next scanline.
     * Or returns <-1,-1> if the line is finished and therefore there is no
     * intersection between the line and the current scanline
     */
    std::pair<short,short> getLinePoints();

    //Uses default destructor, copy constructor and operator=
private:
    short d; ///< Bresenham's error value
    short v; ///< Bresenham's value to add to the error in case it's positive
    short w; ///< Bresenham's value to add to the error in case it's negative
    /// Zero if adx>ady, +1 if adx<=ady and line goes to the right,
    /// -1 if adx<=ady and line goes to the left
    short flag;
    /// Current x coordinate of pixel to draw, or -1 when line drawing has ended
    short h;
    /// When drawing lines with adx>ady last x coordinate (including) to draw.
    /// Note that it can be greater or lower than h depending on the line having
    /// positive or negative slope. When drawing lines with adx<=ady it is
    /// initialized at the ady and decremented by one at each scanline to
    /// remember how many scanlines are left to draw
    short k;
};

#endif //BRESENHAM_FSM_H
