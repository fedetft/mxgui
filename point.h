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

#ifndef POINT_H
#define	POINT_H

namespace mxgui {

/**
 * \ingroup pub_iface
 * Point class. Points are immutable except they can be assigned with operator=
 */
class Point
{
public:
    /**
     * Constructor, create an instance of the Point class given x and y
     */
    Point(short int x, short int y): x_(x), y_(y) {}

    /**
     * Default constructor, yields a point to (0,0)
     */
    Point(): x_(0), y_(0) {}

    /**
     * \return x coordinate
     */
    short int x() const { return x_; }

    /**
     * \return the y coordinate
     */
    short int y() const { return y_; }

    /**
     * Compare two points for equality
     */
    bool operator== (Point p)
    {
        return (this->x_ == p.x_) && (this->y_ == p.y_);
    }

    /**
     * Compare two points for inequality
     */
    bool operator!= (Point p)
    {
        return (this->x_ != p.x_) || (this->y_ != p.y_);
    }

    //Uses default copy constructor and operator=
private:
    short int x_,y_;
};

/**
 * \param a point to test
 * \param b upper left corner of test area
 * \param c lower right corner of test ares
 * \return true if the point a is within the area identified by b and c
 */
inline bool within(Point a, Point b, Point c)
{
    return a.x()>=b.x() && a.y()>=b.y() && a.x()<c.x() && a.y()<c.y();
}

} // namespace mxgui

#endif //POINT_H
