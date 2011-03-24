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

#include "mxgui_settings.h"

#ifndef IMAGE_H
#define	IMAGE_H

namespace mxgui {

class ImageDepth
{
public:
    enum ImageDepth_
    {
        DEPTH_1_BIT,
        DEPTH_8_BIT,
        DEPTH_16_BIT,
        DEPTH_18_BIT,
        DEPTH_24_BIT
    };
private:
    ImageDepth();//Just a wrapper class
};

/**
 * This class contains an image. Images are immutable except they can be
 * assigned with operator=
 */
template<typename T>
class basic_image
{
public:
    /**
     * Construct an Image
     * \param height the image's height
     * \param width the image's width
     * \param data the pointer to the image's data. Ownsership of the data is
     * still of the caller. If the pointer points to const data no special care
     * must be taken, otherwise the caller must free the memory when the Image
     * is no longer useful, to avoid a memory leak.
     */
    basic_image(short int height, short int width, const T *data,
            ImageDepth::ImageDepth_ depth):
            height(height), width(width), data(data), depth(depth) {}

    /**
     * \return the image's height
     */
    short int getHeight() const { return height; }

    /**
     * \return the image's width
     */
    short int getWidth() const { return width; }

    /**
     * \return a const pointer to the image's data
     */
    const T* getData() const { return data; }

    /**
     * \return the number of bit per pixel
     */
    ImageDepth::ImageDepth_ imageDepth() const { return depth; }

    /**
     * Virtual because someday I might implement an Image class with reference
     * counting if the problem of freeing memory becomes relevant
     */
    virtual ~basic_image() {}

    //Uses default copy constructor and operator=. The pointer can be shared
    //without problems since there is no member function to modify the image
    //data nor to return a non-const pointer to it
private:
    short int height, width;
    const T *data;
    ImageDepth::ImageDepth_ depth;
};

///Define the Color class, depending on the COLOR_DEPTH constant
#ifdef MXGUI_COLOR_DEPTH_1_BIT
typedef basic_image<unsigned char> Image;
#elif defined(MXGUI_COLOR_DEPTH_8_BIT)
typedef basic_image<unsigned char> Image;
#elif defined(MXGUI_COLOR_DEPTH_16_BIT)
typedef basic_image<unsigned short> Image;
#endif

} // namespace mxgui

#endif //IMAGE_H
