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

#include <vector>
#include "mxgui/display.h"
#include "math_helpers.h"
#include "triangle_fsm.h"

#ifndef RENDERING_ENGINE_H
#define	RENDERING_ENGINE_H

/**
 * Base class for a rendering engine
 */
class RenderingEngine
{
public:
    /**
     * Constructor
     */
    RenderingEngine();

    /**
     * Set the 3D model to render
     * \param vertices array of triples of floats representing x,y,z coordinates
     * of the vertices of the 3D model. Ownership of the array remains of the
     * caller, that is responsible to keep the pointer valid as long as this
     * class needs to reference it, and to deallocate it afterwards
     * \param numVertices number of vertices of the model
     * \param polygons array of quadruples of short representing polygon
     * vertices. If the polygon is a quad, all four elements are positve or
     * zero, if the polygon is a triangle, the last element of the quadruple is
     * negative. Note that positive or zero values represent indexes into the
     * vertices array. Ownership of the array remains of the caller, that is
     * responsible to keep the pointer valid as long as this class needs to
     * reference it, and to deallocate it afterwards
     * \param numPolygons number of polygons of the model
     */
    void setModel(const float *vertices, int numVertices,
            const short *polygons, int numPolygons);

    /**
     * Set the display area where to render the model
     * \param a upper left point
     * \param b lower right point
     */
    void setDrawArea(mxgui::Point a, mxgui::Point b);

    /**
     * \param t the coordinates of this point are summed to the vertices after
     * they have been "flattened" to 2D points, allowing to perform a
     * translation useful to center the image in screen
     */
    void setTranslation(mxgui::Point t) { this->translation=t; }

    /**
     * \param xfm transformation matrix to apply to each vertex before rendering
     */
    void setTransformMatrix(Matrix3f xfm) { this->xfm=xfm; }

    /**
     * Perform the rendering
     * \param disp display where to draw the 3D model
     */
    void render(mxgui::Display& disp);

    /**
     * Destructor
     */
    virtual ~RenderingEngine();

protected:
    /**
     * Called when the 3D model is changed
     */
    virtual void updateModel();

    /**
     * Subclasses should override this to perform the rendering
     * \param disp display where to draw the 3D model
     */
    virtual void doRender(mxgui::Display& disp)=0;

    const float *vertices;
    int numVertices;
    const short *polygons;
    int numPolygons;
    mxgui::Point a,b,translation;
    Matrix3f xfm;

private:
    RenderingEngine(const RenderingEngine&);
    const RenderingEngine& operator=(const RenderingEngine&);

    /**
     * Validate if passed information is correct, and set valid accordingly
     */
    void validate();

    bool valid;
};

/**
 * Wireframe rendering engine
 */
class WireframeRenderingEngine : public RenderingEngine
{
protected:
    /**
     * Called when the 3D model is changed
     */
    virtual void updateModel();

    /**
     * Perform the rendering
     * \param disp display where to draw the 3D model
     */
    virtual void doRender(mxgui::Display& disp);

private:
    /**
     * \return a dynamically allocated array of x,y tuples with the vertces
     * of the model transformed by the transformation matrix.
     * Don't forget to deallocate the return value when you are done with it
     */
    short *computeTransformVerticesXY();

    /// This variable is updated every time updateModel() is called, it
    /// contains the list of lines in the model, obtained from the list of
    /// polygons in the model. If a line belongs to more than one polygon,
    /// it appears here only once, as an optimization.
    std::vector<std::pair<short,short> > lines;
};

/**
 * Space efficient triangle class. It just contains an index into the
 * polygon list and a "which" field to uniquely identify one of the two
 * triangles that make up a quad, and the minimum y coordinate of its three
 * vertices which has to be precomputed for speed reasons.
 */
class Triangle
{
public:
    /**
     * Constructor
     * \param index index into the polygon list
     * \param which select which of the two triangles that make up a quad
     */
    Triangle(int index, bool which) : index(index), which(which) {}

    /**
     * Update the minimum Y value when the vertices have changed position
     * \param pl polygon list
     * \param vl vertex list
     */
    void updateY(const short *pl, const short *vl)
    {
        using namespace std;
        //Feels like i've messed too much with space-efficient data structures..
        #define deref(x) (vl[3*pl[4*this->index+x]+1])
        if(this->which==0) y=min(deref(0),min(deref(1),deref(2)));
        else               y=min(deref(0),min(deref(2),deref(3)));
        #undef deref
    }
    
    /**
     * \return the minimum Y of a vertex of that triangle
     */
    short minY() const { return y; }

    /**
     * \param pl polygon list
     * \param vl vertex list
     * \return A triangleFSM from the triangle's vertices
     */
    TriangleFSM toFSM(const short *pl, const short *vl,
            const mxgui::Color *colors) const
    {
        short idx[3];
        if(this->which==0)
        {
            idx[0]=pl[4*this->index+0];
            idx[1]=pl[4*this->index+1];
            idx[2]=pl[4*this->index+2];
        } else {
            idx[0]=pl[4*this->index+0];
            idx[1]=pl[4*this->index+2];
            idx[2]=pl[4*this->index+3];
        }
        return TriangleFSM(idx,vl,colors[this->index]);
    }

private:
    unsigned int index:15; ///index into the polygon list
    unsigned int which:1;  ///if 0 consider vertex 1,2,3, else vertex 1,3,4
    short y;
};

/**
 * Sorting function to sort triangles by increasing y coordinate
 * \param a a triangle
 * \param b a triangle
 * \return true if a.minY() < b.minY()
 */
inline bool operator < (Triangle a, Triangle b)
{
    return a.minY() < b.minY();
}

/**
 * Solid rendering engine
 */
class SolidRenderingEngine : public RenderingEngine
{
public:
    /**
     * Constructor
     */
    SolidRenderingEngine();
    
    /**
     * \param polygonColors array of the same size as the number of polygons
     * in the model to specify the color of each polygon. Ownership of the array
     * remains of the caller, that is responsible to keep the pointer valid as
     * long as this class needs to reference it, and to deallocate it afterwards
     */
    void setColors(const mxgui::Color *polygonColors);

protected:
    /**
     * Called when the 3D model is changed
     */
    virtual void updateModel();
    
    /**
     * Perform the rendering
     * \param disp display where to draw the 3D model
     */
    virtual void doRender(mxgui::Display& disp);

private:
    /**
     * \return a dynamically allocated array of x,y,z tuples with the vertces
     * of the model transformed by the transformation matrix.
     * Don't forget to deallocate the return value when you are done with it
     */
    short *computeTransformVerticesXYZ();

    const mxgui::Color *colors;///< Array of colors used to draw polygons
    std::vector<Triangle> triangles;  ///< List of triangles to draw
};

#endif //RENDERING_ENGINE_H
