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

#include <string>
#include <map>

#ifndef FIXES_PARSER_H
#define	FIXES_PARSER_H

/**
 * Parser class for the fixes file format, used to fix kerning issues in TTF
 * files.
 */
class FixesParser
{
public:
    /**
     * Constructor. 
     */
    FixesParser();

    /**
     * Passed the fixes file, all the parsing is done here.
     * \param filename fixes file to parse
     */
    void parse(std::string filename);

    /**
     * \param c a character
     * \return the shift required to fix the glyph, or zero if no fixes required
     */
    int getShift(unsigned char c);

    /**
     * \param c a character
     * \return the pad required to fix the glyph, or zero if no fixes required
     */
    int getPad(unsigned char c);
	
	/**
	 * \param c a character
	 * \return the contrast required to fix the glyph. 1.0f is the default
	 * value. Values 0<=x<1 mean a darker glyph, while values >1 a lighter glyph
	 */
	float getContrast(unsigned char c);

private:

    /**
     * Parse an option within a {} block
     * \param line line to parse
     * \param current glyph to which the {} block refers
     */
    void parseOpt(std::string line, unsigned char current);
	
     ///< Contains all fixes
    std::map<unsigned char,std::pair<std::pair<int,int>,float> > fixes;
};

#endif //FIXES_PARSER_H
