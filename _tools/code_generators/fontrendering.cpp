/***************************************************************************
 *   Copyright (C) 2010 by Terraneo Federico                               *
 *   Copyright (C) 2024 by Terraneo Federico and Nicolas Benatti           *
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
 * fontrendering.cpp
 * Renders a BDF or TTF font and generates C++ code suitable to be used in
 * microcontrollers, especially by the mxgui user interface library.
 *
 * The source code is vaguely organized as a compiler, with:
 * - a frontend that does parsing (for BDF files, since they are text-based) or
 *   rendering (for TrueType files, using FreeType)
 * - an intermediate representation consisting in a vector of Glyph classes,
 *   that is printed into a png image for debugging/validation purposes
 * - a backend, which performs C++ code generation
 *
 * The code is made more complicated by the large number of features modern
 * font formats provide, such as the possibility for a font to exceed its
 * bounding box, the possibility of moving the "drawing pen" backwards when
 * drawing the next char, and the kerning feature that allows to specify an
 * ad-hoc distance for special character combination.
 * The output format is much more simple, supporting none of these features,
 * despite it supports both fixed and variable width glyphs as well as two bits
 * per pixel antialiasing.
 */

#include "font_core.h"
#include "unicode_blocks.h"
#include <algorithm>
#include <iostream>
#include <boost/program_options.hpp>
#include <stdexcept>

using namespace std;
using namespace fontcore;
using namespace boost::program_options;

int main(int argc, char *argv[])
{
    options_description desc("FontRendering utility v1.2\n"
        "Designed by TFT : Terraneo Federico Technologies\nOptions");
    desc.add_options()
        ("help", "Prints this.")
        ("font", value<string>(), "A BDF or TTF font file. Must exist")
        ("add-range", value<vector<string>>(), "Add a range of Unicode codepoints")
        ("replacement-char", value<string>(), "Specify a replacement character different from '�'")
        ("image", value<string>(), "Filename of image to be generated")
        ("header", value<string>(), "Filename of .h file to be generated")
        ("name", value<string>(), "Font name, to give a name to the tables")
        ("height", value<int>(), "Rendering height (for TrueType only)")
        ("pad", value<int>(), "Additional pixels padding (for TrueType only)")
        ("fixes", value<string>(), "Fixes file for kerning issues (TTF only)")
    ;

    variables_map vm;
    store(parse_command_line(argc,argv,desc),vm);
    notify(vm);

    if(vm.empty() || vm.count("help") || (!vm.count("font")) ||
       (!vm.count("image")) || (!vm.count("header")) || (!vm.count("name")))
    {
        cerr<<desc<<endl;
        return 1;
    }

    shared_ptr<FontParser> parser=FontParser::getParser(vm["font"].as<string>());
    parser->setLogStream(cout);

    if(vm.count("height"))
    {
        int height=vm["height"].as<int>();
        if(height<4 || height>24)
            throw(runtime_error("Invalid height, range is from 4 to 24"));
        parser->setTTFHeight(height);
    }

    if(vm.count("pad"))
    {
        int padding=vm["pad"].as<int>();
        if(padding<0 || padding>5)
            throw(runtime_error("Invalid padding, range is from 0 to 5"));
        parser->setTTFPadding(padding);
    }

    if(vm.count("fixes"))
    {
        parser->setFixesFile(vm["fixes"].as<string>());
    }

    if(vm.count("add-range"))
    {
        // convert the list into a an array of pairs and give it to the Manager
        vector<string> rangeList=vm["add-range"].as<vector<string>>();
        vector<pair<char32_t,char32_t>> ranges;
        unsigned int start,end;

        for(string s : rangeList)
        {
            try {
                const int autodetectBase=0;
                start=stoi(s.substr(0,s.find(",")),nullptr,autodetectBase);
                end=stoi(s.substr(s.find(",")+1,s.size()),nullptr,autodetectBase);
            } catch(invalid_argument& e)
            {
                throw(runtime_error("Wrongly formatted ranges! Expected \"<start>,<end>\""));
            }

            if(start>end)
                throw(runtime_error("Start of range is greater than end of range"));

            ranges.push_back({start,end});
        }

        // it's important for the ranges to be sorted!
        sort(ranges.begin(),ranges.end());

        // add the replacement character (it's always the last range,
        // regardless of the ordering)
        ranges.push_back({0xFFFD,0xFFFD});

        UnicodeBlockManager::updateBlocks(ranges);
        parser->setUnicodeBlocks(UnicodeBlockManager::getAvailableBlocks());
    }

    if(vm.count("replacement-char"))
    {
        string s=vm["replacement-char"].as<string>();
        if(s.size()>1)
            throw(runtime_error("The replacement character should not be a string"));

        char32_t replacementCodepoint=static_cast<char32_t>(s[0]);
        UnicodeBlockManager::updateReplacementCharacter(replacementCodepoint);
        parser->setUnicodeBlocks(UnicodeBlockManager::getAvailableBlocks());
    }

    parser->parse();
    shared_ptr<CodeGenerator> generator=CodeGenerator::getGenerator(parser);
    generator->setLogStream(cout);
    generator->generateRendering(vm["image"].as<string>());
    generator->generateCode(vm["header"].as<string>(),vm["name"].as<string>());
    cout<<"Success"<<endl;
    return 0;
}
