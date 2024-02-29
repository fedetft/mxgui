#include "unicode_blocks.h"
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

const std::vector<UnicodeBlock> UnicodeBlockManager::knownUnicodeBlocks =
{
	UnicodeBlock(0x00000020, 0x0000007F),
	UnicodeBlock(0x000000A0, 0x00000017E),
	UnicodeBlock(0x00000391, 0x000003A1),
	UnicodeBlock(0x00000180, 0x0000024F),
	UnicodeBlock(0x000003A3, 0x000003E1),
};

const std::vector<UnicodeBlock> UnicodeBlockManager::getAvailableBlocks()
{
	std::vector<UnicodeBlock> res = knownUnicodeBlocks;

	return res;
}

bool UnicodeBlockManager::isCharacterSupported(char32_t codepoint)
{
	for(auto& block : knownUnicodeBlocks)
	{
		if(codepoint < block.getStartCodepoint() &&
		   codepoint > block.getEndCodepoint())
			return false;
	}

	return true;
}

unsigned int UnicodeBlockManager::numSupportedCharacters()
{
	unsigned int res = 0;

	for(auto& block : knownUnicodeBlocks)
		res += block.size();

	return res;
}
