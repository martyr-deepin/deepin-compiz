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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "smart.h"
#include <iostream>
#include <stdlib.h>
#include <cstring>

class CompPlaceSmartOffscreenTest :
    public ::testing::Test
{
};

class MockPlaceableObject :
    public compiz::place::Placeable
{
    public:

	MockPlaceableObject (const compiz::window::Geometry &geometry,
			     const compiz::window::extents::Extents &extents,
			     const std::vector <CompRect *> &availableWorkareas);
	
	const compiz::window::Geometry & getGeometry () const { return mGeometry; }
	const CompRect                 & getWorkarea () const { return *mWorkarea; }
	const compiz::window::extents::Extents & getExtents () const { return mExtents; }

    private:

	compiz::window::Geometry mGeometry;
	CompRect                 *mWorkarea;
	compiz::window::extents::Extents mExtents;
}

MockPlaceableObject::MockPlaceableObject (const compiz::window::Geometry &geometry,
					  const compiz::window::extents::Extents &extents,
					  const std::vector <CompRect *> &availableWorkareas) :
    compiz::place::Placeable::Placeable (),
    mGeometry (geometry),
    mExtents (extents)
{
    unsigned int areaMax = 0;
    CompRect     *running = availableWorkareas.front ();

    /* Pick the workarea that we best intersect */
    foreach (CompRect *rect, availableWorkareas)
    {
	unsigned int area = abs ((*rect & static_cast <CompRect &> (geometry)).area ());

	if (area > areaMax)
	{
	    running = rect;
	    areaMax = area;
	}
    }

    mWorkarea = running;
}

TEST_F(CompPlaceSmartOffscreenTest, TestOffscreenOne)
{
    CompRect wa1 (0, 24, 1680, 1026);
    CompRect wa2 (1680, 24, 1024, 744);
    compiz::place::Placeable::Vector v;

    std::vector <CompRect *> workAreas;
    workAreas.push_back (&wa1);
    workAreas.push_back (&wa2);

    /* Intersects 1 */
    compiz::window::Geometry g (0, 0, 640, 480);
    compiz::window::extents::Extents e (10, 10, 10, 10);

    MockPlaceableObject p (g, e, workAreas);

    CompPoint pos (g.x (), g.y ());

    compiz::place::smart (p, pos, v);

    std::cout << pos.x () << pos.y () << std::endl;

    EXPECT_EQ (pos, CompPoint (0, 0));
}

