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
 *   As a special exception, if other files instantiate templates or use   *
 *   macros or inline functions from this file, or you compile this file   *
 *   and link it with other works to produce a work based on this file,    *
 *   this file does not by itself cause the resulting work to be covered   *
 *   by the GNU General Public License. However the source code for this   *
 *   file must still be made available in accordance with the GNU General  *
 *   Public License. This exception does not invalidate any other reasons  *
 *   why a work based on this file might be covered by the GNU General     *
 *   Public License.                                                       *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>   *
 ***************************************************************************/

#include "resourcefs.h"
#include "resourcefs_types.h"
#include "drivers/resfs_mp3v2.h"
#ifdef _MIOSIX
#include "miosix.h"
#endif _MIOSIX
#include <cstdio>
#include <algorithm>
#include <cstring>

using namespace std;

namespace resfs {

#ifdef MXGUI_ENABLE_RESOURCEFS
#ifdef _MIOSIX

//
// class ResourceFs
//

/**
 * Simple singleton class to take advantage of threadsafe static constructor
 * calls under Miosix to make sure the function backendInit() is called once
 */
class ResourceFs
{
public:
    /**
     * \return an instance of the ResourceFs class (singleton)
     */
    static ResourceFs& instance()
    {
        static ResourceFs singleton;
        return singleton;
    }

    /**
     * Open a file
     * \param name file name
     * \return file start and size, or size==-1 if any error
     */
    pair<int,int> open(const char *name);

private:
    ResourceFs();

    int numFiles; ///< number of files in the filesystem, or -1 if init failed
};

ResourceFs::ResourceFs() : numFiles(-1)
{
    backendInit();
    Header header;
    backendRead(reinterpret_cast<char*>(&header),0,sizeof(Header));
    if(memcmp(header.marker,"wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww",32)!=0) return;
    if(strcmp(header.fsName,"ResourceFs 1.0")!=0) return;
    if(strcmp(header.osName,"Miosix")!=0) return;
    numFiles=header.fileCount; //TODO: endianness
    #ifdef WITH_BOOTLOG
    iprintf("ResourceFs initialization Ok (%d files)\n",numFiles);
    #endif //WITH_BOOTLOG
}

pair<int,int> ResourceFs::open(const char* name)
{
    if(numFiles<0) return make_pair(0,-1); //Failed initializing fs
    for(int i=0;i<numFiles;i++) //O(n) lookup is not fast, but memory efficient
    {
        FileInfo info;
        backendRead(reinterpret_cast<char*>(&info),
            sizeof(Header)+i*sizeof(FileInfo),sizeof(FileInfo));
        if(strcmp(name,info.name)!=0) continue;
        return make_pair(info.start,info.length); //TODO: endianness
    }
    return make_pair(0,-1); //File not found
}

//
// class ResourceFile
//

void ResourceFile::open(const char *name)
{
    pair<int,int> result=ResourceFs::instance().open(name);
    this->startOffset=result.first;
    this->siz=result.second;
    this->ptr=0;
}

int ResourceFile::read(char *buffer, int len)
{
    if(this->siz<0) return -1;
    int toRead=min(len,this->siz-this->ptr);
    backendRead(buffer,this->startOffset+this->ptr,toRead);
    this->ptr+=toRead;
    return toRead;
}

int ResourceFile::lseek(int pos, int whence)
{
    int temp;
    switch(whence)
    {
        case SEEK_CUR:
            temp=this->ptr+pos;
            break;
        case SEEK_SET:
            temp=pos;
            break;
        case SEEK_END:
            temp=this->siz+pos;
            break;
        default:
            return -1;
    }
    if(temp<0 || temp>this->siz) return -1; //Correctly fails if siz==-1
    this->ptr=temp;
    return temp;
}

#else //_MIOSIX
// _MIOSIX is not defined, we're in the simulator. Here a ResourceFile is
// just a file with "resource/" padded to its name

void ResourceFile::open(const char *name)
{
    siz=-1;
    if(strlen(name)==0) return;
    filename=string("resource/")+name;
    file.open(filename.c_str(),ios::binary);
    if(!file.good()) return;
    file.seekg(0,ios::end);
    siz=file.tellg();
    file.seekg(0,ios::beg);
    ptr=0;
}

int ResourceFile::read(char *buffer, int len)
{
    if(this->siz<0) return -1;
    file.read(buffer,len);
    int result=file.gcount();
    ptr+=result;
    return result;
}

int ResourceFile::lseek(int pos, int whence)
{
    if(this->siz<0) return -1;
    switch(whence)
    {
        case SEEK_CUR:
            file.seekg(pos,ios::cur);
            break;
        case SEEK_SET:
            file.seekg(pos,ios::beg);
            break;
        case SEEK_END:
            file.seekg(pos,ios::end);
            break;
        default:
            return -1;
    }
    if(file.fail())
    {
        file.clear();
        return -1;
    }
    ptr=file.tellg();
    return ptr;
}

ResourceFile::ResourceFile(const ResourceFile& rhs)
{
    this->siz=rhs.siz;
    if(this->siz==-1) return;
    this->filename=rhs.filename;
    this->file.open(rhs.filename.c_str());
    this->ptr=rhs.ptr;
    this->file.seekg(rhs.ptr);
}

ResourceFile& ResourceFile::operator=(const ResourceFile& rhs)
{
    this->siz=rhs.siz;
    if(this->siz==-1) return *this;
    this->filename=rhs.filename;
    this->file.open(rhs.filename.c_str());
    this->ptr=rhs.ptr;
    this->file.seekg(rhs.ptr);
    return *this;
}

#endif //_MIOSIX
#endif //MXGUI_ENABLE_RESOURCEFS

} // namespace resfs
