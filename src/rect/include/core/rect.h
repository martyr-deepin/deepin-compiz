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

#ifndef _COMPRECT_H
#define _COMPRECT_H

#include <core/point.h>
#include <core/size.h>
#include <vector>
#include <list>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xregion.h>
#include "core/point.h"


/**
 * A 2D rectangle, which is likely in screen space. It's data is
 * isolated and can only be mutated with set() methods.
 */
class CompRect {

    public:
	CompRect ();
	CompRect (int x, int y, int width, int height);
	CompRect (const CompRect&);
	CompRect (const XRectangle);

	int x () const;
	int y () const;
	CompPoint pos () const;

	int width () const;
	int height () const;

	int x1 () const;
	int y1 () const;
	int x2 () const;
	int y2 () const;

	int left   () const;
	int right  () const;
	int top    () const;
	int bottom () const;

	int centerX () const;
	int centerY () const;
	CompPoint center () const;

	int area () const;
	
	/**
	 * Returns an X region handle for the CompRect
	 */
	Region region () const;

	void setGeometry (int x, int y,
			  int width, int height);

	void setX      (int);
	void setY      (int);
	void setWidth  (int);
	void setHeight (int);

	void setPos (const CompPoint&);
	void setSize (const CompSize&);

	/** 
	 * Sets the left edge position
	 *
	 * Setting an edge past it's opposite edge will result in both edges
	 * being set to the new value
	 */
	void setLeft    (int);
	/**
	 * Sets the top edge position
	 *
	 * Setting an edge past it's opposite edge will result in both edges
	 * being set to the new value
	 */
	void setTop     (int);
	/** 
	 * Sets the right edge position
	 *
	 * Setting an edge past it's opposite edge will result in both edges
	 * being set to the new value
	 */
	void setRight   (int);
	/** 
	 * Sets the bottom edge position
	 *
	 * Setting an edge past it's opposite edge will result in both edges
	 * being set to the new value
	 */
	void setBottom  (int);

	bool contains (const CompPoint &) const;
	bool contains (const CompRect &) const;
	bool intersects (const CompRect &) const;
	bool isEmpty () const;

	bool operator== (const CompRect &) const;
	bool operator!= (const CompRect &) const;

	/* FIXME: Implement operator|= */

	CompRect operator& (const CompRect &) const;
	CompRect& operator&= (const CompRect &);
	CompRect& operator= (const CompRect &);

	typedef std::vector<CompRect> vector;
	typedef std::vector<CompRect *> ptrVector;
	typedef std::list<CompRect *> ptrList;

    private:
	REGION       mRegion;
};

namespace compiz
{
    namespace rect
    {
	CompPoint wraparoundPoint (const CompRect &bounds,
				   const CompPoint &p);
    }
}


inline int
CompRect::x () const
{
    return mRegion.extents.x1;
}

inline int
CompRect::y () const
{
    return mRegion.extents.y1;
}

inline CompPoint
CompRect::pos () const
{
    return CompPoint (x (), y ());
}

inline int
CompRect::width () const
{
    return mRegion.extents.x2 - mRegion.extents.x1;
}

inline int
CompRect::height () const
{
    return mRegion.extents.y2 - mRegion.extents.y1;
}

inline int
CompRect::x1 () const
{
    return mRegion.extents.x1;
}

inline int
CompRect::y1 () const
{
    return mRegion.extents.y1;
}

inline int
CompRect::x2 () const
{
    return mRegion.extents.x2;
}

inline int
CompRect::y2 () const
{
    return mRegion.extents.y2;
}

inline int
CompRect::left () const
{
    return mRegion.extents.x1;
}

inline int
CompRect::right () const
{
    return mRegion.extents.x2;
}

inline int
CompRect::top () const
{
    return mRegion.extents.y1;
}


inline int
CompRect::bottom () const
{
    return mRegion.extents.y2;
}

inline int
CompRect::centerX () const
{
    return x () + width () / 2;
}

inline int
CompRect::centerY () const
{
    return y () + height () / 2;
}

inline CompPoint
CompRect::center () const
{
    return CompPoint (centerX (), centerY ());
}

#endif
