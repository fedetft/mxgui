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

#include "gltf_helper.h"

const char *getMode(int mode)
{
    switch(mode)
    {
        case TINYGLTF_MODE_TRIANGLES:      return "triangles";
        case TINYGLTF_MODE_TRIANGLE_STRIP: return "triangle strip";
        case TINYGLTF_MODE_TRIANGLE_FAN:   return "triangle fan";
        case TINYGLTF_MODE_POINTS:         return "points";
        case TINYGLTF_MODE_LINE:           return "line";
        case TINYGLTF_MODE_LINE_LOOP:      return "line loop";
        default: return "unknown";
    }
}

const char *getComponentType(int componentType)
{
    switch(componentType)
    {
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:  return "uint8_t";
        case TINYGLTF_COMPONENT_TYPE_SHORT:          return "int16_t";
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: return "uint16_t";
        case TINYGLTF_COMPONENT_TYPE_INT:            return "int32_t";
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:   return "uint32_t";
        case TINYGLTF_COMPONENT_TYPE_FLOAT:          return "float";
        case TINYGLTF_COMPONENT_TYPE_DOUBLE:         return "double";
        default: return "unknown";
    }
}

int getType(int type)
{
    switch(type)
    {
        case TINYGLTF_TYPE_SCALAR: return 1;
        case TINYGLTF_TYPE_VEC2:   return 2;
        case TINYGLTF_TYPE_VEC3:   return 3;
        case TINYGLTF_TYPE_VEC4:   return 4;
        default: abort();
    }
}
