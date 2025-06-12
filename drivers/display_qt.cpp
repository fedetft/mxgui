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
#include "misc_inst.h"
#include "_tools/qtsimulator/window.h"
#include "line.h"

using namespace std;

namespace mxgui {

void registerDisplayHook(DisplayManager& dm)
{
    dm.registerDisplay(&DisplayImpl::instance());
}

//
// class DisplayImpl
//
const short int DisplayImpl::width;
const short int DisplayImpl::height;

DisplayImpl& DisplayImpl::instance()
{
    static DisplayImpl instance;
    return instance;
}

void DisplayImpl::doTurnOn()
{
    //Unsupported for this display, so just ignore
}

void DisplayImpl::doTurnOff()
{
    //Unsupported for this display, so just ignore
}

void DisplayImpl::doSetBrightness(int brt) {}

pair<short int, short int> DisplayImpl::doGetSize() const
{
    return make_pair(height,width);
}

void DisplayImpl::write(Point p, const char *text)
{
    //Qt backend is meant to catch errors, so be bastard
    if(p.x()<0 || p.y()<0)
        throw(logic_error("DisplayImpl::write: negative value in point"));
    if(p.x()>=width || p.y()>=height)
        throw(logic_error("DisplayImpl::write: point outside display bounds"));

    #ifndef PEDANTIC_ITERATORS_CHECK
    font.draw(*this,textColor,p,text);
    #else //PEDANTIC_ITERATORS_CHECK
    font.draw<DisplayImpl,true>(*this,textColor,p,text);
    #endif //PEDANTIC_ITERATORS_CHECK
    beginPixelCalled=false;
}

void DisplayImpl::clippedWrite(Point p, Point a, Point b, const char *text)
{
    //Qt backend is meant to catch errors, so be bastard
    if(a.x()<0 || a.y()<0 || b.x()<0 || b.y()<0)
        throw(logic_error("DisplayImpl::clippedWrite:"
                " negative value in point"));
    if(a.x()>=width || a.y()>=height || b.x()>=width || b.y()>=height)
        throw(logic_error("DisplayImpl::clippedWrite:"
                " point outside display bounds"));
    if(a.x()>b.x() || a.y()>b.y())
        throw(logic_error("DisplayImpl::clippedWrite: reversed points"));

    #ifndef PEDANTIC_ITERATORS_CHECK
    font.clippedDraw(*this,textColor,p,a,b,text);
    #else //PEDANTIC_ITERATORS_CHECK
    font.clippedDraw<DisplayImpl,true>(*this,textColor,p,a,b,text);
    #endif //PEDANTIC_ITERATORS_CHECK
    beginPixelCalled=false;
}

void DisplayImpl::clear(Color color)
{
    clear(Point(0,0),Point(width-1,height-1),color);
}

void DisplayImpl::clear(Point p1, Point p2, Color color)
{
    //Qt backend is meant to catch errors, so be bastard
    if(p1.x()<0 || p1.y()<0 || p2.x()<0 || p2.y()<0)
        throw(logic_error("DisplayImpl::clear: negative value in point"));
    if(p1.x()>=width || p1.y()>=height || p2.x()>=width || p2.y()>=height)
        throw(logic_error("DisplayImpl::clear: point outside display bounds"));
    if(p2.x()<p1.x() || p2.y()<p1.y())
        throw(logic_error("DisplayImpl::clear: p2<p1"));
    
    FrameBuffer& fb=backend.getFrameBuffer();
    for(int i=p1.x();i<=p2.x();i++)
        for(int j=p1.y();j<=p2.y();j++) fb.setPixel(i,j,color);
    beginPixelCalled=false;
}

void DisplayImpl::beginPixel()
{
    beginPixelCalled=true;
}

void DisplayImpl::setPixel(Point p, Color color)
{
    //Qt backend is meant to catch errors, so be bastard
    if(beginPixelCalled==false)
        throw(logic_error("DisplayImpl::setPixel: beginPixel not called"));
    if(p.x()<0 || p.y()<0)
        throw(logic_error("DisplayImpl::setPixel: negative value in point"));
    if(p.x()>=width || p.y()>=height)
        throw(logic_error("DisplayImpl::setPixel: point outside display bounds"));

    backend.getFrameBuffer().setPixel(p.x(),p.y(),color);
}

void DisplayImpl::line(Point a, Point b, Color color)
{
    //Qt backend is meant to catch errors, so be bastard
    if(a.x()<0 || a.y()<0 || b.x()<0 || b.y()<0)
        throw(logic_error("DisplayImpl::line: negative value in point"));
    if(a.x()>=width || a.y()>=height || b.x()>=width || b.y()>=height)
        throw(logic_error("DisplayImpl::line: point outside display bounds"));
    
    Line::draw(*this,a,b,color);
    beginPixelCalled=false;
}

void DisplayImpl::scanLine(Point p, const Color *colors, unsigned short length)
{
    //Qt backend is meant to catch errors, so be bastard
    if(p.x()<0 || p.y()<0)
        throw(logic_error("DisplayImpl::scanLine: negative value in point"));
    if(p.x()>=width || p.y()>=height)
        throw(logic_error("DisplayImpl::scanLine: point outside display bounds"));
    if(p.x()+length>width)
        throw(logic_error("DisplayImpl::scanLine: line too long"));
    pixel_iterator it=begin(p,Point(p.x()+length-1,p.y()),RD);
    for(int i=0;i<length;i++) *it=colors[i];
    beginPixelCalled=false;
}

Color *DisplayImpl::getScanLineBuffer()
{
    if(buffer==nullptr) buffer=new Color[getWidth()];
    return buffer;
}

void DisplayImpl::scanLineBuffer(Point p, unsigned short length)
{
    scanLine(p,buffer,length);
}

void DisplayImpl::drawImage(Point p, const ImageBase& img)
{
    short int xEnd=p.x()+img.getWidth()-1;
    short int yEnd=p.y()+img.getHeight()-1;

    //Qt backend is meant to catch errors, so be bastard
    if(xEnd >= width || yEnd >= height)
        throw(logic_error("Image out of bounds"));

    img.draw(*this,p);
    beginPixelCalled=false;
}

void DisplayImpl::clippedDrawImage(Point p, Point a, Point b,
        const ImageBase& img)
{
    //Qt backend is meant to catch errors, so be bastard
    if(a.x()<0 || a.y()<0 || b.x()<0 || b.y()<0)
        throw(logic_error("DisplayImpl::clippedDrawImage:"
                " negative value in point"));
    if(a.x()>=width || a.y()>=height || b.x()>=width || b.y()>=height)
        throw(logic_error("DisplayImpl::clippedDrawImage:"
                " point outside display bounds"));
    if(a.x()>b.x() || a.y()>b.y())
        throw(logic_error("DisplayImpl::clippedDrawImage: reversed points"));

    img.clippedDraw(*this,p,a,b);
    beginPixelCalled=false;
}

void DisplayImpl::drawRectangle(Point a, Point b, Color c)
{
    line(a,Point(b.x(),a.y()),c);
    line(Point(b.x(),a.y()),b,c);
    line(b,Point(a.x(),b.y()),c);
    line(Point(a.x(),b.y()),a,c);
}

void DisplayImpl::update()
{  
    backend.getSender()->update();
    beginPixelCalled=false;
}

DisplayImpl::pixel_iterator DisplayImpl::begin(Point p1, Point p2, IteratorDirection d)
{
    //Qt backend is meant to catch errors, so be bastard
    if(p1.x()<0 || p1.y()<0 || p2.x()<0 || p2.y()<0)
        throw(logic_error("DisplayImpl::begin: negative value in point"));
    if(p1.x()>=width || p1.y()>=height || p2.x()>=width || p2.y()>=height)
        throw(logic_error("DisplayImpl::begin: point outside display bounds"));
    if(p2.x()<p1.x() || p2.y()<p1.y())
        throw(logic_error("DisplayImpl::begin: p2<p1"));

    //Set the last iterator to a suitable one-past-the last value
    if(d==DR) this->last=pixel_iterator(Point(p2.x()+1,p1.y()),p2,d,this);
    else this->last=pixel_iterator(Point(p1.x(),p2.y()+1),p2,d,this);

    beginPixelCalled=false;
    return pixel_iterator(p1,p2,d,this);
}

DisplayImpl::~DisplayImpl()
{
    if(buffer) delete[] buffer;
}

DisplayImpl::DisplayImpl(): buffer(nullptr), last(), beginPixelCalled(false),
                            backend(QTBackend::instance())
{
    setTextColor(make_pair(Color(SIMULATOR_FGCOLOR),Color(SIMULATOR_BGCOLOR)));
}

} //namespace mxgui

#endif //_MIOSIX
