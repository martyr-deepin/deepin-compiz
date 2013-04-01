/**
 *
 * Compiz group plugin
 *
 * selection.cpp
 *
 * Copyright : (C) 2006-2010 by Patrick Niklaus, Roi Cohen,
 * 				Danny Baumann, Sam Spilsbury
 * Authors: Patrick Niklaus <patrick.niklaus@googlemail.com>
 *          Roi Cohen       <roico.beryl@gmail.com>
 *          Danny Baumann   <maniac@opencompositing.org>
 * 	    Sam Spilsbury   <smspillaz@gmail.com>
 *
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
 **/

#include "group.h"

/*
 * GroupWindow::windowInRegion
 *
 * Determine if a prescribed region intersects a window and determine
 * if the region intersects an adequate percentage of that window
 * (specified by precision) intersects the window
 *
 */
bool
GroupWindow::windowInRegion (CompRegion src,
				  float  precision)
{
    int    i;
    int    area = 0;
    CompRegion buf;

    /* Intersecting region */
    buf = window->region ().intersected (src);

    /* buf area */
    for (i = 0; i < buf.numRects (); i++)
    {
	CompRect box = buf.rects ().at (i);
	area += (box.width ()) * (box.height ()); /* width * height */
    }

    if (area >= WIN_WIDTH (window) * WIN_HEIGHT (window) * precision)
    {
	src = window->region ().subtracted (src);
	return true;
    }

    return false;
}

/*
 * groupFindGroupInWindows
 *
 * Inline utility function, returns true if a group is in a list
 * of windows
 *
 */
static inline bool
groupFindGroupInWindows (GroupSelection *group,
			 CompWindowList &windows)
{
    foreach (CompWindow *cw, windows)
    {
	GROUP_WINDOW (cw);

	if (gw->mGroup == group)
	    return true;
    }

    return false;
}

/*
 * groupFindWindowsInRegion
 *
 * Utility function, finds all the windows in a region and returns
 * a list of them.
 *
 */
static CompWindowList *
groupFindWindowsInRegion (CompRegion     reg)
{
    GROUP_SCREEN (screen);

    float      	   precision = gs->optionGetSelectPrecision () / 100.0f;
    CompWindowList *ret;
    CompWindowList::reverse_iterator rit = screen->windows ().rbegin ();

    ret = new CompWindowList ();

    if (!ret)
	return NULL;

    /* Go back-to-front with selection */
    while (rit != screen->windows ().rend ())
    {
	CompWindow *w = *rit;
	GROUP_WINDOW (w);

	/* If the window is groupable and in our region */
	if (gw->isGroupWindow () &&
	    gw->windowInRegion (reg, precision))
	{
	    /* Don't bother if this window has a group and there are
	     * windows from this group in the selection (all windows
	     * from the group get added to the selection automatically)
	     */
	    if (gw->mGroup && groupFindGroupInWindows (gw->mGroup, *ret))
	    {
		++rit;
		continue;
	    }

	    ret->push_back (w);
	}

	++rit;
    }

    return ret;
}

/*
 * Selection::deselect (CompWindow variant)
 *
 * Remove a window from selection
 *
 */
void
Selection::deselect (CompWindow *w)
{
    if (size ())
    {
	GroupWindow::get (w)->checkFunctions ();
	CompositeWindow::get (w)->addDamage ();
	remove (w);
    }

    GroupWindow::get (w)->mInSelection = false;
}

/*
 * Selection::deselect (GroupSelection variant)
 *
 * Deselect all windows in a group, but don't just call
 * deselect on all windows in the group since that is slow, instead
 * compare lists and pop matching windows
 */

void
Selection::deselect (GroupSelection *group)
{
    /* unselect group */
    CompWindowList copy = (CompWindowList) *this;
    CompWindowList::iterator it = begin ();
    int nsize = size () - group->mWindows.size ();
    unsigned int num = MAX (0, nsize);

    /* Faster than doing groupDeleteSelectionWindow
       for each window in this group. */
    resize (num);

    foreach (CompWindow *cw, copy)
    {
	GROUP_WINDOW (cw);

	if (gw->mGroup == group)
	{
	    gw->mInSelection = false;
	    gw->checkFunctions ();
	    gw->cWindow->addDamage ();
	    continue;
	}

	(*it) = cw;
	++it;
    }
}

/*
 * Selection::select
 *
 * Add a CompWindow to this selection (animate its fade too)
 *
 */
void
Selection::select (CompWindow *w)
{
    GROUP_WINDOW (w);

    /* filter out windows we don't want to be groupable */
    if (!gw->isGroupWindow ())
	return;

    push_back (w);

    gw->mInSelection = true;
    gw->checkFunctions ();
    gw->cWindow->addDamage ();
}

/*
 * Selection::select
 *
 * Add a group to this selection (select every window in the group)
 *
 */

void
Selection::select (GroupSelection *g)
{
    foreach (CompWindow *cw, g->mWindows)
	select (cw);
}

/*
 * Selection::selectRegion
 *
 * Selects all windows in the mouse selection region
 *
 */

void
Selection::selectRegion ()
{
    GROUP_SCREEN (screen);

    CompRegion reg;
    CompRect   rect;
    CompWindowList *ws;

    int x      = MIN (mX1, mX2) - 2;
    int y      = MIN (mY1, mY2) - 2;
    int width  = MAX (mX1, mX2) -
		  MIN (mX1, mX2) + 4;
    int height = MAX (mY1, mY2) -
		  MIN (mY1, mY2) + 4;

    rect = CompRect (x, y, width, height);
    reg = emptyRegion.united (rect);

    gs->cScreen->damageRegion (reg);

    ws = groupFindWindowsInRegion (reg);
    if (ws->size ())
    {
	/* (un)select windows */
	foreach (CompWindow *w, *ws)
	    checkWindow (w);

	if (gs->optionGetAutoGroup ())
	{
	    toGroup ();
	}
    }

    delete ws;
}

/*
 * Selection::toGroup
 *
 * Create a new group from this selection. Also "unselect" all of these
 * windows since they are grouped now!
 *
 */

GroupSelection *
Selection::toGroup ()
{
    if (!empty ())
    {
        CompWindowList::iterator it = begin ();
	CompWindow	         *cw;
        GroupSelection *group = NULL;
        bool           tabbed = false;

	/* Try to find existing groups in this selection and
	 * add windows to that rather than going and creating new
	 * groups
	 */
	foreach (cw, *this)
	{
	    GROUP_WINDOW (cw);

	    if (gw->mGroup)
	    {
	        if (!tabbed || group->mTabBar)
		    group = gw->mGroup;

	        if (group->mTabBar)
		    tabbed = true;
	    }
        }

	if (!group)
	{
	    /* create new group */
	    group = new GroupSelection ();

	    if (!group)
		return NULL;

	    GroupScreen::get (screen)->mGroups.push_front (group);
	}

        for (; it != end (); ++it)
        {
	    cw = *it;
	    GROUP_WINDOW (cw);

	    if (gw->mGroup && (group != gw->mGroup))
	        gw->deleteGroupWindow ();
	    gw->addWindowToGroup (group);
	    gw->cWindow->addDamage ();

	    gw->mInSelection = false;
        }

        /* exit selection */
        clear ();

	return group;
    }

    return NULL;
}

/*
 * Selection::checkWindow
 *
 * Select or deselect a window or group if this window has one.
 *
 */
void
Selection::checkWindow (CompWindow *w)
{
    GROUP_WINDOW (w);

    if (gw->mInSelection)
    {
	if (gw->mGroup)
	{
	    deselect (gw->mGroup);
	}
	else
	{
	    /* unselect single window */
	    deselect (w);
	}
    }
    else
    {
	if (gw->mGroup)
	{
	    /* select group */
	    select (gw->mGroup);
	}
	else
	{
	    /* select single window */
	    select (w);
	}
    }
}

/*
 * GroupScreen::selectSingle
 *
 * Action to select a single window
 *
 */
bool
GroupScreen::selectSingle (CompAction         *action,
				CompAction::State  state,
				CompOption::Vector options)
{
    Window     xid;
    CompWindow *w;

    xid = CompOption::getIntOptionNamed (options, "window", 0);
    w   = screen->findWindow (xid);
    if (w)
	mTmpSel.checkWindow (w);

    return true;
}

/*
 * GroupScreen::select
 *
 * Action to initiate the dragging of the selection rect
 *
 */
bool
GroupScreen::select (CompAction         *action,
			  CompAction::State  state,
			  CompOption::Vector options)
{
    if (mGrabState == ScreenGrabNone)
    {
	grabScreen (ScreenGrabSelect);

	if (state & CompAction::StateInitButton)
	{
	    action->setState (state | CompAction::StateTermButton);
	}

	mTmpSel.mX1 = mTmpSel.mX2 = pointerX;
	mTmpSel.mY1 = mTmpSel.mY2 = pointerY;
    }

    return true;
}

/*
 * GroupSelection::selectTerminate
 *
 * Action to terminate selection rect and select all windows in that
 * rect.
 *
 */
bool
GroupScreen::selectTerminate (CompAction         *action,
				   CompAction::State  state,
				   CompOption::Vector options)
{
    if (mGrabState == ScreenGrabSelect)
    {
        grabScreen (ScreenGrabNone);

        if (mTmpSel.mX1 != mTmpSel.mX2 && mTmpSel.mY1 != mTmpSel.mY2)
        {
	    mTmpSel.selectRegion ();
        }
    }

    action->setState (action->state () &
	     ~(CompAction::StateTermButton | CompAction::StateTermKey));

    return false;
}

/*
 * Selection::damage
 *
 * Damage the selection rect
 *
 */
void
Selection::damage (int xRoot,
		   int yRoot)
{
    GROUP_SCREEN (screen);

    CompRegion reg =
	  CompRegion (MIN (mX1, mX2) - 5,
		      MIN (mY1, mY2) - 5,
		      MAX (mX1, mX2) -
		      MIN (mX1, mX2) + 10,
		      MAX (mY1, mY2) -
		      MIN (mY1, mY2) + 10);

    gs->cScreen->damageRegion (reg);

    mX2 = xRoot;
    mY2 = yRoot;

    reg = CompRegion (MIN (mX1, mX2) - 5,
		      MIN (mY1, mY2) - 5,
		      MAX (mX1, mX2) -
		      MIN (mX1, mX2) + 10,
		      MAX (mY1, mY2) -
		      MIN (mY1, mY2) + 10);

    gs->cScreen->damageRegion (reg);
}
