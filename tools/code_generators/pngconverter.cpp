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

/*
 * pngconverter.cpp
 * convert a png image into a .cpp file to be used with mxgui.
 * number of bit per pixel of output image can be configured as
 * 1bitlinear, 8, 16, 18 or 24.
 */

#include <iostream>
#include <stdexcept>
#include <boost/program_options.hpp>
#include "pngconverter.h"

using namespace std;
using namespace std::tr1;
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

shared_ptr<ImageWriter> ImageWriter::fromPixDepth(image<rgb_pixel>& img,
        bool binary, PixDepth pd)
{
    switch(pd)
    {
        case _1bitlinear:
            return shared_ptr<ImageWriter>(new ImageWriter1bitLinear(img,binary));
        case _8:
            return shared_ptr<ImageWriter>(new ImageWriter8bit(img,binary));
        case _16:
            return shared_ptr<ImageWriter>(new ImageWriter16bit(img,binary));
        case _18:
            return shared_ptr<ImageWriter>(new ImageWriter18bit(img,binary));
        case _24:
            return shared_ptr<ImageWriter>(new ImageWriter24bit(img,binary));
        default:
            throw runtime_error("Unsupported pixel depth");
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
// class  ImageWriter1bitLinear
//

void ImageWriter1bitLinear::write(ofstream& out,
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
// class  ImageWriter8bit
//

void ImageWriter8bit::writePixel(int x, int y, ofstream& out,
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
    } else out<<i;
    if(outImage) outImage->set_pixel(x,y,rgb_pixel(r,g,b));
}

//
// class  ImageWriter16bit
//

void ImageWriter16bit::writePixel(int x, int y, ofstream& out,
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
    } else out<<i;
    if(outImage) outImage->set_pixel(x,y,rgb_pixel(r,g,b));
}

//
// class  ImageWriter18bit
//

void ImageWriter18bit::writePixel(int x, int y, ofstream& out,
        png::image<rgb_pixel> *outImage, rgb_pixel pix)
{
    throw(runtime_error("Implement me"));
//     r=pix.red & 0xfc;
//     g=pix.green & 0xfc;
//     b=pix.blue & 0xfc;
//     file<<(r>>2)<<','<<(g>>2)<<','<<(b>>2);
//     if(outRequested) outImage.set_pixel(x,y,rgb_pixel(r,g,b));
}

//
// class  ImageWriter24bit
//

void ImageWriter24bit::writePixel(int x, int y, ofstream& out,
        png::image<rgb_pixel> *outImage, rgb_pixel pix)
{
    throw(runtime_error("Implement me"));
//     file<<(int)(pix.red)<<','
//         <<(int)(pix.green)<<','
//         <<(int)(pix.blue);
//     if(outRequested) outImage.set_pixel(x,y,pix);
}

int main(int argc, char *argv[])
{
    //Check args
    options_description desc("PngConverter utility v1.22\n"
        "Designed by TFT : Terraneo Federico Technologies\nOptions");
    desc.add_options()
        ("help", "Prints this.")
        ("in", value<string>(), "Input png file (required)")
        ("depth", value<string>(), "Color depth, 1bitlinear,8,16,18 or 24 bits (required)")
        ("out", value<string>(), "Output png file for validation")
        ("binary", "Generate a binary file instead of a .cpp/.h file")
    ;

    variables_map vm;
    store(parse_command_line(argc,argv,desc),vm);
    notify(vm);

    if(vm.count("help") || (!vm.count("in")) || (!vm.count("depth")))
    {
        cerr<<desc<<endl;
        return 1;
    }
    
    //Load image
    image<rgb_pixel> img(vm["in"].as<string>());
    cout<<"Loaded image \""<<vm["in"].as<string>()<<"\". Info:"<<endl;
    cout<<"Height  = "<<img.get_height()<<endl;
    cout<<"Width   = "<<img.get_width()<<endl;

    string depth=vm["depth"].as<string>();
    PixDepth pixDepth;
    int pixDepthInt=0;
    if(depth=="1bitlinear")
    {
        pixDepth=_1bitlinear;
        pixDepthInt=1 | 0x80; //Bit #0=1bit, bit #16=linear
    } else if(depth=="8")
    {
        pixDepth=_8;
        pixDepthInt=8;
    } else if(depth=="16")
    {
        pixDepth=_16;
        pixDepthInt=16;
    } else if(depth=="18")
    {
        pixDepth=_16;
        pixDepthInt=18;
    } else if(depth=="24")
    {
        pixDepth=_24;
        pixDepthInt=24;
    } else
    throw runtime_error("Unsupported pixel depth (not 1bitlinear,8,16,18,24)");

    /*
     * Get output filemane from input filename
     * Example: if in is "/home/mypng.png"
     * filename        is "mypng"
     * path            is "/home/"
     * cppFilename     is "/home/mypng.cpp"
     * hFilename       is "/home/mypng.h"
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
        file<<endl<<"//This file has been automatcally generated by "
                "pngconverter utility"<<endl<<"//Please do not edit"<<endl
                <<"#include \""<<filename<<".h\""<<endl<<endl
                <<"using namespace mxgui;"<<endl<<endl
                <<"static const short int height="<<img.get_height()<<';'<<endl
                <<"static const short int width ="<<img.get_width()<<';'<<endl
                <<endl;
        //Optimization for 16 bit per pixel
        if(pixDepth==_16) 
            file<<"static const unsigned short pixelData[]={"<<endl<<' ';
        else file<<"static const unsigned char pixelData[]={"<<endl<<' ';
    } else {
        unsigned short header[3];
        header[0]=toLittleEndian((unsigned short)img.get_height());
        header[1]=toLittleEndian((unsigned short)img.get_width());
        header[2]=toLittleEndian((unsigned short)pixDepthInt);
        file.write(reinterpret_cast<char*>(&header),sizeof(header));
    }

    shared_ptr<ImageWriter> imgw=ImageWriter::fromPixDepth(img,binary,pixDepth);
    imgw->write(file, outRequested ? &outImage : 0);

    if(!binary)
    {
        // The image is declared simply as "Image" in the .h, while as
        // "basic_image<type>" in the .cpp. This is a trick to allow a
        // compile time check that the image pixel depth is correct.
        // If both the image and the mxgui configurations agree the file
        // will compile. Also, using "Image" in the .h allows to include
        // heaer files that refer to "optional" images without causing
        // compiler errors
        string classname;
        switch(pixDepth)
        {
            case _1bitlinear:
                classname="Color1bitlinear";
                break;
            case _8:
                classname="unsigned char";
                break;
            case _16:
                classname="unsigned short";
                break;
            case _18:
                throw runtime_error("TODO");
                break;
            case _24:
                throw runtime_error("TODO");
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
