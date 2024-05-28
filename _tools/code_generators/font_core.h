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

#ifndef FONT_CORE_H
#define    FONT_CORE_H

#include "unicode_blocks.h"
#include <bitset>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <memory>

namespace fontcore {

/**
 * Convert a string to uppercase
 * \param x input string
 * \return output string
 */
std::string toUpper(const std::string& x);

/**
 * Contains one glyph, stored as a vector of bitsets.
 */
class Glyph
{
public:
    /**
     * The glyph width is currently limited to at most maxWidth when not
     * antialiased, and to at most maxWidth/2 bits when antialiased.
     */
    static const unsigned int maxWidth=64;

    /**
     * Constructor
     */
    Glyph();

    /**
     * \param width width of the glyph
     */
    void setWidth(unsigned int width);

    /**
     * \param data bitmap of the glyph
     */
    void setData(const std::vector<std::bitset<maxWidth> >& data);

    /**
     * \param value the Unicode codepoint that corresponds to this glyph
     */
    void setCodepoint(char32_t value);

    /**
     * Set if glyph is 1 bit per pixel (not antialiased) or 2 bit per pixel
     * \param antialiased true if antialiased
     */
    void setAntialiased(bool antialiased);

    /**
     * \return the glyph's width
     */
    unsigned int getWidth() const;

    /**
     * \return the glyph's height
     */
    unsigned int getHeight() const;

    /**
     * \param x x coordinate of pixel to get
     * \param y y coordinate of pixel to get
     * \return a pixel. If not antialiased the result is 0 or 1, otherwise
     * it is 0 to 3
     */
    unsigned char getPixelAt(int x, int y) const;

    /**
     * \return the Unicode codepoint value that corresponds to this glyph
     */
    char32_t getCodepoint() const;

    /**
     * \return true if glyph is 2 bit per pixel antialiased
     */
    bool isAntialiased() const;

    //Uses default copy constructor and operator=
private:
    unsigned int width; ///Glyph width
    std::vector<std::bitset<maxWidth> > data; ///Glyph bitmap, data[y][x] format
    char32_t codepoint; ///< Unicode codepoint value
    bool antialiased;
};

/**
 * Compare two glyphs. Using to sort the list of glyphs
 */
bool operator< (Glyph a, Glyph b);

/**
 * Parses a font file and returns a vector of glyphs.
 * Only codepoints belonging to known blocks are returned,
 * exluding combiner characters which are not supported.
 * Some might be missing if they were not
 * available in the file, or if they could not fit into the desired
 * height and width
 */
class FontParser
{
public:

    /**
     * Factory method.
     * \param filename a bdf or ttf file
     * \return an instance of a parser able to support the format
     */
    static std::shared_ptr<FontParser> getParser(const std::string& filename);

    /**
     * Constructor
     */
    FontParser(const std::string& filename);

    /**
     * Enable logging mode.
     * Data is printed on the output stream passed. The stream is owned by the
     * caller and must be valid for the full lifetime of the FontParser object
     * it has been passed to
     * \param output where to print debug output
     */
    void setLogStream(std::ostream& output);

    /**
     * Specifies which Unicode blocks the parser must extract
     * from the .ttf file
     */
    void setUnicodeBlocks(const std::vector<UnicodeBlock>& blocks);

    /**
     * Parses a font file, retrieving glyphs
     */
    virtual void parse()=0;

    /**
     * \return the list of glyphs
     */
    std::vector<Glyph> getGlyphs() const;

    /**
     * Set height for rendering truetype fonts only
     * \param height font height
     */
    void setTTFHeight(unsigned int height);

    /**
     * Set padding for truetype fonts only
     * \param padding font padding
     */
    void setTTFPadding(unsigned int padding);

    /**
     * For TTF files only.
     * Specify a fixes file for kerning issues
     */
    void setFixesFile(std::string file);

    /**
     * Pure virtual class
     */
    virtual ~FontParser();
    
protected:
    std::ostream *logStream; ///< Valid only if debugFlag is true
    bool log; ///< True if debugMode has been called
    std::vector<Glyph> glyphs; ///< List of glyphs
    const std::string filename;///< Font file
    std::string fixesFile; ///< Fixes file
    std::vector<UnicodeBlock> blocks; ///< Unicode blocks
    unsigned int ttfHeight; ///< Rendering height (for ttf only)
    unsigned int ttfPadding; ///< Padding (for ttf only)

private:
    FontParser(const FontParser&);
    FontParser& operator= (const FontParser&);
};

/**
 * Starting from a list of Glyphs, generate code (an .h file) containing look up
 * tables. Both fixed width and variable width fonts are supported
 */
class CodeGenerator
{
public:
    /**
     * Factory method.
     * \param parser an instance of a parser that has successfully parsed
     * a font file.
     * \return an instance of a code generator
     */
    static std::shared_ptr<CodeGenerator>
    getGenerator(std::shared_ptr<FontParser> parser);

    /**
     * Constructor.
     * \param the parser
     */
    CodeGenerator(std::shared_ptr<FontParser> parser);

    /**
     * Enable logging mode.
     * Data is printed on the outout stream passed. The stream is owned by the
     * caller and must be valid for the full lifetime of the BDFParser object
     * it has been passed to
     * \param output where to print debug output
     */
    void setLogStream(std::ostream& output);

    /**
     * Generate a PNG image that contains a rendering of the glyphs of a BDF file
     * \param filename name of output PNG file
     */
    void generateRendering(const std::string& filename) const;

    /**
     * Generate the .h file with the look up tables
     * \param filename name of the .h file
     */
    virtual void generateCode(const std::string filename,
        const std::string& fontName)=0;

    /**
     * Destructor.
     */
    virtual ~CodeGenerator();
    
protected:
    std::ostream *logStream; ///< Valid only if debugFlag is true
    bool log; ///< True if debugMode has been called
    std::vector<Glyph> glyphs; ///< List of glyphs

private:
    CodeGenerator(const CodeGenerator&);
    CodeGenerator& operator= (const CodeGenerator&);
};

} //namespace fontcore

#endif  //FONT_CORE_H
