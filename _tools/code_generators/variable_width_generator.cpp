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

#include "variable_width_generator.h"
#include "unicode_blocks.h"

using namespace std;

namespace fontcore {

//
// class VariableWidthGenerator
//

VariableWidthGenerator::VariableWidthGenerator(
        shared_ptr<FontParser> parser): CodeGenerator(parser) {}

void VariableWidthGenerator::generateCode(const std::string filename,
        const std::string& fontName)
{
    if(log) *logStream<<"Font is variable width"<<endl;
    unsigned int height=glyphs.at(0).getHeight();
    const bool aa=glyphs.at(0).isAntialiased();
    if(aa && log) *logStream<<"Font is antialiased"<<endl;
    
    ofstream file(filename.c_str());
    file<<showbase;//So that it will print 0x in front of hexadecimal numbers
    //Write header part
    file<<"#pragma once\n"<<
          "//This font has been converted from BDF/TTF font format to .h\n"<<
          "//using the fontrendering utility which is part of mxgui,\n"<<
          "//the graphics library of the Miosix kernel.\n"<<
          "//Do not modify this file, it has been automatically generated.\n\n";

    //Calculate how many bits are necessary to store a row of a glyph
    int roundedHeight;//Height rounded to 8, 16 or 32
    if(height<=8) roundedHeight=8;
    else if(height<=16) roundedHeight=16;
    else if(height<=32) roundedHeight=32;
    else throw(runtime_error("Character is too high for code generation."
                " Maximum allowed is 32pixels"));
    if(aa) roundedHeight*=2;

    //Write font info data
    std::vector<UnicodeBlock> blocks = UnicodeBlockManager::getAvailableBlocks();
    file<<"const bool "<<fontName<<"IsAntialiased="<<(aa?"true;\n":"false;\n")<<
          "const bool "<<fontName<<"IsFixedWidth=false;\n"<<
          "const unsigned char "<<fontName<<"Height="<<height<<";\n"<<
          "const unsigned char "<<fontName<<"DataSize="<<roundedHeight<<";\n"<<
          "const unsigned char "<<fontName<<"""NumBlocks="<<blocks.size()<<";\n\n";

    //Write range array
    file<<"// The start of range i is blocks[2*i], its size is at blocks[2*i+1]\n";
    file<<"const unsigned int "<<fontName<<"Blocks[]{\n";
    for(int i=0;i<blocks.size();i++)
    {
        UnicodeBlock block = blocks[i];
        unsigned int rangeLen = block.getEndCodepoint()-block.getStartCodepoint()+1;
        file<<" ";
        file<<hex<<block.getStartCodepoint()<<","<<rangeLen;
        if(i != blocks.size()-1)
            file<<",\n";
    }
    file<<"\n};\n\n"<<dec;

    //Write width look up table
    file<<"//The width of character i is "<<fontName<<"Width[i]\n";
    file<<"const unsigned char "<<fontName<<"Width[]={\n ";
    int widthNewline=0;
    for(int i=0;i<glyphs.size();i++)
    {
        if(i!=glyphs.size()-1)
        {
            file<<glyphs.at(i).getWidth()<<",";
            if(++widthNewline==8) { widthNewline=0; file<<"\n "; }
        } else {
            file<<glyphs.at(i).getWidth();
        }
    }
    file<<"\n};\n\n";

    //Write offsets look up table
    file<<"//The first byte of character i is "<<fontName<<"Data["<<
            fontName<<"Offset[i]]\n";
    
    file<<"const unsigned short "<<fontName<<"Offset[]={\n ";
    int offsetNewline=0;
    int offsetCalculated=0;
    for(int i=0;i<glyphs.size();i++)
    {
        file<<offsetCalculated;
        offsetCalculated+=glyphs.at(i).getWidth();
        if(i!=glyphs.size()-1)
        {
            file<<",";
            if(++offsetNewline==8) { offsetNewline=0; file<<"\n "; }
        }
    }
    file<<"\n};\n\n";

    //Write font look up table
    switch(roundedHeight)
    {
        case 8:
            file<<"const unsigned char "<<fontName<<"Data[]={\n";
            break;
        case 16:
            file<<"const unsigned short "<<fontName<<"Data[]={\n";
            break;
        case 32:
            file<<"const unsigned int "<<fontName<<"Data[]={\n";
            break;
        case 64:
            file<<"const unsigned long long "<<fontName<<"Data[]={\n";
            break;

    }
    for(int i=0;i<glyphs.size();i++)
    {
        file<<" ";
        Glyph glyph=glyphs.at(i);
        for(int j=0;j<glyph.getWidth();j++)
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
            if(j==glyph.getWidth()-1 && i==glyphs.size()-1)
            {
                file<<showbase<<hex<<column<<dec<<(roundedHeight==64?"ull":"");
            }
            else file<<showbase<<hex<<column<<dec<<(roundedHeight==64?"ull":"")<<",";
        }
        file<<" //U+"<<noshowbase<<hex<<static_cast<int>(glyph.getCodepoint())<<" ( "
            <<UnicodeBlockManager::codepointToString(glyph.getCodepoint())<<" )";
        if(i!=glyphs.size()-1) file<<"\n";
    }

    file<<"\n};\n";
}

} //namespace fontcore
