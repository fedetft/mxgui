#include "unicode_blocks.h"
#include <clocale>
#include <climits>
#include <cuchar>
#include <cstring>

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
	UnicodeBlock(0x00000020, 0x0000007F),
	UnicodeBlock(0x0000FFFD, 0x0000FFFD)
};

void UnicodeBlockManager::updateBlocks(std::vector<std::pair<char32_t,char32_t>> blocks)
{
	knownUnicodeBlocks.clear();
    for(auto& blk : blocks)
		knownUnicodeBlocks.push_back(UnicodeBlock(blk.first,blk.second));
}

void UnicodeBlockManager::updateReplacementCharacter(char32_t replacementCodepoint)
{
	knownUnicodeBlocks[knownUnicodeBlocks.size()-1]={replacementCodepoint,replacementCodepoint};
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

std::string UnicodeBlockManager::codepointToString(char32_t codepoint)
{
	char unistr[MB_LEN_MAX+1]={0};
	mbstate_t ps;
	
	setlocale(LC_ALL, "en_GB.UTF-8");
	memset(&ps,0,sizeof(ps));
	c32rtomb(unistr,codepoint,&ps);
	
	return unistr;
}
