/***************************************************************************
 *   Copyright (C) 2014 by Terraneo Federico                               *
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

#include "drawing_context_proxy.h"

#ifdef MXGUI_LEVEL_2

namespace mxgui {

//
// class DrawingContextProxy
//

DrawingContextProxy::~DrawingContextProxy() {}

//
// class FullScreenDrawingContextProxy
//

FullScreenDrawingContextProxy::FullScreenDrawingContextProxy(Display& display)
    : dc(display) {}

void FullScreenDrawingContextProxy::write(Point p, const char *text)
{
    dc.write(p,text);
}

void FullScreenDrawingContextProxy::clippedWrite(Point p, Point a, Point b, const char *text)
{
    dc.clippedWrite(p,a,b,text);
}

void FullScreenDrawingContextProxy::clear(Color color)
{
    dc.clear(color);
}

void FullScreenDrawingContextProxy::clear(Point p1, Point p2, Color color)
{
    dc.clear(p1,p2,color);
}

void FullScreenDrawingContextProxy::line(Point a, Point b, Color color)
{
    dc.line(a,b,color);
}

void FullScreenDrawingContextProxy::scanLine(Point p, const Color *colors, unsigned short length)
{
    dc.scanLine(p,colors,length);
}

Color *FullScreenDrawingContextProxy::getScanLineBuffer()
{
    return dc.getScanLineBuffer();
}

void FullScreenDrawingContextProxy::scanLineBuffer(Point p, unsigned short length)
{
    dc.scanLineBuffer(p,length);
}

void FullScreenDrawingContextProxy::drawImage(Point p, const ImageBase& img)
{
    dc.drawImage(p,img);
}

void FullScreenDrawingContextProxy::clippedDrawImage(Point p, Point a, Point b, const ImageBase& img)
{
    dc.clippedDrawImage(p,a,b,img);
}

void FullScreenDrawingContextProxy::drawRectangle(Point a, Point b, Color c)
{
    dc.drawRectangle(a,b,c);
}

short int FullScreenDrawingContextProxy::getHeight() const
{
    return dc.getHeight();
}

short int FullScreenDrawingContextProxy::getWidth() const
{
    return dc.getWidth();
}

void FullScreenDrawingContextProxy::setTextColor(std::pair<Color,Color> colors)
{
    dc.setTextColor(colors);
}

std::pair<Color,Color> FullScreenDrawingContextProxy::getTextColor() const
{
    return dc.getTextColor();
}

void FullScreenDrawingContextProxy::setFont(const Font& font)
{
    dc.setFont(font);
}

Font FullScreenDrawingContextProxy::getFont() const
{
    return dc.getFont();
}

//
// class BackgroudDrawingContextProxy
//



//
// class ForegroundDrawingContextProxy
//



} //namespace miosix

#endif //MXGUI_LEVEL_2
