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

#include <test-rect.h>
#include <iostream>
#include <stdlib.h>

TEST_F(CompRectTest, TestRect)
{
    ASSERT_EQ (mRect, CompRect (0, 0, 0, 0));

    mRect = CompRect (100, 100, 400, 300);

    EXPECT_EQ (mRect.x (), 100);
    EXPECT_EQ (mRect.y (), 100);
    EXPECT_EQ (mRect.width (), 400);
    EXPECT_EQ (mRect.height (), 300);
    EXPECT_EQ (mRect.x2 (), 500);
    EXPECT_EQ (mRect.y2 (), 400);
    EXPECT_EQ (mRect.left (), 100);
    EXPECT_EQ (mRect.right (), 500);
    EXPECT_EQ (mRect.top (), 100);
    EXPECT_EQ (mRect.bottom (), 400);

    EXPECT_EQ (mRect.centerX (), 300);
    EXPECT_EQ (mRect.centerY (), 250);

    EXPECT_EQ (mRect.center (), CompPoint (300, 250));
    EXPECT_EQ (mRect.pos (), CompPoint (100, 100));

    EXPECT_EQ (mRect.area (), 120000);

    mRect.setWidth (-1);
    mRect.setHeight (-1);

    EXPECT_EQ (mRect.area (), 0);

    mRect = CompRect (0, 0, 0, 0);

    EXPECT_TRUE (mRect.isEmpty ());

    mRect.setRight (500);
    mRect.setLeft (100);
    mRect.setTop (50);
    mRect.setBottom (450);

    EXPECT_EQ (mRect, CompRect (100, 50, 400, 400));

    mRect.setLeft (600);

    EXPECT_TRUE (mRect.isEmpty ());
    EXPECT_EQ (mRect.right (), 600);

    mRect.setRight (1000);

    EXPECT_TRUE (mRect.contains (CompPoint (601, 100)));
    EXPECT_TRUE (mRect.contains (CompRect (601, 51, 300, 350)));
    EXPECT_FALSE (mRect.contains (CompRect (601, 41, 900, 500)));
    EXPECT_TRUE (mRect.intersects (CompRect (601, 41, 300, 400)));

    /* Intersection */
    mRect &= CompRect (700, 100, 400, 100);

    EXPECT_EQ (mRect, CompRect (700, 100, 300, 100));
}

