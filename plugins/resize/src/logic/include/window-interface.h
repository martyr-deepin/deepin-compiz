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

#ifndef RESIZE_WINDOW_INTERFACE
#define RESIZE_WINDOW_INTERFACE

#include <core/window.h>

namespace resize
{

class ResizeWindowInterface;
class GLWindowInterface;
class CompositeWindowInterface;

/*
 * Interface between a concrete CompWindow
 * and ResizeLogic.
 *
 * An enabler for having ResizeLogic testable.
 */
class CompWindowInterface
{
    public:
	virtual ~CompWindowInterface () {}

	virtual Window id () = 0;
	virtual CompRect outputRect () const = 0;
	virtual XSyncAlarm syncAlarm () = 0;
	virtual XSizeHints & sizeHints () const = 0;
	virtual CompWindow::Geometry & serverGeometry () const = 0;
	virtual CompWindowExtents & border () const = 0;
	virtual CompWindowExtents & output () const = 0;
	virtual bool constrainNewWindowSize (int width,
					     int height,
					     int *newWidth,
					     int *newHeight) = 0;
	virtual bool syncWait () = 0;
	virtual void sendSyncRequest () = 0;
	virtual void configureXWindow (unsigned int valueMask,
				       XWindowChanges *xwc) = 0;
	virtual void grabNotify (int x, int y,
				 unsigned int state, unsigned int mask) = 0;
	virtual void ungrabNotify () = 0;
	virtual bool shaded () = 0;
	virtual CompSize size () const = 0;
	virtual unsigned int actions () = 0;
	virtual unsigned int type () = 0;
	virtual unsigned int & state () = 0;
	virtual bool overrideRedirect () = 0;
	virtual void updateAttributes (CompStackingUpdateMode stackingMode) = 0;
	virtual int outputDevice () = 0;
	virtual const CompSize serverSize () const = 0;
	virtual void maximize (unsigned int state = 0) = 0;

	/* equivalent of CompMatch::evaluate  */
	virtual bool evaluate (CompMatch &match) = 0;

	virtual ResizeWindowInterface *getResizeInterface () = 0;
	virtual GLWindowInterface *getGLInterface () = 0;
	virtual CompositeWindowInterface *getCompositeInterface () = 0;
};

} /* namespace resize */

#endif /* RESIZE_WINDOW_INTERFACE */
