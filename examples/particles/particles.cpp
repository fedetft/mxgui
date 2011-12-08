
#include "mxgui/entry.h"
#include "mxgui/display.h"
#include "mxgui/misc_inst.h"
#include "mxgui/level2/input.h"
#include "fps_counter.h"
#include <algorithm>
#include <cstdio>
#include <cstdlib>

using namespace std;
using namespace mxgui;

static const int numParticles=512;
static const int desiredFps=60;

/**
 * Particle class
 */
class Particle
{
public:
    /**
     * Reduce brightness of the particle color.
     * Repeated calls will progressively dim the particle to black.
     */
    void fadeColor()
    {
        if(color>0x800) color-=0x800;
        if((color & 0x7c0)>0) color-=0x40;
        if((color & 0x01f)>0) color--;
    }

    /**
     * \return particle color
     */
    Color getColor() const { return color; }

    /**
     * \param c new color of the particle
     */
    void setColor(Color c) { color=c; }

    /**
     * \return particle position
     */
    Point getPosition() const { return position; }

    /**
     * \param p new particle position
     */
    void setPosition(Point p) { position=p; }

    /**
     * \return a reference to xSpeed allowing read/write access
     */
    signed char& xSpeed() { return xSpeed_; }

    /**
     * \return a reference to ySpeed allowing read/write access
     */
    signed char& ySpeed() { return ySpeed_; }

private:
    //Object size 8byte
    Point position;
    signed char xSpeed_, ySpeed_;
    unsigned short color;
};

/**
 * Called @ every frame to animate particles
 * \param particles array of particles, of size numParticles
 * \return true if particles have moved and screen redraw is required
 */
static bool particleAnimation(Particle *particles)
{
    static const unsigned short rainbow[]=
    {
        63489,63495,63501,63507,63513,63519,53279,40991,28703,16415,4127,287,
        671,1055,1439,1823,2045,2039,2033,2027,2021,2016,14304,26592,38880,
        51168,63456,65216,64832,64448,64064,65535
    };

    static const int cosTab[]={0, 1, 1, 1, 0,-1,-1,-1};
    static const int sinTab[]={1, 1, 0,-1,-1,-1, 0, 1};

    Display& disp=Display::instance();
    static int state=0;
    if(state==0)
    {
        Point p(disp.getWidth()/2,disp.getHeight()/2);
        for(int i=0;i<numParticles;i++)
        {
            particles[i].setPosition(p);
            particles[i].setColor(rainbow[rand() % 32]);
            int speedMagnitude=rand()%10==0 ? 0 : 1;
            particles[i].xSpeed()=speedMagnitude*cosTab[i & 7];
            particles[i].ySpeed()=speedMagnitude*sinTab[i & 7];
        }
    } else {
        for(int i=0;i<numParticles;i++)
        {
            Point p=particles[i].getPosition();
            p=Point(p.x()+particles[i].xSpeed()+rand()%7-3,
                    p.y()+particles[i].ySpeed()+rand()%7-3);
            particles[i].setPosition(p);
        }
        if(state>60) for(int i=0;i<numParticles;i++) particles[i].fadeColor();
    }
    if(++state==120) state=0;
    return state>92 ? false : true; //After 60+32 all particles are black
}

/**
 * Called to redraw the particles if they have moved
 * \param sorted array of particles of size numParticles, sorted by ascending y
 * \param oldPosition array of the old particle positions, sorted by ascending
 * y coordinate
 */
static void drawNextFrame(Particle **sorted, Point *oldPosition)
{
    DrawingContext dc(Display::instance());
    const Point last(dc.getWidth(),dc.getHeight());
    dc.beginPixel();
    int a=0, b=0;
    while(a<numParticles && b<numParticles)
    {
        if(oldPosition[a].y()<=sorted[b]->getPosition().y())
        {
            Point p=oldPosition[a];
            if(within(p,Point(0,0),last)) dc.setPixel(p,black);
            a++;
        } else {
            Point p=sorted[b]->getPosition();
            if(within(p,Point(0,0),last)) dc.setPixel(p,sorted[b]->getColor());
            b++;
        }
    }
    while(a<numParticles)
    {
        Point p=oldPosition[a];
        if(within(p,Point(0,0),last)) dc.setPixel(p,black);
        a++;
    }
    while(b<numParticles)
    {
        Point p=sorted[b]->getPosition();
        if(within(p,Point(0,0),last)) dc.setPixel(p,sorted[b]->getColor());
        b++;
    }
}

#ifndef _MIOSIX
#define siprintf sprintf //The low code size integer only sprintf
#endif //_MIOSIX

/**
 * Print fps and cpu usage on screen
 */
static void printFps(int fps, int cpu)
{
    char line[32];
    siprintf(line,"%d fps  %d%% cpu          ",fps,cpu);
    DrawingContext dc(Display::instance());
    dc.write(Point(0,0),line);
}

/**
 * Function used to sort particles by ascending y coordinate
 */
static inline bool particleOrder(const Particle *a, const Particle *b)
{
    return a->getPosition().y() < b->getPosition().y();
}

ENTRY()
{
    Particle *particles=new Particle[numParticles];
    Particle **sorted=new Particle*[numParticles];
    Point *oldPos=new Point[numParticles];
    FpsCounter fps;
    fps.setFpsCap(60);
    for(int i=0;i<numParticles;i++) sorted[i]=&particles[i]; //Initialize sorted
    do { 
        if(particleAnimation(particles))
        {
            sort(sorted,sorted+numParticles,particleOrder);
            drawNextFrame(sorted,oldPos);
            for(int i=0;i<numParticles;i++) oldPos[i]=sorted[i]->getPosition();
        }
        printFps(fps.getFps(),fps.getCpuUsed());
        fps.sleepBetweenFrames();
    } while(InputHandler::instance().popEvent().getEvent()!=EventType::ButtonB);
    delete[] particles;
    delete[] sorted;
    delete[] oldPos;
    return 0;
}
