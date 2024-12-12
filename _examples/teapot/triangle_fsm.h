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

#include "mxgui/point.h"
#include "mxgui/color.h"
#include "bresenham_fsm.h"

#ifndef TRIANGLE_FSM_H
#define	TRIANGLE_FSM_H

/**
 * Finite state machine that draws a triangle one scanline at a time.
 * To be able to correctly stack triangles that share the same edges the
 * topmost and rightmost pixels of the triangle's contour are not drawn.
 */
class TriangleFSM
{
public:
    TriangleFSM();
    
    /**
     * Constructor
     * \param idx array of three short with indices into the vertex list
     * \param vl list of vertices of the model (3D, containing x,y,z)
     * \param color color used to fill-draw the triangle
     */
    TriangleFSM(const short *idx, const short *vl, mxgui::Color color);

    /**
     * Draw part of the triangle that intersects the current scanline and update
     * the FSM so that the next time this member function is called, it will
     * handle the next scanline
     * \param scanLine an array of pixels. Make sure its size is at least the
     * one of the maximum x value of the points passed in the construtor, or
     * buffer overflow will occur
     * \return false if the triangle is finished and therefore there is no
     * intersection between the triangle and the current scanline, in this case
     * the scanLine parameter is not modified
     */
    bool drawScanLine(mxgui::Color scanLine[]);

    /**
     * Advance the FSM by one line without drawing
     * \return false if the triangle is finished and therefore there is no
     * intersection between the triangle and the current scanline, in this case
     * the scanLine parameter is not modified
     */
    bool advanceWithoutDrawing();

    /**
     * \return the triangle's z coordinate
     */
    short getZ() const { return z; }

    //Uses default destructor, copy constructor and operator=
private:
    /// x coordinates of the leftmost edge are computed here using bresenham
    BresenhamFSM leftEdge;
    /// x coordinates of the rightmost edge are computed here using bresenham
    BresenhamFSM rightEdge;
    mxgui::Point mid, last; ///< The second and third points, sorted by y coord.
    mxgui::Color color; ///< Color used to fill-draw the triangle
    short z; ///< The z coordinate of a triangle, used for z-sorting
};

/**
 * Perform z-sorting of triangles
 * \param a a TriangleFSM
 * \param b a TriangleFSM
 * \return true if b is over a
 */
inline bool operator< (const TriangleFSM& a, const TriangleFSM& b)
{
    return a.getZ() < b.getZ();
}

#endif //TRIANGLE_FSM_H
