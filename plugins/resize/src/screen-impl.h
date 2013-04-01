/*
 * Copyright © 2012 Canonical Ltd.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Novell, Inc. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Novell, Inc. makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * NOVELL, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL NOVELL, INC. BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Daniel d'Andrada <daniel.dandrada@canonical.com>
 */

#ifndef RESIZE_SCREEN_IMPL
#define RESIZE_SCREEN_IMPL

#include "screen-interface.h"
#include "window-impl.h"

#include <core/screen.h>

namespace resize
{

class CompScreenImpl : public CompScreenInterface
{
    public:
	CompScreenImpl (CompScreen *impl)
	    : mImpl (impl)
	{
	}

	virtual Window root ()
	{
	    return mImpl->root ();
	}

	virtual CompWindowInterface * findWindow (Window id)
	{
	    return CompWindowImpl::wrap (mImpl->findWindow (id));
	}

	virtual int xkbEvent ()
	{
	    return mImpl->xkbEvent ();
	}

	virtual void handleEvent (XEvent *event)
	{
	    mImpl->handleEvent (event);
	}

	virtual int syncEvent ()
	{
	    return mImpl->syncEvent ();
	}

	virtual Display * dpy ()
	{
	    return mImpl->dpy ();
	}

	virtual void warpPointer (int dx, int dy)
	{
	    mImpl->warpPointer (dx, dy);
	}

	virtual CompOutput::vector & outputDevs ()
	{
	    return mImpl->outputDevs ();
	}

	virtual bool otherGrabExist (const char *n, void *o)
	{
	    return mImpl->otherGrabExist (n, o);
	}

	virtual void updateGrab (CompScreen::GrabHandle handle, Cursor cursor)
	{
	    mImpl->updateGrab (handle, cursor);
	}

	virtual CompScreen::GrabHandle pushGrab (Cursor cursor, const char *name)
	{
	    return mImpl->pushGrab (cursor, name);
	}

	virtual void removeGrab (CompScreen::GrabHandle handle, CompPoint *restorePointer)
	{
	    mImpl->removeGrab (handle, restorePointer);
	}

	/* CompOption::Class */
	virtual CompOption * getOption (const CompString &name)
	{
	    return mImpl->getOption (name);
	}

	/* CompSize */
	virtual int width () const
	{
	    return mImpl->width ();
	}

	virtual int height () const
	{
	    return mImpl->height ();
	}

    private:
	CompScreen *mImpl;
};

} /* namespace resize */

#endif /* RESIZE_SCREEN_IMPL */
