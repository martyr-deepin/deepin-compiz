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

#include "test-window-geometry.h"

class CompWindowGeometryTestGeometry :
    public CompWindowGeometryTest
{
    public:

	CompWindowGeometryTestGeometry ();
	~CompWindowGeometryTestGeometry ();

    protected:

	compiz::window::Geometry g;
};

CompWindowGeometryTestGeometry::CompWindowGeometryTestGeometry () :
    g (50, 100, 200, 300, 5)
{
}

CompWindowGeometryTestGeometry::~CompWindowGeometryTestGeometry ()
{
}

TEST_F(CompWindowGeometryTestGeometry, TestGeometry)
{

    /* apply x only */
    compiz::window::Geometry rg = compiz::window::Geometry ();
    rg.applyChange (g, CHANGE_X);

    EXPECT_EQ (rg, compiz::window::Geometry (50, 0, 0, 0, 0));

    /* apply y only */
    rg = compiz::window::Geometry ();
    rg.applyChange (g, CHANGE_Y);

    EXPECT_EQ (rg, compiz::window::Geometry (0, 100, 0, 0, 0));

    /* apply width only */
    rg = compiz::window::Geometry ();
    rg.applyChange (g, CHANGE_WIDTH);

    EXPECT_EQ (rg, compiz::window::Geometry (0, 0, 200, 0, 0));

    /* apply height only */
    rg = compiz::window::Geometry ();
    rg.applyChange (g, CHANGE_HEIGHT);

    EXPECT_EQ (rg, compiz::window::Geometry (0, 0, 0, 300, 0));

    /* apply width | height */
    rg = compiz::window::Geometry ();
    rg.applyChange (g, CHANGE_WIDTH | CHANGE_HEIGHT);

    EXPECT_EQ (rg, compiz::window::Geometry (0, 0, 200, 300, 0));

    /* change mask for x | y | width | height */
    rg  = compiz::window::Geometry (49, 99, 199, 299, 5);
    unsigned int mask = rg.changeMask (g);

    EXPECT_EQ (rg, compiz::window::Geometry (49, 99, 199, 299, 5));
    EXPECT_EQ (mask, CHANGE_X | CHANGE_Y | CHANGE_WIDTH | CHANGE_HEIGHT);
}

TEST_F(CompWindowGeometryTestGeometry, TestBorders)
{
    compiz::window::Geometry g (1, 1, 1, 1, 1);

    EXPECT_EQ (g.xMinusBorder (), 0);
    EXPECT_EQ (g.yMinusBorder (), 0);
    EXPECT_EQ (g.widthIncBorders (), 3);
    EXPECT_EQ (g.heightIncBorders (), 3);
}
