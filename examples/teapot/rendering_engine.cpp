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

#include <set>
#include <list>
#include <cstring>
//#include <iostream>
#include "mxgui/misc_inst.h"
#include "bresenham_fsm.h"
#include "rendering_engine.h"

using namespace std;
using namespace mxgui;

namespace {
/**
 * A RAII smart pointer for arrays, uncopyable
 */
template<typename T>
class AutoArray
{
public:
    AutoArray(T* ptr) : ptr(ptr) {}
    ~AutoArray() { delete[] ptr; }
    T *get() { return ptr; }
    operator T*() { return ptr; }
private:
    AutoArray(const AutoArray&);
    AutoArray& operator=(const AutoArray&);
    T *ptr;
};
} //anon namespace

//
// class RenderingEngine
//

RenderingEngine::RenderingEngine() : vertices(0), numVertices(0),
        polygons(0), numPolygons(0), a(-1,-1), b(-1,-1), valid(false)
{
    xfm.eye();
}

void RenderingEngine::setModel(const float *vertices, int numVertices,
        const short *polygons, int numPolygons)
{
    this->vertices=vertices;
    this->numVertices=numVertices;
    this->polygons=polygons;
    this->numPolygons=numPolygons;
    if(vertices!=0 && numVertices>0 && polygons!=0 && numPolygons>0)
        updateModel();
    validate();
}

void RenderingEngine::setDrawArea(Point a, Point b)
{
    this->a=a;
    this->b=b;
    validate();
}

void RenderingEngine::render(Display& disp)
{
    if(valid==false) return;
    doRender(disp);
}

RenderingEngine::~RenderingEngine() {}

void RenderingEngine::updateModel() {}

void RenderingEngine::validate()
{
    valid=false;
    if(vertices==0 || numVertices<=0) return;
    if(polygons==0 || numPolygons<=0) return;
    if(a.x()<0 || a.y()<0 || b.x()<=a.x() || b.y()<=a.y()) return;
    valid=true;
}

//
// class WireframeRenderingEngine
//

/**
 * \param a a short
 * \param b a short
 * \return a pair with the two shorts such that first<=second
 */
static inline pair<short,short> sortedPair(short a, short b)
{
    return b>=a ? make_pair(a,b) : make_pair(b,a);
}

void WireframeRenderingEngine::updateModel()
{
    lines.clear();

    //A set is used because it automatically removes duplicate entries and
    //set insertion is O(log n) which makes the whole line extraction and
    //duplicate lines removing O(n*log n). The data is then copied into a
    //vector both for reducing used memory and because a set can't be sorted
    set<pair<short,short> > temp;
    for(int i=0;i<numPolygons;i++)
    {
        const short *polygon=polygons+4*i;
        temp.insert(sortedPair(polygon[0],polygon[1]));
        temp.insert(sortedPair(polygon[1],polygon[2]));
        if(polygon[3]>=0) //Quad or triangle?
        {
            temp.insert(sortedPair(polygon[2],polygon[3]));
            temp.insert(sortedPair(polygon[3],polygon[0]));
        } else temp.insert(sortedPair(polygon[2],polygon[0]));
    }
    
    lines.reserve(temp.size());
    //cout<<temp.size()<<" total lines"<<endl;
    for(set<pair<short, short> >::iterator it=temp.begin();it!=temp.end();++it)
    {
        lines.push_back(*it);
        //cout<<"<"<<it->first<<","<<it->second<<">"<<endl;
    }
}

/**
 * \param line a line identified by the two indexes into the vertex list
 * \param vertices vertex list
 * \return the minimum Y of a line
 */
static inline short minY(pair<short, short> line, const short *vertices)
{
    return min(vertices[2*line.first+1],vertices[2*line.second+1]);
}

/**
 * Small helper class to pass to std::sort() to sort lines by ascending minimum
 * y coordinate
 */
namespace {
class MinYLineSort
{
public:
    MinYLineSort(const short *vertices) : vertices(vertices) {}

    bool operator()(pair<short, short> a, pair<short, short> b)
    {
        return minY(a,vertices) < minY(b,vertices);
    }
private:
    const short *vertices;
};
} //anon namespace

void WireframeRenderingEngine::doRender(Display& disp)
{
    AutoArray<short> xfmVertices(computeTransformVerticesXY());
    MinYLineSort sortOrder(xfmVertices);
    sort(lines.begin(),lines.end(),sortOrder);
    int lineBufferSize=b.x()-a.x()+1;
    AutoArray<Color> lineBuffer(new Color[lineBufferSize]);
    
    DrawingContext dc(disp);
    vector<pair<short,short> >::iterator it=lines.begin();
    short start=minY(*it,xfmVertices);
    if(start>a.y()) dc.clear(a,Point(b.x(),start-1),black);
    list<BresenhamFSM> activeLines;
    //int ml=0;
    for(;start<=b.y();start++)
    {
        memset(lineBuffer,0,sizeof(Color)*lineBufferSize);
        while(it!=lines.end() && minY(*it,xfmVertices)==start)
        {
            Point a(xfmVertices[2*it->first],xfmVertices[2*it->first+1]);
            Point b(xfmVertices[2*it->second],xfmVertices[2*it->second+1]);
            if(a.y()==b.y())
            {
                //Horizontal line, draw it immediately
                int xstart=min(a.x(),b.x());
                int xstop=max(a.x(),b.x());
                for(int i=xstart;i<=xstop;i++) lineBuffer[i]=white;
            } else activeLines.push_back(BresenhamFSM(a,b));
            it++;
        }
        //ml=max<int>(ml,activeLines.size());

        list<BresenhamFSM>::iterator it2;
        for(it2=activeLines.begin();it2!=activeLines.end();)
        {
            if(!it2->drawScanLine(lineBuffer,white)) it2=activeLines.erase(it2);
            else ++it2;
        }
        dc.scanLine(Point(a.x(),start),lineBuffer,lineBufferSize);

        if(activeLines.empty() && it==lines.end()) break;
    }
    if(start<=b.y()) dc.clear(Point(a.x(),start),b,black);
    //cout<<ml<<" max active lines"<<endl;
}

short *WireframeRenderingEngine::computeTransformVerticesXY()
{
    short *result=new short[2*numVertices];
    for(int i=0;i<numVertices;i++)
    {
        Vector3f v=xfm*Vector3f(vertices+3*i);
        result[2*i+0]=translation.x()+v.at(0); //x
        result[2*i+1]=translation.y()-v.at(1); //y
        //cout<<"<"<<result[2*i]<<","<<result[2*i+1]<<">"<<endl;
    }
    return result;
}

//
// class SolidRenderingEngine
//

SolidRenderingEngine::SolidRenderingEngine() : colors(0) {}

void SolidRenderingEngine::setColors(const Color *polygonColors)
{
    this->colors=polygonColors;
}

void SolidRenderingEngine::updateModel()
{
    triangles.clear();
    int numTriangles=numPolygons;
    for(int i=0;i<numPolygons;i++) if(polygons[4*i+3]>=0) numTriangles++;
    triangles.reserve(numTriangles);
    for(int i=0;i<numPolygons;i++)
    {
        triangles.push_back(Triangle(i,false));
        if(polygons[4*i+3]>=0) triangles.push_back(Triangle(i,true));
    }
}

void SolidRenderingEngine::doRender(Display& disp)
{
    if(colors==0) return;
    AutoArray<short> xfmVertices(computeTransformVerticesXYZ());
    vector<Triangle>::iterator it=triangles.begin();
    for(;it!=triangles.end();++it) it->updateY(polygons,xfmVertices);
    sort(triangles.begin(),triangles.end());
    int lineBufferSize=b.x()-a.x()+1;
    AutoArray<Color> lineBuffer(new Color[lineBufferSize]);

    DrawingContext dc(disp);
    it=triangles.begin();
    short start=it->minY();
    if(start>a.y()) dc.clear(a,Point(b.x(),start-1),black);
    list<TriangleFSM> activeTriangles;
    //int mt=0;
    for(;start<=b.y();start++)
    {
        memset(lineBuffer,0,sizeof(Color)*lineBufferSize);
        list<TriangleFSM>::iterator it2;
        for(it2=activeTriangles.begin();it2!=activeTriangles.end();)
        {
            if(!it2->drawScanLine(lineBuffer)) it2=activeTriangles.erase(it2);
            else ++it2;
        }
        dc.scanLine(Point(a.x(),start),lineBuffer,lineBufferSize);

        //Add new triangles after drawing the current scanline as to be able to
        //correctly stack triangles that share vertices the topmost scanline
        //of every triangle isn't drawn on purpose
        while(it!=triangles.end() && it->minY()==start)
        {
            //Insertion-sorting triangles by z coordinate
            //which is faster than inserting at the end and then sorting
            TriangleFSM newTriangle=it->toFSM(polygons,xfmVertices,colors);
            for(it2=activeTriangles.begin();;++it2)
            {
                if(it2==activeTriangles.end() || newTriangle < *it2)
                {
                    activeTriangles.insert(it2,newTriangle);
                    break;
                }
            }
            it++;
        }
        //mt=max<int>(mt,activeTriangles.size());
        if(activeTriangles.empty() && it==triangles.end()) break;
    }
    if(start<=b.y()) dc.clear(Point(a.x(),start),b,black);
    //cout<<mt<<" max active triangles"<<endl;
}

short *SolidRenderingEngine::computeTransformVerticesXYZ()
{
    short *result=new short[3*numVertices];
    for(int i=0;i<numVertices;i++)
    {
        Vector3f v=xfm*Vector3f(vertices+3*i);
        result[3*i+0]=translation.x()+v.at(0); //x
        result[3*i+1]=translation.y()-v.at(1); //y
        result[3*i+2]=(10.0f*v.at(2)); //z
        //cout<<result[3*i+0]<<","<<result[3*i+1]<<","<<result[3*i+2]<<endl;
    }
    return result;
}
