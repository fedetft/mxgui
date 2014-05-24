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

#include "libs/png++/png.hpp"
#include "font_core.h"
#include "bdfparser.h"
#include "ttfparser.h"
#include "fixed_width_generator.h"
#include "variable_width_generator.h"

using namespace std;
using namespace png;
using namespace boost;

namespace fontcore {

//
// Utility functions
//

string toUpper(const string& x)
{
    string result(x);
    for(int i=0;i<result.length();i++) result[i]=toupper(result[i]);
    return result;
}

//
// Class Glyph
//

Glyph::Glyph(): width(0), data(), ASCIIValue(0), antialiased(false) {}

void Glyph::setWidth(unsigned int width)
{
    this->width=width;
}

void Glyph::setData(const vector<bitset<maxWidth> >& data)
{
    this->data=data;
}

void Glyph::setASCII(unsigned char value)
{
    this->ASCIIValue=value;
}

void Glyph::setAntialiased(bool antialiased)
{
    this->antialiased=antialiased;
}

unsigned int Glyph::getWidth() const
{
    return width;
}

unsigned int Glyph::getHeight() const
{
    return data.size();
}

unsigned char Glyph::getPixelAt(int x, int y) const
{
    if(antialiased==false) return data.at(y).test(x) ? 1 : 0;
    unsigned char result=0;
    if(data.at(y).test(2*x)) result=0x1;
    if(data.at(y).test(2*x+1)) result |= 0x2;
    return result;

}

unsigned char Glyph::getASCII() const
{
    return ASCIIValue;
}

bool Glyph::isAntialiased() const
{
    return antialiased;
}

bool operator< (Glyph a, Glyph b)
{
    return a.getASCII() < b.getASCII();
}

//
// Class FontParser
//

shared_ptr<FontParser> FontParser::getParser(const string& filename)
{
    int dot=filename.find_last_of(".");
    if(dot==string::npos)
        throw(runtime_error("Font file name missing extension"));
    string ext=filename.substr(dot);
    if(ext==".bdf") return shared_ptr<FontParser>(new BDFParser(filename));
    if(ext==".ttf") return shared_ptr<FontParser>(new TTFParser(filename));
    throw(runtime_error(string("Unknown font file extension: ")+ext));
}

FontParser::FontParser(const string& filename): logStream(0),
        log(false), fonts(), startConvert(32), endConvert(126),
        filename(filename), fixesFile(""), ttfHeight(0), ttfPadding(0) {}

void FontParser::setLogStream(std::ostream& output)
{
    this->logStream=&output;
    this->log=true;
}

void FontParser::setConversionRange(unsigned char start, unsigned char end)
{
    this->startConvert=start;
    this->endConvert=end;
}

std::vector<Glyph> FontParser::getGlyphs() const
{
    return fonts;
}

void FontParser::setTTFHeight(unsigned int height)
{
    this->ttfHeight=height;
}

void FontParser::setTTFPadding(unsigned int padding)
{
    this->ttfPadding=padding;
}

void FontParser::setFixesFile(std::string file)
{
    this->fixesFile=file;
}

FontParser::~FontParser() {}

//
// Class CodeGenerator
//

shared_ptr<CodeGenerator>
        CodeGenerator::getGenerator(shared_ptr<FontParser> parser)
{
    vector<Glyph> glyphs=parser->getGlyphs();
    if(glyphs.size()==0) throw(runtime_error("Parser has no glyphs"));

    //Check if the font is fixed width or not, code generation differs greatly
    //depending on this
    unsigned int width=glyphs.at(0).getWidth();
    unsigned int height=glyphs.at(0).getHeight();
    bool fixedWidth=true;
    for(int i=1;i<glyphs.size();i++)
    {
        if(glyphs.at(i).getHeight()!=height) throw(runtime_error(
                "Font has variable height. This is unsupported"));
        if(glyphs.at(i).getWidth()!=width) fixedWidth=false;
    }
    if(fixedWidth)
        return shared_ptr<CodeGenerator>(new FixedWidthGenerator(parser));
    return shared_ptr<CodeGenerator>(new VariableWidthGenerator(parser));
}

CodeGenerator::CodeGenerator(shared_ptr<FontParser> parser): logStream(),
        log(false), glyphs(parser->getGlyphs()) {}

void CodeGenerator::setLogStream(std::ostream& output)
{
    this->logStream=&output;
    this->log=true;
}

void CodeGenerator::generateRendering(const std::string& filename) const
{
    //Calculate image size
    int pngWidth=0;
    int pngHehight=glyphs.at(0).getHeight();
    for(int i=0;i<glyphs.size();i++) pngWidth+=glyphs.at(i).getWidth()+1;
    if(pngWidth==0 || pngHehight==0)
        throw(runtime_error("Error: height or width of image is zero"));
    
    if(log) *logStream<<"PNG: height="<<pngHehight<<" width="<<
            pngWidth<<endl;
    if(log) logStream->flush();
    image<rgb_pixel> img(pngWidth,pngHehight);

    const rgb_pixel background(255,255,255);
    const rgb_pixel foreground(0,0,0);
    const rgb_pixel separator(255,255,0);

    int xstart=0;//start of a character
    for(int i=0;i<glyphs.size();i++)
    {
        Glyph current=glyphs.at(i);
        bool aa=current.isAntialiased();
        for(int j=0;j<pngHehight;j++)
        {
            for(int k=0;k<current.getWidth();k++)
            {
                if(aa==false)
                {
                    if(current.getPixelAt(k,j))
                        img.set_pixel(xstart+k,j,foreground);
                    else img.set_pixel(xstart+k,j,background);
                } else {
                    static const unsigned char values[]={0xff, 0xaa, 0x55, 0x0};
                    unsigned char color=values[current.getPixelAt(k,j)];
                    img.set_pixel(xstart+k,j,rgb_pixel(color,color,color));
                }
            }
        }
        for(int j=0;j<pngHehight;j++)
        {
            img.set_pixel(xstart+current.getWidth(),j,separator);
        }
        xstart+=current.getWidth()+1;
    }

    img.write(filename);
}

CodeGenerator::~CodeGenerator() {}

} //namespace fontcore
