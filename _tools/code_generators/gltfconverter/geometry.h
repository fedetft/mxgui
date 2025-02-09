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

#include <vector>
#include <array>
#include "tiny_gltf.h"

enum class PrimitiveType
{
   TRIANGLES,
   QUADS
};

/**
 * Geometry data imported from a gltf file
 */
class Geometry
{
public:
    /**
     * Constructor
     * \param model loaded gltf model
     */
    Geometry(tinygltf::Model model);

    /**
     * Dump the geometry to a C++ header file to be rendered on the microcontroller
     */
    void dump(std::ostream& out, PrimitiveType primitiveType);

private:
    /**
     * gltf file is a tree of nodes
     */
    void importNode(int nodeId);

    /**
     * Import vertices form a node, applying transformations
     */
    void importVertices(const tinygltf::Accessor& accessor);

    /**
     * Import trinages from a node
     * \param offset when flattening all nodes in a single array, need to add
     * offset to keep polygons referencing the correct vertices
     */
    void importTriangles(const tinygltf::Accessor& accessor, int offset);

    //Input data
    tinygltf::Model model;
    //Transformations to apply to the node we're currently importing
    std::vector<float> scale={1.f,1.f,1.f};
    std::vector<float> translate={0.f,0.f,0.f};

public: //TODO: getters
    //Output data
    std::vector<std::array<float,3>> vertices;
    std::vector<std::array<int,3>> polygons;
};
