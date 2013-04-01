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

#ifndef _GLVECTOR_H
#define _GLVECTOR_H

/**
 * Class which describes a point or vector
 * in 3D space
 */
class GLVector {
    public:
	typedef enum {
	    x,
	    y,
	    z,
	    w
	} VectorCoordsEnum;

	GLVector ();
	GLVector (float x, float y, float z, float w = 0.0f);

	/**
	 * Returns a reference to the x, y, z or w value by using
	 * 0, 1, 2, 3 as array-access items
	 */
	float& operator[] (int item);

	/**
	 * Returns a reference to the x, y, z or w value by using
	 * x, y, z, w as array-access items
	 */
	float& operator[] (VectorCoordsEnum coord);

	/**
	 * Returns a readonly x, y, z or w value by using
	 * 0, 1, 2, 3 as array-access items
	 */
	const float & operator[] (int item) const;

	/**
	 * Returns a readonly x, y, z or w value by using
	 * x, y, z, w as array-access items
	 */
	const float & operator[] (VectorCoordsEnum coord) const;

	/**
	 * Adds all elements in a GLVector
	 */
	GLVector& operator+= (const GLVector& rhs);

	/**
	 * Subtracts all elements in a GLVector
	 */
	GLVector& operator-= (const GLVector& rhs);

	/**
	 * Scales all elements in a vector
	 * @param k Scale factor
	 */
	GLVector& operator*= (const float k);

	/**
	 * Scales all elements in a vector by 1 / k
	 * @param k Scale factor
	 */
	GLVector& operator/= (const float k);
	GLVector& operator^= (const GLVector& rhs);

	/**
	 * Returns the norm of this vector
	 */
	float norm ();

	/**
	 * Returns the normalized version of the vector
	 */
	GLVector& normalize ();
	
	/**
	 * Returns the homogenized version of the vector
	 */
	GLVector& homogenize ();

    private:
	friend GLVector operator+ (const GLVector& lhs,
				   const GLVector& rhs);
	friend GLVector operator- (const GLVector& lhs,
				   const GLVector& rhs);
	friend GLVector operator- (const GLVector& vector);
	friend float operator* (const GLVector& lhs,
				const GLVector& rhs);
	friend GLVector operator* (const float       k,
				   const GLVector& vector);
	friend GLVector operator* (const GLVector& vector,
				   const float       k);
	friend GLVector operator/ (const GLVector& lhs,
				   const GLVector& rhs);
	friend GLVector operator^ (const GLVector& lhs,
				   const GLVector& rhs);

	float v[4];
};

#endif
