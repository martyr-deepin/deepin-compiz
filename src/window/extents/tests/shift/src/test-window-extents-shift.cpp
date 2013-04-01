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

#include "test-window-extents.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>

class CompWindowExtentsTestShift :
    public CompWindowExtentsTest
{
protected:

    compiz::window::extents::Extents e;
};

TEST_F(CompWindowExtentsTestShift, TestShift)
{
    CompPoint rp;
    e.left = 5;
    e.right = 7;
    e.top = 10;
    e.bottom = 7;

    /* Check each gravity */
    rp = compiz::window::extents::shift (e, NorthGravity);

    EXPECT_EQ (rp, CompPoint (0, e.top));

    rp = compiz::window::extents::shift (e, NorthWestGravity);

    EXPECT_EQ (rp, CompPoint (e.left, e.top));

    rp = compiz::window::extents::shift (e, NorthEastGravity);

    EXPECT_EQ (rp, CompPoint (-e.right, e.top));

    rp = compiz::window::extents::shift (e, EastGravity);

    EXPECT_EQ (rp, CompPoint (-e.right, 0));

    rp = compiz::window::extents::shift (e, WestGravity);

    EXPECT_EQ (rp, CompPoint (e.left, 0));

    rp = compiz::window::extents::shift (e, SouthGravity);

    EXPECT_EQ (rp, CompPoint (0, -e.bottom));

    rp = compiz::window::extents::shift (e, SouthWestGravity);

    EXPECT_EQ (rp, CompPoint (e.left, -e.bottom));

    rp = compiz::window::extents::shift (e, SouthEastGravity);

    EXPECT_EQ (rp, CompPoint (-e.right, -e.bottom));
}
