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

#include "point.h"
#include "color.h"

#ifndef LINE_H
#define	LINE_H

namespace mxgui {

/**
 * \internal Class containing code to draw a line
 */
class Line
{
public:

    /**
     * Draw a line between point a and point b, with color c on a surface
     * \param surface an object providing beginPixel() and setPixel()
     * \param a first point
     * \param b second point
     * \param c line color
     */
    template<typename T>
    static void draw(T& surface, Point a, Point b, Color c);
};

template<typename T>
void Line::draw(T& surface, Point a, Point b, Color c)
{
    //Bresenham's algorithm
    surface.beginPixel();
    const short dx=b.x()-a.x();
    const short dy=b.y()-a.y();
    const short adx=abs(dx);
    const short ady=abs(dy);
    if(adx>ady)
    {
        short yincr= dy>=0 ? 1 : -1;
        short d=2*ady-adx;
        short v=2*(ady-adx);
        short w=2*ady;
        short y=a.y();
        if(dx>0)
        {
            for(short x=a.x();x<=b.x();x++)
            {
                surface.setPixel(Point(x,y),c);
                if(d>0)
                {
                    y+=yincr;
                    d+=v;
                } else d+=w;
            }
        } else {
            for(short x=a.x();x>=b.x();x--)
            {
                surface.setPixel(Point(x,y),c);
                if(d>0)
                {
                    y+=yincr;
                    d+=v;
                } else d+=w;
            }
        }
    } else {
        short xincr= dx>=0 ? 1 : -1;
        short d=2*adx-ady;
        short v=2*(adx-ady);
        short w=2*adx;
        short x=a.x();
        if(dy>0)
        {
            for(short y=a.y();y<=b.y();y++)
            {
                surface.setPixel(Point(x,y),c);
                if(d>0)
                {
                    x+=xincr;
                    d+=v;
                } else d+=w;
            }
        } else {
            for(short y=a.y();y>=b.y();y--)
            {
                surface.setPixel(Point(x,y),c);
                if(d>0)
                {
                    x+=xincr;
                    d+=v;
                } else d+=w;
            }
        }
    }
}

} //namespace mxgui

#endif //LINE_H
