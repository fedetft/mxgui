/***************************************************************************
 *   Copyright (C) 2010 by Terraneo Federico                               *
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
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>   *
 ***************************************************************************/

#ifndef QTBACKEND_H
#define QTBACKEND_H

#include <list>
#include <cstring>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>

//Forward decl
class UpdateSignalSender;
class FrameBufferLock;

/**
 * A framebuffer object
 * \param N framebuffer width
 * \param M framebuffer height
 */
template<unsigned int N, unsigned int M>
class basic_framebuffer
{
public:
    /// Height of framebuffer
    static const unsigned int width=N;

    /// Width of framebuffer
    static const unsigned int height=M;
    
    /**
     * Initializes buffer to all black
     */
    basic_framebuffer() { clear(); }

    /**
     * Clear the framebuffer to all black.
     */
    void clear() { std::memset(data,0,sizeof(data)); }

    /**
     * Get a pixel
     * \param x
     * \param y
     * \return the pixel
     */
    unsigned short getPixel(int x, int y) const { return data[y][x]; }

    /**
     * Set a pixel
     * \param x
     * \param y
     * \param pixel pixel to set
     */
    void setPixel(int x, int y, unsigned short pixel) { data[y][x]=pixel; }

    /**
     * Get a pointer to the framebuffer data
     * \return the pointer to the framebuffer
     */
    unsigned short *getData() { return &data[0][0]; }

private:
    ///Pixel data, stored as [M][N] because matches QImage's representation
    unsigned short data[M][N];
};

///Framebuffer instantiation
typedef basic_framebuffer<240,320> FrameBuffer;

///Possible button events
enum ButtonEvents
{
    noButton=-1,
    buttonAId=0,
    buttonBId=1,
    touchDownId=2,
    touchUpId=3
};

/**
 * An event sent from the GUI
 */
class Event
{
public:
    /**
     * Constructor for button events
     * \param buttonId which button generated the event
     */
    explicit Event(ButtonEvents buttonId): buttonId(buttonId), x(-1), y(-1) {}

    /**
     * Constructor for touchscreen events (simulated using the mouse)
     * \param x
     * \param y
     */
    Event(int x, int y): buttonId(noButton), x(x), y(y) {}

    /**
     * Constructor for mouse touch down and touch up events that also carry
     * a position information
     */
    Event(ButtonEvents buttonId, int x, int y):
            buttonId(buttonId), x(x), y(y) {}

    const ButtonEvents buttonId;//< Button id or none for touchscreen events
    const int x;
    const int y;
};

/**
 * Class that interfaces QT GUI (running from main thread) and mxgui (running
 * in a background thread)
 */
class QTBackend
{
public:
    /**
     * Singleton
     * \return the only instance
     */
    static QTBackend& instance();

    /**
     * Start application.
     * This starts the background thread.
     * \param sender the object to call to update the screen
     */
    void start(boost::shared_ptr<UpdateSignalSender> sender);

    /**
     * Add an event to the event queue.
     * \param e event to add
     */
    void addEvent(Event e);

    /**
     * Blocking call to get an event from event queue
     * \return the oldes event from the queue
     */
    Event getEvent();

    /**
     * Allows access to the framebuffer object
     * \return the framebuffer
     */
    FrameBuffer& getFrameBuffer() { return fb; }

    /**
     * \return the sender object with an update() member function to refresh
     * the GUI
     */
    boost::shared_ptr<UpdateSignalSender> getSender() const;

private:
    QTBackend(const QTBackend& );
    QTBackend& operator= (const QTBackend& );

    /**
     * Constructor
     */
    QTBackend() {}

    FrameBuffer fb; ///< Framebuffer object

    boost::mutex eqMutex; ///< Mutex to guard the event queue
    boost::condition_variable eqCond; ///< Condvar for blocking getEvent
    std::list<Event> eventQueue; ///< Queue of events from the GUI

    boost::shared_ptr<UpdateSignalSender> sender; ///< Object to update GUI

    friend class FrameBufferLock; //Needs access to fb and fbMutex
};

#endif //QTBACKEND_H
