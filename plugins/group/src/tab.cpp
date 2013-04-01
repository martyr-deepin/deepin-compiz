/**
 *
 * Compiz group plugin
 *
 * tab.cpp
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
 * getCurrentMousePosition
 *
 * Description:
 * Return the current function of the pointer at the given screen.
 * The position is queried trough XQueryPointer directly from the xserver.
 *
 * TODO: Depricate this function and use mousepoll directly
 *
 */
bool
GroupScreen::getCurrentMousePosition (int &x, int &y)
{
    MousePoller poller;
    CompPoint pos = poller.getCurrentPosition ();

    x = pos.x ();
    y = pos.y ();

    return (x != 0 && y != 0);
}

/*
 * GroupWindow::getClippingRegion
 *
 * Description:
 * This function returns a clipping region which is used to clip
 * several events involving window stack such as hover detection
 * in the tab bar or Drag'n'Drop. It creates the clipping region
 * with getting the region of every window above the given window
 * and then adds this region to the clipping region using
 * CompRegion::united (). w->region won't work since it doesn't include
 * the window decoration.
 *
 */
CompRegion
GroupWindow::getClippingRegion ()
{
    CompWindow *cw;
    CompRegion     clip;

    for (cw = window->next; cw; cw = cw->next)
    {
	/* Ignore small invidible windows or hidden ones in our clipping
	 * region */
	if (!cw->invisible () &&
	    !(cw->state () & CompWindowStateHiddenMask))
	{
	    CompRect rect;
	    CompRegion     buf;

	    rect = CompRect (WIN_REAL_X (cw),
			     WIN_REAL_Y (cw),
			     WIN_REAL_WIDTH (cw),
			     WIN_REAL_HEIGHT (cw));

	    buf = buf.united (rect);
	    clip = buf.united (clip);
	}
    }

    return clip;
}


/*
 * GroupWindow::clearWindowInputShape
 *
 * We are not painting this window, so we don't want to interact with
 * it using X11. We can use XShape to read the input shape rects and
 * save the. We then combine the shape rects with NULL, so that the
 * window has no input shape (so it cannot recieve input, all clicks
 * fall through).
 *
 */
void
GroupWindow::clearWindowInputShape (GroupWindow::HideInfo *hideInfo)
{
    XRectangle  *rects;
    int         count = 0, ordering;

    /* Get the shape rects */
    rects = XShapeGetRectangles (screen->dpy (), window->id (),
				 ShapeInput, &count, &ordering);

    if (count == 0)
	return;

    /* check if the returned shape exactly matches the window shape -
     * if that is true, the window currently has no set input shape
     * (so the shape will be the default rectangular shape) */
    if ((count == 1) &&
	(rects[0].x == -window->serverGeometry ().border ()) &&
	(rects[0].y == -window->serverGeometry ().border ()) &&
	(rects[0].width == (window->serverWidth () +
			    window->serverGeometry ().border ())) &&
	(rects[0].height == (window->serverHeight () +
			     window->serverGeometry ().border ())))
    {
	count = 0;
    }

    /* if this window already has saved input rects in the plugin, get
     * rid of them. now */
    if (hideInfo->mInputRects)
	XFree (hideInfo->mInputRects);

    /* Save input shape */
    hideInfo->mInputRects = rects;
    hideInfo->mNInputRects = count;
    hideInfo->mInputRectOrdering = ordering;

    /* Don't get XShape events (we are currently registered for them,
     * if we get them then we will recursively clear the input shape */
    XShapeSelectInput (screen->dpy (), hideInfo->mShapeWindow, NoEventMask);

    /* In ShapeInput, combine NULL with the input shape */
    XShapeCombineRectangles (screen->dpy (), hideInfo->mShapeWindow,
			     ShapeInput, 0, 0, NULL, 0, ShapeSet, 0);

    /* Get XShape events again */
    XShapeSelectInput (screen->dpy (), hideInfo->mShapeWindow,
		       ShapeNotify);
}

/*
 * GroupWindow::setWindowVisibility
 *
 * Set the window to be painted or not (visible) and clear and restore
 * the input shape as necessary. Set the window to skip the taskbar
 * and pager as necessary.
 *
 */
void
GroupWindow::setWindowVisibility (bool visible)
{
    if (!visible && !mWindowHideInfo)
    {
	GroupWindow::HideInfo *info;

	/* Allocate a hide info structure */
	mWindowHideInfo = info = new GroupWindow::HideInfo;
	if (!mWindowHideInfo)
	    return;

	/* ensure the input rects are NULL and get the shape mask */
	info->mInputRects = NULL;
	info->mNInputRects = 0;
	info->mShapeMask = XShapeInputSelected (screen->dpy (),
						window->id ());

	/* We are a reparenting window manager now,
	 * which means that we either shape the frame window,
	 * or if it does not exist, shape the window
	 */
	if (window->frame ())
	{
	    info->mShapeWindow = window->frame ();
	}
	else
	    info->mShapeWindow = window->id ();

	clearWindowInputShape (info);

	/* Do not allow the window to be displated on the taskbar or
	 * pager */
	info->mSkipState =
		window->state () & (CompWindowStateSkipPagerMask |
				    CompWindowStateSkipTaskbarMask);

	window->changeState (window->state () |
			     CompWindowStateSkipPagerMask |
			     CompWindowStateSkipTaskbarMask);
    }
    else if (visible && mWindowHideInfo)
    {
	GroupWindow::HideInfo *info = mWindowHideInfo;

	/* If there are saved input rects, restore them */
	if (info->mNInputRects)
	{
	    XShapeCombineRectangles (screen->dpy (), info->mShapeWindow,
				     ShapeInput, 0, 0,
				     info->mInputRects,
				     info->mNInputRects, ShapeSet,
				     info->mInputRectOrdering);
	}
	/* Otherwise combine with a null mask (automatically restores
	 * single rect input shape) */
	else
	{
	    XShapeCombineMask (screen->dpy (), info->mShapeWindow,
			       ShapeInput, 0, 0, None, ShapeSet);
	}

	if (info->mInputRects)
	    XFree (info->mInputRects);

	/* Recieve shape events again */
	XShapeSelectInput (screen->dpy (),
			   info->mShapeWindow, info->mShapeMask);

	/* Put the window back on the taskbar and pager */
	window->changeState ((window->state () &
				 ~(CompWindowStateSkipPagerMask |
				   CompWindowStateSkipTaskbarMask)) |
			      info->mSkipState);

	delete info;
	mWindowHideInfo = NULL;
    }
}

/*
 * GroupSelection::tabBarTimeout
 *
 * Description:
 * This function is called when the time expired (== timeout).
 * We use this to realize a delay with the bar hiding after tab change.
 * handleAnimation sets up a timer after the animation has finished.
 * This function itself basically just sets the tab bar to a PaintOff status
 * through calling groupSetTabBarVisibility.
 * The PERMANENT mask allows you to force hiding even of
 * PaintPermanentOn tab bars.
 *
 */
bool
GroupSelection::tabBarTimeout ()
{
    tabSetVisibility (false, PERMANENT);

    return false;	/* This will free the timer. */
}

/*
 * GroupSelection::showDelayTimeout
 *
 * Show the tab bar at the mouse position after a timeout
 *
 */
bool
GroupSelection::showDelayTimeout ()
{
    int            mouseX, mouseY;
    CompWindow     *topTab;

    GROUP_SCREEN (screen);

    /* If this isn't the top window of the group, then don't bother
     * to show the tab bar (it really shouldn't be shown in this case
     * but it could be a corner case where the window shape is restored
     * and we hover over it */
    if (!HAS_TOP_WIN (this))
    {
	gs->mShowDelayTimeoutHandle.stop ();
	return false;	/* This will free the timer. */
    }

    topTab = TOP_TAB (this);

    GROUP_WINDOW (topTab);

    gs->getCurrentMousePosition (mouseX, mouseY);

    /* Recalc tab bar pos based on mouse pos */
    mTabBar->recalcTabBarPos (mouseX, WIN_REAL_X (topTab),
			  WIN_REAL_X (topTab) + WIN_REAL_WIDTH (topTab));

    tabSetVisibility (true, 0);

    gw->checkFunctions ();
    gs->checkFunctions ();

    gs->mShowDelayTimeoutHandle.stop ();
    return false;	/* This will free the timer. */
}

/*
 * GroupSelection::tabSetVisibility
 *
 * Description:
 * This function is used to set the visibility of the tab bar.
 * The "visibility" is indicated through the PaintState, which
 * can be PaintOn, PaintOff, PaintFadeIn, PaintFadeOut
 * and PaintPermantOn.
 * Currently the mask paramater is mostely used for the PERMANENT mask.
 * This mask affects how the visible parameter is handled, for example if
 * visibule is set to true and the mask to PERMANENT state it will set
 * PaintPermanentOn state for the tab bar. When visibile is false, mask 0
 * and the current state of the tab bar is PaintPermanentOn it won't do
 * anything because its not strong enough to disable a
 * Permanent-State, for those you need the mask.
 *
 * Determine the cases where we want to show the tab bar. Set up the
 * render and the animation.
 *
 */
void
GroupSelection::tabSetVisibility (bool           visible,
				  unsigned int   mask)
{
    GroupTabBar *bar;
    CompWindow  *topTab;
    PaintState  oldState;

    GROUP_SCREEN (screen);

    /* Don't bother if this isn't the top window or there is no tab bar
     */
    if (!mWindows.size () || !mTabBar || !HAS_TOP_WIN (this))
	return;

    bar = mTabBar;
    topTab = TOP_TAB (this);
    oldState = bar->mState;

    /* Start polling the mouse inside the tab bar */
    if (visible)
	mPoller.start ();
    else
	mPoller.stop ();

    /* hide tab bars for invisible top windows */
    if ((topTab->state () & CompWindowStateHiddenMask) ||
	 topTab->invisible ())
    {
	bar->mState = PaintOff;
	gs->switchTopTabInput (this, true);
    }
    /* Make the tab bar painted permanently in the case that we specify
     * a permanent mask */
    else if (visible && bar->mState != PaintPermanentOn &&
	     (mask & PERMANENT))
    {
	bar->mState = PaintPermanentOn;
	gs->switchTopTabInput (this, false);
    }
    /* If there is no longer an need to paint permanently, then just
     * paint this normally */
    else if (visible && bar->mState == PaintPermanentOn &&
	     !(mask & PERMANENT))
    {
	bar->mState = PaintOn;
    }
    /* If we're visible and need to fade in or out, re-render the tab
     * bar animations */
    else if (visible && (bar->mState == PaintOff ||
			 bar->mState == PaintFadeOut))
    {
	/* Set up the tab bar animations */
	if (gs->optionGetBarAnimations () && bar->mBgLayer)
	{
	    bar->mBgLayer->mBgAnimation = BackgroundLayer::AnimationReflex;
	    bar->mBgLayer->mBgAnimationTime = gs->optionGetReflexTime () * 1000.0;
	}
	bar->mState = PaintFadeIn;
	gs->switchTopTabInput (this, false);
    }
    /* Otherwise if we're not visible, and haven't used the PERMANENT
     * strength mask for permenancy, then fade out */
    else if (!visible &&
	     (bar->mState != PaintPermanentOn || (mask & PERMANENT)) &&
	     (bar->mState == PaintOn || bar->mState == PaintPermanentOn ||
	      bar->mState == PaintFadeIn))
    {
	bar->mState = PaintFadeOut;
	gs->switchTopTabInput (this, true);
    }

    /* If we're fading in or out set up the animation and damage */
    if (bar->mState == PaintFadeIn || bar->mState == PaintFadeOut)
	bar->mAnimationTime = (gs->optionGetFadeTime () * 1000) - bar->mAnimationTime;

    if (bar->mState != oldState)
	bar->damageRegion ();
}

/*
 * GroupTabBarSlot::getDrawOffset ()
 *
 * Description:
 * Its used when the draggedSlot is dragged to another viewport.
 * It calculates a correct offset to the real slot position.
 *
 */
void
GroupTabBarSlot::getDrawOffset (int &hoffset,
				int &voffset)
{
    CompWindow *w, *topTab;
    int        x, y, vx, vy;
    CompPoint  vp;
    CompWindow::Geometry winGeometry;

    if (!mWindow)
	return;

    w = mWindow;

    GROUP_WINDOW (w);
    GROUP_SCREEN (screen);

    /* If this isn't the dragged slot, don't bother */
    if (this != gs->mDraggedSlot || !gw->mGroup)
    {
	hoffset = 0;
	voffset = 0;

	return;
    }

    /* Figure out the top tab */
    if (HAS_TOP_WIN (gw->mGroup))
	topTab = TOP_TAB (gw->mGroup);
    else if (HAS_PREV_TOP_WIN (gw->mGroup))
	topTab = PREV_TOP_TAB (gw->mGroup);
    else
    {
	hoffset = 0;
	voffset = 0;
	return;
    }

    x = WIN_CENTER_X (topTab) - WIN_WIDTH (w) / 2;
    y = WIN_CENTER_Y (topTab) - WIN_HEIGHT (w) / 2;

    /* Determine the viewport offset */
    winGeometry = CompWindow::Geometry (x, y, w->serverWidth (),
					      w->serverHeight (),
					w->serverGeometry ().border ());

    screen->viewportForGeometry (winGeometry, vp);

    vx = vp.x ();
    vy = vp.y ();

    /* the offset is the distance from the left edge of the initial viewport */
    hoffset = ((screen->vp ().x () - vx) % screen->vpSize ().width ())
	      * screen->width ();

    voffset = ((screen->vp ().y () - vy) % screen->vpSize ().height ())
	      * screen->height ();
}

/*
 * GroupSelection::handleHoverDetection
 *
 * Description:
 * This function is called on the mousepoll update to handle whether a
 * new tab has been hovered on the bar (and then it updates the text
 * accordingly)
 */
void
GroupSelection::handleHoverDetection (const CompPoint &p)
{
    GroupTabBar *bar = mTabBar;
    CompWindow  *topTab = TOP_TAB (this);
    bool        inLastSlot;

    GROUP_SCREEN (screen);

    if ((bar->mState != PaintOff) && !HAS_TOP_WIN (this))
	return;

    /* then check if the mouse is in the last hovered slot --
       this saves a lot of CPU usage */
    inLastSlot = bar->mHoveredSlot &&
		 bar->mHoveredSlot->mRegion.contains (p);

    if (!inLastSlot)
    {
	CompRegion      clip;
	GroupTabBarSlot *slot;

	bar->mHoveredSlot = NULL;
	clip = GroupWindow::get (topTab)->getClippingRegion ();

	foreach (slot, bar->mSlots)
	{
	    /* We need to clip the slot region with the clip region first.
	       This is needed to respect the window stack, so if a window
	       covers a port of that slot, this part won't be used
	       for in-slot-detection. */
	    CompRegion reg = slot->mRegion.subtracted (clip);

	    if (reg.contains (p))
	    {
		bar->mHoveredSlot = slot;
		break;
	    }
	}

	if (bar->mTextLayer)
	{
	    /* trigger a FadeOut of the text */
	    if ((bar->mHoveredSlot != bar->mTextSlot) &&
		(bar->mTextLayer->mState == PaintFadeIn ||
		 bar->mTextLayer->mState == PaintOn))
	    {
		bar->mTextLayer->mAnimationTime =
		    (gs->optionGetFadeTextTime () * 1000) -
		    bar->mTextLayer->mAnimationTime;
		bar->mTextLayer->mState = PaintFadeOut;
	    }

	    /* or trigger a FadeIn of the text */
	    else if ((bar->mTextLayer->mState == PaintFadeOut  ||
		      bar->mTextLayer->mState == PaintOff) &&
		      bar->mHoveredSlot == bar->mTextSlot && bar->mHoveredSlot)
	    {
		bar->mTextLayer->mAnimationTime =
		    (gs->optionGetFadeTextTime () * 1000) -
		    bar->mTextLayer->mAnimationTime;
		bar->mTextLayer->mState = PaintFadeIn;
	    }

	    bar->damageRegion ();
	    GroupWindow::get (topTab)->checkFunctions ();
	}

	gs->checkFunctions ();
    }

    return;
}

/*
 * GroupTabBar::handleTabBarFade
 *
 * Description:
 * This function is called from preparePaint
 * to handle the tab bar fade. It checks the animationTime and updates it,
 * so we can calculate the alpha of the tab bar in the painting code with it.
 *
 * Returns true if there is animation still remaining
 *
 */
bool
GroupTabBar::handleTabBarFade (int msSinceLastPaint)
{
    mAnimationTime -= msSinceLastPaint;

    if (mAnimationTime > 0)
	return true;
    else
	mAnimationTime = 0;

    /* Fade finished */
    if (mAnimationTime == 0)
    {
	if (mState == PaintFadeIn)
	{
	    mState = PaintOn;
	}
	else if (mState == PaintFadeOut)
	{
	    mState = PaintOff;

	    if (mTextLayer)
	    {
		/* Tab-bar is no longer painted, clean up
		   text animation variables. */
		mTextLayer->mAnimationTime = 0;
		mTextLayer->mState = PaintOff;
		mTextSlot = mHoveredSlot = NULL;

		mTextLayer = TextLayer::rebuild (mTextLayer);

		if (mTextLayer)
		    mTextLayer->render ();
	    }
	}
    }

    return false;
}

/*
 * GroupTabBar::handleTextFade
 *
 * Description:
 * This function is called from groupPreparePaintScreen
 * to handle the text fade. It checks the animationTime and updates it,
 * so we can calculate the alpha of the text in the painting code with it.
 *
 * Returns true if there is still animation remaining
 *
 */
bool
GroupTabBar::handleTextFade (int	       msSinceLastPaint)
{
    TextLayer *textLayer = mTextLayer;
    bool		   continuePainting = false;

    /* Fade in progress... */
    if ((textLayer->mState == PaintFadeIn ||
	 textLayer->mState == PaintFadeOut) &&
	textLayer->mAnimationTime > 0)
    {
	textLayer->mAnimationTime -= msSinceLastPaint;

	if (textLayer->mAnimationTime < 0)
	    textLayer->mAnimationTime = 0;

	/* Fade has finished. */
	if (textLayer->mAnimationTime == 0)
	{
	    if (textLayer->mState == PaintFadeIn)
		textLayer->mState = PaintOn;

	    else if (textLayer->mState == PaintFadeOut)
		textLayer->mState = PaintOff;
	}
	else
	    continuePainting = true;
    }

    if (textLayer->mState == PaintOff && mHoveredSlot &&
	mHoveredSlot != mTextSlot)
    {
	/* Start text animation for the new hovered slot. */
	mTextSlot = mHoveredSlot;
	textLayer->mState = PaintFadeIn;
	textLayer->mAnimationTime =
	    (GroupScreen::get (screen)->optionGetFadeTextTime () * 1000);

	mTextLayer = textLayer = TextLayer::rebuild (textLayer);

	if (textLayer)
	    mTextLayer->render ();

	continuePainting = true;
    }

    else if (textLayer->mState == PaintOff && mTextSlot)
    {
	/* Clean Up. */
	mTextSlot = NULL;
	mTextLayer = textLayer = TextLayer::rebuild (textLayer);

	if (textLayer)
	    mTextLayer->render ();
    }

    return continuePainting;

}

/*
 * BackgroundLayer::handleAnimation
 *
 * Description: Handles the different animations for the tab bar defined in
 * BackgroundLayer::AnimationType. Basically that means this function updates
 * tabBar->animation->time as well as checking if the animation is already
 * finished.
 *
 * Returns true if more animation needed
 *
 */
bool
BackgroundLayer::handleAnimation (int            msSinceLastPaint)
{
    mBgAnimationTime -= msSinceLastPaint;

    if (mBgAnimationTime <= 0)
    {
	mBgAnimationTime = 0;
	mBgAnimation = AnimationNone;

	render ();

	return false;
    }

    return true;
}
/*
 * tabChangeActivateEvent
 *
 * Description: Creates a compiz event to let other plugins know about
 * the starting and ending point of the tab changing animation
 */
void
GroupScreen::tabChangeActivateEvent (bool activating)
{
    CompOption::Vector o;

    CompOption opt ("root", CompOption::TypeInt);
    opt.value ().set ((int) screen->root ());

    o.push_back (opt);

    CompOption opt2 ("active", CompOption::TypeBool);
    opt2.value ().set (activating);

    o.push_back (opt2);

    screen->handleCompizEvent ("group", "tabChangeActivate", o);
}

/*
 * GroupSelection::handleAnimation
 *
 * Description:
 * This function handles the change animation. It's called
 * from handleChanges. Don't let the changeState
 * confuse you, PaintFadeIn equals with the start of the
 * rotate animation and PaintFadeOut is the end of these
 * animation.
 *
 * This gets called when a window has finished rotating to 90 degrees to
 * the viewer (so it is just invisible). Handle setting up the tab
 * state and focussing the new windows and such.
 *
 */
bool
GroupSelection::handleAnimation ()
{
    bool newAnim = false;

    GROUP_SCREEN (screen);

    /* This was an outgoing tab */
    if (mTabBar->mChangeState == GroupTabBar::TabChangeOldOut)
    {
	CompWindow      *top = TOP_TAB (this);
	bool            activate;

	/* recalc here is needed (for y value)! */
	mTabBar->recalcTabBarPos (
			  mTabBar->mRegion.boundingRect ().centerX (),
			  WIN_REAL_X (top),
			  WIN_REAL_X (top) + WIN_REAL_WIDTH (top));

	/* Add time progress to the animation again. Don't
	 * have a negative value */
	mTabBar->mChangeAnimationTime +=
			      gs->optionGetChangeAnimationTime () * 500;

	if (mTabBar->mChangeAnimationTime <= 0)
	    mTabBar->mChangeAnimationTime = 0;

	/* We need to put the new tab in */
	mTabBar->mChangeState = GroupTabBar::TabChangeNewIn;

	/* Activate the new tab which just rotated in */
	activate = !mTabBar->mCheckFocusAfterTabChange;
	if (!activate)
	{
/*
	    CompFocusResult focus;
	    focus    = allowWindowFocus (top, NO_FOCUS_MASK, s->x, s->y, 0);
	    activate = focus == CompFocusAllowed;
*/
	}

	/* Activate the top tab */
	if (activate)
	    top->activate ();

	mTabBar->mCheckFocusAfterTabChange = false;
	newAnim = true;
    }

    /* This was an incoming tab (animation reversed) */
    if (mTabBar->mChangeState == GroupTabBar::TabChangeNewIn &&
	mTabBar->mChangeAnimationTime <= 0)
    {
	int oldChangeAnimationTime = mTabBar->mChangeAnimationTime;

	gs->tabChangeActivateEvent (false);

	/* Set the previous top tab visibility to false since we can't
	 * see it yet! */
	if (mTabBar->mPrevTopTab)
	    GroupWindow::get (PREV_TOP_TAB (this))->setWindowVisibility (false);

	/* Now the new previous top tab is the current one */
	GroupWindow::get (PREV_TOP_TAB (this))->checkFunctions ();
	GroupWindow::get (TOP_TAB (this))->checkFunctions ();
	mTabBar->mPrevTopTab = mTabBar->mTopTab;
	mTabBar->mChangeState = GroupTabBar::NoTabChange;

	/* If we were heading towards a new top tab, then change
	 * this next one to the new one we want to change to */
	if (mTabBar->mNextTopTab)
	{
	    GroupTabBarSlot *next = mTabBar->mNextTopTab;
	    mTabBar->mNextTopTab = NULL;

	    gs->changeTab (next, mTabBar->mNextDirection);

	    if (mTabBar->mChangeState == GroupTabBar::TabChangeOldOut)
	    {
		/* If a new animation was started. */
		mTabBar->mChangeAnimationTime += oldChangeAnimationTime;
	    }
	}

	/* Don't use a negative value */
	if (mTabBar->mChangeAnimationTime <= 0)
	{
	    mTabBar->mChangeAnimationTime = 0;
	}
	/* If we can't show the tab bar immediately, then show it now
	 * and set the fade out timeout */
	else if (gs->optionGetVisibilityTime () != 0.0f &&
		 mTabBar->mChangeState == GroupTabBar::NoTabChange)
	{
	    tabSetVisibility (true, PERMANENT |
				    SHOW_BAR_INSTANTLY_MASK);

	    if (mTabBar->mTimeoutHandle.active ())
		mTabBar->mTimeoutHandle.stop ();

	    mTabBar->mTimeoutHandle.setTimes (gs->optionGetVisibilityTime () * 1000,
					      gs->optionGetVisibilityTime () * 1200);

	    mTabBar->mTimeoutHandle.setCallback (
			   boost::bind (&GroupSelection::tabBarTimeout,
					this));

	    mTabBar->mTimeoutHandle.start ();
	}

	newAnim = true;
    }

    gs->checkFunctions ();

    return newAnim;
}

/*
 * GroupWindow::adjustTabVelocity
 *
 * adjust velocity for each animation step (adapted from the scale plugin)
 */
int
GroupWindow::adjustTabVelocity ()
{
    float dx, dy, adjust, amount;
    float x1, y1;

    x1 = mDestination.x ();
    y1 = mDestination.y ();

    /* dx is how much movement we currently have remaining
     * in this case finalPos - (orgPos + mTx) */
    dx = x1 - (mOrgPos.x () + mTx);
    /* multiply this distance (gets smaller every time)
     * by 0.15 to get our adjust amount */
    adjust = dx * 0.15f;
    /* get the absolute value of the distance
     * and multiply by 1.5 to get our velcoty adjust amount */
    amount = fabs (dx) * 1.5f;
    /* Ensure that we are not too fast or slow */
    if (amount < 0.5f)
	amount = 0.5f;
    else if (amount > 5.0f)
	amount = 5.0f;

    /* velocity is scaled down slightly and adjusted by 1/10th of the
     * distance remaining */
    mXVelocity = (amount * mXVelocity + adjust) / (amount + 1.0f);

    /* dy is how much movement we currently have remaining
     * in this case finalPos - (orgPos + mTx) */
    dy = y1 - (mOrgPos.y () + mTy);
    /* multiply this distance (gets smaller every time)
     * by 0.15 to get our adjust amount */
    adjust = dy * 0.15f;
    /* get the absolute value of the distance
     * and multiply by 1.5 to get our velcoty adjust amount */
    amount = fabs (dy) * 1.5f;
    if (amount < 0.5f)
	amount = 0.5f;
    else if (amount > 5.0f)
	amount = 5.0f;

    /* Ensure that we are not too fast or slow */
    mYVelocity = (amount * mYVelocity + adjust) / (amount + 1.0f);

    /* If the distance is particularly short and we are travelling slowly,
     * then just round-down to zero and lock window in place */
    if (fabs (dx) < 0.1f && fabs (mXVelocity) < 0.2f &&
	fabs (dy) < 0.1f && fabs (mYVelocity) < 0.2f)
    {
	mXVelocity = mYVelocity = 0.0f;
	mTx = x1 - window->serverX ();
	mTy = y1 - window->serverY ();

	return 0;
    }
    return 1;
}

/*
 * GroupSelection::finishTabbing
 *
 * Cleans up the tab bar related things - called from
 * donePaint after the tabbing animation. In the case of
 * tabbing, then set appropriate window visibilities
 * and move windows to just behind the tab bar. Otherwise
 * if we are untabbing then we can get rid of the tab bar, and
 * if ungrouping, get rid of the whole group
 *
 */

void
GroupSelection::finishTabbing ()
{
    GROUP_SCREEN (screen);

    /* Complete untabbing (but not ungrouping)
     * case, delete the tab bar */
    if (mTabbingState == Untabbing &&
	mUngroupState != UngroupSingle)
    {
	delete mTabBar;
	mTabBar = NULL;
	mTopId = None;
    }

    /* We are finished tabbing now */
    mTabbingState = NoTabbing;

    gs->tabChangeActivateEvent (false);

    if (mTabBar)
    {
	/* tabbing case - hide all non-toptab windows */
	GroupTabBarSlot *slot;

	foreach (slot, mTabBar->mSlots)
	{
	    CompWindow *w = slot->mWindow;
	    if (!w)
		continue;

	    GROUP_WINDOW (w);

	    /* Don't hide this window if we are ungrouping or if
	     * this is the top tab */
	    if (slot == mTabBar->mTopTab || (gw->mAnimateState & IS_UNGROUPING))
		continue;

	    gw->setWindowVisibility (false);
	}
	/* The last top tab (for change animation purposes) is now
	 * this current top tab */
	mTabBar->mPrevTopTab = mTabBar->mTopTab;
    }

    /* Move all windows to their animation target position */
    for (CompWindowList::iterator it = mWindows.begin ();
	 it != mWindows.end ();
	 ++it)
    {
	CompWindow *w = *it;
	GROUP_WINDOW (w);

	/* move window to target position */
	gs->mQueued = true;
	w->move (gw->mDestination.x () - WIN_X (w),
		 gw->mDestination.y () - WIN_Y (w), true);
	gs->mQueued = false;
	w->syncPosition ();

	if (mUngroupState == UngroupSingle &&
	    (gw->mAnimateState & IS_UNGROUPING))
	{
	    /* Possibility of stack breakage here, stop here */
	    gw->removeWindowFromGroup ();
	    it = mWindows.end ();
	}

	gw->mAnimateState = 0;
	gw->mTx = gw->mTy = gw->mXVelocity = gw->mYVelocity = 0.0f;
	gw->checkFunctions ();
    }

    gs->checkFunctions ();

    /* Kill the group if we just ungrouped the whole thing */
    if (mUngroupState == UngroupAll)
	fini ();
    else
	mUngroupState = UngroupNone;
}

/*
 * GroupSelection::drawTabAnimation
 *
 * Description:
 * This function is called from GroupScreen::preparePaint, to move
 * all the animated windows, with the required animation step.
 * The function goes through all grouped animated windows, calculates
 * the required step using adjustTabVelocity, moves the window,
 * and then checks if the animation is finished for that window.
 *
 */
bool
GroupSelection::drawTabAnimation (int	      msSinceLastPaint)
{
    int        steps;
    float      amount, chunk;
    bool       doTabbing;

    GROUP_SCREEN (screen);

    /* a higher amount here means that more steps are calculated in the velocity,
     * but also that the velocity is a higher scale up factor, so the animation
     * happens quicker */
    amount = msSinceLastPaint * 0.05f * gs->optionGetTabbingSpeed ();
    /* more steps means that the velocity is recalculated more times, which increases the chance
     * for "overshoot" */
    steps = amount / (0.5f * gs->optionGetTabbingTimestep ());
    if (!steps)
	steps = 1;
    chunk = amount / (float)steps;

    /* Do this for each calculated animation step */
    while (steps--)
    {
	doTabbing = false;

	foreach (CompWindow *cw, mWindows)
	{
	    if (!cw)
		continue;

	    GROUP_WINDOW (cw);

	    if (!(gw->mAnimateState & IS_ANIMATED))
		continue;

	    /* adjustTabVelocity will adjust the speed of the window
	     * movement. At the end of the animation it will return 0,
	     * so that means that this window has finished animating
	     */
	    if (!gw->adjustTabVelocity ())
	    {
		gw->mAnimateState |= FINISHED_ANIMATION;
		gw->mAnimateState &= ~IS_ANIMATED;
	    }

	    /* Move translation amount to the window */
	    gw->mTx += gw->mXVelocity * chunk;
	    gw->mTy += gw->mYVelocity * chunk;

	    /* Keep doing the tabbing animation if this window is
	     * still animated */
	    doTabbing |= (gw->mAnimateState & IS_ANIMATED);
	}

	if (!doTabbing)
	{
	    /* tabbing animation finished */
	    finishTabbing ();
	    break;
	}
    }

    return doTabbing;
}

/*
 * GroupScreen::updateTabBars
 *
 * Description:
 * This function is responsible for showing / unshowing the tab-bars,
 * when the title-bars / tab-bars are hovered.
 * The function is called whenever a new window is entered,
 * checks if the entered window is a window frame (and if the title
 * bar part of that frame was hovered) or if it was the input
 * prevention window of a tab bar, and sets tab-bar visibility
 * according to that.
 *
 */
void
GroupScreen::updateTabBars (Window enteredWin)
{
    CompWindow     *w = NULL;
    GroupSelection *hoveredGroup = NULL;

    /* do nothing if the screen is grabbed, as the frame might be drawn
       transformed */
    if (!screen->otherGrabExist ("group", "group-drag", NULL))
    {
	/* first check if the entered window is a frame */
	foreach (w, screen->windows ())
	{
	    if (w->frame () == enteredWin)
		break;
	}
    }

    if (w)
    {
	/* is the window the entered frame belongs to inside
	   a tabbed group? if no, it's not interesting for us */
	GROUP_WINDOW (w);

	if (gw->mGroup && gw->mGroup->mTabBar)
	{
	    int mouseX, mouseY;
	    /* it is grouped and tabbed, so now we have to
	       check if we hovered the title bar or the frame */
	    if (getCurrentMousePosition (mouseX, mouseY))
	    {
		CompRect rect;
		CompRegion     reg;

		/* titlebar of the window */
		rect = CompRect (WIN_X (w) - w->border ().left,
				 WIN_Y (w) - w->border ().top,
				 WIN_WIDTH (w) + w->border ().right,
				 WIN_Y (w) - (WIN_Y (w) - w->border ().top));

		reg = reg.united (rect);

		/* this is a hovered group if the mouse is inside the
		 * titlebar region */
		if (reg.contains (CompPoint (mouseX, mouseY)))
		{
		    hoveredGroup = gw->mGroup;
		}
	    }
	}
    }

    /* if we didn't hover a title bar, check if we hovered
       a tab bar (means: input prevention window) */
    if (!hoveredGroup)
    {
	GroupSelection *group;

	foreach (group, mGroups)
	{
	    if (group->mTabBar &&
		group->mTabBar->mInputPrevention == enteredWin)
	    {
		/* only accept it if the IPW is mapped */
		if (group->mTabBar->mIpwMapped)
		{
		    hoveredGroup = group;
		    break;
		}
	    }
	}
    }

    /* if we found a hovered tab bar different than the last one
       (or left a tab bar), hide the old one */
    if (mLastHoveredGroup && (hoveredGroup != mLastHoveredGroup))
	mLastHoveredGroup->tabSetVisibility (false, 0);

    /* if we entered a tab bar (or title bar), show the tab bar */
    if (hoveredGroup && HAS_TOP_WIN (hoveredGroup) &&
	!TOP_TAB (hoveredGroup)->grabbed ())
    {
	GroupTabBar *bar = hoveredGroup->mTabBar;

	/* If the tab bar isn't painting or is fading out, set up
	 * the show delay time and make the tab bar appear (don't
	 * delay on fade out though)
	 */
	if (bar && ((bar->mState == PaintOff) ||
	    (bar->mState == PaintFadeOut)))
	{
	    int showDelayTime = optionGetTabbarShowDelay () * 1000;

	    /* Show the tab-bar after a delay,
	       only if the tab-bar wasn't fading out. */
	    if (showDelayTime > 0 && (bar->mState == PaintOff))
	    {
		if (mShowDelayTimeoutHandle.active ())
		    mShowDelayTimeoutHandle.stop ();

		mShowDelayTimeoutHandle.setTimes (showDelayTime,
						  showDelayTime * 1.2);

		mShowDelayTimeoutHandle.setCallback (
			boost::bind (&GroupSelection::showDelayTimeout,
				     hoveredGroup));
		mShowDelayTimeoutHandle.start ();
	    }
	    else
		hoveredGroup->showDelayTimeout ();
	}
    }
    else
	checkFunctions ();

    mLastHoveredGroup = hoveredGroup;
}



/*
 * GroupScreen::getConstrainRegion
 *
 * Description: Get the region on screen where windows are allowed to
 * move to during the untabbing animation
 *
 */
CompRegion
GroupScreen::getConstrainRegion ()
{
    CompRegion     region;
    CompRect       r;

    /* Get the region for each united output device. We cannot just
     * use screen->width () * screen->height () since that doesn't
     * account for weird multihead configs, where there are gaps
     * or different screen sizes and the like. */
    for (unsigned int i = 0;i < screen->outputDevs ().size (); i++)
	region = CompRegion (screen->outputDevs ()[i]).united (region);

    foreach (CompWindow *w, screen->windows ())
    {
	if (!w->mapNum ())
	    continue;

	/* Don't place windows underneath any panels */
	if (w->struts ())
	{
	    r = CompRect (w->struts ()->top.x,
			  w->struts ()->top.y,
			  w->struts ()->top.width,
			  w->struts ()->top.height);

	    region = region.subtracted (r);

	    r = CompRect (w->struts ()->bottom.x,
			  w->struts ()->bottom.y,
			  w->struts ()->bottom.width,
			  w->struts ()->bottom.height);

	    region = region.subtracted (r);

	    r = CompRect (w->struts ()->left.x,
			  w->struts ()->left.y,
			  w->struts ()->left.width,
			  w->struts ()->left.height);

	    region = region.subtracted (r);

	    r = CompRect (w->struts ()->right.x,
			  w->struts ()->right.y,
			  w->struts ()->right.width,
			  w->struts ()->right.height);

	    region = region.subtracted (r);
	}
    }

    return region;
}

/*
 * GroupWindow::constrainMovement
 *
 */
bool
GroupWindow::constrainMovement (CompRegion constrainRegion,
				     int        dx,
				     int        dy,
				     int        &new_dx,
				     int        &new_dy)
{
    int status, xStatus;
    int origDx = dx, origDy = dy;
    int x, y, width, height;
    CompWindow *w = window;

    if (!mGroup)
	return false;

    if (!dx && !dy)
	return false;

    x = mOrgPos.x () - w->border ().left + dx;
    y = mOrgPos.y ()- w->border ().top + dy;
    width = WIN_REAL_WIDTH (w);
    height = WIN_REAL_HEIGHT (w);

    /* Check if, with the movement, the constrainRegion does not
     * contain the window rect */
    status = constrainRegion.contains (CompRect (x, y, width, height));

    /* Adjust dx */
    xStatus = status;
    while (dx && (xStatus != RectangleIn))
    {
	/* check if, with dy taken out (so the rect will only be
	 * outside the region by some x amount) if the rect is still
	 * outside the reigon */
	xStatus = constrainRegion.contains (CompRect (x, y - dy, width, height));

	/* If it is, then move it slightly left or right based
	 * on which direction it was moving on */
	if (xStatus != RectangleIn)
	    dx += (dx < 0) ? 1 : -1;

	/* new x value is based on our new dx value */
	x = mOrgPos.x () - w->border ().left + dx;
    }

    /* Adjust dx */
    while (dy && (status != RectangleIn))
    {
	/* check if, with dy taken out (so the rect will only be
	 * outside the region by some x amount) if the rect is still
	 * outside the reigon */
	status = constrainRegion.contains (CompRect
						(x, y, width, height));

	/* If it is, then move it slightly left or right based
	 * on which direction it was moving on */
	if (status != RectangleIn)
	    dy += (dy < 0) ? 1 : -1;

	/* new x value is based on our new dx value */
	y = mOrgPos.y () - w->border ().top + dy;
    }

    new_dx = dx;
    new_dy = dy;

    /* return changed */
    return ((dx != origDx) || (dy != origDy));
}

/*
 * GroupSelection::groupApplyConstraining
 *
 */
void
GroupSelection::applyConstraining (CompRegion	    constrainRegion,
				   Window	    constrainedWindow,
				   int	    	    dx,
				   int	    	    dy)
{
    if (!dx && !dy)
	return;

    foreach (CompWindow *w, mWindows)
    {
	GROUP_WINDOW (w);

	/* ignore certain windows: we don't want to apply the constraining
	   results on the constrained window itself, nor do we want to
	   change the target position of unamimated windows and of
	   windows which already are constrained */
	if (w->id () == constrainedWindow)
	    continue;

	/* Do not bother if we are not animated or not constraining */
	if (!(gw->mAnimateState & IS_ANIMATED))
	    continue;

	if (gw->mAnimateState & DONT_CONSTRAIN)
	    continue;

	if (!(gw->mAnimateState & CONSTRAINED_X))
	{
	    int dummy;
	    gw->mAnimateState |= IS_ANIMATED;

	    /* applying the constraining result of another window
	       might move the window offscreen, too, so check
	       if this is not the case */
	    if (gw->constrainMovement (constrainRegion, dx, 0, dx, dummy))
		gw->mAnimateState |= CONSTRAINED_X;

	    gw->mDestination.setX (gw->mDestination.x () + dx);
	}

	if (!(gw->mAnimateState & CONSTRAINED_Y))
	{
	    int dummy;
	    gw->mAnimateState |= IS_ANIMATED;

	    /* analog to X case */
	    if (gw->constrainMovement (constrainRegion, 0, dy, dummy, dy))
		gw->mAnimateState |= CONSTRAINED_Y;

	    gw->mDestination.setY (gw->mDestination.y () + dy);
	}
    }
}

/*
 * GroupSelection::startTabbingAnimation
 *
 * Set up the tabbing animation, which will eventually set up
 * the tab structures too. "tab" here indicates whether we are
 * tabbing or untabbing (and takes appropriate action)
 *
 */
void
GroupSelection::startTabbingAnimation (bool           tab)
{
    GROUP_SCREEN (screen);

    if ((mTabbingState != NoTabbing))
	return;

    mTabbingState = (tab) ? Tabbing : Untabbing;
    gs->tabChangeActivateEvent (true);

    if (!tab)
    {
	/* we need to set up the X/Y constraining on untabbing */
	CompRegion constrainRegion = gs->getConstrainRegion ();
	bool   constrainedWindows = true;

	/* reset all flags */
	foreach (CompWindow *cw, mWindows)
	{
	    GROUP_WINDOW (cw);
	    gw->mAnimateState &= ~(CONSTRAINED_X | CONSTRAINED_Y |
				  DONT_CONSTRAIN);
	}

	/* as we apply the constraining in a flat loop,
	   we may need to run multiple times through this
	   loop until all constraining dependencies are met */
	while (constrainedWindows)
	{
	    constrainedWindows = false;
	    /* loop through all windows and try to constrain their
	       animation path (going from gw->mOrgPos to
	       gw->mDestination) to the active screen area */
	    foreach (CompWindow *w, mWindows)
	    {
		int        dx, dy;\
		int        constrainStatus;
		GroupWindow *gw = GroupWindow::get (w);
		CompRect   statusRect (gw->mOrgPos.x () - w->border ().left,
				       gw->mOrgPos.y () - w->border ().top,
				       WIN_REAL_WIDTH (w),
				       WIN_REAL_HEIGHT (w));

		/* ignore windows which aren't animated and/or
		   already are at the edge of the screen area */
		if (!(gw->mAnimateState & IS_ANIMATED))
		    continue;

		if (gw->mAnimateState & DONT_CONSTRAIN)
		    continue;

		/* is the original position inside the screen area? */
		constrainStatus = constrainRegion.contains (statusRect);

		/* constrain the movement */
		if (gw->constrainMovement (constrainRegion,
					    gw->mDestination.x () - gw->mOrgPos.x (),
					    gw->mDestination.y () - gw->mOrgPos.y (),
					    dx, dy))
		{
		    /* handle the case where the window is outside the screen
		       area on its whole animation path */
		    if (constrainStatus != RectangleIn && !dx && !dy)
		    {
			gw->mAnimateState |= DONT_CONSTRAIN;
			gw->mAnimateState |= CONSTRAINED_X | CONSTRAINED_Y;

			/* use the original position as last resort */
			gw->mDestination = gw->mMainTabOffset;
		    }
		    else
		    {
			/* if we found a valid target position, apply
			   the change also to other windows to retain
			   the distance between the windows */
			gw->mGroup->applyConstraining (constrainRegion, w->id (),
						dx - gw->mDestination.x () +
						gw->mOrgPos.x (),
						dy - gw->mDestination.y () +
						gw->mOrgPos.y ());

			/* if we hit constraints, adjust the mask and the
			   target position accordingly */
			if (dx != (gw->mDestination.x () - gw->mOrgPos.x ()))
			{
			    gw->mAnimateState |= CONSTRAINED_X;
			    gw->mDestination.setX (gw->mOrgPos.x () + dx);
			}

			if (dy != (gw->mDestination.y () - gw->mOrgPos.y ()))
			{
			    gw->mAnimateState |= CONSTRAINED_Y;
			    gw->mDestination.setY (gw->mOrgPos.y () + dy);
			}

			constrainedWindows = true;
		    }
		}

		gw->checkFunctions ();
	    }
	}
    }
    else
    {
	foreach (CompWindow *w, mWindows)
	{
	    GROUP_WINDOW (w);

	    gw->checkFunctions ();
	}
    }

    gs->checkFunctions ();
}

/*
 * GroupSelection::tabGroup
 *
 * Set up the tab bar structures in this group, render layers
 * and set up the original positions and destinations in the tabbing
 * animation
 *
 */
void
GroupSelection::tabGroup (CompWindow *main)
{
    GroupTabBarSlot *slot;
    CompSize 	    layerSize;
    int             space, thumbSize;

    GROUP_WINDOW (main);
    GROUP_SCREEN (screen);

    if (mTabBar)
	return;

    /* Since we are clearing the input shape of the window, we need
     * access to the XShape extension. If we don't have it, it is
     * not safe to continue doing so, so just abort here */
    if (!screen->XShape ())
    {
	compLogMessage ("group", CompLogLevelError,
			"No X shape extension! Tabbing disabled.");
	return;
    }

    mTabBar = new GroupTabBar (this, main);
    if (!mTabBar)
	return;

    /* We are not tabbing in yet (this is necessary so we can
     * "change" to our top tab) */
    mTabbingState = NoTabbing;
    /* Slot is initialized after GroupTabBar is created */
    gs->changeTab (gw->mSlot, GroupTabBar::RotateUncertain);

    /* Set up tab bar dimentions */
    mTabBar->recalcTabBarPos (WIN_CENTER_X (main),
			  WIN_X (main), WIN_X (main) + WIN_WIDTH (main));

    layerSize = CompSize (mTabBar->mRegion.boundingRect ().width (),
			  mTabBar->mRegion.boundingRect ().height ());

    /* set up the text layer */
    mTabBar->mTextLayer = TextLayer::create (layerSize, this);
    if (mTabBar->mTextLayer)
    {
	TextLayer *layer;

	layer = mTabBar->mTextLayer;
	layer->mState = PaintFadeIn;
	layer->mAnimationTime = gs->optionGetFadeTextTime () * 1000;

	layer->render ();
    }

    /* we need a buffer for DnD here */
    space = gs->optionGetThumbSpace ();
    thumbSize = gs->optionGetThumbSize ();

    layerSize = CompSize (layerSize.width () + space + thumbSize,
			  layerSize.height ());

    /* create background layer */
    mTabBar->mBgLayer = BackgroundLayer::create (layerSize, this);
    if (mTabBar->mBgLayer)
    {
	mTabBar->mBgLayer->mState = PaintOn;
	mTabBar->mBgLayer->mAnimationTime = 0;
	mTabBar->mBgLayer->render ();
    }

    layerSize = CompSize (mTabBar->mTopTab->mRegion.boundingRect ().width (),
			  mTabBar->mTopTab->mRegion.boundingRect ().height ());

    /* create selection layer */
    mTabBar->mSelectionLayer = SelectionLayer::create (layerSize, this);
    if (mTabBar->mSelectionLayer)
    {
	CompSize size =
	    CompSize (mTabBar->mTopTab->mRegion.boundingRect ().width (),
		      mTabBar->mTopTab->mRegion.boundingRect ().height ());
	mTabBar->mSelectionLayer->mState = PaintOn;
	mTabBar->mSelectionLayer->mAnimationTime = 0;
	mTabBar->mSelectionLayer = SelectionLayer::rebuild (mTabBar->mSelectionLayer,
							    size);
	if (mTabBar->mSelectionLayer)
	    mTabBar->mSelectionLayer->render ();
    }

    if (!HAS_TOP_WIN (this))
	return;

    /* for each of the windows in the tabbed group, we need to set up
     * the tabbing animation original positions, tab offsets and
     * destinations (as well as initial translation) for each of
     * the windows */
    foreach (slot, mTabBar->mSlots)
    {
	CompWindow *cw = slot->mWindow;

	GROUP_WINDOW (cw);

	/* In the case where we are still animating out or have finished
	 * the animation but not yet moved the windows, move the windows
	 * to their old target position */
	if (gw->mAnimateState & (IS_ANIMATED | FINISHED_ANIMATION))
	    cw->move (gw->mDestination.x () - WIN_X (cw),
		      gw->mDestination.y () - WIN_Y (cw), true);

	/* new target position is centered to the main window */
	gw->mDestination = CompPoint (WIN_CENTER_X (main) - (WIN_WIDTH (cw) / 2),
				      WIN_CENTER_Y (main) - (WIN_HEIGHT (cw) / 2));

	/* This is required for determining how much we want to animate
	 * back out again once we are untabbing */
	gw->mMainTabOffset = CompPoint (WIN_X (cw), WIN_Y (cw)) -
					gw->mDestination;

	/* Initial translation is negative the window position minus
	 * the last original position */
	if (gw->mTx || gw->mTy)
	{
	    gw->mTx -= (WIN_X (cw) - gw->mOrgPos.x ());
	    gw->mTy -= (WIN_Y (cw) - gw->mOrgPos.y ());
	}

	/* Now the original position is the current position */
	gw->mOrgPos = CompPoint (WIN_X (cw), WIN_Y (cw));

	gw->mAnimateState = IS_ANIMATED;
	gw->mXVelocity = gw->mYVelocity = 0.0f;
    }

    /* Start the animation */
    startTabbingAnimation (true);
}

/*
 * GroupSelection::untabGroup
 *
 * Set up the group untabbing. If we are in the middle of a change
 * animation, then use the last top tab for this group as the window
 * where other windows will base the animation. (If the animation is
 * complete, then mPrevTopTab is going to be the same as mTopTab
 * anyways, so using it is always safe).
 */
void
GroupSelection::untabGroup ()
{
    int             oldX, oldY;
    CompWindow      *prevTopTab;
    GroupTabBarSlot *slot;

    GROUP_SCREEN (screen);

    if (!HAS_TOP_WIN (this))
	return;

    /* Sometimes mPrevTopTab might not be set, so use mTopTab instead */
    if (mTabBar->mPrevTopTab)
	prevTopTab = PREV_TOP_TAB (this);
    else
    {
	/* If prevTopTab isn't set, we have no choice but using topTab.
	   It happens when there is still animation, which
	   means the tab wasn't changed anyway. */
	prevTopTab = TOP_TAB (this);
    }

    /* Save the top tab into mLastTopTab - we will be using this one
     * to base our animation off of (since we are now setting mTopTab
     * to NULL). */
    mTabBar->mLastTopTab = TOP_TAB (this);
    mTabBar->mTopTab = NULL;
    mTabBar->mChangeState = GroupTabBar::NoTabChange;

    foreach (slot, mTabBar->mSlots)
    {
	CompWindow *cw = slot->mWindow;

	GROUP_WINDOW (cw);

	/* If there is currently an animation happening and windows
	 * haven't been moved yet, move them into their destination
	 * positions */
	if (gw->mAnimateState & (IS_ANIMATED | FINISHED_ANIMATION))
	{
	    gs->mQueued = true;
	    cw->move(gw->mDestination.x () - WIN_X (cw),
		     gw->mDestination.y () - WIN_Y (cw), true);
	    gs->mQueued = false;
	}

	/* All windows are now visible */
	gw->setWindowVisibility (true);

	/* save the old original position - we might need it
	   if constraining fails */
	oldX = gw->mOrgPos.x ();
	oldY = gw->mOrgPos.y ();

	/* The original position or animation starting point here is
	 * centered to the top tab which we are animating *out* of */
	gw->mOrgPos = CompPoint (WIN_CENTER_X (prevTopTab) - WIN_WIDTH (cw) / 2,
				 WIN_CENTER_Y (prevTopTab) - WIN_HEIGHT (cw) / 2);

	/* Destination is whatever the original position is plus the
	 * offsets we saved when tabbing */
	gw->mDestination = gw->mOrgPos + gw->mMainTabOffset;

	/* Set initial translation to the old new centered position
	 * minus the old "original position"
	 */
	if (gw->mTx || gw->mTy)
	{
	    gw->mTx -= (gw->mOrgPos.x () - oldX);
	    gw->mTy -= (gw->mOrgPos.y () - oldY);
	}

	/* now the offset is just our old orig pos */
	gw->mMainTabOffset = CompPoint (oldX, oldY);

	gw->mAnimateState = IS_ANIMATED;
	gw->mXVelocity = gw->mYVelocity = 0.0f;
    }

    /* set up the animation */
    mTabbingState = NoTabbing;
    startTabbingAnimation (false);

    gs->cScreen->damageScreen ();
}

/*
 * GroupScreen::changeTab
 *
 * Change the tab to a new topTab by some direction. Passing
 * DirectionUncertain will cause this function to calculate the most
 * appropriate direction for this tab
 *
 */
bool
GroupScreen::changeTab (GroupTabBarSlot             *topTab,
			GroupTabBar::ChangeAnimationDirection direction)
{
    CompWindow     *w, *oldTopTab;
    GroupSelection *group;

    if (!topTab)
	return true;

    w = topTab->mWindow;

    GROUP_WINDOW (w);

    group = gw->mGroup;

    /* Don't change if we are still constructing the tab bar */
    if (!group || !group->mTabBar ||
	group->mTabbingState != GroupSelection::NoTabbing)
	return true;

    /* Don't change if we are not currently changing and the requested
     * top tab is the requested top tab. We will still change back
     * in the case we are in the middle of a change animation and we
     * want to change back */
    if (group->mTabBar->mChangeState == GroupTabBar::NoTabChange &&
	group->mTabBar->mTopTab == topTab)
	return true;

    /* If the tab we are currently changing to is the requested top
     * tab, then don't bother starting the animation again */
    if (group->mTabBar->mChangeState != GroupTabBar::NoTabChange &&
	group->mTabBar->mNextTopTab == topTab)
	return true;

    /* We need this for movement and damage purposes */
    oldTopTab = group->mTabBar->mTopTab ?
		group->mTabBar->mTopTab->mWindow : NULL;

    /* If we are currently changing, set the next direction so that
     * the animation will start again there */
    if (group->mTabBar->mChangeState != GroupTabBar::NoTabChange)
	group->mTabBar->mNextDirection = direction;
    /* Set left or right respectively */
    else if (direction == GroupTabBar::RotateLeft)
	group->mTabBar->mChangeAnimationDirection = 1;
    else if (direction == GroupTabBar::RotateRight)
	group->mTabBar->mChangeAnimationDirection = -1;
    /* The requested change direction is RotateUncertain, so work
     * out the best way to rotate */
    else
    {
	int             distanceOld = 0, distanceNew = 0;
	GroupTabBarSlot::List::iterator it = group->mTabBar->mSlots.begin ();

	/* Count from left to the current top tab */
	if (group->mTabBar->mTopTab)
	    for (; (*it) && ((*it) != group->mTabBar->mTopTab);
		 ++it, distanceOld++);

	/* Count from left to the requested top tab */
	for (it = group->mTabBar->mSlots.begin (); (*it) && ((*it) != topTab);
	     ++it, distanceNew++);

	if (distanceNew < distanceOld)
	    group->mTabBar->mChangeAnimationDirection = 1;   /*left */
	else
	    group->mTabBar->mChangeAnimationDirection = -1;  /* right */

	/* check if the opposite direction is shorter */
	if (abs (distanceNew - distanceOld) > ((int) group->mTabBar->mSlots.size () / 2))
	    group->mTabBar->mChangeAnimationDirection *= -1;
    }

    /* If we are currently in the middle of an animation, we need to
     * handle this case */
    if (group->mTabBar->mChangeState != GroupTabBar::NoTabChange)
    {
	/* If we need to go back to the last top tab, then rotate
	 * backwards */
	if (group->mTabBar->mPrevTopTab == topTab)
	{
	    /* Reverse animation. */
	    GroupTabBarSlot *tmp = group->mTabBar->mTopTab;
	    bool changeOldOut = (group->mTabBar->mChangeState ==
				 GroupTabBar::TabChangeOldOut);
	    group->mTabBar->mTopTab = group->mTabBar->mPrevTopTab;
	    group->mTopId = group->mTabBar->mTopTab->mWindow->id ();
	    group->mTabBar->mPrevTopTab = tmp;

	    group->mTabBar->mChangeAnimationDirection *= -1;
	    group->mTabBar->mChangeAnimationTime =
		optionGetChangeAnimationTime () * 500 -
		group->mTabBar->mChangeAnimationTime;
	    group->mTabBar->mChangeState = changeOldOut ?
	     GroupTabBar::TabChangeNewIn : GroupTabBar::TabChangeOldOut;

	    group->mTabBar->mNextTopTab = NULL;
	}
	/* Otherwise the next one we want to go to is this one
	 * (rotate past a few tabs first) */
	else
	    group->mTabBar->mNextTopTab = topTab;
    }
    /* Otherwise we need to set up the change animation and the tab bar
     */
    else
    {
	group->mTabBar->mTopTab = topTab;
	group->mTopId = topTab->mWindow->id ();
	CompSize size (group->mTabBar->mTopTab->mRegion.boundingRect ().width (),
		       group->mTabBar->mTopTab->mRegion.boundingRect ().height ());

	/* Rebuild layers and render */
	group->mTabBar->mTextLayer = TextLayer::rebuild (group->mTabBar->mTextLayer);

	if (group->mTabBar->mTextLayer)
	    group->mTabBar->mTextLayer->render ();
	group->mTabBar->mSelectionLayer =
	       SelectionLayer::rebuild (group->mTabBar->mSelectionLayer,
					size);
	if (group->mTabBar->mSelectionLayer)
	    group->mTabBar->mSelectionLayer->render ();
	if (oldTopTab)
	    CompositeWindow::get (oldTopTab)->addDamage ();
	CompositeWindow::get (w)->addDamage ();
    }

    /* If we are not changing to another top tab */
    if (topTab != group->mTabBar->mNextTopTab)
    {
	/* Make this window visible */
	gw->setWindowVisibility (true);
	/* Center windows around the old top tab */
	if (oldTopTab)
	{
	    int dx, dy;

	    dx = WIN_CENTER_X (oldTopTab) - WIN_CENTER_X (w);
	    dy = WIN_CENTER_Y (oldTopTab) - WIN_CENTER_Y (w);

	    mQueued = true;
	    w->move (dx, dy, false);
	    w->syncPosition ();
	    mQueued = false;
	}

	/* If there is a previous window, change in the new tab */
	if (HAS_PREV_TOP_WIN (group))
	{
	    /* we use only the half time here -
	       the second half will be PaintFadeOut */
	    group->mTabBar->mChangeAnimationTime =
		optionGetChangeAnimationTime () * 500;
	    tabChangeActivateEvent (true);
	    group->mTabBar->mChangeState = GroupTabBar::TabChangeOldOut;
	}
	/* Otherwise activate the window */
	else
	{
	    bool activate;

	    /* No window to do animation with. */
	    if (HAS_TOP_WIN (group))
		group->mTabBar->mPrevTopTab = group->mTabBar->mTopTab;
	    else
		group->mTabBar->mPrevTopTab = NULL;

	    activate = !group->mTabBar->mCheckFocusAfterTabChange;
	    if (!activate)
	    {
		/*
		CompFocusResult focus;

		focus    = allowWindowFocus (w, NO_FOCUS_MASK, s->x, s->y, 0);
		activate = focus == CompFocusAllowed;
		*/
	    }

	    if (activate)
		w->activate ();

	    group->mTabBar->mCheckFocusAfterTabChange = false;
	}
    }

    if (group->mTabBar->mPrevTopTab)
    {
	CompWindow *pw = group->mTabBar->mPrevTopTab->mWindow;
	GroupWindow::get (pw)->checkFunctions ();
    }

    if (group->mTabBar->mTopTab)
    {
	CompWindow *tw = group->mTabBar->mTopTab->mWindow;
	GroupWindow::get (tw)->checkFunctions ();
    }

    GroupScreen::get (screen)->checkFunctions ();

    return true;
}

/*
 * GroupScreen::recalcSlotPos
 *
 * Recalculate the slot region.
 */
void
GroupScreen::recalcSlotPos (GroupTabBarSlot *slot,
				 int		 slotPos)
{
    GroupSelection *group;
    CompRect       box;
    int            space, thumbSize;

    GROUP_WINDOW (slot->mWindow);
    group = gw->mGroup;

    if (!HAS_TOP_WIN (group) || !group->mTabBar)
	return;

    space = optionGetThumbSpace ();
    thumbSize = optionGetThumbSize ();

    slot->mRegion = emptyRegion;

    /* Padding between slots, account for this */
    box.setX (space + ((thumbSize + space) * slotPos));
    box.setY (space);

    box.setWidth (thumbSize);
    box.setHeight (thumbSize);

    slot->mRegion = CompRegion (box);
}

/*
 * GroupSelection::recalcTabBarPos
 *
 * Recalculate the tab bar region (size and position) given a maximum
 * size and a central position
 *
 */
void
GroupTabBar::recalcTabBarPos (int		middleX,
			      int		minX1,
			      int		maxX2)
{
    GroupTabBarSlot *slot;
    CompWindow      *topTab;
    bool            isDraggedSlotGroup = false;
    int             space, barWidth;
    int             thumbSize;
    int             tabsWidth = 0, tabsHeight = 0;
    int             currentSlot;
    CompRect	    box;

    GROUP_SCREEN (screen);

    if (!HAS_TOP_WIN (mGroup))
	return;

    topTab = TOP_TAB (mGroup);
    space = gs->optionGetThumbSpace ();

    /* calculate the space which the tabs need
     * Note that we aren't going to get any size
     * if we haven't calculated slot regions yet */
    foreach (slot, mSlots)
    {
	if (slot == gs->mDraggedSlot && gs->mDragged)
	{
	    /* if this is a dragged slot group, then we will be
	     * calculating the space this slot would have taken
	     * through the spring values */
	    isDraggedSlotGroup = true;
	    continue;
	}

	/* Add the slot region to our current tabs width */
	tabsWidth += (slot->mRegion.boundingRect ().width ());
	if ((slot->mRegion.boundingRect ().height ()) > tabsHeight)
	    tabsHeight = slot->mRegion.boundingRect ().height ();
    }

    /* just a little work-a-round for first call
       FIXME: remove this! */
    thumbSize = gs->optionGetThumbSize ();
    if (mSlots.size () && tabsWidth <= 0)
    {
	/* first call */
	tabsWidth = thumbSize * mSlots.size ();

	if (mSlots.size () && tabsHeight < thumbSize)
	{
	    /* we need to do the standard height too */
	    tabsHeight = thumbSize;
	}

	/* Get rid of an extra space on this group */
	if (isDraggedSlotGroup)
	    tabsWidth -= thumbSize;
    }

    barWidth = space * (mSlots.size () + 1) + tabsWidth;

    if (isDraggedSlotGroup)
    {
	/* 1 tab is missing, so we have 1 less border */
	barWidth -= space;
    }

    /* If the maximum width is less than the bar width, set the x
     * position of the bar to the centered average of the maximum width
     * and the bar width */
    if (maxX2 - minX1 < barWidth)
	box.setX ((maxX2 + minX1) / 2 - barWidth / 2);
    /* If the middle point minus half the calculated bar width is less
     * than the minimum x1 point, constrain to the x1 point */
    else if (middleX - barWidth / 2 < minX1)
	box.setX (minX1);
    /* if the middle point plus half the bar width is more than the
     * maximum x point, then adjust the x point accordingly */
    else if (middleX + barWidth / 2 > maxX2)
	box.setX (maxX2 - barWidth);
    /* Otherwise the x point should be set to the middle point minus
     * the bar width (no constraining) */
    else
	box.setX (middleX - barWidth / 2);

    /* Y position is always the y position of the top tab */
    box.setY (WIN_Y (topTab));
    /* Width is the calculated bar width */
    box.setWidth (barWidth);
    /* Height is twice the padding plus the tabs height */
    box.setHeight (space * 2 + tabsHeight);

    /* Resize the reigon of the tab bar based on the calcuated box */
    resizeTabBarRegion (box, true);

    /* recalc every slot region */
    currentSlot = 0;
    foreach (slot, mSlots)
    {
	/* We calculate the dragged slot region later so don't
	 * recalculate it now */
	if (slot == gs->mDraggedSlot && gs->mDragged)
	    continue;

	/* Recalculate individual slot position */
	gs->recalcSlotPos (slot, currentSlot);
	/* Reposition slot */
	slot->mRegion.translate (mRegion.boundingRect ().x1 (),
				 mRegion.boundingRect ().y1 ());

	/* Set spring area to the center of the slot */
	slot->mSpringX = (slot->mRegion.boundingRect ().centerX ());
	slot->mSpeed = 0;
	slot->mMsSinceLastMove = 0;

	currentSlot++;
    }

    /* Left and right spring points are the left and right parts
     * of the bar */
    mLeftSpringX = box.x ();
    mRightSpringX = box.x () + box.width ();

    mRightSpeed = 0;
    mLeftSpeed = 0;

    mRightMsSinceLastMove = 0;
    mLeftMsSinceLastMove = 0;
}

/*
 * GroupTabBar::damageRegion
 *
 * Damage the tab bar region
 */
void
GroupTabBar::damageRegion ()
{
    CompRegion reg (mRegion);
    int x1 = reg.boundingRect ().x1 ();
    int x2 = reg.boundingRect ().x2 ();
    int y1 = reg.boundingRect ().y1 ();
    int y2 = reg.boundingRect ().y2 ();

    /* we use 15 pixels as damage buffer here, as there is a 10 pixel wide
       border around the selected slot which also needs to be damaged
       properly - however the best way would be if slot->mRegion was
       sized including the border */

static const unsigned short DAMAGE_BUFFER = 20;

    /* If there is a front slot in this bar, then we need to damage
     * areas just outside where the slot might be sitting */
    if (mSlots.size ())
    {
	const CompRect &bnd = mSlots.front ()->mRegion.boundingRect ();
	x1 = MIN (x1, bnd.x1 ());
	y1 = MIN (y1, bnd.y1 ());
	x2 = MAX (x2, bnd.x2 ());
	y2 = MAX (y2, bnd.y2 ());
    }

    x1 -= DAMAGE_BUFFER;
    y1 -= DAMAGE_BUFFER;
    x2 += DAMAGE_BUFFER;
    y2 += DAMAGE_BUFFER;

    reg = CompRegion (x1, y1,
		      x2 - x1,
		      y2 - y1);

    GroupScreen::get (screen)->cScreen->damageRegion (reg);
}

/*
 * GroupTabBar::moveTabBarRegion
 *
 * Move the reigon of the tab bar (which moves the drawn texture
 * and tabs). syncIPW is whether or not to move the X11 input prevention
 * window - usually false if we are just animating the tab bar around
 *
 */

void
GroupTabBar::moveTabBarRegion (int		   dx,
			       int		   dy,
			       bool	   	   syncIPW)
{
    damageRegion ();

    mRegion.translate (dx, dy);

    if (syncIPW)
	XMoveWindow (screen->dpy (),
		     mInputPrevention,
		     mLeftSpringX,
		     mRegion.boundingRect ().y1 ());

    damageRegion ();
}

/*
 * GroupTabBar::resizeTabBarRegion
 *
 * Resize the region that the tab bar covers by some CompRect.
 * syncIPW is whether or not to XConfigure the input prevention
 * window (don't use this unless the tab bar position is being set
 * like that permanently).
 *
 */
void
GroupTabBar::resizeTabBarRegion (CompRect	&box,
				 bool           syncIPW)
{
    int oldWidth;

    GROUP_SCREEN (screen);

    damageRegion ();

    oldWidth = mRegion.boundingRect ().width ();

    /* If the old width is not the same as the new one and we are
     * syncing the IPW, rebuild the background layer */
    if (mBgLayer && oldWidth != box.width () && syncIPW)
    {
	mBgLayer =
	    BackgroundLayer::rebuild (mBgLayer,
				 CompSize (box.width () +
				    gs->optionGetThumbSpace () +
				    gs->optionGetThumbSize (),
				    box.height ()));
	if (mBgLayer)
	    mBgLayer->render ();

	/* invalidate old width */
	mOldWidth = 0;
    }

    mRegion = CompRegion (box);

    /* Configure the IPW */
    if (syncIPW)
    {
	XWindowChanges xwc;

	xwc.x = box.x ();
	xwc.y = box.y ();
	xwc.width = box.width ();
	xwc.height = box.height ();

	if (!mIpwMapped)
	    XMapWindow (screen->dpy (), mInputPrevention);

	XMoveResizeWindow (screen->dpy (), mInputPrevention, xwc.x, xwc.y, xwc.width, xwc.height);

	if (!mIpwMapped)
	    XUnmapWindow (screen->dpy (), mInputPrevention);
    }

    damageRegion ();
}

/*
 * GroupTabBar::insertTabBarSlotBefore
 *
 * Insert a tab bar slot before some other slot and relink stack
 *
 */
void
GroupTabBar::insertTabBarSlotBefore (GroupTabBarSlot *slot,
				     GroupTabBarSlot *nextSlot)
{
    GroupTabBarSlot *prev = nextSlot->mPrev;
    GroupTabBarSlot::List::iterator pos = std::find (mSlots.begin (),
						     mSlots.end (),
						     nextSlot);

    mSlots.insert (pos, slot);
    slot->mTabBar = this;

    if (prev)
    {
	slot->mPrev = prev;
	prev->mNext = slot;
    }
    else
    {
	slot->mPrev = NULL;
    }

    slot->mNext = nextSlot;
    nextSlot->mPrev = slot;

    /* Moving bar->mRegion.boundingRect ().x1 () / x2 as minX1 / maxX2 will work,
       because the tab-bar got wider now, so it will put it in
       the average between them, which is
       (bar->mRegion.boundingRect ().centerX ()) anyway. */
    recalcTabBarPos (mRegion.boundingRect ().centerX (),
		     mRegion.boundingRect ().x1 (), mRegion.boundingRect ().x2 ());
}

/*
 * GroupSelection::insertTabBarSlotAfter
 *
 * Insert a tab bar slot after some previous slot and relink the
 * stack
 *
 */
void
GroupTabBar::insertTabBarSlotAfter (GroupTabBarSlot *slot,
				    GroupTabBarSlot *prevSlot)
{
    GroupTabBarSlot *next = prevSlot->mNext;
    GroupTabBarSlot::List::iterator pos = std::find (mSlots.begin (),
						     mSlots.end (),
						     next);

    mSlots.insert (pos, slot);
    slot->mTabBar = this;

    if (next)
    {
	slot->mNext = next;
	next->mPrev = slot;
    }
    else
    {
	slot->mNext = NULL;
    }

    slot->mPrev = prevSlot;
    prevSlot->mNext = slot;

    /* Moving bar->mRegion.boundingRect ().x1 () / x2 as minX1 / maxX2 will work,
       because the tab-bar got wider now, so it will put it in the
       average between them, which is
       (bar->mRegion.boundingRect ().x1 () + bar->mRegion.boundingRect ().x2 ()) / 2 anyway. */
    recalcTabBarPos (mRegion.boundingRect ().centerX (),
		     mRegion.boundingRect ().x1 (),
		     mRegion.boundingRect ().x2 ());
}

/*
 * GroupSelection::insertTabBarSlot
 *
 * Insert a tab bar slot into the tab bar. This inserts it directly
 * into the end of the tab bar
 *
 */
void
GroupTabBar::insertTabBarSlot (GroupTabBarSlot *slot)
{
    if (mSlots.size ())
    {
	mSlots.back ()->mNext = slot;
	slot->mPrev = mSlots.back ();
	slot->mNext = NULL;
    }
    else
    {
	slot->mPrev = NULL;
	slot->mNext = NULL;
    }

    mSlots.push_back (slot);
    slot->mTabBar = this;

    /* Moving bar->mRegion.boundingRect ().x1 () / x2 as minX1 / maxX2 will work,
       because the tab-bar got wider now, so it will put it in
       the average between them, which is
       (bar->mRegion.boundingRect ().x1 () + bar->mRegion.boundingRect ().x2 ()) / 2 anyway. */
    recalcTabBarPos ((mRegion.boundingRect ().centerX ()),
		      mRegion.boundingRect ().x1 (),
		      mRegion.boundingRect ().x2 ());
}

/*
 * GroupTabBar::unhookTabBarSlot
 *
 * Take out a tab bar slot from the tab bar and make it a dragged
 * slot. If temporary is false, then remove this window from the
 * group
 *
 */
void
GroupTabBar::unhookTabBarSlot (GroupTabBarSlot *slot,
			       bool            temporary)
{
    GroupTabBarSlot *tempSlot = NULL;
    /* query for next and previous in linked list */
    GroupTabBarSlot *prev = slot->mPrev;
    GroupTabBarSlot *next = slot->mNext;
    CompWindow      *w = slot->mWindow;
    GroupSelection  *group = mGroup;

    GROUP_SCREEN (screen);

    /* check if slot is not already unhooked */
    foreach (tempSlot, mSlots)
	if (tempSlot == slot)
	    break;

    if (!tempSlot)
	return;

    /* relink stack */
    if (prev)
	prev->mNext = next;

    if (next)
	next->mPrev = prev;

    /* This slot's previous and next slots are now NULL */
    slot->mPrev = NULL;
    slot->mNext = NULL;
    slot->mTabBar = NULL;

    mSlots.remove (slot);

    /* If it isn't temporary. Change to another tab */
    if (!temporary)
    {
	/* If this was the previous top tab, set that to null */
	if (IS_PREV_TOP_TAB (w, group))
	    group->mTabBar->mPrevTopTab = NULL;
	if (IS_TOP_TAB (w, group))
	{
	    group->mTabBar->mTopTab = NULL;
	    group->mTopId = None;

	    /* Change to the next tab first, otherwise the previous one
	     */
	    if (next)
		gs->changeTab (next, RotateRight);
	    else if (prev)
		gs->changeTab (prev, RotateLeft);

	    if (gs->optionGetUntabOnClose ())
		group->untabGroup ();
	}
    }

    /* set slot points to NULL so we don't use them again */
    if (slot == mHoveredSlot)
	mHoveredSlot = NULL;

    if (slot == mTextSlot)
    {
	mTextSlot = NULL;

	/* Fade out text */
	if (mTextLayer)
	{
	    if (mTextLayer->mState == PaintFadeIn ||
		mTextLayer->mState == PaintOn)
	    {
		mTextLayer->mAnimationTime =
		    (gs->optionGetFadeTextTime () * 1000) -
		    mTextLayer->mAnimationTime;
		mTextLayer->mState = PaintFadeOut;
	    }
	}
    }

    /* Moving bar->mRegion.boundingRect ().x1 () / x2 as minX1 / maxX2 will work,
       because the tab-bar got thiner now, so
       (bar->mRegion.boundingRect ().x1 () + bar->mRegion.boundingRect ().x2 ()) / 2
       Won't cause the new x1 / x2 to be outside the original region. */
    recalcTabBarPos (mRegion.boundingRect ().centerX (),
		      mRegion.boundingRect ().x1 (),
		      mRegion.boundingRect ().x2 ());
}

/*
 * GroupSelection::deleteTabBarSlot
 *
 * Destroys a tab bar slot structure
 *
 */
void
GroupTabBar::deleteTabBarSlot (GroupTabBarSlot *slot)
{
    CompWindow *w = slot->mWindow;

    GROUP_WINDOW (w);
    GROUP_SCREEN (screen);

    /* It can't be part of any tab bar */
    unhookTabBarSlot (slot, false);

    /* New region is just a blank region */
    slot->mRegion = CompRegion ();

    /* If this is the dragged slot, set the pointers for that to
     * null, and remove grabs */
    if (slot == gs->mDraggedSlot)
    {
	gs->mDraggedSlot = NULL;
	gs->mDragged = false;

	if (gs->mGrabState == GroupScreen::ScreenGrabTabDrag)
	    gs->grabScreen (GroupScreen::ScreenGrabNone);
    }

    /* This window now has no slot */
    gw->mSlot = NULL;
    gs->writeSerializedData ();
    delete slot;
}

GroupTabBarSlot::GroupTabBarSlot (CompWindow *w, GroupTabBar *bar) :
    GLLayer (CompSize (0,0), bar->mGroup), // FIXME: make this the size?
    mWindow (w),
    mTabBar (bar)
{
}

/*
 * GroupTabBar::groupCreateSlot
 *
 * Factory function to create a new tab bar slot for some window
 * (automatically inserts it into the tab bar)
 *
 */
void
GroupTabBar::createSlot (CompWindow      *w)
{
    GroupTabBarSlot *slot;

    GROUP_WINDOW (w);
    GROUP_SCREEN (screen);

    slot = new GroupTabBarSlot (w, this);
    if (!slot)
        return;

    insertTabBarSlot (slot);
    gw->mSlot = slot;
    gs->writeSerializedData ();
}

#define SPRING_K     GroupScreen::get (screen)->optionGetDragSpringK()
#define FRICTION     GroupScreen::get (screen)->optionGetDragFriction()
#define SIZE	     GroupScreen::get (screen)->optionGetThumbSize()
#define BORDER	     GroupScreen::get (screen)->optionGetBorderRadius()
#define Y_START_MOVE GroupScreen::get (screen)->optionGetDragYDistance()
#define SPEED_LIMIT  GroupScreen::get (screen)->optionGetDragSpeedLimit()

/*
 * groupSpringForce
 *
 */
static inline int
groupSpringForce (CompScreen *s,
		  int        centerX,
		  int        springX)
{
    /* Each slot has a spring attached to it, starting at springX,
       and ending at the center of the slot (centerX).
       The spring will cause the slot to move, using the
       well-known physical formula F = k * dl... */
    return -SPRING_K * (centerX - springX);
}

/*
 * groupDraggedSlotForce
 *
 */
static int
groupDraggedSlotForce (CompScreen *s,
		       int        distanceX,
		       int        distanceY)
{
    /* The dragged slot will make the slot move, to get
       DnD animations (slots will make room for the newly inserted slot).
       As the dragged slot is closer to the slot, it will put
       more force on the slot, causing it to make room for the dragged slot...
       But if the dragged slot gets too close to the slot, they are
       going to be reordered soon, so the force will get lower.

       If the dragged slot is in the other side of the slot,
       it will have to make force in the opposite direction.

       So we the needed funtion is an odd function that goes
       up at first, and down after that.
       Sinus is a function like that... :)

       The maximum is got when x = (x1 + x2) / 2,
       in this case: x = SIZE + BORDER.
       Because of that, for x = SIZE + BORDER,
       we get a force of SPRING_K * (SIZE + BORDER) / 2.
       That equals to the force we get from the the spring.
       This way, the slot won't move when its distance from
       the dragged slot is SIZE + BORDER (which is the default
       distance between slots).
       */

    /* The maximum value */
    float a = SPRING_K * (SIZE + BORDER) / 2;
    /* This will make distanceX == 2 * (SIZE + BORDER) to get 0,
       and distanceX == (SIZE + BORDER) to get the maximum. */
    float b = PI /  (2 * SIZE + 2 * BORDER);

    /* If there is some distance between the slots in the y axis,
       the slot should get less force... For this, we change max
       to a lower value, using a simple linear function. */

    if (distanceY < Y_START_MOVE)
	a *= 1.0f - (float)distanceY / Y_START_MOVE;
    else
	a = 0;

    if (abs (distanceX) < 2 * (SIZE + BORDER))
	return a * sin (b * distanceX);
    else
	return 0;
}

/*
 * groupApplyFriction
 *
 */
static inline void
groupApplyFriction (CompScreen *s,
		    int        *speed)
{
    if (abs (*speed) < FRICTION)
	*speed = 0;
    else if (*speed > 0)
	*speed -= FRICTION;
    else if (*speed < 0)
	*speed += FRICTION;
}

/*
 * groupApplySpeedLimit
 *
 */
static inline void
groupApplySpeedLimit (CompScreen *s,
		      int        *speed)
{
    if (*speed > SPEED_LIMIT)
	*speed = SPEED_LIMIT;
    else if (*speed < -SPEED_LIMIT)
	*speed = -SPEED_LIMIT;
}

/*
 * GroupTabBar::applyForces
 *
 * Apply forces to slots, move them around accordingly
 *
 */
bool
GroupTabBar::applyForces (GroupTabBarSlot *draggedSlot)
{
    GroupTabBarSlot *slot, *slot2;
    int             centerX, centerY;
    int             draggedCenterX, draggedCenterY;
    bool	    forces = false;

    /* Calculate the dragged slot center to calculate forces on the
     * other tabs */
    if (draggedSlot)
    {
	int vx, vy;

	draggedSlot->getDrawOffset (vx, vy);

	draggedCenterX = draggedSlot->mRegion.boundingRect ().centerX () + vx;
	draggedCenterY = draggedSlot->mRegion.boundingRect ().centerY () + vy;
    }
    else
    {
	draggedCenterX = 0;
	draggedCenterY = 0;
    }

    /* mLeft/RightSpeed describe the stretching of the tab bar
     * during animations. This is basically just adding the new
     * adding the spring factor times the distance between the
     * anchor point and spring point */
    mLeftSpeed += groupSpringForce(screen,
				   mRegion.boundingRect ().x1 (),
				   mLeftSpringX);
    mRightSpeed += groupSpringForce(screen,
				    mRegion.boundingRect ().x2 (),
				    mRightSpringX);

    /* Apply forces on the tab bar for this dragged slot */
    if (draggedSlot)
    {
	int leftForce, rightForce;

	/* Forces here are basically the center point of tab minus the
	 * dragged center point and the center y point of the tab bar
	 * minus the dragged y center
	 */

	leftForce = groupDraggedSlotForce(screen,
					  mRegion.boundingRect ().x1 () -
					  SIZE / 2 - draggedCenterX,
					  abs ((mRegion.boundingRect ().centerY ()) / 2 -
					       draggedCenterY));

	rightForce = groupDraggedSlotForce (screen,
					    mRegion.boundingRect ().x2 () +
					    SIZE / 2 - draggedCenterX,
					    abs ((mRegion.boundingRect ().centerY ()) / 2 -
						 draggedCenterY));

	if (leftForce < 0)
	    mLeftSpeed += leftForce;
	if (rightForce > 0)
	    mRightSpeed += rightForce;
    }

    /* Now apply the spring force on each slot */
    foreach (slot, mSlots)
    {
	centerX = slot->mRegion.boundingRect ().centerX ();
	centerY = slot->mRegion.boundingRect ().centerY ();

	/* Slot gets faster or slower for difference in calculated center
	 * X and precalculated spring x */
	slot->mSpeed += groupSpringForce (screen, centerX, slot->mSpringX);

	/* Apply dragged slot force on the other slots */
	if (draggedSlot && draggedSlot != slot)
	{
	    int draggedSlotForce;
	    draggedSlotForce =
		groupDraggedSlotForce(screen, centerX - draggedCenterX,
				      abs (centerY - draggedCenterY));

	    slot->mSpeed += draggedSlotForce;
	    slot2 = NULL;

	    /* If applying a negative force, the other slot to apply
	     * force on is the left slot, otherwise the right one.
	     * Then recursively calculate slot positions until we have
	     * reached a NULL point on the tab bar (too far left or
	     * right)
	     */

	    if (draggedSlotForce < 0)
	    {
		slot2 = slot->mPrev;
		mLeftSpeed += draggedSlotForce;
	    }
	    else if (draggedSlotForce > 0)
	    {
		slot2 = slot->mNext;
		mRightSpeed += draggedSlotForce;
	    }

	    while (slot2)
	    {
		if (slot2 != draggedSlot)
		    slot2->mSpeed += draggedSlotForce;

		slot2 = (draggedSlotForce < 0) ? slot2->mPrev : slot2->mNext;
	    }
	}
    }

    /* Apply frictions and speed limits to the tab motions */
    foreach (slot, mSlots)
    {
	groupApplyFriction (screen, &slot->mSpeed);
	groupApplySpeedLimit (screen, &slot->mSpeed);

	forces |= (slot->mSpeed != 0);
    }

    /* Apply frictions and speed limits to the left and right sides */
    groupApplyFriction (screen, &mLeftSpeed);
    groupApplySpeedLimit (screen, &mLeftSpeed);

    groupApplyFriction (screen, &mRightSpeed);
    groupApplySpeedLimit (screen, &mRightSpeed);

    forces |= (mLeftSpeed != 0 || mRightSpeed != 0);

    return forces;
}

/*
 * GroupTabBar::applySpeeds
 *
 */
void
GroupTabBar::applySpeeds (int            msSinceLastRepaint)
{
    GroupTabBarSlot *slot;
    int             move;
    CompRect	    box = mRegion.boundingRect ();
    bool            updateTabBar = false;

    /* For animation purposes we need to know how many ms there have
     * been since the last movement */

    mLeftMsSinceLastMove += msSinceLastRepaint;
    mRightMsSinceLastMove += msSinceLastRepaint;

    /* Left  - x position of the region is just the calculated
     * speed times how many ms there have been since the last move
     * (/1000) */
    move = mLeftSpeed * mLeftMsSinceLastMove / 1000;
    if (move)
    {
	box.setX (box.x () + move);
	box.setWidth (box.width () - move);

	mLeftMsSinceLastMove = 0;
	updateTabBar = true;
    }
    /* Otherwise if the left speed is zero and the region point is
     * still not the same as the left spring point, it might be
     * friction preventing us from getting to the position, so temp
     * overcome this */
    else if (mLeftSpeed == 0 &&
	     mRegion.boundingRect ().x1 () != mLeftSpringX &&
	     (SPRING_K * abs (mRegion.boundingRect ().x1 () - mLeftSpringX) <
	      FRICTION))
    {
	/* Friction is preventing from the left border to get
	   to its original position. */
	box.setX (box.x () + mLeftSpringX - mRegion.boundingRect ().x1 ());
	box.setWidth (box.width () - mLeftSpringX - mRegion.boundingRect ().x1 ());

	mLeftMsSinceLastMove = 0;
	updateTabBar = true;
    }
    else if (mLeftSpeed == 0)
	mLeftMsSinceLastMove = 0;

    /* Right */
    move = mRightSpeed * mRightMsSinceLastMove / 1000;
    if (move)
    {
	box.setWidth (box.width () + move);

	mRightMsSinceLastMove = 0;
	updateTabBar = true;
    }
    else if (mRightSpeed == 0 &&
	     mRegion.boundingRect ().x2 () != mRightSpringX &&
	     (SPRING_K * abs (mRegion.boundingRect ().x2 () - mRightSpringX) <
	      FRICTION))
    {
	/* Friction is preventing from the right border to get
	   to its original position. */
	box.setWidth (box.width () + mLeftSpringX - mRegion.boundingRect ().x1 ());

	mLeftMsSinceLastMove = 0;
	updateTabBar = true;
    }
    else if (mRightSpeed == 0)
	mRightMsSinceLastMove = 0;

    /* If we need to update the bar, then resize the actual region */
    if (updateTabBar)
	resizeTabBarRegion (box, false);

    /* Apply movement to slots */
    foreach (slot, mSlots)
    {
	int slotCenter;

	/* Move slots by precalculated speeds */
	slot->mMsSinceLastMove += msSinceLastRepaint;
	move = slot->mSpeed * slot->mMsSinceLastMove / 1000;
	slotCenter = slot->mRegion.boundingRect ().centerX ();

	if (move)
	{
	    slot->mRegion.translate (move, 0);
	    slot->mMsSinceLastMove = 0;
	}
	else if (slot->mSpeed == 0 &&
		 slotCenter != slot->mSpringX &&
		 SPRING_K * abs (slotCenter - slot->mSpringX) < FRICTION)
	{
	    /* Friction is preventing from the slot to get
	       to its original position. */

	    slot->mRegion.translate (slot->mSpringX - slotCenter, 0);
	    slot->mMsSinceLastMove = 0;
	}
	else if (slot->mSpeed == 0)
	    slot->mMsSinceLastMove = 0;
    }
}

/*
 * GroupTabBar::GroupTabBar
 *
 * Constructor for the tab bar - initialize it based on a parent group
 * and a top tab. Make sure that X windows are created  and create
 * slots for this tab bar
 *
 */
GroupTabBar::GroupTabBar (GroupSelection *group,
			  CompWindow     *topTab) :
    mSlots (CompSize (0,0), group),
    mGroup (group),
    mTopTab (NULL),
    mPrevTopTab (NULL),
    mLastTopTab (NULL),
    mNextTopTab (NULL),
    mCheckFocusAfterTabChange (false),
    mChangeAnimationTime (0),
    mChangeAnimationDirection (0),
    mChangeState (NoTabChange),
    mHoveredSlot (NULL),
    mTextSlot (NULL),
    mTextLayer (NULL),
    mBgLayer (NULL),
    mSelectionLayer (NULL),
    mState (PaintOff),
    mAnimationTime (0),
    mOldWidth (0),
    mLeftSpringX (0),
    mRightSpringX (0),
    mLeftSpeed (0),
    mRightSpeed (0),
    mLeftMsSinceLastMove (0),
    mRightMsSinceLastMove (0),
    mInputPrevention (None),
    mIpwMapped (false)
{
    mGroup->mTabBar = this; /* only need to do this because
			     * GroupTabBar::createSlot checks
			     * for mTabBar
			     */
    mGroup->mTopId = topTab->id ();

    mSlots.clear ();
    foreach (CompWindow *cw, mGroup->mWindows)
	createSlot (cw);

    createInputPreventionWindow ();
    mTopTab = GroupWindow::get (topTab)->mSlot;
    group->mTopId = topTab->id ();

    recalcTabBarPos (WIN_CENTER_X (topTab),
			  WIN_X (topTab), WIN_X (topTab) + WIN_WIDTH (topTab));
}

/*
 * GroupTabBar::~GroupTabBar
 *
 * Destructor for GroupTabBar, kill the cairo layers and kill the
 * input prevention window
 *
 */
GroupTabBar::~GroupTabBar ()
{
    while (mSlots.size ())
	deleteTabBarSlot (mSlots.front ());

    if (mTextLayer->mPixmap)
	XFreePixmap (screen->dpy (), mTextLayer->mPixmap);
    delete mTextLayer;
    delete mBgLayer;
    delete mSelectionLayer;

    mGroup->mTabBar->destroyInputPreventionWindow ();

    if (mTimeoutHandle.active ())
	mTimeoutHandle.stop ();
}

/*
 * GroupScreen::initTab
 *
 * Action to tab the windows in a group
 *
 */
bool
GroupScreen::initTab (CompAction         *action,
			   CompAction::State  state,
			   CompOption::Vector options)
{
    Window     xid;
    CompWindow *w;
    bool       allowUntab = true;

    xid = CompOption::getIntOptionNamed (options, "window", 0);
    w   = screen->findWindow (xid);
    if (!w)
	return true;

    GROUP_WINDOW (w);

    /* If the windows are selected, we can skip the "group" phase
     * and go straight on to tabbing */
    if (gw->mInSelection)
    {
	groupWindows (action, state, options);
	/* If the window was selected, we don't want to
	   untab the group, because the user probably
	   wanted to tab the selected windows. */
	allowUntab = false;
    }

    if (!gw->mGroup)
	return true;

    /* Tab the group if there is no tab bar */
    if (!gw->mGroup->mTabBar)
	gw->mGroup->tabGroup (w);
    else if (allowUntab)
	gw->mGroup->untabGroup ();

    cScreen->damageScreen ();

    return true;
}

/*
 * GroupScreen::changeTabLeft
 *
 * Action to change the top tab of the group to the tab to the left
 * of the top tab
 *
 */
bool
GroupScreen::changeTabLeft (CompAction          *action,
				 CompAction::State   state,
				 CompOption::Vector  options)
{
    Window     xid;
    CompWindow *w, *topTab;

    xid = CompOption::getIntOptionNamed (options, "window", 0);
    w   = topTab = screen->findWindow (xid);
    if (!w)
	return true;

    GROUP_WINDOW (w);

    if (!gw->mSlot || !gw->mGroup || !gw->mGroup->mTabBar ||
	!gw->mGroup->mTabBar->mTopTab)
	return true;

    if (gw->mGroup->mTabBar->mNextTopTab)
	topTab = NEXT_TOP_TAB (gw->mGroup);
    else if (gw->mGroup->mTabBar->mTopTab)
    {
	/* If there are no tabbing animations,
	   topTab is never NULL. */
	topTab = TOP_TAB (gw->mGroup);
    }

    gw = GroupWindow::get (topTab);

    if (gw->mSlot->mPrev)
	return changeTab (gw->mSlot->mPrev, GroupTabBar::RotateLeft);
    else
	return changeTab (gw->mGroup->mTabBar->mSlots.back (), GroupTabBar::RotateLeft);
}

/*
 * changeTabRight
 *
 */
bool
GroupScreen::changeTabRight (CompAction         *action,
				  CompAction::State  state,
				  CompOption::Vector options)
{
    Window     xid;
    CompWindow *w, *topTab;

    xid = CompOption::getIntOptionNamed (options, "window", 0);
    w   = topTab = screen->findWindow (xid);
    if (!w)
	return true;

    GROUP_WINDOW (w);

    if (!gw->mSlot || !gw->mGroup || !gw->mGroup->mTabBar)
	return true;

    if (gw->mGroup->mTabBar->mNextTopTab)
	topTab = NEXT_TOP_TAB (gw->mGroup);
    else if (gw->mGroup->mTabBar->mTopTab)
    {
	/* If there are no tabbing animations,
	   topTab is never NULL. */
	topTab = TOP_TAB (gw->mGroup);
    }

    gw = GroupWindow::get (topTab);

    if (gw->mSlot->mNext)
	return changeTab (gw->mSlot->mNext, GroupTabBar::RotateRight);
    else
	return changeTab (gw->mGroup->mTabBar->mSlots.front (),
			  GroupTabBar::RotateRight);
}

/*
 * switchTopTabInput
 *
 * If the IPW is created, then map it accordingly
 *
 */
void
GroupScreen::switchTopTabInput (GroupSelection *group,
				bool	    enable)
{
    if (!group->mTabBar || !HAS_TOP_WIN (group))
	return;

    if (!group->mTabBar->mInputPrevention)
	group->mTabBar->createInputPreventionWindow ();

    if (!enable)
    {
	XMapWindow (screen->dpy (),
		    group->mTabBar->mInputPrevention);

    }
    else
    {
	XUnmapWindow (screen->dpy (),
		      group->mTabBar->mInputPrevention);
    }

    group->mTabBar->mIpwMapped = !enable;
}

/*
 * GroupTabBar::createInputPreventionWindow
 *
 */
void
GroupTabBar::createInputPreventionWindow ()
{
    if (!mInputPrevention)
    {
	XSetWindowAttributes attrib;
	attrib.override_redirect = true;

	mInputPrevention =
	    XCreateWindow (screen->dpy (),
			   screen->root (), -100, -100, 1, 1, 0,
			   CopyFromParent, InputOnly,
			   CopyFromParent, CWOverrideRedirect, &attrib);

	mIpwMapped = false;
    }
}

/*
 * GroupTabBar::destroyInputPreventionWindow
 *
 */
void
GroupTabBar::destroyInputPreventionWindow ()
{
    if (mInputPrevention)
    {
	XDestroyWindow (screen->dpy (),
			mInputPrevention);

	mInputPrevention = None;
	mIpwMapped = true;
    }
}
