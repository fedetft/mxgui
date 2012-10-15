
#ifndef PNGCONVERTER_H
#define PNGCONVERTER_H

#include <string>
#include <ostream>
#include <tr1/memory>
#include "libs/png++/png.hpp"

/**
 * Possible pixel formats for an image
 */
enum PixDepth
{
    _1bitlinear, ///< 1bit per pixels, arranged as hoizontal scanlines
    _8,          ///< 8bit per pixel
    _16,         ///< 16bit per pixel
    _18,         ///< 18bit per pixel
    _24          ///< 24bit per pixel
};

/**
 * Abstract base class from which image drivers derive
 */
class ImageWriter
{
public:
    /**
     * Factory member function, returns an appropriate image writer
     * depending on the pixel depth
     * \param img source image
     * \param binary whether the desired output format is a binary file
     * or a C++ header file
     * \param pd desired pixel depth for the target image
     */
    static std::tr1::shared_ptr<ImageWriter> fromPixDepth(
            png::image<png::rgb_pixel>& img, bool binary, PixDepth pd);

    /**
     * Write the image to its output format
     * \param out output file where the image will be written
     * \param outImage secondary output, a png image used for validation output.
     * Optional, can be NULL if not needed.
     */
    virtual void write(std::ofstream& out, png::image<png::rgb_pixel> *outImage=0);

    /**
     * Destructor
     */
    virtual ~ImageWriter() {}

protected:
    /**
     * Write a single pixel. Subclasses need to implemen this if they want
     * the default implementation of write() to work, otherwise can directly
     * reimplement write().
     * \param x x coord of the pixel to write
     * \param y y coord of the pixel to write
     * \param out ostream where to write the pixel data
     * \param outImage if not NULL the implementation needs to set a pixel
     * also in this image, with the same colr depth conversion.
     * \param pix pixel taken from the source image, that needs to be written
     * to the target image
     */
    virtual void writePixel(int x, int y, std::ofstream& out,
            png::image<png::rgb_pixel> *outImage, png::rgb_pixel pix)=0;

    /**
     * \return after how many pixel to insert a carriage return when
     * the output format is a C++ header file
     */
    virtual int pixPerLine() const=0;

    /**
     * Constructor
     * \param img source image
     * \param binary whether the desired output format is a binary file
     * or a C++ header file
     */
    ImageWriter(png::image<png::rgb_pixel>& img, bool binary)
            : img(img), binary(binary) {}
    
    png::image<png::rgb_pixel> img; ///< Source image
    const bool binary;              ///< Output format selection
};

/**
 * Class to produce 1 bit linear images.
 * The image is stored as a sequence of bits representing horizontal
 * lines of the image. Lines are padded to 8bit boundaries
 */
class ImageWriter1bitLinear : public ImageWriter
{
public:
    /**
     * Constructor
     * \param img source image
     * \param binary whether the desired output format is a binary file
     * or a C++ header file
     */
    ImageWriter1bitLinear(png::image<png::rgb_pixel>& img, bool binary)
            : ImageWriter(img, binary) {}
    
    /**
     * This image format is a bit different, and has to redefine
     * a different writing algorithm
     */
    virtual void write(std::ofstream& out,
            png::image<png::rgb_pixel> *outImage=0);

protected:
    /**
     * \return after how many pixel to insert a carriage return when
     * the output format is a C++ header file
     */
    virtual int pixPerLine() const { return 16; }

    /**
     * Write a pixel using the appropriate image format
     */
    virtual void writePixel(int x, int y, std::ofstream& out,
            png::image<png::rgb_pixel> *outImage, png::rgb_pixel pix) {} //Unused
};

/**
 * Class to produce 8 bit per pixel images
 */
class ImageWriter8bit : public ImageWriter
{
public:
    /**
     * Constructor
     * \param img source image
     * \param binary whether the desired output format is a binary file
     * or a C++ header file
     */
    ImageWriter8bit(png::image<png::rgb_pixel>& img, bool binary)
            : ImageWriter(img, binary) {}

protected:
    /**
     * \return after how many pixel to insert a carriage return when
     * the output format is a C++ header file
     */
    virtual int pixPerLine() const { return 16; }
    
    /**
     * Write a pixel using the appropriate image format
     */
    virtual void writePixel(int x, int y, std::ofstream& out,
            png::image<png::rgb_pixel> *outImage, png::rgb_pixel pix);
};

/**
 * Class to produce 16 bit per pixel images
 */
class ImageWriter16bit : public ImageWriter
{
public:
    /**
     * Constructor
     * \param img source image
     * \param binary whether the desired output format is a binary file
     * or a C++ header file
     */
    ImageWriter16bit(png::image<png::rgb_pixel>& img, bool binary)
            : ImageWriter(img, binary) {}

protected:
    /**
     * \return after how many pixel to insert a carriage return when
     * the output format is a C++ header file
     */
    virtual int pixPerLine() const { return 8; }

    /**
     * Write a pixel using the appropriate image format
     */
    virtual void writePixel(int x, int y, std::ofstream& out,
            png::image<png::rgb_pixel> *outImage, png::rgb_pixel pix);
};

/**
 * Class to produce 18 bit per pixel images
 */
class ImageWriter18bit : public ImageWriter
{
public:
    /**
     * Constructor
     * \param img source image
     * \param binary whether the desired output format is a binary file
     * or a C++ header file
     */
    ImageWriter18bit(png::image<png::rgb_pixel>& img, bool binary)
            : ImageWriter(img, binary) {}
   
protected:
    /**
     * \return after how many pixel to insert a carriage return when
     * the output format is a C++ header file
     */
    virtual int pixPerLine() const { return 6; }
    
    /**
     * Write a pixel using the appropriate image format
     */
    virtual void writePixel(int x, int y, std::ofstream& out,
            png::image<png::rgb_pixel> *outImage, png::rgb_pixel pix);
};

/**
 * Class to produce 24 bit per pixel images
 */
class ImageWriter24bit : public ImageWriter
{
public:
    /**
     * Constructor
     * \param img source image
     * \param binary whether the desired output format is a binary file
     * or a C++ header file
     */
    ImageWriter24bit(png::image<png::rgb_pixel>& img, bool binary)
            : ImageWriter(img, binary) {}
       
protected:
    /**
     * \return after how many pixel to insert a carriage return when
     * the output format is a C++ header file
     */
    virtual int pixPerLine() const { return 6; }
    
    /**
     * Write a pixel using the appropriate image format
     */
    virtual void writePixel(int x, int y, std::ofstream& out,
            png::image<png::rgb_pixel> *outImage, png::rgb_pixel pix);
};

#endif //PNGCONVERTER_H
