/*
 * Copyright (c) 2006 Darryll Truchan <moppsy@comcast.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "put.h"

#include <cmath>

COMPIZ_PLUGIN_20090315 (put, PutPluginVTable);

#define PUT_ONLY_EMPTY(type) (type >= PutEmptyBottomLeft && \
			      type <= PutEmptyTopRight)

#define TOP_BORDER(w) ((w)->border ().top)
#define LEFT_BORDER(w) ((w)->border ().left)
#define RIGHT_BORDER(w) ((w)->border ().right + 2 * (w)->serverGeometry ().border ())
#define BOTTOM_BORDER(w) ((w)->border ().bottom + 2 * (w)->serverGeometry ().border ())

#define HALF_WIDTH(w) ((w)->serverWidth () / 2 + (w)->serverGeometry ().border ())
#define HALF_HEIGHT(w) ((w)->serverHeight () / 2 + (w)->serverGeometry ().border ())

/*
 * Maximumize functions
 * Functions are from Maximumize plugin
 * (Author:Kristian Lyngstøl  <kristian@bohemians.org>)
 */

/* Generates a region containing free space (here the
 * active window counts as free space). The region argument
 * is the start-region (ie: the output dev).
 * Logic borrowed from opacify (courtesy of myself).
 */
CompRegion
PutScreen::emptyRegion (CompWindow      *window,
			const CompRect& outputRect)
{
    CompRegion newRegion;

    newRegion += outputRect;

    foreach(CompWindow *w, screen->windows ())
    {
        if (w->id () == window->id ())
            continue;

        if (w->invisible () || /*w->hidden () ||*/ w->minimized ())
            continue;

	if (w->wmType () & CompWindowTypeDesktopMask)
	    continue;

	if (w->wmType () & CompWindowTypeDockMask)
	{
	    if (w->struts ())
	    {
		CompRegion tmpRegion;

		tmpRegion += w->struts ()->left;
		tmpRegion += w->struts ()->right;
		tmpRegion += w->struts ()->top;
		tmpRegion += w->struts ()->bottom;

		newRegion -= tmpRegion;
	    }
	    continue;
	}

	newRegion -= w->serverBorderRect ();
    }

    return newRegion;
}

/* Returns true if box a has a larger area than box b.
 */
bool
PutScreen::boxCompare (const CompRect& a,
		       const CompRect& b)
{
    int areaA, areaB;

    areaA = a.width () * a.height ();
    areaB = b.width () * b.height ();

    return (areaA > areaB);
}

/* Extends the given box for Window w to fit as much space in region r.
 * If XFirst is true, it will first expand in the X direction,
 * then Y. This is because it gives different results.
 * PS: Decorations are icky.
 */

static const unsigned short LEFT = 0;
static const unsigned short RIGHT = 1;
static const unsigned short TOP = 2;
static const unsigned short BOTTOM = 3;

inline void
addToCorner (CompRect&    rect,
	     unsigned int corner,
	     const short  inc)
{
    switch (corner) {
	case LEFT:
	    rect.setX (rect.x () + inc);
	    break;
	case RIGHT:
	    rect.setWidth (rect.width () + inc);
	    break;
	case TOP:
	    rect.setY (rect.y () + inc);
	    break;
	case BOTTOM:
	    rect.setHeight (rect.height () + inc);
	    break;
    }
}

inline void
expandCorner (CompWindow        *w,
	      CompRect&         tmp,
	      const CompRegion& r,
	      unsigned int      corner,
	      int               direction)
{
    bool touch = false;

#define CHECKREC                                                               \
    r.contains (CompRect (tmp.x () - LEFT_BORDER (w),                         \
			  tmp.y () - TOP_BORDER (w),                          \
			  tmp.width () + LEFT_BORDER (w) + RIGHT_BORDER (w), \
			  tmp.height () + TOP_BORDER (w) + BOTTOM_BORDER (w)))

    while (CHECKREC) {
	addToCorner (tmp, corner, direction);
	touch = true;
    }

    if (touch)
	addToCorner (tmp, corner, -direction);

#undef CHECKREC
}

CompRect
PutScreen::extendBox (CompWindow        *w,
		      const CompRect&   tmp,
		      const CompRegion& r,
		      bool              xFirst,
		      bool              left,
		      bool              right,
		      bool              up,
		      bool              down)
{
    short int counter = 0;
    CompRect  result = tmp;

    while (counter < 1)
    {
	if ((xFirst && counter == 0) || (!xFirst && counter == 1))
	{
	    if (left)
		expandCorner (w, result, r, LEFT, -1);

	    if (right)
		expandCorner (w, result, r, RIGHT, 1);

	    counter++;
	}

	if ((xFirst && counter == 1) || (!xFirst && counter == 0))
	{
	    if (down)
		expandCorner (w, result, r, BOTTOM, 1);

	    if (up)
		expandCorner (w, result, r, TOP, -1);

	    counter++;
	}
    }

    return result;
}

/* Create a box for resizing in the given region
 * Also shrinks the window box in case of minor overlaps.
 * FIXME: should be somewhat cleaner.
 */
CompRect
PutScreen::findRect (CompWindow        *w,
		     const CompRegion& r,
		     bool              left,
		     bool              right,
		     bool              up,
		     bool              down)
{
    CompRect windowBox, ansA, ansB, orig;

    windowBox.setGeometry (w->serverX (), w->serverY (),
			   w->serverWidth (), w->serverHeight ());

    orig = windowBox;

    ansA = extendBox (w, windowBox, r, true, left, right, up, down);
    ansB = extendBox (w, windowBox, r, false, left, right, up, down);

    if (boxCompare (orig, ansA) && boxCompare (orig, ansB))
	return orig;

    if (boxCompare (ansA, ansB))
	return ansA;
    else
	return ansB;

}

/* Calls out to compute the resize */
unsigned int
PutScreen::computeResize (CompWindow     *w,
			  XWindowChanges *xwc,
			  bool           left,
			  bool           right,
			  bool           up,
			  bool           down)
{
    unsigned int mask = 0;
    CompRect     box;
    CompRegion   region;
    int          outputDevice = w->outputDevice ();

    region = emptyRegion (w, screen->outputDevs ()[outputDevice]);
    box    = findRect (w, region, left, right, up, down);

    if (box.x () != w->serverX ())
	mask |= CWX;

    if (box.y () != w->serverY ())
	mask |= CWY;

    if (box.width () != w->serverWidth ())
	mask |= CWWidth;

    if (box.height () != w->height ())
	mask |= CWHeight;

    xwc->x = box.x ();
    xwc->y = box.y ();
    xwc->width = box.width ();
    xwc->height = box.height ();

    return mask;
}

/*
 * End of Maximumize functions
 */

/*
 * calculate the velocity for the moving window
 */
int
PutScreen::adjustVelocity (CompWindow *w)
{
    float dx, dy, adjust, amount;
    float x1, y1;

    PUT_WINDOW (w);

    x1 = pw->targetX;
    y1 = pw->targetY;

    dx = x1 - (w->x () + pw->tx);
    dy = y1 - (w->y () + pw->ty);

    adjust = dx * 0.15f;
    amount = fabs (dx) * 1.5;
    if (amount < 0.5f)
	amount = 0.5f;
    else if (amount > 5.0f)
	amount = 5.0f;

    pw->xVelocity = (amount * pw->xVelocity + adjust) / (amount + 1.0f);

    adjust = dy * 0.15f;
    amount = fabs (dy) * 1.5f;
    if (amount < 0.5f)
	amount = 0.5f;
    else if (amount > 5.0f)
	amount = 5.0f;

    pw->yVelocity = (amount * pw->yVelocity + adjust) / (amount + 1.0f);

    if (fabs (dx) < 0.1f && fabs (pw->xVelocity) < 0.2f &&
	fabs (dy) < 0.1f && fabs (pw->yVelocity) < 0.2f)
    {
	/* animation done */
	pw->xVelocity = pw->yVelocity = 0.0f;

	pw->tx = x1 - w->x ();
	pw->ty = y1 - w->y ();
	return 0;
    }
    return 1;
}

void
PutScreen::finishWindowMovement (CompWindow *w)
{
    PUT_WINDOW (w);

    w->move (pw->targetX - w->x (),
	     pw->targetY - w->y (),
	     true);
    w->syncPosition ();

    if (w->state () & (MAXIMIZE_STATE | CompWindowStateFullscreenMask))
	w->updateAttributes (CompStackingUpdateModeNone);
}

unsigned int
PutScreen::getOutputForWindow (CompWindow *w)
{
    PUT_WINDOW (w);

    if (!pw->adjust)
	return w->outputDevice ();

    /* outputDeviceForWindow uses the server geometry,
       so specialcase a running animation, which didn't
       apply the server geometry yet */
    CompWindow::Geometry geom;

    geom.set (w->x () + pw->tx, w->y () + pw->ty,
	      w->width (), w->height (), w->geometry ().border ());

    return screen->outputDeviceForGeometry (geom);
}

CompPoint
PutScreen::getDistance (CompWindow         *w,
			PutType            type,
			CompOption::Vector &option)
{
    CompScreen *s = screen;
    int        x, y, dx, dy, posX, posY;
    int        viewport, output;
    CompRect   workArea;
    CompPoint  result;

    PUT_SCREEN (s);
    PUT_WINDOW (w);

    posX = CompOption::getIntOptionNamed (option,"x", 0);
    posY = CompOption::getIntOptionNamed (option,"y", 0);

    /* get the output device number from the options list */
    output = CompOption::getIntOptionNamed (option,"output", -1);
    /* no output in options list -> use the current output */
    if (output == -1)
    {
	/* no output given, so use the current output if
	   this wasn't a double tap */

	if (ps->lastType != type || ps->lastWindow != w->id ())
	    output = getOutputForWindow (w);
    }
    else
    {
	/* make sure the output number is not out of bounds */
	output = MIN (output,  (int) s->outputDevs ().size () - 1);
    }

    if (output == -1)
    {
	/* user double-tapped the key, so use the screen work area */
	workArea = s->workArea ();
	/* set the type to unknown to have a toggle-type behaviour
	   between 'use output work area' and 'use screen work area' */
	ps->lastType = PutUnknown;
    }
    else
    {
	/* single tap or output provided via options list,
	   use the output work area */
	workArea = s->getWorkareaForOutput (output);
	ps->lastType = type;
    }

    if (PUT_ONLY_EMPTY (type))
    {
	unsigned int   mask;
	XWindowChanges xwc;
	bool           left, right, up, down;

	left = right = up = down = false;

	switch (type) {
	    case PutEmptyBottomLeft:
		left = down = true;
		break;
	    case PutEmptyBottom:
		down = true;
		break;
	    case PutEmptyBottomRight:
		right = down = true;
		break;
	    case PutEmptyLeft:
		left = true;
		break;
	    case PutEmptyCenter:
		left = right = up = down = true;
		break;
	    case PutEmptyRight:
		right = true;
		break;
	    case PutEmptyTopLeft:
		left = up = true;
		break;
	    case PutEmptyTop:
		up = true;
		break;
	    case PutEmptyTopRight:
		right = up = true;
		break;
	    default:
		break;
	}

	mask = computeResize (w, &xwc, left,right,up,down);
	if (mask)
	{
	    if (w->constrainNewWindowSize (xwc.width, xwc.height,
					   &xwc.width, &xwc.height))
		mask |= CWWidth | CWHeight;
	}

	workArea.setGeometry (xwc.x, xwc.y, xwc.width, xwc.height);
    }
    /* the windows location */
    x = w->x () + pw->tx;
    y = w->y () + pw->ty;

    switch (type) {
    case PutEmptyCenter:
    case PutCenter:
	/* center of the screen */
	dx = (workArea.width () / 2) - HALF_WIDTH (w) -
	      w->serverGeometry ().border () - (x - workArea.x ());
	dy = (workArea.height () / 2) - HALF_HEIGHT (w) -
	      w->serverGeometry ().border () - (y - workArea.y ());
	break;
    case PutLeft:
	/* center of the left edge */
	dx = -(x - workArea.x ()) + LEFT_BORDER (w) + ps->optionGetPadLeft ();
	dy = (workArea.height () / 2) - HALF_HEIGHT (w) -
	     (y - workArea.y ());
	break;
    case PutEmptyLeft:
	/* center of the left edge */
	workArea.setX (workArea.x () - LEFT_BORDER (w));
	dx = -(x - workArea.x ()) + LEFT_BORDER (w) + ps->optionGetPadLeft ();
	dy = (workArea.height () / 2) - HALF_HEIGHT (w) -
	     (y - workArea.y ());
	break;
    case PutTopLeft:
	/* top left corner */
	dx = -(x - workArea.x ()) + LEFT_BORDER (w) + ps->optionGetPadLeft ();
	dy = -(y - workArea.y ()) + TOP_BORDER (w) + ps->optionGetPadTop ();
	break;
    case PutEmptyTopLeft:
	/* top left corner */
	workArea.setX (workArea.x () - LEFT_BORDER (w));
	workArea.setY (workArea.y () - TOP_BORDER (w));
	dx = -(x - workArea.x ()) + LEFT_BORDER (w) + ps->optionGetPadLeft ();
	dy = -(y - workArea.y ()) + TOP_BORDER (w) + ps->optionGetPadTop ();
	break;
    case PutTop:
	/* center of top edge */
	dx = (workArea.width () / 2) - HALF_WIDTH (w) -
	     (x - workArea.x ());
	dy = -(y - workArea.y ()) + TOP_BORDER (w) + ps->optionGetPadTop ();
	break;
    case PutEmptyTop:
	/* center of top edge */
	workArea.setY (workArea.x () - TOP_BORDER (w));
	dx = (workArea.width () / 2) - HALF_WIDTH (w) -
	     (x - workArea.x ());
	dy = -(y - workArea.y ()) + TOP_BORDER (w) + ps->optionGetPadTop ();
	break;
    case PutTopRight:
	/* top right corner */
	dx = workArea.width () - w->serverWidth () - (x - workArea.x ()) -
	     RIGHT_BORDER (w) - ps->optionGetPadRight ();
	dy = -(y - workArea.y ()) + TOP_BORDER (w) + ps->optionGetPadTop ();
	break;
    case PutEmptyTopRight:
	/* top right corner */
	workArea.setX (workArea.x () + RIGHT_BORDER (w));
	workArea.setY (workArea.y () - TOP_BORDER (w));
	dx = workArea.width () - w->serverWidth () - (x - workArea.x ()) -
	     RIGHT_BORDER (w) - ps->optionGetPadRight ();
	dy = -(y - workArea.y ()) + TOP_BORDER (w) + ps->optionGetPadTop ();
	break;
    case PutRight:
	/* center of right edge */
	dx = workArea.width () - w->serverWidth () - (x - workArea.x ()) -
	     RIGHT_BORDER (w) - ps->optionGetPadRight ();
	dy = (workArea.height () / 2) - HALF_HEIGHT (w) -
	     (y - workArea.y ());
	break;
    case PutEmptyRight:
	/* center of right edge */
	workArea.setX (workArea.x () + RIGHT_BORDER (w));
	dx = workArea.width () - w->serverWidth () - (x - workArea.x ()) -
	     RIGHT_BORDER (w) - ps->optionGetPadRight ();
	dy = (workArea.height () / 2) - HALF_HEIGHT (w) -
	     (y - workArea.y ());
	break;
    case PutBottomRight:
	/* bottom right corner */
	dx = workArea.width () - w->serverWidth () - (x - workArea.x ()) -
	    RIGHT_BORDER (w) - ps->optionGetPadRight ();
	dy = workArea.height () - w->serverHeight () - (y - workArea.y ()) -
	    BOTTOM_BORDER (w) - ps->optionGetPadBottom ();
	break;
    case PutEmptyBottomRight:
	/* bottom right corner */
	workArea.setX (workArea.x () + RIGHT_BORDER (w));
	workArea.setY (workArea.y () + BOTTOM_BORDER (w));
	dx = workArea.width () - w->serverWidth () - (x - workArea.x ()) -
	     RIGHT_BORDER (w) - ps->optionGetPadRight ();
	dy = workArea.height () - w->serverHeight () - (y - workArea.y ()) -
	     BOTTOM_BORDER (w)- ps->optionGetPadBottom ();
	break;
    case PutBottom:
	/* center of bottom edge */
	dx = (workArea.width () / 2) - HALF_WIDTH (w) -
	     (x - workArea.x ());
	dy = workArea.height () - w->serverHeight () - (y - workArea.y ()) -
	     BOTTOM_BORDER (w) - ps->optionGetPadBottom ();
	break;
    case PutEmptyBottom:
	/* center of bottom edge */
	workArea.setY (workArea.y () + BOTTOM_BORDER (w));
	dx = (workArea.width () / 2) - HALF_WIDTH (w) -
	     (x - workArea.x ());
	dy = workArea.height () - w->serverHeight () - (y - workArea.y ()) -
	     BOTTOM_BORDER (w) - ps->optionGetPadBottom ();
	break;
    case PutBottomLeft:
	/* bottom left corner */
	dx = -(x - workArea.x ()) + LEFT_BORDER (w) + ps->optionGetPadLeft ();
	dy = workArea.height () - w->serverHeight () - (y - workArea.y ()) -
	     BOTTOM_BORDER (w) - ps->optionGetPadBottom ();
	break;
    case PutEmptyBottomLeft:
	/* bottom left corner */
	workArea.setX (workArea.x () - LEFT_BORDER (w));
	workArea.setY (workArea.y () + BOTTOM_BORDER (w));
	dx = -(x - workArea.x ()) + LEFT_BORDER (w) + ps->optionGetPadLeft ();
	dy = workArea.height () - w->serverHeight () - (y - workArea.y ()) -
	     BOTTOM_BORDER (w) - ps->optionGetPadBottom ();
	break;
    case PutRestore:
	/* back to last position */
	dx = pw->lastX - x;
	dy = pw->lastY - y;
	break;
    case PutViewport:
	{
	    int vpX, vpY, hDirection, vDirection;

	    /* get the viewport to move to from the options list */
	    viewport = CompOption::getIntOptionNamed (option, "viewport", -1);

	    /* if viewport wasn't supplied, bail out */
	    if (viewport < 0)
		return result;

	    /* split 1D viewport value into 2D x and y viewport */
	    vpX = viewport % s->vpSize ().width ();
	    vpY = viewport / s->vpSize ().width ();
	    if (vpY > (int) s->vpSize ().height ())
		vpY = s->vpSize ().height () - 1;

	    /* take the shortest horizontal path to the destination viewport */
	    hDirection = (vpX - s->vp ().x ());
	    if (hDirection > (int) s->vpSize ().width () / 2)
		hDirection = (hDirection - s->vpSize ().width ());
	    else if (hDirection < - ((int) s->vpSize ().width ()) / 2)
		hDirection = (hDirection + s->vpSize ().width ());

	    /* we need to do this for the vertical destination viewport too */
	    vDirection = (vpY - s->vp ().y ());
	    if (vDirection > (int) s->vpSize ().height () / 2)
		vDirection = (vDirection - s->vpSize ().height ());
	    else if (vDirection < -((int) s->vpSize ().height ()) / 2)
		vDirection = (vDirection + s->vpSize ().height ());

	    dx = s->width () * hDirection;
	    dy = s->height () * vDirection;
	    break;
	}
    case PutViewportLeft:
	/* move to the viewport on the left */
	dx = (s->vp ().x () >= 1) ? -s->width () : 0;
	dy = 0;
	break;
    case PutViewportRight:
	/* move to the viewport on the right */
	dx = (s->vp ().x () < s->vpSize ().width ()-1) ? s->width () : 0;
	dy = 0;
	break;
    case PutViewportUp:
	/* move to the viewport above */
	dx = 0;
	dy = (s->vp ().y () >= 1) ? -s->height () : 0;
	break;
    case PutViewportDown:
	/* move to the viewport below */
	dx = 0;
	dy = (s->vp ().y () < s->vpSize ().height ()-1) ? s->height () : 0;
	break;
    case PutNextOutput:
	{
	    int        outputNum, currentNum;
	    int        nOutputDev = s->outputDevs ().size ();
	    CompOutput *currentOutput, *newOutput;

	    if (nOutputDev < 2)
		return result;

	    currentNum = getOutputForWindow (w);
	    outputNum  = (currentNum + 1) % nOutputDev;
	    outputNum  = CompOption::getIntOptionNamed (option,"output",
							outputNum);

	    if (outputNum >= nOutputDev)
		return result;

	    currentOutput = &s->outputDevs ().at(currentNum);
	    newOutput     = &s->outputDevs ().at(outputNum);

	    /* move by the distance of the output center points */
	    dx = (newOutput->x1 () + newOutput->width () / 2) -
		 (currentOutput->x1 () + currentOutput->width () / 2);
	    dy = (newOutput->y1 () + newOutput->height () / 2) -
		 (currentOutput->y1 () + currentOutput->height () / 2);

	    /* update work area for new output */
	    workArea = newOutput->workArea ();
	}
	break;
    case PutAbsolute:
	/* move the window to an exact position */
	if (posX < 0)
	    /* account for a specified negative position,
	       like geometry without (-0) */
	    dx = posX + s->width () - w->serverWidth () - x - RIGHT_BORDER (w);
	else
	    dx = posX - x + LEFT_BORDER (w);

	if (posY < 0)
	    /* account for a specified negative position,
	       like geometry without (-0) */
	    dy = posY + s->height () - w->height () - y - BOTTOM_BORDER (w);
	else
	    dy = posY - y + TOP_BORDER (w);
	break;
    case PutRelative:
	/* move window by offset */
	dx = posX;
	dy = posY;
	break;
    case PutPointer:
	{
	    /* move the window to the pointers position
	     * using the current quad of the screen to determine
	     * which corner to move to the pointer
	     */
	    int          rx, ry;
	    Window       root, child;
	    int          winX, winY;
	    unsigned int pMask;

	    /* get the pointers position from the X server */
	    if (XQueryPointer (s->dpy (), w->id (), &root, &child,
			       &rx, &ry, &winX, &winY, &pMask))
	    {
		if (ps->optionGetWindowCenter ())
		{
		    /* window center */
		    dx = rx - HALF_WIDTH (w) - x;
		    dy = ry - HALF_HEIGHT (w) - y;
		}
		else if (rx < (int) s->workArea ().width () / 2 &&
			 ry < (int) s->workArea ().height () / 2)
		{
		    /* top left quad */
		    dx = rx - x + LEFT_BORDER (w);
		    dy = ry - y + TOP_BORDER (w);
		}
		else if (rx < (int) s->workArea ().width () / 2 &&
			 ry >= (int) s->workArea ().height () / 2)
		{
		    /* bottom left quad */
		    dx = rx - x + LEFT_BORDER (w);
		    dy = ry - w->height () - y - BOTTOM_BORDER (w);
		}
		else if (rx >= (int) s->workArea ().width () / 2 &&
			 ry < (int) s->workArea ().height () / 2)
		{
		    /* top right quad */
		    dx = rx - w->width () - x - RIGHT_BORDER (w);
		    dy = ry - y + TOP_BORDER (w);
		}
		else
		{
		    /* bottom right quad */
		    dx = rx - w->width () - x - RIGHT_BORDER (w);
		    dy = ry - w->height () - y - BOTTOM_BORDER (w);
		}
	    }
	    else
	    {
		dx = dy = 0;
	    }
	}
	break;
    default:
	/* if an unknown type is specified, do nothing */
	dx = dy = 0;
	break;
    }

    if ((dx || dy) && ps->optionGetAvoidOffscreen () &&
	!(w->type () & CompWindowTypeFullscreenMask))
    {
	/* avoids window borders offscreen, but allow full
	   viewport movements */
	int               inDx, dxBefore;
	int               inDy, dyBefore;
	CompWindowExtents extents, area;

	inDx = dxBefore = dx % s->width ();
	inDy = dyBefore = dy % s->height ();

	extents.left   = x + inDx - LEFT_BORDER (w);
	extents.top    = y + inDy - TOP_BORDER (w);
	extents.right  = x + inDx + w->serverWidth () + RIGHT_BORDER (w);
	extents.bottom = y + inDy + w->serverHeight () + BOTTOM_BORDER (w);

	area.left   = workArea.left () + ps->optionGetPadLeft ();
	area.top    = workArea.top () + ps->optionGetPadTop ();
	area.right  = workArea.right () - ps->optionGetPadRight ();
	area.bottom = workArea.bottom () - ps->optionGetPadBottom ();

	if (extents.left < area.left)
	{
	    inDx += area.left - extents.left;
	}
	else if (w->serverWidth () <= workArea.width () &&
		 extents.right > area.right)
	{
	    inDx += area.right - extents.right;
	}

	if (extents.top < area.top)
	{
	    inDy += area.top - extents.top;
	}
	else if (w->serverHeight () <= workArea.height () &&
		 extents.bottom > area.bottom)
	{
	    inDy += area.bottom - extents.bottom;
	}

	/* apply the change */
	dx += inDx - dxBefore;
	dy += inDy - dyBefore;
    }

    result.set (dx, dy);

    return result;
}

void
PutScreen::preparePaint (int ms)
{

    PUT_SCREEN (screen);

    if (ps->moreAdjust && ps->grabIndex)
    {
	int        steps;
	float      amount, chunk;

	amount = ms * 0.025f * ps->optionGetSpeed ();
	steps = amount / (0.5f * ps->optionGetTimestep ());
	if (!steps)
	    steps = 1;
	chunk = amount / (float)steps;

	while (steps--)
	{
	    Window endAnimationWindow = None;

	    ps->moreAdjust = 0;
	    foreach(CompWindow *w, screen->windows ())
	    {
		PUT_WINDOW (w);

		if (pw->adjust)
		{
		    pw->adjust = adjustVelocity (w);
		    ps->moreAdjust |= pw->adjust;

		    pw->tx += pw->xVelocity * chunk;
		    pw->ty += pw->yVelocity * chunk;

		    if (!pw->adjust)
		    {
			/* animation done */
			finishWindowMovement (w);

			if (w->id  () == screen->activeWindow ())
			    endAnimationWindow = w->id ();

			pw->tx = pw->ty = 0;
		    }
		}
    	    }
	    if (!ps->moreAdjust)
	    {
		/* unfocus moved window if enabled */
		if (ps->optionGetUnfocusWindow ())
		    screen->focusDefaultWindow ();
		else if (endAnimationWindow)
		    screen->sendWindowActivationRequest (endAnimationWindow);
		break;
    	    }
	}
    }

    cScreen->preparePaint (ms);
}

/* This is the guts of the paint function. You can transform the way the
 * entire output is painted or you can just draw things on screen with openGL.
 * The unsigned int here is a mask for painting the screen, see opengl/opengl.h
 * on how you can change it
 */

bool
PutScreen::glPaintOutput (const GLScreenPaintAttrib     &attrib,
			  const GLMatrix		&transform,
			  const CompRegion		&region,
			  CompOutput 		        *output,
			  unsigned int		        mask)
{
    bool status;

    PUT_SCREEN (screen);

    if (ps->moreAdjust)
	mask |= PAINT_SCREEN_WITH_TRANSFORMED_WINDOWS_MASK;

    status = gScreen->glPaintOutput (attrib, transform, region, output, mask);

    return status;
}

void
PutScreen::donePaint ()
{
    PUT_SCREEN (screen);

    if (ps->moreAdjust && ps->grabIndex)
    {
	cScreen->damageScreen (); // FIXME
    }
    else
    {
	if (ps->grabIndex)
	{
	    /* release the screen grab */
	    screen->removeGrab (ps->grabIndex, NULL);
	    ps->grabIndex = 0;
	}
    }

    cScreen->donePaint ();
}

/* This is our event handler. It directly hooks into the screen's X Event handler and allows us to handle
 * our raw X Events
 */

void
PutScreen::handleEvent (XEvent *event)
{
    switch (event->type) {
    /* handle client events */
    case ClientMessage:
	/* accept the custom atom for putting windows */
	if (event->xclient.message_type == compizPutWindowAtom)
	{
	    CompWindow *w;

	    w = screen->findWindow (event->xclient.window);
	    if (w)
	    {
		/*
		 * get the values from the xclientmessage event and populate
		 * the options for put initiate
		 *
		 * the format is 32
		 * and the data is
		 * l[0] = x position - unused (for future PutExact)
		 * l[1] = y position - unused (for future PutExact)
		 * l[2] = viewport number
		 * l[3] = put type, int value from enum
		 * l[4] = output number
		 */
		CompOption::Vector opt (5);

		CompOption::Value value0 = (int) event->xclient.window;
		opt.push_back (CompOption ( "window",CompOption::TypeInt));
		opt[0].set (value0);

		CompOption::Value value1 = (int) event->xclient.data.l[0];
		opt.push_back (CompOption ( "x",CompOption::TypeInt));
		opt[1].set (value1);

		CompOption::Value value2 = (int) event->xclient.data.l[1];
		opt.push_back (CompOption ( "y",CompOption::TypeInt));
		opt[2].set (value2);

		CompOption::Value value3 = (int) event->xclient.data.l[2];
		opt.push_back (CompOption ( "viewport",CompOption::TypeInt));
		opt[3].set (value3);

		CompOption::Value value4 = (int) event->xclient.data.l[4];
		opt.push_back (CompOption ( "output",CompOption::TypeInt));
		opt[4].set (value4);

		initiateCommon (NULL, 0, opt,
				(PutType) event->xclient.data.l[3]);
	    }
	}
	break;
    default:
	break;
    }
    screen->handleEvent (event);
}

/* This gets called whenever the window needs to be repainted.
 * WindowPaintAttrib gives you some attributes like brightness/saturation etc
 * to play around with. GLMatrix is the window's transformation matrix. The
 * unsigned int is the mask, have a look at opengl.h on what you can do
 * with it */
bool
PutWindow::glPaint (const GLWindowPaintAttrib &attrib,
		    const GLMatrix            &transform,
		    const CompRegion          &region,
		    unsigned int              mask)
{
    GLMatrix wTransform (transform);

    if (adjust)
    {
	wTransform.translate (tx, ty, 0.0f);
	mask |= PAINT_WINDOW_TRANSFORMED_MASK;
    }

    return gWindow->glPaint (attrib, wTransform, region, mask);
}

/*
 * initiate action callback
 */
bool
PutScreen::initiateCommon (CompAction         *action,
			   CompAction::State  state,
			   CompOption::Vector &option,
			   PutType            type)
{
    CompWindow *w;
    Window     xid;

    xid = CompOption::getIntOptionNamed (option,  "window", 0);
    if (!xid)
	xid = screen->activeWindow ();

    w = screen->findWindow (xid);
    if (w)
    {
	CompScreen *s = screen;
	CompPoint  delta;

	/* we don't want to do anything with override redirect windows */
	if (w->overrideRedirect ())
	    return false;

	/* we don't want to be moving the desktop and docks */
	if (w->type () & (CompWindowTypeDesktopMask |
			  CompWindowTypeDockMask))
	    return false;

	/* don't move windows without move action */
	if (!(w->actions () & CompWindowActionMoveMask))
	    return false;

	/* only allow movement of fullscreen windows to next output */
	if (type != PutNextOutput &&
	    (w->type () & CompWindowTypeFullscreenMask))
	{
	    return false;
	}

	/*
	 * handle the put types
	 */
	delta = getDistance (w, type, option);

	/* don't do anything if there is nothing to do */
	if (!delta.x () && !delta.y ())
	    return true;

	if (!grabIndex)
	{
	    /* this will keep put from working while something
	       else has a screen grab */
	    if (s->otherGrabExist ("put", NULL))
		return false;

	    /* we are ok, so grab the screen */
	    grabIndex = s->pushGrab (s->invisibleCursor (), "put");
	}

	if (grabIndex)
	{
	    PUT_WINDOW (w);

	    lastWindow = w->id ();

	    /* save the windows position in the saveMask
	     * this is used when unmaximizing the window
	     */
	    if (w->saveMask () & CWX)
		w->saveWc ().x += delta.x ();

	    if (w->saveMask () & CWY)
		w->saveWc ().y += delta.y ();

	    /* Make sure everyting starts out at the windows
	       current position */
	    pw->lastX = w->x () + pw->tx;
	    pw->lastY = w->y () + pw->ty;

	    pw->targetX = pw->lastX + delta.x ();
	    pw->targetY = pw->lastY + delta.y ();

	    /* mark for animation */
	    pw->adjust = true;
	    moreAdjust = true;

	    /* cause repainting */
	    pw->cWindow->addDamage ();
	}
    }

    /* tell event.c handleEvent to not call XAllowEvents */
    return false;
}

PutType
PutScreen::typeFromString (const CompString &type)
{
    if (type == "absolute")
	return PutAbsolute;
    else if (type == "relative")
	return PutRelative;
    else if (type == "pointer")
	return PutPointer;
    else if (type == "viewport")
	return (PutType) PutViewport;
    else if (type == "viewportleft")
	return PutViewportLeft;
    else if (type == "viewportright")
	return PutViewportRight;
    else if (type == "viewportup")
	return PutViewportUp;
    else if (type == "viewportdown")
	return PutViewportDown;
    else if (type == "nextoutput")
	return PutNextOutput;
    else if (type == "restore")
	return PutRestore;
    else if (type == "bottomleft")
	return PutBottomLeft;
    else if (type == "emptybottomleft")
	return PutEmptyBottomLeft;
    else if (type == "left")
	return PutLeft;
    else if (type == "emptyleft")
	return PutEmptyLeft;
    else if (type == "topleft")
	return PutTopLeft;
    else if (type == "emptytopleft")
	return PutEmptyTopLeft;
    else if (type == "top")
	return PutTop;
    else if (type == "emptytop")
	return PutEmptyTop;
    else if (type == "topright")
	return PutTopRight;
    else if (type == "emptytopright")
	return PutEmptyTopRight;
    else if (type == "right")
	return PutRight;
    else if (type == "emptyright")
	return PutEmptyRight;
    else if (type == "bottomright")
	return PutBottomRight;
    else if (type == "emptybottomright")
	return PutEmptyBottomRight;
    else if (type == "bottom")
	return PutBottom;
    else if (type == "emptybottom")
	return PutEmptyBottom;
    else if (type == "center")
	return PutCenter;
    else if (type == "emptycenter")
	return PutEmptyCenter;
    else
	return PutUnknown;
}


bool
PutScreen::initiate (CompAction         *action,
		     CompAction::State  state,
		     CompOption::Vector &option)
{
    PutType    type = PutUnknown;
    CompString typeString;

    typeString = CompOption::getStringOptionNamed (option, "type");
    if (!typeString.empty ())
    	type = typeFromString (typeString);

/*    if (type == (PutType) PutViewport)
	return toViewport (action, state, option);
    else*/
    return initiateCommon (action, state, option,type);
}

bool
PutScreen::toViewport (CompAction         *action,
		       CompAction::State  state,
		       CompOption::Vector &option,
		       int vp)
{
    int last = option.size ();
    option.resize(last+1);
    option[last].setName ("viewport",CompOption::TypeInt);
    option[last].value ().set (vp-1);

    return initiateCommon (action, state, option, (PutType) PutViewport);
}


PutScreen::PutScreen (CompScreen *screen) :
    PluginClassHandler <PutScreen, CompScreen> (screen),
    PutOptions (),
    screen (screen),
    cScreen (CompositeScreen::get (screen)),
    gScreen (GLScreen::get (screen)),
    lastWindow (None),
    lastType (PutUnknown),
    moreAdjust (false),
    grabIndex (0)
{
    ScreenInterface::setHandler (screen);
    CompositeScreenInterface::setHandler (cScreen);
    GLScreenInterface::setHandler (gScreen);

    compizPutWindowAtom = XInternAtom(screen->dpy (),
				      "_COMPIZ_PUT_WINDOW", 0);

    optionSetPutPutInitiate (boost::bind (&PutScreen::initiate, this,
					  _1, _2, _3));

#define setAction(action, type) \
    optionSet##action##KeyInitiate (boost::bind (&PutScreen::initiateCommon,   \
					      this, _1,_2,_3,type));           \
    optionSet##action##ButtonInitiate (boost::bind (&PutScreen::initiateCommon,\
					      this, _1,_2,_3,type))

    setAction (PutRestore, PutRestore);
    setAction (PutPointer, PutPointer);
    setAction (PutNextOutput, PutNextOutput);
    setAction (PutCenter, PutCenter);
    setAction (PutEmptyCenter, PutEmptyCenter);
    setAction (PutLeft, PutLeft);
    setAction (PutEmptyLeft, PutEmptyLeft);
    setAction (PutRight, PutRight);
    setAction (PutEmptyRight, PutEmptyRight);
    setAction (PutTop, PutTop);
    setAction (PutEmptyTop, PutEmptyTop);
    setAction (PutBottom, PutBottom);
    setAction (PutEmptyBottom, PutEmptyBottom);
    setAction (PutTopleft, PutTopLeft);
    setAction (PutEmptyTopleft, PutEmptyTopLeft);
    setAction (PutTopright, PutTopRight);
    setAction (PutEmptyTopright, PutEmptyTopRight);
    setAction (PutBottomleft, PutBottomLeft);
    setAction (PutEmptyBottomleft, PutEmptyBottomLeft);
    setAction (PutBottomright, PutBottomRight);
    setAction (PutEmptyBottomright, PutEmptyBottomRight);

#define setViewportAction(num)						       \
    optionSetPutViewport##num##KeyInitiate(boost::bind (&PutScreen::toViewport,\
				this, _1,_2,_3,num));

    setViewportAction(1);
    setViewportAction(2);
    setViewportAction(3);
    setViewportAction(4);
    setViewportAction(5);
    setViewportAction(6);
    setViewportAction(7);
    setViewportAction(8);
    setViewportAction(9);
    setViewportAction(10);
    setViewportAction(11);
    setViewportAction(12);

#define setDirectionAction(action) \
    optionSet##action##KeyInitiate(boost::bind (&PutScreen::initiateCommon, \
						this, _1,_2,_3,action))

    setDirectionAction(PutViewportLeft);
    setDirectionAction(PutViewportRight);
    setDirectionAction(PutViewportUp);
    setDirectionAction(PutViewportDown);
}

PutWindow::PutWindow (CompWindow *window) :
    PluginClassHandler <PutWindow, CompWindow> (window),
    window (window),
    cWindow (CompositeWindow::get (window)),
    gWindow (GLWindow::get (window)),
    xVelocity (0),
    yVelocity (0),
    tx (0),
    ty (0),
    lastX (window->serverX ()),
    lastY (window->serverY ()),
    adjust (false)
{
    WindowInterface::setHandler (window);
    CompositeWindowInterface::setHandler (cWindow);
    GLWindowInterface::setHandler (gWindow);
}

bool
PutPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION))
	 return false;
    if (!CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI))
	 return false;
    if (!CompPlugin::checkPluginABI ("opengl", COMPIZ_OPENGL_ABI))
	 return false;

    return true;
}
