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

#ifndef BDFPARSER_H
#define BDFPARSER_H

#include "font_core.h"

namespace fontcore {

/**
 * Parses a font file and returns a vector of glyphs.
 * Only codepoints belonging to known blocks are returned,
 * exluding combiner characters which are not supported.
 * Some might be missing if they were not
 * available in the file, or if they could not fit into the desired
 * height and width
 */
class BDFParser: public FontParser
{
public:
    /**
     * Constructor
     */
    BDFParser(const std::string& filename);

    /**
     * Parses a bdf file, retrieving glyphs for the ASCII subset
     */
    void parse();

private:
    /**
     * \return the font height
     */
    unsigned int getHeight() const;
    
    /**
     * Parses the bdf file searching for the next STARTCHAR..ENDCHAR block
     * and return all the lines within that block.
     * \param file input stream from where to read. It is expected that
     * the input stream has exception on eof enabled
     * \return the list of lines within STARTCHAR and ENDCHAR
     */
    std::vector<std::string> getNextChar(std::ifstream& file);

    /**
     * Generate a glyph from the data between STARTCHAR..ENDCHAR found in
     * a bdf file, and adds it to the glyphs.
     * \param data the lines found in a bdf file enclosed in a STARTCHAR..ENDCHAR
     * block.
     */
    void generateGlyph(const std::vector<std::string>& data);

    /**
     * Create a fallback glyph in the case is not
     * present in the .bdf file
     */
    Glyph generateFallbackGlyph(char32_t codepoint, unsigned int height);

    /**
     * \return a list of codepoints which were absent from the
     * .bdf file, and thus need a fallback glyph
     */
    std::vector<char32_t> computeFailedCodepoints();

    bool isReplacementNormal;///< tells whether the replacement character is also included in normal ranges
    Glyph replacementGlyph;///< used only in case the repl. character is also normal
    unsigned int dWidth;///< width of the currently-being-parsed glyph, useful to set the width of the fallback glyphs which are generated after the conversion process
    int fontAscent, fontDescent; ///< See description of bdf format on wikipedia
};

}//namespace bdfcore

#endif //BDFPARSER_H
