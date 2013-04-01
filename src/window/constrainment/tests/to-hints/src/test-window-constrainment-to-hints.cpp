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

#include "test-window-constrainment.h"
#include <cstring>

class CompWindowConstrainmentTestToHints :
    public CompWindowConstrainmentTest
{
};

TEST_F(CompWindowConstrainmentTestToHints, ToHints)
{
    /* No hints, size is the same */
    XSizeHints hints;
    CompSize   size (1000, 1000);

    memset (&hints, 0, sizeof (XSizeHints));

    size = compiz::window::constrainment::constrainToHints (hints, size, 0, 0);

    EXPECT_EQ (size, CompSize (1000, 1000));

    /* Minimum size specified, constrain to minimum size */
    size = CompSize (100, 100);
    memset (&hints, 0, sizeof (XSizeHints));

    hints.flags |= PMinSize;
    hints.min_width = 500;
    hints.min_height = 500;

    size = compiz::window::constrainment::constrainToHints (hints, size, 0, 0);

    EXPECT_EQ (size, CompSize (500, 500));

    /* Base size specified, constrain to base size as minimum size */
    size = CompSize (100, 100);
    memset (&hints, 0, sizeof (XSizeHints));

    hints.flags |= PBaseSize;
    hints.base_width = 500;
    hints.base_height = 500;

    size = compiz::window::constrainment::constrainToHints (hints, size, 0, 0);

    EXPECT_EQ (size, CompSize (500, 500));

    /* Minimum and base size specified, constrain to min size as minimum size */
    size = CompSize (100, 100);
    memset (&hints, 0, sizeof (XSizeHints));

    hints.flags |= PBaseSize | PMinSize;
    hints.base_width = 700;
    hints.base_height = 700;
    hints.min_width = 500;
    hints.min_height = 500;

    size = compiz::window::constrainment::constrainToHints (hints, size, 0, 0);

    EXPECT_EQ (size, CompSize (500, 500));

    /* Maximum size specified, constrain to minimum size */
    size = CompSize (1000, 1000);
    memset (&hints, 0, sizeof (XSizeHints));

    hints.flags |= PMaxSize;
    hints.max_width = 500;
    hints.max_height = 500;

    size = compiz::window::constrainment::constrainToHints (hints, size, 0, 0);

    EXPECT_EQ (size, CompSize (500, 500));

    /* Resize flags specified, constrain to closest low step of
     * increments for size specified */
    size = CompSize (1002, 1002);
    memset (&hints, 0, sizeof (XSizeHints));

    hints.flags |= PBaseSize | PResizeInc;
    hints.base_width = 500;
    hints.base_height = 500;
    hints.width_inc = 5;
    hints.height_inc = 5;

    size = compiz::window::constrainment::constrainToHints (hints, size, 0, 0);

    EXPECT_EQ (size, CompSize (1000, 1000));

    /* Resize flags specified, constrain to closest low step of
     * increments for size specified */
    size = CompSize (1004, 1004);
    memset (&hints, 0, sizeof (XSizeHints));

    hints.flags |= PBaseSize | PResizeInc;
    hints.base_width = 500;
    hints.base_height = 500;
    hints.width_inc = 5;
    hints.height_inc = 5;

    size = compiz::window::constrainment::constrainToHints (hints, size, 0, 0);

    EXPECT_EQ (size, CompSize (1000, 1000));

    /* Resize flags specified, constrain to closest low step of
     * increments for size specified */
    size = CompSize (1006, 1006);
    memset (&hints, 0, sizeof (XSizeHints));

    hints.flags |= PBaseSize | PResizeInc;
    hints.base_width = 500;
    hints.base_height = 500;
    hints.width_inc = 5;
    hints.height_inc = 5;

    size = compiz::window::constrainment::constrainToHints (hints, size, 0, 0);

    EXPECT_EQ (size, CompSize (1005, 1005));

    /* Don't require constrainment on width */
    size = CompSize (1002, 1002);
    memset (&hints, 0, sizeof (XSizeHints));

    hints.flags |= PBaseSize | PResizeInc;
    hints.base_width = 500;
    hints.base_height = 500;
    hints.width_inc = 5;
    hints.height_inc = 5;

    size = compiz::window::constrainment::constrainToHints (hints, size, 0, compiz::window::constrainment::PHorzResizeInc);

    EXPECT_EQ (size, CompSize (1002, 1000));

    /* Don't require constrainment on height */
    size = CompSize (1002, 1002);
    memset (&hints, 0, sizeof (XSizeHints));

    hints.flags |= PBaseSize | PResizeInc;
    hints.base_width = 500;
    hints.base_height = 500;
    hints.width_inc = 5;
    hints.height_inc = 5;

    size = compiz::window::constrainment::constrainToHints (hints, size, 0, compiz::window::constrainment::PVertResizeInc);

    EXPECT_EQ (size, CompSize (1000, 1002));

    /* Aspect ratios - don't allow sizes less than 1:2 or more than 2:5
     * clamping to the largest size */
    size = CompSize (4000, 5000);
    memset (&hints, 0, sizeof (XSizeHints));

    hints.flags |= PAspect;
    hints.min_aspect.x = 1;
    hints.min_aspect.y = 2;
    hints.max_aspect.x = 2;
    hints.max_aspect.y = 5;

    size = compiz::window::constrainment::constrainToHints (hints, size, 0, 0);

    EXPECT_EQ (size, CompSize (2000, 5000));

    /* Aspect ratios - don't allow sizes less than 1:2 or more than 2:5
     * clamping to the largest size */
    size = CompSize (12, 20);
    memset (&hints, 0, sizeof (XSizeHints));

    hints.flags |= PAspect;
    hints.min_aspect.x = 1;
    hints.min_aspect.y = 2;
    hints.max_aspect.x = 2;
    hints.max_aspect.y = 5;

    size = compiz::window::constrainment::constrainToHints (hints, size, 0, 0);

    EXPECT_EQ (size, CompSize (8, 20));
}
