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

#include "fixed_width_generator.h"

using namespace std;

namespace fontcore {

//
// class FixedWidthGenerator
//

FixedWidthGenerator::FixedWidthGenerator(
        shared_ptr<FontParser> parser): CodeGenerator(parser) {}

void FixedWidthGenerator::generateCode(const std::string filename,
        const std::string& fontName)
{
    if(log) *logStream<<"Font is fixed width"<<endl;
    unsigned int height=glyphs.at(0).getHeight();
    unsigned int width=glyphs.at(0).getWidth();
    const bool aa=glyphs.at(0).isAntialiased();
    if(aa && log) *logStream<<"Font is antialiased"<<endl;

    file.open(filename.c_str());
    file<<showbase;//So that it will print 0x in front of hexadecimal numbers
    //Write header part
    file<<"#ifndef FONT_"<<toUpper(fontName)<<"_H\n"<<
          "#define FONT_"<<toUpper(fontName)<<"_H\n"<<
          "//This font has been converted from BDF/TTF font format to .h\n"<<
          "//using fontrendering utility written by Terraneo Federico.\n"<<
          "//Do not modify this file, it has been automatically generated.\n\n";

    //Calculate how many bits are necessary to store a row of a glyph
    int roundedHeight;//Height rounded to 8, 16 or 32
    if(height<=8) roundedHeight=8;
    else if(height<=16) roundedHeight=16;
    else if(height<=32) roundedHeight=32;
    else  throw(runtime_error("Character is too high for code generation."
                " Maximum allowed is 32bit"));
    if(aa) roundedHeight*=2;
    
    //Write font info data
    file<<"const bool "<<fontName<<"IsAntialiased="<<(aa?"true;\n":"false;\n")<<
          "const bool "<<fontName<<"IsFixedWidth=true;\n"<<
          "const unsigned char "<<fontName<<"Height="<<height<<";\n"<<
          "const unsigned char "<<fontName<<"Width="<<width<<";\n"<<
          "const unsigned char "<<fontName<<"DataSize="<<roundedHeight<<";\n\n";

	//Write range array
	std::vector<UnicodeBlock> blocks = UnicodeBlockManager::getAvailableBlocks();
	file<<"const unsigned char "<<fontName<<"""NumBlocks="<<blocks.size()<<";\n";
	file<<"// The start of range i is blocks[2*i], its size is at blocks[2*i+1]\n";
	file<<"const unsigned int "<<fontName<<"Blocks[]{\n";
	for(int i=0;i<blocks.size();i++)
	{
		UnicodeBlock block = blocks[i];
		file<<hex<<block.getStartCodepoint()<<","<<block.getEndCodepoint();
		if(i != blocks.size()-1)
			file<<",\n";
	}
	file<<"\n};\n\n"<<dec;
	
    //Write font look up table
    switch(roundedHeight)
    {
        case 8:
            file<<"const unsigned char "<<fontName<<"Data[]["<<width<<"]={\n";
            break;
        case 16:
            file<<"const unsigned short "<<fontName<<"Data[]["<<width<<"]={\n";
            break;
        case 32:
            file<<"const unsigned int "<<fontName<<"Data[]["<<width<<"]={\n";
            break;
        case 64:
            file<<"const unsigned long long "<<fontName<<"Data[]["<<width<<
                    "]={\n";
            break;

    }
    for(int i=0;i<glyphs.size();i++)
    {
        Glyph glyph=glyphs.at(i);
        file<<" { //U+"<<static_cast<int>(glyph.getCodepoint())<<" ( "<<
                glyph.getCodepoint()<<" )\n  ";
        for(int j=0;j<width;j++)
        {
            unsigned long long column=0;
            for(int k=0;k<height;k++)
            {
                if(aa==false)
                {
                    //if(glyph.getPixelAt(j,k)) column|=1ull<<(roundedHeight-1-k);
                    if(glyph.getPixelAt(j,k)) column|=1ull<<k;
                } else {
                    unsigned char pixel=glyph.getPixelAt(j,k);
                    //if(pixel & 2) column|=1ull<<(roundedHeight-1-2*k);
                    //if(pixel & 1) column|=1ull<<(roundedHeight-1-2*k-1);
                    if(pixel & 2) column|=1ull<<2*k+1;
                    if(pixel & 1) column|=1ull<<2*k;
                }
            }
            if(j!=width-1)
                file<<hex<<column<<dec<<(roundedHeight==64?"ull":"")<<",";
            else file<<hex<<column<<dec<<(roundedHeight==64?"ull":"");
        }
        if(i!=glyphs.size()-1) file<<"\n },\n"; else file<<"\n }\n";
    }

    file<<"};\n";

    //Write last part
    file<<"\n#endif //FONT_"<<toUpper(fontName)<<"_H\n";
    file.close();
}

} //namepace fontcore

