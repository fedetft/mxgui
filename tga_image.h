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

#include "image.h"
#include <cstdio>

#ifndef TGA_IMAGE_H
#define	TGA_IMAGE_H

namespace mxgui {

/**
 * This is a class for handling .tga images stored on disk.
 * It is optimized for memory usage, so that it can used on arbitrary sized
 * images on microcontrollers. To do this it does not load the image on disk
 * and only fseeks around as needed. The obvious drawback is that it is heavy
 * on disk bandwidth, but there's no other way to do that when you don't have
 * enough RAM
 */
class TgaImage : public ImageBase
{
public:
    /**
     * Default constructor
     */
    TgaImage() : ImageBase(), name(0), f(0), offset(0) {}

    /**
     * Construct from a filename
     * \param filename file name of tga image
     */
    explicit TgaImage(const char *filename): name(0) { this->open(filename); }

    /**
     * Copy constructor
     * \param rhs instance to copy from
     */
    TgaImage(const TgaImage& rhs): name(0) { this->open(rhs.name); }

    /**
     * Operator =
     * \param rhs instance to copy from
     * \return reference to *this
     */
    const TgaImage& operator= (const TgaImage& rhs)
    {
        if(this!=&rhs) this->open(rhs.name);
        return *this;
    }

    /**
     * Open a tga file
     * \param filename
     */
    void open(const char *filename);

    /**
     * Clode tga file
     */
    void close();

    /**
     * \return true if image is open
     */
    bool isOpen() const { return name!=0; }

    /**
     * Get pixels from tha image. This member function can be used to get
     * up to a full horizontal line of pixels from an image.
     * \param p Start point, within <0,0> and <getWidth()-1,getHeight()-1>
     * \param colors pixel data is returned here. Array size must be equal to
     * the length parameter
     * \param length number of pixel to retrieve from the starting point.
     * start.x()+length must be less or equal to getWidth()
     * \return true if success. If false then it means the class does not
     * represent a valid image, or a disk error occurred in case the image
     * is stored on disk.
     */
    virtual bool getScanLine(mxgui::Point p, mxgui::Color colors[],
            unsigned short length) const;

    /**
     * Destructor
     */
    virtual ~TgaImage() { close(); }

private:

    /**
     * Header of a tga image
     */
    struct __attribute__((packed)) TgaHeader
    {
        unsigned char idLength;
        unsigned char colorMapType;
        unsigned char imgType;

        unsigned short colorMapFirstEntryIndex;
        unsigned short colorMapLength;
        unsigned char colorMapEntrySize;

        unsigned short xOrigin;
        unsigned short yOrigin;
        unsigned short width;
        unsigned short height;
        unsigned char pixDepth;
        unsigned char imgDesc;
    };

    char *name; ///< File name. Also if == NULL it means the file is closed
    FILE *f; ///< Tga image file
    short offset; ///< Offset from start of file where image data starts
};

} //namespace mmxgui

#endif //TGA_IMAGE_H
