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

#include "libs/png++/png.hpp"
#include "ttfparser.h"
#include "unicode_blocks.h"
#include <cmath>

using namespace std;

namespace fontcore {

//
// class TTFParser
//

TTFParser::TTFParser(const std::string& filename): FontParser(filename) {}

void TTFParser::parse()
{
    if(this->fixesFile.empty()==false)
    {
        fixes.parse(this->fixesFile);
    }
    //Clear results of a previous parse
    fonts.clear();
    //Start a new one
    if(ttfHeight<4 || ttfHeight>24)
        throw(runtime_error("Invalid rendering height"));

    FT_Library library;
    if(FT_Init_FreeType(&library))
        throw(logic_error("Failed initializing FreeType"));
    FT_Face face;
	int error=FT_New_Face(library,filename.c_str(),0,&face);
    if(error) throw(logic_error("FreeType failed opening font file"));
    error=FT_Set_Pixel_Sizes(face,0,ttfHeight);
	if(error) throw(logic_error("Couldn't set font height"));
	
    //Compute font ascent and descent as the maximum for all glyphs to render
    ascent=0;
	descent=0;
	for(auto& block : this->blocks)
	{
		for(unsigned int k=block.getStartCodepoint();k<=block.getEndCodepoint();k++)
		{
			error=FT_Load_Char(face,k,FT_LOAD_RENDER);
			if(error)
				throw(logic_error(string("Encountered an error rendering: ")+
								  static_cast<char>(k)));
			int topBearing=face->glyph->metrics.horiBearingY/64;
			if(topBearing>ascent) ascent=topBearing;
			int bottomBearing=face->glyph->metrics.height/64-
				face->glyph->metrics.horiBearingY/64;
			if(bottomBearing>descent) descent=bottomBearing;
		}
	}
	
    //Height can be higher than the one given for rendering, since glyphs
    //are allowed to exceed their bounding boxes *sigh*
    realHeight=ascent+descent;
	cout<<"ascent="<<ascent<<" descent="<<descent<<
			" height="<<realHeight<<endl;
    if(realHeight<ttfHeight) throw(runtime_error("Unexpected"));

	for(auto& block : this->blocks)
	{
		for(unsigned int chr=block.getStartCodepoint();chr<=block.getEndCodepoint();chr++)
		{
			error=FT_Load_Char(face,chr,FT_LOAD_RENDER);
			if(error)
				throw(logic_error(string("Encountered an error rendering: ")+
								  static_cast<char>(chr)));
			generateGlyph(face,chr);
		}
	}
    FT_Done_FreeType(library);
}

void TTFParser::generateGlyph(const FT_Face& face, char32_t chr)
{
    /*cout<<static_cast<char>(chr);
    cout<<" h"<<face->glyph->metrics.height/64;
    cout<<" w"<<face->glyph->metrics.width/64;
    cout<<" ha"<<face->glyph->metrics.horiAdvance/64;
    cout<<" va"<<face->glyph->metrics.vertAdvance/64;
    cout<<" hbx"<<face->glyph->metrics.horiBearingX/64;
    cout<<" hby"<<face->glyph->metrics.horiBearingY/64<<endl;*/

    if(log) *logStream<<"Rendering glyph "<<showbase<<hex<<static_cast<int>(chr)<<" ("
			<<dec<<chr<<")...";

    //First, extract relevant data from the FreeType glyph format
    const unsigned char *buffer=face->glyph->bitmap.buffer;
    const int x=face->glyph->metrics.width/64;
    const int y=face->glyph->metrics.height/64;
    const int currentAscent=face->glyph->metrics.horiBearingY/64;
	const int xBearing=face->glyph->metrics.horiBearingX/64;
    int advance=face->glyph->metrics.horiAdvance/64;

    if(x>advance) advance=x; //FIX for something weird
	if(xBearing<0) advance+=xBearing;
    if(advance>Glyph::maxWidth/2)
        throw(runtime_error("Glyphs with width larger than 16 pixels not"
                " supported"));

    //Then, make a Glyph object and set initial properties
    Glyph result;
    result.setCodepoint(chr);
    result.setAntialiased(true);
    result.setWidth(advance+fixes.getPad(chr)-fixes.getShift(chr));

    //Initial padding
    vector<bitset<Glyph::maxWidth> > theBitmap;
    for(int i=0;i<realHeight-(currentAscent+descent);i++)
        theBitmap.push_back(bitset<Glyph::maxWidth>(0));

    //Real glyph data
    for(int i=0;i<y;i++)
    {
        bitset<Glyph::maxWidth> line(0);
        for(int j=0;j<x;j++)
        {
            unsigned char color=buffer[j+x*i];
			float contrast=fixes.getContrast(chr);
			if(contrast!=1.0f)
			    color=255*pow(static_cast<float>(color)/255.0f,contrast);
			if(j+xBearing<0) continue;
            line[2*(j+xBearing)]=(color & 0x40) ? 1 : 0;
            line[2*(j+xBearing)+1]=(color & 0x80) ? 1 : 0;
        }
        line>>=2*fixes.getShift(chr);
        theBitmap.push_back(line);
    }

    //Final padding
    int remaining=realHeight-theBitmap.size();
    if(remaining<0) throw(runtime_error("This should not happen"));
    for(int i=0;i<remaining;i++)
        theBitmap.push_back(bitset<Glyph::maxWidth>(0));

    //Additional user-requested padding
    for(int i=0;i<this->ttfPadding;i++)
        theBitmap.push_back(bitset<Glyph::maxWidth>(0));

    result.setData(theBitmap);
    fonts.push_back(result);
    if(log) *logStream<<"Done"<<endl;
}

} //namespace fontcore

