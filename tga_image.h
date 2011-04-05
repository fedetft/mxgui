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
#include <cstring>

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
template<typename T>
class basic_tga_image : public basic_image_base<T>
{
public:
    /**
     * Default constructor
     */
    basic_tga_image() : basic_tga_image(), name(0), f(0), offset(0) {}

    /**
     * Construct from a filename
     * \param filename file name of tga image
     */
    explicit basic_tga_image(const char *filename);

    /**
     * Copy constructor
     * \param rhs instance to copy from
     */
    basic_tga_image(const basic_tga_image& rhs);

    /**
     * Operator =
     * \param rhs instance to copy from
     * \return reference to *this
     */
    const basic_tga_image& operator= (const basic_tga_image& rhs);

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

template<typename T>
basic_tga_image<T>::basic_tga_image(const char *filename)
        : basic_image_base<T>(), name(0), f(0), offset(0)
{
    this->open(filename);
}

template<typename T>
basic_tga_image<T>::basic_tga_image(const basic_tga_image<T>& rhs)
        : basic_image_base<T>(), name(0), f(0), offset(0)
{
    this->open(rhs.name);
}

template<typename T>
const basic_tga_image<T>& basic_tga_image<T>::operator=(
        const basic_tga_image<T>& rhs)
{
    this->open(rhs.name);
    return *this;
}

template<typename T>
void basic_tga_image<T>::open(const char *filename)
{
    if(filename==0) return;
    this->close();

    //Try to open file
    this->f=fopen(filename,"r");
    if(this->f==NULL) return;
    //setbuf(this->f,0); //Too slow if unbuffered

    //Try to parse header
    bool fail=false;
    TgaHeader header;
    if(fread(&header,1,sizeof(TgaHeader),this->f)!=sizeof(TgaHeader)) fail=true;
    //TODO: more support for options
    if(header.colorMapType!=0) fail=true; //Color maps unsupported
    if(header.imgType!=2) fail=true;      //Only uncompressed truecolor image
    if(header.pixDepth!=24) fail=true;    //Only 24bit images supported
    if(fail)
    {
        fclose(this->f);
        return;
    }

    //Fill image data
    this->height=header.height;
    this->width=header.width;
    //Image is stored with reversed y axis, so store the end of the image as
    //offset and then use subraction to get current line
    this->offset=sizeof(TgaHeader)+header.idLength;

    //Last, copy file name to local variable
    int length=strlen(filename)+1;
    this->name=new char[length];
    strcpy(this->name,filename);
}

template<typename T>
void basic_tga_image<T>::close()
{
    if(this->name==0) return;
    delete[] this->name;
    this->name=0;
    fclose(this->f);
    this->width=0;
    this->height=0;
    this->offset=0;
}

template<typename T>
bool basic_tga_image<T>::getScanLine(mxgui::Point p, mxgui::Color colors[],
            unsigned short length) const
{
    if(this->isOpen()==false) return false;
    if(p.x()<0 || p.x()<0) return false;
    if(p.x()>=this->getWidth() || p.y()>=this->getHeight()) return false;
    int o=p.x()+this->getWidth()*p.y();
    fseek(f,this->offset+3*o,SEEK_SET);
    //TODO: specialize: this only works if MXGUI_COLOR_DEPTH_16_BIT
    for(int i=0;i<length;i++)
    {
        unsigned char pix[3];
        if(fread(pix,1,3,f)!=3) return false;
        colors[i]=(pix[2] & 0xf8)<<8 | (pix[1] & 0xfc)<<3 | pix[0]>>3;
    }
    return true;
}

///Define the TgaImage class, depending on the COLOR_DEPTH constant
#ifdef MXGUI_COLOR_DEPTH_1_BIT
//When 1bit per pixel mode TgaImage is not available
#elif defined(MXGUI_COLOR_DEPTH_8_BIT)
typedef basic_tga_image<unsigned char> TgaImage;
#elif defined(MXGUI_COLOR_DEPTH_16_BIT)
typedef basic_tga_image<unsigned short> TgaImage;
#endif

} //namespace mmxgui

#endif //TGA_IMAGE_H
