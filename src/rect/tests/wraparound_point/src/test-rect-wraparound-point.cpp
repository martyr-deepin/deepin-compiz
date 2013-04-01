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

class CompRectTestWraparound :
    public CompRectTest
{
public:

    CompRectTestWraparound ();
    ~CompRectTestWraparound ();
};


CompRectTestWraparound::CompRectTestWraparound ()
{
    /* x1: -500
     * x2:  750
     * y1: -400
     * y2:  800
     */
    mRect = CompRect (-500, -400, 1250, 1200);
}

CompRectTestWraparound::~CompRectTestWraparound ()
{
}

TEST_F(CompRectTestWraparound, TestWraparound)
{
    CompPoint outsider = CompPoint (2501, 2401);
    CompPoint inside = CompPoint ();

    inside = compiz::rect::wraparoundPoint (mRect, outsider);

    RecordProperty ("OutsidePointX", outsider.x ());
    RecordProperty ("OutsidePointY", outsider.y ());
    RecordProperty ("InsidePointX", inside.x ());
    RecordProperty ("InsidePointY", inside.y ());

    EXPECT_EQ (inside, CompPoint (-499, -399));

    outsider = CompPoint (-1751, -1601);

    inside = compiz::rect::wraparoundPoint (mRect, outsider);

    RecordProperty ("OutsidePointX", outsider.x ());
    RecordProperty ("OutsidePointY", outsider.y ());
    RecordProperty ("InsidePointX", inside.x ());
    RecordProperty ("InsidePoinY", inside.y ());

    EXPECT_EQ (inside, CompPoint (749, 799));
}
