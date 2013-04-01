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

#ifndef RESIZE_WINDOW_IMPL_H
#define RESIZE_WINDOW_IMPL_H

#include <core/window.h>
#include "window-interface.h"

#include "resize-window-impl.h"
#include "gl-window-impl.h"
#include "composite-window-impl.h"

namespace resize
{

class CompWindowImpl : public CompWindowInterface
{
    public:
	CompWindowImpl (CompWindow *impl)
	    : mImpl (impl)
	{
	    mResizeImpl = ResizeWindowImpl::wrap(ResizeWindow::get (impl));

	    mGLImpl = GLWindowImpl::wrap(GLWindow::get (impl));
	    if (mGLImpl)
		mGLImpl->resizeWindow = ResizeWindow::get (impl);

	    mCompositeImpl = CompositeWindowImpl::wrap(CompositeWindow::get (impl));
	    if (mCompositeImpl)
		mCompositeImpl->resizeWindow = ResizeWindow::get (impl);
	}

	virtual ~CompWindowImpl ()
	{
	    delete mResizeImpl;
	    delete mGLImpl;
	    delete mCompositeImpl;
	}

	CompWindow *impl()
	{
	    return mImpl;
	}

	static CompWindowImpl *wrap (CompWindow *impl)
	{
	    if (impl)
		return new CompWindowImpl (impl);
	    else
		return NULL;
	}

	virtual Window id ()
	{
	    return mImpl->id ();
	}

	virtual CompRect outputRect () const
	{
	    return mImpl->outputRect ();
	}

	virtual XSyncAlarm syncAlarm ()
	{
	    return mImpl->syncAlarm ();
	}

	virtual XSizeHints & sizeHints () const
	{
	    return mImpl->sizeHints ();
	}

	virtual CompWindow::Geometry & serverGeometry () const
	{
	    return mImpl->serverGeometry ();
	}

	virtual CompWindowExtents & border () const
	{
	    return mImpl->border ();
	}

	virtual CompWindowExtents & output () const
	{
	    return mImpl->output ();
	}

	virtual bool constrainNewWindowSize (int width,
					     int height,
					     int *newWidth,
					     int *newHeight)
	{
	    return mImpl->constrainNewWindowSize (width,
						 height,
						 newWidth,
						 newHeight);
	}

	virtual bool syncWait ()
	{
	    return mImpl->syncWait ();
	}

	virtual void sendSyncRequest ()
	{
	    mImpl->sendSyncRequest ();
	}

	virtual void configureXWindow (unsigned int valueMask,
				       XWindowChanges *xwc)
	{
	    mImpl->configureXWindow (valueMask, xwc);
	}

	virtual void grabNotify (int x, int y,
				 unsigned int state, unsigned int mask)
	{
	    mImpl->grabNotify (x, y, state, mask);
	}

	virtual void ungrabNotify ()
	{
	    mImpl->ungrabNotify ();
	}

	virtual bool shaded ()
	{
	    return mImpl->shaded ();
	}

	virtual CompSize size () const
	{
	    return mImpl->size ();
	}

	virtual unsigned int actions ()
	{
	    return mImpl->actions ();
	}

	virtual unsigned int type ()
	{
	    return mImpl->type ();
	}

	virtual unsigned int & state ()
	{
	    return mImpl->state ();
	}

	virtual bool overrideRedirect ()
	{
	    return mImpl->overrideRedirect ();
	}

	virtual void updateAttributes (CompStackingUpdateMode stackingMode)
	{
	    mImpl->updateAttributes (stackingMode);
	}

	virtual int outputDevice ()
	{
	    return mImpl->outputDevice ();
	}

	virtual const CompSize serverSize () const
	{
	    return mImpl->serverSize ();
	}

	virtual void maximize (unsigned int state = 0)
	{
	    mImpl->maximize (state);
	}

	virtual bool evaluate (CompMatch &match)
	{
	    return match.evaluate (mImpl);
	}

	virtual ResizeWindowInterface *getResizeInterface ()
	{
	    return mResizeImpl;
	}

	virtual GLWindowInterface *getGLInterface ()
	{
	    return mGLImpl;
	}

	virtual CompositeWindowInterface *getCompositeInterface ()
	{
	    return mCompositeImpl;
	}

    private:
	CompWindow *mImpl;
	ResizeWindowImpl *mResizeImpl;
	GLWindowImpl *mGLImpl;
	CompositeWindowImpl *mCompositeImpl;
};

} /* namespace resize */

#endif /* RESIZE_WINDOW_IMPL_H */
