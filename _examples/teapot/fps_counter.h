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

#ifndef FPS_COUNTER_H
#define	FPS_COUNTER_H

/**
 * Class to sense and control the number of frames per second of a graphic
 * application, such as 3D rendering.
 */
class FpsCounter
{
public:
    /**
     * Constructor, defaults to uncapped fps.
     */
    FpsCounter();

    /**
     * \param fps limit to fps value. Can be between 1 and 100, or 0 to disable
     * fps capping. In this case sleepBetweenFrames returns immediately.
     */
    void setFpsCap(unsigned short cap);

    /**
     * \return the currently set fps capping value
     */
    unsigned short getFpsCap() const { return fpsCap; }

    /**
     * \return actual measured fps value.
     * Note: on the simulator just returns zero.
     */
    unsigned short getFps() const { return fps; }

    /**
     * \return an estimate of CPU usage based on the ratio between time spent
     * sleeping in sleepBetweenFrames() and the running time.
     * Note: on the simulator just returns 0%.
     */
    unsigned short getCpuUsed() const { return cpu; }

    /**
     * Meant to be called once per drawn frame, it sleeps as needed to keep
     * desired fps rate, and updates fps and cpu statistics.
     */
    void sleepBetweenFrames();

    //Uses default copy constructor and operator=
private:
    unsigned short fpsCap, cnt, cpu, fps;
    unsigned int cpuAvg, fpsAvg;
    long long prev, next;
    static const int updatePeriod=20;
};

#endif //FPS_COUNTER_H
