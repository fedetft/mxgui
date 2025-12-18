/*
 * mxgui port of the `xrayswarm' screensaver from xscreensaver.
 * Modifications Copyright (c) 2025 Daniele Cattaneo
 * 
 * The modifications port the screensaver to mxgui, and remove/change
 * several ugly features of the original (the entire randomized color schemes
 * system, all randomness on the number of bugs/targets, some of the random
 * small changes.) I could have C++-ified the code but I didn't bother, sorry.
 */
/*
 * Copyright (c) 2000 by Chris Leger (xrayjones@users.sourceforge.net)
 *
 * xrayswarm - a shameless ripoff of the 'swarm' screensaver on SGI
 *   boxes.
 *
 * Version 1.0 - initial release.  doesn't read any special command-line
 *   options, and only supports the variable 'delay' via Xresources.
 *   (the delay resouces is most useful on systems w/o gettimeofday, in
 *   which case automagical level-of-detail for FPS maintainance can't
 *   be used.)
 *
 *   The code isn't commented, but isn't too ugly. It should be pretty
 *   easy to understand, with the exception of the colormap stuff.
 *
 */
/*
Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.
*/

#include "mxgui/entry.h"
#include "mxgui/display.h"
#include "mxgui/misc_inst.h"
#include <cmath>
#include <sys/time.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

using namespace std;
using namespace mxgui;

#define MAX_TRAIL_LEN 60
#define MAX_BUGS 25
#define MAX_TARGETS 10

#define COLOR_CHANGE_PERIOD 1000

#define sq(x) ((x)*(x))

struct bug
{
    float pos[3];
    int hist[MAX_TRAIL_LEN][2];
    float vel[2];
    bug *closest;
};

struct state
{
    int xsize, ysize;
    int xc, yc;
    unsigned long delay;
    float maxx, maxy;

    float dt;
    float targetVel;
    float targetAcc;
    float maxVel;
    float maxAcc;
    float noise;
    float minVelMultiplier;

    int nbugs;
    int ntargets;
    int trailLen;

    float dtInv;
    float halfDtSq;
    float targetVelSq;
    float maxVelSq;
    float minVelSq;
    float minVel;

    bug bugs[MAX_BUGS];
    bug targets[MAX_TARGETS];
    int head;
    int tail;
    float changeProb;
    int colorWheelTime;
    int checkIndex;
};

static inline double frand(double max)
{
    return ((double)random() * max) / (double)0x7fffffff;
}

static void initGraphics(state *st)
{
    st->xsize = DisplayManager::instance().getDisplay().getWidth()-1;
    st->ysize = DisplayManager::instance().getDisplay().getHeight()-1;
    st->xc = st->xsize >> 1;
    st->yc = st->ysize >> 1;

    st->maxx = 1.0;
    st->maxy = st->ysize/(float)st->xsize;
}

static void initBugs(state *st)
{
    bug *b;
    int i;

    st->head = st->tail = 0;

    memset((char *)st->bugs, 0,MAX_BUGS*sizeof(bug));
    memset((char *)st->targets, 0, MAX_TARGETS*sizeof(bug));

    if (st->ntargets < 0) st->ntargets = MAX_TARGETS*8/10;
    if (st->ntargets < 1) st->ntargets = 1;

    if (st->nbugs < 0) st->nbugs = MAX_BUGS*8/10;
    if (st->nbugs <= st->ntargets) st->nbugs = st->ntargets+1;

    if (st->trailLen < 0) st->trailLen = MAX_TRAIL_LEN;

    if (st->nbugs > MAX_BUGS) st->nbugs = MAX_BUGS;
    if (st->ntargets > MAX_TARGETS) st->ntargets = MAX_TARGETS;
    if (st->trailLen > MAX_TRAIL_LEN) st->trailLen = MAX_TRAIL_LEN;

    b = st->bugs;
    for (i = 0; i < st->nbugs; i++, b++)
    {
        b->pos[0] = frand(st->maxx);
        b->pos[1] = frand(st->maxy);
        b->vel[0] = frand(st->maxVel/2);
        b->vel[1] = frand(st->maxVel/2);

        b->hist[st->head][0] = b->pos[0]*st->xsize;
        b->hist[st->head][1] = b->pos[1]*st->xsize;
        b->closest = &st->targets[random()%st->ntargets];
    }

    b = st->targets;
    for (i = 0; i < st->ntargets; i++, b++)
    {
        b->pos[0] = frand(st->maxx);
        b->pos[1] = frand(st->maxy);

        b->vel[0] = frand(st->targetVel/2);
        b->vel[1] = frand(st->targetVel/2);

        b->hist[st->head][0] = b->pos[0]*st->xsize;
        b->hist[st->head][1] = b->pos[1]*st->xsize;
    }
}

static void computeConstants(state *st)
{
    st->halfDtSq = st->dt*st->dt*0.5;
    st->dtInv = 1.0/st->dt;
    st->targetVelSq = st->targetVel*st->targetVel;
    st->maxVelSq = st->maxVel*st->maxVel;
    st->minVel = st->maxVel*st->minVelMultiplier;
    st->minVelSq = st->minVel*st->minVel;
}

static void updateState(state *st)
{
    int i;
    bug *b;
    float ax, ay, temp;
    float theta;
    bug *b2;
    int j;

    st->colorWheelTime = (st->colorWheelTime+1)%COLOR_CHANGE_PERIOD;
    st->head = (st->head + 1) % st->trailLen;

    for (j = 0; j < 5; j++)
    {
        /* update closest target for the bug indicated by checkIndex */
        st->checkIndex = (st->checkIndex + 1) % st->nbugs;
        b = &st->bugs[st->checkIndex];

        ax = b->closest->pos[0] - b->pos[0];
        ay = b->closest->pos[1] - b->pos[1];
        temp = ax * ax + ay * ay;
        for (i = 0; i < st->ntargets; i++)
        {
            b2 = &st->targets[i];
            if (b2 == b->closest) continue;
            ax = b2->pos[0] - b->pos[0];
            ay = b2->pos[1] - b->pos[1];
            theta = ax * ax + ay * ay;
            if (theta < temp * 2)
            {
                b->closest = b2;
                temp = theta;
            }
        }
    }

    /* update target state */

    b = st->targets;
    for (i = 0; i < st->ntargets; i++, b++)
    {
        theta = frand(6.28);
        ax = st->targetAcc * cos(theta);
        ay = st->targetAcc * sin(theta);

        b->vel[0] += ax * st->dt;
        b->vel[1] += ay * st->dt;

        /* check velocity */
        temp = sq(b->vel[0]) + sq(b->vel[1]);
        if (temp > st->targetVelSq)
        {
            temp = st->targetVel / sqrt(temp);
            /* save old vel for acc computation */
            ax = b->vel[0];
            ay = b->vel[1];

            /* compute new velocity */
            b->vel[0] *= temp;
            b->vel[1] *= temp;

            /* update acceleration */
            ax = (b->vel[0] - ax) * st->dtInv;
            ay = (b->vel[1] - ay) * st->dtInv;
        }

        /* update position */
        b->pos[0] += b->vel[0] * st->dt + ax * st->halfDtSq;
        b->pos[1] += b->vel[1] * st->dt + ay * st->halfDtSq;

        /* check limits on targets */
        if (b->pos[0] < 0)
        {
            /* bounce */
            b->pos[0] = -b->pos[0];
            b->vel[0] = -b->vel[0];
        }
        else if (b->pos[0] >= st->maxx)
        {
            /* bounce */
            b->pos[0] = 2 * st->maxx - b->pos[0];
            b->vel[0] = -b->vel[0];
        }
        if (b->pos[1] < 0)
        {
            /* bounce */
            b->pos[1] = -b->pos[1];
            b->vel[1] = -b->vel[1];
        }
        else if (b->pos[1] >= st->maxy)
        {
            /* bounce */
            b->pos[1] = 2 * st->maxy - b->pos[1];
            b->vel[1] = -b->vel[1];
        }

        b->hist[st->head][0] = b->pos[0] * st->xsize;
        b->hist[st->head][1] = b->pos[1] * st->xsize;
    }

    /* update bug state */
    b = st->bugs;
    for (i = 0; i < st->nbugs; i++, b++)
    {
        theta = atan2(b->closest->pos[1] - b->pos[1] + frand(st->noise),
                      b->closest->pos[0] - b->pos[0] + frand(st->noise));
        ax = st->maxAcc * cos(theta);
        ay = st->maxAcc * sin(theta);

        b->vel[0] += ax * st->dt;
        b->vel[1] += ay * st->dt;

        /* check velocity */
        temp = sq(b->vel[0]) + sq(b->vel[1]);
        if (temp > st->maxVelSq)
        {
            temp = st->maxVel / sqrt(temp);

            /* save old vel for acc computation */
            ax = b->vel[0];
            ay = b->vel[1];

            /* compute new velocity */
            b->vel[0] *= temp;
            b->vel[1] *= temp;

            /* update acceleration */
            ax = (b->vel[0] - ax) * st->dtInv;
            ay = (b->vel[1] - ay) * st->dtInv;
        } else if (temp < st->minVelSq) {
            temp = st->minVel / sqrt(temp);

            /* save old vel for acc computation */
            ax = b->vel[0];
            ay = b->vel[1];

            /* compute new velocity */
            b->vel[0] *= temp;
            b->vel[1] *= temp;

            /* update acceleration */
            ax = (b->vel[0] - ax) * st->dtInv;
            ay = (b->vel[1] - ay) * st->dtInv;
        }

        /* update position */
        b->pos[0] += b->vel[0] * st->dt + ax * st->halfDtSq;
        b->pos[1] += b->vel[1] * st->dt + ay * st->halfDtSq;

        /* check limits on targets */
        if (b->pos[0] < 0)
        {
            /* bounce */
            b->pos[0] = -b->pos[0];
            b->vel[0] = -b->vel[0];
        } else if (b->pos[0] >= st->maxx) {
            /* bounce */
            b->pos[0] = 2 * st->maxx - b->pos[0];
            b->vel[0] = -b->vel[0];
        }
        if (b->pos[1] < 0)
        {
            /* bounce */
            b->pos[1] = -b->pos[1];
            b->vel[1] = -b->vel[1];
        } else if (b->pos[1] >= st->maxy) {
            /* bounce */
            b->pos[1] = 2 * st->maxy - b->pos[1];
            b->vel[1] = -b->vel[1];
        }

        b->hist[st->head][0] = b->pos[0] * st->xsize;
        b->hist[st->head][1] = b->pos[1] * st->xsize;
    }
}

static void mutateBug(state *st, int which)
{
    int i, j;

    if (which == 0)
    {
        /* turn bug into target */
        if (st->ntargets < MAX_TARGETS - 1 && st->nbugs > 1)
        {
            i = random() % st->nbugs;
            memcpy((char *)&st->targets[st->ntargets], (char *)&st->bugs[i], sizeof(bug));
            memcpy((char *)&st->bugs[i], (char *)&st->bugs[st->nbugs - 1], sizeof(bug));
            // st->targets[st->ntargets].pos[0] = frand(st->maxx);
            // st->targets[st->ntargets].pos[1] = frand(st->maxy);
            st->nbugs--;
            st->ntargets++;

            for (i = 0; i < st->nbugs; i += st->ntargets)
                st->bugs[i].closest = &st->targets[st->ntargets - 1];
        }
    } else {
        /* turn target into bug */
        if (st->ntargets > 1 && st->nbugs < MAX_BUGS - 1)
        {
            /* pick a target */
            i = random() % st->ntargets;

            /* copy state into a new bug */
            memcpy((char *)&st->bugs[st->nbugs], (char *)&st->targets[i], sizeof(bug));
            st->ntargets--;

            /* pick a target for the new bug */
            st->bugs[st->nbugs].closest = &st->targets[random() % st->ntargets];

            for (j = 0; j < st->nbugs; j++)
            {
                if (st->bugs[j].closest == &st->targets[st->ntargets])
                {
                    st->bugs[j].closest = &st->targets[i];
                } else if (st->bugs[j].closest == &st->targets[i]) {
                    st->bugs[j].closest = &st->targets[random() % st->ntargets];
                }
            }
            st->nbugs++;

            /* copy the last ntarget into the one we just deleted */
            memcpy(&st->targets[i], (char *)&st->targets[st->ntargets], sizeof(bug));
        }
    }
}

static void mutateParam(float *param)
{
    *param *= 0.75+frand(0.5);
}

static void randomSmallChange(state *st)
{
    int whichCase = 0;

    whichCase = random()%10;
    
    switch(whichCase) {
    case 0:
        /* acceleration */
        mutateParam(&st->maxAcc);
        break;

    case 1:
        /* target acceleration */
        mutateParam(&st->targetAcc);
        break;

    case 2:
        /* velocity */
        mutateParam(&st->maxVel);
        break;

    case 3: 
        /* target velocity */
        mutateParam(&st->targetVel);
        break;

    case 4:
        /* noise */
        mutateParam(&st->noise);
        break;

    case 5:
        /* minVelMultiplier */
        mutateParam(&st->minVelMultiplier);
        break;
        
    case 6:
    case 7:
        /* target to bug */
        if (st->ntargets < 2) break;
        mutateBug(st, 1);
        break;

    case 8:
    case 9:
        /* bug to target */
        if (st->nbugs < 2) break;
        mutateBug(st, 0);
        if (st->nbugs < 2) break;
        mutateBug(st, 0);
        break;
    }

    if (st->minVelMultiplier < 0.3) st->minVelMultiplier = 0.3;
    else if (st->minVelMultiplier > 0.9) st->minVelMultiplier = 0.9;
    if (st->noise < 0.04) st->noise = 0.04;
    if (st->maxVel < 0.02) st->maxVel = 0.02;
    if (st->targetVel < 0.02) st->targetVel = 0.02;
    if (st->targetAcc > st->targetVel*0.7) st->targetAcc = st->targetVel*0.7;
    if (st->maxAcc > st->maxVel*0.7) st->maxAcc = st->maxVel*0.7;
    if (st->targetAcc > st->targetVel*0.7) st->targetAcc = st->targetVel*0.7;
    if (st->maxAcc < 0.01) st->maxAcc = 0.01;
    if (st->targetAcc < 0.005) st->targetAcc = 0.005;

    computeConstants(st);
}

static int redValueForHSBPhase(int phase, int value)
{
    switch (phase%6) {
        case 0: case 5: return 31;
        case 1: return 31-value;
        case 2: case 3: return 0;
        default:
        case 4: return value;
    }
}

static mxgui::Color mixColor(int r, int g, int b, int l)
{
    r = r * l / 31;
    g = g * l / 31;
    b = b * l / 31;
    return r + (g << 6) + (b << (5+6));
}

static void eraseLastBugsTrail(DrawingContext& dc, state *st)
{
    bug *b;
    int i;
    int temp;

    if (((st->head+1)%st->trailLen) == st->tail) {
        /* first, erase last segment of bugs if necessary */
        temp = (st->tail+1) % st->trailLen;
        b = st->bugs;
        for (i = 0; i < st->nbugs; i++, b++)
            dc.line(Point(b->hist[st->tail][0], b->hist[st->tail][1]), 
                    Point(b->hist[temp][0], b->hist[temp][1]), black);
        b = st->targets;
        for (i = 0; i < st->ntargets; i++, b++)
            dc.line(Point(b->hist[st->tail][0], b->hist[st->tail][1]),
                    Point(b->hist[temp][0], b->hist[temp][1]), black);
        st->tail = (st->tail+1)%st->trailLen;
    }
}

static void drawBugs(DrawingContext& dc, state *st)
{
    bug *b;
    int i, j;
    int temp;

    int huePhase = st->colorWheelTime / (COLOR_CHANGE_PERIOD/6);
    int value = st->colorWheelTime % (COLOR_CHANGE_PERIOD/6);
    value = value*31/(COLOR_CHANGE_PERIOD/6);
    int bugR, bugG, bugB, targetR, targetG, targetB;
    bugR = targetB = redValueForHSBPhase(huePhase, value);
    bugG = targetR = redValueForHSBPhase(huePhase+2, value);
    bugB = targetG = redValueForHSBPhase(huePhase+4, value);
    
    for (j = st->tail; j != st->head; j = temp) {
        int t = (j-st->head)%st->trailLen-1;
        if (t<0) t += st->trailLen;
        t = t*31/(st->trailLen-2);
        mxgui::Color bugSegmentColor = mixColor(bugR, bugG, bugB, t);
        mxgui::Color targetSegmentColor = mixColor(targetR, targetG, targetB, t);
        temp = (j+1)%st->trailLen;
        b = st->bugs;
        for (i = 0; i < st->nbugs; i++, b++)
            dc.line(Point(b->hist[j][0], b->hist[j][1]), 
                    Point(b->hist[temp][0], b->hist[temp][1]), bugSegmentColor);
        b = st->targets;
        for (i = 0; i < st->ntargets; i++, b++)
            dc.line(Point(b->hist[j][0], b->hist[j][1]),
                    Point(b->hist[temp][0], b->hist[temp][1]), targetSegmentColor);
    }
}

state *init()
{
    state *st = (state *) calloc (1, sizeof(*st));
    int i;

    st->dt = 0.3;
    st->targetVel = 0.03;
    st->targetAcc = 0.02;
    st->maxVel = 0.05;
    st->maxAcc = 0.03;
    st->noise = 0.01;
    st->minVelMultiplier = 0.5;

    st->nbugs = -1;
    st->ntargets = -1;
    st->trailLen = -1;

    st->changeProb = 0.08;
    st->colorWheelTime = -1;

    initGraphics(st);
    computeConstants(st);
    initBugs(st);

    if (st->changeProb > 0)
        for (i = random()%5+5; i >= 0; i--) randomSmallChange(st);

    return st;
}

static void free(state *st)
{
    free(st);
}

ENTRY()
{
    #ifndef MXGUI_COLOR_DEPTH_16_BIT
    #error "selected color depth not yet handled by this program"
    #endif

    state *st = init();
    #ifndef _MIOSIX
    for(;;)
    {
        updateState(st);
        {
            DrawingContext dc(DisplayManager::instance().getDisplay());
            eraseLastBugsTrail(dc, st);
            drawBugs(dc, st);
        }
        usleep(1000000/30);
    }
    #else
    long long frameStartTime=miosix::getTime();
    long long frameTime=0;
    const long long targetUpdateRate=1000000000/30; // 30 FPS
    int nUpdates=1;
    for(;;)
    {
        {
            DrawingContext dc(DisplayManager::instance().getDisplay());
            for(int i=0; i<nUpdates; i++)
            {
                updateState(st);
                eraseLastBugsTrail(dc, st);
            }
            drawBugs(dc, st);
        }
        long long frameEndTime=miosix::getTime();
        frameTime=frameEndTime-frameStartTime;
        nUpdates=static_cast<int>(frameTime/targetUpdateRate)+1;
        frameStartTime+=nUpdates*targetUpdateRate;
        miosix::Thread::nanoSleepUntil(frameStartTime);
    }
    #endif
    return 0;
}
