/**
 *
 * Compiz group plugin
 *
 * group.cpp
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

const double PI = 3.14159265359f;

/*
 * GroupExp class
 *
 * A custom subclass of CompMatch::Expression which we can create
 * on matchInitExp to evaluate whether or not a window is actually
 * grouped.
 *
 */

class GroupExp :
    public CompMatch::Expression
{
    public:
	GroupExp (const CompString &str);

	bool evaluate (CompWindow *w);

	bool value;
};

/*
 * GroupExp::GroupExp
 *
 * Internal value here is the "1" or "0" of group=
 * (eg we pass str.substr (6) to get that). Compare whether
 * the window is actually in a group against this value when
 * evaluating the match
 *
 */

GroupExp::GroupExp (const CompString &str) :
    value (strtol (str.c_str (), NULL, 0))
{
}

/*
 * GroupExp::evaluate
 *
 * Compare if we wanted a window to be grouped and whether or not
 * the window is actually in fact grouped
 */

bool
GroupExp::evaluate (CompWindow *w)
{
    GROUP_WINDOW (w);

    return ((value && gw->mGroup) || (!value && !gw->mGroup));
}

/*
 * GroupScreen::matchInitExp
 *
 * Every time a match option is initialized, this wrapped function gets
 * called so that we can create an expression if we need to.
 *
 * Note here that core parses each match string and tokenizes them
 * based on spaces, so we only need to worry about matching
 * "group=" and nothing else.
 *
 */

CompMatch::Expression *
GroupScreen::matchInitExp (const CompString &str)
{
    /* Create a new match object */

    if (str.find ("group=") == 0)
	return new GroupExp (str.substr (6));

    return screen->matchInitExp (str);
}

/*
 * GroupScreen::matchExpHandlerChanged
 *
 * This gets called whenever some plugin needs to check all windows
 * again to see if they still match, so go ahead and update match
 * properties for windows if they are relevant here
 */

void
GroupScreen::matchExpHandlerChanged ()
{
    screen->matchExpHandlerChanged ();

    foreach (CompWindow *w, screen->windows ())
    {
	if (GroupWindow::get (w)->mGroup)
	    screen->matchPropertyChanged (w);
    }
}

/*
 * GroupWindow::isGroupWindow
 *
 * Simple set of condensed checks on the window to check if it
 * makes sense to handle this window with the group plugin
 *
 */

bool
GroupWindow::isGroupWindow ()
{
    GROUP_SCREEN (screen);

    if (window->overrideRedirect ())
	return false;

    if (window->type () & CompWindowTypeDesktopMask)
	return false;

    if (window->invisible ())
	return false;

    if (!gs->optionGetWindowMatch ().evaluate (window))
	return false;

    return true;
}

/*
 * GroupWindow::dragHoverTimeout
 *
 * Description:
 * Activates a window after a certain time a slot has been dragged over it.
 *
 */
bool
GroupWindow::dragHoverTimeout ()
{
    GROUP_SCREEN (screen);

    if (gs->optionGetBarAnimations () && mGroup->mTabBar &&
	mGroup->mTabBar->mBgLayer)
    {
	BackgroundLayer *bg = mGroup->mTabBar->mBgLayer;

	bg->mBgAnimation = BackgroundLayer::AnimationPulse;
	bg->mBgAnimationTime = gs->optionGetPulseTime () * 1000;
    }

    window->activate ();

    return false;
}

/*
 * GroupWindow::updateResizeRectangle
 *
 * Updates the new resize rect of grouped windows of a group
 * which is currently being resized. This checks the difference
 * of the "master geometry" and the original geometry of the grabbed
 * window of the group and applies it to this window. Obviously,
 * we can't do anything like resize past boundaries so constrain the
 * window size if this happens.
 * Also set the relevant configure masks
 */

unsigned int
GroupWindow::updateResizeRectangle (CompRect   masterGeometry,
					 bool	    damage)
{
    CompRect     newGeometry;
    CompRect	 &origGeometry = mGroup->mResizeInfo->mOrigGeometry;
    unsigned int mask = 0;
    int          newWidth, newHeight;
    int          widthDiff, heightDiff;

    if (mResizeGeometry.isEmpty () || !mGroup->mResizeInfo)
	return 0;

    /* New geometry //position// is the difference between the master geometry
     * and the original geometry
     */

    newGeometry.setX (WIN_X (window) + (masterGeometry.x () -
			      mGroup->mResizeInfo->mOrigGeometry.x ()));
    newGeometry.setY (WIN_Y (window) + (masterGeometry.y () -
			      mGroup->mResizeInfo->mOrigGeometry.y ()));

    /* New geometry //size// is the difference in sizes between the master and original
     * geometry, plus the size of this window (obviously check for negative values)
     */

    widthDiff = masterGeometry.width () - origGeometry.width ();
    newGeometry.setWidth (MAX (1, WIN_WIDTH (window) + widthDiff));
    heightDiff = masterGeometry.height () - origGeometry.height ();
    newGeometry.setHeight (MAX (1, WIN_HEIGHT (window) + heightDiff));

    if (window->constrainNewWindowSize (newGeometry.width (), newGeometry.height (),
					&newWidth, &newHeight))
    {
	newGeometry.setSize (CompSize (newWidth, newHeight));
    }

    if (damage)
    {
	if (mResizeGeometry != newGeometry)
	{
	    cWindow->addDamage ();
	}
    }

    /* Set appropriate XConfigure masks */

    if (newGeometry.x () != mResizeGeometry.x ())
    {
	mResizeGeometry.setX (newGeometry.x ());
	mask |= CWX;
    }
    if (newGeometry.y () != mResizeGeometry.y ())
    {
	mResizeGeometry.setY (newGeometry.y ());
	mask |= CWY;
    }
    if (newGeometry.width () != mResizeGeometry.width ())
    {
	mResizeGeometry.setWidth (newGeometry.width ());
	mask |= CWWidth;
    }
    if (newGeometry.height () != mResizeGeometry.height ())
    {
	mResizeGeometry.setHeight (newGeometry.height ());
	mask |= CWHeight;
    }

    return mask;
}

/*
 * GroupScreen::grabScreen
 *
 * Convenience function to grab the screen with different grab
 * masks, etc
 *
 */
void
GroupScreen::grabScreen (GroupScreen::GrabState newState)
{
    if ((mGrabState != newState) && mGrabIndex)
    {
	screen->removeGrab (mGrabIndex, NULL);
	mGrabIndex = 0;
    }

    if (newState == ScreenGrabSelect)
    {
	mGrabIndex = screen->pushGrab (None, "group");
    }
    else if (newState == ScreenGrabTabDrag)
    {
	mGrabIndex = screen->pushGrab (None, "group-drag");
    }

    mGrabState = newState;

    checkFunctions ();
}

/*
 * GroupSelection::raiseWindows
 *
 * Raises all windows in a group
 *
 * Creates a list of all windows that need to be raised
 * and then raises them all in one go (restacks them below the top
 * window).
 *
 * Doesn't appear to work with 0.9, perhaps because of the changes
 * in the restacking code
 *
 */
void
GroupSelection::raiseWindows (CompWindow     *top)
{
    CompWindowList stack;
    CompWindowList::iterator it;

    if (mWindows.size () == 1)
	return;

    stack.resize (mWindows.size () - 1);

    it = stack.begin ();

    foreach (CompWindow *w, screen->windows ())
    {
	GROUP_WINDOW (w);
	if ((w->id () != top->id ()) && (gw->mGroup == this))
	{
	    (*it) = w;
	    ++it;
	}
    }

    foreach (CompWindow *cw, stack)
	cw->restackBelow (top);
}

/*
 * GroupSelection::minimizeWindows
 *
 * Minimizes all windows in a group. Don't minimize the principal
 * window twice, obviously
 *
 */
void
GroupSelection::minimizeWindows (CompWindow     *top,
				 bool           minimize)
{
    foreach (CompWindow *w, mWindows)
    {
	if (w->id () == top->id ())
	    continue;

	if (minimize)
	    w->minimize ();
	else
	    w->unminimize ();
    }
}

/*
 * GroupSelection::shadeWindows
 *
 * Shade all windows in a group
 *
 * After shading we need to update the window attributes
 *
 */
void
GroupSelection::shadeWindows (CompWindow     *top,
			      bool           shade)
{
    unsigned int state;

    foreach (CompWindow *w, mWindows)
    {
	if (w->id () == top->id ())
	    continue;

	if (shade)
	    state = w->state () | CompWindowStateShadedMask;
	else
	    state = w->state () & ~CompWindowStateShadedMask;

	w->changeState (state);
	w->updateAttributes (CompStackingUpdateModeNone);
    }
}

/*
 * GroupSelection::moveWindows
 *
 * Move all windows in a group
 *
 * This does not move all windows straight away, rather it enqueues a
 * a w->move () into the window structure itself, to be batch-dequeued
 * at before the next handleEvent () call. This is because calling
 * wrapped functions half way through the wrap chain is just going
 * to be problematic for us.
 *
 * Also if a window is maximized and there was a viewport change,
 * move the window backwards. Otherwise ignore viewport changes.
 */

void
GroupSelection::moveWindows (CompWindow *top,
			     int 	dx,
			     int 	dy,
			     bool 	immediate,
			     bool	viewportChange)
{
    foreach (CompWindow *cw, mWindows)
    {
	if (!cw)
	    continue;

	if (cw->id () == top->id ())
	    continue;

	GROUP_WINDOW (cw);

	if (cw->state () & MAXIMIZE_STATE)
	{
	    if (viewportChange)
		gw->enqueueMoveNotify (-dx, -dy, immediate, true);
	}
	else if (!viewportChange)
	{
	    gw->mNeedsPosSync = true;
	    gw->enqueueMoveNotify (dx, dy, immediate, true);
	}
    }
}

/*
 * GroupSelection::prepareResizeWindows
 *
 * Description: Sets the resize geometry of this group
 * and makes windows appear "stretched".
 *
 * Use it for animation or something. Currently used to
 * paint windows as stretched while the primary window
 * is resizing (eg through rectangle, outline or stretch mode)
 *
 */

void
GroupSelection::prepareResizeWindows (CompRect &rect)
{
    foreach (CompWindow *cw, mWindows)
    {
	GroupWindow *gcw;

	gcw = GroupWindow::get (cw);
	if (!gcw->mResizeGeometry.isEmpty ())
	{
	    if (gcw->updateResizeRectangle (rect, true))
	    {
		gcw->cWindow->addDamage ();
	    }
	}
    }
}

/*
 * GroupSelection::resizeWindows
 *
 * Description: Configures windows according to set resize geometry
 * in prepareResizeWindows
 *
 */

void
GroupSelection::resizeWindows (CompWindow *top)
{
    CompRect   rect;

    GROUP_SCREEN (screen);

    gs->dequeueMoveNotifies ();

    if (mResizeInfo)
    {
	rect = CompRect (WIN_X (top),
			 WIN_Y (top),
			 WIN_WIDTH (top),
			 WIN_HEIGHT (top));
    }

    foreach (CompWindow *cw, mWindows)
    {
	if (!cw)
	    continue;

	if (cw->id () != top->id ())
	{
	    GROUP_WINDOW (cw);
	    GroupWindow  *gwtt = GroupWindow::get (top);

	    if (!gw->mResizeGeometry.isEmpty ())
	    {
		unsigned int mask;

		gw->mResizeGeometry = CompRect (WIN_X (cw),
						WIN_Y (cw),
						WIN_WIDTH (cw),
						WIN_HEIGHT (cw));

		mask = gw->updateResizeRectangle (rect, false);
		if (mask)
		{
		    XWindowChanges xwc;
		    xwc.x      = gw->mResizeGeometry.x ();
		    xwc.y      = gw->mResizeGeometry.y ();
		    xwc.width  = gw->mResizeGeometry.width ();
		    xwc.height = gw->mResizeGeometry.height ();

		    if (top->mapNum () && (mask & (CWWidth | CWHeight)))
			top->sendSyncRequest ();

		    cw->configureXWindow (mask, &xwc);
		}
		else
		{
		    gwtt->mResizeGeometry = CompRect (0, 0, 0, 0);
		}
	    }
	    if (GroupWindow::get (top)->mNeedsPosSync)
	    {
		cw->syncPosition ();
		gwtt->mNeedsPosSync = false;
	    }
	    gwtt->enqueueUngrabNotify ();
	}
    }

    if (mResizeInfo)
    {
	delete mResizeInfo;
	mResizeInfo = NULL;
    }

    mGrabWindow = None;
    mGrabMask = 0;
}

/*
 * GroupSelection::maximizeWindows
 *
 * Maximizes every window in a group, simply changing it's state
 *
 */

void
GroupSelection::maximizeWindows (CompWindow *top)
{
    foreach (CompWindow *cw, mWindows)
    {
	if (!cw)
	    continue;

	if (cw->id () == top->id ())
	    continue;

	cw->maximize (top->state () & MAXIMIZE_STATE);
    }
}

/*
 * GroupWindow::deleteGroupWindow ()
 *
 * Finitializes any remaining group related bits on a window
 * structure after we have detached from a group.
 *
 */
void
GroupWindow::deleteGroupWindow ()
{
    GroupSelection *group;

    GROUP_SCREEN (screen);

    if (!mGroup)
	return;

    group = mGroup;

    /* If this is the dragged slot in the group, unhook it
     * from the tab bar. Otherwise get rid of it */
    if (group->mTabBar && mSlot)
    {
	if (gs->mDraggedSlot && gs->mDragged &&
	    gs->mDraggedSlot->mWindow->id () == window->id ())
	{
	    group->mTabBar->unhookTabBarSlot (mSlot, false);
	}
	else
	    group->mTabBar->deleteTabBarSlot (mSlot);
    }

    /* If the group has any windows left ... */
    if (group->mWindows.size ())
    {
	/* If the group has more than one window left, and, if
	 * after removing the window, there is only one window left,
	 * then there makes no sense to paint glow on the "group"
	 * with just one window left, (the window *is* grouped, but
	 * for the purposes of what the user can see, the window is just
	 * like any other window, and when we add it to another group,
	 * it will just re-use some of the structures we have already
	 */
	if (group->mWindows.size () > 1)
	{
	    group->mWindows.remove (window);
	    group->mWindowIds.remove (window->id ());

	    if (group->mWindows.size () == 1)
	    {
		/* Glow was removed from this window, too.
		 * Since there is only one window left here,
		 * it is safe to use front ()
		 */
		GROUP_WINDOW (group->mWindows.front ());
		gw->cWindow->damageOutputExtents ();
		gw->window->updateWindowOutputExtents ();

		if (gs->optionGetAutoUngroup ())
		{
		    if (group->mTabBar->mChangeState !=
					       GroupTabBar::NoTabChange)
		    {
			/* a change animation is pending: this most
			   likely means that a window must be moved
			   back onscreen, so we do that here */
			GroupWindow *glw =
			    GroupWindow::get (group->mWindows.front ());

			glw->setWindowVisibility (true);
		    }
		    if (!gs->optionGetAutotabCreate ())
			group->fini ();
		}
	    }
	}
	else
	{
	    group->mWindows.clear ();
	    group->mWindowIds.clear ();
	    group->fini ();
	}

	mGroup = NULL;

	screen->matchPropertyChanged (window);
	cWindow->damageOutputExtents ();
	window->updateWindowOutputExtents ();
	gs->writeSerializedData ();
    }
}

/*
 * GroupWindow::removeWindowFromGroup
 *
 * Takes a window out of a group, there might be a tab bar, so this
 * is really just a wrapper function to handle the untabbing of
 * the single window //first// before going ahead and getting rid
 * of the window from the group.
 *
 */

void
GroupWindow::removeWindowFromGroup ()
{
    GROUP_SCREEN (screen);

    if (!mGroup)
	return;

    if (mGroup->mTabBar && !(mAnimateState & IS_UNGROUPING) &&
	(mGroup->mWindows.size () > 1))
    {
	GroupSelection *group = mGroup;

	/* if the group is tabbed, setup untabbing animation. The
	   window will be deleted from the group at the
	   end of the untabbing. */
	if (HAS_TOP_WIN (group))
	{
	    CompWindow *tw = TOP_TAB (group);
	    int        oldX = mOrgPos.x ();
	    int        oldY = mOrgPos.y ();

	    /* The "original position" of the window for the purposes
	     * of the untabbing animation is centered to the top tab */
	    mOrgPos =
	       CompPoint (WIN_CENTER_X (tw) - (WIN_WIDTH (window) / 2),
			  WIN_CENTER_Y (tw) - (WIN_HEIGHT (window) / 2));

	    /* Destination is here is the "original" position of the window
	     * relative to how far away it was from the main tab */
	    mDestination = mOrgPos + mMainTabOffset;

	    /* The new "main tab offset" is now the original position */
	    mMainTabOffset = CompPoint (oldX, oldY);

	    /* Kick off the animation */

	    if (mTx || mTy)
	    {
		mTx -= (mOrgPos.x () - oldX);
		mTy -= (mOrgPos.y () - oldY);
	    }

	    mAnimateState = IS_ANIMATED;
	    mXVelocity = mYVelocity = 0.0f;
	}

	/* Although when there is no top-tab, it will never really
	   animate anything, if we don't start the animation,
	   the window will never get removed. */
	group->startTabbingAnimation (false);

	setWindowVisibility (true);
	group->mUngroupState = GroupSelection::UngroupSingle;
	mAnimateState |= IS_UNGROUPING;
    }
    else
    {
	/* no tab bar - delete immediately */
	deleteGroupWindow ();

	if (gs->optionGetAutotabCreate () && isGroupWindow ())
	{
	    GroupSelection *g;
	    gs->mTmpSel.clear ();
	    gs->mTmpSel.select (window);
	    g = gs->mTmpSel.toGroup ();
	    if (g)
		g->tabGroup (window);
	}
    }

    checkFunctions ();
}

GroupSelection::~GroupSelection ()
{
}

/*
 * GroupSelection::fini
 *
 * This is //essentially// like a destructor, although it handles
 * setting up the untabbing animation before the group is freed.
 *
 * We cannot put this code in the destructor, since there is no way
 * to prevent the object from being freed if we need to do things
 * like trigger animations
 */
void
GroupSelection::fini ()
{
    GROUP_SCREEN (screen);

    if (mWindows.size ())
    {
	if (mTabBar)
	{
	    /* set up untabbing animation and delete the group
	       at the end of the animation */
	    untabGroup ();
	    mUngroupState = UngroupAll;
	    return;
	}

	/* For every window in the group, we need to do a few
	 * tear down related things (faster than calling
	 * removeWindowFromGroup on every single one)
	 * which includes damaging the current glow region
	 * (output extents), updating the X11 window property
	 * reflect that this window is now gone from the group
	 * and creating new autotabbed groups from the windows
	 * when they are removed
	 */
	foreach (CompWindow *cw, mWindows)
	{
	    GROUP_WINDOW (cw);

	    CompositeWindow::get (cw)->damageOutputExtents ();
	    gw->mGroup = NULL;

	    screen->matchPropertyChanged (cw);
	    cw->updateWindowOutputExtents ();
	    gs->writeSerializedData ();

	    if (gs->optionGetAutotabCreate () && gw->isGroupWindow ())
	    {
		GroupSelection *g;
		gs->mTmpSel.clear ();
		gs->mTmpSel.select (cw);
		g = gs->mTmpSel.toGroup ();
		if (g)
		    g->tabGroup (cw);
	    }

	    gw->checkFunctions ();
	}

	mWindows.clear ();
    }
    else if (mTabBar)
    {
	delete mTabBar;
	mTabBar = NULL;
	mTopId = None;
    }

    /* Pop this group from the groups list */
    gs->mGroups.remove (this);

    /* Make sure there are no dangling pointers to this in GroupScreen */
    if (this == gs->mLastHoveredGroup)
	gs->mLastHoveredGroup = NULL;
    if (this == gs->mLastRestackedGroup)
	gs->mLastRestackedGroup = NULL;

    /* This is slightly evil, but necessary, since it is not possible
     * to make the destructor private (since the object would be
     * non-instantiatable). Also, we don't use the class at all
     * after this, so we can let it's memory go, really
     */
    delete this;
}

void
GroupSelection::changeColor ()
{
    GROUP_SCREEN (screen);

    /* Generate new color */
    float    factor = ((float)RAND_MAX + 1) / 0xffff;

    mColor[0] = (int)(rand () / factor);
    mColor[1] = (int)(rand () / factor);
    mColor[2] = (int)(rand () / factor);
    mColor[3] = 0xffff;

    /* Re-render the selection layer, if it is there */
    if (mTabBar && mTabBar->mSelectionLayer)
    {
	const CompRect &bRect = mTabBar->mTopTab->mRegion.boundingRect ();
	CompSize size (bRect.width (),
		       bRect.height ());
	SelectionLayer *sl = mTabBar->mSelectionLayer;
	SelectionLayer::rebuild (sl, size);

	if (mTabBar->mSelectionLayer)
	    mTabBar->mSelectionLayer->render ();
	gs->cScreen->damageScreen ();
    }
}

/*
 * GroupSelection::GroupSelection
 *
 * Constructor for GroupSelection. Creates an empty group, sets up
 * the color and determines a new ID number.
 *
 */

GroupSelection::GroupSelection () :
    mScreen (screen),
    mTabBar (NULL),
    mTabbingState (NoTabbing),
    mUngroupState (UngroupNone),
    mGrabWindow (None),
    mGrabMask (0),
    mResizeInfo (NULL),
    mTopId (None)
{
    boost::function<void (const CompPoint &)> cb =
		boost::bind (&GroupSelection::handleHoverDetection,
			     this, _1);

    mPoller.setCallback (cb);

    /* glow color */
    changeColor ();
}

/*
 * GroupWindow::addWindowToGroup
 *
 * Adds a window to a group.
 *
 * Note that if NULL is passed, a new group is created. "initialIdent"
 * is there for restoring from window properties, otherwise you can just
 * pass 0.
 *
 */
void
GroupWindow::addWindowToGroup (GroupSelection *group)
{
    GROUP_SCREEN (screen);

    if (mGroup)
	return;

    if (group)
    {
	/* If a group was specified, just add this window to it */
	CompWindow *topTab = NULL;

	mGroup = group;

	group->mWindows.push_back (window);
	group->mWindowIds.push_back (window->id ());

	/* Update glow regions and X11 property */

	checkFunctions ();
	window->updateWindowOutputExtents ();
	cWindow->damageOutputExtents ();

	gs->writeSerializedData ();

	/* If we have more than one window in this group just recently,
	 * then update the first window too, */
	if (group->mWindows.size () == 2)
	{
	    /* first window in the group got its glow, too */
	    GroupWindow::get (group->mWindows.front ())->checkFunctions ();
	    group->mWindows.front ()->updateWindowOutputExtents ();
	    CompositeWindow::get (group->mWindows.front ())->damageOutputExtents ();
	}

	/* If there is a tab bar for this group, then we need to set up
	 * the tabbing animation */
	if (group->mTabBar)
	{
	    /* If a window is being dragged out of a group, then
	     * that group will have no top-tab, so we need to get
	     * the last known top-tab for that group and untab it
	     */
	    if (HAS_TOP_WIN (group))
		topTab = TOP_TAB (group);
	    else if (HAS_PREV_TOP_WIN (group))
	    {
		topTab = PREV_TOP_TAB (group);
		group->mTabBar->mTopTab = group->mTabBar->mPrevTopTab;
		group->mTopId = group->mTabBar->mTopTab->mWindow->id ();
		group->mTabBar->mPrevTopTab = NULL;
	    }

	    if (topTab)
	    {
		if (!mSlot)
		    group->mTabBar->createSlot (window);

		/* Set up the tabbing animation */
		mDestination = CompPoint (WIN_CENTER_X (topTab) -
					  (WIN_WIDTH (window) / 2),
					  WIN_CENTER_Y (topTab) -
					  (WIN_HEIGHT (window) / 2));
		mMainTabOffset = CompPoint (WIN_X (window),
					    WIN_Y (window)) -
				 mDestination;
		mOrgPos = CompPoint (WIN_X (window), WIN_Y (window));
		mXVelocity = mYVelocity = 0.0f;
		mAnimateState = IS_ANIMATED;
		group->startTabbingAnimation (true);

		cWindow->addDamage ();
	    }
	}

	screen->matchPropertyChanged (window);
    }

    gs->writeSerializedData ();

    checkFunctions ();
}

/*
 * GroupScreen::groupWindows
 *
 * Triggerable action to group windows, just adds all the
 * windows in the current selection to a group, or creates a new
 * one and adds them to that.
 *
 */
bool
GroupScreen::groupWindows (CompAction         *action,
			   CompAction::State  state,
			   CompOption::Vector options)
{
    mTmpSel.toGroup ();

    return false;
}

/*
 * GroupScreen::ungroupWindows
 *
 * Actions to ungroup the windows
 *
 */
bool
GroupScreen::ungroupWindows (CompAction          *action,
			     CompAction::State   state,
			     CompOption::Vector  options)
{
    Window     xid;
    CompWindow *w;

    xid = CompOption::getIntOptionNamed (options, "window", 0);
    w   = screen->findWindow (xid);
    if (w)
    {
	GROUP_WINDOW (w);

	/* Find the group of the selected window, kill it */
	if (gw->mGroup)
	    gw->mGroup->fini ();
    }

    return false;
}

/*
 * GroupScreen::removeWindow
 *
 * Triggerable action to remove a single window from a group
 *
 */
bool
GroupScreen::removeWindow (CompAction         *action,
				CompAction::State  state,
				CompOption::Vector options)
{
    Window     xid;
    CompWindow *w;

    xid = CompOption::getIntOptionNamed (options, "window", 0);
    w   = screen->findWindow (xid);
    if (w)
    {
	GROUP_WINDOW (w);

	if (gw->mGroup)
	    gw->removeWindowFromGroup ();
    }

    return false;
}

/*
 * GroupScreen::closeWindows
 *
 * Action to close all windows in a group
 *
 */
bool
GroupScreen::closeWindows (CompAction           *action,
			   CompAction::State    state,
			   CompOption::Vector   options)
{
    Window     xid;
    CompWindow *w;

    xid = CompOption::getIntOptionNamed (options, "window", 0);
    w   = screen->findWindow (xid);
    if (w)
    {
	GROUP_WINDOW (w);

	if (gw->mGroup)
	{
	    foreach (CompWindow *cw, gw->mGroup->mWindows)
		cw->close (screen->getCurrentTime ());
	}
    }

    return false;
}

/*
 * GroupScreen::changeColor
 *
 * Action to change the color of a group
 *
 */
bool
GroupScreen::changeColor (CompAction           *action,
			  CompAction::State    state,
			  CompOption::Vector   options)
{
    Window     xid;
    CompWindow *w;

    xid = CompOption::getIntOptionNamed (options, "window", 0);
    w   = screen->findWindow (xid);
    if (w)
    {
	GROUP_WINDOW (w);

	if (gw->mGroup)
	    gw->mGroup->changeColor ();
    }

    return false;
}

/*
 * GroupScreen::setIgnore
 *
 * Triggerable action to make this group not behave like a group
 * for a short amount of time
 *
 */
bool
GroupScreen::setIgnore (CompAction         *action,
			CompAction::State  state,
			CompOption::Vector options)
{
    mIgnoreMode = true;

    if (state & CompAction::StateInitKey)
	action->setState (state | CompAction::StateTermKey);

    return false;
}

/*
 * GroupScreen::unsetIgnore
 *
 * Triggerable action to make this group behave like a group
 *
 */
bool
GroupScreen::unsetIgnore (CompAction          *action,
			  CompAction::State   state,
			  CompOption::Vector  options)
{
    mIgnoreMode = false;

    action->setState (state & ~CompAction::StateTermKey);

    return false;
}

/*
 * GroupScreen::handleButtonPressEvent
 *
 * Generally delegated from GroupScreen::handleEvent,
 * do things like handling clicks on tab thumbnails,
 * scrolling on the tab bar etc
 *
 */
void
GroupScreen::handleButtonPressEvent (XEvent *event)
{
    GroupSelection *group;
    int            xRoot, yRoot, button;

    xRoot  = event->xbutton.x_root;
    yRoot  = event->xbutton.y_root;
    button = event->xbutton.button;

    foreach (group, mGroups)
    {
	if (!group->mTabBar)
	    continue;

	/* if we didn't click on a tab bar, we don't care*/
	if (group->mTabBar->mInputPrevention != event->xbutton.window)
	    continue;

	switch (button) {
	/* Left mouse button on the tab bar, did we click on a slot? */
	case Button1:
	    {
		GroupTabBarSlot *slot;

		foreach (slot, group->mTabBar->mSlots)
		{
		    if (slot->mRegion.contains (CompPoint (xRoot,
							   yRoot)))
		    {
			/* Set the draggedSlot to this one,
			 * don't select the tab yet, we are supposed to
			 * do that in ::handleButtonReleaseEvent */
			mDraggedSlot = slot;
			/* The slot isn't dragged yet */
			mDragged = false;
			mPrevX = xRoot;
			mPrevY = yRoot;

			if (!screen->otherGrabExist ("group",
						     "group-drag",
						     NULL))
			    grabScreen (ScreenGrabTabDrag);
		    }
		}
	    }
	    break;
	/* Scroll up or down on the bar */
	case Button4:
	case Button5:
	    {
		CompWindow  *topTab = NULL;
		GroupWindow *gw;

		/* If a tab animation is already in progress,
		 * mTopTab will not be set to the top tab we have
		 * already switched to, instead it will be the new
		 * mTopTab when the animation finishes. Obviously, we
		 * don't want to just trigger the same animation again
		 * for another scroll, so set the "topTab" in this case
		 * to the next one after the animation finishes (so we
		 * change to the one before or after that one)
		 */

		if (group->mTabBar->mNextTopTab)
		    topTab = NEXT_TOP_TAB (group);
		else if (group->mTabBar->mTopTab)
		{
		    /* If there are no tabbing animations,
		       topTab is never NULL. */
		    topTab = TOP_TAB (group);
		}

		if (!topTab)
		    return;

		gw = GroupWindow::get (topTab);

		/* Change tab left */
		if (button == Button4)
		{
		    GroupTabBarSlot *prev = gw->mSlot->mPrev;
		    if (prev)
			changeTab (prev, GroupTabBar::RotateLeft);
		    else
			changeTab (gw->mGroup->mTabBar->mSlots.back (),
					       GroupTabBar::RotateLeft);
		}
		/* Change tab right */
		else
		{
		    GroupTabBarSlot *next = gw->mSlot->mNext;
		    if (next)
			changeTab (next, GroupTabBar::RotateRight);
		    else
			changeTab (gw->mGroup->mTabBar->mSlots.front (),
					      GroupTabBar::RotateRight);
		}
		break;
	    }
	}

	break;
    }
}

/*
 * GroupScreen::handleButtonReleaseEvent
 *
 * Delegated from ::handleEvent, this handles any button release
 * event on the tab bar, so we need to "deposit" dragged slots
 * as well as changing tabs
 *
 */
void
GroupScreen::handleButtonReleaseEvent (XEvent *event)
{
    GroupSelection *group;
    int            vx, vy;
    CompRegion     newRegion;
    bool           inserted = false;
    bool           wasInTabBar = false;

    if (event->xbutton.button != 1)
	return;

    /* If there is no dragged slot, return
     * Note that this doesn't necessarily mean that
     * we are not //dragging// a slot (since a slot gets picked
     * as mDraggedSlot as soon as it is clicked, not dragged
     */
    if (!mDraggedSlot)
	return;

    /* If we were not dragged, then we were simply just selecting
     * this tab! Just change to this tab */
    if (!mDragged)
    {
	changeTab (mDraggedSlot, GroupTabBar::RotateUncertain);
	mDraggedSlot = NULL;

	if (mGrabState == ScreenGrabTabDrag)
	    grabScreen (ScreenGrabNone);

	return;
    }

    GROUP_WINDOW (mDraggedSlot->mWindow);

    newRegion = mDraggedSlot->mRegion;

    /* newRegion is the region which we are dragging the tab into,
     * which has draw offset corrections applied */

    mDraggedSlot->getDrawOffset (vx, vy);
    newRegion.translate (vx, vy);

    foreach (group, mGroups)
    {
	bool            inTabBar;
	CompRegion      clip, buf;
	GroupTabBarSlot *slot;

	if (!group->mTabBar || !HAS_TOP_WIN (group))
	    continue;

	/* create clipping region */
	clip = GroupWindow::get (TOP_TAB (group))->getClippingRegion ();

	/* if our tab region doesn't intersect the tabbar at all,
	 * then we aren't in the tab bar, so just check the next group */
	buf = newRegion.intersected (group->mTabBar->mRegion);
	buf = buf.subtracted (clip);

	inTabBar = !buf.isEmpty ();

	if (!inTabBar)
	    continue;

	/* wasInTabBar has a higher scope than here - if it is false
	 * then the window is removed from the parent group */

	wasInTabBar = true;

	foreach (slot, group->mTabBar->mSlots)
	{
	    GroupTabBarSlot *tmpDraggedSlot;
	    GroupSelection  *tmpGroup;
	    CompRegion      slotRegion;
	    CompRect	    rect;
	    bool            inSlot;

	    if (slot == mDraggedSlot)
		continue;

	    /* Construct a rectangle of "acceptable drop area"
	     * for the tab, which is usually in the spring-created space
	     * between the two tabs which we want to drop this one
	     *
	     * It should be this one :
	     *
	     * |-------------------------------|
	     * | |-------| |        | |-------||
	     * | |  TAB  | | REGION | |  TAB  ||
	     * | |-------| |        | |-------||
	     * |-------------------------------|
	     * 			^
	     *             |---------|
	     *             | DRAGGED |
	     *             |---------|
	     * */

	    /* If there is a slot to the left, then set the leftmost
	     * point of the insertion region at the rightmost point
	     * of the left slot */
	    if (slot->mPrev && slot->mPrev != mDraggedSlot)
	    {
		rect.setX (slot->mPrev->mRegion.boundingRect ().x2 ());
	    }
	    /* Otherwise if the dragged slot is the previous slot to
	     * this one, then set the leftmost insertion point to
	     * the previous slot to the dragged one (this is so that
	     * the leftmost point isn't set to the rightmost point
	     * on the dragged slot, which obviously results in a
	     * zero-width region) */
	    else if (slot->mPrev && slot->mPrev == mDraggedSlot &&
		     mDraggedSlot->mPrev)
	    {
		rect.setX (mDraggedSlot->mPrev->mRegion.boundingRect ().x2 ());
	    }
	    /* Otherwise, this is the left edge of the tab bar, so set
	     * the leftmost point to the leftmost point of the tab bar
	     * region */
	    else
		rect.setX (group->mTabBar->mRegion.boundingRect ().x1 ());

	    /* The Y point of the insertion region is obviously the top
	     * of the tab bar */
	    rect.setY (slot->mRegion.boundingRect ().y1 ());

	    /* If there is a slot to the right of this one, and it isn't
	     * the dragged slot, set the width of the region to the
	     * leftmost point of the right slot (minus the x point
	     * of the rect to get a relative width) */
	    if (slot->mNext && slot->mNext != mDraggedSlot)
	    {
		const CompRect &r = slot->mNext->mRegion.boundingRect ();
		rect.setWidth (r.x1 () - rect.x ());
	    }
	    /* Otherwise, if the slot to the right of this one is the
	     * dragged slot, then check the same thing on the slot
	     * to the right of this one */
	    else if (slot->mNext && slot->mNext == mDraggedSlot &&
		     mDraggedSlot->mNext)
	    {
		const CompRegion &r = mDraggedSlot->mNext->mRegion;
		rect.setWidth (r.boundingRect ().x1 () - rect.x ());
	    }
	    /* Otherwise, this is the rightmost edge of the tab bar, so
	     * set the width to the edge of the tab bar minus the
	     * x-point of insertion region */
	    else
	    {
		const CompRect &r = group->mTabBar->mRegion.boundingRect ();
		rect.setWidth (r.x2 ()); // FIXME: wrong ?
	    }

	    rect.setY (slot->mRegion.boundingRect ().y1 ()); // FIXME: redundant?
	    rect.setHeight (slot->mRegion.boundingRect ().height ());

	    slotRegion = CompRegion (rect);

	    /* We are ok to insert this slot into the tab bar at this
	     * position if the slot region (newRegion) intersects the
	     * calculated insertion region (slotRegion) */
	    inSlot = slotRegion.intersects (newRegion);

	    /* If we failed that, try the next slot, and so on */

	    if (!inSlot)
		continue;

	    tmpDraggedSlot = mDraggedSlot;

	    /* The window is now in a new group */
	    if (group != gw->mGroup)
	    {
		CompWindow     *w = mDraggedSlot->mWindow;
		GroupWindow    *gdw = GroupWindow::get (w);
		GroupSelection *tmpGroup = gw->mGroup;
		int            oldPosX = WIN_CENTER_X (w);
		int            oldPosY = WIN_CENTER_Y (w);

		/* if the dragged window is not the top tab,
		   move it onscreen */
		if (tmpGroup->mTabBar->mTopTab &&
		    !IS_TOP_TAB (w, tmpGroup))
		{
		    CompWindow *tw = TOP_TAB (tmpGroup);

		    oldPosX = WIN_CENTER_X (tw) + gw->mMainTabOffset.x ();
		    oldPosY = WIN_CENTER_Y (tw) + gw->mMainTabOffset.y ();

		    GroupWindow::get (w)->setWindowVisibility (true);
		}

		/* Change the group. */
		gdw->deleteGroupWindow ();
		gdw->addWindowToGroup (group);

		/* we saved the original center position in oldPosX/Y before -
		   now we should apply that to the new main tab offset */
		if (HAS_TOP_WIN (group))
		{
		    CompWindow *tw = TOP_TAB (group);
		    gw->mMainTabOffset.setX (oldPosX - WIN_CENTER_X (tw));
		    gw->mMainTabOffset.setY (oldPosY - WIN_CENTER_Y (tw));
		}
	    }
	    else
		group->mTabBar->unhookTabBarSlot (mDraggedSlot, true);

	    /* reset dragged state */
	    mDraggedSlot = NULL;
	    mDragged = false;
	    inserted = true;

	    /* Insert the slot before or after depending on the position */
	    if ((tmpDraggedSlot->mRegion.boundingRect ().x1 () +
		 tmpDraggedSlot->mRegion.boundingRect ().x2 () +
		 (2 * vx)) / 2 >
		 (slot->mRegion.boundingRect ().x1 () +
		  slot->mRegion.boundingRect ().x2 ()) / 2)
	    {
		group->mTabBar->insertTabBarSlotAfter (tmpDraggedSlot,
						       slot);
	    }
	    else
		group->mTabBar->insertTabBarSlotBefore (tmpDraggedSlot,
							slot);

	    group->mTabBar->damageRegion ();

	    /* Hide tab-bars. */
	    foreach (tmpGroup, mGroups)
	    {
		if (group == tmpGroup)
		    tmpGroup->tabSetVisibility (true, 0);
		else
		    tmpGroup->tabSetVisibility (false, PERMANENT);
	    }

	    break;
	}

	if (inserted)
	    break;
    }

    /* If there was no successful inseration, then remove the
     * dragged slot from it's original group */

    if (!inserted)
    {
	CompWindow     *draggedSlotWindow = mDraggedSlot->mWindow;
	GroupWindow    *gdsw = GroupWindow::get (draggedSlotWindow);
	GroupSelection *tmpGroup;

	foreach (tmpGroup, mGroups)
	    tmpGroup->tabSetVisibility (false, PERMANENT);

	mDraggedSlot = NULL;
	mDragged = false;

	if (optionGetDndUngroupWindow () && !wasInTabBar)
	{
	    gdsw->removeWindowFromGroup ();
	}
	else if (gw->mGroup && gw->mGroup->mTabBar->mTopTab)
	{
	    gw->mGroup->mTabBar->recalcTabBarPos (
	        (gw->mGroup->mTabBar->mRegion.boundingRect ().x1 () +
		 gw->mGroup->mTabBar->mRegion.boundingRect ().x2 ()) / 2,
		 gw->mGroup->mTabBar->mRegion.boundingRect ().x1 (),
		 gw->mGroup->mTabBar->mRegion.boundingRect ().x2 ());
	}

	/* to remove the painted slot */
	cScreen->damageScreen ();
    }

    if (mGrabState == ScreenGrabTabDrag)
	grabScreen (ScreenGrabNone);

    if (mDragHoverTimeoutHandle.active ())
    {
	mDragHoverTimeoutHandle.stop ();
    }
}

/*
 * GroupScreen::handleMotionEvent
 *
 * When dragging tabs, make sure that we are damaging the screen region around the tab.
 * Also, if we have "dragged" a "non-dragged" tab enough (>5px) then mark it as "dragged"
 * and make it's parent tab bar always visible
 *
 */

/* the radius to determine if it was a click or a drag */
static const unsigned short RADIUS = 5;

void
GroupScreen::handleMotionEvent (int xRoot, int yRoot)
{
    /* We are dragging a tab here */
    if (mGrabState == ScreenGrabTabDrag)
    {
	int    dx, dy;
	CompRegion &draggedRegion = mDraggedSlot->mRegion;

	dx = xRoot - mPrevX;
	dy = yRoot - mPrevY;

	/* Don't want to start dragging the slot unless we have moved
	 * the mouse more than 5px. Don't need to do this calculation
	 * either if we have already set mDragged */
	if (mDragged || abs (dx) > RADIUS || abs (dy) > RADIUS)
	{
	    CompRegion cReg;
	    int        vx, vy;
	    int        x1, x2, y1, y2;
	    mPrevX = xRoot;
	    mPrevY = yRoot;

	    /* mDragged is not marked, start dragging the slot */
	    if (!mDragged)
	    {
		GroupSelection *group;

		GROUP_WINDOW (mDraggedSlot->mWindow);

		mDragged = true;

		foreach (group, mGroups)
		    group->tabSetVisibility (true, PERMANENT);

		const CompRect &box =
			   gw->mGroup->mTabBar->mRegion.boundingRect ();
		gw->mGroup->mTabBar->recalcTabBarPos (
				      (box.x1 () + box.x2 ()) / 2,
				      box.x1 (), box.x2 ());

		checkFunctions ();
	    }

	    /* Damage slot region */
	    mDraggedSlot->getDrawOffset (vx, vy);

	    x1 = draggedRegion.boundingRect ().x1 () + vx;
	    y1 = draggedRegion.boundingRect ().y1 () + vy;
	    x2 = draggedRegion.boundingRect ().x2 () + vx;
	    y2 = draggedRegion.boundingRect ().y2 () + vy;

	    cReg = CompRegion (x1, y1,
			       x2 - x1,
			       y2 - y1);

	    cScreen->damageRegion (cReg);

	    /* Move the slot, adjust the spring */
	    mDraggedSlot->mRegion.translate (dx, dy);
	    mDraggedSlot->mSpringX =
		(mDraggedSlot->mRegion.boundingRect ().x1 () +
		 mDraggedSlot->mRegion.boundingRect ().x2 ()) / 2;

	    x1 = draggedRegion.boundingRect ().x1 () + vx;
	    y1 = draggedRegion.boundingRect ().y1 () + vy;
	    x2 = draggedRegion.boundingRect ().x2 () + vx;
	    y2 = draggedRegion.boundingRect ().y2 () + vy;

	    cReg = CompRegion (x1, y1,
			       x2 - x1,
			       y2 - y1);

	    cScreen->damageRegion (cReg);
	}
    }
    else if (mGrabState == ScreenGrabSelect)
    {
	mTmpSel.damage (xRoot, yRoot);
    }
}

/*
 * GroupWindow::windowNotify
 *
 * Function called on window events - on these events do
 * certain grouped actions, such as Shade/Unshade,
 * Minimize/Unminimize etc
 *
 */
void
GroupWindow::windowNotify (CompWindowNotify n)
{
    GROUP_SCREEN (screen);
    bool 	 visible = false;

    if (!mGroup)
	return window->windowNotify (n);

    switch (n)
    {
	/* Minimize or shade all windows if this window has just
	 * been minimized or shaded */
	case CompWindowNotifyShade:

	    mWindowState = GroupWindow::WindowShaded;
	    visible = false;

	    if (mGroup && gs->optionGetShadeAll ())
		mGroup->shadeWindows (window, true);

	    break;

	case CompWindowNotifyMinimize:

	    mWindowState = GroupWindow::WindowMinimized;
	    visible = false;

	    if (mGroup && gs->optionGetMinimizeAll ())
		mGroup->minimizeWindows (window, true);

	    break;

	/* Unminimize/Unshade */
	case CompWindowNotifyUnminimize:

	    if (gs->optionGetMinimizeAll ())
		mGroup->minimizeWindows (window, false);

	    visible = true;
	    break;

	case CompWindowNotifyUnshade:

	    if (gs->optionGetShadeAll ())
		mGroup->shadeWindows (window, false);

	    visible = true;
	    break;

	case CompWindowNotifyClose:
	    /* The window was closed. In this case delete the, if
	     * the group is not already ungrouping, then delete
	     * this window from the group */
	    /* close event */
	    if (!(mAnimateState & IS_UNGROUPING))
	    {
		/* mGroup is going to be set to NULL here, so better
		 * just return early */
		deleteGroupWindow ();
		gs->cScreen->damageScreen ();
	        return window->windowNotify (n);
	    }

	    break;

	case CompWindowNotifyRestack:
	    /* If there are other windows in this group, then raise
	     * all the windows (but ignore if this group was just
	     * restacked then) */
	    if (mGroup && !mGroup->mTabBar &&
		(mGroup != gs->mLastRestackedGroup))
	    {
		if (gs->optionGetRaiseAll ())
		    mGroup->raiseWindows (window);
	    }
	    if (window->managed () && !window->overrideRedirect ())
		gs->mLastRestackedGroup = mGroup;

	    return window->windowNotify (n);

	    break;

	default:
	    return window->windowNotify (n);
	    break;
    }

    if (visible)
	mWindowState = WindowNormal;
    else
    {
	/* Since the group is not visible, we can do some
	* tear-down, for now */
	if (mGroup->mTabBar && IS_TOP_TAB (window, mGroup))
	{
	    /* on unmap of the top tab, hide the tab bar and the
	       input prevention window */
	    mGroup->tabSetVisibility (false, PERMANENT);
	}
    }

    return window->windowNotify (n);
}

/*
 * GroupScreen::handleEvent
 *
 * Wrappable function to handle X11 Events.
 *
 * Handle group minimize, unmap, close etc.
 *
 */
void
GroupScreen::handleEvent (XEvent      *event)
{
    CompWindow *w;

    switch (event->type) {
    case MotionNotify:
	handleMotionEvent (pointerX, pointerY);
	break;

    case ButtonPress:
	handleButtonPressEvent (event);
	break;

    case ButtonRelease:
	handleButtonReleaseEvent (event);
	break;

    case MapNotify:
	w = screen->findWindow (event->xmap.window);
	if (w)
	{
	    foreach (CompWindow *cw, screen->windows ())
	    {
		if (w->id () == cw->frame ())
		{
		    /* Should not unmap frame window here */
		    //if (gw->mWindowHideInfo)
			//XUnmapWindow (screen->dpy (), cw->frame ());
		}
	    }
	}
	break;

    case ClientMessage:
	/* New active window */
	if (event->xclient.message_type == Atoms::winActive)
	{
	    w = screen->findWindow (event->xclient.window);
	    if (w)
	    {
		GROUP_WINDOW (w);

		/* If this window is not the top tab, then change
		 * the top tab to this one */
		if (gw->mGroup && gw->mGroup->mTabBar &&
		    !IS_TOP_TAB (w, gw->mGroup))
		{
		    GroupTabBar *bar = gw->mGroup->mTabBar;
		    bar->mCheckFocusAfterTabChange = true;
		    changeTab (gw->mSlot, GroupTabBar::RotateUncertain);
		}
	    }
	}
	/* The window has a new resize geometry. Note that this isn't
	 * a configureNotify as such, since for some resize modes
	 * (stretch, outline, etc) we don't send them until the window
	 * has been ungrabbed. However, we still need to know the
	 * current resize geometry of the window, so the resize plugin
	 * will tell us this */
	else if (event->xclient.message_type == mResizeNotifyAtom)
	{
	    CompWindow *w;
	    w = screen->findWindow (event->xclient.window);

	    if (!w)
		break;

	    foreach (GroupSelection *group, mGroups)
	    {
		/* Don't bother handling this window if the group isn't
		 * being resized or if this is the currently primary
		 * window */
		if (!(group->mResizeInfo &&
		      w == group->mResizeInfo->mResizedWindow))
		      continue;

		if (group)
		{
		    CompRect rect (event->xclient.data.l[0],
			      	   event->xclient.data.l[1],
			      	   event->xclient.data.l[2],
			      	   event->xclient.data.l[3]);

		    group->prepareResizeWindows (rect);
		}
	    }
	}
	break;

    default:
        /* Shape Notify events can mess up our cleared input shape,
	 * so just handle them and save/clear the shape again */
	if (event->type == screen->shapeEvent () + ShapeNotify)
	{
	    XShapeEvent *se = (XShapeEvent *) event;
	    if (se->kind == ShapeInput)
	    {
		CompWindow *w;
		w = screen->findWindow (se->window);
		if (w)
		{
		    GROUP_WINDOW (w);

		    if (gw->mWindowHideInfo)
			gw->clearWindowInputShape (gw->mWindowHideInfo);
		}
	    }
	}
	break;
    }

    screen->handleEvent (event);

    switch (event->type) {
    /* Window got a new name, update the tab bar text */
    case PropertyNotify:
	if (event->xproperty.atom == Atoms::wmName)
	{
	    CompWindow *w;
	    w = screen->findWindow (event->xproperty.window);
	    if (w)
	    {
		GROUP_WINDOW (w);

		if (gw->mGroup && gw->mGroup->mTabBar &&
		    gw->mGroup->mTabBar->mTextSlot    &&
		    gw->mGroup->mTabBar->mTextSlot->mWindow == w)
		{
		    /* make sure we are using the updated name */
		    TextLayer *l = gw->mGroup->mTabBar->mTextLayer;
		    gw->mGroup->mTabBar->mTextLayer =
		        TextLayer::rebuild (l);

		    if (gw->mGroup->mTabBar->mTextLayer)
			gw->mGroup->mTabBar->mTextLayer->render ();
		    gw->mGroup->mTabBar->damageRegion ();
		}
	    }
	}
	break;

    case EnterNotify:
	{
	    CompWindow *w;
	    w = screen->findWindow (event->xcrossing.window);

	    /* Whenever we enter a new window, we must update the screen
	     * to only show relevant tab bars */
	    updateTabBars (event->xcrossing.window);

	    if (w)
	    {
		GROUP_WINDOW (w);

		if (mShowDelayTimeoutHandle.active ())
		    mShowDelayTimeoutHandle.stop ();

		if (gw->mGroup)
		{
		    /* If we are dragging a slot over this window, then
		     * set a timer to raise the window after a certain
		     * amount of time */
		    if (mDraggedSlot && mDragged &&
			IS_TOP_TAB (w, gw->mGroup))
		    {
			int hoverTime;
			hoverTime = optionGetDragHoverTime () * 1000;
			if (mDragHoverTimeoutHandle.active ())
			    mDragHoverTimeoutHandle.stop ();

			if (hoverTime > 0)
			{
			    mDragHoverTimeoutHandle.setCallback (
				boost::bind (
				    &GroupWindow::dragHoverTimeout,
				    gw));
			    mDragHoverTimeoutHandle.setTimes (hoverTime,
						       hoverTime * 1.2);
			    mDragHoverTimeoutHandle.start ();
			}
		    }
		}
	    }
	}
	break;

    case ConfigureNotify:
	{
	    CompWindow *w;

	    w = screen->findWindow (event->xconfigure.window);
	    if (w)
	    {
		GROUP_WINDOW (w);

		/* Window with tab bar restacked, restack the IPW */
		if (gw->mGroup && gw->mGroup->mTabBar &&
		    IS_TOP_TAB (w, gw->mGroup)      &&
		    gw->mGroup->mTabBar->mInputPrevention &&
		    gw->mGroup->mTabBar->mIpwMapped)
		{
		    XWindowChanges xwc;

		    xwc.stack_mode = Above;
		    xwc.sibling = w->id ();

		    XConfigureWindow (screen->dpy (),
				      gw->mGroup->mTabBar->mInputPrevention,
				      CWSibling | CWStackMode, &xwc);
		}
	    }
	}
	break;

    default:
	break;
    }
}

/*
 * GroupWindow::resizeNotify
 *
 * Description: We get resizeNotify once the window is actually
 * configured (note the difference between the COMPIZ_RESIZE_NOTIFY)
 * ClientMessage above.
 *
 * If this window had a resize geometry, set it to zero-size. If
 * the window had glow quads, we need to recalculate them. We also
 * need to recalculate the tab bar position.
 *
 * Note that we don't handle grouped resize here, since that would
 * mean that we have to handle queuing (since the function is wrapped).
 * Instead we handle that in ConfigureNotify in
 * GroupScreen::handleEvent above.
 *
 */
void
GroupWindow::resizeNotify (int dx,
			   int dy,
			   int dwidth,
			   int dheight)
{
    GROUP_SCREEN (screen);

    if (!mResizeGeometry.isEmpty ())
    {
	mResizeGeometry = CompRect (0, 0, 0, 0);
    }

    window->resizeNotify (dx, dy, dwidth, dheight);

    if (mGlowQuads)
    {
	/* FIXME: we need to find a more multitexture friendly way
	 * of doing this */
	GLTexture::Matrix tMat = gs->mGlowTexture.at (0)->matrix ();
	computeGlowQuads (&tMat);
    }

    /* If this was the top tab, then re-center the tab bar */
    if (mGroup && mGroup->mTabBar && IS_TOP_TAB (window, mGroup))
    {
	if (mGroup->mTabBar->mState != PaintOff)
	{
	    mGroup->mTabBar->recalcTabBarPos (pointerX,
				  WIN_X (window),
				  WIN_X (window) + WIN_WIDTH (window));
	}
    }
}

/*
 * GroupWindow::moveNotify
 *
 * We need to recalculate the glow quads here (since the position
 * changed, and they are contained in the quads). We also need to
 * update the tab bar positon and move windows in the group (queued).
 *
 */
void
GroupWindow::moveNotify (int    dx,
			 int    dy,
			 bool   immediate)
{
    bool       viewportChange;

    GROUP_SCREEN (screen);

    window->moveNotify (dx, dy, immediate);

    /* mGlowQuads contains positional info, so we need to recalc that */
    if (mGlowQuads)
    {
	/* FIXME: we need to find a more multitexture friendly way
	 * of doing this */
	GLTexture::Matrix tMat = gs->mGlowTexture.at (0)->matrix ();
	computeGlowQuads (&tMat);
    }

    /* Don't bother if the window is not grouped */
    if (!mGroup || gs->mQueued)
	return;

    /* FIXME: we need a reliable, 100% safe way to detect window
       moves caused by viewport changes here */
    viewportChange = ((dx && !(dx % screen->width ())) ||
		      (dy && !(dy % screen->height ())));

    /* If the viewport changes and the tab bar is tabbing or untabbing
     * then add the viewport change distance to the animation (since
     * we are overriding where the window is painted) */
    if (viewportChange && (mAnimateState & IS_ANIMATED))
    {
	mDestination += CompPoint (dx, dy);
    }

    /* If we are moving a window with a tab bar, then make the tab
     * bar springy and move it too */
    if (mGroup->mTabBar && IS_TOP_TAB (window, mGroup))
    {
	GroupTabBarSlot *slot;
	GroupTabBar     *bar = mGroup->mTabBar;

	bar->mRightSpringX += dx;
	bar->mLeftSpringX += dx;

	bar->moveTabBarRegion (dx, dy, true);

	foreach (slot, bar->mSlots)
	{
	    slot->mRegion.translate (dx, dy);
	    slot->mSpringX += dx;
	}
    }

    /* Don't bother moving all group windows if we are not moving all
     * ignoring group actions, or if we are currently in a tabbing
     * animation or if the grabbed window isn't this one (can be caused
     * by moveNotifies when moving other group windows)
     * or if we aren't grabbed this window for a move */
    if (!gs->optionGetMoveAll () || gs->mIgnoreMode ||
	(mGroup->mTabbingState != GroupSelection::NoTabbing) ||
	(mGroup->mGrabWindow != window->id ()) ||
	!(mGroup->mGrabMask & CompWindowGrabMoveMask))
    {
	return;
    }

    mGroup->moveWindows (window, dx, dy, immediate, viewportChange);
}

/*
 * GroupWindow::grabNotify
 *
 * Description: when the window is grabbed, it might be grabbed
 * for a resize, in which case we need to set up the resize-all bits
 * (including resize geometry)
 */

void
GroupWindow::grabNotify (int          x,
			 int	      y,
			 unsigned int state,
			 unsigned int mask)
{
    GROUP_SCREEN (screen);

    gs->mLastGrabbedWindow = window->id ();

    if (mGroup && !gs->mIgnoreMode && !gs->mQueued)
    {
	bool doResizeAll;

	doResizeAll = gs->optionGetResizeAll () &&
	              (mask & CompWindowGrabResizeMask);

	/* Make the tab bar invisible for now */
	if (mGroup->mTabBar)
	    mGroup->tabSetVisibility (false, 0);

	foreach (CompWindow *cw, mGroup->mWindows)
	{
	    if (!cw)
		continue;

	    if (cw->id () != window->id ())
	    {
		GroupWindow *gcw = GroupWindow::get (cw);

		/* Enqueue a grabNotify (since this function is
		 * wrapped) */
		gcw->enqueueGrabNotify (x, y, state, mask);

		if (doResizeAll && !(cw->state () & MAXIMIZE_STATE))
		{
		    if (gcw->mResizeGeometry.isEmpty ())
		    {
			gcw->mResizeGeometry =
					CompRect (WIN_X (cw),
						  WIN_Y (cw),
						  WIN_WIDTH (cw),
						  WIN_HEIGHT (cw));
			gcw->checkFunctions ();
		    }
		}
	    }
	}

	/* Set up a new master resize info with this window as the
	 * master window and the current geometry as the master geometry
	 */
	if (doResizeAll)
	{
	    if (!mGroup->mResizeInfo)
		mGroup->mResizeInfo = new GroupSelection::ResizeInfo;

	    if (mGroup->mResizeInfo)
	    {
		mGroup->mResizeInfo->mResizedWindow       = window;
		mGroup->mResizeInfo->mOrigGeometry =
					CompRect (WIN_X (window),
						  WIN_Y (window),
						  WIN_WIDTH (window),
						  WIN_HEIGHT (window));
	    }
	}

	mGroup->mGrabWindow = window->id ();
	mGroup->mGrabMask = mask;
    }

    gs->checkFunctions ();

    window->grabNotify (x, y, state, mask);
}

/*
 * GroupWindow::ungrabNotify
 *
 * When the window is ungrabbed, resize them according to our set
 * resizeGeometry (handled in mGroup->resizeWindows)
 */

void
GroupWindow::ungrabNotify ()
{
    GROUP_SCREEN (screen);

    gs->mLastGrabbedWindow = None;

    if (mGroup && !gs->mIgnoreMode && !gs->mQueued)
    {
	mGroup->resizeWindows (window); // should really include the size info here
    }

    gs->checkFunctions ();

    window->ungrabNotify ();
}

/*
 * GroupWindow::damageRect
 *
 * A request was made to damage everything in this window.
 *
 * If this window just appeared (initial) then add it to an autotabbed
 * group if we are doing that. Or it might be the case that the window
 * was just unshaded or unminimzied (initial as well) so apply
 * those actions to the group too
 *
 * Otherwise, damage the stretch rectangle for the window, damage
 * the slot (since the group was just updated)
 */

bool
GroupWindow::damageRect (bool	        initial,
			 const CompRect &rect)
{
    bool       status;

    GROUP_SCREEN (screen);

    status = cWindow->damageRect (initial, rect);

    /* Window just appeared */
    if (initial)
    {
	if ((gs->optionGetAutotabWindows ().size () ||
	     gs->optionGetAutotabCreate ()) &&
	     !mGroup && mWindowState == WindowNormal)
	{
	    GroupSelection *g = NULL;
	    /* First check if this window should be added to an
	     * existing group */

	    foreach (CompOption::Value &v, gs->optionGetAutotabWindows ())
	    {
		if (v.match ().evaluate (window))
		{
		    bool foundGroup = false;
		    foreach (GroupSelection *lg, gs->mGroups)
		    {
			foreach (CompWindow *w, lg->mWindows)
			{
			    if (v.match ().evaluate (w))
			    {
				foundGroup = true;
				g = lg;
				break;
			    }
			}

			if (foundGroup)
			    break;
		    }

		    if (foundGroup)
			break;
		}
	    }

	    if (!g)
	    {
		gs->mTmpSel.clear ();
		gs->mTmpSel.select (window);
		g = gs->mTmpSel.toGroup ();
	    }
	    else
	    {
		addWindowToGroup (g);
	    }

	    /* If 'g' is NULL here then a new group will be created
	     * so better use mGroup here instead */
	    if (g)
		g->tabGroup (window);
	}

	checkFunctions (); // we don't need damageRect after this
    }

    /* Damage resize rectangle */
    if (!mResizeGeometry.isEmpty ())
    {
	CompRect box;
	float  dummy = 1;

	getStretchRectangle (box, dummy, dummy);
	gs->damagePaintRectangle (box);
    }

    /* Damage slot */
    if (mSlot)
    {
	int    vx, vy;
	CompRegion reg;

	mSlot->getDrawOffset (vx, vy);
	if (vx || vy)
	{
	    reg = reg.united (mSlot->mRegion);
	    reg.translate (vx, vy);
	}
	else
	    reg = mSlot->mRegion;

	gs->cScreen->damageRegion (reg);
    }

    return status;
}

/*
 * GroupWindow::stateChangeNotify
 *
 * A change to the window state was detected (could be maximization)
 * so if it was, then apply that to the group (if applicable)
 */

void
GroupWindow::stateChangeNotify (unsigned int lastState)
{
    GROUP_SCREEN (screen);

    if (mGroup && !gs->mIgnoreMode)
    {
	if (((lastState & MAXIMIZE_STATE) != (window->state () & MAXIMIZE_STATE)) &&
	    gs->optionGetMaximizeUnmaximizeAll ())
	{
	    mGroup->maximizeWindows (window);
	}
    }

    window->stateChangeNotify (lastState);
}

/*
 * GroupWindow::activate
 *
 * Window got activated, change the tab to the new active window
 */
void
GroupWindow::activate ()
{
    GROUP_SCREEN (screen);

    if (mGroup && mGroup->mTabBar && !IS_TOP_TAB (window, mGroup))
	gs->changeTab (mSlot, GroupTabBar::RotateUncertain);

    window->activate ();
}
