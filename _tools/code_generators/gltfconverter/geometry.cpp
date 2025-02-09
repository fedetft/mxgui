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

//NOTE: tiny_gltf has no documentation, only examples/glview/glview.cc
//and https://www.khronos.org/files/gltf20-reference-guide.pdf

#include <iostream>
#include "geometry.h"
#include "gltf_helper.h"

using namespace std;
using namespace tinygltf;

Geometry::Geometry(Model model) : model(model)
{
    auto& scene=model.scenes.at(model.defaultScene>-1 ? model.defaultScene : 0);
    cerr<<"Scene: "<<scene.name<<"\n";
    for(int& nodeId : scene.nodes) importNode(nodeId);
}

void Geometry::importNode(int nodeId)
{
    cerr<<"Node "<<nodeId<<": "<<model.nodes.at(nodeId).name<<"\n";
    auto& node=model.nodes.at(nodeId);

    //TODO: implement rotations and generic matrix rototranslations?
    if(node.rotation.size()==4) cerr<<"TODO rotation unimplemented\n";
    if(node.scale.size()!=3) scale={1.f,1.f,1.f};
    else scale=vector<float>(begin(node.scale),end(node.scale));
    if(node.translation.size()!=3) translate={0.f,0.f,0.f};
    else translate=vector<float>(begin(node.translation),end(node.translation));
    if(node.matrix.size()==16) cerr<<"TODO rototranslation unimplemented\n";

    auto& mesh=model.meshes.at(node.mesh);
    for(const Primitive& primitive : mesh.primitives)
    {
        cerr<<"Primitive: "<<getMode(primitive.mode)<<"\n";
        if(primitive.indices<0 || primitive.mode!=TINYGLTF_MODE_TRIANGLES)
        {
            cerr<<"TODO: Currently only supporting triangles\n";
            continue;
        }

        //NOTE: we must cache the vertices size now before we add new ones
        //TODO: what if multiple nodes use vertices with the same values? we
        //could garbage-collect vertices and update polygons to index the
        //de-duplicated ones.
        int offset=vertices.size();
        for(auto& attrib : primitive.attributes)
        {
            //Primitives have POSITION, NORMAL, TEXCOORD_0, only need POSITION
            if(attrib.first!="POSITION") continue;
            auto& accessor=model.accessors.at(attrib.second);
            cerr<<"Vertices are "
                <<accessor.count<<"*"<<getType(accessor.type)
                <<" "<<getComponentType(accessor.componentType)<<"\n";
            importVertices(accessor);
        }

        auto& accessor=model.accessors.at(primitive.indices);
        cerr<<"Indices are "
            <<accessor.count<<"*"<<getType(accessor.type)
            <<" "<<getComponentType(accessor.componentType)<<"\n";
        importTriangles(accessor,offset);
    }
    //Recursively import child nodes
    for(int child : node.children) importNode(child);
}

void Geometry::importVertices(const Accessor& accessor)
{
    auto& bufferView=model.bufferViews.at(accessor.bufferView);
    auto& buffer=model.buffers.at(bufferView.buffer);
    const unsigned char *b=buffer.data.data()+bufferView.byteOffset;
    const unsigned char *e=b+bufferView.byteLength;
    //TODO: what are bufferView.byteStride and bufferView.target for?
    int index=0;
    int type=accessor.componentType;
    for(int i=0;i<accessor.count;i++)
    {
        //NOTE: Y and Z are swapped, go figure...
        vertices.push_back({
            typedValueCast<float>(b,e,type,index+0)*scale.at(0)+translate.at(0),
            typedValueCast<float>(b,e,type,index+2)*scale.at(2)+translate.at(2),
            typedValueCast<float>(b,e,type,index+1)*scale.at(1)+translate.at(1)
        });
        index+=3;
    }
}

void Geometry::importTriangles(const Accessor& accessor, int offset)
{
    auto& bufferView=model.bufferViews.at(accessor.bufferView);
    auto& buffer=model.buffers.at(bufferView.buffer);
    const unsigned char *b=buffer.data.data()+bufferView.byteOffset;
    const unsigned char *e=b+bufferView.byteLength;
    //TODO: what are bufferView.byteStride and bufferView.target for?
    if(getType(accessor.type)!=1) throw runtime_error("expecting scalar");
    int index=0, point=0;
    array<int,3> triangle;
    for(int i=0;i<accessor.count;i++)
    {
        triangle.at(point++)=typedValueCast<int>(b,e,accessor.componentType,index++)+offset;
        if(point<3) continue;
        point=0;
        polygons.push_back(triangle);
    }
}

void Geometry::dump(std::ostream& out, PrimitiveType primitiveType)
{
    out<<"\n#pragma once\n\n"
       <<"static const int numVertices="<<vertices.size()<<";\n"
       <<"static const int numPolygons="<<polygons.size()<<";\n\n"
       <<"static const float vertices[]=\n{\n";
    for(int i=0;i<vertices.size();i++)
    {
        out<<"\t"<<vertices[i][0]<<", "<<vertices[i][1]<<", "<<vertices[i][2];
        if(i==vertices.size()-1) out<<"\n"; else out<<",\n";
    }
    out<<"};\n\nstatic const short polygons[]=\n{\n";
    for(int i=0;i<polygons.size();i++)
    {
        out<<"\t"<<polygons[i][0]<<", "<<polygons[i][1]<<", "<<polygons[i][2];
        //Miosix renderer can import a mesh with a mix of both triangles and
        //quads, every polygon has 4 points, and triangles have -1 for the
        //last point
        if(primitiveType==PrimitiveType::QUADS) out<<", -1";
        if(i==polygons.size()-1) out<<"\n"; else out<<",\n";
    }
    out<<"};\n";
}
