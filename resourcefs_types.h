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

// ResourceFs: simple read only directory-less filesystem for flash memories

#ifndef MXGUI_LIBRARY
#error "This is header is private, it can be used only within mxgui."
#error "If your code depends on a private header, it IS broken."
#endif //MXGUI_LIBRARY

#ifndef RESOURCEFS_TYPES_H
#define RESOURCEFS_TYPES_H

/**
 * \internal Header for the ResourceFs filesystem, stored @ address 0
 */
struct Header
{
	char marker[32];        ///< 32 'w' characters, not null terminated
	char fsName[16];        ///< "ResourceFs 1.0", null terminated
	char osName[8];         ///< "Miosix", null terminated
	unsigned int fileCount; ///< # of files in the filesystem, little endian
	unsigned int unused;    ///< Unused, just rounds up the Header to 64 bytes
};

/// \internal Maximum length of a file name in the ResourceFs filesystem.
const unsigned int resourceFsFileMax=23;

/**
 * \internal Inode for ResourceFs filesystem, there is one instance of this
 * struct for each file, stored after the header
 */
struct FileInfo
{
	unsigned int start;             ///< File start offset, little endian
	unsigned int length;            ///< File length, little endian
	char name[resourceFsFileMax+1]; ///< File name, null teminated
};

#endif //RESOURCEFS_TYPES_H
