
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
        static constexpr RGB565Color decrement = RGB565Color::fromRaw(0x0841);
        color = color - decrement; 
    }

    /**
     * \return particle color
     */
    Color getColor() const { return Color::fromRGB565(color); }

    /**
     * \param c new color of the particle
     */
    void setColor(RGB565Color c) { color=c; }

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
    RGB565Color color;
};

/**
 * Called @ every frame to animate particles
 * \param particles array of particles, of size numParticles
 * \return true if particles have moved and screen redraw is required
 */
static bool particleAnimation(Particle *particles)
{
    static constexpr RGB565Color rainbow[]=
    {
        RGB565Color::fromRaw(63489),RGB565Color::fromRaw(63495),RGB565Color::fromRaw(63501),
        RGB565Color::fromRaw(63507),RGB565Color::fromRaw(63513),RGB565Color::fromRaw(63519),
        RGB565Color::fromRaw(53279),RGB565Color::fromRaw(40991),RGB565Color::fromRaw(28703),
        RGB565Color::fromRaw(16415),RGB565Color::fromRaw(4127),RGB565Color::fromRaw(287),
        RGB565Color::fromRaw(671),RGB565Color::fromRaw(1055),RGB565Color::fromRaw(1439),
        RGB565Color::fromRaw(1823),RGB565Color::fromRaw(2045),RGB565Color::fromRaw(2039),
        RGB565Color::fromRaw(2033),RGB565Color::fromRaw(2027),RGB565Color::fromRaw(2021),
        RGB565Color::fromRaw(2016),RGB565Color::fromRaw(14304),RGB565Color::fromRaw(26592),
        RGB565Color::fromRaw(38880),RGB565Color::fromRaw(51168),RGB565Color::fromRaw(63456),
        RGB565Color::fromRaw(65216),RGB565Color::fromRaw(64832),RGB565Color::fromRaw(64448),
        RGB565Color::fromRaw(64064),RGB565Color::fromRaw(65535)
    };

    static const int cosTab[]={0, 1, 1, 1, 0,-1,-1,-1};
    static const int sinTab[]={1, 1, 0,-1,-1,-1, 0, 1};

    Display& disp=DisplayManager::instance().getDisplay();
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
    DrawingContext dc(DisplayManager::instance().getDisplay());
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
    DrawingContext dc(DisplayManager::instance().getDisplay());
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
