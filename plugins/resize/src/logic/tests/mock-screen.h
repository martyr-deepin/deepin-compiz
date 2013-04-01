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

#ifndef RESIZE_MOCK_SCREEN_H
#define RESIZE_MOCK_SCREEN_H

#include <gmock/gmock.h>

#include "screen-interface.h"
#include "gl-screen-interface.h"
#include "composite-screen-interface.h"

namespace resize
{

class MockScreen :  public CompScreenInterface,
		    public GLScreenInterface,
		    public CompositeScreenInterface
{
    public:
	MOCK_METHOD0(root, Window());
	MOCK_METHOD1(findWindow, CompWindowInterface*(Window id));
	MOCK_METHOD0(xkbEvent, int());
	MOCK_METHOD1(handleEvent, void(XEvent *event));
	MOCK_METHOD0(syncEvent, int());
	MOCK_METHOD0(dpy, Display*());
	MOCK_METHOD2(warpPointer, void(int dx, int dy));
	MOCK_METHOD0(outputDevs, CompOutput::vector&());
	MOCK_METHOD2(otherGrabExist, bool(const char *, void *));
	MOCK_METHOD2(updateGrab, void(CompScreen::GrabHandle handle, Cursor cursor));
	MOCK_METHOD2(pushGrab, CompScreen::GrabHandle(Cursor cursor, const char *name));
	MOCK_METHOD2(removeGrab, void(CompScreen::GrabHandle handle, CompPoint *restorePointer));

	MOCK_METHOD1(getOption, CompOption*(const CompString &name));

	MOCK_CONST_METHOD0(width, int());
	MOCK_CONST_METHOD0(height, int());

	/* from GLSCreenInterface  */
	MOCK_METHOD1(glPaintOutputSetEnabled, void(bool enable));

	/* from CompositeScreenInterface */
	MOCK_METHOD0(compositingActive, bool());
	MOCK_METHOD1(damageRegion, void(const CompRegion &r));
};

} /* namespace resize */

#endif /* RESIZE_MOCK_SCREEN_H */
