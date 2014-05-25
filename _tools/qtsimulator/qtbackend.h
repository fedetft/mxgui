/***************************************************************************
 *   Copyright (C) 2010, 2011 by Terraneo Federico                         *
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

#include <cstring>
#include <boost/shared_ptr.hpp>
#include "config/mxgui_settings.h"
#include "color.h"

//Forward decl
class UpdateSignalSender;

/**
 * A framebuffer object
 * \param T the color type, that is, the type of one pixel
 * \param N framebuffer width
 * \param M framebuffer height
 * \param F framebuffer fill, used for clear()
 */
template<typename T, unsigned int N, unsigned int M, unsigned int F>
class basic_framebuffer
{
public:
    /// Height of framebuffer
    static const unsigned int width=N;

    /// Width of framebuffer
    static const unsigned int height=M;
    
    /**
     * Initializes buffer to the fill color
     */
    basic_framebuffer() { clear(); }

    /**
     * Clear the framebuffer to the fill color
     */
    void clear()
    {
        for(int i=0;i<M;i++) for(int j=0;j<N;j++) data[i][j]=F;
    }

    /**
     * Get a pixel
     * \param x
     * \param y
     * \return the pixel
     */
    T getPixel(int x, int y) const { return data[y][x]; }

    /**
     * Set a pixel
     * \param x
     * \param y
     * \param pixel pixel to set
     */
    void setPixel(int x, int y, T pixel) { data[y][x]=pixel; }

    /**
     * Get a pointer to the framebuffer data
     * \return the pointer to the framebuffer
     */
    void *getData() { return &data[0][0]; }

private:
    ///Pixel data, stored as [M][N] because matches QImage's representation
    T data[M][N];
};

/**
 * Partial specialization of basic_framebuffer for
 * 1 bit per pixel linear framebuffers
 * \param N framebuffer width
 * \param M framebuffer height
 * \param F framebuffer fill, used for clear()
 */
template<unsigned int N, unsigned int M, unsigned int F>
class basic_framebuffer<mxgui::Color1bitlinear, N, M, F>
{
public:
    /// Height of framebuffer
    static const unsigned int width=N;

    /// Width of framebuffer
    static const unsigned int height=M;
    
    /**
     * Initializes buffer to the fill color
     */
    basic_framebuffer() { clear(); }

    /**
     * Clear the framebuffer to the fill color
     */
    void clear()
    {
        unsigned char fill=F ? 0xff : 0x00;
        std::memset(data,fill,sizeof(data));
    }

    /**
     * Get a pixel
     * \param x
     * \param y
     * \return the pixel
     */
    mxgui::Color1bitlinear getPixel(int x, int y) const
    {
        return data[y][x/8] & (1<<(x & 7)) ? 1 : 0;
    }

    /**
     * Set a pixel
     * \param x
     * \param y
     * \param pixel pixel to set
     */
    void setPixel(int x, int y, mxgui::Color1bitlinear pixel)
    {
        if(pixel) data[y][x/8] |= (1<<(x & 7));
        else data[y][x/8] &=~ (1<<(x & 7));
    }

    /**
     * Get a pointer to the framebuffer data
     * \return the pointer to the framebuffer
     */
    void *getData() { return &data[0][0]; }

private:
    ///Pixel data, stored as [M][N] because matches QImage's representation
    unsigned char data[M][(N+7)/8];
};

///Framebuffer instantiation
typedef basic_framebuffer<mxgui::Color,
    mxgui::SIMULATOR_DISP_WIDTH,
    mxgui::SIMULATOR_DISP_HEIGHT,
    mxgui::SIMULATOR_BGCOLOR> FrameBuffer;

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
    boost::shared_ptr<UpdateSignalSender> getSender() const { return sender; }

private:
    QTBackend(const QTBackend& );
    QTBackend& operator= (const QTBackend& );

    /**
     * Constructor
     */
    QTBackend(): started(false) {}

    FrameBuffer fb; ///< Framebuffer object
    bool started; ///< True if the background thread has already been started
    boost::shared_ptr<UpdateSignalSender> sender; ///< Object to update GUI
};

#endif //QTBACKEND_H
