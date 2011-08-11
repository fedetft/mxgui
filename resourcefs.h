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

#ifndef MXGUI_LIBRARY
#error "This is header is private, it can be used only within mxgui."
#error "If your code depends on a private header, it IS broken."
#endif //MXGUI_LIBRARY

#include "mxgui_settings.h"
#include <unistd.h>

#ifndef RESOURCEFS_H
#define	RESOURCEFS_H

#ifdef MXGUI_ENABLE_RESOURCEFS

namespace resfs {

/**
 * This class allows to access files in the resourceFs filesystem,
 * available on some targets as a way to offload image storage on an external
 * flash memory, and retrieve them by name.
 */
class ResourceFile
{
public:
    /**
     * Default constructor, yields a closed file
     */
    ResourceFile(): siz(-1) {}

    /**
     * Constructor
     * \param name file name
     */
    explicit ResourceFile(const char *name) { this->open(name); }

    /**
     * Open a file in the ResourceFs filesystem
     * \param name file name
     */
    void open(const char *name);

    /**
     * \return true if the file was opened successfully
     */
    bool isOpen() const { return siz>=0; }

    /**
     * Close the file
     */
    void close() { siz=-1; }

    /**
     * \return file size if file was opened successfully, or -1
     */
    int size() const { return siz; }

    /**
     * Read data from the file
     * \param buffer data will be stored here
     * \param len number of bytes to read
     * \return number of bytes read, can be less than len if eof
     */
    int read(char *buffer, int len);

    /**
     * Move the file read pointer
     * \param pos offset
     * \param whence SEEK_SET, SEEK_CUR or SEEK_END
     * \return new value for the file read pointer
     */
    int lseek(int pos, int whence);

    //Uses default copy constructor, operator=

private:

    int startOffset; ///< Physical address on flash where the file starts
    int siz;         ///< File size
    int ptr;         ///< Read pointer, 0<=readPtr<=size
};

} // namespace resfs

#endif //MXGUI_ENABLE_RESOURCEFS

#endif //RESOURCEFS_H
