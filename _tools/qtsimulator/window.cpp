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
#include <iostream>
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
#if defined(MXGUI_PIXEL_FORMAT_GRAY1)
        QImage::Format_MonoLSB),
#elif defined(MXGUI_PIXEL_FORMAT_RGB565)
        QImage::Format_RGB16),
#else
        QImage::Format_ARGB32),
#endif
        w(this), layout(&w), buttonA("<"), buttonB(">"),
        sender(std::make_shared<UpdateSignalSender>())
{
#ifdef MXGUI_PIXEL_FORMAT_GRAY1
    image.setColorCount(2);
    image.setColor(0,qRgb(0,0,0));
    image.setColor(1,qRgb(255,255,255));
#elif !defined(MXGUI_PIXEL_FORMAT_RGB565)
    std::cout<<"Warning: using generic pixel format fallback in qtsimulator"<<std::endl;
#endif
    this->setFixedSize(FrameBuffer::width,FrameBuffer::height+50);
    w.setFixedSize(FrameBuffer::width,50);
    w.move(QPoint(0,FrameBuffer::height));
    layout.addWidget(&buttonA);
    layout.addWidget(&buttonB);
    connect(&buttonA,SIGNAL(pressed()),this,SLOT(aPressed()));
    connect(&buttonA,SIGNAL(released()),this,SLOT(aReleased()));
    connect(&buttonB,SIGNAL(pressed()),this,SLOT(bPressed()));
    connect(&buttonB,SIGNAL(released()),this,SLOT(bReleased()));
    //Note: the Qt::BlockingQueuedConnection is important to ensure that the
    //background thread does not write the framebuffer while the main thread
    //reads it.
    connect(sender.get(),SIGNAL(sendUpdate()),this,SLOT(updateFrameBuffer()),
            Qt::BlockingQueuedConnection);
    this->setWindowTitle(tr("Mxgui simulator"));
    this->show();
    QTBackend& qb=QTBackend::instance();
    updateFrameBuffer();
    this->update();
    qb.start(sender);
}

void Window::updateFrameBuffer()
{
    FrameBuffer& buffer=QTBackend::instance().getFrameBuffer();
#if defined(MXGUI_PIXEL_FORMAT_GRAY1) || defined(MXGUI_PIXEL_FORMAT_RGB565)
    std::memcpy(image.bits(),buffer.getData(),image.sizeInBytes());
#else
    static bool warned = false;
    if(!warned)
    {
        qDebug("Warning: using slow fallback for pixel format conversion in simulator.");
        warned = true;
    }
    for(int y=0;y<FrameBuffer::height;y++)
    {
        for(int x=0;x<FrameBuffer::width;x++)
        {
            Color c=buffer.getPixel(x,y);
            image.setPixel(x,y,qRgb(c.getR(),c.getG(),c.getB()));
        }
    }
#endif
    this->update();
}

void Window::aPressed()
{
    addEvent(Event(EventType::ButtonA,EventDirection::DOWN));
}

void Window::aReleased()
{
    addEvent(Event(EventType::ButtonA,EventDirection::UP));
}

void Window::bPressed()
{
    addEvent(Event(EventType::ButtonB,EventDirection::DOWN));
}

void Window::bReleased()
{
    addEvent(Event(EventType::ButtonB,EventDirection::UP));
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
    addEvent(Event(EventType::TouchMove,Point(event->x(),event->y()),
        EventDirection::DOWN));
}

void Window::mousePressEvent(QMouseEvent *event)
{
    if(event->x()<0 || event->x()>=FrameBuffer::width) return;
    if(event->y()<0 || event->y()>=FrameBuffer::height) return;
    addEvent(Event(EventType::TouchDown,Point(event->x(),event->y()),
        EventDirection::DOWN));
}

void Window::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->x()<0 || event->x()>=FrameBuffer::width) return;
    if(event->y()<0 || event->y()>=FrameBuffer::height) return;
    addEvent(Event(EventType::TouchUp,Point(event->x(),event->y()),
        EventDirection::UP));
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
    char k=s[0].toLatin1();
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
    char k=s[0].toLatin1();
    addEvent(Event(EventType::KeyUp,k));
}
