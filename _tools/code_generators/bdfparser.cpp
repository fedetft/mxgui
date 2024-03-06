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

#include "bdfparser.h"
#include "unicode_blocks.h"
#include <sstream>
#include <cassert>
#include <cctype>
#include <algorithm>

using namespace std;

namespace fontcore {

//
// Class BDFParser
//

BDFParser::BDFParser(const string& filename): FontParser(filename),
        fontAscent(0), fontDescent(0) {}

void BDFParser::parse()
{
    //Check that no fixes file has been specified, since it is specific to TTF
    if(this->fixesFile.empty()==false && log)
    {
        (*logStream)<<"Warning: ignoring fixes file since unsupported for BDF"
                <<endl;
    }
    //Clear results of a previous parse
    fonts.clear();
    //Start a new one
    ifstream file;
    file.exceptions(ifstream::eofbit | ifstream::failbit | ifstream::badbit);
    try {
        file.open(filename.c_str());
    } catch(ifstream::failure& e)
    {
        stringstream ss;
        ss<<"Failed to open file"<<endl;
        throw(runtime_error(ss.str()));
    }
    try {
        bool ascentFound=false, descentFound=false;
        for(;;)
        {
            string line;
            getline(file,line);
            if(line.substr(0,11)=="FONT_ASCENT")
            {
                stringstream ss(line.substr(12));
                ss>>fontAscent;
                ascentFound=true;
            }
            if(line.substr(0,12)=="FONT_DESCENT")
            {
                stringstream ss(line.substr(13));
                ss>>fontDescent;
                descentFound=true;
            }
            if(ascentFound && descentFound) break;
        }
    } catch(ifstream::failure& e)
    {
        stringstream ss;
        ss<<"Failed to find FONT_ASCENT or FONT_DESCENT tags"<<endl;
        throw(runtime_error(ss.str()));
    }
    if(fontAscent<0 || fontDescent<0)
    {
        stringstream ss;
        ss<<"Error: fontAscent or fontDescent invalid"<<endl;
        throw(runtime_error(ss.str()));
    }
    for(;;)
    {
        try {
            vector<string> result=getNextChar(file);
            generateGlyph(result);

        } catch(ifstream::failure& e)
        {
            //Eof found, sort characters and return
            sort(fonts.begin(),fonts.end());
			unsigned int supportedCharacters =
				UnicodeBlockManager::numSupportedCharacters();
            if(fonts.size()<(supportedCharacters) && log)
            {
                *logStream<<"Warning converted only "<<fonts.size()<<
                        " characters instead of "<<supportedCharacters<<
                        " Some are missing from the bdf file"<<endl;
            }
            return;
        }
    }
}

unsigned int BDFParser::getHeight() const
{
    return fontAscent+fontDescent;
}

vector<string> BDFParser::getNextChar(ifstream& file)
{
    vector<string> result;
    bool start=false;
    for(;;)
    {
        string line;
        getline(file,line);
        if(line.substr(0,9)=="STARTCHAR") start=true;
        else if(line=="ENDCHAR") return result;
        else if(start) result.push_back(line);
    }
}

void BDFParser::generateGlyph(vector<string> data)
{
    Glyph result;
	
    //Look for the ENCODING tag
    for(int i=0;i<data.size();i++)
    {
        if(data.at(i).substr(0,8)=="ENCODING")
        {
            stringstream ss(data.at(i).substr(9));
            int encoding;
            ss>>encoding;
            if(!UnicodeBlockManager::isCharacterSupported(encoding)) return;
            result.setCodepoint(static_cast<char32_t>(encoding));
            break;
        }
    }
    if(log) *logStream<<showbase<<hex<<"Parsing glyph "<<static_cast<int>(result.getCodepoint())<<
            " ("<<result.getCodepoint()<<")...";

    //Look for the DWIDTH tag
    bool dwidthFound=false;
    for(int i=0;i<data.size();i++)
    {
        if(data.at(i).substr(0,6)=="DWIDTH")
        {
            dwidthFound=true;
            stringstream ss(data.at(i).substr(7));
            int dwidth;
            ss>>dwidth;
            if(dwidth<=0 || dwidth>Glyph::maxWidth)
            {
                stringstream ss;
                ss<<"Error: Glyph "<<result.getCodepoint()<<
					" has invalid DWIDTH ("<<dec<<dwidth<<")";
                throw(runtime_error(ss.str()));
            }
            result.setWidth(dwidth);
            break;
        }
    }
    if(dwidthFound==false)
    {
        stringstream ss;
        ss<<"Error: Glyph "<<hex<<result.getCodepoint()<<" has no DWIDTH tag";
        throw(runtime_error(ss.str()));
    }

    //Get the bounding box
    bool bbxFound=false;
    int bbxA, bbxB, bbxC, bbxD;
    for(int i=0;i<data.size();i++)
    {
        if(data.at(i).substr(0,3)=="BBX")
        {
            bbxFound=true;
            stringstream ss(data.at(i).substr(4));
            ss>>bbxA>>bbxB>>bbxC>>bbxD;
            break;
        }
    }
    if(bbxFound==false)
    {
        stringstream ss;
        ss<<"Error: Glyph "<<result.getCodepoint()<<" has no BBX tag";
        throw(runtime_error(ss.str()));
    }

    //Some check on the bbx data
    if(bbxA<0 || bbxB<0)
    {
        stringstream ss;
        ss<<"Error: Glyph "<<result.getCodepoint()<<" has bbxA or bbxB<0";
        throw(runtime_error(ss.str()));
    }

    //Now build the bitmap
    vector<bitset<Glyph::maxWidth> > theBitmap;
    //First, see if we need to add padding as required by the bounding box
    if(fontAscent-bbxB-bbxD<0)
    {
        stringstream ss;
        ss<<"Error: Glyph "<<showbase<<hex<<result.getCodepoint()<<
            " has bad alignment. This might be caused by an insufficent "
            "fontAscent for the boundingBox of this character";
        throw(runtime_error(ss.str()));
    }
    for(int i=0;i<fontAscent-bbxB-bbxD;i++)
    {
        theBitmap.push_back(bitset<Glyph::maxWidth>(0));
    }

    //Now, look for the BITMAP tag and add data
    for(int i=0;i<data.size();i++)
    {
        if(data.at(i).substr(0,6)=="BITMAP")
        {
            for(int j=i+1;j<data.size();j++)
            {
                stringstream ss(data.at(j));
                unsigned int bitmapLine;
                ss>>hex>>bitmapLine;
                bitset<Glyph::maxWidth> mirrorInput(bitmapLine);
                bitset<Glyph::maxWidth> mirrorOutput;
                unsigned numBits;//Number of bits of the padding (8, 16, 24, 32)
                numBits=bbxA;
                if(numBits<32 && numBits>24) numBits=32;
                if(numBits<24 && numBits>16) numBits=24;
                if(numBits<16 && numBits>8) numBits=16;
                if(numBits<8) numBits=8;
                //Bits are in reverse order, they need to be mirrored
                for(int k=0;k<min(result.getWidth(),numBits);k++)
                {
                    bool bit=mirrorInput[numBits-1-k];
                    if(k+bbxC>=0) mirrorOutput[k+bbxC]=bit;
                }
                theBitmap.push_back(mirrorOutput);
            }
            break;
        }
    }

    //Add padding if required
    int remaining=getHeight()-theBitmap.size();
    while(remaining>0)
    {
        theBitmap.push_back(bitset<Glyph::maxWidth>(0));
        remaining=getHeight()-theBitmap.size();
    }
    
    //Check if something went wrong
    if(theBitmap.size()>getHeight())
    {
        stringstream ss;
        ss<<"Error: Glyph "<<result.getCodepoint()<<
            " has a bigger height ("<<dec<<theBitmap.size()<<") than the "
            "maximum allowed. This might be caused by an insufficient "
            "fontAscent or fontDescent to fit the boundingBox";
        throw(runtime_error(ss.str()));
    }
    result.setData(theBitmap);
    fonts.push_back(result);
    if(log) *logStream<<"Done"<<endl;
}

}//namespace bdfcore
