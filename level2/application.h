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

#ifndef APPLICATION_H
#define	APPLICATION_H

#include <vector>
#include <list>
#include <tr1/functional>
#include <tr1/memory>
#include <config/mxgui_settings.h>
#include "display.h"
#include "input.h"
#include "drawing_context_proxy.h"

#ifdef MXGUI_LEVEL_2

namespace mxgui {

//forward decls
class Window;

/**
 * \ingroup pub_iface_2
 * This class contains all the configuration options for a window
 */
class WindowPreferences
{
public:
    /**
     * Constructor
     * \param foreground foreground color
     * \param background background color
     * \param font default font
     */
    WindowPreferences(Color foreground, Color background, Font font)
            : foreground(foreground), background(background), font(font) {}

    Color foreground; ///< Foreground color
    Color background; ///< Background color
    Font font;        ///< Default font
};

/**
 * \ingroup pub_iface_2
 * Any object that can be drawn on screen has to extend Drawable
 */
class Drawable
{
public:
    /**
     * Constructor
     * \param w window to which this object belongs
     * \param da area on screen occupied by this object
     */
    Drawable(Window *w, DrawArea da);
    
    /**
     * Constructor
     * \param w window to which this object belongs
     * \param p upper left point of the text label
     * \param width width of the text label
     * \param height height of the text label
     */
    Drawable(Window *w, Point p, short width, short height);
    
    /**
     * Signal that this object needs to be redrawn
     */
    void enqueueForRedraw();
    
    /**
     * \internal
     * Called after onDraw() by the parent window when the Drawable is being
     * redrawn, do not call this directly.
     */
    void redrawDone() { needRedraw=false; }
    
    /**
     * \return true if this Drawable needs to be redrawn 
     */
    bool needsRedraw() const { return needRedraw; }
    
    /**
     * \internal
     * Override this member function to draw the object. Called by the parent
     * Window, do not call this directly.
     * \param dc drawing context used to draw the object
     */
    virtual void onDraw(DrawingContextProxy& dc)=0;
    
    /**
     * \internal
     * Override this member function to handle user input events. Called by the
     * parent Window, do not call this directly.
     * \param e event
     */
    virtual void onEvent(Event e);
    
    /**
     * Destructor
     */
    virtual ~Drawable();
    
protected:
    /**
     * \return the window
     */
    Window *getWindow() { return w; }
    
    /**
     * \return the draw area of the object
     */
    DrawArea getDrawArea() const { return da; }
    
private:
    Window *w;       ///< Window to which this drawable belongs
    DrawArea da;     ///< Area on screen occupied by this object
    bool needRedraw; ///< True if this object needs to be redrawn
};

/**
 * \ingroup pub_iface_2
 * Windows are the central point of mxgui level 2, this part of the library
 * allows multiple applications to run concurrently and share the same display.
 * Applications are classes that derive from Window.
 */
class Window
{
public:
    /**
     * Constructor
     */
    Window();
    
    /**
     * \internal
     * Called by the Drawable constructor to register itself to a window.
     * Do not call from user code
     * \param d drawable to register
     */
    void addDrawable(Drawable *d);
    
    /**
     * \internal
     * Called by the Drawable destructor to register itself to a window.
     * Do not call from user code
     * \param d drawable to register
     */
    void removeDrawable(Drawable *d);
    
    /**
     * \internal
     * Called by a drawable to signal that it needs to be redrawn.
     * Do not call from user code
     * \param d drawable that needs to be redrawn
     */
    void needsPartialRedraw(Drawable *d);
    
    /**
     * \internal
     * Called by the window manager to send user events to this window.
     * Do not call from user code
     * \param e event to post
     */
    void postEvent(Event e);
    
    /**
     * \internal
     * Handle the window logic
     */
    void eventLoop();
    
    /**
     * \return the window preferences
     */
    const WindowPreferences& getPreferences() const { return prefs; }

    /**
     * Destructor
     */
    virtual ~Window();

private:
    /**
     * Post an event to the event loop. Needs the mutex locked
     * \param e event to post
     */
    void postEventImpl(Event e);
    
    /**
     * \return an event to run the event loop. Blocking 
     */
    Event getEvent();
    
    std::list<Drawable *> drawables; ///< List of drawables on the window
    std::list<Event> events;         ///< List of unprocessed events
    pthread_mutex_t mutex;           ///< To serialize concurrent access
    pthread_cond_t cond;             ///< Condition variable for the evnt loop
    WindowPreferences prefs;         ///< Window preferences
    bool redrawNeeded;               ///< True if a redraw is needed
};

/**
 * \ingroup pub_iface_2
 * The window manager multiplexes display and events between windows
 */
class WindowManager
{
public:
    /**
     * \return an instance of the window manager (singleton)
     */
    static WindowManager& instance();

    /**
     * \param window Window to start.
     * \param modal if true the application will be started in the thread that
     * calls start, and therefore the call to start won't return till the
     * application will terminate.
     * \return true if success, false on failure
     */
    bool start(std::tr1::shared_ptr<Window> window, bool modal=false);
    
private:
    WindowManager();
    
    pthread_mutex_t mutex;
    std::vector<std::tr1::shared_ptr<Window> > windows;
};

} //namespace mxgui

#endif //MXGUI_LEVEL_2

#endif //APPLICATION_H
