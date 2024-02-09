#include "unicode_blocks.h"
#include <stdexcept>
#include <utility>
#include <exception>
#include <vector>

using namespace std;
using namespace fontcore;

//
// class UnicodeBlock
//

UnicodeBlock::UnicodeBlock(char32_t startCodepoint, char32_t endCodepoint)
    : startCodepoint(startCodepoint), endCodepoint(endCodepoint) {}

unsigned int UnicodeBlock::getStartCodepoint() const
{
	return this->startCodepoint;
}

unsigned int UnicodeBlock::getEndCodepoint() const
{
	return this->endCodepoint;
}

unsigned int UnicodeBlock::size() const
{
	return this->endCodepoint - this->startCodepoint + 1;
}

UnicodeBlock& UnicodeBlock::operator=(const UnicodeBlock& other)
{
	if(this == &other)
		return *this;

	this->startCodepoint = other.startCodepoint;
	this->endCodepoint = other.endCodepoint;
	return *this;
}

//
// class UnicodeBlockManager
//

const std::map<UnicodeBlockId, UnicodeBlock> UnicodeBlockManager::knownUnicodeBlocks =
{
	{BASIC_LATIN, UnicodeBlock(0x00000000, 0x0000007F)},
	{LATIN1_SUPPLEMENT, UnicodeBlock(0x00000080, 0x000000FF)},
	{LATIN_EXTENDED_A, UnicodeBlock(0x00000100, 0x0000017F)},
	{LATIN_EXTENDED_B, UnicodeBlock(0x00000180, 0x0000024F)},
	{GREEK_COPTIC, UnicodeBlock(0x00000370, 0x000003FF)},
	{MATH_OPERATORS, UnicodeBlock(0x00002200, 0x000022FF)}
};

const std::vector<UnicodeBlock> UnicodeBlockManager::getAvailableBlocks()
{
	std::vector<UnicodeBlock> res;

	for(auto &block : knownUnicodeBlocks)
		res.push_back(block.second);

	return res;
}

bool UnicodeBlockManager::isCharacterSupported(char32_t codepoint)
{
	for(auto& blockId : knownUnicodeBlocks)
	{
		UnicodeBlock range = blockId.second;
		if(codepoint < range.getStartCodepoint() &&
		   codepoint > range.getEndCodepoint())
			return false;
	}

	return true;
}

unsigned int UnicodeBlockManager::numSupportedCharacters()
{
	unsigned int res = 0;

	for(auto& blockId : knownUnicodeBlocks)
	{
		UnicodeBlock range = blockId.second;
		res += range.size();
	}

	return res;
}
