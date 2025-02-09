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
#include <cassert>
#include "tiny_gltf.h"

using namespace std;
using namespace tinygltf;

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

template<typename T>
void dumpValue(const T* array, const unsigned char *e, int index, double a, double b)
{
    assert(reinterpret_cast<const unsigned char*>(array+index)<e);
    if(a==1. && b==0.) cout<<array[index]<<", ";
    else cout<<a*array[index]+b<<", ";
}

void dumpTypedValue(const unsigned char *s, const unsigned char *e, int type, int index, double a=1., double b=0.)
{
    switch(type)
    {
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
            dumpValue(reinterpret_cast<const uint8_t*>(s),e,index,a,b);
            break;
        case TINYGLTF_COMPONENT_TYPE_SHORT:
            dumpValue(reinterpret_cast<const int16_t*>(s),e,index,a,b);
            break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
            dumpValue(reinterpret_cast<const uint16_t*>(s),e,index,a,b);
            break;
        case TINYGLTF_COMPONENT_TYPE_INT:
            dumpValue(reinterpret_cast<const int32_t*>(s),e,index,a,b);
            break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
            dumpValue(reinterpret_cast<const uint32_t*>(s),e,index,a,b);
            break;
        case TINYGLTF_COMPONENT_TYPE_FLOAT:
            dumpValue(reinterpret_cast<const float*>(s),e,index,a,b);
            break;
        case TINYGLTF_COMPONENT_TYPE_DOUBLE:
            dumpValue(reinterpret_cast<const double*>(s),e,index,a,b);
            break;
    }
}

void dumpPosition(const Model& model, const Accessor& accessor,
                  vector<double> scale, vector<double> translate)
{
    auto& bufferView=model.bufferViews.at(accessor.bufferView);
    auto& buffer=model.buffers.at(bufferView.buffer);
    const unsigned char *s=buffer.data.data()+bufferView.byteOffset;
    const unsigned char *e=s+bufferView.byteLength;
    //TODO: what are bufferView.byteStride and bufferView.target for?
    int index=0;
    for(int i=0;i<accessor.count;i++)
    {
        //Y and Z are swapped, go figure...
        dumpTypedValue(s,e,accessor.componentType,index+0,scale.at(0),translate.at(0));
        dumpTypedValue(s,e,accessor.componentType,index+2,scale.at(2),translate.at(2));
        dumpTypedValue(s,e,accessor.componentType,index+1,scale.at(1),translate.at(1));
        index+=3;
        cout<<"\n";
    }
}

void dumpTriangles(const Model& model, const Accessor& accessor, int offset)
{
    auto& bufferView=model.bufferViews.at(accessor.bufferView);
    auto& buffer=model.buffers.at(bufferView.buffer);
    const unsigned char *s=buffer.data.data()+bufferView.byteOffset;
    const unsigned char *e=s+bufferView.byteLength;
    //TODO: what are bufferView.byteStride and bufferView.target for?
    int index=0;
    for(int i=0;i<accessor.count;i++)
    {
        for(int j=0;j<getType(accessor.type);j++)
        {
            dumpTypedValue(s,e,accessor.componentType,index++,1.,offset);
        }
        if(index % 3==0) cout<<"-1,\n";
    }
}

int numVertices=0; //TODO: remove global variables
int numPolygons=0;

void dumpNode(const Model& model, int nodeId)
{
    cerr<<"Node "<<nodeId<<": "<<model.nodes.at(nodeId).name<<"\n";
    auto& node=model.nodes.at(nodeId);
    if(node.rotation.size()==4) cerr<<"TODO rotation unimplemented\n";
    vector<double> scale={1.,1.,1.};
    if(node.scale.size()==3) scale=node.scale;
    vector<double> translate={0.,0.,0.};
    if(node.translation.size()==3) translate=node.translation;
    if(node.matrix.size()==16) cerr<<"TODO rototranslation unimplemented\n";
    auto& mesh=model.meshes.at(node.mesh);
    int vertices,polygons;
    for(const Primitive& primitive : mesh.primitives)
    {
        if(primitive.indices<0) return;
        cerr<<"Primitive: "<<getMode(primitive.mode)<<"\n";
        //Currently only supporting triangles
        if(primitive.mode!=TINYGLTF_MODE_TRIANGLES) continue;

        for(auto& attrib : primitive.attributes)
        {
            //Primitives have POSITION, NORMAL, TEXCOORD_0, only need first
            if(attrib.first!="POSITION") continue;
            auto& accessor=model.accessors.at(attrib.second);
            cerr<<"Vertices are "
                <<accessor.count<<"*"<<getType(accessor.type)
                <<" "<<getComponentType(accessor.componentType)<<"\n";
            vertices=accessor.count;
            dumpPosition(model,accessor,scale,translate);
        }

        auto& accessor=model.accessors.at(primitive.indices);
            cerr<<"Indices are "
                <<accessor.count<<"*"<<getType(accessor.type)
                <<" "<<getComponentType(accessor.componentType)<<"\n";
        polygons=accessor.count/3;
        dumpTriangles(model,accessor,numVertices);

        numVertices+=vertices;
        numPolygons+=polygons;
    }
    //Recursively dump child nodes
    for(int child : node.children) dumpNode(model, child);
}

int main(int argc, char *argv[])
{
    if(argc!=2) return 1;
    Model model;
    TinyGLTF loader;
    std::string err;
    std::string warn;
    //bool ret = loader.LoadASCIIFromFile(&model,&err,&warn,argv[1]);
    bool ret=loader.LoadBinaryFromFile(&model,&err,&warn,argv[1]);
    if(!warn.empty()) cout<<"Warn: "<<warn<<"\n";
    if(!err.empty()) cout<<"Err: "<<err<<"\n";
    if(!ret) return -1;

    auto& scene=model.scenes.at(model.defaultScene>-1 ? model.defaultScene : 0);
    cerr<<"Scene: "<<scene.name<<"\n";
    for(int& nodeId : scene.nodes) dumpNode(model,nodeId);
    cout<<"static const int numVertices="<<numVertices<<";\n";
    cout<<"static const int numPolygons="<<numPolygons<<";\n";
    return 0;
}
