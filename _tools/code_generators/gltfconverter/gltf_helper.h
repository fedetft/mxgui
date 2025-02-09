/***************************************************************************
 *   Copyright (C) 2025 by Terraneo Federico                               *
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

#pragma once

#include <stdexcept>
#include "tiny_gltf.h"

/**
 * \param mode TINYGLTF_MODE_*
 * \return a string describing the mode
 */
const char *getMode(int mode);

/**
 * \param mode TINYGLTF_COMPONENT_TYPE_*
 * \return a string describing the component type
 */
const char *getComponentType(int componentType);

/**
 * \param mode TINYGLTF_TYPE_*
 * \return an integer counting the number of components of the type
 */
int getType(int type);

/**
 * \internal implementation detail of typedValueCast
 * Do a double cast and array indexing. First, cast the untyped byte buffer to
 * the type of the data that's stored into it U*, then perform array indexing
 * to get at the desired element in the array, and then finally cast the array
 * elmenet to the type that we want T, whcih could differ from U.
 * \tparam T the desired target type
 * \tparam U the type of the buffer values
 * \param b buffer first byte
 * \param e one past the last buffer bytes
 * \param index index into the buffer but once it has been cast to its typed U*
 */
template<typename T, typename U>
static T cast(const unsigned char *b, const unsigned char *e, int index)
{
    //TODO: what abount endianness? gltf binary seems to assume little?
    auto *array=reinterpret_cast<const U*>(b);
    if(reinterpret_cast<const unsigned char*>(array+index)>=e)
        throw std::range_error("typedValueCast: index out of bounds");
    return static_cast<T>(array[index]);
}

/**
 * \tparam T the type that we want, regardless of the type data is actually stored
 * \param b buffer first byte
 * \param e one past the last buffer bytes
 * \param type the type data is stored in the byte buffer
 * \param index index into the typed buffer array
 */
template<typename T>
T typedValueCast(const unsigned char *b, const unsigned char *e, int type, int index)
{
    switch(type)
    {
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
            return cast<T,uint8_t>(b,e,index);
            break;
        case TINYGLTF_COMPONENT_TYPE_SHORT:
            return cast<T,int16_t>(b,e,index);
            break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
            return cast<T,uint16_t>(b,e,index);
            break;
        case TINYGLTF_COMPONENT_TYPE_INT:
            return cast<T,int32_t>(b,e,index);
            break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
            return cast<T,uint32_t>(b,e,index);
            break;
        case TINYGLTF_COMPONENT_TYPE_FLOAT:
            return cast<T,float>(b,e,index);
            break;
        case TINYGLTF_COMPONENT_TYPE_DOUBLE:
            return cast<T,double>(b,e,index);
            break;
        default:
            throw std::range_error("typedValueCast: unknown type");
    }
}
