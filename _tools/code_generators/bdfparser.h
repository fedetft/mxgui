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
 * Parses a bdf file and returns a vector of fonts.
 * Only ASCII fonts are returned, some might be missing if they were not
 * available in the bdf file, or if they could not fit into the desired
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
     * a bdf file, and adds it to the fonts (only if it is an ASCII glyph).
     * \param data the lines found in a bdf file encosed in a STARTCHAR..ENDCHAR
     * block.
     */
    void generateGlyph(std::vector<std::string> data);

    int fontAscent, fontDescent; ///< See description of bdf format on wikipedia
};

}//namespace bdfcore

#endif //BDFPARSER_H
