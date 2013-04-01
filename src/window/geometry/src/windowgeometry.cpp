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

#include <core/windowgeometry.h>


compiz::window::Geometry::Geometry () :
    mBorder (0)
{
}

compiz::window::Geometry::Geometry (int x,
				int y,
				int width,
				int height,
				int border) :
    CompRect (x, y, width, height),
    mBorder (border)
{
}

int
compiz::window::Geometry::border () const
{
    return mBorder;
}

void
compiz::window::Geometry::setBorder (int border)
{
    mBorder = border;
}

void
compiz::window::Geometry::set (int x,
			   int y,
			   int width,
			   int height,
			   int border)
{
    setX (x);
    setY (y);
    setWidth (width);
    setHeight (height);
    mBorder = border;
}

unsigned int
compiz::window::Geometry::changeMask (const compiz::window::Geometry &g) const
{
    unsigned int mask = 0;

    if (g.x () != x ())
	mask |= CHANGE_X;

    if (g.y () != y ())
	mask |= CHANGE_Y;

    if (g.width () != width ())
	mask |= CHANGE_WIDTH;

    if (g.height () != height ())
	mask |= CHANGE_HEIGHT;

    if (g.border () != border ())
	mask |= CHANGE_BORDER;

    return mask;
}

compiz::window::Geometry
compiz::window::Geometry::change (const compiz::window::Geometry &g, unsigned int mask) const
{
    compiz::window::Geometry rg = *this;

    rg.applyChange (g, mask);

    return rg;
}

void
compiz::window::Geometry::applyChange (const compiz::window::Geometry &g, unsigned int mask)
{
    if (mask & CHANGE_X)
	setX (g.x ());

    if (mask & CHANGE_Y)
	setY (g.y ());

    if (mask & CHANGE_WIDTH)
	setWidth (g.width ());

    if (mask & CHANGE_HEIGHT)
	setHeight (g.height ());

    if (mask & CHANGE_BORDER)
	setBorder (g.border ());
}
