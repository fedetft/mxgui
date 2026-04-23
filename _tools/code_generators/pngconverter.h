
#ifndef PNGCONVERTER_H
#define PNGCONVERTER_H

#include <string>
#include <ostream>
#include <memory>
#include "libs/png++/png.hpp"

/**
 * Possible pixel formats for an image
 */
enum class PixelFormat
{
    Gray1, ///< 1bit per pixels, arranged as hoizontal scanlines
    Gray4, ///< 4bit per pixel grayscale
    RGB332,     ///< 8bit per pixel 332
    RGB565      ///< 16bit per pixel 565
};

/**
 * Abstract base class from which image writers derive
 */
class ImageWriter
{
public:
    /**
     * Factory member function, returns an appropriate image writer
     * depending on the pixel format
     * \param img source image
     * \param binary whether the desired output format is a binary file
     * or a C++ header file
     * \param pf desired pixel format for the target image
     */
    static std::shared_ptr<ImageWriter> fromPixelFormat(
            png::image<png::rgb_pixel>& img, bool binary, PixelFormat pf);

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
     * Write a single pixel. Subclasses need to implement this if they want
     * the default implementation of write() to work, otherwise can directly
     * reimplement write().
     * \param x x coord of the pixel to write
     * \param y y coord of the pixel to write
     * \param out ostream where to write the pixel data
     * \param outImage if not NULL the implementation needs to set a pixel
     * also in this image, with the same pixel format conversion.
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
 * Class to produce Gray1 pixel format images.
 * The image is stored as a sequence of bits representing horizontal
 * lines of the image. Lines are padded to 8bit boundaries
 */
class ImageWriterGray1 : public ImageWriter
{
public:
        /**
         * Constructor
         * \param img source image
         * \param binary whether the desired output format is a binary file
         * or a C++ header file
         */
        ImageWriterGray1(png::image<png::rgb_pixel>& img, bool binary)
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
 * Class to produce Gray4 pixel format images. 0-15 grayscale. Packed.
 */
class ImageWriterGray4 : public ImageWriter
{
public:
    ImageWriterGray4(png::image<png::rgb_pixel>& img, bool binary)
            : ImageWriter(img, binary) {}
    
    virtual void write(std::ofstream& out,
            png::image<png::rgb_pixel> *outImage=0);

protected:
    virtual int pixPerLine() const { return 16; }

    virtual void writePixel(int x, int y, std::ofstream& out,
            png::image<png::rgb_pixel> *outImage, png::rgb_pixel pix) {} //Unused
};

/**
 * Class to produce RGB332 pixel format images
 */
class ImageWriterRGB332 : public ImageWriter
{
public:
    /**
     * Constructor
     * \param img source image
     * \param binary whether the desired output format is a binary file
     * or a C++ header file
     */
    ImageWriterRGB332(png::image<png::rgb_pixel>& img, bool binary)
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
 * Class to produce RGB565 pixel format images
 */
class ImageWriterRGB565 : public ImageWriter
{
public:
    /**
     * Constructor
     * \param img source image
     * \param binary whether the desired output format is a binary file
     * or a C++ header file
     */
    ImageWriterRGB565(png::image<png::rgb_pixel>& img, bool binary)
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

#endif //PNGCONVERTER_H
