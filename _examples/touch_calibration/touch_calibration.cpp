/***************************************************************************
 *   Copyright (C) 2010, 2011, 2012, 2013, 2014 by Terraneo Federico       *
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
#include "mxgui/entry.h"
#include "mxgui/display.h"
#include "mxgui/misc_inst.h"
#include "mxgui/level2/input.h"
#include <cstdio>
#include <cstring>
#include <algorithm>

using namespace mxgui;

// function to draw the crosses
static void drawCross(DrawingContext &dc, Point c, int r)
{
    dc.line(Point(c.x() - r, c.y()), Point(c.x() + r, c.y()), white);
    dc.line(Point(c.x(), c.y() - r), Point(c.x(), c.y() + r), white);
}

struct Calib
{
    double min, max;
};

// The transformation from RAW to pixels is performed through a linear transformation of the form:
//    pixel = a * raw + b
static void calibrationFrom2Points(double raw1, double pix1,
                                   double raw2, double pix2,
                                   double W,
                                   Calib &out)
{
    double a = (pix2 - pix1) / (raw2 - raw1);
    double b = pix1 - a * raw1;

    out.min = -b / a;
    out.max = (W - b) / a;
}

ENTRY()
{
    Display &display = DisplayManager::instance().getDisplay();
    InputHandler &backend = InputHandler::instance();

    // calibration reset
    backend.setTouchscreenCalibration(0.0, 0.0, 0.0, 0.0);

    const short w = display.getWidth() - 1;
    const short h = display.getHeight() - 1;

    short oldX = 0, oldY = 0;

    // cross points array
    Point targets[] = {
        Point(30, 30),
        Point(w - 30, 30),
        Point(w - 30, h - 30),
        Point(30, h - 30)};

    enum State
    {
        WAIT_DOWN,
        WAIT_UP
    };
    State state = WAIT_DOWN;

    int idx = 0;

    DrawingContext dc(display);
    drawCross(dc, targets[idx], 20);

    Point rawDatas[4];

    for (;;)
    {
        Event e = backend.getEvent();

        if (e.getEvent() != EventType::TouchDown &&
            e.getEvent() != EventType::TouchMove &&
            e.getEvent() != EventType::TouchUp)
        {
            continue;
        }

        if (state == WAIT_DOWN)
        {
            if (e.getEvent() == EventType::TouchDown)
            {
                rawDatas[idx] = e.getPoint();

                // I wait for the touch-up before showing the next cross.
                state = WAIT_UP;
            }
        }
        else
        {
            if (e.getEvent() == EventType::TouchUp)
            {
                idx++;

                if (idx >= 4)
                {
                    dc.clear(black);

                    // compute the calibration parameters
                    Calib cx1, cy1, cx2, cy2;

                    calibrationFrom2Points((double)rawDatas[0].x(), (double)targets[0].x(),
                                           (double)rawDatas[2].x(), (double)targets[2].x(),
                                           w, cx1);

                    calibrationFrom2Points((double)rawDatas[0].y(), (double)targets[0].y(),
                                           (double)rawDatas[2].y(), (double)targets[2].y(),
                                           h, cy1);

                    calibrationFrom2Points((double)rawDatas[1].x(), (double)targets[1].x(),
                                           (double)rawDatas[3].x(), (double)targets[3].x(),
                                           w, cx2);

                    calibrationFrom2Points((double)rawDatas[1].y(), (double)targets[1].y(),
                                           (double)rawDatas[3].y(), (double)targets[3].y(),
                                           h, cy2);

                    cx1.max = (cx1.max + cx2.max) / 2;
                    cx1.min = (cx1.min + cx2.min) / 2;
                    cy1.max = (cy1.max + cy2.max) / 2;
                    cy1.min = (cy1.min + cy2.min) / 2;

                    backend.setTouchscreenCalibration(cx1.min, cx1.max, cy1.min, cy1.max);
                    for (;;)
                    {
                        Event e2 = backend.getEvent();
                        switch (e2.getEvent())
                        {
                        case EventType::ButtonA:
                            display.turnOff();
                            return 0;
                        case EventType::TouchDown:
                        case EventType::TouchUp:
                        case EventType::TouchMove:
                        {
                            dc.line(Point(0, oldY), Point(w, oldY), black);
                            dc.line(Point(oldX, 0), Point(oldX, h), black);
                            oldX = e2.getPoint().x();
                            oldY = e2.getPoint().y();

                            dc.line(Point(0, oldY), Point(w, oldY), white);
                            dc.line(Point(oldX, 0), Point(oldX, h), white);
                            char line[128];
                            siprintf(line, "(%d, %d)          ", oldX, oldY);
                            dc.write(Point(0, 0), line);
                            break;
                        }
                        default:
                            break;
                        }
                    }
                }

                // mostra prossima croce
                dc.clear(black);
                drawCross(dc, targets[idx], 20);

                state = WAIT_DOWN;
            }
        }
    }
}
