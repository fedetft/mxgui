/***************************************************************************
 *   Copyright (C) 2024 by Ignazio Neto dell'Acqua                         *
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
#include <vector>

using namespace std;
using namespace mxgui;

struct ColorArea {
    Color color;
    unsigned short x1, y1, x2, y2; 
};

struct InvalidArea{    
    unsigned short x1, y1, x2, y2; 
};

class SimonGame {
private:
    Display& display;
    InputHandler& inputHandler;
    std::vector<int> sequence;
    std::vector<ColorArea> areas;    
    int baseDelay = 3000;
    int minDelay = 100; 
    std::vector<InvalidArea> invalidAreas;  
    
    void fillRectangle(ColorArea colorArea) {
        DrawingContext dc(display);
        for (int y = colorArea.y1; y <= colorArea.y2; ++y) {
            dc.line(Point(colorArea.x1, y), Point(colorArea.x2, y), colorArea.color);
        }
    }

    void drawAllAreas() {
        DrawingContext dc(display);
        for (ColorArea colorArea : areas) {
            fillRectangle(colorArea);
        }
    }

    void displayGameOver() {
        {
            DrawingContext dc(display);
            dc.clear(black);
            dc.setTextColor(white, black); 
            dc.setFont(tahoma);
            dc.write(Point(display.getWidth() / 2 - 30, display.getHeight() / 2 - 10), "Game Over"); 
        }
        miosix::delayMs(2000);
    }

    void removePendingEvents(){
        for (;;) {            
            Event e = inputHandler.popEvent(); 
            if (e.getEvent() == EventType::None) {
                break;
            }
        }
    }

    void initialMenu() {
        removePendingEvents();
        drawAllAreas();

        {
            DrawingContext dc(display);
            dc.setTextColor(white, black);
            dc.setFont(tahoma); 
            
            dc.write(Point(display.getWidth() / 2 - 47, display.getHeight() / 2 - 10), "SIMON SAYS GAME");
            dc.write(Point(display.getWidth() / 2 - 72, display.getHeight() / 2 + 10), "Scroll or press button to start");
        }
        
        for (;;) {
            Event e = inputHandler.popEvent();
            if (e.getEvent() == EventType::TouchMove || e.getEvent() == EventType::ButtonJoy || e.getEvent() == EventType::ButtonA) {
                DrawingContext dc(display);
                miosix::delayMs(1000);
                dc.clear(black);
                break;
            }
        }
    }

    void displayColor(int color) {
        ColorArea area = areas[color];
        fillRectangle(area);
    }
    
    void displaySequence() {
        int delay = max(minDelay, baseDelay - static_cast<int>(sequence.size() * 200));
        miosix::delayMs(200);
        for (int color : sequence) {
            DrawingContext dc(display);
            dc.clear(black);
            miosix::delayMs(100);
            displayColor(color);
            miosix::delayMs(delay);
        }
    }

    Point getCenter(ColorArea colorArea) {
        unsigned short centerX = (colorArea.x1 + colorArea.x2) / 2;
        unsigned short centerY = (colorArea.y1 + colorArea.y2) / 2;
        return Point(centerX, centerY); 
    }


    Point handleTouch(){
        removePendingEvents();
        for(;;)
        {
            Event e = inputHandler.popEvent();
            switch(e.getEvent())
            { 
                case EventType::TouchDown:
                {
                    Point touchedPoint = e.getPoint();
                    bool isInvalid = false;
                    for(InvalidArea invalidArea : invalidAreas){
                        if (touchedPoint.x() >= invalidArea.x1 && touchedPoint.x() <= invalidArea.x2 && touchedPoint.y() >= invalidArea.y1 && touchedPoint.y() <= invalidArea.y2){
                            isInvalid = true;
                            break;
                        }
                    }
                    if(!isInvalid){
                        return e.getPoint();
                    }
                    break;
                }
                case EventType::ButtonUpDown:
                {
                    return getCenter(areas[3]);
                    break;
                }
                case EventType::ButtonDownDown:
                {
                    return getCenter(areas[1]);
                    break;
                }
                case EventType::ButtonLeftDown:
                {
                    return getCenter(areas[0]);
                    break;
                }
                case EventType::ButtonRightDown:
                {
                    return getCenter(areas[2]);
                    break;
                }
                default:
                break;
            }
        }
    }    

    bool getUserInput() {
        for (int expectedColor : sequence) {
            Point touchedPoint = handleTouch();
            ColorArea area = areas[expectedColor];
            if (!(touchedPoint.x() >= area.x1 && touchedPoint.x() <= area.x2 && touchedPoint.y() >= area.y1 && touchedPoint.y() <= area.y2))
                return false;
            miosix::delayMs(100);
        }
        sequence.push_back(static_cast<int>(rand() % 4));
        return true;
    }

public:
    SimonGame(Display& disp, InputHandler& handler)
        : display(disp), inputHandler(handler) {
        srand(static_cast<unsigned int>(time(0)));        
        unsigned short maxX = display.getWidth() - 1;
        unsigned short maxY = display.getHeight() - 1;
        areas.push_back({red, static_cast<unsigned short>(maxX / 4), 0, static_cast<unsigned short>(3* maxX / 4), static_cast<unsigned short>(maxY / 3)});
        areas.push_back({blue, 0, static_cast<unsigned short>(maxY / 3), static_cast<unsigned short>(maxX / 2), static_cast<unsigned short>(2 * maxY / 3)});
        areas.push_back({green, static_cast<unsigned short>(maxX / 2), static_cast<unsigned short>(maxY / 3), maxX, static_cast<unsigned short>(2 * maxY / 3)});
        areas.push_back({white, static_cast<unsigned short>(maxX / 4), static_cast<unsigned short>(2 * maxY / 3), static_cast<unsigned short>(3* maxX / 4), maxY});
        invalidAreas.push_back({0, 0, static_cast<unsigned short>(maxX / 4), static_cast<unsigned short>(maxY / 3)});
        invalidAreas.push_back({static_cast<unsigned short>(3* maxX / 4), 0, maxX, static_cast<unsigned short>(maxY / 3)});
        invalidAreas.push_back({0, static_cast<unsigned short>(2* maxY / 3), static_cast<unsigned short>(maxX / 4), maxY});
        invalidAreas.push_back({static_cast<unsigned short>(3* maxX / 4), static_cast<unsigned short>(2* maxY / 3), maxX, maxY});
    }

   void run() {
        for (;;) {            
            initialMenu();
            sequence.clear();
            sequence.push_back(static_cast<int>(rand() % 4));
            
            for (;;) {
                displaySequence();
                drawAllAreas();
                if (!getUserInput()) {
                    displayGameOver(); 
                    break;
                }
            }
        }
    }
};

ENTRY()
{
    Display& display = DisplayManager::instance().getDisplay();
    InputHandler& inputHandler = InputHandler::instance();

    SimonGame game(display, inputHandler);
    game.run();

    return 0;
}
