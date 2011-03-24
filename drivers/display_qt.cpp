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

#ifndef _MIOSIX

#include "display_qt.h"
#include "mxgui/misc_inst.h"
#include "mxgui/tools/qtsimulator/window.h"
#include <iostream>

using namespace std;

namespace mxgui {

//
// class DisplayQt
//

DisplayQt::DisplayQt(): textColor(), font(droid11), last(),
        beginPixelCalled(false), backend(QTBackend::instance())
{
    setTextColor(Color(0xffff),Color(0x0000));
}

void DisplayQt::write(Point p, const char *text)
{ 
    font.draw(*this,textColor,p,text);
    beginPixelCalled=false;
}

void DisplayQt::clear(Color color)
{
    clear(Point(0,0),Point(width-1,height-1),color);
}

void DisplayQt::clear(Point p1, Point p2, Color color)
{
    //Qt backend is meant to catch errors, so be bastard
    if(p1.x()<0 || p1.y()<0 || p2.x()<0 || p2.y()<0)
        throw(logic_error("DisplayQt::clear: negative value in point"));
    if(p1.x()>=width || p1.y()>=height || p2.x()>=width || p2.y()>=height)
        throw(logic_error("DisplayQt::clear: point outside display bounds"));
    if(p2.x()<p1.x() || p2.y()<p1.y())
        throw(logic_error("DisplayQt::clear: p2<p1"));
    
    FrameBuffer& fb=backend.getFrameBuffer();
    for(int i=p1.x();i<=p2.x();i++)
        for(int j=p1.y();j<=p2.y();j++) fb.setPixel(i,j,color.value());
    beginPixelCalled=false;
}

void DisplayQt::beginPixel()
{
    beginPixelCalled=true;
}

void DisplayQt::setPixel(Point p, Color color)
{
    //Qt backend is meant to catch errors, so be bastard
    if(beginPixelCalled==false)
        throw(logic_error("DisplayQt::setPixel: beginPixel not called"));
    if(p.x()<0 || p.y()<0)
        throw(logic_error("DisplayQt::setPixel: negative value in point"));
    if(p.x()>=width || p.y()>=height)
        throw(logic_error("DisplayQt::setPixel: point outside display bounds"));

    backend.getFrameBuffer().setPixel(p.x(),p.y(),color.value());
}

void DisplayQt::line(Point a, Point b, Color color)
{
    //Qt backend is meant to catch errors, so be bastard
    if(a.x()<0 || a.y()<0 || b.x()<0 || b.y()<0)
        throw(logic_error("DisplayQt::line: negative value in point"));
    if(a.x()>=width || a.y()>=height || b.x()>=width || b.y()>=height)
        throw(logic_error("DisplayQt::line: point outside display bounds"));

    beginPixel();
    const short int dx=b.x()-a.x();
    const short int dy=b.y()-a.y();
    if(dx==0 && dy==0)
    {
        setPixel(a,color);
        return;
    }
    if(abs(dx)>=abs(dy))
    {
        int m=(dy*width)/dx;
        short int x;
        if(dx>0)
        {
            for(x=a.x();x<b.x();x++)
                setPixel(Point(x,a.y()+((m*(x-a.x()))/width)),color);
            setPixel(b,color);
        } else {
            for(x=b.x();x<a.x();x++)
                setPixel(Point(x,b.y()+((m*(x-b.x()))/width)),color);
            setPixel(a,color);
        }
    } else {
        int m=(dx*height)/dy;
        short int y;
        if(dy>0)
        {
            for(y=a.y();y<b.y();y++)
                setPixel(Point(a.x()+((m*(y-a.y()))/height),y),color);
            setPixel(b,color);
        } else {
            for(y=b.y();y<a.y();y++)
                setPixel(Point(b.x()+((m*(y-b.y()))/height),y),color);
            setPixel(a,color);
        }
    }
    beginPixelCalled=false;
}

void DisplayQt::drawImage(Point p, Image img)
{
    short int xEnd=p.x()+img.getWidth()-1;
    short int yEnd=p.y()+img.getHeight()-1;

    //Qt backend is meant to catch errors, so be bastard
    if(xEnd >= width || yEnd >= height)
        throw(logic_error("Image out of bounds"));
    
    if(img.imageDepth()!=ImageDepth::DEPTH_16_BIT) return;
    pixel_iterator it=begin(p,Point(xEnd,yEnd),RD);
    const unsigned short *imgData=img.getData();
    int imgSize=img.getHeight()*img.getWidth();
    for(int i=0;i<imgSize;i++)
    {
        *it=Color(imgData[i]);
    }
}

void DisplayQt::drawRectangle(Point a, Point b, Color c)
{
    line(a,Point(b.x(),a.y()),c);
    line(Point(b.x(),a.y()),b,c);
    line(b,Point(a.x(),b.y()),c);
    line(Point(a.x(),b.y()),a,c);
}

void DisplayQt::turnOn()
{
    //Unsupported for this display, so just ignore
}

void DisplayQt::turnOff()
{
    //Unsupported for this display, so just ignore
}

void DisplayQt::setTextColor(Color fgcolor, Color bgcolor)
{
    unsigned short fgR=fgcolor.value(); //& 0xf800; Optimization, & not required
    unsigned short bgR=bgcolor.value(); //& 0xf800; Optimization, & not required
    unsigned short fgG=fgcolor.value() & 0x7e0;
    unsigned short bgG=bgcolor.value() & 0x7e0;
    unsigned short fgB=fgcolor.value() & 0x1f;
    unsigned short bgB=bgcolor.value() & 0x1f;
    unsigned short deltaR=((fgR-bgR)/3) & 0xf800;
    unsigned short deltaG=((fgG-bgG)/3) & 0x7e0;
    unsigned short deltaB=((fgB-bgB)/3) & 0x1f;
    unsigned short delta=deltaR | deltaG | deltaB;
    textColor[3]=fgcolor;
    textColor[2]=Color(bgcolor.value()+2*delta);
    textColor[1]=Color(bgcolor.value()+delta);
    textColor[0]=bgcolor;
    //cout<<hex<<"<"<<textColor[0].value()<<","<<textColor[1].value()<<","<<
    //        textColor[2].value()<<","<<textColor[3].value()<<">"<<endl;
}

void DisplayQt::setFont(const Font& font)
{
    this->font=font;
}

void DisplayQt::update()
{  
    backend.getSender()->update();
    beginPixelCalled=false;
}

DisplayQt::pixel_iterator DisplayQt::begin(Point p1, Point p2, IteratorDirection d)
{
    //Qt backend is meant to catch errors, so be bastard
    if(p1.x()<0 || p1.y()<0 || p2.x()<0 || p2.y()<0)
        throw(logic_error("DisplayQt::begin: negative value in point"));
    if(p1.x()>=width || p1.y()>=height || p2.x()>=width || p2.y()>=height)
        throw(logic_error("DisplayQt::begin: point outside display bounds"));
    if(p2.x()<p1.x() || p2.y()<p1.y())
        throw(logic_error("DisplayQt::begin: p2<p1"));

    //Set the last iterator to a suitable one-past-the last value
    if(d==DR) this->last=pixel_iterator(Point(p2.x()+1,p1.y()),p2,d,this);
    else this->last=pixel_iterator(Point(p1.x(),p2.y()+1),p2,d,this);

    beginPixelCalled=false;
    return pixel_iterator(p1,p2,d,this);
}

} //namespace mxgui

#endif //_MIOSIX
