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

#include <cmath>

#ifndef MATH_HELPERS_H
#define	MATH_HELPERS_H

/**
 * Square matrix
 * \param T type of an element of the matrix
 * \param n matrix order
 */
template<typename T, int n>
class matrix_base
{
public:
    /**
     * Default constructor, yields a matrix with uninitialized values for speed
     * reasons
     */
    matrix_base() {}

    /**
     * Construct a matrix from an array of elements
     * \param v array of elements. Its size must be n*n
     */
    explicit matrix_base(const T *v) { for(int i=0;i<n*n;i++) data[i]=v[i]; }

    /**
     * \param row matrix row
     * \param col matrix colums
     * \return value[row][col]
     */
    T& at(unsigned int row, unsigned int col) { return data[n*row+col]; }

    /**
     * \param row matrix row
     * \param col matrix colums
     * \return value[row][col]
     */
    T  at(unsigned int row, unsigned int col) const { return data[n*row+col]; }

    /**
     * Matrix sum
     */
    matrix_base operator+(const matrix_base& rhs) const
    {
        matrix_base result;
        for(int i=0;i<n*n;i++) result.data[i]=this->data[i]+rhs.data[i];
        return result;
    }

    /**
     * Matrix sum
     */
    const matrix_base& operator+=(const matrix_base& rhs)
    {
        for(int i=0;i<n*n;i++) this->data[i]+=rhs.data[i];
        return *this;
    }

    /**
     * Matrix product
     */
    matrix_base operator*(const matrix_base& rhs) const
    {
        matrix_base result;
        result.clear();
        for(int i=0;i<n;i++)
            for(int j=0;j<n;j++)
                for(int k=0;k<n;k++) result.at(i,j)+=this->at(i,k)*rhs.at(k,j);
        return result;
    }

    /**
     * Matrix product
     */
    const matrix_base& operator*=(const matrix_base& rhs)
    {
        return *this=*this*rhs;
    }

    /**
     * Set all elements to zero
     */
    void clear() { for(int i=0;i<n*n;i++) data[i]=0; }

    /**
     * Set the matrix to the identity matrix
     */
    void eye() { for(int i=0;i<n;i++) for(int j=0;j<n;j++) at(i,j)= i==j?1:0; }

    //Uses default copy constructor and operator=
private:
    T data[n*n];
};

/**
 * Column vector
 * \param T type of an element of the vector
 * \param n vector size
 */
template<typename T, int n>
class vector_base
{
public:
    /**
     * Default constructor, yields a vector with uninitialized values for speed
     * reasons
     */
    vector_base() {}

    /**
     * Construct a vector from an array of elements
     * \param v array of elements. Its size must be n
     */
    explicit vector_base(const T *v) { for(int i=0;i<n;i++) data[i]=v[i]; }

    /**
     * \param row vector row
     * \return value[row]
     */
    T& at(unsigned int row) { return data[row]; }

    /**
     * \param row vector row
     * \return value[row]
     */
    T  at(unsigned int row) const { return data[row]; }

    /**
     * Vector sum
     */
    vector_base operator+(const vector_base& rhs) const
    {
        vector_base result;
        for(int i=0;i<n;i++) result.data[i]=this->data[i]+rhs.data[i];
        return result;
    }

    /**
     * Vector sum
     */
    const vector_base& operator+=(const vector_base& rhs)
    {
        for(int i=0;i<n;i++) this->data[i]+=rhs.data[i];
        return *this;
    }

    /**
     * Vector product
     */
    vector_base operator*(const vector_base& rhs) const
    {
        vector_base result;
        for(int i=0;i<n;i++) result.data[i]=this->data[i]*rhs.data[i];
        return result;
    }

    /**
     * Vector product
     */
    const vector_base& operator*=(const vector_base& rhs)
    {
        for(int i=0;i<n;i++) this->data[i]*=rhs.data[i];
        return *this;
    }

    /**
     * Set all elements to zero
     */
    void clear() { for(int i=0;i<n;i++) data[i]=0; }

    //Uses default copy constructor and operator=
private:
    T data[n];
};

/**
 * Matrix * vector multiplication
 */
template<typename T, int n>
vector_base<T,n> operator*(const matrix_base<T,n>& m, const vector_base<T,n>& v)
{
    vector_base<T,n> result;
    result.clear();
    for(int i=0;i<n;i++) for(int j=0;j<n;j++) result.at(i)+=m.at(i,j)*v.at(j);
    return result;
}

typedef matrix_base<float,3> Matrix3f; ///< 3 dimensional matrix of float
typedef vector_base<float,3> Vector3f; ///< 3 dimensional vector of float

/**
 * x axis rotation
 * \param angle rotation angle
 */
inline Matrix3f xrot(float angle)
{
    using namespace std;
    float rotation[]=
    {
        1.0f,       0.0f,        0.0f,
        0.0f, cos(angle), -sin(angle),
        0.0f, sin(angle),  cos(angle)
    };
    return Matrix3f(rotation);
}

/**
 * y axis rotation
 * \param angle rotation angle
 */
inline Matrix3f yrot(float angle)
{
    using namespace std;
    float rotation[]=
    {
         cos(angle), 0.0f, sin(angle),
               0.0f, 1.0f,       0.0f,
        -sin(angle), 0.0f,  cos(angle)
    };
    return Matrix3f(rotation);
}

/**
 * z axis rotation
 * \param angle rotation angle
 */
inline Matrix3f zrot(float angle)
{
    using namespace std;
    float rotation[]=
    {
        cos(angle), -sin(angle), 0.0f,
        sin(angle),  cos(angle), 0.0f,
              0.0f,        0.0f, 1.0f
    };
    return Matrix3f(rotation);
}

/**
 * Scaling matrix
 * \param ratio scale ratio, must be >=0 for the transformation to be a scaling
 */
inline Matrix3f scale(float ratio)
{
    float scaling[]=
    {
        ratio,  0.0f,  0.0f,
         0.0f, ratio,  0.0f,
         0.0f,  0.0f, ratio
    };
    return Matrix3f(scaling);
}

#endif //MATH_HELPERS_H
