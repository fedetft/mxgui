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
    
    boost::shared_ptr<UpdateSignalSender> sender; ///< Object to update GUI

    friend class FrameBufferLock; //Needs access to fb and fbMutex
};

#endif //QTBACKEND_H
