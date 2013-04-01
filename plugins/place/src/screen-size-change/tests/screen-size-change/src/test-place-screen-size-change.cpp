/*
 * Copyright © 2011 Canonical Ltd.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Canonical Ltd. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Canonical Ltd. makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * CANONICAL, LTD. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL CANONICAL, LTD. BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Authored by: Sam Spilsbury <sam.spilsbury@canonical.com>
 */

#include <test-screen-size-change.h>
#include <screen-size-change.h>
#include <iostream>
#include <stdlib.h>
#include <cstring>

class CompPlaceScreenSizeChangeTestScreenSizeChange :
    public CompPlaceScreenSizeChangeTest
{
};

class MockScreenSizeChangeObject :
    public compiz::place::ScreenSizeChangeObject
{
    public:

	MockScreenSizeChangeObject (const compiz::window::Geometry &);
	~MockScreenSizeChangeObject ();

	const compiz::window::Geometry & getGeometry () const;
	void applyGeometry (compiz::window::Geometry &n,
			    compiz::window::Geometry &o);
	const CompPoint & getViewport () const;
	const CompRect &  getWorkarea (const compiz::window::Geometry &g) const;
	const compiz::window::extents::Extents & getExtents () const;

	void setVp (const CompPoint &);
	void setWorkArea (const CompRect &);
	void setExtents (unsigned int left,
			 unsigned int right,
			 unsigned int top,
			 unsigned int bottom);

	void setGeometry (const compiz::window::Geometry &g);

    private:

	CompPoint			 mCurrentVp;
	CompRect			 mCurrentWorkArea;
	compiz::window::extents::Extents mCurrentExtents;
	compiz::window::Geometry         mCurrentGeometry;
};

MockScreenSizeChangeObject::MockScreenSizeChangeObject (const compiz::window::Geometry &g) :
    ScreenSizeChangeObject (g),
    mCurrentVp (0, 0),
    mCurrentWorkArea (50, 50, 1000, 1000),
    mCurrentGeometry (g)
{
    memset (&mCurrentExtents, 0, sizeof (compiz::window::extents::Extents));
}

MockScreenSizeChangeObject::~MockScreenSizeChangeObject ()
{
}

const compiz::window::Geometry &
MockScreenSizeChangeObject::getGeometry () const
{
    return mCurrentGeometry;
}

void
MockScreenSizeChangeObject::applyGeometry (compiz::window::Geometry &n,
					   compiz::window::Geometry &o)
{
    EXPECT_EQ (mCurrentGeometry, o);

    std::cout << "DEBUG: new geometry : " << n.x () << " "
					  << n.y () << " "
					  << n.width () << " "
					  << n.height () << " "
					  << n.border () << std::endl;

    std::cout << "DEBUG: old geometry : " << o.x () << " "
					  << o.y () << " "
					  << o.width () << " "
					  << o.height () << " "
					  << o.border () << std::endl;

    mCurrentGeometry = n;
}

const CompPoint &
MockScreenSizeChangeObject::getViewport () const
{
    return mCurrentVp;
}

const CompRect &
MockScreenSizeChangeObject::getWorkarea (const compiz::window::Geometry &g) const
{
    return mCurrentWorkArea;
}

const compiz::window::extents::Extents &
MockScreenSizeChangeObject::getExtents () const
{
    return mCurrentExtents;
}

void
MockScreenSizeChangeObject::setVp (const CompPoint &p)
{
    mCurrentVp = p;
}

void
MockScreenSizeChangeObject::setWorkArea (const CompRect &wa)
{
    mCurrentWorkArea = wa;
}

void
MockScreenSizeChangeObject::setExtents (unsigned int left,
				        unsigned int right,
					unsigned int top,
					unsigned int bottom)
{
    mCurrentExtents.left = left;
    mCurrentExtents.right = right;
    mCurrentExtents.top = top;
    mCurrentExtents.bottom = bottom;
}

void
MockScreenSizeChangeObject::setGeometry (const compiz::window::Geometry &g)
{
    mCurrentGeometry = g;
}

void
reserveStruts (CompRect &workArea)
{
    workArea.setLeft (workArea.left () + 24);
    workArea.setTop (workArea.top () + 24);
    workArea.setBottom (workArea.bottom () - 24);
}

TEST_F(CompPlaceScreenSizeChangeTestScreenSizeChange, TestScreenSizeChange)
{
    CompSize		     current, old;
    compiz::window::Geometry g (200, 250, 300, 400, 0);

    MockScreenSizeChangeObject ms (g);

    current = CompSize (1280, 800);

    /* Reserve top, bottom and left parts of the screen for
     * fake "24px" panels */
    CompRect workArea = CompRect (0, 0, current.width (), current.height ());
    reserveStruts (workArea);

    ms.setWorkArea (workArea);

    /* First test that changing the screen size
     * to something smaller here doesn't cause our
     * (small) window to be moved */

    old = current;
    current = CompSize (1024, 768);

    workArea = CompRect (0, 0, current.width (), current.height ());
    reserveStruts (workArea);

    ms.setWorkArea (workArea);

    g = ms.adjustForSize (old, current);

    EXPECT_EQ (g, compiz::window::Geometry (200, 250, 300, 400, 0));

    /* Making the screen size bigger with no
     * saved geometry should cause the window not to move */

    old = current;
    current = CompSize (2048, 768);

    workArea = CompRect (0, 0, current.width (), current.height ());
    reserveStruts (workArea);

    ms.setWorkArea (workArea);

    g = ms.adjustForSize (old, current);

    EXPECT_EQ (g, compiz::window::Geometry (200, 250, 300, 400, 0));

    /* Move the window to the other "monitor" */

    ms.setGeometry (compiz::window::Geometry (1025, 250, 300, 400, 0));

    old = current;

    /* Unplug a "monitor" */
    current = CompSize (1024, 768);

    workArea = CompRect (0, 0, current.width (), current.height ());
    reserveStruts (workArea);

    ms.setWorkArea (workArea);

    g = ms.adjustForSize (old, current);

    EXPECT_EQ (g, compiz::window::Geometry (724, 250, 300, 400, 0));

    old = current;

    /* Re-plug the monitor - window should go back
     * to the same position */
    current = CompSize (2048, 768);

    workArea = CompRect (0, 0, current.width (), current.height ());
    reserveStruts (workArea);

    ms.setWorkArea (workArea);

    g = ms.adjustForSize (old, current);

    EXPECT_EQ (g, compiz::window::Geometry (1025, 250, 300, 400, 0));

    old = current;

    /* Plug 2 monitors downwards, no change */
    current = CompSize (2048, 1536);

    workArea = CompRect (0, 0, current.width (), current.height ());
    reserveStruts (workArea);

    ms.setWorkArea (workArea);

    g = ms.adjustForSize (old, current);

    EXPECT_EQ (g, compiz::window::Geometry (1025, 250, 300, 400, 0));

    /* Move the window to the bottom "monitor" */

    ms.setGeometry (compiz::window::Geometry (1025, 791, 300, 400, 0));

    old = current;

    /* Unplug bottom "monitor" */
    current = CompSize (2048, 768);

    workArea = CompRect (0, 0, current.width (), current.height ());
    reserveStruts (workArea);

    ms.setWorkArea (workArea);

    g = ms.adjustForSize (old, current);

    EXPECT_EQ (g, compiz::window::Geometry (1025, 344, 300, 400, 0));

    old = current;

    /* Re-plug bottom "monitor" */
    current = CompSize (2048, 1356);

    workArea = CompRect (0, 0, current.width (), current.height ());
    reserveStruts (workArea);

    ms.setWorkArea (workArea);

    g = ms.adjustForSize (old, current);

    EXPECT_EQ (g, compiz::window::Geometry (1025, 791, 300, 400, 0));

    /* Move the entire window right a viewport */

    g.setPos (g.pos () + CompPoint (current.width (), 0));

    ms.setGeometry (g);

    /* Now change the screen resolution again - the window should
     * move to be within the constrained size of its current
     * viewport */

    /* Unplug a "monitor" */
    old = current;
    current = CompSize (1024, 1356);

    workArea = CompRect (0, 0, current.width (), current.height ());
    reserveStruts (workArea);

    ms.setWorkArea (workArea);

    g = ms.adjustForSize (old, current);

    EXPECT_EQ (g, compiz::window::Geometry (current.width () + 724, 791, 300, 400, 0));
}

