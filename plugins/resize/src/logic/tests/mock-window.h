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

#ifndef RESIZE_MOCK_WINDOW_H
#define RESIZE_MOCK_WINDOW_H

#include <gmock/gmock.h>

#include "window-interface.h"
#include "gl-window-interface.h"
#include "composite-window-interface.h"

namespace resize
{

class MockWindow :  public CompWindowInterface,
		    public GLWindowInterface,
		    public CompositeWindowInterface
{
public:
    MOCK_METHOD0(id, Window());
    MOCK_CONST_METHOD0(outputRect, CompRect());
    MOCK_METHOD0(syncAlarm, XSyncAlarm());
    MOCK_CONST_METHOD0(sizeHints, XSizeHints&());
    MOCK_CONST_METHOD0(serverGeometry, CompWindow::Geometry &());
    MOCK_CONST_METHOD0(border, CompWindowExtents&());
    MOCK_CONST_METHOD0(output, CompWindowExtents&());
    MOCK_METHOD4(constrainNewWindowSize, bool (int width,
					       int height,
					       int *newWidth,
					       int *newHeight));
    MOCK_METHOD0(syncWait, bool());
    MOCK_METHOD0(sendSyncRequest, void());
    MOCK_METHOD2(configureXWindow, void(unsigned int valueMask,
					XWindowChanges *xwc));
    MOCK_METHOD4(grabNotify, void (int x, int y,
				   unsigned int state,
				   unsigned int mask));
    MOCK_METHOD0(ungrabNotify, void());
    MOCK_METHOD0(shaded, bool());
    MOCK_CONST_METHOD0(size, CompSize());
    MOCK_METHOD0(actions, unsigned int());
    MOCK_METHOD0(type, unsigned int());
    MOCK_METHOD0(state, unsigned int &());
    MOCK_METHOD0(overrideRedirect, bool());
    MOCK_METHOD1(updateAttributes, void(CompStackingUpdateMode stackingMode));
    MOCK_METHOD0(outputDevice, int());
    MOCK_CONST_METHOD0(serverSize, const CompSize());
    MOCK_METHOD1(maximize, void(unsigned int state));
    MOCK_METHOD1(evaluate, bool(CompMatch &match));

    MOCK_METHOD0(getResizeInterface, ResizeWindowInterface*());
    MOCK_METHOD0(getGLInterface, GLWindowInterface*());
    MOCK_METHOD0(getCompositeInterface, CompositeWindowInterface*());

    /* from GLWindowInterface  */
    MOCK_METHOD1(glPaintSetEnabled, void(bool enable));

    /* from CompositeWindowInterface  */
    MOCK_METHOD1(damageRectSetEnabled, void(bool enable));
};

} /* namespace resize */

#endif /* RESIZE_MOCK_WINDOW_H */
