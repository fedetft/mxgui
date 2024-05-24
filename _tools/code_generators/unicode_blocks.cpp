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
    return this->endCodepoint-this->startCodepoint+1;
}

UnicodeBlock& UnicodeBlock::operator=(const UnicodeBlock& other)
{
    if(this==&other)
        return *this;

    this->startCodepoint=other.startCodepoint;
    this->endCodepoint=other.endCodepoint;
    return *this;
}

//
// class UnicodeBlockManager
//

std::vector<UnicodeBlock> UnicodeBlockManager::knownUnicodeBlocks =
{
    UnicodeBlock(0x00000020,0x0000007E),
    UnicodeBlock(0x0000FFFD,0x0000FFFD)
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

char32_t UnicodeBlockManager::getReplacementCharacter()
{
    return knownUnicodeBlocks[knownUnicodeBlocks.size()-1].getStartCodepoint();
}

const std::vector<UnicodeBlock> UnicodeBlockManager::getAvailableBlocks()
{
    std::vector<UnicodeBlock> res=knownUnicodeBlocks;
    return res;
}

bool UnicodeBlockManager::isCharacterSupported(char32_t codepoint)
{
    for(auto& block : knownUnicodeBlocks)
    {
        if(codepoint>=block.getStartCodepoint() &&
           codepoint<=block.getEndCodepoint())
            return true;
    }

    return false;
}

bool UnicodeBlockManager::isReplacementNormal()
{
    char32_t repl = getReplacementCharacter();
    for(int i=0;i<knownUnicodeBlocks.size()-1;i++)
    {
        UnicodeBlock block=knownUnicodeBlocks[i];
        if(repl>=block.getStartCodepoint() &&
           repl<=block.getEndCodepoint())
            return true;
    }

    return false;
}

unsigned int UnicodeBlockManager::numSupportedCharacters()
{
    unsigned int res=0;

    for(auto& block : knownUnicodeBlocks)
        res+=block.size();

    return res;
}

std::string UnicodeBlockManager::codepointToString(char32_t codepoint)
{
    char unistr[MB_LEN_MAX+1]={0};
    mbstate_t ps;

    setlocale(LC_ALL,"en_GB.UTF-8");
    memset(&ps,0,sizeof(ps));
    c32rtomb(unistr,codepoint,&ps);

    return unistr;
}
