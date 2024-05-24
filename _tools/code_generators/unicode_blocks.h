/***************************************************************************
 *   Copyright (C) 2024 by Nicolas Benatti                                 *
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

#ifndef UNICODEBLOCKS_H
#define UNICODEBLOCKS_H

#include <string>
#include <vector>
#include <utility>

/**
 * \file unicode_blocks.h
 * This file contains definitions for Unicode blocks.
 * When converting a font with this tool, user must specify a
 * range of characters to be included 
 */

namespace fontcore {

/**
 * \ingroup pub_iface
 * Represents a Unicode block, i.e.,
 * a range of codepoints.
 */
class UnicodeBlock
{
public:
    /**
     * \return the first codepoint of the range
     */
    unsigned int getStartCodepoint() const;

    /**
     * \return the last codepoint of the range
     */
    unsigned int getEndCodepoint() const;

    /**
     * \return the number of characters in the range
     */
    unsigned int size() const;

    UnicodeBlock& operator=(const UnicodeBlock& other);

protected:
    /**
     * Constructor
     */
    UnicodeBlock(char32_t startCodepoint, char32_t endCodepoint);

private:
    char32_t startCodepoint;
    char32_t endCodepoint;

    /* we want to make other classes aware of
     * Unicode blocks, but at the same time
     * prohibit instantiation of any block outside of the manager
     */
    friend class UnicodeBlockManager;
};

/**
 * \ingroup pub_iface
 * Manager of all the character ranges to be converted.
 * The user specifies a certain number of Unicode blocks
 * to include in the output header file, otherwise
 * a default character table is used
 */
class UnicodeBlockManager
{
public:
    /**
     * \return all the blocks supported by the system
     */
    static const std::vector<UnicodeBlock> getAvailableBlocks();

    /**
     * Overwrite the previous list of Unicode blocks with a new one.
     * NOTE: the input list is assumed to be sorted by increasing
     * start codepoint
     */
    static void updateBlocks(std::vector<std::pair<char32_t,char32_t>> blocks);

    /**
     * Change the replacement character, which is stored in the last,
     * out-of-order range
     */
    static void updateReplacementCharacter(char32_t replacementCodepoint);

    /**
     * \return the replacement character (usually '�' or '?')
     */
    static char32_t getReplacementCharacter();

    /**
     * Checks whether a particular character is supported.
     * This is particularly useful for bdf font files
     * \return true if the character belongs to a supported block
     */
    static bool isCharacterSupported(char32_t codepoint);

    /**
     * Checks whether the replacement character is also
     * included in another range for normal use
     * \return true if the character is normal
     */
    static bool isReplacementNormal();

    /**
     * \return the total number of supported characters
     */
    static unsigned int numSupportedCharacters();

    /**
     * Convert a fixed-width UTF-32 codepoint in a
     * multibyte-encoded string, to be able to print it
     */
    static std::string codepointToString(char32_t codepoint);

private:
    ///< Unicode blocks known by the system
    static std::vector<UnicodeBlock> knownUnicodeBlocks;
};

} //namespace fontcore

#endif //UNICODEBLOCKS_H
