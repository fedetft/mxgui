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

#include "tga_image.h"
#include <cstring>

namespace mxgui {

void TgaImage::open(const char *filename)
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
    //TODO: endianness
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

void TgaImage::close()
{
    if(this->name==0) return;
    delete[] this->name;
    this->name=0;
    fclose(this->f);
    this->width=0;
    this->height=0;
    this->offset=0;
}

bool TgaImage::getScanLine(mxgui::Point p, mxgui::Color colors[],
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

} // namespace mxgui
