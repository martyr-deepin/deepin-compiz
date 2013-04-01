/*
 * Compiz Fusion Maximumize plugin
 *
 * Copyright 2007-2008 Kristian Lyngstøl <kristian@bohemians.org>
 * Copyright 2008 Eduardo Gurgel Pinho <edgurgel@gmail.com>
 * Copyright 2008 Marco Diego Aurelio Mesquita <marcodiegomesquita@gmail.com>
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
 * Author(s):
 * Kristian Lyngstøl <kristian@bohemians.org>
 * Eduardo Gurgel <edgurgel@gmail.com>
 * Marco Diego Aurélio Mesquita <marcodiegomesquita@gmail.com>
 *
 * Description:
 *
 * Maximumize resizes a window so it fills as much of the free space in any
 * direction as possible without overlapping with other windows.
 *
 */

#include "maximumize.h"

COMPIZ_PLUGIN_20090315 (maximumize, MaximumizePluginVTable);

/* Convenience constants to make the code more readable (hopefully) */
const short		REDUCE  = -1;
const unsigned short	INCREASE = 1;

/* Returns true if rectangles a and b intersect by at least 40 in both
 * directions
 */
bool
MaximumizeScreen::substantialOverlap (const CompRect& a,
				      const CompRect& b)
{
    if (a.x2 () <= b.x1 () + 40)
	return false;
    if (b.x2 () <= a.x1 () + 40)
	return false;
    if (a.y2 () <= b.y1 () + 40)
	return false;
    if (b.y2 () <= a.y1 () + 40)
	return false;

    return true;
}

/* Generates a region containing free space (here the
 * active window counts as free space). The region argument
 * is the start-region (ie: the output dev).
 * Logic borrowed from opacify (courtesy of myself).
 */

CompRegion
MaximumizeScreen::findEmptyRegion (CompWindow      *window,
				   const CompRect& output)
{
    CompRegion newRegion (output);
    CompRect   tmpRect, windowRect;

    if (optionGetIgnoreOverlapping ())
	windowRect = window->serverBorderRect ();

    foreach (CompWindow *w, screen->windows ())
    {
	CompRegion tmpRegion;

	if (w->id () == window->id ())
            continue;

        if (w->invisible () /*|| w->hidden */|| w->minimized ())
            continue;

	if (w->wmType () & CompWindowTypeDesktopMask)
	    continue;

	if (w->wmType () & CompWindowTypeDockMask)
	{
	    if (w->struts ())
	    {
		tmpRegion += w->struts ()->left;
		tmpRegion += w->struts ()->right;
		tmpRegion += w->struts ()->top;
		tmpRegion += w->struts ()->bottom;

		newRegion -= tmpRegion;
	    }
	    continue;
	}

	if (optionGetIgnoreSticky () &&
	    (w->state () & CompWindowStateStickyMask) &&
	    !(w->wmType () & CompWindowTypeDockMask))
	{
	    continue;
	}

	tmpRect = w->serverBorderRect ();

	if (optionGetIgnoreOverlapping () &&
	    substantialOverlap (tmpRect, windowRect))
	{
	    continue;
	}

	tmpRegion += tmpRect;
	newRegion -= tmpRegion;
    }

    return newRegion;
}

/* Returns true if box a has a larger area than box b.  */
bool
MaximumizeScreen::boxCompare (const CompRect& a,
			      const CompRect& b)
{
    int areaA, areaB;

    areaA = a.width () * a.height ();
    areaB = b.width () * b.height ();

    return (areaA > areaB);
}

/* While the rectangle has space, add inc to i. When it CHEKCREC fails (ie:
 * we overstepped our boundaries), reduce i by inc; undo the last change.
 * inc is either 1 or -1, but could easily be looped over for fun and
 * profit. (Ie: start with -100, hit the wall, go with -20, then -1, etc.)
 * 
 * NOTE: We have to pass along tmp, r and w for CHECKREC. 
 * FIXME:  
 */

/* If a window, with decorations, defined by tmp and w is still in free
 * space, evaluate to true. 
 */

inline void
MaximumizeScreen::addToCorner (CompRect&   rect,
			       Corner      corner,
			       const short inc)
{
    switch (corner) {
	case X1:
	    rect.setX (rect.x () + inc);
	    break;
	case X2:
	    rect.setWidth (rect.width () + inc);
	    break;
	case Y1:
	    rect.setY (rect.y () + inc);
	    break;
	case Y2:
	    rect.setHeight (rect.height () + inc);
	    break;
    }
}

#define CHECKREC \
    r.contains (CompRect (tmp.x1 () - w->border ().left,   \
			  tmp.y1 () - w->border ().top,    \
			  tmp.width () + w->border ().right + w->border ().left,  \
			  tmp.height () + w->border ().bottom + w->border ().top))

void
MaximumizeScreen::growGeneric (CompWindow        *w,
			       CompRect&         tmp,
			       const CompRegion& r,
			       Corner            corner,
			       const short       inc)
{
    bool touch = false;

    while (CHECKREC)
    {
	addToCorner (tmp, corner, inc);
	touch = true;
    }

    if (touch)
	addToCorner (tmp, corner, -inc);
}

/* Grow a box in the left/right directions as much as possible without
 * overlapping other relevant windows.  */
void
MaximumizeScreen::growWidth (CompWindow        *w,
			     CompRect&         tmp,
			     const CompRegion& r,
			     const MaxSet&     mset)
{
    if (mset.left)
	growGeneric (w, tmp, r, X1, REDUCE);

    if (mset.right)
	growGeneric (w, tmp, r, X2, INCREASE);
}

/* Grow box in the up/down direction as much as possible without
 * overlapping other relevant windows. */
void 
MaximumizeScreen::growHeight (CompWindow        *w,
			      CompRect&         tmp,
			      const CompRegion& r,
			      const MaxSet&     mset)
{
    if (mset.down)
	growGeneric (w, tmp, r, Y2, INCREASE);
    
    if (mset.up)
	growGeneric (w, tmp, r, Y1, REDUCE);
}

/* Extends the given box for Window w to fit as much space in region r.
 * If XFirst is true, it will first expand in the X direction,
 * then Y. This is because it gives different results.
 */
CompRect
MaximumizeScreen::extendBox (CompWindow        *w,
			     const CompRect&   tmp,
			     const CompRegion& r,
			     bool	       xFirst,
			     const MaxSet&     mset)
{
    CompRect result = tmp;

    if (xFirst)
    {
	growWidth (w, result, r, mset);
	growHeight (w, result, r, mset);
    }
    else
    {
	growHeight (w, result, r, mset);
	growWidth (w, result, r, mset);
    }

    return result;
}

/* These two functions set the width and height respectively, with gravity
 * towards the center of the window. They will set the box-width to width
 * as long as at least one of the sides can be modified. Same for height.
 */

void
MaximumizeScreen::setBoxWidth (CompRect&     box,
			       const int     width,
			       const MaxSet& mset)
{
    int original = box.width ();
    int increment;

    if (!mset.left && !mset.right)
	return;

    if (mset.left != mset.right)
	increment = original - width;
    else
	increment = (original - width) / 2;

    box.setX (box.x () + (mset.left ? increment : 0));
    box.setWidth (box.width () - (mset.right ? increment : 0));
}

void
MaximumizeScreen::setBoxHeight (CompRect&     box,
				const int     height,
				const MaxSet& mset)
{
    int original = box.height ();
    int increment;

    if (!mset.down && !mset.up)
	return ;

    if (mset.up != mset.down)
	increment = original - height;
    else
	increment = (original - height) / 2;

    box.setY (box.y () + (mset.up ? increment : 0));
    box.setHeight (box.height () - (mset.down ? increment : 0));
}

/* Reduce box size by setting width and height to 1/4th or the minimum size
 * allowed, whichever is bigger.
 */
CompRect
MaximumizeScreen::minimumize (CompWindow      *w,
			      const CompRect& box,
			      const MaxSet&   mset)
{
    const int minWidth = w->sizeHints ().min_width;
    const int minHeight = w->sizeHints ().min_height;
    int       width, height;
    CompRect  result = box;

    /* unmaximize first */
    w->maximize (0);

    width  = result.width ();
    height = result.height ();

    if (width / 4 < minWidth)
	setBoxWidth (result, minWidth, mset);
    else
	setBoxWidth (result, width / 4, mset);

    if (height / 4 < minHeight)
	setBoxHeight (result, minHeight, mset);
    else
	setBoxHeight (result, height / 4, mset);

    return result;
}

/* Create a box for resizing in the given region
 * Also shrinks the window box in case of minor overlaps.
 * FIXME: should be somewhat cleaner.
 */
CompRect
MaximumizeScreen::findRect (CompWindow        *w,
			    const CompRegion& r,
			    const MaxSet&     mset)
{
    CompRect windowBox, ansA, ansB, orig;

    windowBox.setGeometry (w->serverX (),
			   w->serverY (),
			   w->serverWidth (),
			   w->serverHeight ());
    orig = windowBox;

    if (mset.shrink)
	windowBox = minimumize (w, windowBox, mset);

    if (!mset.grow)
	return windowBox;

    ansA = extendBox (w, windowBox, r, true, mset);
    ansB = extendBox (w, windowBox, r, false, mset);

    if (!optionGetAllowShrink ())
    {
	if (boxCompare (orig, ansA) && boxCompare (orig, ansB))
	    return orig;
    }
    else
    {
	/* Order is essential here. */
	if (!boxCompare (ansA, windowBox) && !boxCompare (ansB, windowBox))
	    return orig;
    }


    if (boxCompare (ansA, ansB))
	return ansA;
    else
	return ansB;

}

/* Calls out to compute the resize */
unsigned int
MaximumizeScreen::computeResize (CompWindow     *w,
				 XWindowChanges *xwc,
				 const MaxSet&  mset)
{
    int               outputDevice = w->outputDevice ();
    const CompOutput& output = screen->outputDevs ()[outputDevice];
    CompRegion        region;
    unsigned int      mask = 0;
    CompRect          box;

    region = findEmptyRegion (w, output);
    box    = findRect (w, region, mset);

    if (box.x1 () != w->serverX ())
	mask |= CWX;

    if (box.y1 () != w->serverY ())
	mask |= CWY;

    if (box.width () != w->serverWidth ())
	mask |= CWWidth;

    if (box.height () != w->serverHeight ())
	mask |= CWHeight;

    xwc->x      = box.x1 ();
    xwc->y      = box.y1 ();
    xwc->width  = box.width ();
    xwc->height = box.height ();

    return mask;
}

/* General trigger. This is for maximumizing / minimumizing without a direction
 * Note that the function is static in the class so 'this' in unavailable,
 * we have to use MAX_SCREEN (screen) to get the object
 */
bool
MaximumizeScreen::triggerGeneral (CompAction         *action,
				  CompAction::State  state,
				  CompOption::Vector &options,
				  bool		     grow)
{
    Window     xid;
    CompWindow *w;

    xid = CompOption::getIntOptionNamed (options, "window");
    w   = screen->findWindow (xid);
    if (w)
    {
	int            width, height;
	unsigned int   mask;
	XWindowChanges xwc;
	MaxSet	       mset;

	MAX_SCREEN (screen);

	if (screen->otherGrabExist (0))
	   return false;

	mset.left  = ms->optionGetMaximumizeLeft ();
	mset.right = ms->optionGetMaximumizeRight ();
	mset.up    = ms->optionGetMaximumizeUp ();
	mset.down  = ms->optionGetMaximumizeDown ();

	mset.grow   = grow;
	mset.shrink = true;

	mask = computeResize (w, &xwc, mset);
	if (mask)
	{
	    if (w->constrainNewWindowSize (xwc.width, xwc.height,
					   &width, &height))
	    {
		mask |= CWWidth | CWHeight;
		xwc.width  = width;
		xwc.height = height;
	    }

	    if (w->mapNum () && (mask & (CWWidth | CWHeight)))
		w->sendSyncRequest ();

	    w->configureXWindow (mask, &xwc);
	}
    }

    return true;
}

/* 
 * Maximizing on specified direction.
 */
bool
MaximumizeScreen::triggerDirection (CompAction         *action,
				    CompAction::State  state,
				    CompOption::Vector &options,
				    bool	       left,
				    bool	       right,
				    bool	       up,
				    bool	       down,
				    bool	       grow)
{
    Window     xid;
    CompWindow *w;

    xid = CompOption::getIntOptionNamed (options, "window");
    w   = screen->findWindow (xid);
    if (w)
    {
	int            width, height;
	unsigned int   mask;
	XWindowChanges xwc;
	MaxSet	       mset;

	if (screen->otherGrabExist (0))
	    return false;

	mset.left   = left;
	mset.right  = right;
	mset.up     = up;
	mset.down   = down;
	mset.grow   = grow;
	mset.shrink = !grow;

	mask = computeResize (w, &xwc, mset);
	if (mask)
	{
	    if (w->constrainNewWindowSize (xwc.width, xwc.height,
					   &width, &height))
	    {
		mask |= CWWidth | CWHeight;
		xwc.width  = width;
		xwc.height = height;
	    }

	    if (w->mapNum () && (mask & (CWWidth | CWHeight)))
		w->sendSyncRequest ();

	    w->configureXWindow (mask, &xwc);
	}
    }

    return true;
}

/* Configuration, initialization, boring stuff. --------------------- */

/* Screen Constructor */
MaximumizeScreen::MaximumizeScreen (CompScreen *screen) :
    PluginClassHandler <MaximumizeScreen, CompScreen> (screen)
{
/* This macro uses boost::bind to have lots of callbacks trigger the same
 * function with different arguments */
#define MAXSET(opt, up, down, left, right)			      \
    optionSet##opt##Initiate (boost::bind (&MaximumizeScreen::triggerDirection,\
					   this, _1, _2, _3, up,  \
					   down, left, right, true))
#define MINSET(opt, up, down, left, right)			      \
    optionSet##opt##Initiate (boost::bind (&MaximumizeScreen::triggerDirection,\
					   this, _1, _2, _3, up,  \
					   down, left, right, false))

    /* Maximumize Bindings */
    optionSetTriggerMaxKeyInitiate (
	boost::bind (&MaximumizeScreen::triggerGeneral,this, _1, _2, _3, true));

    /* TriggerDirection, left, right, up, down */
    MAXSET (TriggerMaxLeft, true, false, false, false);
    MAXSET (TriggerMaxRight, true, false, false, false);
    MAXSET (TriggerMaxUp, false, false, true, false);
    MAXSET (TriggerMaxDown, false, false, false, true);
    MAXSET (TriggerMaxHorizontally, true, true, false, false);
    MAXSET (TriggerMaxVertically, false, false, true, true);
    MAXSET (TriggerMaxUpLeft, true, false, true, false);
    MAXSET (TriggerMaxUpRight, false, true, true, false);
    MAXSET (TriggerMaxDownLeft, true, false, false, true);
    MAXSET (TriggerMaxDownRight, false, true, false, true);

    /* Maximumize Bindings */
    optionSetTriggerMinKeyInitiate (
	boost::bind (&MaximumizeScreen::triggerGeneral,
		     this, _1, _2, _3, false));

    MINSET (TriggerMinLeft, true, false, false, false);
    MINSET (TriggerMinRight, true, false, false, false);
    MINSET (TriggerMinUp, false, false, true, false);
    MINSET (TriggerMinDown, false, false, false, true);
    MINSET (TriggerMinHorizontally, true, true, false, false);
    MINSET (TriggerMinVertically, false, false, true, true);
    MINSET (TriggerMinUpLeft, true, false, true, false);
    MINSET (TriggerMinUpRight, false, true, true, false);
    MINSET (TriggerMinDownLeft, true, false, false, true);
    MINSET (TriggerMinDownRight, false, true, false, true);

#undef MAXSET
#undef MINSET
}

/* Initial plugin init function called. Checks to see if we are ABI
 * compatible with core, otherwise unload */

bool
MaximumizePluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION))
	return false;

    return true;
}
