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

#ifndef _COMPWINDOWGEOMETRYSAVER_H
#define _COMPWINDOWGEOMETRYSAVER_H

#include <core/rect.h>
#include <core/windowgeometry.h>

namespace compiz
{
namespace window
{

/**
 * A one-level push-pop stack for saving window geometry
 * parameters, eg, to maximize and demaximize windows
 * and make sure they are restored to the same place
 * relative to the window */
class GeometrySaver
{
public:

    GeometrySaver (const Geometry &g);

    /**
     * Push some new geometry into the saved bits
     *
     * @param g a const compiz::window::Geometry & of the geometry
     * you wish to push
     * @param mask an unsigned int indicating which bits of the
     * specified geometry should be saved
     * @return the bits actually saved
     */
    unsigned int push (const Geometry &g, unsigned int mask);

    /**
     * Restore saved geometry
     *
     * @param g a compiz::window::Geometry & of the geometry
     * which should be written into
     * @param mask an unsigned int indicating which bits of the
     * geometry should be restored
     * @return the bits actually restored
     */
    unsigned int pop (Geometry &g, unsigned int mask);

    /**
     * Force update certain saved geometry bits
     *
     * @param g a const compiz::window::Geometry & of the geometry
     * you wish to update
     * @param mask an unsigned int indicating which bits of the
     * specified geometry should be updated
     */
    void update (const Geometry &g, unsigned int mask);

    unsigned int get (Geometry &g);

private:

    Geometry     mGeometry;
    unsigned int mMask;
};

}
}

#endif
