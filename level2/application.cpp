/***************************************************************************
 *   Copyright (C) 2011, 2012, 2013, 2014 by Terraneo Federico             *
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

#include "application.h"
#include "pthread_lock.h"
#include "misc_inst.h"

#ifdef MXGUI_LEVEL_2

using namespace std;
using namespace std::tr1;

namespace mxgui {

//
// class Drawable
//

Drawable::Drawable(Window* w, DrawArea da) : w(w), da(da), needRedraw(false)
{
    w->addDrawable(this);
}

Drawable::Drawable(Window *w, Point p, short width, short height)
    : w(w), da(make_pair(p,Point(p.x()+width,p.y()+height))), needRedraw(false)
{
    w->addDrawable(this);
}

void Drawable::enqueueForRedraw()
{
    needRedraw=true;
    w->needsPartialRedraw(this);
}

void Drawable::onEvent(Event e) {}

Drawable::~Drawable()
{
    w->removeDrawable(this);
}

//
// class Window
//

Window::Window() : prefs(white,black,
#ifdef MXGUI_FONT_DROID11
    droid11),
#elif defined(MXGUI_FONT_TAHOMA)
    tahoma),
#elif defined(MXGUI_FONT_MISCFIXED)
    miscFixed),
#else
#error "Need a font"
#endif
    redrawNeeded(false)
{
    pthread_mutex_init(&mutex,NULL);
    pthread_cond_init(&cond,NULL);
}

void Window::addDrawable(Drawable* d)
{
    PthreadLock lock(mutex);
    drawables.push_back(d);
}

void Window::removeDrawable(Drawable* d)
{
    PthreadLock lock(mutex);
    drawables.remove(d); //O(n) removal, space-speed tradeoff
}

void Window::needsPartialRedraw(Drawable* d)
{
    PthreadLock lock(mutex);
    //This function needs to be callable also by a thread different from the one
    //that runs the event loop, so we need to post an event to wake the event
    //loop thread
    if(redrawNeeded==false) postEventImpl(Event(EventType::WindowPartialRedraw));
    
    redrawNeeded=true;
}

void Window::postEvent(Event e)
{
    PthreadLock lock(mutex);
    postEventImpl(e);
}

void Window::eventLoop()
{
    for(;;)
    {
        Event e=getEvent();
        
        if(e.getEvent()==EventType::WindowQuit) return;
        if(e.getEvent()==EventType::WindowPartialRedraw)
        {
            FullScreenDrawingContextProxy dc(Display::instance());//FIXME: get it fron the window manager
            dc.setTextColor(make_pair(prefs.foreground,prefs.background));
            for(list<Drawable*>::iterator it=drawables.begin();
                it!=drawables.end();++it)
            {
                if((*it)->needsRedraw()==false) continue;
                (*it)->onDraw(dc);
                (*it)->redrawDone();
            }
            //Filter out this event
            continue;
        }
        if(e.getEvent()==EventType::WindowForeground)
        {
            FullScreenDrawingContextProxy dc(Display::instance());//FIXME: get it fron the window manager
            dc.setTextColor(make_pair(prefs.foreground,prefs.background));
            dc.clear(prefs.background);
            for(list<Drawable*>::iterator it=drawables.begin();
                it!=drawables.end();++it)
            {
                (*it)->onDraw(dc);
                (*it)->redrawDone();
            }
            //Do not filter this out
        }
        //Forward event. For now we do not yet have a way for a Drawable to
        //register only for a certain class of events, such as touch events only
        //in their draw area, but we simply forward each event to all Drawables,
        //this is a space-speed tradeoff
        for(list<Drawable*>::iterator it=drawables.begin();
            it!=drawables.end();++it)
                (*it)->onEvent(e);
    }
}

void Window::postEventImpl(Event e)
{
    events.push_back(e);
    pthread_cond_signal(&cond);
}

Event Window::getEvent()
{
    PthreadLock lock(mutex);
    for(;;)
    {
        if(events.empty()==false)
        {
            Event result=events.front();
            events.pop_front();
            //Filter out WindowPartialRedraw that is done through redrawNeeded
            if(result.getEvent()==EventType::WindowPartialRedraw) continue;
            return result;
        } else {
            //No event, check if we need to redraw. The idea behind this is:
            //in case more than one event show up, we first process them all
            //and redraw after all events if redrawNeeded is true
            if(redrawNeeded)
            {
                redrawNeeded=false;
                return Event(EventType::WindowPartialRedraw);
            } else {
                //No event and no redraw needed, wait for an event
                pthread_cond_wait(&cond,&mutex);
            }
        }
    }
}

Window::~Window()
{
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
}

//
// class WindowManager
//

WindowManager& WindowManager::instance()
{
    static WindowManager singleton;
    return singleton;
}

bool WindowManager::start(shared_ptr<Window> window, bool modal)
{
    if(window==0) return false;
    PthreadLock lock(mutex);
    if(windows.size()>=level2MaxNumApps) return false;
}

WindowManager::WindowManager()
{
    pthread_mutex_init(&mutex,NULL);
}

} //namespace mxgui

#endif //MXGUI_LEVEL_2
