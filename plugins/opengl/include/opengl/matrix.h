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

#ifndef _GLMATRIX_H
#define _GLMATRIX_H

#include <opengl/vector.h>

class CompOutput;

class GLMatrix {
    public:
	GLMatrix ();
	GLMatrix (const float *);

	const float* getMatrix () const;

	GLMatrix& operator*= (const GLMatrix& rhs);

	float& operator[] (unsigned int pos);

	void reset ();
	void toScreenSpace (const CompOutput *output, float z);

	bool invert ();

	void rotate (const float angle, const float x,
		     const float y, const float z);
	void rotate (const float angle, const GLVector& vector);

	void scale (const float x, const float y, const float z);
	void scale (const GLVector& vector);

	void translate (const float x, const float y, const float z);
	void translate (const GLVector& vector);

    private:
	friend GLMatrix operator* (const GLMatrix& lhs,
				   const GLMatrix& rhs);
	friend GLVector operator* (const GLMatrix& lhs,
				   const GLVector& rhs);

	float m[16];
};

#endif
