
#include "display.h"
#include "drivers/display_stm3210e-eval.h"
#include "drivers/display_mp3v2.h"
#include "drivers/display_qt.h"

namespace mxgui {

//
// class PthreadLock
//
namespace {
class PthreadLock
{
public:
    PthreadLock(pthread_mutex_t& m) : mutex(m) { pthread_mutex_lock(&mutex); }
    ~PthreadLock() { pthread_mutex_unlock(&mutex); }
private:
    pthread_mutex_t& mutex;
};
} //anon namespace

//
// class Display
//

Display& Display::instance(const char *id)
{
    static DisplayImpl implementation;
    static Display singleton(&implementation);
    return singleton;
}

void Display::turnOn()
{
    PthreadLock lock(dispMutex);
    pImpl->turnOn();
}

void Display::turnOff()
{
    PthreadLock lock(dispMutex);
    pImpl->turnOff();
}

short int Display::getHeight() const
{
    return pImpl->getHeight();
}

short int Display::getWidth() const
{
    return pImpl->getWidth();
}

Display::Display(DisplayImpl *impl) : pImpl(impl)
{
    pthread_mutex_init(&dispMutex,NULL);
}

//
// class DrawingContext
//

DrawingContext::DrawingContext(Display& display) : display(display)
{
    pthread_mutex_lock(&display.dispMutex);
}

void DrawingContext::write(Point p, const char* text)
{
    display.pImpl->write(p,text);
}

void DrawingContext::clippedWrite(Point p, Point a, Point b, const char* text)
{
    display.pImpl->clippedWrite(p,a,b,text);
}

void DrawingContext::clear(Color color)
{
    display.pImpl->clear(color);
}

void DrawingContext::clear(Point p1, Point p2, Color color)
{
    display.pImpl->clear(p1,p2,color);
}

void DrawingContext::beginPixel()
{
    display.pImpl->beginPixel();
}

void DrawingContext::setPixel(Point p, Color color)
{
    display.pImpl->setPixel(p,color);
}

void DrawingContext::line(Point a, Point b, Color color)
{
    display.pImpl->line(a,b,color);
}

void DrawingContext::scanLine(Point p, const Color* colors, unsigned short length)
{
    display.pImpl->scanLine(p,colors,length);
}

void DrawingContext::drawImage(Point p, const ImageBase& img)
{
    display.pImpl->drawImage(p,img);
}

void DrawingContext::clippedDrawImage(Point p, Point a, Point b, const ImageBase& img)
{
    display.pImpl->clippedDrawImage(p,a,b,img);
}

void DrawingContext::drawRectangle(Point a, Point b, Color c)
{
    display.pImpl->drawRectangle(a,b,c);
}

short int DrawingContext::getHeight() const
{
    return display.pImpl->getHeight();
}

short int DrawingContext::getWidth() const
{
    return display.pImpl->getWidth();
}

void DrawingContext::setTextColor(Color fgcolor, Color bgcolor)
{
    display.pImpl->setTextColor(fgcolor,bgcolor);
}

Color DrawingContext::getForeground() const
{
    return display.pImpl->getForeground();
}

Color DrawingContext::getBackground() const
{
    return display.pImpl->getBackground();
}

void DrawingContext::setFont(const Font& font)
{
    display.pImpl->setFont(font);
}

Font DrawingContext::getFont() const
{
    return display.pImpl->getFont();
}

DrawingContext::~DrawingContext()
{
    display.pImpl->update();
    pthread_mutex_unlock(&display.dispMutex);
}

} //namespace mxgui
