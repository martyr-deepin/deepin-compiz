/*
 * Copyright © 2005 Novell, Inc.
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
 * Author: David Reveman <davidr@novell.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/cursorfont.h>

#include <core/atoms.h>
#include "move.h"

COMPIZ_PLUGIN_20090315 (move, MovePluginVTable)

const unsigned short KEY_MOVE_INC = 24;

const unsigned short SNAP_BACK = 20;
const unsigned short SNAP_OFF  = 100;

static bool
moveInitiate (CompAction      *action,
	      CompAction::State state,
	      CompOption::Vector &options)
{
    CompWindow *w;
    Window     xid;

    MOVE_SCREEN (screen);

    xid = CompOption::getIntOptionNamed (options, "window");

    w = screen->findWindow (xid);
    if (w && (w->actions () & CompWindowActionMoveMask))
    {
	CompRect     workArea;
	unsigned int mods;
	int          x, y, button;
	bool         sourceExternalApp;

	CompScreen *s = screen;

	mods = CompOption::getIntOptionNamed (options, "modifiers", 0);

	x = CompOption::getIntOptionNamed (options, "x", w->geometry ().x () +
					   (w->size ().width () / 2));
	y = CompOption::getIntOptionNamed (options, "y", w->geometry ().y () +
					   (w->size ().height () / 2));

	button = CompOption::getIntOptionNamed (options, "button", -1);

	if (s->otherGrabExist ("move", NULL))
	    return false;

	if (ms->w)
	    return false;

	if (w->type () & (CompWindowTypeDesktopMask |
		       CompWindowTypeDockMask    |
		       CompWindowTypeFullscreenMask))
	    return false;

	if (w->overrideRedirect ())
	    return false;

	if (state & CompAction::StateInitButton)
	    action->setState (action->state () | CompAction::StateTermButton);

	if (ms->region)
	{
	    XDestroyRegion (ms->region);
	    ms->region = NULL;
	}

	ms->status = RectangleOut;

	ms->savedX = w->serverGeometry ().x ();
	ms->savedY = w->serverGeometry ().y ();

	ms->x = 0;
	ms->y = 0;

	lastPointerX = x;
	lastPointerY = y;

	sourceExternalApp =
	    CompOption::getBoolOptionNamed (options, "external", false);
	ms->yConstrained = sourceExternalApp && ms->optionGetConstrainY ();

	ms->origState = w->state ();

	workArea = s->getWorkareaForOutput (w->outputDevice ());

	ms->snapBackY = w->serverGeometry ().y () - workArea.y ();
	ms->snapOffY  = y - workArea.y ();

	if (!ms->grab)
	    ms->grab = s->pushGrab (ms->moveCursor, "move");

	if (ms->grab)
	{
	    unsigned int grabMask = CompWindowGrabMoveMask |
				    CompWindowGrabButtonMask;

	    if (sourceExternalApp)
		grabMask |= CompWindowGrabExternalAppMask;

	    ms->w = w;

	    ms->releaseButton = button;

	    w->grabNotify (x, y, mods, grabMask);

	    /* Click raise happens implicitly on buttons 1, 2 and 3 so don't
	     * restack this window again if the action buttonbinding was from
	     * one of those buttons */
	    if (screen->getOption ("raise_on_click")->value ().b () &&
		button != Button1 && button != Button2 && button != Button3)
		w->updateAttributes (CompStackingUpdateModeAboveFullscreen);

	    if (state & CompAction::StateInitKey)
	    {
		int xRoot, yRoot;

		xRoot = w->geometry ().x () + (w->size ().width () / 2);
		yRoot = w->geometry ().y () + (w->size ().height () / 2);

		s->warpPointer (xRoot - pointerX, yRoot - pointerY);
	    }

	    if (ms->moveOpacity != OPAQUE)
	    {
		MOVE_WINDOW (w);

		if (mw->cWindow)
		    mw->cWindow->addDamage ();
		if (mw->gWindow)
		mw->gWindow->glPaintSetEnabled (mw, true);
	    }
	}
    }

    return false;
}

static bool
moveTerminate (CompAction      *action,
	       CompAction::State state,
	       CompOption::Vector &options)
{
    MOVE_SCREEN (screen);

    if (ms->w)
    {
	if (state & CompAction::StateCancel)
	    ms->w->move (ms->savedX - ms->w->geometry ().x (),
			 ms->savedY - ms->w->geometry ().y (), false);

	ms->w->syncPosition ();

	/* update window attributes as window constraints may have
	   changed - needed e.g. if a maximized window was moved
	   to another output device */
	ms->w->updateAttributes (CompStackingUpdateModeNone);

	ms->w->ungrabNotify ();

	if (ms->grab)
	{
	    screen->removeGrab (ms->grab, NULL);
	    ms->grab = NULL;
	}

	if (ms->moveOpacity != OPAQUE)
	{
	    MOVE_WINDOW (ms->w);

	    if (mw->cWindow)
		mw->cWindow->addDamage ();
	    if (mw->gWindow)
		mw->gWindow->glPaintSetEnabled (mw, false);
	}

	ms->w             = 0;
	ms->releaseButton = 0;
    }

    action->setState (action->state () & ~(CompAction::StateTermKey |
					   CompAction::StateTermButton));

    return false;
}

/* creates a region containing top and bottom struts. only struts that are
   outside the screen workarea are considered. */
static Region
moveGetYConstrainRegion (CompScreen *s)
{
    CompWindow   *w;
    Region       region;
    REGION       r;
    CompRect     workArea;
    BoxRec       extents;

    region = XCreateRegion ();
    if (!region)
	return NULL;

    r.rects    = &r.extents;
    r.numRects = r.size = 1;

    r.extents.x1 = MINSHORT;
    r.extents.y1 = 0;
    r.extents.x2 = 0;
    r.extents.y2 = s->height ();

    XUnionRegion (&r, region, region);

    r.extents.x1 = s->width ();
    r.extents.x2 = MAXSHORT;

    XUnionRegion (&r, region, region);

    for (unsigned int i = 0; i < s->outputDevs ().size (); i++)
    {
	XUnionRegion (s->outputDevs ()[i].region (), region, region);

	workArea = s->getWorkareaForOutput (i);
	extents = s->outputDevs ()[i].region ()->extents;

	foreach (w, s->windows ())
	{
	    if (!w->mapNum ())
		continue;

	    if (w->struts ())
	    {
		r.extents.x1 = w->struts ()->top.x;
		r.extents.y1 = w->struts ()->top.y;
		r.extents.x2 = r.extents.x1 + w->struts ()->top.width;
		r.extents.y2 = r.extents.y1 + w->struts ()->top.height;

		if (r.extents.x1 < extents.x1)
		    r.extents.x1 = extents.x1;
		if (r.extents.x2 > extents.x2)
		    r.extents.x2 = extents.x2;
		if (r.extents.y1 < extents.y1)
		    r.extents.y1 = extents.y1;
		if (r.extents.y2 > extents.y2)
		    r.extents.y2 = extents.y2;

		if (r.extents.x1 < r.extents.x2 && r.extents.y1 < r.extents.y2)
		{
		    if (r.extents.y2 <= workArea.y ())
			XSubtractRegion (region, &r, region);
		}

		r.extents.x1 = w->struts ()->bottom.x;
		r.extents.y1 = w->struts ()->bottom.y;
		r.extents.x2 = r.extents.x1 + w->struts ()->bottom.width;
		r.extents.y2 = r.extents.y1 + w->struts ()->bottom.height;

		if (r.extents.x1 < extents.x1)
		    r.extents.x1 = extents.x1;
		if (r.extents.x2 > extents.x2)
		    r.extents.x2 = extents.x2;
		if (r.extents.y1 < extents.y1)
		    r.extents.y1 = extents.y1;
		if (r.extents.y2 > extents.y2)
		    r.extents.y2 = extents.y2;

		if (r.extents.x1 < r.extents.x2 && r.extents.y1 < r.extents.y2)
		{
		    if (r.extents.y1 >= workArea.bottom ())
			XSubtractRegion (region, &r, region);
		}
	    }
	}
    }

    return region;
}

static void
moveHandleMotionEvent (CompScreen *s,
		       int	  xRoot,
		       int	  yRoot)
{
    MOVE_SCREEN (s);

    if (ms->grab)
    {
	int	     dx, dy;
	int	     wX, wY;
	int          wWidth, wHeight;
	CompWindow   *w;

	w = ms->w;

	wX      = w->geometry ().x ();
	wY      = w->geometry ().y ();
	wWidth  = w->geometry ().widthIncBorders ();
	wHeight = w->geometry ().heightIncBorders ();

	ms->x += xRoot - lastPointerX;
	ms->y += yRoot - lastPointerY;

	if (w->type () & CompWindowTypeFullscreenMask)
	{
	    dx = dy = 0;
	}
	else
	{
	    CompRect workArea;
	    int	     min, max;

	    dx = ms->x;
	    dy = ms->y;

	    workArea = s->getWorkareaForOutput (w->outputDevice ());

	    if (ms->yConstrained)
	    {
		if (!ms->region)
		    ms->region = moveGetYConstrainRegion (s);

		/* make sure that the top border extents or the top row of
		   pixels are within what is currently our valid screen
		   region */
		if (ms->region)
		{
		    int x, y, width, height;
		    int status;

		    x	   = wX + dx - w->border ().left;
		    y	   = wY + dy - w->border ().top;
		    width  = wWidth + w->border ().left + w->border ().right;
		    height = w->border ().top ? w->border ().top : 1;

		    status = XRectInRegion (ms->region, x, y,
					    (unsigned int) width,
					    (unsigned int) height);

		    /* only constrain movement if previous position was valid */
		    if (ms->status == RectangleIn)
		    {
			int xStatus = status;

			while (dx && xStatus != RectangleIn)
			{
			    xStatus = XRectInRegion (ms->region,
						     x, y - dy,
						     (unsigned int) width,
						     (unsigned int) height);

			    if (xStatus != RectangleIn)
				dx += (dx < 0) ? 1 : -1;

			    x = wX + dx - w->border ().left;
			}

			while (dy && status != RectangleIn)
			{
			    status = XRectInRegion (ms->region,
						    x, y,
						    (unsigned int) width,
						    (unsigned int) height);

			    if (status != RectangleIn)
				dy += (dy < 0) ? 1 : -1;

			    y = wY + dy - w->border ().top;
			}
		    }
		    else
		    {
			ms->status = status;
		    }
		}
	    }

	    if (ms->optionGetSnapoffMaximized ())
	    {
		if (w->state () & CompWindowStateMaximizedVertMask)
		{
		    if (abs (yRoot - workArea.y () - ms->snapOffY) >= SNAP_OFF)
		    {
			if (!s->otherGrabExist ("move", NULL))
			{
			    int width = w->serverGeometry ().width ();

			    w->saveMask () |= CWX | CWY;

			    if (w->saveMask ()& CWWidth)
				width = w->saveWc ().width;

			    w->saveWc ().x = xRoot - (width >> 1);
			    w->saveWc ().y = yRoot + (w->border ().top >> 1);

			    ms->x = ms->y = 0;

			    w->maximize (0);

			    ms->snapOffY = ms->snapBackY;

			    return;
			}
		    }
		}
		else if (ms->origState & CompWindowStateMaximizedVertMask)
		{
		    if (abs (yRoot - workArea.y () - ms->snapBackY) < SNAP_BACK)
		    {
			if (!s->otherGrabExist ("move", NULL))
			{
			    int wy;

			    /* update server position before maximizing
			       window again so that it is maximized on
			       correct output */
			    w->syncPosition ();

			    w->maximize (ms->origState);

			    wy  = workArea.y () + (w->border ().top >> 1);
			    wy += w->sizeHints ().height_inc >> 1;

			    s->warpPointer (0, wy - pointerY);

			    return;
			}
		    }
		}
	    }

	    if (w->state () & CompWindowStateMaximizedVertMask)
	    {
		min = workArea.y () + w->border ().top;
		max = workArea.bottom () - w->border ().bottom - wHeight;

		if (wY + dy < min)
		    dy = min - wY;
		else if (wY + dy > max)
		    dy = max - wY;
	    }

	    if (w->state () & CompWindowStateMaximizedHorzMask)
	    {
		if (wX > (int) s->width () ||
		    wX + w->size ().width () < 0)
		    return;

		if (wX + wWidth < 0)
		    return;

		min = workArea.x () + w->border ().left;
		max = workArea.right () - w->border ().right - wWidth;

		if (wX + dx < min)
		    dx = min - wX;
		else if (wX + dx > max)
		    dx = max - wX;
	    }
	}

	if (dx || dy)
	{
	    w->move (wX + dx - w->geometry ().x (),
		     wY + dy - w->geometry ().y (), false);

	    if (!ms->optionGetLazyPositioning ())
		w->syncPosition ();

	    ms->x -= dx;
	    ms->y -= dy;
	}
    }
}

void
MoveScreen::handleEvent (XEvent *event)
{
    switch (event->type) {
	case ButtonPress:
	case ButtonRelease:
	    if (event->xbutton.root == screen->root ())
	    {
		if (grab)
		{
		    if (releaseButton == -1 ||
			releaseButton == (int) event->xbutton.button)
		    {
			moveTerminate (&optionGetInitiateButton (),
				       CompAction::StateTermButton,
				       noOptions ());
		    }
		}
	    }
	    break;
	case KeyPress:
	    if (event->xkey.root == screen->root ())
	    {
		if (grab)
		{
		    for (unsigned int i = 0; i < NUM_KEYS; i++)
		    {
			if (event->xkey.keycode == key[i])
			{
			    XWarpPointer (screen->dpy (), None, None,
					  0, 0, 0, 0,
					  mKeys[i].dx * KEY_MOVE_INC,
					  mKeys[i].dy * KEY_MOVE_INC);
			    break;
			}
		    }
		}
	    }
	    break;
	case MotionNotify:
	    if (event->xmotion.root == screen->root ())
		moveHandleMotionEvent (screen, pointerX, pointerY);
	    break;
	case EnterNotify:
	case LeaveNotify:
	    if (event->xcrossing.root == screen->root ())
		moveHandleMotionEvent (screen, pointerX, pointerY);
	    break;
	case ClientMessage:
	    if (event->xclient.message_type == Atoms::wmMoveResize)
	    {
		unsigned   long type = (unsigned long) event->xclient.data.l[2];

		MOVE_SCREEN (screen);

		if (type == WmMoveResizeMove ||
		    type == WmMoveResizeMoveKeyboard)
		{
		    CompWindow *w;
		    w = screen->findWindow (event->xclient.window);
		    if (w)
		    {
			CompOption::Vector o;

			o.push_back (CompOption ("window", CompOption::TypeInt));
			o[0].value ().set ((int) event->xclient.window);

			o.push_back (CompOption ("external",
				     CompOption::TypeBool));
			o[1].value ().set (true);

			if (event->xclient.data.l[2] == WmMoveResizeMoveKeyboard)
			{
			    moveInitiate (&optionGetInitiateKey (),
					  CompAction::StateInitKey, o);
			}
			else
			{

			    /* TODO: not only button 1 */
			    if (pointerMods & Button1Mask)
			    {
				o.push_back (CompOption ("modifiers", CompOption::TypeInt));
				o[2].value ().set ((int) pointerMods);

				o.push_back (CompOption ("x", CompOption::TypeInt));
				o[3].value ().set ((int) event->xclient.data.l[0]);

				o.push_back (CompOption ("y", CompOption::TypeInt));
				o[4].value ().set ((int) event->xclient.data.l[1]);

				o.push_back (CompOption ("button", CompOption::TypeInt));
				o[5].value ().set ((int) (event->xclient.data.l[3] ?
					       event->xclient.data.l[3] : -1));

				moveInitiate (&optionGetInitiateButton (),
					      CompAction::StateInitButton, o);

				moveHandleMotionEvent (screen, pointerX, pointerY);
			    }
			}
		    }
		}
		else if (ms->w && type == WmMoveResizeCancel)
		{
		    if (ms->w->id () == event->xclient.window)
		    {
			moveTerminate (&optionGetInitiateButton (),
				       CompAction::StateCancel, noOptions ());
			moveTerminate (&optionGetInitiateKey (),
				       CompAction::StateCancel, noOptions ());

		    }
		}
	    }
	    break;
	case DestroyNotify:
	    if (w && w->id () == event->xdestroywindow.window)
	    {
		moveTerminate (&optionGetInitiateButton (), 0, noOptions ());
		moveTerminate (&optionGetInitiateKey (), 0, noOptions ());
	    }
	    break;
	case UnmapNotify:
	    if (w && w->id () == event->xunmap.window)
	    {
		moveTerminate (&optionGetInitiateButton (), 0, noOptions ());
		moveTerminate (&optionGetInitiateKey (), 0, noOptions ());
	    }
	default:
	    break;
    }

    screen->handleEvent (event);
}

bool
MoveWindow::glPaint (const GLWindowPaintAttrib &attrib,
		     const GLMatrix            &transform,
		     const CompRegion          &region,
		     unsigned int              mask)
{
    GLWindowPaintAttrib sAttrib = attrib;
    bool                status;

    MOVE_SCREEN (screen);

    if (ms->grab)
    {
	if (ms->w == window && ms->moveOpacity != OPAQUE)
	{
	    /* modify opacity of windows that are not active */
	    sAttrib.opacity = (sAttrib.opacity * ms->moveOpacity) >> 16;
	}
    }

    status = gWindow->glPaint (sAttrib, transform, region, mask);

    return status;
}

void
MoveScreen::updateOpacity ()
{
    moveOpacity = (optionGetOpacity () * OPAQUE) / 100;
}

bool
MoveScreen::registerPaintHandler(compiz::composite::PaintHandler *pHnd)
{
    hasCompositing = true;
    cScreen->registerPaintHandler (pHnd);
    return true;
}

void
MoveScreen::unregisterPaintHandler()
{
    hasCompositing = false;
    cScreen->unregisterPaintHandler ();
}

MoveScreen::MoveScreen (CompScreen *screen) :
    PluginClassHandler<MoveScreen,CompScreen> (screen),
    cScreen (CompositeScreen::get (screen)),
    w (0),
    region (NULL),
    status (RectangleOut),
    releaseButton (0),
    grab (NULL),
    hasCompositing (false),
    yConstrained (false)
{
    updateOpacity ();

    for (unsigned int i = 0; i < NUM_KEYS; i++)
	key[i] = XKeysymToKeycode (screen->dpy (),
				   XStringToKeysym (mKeys[i].name));

    moveCursor = XCreateFontCursor (screen->dpy (), XC_fleur);
    if (cScreen)
    {
	CompositeScreenInterface::setHandler (cScreen);
	hasCompositing =
	    cScreen->compositingActive ();
    }

    optionSetOpacityNotify (boost::bind (&MoveScreen::updateOpacity, this));

    optionSetInitiateButtonInitiate (moveInitiate);
    optionSetInitiateButtonTerminate (moveTerminate);

    optionSetInitiateKeyInitiate (moveInitiate);
    optionSetInitiateKeyTerminate (moveTerminate);

    ScreenInterface::setHandler (screen);
}

MoveScreen::~MoveScreen ()
{
    if (region)
	XDestroyRegion (region);

    if (moveCursor)
	XFreeCursor (screen->dpy (), moveCursor);
}

bool
MovePluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION))
	 return false;

    return true;
}

