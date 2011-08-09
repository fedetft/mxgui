 /***************************************************************************
  *   Copyright (C) 2011 by Terraneo Federico                               *
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
 
 /*
  * resourcefs.cpp
  * store a set of files in a lightweight single directory filesystem-like
  * structure. The generated output is a binary file suitable to be stored
  * into a flash memory.
  * The idea behind this is to store images in an external flash instead of
  * mixing them with code in the internal flash of a microcontroller.
  * It usually results in slower access to the images, but leaves the
  * microcontroller's internal memory free for storing code.
  */
 
#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <algorithm>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include "resourcefs_types.h"

using namespace std;
using namespace boost::program_options;
using namespace boost::filesystem;

/**
 * \param an unsigned int
 * \return the same int forced into little endian representation
 */
unsigned int toLittleEndian(unsigned int x)
{
	static bool first=true, little;
	union {
		unsigned int a;
		unsigned char b[4];
	} endian;
	if(first)
	{
		endian.a=0x12;
		little=endian.b[0]==0x12;
		first=false;
	}
	if(little) return x;
	endian.a=x;
	swap(endian.b[0],endian.b[3]);
	swap(endian.b[1],endian.b[2]);
	return endian.a;
}

int main(int argc, char *argv[])
{
	// Check args
	options_description desc("ResourceFs utility v1.00\n"
		"Designed by TFT : Terraneo Federico Technologies\nOptions");
	desc.add_options()
		("help", "Prints this.")
		("in", value<string>(),  "Input directory (required)")
		("out", value<string>(), "Output file name (required)")
	;
	variables_map vm;
	store(parse_command_line(argc,argv,desc),vm);
	notify(vm);
	if(vm.count("help") || (!vm.count("in")) || (!vm.count("out")))
	{
		cerr<<desc<<endl;
		return 1;
	}

	//Traverse the input directory storing path of files 
	path inDir(vm["in"].as<string>());
	directory_iterator end;
	vector<path> files;
	for(directory_iterator it(inDir);it!=end;++it)
	{
		if(is_directory(it->status()))
		{
			cerr<<"Warning: ignoring subdirectory \""<<it->path()<<"\""<<endl;
		} else if(it->leaf().length()>resourceFsFileMax)
		{
			cerr<<"Error: file name \""<<it->leaf()<<"\" is too long"<<endl;
			return 1;
		} else files.push_back(it->path());
	}

	// Constructing the filesystem header
	Header header;
	memset(&header,0,sizeof(Header));
	memcpy(header.marker,"wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww",32);
	memcpy(header.fsName,"ResourceFs 1.0",14);
	memcpy(header.osName,"Miosix",6);
	header.fileCount=toLittleEndian(files.size());

	// Building the output filesystem
	ofstream out(vm["out"].as<string>().c_str(),ios::binary);
	if(!out.good())
	{
		cerr<<"Can't open otput file"<<endl;
		return 1;
	}
	out.write((char*)&header,sizeof(Header)); //Write header
	int start=64+32*files.size();
	out.seekp(start,ios::beg); //Skip inodes and write individual file contents
	vector<FileInfo> fileInfos;
	for(int i=0;i<files.size();i++)
	{
		ifstream in(files[i].string().c_str(),ios::binary);
		FileInfo file;
		memset(&file,0,sizeof(FileInfo));
		file.start=toLittleEndian(start);
		in.seekg(0,ios::end);
		file.length=toLittleEndian(in.tellg());
		start+=in.tellg();
		strcpy(file.name,files[i].leaf().c_str());
		fileInfos.push_back(file);

		in.seekg(0,ios::beg);
		for(;;)
		{
			char buf[1024];
			in.read(buf,1024);
			int readBytes=in.gcount();
			if(readBytes==0) break;
			out.write(buf,readBytes);
		}
	}

	out.seekp(64,ios::beg); //Get back to after th header and write inodes
	for(int i=0;i<fileInfos.size();i++)
		out.write((char*)&fileInfos[i],sizeof(FileInfo));
	return 0;
}
