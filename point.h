/***************************************************************************
 *   Copyright (C) 2010 by Terraneo Federico                               *
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
 * Point class. Points are immutable except they can be assigned with operator=
 */
template<typename T>
class basic_point
{
public:
    /**
     * Constructor, create an instance of the Point class given x and y
     */
    basic_point(T x, T y): x_(x), y_(y) {}

    /**
     * Default constructor, yields a point to (0,0)
     */
    basic_point(): x_(0), y_(0) {}

    /**
     * \return x coordinate
     */
    T x() const { return x_; }

    /**
     * \return the y coordinate
     */
    T y() const { return y_; }

    /**
     * Compare two points for equality
     */
    bool operator== (basic_point p)
    {
        return (this->x_ == p.x_) && (this->y_ == p.y_);
    }

    /**
     * Compare two points for inequality
     */
    bool operator!= (basic_point p)
    {
        return (this->x_ != p.x_) || (this->y_ != p.y_);
    }

    //Uses default copy constructor and operator=
private:
    T x_,y_;
};

///Point of short int. No configuration option here since 16 bits is acceptable
///for any display type. Note that the data type is signed. This is intentional
///because algorithms like the one to draw a line calculate the difference
///between point coordinates, and the result can be negative
typedef basic_point<short int> Point;

} // namespace mxgui

#endif //POINT_H
