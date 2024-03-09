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

std::vector<UnicodeBlock> UnicodeBlockManager::knownUnicodeBlocks =
{
	UnicodeBlock(0x00000000, 0x0000007F),
	UnicodeBlock(0x000000A0, 0x0000017F),
	UnicodeBlock(0x00000391, 0x000003E1),
	UnicodeBlock(0x00000400, 0x00000479),
	UnicodeBlock(0x0000048A, 0x0000052F)
};

void UnicodeBlockManager::updateBlocks(std::vector<std::pair<char32_t,char32_t>> blocks)
{
	knownUnicodeBlocks.clear();
    for(auto& blk : blocks)
		knownUnicodeBlocks.push_back(UnicodeBlock(blk.first,blk.second));
}    

const std::vector<UnicodeBlock> UnicodeBlockManager::getAvailableBlocks()
{
	std::vector<UnicodeBlock> res = knownUnicodeBlocks;

	return res;
}

bool UnicodeBlockManager::isCharacterSupported(char32_t codepoint)
{
	for(auto& block : knownUnicodeBlocks)
	{
		if(codepoint >= block.getStartCodepoint() &&
		   codepoint <= block.getEndCodepoint())
			return true;
	}

	return false;
}

unsigned int UnicodeBlockManager::numSupportedCharacters()
{
	unsigned int res = 0;

	for(auto& block : knownUnicodeBlocks)
		res += block.size();

	return res;
}
