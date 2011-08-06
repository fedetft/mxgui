
#include "display.h"
#include "drivers/display_stm3210e-eval.h"
#include "drivers/display_mp3v2.h"
#include "drivers/display_qt.h"

namespace mxgui {

Display& Display::instance(const char *id)
{
    static DisplayImpl implementation;
    static Display singleton(&implementation);
    return singleton;
}

void Display::write(Point p, const char* text)
{
    pImpl->write(p,text);
}

void Display::clippedWrite(Point p, Point a, Point b, const char* text)
{
    pImpl->clippedWrite(p,a,b,text);
}

void Display::clear(Color color)
{
    pImpl->clear(color);
}

void Display::clear(Point p1, Point p2, Color color)
{
    pImpl->clear(p1,p2,color);
}

void Display::beginPixel()
{
    pImpl->beginPixel();
}

void Display::setPixel(Point p, Color color)
{
    pImpl->setPixel(p,color);
}

void Display::line(Point a, Point b, Color color)
{
    pImpl->line(a,b,color);
}

void Display::scanLine(Point p, const Color* colors, unsigned short length)
{
    pImpl->scanLine(p,colors,length);
}

void Display::drawImage(Point p, const ImageBase& img)
{
    pImpl->drawImage(p,img);
}

void Display::clippedDrawImage(Point p, Point a, Point b, const ImageBase& img)
{
    pImpl->clippedDrawImage(p,a,b,img);
}

void Display::drawRectangle(Point a, Point b, Color c)
{
    pImpl->drawRectangle(a,b,c);
}

short int Display::getHeight() const
{
    return pImpl->getHeight();
}

short int Display::getWidth() const
{
    return pImpl->getWidth();
}

void Display::turnOn()
{
    pImpl->turnOn();
}

void Display::turnOff()
{
    pImpl->turnOff();
}

void Display::setTextColor(Color fgcolor, Color bgcolor)
{
    pImpl->setTextColor(fgcolor,bgcolor);
}

Color Display::getForeground() const
{
    return pImpl->getForeground();
}

Color Display::getBackground() const
{
    return pImpl->getBackground();
}

void Display::setFont(const Font& font)
{
    pImpl->setFont(font);
}

Font Display::getFont() const
{
    return pImpl->getFont();
}

void Display::update()
{
    pImpl->update();
}

Display::Display(DisplayImpl *impl) : pImpl(impl) {}

} //namespace mxgui
