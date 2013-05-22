/*
 * Copyright (C) 2001 Havoc Pennington
 * Copyright (C) 2002, 2003 Red Hat, Inc.
 * Copyright (C) 2003 Rob Adams
 * Copyright (C) 2005 Novell, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include "place.h"

COMPIZ_PLUGIN_20090315 (place, PlacePluginVTable)

#define XWINDOWCHANGES_INIT {0, 0, 0, 0, 0, None, 0}

PlaceScreen::PlaceScreen (CompScreen *screen) :
    PluginClassHandler<PlaceScreen, CompScreen> (screen),
    mPrevSize (screen->width (), screen->height ()),
    mStrutWindowCount (0),
    fullPlacementAtom (XInternAtom (screen->dpy (),
    				    "_NET_WM_FULL_PLACEMENT", 0))
{
    ScreenInterface::setHandler (screen);
    mResChangeFallbackHandle.setTimes (4000, 4500); /* 4 Seconds */

    screen->updateSupportedWmHints ();
}

PlaceScreen::~PlaceScreen ()
{
    screen->addSupportedAtomsSetEnabled (this, false);

    mResChangeFallbackHandle.stop ();
    screen->updateSupportedWmHints ();
}

CompWindowList
compiz::place::collectStrutWindows (const CompWindowList &all)
{
    CompWindowList l;

    foreach (CompWindow *w, all)
    {
	if (!w->managed () ||
	    w->overrideRedirect ())
	    continue;

	if (w->struts ())
	    l.push_back (w);
    }

    return l;
}

void
PlaceScreen::doHandleScreenSizeChange (int  newWidth,
				       int  newHeight)
{
    foreach (CompWindow *w, screen->windows ())
    {
	if (!w->managed ())
	    continue;

	if (w->wmType () & (CompWindowTypeDockMask |
			    CompWindowTypeDesktopMask))
	    continue;

	PlaceWindow::get (w)->adjustForSize (mPrevSize, CompSize (newWidth, newHeight));
    }
}

const compiz::window::Geometry &
PlaceWindow::getGeometry () const
{
    return window->serverGeometry ();
}

const CompPoint &
PlaceWindow::getViewport () const
{
    return screen->vp ();
}

const CompRect &
PlaceWindow::getWorkarea (const compiz::window::Geometry &g) const
{
    return screen->getWorkareaForOutput (screen->outputDeviceForGeometry (g));
}

const CompRect &
PlaceWindow::getWorkarea () const
{
    return getWorkarea (window->serverGeometry ());
}

const compiz::window::extents::Extents &
PlaceWindow::getExtents () const
{
    return window->border ();
}

unsigned int
PlaceWindow::getState () const
{
    unsigned int state = 0;

    if (window->state () & CompWindowStateAboveMask)
	state |= compiz::place::WindowAbove;
    if (window->state () & CompWindowStateBelowMask)
	state |= compiz::place::WindowBelow;

    return state;
}

void
PlaceWindow::applyGeometry (compiz::window::Geometry &ng,
			    compiz::window::Geometry &og)
{
    CompRect workArea = screen->getWorkareaForOutput (
			    screen->outputDeviceForGeometry (og));

    XWindowChanges xwc = XWINDOWCHANGES_INIT;
    unsigned int   mask = og.changeMask (ng);

    xwc.x = ng.x ();
    xwc.y = ng.y ();
    xwc.width = ng.width ();
    xwc.height = ng.height ();
    xwc.border_width = ng.border ();

    window->configureXWindow (mask, &xwc);

    if ((window->actions () & MAXIMIZE_STATE) == MAXIMIZE_STATE &&
	(window->mwmDecor () & (MwmDecorAll | MwmDecorTitle))   &&
	!(window->state () & CompWindowStateFullscreenMask))
    {
	if (og.width () >= workArea.width () &&
	    og.height () >= workArea.height ())
	{
	    sendMaximizationRequest ();
	}
    }
}

bool
PlaceScreen::handleScreenSizeChangeFallback (int width,
					     int height)
{
    /* If countdown is not finished yet (i.e. at least one struct window didn't
     * update its struts), reset the count down and move windows around here */

    if (mStrutWindowCount > 0) /* no windows with struts found */
    {
	mStrutWindowCount = 0;
	doHandleScreenSizeChange (width, height);
    }

    return false;
}

void
PlaceScreen::handleScreenSizeChange (int width,
				     int height)
{
    if (mPrevSize == CompSize (width, height))
	return;

    mResChangeFallbackHandle.stop ();
    mStrutWindows = compiz::place::collectStrutWindows (screen->windows ());

    /* Don't wait for strut windows to update if there are none */
    if (mStrutWindows.empty ())
	doHandleScreenSizeChange (width, height);
    else
    {
	/* Wait for windows with set struts to update their struts, but
	 * if one of them isn't updating them, have a fallback to ignore them */
	mResChangeFallbackHandle.setCallback (
		      boost::bind (&PlaceScreen::handleScreenSizeChangeFallback,
				   this, width, height));
	mResChangeFallbackHandle.start ();
    }

}

void
PlaceScreen::handleEvent (XEvent *event)
{
    if (event->type == ConfigureNotify &&
	event->xconfigure.window == screen->root ())
    {
	mPrevSize.setWidth (screen->width ());
	mPrevSize.setHeight (screen->height ());
    }

    screen->handleEvent (event);

    switch (event->type)
    {
	case ConfigureNotify:
	    {
		if (event->xconfigure.window == screen->root ())
		{
		    handleScreenSizeChange (event->xconfigure.width,
					    event->xconfigure.height);
		}
	    }
	    break;
	case PropertyNotify:
	    if (event->xproperty.atom == Atoms::wmStrut ||
	        event->xproperty.atom == Atoms::wmStrutPartial)
	    {
	        CompWindow *w;

	        w = screen->findWindow (event->xproperty.window);
	        if (w)
	        {
		    if (!mStrutWindows.empty ())
		    {
			mStrutWindows.remove (w);
			/* Only do when handling screen size change.
			   ps->strutWindowCount is 0 at any other time */
			if (mStrutWindows.empty ())
			    doHandleScreenSizeChange (screen->width (),
						      screen->height ()); /* 2nd pass */
		    }
	        }
	    }
    }
}

/* sort functions */

static bool
compareLeftmost (compiz::place::Placeable *a,
		 compiz::place::Placeable *b)
{
    int ax, bx;

    ax = a->geometry ().x () - a->extents ().left;
    bx = b->geometry ().x () - a->extents ().left;

    return (ax <= bx);
}

static bool
compareTopmost (compiz::place::Placeable *a,
		compiz::place::Placeable *b)
{
    int ay, by;

    ay = a->geometry ().y () - a->extents ().top;
    by = b->geometry ().y () - a->extents ().top;

    return (ay <= by);
}

static bool
compareNorthWestCorner (compiz::place::Placeable *a,
			compiz::place::Placeable *b)
{
    int fromOriginA;
    int fromOriginB;
    int ax, ay, bx, by;

    ax = a->geometry ().x () - a->extents ().left;
    bx = b->geometry ().x () - a->extents ().left;

    ay = a->geometry ().y () - a->extents ().top;
    by = b->geometry ().y () - a->extents ().top;

    /* probably there's a fast good-enough-guess we could use here. */
    fromOriginA = sqrt (ax * ax + ay * ay);
    fromOriginB = sqrt (bx * bx + by * by);

    return (fromOriginA <= fromOriginB);
}

PlaceWindow::PlaceWindow (CompWindow *w) :
    PluginClassHandler<PlaceWindow, CompWindow> (w),
    compiz::place::ScreenSizeChangeObject (w->serverGeometry ()),
    window (w),
    ps (PlaceScreen::get (screen))
{
    WindowInterface::setHandler (w);
}

PlaceWindow::~PlaceWindow ()
{
    if (!ps->mStrutWindows.empty() && window->struts())
    {
	ps->mStrutWindows.remove(window);
	if (ps->mStrutWindows.empty())
	{
	    ps->doHandleScreenSizeChange(screen->width(), screen->height());
	}
    }
}

bool
PlaceWindow::place (CompPoint &pos)
{
    bool      status = window->place (pos);
    CompPoint viewport;

    if (status)
	return status;

    doPlacement (pos);
    if (matchViewport (viewport))
    {
	int x, y;

	viewport.setX (MAX (MIN (viewport.x (),
				 screen->vpSize ().width () - 1), 0));
	viewport.setY (MAX (MIN (viewport.y (),
				 screen->vpSize ().height () - 1), 0));

	x = pos.x () % screen->width ();
	if (x < 0)
	    x += screen->width ();
	y = pos.y () % screen->height ();
	if (y < 0)
	    y += screen->height ();

	pos.setX (x + (viewport.x () - screen->vp ().x ()) * screen->width ());
	pos.setY (y + (viewport.y () - screen->vp ().y ()) * screen->height ());
    }

    return true;
}

CompRect
PlaceWindow::doValidateResizeRequest (unsigned int &mask,
				      XWindowChanges *xwc,
				      bool	   sizeOnly,
				      bool	   clampToViewport)
{
    CompRect workArea;
    int	     x, y, left, right, bottom, top;
    CompWindow::Geometry geom;
    int      output;

    geom.set (xwc->x, xwc->y, xwc->width, xwc->height,
	      window->serverGeometry ().border ());

    if (clampToViewport)
    {
	/* left, right, top, bottom target coordinates, clamed to viewport
	 * sizes as we don't need to validate movements to other viewports;
	 * we are only interested in inner-viewport movements */

	x = geom.x () % screen->width ();
	if ((geom.x2 ()) < 0)
	    x += screen->width ();

	y = geom.y () % screen->height ();
	if ((geom.y2 ()) < 0)
	    y += screen->height ();
    }
    else
    {
	x = geom.x ();
	y = geom.y ();
    }

    left   = x - window->border ().left;
    right  = left + geom.widthIncBorders () +  (window->border ().left +
						window->border ().right);
    top    = y - window->border ().top;
    bottom = top + geom.heightIncBorders () + (window->border ().top +
					       window->border ().bottom);

    output   = screen->outputDeviceForGeometry (geom);
    workArea = screen->getWorkareaForOutput (output);

    if (clampToViewport &&
    	xwc->width >= workArea.width () &&
	xwc->height >= workArea.height ())
    {
	if ((window->actions () & MAXIMIZE_STATE) == MAXIMIZE_STATE &&
	    (window->mwmDecor () & (MwmDecorAll | MwmDecorTitle))   &&
	    !(window->state () & CompWindowStateFullscreenMask))
	{
	    sendMaximizationRequest ();
	}
    }

    if ((right - left) > workArea.width ())
    {
	left  = workArea.left ();
	right = workArea.right ();
    }
    else
    {
	if (left < workArea.left ())
	{
	    right += workArea.left () - left;
	    left  = workArea.left ();
	}

	if (right > workArea.right ())
	{
	    left -= right - workArea.right ();
	    right = workArea.right ();
	}
    }

    if ((bottom - top) > workArea.height ())
    {
	top    = workArea.top ();
	bottom = workArea.bottom ();
    }
    else
    {
	if (top < workArea.top ())
	{
	    bottom += workArea.top () - top;
	    top    = workArea.top ();
	}

	if (bottom > workArea.bottom ())
	{
	    top   -= bottom - workArea.bottom ();
	    bottom = workArea.bottom ();
	}
    }

    /* bring left/right/top/bottom to actual window coordinates */
    left   += window->border ().left;
    right  -= window->border ().right + 2 * window->serverGeometry ().border ();
    top    += window->border ().top;
    bottom -= window->border ().bottom + 2 * window->serverGeometry ().border ();

    /* always validate position if the application changed only its size,
     * as it might become partially offscreen because of that */
    if (!(mask) & (CWX | CWY) && (mask & (CWWidth | CWHeight)))
	sizeOnly = false;

    if ((right - left) != xwc->width)
    {
	xwc->width = right - left;
	mask       |= CWWidth;
	sizeOnly   = false;
    }

    if ((bottom - top) != xwc->height)
    {
	xwc->height = bottom - top;
	mask        |= CWHeight;
	sizeOnly    = false;
    }

    if (!sizeOnly)
    {
	if (left != x)
	{
	    xwc->x += left - x;
	    mask   |= CWX;
	}

	if (top != y)
	{
	    xwc->y += top - y;
	    mask   |= CWY;
	}
    }

    return workArea;
}

void
PlaceWindow::validateResizeRequest (unsigned int   &mask,
				    XWindowChanges *xwc,
				    unsigned int   source)
{
    CompRect             workArea;
    CompWindow::Geometry geom;
    bool                 sizeOnly = false;

    window->validateResizeRequest (mask, xwc, source);

    if (!mask)
	return;

    if (source == ClientTypePager)
	return;

    if (window->state () & CompWindowStateFullscreenMask)
	return;

    if (window->wmType () & (CompWindowTypeDockMask |
			     CompWindowTypeDesktopMask))
	return;

    /* do nothing if the window was already (at least partially) offscreen */
    if (window->serverX () < 0                         ||
	window->serverX () + window->serverWidth () > screen->width () ||
	window->serverY () < 0                         ||
	window->serverY () + window->serverHeight () > screen->height ())
    {
	return;
    }

    if (hasUserDefinedPosition (false))
	/* try to keep the window position intact for USPosition -
	   obviously we can't do that if we need to change the size */
	sizeOnly = true;

    doValidateResizeRequest (mask, xwc, sizeOnly, true);

}

void
PlaceScreen::addSupportedAtoms (std::vector<Atom> &atoms)
{

    atoms.push_back (fullPlacementAtom);

    screen->addSupportedAtoms (atoms);
}

void
PlaceWindow::doPlacement (CompPoint &pos)
{
    CompRect          workArea;
    CompPoint         targetVp;
    PlacementStrategy strategy;
    bool              keepInWorkarea;
    int		      mode;

    if (matchPosition (pos, keepInWorkarea))
    {
	strategy = keepInWorkarea ? ConstrainOnly : NoPlacement;
    }
    else
    {
	strategy = getStrategy ();
	if (strategy == NoPlacement)
	    return;
    }

    mode = getPlacementMode ();
    const CompOutput &output = getPlacementOutput (mode, strategy, pos);
    workArea = output.workArea ();

    targetVp = window->initialViewport ();

    if (strategy == PlaceOverParent)
    {
	CompWindow *parent;

	parent = screen->findWindow (window->transientFor ());
	if (parent)
	{
	    /* center over parent horizontally */
	    pos.setX (parent->serverBorderRect ().x () +
		      (parent->serverBorderRect ().width () / 2) -
		      (window->serverBorderRect ().width () / 2));

	    /* "visually" center vertically, leaving twice as much space below
	       as on top */
	    pos.setY (parent->serverBorderRect ().y () +
		      (parent->serverBorderRect ().height () -
		       window->serverBorderRect ().height ()) / 3);

	    /* if parent is visible on current viewport, clip to work area;
	       don't constrain further otherwise */
	    if (parent->serverBorderRect ().x () < screen->width ()           &&
		parent->serverBorderRect ().x () + parent->serverBorderRect ().width () > 0 &&
		parent->serverBorderRect ().y () < screen->height ()          &&
		parent->serverBorderRect ().y () + parent->serverBorderRect ().height () > 0)
	    {
		targetVp = parent->defaultViewport ();
		strategy = ConstrainOnly;
	    }
	    else
	    {
		strategy = NoPlacement;
	    }
	}
    }

    if (strategy == PlaceCenteredOnScreen)
    {
	/* center window on current output device */

	pos.setX (output.x () +
		  (output.width () - window->serverGeometry ().width ()) /2);
	pos.setY (output.y () +
		  (output.height () - window->serverGeometry ().height ()) / 2);

	strategy = ConstrainOnly;
    }

    workArea.setX (workArea.x () +
                   (targetVp.x () - screen->vp ().x ()) * screen->width ());
    workArea.setY (workArea.y () +
                   (targetVp.y () - screen->vp ().y ()) * screen->height ());

    if (strategy == PlaceOnly || strategy == PlaceAndConstrain)
    {
	/* Construct list of placeables */
	compiz::place::Placeable::Vector placeables;

	foreach (CompWindow *w, screen->windows ())
	{
	    PLACE_WINDOW (w);

	    if (windowIsPlaceRelevant (w))
		placeables.push_back (static_cast <compiz::place::Placeable *> (pw));
	}

	switch (mode) {
	    case PlaceOptions::ModeCascade:
	    placeCascade (workArea, pos);
	    break;
	case PlaceOptions::ModeCentered:
	    placeCentered (workArea, pos);
	    break;
	case PlaceOptions::ModeRandom:
	    placeRandom (workArea, pos);
	    break;
	case PlaceOptions::ModePointer:
	    placePointer (workArea, pos);
	    break;
	case PlaceOptions::ModeMaximize:
	    sendMaximizationRequest ();
	    break;
	case PlaceOptions::ModeSmart:
	    placeSmart (pos, placeables);
	    break;
	}

	/* When placing to the fullscreen output, constrain to one
	   output nevertheless */
	if ((unsigned int) output.id () == (unsigned int) ~0)
	{
	    int                  id;
	    CompWindow::Geometry geom (window->serverGeometry ());

	    geom.setPos (pos);

	    id       = screen->outputDeviceForGeometry (geom);
	    workArea = screen->getWorkareaForOutput (id);

	    workArea.setX (workArea.x () +
	                   (targetVp.x () - screen->vp ().x ()) *
			   screen->width ());
	    workArea.setY (workArea.y () +
	                   (targetVp.y () - screen->vp ().y ()) *
			   screen->height ());
	}

	/* Maximize windows if they are too big for their work area (bit of
	 * a hack here). Assume undecorated windows probably don't intend to
	 * be maximized.
	 */
	if ((window->actions () & MAXIMIZE_STATE) == MAXIMIZE_STATE &&
	    (window->mwmDecor () & (MwmDecorAll | MwmDecorTitle))   &&
	    !(window->state () & CompWindowStateFullscreenMask))
	{
	    if (window->serverWidth () >= workArea.width () &&
		window->serverHeight () >= workArea.height ())
	    {
		sendMaximizationRequest ();
	    }
	}
    }

    if (strategy == ConstrainOnly || strategy == PlaceAndConstrain)
	constrainToWorkarea (workArea, pos);
}

void
PlaceWindow::placeCascade (const CompRect &workArea,
			   CompPoint      &pos)
{
    Placeable::Vector placeables;

    /* Find windows that matter (not minimized, on same workspace
     * as placed window, may be shaded - if shaded we pretend it isn't
     * for placement purposes)
     */
    foreach (CompWindow *w, screen->windows ())
    {
	if (!windowIsPlaceRelevant (w))
	    continue;

	if (w->type () & (CompWindowTypeFullscreenMask |
			  CompWindowTypeUnknownMask))
	    continue;

	if (w->serverX () >= workArea.right ()                              ||
	    w->serverX () + w->serverGeometry ().width () <= workArea.x  () ||
	    w->serverY () >= workArea.bottom ()                             ||
	    w->serverY () + w->serverGeometry ().height () <= workArea.y ())
	    continue;

	placeables.push_back (static_cast <Placeable *> (PlaceWindow::get (w)));
    }

    if (!cascadeFindFirstFit (placeables, workArea, pos))
    {
	/* if the window wasn't placed at the origin of screen,
	 * cascade it onto the current screen
	 */
	cascadeFindNext (placeables, workArea, pos);
    }
}

void
PlaceWindow::placeCentered (const CompRect &workArea,
			    CompPoint      &pos)
{
    pos.setX (workArea.x () +
	      (workArea.width () - window->serverGeometry ().width ()) / 2);
    pos.setY (workArea.y () +
	      (workArea.height () - window->serverGeometry ().height ()) / 2);
}

void
PlaceWindow::placeRandom (const CompRect &workArea,
			  CompPoint      &pos)
{
    int remainX, remainY;

    pos.setX (workArea.x ());
    pos.setY (workArea.y ());

    remainX = workArea.width () - window->serverGeometry ().width ();
    if (remainX > 0)
	pos.setX (pos.x () + (rand () % remainX));

    remainY = workArea.height () - window->serverGeometry ().height ();
    if (remainY > 0)
	pos.setY (pos.y () + (rand () % remainY));
}

void
PlaceWindow::placePointer (const CompRect &workArea,
			   CompPoint	  &pos)
{
    if (PlaceScreen::get (screen)->getPointerPosition (pos))
    {
	unsigned int dx = (window->serverGeometry ().widthIncBorders () / 2);
	unsigned int dy = (window->serverGeometry ().heightIncBorders () / 2);
	pos -= CompPoint (dx, dy);
    }
    else
	placeCentered (workArea, pos);
}

using namespace compiz::place;

void
PlaceWindow::placeSmart (CompPoint			&pos,
			 const compiz::place::Placeable::Vector &placeables)
{
    compiz::place::smart (this, pos, placeables);
}

static void
centerTileRectInArea (CompRect       &rect,
		      const CompRect &workArea)
{
    int fluff;

    /* The point here is to tile a window such that "extra"
     * space is equal on either side (i.e. so a full screen
     * of windows tiled this way would center the windows
     * as a group)
     */

    fluff  = (workArea.width () % (rect.width () + 1)) / 2;
    rect.setX (workArea.x () + fluff);

    fluff  = (workArea.height () % (rect.height () + 1)) / 3;
    rect.setY (workArea.y () + fluff);
}

static bool
rectOverlapsWindow (const CompRect       &rect,
		    const compiz::place::Placeable::Vector &placeables)
{
    CompRect dest;

    foreach (compiz::place::Placeable *other, placeables)
    {
	CompRect intersect;
	CompRect sbr = other->geometry ();
	sbr.setLeft (sbr.left () - other->extents ().left);
	sbr.setRight (sbr.right () + other->extents ().right);
	sbr.setTop (sbr.top () - other->extents ().top);
	sbr.setBottom (sbr.bottom () - other->extents ().bottom);

	intersect = rect & sbr;
	if (!intersect.isEmpty ())
	    return true;
    }

    return false;
}

/* Find the leftmost, then topmost, empty area on the workspace
 * that can contain the new window.
 *
 * Cool feature to have: if we can't fit the current window size,
 * try shrinking the window (within geometry constraints). But
 * beware windows such as Emacs with no sane minimum size, we
 * don't want to create a 1x1 Emacs.
 */
bool
PlaceWindow::cascadeFindFirstFit (const Placeable::Vector &placeables,
				  const CompRect       &workArea,
				  CompPoint            &pos)
{
    /* This algorithm is limited - it just brute-force tries
     * to fit the window in a small number of locations that are aligned
     * with existing windows. It tries to place the window on
     * the bottom of each existing window, and then to the right
     * of each existing window, aligned with the left/top of the
     * existing window in each of those cases.
     */
    bool           retval = false;
    Placeable::Vector belowSorted, rightSorted;

    /* Below each window */
    belowSorted = placeables;
    std::sort (belowSorted.begin (), belowSorted.end (), compareLeftmost);
    std::sort (belowSorted.begin (), belowSorted.end (), compareTopmost);

    /* To the right of each window */
    rightSorted = placeables;
    std::sort (belowSorted.begin (), belowSorted.end (), compareTopmost);
    std::sort (belowSorted.begin (), belowSorted.end (), compareLeftmost);

    CompRect rect = this->geometry ();

    rect.setLeft (rect.left () - this->extents ().left);
    rect.setRight (rect.right () + this->extents ().right);
    rect.setTop (rect.top () - this->extents ().top);
    rect.setBottom (rect.bottom () - this->extents ().bottom);

    centerTileRectInArea (rect, workArea);

    if (workArea.contains (rect) && !rectOverlapsWindow (rect, placeables))
    {
	pos.setX (rect.x () + this->extents ().left);
	pos.setY (rect.y () + this->extents ().top);
	retval = true;
    }

    if (!retval)
    {
	/* try below each window */
	foreach (Placeable *p, belowSorted)
	{
	    CompRect outerRect;

	    if (retval)
		break;

	    outerRect = p->geometry ();

	    outerRect.setLeft (rect.left () - this->extents ().left);
	    outerRect.setRight (rect.right () + this->extents ().right);
	    outerRect.setTop (rect.top () - this->extents ().top);
	    outerRect.setBottom (rect.bottom () - this->extents ().bottom);

	    outerRect.setX (outerRect.x ());
	    outerRect.setY (outerRect.bottom ());

	    if (workArea.contains (rect) &&
		!rectOverlapsWindow (rect, belowSorted))
	    {
		pos.setX (rect.x () + this->extents ().left);
		pos.setY (rect.y () + this->extents ().top);
		retval = true;
	    }
	}
    }

    if (!retval)
    {
	/* try to the right of each window */
	foreach (Placeable *p, rightSorted)
	{
	    CompRect outerRect;

	    if (retval)
		break;

	    outerRect = p->geometry ();

	    outerRect.setLeft (rect.left () - this->extents ().left);
	    outerRect.setRight (rect.right () + this->extents ().right);
	    outerRect.setTop (rect.top () - this->extents ().top);
	    outerRect.setBottom (rect.bottom () - this->extents ().bottom);

	    outerRect.setX (outerRect.right ());
	    outerRect.setY (outerRect.y ());

	    if (workArea.contains (rect) &&
		!rectOverlapsWindow (rect, rightSorted))
	    {
		pos.setX (rect.x () + this->extents ().left);
		pos.setY (rect.y () + this->extents ().top);
		retval = true;
	    }
	}
    }

    return retval;
}

void
PlaceWindow::cascadeFindNext (const Placeable::Vector &placeables,
			      const CompRect	      &workArea,
			      CompPoint		      &pos)
{
    Placeable::Vector           sorted;
    Placeable::Vector::iterator iter;
    int                         cascadeX, cascadeY;
    int                         xThreshold, yThreshold;
    int                         winWidth, winHeight;
    int                         cascadeStage;

    sorted = placeables;
    std::sort (sorted.begin (), sorted.end (), compareNorthWestCorner);

    /* This is a "fuzzy" cascade algorithm.
     * For each window in the list, we find where we'd cascade a
     * new window after it. If a window is already nearly at that
     * position, we move on.
     */

    /* arbitrary-ish threshold, honors user attempts to
     * manually cascade.
     */
static const unsigned short CASCADE_FUZZ = 15;

    xThreshold = MAX (this->extents ().left, CASCADE_FUZZ);
    yThreshold = MAX (this->extents ().top, CASCADE_FUZZ);

    /* Find furthest-SE origin of all workspaces.
     * cascade_x, cascade_y are the target position
     * of NW corner of window frame.
     */

    cascadeX = MAX (0, workArea.x ());
    cascadeY = MAX (0, workArea.y ());

    /* Find first cascade position that's not used. */

    winWidth  = window->serverWidth ();
    winHeight = window->serverHeight ();

    cascadeStage = 0;
    for (iter = sorted.begin (); iter != sorted.end (); ++iter)
    {
	Placeable  *p = *iter;
	int        wx, wy;

	/* we want frame position, not window position */
	wx = p->geometry ().x () - p->extents ().left;
	wy = p->geometry ().y () - p->extents ().top;

	if (abs (wx - cascadeX) < xThreshold &&
	    abs (wy - cascadeY) < yThreshold)
	{
	    /* This window is "in the way", move to next cascade
	     * point. The new window frame should go at the origin
	     * of the client window we're stacking above.
	     */
	    wx = cascadeX = p->geometry ().x ();
	    wy = cascadeY = p->geometry ().y ();

	    /* If we go off the screen, start over with a new cascade */
	    if ((cascadeX + winWidth > workArea.right ()) ||
		(cascadeY + winHeight > workArea.bottom ()))
	    {
		cascadeX = MAX (0, workArea.x ());
		cascadeY = MAX (0, workArea.y ());

static const unsigned short CASCADE_INTERVAL = 50; /* space between top-left corners of cascades */

		cascadeStage += 1;
		cascadeX += CASCADE_INTERVAL * cascadeStage;

		/* start over with a new cascade translated to the right,
		 * unless we are out of space
		 */
		if (cascadeX + winWidth < workArea.right ())
		{
		    iter = sorted.begin ();
		    continue;
		}
		else
		{
		    /* All out of space, this cascade_x won't work */
		    cascadeX = MAX (0, workArea.x ());
		    break;
		}
	    }
	}
	else
	{
	    /* Keep searching for a further-down-the-diagonal window. */
	}
    }

    /* cascade_x and cascade_y will match the last window in the list
     * that was "in the way" (in the approximate cascade diagonal)
     */

    /* Convert coords to position of window, not position of frame. */
    pos.setX (cascadeX + this->extents ().left);
    pos.setY (cascadeY + this->extents ().top);
}

bool
PlaceWindow::hasUserDefinedPosition (bool acceptPPosition)
{
    PLACE_SCREEN (screen);

    CompMatch &match = ps->optionGetForcePlacementMatch ();

    if (match.evaluate (window))
	return false;

    if (acceptPPosition && (window->sizeHints ().flags & PPosition))
	return true;

    if ((window->type () & CompWindowTypeNormalMask) ||
	ps->optionGetWorkarounds ())
    {
	/* Only accept USPosition on non-normal windows if workarounds are
	 * enabled because apps claiming the user set -geometry for a
	 * dialog or dock are most likely wrong
	 */
	if (window->sizeHints ().flags & USPosition)
	    return true;
    }

    return false;
}

PlaceWindow::PlacementStrategy
PlaceWindow::getStrategy ()
{
    if (window->type () & (CompWindowTypeDockMask       |
			   CompWindowTypeDesktopMask    |
			   CompWindowTypeUtilMask       |
			   CompWindowTypeToolbarMask    |
			   CompWindowTypeMenuMask       |
			   CompWindowTypeFullscreenMask |
			   CompWindowTypeUnknownMask))
    {
	/* assume the app knows best how to place these */
	return NoPlacement;
    }

    if (window->wmType () & (CompWindowTypeDockMask |
			     CompWindowTypeDesktopMask))
    {
	/* see above */
	return NoPlacement;
    }

    if (hasUserDefinedPosition (true))
	return ConstrainOnly;

   if (window->transientFor () &&
       (window->type () & (CompWindowTypeDialogMask |
			   CompWindowTypeModalDialogMask)))
    {
	CompWindow *parent = screen->findWindow (window->transientFor ());

	if (parent && parent->managed ())
	    return PlaceOverParent;
    }

    if (window->type () & (CompWindowTypeDialogMask      |
			   CompWindowTypeModalDialogMask |
			   CompWindowTypeSplashMask))
    {
	return PlaceCenteredOnScreen;
    }

    return PlaceAndConstrain;
}

const CompOutput &
PlaceWindow::getPlacementOutput (int		   mode,
				 PlacementStrategy strategy,
				 CompPoint         pos)
{
    int output = -1;
    int multiMode;

    /* short cut: it makes no sense to determine a placement
       output if there is only one output */
    if (screen->outputDevs ().size () == 1)
	return screen->outputDevs ().at (0);

    switch (strategy) {
    case PlaceOverParent:
	{
	    CompWindow *parent;

	    parent = screen->findWindow (window->transientFor ());
	    if (parent)
		output = parent->outputDevice ();
	}
	break;
    case ConstrainOnly:
	{
	    CompWindow::Geometry geom = window->serverGeometry ();

	    geom.setPos (pos);
	    output = screen->outputDeviceForGeometry (geom);
	}
	break;
    default:
	break;
    }

    if (output >= 0)
	return screen->outputDevs ()[output];

    multiMode = ps->optionGetMultioutputMode ();
    /* force 'output with pointer' for placement under pointer */
    if (mode == PlaceOptions::ModePointer)
	multiMode = PlaceOptions::MultioutputModeUseOutputDeviceWithPointer;

    switch (multiMode) {
	case PlaceOptions::MultioutputModeUseActiveOutputDevice:
	    return screen->currentOutputDev ();
	    break;
	case PlaceOptions::MultioutputModeUseOutputDeviceWithPointer:
	    {
		CompPoint p;
		if (PlaceScreen::get (screen)->getPointerPosition (p))
		{
		    output = screen->outputDeviceForPoint (p.x (), p.y ());
		}
	    }
	    break;
	case PlaceOptions::MultioutputModeUseOutputDeviceOfFocussedWindow:
	    {
		CompWindow *active;

		active = screen->findWindow (screen->activeWindow ());
		if (active)
		    output = active->outputDevice ();
	    }
	    break;
	case PlaceOptions::MultioutputModePlaceAcrossAllOutputs:
	    /* only place on fullscreen output if not placing centered, as the
	    constraining will move the window away from the center otherwise */
	    if (strategy != PlaceCenteredOnScreen)
		return screen->fullscreenOutput ();
	    break;
    }

    if (output < 0)
	return screen->currentOutputDev ();

    return screen->outputDevs ()[output];
}

int
PlaceWindow::getPlacementMode ()
{
    CompOption::Value::Vector& matches = ps->optionGetModeMatches ();
    CompOption::Value::Vector& modes   = ps->optionGetModeModes ();
    int                        i, min;

    min = MIN (matches.size (), modes.size ());

    for (i = 0; i < min; i++)
	if (matches[i].match ().evaluate (window))
	    return modes[i].i ();

    return ps->optionGetMode ();
}

void
PlaceWindow::constrainToWorkarea (const CompRect &workArea,
				  CompPoint      &pos)
{
    CompWindowExtents extents;
    int               delta;

    extents.left   = pos.x () - window->border ().left;
    extents.top    = pos.y () - window->border ().top;
    extents.right  = extents.left + window->serverGeometry ().widthIncBorders () +
		     (window->border ().left +
		      window->border ().right);
    extents.bottom = extents.top + window->serverGeometry ().heightIncBorders () +
		     (window->border ().top +
		      window->border ().bottom);

    delta = workArea.right () - extents.right;
    if (delta < 0)
	extents.left += delta;

    delta = workArea.left () - extents.left;
    if (delta > 0)
	extents.left += delta;

    delta = workArea.bottom () - extents.bottom;
    if (delta < 0)
	extents.top += delta;

    delta = workArea.top () - extents.top;
    if (delta > 0)
	extents.top += delta;

    pos.setX (extents.left + window->border ().left);
    pos.setY (extents.top  + window->border ().top);

}

bool
PlaceWindow::windowIsPlaceRelevant (CompWindow *w)
{
    if (w->id () == window->id ())
	return false;
    if (!w->isViewable () && !w->shaded ())
	return false;
    if (w->overrideRedirect ())
	return false;
    if (w->wmType () & (CompWindowTypeDockMask | CompWindowTypeDesktopMask))
	return false;

    return true;
}

void
PlaceWindow::sendMaximizationRequest ()
{
    XEvent  xev;
    Display *dpy = screen->dpy ();

    xev.xclient.type    = ClientMessage;
    xev.xclient.display = dpy;
    xev.xclient.format  = 32;

    xev.xclient.message_type = Atoms::winState;
    xev.xclient.window	     = window->id ();

    xev.xclient.data.l[0] = 1;
    xev.xclient.data.l[1] = Atoms::winStateMaximizedHorz;
    xev.xclient.data.l[2] = Atoms::winStateMaximizedVert;
    xev.xclient.data.l[3] = 0;
    xev.xclient.data.l[4] = 0;

    XSendEvent (dpy, screen->root (), false,
		SubstructureRedirectMask | SubstructureNotifyMask, &xev);
}

bool
PlaceScreen::getPointerPosition (CompPoint &p)
{
    Window wDummy;
    int	   iDummy;
    unsigned int uiDummy;
    int x, y;
    bool ret;

    /* this means a server roundtrip, which kind of sucks; this
     * this code should be removed as soon as we have software cursor
     * rendering and thus a cache pointer co-ordinate */

    ret = XQueryPointer (screen->dpy (), screen->root (), &wDummy, &wDummy,
    			  &x, &y, &iDummy, &iDummy, &uiDummy);

    p.set (x, y);

    return ret;
}

bool
PlaceWindow::matchXYValue (CompOption::Value::Vector &matches,
			   CompOption::Value::Vector &xValues,
			   CompOption::Value::Vector &yValues,
			   CompPoint                 &pos,
			   CompOption::Value::Vector *constrainValues,
			   bool                      *keepInWorkarea)
{
    unsigned int i, min;

    if (window->type () & CompWindowTypeDesktopMask)
	return false;

    min = MIN (matches.size (), xValues.size ());
    min = MIN (min, yValues.size ());

    for (i = 0; i < min; i++)
    {
	if (matches[i].match ().evaluate (window))
	{
	    pos.setX (xValues[i].i ());
	    pos.setY (yValues[i].i ());

	    if (keepInWorkarea)
	    {
		if (constrainValues && constrainValues->size () > i)
		    *keepInWorkarea = (*constrainValues)[i].b ();
		else
		    *keepInWorkarea = true;
	    }

	    return true;
	}
    }

    return false;
}

bool
PlaceWindow::matchPosition (CompPoint &pos,
			    bool      &keepInWorkarea)
{
    return matchXYValue (
	ps->optionGetPositionMatches (),
	ps->optionGetPositionXValues (),
	ps->optionGetPositionYValues (),
	pos,
	&ps->optionGetPositionConstrainWorkarea (),
	&keepInWorkarea);
}

bool
PlaceWindow::matchViewport (CompPoint &pos)
{
    if (matchXYValue (ps->optionGetViewportMatches (),
		      ps->optionGetViewportXValues (),
		      ps->optionGetViewportYValues (),
		      pos))
    {
	/* Viewport matches are given 1-based, so we need to adjust that */
	pos.setX (pos.x () - 1);
	pos.setY (pos.y () - 1);

	return true;
    }

    return false;
}

void
PlaceWindow::grabNotify (int x,
			 int y,
			 unsigned int state,
			 unsigned int mask)
{
    /* Don't restore geometry if the user moved the window */
    if (screen->grabExist ("move") ||
	screen->grabExist ("resize"))
	unset ();

    window->grabNotify (x, y, state, mask);
}

bool
PlacePluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION))
	return false;

    return true;
}



