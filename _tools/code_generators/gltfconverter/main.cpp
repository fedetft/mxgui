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

#include <iostream>
#include <fstream>
#include "tiny_gltf.h"
#include "geometry.h"

using namespace std;
using namespace tinygltf;

int main(int argc, char *argv[])
{
    if(argc<3)
    {
        cerr<<"usage: gltfconverter input_file.glb output_file.h [--quads]\n";
        return 1;
    }
    ofstream out(argv[2]);
    if(!out)
    {
        cerr<<"can't open output file\n";
        return 1;
    }
    Model model;
    TinyGLTF loader;
    string err;
    string warn;
    //bool ret = loader.LoadASCIIFromFile(&model,&err,&warn,argv[1]);
    bool ret=loader.LoadBinaryFromFile(&model,&err,&warn,argv[1]);
    if(!warn.empty()) cout<<"Warn: "<<warn<<"\n";
    if(!err.empty()) cout<<"Err: "<<err<<"\n";
    if(!ret) return -1;

    Geometry geometry(model);
    PrimitiveType p=PrimitiveType::TRIANGLES;
    if(argc==4 && string(argv[3])=="--quads") p=PrimitiveType::QUADS;
    geometry.dump(out,p);
    return 0;
}
