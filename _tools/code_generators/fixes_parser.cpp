/***************************************************************************
 *   Copyright (C) 2010, 2011 by Terraneo Federico                         *
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

#include "fixes_parser.h"
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

FixesParser::FixesParser() {}

void FixesParser::parse(string filename)
{
    ifstream file(filename.c_str());
    if(file.good()==false) throw(runtime_error("Failed reading fixes file"));
    string line;
    unsigned char current=0;
    bool openParen=false;
    while(getline(file,line))
    {
        if(line.empty() || line.at(0)=='\n' || line.at(0)=='#') continue;
        if(current==0)
        {
            stringstream ss(line);
            string temp;
            ss>>temp;
            if(temp!="glyph")
                throw(runtime_error(string("FixesParser: in '")+filename+
                        "' syntax error, expected 'glyph' but found '"+
                        temp+"'"));
            ss>>current;
            string shouldBeEmpty;
            getline(ss,shouldBeEmpty);
            if(shouldBeEmpty.empty()==false)
                throw(runtime_error(string("FixesParser: in '")+filename+
                        "' unexpected characters '"+shouldBeEmpty+"'"));
            cout<<"Fixes: found '"<<current<<"'"<<endl; //FIXME: use logstream
        } else {
            if(openParen==false)
            {
                if(line.at(0)!='{')
                    throw(runtime_error(string("FixesParser: in '")+filename+
                        "' expected '{' but found '"+line+"'"));
                openParen=true;
            } else {
                if(line.at(0)=='}')
                {
                    openParen=false;
                    current=0;
                } else parseOpt(line,current);
            }
        }
    }
}

int FixesParser::getShift(unsigned char c)
{
    map<unsigned char,pair<pair<int,int>,float> >::iterator it=fixes.find(c);
    if(it==fixes.end()) return 0;
    return it->second.first.first;
}

int FixesParser::getPad(unsigned char c)
{
    map<unsigned char,pair<pair<int,int>,float> >::iterator it=fixes.find(c);
    if(it==fixes.end()) return 0;
    return it->second.first.second;
}

float FixesParser::getContrast(unsigned char c)
{
    map<unsigned char,pair<pair<int,int>,float> >::iterator it=fixes.find(c);
    if(it==fixes.end()) return 1.0f;
    return it->second.second;
}

void FixesParser::parseOpt(string line, unsigned char current)
{
    stringstream ss(line);
    ss.exceptions();
    string command;
    ss>>command;
    if(command=="lshift")
    {
        int value;
        ss>>value;
        map<unsigned char,pair<pair<int,int>,float> >::iterator it;
		it=fixes.find(current);
        if(it==fixes.end())
        {
            fixes[current]=make_pair(make_pair(value,0),1.0f);
        } else {
            fixes[current]=make_pair(make_pair(value,it->second.first.second),
			    it->second.second);
        }
    } else if(command=="rpad")
    {
        int value;
        ss>>value;
        map<unsigned char,pair<pair<int,int>,float> >::iterator it;
		it=fixes.find(current);
        if(it==fixes.end())
        {
            fixes[current]=make_pair(make_pair(0,value),1.0f);
        } else {
            fixes[current]=make_pair(make_pair(it->second.first.first,value),
			    it->second.second);
        }
    } else if(command=="contrast")
	{
	    float value;
		ss>>value;
		map<unsigned char,pair<pair<int,int>,float> >::iterator it;
		it=fixes.find(current);
        if(it==fixes.end())
        {
            fixes[current]=make_pair(make_pair(0,0),value);
        } else {
            fixes[current]=make_pair(it->second.first,value);
        }
    } else {
        throw(runtime_error(string("FixesParser: unexpected token '")+
                command+"'; valid tokens are 'lshift' or 'rpad'"));
    }
}
