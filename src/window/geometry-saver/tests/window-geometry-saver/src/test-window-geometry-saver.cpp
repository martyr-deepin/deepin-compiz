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

#include "test-window-geometry-saver.h"

class CompWindowGeometryTestSaver :
    public CompWindowGeometryTest
{
public:

    CompWindowGeometryTestSaver ();
    virtual ~CompWindowGeometryTestSaver ();

protected:

    compiz::window::Geometry      g;
    compiz::window::GeometrySaver saver;
};

CompWindowGeometryTestSaver::CompWindowGeometryTestSaver () :
    g (100, 100, 300, 300, 5),
    saver (g)
{
}

CompWindowGeometryTestSaver::~CompWindowGeometryTestSaver ()
{
}

TEST_F(CompWindowGeometryTestSaver, TestSaver)
{
    /* g by default */
    compiz::window::Geometry rg;
    unsigned int             mask = saver.get (rg);

    EXPECT_EQ (mask, 0);
    EXPECT_EQ (rg, compiz::window::Geometry (100, 100, 300, 300, 5));

    /* Push X value on to the saved geometry */
    saver.push (g, CHANGE_X);
    mask = saver.get (rg);

    EXPECT_EQ (mask, CHANGE_X);
    EXPECT_EQ (rg, compiz::window::Geometry (100, 100, 300, 300, 5));

    /* Push Y and Width values on to the saved geometry */
    saver.push (g, CHANGE_Y | CHANGE_WIDTH);
    mask = saver.get (rg);

    EXPECT_EQ (mask, CHANGE_X | CHANGE_Y | CHANGE_WIDTH);
    EXPECT_EQ (rg, compiz::window::Geometry (100, 100, 300, 300, 5));

    /* Pop Y value off the saved geoemtry */
    rg = compiz::window::Geometry ();
    mask = saver.pop (rg, CHANGE_Y);

    EXPECT_EQ (mask, CHANGE_Y);
    EXPECT_EQ (rg, compiz::window::Geometry (0, 100, 0, 0, 0));

    /* Attempt to pop X Y and Height off the saved geometry,
     * but since Y is not saved, only expect X */
    rg = compiz::window::Geometry ();
    mask = saver.pop (rg, CHANGE_X | CHANGE_Y | CHANGE_HEIGHT);

    EXPECT_EQ (mask, CHANGE_X);
    EXPECT_EQ (rg, compiz::window::Geometry (100, 0, 0, 0, 0));

    /* Update the saved geometry (eg, workspace change) and
     * pop the new value off */
    rg = compiz::window::Geometry ();
    g.setWidth (1200);
    saver.update (g, CHANGE_WIDTH);
    mask = saver.pop (rg, CHANGE_WIDTH);

    EXPECT_EQ (mask, CHANGE_WIDTH);
    EXPECT_EQ (rg, compiz::window::Geometry (0, 0, 1200, 0, 0));

    /* Try to push twice, only allow the first value to be popped off */
    rg = compiz::window::Geometry ();
    g.setWidth (1000);
    saver.push (g, CHANGE_WIDTH);
    g.setWidth (1200);
    saver.push (g, CHANGE_WIDTH);

    mask = saver.pop (rg, CHANGE_WIDTH);

    EXPECT_EQ (mask, CHANGE_WIDTH);
    EXPECT_EQ (rg, compiz::window::Geometry (0, 0, 1000, 0, 0));
}
