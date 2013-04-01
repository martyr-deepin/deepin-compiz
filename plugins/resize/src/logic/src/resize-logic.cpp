/*
 * Copyright © 2005 Novell, Inc.
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
 * Authors: David Reveman <davidr@novell.com>
 *	    Daniel d'Andrada <daniel.dandrada@canonical.com>
 */

#include <core/core.h>
#include <core/atoms.h>

#include "resize-logic.h"

#include "resize_options.h"

#include "screen-interface.h"
#include "gl-screen-interface.h"
#include "composite-screen-interface.h"
#include "window-interface.h"
#include "composite-window-interface.h"
#include "gl-window-interface.h"
#include "resize-window-interface.h"
#include "property-writer-interface.h"

#define XWINDOWCHANGES_INIT {0, 0, 0, 0, 0, None, 0}

static const unsigned short TOUCH_LEFT = 1;
static const unsigned short TOUCH_RIGHT = 2;
static const unsigned short TOUCH_TOP = 3;
static const unsigned short TOUCH_BOTTOM = 4;

using namespace resize;

ResizeLogic::ResizeLogic() :
    mScreen (NULL),
    w (NULL),
    centered (false),
    maximized_vertically (false),
    outlineMask (0),
    rectangleMask (0),
    stretchMask (0),
    centeredMask (0),
    releaseButton (0),
    grabIndex (0),
    isConstrained (false),
    offWorkAreaConstrained (true),
    options (NULL),
    cScreen (NULL),
    gScreen (NULL)
{
    rKeys[0].name	= "Left";
    rKeys[0].dx		= -1;
    rKeys[0].dy		= 0;
    rKeys[0].warpMask	= ResizeLeftMask | ResizeRightMask;
    rKeys[0].resizeMask = ResizeLeftMask;

    rKeys[1].name	= "Right";
    rKeys[1].dx		= 1;
    rKeys[1].dy		= 0;
    rKeys[1].warpMask	= ResizeLeftMask | ResizeRightMask;
    rKeys[1].resizeMask	= ResizeRightMask;

    rKeys[2].name	= "Up";
    rKeys[2].dx		= 0;
    rKeys[2].dy		= -1;
    rKeys[2].warpMask	= ResizeUpMask | ResizeDownMask;
    rKeys[2].resizeMask	= ResizeUpMask;

    rKeys[3].name	= "Down";
    rKeys[3].dx		= 0;
    rKeys[3].dy		= 1;
    rKeys[3].warpMask	= ResizeUpMask | ResizeDownMask;
    rKeys[3].resizeMask	= ResizeDownMask;
}

ResizeLogic::~ResizeLogic()
{
}

void
ResizeLogic::handleEvent (XEvent *event)
{
    switch (event->type) {
	case KeyPress:
	    if (event->xkey.root == mScreen->root ())
		handleKeyEvent (event->xkey.keycode);
	    break;
	case ButtonRelease:
	    if (event->xbutton.root == mScreen->root ())
	    {
		if (grabIndex)
		{
		    if (releaseButton         == -1 ||
			(int) event->xbutton.button == releaseButton)
		    {
			CompAction *action = &options->optionGetInitiateButton ();

			terminateResize (action, CompAction::StateTermButton,
					 noOptions ());
		    }
		}
	    }
	    break;
	case MotionNotify:
	    if (event->xmotion.root == mScreen->root ())
		handleMotionEvent (pointerX, pointerY);
	    break;
	case EnterNotify:
	case LeaveNotify:
	    if (event->xcrossing.root == mScreen->root ())
		handleMotionEvent (pointerX, pointerY);
	    break;
	case ClientMessage:
	    if (event->xclient.message_type == Atoms::wmMoveResize)
	    {
		CompWindowInterface *w;
		unsigned long	    type = event->xclient.data.l[2];

		if (type <= WmMoveResizeSizeLeft ||
		    type == WmMoveResizeSizeKeyboard)
		{
		    w = mScreen->findWindow (event->xclient.window);
		    if (w)
		    {
			CompOption::Vector o (0);

			o.push_back (CompOption ("window",
				     CompOption::TypeInt));
			o[0].value ().set ((int) event->xclient.window);

			o.push_back (CompOption ("external",
				     CompOption::TypeBool));
			o[1].value ().set (true);

			if (event->xclient.data.l[2] == WmMoveResizeSizeKeyboard)
			{
			    initiateResizeDefaultMode (&options->optionGetInitiateKey (),
						       CompAction::StateInitKey,
						       o);
			}
			else
			{
			    /* TODO: not only button 1 */
			    if (pointerMods & Button1Mask)
			    {
				static unsigned int mask[] = {
				    ResizeUpMask | ResizeLeftMask,
				    ResizeUpMask,
				    ResizeUpMask | ResizeRightMask,
				    ResizeRightMask,
				    ResizeDownMask | ResizeRightMask,
				    ResizeDownMask,
				    ResizeDownMask | ResizeLeftMask,
				    ResizeLeftMask,
				};
				o.push_back (CompOption ("modifiers",
					     CompOption::TypeInt));
				o.push_back (CompOption ("x",
					     CompOption::TypeInt));
				o.push_back (CompOption ("y",
					     CompOption::TypeInt));
				o.push_back (CompOption ("direction",
					     CompOption::TypeInt));
				o.push_back (CompOption ("button",
					     CompOption::TypeInt));

				o[2].value ().set ((int) pointerMods);
				o[3].value ().set
				    ((int) event->xclient.data.l[0]);
				o[4].value ().set
				    ((int) event->xclient.data.l[1]);
				o[5].value ().set
				    ((int) mask[event->xclient.data.l[2]]);
				o[6].value ().set
				    ((int) (event->xclient.data.l[3] ?
				     event->xclient.data.l[3] : -1));

				initiateResizeDefaultMode (
				    &options->optionGetInitiateButton (),
				    CompAction::StateInitButton, o);

				handleMotionEvent (pointerX, pointerY);
			    }
			}
		    }
		}
		else if (this->w && type == WmMoveResizeCancel)
		{
		    if (this->w->id () == event->xclient.window)
		    {
			terminateResize (&options->optionGetInitiateButton (),
					 CompAction::StateCancel, noOptions ());
			terminateResize (&options->optionGetInitiateKey (),
					 CompAction::StateCancel, noOptions ());
		    }
		}
	    }
	    break;
	case DestroyNotify:
	    if (w && w->id () == event->xdestroywindow.window)
	    {
		terminateResize (&options->optionGetInitiateButton (), 0, noOptions ());
		terminateResize (&options->optionGetInitiateKey (), 0, noOptions ());
	    }
	    break;
	case UnmapNotify:
	    if (w && w->id () == event->xunmap.window)
	    {
		terminateResize (&options->optionGetInitiateButton (), 0, noOptions ());
		terminateResize (&options->optionGetInitiateKey (), 0, noOptions ());
	    }
	default:
	    break;
    }

    if (event->type == mScreen->xkbEvent ())
    {
	XkbAnyEvent *xkbEvent = (XkbAnyEvent *) event;

	if (xkbEvent->xkb_type == XkbStateNotify)
	{
	    XkbStateNotifyEvent *stateEvent = (XkbStateNotifyEvent *) event;

	    /* Check if we need to change to outline mode */

	    unsigned int mods = 0xffffffff;
	    bool	 modifierMode = false;
	    int		 oldMode = mode;

	    if (outlineMask)
		mods = outlineMask;

	    if ((stateEvent->mods & mods) == mods)
	    {
		modifierMode = true;
		mode = ResizeOptions::ModeOutline;
	    }

	    mods = 0xffffffff;
	    if (rectangleMask)
		mods = rectangleMask;

	    if ((stateEvent->mods & mods) == mods)
	    {
		modifierMode = true;
		mode = ResizeOptions::ModeRectangle;
	    }

	    mods = 0xffffffff;
	    if (stretchMask)
		mods = stretchMask;

	    if ((stateEvent->mods & mods) == mods)
	    {
		modifierMode = true;
		mode = ResizeOptions::ModeStretch;
	    }

	    mods = 0xffffffff;
	    if (centeredMask)
		mods = centeredMask;

	    /* No modifier mode set, check match options */
	    if (w)
	    {
		if (w->evaluate (options->optionGetNormalMatch ()))
		{
		    modifierMode = true;
		    mode = ResizeOptions::ModeNormal;
		}

		if (w->evaluate (options->optionGetOutlineMatch ()))
		{
		    modifierMode = true;
		    mode = ResizeOptions::ModeOutline;
		}

		if (w->evaluate (options->optionGetRectangleMatch ()))
		{
		    modifierMode = true;
		    mode = ResizeOptions::ModeRectangle;
		}

		if (w->evaluate (options->optionGetStretchMatch ()))
		{
		    modifierMode = true;
		    mode = ResizeOptions::ModeStretch;
		}
	    }

	    if (!modifierMode)
		mode = options->optionGetMode ();

	    if (w && oldMode != mode)
	    {
		Box box;

		getStretchRectangle (&box);
		damageRectangle (&box);
		getPaintRectangle (&box);
		damageRectangle (&box);

		box.x1 = w->outputRect ().x ();
		box.y1 = w->outputRect ().y ();
		box.x2 = box.x1 + w->outputRect ().width ();
		box.y2 = box.y1 + w->outputRect ().height ();

		damageRectangle (&box);
	    }

	    if ((stateEvent->mods & mods) == mods)
	    {
		centered = true;
	    }
	    else if (w)
	    {
		if (!w->evaluate (options->optionGetResizeFromCenterMatch ()))
		    centered = false;
		else
		    centered = true;
	    }
	    else
	    {
		centered = false;
	    }
	}
    }

    mScreen->handleEvent (event);

    if (event->type == mScreen->syncEvent () + XSyncAlarmNotify)
    {
	if (w)
	{
	    XSyncAlarmNotifyEvent *sa;

	    sa = (XSyncAlarmNotifyEvent *) event;

	    if (w->syncAlarm () == sa->alarm)
		updateWindowSize ();
	}
    }
}

void
ResizeLogic::handleKeyEvent (KeyCode keycode)
{
    if (grabIndex && w)
    {
	int	   widthInc, heightInc;

	widthInc  = w->sizeHints ().width_inc;
	heightInc = w->sizeHints ().height_inc;

	if (widthInc < MIN_KEY_WIDTH_INC)
	    widthInc = MIN_KEY_WIDTH_INC;

	if (heightInc < MIN_KEY_HEIGHT_INC)
	    heightInc = MIN_KEY_HEIGHT_INC;

	for (unsigned int i = 0; i < NUM_KEYS; i++)
	{
	    if (keycode != key[i])
		continue;

	    if (mask & rKeys[i].warpMask)
	    {
		XWarpPointer (mScreen->dpy (), None, None, 0, 0, 0, 0,
			      rKeys[i].dx * widthInc, rKeys[i].dy * heightInc);
	    }
	    else
	    {
		int x, y, left, top, width, height;

		CompWindow::Geometry server = w->serverGeometry ();
		const CompWindowExtents    &border  = w->border ();

		left   = server.x () - border.left;
		top    = server.y () - border.top;
		width  = border.left + server.width () + border.right;
		height = border.top  + server.height () + border.bottom;

		x = left + width  * (rKeys[i].dx + 1) / 2;
		y = top  + height * (rKeys[i].dy + 1) / 2;

		mScreen->warpPointer (x - pointerX, y - pointerY);

		mask = rKeys[i].resizeMask;

		mScreen->updateGrab (grabIndex, cursor[i]);
	    }
	    break;
	}
    }
}

void
ResizeLogic::handleMotionEvent (int xRoot, int yRoot)
{
    if (grabIndex)
    {
	BoxRec box;
	int    wi, he, cwi, che;        /* size of window contents (c prefix for constrained)*/
	int    wX, wY, wWidth, wHeight; /* rect. for window contents+borders */

	wi = savedGeometry.width;
	he = savedGeometry.height;

	if (!mask)
	{
	    setUpMask (xRoot, yRoot);
	}
	else
	{
	    accumulatePointerMotion (xRoot, yRoot);
	}

	if (mask & ResizeLeftMask)
	    wi -= pointerDx;
	else if (mask & ResizeRightMask)
	    wi += pointerDx;

	if (mask & ResizeUpMask)
	    he -= pointerDy;
	else if (mask & ResizeDownMask)
	    he += pointerDy;

	if (w->state () & CompWindowStateMaximizedVertMask)
	    he = w->serverGeometry ().height ();

	if (w->state () & CompWindowStateMaximizedHorzMask)
	    wi = w->serverGeometry ().width ();

	cwi = wi;
	che = he;

	if (w->constrainNewWindowSize (wi, he, &cwi, &che) &&
	    mode != ResizeOptions::ModeNormal)
	{
	    Box box;

	    /* Also, damage relevant paint rectangles */
	    if (mode == ResizeOptions::ModeRectangle ||
	        mode == ResizeOptions::ModeOutline)
		getPaintRectangle (&box);
	    else if (mode == ResizeOptions::ModeStretch)
		getStretchRectangle (&box);

	    damageRectangle (&box);
	}

	if (offWorkAreaConstrained)
	    constrainToWorkArea (che, cwi);

	wi = cwi;
	he = che;

	/* compute rect. for window + borders */
	computeWindowPlusBordersRect (wX, wY, wWidth, wHeight, /*out*/
				      wi, he); /*in*/

	snapWindowToWorkAreaBoundaries (wi, he, wX, wY, wWidth, wHeight);

	if (isConstrained)
	    limitMovementToConstraintRegion (wi, he, /*in/out*/
					     xRoot, yRoot,
					     wX, wY, wWidth, wHeight); /*in*/

	if (mode != ResizeOptions::ModeNormal)
	{
	    if (mode == ResizeOptions::ModeStretch)
		getStretchRectangle (&box);
	    else
		getPaintRectangle (&box);

	    damageRectangle (&box);
	}

	enableOrDisableVerticalMaximization (yRoot);

	computeGeometry (wi, he);

	if (mode != ResizeOptions::ModeNormal)
	{
	    if (mode == ResizeOptions::ModeStretch)
		getStretchRectangle (&box);
	    else
		getPaintRectangle (&box);

	    damageRectangle (&box);
	}
	else
	{
	    updateWindowSize ();
	}

	updateWindowProperty ();
	sendResizeNotify ();
    }
}

void
ResizeLogic::updateWindowSize ()
{
    if (w->syncWait ())
	return;

    if (w->serverGeometry ().width ()  != geometry.width ||
	w->serverGeometry ().height () != geometry.height)
    {
	XWindowChanges xwc = XWINDOWCHANGES_INIT;

	xwc.x	   = geometry.x;
	xwc.y	   = geometry.y;
	xwc.width  = geometry.width;
	xwc.height = geometry.height;

	w->sendSyncRequest ();

	w->configureXWindow (CWX | CWY | CWWidth | CWHeight, &xwc);
    }
}

void
ResizeLogic::updateWindowProperty ()
{
    CompOption::Vector data = resizeInformationAtom->getReadTemplate ();;
    CompOption::Value v;

    if (data.size () != 4)
	return;

    v = geometry.x;
    data.at (0).set (v);

    v = geometry.y;
    data.at (1).set (v);

    v = geometry.width;
    data.at (2).set (v);

    v = geometry.height;
    data.at (3).set (v);

    resizeInformationAtom->updateProperty (w->id (), data, XA_CARDINAL);
}

void
ResizeLogic::sendResizeNotify ()
{
    XEvent xev;

    xev.xclient.type    = ClientMessage;
    xev.xclient.display = mScreen->dpy ();
    xev.xclient.format  = 32;

    xev.xclient.message_type = resizeNotifyAtom;
    xev.xclient.window	     = w->id ();

    xev.xclient.data.l[0] = geometry.x;
    xev.xclient.data.l[1] = geometry.y;
    xev.xclient.data.l[2] = geometry.width;
    xev.xclient.data.l[3] = geometry.height;
    xev.xclient.data.l[4] = 0;

    XSendEvent (mScreen->dpy (), mScreen->root (), false,
		SubstructureRedirectMask | SubstructureNotifyMask, &xev);
}

void
ResizeLogic::finishResizing ()
{
    w->ungrabNotify ();

    resizeInformationAtom->deleteProperty (w->id ());

    w = NULL;
}

void
ResizeLogic::getPaintRectangle (BoxPtr pBox)
{
    pBox->x1 = geometry.x - w->border ().left;
    pBox->y1 = geometry.y - w->border ().top;
    pBox->x2 = geometry.x + geometry.width +
	       w->serverGeometry ().border () * 2 + w->border ().right;

    if (w->shaded ())
	pBox->y2 = geometry.y + w->size ().height () + w->border ().bottom;
    else
	pBox->y2 = geometry.y + geometry.height +
		   w->serverGeometry ().border () * 2 + w->border ().bottom;
}

void
ResizeLogic::getStretchRectangle (BoxPtr pBox)
{
    BoxRec box;
    float  xScale, yScale;

    getPaintRectangle (&box);
    w->getResizeInterface ()->getStretchScale (&box, &xScale, &yScale);

    pBox->x1 = (int) (box.x1 - (w->output ().left - w->border ().left) * xScale);
    pBox->y1 = (int) (box.y1 - (w->output ().top - w->border ().top) * yScale);
    pBox->x2 = (int) (box.x2 + w->output ().right * xScale);
    pBox->y2 = (int) (box.y2 + w->output ().bottom * yScale);
}

Cursor
ResizeLogic::cursorFromResizeMask (unsigned int mask)
{
    Cursor cursor;

    if (mask & ResizeLeftMask)
    {
	if (mask & ResizeDownMask)
	    cursor = downLeftCursor;
	else if (mask & ResizeUpMask)
	    cursor = upLeftCursor;
	else
	    cursor = leftCursor;
    }
    else if (mask & ResizeRightMask)
    {
	if (mask & ResizeDownMask)
	    cursor = downRightCursor;
	else if (mask & ResizeUpMask)
	    cursor = upRightCursor;
	else
	    cursor = rightCursor;
    }
    else if (mask & ResizeUpMask)
    {
	cursor = upCursor;
    }
    else
    {
	cursor = downCursor;
    }

    return cursor;
}

void
ResizeLogic::snapWindowToWorkAreaBoundaries (int &wi, int &he,
					      int &wX, int &wY,
					      int &wWidth, int &wHeight)
{
    int workAreaSnapDistance = 15;

    /* Check if resized edge(s) are near output work-area boundaries */
    foreach (CompOutput &output, mScreen->outputDevs ())
    {
        const CompRect &workArea = output.workArea ();

        /* if window and work-area intersect in x axis */
        if (wX + wWidth > workArea.x () &&
                wX < workArea.x2 ())
        {
            if (mask & ResizeLeftMask)
            {
                int dw = workArea.x () - wX;

                if (0 < dw && dw < workAreaSnapDistance)
                {
                    wi     -= dw;
                    wWidth -= dw;
                    wX     += dw;
                }
            }
            else if (mask & ResizeRightMask)
            {
                int dw = wX + wWidth - workArea.x2 ();

                if (0 < dw && dw < workAreaSnapDistance)
                {
                    wi     -= dw;
                    wWidth -= dw;
                }
            }
        }

        /* if window and work-area intersect in y axis */
        if (wY + wHeight > workArea.y () &&
                wY < workArea.y2 ())
        {
            if (mask & ResizeUpMask)
            {
                int dh = workArea.y () - wY;

                if (0 < dh && dh < workAreaSnapDistance)
                {
                    he      -= dh;
                    wHeight -= dh;
                    wY      += dh;
                }
            }
            else if (mask & ResizeDownMask)
            {
                int dh = wY + wHeight - workArea.y2 ();

                if (0 < dh && dh < workAreaSnapDistance)
                {
                    he      -= dh;
                    wHeight -= dh;
                }
            }
        }
    }
}

void
ResizeLogic::setUpMask (int xRoot, int yRoot)
{
    int xDist, yDist;
    int minPointerOffsetX, minPointerOffsetY;

    CompWindow::Geometry server = w->serverGeometry ();

    xDist = xRoot - (server.x () + (server.width () / 2));
    yDist = yRoot - (server.y () + (server.height () / 2));

    /* decision threshold is 10% of window size */
    minPointerOffsetX = MIN (20, server.width () / 10);
    minPointerOffsetY = MIN (20, server.height () / 10);

    /* if we reached the threshold in one direction,
       make the threshold in the other direction smaller
       so there is a chance that this threshold also can
       be reached (by diagonal movement) */
    if (abs (xDist) > minPointerOffsetX)
	minPointerOffsetY /= 2;
    else if (abs (yDist) > minPointerOffsetY)
	minPointerOffsetX /= 2;

    if (abs (xDist) > minPointerOffsetX)
    {
	if (xDist > 0)
	    mask |= ResizeRightMask;
	else
	    mask |= ResizeLeftMask;
    }

    if (abs (yDist) > minPointerOffsetY)
    {
	if (yDist > 0)
	    mask |= ResizeDownMask;
	else
	    mask |= ResizeUpMask;
    }

    /* if the pointer movement was enough to determine a
       direction, warp the pointer to the appropriate edge
       and set the right cursor */
    if (mask)
    {
	Cursor     cursor;
	CompAction *action;
	int        pointerAdjustX = 0;
	int        pointerAdjustY = 0;

	action = &options->optionGetInitiateKey ();
	action->setState (action->state () |
			  CompAction::StateTermButton);

	if (mask & ResizeRightMask)
	    pointerAdjustX = server.x () + server.width () +
		w->border ().right - xRoot;
	else if (mask & ResizeLeftMask)
	    pointerAdjustX = server.x () - w->border ().left -
		xRoot;

	if (mask & ResizeDownMask)
	    pointerAdjustY = server.y () + server.height () +
		w->border ().bottom - yRoot;
	else if (mask & ResizeUpMask)
	    pointerAdjustY = server.y () - w->border ().top - yRoot;

	mScreen->warpPointer (pointerAdjustX, pointerAdjustY);

	cursor = cursorFromResizeMask (mask);
	mScreen->updateGrab (grabIndex, cursor);
    }
}

void
ResizeLogic::accumulatePointerMotion (int xRoot, int yRoot)
{
    /* only accumulate pointer movement if a mask is
       already set as we don't have a use for the
       difference information otherwise */

    if (centered || options->optionGetResizeFromCenter ())
    {
	pointerDx += (xRoot - lastPointerX) * 2;
	pointerDy += (yRoot - lastPointerY) * 2;
    }
    else
    {
	pointerDx += xRoot - lastPointerX;
	pointerDy += yRoot - lastPointerY;
    }

    /* If we hit the edge of the screen while resizing
     * the window and the adjacent window edge has not hit
     * the edge of the screen, then accumulate pointer motion
     * in the opposite direction. (So the apparant x / y
     * mixup here is intentional)
     */

    if (isConstrained)
    {
	if (mask == ResizeLeftMask)
	{
	    if (xRoot == 0 &&
		geometry.x - w->border ().left > grabWindowWorkArea->left ())
		pointerDx += abs (yRoot - lastPointerY) * -1;
	}
	else if (mask == ResizeRightMask)
	{
	    if (xRoot == mScreen->width () -1 &&
		geometry.x + geometry.width + w->border ().right < grabWindowWorkArea->right ())
		pointerDx += abs (yRoot - lastPointerY);
	}
	if (mask == ResizeUpMask)
	{
	    if (yRoot == 0 &&
		geometry.y - w->border ().top > grabWindowWorkArea->top ())
		pointerDy += abs (xRoot - lastPointerX) * -1;
	}
	else if (mask == ResizeDownMask)
	{
	    if (yRoot == mScreen->height () -1 &&
		geometry.y + geometry.height + w->border ().bottom < grabWindowWorkArea->bottom ())
		pointerDx += abs (yRoot - lastPointerY);
	}
    }
}

void
ResizeLogic::constrainToWorkArea (int &che, int &cwi)
{
    if (mask & ResizeUpMask)
    {
	int decorTop = savedGeometry.y + savedGeometry.height -
	    (che + w->border ().top);

	if (grabWindowWorkArea->y () > decorTop)
	    che -= grabWindowWorkArea->y () - decorTop;
    }
    if (mask & ResizeDownMask)
    {
	int decorBottom = savedGeometry.y + che + w->border ().bottom;

	if (decorBottom >
	    grabWindowWorkArea->y () + grabWindowWorkArea->height ())
	    che -= decorBottom - (grabWindowWorkArea->y () +
				  grabWindowWorkArea->height ());
    }
    if (mask & ResizeLeftMask)
    {
	int decorLeft = savedGeometry.x + savedGeometry.width -
	    (cwi + w->border ().left);

	if (grabWindowWorkArea->x () > decorLeft)
	    cwi -= grabWindowWorkArea->x () - decorLeft;
    }
    if (mask & ResizeRightMask)
    {
	int decorRight = savedGeometry.x + cwi + w->border ().right;

	if (decorRight >
	    grabWindowWorkArea->x () + grabWindowWorkArea->width ())
	    cwi -= decorRight - (grabWindowWorkArea->x () +
				 grabWindowWorkArea->width ());
    }
}

void
ResizeLogic::limitMovementToConstraintRegion (int &wi, int &he,
					       int xRoot, int yRoot,
					       int wX, int wY,
					       int wWidth, int wHeight)
{
    int minHeight = 50;

    /* rect. for a minimal height window + borders
       (used for the constraining in X axis) */
    int minimalInputHeight = minHeight +
	w->border ().top + w->border ().bottom;

    /* small hot-spot square (on window's corner or edge) that is to be
       constrained to the combined output work-area region */
    int x, y;
    int width = w->border ().top; /* square size = title bar height */
    int height = width;
    bool status; /* whether or not hot-spot is in the region */

    /* compute x & y for constrained hot-spot rect */
    if (mask & ResizeLeftMask)
	x = wX;
    else if (mask & ResizeRightMask)
	x = wX + wWidth - width;
    else
	x = MIN (MAX (xRoot, wX), wX + wWidth - width);

    if (mask & ResizeUpMask)
	y = wY;
    else if (mask & ResizeDownMask)
	y = wY + wHeight - height;
    else
	y = MIN (MAX (yRoot, wY), wY + wHeight - height);

    status = constraintRegion.contains (x, y, width, height);

    /* only constrain movement if previous position was valid */
    if (inRegionStatus)
    {
	bool xStatus = false;
	int yForXResize = y;
	int nx = x;
	int nw = wi;
	int nh = he;
	int minWidth  = 50;

	if (mask & (ResizeLeftMask | ResizeRightMask))
	{
	    xStatus = status;

	    if (mask & ResizeUpMask)
		yForXResize = wY + wHeight - minimalInputHeight;
	    else if (mask & ResizeDownMask)
		yForXResize = wY + minimalInputHeight - height;
	    else
		yForXResize = y;

	    if (!constraintRegion.contains (x, yForXResize,
					    width, height))
	    {
		if (lastGoodHotSpotY >= 0)
		    yForXResize = lastGoodHotSpotY;
		else
		    yForXResize = y;
	    }
	}
	if (mask & ResizeLeftMask)
	{
	    while ((nw > minWidth) && !xStatus)
	    {
		xStatus = constraintRegion.contains (nx, yForXResize,
						     width, height);
		if (!xStatus)
		{
		    nw--;
		    nx++;
		}
	    }
	    if (nw > minWidth)
	    {
		x = nx;
		wi = nw;
	    }
	}
	else if (mask & ResizeRightMask)
	{
	    while ((nw > minWidth) && !xStatus)
	    {
		xStatus = constraintRegion.contains (nx, yForXResize,
						     width, height);
		if (!xStatus)
		{
		    nw--;
		    nx--;
		}
	    }
	    if (nw > minWidth)
	    {
		x = nx;
		wi = nw;
	    }
	}

	if (mask & ResizeUpMask)
	{
	    while ((nh > minHeight) && !status)
	    {
		status = constraintRegion.contains (x, y,
						    width, height);
		if (!status)
		{
		    nh--;
		    y++;
		}
	    }
	    if (nh > minHeight)
		he = nh;
	}
	else if (mask & ResizeDownMask)
	{
	    while ((nh > minHeight) && !status)
	    {
		status = constraintRegion.contains (x, y,
						    width, height);
		if (!status)
		{
		    nh--;
		    y--;
		}
	    }
	    if (nh > minHeight)
		he = nh;
	}

	if (((mask & (ResizeLeftMask | ResizeRightMask)) && xStatus) ||
	    ((mask & (ResizeUpMask | ResizeDownMask)) && status))
	{
	    /* hot-spot inside work-area region, store good values */
	    lastGoodHotSpotY = y;
	    lastGoodSize     = CompSize (wi, he);
	}
	else
	{
	    /* failed to find a good hot-spot position, restore size */
	    wi = lastGoodSize.width ();
	    he = lastGoodSize.height ();
	}
    }
    else
    {
	inRegionStatus = status;
    }
}

void
ResizeLogic::computeWindowPlusBordersRect (int &wX, int &wY,
					    int &wWidth, int &wHeight,
					    int wi, int he)
{
    wWidth  = wi + w->border ().left + w->border ().right;
    wHeight = he + w->border ().top + w->border ().bottom;

    if (centered || options->optionGetResizeFromCenter ())
    {
	if (mask & ResizeLeftMask)
	    wX = geometry.x + geometry.width -
		(wi + w->border ().left);
	else
	    wX = geometry.x - w->border ().left;

	if (mask & ResizeUpMask)
	    wY = geometry.y + geometry.height -
		(he + w->border ().top);
	else
	    wY = geometry.y - w->border ().top;
    }
    else
    {
	if (mask & ResizeLeftMask)
	    wX = savedGeometry.x + savedGeometry.width -
		(wi + w->border ().left);
	else
	    wX = savedGeometry.x - w->border ().left;

	if (mask & ResizeUpMask)
	    wY = savedGeometry.y + savedGeometry.height -
		(he + w->border ().top);
	else
	    wY = savedGeometry.y - w->border ().top;
    }
}

void
ResizeLogic::enableOrDisableVerticalMaximization (int yRoot)
{
    /* maximum distance between the pointer and a work area edge (top or bottom)
       for a vertical maximization */
    const int max_edge_distance = 5;

    if (!options->optionGetMaximizeVertically())
	return;

    if (!offWorkAreaConstrained)
	return;

    if (centered || options->optionGetResizeFromCenter ())
    {
	if (maximized_vertically)
	{
	    geometry = geometryWithoutVertMax;
	    maximized_vertically = false;
	}
    }
    else if (mask & ResizeUpMask)
    {
	if (yRoot - grabWindowWorkArea->top() <= max_edge_distance
	    && !maximized_vertically)
	{
	    maximized_vertically = true;
	    geometryWithoutVertMax = geometry;
	}
	else if (yRoot - grabWindowWorkArea->top() > max_edge_distance
		 && maximized_vertically)
	{
	    geometry = geometryWithoutVertMax;
	    maximized_vertically = false;
	}
    }
    else if (mask & ResizeDownMask)
    {
	if (grabWindowWorkArea->bottom() - yRoot <= max_edge_distance
	    && !maximized_vertically)
	{
	    maximized_vertically = true;
	    geometryWithoutVertMax = geometry;
	}
	else if (grabWindowWorkArea->bottom() - yRoot > max_edge_distance
		 && maximized_vertically)
	{
	    geometry = geometryWithoutVertMax;
	    maximized_vertically = false;
	}
    }
}

void
ResizeLogic::computeGeometry(int wi, int he)
{
    XRectangle *regular_geometry;
    if (maximized_vertically)
	regular_geometry = &geometryWithoutVertMax;
    else
	regular_geometry = &geometry;

    if (centered || options->optionGetResizeFromCenter ())
    {
	if ((mask & ResizeLeftMask) || (mask & ResizeRightMask))
	    regular_geometry->x -= ((wi - regular_geometry->width) / 2);
	if ((mask & ResizeUpMask) || (mask & ResizeDownMask))
	    regular_geometry->y -= ((he - regular_geometry->height) / 2);
    }
    else
    {
	if (mask & ResizeLeftMask)
	    regular_geometry->x -= wi - regular_geometry->width;
	if (mask & ResizeUpMask)
	    regular_geometry->y -= he - regular_geometry->height;
    }

    regular_geometry->width  = wi;
    regular_geometry->height = he;

    if (maximized_vertically)
    {
	geometry.x = geometryWithoutVertMax.x;
	geometry.width = geometryWithoutVertMax.width;
	geometry.y = grabWindowWorkArea->y() + w->border().top;
	geometry.height = grabWindowWorkArea->height() - w->border().top
	    - w->border().bottom;
    }
}


bool
ResizeLogic::initiateResize (CompAction		*action,
			     CompAction::State	state,
			     CompOption::Vector	&options,
			     unsigned int	initMode)
{
    CompWindowInterface *w;
    Window     xid;

    xid = CompOption::getIntOptionNamed (options, "window");

    w = mScreen->findWindow (xid);
    if (w && (w->actions () & CompWindowActionResizeMask))
    {
	int          x, y;
	int	     button;

	CompWindow::Geometry server = w->serverGeometry ();

	x = CompOption::getIntOptionNamed (options, "x", pointerX);
	y = CompOption::getIntOptionNamed (options, "y", pointerY);

	button = CompOption::getIntOptionNamed (options, "button", -1);

	mask = CompOption::getIntOptionNamed (options, "direction");

	/* Initiate the resize in the direction suggested by the
	 * sector of the window the mouse is in, eg drag in top left
	 * will resize up and to the left.  Keyboard resize starts out
	 * with the cursor in the middle of the window and then starts
	 * resizing the edge corresponding to the next key press. */
	if (state & CompAction::StateInitKey)
	{
	    mask = 0;
	}
	else if (!mask)
	{
	    int sectorSizeX = server.width () / 3;
	    int sectorSizeY = server.height () / 3;
	    int posX        = x - server.x ();
	    int posY        = y - server.y ();

	    if (posX < sectorSizeX)
		mask |= ResizeLeftMask;
	    else if (posX > (2 * sectorSizeX))
		mask |= ResizeRightMask;

	    if (posY < sectorSizeY)
		mask |= ResizeUpMask;
	    else if (posY > (2 * sectorSizeY))
		mask |= ResizeDownMask;

	    /* if the pointer was in the middle of the window,
	       just prevent input to the window */

	    if (!mask)
		return true;
	}

	if (mScreen->otherGrabExist ("resize", NULL))
	    return false;

	if (this->w)
	    return false;

	if (w->type () & (CompWindowTypeDesktopMask |
		          CompWindowTypeDockMask	 |
		          CompWindowTypeFullscreenMask))
	    return false;

	if (w->overrideRedirect ())
	    return false;

	if (state & CompAction::StateInitButton)
	    action->setState (action->state () | CompAction::StateTermButton);

	if (w->shaded ())
	    mask &= ~(ResizeUpMask | ResizeDownMask);

	this->w		= w;

	savedGeometry.x		= server.x ();
	savedGeometry.y		= server.y ();
	savedGeometry.width	= server.width ();
	savedGeometry.height	= server.height ();

	geometry = savedGeometry;

	pointerDx = x - pointerX;
	pointerDy = y - pointerY;

	centered |= w->evaluate (this->options->optionGetResizeFromCenterMatch ());

	if ((w->state () & MAXIMIZE_STATE) == MAXIMIZE_STATE)
	{
	    /* if the window is fully maximized, showing the outline or
	       rectangle would be visually distracting as the window can't
	       be resized anyway; so we better don't use them in this case */
	    mode = ResizeOptions::ModeNormal;
	}
	else if (!gScreen || !cScreen ||
		 !cScreen->compositingActive ())
	{
	    mode = ResizeOptions::ModeNormal;
	}
	else
	{
	    mode = initMode;
	}

	if (mode != ResizeOptions::ModeNormal)
	{
	    if (w->getGLInterface () && mode == ResizeOptions::ModeStretch)
		w->getGLInterface ()->glPaintSetEnabled (true);
	    if (w->getCompositeInterface () && mode == ResizeOptions::ModeStretch)
		w->getCompositeInterface ()->damageRectSetEnabled (true);
	    gScreen->glPaintOutputSetEnabled (true);
	}

	if (!grabIndex)
	{
	    Cursor cursor;

	    if (state & CompAction::StateInitKey)
		cursor = middleCursor;
	    else
		cursor = cursorFromResizeMask (mask);

	    grabIndex = mScreen->pushGrab (cursor, "resize");
	}

	if (grabIndex)
	{
	    BoxRec box;
	    unsigned int grabMask = CompWindowGrabResizeMask |
				    CompWindowGrabButtonMask;
	    bool sourceExternalApp =
		CompOption::getBoolOptionNamed (options, "external", false);

	    if (sourceExternalApp)
		grabMask |= CompWindowGrabExternalAppMask;

	    releaseButton = button;

	    w->grabNotify (x, y, state, grabMask);

	    /* Click raise happens implicitly on buttons 1, 2 and 3 so don't
	     * restack this window again if the action buttonbinding was from
	     * one of those buttons */
	    if (mScreen->getOption ("raise_on_click")->value ().b () &&
		button != Button1 && button != Button2 && button != Button3)
		w->updateAttributes (CompStackingUpdateModeAboveFullscreen);

	    /* using the paint rectangle is enough here
	       as we don't have any stretch yet */
	    getPaintRectangle (&box);
	    damageRectangle (&box);

	    if (state & CompAction::StateInitKey)
	    {
		int xRoot, yRoot;

		xRoot = server.x () + (server.width () / 2);
		yRoot = server.y () + (server.height () / 2);

		mScreen->warpPointer (xRoot - pointerX, yRoot - pointerY);
	    }

	    isConstrained = sourceExternalApp;

	    /* Update offWorkAreaConstrained and workArea at grab time */
	    offWorkAreaConstrained = false;
	    if (sourceExternalApp)
	    {
		int output = w->outputDevice ();
		int lco, tco, bco, rco;
		bool sl = mScreen->outputDevs ().at (output).workArea ().left () >
			  w->serverGeometry ().left ();
		bool sr = mScreen->outputDevs ().at (output).workArea ().right () <
			  w->serverGeometry ().right ();
		bool st = mScreen->outputDevs ().at (output).workArea ().top () >
			  w->serverGeometry ().top ();
		bool sb = mScreen->outputDevs ().at (output).workArea ().bottom () <
			  w->serverGeometry ().bottom ();

		lco = tco = bco = rco = output;

		/* Prevent resizing beyond work area edges when resize is
		   initiated externally (e.g. with window frame or menu)
		   and not with a key (e.g. alt+button) */
		offWorkAreaConstrained = true;

		lco = getOutputForEdge (output, TOUCH_RIGHT, sl);
		rco = getOutputForEdge (output, TOUCH_LEFT, sr);
		tco = getOutputForEdge (output, TOUCH_BOTTOM, st);
		bco = getOutputForEdge (output, TOUCH_TOP, sb);

		/* Now we need to form one big rect which describes
		 * the available workarea */

		int left = mScreen->outputDevs ().at (lco).workArea ().left ();
		int right = mScreen->outputDevs ().at (rco).workArea ().right ();
		int top = mScreen->outputDevs ().at (tco).workArea ().top ();
		int bottom = mScreen->outputDevs ().at (bco).workArea ().bottom ();

		grabWindowWorkArea.reset (new CompRect (0, 0, 0, 0));
		{
		    grabWindowWorkArea->setLeft (left);
		    grabWindowWorkArea->setRight (right);
		    grabWindowWorkArea->setTop (top);
		    grabWindowWorkArea->setBottom (bottom);
		}


		inRegionStatus   = false;
		lastGoodHotSpotY = -1;
		lastGoodSize     = w->serverSize ();

		/* Combine the work areas of all outputs */
		constraintRegion = emptyRegion;
		foreach (CompOutput &output, mScreen->outputDevs ())
		    constraintRegion += output.workArea ();
	    }
	}

        maximized_vertically = false;
    }

    return false;
}

bool
ResizeLogic::terminateResize (CompAction        *action,
			      CompAction::State  state,
			      CompOption::Vector &options)
{
    if (w)
    {
	XWindowChanges xwc = XWINDOWCHANGES_INIT;
	unsigned int   mask = 0;

	if (mode == ResizeOptions::ModeNormal)
	{
	    if (state & CompAction::StateCancel)
	    {
		xwc.x      = savedGeometry.x;
		xwc.y      = savedGeometry.y;
		xwc.width  = savedGeometry.width;
		xwc.height = savedGeometry.height;

		mask = CWX | CWY | CWWidth | CWHeight;
	    }
	    else if (maximized_vertically)
	    {
		w->maximize (CompWindowStateMaximizedVertMask);

		xwc.x      = geometryWithoutVertMax.x;
		xwc.y      = geometryWithoutVertMax.y;
		xwc.width  = geometryWithoutVertMax.width;
		xwc.height = geometryWithoutVertMax.height;

		mask = CWX | CWY | CWWidth | CWHeight;
	    }
	}
	else
	{
	    XRectangle finalGeometry;

	    if (state & CompAction::StateCancel)
		finalGeometry = savedGeometry;
	    else
		finalGeometry = geometry;

	    if (memcmp (&finalGeometry, &savedGeometry, sizeof (finalGeometry)) == 0)
	    {
		BoxRec box;

		if (mode == ResizeOptions::ModeStretch)
		    getStretchRectangle (&box);
		else
		    getPaintRectangle (&box);

		damageRectangle (&box);
	    }
	    else
	    {
		if (maximized_vertically)
		{
		    w->maximize(CompWindowStateMaximizedVertMask);
		    xwc.x      = finalGeometry.x;
		    xwc.width  = finalGeometry.width;
		    mask = CWX | CWWidth ;
		}
		else
		{
		    xwc.x      = finalGeometry.x;
		    xwc.y      = finalGeometry.y;
		    xwc.width  = finalGeometry.width;
		    xwc.height = finalGeometry.height;
		    mask = CWX | CWY | CWWidth | CWHeight;
		}

	    }

	    if (mode != ResizeOptions::ModeNormal)
	    {
		if (w->getGLInterface () && mode == ResizeOptions::ModeStretch)
		    w->getGLInterface ()->glPaintSetEnabled (false);
		if (w->getCompositeInterface () && mode == ResizeOptions::ModeStretch)
		    w->getCompositeInterface ()->damageRectSetEnabled (false);
		gScreen->glPaintOutputSetEnabled (false);
	    }
	}

	if ((mask & CWWidth) &&
	    xwc.width == (int) w->serverGeometry ().width ())
	    mask &= ~CWWidth;

	if ((mask & CWHeight) &&
	    xwc.height == (int) w->serverGeometry ().height ())
	    mask &= ~CWHeight;

	if (mask)
	{
	    if (mask & (CWWidth | CWHeight))
		w->sendSyncRequest ();

	    w->configureXWindow (mask, &xwc);
	}

	finishResizing ();

	if (grabIndex)
	{
	    mScreen->removeGrab (grabIndex, NULL);
	    grabIndex = 0;
	}

	releaseButton = 0;
    }

    action->setState (action->state () & ~(CompAction::StateTermKey |
					   CompAction::StateTermButton));

    return false;
}

bool
ResizeLogic::initiateResizeDefaultMode (CompAction	    *action,
					CompAction::State   state,
					CompOption::Vector  &options)
{
    CompWindowInterface   *w;
    unsigned int mode;

    w = mScreen->findWindow (CompOption::getIntOptionNamed (options, "window"));
    if (!w)
	return false;

    mode = this->options->optionGetMode ();

    if (w->evaluate (this->options->optionGetNormalMatch ()))
	mode = ResizeOptions::ModeNormal;
    if (w->evaluate (this->options->optionGetOutlineMatch ()))
	mode = ResizeOptions::ModeOutline;
    if (w->evaluate (this->options->optionGetRectangleMatch ()))
	mode = ResizeOptions::ModeRectangle;
    if (w->evaluate (this->options->optionGetStretchMatch ()))
	mode = ResizeOptions::ModeStretch;

    return initiateResize (action, state, options, mode);
}

void
ResizeLogic::damageRectangle (BoxPtr pBox)
{
    int x1, x2, y1, y2;

    x1 = pBox->x1 - 1;
    y1 = pBox->y1 - 1;
    x2 = pBox->x2 + 1;
    y2 = pBox->y2 + 1;

    if (cScreen)
	cScreen->damageRegion (CompRect (x1, y1, x2 - x1, y2 - y1));
}

/* Be a little bit intelligent about how we calculate
 * the workarea. Basically we want to be enclosed in
 * any area that is obstructed by panels, but not
 * where two outputs meet
 *
 * Also, it does not make sense to resize over
 * non-touching outputs, so detect that case too
 * */

int
ResizeLogic::getOutputForEdge (int windowOutput, unsigned int touch, bool skipFirst)
{
    int op, wap;
    int ret = windowOutput;

    getPointForTp (touch, windowOutput, op, wap);

    if ((op == wap) || skipFirst)
    {
	int co = windowOutput;

	do
	{
	    int oco = co;

	    co = findTouchingOutput (op, touch);

	    /* Could not find a leftmost output from here
	     * so we must have hit the edge of the universe */
	    if (co == -1)
	    {
		ret = oco;
		co = -1;
		break;
	    }

	    getPointForTp (touch, co, op, wap);

	    /* There is something in the way here.... */
	    if (op != wap)
	    {
		ret = co;
		co = -1;
	    }
	}
        while (co != -1);
    }

    return ret;
}

unsigned int
ResizeLogic::findTouchingOutput (int touchPoint, unsigned int side)
{
    for (unsigned int i = 0; i < mScreen->outputDevs ().size (); i++)
    {
	CompOutput &o = mScreen->outputDevs ().at (i);
	if (side == TOUCH_LEFT)
	{
	    if (o.left () == touchPoint)
		return  i;
	}
	if (side == TOUCH_RIGHT)
	{
	    if (o.right () == touchPoint)
		return  i;
	}
	if (side == TOUCH_TOP)
	{
	    if (o.top () == touchPoint)
		return  i;
	}
	if (side == TOUCH_BOTTOM)
	{
	    if (o.bottom () == touchPoint)
		return  i;
	}
    }

    return -1;
}

void
ResizeLogic::getPointForTp (unsigned int tp,
			    unsigned int output,
			    int &op,
			    int &wap)
{
    CompRect og = CompRect (mScreen->outputDevs ().at (output));
    CompRect wag = mScreen->outputDevs ().at (output).workArea ();

    switch (tp)
    {
	case TOUCH_LEFT:
	    op = og.right ();
	    wap = wag.right ();
	    break;
	case TOUCH_RIGHT:
	    op = og.left ();
	    wap = wag.left ();
	    break;
	case TOUCH_TOP:
	    op = og.bottom ();
	    wap = wag.bottom ();
	    break;
	case TOUCH_BOTTOM:
	    op = og.top ();
	    wap = wag.top ();
	    break;
	default:
	    return;
    }
}
