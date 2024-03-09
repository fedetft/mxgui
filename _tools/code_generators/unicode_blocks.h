#ifndef UNICODEBLOCKS_H
#define UNICODEBLOCKS_H

#include <string>
#include <vector>
#include <utility>

/**
 * \file unicode_blocks.h
 * This file contains definitions for supported Unicode blocks.
 * When converting a font with this tool, user must specify blocks
 * to be included from this list
 */

namespace fontcore {
	
/**
 * Represents a Unicode block, i.e.,
 * a range of codepoints grouped under the same name.
 */
class UnicodeBlock
{
public:
	/**
	   \return the first codepoint of the range
	*/
	unsigned int getStartCodepoint() const;
	
	/**
	   \return the last codepoint of the range
	*/
	unsigned int getEndCodepoint() const;

	/**
	 * \return the number of characters in the range
	 */
	unsigned int size() const;

	UnicodeBlock& operator=(const UnicodeBlock& other);
	
protected:
	/**
	   Constructor
	*/
	UnicodeBlock(char32_t startCodepoint, char32_t endCodepoint);
	
private:
	char32_t startCodepoint;
	char32_t endCodepoint;

	/* we want to make other classes aware of
	 * Unicode blocks, but at the same time
	 * prohibit instantiation of any block outside of the manager*/
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
	 * Overwrites the previous list of Unicode blocks with a new one.
	 * NOTE: the input list is assumed to be sorted by increasing
	 * start codepoint
	 */
	static void updateBlocks(std::vector<std::pair<char32_t,char32_t>> blocks);
	
	/**
	 * Checks whether a particular character is supported.
	 * This is particularly useful for bdf font files
	 *  \return true if the character belongs to a supported block
	 */
	static bool isCharacterSupported(char32_t codepoint);

	/**
	   \return the total number of supported characters
	*/
	static unsigned int numSupportedCharacters();
	
private:
    ///< Unicode blocks known by the system
	static std::vector<UnicodeBlock> knownUnicodeBlocks;
};

} //namespace fontcore

#endif //UNICODEBLOCKS_H
