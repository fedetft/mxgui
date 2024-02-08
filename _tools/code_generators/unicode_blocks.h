#ifndef UNICODEBLOCKS_H
#define UNICODEBLOCKS_H

#include <map>
#include <string>

/**
 * \file unicode_blocks.h
 * This file contains definitions for supported Unicode blocks.
 * When converting a font with this tool, user must specify blocks
 * to be included from this list
 */

/**
 * Represents a Unicode block, i.e.,
 * a range of codepoints grouped under the same name.
 */
class UnicodeBlock {
public:
	/**
	   Constructor.
	*/
    UnicodeBlock(unsigned int startCodepoint,
				 unsigned int endCodepoint): startCodepoint(startCodepoint), endCodepoint(endCodepoint) {}

	/**
	   \return the first codepoint of the range.
	*/
	unsigned int getStartCodepoint() const { return this->startCodepoint; }

	/**
	   \return the last codepoint of the range.
	*/
	unsigned int getEndCodepoint() const { return this->endCodepoint; }
private:
	const char32_t startCodepoint;
	const char32_t endCodepoint;
};

class UnicodeBlockManager {
public:
	/**
	 * \return a known Unicode block given its name
	 * \throws logic_error if the block is not supported by the system
	 */
	static const std::pair<char32_t, char32_t> getKnownBlock(std::string name);
private:
	enum UnicodeBlockId {
		BASIC_LATIN,
		LATIN1_SUPPLEMENT,
		LATIN_EXTENDED_A,
		LATIN_EXTENDED_B,
		GREEK_COPTIC,
		MATH_OPERATORS
	};

	///< Unicode blocks known by the system
	const std::map<UnicodeBlockId, UnicodeBlock> knownUnicodeBlocks = {
		{BASIC_LATIN, UnicodeBlock(0x00000000, 0x0000007F)},
		{LATIN1_SUPPLEMENT, UnicodeBlock(0x00000080, 0x000000FF)},
		{LATIN_EXTENDED_A, UnicodeBlock(0x00000010, 0x0000017F)},
		{LATIN_EXTENDED_B, UnicodeBlock(0x00000180, 0x0000024F)},
		{GREEK_COPTIC, UnicodeBlock(0x00000370, 0x000003FF)},
		{MATH_OPERATORS, UnicodeBlock(0x00002200, 0x000022FF)}
	};
};
	
#endif //UNICODEBLOCKS_H
