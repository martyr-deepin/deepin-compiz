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

#include <test-constrain-to-workarea.h>
#include <constrain-to-workarea.h>
#include <iostream>
#include <stdlib.h>
#include <cstring>

class CompPlaceTestConstrainToWorkarea :
    public CompPlaceTest
{
};

TEST_F(CompPlaceTestConstrainToWorkarea, TestConstrainToWorkarea)
{
    CompSize screensize (1000, 2000);
    CompRect workArea (50, 50, 900, 1900);
    compiz::window::Geometry g (100, 100, 200, 200, 0);
    compiz::window::extents::Extents extents;
    unsigned int flags = 0;

    memset (&extents, 0, sizeof (compiz::window::extents::Extents));

    /* Do nothing */
    compiz::place::clampGeometryToWorkArea (g, workArea, extents, flags, screensize);

    EXPECT_EQ (g, compiz::window::Geometry (100, 100, 200, 200, 0));

    /* Larger than workArea */
    g = compiz::window::Geometry (50, 50, 950, 1950, 0);

    compiz::place::clampGeometryToWorkArea (g, workArea, extents, flags, screensize);

    EXPECT_EQ (g, compiz::window::Geometry (50, 50, 900, 1900, 0));

    /* Outside top left */
    g = compiz::window::Geometry (0, 0, 900, 1900, 0);

    compiz::place::clampGeometryToWorkArea (g, workArea, extents, flags, screensize);

    EXPECT_EQ (g, compiz::window::Geometry (50, 50, 900, 1900, 0));

    /* Outside top right */
    g = compiz::window::Geometry (100, 0, 900, 1900, 0);

    compiz::place::clampGeometryToWorkArea (g, workArea, extents, flags, screensize);

    EXPECT_EQ (g, compiz::window::Geometry (50, 50, 900, 1900, 0));

    /* Outside bottom left */
    g = compiz::window::Geometry (0, 100, 900, 1900, 0);

    compiz::place::clampGeometryToWorkArea (g, workArea, extents, flags, screensize);

    EXPECT_EQ (g, compiz::window::Geometry (50, 50, 900, 1900, 0));

    /* Outside bottom right */
    g = compiz::window::Geometry (100, 100, 900, 1900, 0);

    compiz::place::clampGeometryToWorkArea (g, workArea, extents, flags, screensize);

    EXPECT_EQ (g, compiz::window::Geometry (50, 50, 900, 1900, 0));

    /* For the size only case, we should not
     * change the position of the window if
     * the size does not change */
    g = compiz::window::Geometry (0, 0, 900, 1900, 0);
    flags = compiz::place::clampGeometrySizeOnly;

    compiz::place::clampGeometryToWorkArea (g, workArea, extents, flags, screensize);

    EXPECT_EQ (g, compiz::window::Geometry (0, 0, 900, 1900, 0));

    g = compiz::window::Geometry (0, 0, 1000, 2000, 0);
    flags = compiz::place::clampGeometrySizeOnly;

    compiz::place::clampGeometryToWorkArea (g, workArea, extents, flags, screensize);

    EXPECT_EQ (g, compiz::window::Geometry (50, 50, 900, 1900, 0));
}
