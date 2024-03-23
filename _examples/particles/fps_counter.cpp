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

#include <algorithm>
#include "fps_counter.h"

#ifdef _MIOSIX
#include "miosix.h"
using namespace miosix;
#else //_MIOSIX
#include <unistd.h>
#endif //_MIOSIX

FpsCounter::FpsCounter() : fpsCap(0), cnt(0), cpu(0), fps(0),
        cpuAvgAccum(0), fpsAvgAccum(0), prev(0), next(0) {}

void FpsCounter::setFpsCap(unsigned short cap)
{
    fpsCap=std::min<int>(cap,100);
    cnt=cpuAvgAccum=fpsAvgAccum=0;
    if(fpsCap==0) cpu=100; //In this case CPU% is assumed to be 100%
}

void FpsCounter::sleepBetweenFrames()
{
    #if _MIOSIX
    const long long now=getTime();

    const long long deltaT=now-prev;
    prev=now;
    fpsAvgAccum+=deltaT==0 ? 9990 : static_cast<int>(10000000000LL/deltaT);
    
    if(fpsCap!=0)
    {
        const long long period=1000000000LL/fpsCap;
        if(now>=next) //"deadline miss"
        {
            next=now+period;
            cpuAvgAccum+=100;
        } else {
            const long long sleepT=std::min(period,static_cast<long long>(next-now));
            cpuAvgAccum+=static_cast<unsigned int>((100*(period-sleepT))/period);
            miosix::Thread::nanoSleepUntil(next);
            next+=period;
        }
    }

    if(++cnt>=updatePeriod)
    {
        fps=((fpsAvgAccum/updatePeriod)+5)/10; // +5 to round up
        if(fpsCap!=0) cpu=cpuAvgAccum/updatePeriod;
        cnt=cpuAvgAccum=fpsAvgAccum=0;
    }
    #else //_MIOSIX
    if(fpsCap>0) usleep(1000000/fpsCap);
    #endif //_MIOSIX
}
