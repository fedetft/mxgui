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

#include "resource_image.h"
#include "resourcefs.h"

#ifdef MXGUI_ENABLE_RESOURCEFS

namespace mxgui {

class ResourceImageImpl
{
public:
    resfs::ResourceFile file;
};

//
// class ResourceImage
//

ResourceImage::ResourceImage(const ResourceImage& rhs) : pImpl(0)
{
    if(rhs.isOpen()==false) return;
    pImpl=new ResourceImageImpl;
    pImpl->file=rhs.pImpl->file;
    this->height=rhs.height;
    this->width=rhs.width;
}

ResourceImage& ResourceImage::operator=(const ResourceImage& rhs)
{
    if(this==&rhs) return *this;
    close();
    if(rhs.isOpen()==false) return *this;
    pImpl=new ResourceImageImpl;
    pImpl->file=rhs.pImpl->file;
    this->height=rhs.height;
    this->width=rhs.width;
    return *this;
}

void ResourceImage::open(const char* name)
{
    close();
    pImpl=new ResourceImageImpl;
    pImpl->file.open(name);
    if(pImpl->file.isOpen()==false) { close(); return; }
    short imgData[3]; //imgData={height, width, bitsPerPixel}
    pImpl->file.read(reinterpret_cast<char*>(&imgData),6);
    this->height=imgData[0]; //TODO: endianness
    this->width=imgData[1]; //TODO: endianness
    #ifdef MXGUI_COLOR_DEPTH_1_BIT_LINEAR
    if(imgData[2]!=0x80 | 1) close(); //TODO: endianness
    #elif defined(MXGUI_COLOR_DEPTH_8_BIT)
    if(imgData[2]!=8) close(); //TODO: endianness
    #elif defined(MXGUI_COLOR_DEPTH_16_BIT)
    if(imgData[2]!=16) close(); //TODO: endianness
    #endif
    if(height<0 || width<0) close();
}

void ResourceImage::close()
{
    if(pImpl) delete pImpl;
    pImpl=0;
}

bool ResourceImage::getScanLine(mxgui::Point p, mxgui::Color colors[],
            unsigned short length) const
{
    //TODO: specialize, only works for 16 bit per pixel
    if(isOpen()==false) return false;
    if(p.x()<0 || p.x()<0) return false;
    if(p.x()>=this->getWidth() || p.y()>=this->getHeight()) return false;
    int o=p.x()+this->getWidth()*p.y();
    pImpl->file.lseek(6+2*o,SEEK_SET);
    //TODO: endianness
    int readBytes=pImpl->file.read(reinterpret_cast<char*>(colors),2*length);
    return readBytes==2*length;
}

} // namespace mxgui

#endif //MXGUI_ENABLE_RESOURCEFS
