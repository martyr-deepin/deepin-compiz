/*
 * Copyright © 2008 Dennis Kasprzyk
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
 * Authors: Dennis Kasprzyk <onestone@compiz-fusion.org>
 */

#ifndef _COMPSIZE_H
#define _COMPSIZE_H

/**
 * A 2D size (likely in screen space) that can only be mutated with set() methods,
 * since it'd data members are private.
 */
class CompSize {

    public:
	CompSize ();
	CompSize (int, int);

	int width () const;
	int height () const;

	void setWidth (int);
	void setHeight (int);

	bool operator== (const CompSize &other) const
	{
	    return (this->mWidth == other.mWidth &&
		    this->mHeight == other.mHeight);
	}

	bool operator!= (const CompSize &other) const
	{
	    return !(*this == other);
	}

    private:
	int mWidth, mHeight;
};

inline int
CompSize::width () const
{
    return mWidth;
}

inline int
CompSize::height () const
{
    return mHeight;
}

#endif
