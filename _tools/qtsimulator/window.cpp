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

#include "window.h"
#include "qtbackend.h"
#include "drivers/event_qt.h"
#include <cstring>
#include <QPainter>

using namespace mxgui;

//
// class UpdateSignalSender
//

void UpdateSignalSender::update()
{
    emit sendUpdate();
}

//
// class Window
//

Window::Window(QWidget *parent): QWidget(parent),
        image(QSize(FrameBuffer::width,FrameBuffer::height),
#ifdef MXGUI_COLOR_DEPTH_16_BIT
        QImage::Format_RGB16),
#elif defined(MXGUI_COLOR_DEPTH_1_BIT_LINEAR)
        QImage::Format_MonoLSB),
#endif
        w(this), layout(&w), buttonA("<"), buttonB(">"),
        sender(new UpdateSignalSender)
{
    this->setFixedSize(FrameBuffer::width,FrameBuffer::height+50);
    w.setFixedSize(FrameBuffer::width,50);
    w.move(QPoint(0,FrameBuffer::height));
    layout.addWidget(&buttonA);
    layout.addWidget(&buttonB);
    connect(&buttonA,SIGNAL(clicked()),this,SLOT(aClicked()));
    connect(&buttonB,SIGNAL(clicked()),this,SLOT(bClicked()));
    //Note: the Qt::BlockingQueuedConnection is important to ensure that the
    //background thread does not write the framebuffer while the main thread
    //reads it.
    connect(sender.get(),SIGNAL(sendUpdate()),this,SLOT(updateFrameBuffer()),
            Qt::BlockingQueuedConnection);
    this->setWindowTitle(tr("Mxgui simulator"));
    this->show();
    QTBackend& qb=QTBackend::instance();
    std::memcpy(image.bits(),qb.getFrameBuffer().getData(),image.byteCount());
    this->update();
    qb.start(sender);
}

void Window::updateFrameBuffer()
{
    FrameBuffer& buffer=QTBackend::instance().getFrameBuffer();
    std::memcpy(image.bits(),buffer.getData(),image.byteCount());
    this->update();
}

void Window::aClicked()
{
    addEvent(Event(EventType::ButtonA));
}

void Window::bClicked()
{
    addEvent(Event(EventType::ButtonB));
}

void Window::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawImage(QPoint(0,0),image);
}

void Window::mouseMoveEvent(QMouseEvent *event)
{
    if(event->x()<0 || event->x()>=FrameBuffer::width) return;
    if(event->y()<0 || event->y()>=FrameBuffer::height) return;
    addEvent(Event(EventType::TouchMove,Point(event->x(),event->y())));
}

void Window::mousePressEvent(QMouseEvent *event)
{
    if(event->x()<0 || event->x()>=FrameBuffer::width) return;
    if(event->y()<0 || event->y()>=FrameBuffer::height) return;
    addEvent(Event(EventType::TouchDown,Point(event->x(),event->y())));
}

void Window::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->x()<0 || event->x()>=FrameBuffer::width) return;
    if(event->y()<0 || event->y()>=FrameBuffer::height) return;
    addEvent(Event(EventType::TouchUp,Point(event->x(),event->y())));
}

void Window::mouseDoubleClickEvent(QMouseEvent *event)
{
    //Event ignored
}

void Window::keyPressEvent(QKeyEvent *event)
{
    if(event->key()>0xff)
    {
        //Not a letter, number or anything like that, just forward it
        QWidget::keyPressEvent(event);
        return;
    }
    QString s=event->text();
    if(s.size()==0) return;
    char k=s[0].toAscii();
    addEvent(Event(EventType::KeyDown,k));
}

void Window::keyReleaseEvent(QKeyEvent *event)
{
    if(event->key()>0xff)
    {
        //Not a letter, number or anything like that, just forward it
        QWidget::keyReleaseEvent(event);
        return;
    }
    QString s=event->text();
    if(s.size()==0) return;
    char k=s[0].toAscii();
    addEvent(Event(EventType::KeyUp,k));
}
