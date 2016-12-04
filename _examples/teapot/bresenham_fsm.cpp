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

#include "bresenham_fsm.h"

BresenhamFSM::BresenhamFSM() : h(-1) {}

BresenhamFSM::BresenhamFSM(mxgui::Point a, mxgui::Point b)
{
    if(a.y()>b.y()) std::swap(a,b);
    const short dx=b.x()-a.x();
    const short ady=b.y()-a.y(); //Guaranteed to be positive due to swap above
    const short adx=std::abs(dx);
    if(adx>ady)
    {
        flag=0;
        d=2*ady-adx;
        v=2*(ady-adx);
        w=2*ady;
        h=a.x();
        k=b.x();
    } else {
        flag= dx>=0 ? 1 : -1;
        d=2*adx-ady;
        v=2*(adx-ady);
        w=2*adx;
        h=a.x();
        k=ady;
    }
}

bool BresenhamFSM::drawScanLine(mxgui::Color scanLine[], mxgui::Color lineColor)
{
    if(h<0) return false; //h<0 is used to signal that the line has ended
    if(flag==0) //this implies adx>ady
    {
        if(h<k) //line slope is positive?
        {
            for(;h<=k;h++)
            {
                scanLine[h]=lineColor;
                if(d>0)
                {
                    d+=v;
                    h++;
                    break; //Moving to next y scanLine
                } else d+=w;
            }
            if(h>k) h=-1; //Line drawing has ended
        } else {
            for(;h>=k;h--)
            {
                scanLine[h]=lineColor;
                if(d>0)
                {
                    d+=v;
                    h--;
                    break; //Moving to next y scanLine
                } else d+=w;
            }
            if(h<k) h=-1; //Line drawing has ended
        }
    } else { //Steep line have only one pixel to draw per scanline
        scanLine[h]=lineColor;
        if(d>0)
        {
            h+=flag;
            d+=v;
        } else d+=w;
        if(--k<0) h=-1; //Line drawing has ended
    }
    return true;
}

std::pair<short,short> BresenhamFSM::getLinePoints()
{
    if(h<0) return std::make_pair(-1,-1); //h<0 : the line has ended
    short leftmost, rightmost;
    if(flag==0) //this implies adx>ady
    {
        if(h<k) //line slope is positive?
        {
            leftmost=h;
            for(;h<=k;h++)
            {
                if(d>0)
                {
                    d+=v;
                    h++;
                    break; //Moving to next y scanLine
                } else d+=w;
            }
            rightmost=h-1;
            if(h>k) h=-1; //Line drawing has ended
        } else {
            rightmost=h;
            for(;h>=k;h--)
            {
                if(d>0)
                {
                    d+=v;
                    h--;
                    break; //Moving to next y scanLine
                } else d+=w;
            }
            leftmost=h+1;
            if(h<k) h=-1; //Line drawing has ended
        }
    } else { //Steep line have only one pixel to draw per scanline
        leftmost=rightmost=h;
        if(d>0)
        {
            h+=flag;
            d+=v;
        } else d+=w;
        if(--k<0) h=-1; //Line drawing has ended
    }
    return std::make_pair(leftmost,rightmost);
}

/*
// Testcase code for BresenhamFSM class

#include "mxgui/entry.h"
#include "mxgui/display.h"
#include "mxgui/misc_inst.h"
#include <cmath>
#include <unistd.h>

using namespace mxgui;

void ll(float angle)
{
    DrawingContext dc(DisplayManager::instance().getDisplay());
    dc.clear(0);
    Point a(240/2,320/2),b(240/2+100*std::cos(angle),320/2+100*std::sin(angle));

    //Testcase for BresenhamFSM::drawScanLine()
//    BresenhamFSM lineFSM(a,b);
//    int y=320/2;
//    for(;;)
//    {
//        Color line[240];
//        memset(line,0,sizeof(line));
//        if(lineFSM.drawScanLine(line,0xffff)==false) break;
//        dc.scanLine(Point(0,y++),line,240);
//    }
//    dc.line(a,b,0xff00);

    //Draw crosshair around point b to spot eventual pixels beyond the line
    dc.beginPixel();
    dc.setPixel(Point(b.x(),b.y()+2),blue);
    dc.setPixel(Point(b.x(),b.y()+3),blue);
    dc.setPixel(Point(b.x()+2,b.y()),blue);
    dc.setPixel(Point(b.x()+3,b.y()),blue);
    dc.setPixel(Point(b.x(),b.y()-2),blue);
    dc.setPixel(Point(b.x(),b.y()-3),blue);
    dc.setPixel(Point(b.x()-2,b.y()),blue);
    dc.setPixel(Point(b.x()-3,b.y()),blue);

    //Testcase for BresenhamFSM::getLinePoints()
    dc.line(a,b,white);
    BresenhamFSM lineFSM(a,b);
    dc.beginPixel();
    for(int y=320/2;;y++)
    {

        pair<short,short> l=lineFSM.getLinePoints();
        if(l.first==-1) break;
        if(l.first!=l.second)
        {
            dc.setPixel(Point(l.first,y),red);
            dc.setPixel(Point(l.second,y),green);
        } else dc.setPixel(Point(l.first,y),red | green);
    }
}

ENTRY()
{
    for(float angle=0;;angle+=5*M_PI/360.0f) { ll(angle); usleep(1000000); }
}
*/
