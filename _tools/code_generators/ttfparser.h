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

#ifndef TTFPARSER_H
#define	TTFPARSER_H

#include "font_core.h"
#include "fixes_parser.h"
#include <ft2build.h>
#include FT_FREETYPE_H

namespace fontcore {

/**
 * Parses a truetype file and renders a vector of fonts.
 * Only ASCII fonts are returned, some might be missing if they were not
 * available in the ttf file, or if they could not fit into the desired
 * height and width
 */
class TTFParser: public FontParser
{
public:
    /**
     * Constructor
     */
    TTFParser(const std::string& filename);

    /**
     * Parses a bdf file, retrieving glyphs for the ASCII subset
     */
    void parse();

private:

    /**
     * Add a glyph to the list of glyphs
     * \param face the currently rendered glyph
     */
    void generateGlyph(const FT_Face& face, unsigned char chr);

    int ascent; ///<Ascent of font
    int descent; ///<Descent of font
    unsigned int realHeight; ///<Real font height, computed as ascent+descent
    FixesParser fixes; ///< Fixes
};

}//namespace bdfcore

#endif //TTFPARSER_H
