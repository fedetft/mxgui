/***************************************************************************
 *   Copyright (C) 2018 by Terraneo Federico                               *
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

#include "display_generic_1bpp.h"
#include "font.h"
#include "image.h"
#include "misc_inst.h"
#include "line.h"
#include <algorithm>

using namespace std;

namespace mxgui {

//
// Class DisplayGeneric1BPP
//

DisplayGeneric1BPP::DisplayGeneric1BPP(short width, short height)
    : width(width), height(height), fbSize(width*height/8),
      backbuffer(new unsigned char[fbSize]), buffer(new Color[width]) {}

pair<short int, short int> DisplayGeneric1BPP::doGetSize() const
{
    return make_pair(height,width);
}

void DisplayGeneric1BPP::write(Point p, const char *text)
{
    font.draw(*this,textColor,p,text);
}

void DisplayGeneric1BPP::clippedWrite(Point p, Point a, Point b, const char *text)
{
    font.clippedDraw(*this,textColor,p,a,b,text);
}

void DisplayGeneric1BPP::clear(Color color)
{
    memset(backbuffer,conv2(color),fbSize);
}

void DisplayGeneric1BPP::clear(Point p1, Point p2, Color color)
{
    if(p1.x()<0 || p2.x()<p1.x() || p2.x()>=width
     ||p1.y()<0 || p2.y()<p1.y() || p2.y()>=height) return;
    
    //Vertical line is the most optimized
    for(short x=p1.x();x<=p2.x();x++) line(Point(x,p1.y()),Point(x,p2.y()),color);
}

void DisplayGeneric1BPP::beginPixel() {}

void DisplayGeneric1BPP::setPixel(Point p, Color color)
{
    int offset=p.x()+(p.y()/8)*width;
    if(offset<0 || offset>=fbSize) return;
    //TODO: optimize with bit banding
    if(color) backbuffer[offset] |=  (1<<(p.y() & 0x7));
    else      backbuffer[offset] &= ~(1<<(p.y() & 0x7));
}

void DisplayGeneric1BPP::line(Point a, Point b, Color color)
{
    //Horizontal line speed optimization
    if(a.y()==b.y())
    {
        short minx=min(a.x(),b.x());
        short maxx=max(a.x(),b.x());
        if(minx<0 || maxx>=width || a.y()<0 || a.y()>=height) return;
        for(short x=minx;x<=maxx;x++) doSetPixel(x,a.y(),color);
        return;
    }
    //Vertical line speed optimization
    if(a.x()==b.x())
    {
        short miny=min(a.y(),b.y());
        short maxy=max(a.y(),b.y())+1;
        if(a.x()<0 || a.x()>=width || miny<0 || maxy>height) return;
        if(maxy-miny<8) for(short y=miny;y<maxy;y++) doSetPixel(a.x(),y,color);
        else {
            short minyaligned=(miny+7) & ~0x7;
            short maxyaligned=maxy & ~0x7;
            for(short y=miny;y<minyaligned;y++) doSetPixel(a.x(),y,color);
            unsigned char *ptr=backbuffer+a.x()+(minyaligned/8)*width;
            for(short y=minyaligned;y<maxyaligned;y+=8)
            {
                *ptr=conv2(color);
                ptr+=width;
            }
            for(short y=maxyaligned;y<maxy;y++) doSetPixel(a.x(),y,color);
        }
        return;
    }
    //General case
    Line::draw(*this,a,b,color);
}

void DisplayGeneric1BPP::scanLine(Point p, const Color *colors, unsigned short length)
{
    if(p.x()<0 || static_cast<int>(p.x())+static_cast<int>(length)>width
        ||p.y()<0 || p.y()>=height) return;
    for(short x=0;x<length;x++) doSetPixel(p.x()+x,p.y(),colors[x]);
}

Color *DisplayGeneric1BPP::getScanLineBuffer()
{
    return buffer;
}

void DisplayGeneric1BPP::scanLineBuffer(Point p, unsigned short length)
{
    scanLine(p,buffer,length);
}

void DisplayGeneric1BPP::drawImage(Point p, const ImageBase& img)
{
    short int xEnd=p.x()+img.getWidth()-1;
    short int yEnd=p.y()+img.getHeight()-1;
    if(p.x()<0 || p.y()<0 || xEnd<p.x() || yEnd<p.y()
        ||xEnd >= width || yEnd >= height) return;

//    const unsigned short *imgData=img.getData();
//    if(imgData!=0)
//    {
//        //TODO Optimized version for in-memory images
//    } else
    img.draw(*this,p);
}

void DisplayGeneric1BPP::clippedDrawImage(Point p, Point a, Point b, const ImageBase& img)
{
//    const unsigned short *imgData=img.getData();
//    if(imgData!=0)
//    {
//        //TODO Optimized version for in-memory images
//    } else
    img.clippedDraw(*this,p,a,b);
}

void DisplayGeneric1BPP::drawRectangle(Point a, Point b, Color c)
{
    line(a,Point(b.x(),a.y()),c);
    line(Point(b.x(),a.y()),b,c);
    line(b,Point(a.x(),b.y()),c);
    line(Point(a.x(),b.y()),a,c);
}

DisplayGeneric1BPP::pixel_iterator DisplayGeneric1BPP::begin(Point p1, Point p2,
        IteratorDirection d)
{
    bool fail=false;
    if(p1.x()<0 || p1.y()<0 || p2.x()<0 || p2.y()<0) fail=true;
    if(p1.x()>=width || p1.y()>=height || p2.x()>=width || p2.y()>=height) fail=true;
    if(p2.x()<p1.x() || p2.y()<p1.y()) fail=true;
    if(fail)
    {
        //Return invalid (dummy) iterators
        this->last=pixel_iterator();
        return this->last;
    }

    //Set the last iterator to a suitable one-past-the last value
    if(d==DR) this->last=pixel_iterator(Point(p2.x()+1,p1.y()),p2,d,this);
    else this->last=pixel_iterator(Point(p1.x(),p2.y()+1),p2,d,this);

    return pixel_iterator(p1,p2,d,this);
}

DisplayGeneric1BPP::~DisplayGeneric1BPP() {}

} //namespace mxgui
