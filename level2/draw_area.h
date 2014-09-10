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

#ifndef DRAW_AREA_H
#define	DRAW_AREA_H

#include <utility>
#include "point.h"

namespace mxgui {

/**
 * \ingroup pub_iface_2
 * This class represents a rectangle on screen, used to specify a drawing area
 * \param first upper left point
 * \param second lower right point
 */
typedef std::pair<Point,Point> DrawArea;

/**
 * This class just encapsulates the Alignment_ enum so that the enum names don't
 * clobber the global namespace.
 */
class Alignment
{
public:
    /**
     * Possible alignments
     */
    enum Alignment_
    {
        TOP,
        BOTTOM,
        RIGHT,
        LEFT,
        CENTER
    };
    
private:
    Alignment(); //Just a wrapper class, disallow creating instances
};

} //namespace mxgui

#endif //DRAW_AREA_H
