#ifndef UNICODEBLOCKS_H
#define UNICODEBLOCKS_H

#include <map>
#include <string>
#include <vector>

/**
 * \file unicode_blocks.h
 * This file contains definitions for supported Unicode blocks.
 * When converting a font with this tool, user must specify blocks
 * to be included from this list
 */

namespace fontcore {

/**
 * numerical IDs for the blocks
 */
enum UnicodeBlockId
{
	BASIC_LATIN,
	LATIN1_SUPPLEMENT,
	LATIN_EXTENDED_A,
	LATIN_EXTENDED_B,
	GREEK_COPTIC,
	MATH_OPERATORS
};
	
/**
 * Represents a Unicode block, i.e.,
 * a range of codepoints grouped under the same name.
 */
class UnicodeBlock
{
public:
	/**
	   Constructor
	*/
    UnicodeBlock(char32_t startCodepoint, char32_t endCodepoint);

	/**
	   \return the first codepoint of the range
	*/
	unsigned int getStartCodepoint() const { return this->startCodepoint; }

	/**
	   \return the last codepoint of the range
	*/
	unsigned int getEndCodepoint() const { return this->endCodepoint; }
private:
	const char32_t startCodepoint;
	const char32_t endCodepoint;
};

class UnicodeBlockManager
{
public:
	/**
	 * \return a known Unicode block given its name
	 * \throws logic_error if the block is not supported by the system
	 */
	static const UnicodeBlock getKnownBlock(const std::string& name);

	/**
	 * \return all the block supported by the system
	 */
	static const std::vector<UnicodeBlock> getAllKnownBlocks();
	
private:
	/**
	 * \return block id given its name
	 * \throws logic_error if the block is unsupported
	 */
	static UnicodeBlockId unicodeBlocknameToId(const std::string& name);

	///< Unicode blocks known by the system
	static const std::map<UnicodeBlockId, UnicodeBlock> knownUnicodeBlocks;
};

} //namespace fontcore

#endif //UNICODEBLOCKS_H
