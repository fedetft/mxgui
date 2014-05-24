/***************************************************************************
 *   Copyright (C) 2013 by Terraneo Federico                               *
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

#ifndef POWER_MANAGER_H
#define	POWER_MANAGER_H

#include <pthread.h>
#include <mxgui/display.h>
#ifdef _MIOSIX
#include <interfaces/bsp.h>
#endif //_MIOSIX


/**
 * A higher level power manager, adjusts display brightness based on ambient
 * light, and displays a battery icon if requested.
 */
class PowerManager
{
public:
    /**
     * \return an instance of the power manager (ingleton)
     */
    static PowerManager& instance();
    
    /**
     * Selects if the battery icon should be shown in the top left corner of the
     * screen. If called with true as parameter when the battery icon is already
     * shown, it forces a redraw of the icon.
     * \param show if true, show the battery icon
     */
    void showBatterIcon(bool show);
    
    /**
     * Although not related to the power management, this allows to show the
     * time in the top right corner of the screen. If called with true as
     * parameter when the time is already shown, it forces a redraw of the time.
     * \param show if true, show the time  
     */
    void showTime(bool show);
    
    /**
     * \return true if the battery icon is shown 
     */
    bool isBatteryIconShown() const { return showIconFlag; }
    
    /**
     * \return true if the time is shown 
     */
    bool isTimeShown() const { return showTimeFlag; }
    
    /**
     * Turn off everyithing and go in deep sleep until the power button is pressed
     */
    void powerSave();
    
private:
    PowerManager(const PowerManager&);
    PowerManager& operator= (const PowerManager&);
    
    /**
     * This is an active object, it contains a thread an this is its main loop
     */
    void mainLoop();
    
    /**
     * Trick to us a member function as a thread main loop 
     */
    static void* threadLauncher(void *arg)
    {
        reinterpret_cast<PowerManager*>(arg)->mainLoop();
        return 0;
    }
    
    /**
     * Draw the battery icon
     */
    void drawBatteryIcon();
    
    /**
     * Prints the time
     */
    void printTime();
    
    /**
     * Adjust display brightness
     */
    void adjustBrightness();
    
    /**
     * Constructor
     */
    PowerManager();
    
    mxgui::Display& display;
    #ifdef _MIOSIX
    miosix::PowerManagement& pmu;
    miosix::LightSensor& light;
    miosix::Rtc& rtc;
    #endif //_MIOSIX
    pthread_mutex_t mutex;
    bool showIconFlag;
    bool showTimeFlag;
    short count;
    short timeStrPixels; //Cached
    short oldBrightness;
};

#endif //POWER_MANAGER_H
