/**
 *
 * Compiz group plugin
 *
 * init.cpp
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


COMPIZ_PLUGIN_20090315 (group, GroupPluginVTable);

/* If this is false, then there is no point in trying to render text
 * since it will fail */
bool gTextAvailable;

/*
 * GroupScreen::optionChanged
 *
 * An option was just changed. Since we aren't constantly re-rendering
 * things like glow, the tab bar, the font, etc, we need to re-render
 * applicable things */
void
GroupScreen::optionChanged (CompOption *opt,
			    Options    num)
{
    GroupSelection *group;

    switch (num)
    {
	case GroupOptions::TabBaseColor:
	case GroupOptions::TabHighlightColor:
	case GroupOptions::TabBorderColor:
	case GroupOptions::TabStyle:
	case GroupOptions::BorderRadius:
	case GroupOptions::BorderWidth:
	    foreach (group, mGroups)
		if (group->mTabBar)
		    group->mTabBar->mBgLayer->render ();
	    break;
	case GroupOptions::TabbarFontSize:
	case GroupOptions::TabbarFontColor:
	    foreach (group, mGroups)
		if (group->mTabBar)
		{
		    group->mTabBar->mTextLayer =
		      TextLayer::rebuild (group->mTabBar->mTextLayer);

		    if (group->mTabBar->mTextLayer)
			group->mTabBar->mTextLayer->render ();
		}
	    break;
	case GroupOptions::ThumbSize:
	case GroupOptions::ThumbSpace:
	    foreach (group, mGroups)
		if (group->mTabBar)
		{
		    CompRect box = group->mTabBar->mRegion.boundingRect ();
		    group->mTabBar->recalcTabBarPos (
					 (box.x1 () + box.x2 ()) / 2,
					  box.x1 (), box.x2 ());
		}
	    break;
	case GroupOptions::Glow:
	case GroupOptions::GlowSize:
	    {
		/* We have new output extents, so update them
		 * and damage them */
		foreach (CompWindow *w, screen->windows ())
		{
		    GROUP_WINDOW (w);
		    GLTexture::Matrix tMat =
					 mGlowTexture.at (0)->matrix ();

		    gw->computeGlowQuads (&tMat);
		    if (gw->mGlowQuads)
		    {
			gw->cWindow->damageOutputExtents ();
			gw->window->updateWindowOutputExtents ();
			gw->cWindow->damageOutputExtents ();
		    }
		}
		break;
	    }
	case GroupOptions::GlowType:
	    {
		int		      glowType;
		GlowTextureProperties *glowProperty;

		/* Since we have a new glow texture, we have to rebind
		 * it and recalculate it */

		glowType = optionGetGlowType ();
		glowProperty = &mGlowTextureProperties[glowType];

		mGlowTexture = GLTexture::imageDataToTexture (
				    glowProperty->textureData,
				    CompSize (glowProperty->textureSize,
					      glowProperty->textureSize),
				    GL_RGBA, GL_UNSIGNED_BYTE);

		if (optionGetGlow () && !mGroups.empty ())
		{
		    foreach (CompWindow *w, screen->windows ())
		    {
			GLTexture::Matrix tMat = mGlowTexture.at (0)->matrix ();
			GroupWindow::get (w)->computeGlowQuads (&tMat);
		    }

		    cScreen->damageScreen ();
		}
		break;
	    }

	case GroupOptions::MoveAll:
	case GroupOptions::ResizeAll:
	case GroupOptions::MinimizeAll:
	case GroupOptions::ShadeAll:
	case GroupOptions::MaximizeUnmaximizeAll:
	case GroupOptions::RaiseAll:
	    foreach (GroupSelection *group, mGroups)
		foreach (CompWindow *w, group->mWindows)
		    GroupWindow::get (w)->checkFunctions ();

	    break;

	default:
	    break;
    }
}

/*
 * GroupScreen::applyInitialActions
 *
 * timer callback for stuff that needs to be called after all
 * screens and windows are initialized
 *
 */
bool
GroupScreen::applyInitialActions ()
{
    CompWindowList::reverse_iterator rit = screen->windows ().rbegin ();
    /* we need to do it from top to buttom of the stack to avoid problems
       with a reload of Compiz and tabbed static groups. (topTab will always
       be above the other windows in the group) */
    while (rit != screen->windows ().rend ())
    {
	CompWindow *w = *rit;

	GROUP_WINDOW (w);

	/* Otherwise, add this window to a group on it's own if we are
	 * auto-tabbing */
	if (optionGetAutotabCreate () && gw->isGroupWindow ())
	{
	    if (!gw->mGroup && (gw->mWindowState ==
				GroupWindow::WindowNormal))
	    {
		GroupSelection *g;
		mTmpSel.clear ();
		mTmpSel.select (w);
		g = mTmpSel.toGroup ();

		if (g)
		    g->tabGroup (w);
	    }
	}

	++rit;
    }

    return false;
}

/*
 * GroupScreen::checkFunctions
 *
 * Checks to enable and disable interfaced functions in GroupScreen
 * if we do or don't need them. Keeping them enabled costs CPU usage
 * so do this only when needed
 *
 */

#define HANDLE_EVENT (1 << 0)
#define GL_PAINT_OUTPUT (1 << 1)
#define GL_PAINT_TRANSFORMED_OUTPUT (1 << 2)
#define PREPARE_PAINT (1 << 3)
#define DONE_PAINT (1 << 4)

void
GroupScreen::checkFunctions ()
{
    unsigned long functionsMask = 0;

    /* We need to enable our output paint hook if we are
     * -> Painting a selection rect
     * -> Painting a dragged tab
     * -> We have some groups AND
     *    -> There is a "stretched window" OR
     *    -> We are doing the tab change animation OR
     *    -> We are doing the tabbing/untabbing animation OR
     *    -> We are painting the tab bar
     *    (Since we need to enable the
     *     PAINT_SCREEN_WITH_TRANSFORMED_WINDOWS_MASK which
     *     allows for matrix transformation of windows)
     */

    if (mGrabState == GroupScreen::ScreenGrabSelect ||
	 mGrabState == GroupScreen::ScreenGrabTabDrag)
	 functionsMask |= (GL_PAINT_OUTPUT |
			   GL_PAINT_TRANSFORMED_OUTPUT);
    else if (!mGroups.empty ())
    {
	foreach (GroupSelection *group, mGroups)
	{
	    if ((group->mTabbingState != GroupSelection::NoTabbing) ||
	        (group->mTabBar &&
		 (group->mTabBar->mChangeState == GroupTabBar::NoTabChange ||
		  group->mTabBar->mState != PaintOff)) ||
		 group->mResizeInfo)
	    {
		functionsMask |= (GL_PAINT_OUTPUT |
				  GL_PAINT_TRANSFORMED_OUTPUT);
		break;
	    }
	}
    }

     /* We need to enable preparePaint if:
      * -> There is an animation going on
      * -> There is a tab bar with slots visible and a dragged slot
      *    (since this creates forces on the other slots)
      *
      * enabling preparePaint implicitly enabled donePaint
      *
      */



    foreach (GroupSelection *group, mGroups)
    {
	if ((group->mTabbingState != GroupSelection::NoTabbing) ||
	    (group->mTabBar &&
	     (group->mTabBar->mChangeState != GroupTabBar::NoTabChange ||
	      (group->mTabBar->mState == PaintFadeIn ||
	       group->mTabBar->mState == PaintFadeOut) ||
	      (group->mTabBar->mTextLayer &&
	       (group->mTabBar->mTextLayer->mState == PaintFadeIn ||
	        group->mTabBar->mTextLayer->mState == PaintFadeOut)) ||
	      (group->mTabBar->mBgLayer &&
	       group->mTabBar->mBgLayer->mBgAnimation) ||
	      (group->mTabBar->mSlots.size () && mDraggedSlot))))
	{
	    functionsMask |= (PREPARE_PAINT | DONE_PAINT);
	    break;
	}
    }

    cScreen->preparePaintSetEnabled (this, functionsMask & PREPARE_PAINT);
    cScreen->donePaintSetEnabled (this, functionsMask & DONE_PAINT);
    gScreen->glPaintOutputSetEnabled (this, functionsMask &
					     GL_PAINT_OUTPUT);
    gScreen->glPaintTransformedOutputSetEnabled (this, functionsMask &
					   GL_PAINT_TRANSFORMED_OUTPUT);
}

/*
 * GroupScreen::GroupScreen
 *
 * Constructor for GroupScreen. Set up atoms, glow texture, queues, etc
 *
 */
GroupScreen::GroupScreen (CompScreen *s) :
    PluginClassHandler <GroupScreen, CompScreen> (s),
    cScreen (CompositeScreen::get (screen)),
    gScreen (GLScreen::get (screen)),
    mIgnoreMode (false),
    mGlowTextureProperties ((GlowTextureProperties *) glowTextureProperties),
    mLastRestackedGroup (NULL),
    mResizeNotifyAtom (XInternAtom (screen->dpy (),
				    "_COMPIZ_RESIZE_NOTIFY", 0)),
    mPendingMoves (NULL),
    mPendingGrabs (NULL),
    mPendingUngrabs (NULL),
    mQueued (false),
    mGrabState (ScreenGrabNone),
    mGrabIndex (0),
    mLastHoveredGroup (NULL),
    mDraggedSlot (NULL),
    mDragged (false),
    mPrevX (0),
    mPrevY (0),
    mLastGrabbedWindow (None)
{
    ScreenInterface::setHandler (screen);
    GLScreenInterface::setHandler (gScreen, false);
    CompositeScreenInterface::setHandler (cScreen);

    int glowType = optionGetGlowType ();
    boost::function <void (CompOption *, GroupOptions::Options)> oSetCb
	    = boost::bind (&GroupScreen::optionChanged,
					  this, _1, _2);
    /* one-shot timeout for stuff that needs to be initialized after
       all screens and windows are initialized */
    mInitialActionsTimeoutHandle.start (boost::bind (
					&GroupScreen::applyInitialActions,
					this), 0, 0);

    mDequeueTimeoutHandle.setCallback (boost::bind (
				       &GroupScreen::dequeueTimer, this));
    mDequeueTimeoutHandle.setTimes (0, 0);

    /* Bind the glow texture now */
    mGlowTexture =
    GLTexture::imageDataToTexture (mGlowTextureProperties[glowType].textureData,
				   CompSize (mGlowTextureProperties[glowType].textureSize,
					     mGlowTextureProperties[glowType].textureSize),
				   GL_RGBA, GL_UNSIGNED_BYTE);

    /* Set option callback code */
    optionSetTabHighlightColorNotify (oSetCb);
    optionSetTabBaseColorNotify (oSetCb);
    optionSetTabBorderColorNotify (oSetCb);
    optionSetTabbarFontSizeNotify (oSetCb);
    optionSetTabbarFontColorNotify (oSetCb);
    optionSetGlowNotify (oSetCb);
    optionSetGlowTypeNotify (oSetCb);
    optionSetGlowSizeNotify (oSetCb);
    optionSetTabStyleNotify (oSetCb);
    optionSetThumbSizeNotify (oSetCb);
    optionSetThumbSpaceNotify (oSetCb);
    optionSetBorderWidthNotify (oSetCb);
    optionSetBorderRadiusNotify (oSetCb);

    optionSetSelectButtonInitiate (boost::bind (&GroupScreen::select,
						     this, _1, _2, _3));
    optionSetSelectButtonTerminate (boost::bind
					(&GroupScreen::selectTerminate,
						     this, _1, _2, _3));
    optionSetSelectSingleKeyInitiate (boost::bind
					(&GroupScreen::selectSingle,
						     this, _1, _2, _3));
    optionSetGroupKeyInitiate (boost::bind
					(&GroupScreen::groupWindows,
						     this, _1, _2, _3));
    optionSetUngroupKeyInitiate (boost::bind
					(&GroupScreen::ungroupWindows,
						     this, _1, _2, _3));
    optionSetTabmodeKeyInitiate (boost::bind (&GroupScreen::initTab,
						     this, _1, _2, _3));
    optionSetChangeTabLeftKeyInitiate (boost::bind
					   (&GroupScreen::changeTabLeft,
						     this, _1, _2, _3));
    optionSetChangeTabRightKeyInitiate (boost::bind
					  (&GroupScreen::changeTabRight,
						     this, _1, _2, _3));
    optionSetRemoveKeyInitiate (boost::bind (&GroupScreen::removeWindow,
						     this, _1, _2, _3));
    optionSetCloseKeyInitiate (boost::bind (&GroupScreen::closeWindows,
						     this, _1, _2, _3));
    optionSetIgnoreKeyInitiate (boost::bind (&GroupScreen::setIgnore,
						     this, _1, _2, _3));
    optionSetIgnoreKeyTerminate (boost::bind (&GroupScreen::unsetIgnore,
						     this, _1, _2, _3));
    optionSetChangeColorKeyInitiate (boost::bind
					     (&GroupScreen::changeColor,
						     this, _1, _2, _3));

}

/*
 * GroupScreen::~GroupScreen
 *
 * Screen properties tear-down, delete all the groups, destroy IPWs
 * etc
 *
 */
GroupScreen::~GroupScreen ()
{
    if (mGroups.size ())
    {
	GroupSelection *group;
	GroupSelection::List::reverse_iterator rit = mGroups.rbegin ();

	while (rit != mGroups.rend ())
	{
	    group = *rit;

	    group->mWindows.clear ();
	    group->mWindowIds.clear ();

	    if (group->mTabBar)
	    {
		std::list <GroupTabBarSlot *>::reverse_iterator rit =
				      group->mTabBar->mSlots.rbegin ();

		/* We need to delete the slots first since otherwise
		 * the tab bar will automatically try to change to
		 * the next slot after the one that was deleted, but
		 * it can't since we have already deleted all of our
		 * window structures */
		while (rit != group->mTabBar->mSlots.rend ())
		{
		    GroupTabBarSlot *slot = *rit;
		    delete slot;
		    --rit;
		}

		group->mTabBar->mSlots.clear ();

		delete group->mTabBar;
	    }

	    delete group;
	    ++rit;
	}
    }

    mTmpSel.clear ();

    if (mGrabIndex)
	grabScreen (ScreenGrabNone);

    if (mDragHoverTimeoutHandle.active ())
	mDragHoverTimeoutHandle.stop ();

    if (mShowDelayTimeoutHandle.active ())
	mShowDelayTimeoutHandle.stop ();

    if (mDequeueTimeoutHandle.active ())
	mDequeueTimeoutHandle.stop ();

    if (mInitialActionsTimeoutHandle.active ())
	mInitialActionsTimeoutHandle.stop ();
}

/*
 * GroupWindow::checkFunctions
 *
 * Function to check if we need to enable any of our wrapped
 * functions.
 *
 */

#define GL_PAINT (1 << 0)
#define GL_DRAW (1 << 1)
#define DAMAGE_RECT (1 << 2)
#define GET_OUTPUT_EXTENTS (1 << 3)
#define MOVE_NOTIFY (1 << 4)
#define RESIZE_NOTIFY (1 << 5)
#define GRAB_NOTIFY (1 << 6)
#define WINDOW_NOTIFY (1 << 7)
#define STATECHANGE_NOTIFY (1 << 8)
#define ACTIVATE_NOTIFY (1 << 9)

void
GroupWindow::checkFunctions ()
{
    unsigned long functionsMask = 0;

    GROUP_SCREEN (screen);

    /* For glPaint, the window must either be:
     * -> In an animation (eg rotating, tabbing, etc)
     * -> Having its tab bar shown
     * -> "Selected" (but not yet grouped)
     * -> Being Stretched
     * -> Have a hide info struct (since we need to hide the window)
     */

    if (checkRotating () || checkTabbing () || checkShowTabBar ()
	|| !mResizeGeometry.isEmpty () || mWindowHideInfo
	|| mInSelection)
	functionsMask |= GL_PAINT;


    /* For glDraw, the window must be:
     * -> Window must be in a group
     * -> Window must have glow quads
     */

    if (mGroup && (mGroup->mWindows.size () > 1) && mGlowQuads)
	functionsMask |= (GL_DRAW | GET_OUTPUT_EXTENTS);

    /* For damageRect, the window must be:
     * -> Non empty resize rectangle (we need to update the resize
     * rect region of this window)
     * -> Have a slot (we need to damage the slot area)
     * Strictly speaking, we also use damageRect to check for initial
     * damages (for when a window was first painted on screen), but
     * since we don't start checking functions until after that happens
     * we dont need to worry about this case here)
     */

    if (mSlot || !mResizeGeometry.isEmpty ())
	functionsMask |= DAMAGE_RECT;

    /* For the various notification functions, the window
     * needs to be in a group, but we should disable them
     * if the options say we don't need to handle that particular
     * funciton
     */

    if (mGroup)
    {
	/* Even if we are not resizing all windows we still need
	 * to recalc the tab bar position anyways, so do that
	 */

	if (gs->optionGetResizeAll () ||
	    (mGroup->mTabBar && IS_TOP_TAB (window, mGroup)))
	    functionsMask |= RESIZE_NOTIFY;
	if (mGlowQuads || gs->optionGetMoveAll () ||
	    (mGroup->mTabBar && IS_TOP_TAB (window, mGroup)))
	    functionsMask |= MOVE_NOTIFY;
	if (gs->optionGetMaximizeUnmaximizeAll ())
	    functionsMask |= STATECHANGE_NOTIFY;
	if (gs->optionGetRaiseAll ())
	    functionsMask |= ACTIVATE_NOTIFY;

	functionsMask |= WINDOW_NOTIFY;
    }


    gWindow->glPaintSetEnabled (this, functionsMask & GL_PAINT);
    gWindow->glDrawSetEnabled (this, functionsMask & GL_DRAW);
    cWindow->damageRectSetEnabled (this, functionsMask & DAMAGE_RECT);
    window->getOutputExtentsSetEnabled (this, functionsMask &
					      GET_OUTPUT_EXTENTS);
    window->resizeNotifySetEnabled (this, functionsMask &
					  RESIZE_NOTIFY);
    window->moveNotifySetEnabled (this, functionsMask & MOVE_NOTIFY);
    window->stateChangeNotifySetEnabled (this, functionsMask &
					        STATECHANGE_NOTIFY);
    window->activateSetEnabled (this, functionsMask & ACTIVATE_NOTIFY);
    window->windowNotifySetEnabled (this, functionsMask & WINDOW_NOTIFY);
}

/*
 * GroupWindow::GroupWindow
 *
 * Constructor for GroupWindow, set up the hide info, animation state
 * resize geometry, glow quads etc
 *
 */
GroupWindow::GroupWindow (CompWindow *w) :
    PluginClassHandler <GroupWindow, CompWindow> (w),
    window (w),
    cWindow (CompositeWindow::get (w)),
    gWindow (GLWindow::get (w)),
    mGroup (NULL),
    mInSelection (false),
    mSlot (NULL),
    mNeedsPosSync (false),
    mGlowQuads (NULL),
    mWindowHideInfo (NULL),
    mResizeGeometry (CompRect (0, 0, 0, 0)),
    mAnimateState (0),
    mTx (0.0f),
    mTy (0.0f),
    mXVelocity (0.0f),
    mYVelocity (0.0f)
{
    GLTexture::Matrix mat;

    GROUP_SCREEN (screen);

    mat = gs->mGlowTexture.front ()->matrix ();

    WindowInterface::setHandler (window, false);
    CompositeWindowInterface::setHandler (cWindow, true);
    GLWindowInterface::setHandler (gWindow, false);

    window->grabNotifySetEnabled (this, true);
    window->ungrabNotifySetEnabled (this, true);

    mOrgPos = CompPoint (0, 0);
    mMainTabOffset = CompPoint (0, 0);
    mDestination = CompPoint (0, 0);

    if (w->minimized ())
	mWindowState = WindowMinimized;
    else if (w->shaded ())
	mWindowState = WindowShaded;
    else
	mWindowState = WindowNormal;

    computeGlowQuads (&mat);
}

/*
 * GroupWindow::~GroupWindow
 *
 * Tear down for when we don't need group data on a window anymore
 *
 */
GroupWindow::~GroupWindow ()
{
    if (mWindowHideInfo)
	setWindowVisibility (true);

    if (mGlowQuads)
	delete[] mGlowQuads;
}

bool
GroupPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION) ||
	!CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI) ||
        !CompPlugin::checkPluginABI ("opengl", COMPIZ_OPENGL_ABI) ||
	!CompPlugin::checkPluginABI ("mousepoll", COMPIZ_MOUSEPOLL_ABI))
	return false;

    if (!CompPlugin::checkPluginABI ("text", COMPIZ_TEXT_ABI))
	gTextAvailable = false;
    else
	gTextAvailable = true;

    return true;
}
