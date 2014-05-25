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
#include <config/mxgui_settings.h>

#ifndef RESOURCE_IMAGE_H
#define	RESOURCE_IMAGE_H

#ifdef MXGUI_ENABLE_RESOURCEFS

namespace mxgui {

class ResourceImageImpl; //Forward declaration

/**
 * \ingroup pub_iface
 * This is a class for handling images stored in the ResourceFs filesystem,
 * and is the primary reason why ResourceFs was designed.
 */
class ResourceImage : public ImageBase
{
public:
    /**
     * Default constructor
     */
    ResourceImage(): pImpl(0) {}

    /**
     * Constructor.
     * \param name name of image file inside the resource filesystem
     */
    explicit ResourceImage(const char *name): pImpl(0) { this->open(name); }

    /**
     * Copy constructor
     * \param rhs instance to copy from
     */
    ResourceImage(const ResourceImage& rhs);

    /**
     * Operator =
     * \param rhs instance to copy from
     * \return reference to *this
     */
    ResourceImage& operator=(const ResourceImage&);

    /**
     * Open a file
     * \param name name of image file inside the resource filesystem
     */
    void open(const char *name);

    /**
     * Close image file
     */
    void close();

    /**
     * \return true if image is open
     */
    bool isOpen() const { return pImpl!=0; }

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
     * Desctructor
     */
    virtual ~ResourceImage() { close(); }

private:
    ResourceImageImpl *pImpl;
};

} // namespace mxgui

#endif //MXGUI_ENABLE_RESOURCEFS

#endif //RESOURCE_IMAGE_H
