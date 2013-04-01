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

#ifndef RESIZE_SCREEN_INTERFACE_H
#define RESIZE_SCREEN_INTERFACE_H

#include <core/screen.h>

namespace resize
{

class CompWindowInterface;

/*
 * Interface between a concrete CompScreen
 * and ResizeLogic.
 *
 * An enabler for having ResizeLogic testable.
 */
class CompScreenInterface
{
    public:
	virtual ~CompScreenInterface () {}

	virtual Window root () = 0;
	virtual CompWindowInterface * findWindow (Window id) = 0;
	virtual int xkbEvent () = 0;
	virtual void handleEvent (XEvent *event) = 0;
	virtual int syncEvent () = 0;
	virtual Display * dpy () = 0;
	virtual void warpPointer (int dx, int dy) = 0;
	virtual CompOutput::vector & outputDevs () = 0;
	virtual bool otherGrabExist (const char *, void *) = 0;
	virtual void updateGrab (CompScreen::GrabHandle handle, Cursor cursor) = 0;
	virtual CompScreen::GrabHandle pushGrab (Cursor cursor, const char *name) = 0;
	virtual void removeGrab (CompScreen::GrabHandle handle, CompPoint *restorePointer) = 0;

	/* CompOption::Class */
	virtual CompOption * getOption (const CompString &name) = 0;

	/* CompSize */
	virtual int width () const = 0;
	virtual int height () const = 0;
};

} /* namespace resize */

#endif /* RESIZE_SCREEN_INTERFACE_H */
