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
#include <iostream>
#include <sstream>
#include <boost/program_options.hpp>

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
        ("image", value<string>(), "Filename of image to be generated")
        ("header", value<string>(), "Filename of .h file to be generated")
        ("name", value<string>(), "Font name, to give a name to the tables")
        ("height", value<int>(), "Rendering height (for TrueType only)")
        ("pad", value<int>(), "Additional pixels padding (for TrueType only)")
        ("range", value<string>(), "Overrides default of converting only ASCII")
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

    if(vm.count("range"))
    {
        stringstream ss(vm["range"].as<string>());
        int a,b;
        char c;
        ss>>a>>c>>b;
        if(c!=',' || a<=0 || a>255 || b<=0 || b>255)
            throw(runtime_error("Invalid range"));
        parser->setConversionRange(a,b);
    }

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

    parser->parse();
    shared_ptr<CodeGenerator> generator=CodeGenerator::getGenerator(parser);
    generator->setLogStream(cout);
    generator->generateRendering(vm["image"].as<string>());
    generator->generateCode(vm["header"].as<string>(),vm["name"].as<string>());
    cout<<"Success"<<endl;
    return 0;
}
