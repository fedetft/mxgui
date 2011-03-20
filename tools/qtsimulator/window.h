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

#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>
#include <QImage>
#include <QPushButton>
#include <QBoxLayout>
#include <QMouseEvent>
#include <boost/shared_ptr.hpp>

/**
 * This class just provides the update() member function to refresh the GUI.
 * By using the signal/slot mechanism, it can be safely called from background
 * threads
 */
class UpdateSignalSender : public QObject
{
Q_OBJECT
public:
    /**
     * Constructor.
     */
    UpdateSignalSender() {}

    /**
     * Send a signal to the GUI requesting for a refresh. This causes the main
     * thread to read the framebuffer stored in the QTBackend class and update
     * the screen with its content. This call is blocking, it waits for the
     * main thread to copy the framebuffer content in its own private copy.
     * The fact that this call blocks the thread that calls it waiting for the
     * main thread to do something implies that it *cannot* be called from the
     * main thread itself or deadlock will occur.
     */
    void update();

signals:
    /**
     * Signal called by update.
     */
    void sendUpdate();

private:
    UpdateSignalSender(const UpdateSignalSender& );
    UpdateSignalSender& operator= (const UpdateSignalSender& );
};

/**
 * Main window
 */
class Window : public QWidget
{
Q_OBJECT
public:
    /**
     * Constructor.
     * \param parent parent widget
     */
    explicit Window(QWidget *parent=0);

private slots:
    /**
     * Slot to receive the update signal
     */
    void updateFrameBuffer();

    /**
     * First button clicked
     */
    void aClicked();

    /**
     * Second button clicked
     */
    void bClicked();

private:
    /**
     * Overridden to allow drawing the framebuffer on screen
     * \param event paint event
     */
    void paintEvent(QPaintEvent *event);

    /**
     * Overridden to emulate touchscreen
     * \param event mouse event
     */
    void mouseMoveEvent(QMouseEvent *event);

    /**
     * Overridden to emulate touchscreen
     * \param event mouse event
     */
    void mousePressEvent(QMouseEvent *event);

    /**
     * Overridden to emulate touchscreen
     * \param event mouse event
     */
    void mouseReleaseEvent(QMouseEvent *event);

    /**
     * Overridden to emulate touchscreen
     * \param event mouse event
     */
    void mouseDoubleClickEvent(QMouseEvent *event);

    QImage image; ///< Image used to display framebuffer data
    QWidget w; ///< Widget used as a container for the layout below
    QHBoxLayout layout; ///< Horizontal layout for buttons
    QPushButton buttonA; ///< First button
    QPushButton buttonB; ///< Second button
    boost::shared_ptr<UpdateSignalSender> sender; ///< Object that sends updates
};

#endif //WINDOW_H
