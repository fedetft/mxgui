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

#pragma once

#include "mxgui_settings.h"
#include "point.h"
#include "color.h"
#include "iterator_direction.h"
#include <algorithm>
#include <type_traits>

namespace mxgui {

namespace impl {
/**
 * \internal
 * Quick 'n dirty auto_ptr like class for arrays. Not complete by any means,
 * but meant to be used internally only. That's why the impl namespace
 */
template<typename T>
class AutoArray
{
public:
    AutoArray(T* ptr) : ptr(ptr) {}
    ~AutoArray() { delete[] ptr; }
    T *get() { return ptr; }
private:
    AutoArray(const AutoArray&);
    AutoArray& operator=(const AutoArray&);
    T *ptr;
};

} //namespace impl

/**
 * \ingroup pub_iface
 * Base class from which image classes derive. This class is pure virtual and
 * as such it is not meant to be directly instantiated.
 */
template<typename T>
class basic_image_base
{
public:
    /**
     * Default constructor
     */
    basic_image_base() : height(0), width(0) {}

    /**
     * Construct an Image
     * \param height the image's height
     * \param width the image's width
     */
    basic_image_base(short height, short width): height(height), width(width) {}

    /**
     * \return the image's height
     */
    short int getHeight() const { return height; }

    /**
     * \return the image's width
     */
    short int getWidth() const { return width; }

    /**
     * Image classes that derive form this class can be divided in two
     * kinds: those that make the whole image available as const T* pointer,
     * and those that load only a small portion of the image at a time in
     * order to minimize RAM usage. This member function can be used to
     * differentiate between the two: if the returned pointer is not NULL then
     * it is a valid pointer to the whole image that can be used to draw the
     * whole image in an optimized way. Otherwise the only way to get image data
     * is to use getScanLine() to retrieve the image one scanline at a time.
     * Note that getScanLine() always work for any kind of image.
     * \return a const pointer to the image's data if available, or NULL
     * otherwise
     */
    virtual const T* getData() const { return 0; }

    /**
     * Get pixels from tha image. This member function can be used to get
     * up to a full horizontal line of pixels from an image.
     * \param p Start point, within <0,0> and <getWidth()-1,getHeight()-1>
     * \param colors pixel data is returned here. Array size must be equal to
     * the length parameter
     * \param length number of pixel to retrieve from the starting point.
     * start.x()+length must be less or equal to getWidth()
     * \return true if success. If false then it means the class does not
     * represent a valid image, or a disk error occurred in case the image
     * is stored on disk.
     */
    virtual bool getScanLine(mxgui::Point p, mxgui::Color colors[],
            unsigned short length) const;

    /**
     * Draw an image on a surface
     * \param surface an object that provides pixel iterators.
     * \param p point of the upper left corner where the image will be drawn
     */
    template<typename U>
    void draw(U& surface, Point p) const;

    /**
     * Draw part of an image on a surface
     * \param surface an object that provides pixel iterators.
     * \param p point of the upper left corner where the image will be drawn.
     * Negative coordinates are allowed, as long as the clipped view has
     * positive or zero coordinates
     * \param a Upper left corner of clipping rectangle
     * \param b Lower right corner of clipping rectangle
     */
    template<typename U>
    void clippedDraw(U& surface, Point p, Point a, Point b) const;

    /**
     * Virtual destructor
     */
    virtual ~basic_image_base()=0;

    //Uses default copy constructor and operator=
protected:
    short int height, width;
};

template<typename T>
bool basic_image_base<T>::getScanLine(mxgui::Point p, mxgui::Color colors[],
            unsigned short length) const
{
    if(p.x()<0 || p.y()<0) return false;
    if(p.x()>=this->getWidth() || p.y()>=this->getHeight()) return false;
    const T* data=this->getData();
    if(data==0) return false;
    data+=p.x()+p.y()*this->getWidth();
    
    // If the image pixel format matches display pixel format, we can do a direct copy
    if constexpr (std::is_same<T, mxgui::Color>::value)
    {
        for(unsigned short i=0;i<length;i++) colors[i]=data[i];
    }
    // Otherwise conversion is needed 
    // (consider to re-generate the image in the display format
    // using _tools/code_generator/pngconverter)
    else
    {
        for(unsigned short i=0;i<length;i++)
        {
            colors[i]=mxgui::Color(data[i].getR(), data[i].getG(), data[i].getB());
        }
    }
    return true;
}

// Specialization for Gray1Color
template<>
inline bool basic_image_base<Gray1Color>::getScanLine(mxgui::Point p, mxgui::Color colors[],
            unsigned short length) const
{
    if(p.x()<0 || p.y()<0) return false;
    if(p.x()>=this->getWidth() || p.y()>=this->getHeight()) return false;
    const unsigned char* data = reinterpret_cast<const unsigned char*>(this->getData());
    if(data==0) return false;

    // Calculate stride: 1bpp are byte aligned
    int stride = ((this->getWidth() + 7) & (~7)) / 8;
    const unsigned char* rowData = data + (p.y() * stride);
    
    for(unsigned short i=0; i<length; i++)
    {
        int x = p.x() + i;
        if(x >= this->getWidth()) break;
        
        unsigned char byte = rowData[x / 8];
        int bitIndex = x & 7;
        
        bool set = (byte & (0x80 >> bitIndex));
        colors[i] = set ? Color::white() : Color::black();
    }
    return true;
}

// Specialization for Gray4Color
template<>
inline bool basic_image_base<Gray4Color>::getScanLine(mxgui::Point p, mxgui::Color colors[],
            unsigned short length) const
{
    if(p.x()<0 || p.y()<0) return false;
    if(p.x()>=this->getWidth() || p.y()>=this->getHeight()) return false;
    const unsigned char* data = reinterpret_cast<const unsigned char*>(this->getData());
    if(data==0) return false;

    // Calculate stride: 4bpp are byte aligned
    int stride = (this->getWidth() + 1) / 2;
    const unsigned char* rowData = data + (p.y() * stride);
    
    for(unsigned short i=0; i<length; i++)
    {
        int x = p.x() + i;
        if(x >= this->getWidth()) break;
        
        unsigned char byte = rowData[x / 2];
        unsigned char val = (x % 2 == 0) ? (byte >> 4) : (byte & 0x0F);

        if constexpr (std::is_same<mxgui::Color, Gray4Color>::value)
        {
            colors[i] = Color::fromRaw(val);
        }
        else
        {
            Gray4Color srcColor = Gray4Color::fromRaw(val);
            colors[i] = Color(srcColor.getR(), srcColor.getG(), srcColor.getB());
        }
    }
    return true;
}

template<typename T> template<typename U>
void basic_image_base<T>::draw(U& surface, Point p) const
{
    using namespace std;
    const T *imgData=this->getData();
    if(imgData!=0)
    {
        short int xEnd=p.x()+this->getWidth()-1;
        short int yEnd=p.y()+this->getHeight()-1;
        typename U::pixel_iterator it=surface.begin(p,Point(xEnd,yEnd),RD);
        int imgSize=this->getHeight()*this->getWidth();
        
        // Block copy optimization for matching formats
        if constexpr (std::is_same<T, mxgui::Color>::value)
        {
             for(int i=0;i<imgSize;i++) *it=*imgData++; 
        }
        else
        {
               for(int i=0;i<imgSize;i++)
               {
                  const T& src=*imgData++;
                  *it=Color(src.getR(), src.getG(), src.getB());
               }
        }
    } else {
        // Fallback for non-memory-mapped images
        short length=this->width;
        impl::AutoArray<Color> line(new Color[length]);
        for(short i=0;i<this->height;i++)
        {
            if(this->getScanLine(Point(0,i),line.get(),length)==false) return;
            surface.scanLine(Point(p.x(),p.y()+i),line.get(),length);
        }
    }
}

// Specialization for Gray1Color
template<> template<typename U>
void basic_image_base<Gray1Color>::draw(U& surface, Point p) const
{
    using namespace std;
    const unsigned char *imgData=reinterpret_cast<const unsigned char*>(this->getData());
    if(imgData!=0)
    {
        short int xEnd=p.x()+this->getWidth()-1;
        short int yEnd=p.y()+this->getHeight()-1;
        typename U::pixel_iterator it=surface.begin(p,Point(xEnd,yEnd),RD);
        int h=this->getHeight();
        int w=this->getWidth();
        int stride=((w+7) & (~7))/8; //1bpp images have lines byte aligned
        int last= w/8;
        for(int i=0;i<h;i++)
        {
            int base=i*stride;
            for(int j=0;j<last;j++)
            {
                unsigned char data=imgData[base+j];
                // Pixels are packed MSB first
                *it = (data & 0x80) ? Color::white() : Color::black(); data<<=1;
                *it = (data & 0x80) ? Color::white() : Color::black(); data<<=1;
                *it = (data & 0x80) ? Color::white() : Color::black(); data<<=1;
                *it = (data & 0x80) ? Color::white() : Color::black(); data<<=1;
                *it = (data & 0x80) ? Color::white() : Color::black(); data<<=1;
                *it = (data & 0x80) ? Color::white() : Color::black(); data<<=1;
                *it = (data & 0x80) ? Color::white() : Color::black(); data<<=1;
                *it = (data & 0x80) ? Color::white() : Color::black();
            }
            if (w & 7)
            {
                unsigned char data=imgData[base+last]; // Last partial byte
                for(int k=0; k<(w & 7); k++)
                {
                    *it = (data & 0x80) ? Color::white() : Color::black();
                    data<<=1;
                }
            }
        }
    } else {
        // Fallback for non-memory-mapped images
        short length=this->width;
        impl::AutoArray<Color> line(new Color[length]);
        for(short i=0;i<this->height;i++)
        {
            if(this->getScanLine(Point(0,i),line.get(),length)==false) return;
            surface.scanLine(Point(p.x(),p.y()+i),line.get(),length);
        }
    }
}

// Specialization for Gray4Color
template<> template<typename U>
void basic_image_base<Gray4Color>::draw(U& surface, Point p) const
{
    using namespace std;
    const unsigned char *imgData=reinterpret_cast<const unsigned char*>(this->getData());
    if(imgData!=0)
    {
        short int xEnd=p.x()+this->getWidth()-1;
        short int yEnd=p.y()+this->getHeight()-1;
        typename U::pixel_iterator it=surface.begin(p,Point(xEnd,yEnd),RD);
        int h=this->getHeight();
        int w=this->getWidth();
        int stride=((w+1) & (~1))/2; // 4bpp images have lines byte aligned (2 pixels per byte)
        int pairs= w/2; 
        
        for(int i=0;i<h;i++)
        {
            int base=i*stride;
            for(int j=0;j<pairs;j++)
            {
                unsigned char data=imgData[base+j];
                if constexpr (std::is_same<mxgui::Color, Gray4Color>::value)
                {
                    *it = Color::fromRaw(data >> 4);
                    *it = Color::fromRaw(data & 0x0F);
                }
                else
                {
                    Gray4Color srcHigh = Gray4Color::fromRaw(data >> 4);
                    Gray4Color srcLow = Gray4Color::fromRaw(data & 0x0F);
                    *it = Color(srcHigh.getR(), srcHigh.getG(), srcHigh.getB());
                    *it = Color(srcLow.getR(), srcLow.getG(), srcLow.getB());
                }
            }
            if (w & 1)
            {
                unsigned char data=imgData[base+pairs]; // Last partial byte
                if constexpr (std::is_same<mxgui::Color, Gray4Color>::value)
                {
                    *it = Color::fromRaw(data >> 4);
                }
                else
                {
                    Gray4Color srcHigh = Gray4Color::fromRaw(data >> 4);
                    *it = Color(srcHigh.getR(), srcHigh.getG(), srcHigh.getB());
                }
            }
        }
    } else {
        // Fallback for non-memory-mapped images
        short length=this->width;
        impl::AutoArray<Color> line(new Color[length]);
        for(short i=0;i<this->height;i++)
        {
            if(this->getScanLine(Point(0,i),line.get(),length)==false) return;
            surface.scanLine(Point(p.x(),p.y()+i),line.get(),length);
        }
    }
}

template<typename T> template<typename U>
void basic_image_base<T>::clippedDraw(U& surface,
        Point p, Point a, Point b) const
{
    using namespace std;
    //Find rectangle wich is the non-empty intersection of the image rectangle
    //with the clip rectangle
    short xa=max(p.x(),a.x());
    short xb=min<short>(p.x()+this->getWidth()-1,b.x());
    if(xa>xb) return; //Empty intersection

    short ya=max(p.y(),a.y());
    short yb=min<short>(p.y()+this->getHeight()-1,b.y());
    if(ya>yb) return; //Empty intersection

    //Draw image
    short nx=xb-xa+1;
    short ny=yb-ya+1;
    const T *imgData=this->getData();
    if(imgData!=0)
    {
        typename U::pixel_iterator it=surface.begin(Point(xa,ya),
                Point(xb,yb),RD);
        int skipStart=(ya-p.y())*this->getWidth()+(xa-p.x());
        imgData+=skipStart;
        int toSkip=(xa-p.x())+((p.x()+this->getWidth()-1)-xb);
        for(short i=0;i<ny;i++)
        {
            if constexpr (std::is_same<T, mxgui::Color>::value)
            {
                for(short j=0;j<nx;j++) *it=*imgData++;
            }
            else
            {
                for(short j=0;j<nx;j++)
                {
                    const T& src=*imgData++;
                    *it=Color(src.getR(), src.getG(), src.getB());
                }
            }
            imgData+=toSkip;
        }      
    } else {
        // Fallback for non-memory-mapped images
        impl::AutoArray<Color> line(new Color[nx]);
        for(short i=0;i<ny;i++)
        {
            if(this->getScanLine(Point(xa-p.x(),ya-p.y()+i),line.get(),nx)
                    ==false) return;
            surface.scanLine(Point(xa,ya+i),line.get(),nx);
        }
    }
}

// Specialization for Gray1Color
template<> template<typename U>
void basic_image_base<Gray1Color>::clippedDraw(U& surface,
        Point p, Point a, Point b) const
{
    using namespace std;
    //Find rectangle wich is the non-empty intersection of the image rectangle
    //with the clip rectangle
    short xa=max(p.x(),a.x());
    short xb=min<short>(p.x()+this->getWidth()-1,b.x());
    if(xa>xb) return; //Empty intersection

    short ya=max(p.y(),a.y());
    short yb=min<short>(p.y()+this->getHeight()-1,b.y());
    if(ya>yb) return; //Empty intersection

    short nx=xb-xa+1;
    short ny=yb-ya+1;
    const unsigned char *imgData=reinterpret_cast<const unsigned char*>(this->getData());
    if(imgData!=0)
    {
        typename U::pixel_iterator it=surface.begin(Point(xa,ya),
                Point(xb,yb),RD);
        
        int w=this->getWidth();
        int stride=((w+7) & (~7))/8;
        
        // Loop over rows in the clipped region
        for(int i=0;i<ny;i++)
        {
            int y = ya + i; // Absolute Y coordinate in image
            int rowBase = (y - p.y()) * stride; // Offset to start of row
            
            // Loop over columns in clipped region
            for(int j=0; j<nx; j++)
            {
                int x = xa + j; // Absolute X coordinate in image
                int imgX = x - p.x(); // Image relative X
                
                unsigned char data = imgData[rowBase + (imgX / 8)];
                int bitIndex = imgX & 7; // 0..7
                bool set = (data & (0x80 >> bitIndex));
                *it = set ? Color::white() : Color::black();
            }
        }       
    } else {
        // Fallback for non-memory-mapped images
        impl::AutoArray<Color> line(new Color[nx]);
        for(short i=0;i<ny;i++)
        {
            if(this->getScanLine(Point(xa-p.x(),ya-p.y()+i),
                    line.get(),nx)==false) return;
            surface.scanLine(Point(xa,ya+i),line.get(),nx);
        }
    }
}

// Specialization for clippedDraw Gray4Color
template<> template<typename U>
void basic_image_base<Gray4Color>::clippedDraw(U& surface,
        Point p, Point a, Point b) const
{
    using namespace std;
     //Find rectangle wich is the non-empty intersection of the image rectangle
    //with the clip rectangle
    short xa=max(p.x(),a.x());
    short xb=min<short>(p.x()+this->getWidth()-1,b.x());
    if(xa>xb) return; //Empty intersection

    short ya=max(p.y(),a.y());
    short yb=min<short>(p.y()+this->getHeight()-1,b.y());
    if(ya>yb) return; //Empty intersection

    short nx=xb-xa+1;
    short ny=yb-ya+1;
    const unsigned char *imgData=reinterpret_cast<const unsigned char*>(this->getData());
    if(imgData!=0)
    {
        typename U::pixel_iterator it=surface.begin(Point(xa,ya),
                Point(xb,yb),RD);
        
        int w=this->getWidth();
        int stride=((w+1) & (~1))/2;
        
        // Loop over rows in the clipped region
        for(int i=0;i<ny;i++)
        {
            int y = ya + i; // Absolute Y coordinate in image
            int rowBase = (y - p.y()) * stride; // Offset to start of row
            
            // Loop over columns in clipped region
            for(int j=0; j<nx; j++)
            {
                int x = xa + j; // Absolute X coordinate in image
                int imgX = x - p.x(); // Image relative X
                
                unsigned char data = imgData[rowBase + (imgX / 2)];
                bool highNibble = (imgX & 1) == 0;
                unsigned char val = highNibble ? (data >> 4) : (data & 0x0F);

                if constexpr (std::is_same<mxgui::Color, Gray4Color>::value)
                {
                    *it = Color::fromRaw(val);
                }
                else
                {
                    Gray4Color srcColor = Gray4Color::fromRaw(val);
                    *it = Color(srcColor.getR(), srcColor.getG(), srcColor.getB());
                }
            }
        }       
    } else {
        impl::AutoArray<Color> line(new Color[nx]);
        for(short i=0;i<ny;i++)
        {
            if(this->getScanLine(Point(xa-p.x(),ya-p.y()+i),line.get(),nx)
                    ==false) return;
            surface.scanLine(Point(xa,ya+i),line.get(),nx);
        }
    }
}

template<typename T>
basic_image_base<T>::~basic_image_base() {}

/// \ingroup pub_iface
/// Define the ImageBase class
typedef basic_image_base<Color> ImageBase;

/**
 * \ingroup pub_iface
 * This class is an image compiled statically with the code.
 * 
 * The expected use of this class is like this:
 * - user has an image in .png format that he/she wants to embed into the
 *   firmware
 * - user uses the pngconverter tool to convert the image into a .cpp and  a .h
 *   file with a static instance of this class, named after the png file name
 * - user can then draw the image onscreen
 * 
 * Images are immutable except they can be assigned with operator=
 */
template<typename T>
class basic_image : public basic_image_base<T>
{
public:
    /**
     * Construct an Image
     * \param height the image's height
     * \param width the image's width
     * \param data the pointer to the image's data. Ownsership of the data is
     * still of the caller. If the pointer points to const data no special care
     * must be taken, otherwise the caller must free the memory when the Image
     * is no longer useful, to avoid a memory leak.
     */
    basic_image(short int height, short int width, const void *data)
            : basic_image_base<T>(height, width),
              data(reinterpret_cast<const T*>(data)) {}

    /**
     * \return a const pointer to the image's data
     */
    virtual const T* getData() const { return data; }

    /**
     * Virtual destructor. The pointer is not deallocated because this class
     * is meant to keep a pointer to a non dynamically allocated image, such
     * as to a const array in .rodata
     */
    virtual ~basic_image() {}

    //Uses default copy constructor and operator=. The pointer can be shared
    //without problems since there is no member function to modify the image
    //data nor to return a non-const pointer to it
private:
    const T *data;
};

/// \ingroup pub_iface
/// Define the Image class
typedef basic_image<Color> Image;

} // namespace mxgui
