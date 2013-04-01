/*
 * Copyright © 2012 Canonical Ltd.
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
 *          Sam Spilsbury <sam.spilsbury@canonical.com>
 */

#include <core/servergrab.h>
#include "pixmapbinding.h"

PixmapBinding::PixmapBinding (const NewPixmapReadyCallback &cb,
				WindowPixmapGetInterface *pmg,
				WindowAttributesGetInterface *wag,
				PixmapFreezerInterface       *pf,
				ServerGrabInterface *sg) :
    mPixmap (),
    mSize (),
    needsRebind (true),
    bindFailed (false),
    newPixmapReadyCallback (cb),
    windowPixmapRetreiver (pmg),
    windowAttributesRetreiver (wag),
    pixmapFreezer (pf),
    serverGrab (sg)
{
}

PixmapBinding::~PixmapBinding ()
{
    needsRebind = false;
}

const CompSize &
PixmapBinding::size () const
{
    return mSize;
}

Pixmap
PixmapBinding::pixmap () const
{
    static Pixmap nPixmap = None;

    if (needsRebind ||
	!mPixmap.get ())
	return nPixmap;

    return mPixmap->pixmap ();
}

bool
PixmapBinding::bind ()
{
    /* don't try to bind window again if it failed previously */
    if (bindFailed)
	return false;

    if (needsRebind)
    {
	XWindowAttributes attr;

	/* We have to grab the server here to make sure that window
	   is mapped when getting the window pixmap */
	ServerLock mLock (serverGrab);

	windowAttributesRetreiver->getAttributes (attr);
	if (attr.map_state != IsViewable ||
	    (attr.width == 0 && attr.border_width == 0) ||
	    (attr.height == 0 && attr.border_width == 0))
	{
	    bindFailed = true;
	    needsRebind = false;
	    return false;
	}

	WindowPixmapInterface::Ptr newPixmap = windowPixmapRetreiver->getPixmap ();
	CompSize newSize = CompSize (attr.border_width * 2 + attr.width,
				     attr.border_width * 2 + attr.height);

	if (newPixmap->pixmap () && newSize.width () && newSize.height ())
	{
	    /* Notify renderer that a new pixmap is about to
	     * be bound */
	    if (newPixmapReadyCallback)
		newPixmapReadyCallback ();

	    /* Assign new pixmap */
	    std::auto_ptr <WindowPixmap> newPixmapWrapper (new WindowPixmap (newPixmap));
	    mPixmap = newPixmapWrapper;
	    mSize = newSize;

	    needsRebind = false;
	}
	else
	{
	    bindFailed = true;
	    needsRebind = false;
	    return false;
	}
    }
    return true;
}

void
PixmapBinding::release ()
{
    if (!pixmapFreezer->frozen ())
	needsRebind = true;
}

void
PixmapBinding::setNewPixmapReadyCallback (const NewPixmapReadyCallback &cb)
{
    newPixmapReadyCallback = cb;
}

void
PixmapBinding::allowFurtherRebindAttempts ()
{
    bindFailed = false;
    needsRebind = true;
}
