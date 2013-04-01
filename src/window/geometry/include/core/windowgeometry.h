/*
 * Copyright © 2011 Canonical Ltd.
 * Copyright © 2008 Dennis Kasprzyk
 * Copyright © 2007 Novell, Inc.
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
 *          David Reveman <davidr@novell.com>
 */

#ifndef _COMPWINDOWGEOMETRY_H
#define _COMPWINDOWGEOMETRY_H

#include <core/rect.h>

enum ChangeMask
{
    CHANGE_X = 1 << 0,
    CHANGE_Y = 1 << 1,
    CHANGE_WIDTH = 1 << 2,
    CHANGE_HEIGHT = 1 << 3,
    CHANGE_BORDER = 1 << 4
};

namespace compiz
{
namespace window
{

/**
  * A mutable object about the dimensions and location of a CompWindow.
  */
class Geometry :
    public CompRect
{
public:
    Geometry ();
    Geometry (int x, int y, int width, int height, int border);

    int border () const;

    void set (int x, int y, int width, int height, int border);
    void setBorder (int border);

    unsigned int	      changeMask (const compiz::window::Geometry &g) const;
    compiz::window::Geometry  change (const compiz::window::Geometry &g, unsigned int mask) const;
    void		      applyChange (const compiz::window::Geometry &g, unsigned int mask);

    int xMinusBorder () const { return x () - mBorder; }
    int yMinusBorder () const { return y () - mBorder; }

    unsigned int widthIncBorders () const { return width () + mBorder * 2; }
    unsigned int heightIncBorders () const { return height () + mBorder * 2; }

private:
    int mBorder;
};

}
}

#endif
