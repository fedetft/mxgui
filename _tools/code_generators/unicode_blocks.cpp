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


//
// class UnicodeBlockManager
//

const std::map<UnicodeBlockId, UnicodeBlock> UnicodeBlockManager::knownUnicodeBlocks =
{
	{BASIC_LATIN, UnicodeBlock(0x00000000, 0x0000007F)},
	{LATIN1_SUPPLEMENT, UnicodeBlock(0x00000080, 0x000000FF)},
	{LATIN_EXTENDED_A, UnicodeBlock(0x00000010, 0x0000017F)},
	{LATIN_EXTENDED_B, UnicodeBlock(0x00000180, 0x0000024F)},
	{GREEK_COPTIC, UnicodeBlock(0x00000370, 0x000003FF)},
	{MATH_OPERATORS, UnicodeBlock(0x00002200, 0x000022FF)}
};

const UnicodeBlock UnicodeBlockManager::getKnownBlock(const std::string &name)
{
	UnicodeBlockId id = unicodeBlocknameToId(name);

	if(knownUnicodeBlocks.count(id) < 1)
		throw(logic_error("Invalid code block"));
	
	return knownUnicodeBlocks.at(id);
}

const std::vector<UnicodeBlock> UnicodeBlockManager::getAllKnownBlocks()
{
	std::vector<UnicodeBlock> res;

	for(auto &block : knownUnicodeBlocks)
	{
		res.push_back(block.second);
	}

	return res;
}



