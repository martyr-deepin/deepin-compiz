/*
 * Copyright © 2008 Danny Baumann
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Dennis Kasprzyk not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Dennis Kasprzyk makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * DENNIS KASPRZYK DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL DENNIS KASPRZYK BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Authors: Danny Baumann <maniac@compiz-fusion.org>
 */

/*
 * Copyright (C) 1999-2005  Brian Paul   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * From Mesa 3-D graphics library.
 */

#include <core/output.h>

#include <opengl/matrix.h>

#include <string.h>
#include <math.h>
#include <cassert>

#define DEG2RAD (M_PI / 180.0f)

/**
 * Identity matrix.
 */
static const float identity[16] = {
    1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0
};

#define A(row, col) a[(col << 2) + row]
#define B(row, col) b[(col << 2) + row]
#define P(row, col) product[(col << 2) + row]

/**
 * Perform a full 4x4 matrix multiplication.
 *
 * \param a matrix.
 * \param b matrix.
 * \param product will receive the product of \p a and \p b.
 *
 * \warning Is assumed that \p product != \p b. \p product == \p a is allowed.
 *
 * \note KW: 4*16 = 64 multiplications
 *
 * \author This \c matmul was contributed by Thomas Malik
 */
static void
matmul4 (float       *product,
	 const float *a,
	 const float *b)
{
    int i;

    for (i = 0; i < 4; i++)
    {
	const float ai0 = A(i,0), ai1 = A(i,1), ai2 = A(i,2), ai3 = A(i,3);

	P(i,0) = ai0 * B(0,0) + ai1 * B(1,0) + ai2 * B(2,0) + ai3 * B(3,0);
	P(i,1) = ai0 * B(0,1) + ai1 * B(1,1) + ai2 * B(2,1) + ai3 * B(3,1);
	P(i,2) = ai0 * B(0,2) + ai1 * B(1,2) + ai2 * B(2,2) + ai3 * B(3,2);
	P(i,3) = ai0 * B(0,3) + ai1 * B(1,3) + ai2 * B(2,3) + ai3 * B(3,3);
    }
}

GLMatrix::GLMatrix ()
{
    memcpy (m, identity, sizeof (m));
}

GLMatrix::GLMatrix (const float *mat)
{
    memcpy (m, mat, sizeof (m));
}

void
GLMatrix::reset ()
{
    memcpy (m, identity, sizeof (m));
}

const float *
GLMatrix::getMatrix () const
{
    return m;
}

GLMatrix&
GLMatrix::operator*= (const GLMatrix& rhs)
{
    *this = *this * rhs;

    return *this;
}

GLMatrix
operator* (const GLMatrix& lhs,
	   const GLMatrix& rhs)
{
    GLMatrix result;

    matmul4 (result.m, lhs.m, rhs.m);

    return result;
}

GLVector
operator* (const GLMatrix& lhs,
	   const GLVector& rhs)
{
    GLVector  result;
    const float *a = lhs.m;
    int         i;

    for (i = 0; i < 4; i++)
    {
	result[i] = A(i,0) * rhs[0] + A(i,1) * rhs[1] +
	            A(i,2) * rhs[2] + A(i,3) * rhs[3];
    }

    return result;
}

#undef A
#undef B
#undef P

/*
** Invert 4x4 matrix.
** Contributed by David Moore (See Mesa bug #6748)
*/
bool GLMatrix::invert ()
{
    float inv[16], det;
    int i;

    inv[0] =   m[5]*m[10]*m[15] - m[5]*m[11]*m[14] - m[9]*m[6]*m[15]
             + m[9]*m[7]*m[14] + m[13]*m[6]*m[11] - m[13]*m[7]*m[10];
    inv[4] =  -m[4]*m[10]*m[15] + m[4]*m[11]*m[14] + m[8]*m[6]*m[15]
             - m[8]*m[7]*m[14] - m[12]*m[6]*m[11] + m[12]*m[7]*m[10];
    inv[8] =   m[4]*m[9]*m[15] - m[4]*m[11]*m[13] - m[8]*m[5]*m[15]
             + m[8]*m[7]*m[13] + m[12]*m[5]*m[11] - m[12]*m[7]*m[9];
    inv[12] = -m[4]*m[9]*m[14] + m[4]*m[10]*m[13] + m[8]*m[5]*m[14]
             - m[8]*m[6]*m[13] - m[12]*m[5]*m[10] + m[12]*m[6]*m[9];
    inv[1] =  -m[1]*m[10]*m[15] + m[1]*m[11]*m[14] + m[9]*m[2]*m[15]
             - m[9]*m[3]*m[14] - m[13]*m[2]*m[11] + m[13]*m[3]*m[10];
    inv[5] =   m[0]*m[10]*m[15] - m[0]*m[11]*m[14] - m[8]*m[2]*m[15]
             + m[8]*m[3]*m[14] + m[12]*m[2]*m[11] - m[12]*m[3]*m[10];
    inv[9] =  -m[0]*m[9]*m[15] + m[0]*m[11]*m[13] + m[8]*m[1]*m[15]
             - m[8]*m[3]*m[13] - m[12]*m[1]*m[11] + m[12]*m[3]*m[9];
    inv[13] =  m[0]*m[9]*m[14] - m[0]*m[10]*m[13] - m[8]*m[1]*m[14]
             + m[8]*m[2]*m[13] + m[12]*m[1]*m[10] - m[12]*m[2]*m[9];
    inv[2] =   m[1]*m[6]*m[15] - m[1]*m[7]*m[14] - m[5]*m[2]*m[15]
             + m[5]*m[3]*m[14] + m[13]*m[2]*m[7] - m[13]*m[3]*m[6];
    inv[6] =  -m[0]*m[6]*m[15] + m[0]*m[7]*m[14] + m[4]*m[2]*m[15]
             - m[4]*m[3]*m[14] - m[12]*m[2]*m[7] + m[12]*m[3]*m[6];
    inv[10] =  m[0]*m[5]*m[15] - m[0]*m[7]*m[13] - m[4]*m[1]*m[15]
             + m[4]*m[3]*m[13] + m[12]*m[1]*m[7] - m[12]*m[3]*m[5];
    inv[14] = -m[0]*m[5]*m[14] + m[0]*m[6]*m[13] + m[4]*m[1]*m[14]
             - m[4]*m[2]*m[13] - m[12]*m[1]*m[6] + m[12]*m[2]*m[5];
    inv[3] =  -m[1]*m[6]*m[11] + m[1]*m[7]*m[10] + m[5]*m[2]*m[11]
             - m[5]*m[3]*m[10] - m[9]*m[2]*m[7] + m[9]*m[3]*m[6];
    inv[7] =   m[0]*m[6]*m[11] - m[0]*m[7]*m[10] - m[4]*m[2]*m[11]
             + m[4]*m[3]*m[10] + m[8]*m[2]*m[7] - m[8]*m[3]*m[6];
    inv[11] = -m[0]*m[5]*m[11] + m[0]*m[7]*m[9] + m[4]*m[1]*m[11]
             - m[4]*m[3]*m[9] - m[8]*m[1]*m[7] + m[8]*m[3]*m[5];
    inv[15] =  m[0]*m[5]*m[10] - m[0]*m[6]*m[9] - m[4]*m[1]*m[10]
             + m[4]*m[2]*m[9] + m[8]*m[1]*m[6] - m[8]*m[2]*m[5];

    det = m[0]*inv[0] + m[1]*inv[4] + m[2]*inv[8] + m[3]*inv[12];
    if (det == 0)
        return false;

    det = 1.0f / det;

    for (i = 0; i < 16; i++)
        m[i] = inv[i] * det;

    return true;
}

/**
 * Generate a 4x4 transformation matrix from glRotate parameters, and
 * post-multiply the input matrix by it.
 *
 * \author
 * This function was contributed by Erich Boleyn (erich@uruk.org).
 * Optimizations contributed by Rudolf Opalla (rudi@khm.de).
 */
void
GLMatrix::rotate (const float angle,
		    const float xRot,
		    const float yRot,
		    const float zRot)
{
    float x = xRot, y = yRot, z = zRot;
    float s, c;
    float matrix[16];
    bool  optimized;

    s = (float) sin (angle * DEG2RAD);
    c = (float) cos (angle * DEG2RAD);

    memcpy (matrix, identity, sizeof (matrix));
    optimized = false;

#define M(row, col)  matrix[col * 4 + row]

    if (x == 0.0f)
    {
	if (y == 0.0f)
	{
	    if (z != 0.0f)
	    {
		optimized = true;
		/* rotate only around z-axis */
		M(0,0) = c;
		M(1,1) = c;
		if (z < 0.0f)
		{
		    M(0,1) = s;
		    M(1,0) = -s;
		}
		else
		{
		    M(0,1) = -s;
		    M(1,0) = s;
		}
	    }
	}
	else if (z == 0.0f)
	{
	    optimized = true;
	    /* rotate only around y-axis */
	    M(0,0) = c;
	    M(2,2) = c;
	    if (y < 0.0f)
	    {
		M(0,2) = -s;
		M(2,0) = s;
	    }
	    else
	    {
		M(0,2) = s;
		M(2,0) = -s;
	    }
	}
    }
    else if (y == 0.0f)
    {
	if (z == 0.0f)
	{
	    optimized = true;
	    /* rotate only around x-axis */
	    M(1,1) = c;
	    M(2,2) = c;
	    if (x < 0.0f)
	    {
		M(1,2) = s;
		M(2,1) = -s;
	    }
	    else
	    {
		M(1,2) = -s;
		M(2,1) = s;
	    }
	}
    }

    if (!optimized)
    {
	float xx, yy, zz, xy, yz, zx, xs, ys, zs, one_c;
	const float mag = sqrtf (x * x + y * y + z * z);

	if (mag <= 1.0e-4)
	{
	    /* no rotation, leave mat as-is */
	    return;
	}

	x /= mag;
	y /= mag;
	z /= mag;


	/*
	 *     Arbitrary axis rotation matrix.
	 *
	 *  This is composed of 5 matrices, Rz, Ry, T, Ry', Rz', multiplied
	 *  like so:  Rz * Ry * T * Ry' * Rz'.  T is the final rotation
	 *  (which is about the X-axis), and the two composite transforms
	 *  Ry' * Rz' and Rz * Ry are (respectively) the rotations necessary
	 *  from the arbitrary axis to the X-axis then back.  They are
	 *  all elementary rotations.
	 *
	 *  Rz' is a rotation about the Z-axis, to bring the axis vector
	 *  into the x-z plane.  Then Ry' is applied, rotating about the
	 *  Y-axis to bring the axis vector parallel with the X-axis.  The
	 *  rotation about the X-axis is then performed.  Ry and Rz are
	 *  simply the respective inverse transforms to bring the arbitrary
	 *  axis back to it's original orientation.  The first transforms
	 *  Rz' and Ry' are considered inverses, since the data from the
	 *  arbitrary axis gives you info on how to get to it, not how
	 *  to get away from it, and an inverse must be applied.
	 *
	 *  The basic calculation used is to recognize that the arbitrary
	 *  axis vector (x, y, z), since it is of unit length, actually
	 *  represents the sines and cosines of the angles to rotate the
	 *  X-axis to the same orientation, with theta being the angle about
	 *  Z and phi the angle about Y (in the order described above)
	 *  as follows:
	 *
	 *  cos ( theta ) = x / sqrt ( 1 - z^2 )
	 *  sin ( theta ) = y / sqrt ( 1 - z^2 )
	 *
	 *  cos ( phi ) = sqrt ( 1 - z^2 )
	 *  sin ( phi ) = z
	 *
	 *  Note that cos ( phi ) can further be inserted to the above
	 *  formulas:
	 *
	 *  cos ( theta ) = x / cos ( phi )
	 *  sin ( theta ) = y / sin ( phi )
	 *
	 *  ...etc.  Because of those relations and the standard trigonometric
	 *  relations, it is pssible to reduce the transforms down to what
	 *  is used below.  It may be that any primary axis chosen will give the
	 *  same results (modulo a sign convention) using thie method.
	 *
	 *  Particularly nice is to notice that all divisions that might
	 *  have caused trouble when parallel to certain planes or
	 *  axis go away with care paid to reducing the expressions.
	 *  After checking, it does perform correctly under all cases, since
	 *  in all the cases of division where the denominator would have
	 *  been zero, the numerator would have been zero as well, giving
	 *  the expected result.
	 */

	xx = x * x;
	yy = y * y;
	zz = z * z;
	xy = x * y;
	yz = y * z;
	zx = z * x;
	xs = x * s;
	ys = y * s;
	zs = z * s;
	one_c = 1.0f - c;

	/* We already hold the identity-matrix so we can skip some statements */
	M(0,0) = (one_c * xx) + c;
	M(0,1) = (one_c * xy) - zs;
	M(0,2) = (one_c * zx) + ys;
/*    M(0,3) = 0.0F; */

	M(1,0) = (one_c * xy) + zs;
	M(1,1) = (one_c * yy) + c;
	M(1,2) = (one_c * yz) - xs;
/*    M(1,3) = 0.0F; */

	M(2,0) = (one_c * zx) - ys;
	M(2,1) = (one_c * yz) + xs;
	M(2,2) = (one_c * zz) + c;
/*    M(2,3) = 0.0F; */

/*
  M(3,0) = 0.0F;
  M(3,1) = 0.0F;
  M(3,2) = 0.0F;
  M(3,3) = 1.0F;
*/
    }
#undef M

    matmul4 (m, m, matrix);
}

void
GLMatrix::rotate (const float       angle,
		    const GLVector& vector)
{
    rotate (angle,
	    vector[GLVector::x],
	    vector[GLVector::y],
	    vector[GLVector::z]);
}

/**
 * Multiply a matrix with a general scaling matrix.
 *
 * \param matrix matrix.
 * \param x x axis scale factor.
 * \param y y axis scale factor.
 * \param z z axis scale factor.
 *
 * Multiplies in-place the elements of \p matrix by the scale factors.
 */
void
GLMatrix::scale (const float x,
		   const float y,
		   const float z)
{
    m[0] *= x; m[4] *= y; m[8]  *= z;
    m[1] *= x; m[5] *= y; m[9]  *= z;
    m[2] *= x; m[6] *= y; m[10] *= z;
    m[3] *= x; m[7] *= y; m[11] *= z;
}

void
GLMatrix::scale (const GLVector& vector)
{
    scale (vector[GLVector::x],
	   vector[GLVector::y],
	   vector[GLVector::z]);
}

/**
 * Multiply a matrix with a translation matrix.
 *
 * \param matrix matrix.
 * \param x translation vector x coordinate.
 * \param y translation vector y coordinate.
 * \param z translation vector z coordinate.
 *
 * Adds the translation coordinates to the elements of \p matrix in-place.
 */
void
GLMatrix::translate (const float x,
		       const float y,
		       const float z)
{
    m[12] = m[0] * x + m[4] * y + m[8]  * z + m[12];
    m[13] = m[1] * x + m[5] * y + m[9]  * z + m[13];
    m[14] = m[2] * x + m[6] * y + m[10] * z + m[14];
    m[15] = m[3] * x + m[7] * y + m[11] * z + m[15];
}

void
GLMatrix::translate (const GLVector& vector)
{
    translate (vector[GLVector::x],
	       vector[GLVector::y],
	       vector[GLVector::z]);
}

void
GLMatrix::toScreenSpace (const CompOutput *output,
			 float            z)
{
    translate (-0.5f, -0.5f, z);
    scale (1.0f / output->width (), -1.0f / output->height (), 1.0f);
    translate (-output->x1 (), -output->y2 (), 0.0f);
}

float&
GLMatrix::operator[] (unsigned int pos)
{
    assert (pos <= 15);
    return m[pos];
}
