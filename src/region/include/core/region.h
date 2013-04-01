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

#ifndef _COMPREGION_H
#define _COMPREGION_H

#include <X11/Xutil.h>
#include <X11/Xregion.h>

#include <core/rect.h>
#include <core/point.h>

/**
 * A 2D region with an (x,y) position and arbitrary dimensions similar to
 * an XRegion. It's data membmers are private and  must be manipulated with
 * set() methods.
 */
class CompRegion {
    public:
	typedef std::vector<CompRegion> List;
	typedef std::vector<CompRegion *> PtrList;
	typedef std::vector<CompRegion> Vector;
	typedef std::vector<CompRegion *> PtrVector;

    public:
	CompRegion ();
	CompRegion (const CompRegion &);
	CompRegion (int x, int y, int w, int h);
        CompRegion (const CompRect &);
	~CompRegion ();

	/**
	 * Returns a CompRect which encapsulates a given CompRegion
	 */
	CompRect boundingRect () const;

	bool isEmpty () const;
	
	/**
	 * Returns the number of XRectangles in the XRegion handle
	 */
	int numRects () const;
	
	/**
	 * Returns a vector of all the XRectangles in the XRegion handle
	 */
	CompRect::vector rects () const;
	
	/**
	 * Returns the internal XRegion handle
	 */
	Region handle () const;

	/**
	 * Returns true if the specified CompPoint falls within the
	 * CompRegion
	 */
	bool contains (const CompPoint &) const;
	/**
	 * Returns true if the specified CompRect falls within the
	 * CompRegion
	 */
	bool contains (const CompRect &) const;
	/**
	 * Returns true if the specified size falls withing the
	 * CompRegion
	 */
	bool contains (int x, int y, int width, int height) const;

	/**
	 * Returns a CompRegion that is the result of an intersect with
	 * the specified CompRegion and the region
	 */
	CompRegion intersected (const CompRegion &) const;
	/**
	 * Returns a CompRegion that is the result of an intersect with
	 * the specified CompRect and the region
	 */
	CompRegion intersected (const CompRect &) const;
	/**
	 * Returns true if a specified CompRegion intersects a region
	 */
	bool intersects (const CompRegion &) const;
	/**
	 * Returns true if a specified CompRect intersects a region
	 */
	bool intersects (const CompRect &) const;
	/**
	 * Returns a CompRegion covering the area of the region
	 * and not including the area of the specified CompRegion
	 */
	CompRegion subtracted (const CompRegion &) const;
	/**
	 * Returns a CompRegion covering the area of the region
	 * and not including the area of the specified CompRect
	 */
	CompRegion subtracted (const CompRect &) const;
	/**
	 * Moves a region by x and y amount
	 */
	void translate (int dx, int dy);
	/**
	 * Moves a region by an amount specified by the co-ordinates of a
	 * CompPoint
	 */
	void translate (const CompPoint &);
	/**
	 * Returns a CompRegion which is the result of the region being moved
	 * by dx and dy amount
	 */
	CompRegion translated (int, int) const;
	/**
	 * Returns a CompRegion which is the result of the region being moved
	 * by an amount specified by the co-ordinates of a CompPoint
	 */
	CompRegion translated (const CompPoint &) const;
	void shrink (int, int);
	void shrink (const CompPoint &);
	CompRegion shrinked (int, int) const;
	CompRegion shrinked (const CompPoint &) const;
	/**
	 * Returns a CompRegion which is the result of the region joined
	 * with a specified CompRegion
	 */
	CompRegion united (const CompRegion &) const;
	/**
	 * Returns a CompRegion which is the result of the region joined
	 * with a specified CompRect
	 */
	CompRegion united (const CompRect &) const;
	/**
	 * Returns a CompRegion which is the result of the region joined
	 * with a specified CompRegion, excluding the area in which
	 * both regions intersect
	 */
	CompRegion xored (const CompRegion &) const; 

	bool operator== (const CompRegion &) const;
	bool operator!= (const CompRegion &) const;
	const CompRegion operator& (const CompRegion &) const;
	const CompRegion operator& (const CompRect &) const;
	CompRegion & operator&= (const CompRegion &);
	CompRegion & operator&= (const CompRect &);
	const CompRegion operator+ (const CompRegion &) const;
	const CompRegion operator+ (const CompRect &) const;
	CompRegion & operator+= (const CompRegion &);
	CompRegion & operator+= (const CompRect &);
	const CompRegion operator- (const CompRegion &) const;
	const CompRegion operator- (const CompRect &) const;
	CompRegion & operator-= (const CompRegion &);
	CompRegion & operator-= (const CompRect &);
	CompRegion & operator= (const CompRegion &);

	const CompRegion operator^ (const CompRegion &) const;
	CompRegion & operator^= (const CompRegion &);
	const CompRegion operator| (const CompRegion &) const;
	CompRegion & operator|= (const CompRegion &);

    protected:
	/* Construct a CompRegion based on an externally managed Region */
	explicit CompRegion (Region);
	void init ();
	void *priv;
};

class CompRegionRef : public CompRegion
{
    public:
	explicit CompRegionRef (Region);
	~CompRegionRef ();
};

extern const CompRegion infiniteRegion;
extern const CompRegion emptyRegion;

#endif
