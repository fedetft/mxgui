/***************************************************************************
 *   Copyright (C) 2010, 2011, 2012, 2013, 2014 by Terraneo Federico       *
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

/*
 * pngconverter.cpp
 * convert a png image into a .cpp file to be used with mxgui.
 * Supported pixel formats: gray1, gray4, rgb332, rgb565.
 */

#include <iostream>
#include <stdexcept>
#include <boost/program_options.hpp>
#include "pngconverter.h"

using namespace std;
using namespace png;
using namespace boost::program_options;

/**
 * \param an unsigned short
 * \return the same short forced into little endian representation
 */
static unsigned short toLittleEndian(unsigned short x)
{
	static bool first=true, little;
	union {
		unsigned short a;
		unsigned char b[2];
	} endian;
	if(first)
	{
		endian.a=0x12;
		little=endian.b[0]==0x12;
		first=false;
	}
	if(little) return x;
	endian.a=x;
	swap(endian.b[0],endian.b[1]);
	return endian.a;
}

/**
 * \param x a string
 * \return the same string with uppercase characters
 */
static string toUpper(const string& x)
{
    string result(x);
    for(int i=0;i<result.length();i++) result[i]=toupper(result[i]);
    return result;
}

//
// class ImageWriter
//

std::shared_ptr<ImageWriter> ImageWriter::fromPixelFormat(image<rgb_pixel>& img,
        bool binary, PixelFormat pf)
{
    switch(pf)
    {
        case PixelFormat::Gray1:
            return shared_ptr<ImageWriter>(new ImageWriterGray1(img,binary));
         case PixelFormat::Gray4:
            return shared_ptr<ImageWriter>(new ImageWriterGray4(img,binary));
        case PixelFormat::RGB332:
            return shared_ptr<ImageWriter>(new ImageWriterRGB332(img,binary));
        case PixelFormat::RGB565:
            return shared_ptr<ImageWriter>(new ImageWriterRGB565(img,binary));
        default:
            throw runtime_error("Unsupported pixel format");
    }
}

void ImageWriter::write(ofstream& out, image<rgb_pixel> *outImage)
{
    int numPerLine=0;//Number of pixel per line. when reaches limit, wrap
    for(int y=0;y<img.get_height();y++)
    {
        for(int x=0;x<img.get_width();x++)
        {
            writePixel(x,y,out,outImage,img.get_pixel(x,y));
            if(binary) continue;
            if((x==img.get_width()-1)&&(y==img.get_height()-1))
            {
                //It's the last pixel, so do not print ','
            } else {
                out<<',';
                if(++numPerLine==pixPerLine())
                {
                    numPerLine=0;
                    out<<endl<<' ';
                }
            }
        }
    }
}

//
// class  ImageWriterGray1
//

void ImageWriterGray1::write(ofstream& out,
        image<rgb_pixel> *outImage)
{
    int numPerLine=0;//Number of pixel per line. when reaches limit, wrap
    int ctr=0;
    unsigned char buffer=0;
    for(int y=0;y<img.get_height();y++)
    {
        for(int x=0;x<img.get_width();x++)
        {
            rgb_pixel pix=img.get_pixel(x,y);
            if(pix.red!=0 && pix.blue!=0 && pix.green!=0)
            {
                buffer |= 0x1;
                if(outImage)
                    outImage->set_pixel(x,y,rgb_pixel(255,255,255));
            } else {
                if(outImage)
                    outImage->set_pixel(x,y,rgb_pixel(0,0,0));
            }
            if(++ctr>=8)
            {
                ctr=0;
                if(binary) out.write(reinterpret_cast<char*>(&buffer),1);
                else {
                    out<<static_cast<int>(buffer);
                    if(x!=img.get_width()-1 || y!=img.get_height()-1) 
                    {
                        out<<',';
                        if(++numPerLine==pixPerLine())
                        {
                            numPerLine=0;
                            out<<endl<<' ';
                        }
                    }
                }
                buffer=0;
            } else buffer<<=1;
        }
        //End of a line, flush buffer
        if(ctr!=0)
        { 
            buffer<<=(7-ctr);
            if(binary) out.write(reinterpret_cast<char*>(&buffer),1);
            else {
                out<<static_cast<int>(buffer);
                if(y!=img.get_height()-1) 
                {
                    out<<',';
                    if(++numPerLine==pixPerLine())
                    {
                        numPerLine=0;
                        out<<endl<<' ';
                    }
                }
            }
            buffer=0;
            ctr=0;
        }
    }    
}

//
// class  ImageWriterGray4
//

void ImageWriterGray4::write(ofstream& out,
        image<rgb_pixel> *outImage)
{
    int numPerLine=0;//Number of pixel per line. when reaches limit, wrap
    int ctr=0;
    unsigned char buffer=0;
    for(int y=0;y<img.get_height();y++)
    {
        for(int x=0;x<img.get_width();x++)
        {
            rgb_pixel pix=img.get_pixel(x,y);
            // Convert to grayscale 4 bit (0-15)
            // Using same coefficients as Gray4Color: (r * 77 + g * 150 + b * 29) >> 12
            unsigned char gray = (pix.red * 77 + pix.green * 150 + pix.blue * 29) >> 12;
            if (gray > 15) gray = 15;

            if(outImage)
            {
                // Convert back to RGB for preview
                unsigned char c = (gray << 4) | gray;
                outImage->set_pixel(x,y,rgb_pixel(c,c,c));
            }
            
            // Pack into buffer. MSB first means first pixel is in high nibble.
            if(ctr==0)
            {
                buffer = (gray << 4);
                ctr++;
            }
            else
            {
                buffer |= (gray & 0x0F);
                ctr=0;
                
                // Write byte
                if(binary) out.write(reinterpret_cast<char*>(&buffer),1);
                else {
                    out<<"0x"<<hex<<static_cast<int>(buffer)<<dec;
                    if(x!=img.get_width()-1 || y!=img.get_height()-1) 
                    {
                        out<<',';
                        if(++numPerLine==16) // 16 bytes per line
                        {
                            numPerLine=0;
                            out<<endl<<' ';
                        }
                    }
                }
                buffer=0;
            }
        }
        //End of a line, flush buffer if we have an odd number of pixels
        if(ctr!=0)
        { 
            // buffer already has the high nibble set, low nibble is 0
            if(binary) out.write(reinterpret_cast<char*>(&buffer),1);
            else {
                out<<"0x"<<hex<<static_cast<int>(buffer)<<dec;
                if(y!=img.get_height()-1) 
                {
                    out<<',';
                    if(++numPerLine==16)
                    {
                        numPerLine=0;
                        out<<endl<<' ';
                    }
                }
            }
            buffer=0;
            ctr=0;
        }
    }    
}

//
// class  ImageWriterRGB332
//

void ImageWriterRGB332::writePixel(int x, int y, ofstream& out,
        png::image<rgb_pixel> *outImage, rgb_pixel pix)
{
    unsigned int r=pix.red & (7<<5);
    unsigned int g=pix.green & (7<<5);
    unsigned int b=pix.blue & (3<<6);
    unsigned int i=r | g>>3 | b>>6;
    if(binary)
    {
        unsigned char c=static_cast<unsigned char>(i);
        out.write(reinterpret_cast<char*>(&c),1);
    } else out<<"0x"<<hex<<i<<dec;

    if(outImage) outImage->set_pixel(x,y,rgb_pixel(r,g,b));
}

//
// class  ImageWriterRGB565
//

void ImageWriterRGB565::writePixel(int x, int y, ofstream& out,
        png::image<rgb_pixel> *outImage, rgb_pixel pix)
{
    unsigned int r=(pix.red & (31<<3))>>3;
    unsigned int g=(pix.green & (63<<2))>>2;
    unsigned int b=(pix.blue & (31<<3))>>3;
    unsigned int i=(r<<(5+6) | g<<5 | b);
    if(binary)
    {
        unsigned short s=toLittleEndian(static_cast<unsigned short>(i));
        out.write(reinterpret_cast<char*>(&s),2);
    } else out<<"0x"<<hex<<i<<dec;
    if(outImage) outImage->set_pixel(x,y,rgb_pixel(r,g,b));
}

int main(int argc, char *argv[])
{
    //Check args
    options_description desc("PngConverter utility v1.22\n"
        "Designed by TFT : Terraneo Federico Technologies\nOptions");
    desc.add_options()
        ("help", "Prints this.")
        ("in", value<string>(), "Input png file (required)")
        ("format", value<string>(), "Pixel format: gray1, gray4, rgb332, rgb565 (required)")
        ("out", value<string>(), "Output png file for validation")
        ("outdir", value<string>(), "Directory where to generate files (default is src dir)")
        ("binary", "Generate a binary file instead of a .cpp/.h file")
    ;

    variables_map vm;
    store(parse_command_line(argc,argv,desc),vm);
    notify(vm);

    if(vm.count("help") || (!vm.count("in")) || (!vm.count("format")))
    {
        cerr<<desc<<endl;
        return 1;
    }
    
    //Load image
    image<rgb_pixel> img(vm["in"].as<string>());
    cout<<"Loaded image \""<<vm["in"].as<string>()<<"\". Info:"<<endl;
    cout<<"Height  = "<<img.get_height()<<endl;
    cout<<"Width   = "<<img.get_width()<<endl;

    string format=vm["format"].as<string>();
    PixelFormat pf;
    if(format=="gray1")
    {
        pf=PixelFormat::Gray1;
    } else if(format=="gray4")
    {
        pf=PixelFormat::Gray4;
    } else if(format=="rgb332")
    {
        pf=PixelFormat::RGB332;
    } else if(format=="rgb565")
    {
        pf=PixelFormat::RGB565;
    } else
    throw runtime_error("Unsupported pixel format (not gray1, gray4, rgb332, rgb565)");
    /*
     * Get output filemane from input filename
     * Example: if in is "/home/mypng.png"
     * filename        is "mypng"
     * path            is "/home/"
     * cppFilename     is "/home/mypng.cpp"
     * hFilename       is "/home/mypng.h"
     * unless outdir is set
     */
    string path="";
    string filename=vm["in"].as<string>();
    //Remove path
    size_t lastSlash=filename.find_last_of('/');
    if(lastSlash!=string::npos)
    {
        path=filename.substr(0,lastSlash+1);
        filename=filename.substr(lastSlash+1,filename.length());
    }
    //Override path if requested
    if(vm.count("outdir")) path=vm["outdir"].as<string>()+'/';
    //Remove extension
    filename=filename.substr(0,filename.find('.'));
    string cppFilename=path+filename+".cpp";
    string hFilename=path+filename+".h";

    //Convert image, step 1 (make .cpp file)
    const bool binary=vm.count("binary");
    ofstream file(binary ? filename.c_str() : cppFilename.c_str(),ios::binary);
    if(!file.good())
        throw(runtime_error(string("Can't open file: ")+cppFilename));

    bool outRequested=false;
    image<rgb_pixel> outImage;
    if(vm.count("out"))
    {
        outRequested=true;
        outImage=image<rgb_pixel>(img.get_width(),img.get_height());
    }
 
    if(binary==false)
    {
        file<<endl<<"//This file has been automatically generated by "
                "pngconverter utility"<<endl<<"//Please do not edit"<<endl
                <<"#include \""<<filename<<".h\""<<endl<<endl
                <<"using namespace mxgui;"<<endl<<endl
                <<"static const short int height="<<img.get_height()<<';'<<endl
                <<"static const short int width ="<<img.get_width()<<';'<<endl
                <<endl;
        //RGB565 uses unsigned short storage
        if(pf==PixelFormat::RGB565) 
        {
            file<<"static const unsigned short pixelData[]={"<<endl<<' ';
        }
        else file<<"static const unsigned char pixelData[]={"<<endl<<' '; // Gray1, Gray4, RGB332 use raw byte storage
    } else {
        unsigned short header[3];
        header[0]=toLittleEndian((unsigned short)img.get_height());
        header[1]=toLittleEndian((unsigned short)img.get_width());
        header[2]=toLittleEndian(static_cast<unsigned short>(pf));
        file.write(reinterpret_cast<char*>(&header),sizeof(header));
    }

    shared_ptr<ImageWriter> imgw=ImageWriter::fromPixelFormat(img,binary,pf);
    imgw->write(file, outRequested ? &outImage : 0);

    if(!binary)
    {
        // The image is declared simply as "Image" in the .h, while as
        // "basic_image<type>" in the .cpp. This is a trick to allow a
        // compile time check that the image pixel format is correct.
        // If both the image and the mxgui configurations agree the file
        // will compile. Also, using "Image" in the .h allows to include
        // header files that refer to "optional" images without causing
        // compiler errors
        string classname;
        switch(pf)
        {
            case PixelFormat::Gray1:
                classname="Gray1Color";
                break;
            case PixelFormat::Gray4:
                classname="Gray4Color";
                break;
            case PixelFormat::RGB332:
                classname="RGB332Color";
                break;
            case PixelFormat::RGB565:
                classname="RGB565Color";
                break;
        }
        file<<endl<<"};"<<endl<<endl<<"const basic_image<"<<classname<<"> "
            <<filename<<"(height,width,pixelData);";
    }
    file.close();

    if(outRequested) outImage.write(vm["out"].as<string>());

    //Convert image, step 2 (make .h file)
    if(binary) return 0;
    file.open(hFilename.c_str());
    if(!file.good())
        throw(runtime_error(string("Can't open file: ")+hFilename));
    
    file<<endl<<"//This file has been automatcally generated by "
            "pngconverter utility"<<endl<<"//Please do not edit"<<endl
            <<"#ifndef "<<toUpper(filename)<<"_H"<<endl
            <<"#define "<<toUpper(filename)<<"_H"<<endl<<endl
            <<"#include \"mxgui/image.h\""<<endl<<endl
            <<"extern const mxgui::Image "<<filename<<";"<<endl<<endl
            <<"#endif //"<<toUpper(filename)<<"_H"<<endl;
    file.close();
    return 0;
}
