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

#ifndef VARIABLE_WIDTH_GENERATOR_H
#define	VARIABLE_WIDTH_GENERATOR_H

#include "font_core.h"

namespace fontcore {

/**
 * Starting from a list of Glyphs, generate code (an .h file) containing look up
 * tables.
 */
class VariableWidthGenerator: public CodeGenerator
{
public:
    /**
     * Constructor.
     */
    VariableWidthGenerator(boost::shared_ptr<FontParser> parser);

    /**
     * Generate the .h file with the look up tables
     * \param filename name of the .h file
     */
    void generateCode(const std::string filename, const std::string& fontName);
};

} //namepace fontcore

#endif //VARIABLE_WIDTH_GENERATOR_H
