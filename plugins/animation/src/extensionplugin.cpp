/*
 * Animation plugin for compiz/beryl
 *
 * animation.c
 *
 * Copyright : (C) 2006 Erkin Bahceci
 * E-mail    : erkinbah@gmail.com
 *
 * Based on Wobbly and Minimize plugins by
 *           : David Reveman
 * E-mail    : davidr@novell.com>
 *
 * Particle system added by : (C) 2006 Dennis Kasprzyk
 * E-mail                   : onestone@beryl-project.org
 *
 * Beam-Up added by : Florencio Guimaraes
 * E-mail           : florencio@nexcorp.com.br
 *
 * Hexagon tessellator added by : Mike Slegeir
 * E-mail                       : mikeslegeir@mail.utexas.edu>
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
 */

#include "private.h"


ExtensionPluginAnimation::ExtensionPluginAnimation
    (const CompString &name,
     unsigned int nEffects,
     AnimEffect *effects,
     CompOption::Vector *effectOptions,
     unsigned int firstEffectOptionIndex) :
    ExtensionPluginInfo (name, nEffects, effects, effectOptions,
			 firstEffectOptionIndex),
    mAWinWasRestackedJustNow (false),
    mRestackAnimCount (0)
{
}

ExtensionPluginAnimation::~ExtensionPluginAnimation ()
{
}

void
ExtensionPluginAnimation::postPreparePaintGeneral ()
{
    if (mAWinWasRestackedJustNow)
	mAWinWasRestackedJustNow = false;
}

void
ExtensionPluginAnimation::cleanUpParentChildChainItem (AnimWindow *aw)
{
    PersistentDataMap::iterator itData =
	aw->persistentData.find ("restack");
    if (itData != aw->persistentData.end ()) // if found
    {
	RestackPersistentData *restackData =
	    static_cast<RestackPersistentData *> (itData->second);

	if (restackData->mWinThisIsPaintedBefore &&
	    !restackData->mWinThisIsPaintedBefore->destroyed ())
	{
	    RestackPersistentData *dataOther = static_cast<RestackPersistentData *>
		(AnimWindow::get (restackData->mWinThisIsPaintedBefore)->
		 persistentData["restack"]);
	    if (dataOther)
		dataOther->mWinToBePaintedBeforeThis = 0;
	}
	restackData->mWinThisIsPaintedBefore = 0;
	restackData->mMoreToBePaintedPrev = 0;
	restackData->mMoreToBePaintedNext = 0;
    }

    itData = aw->persistentData.find ("dodge");
    if (itData != aw->persistentData.end ()) // if found
    {
	DodgePersistentData *dodgeData =
	    static_cast<DodgePersistentData *> (itData->second);

	dodgeData->isDodgeSubject = false;
	dodgeData->skipPostPrepareScreen = false;
    }
}

bool
ExtensionPluginAnimation::paintShouldSkipWindow (CompWindow *w)
{
    AnimWindow *aw = AnimWindow::get (w);
    PersistentDataMap::iterator itData = aw->persistentData.find ("restack");
    if (itData != aw->persistentData.end ()) // if found
    {
	RestackPersistentData *data =
	    static_cast<RestackPersistentData *> (itData->second);

	// Increment (glPaint) visit count
	data->mVisitCount++;

	// If the window is (to be) painted somewhere other than in its
	// original stacking order, we don't need to paint it now.
	if (aw->curAnimation ()->info ()->isRestackAnim &&
	    dynamic_cast<RestackAnim *> (aw->curAnimation ())->paintedElsewhere ())
	    return true;
    }
    return false;
}

/// Returns whether this window is relevant for fade focus.
bool
ExtensionPluginAnimation::relevantForRestackAnim (CompWindow *w)
{
    unsigned int wmType = w->wmType ();
    if (!((wmType &
	   // these two are to be used as "host" windows
	   // to host the painting of windows being focused
	   // at a stacking order lower than them
	   (CompWindowTypeDockMask | CompWindowTypeSplashMask)) ||
	  wmType == CompWindowTypeNormalMask ||
	  wmType == CompWindowTypeDialogMask ||
	  wmType == CompWindowTypeUtilMask ||
	  wmType == CompWindowTypeUnknownMask))
    {
	return false;
    }
    return !w->destroyed ();
}

void
ExtensionPluginAnimation::prePreparePaintGeneral ()
{
    if (!mAWinWasRestackedJustNow)
	return;

    bool focusAnimInitiated = false;
    AnimScreen *as = AnimScreen::get (::screen);

    // Go in reverse order so that restack chains are handled properly
    for (CompWindowVector::reverse_iterator rit = mLastClientList.rbegin ();
	 rit != mLastClientList.rend (); ++rit)
    {
	CompWindow *w = (*rit);
	AnimWindow *aw = AnimWindow::get (w);
	RestackPersistentData *data = static_cast<RestackPersistentData *>
	    (aw->persistentData["restack"]);
	if (!data)
	    continue;
	RestackInfo *restackInfo = data->restackInfo ();
	if (!restackInfo)
	    continue;

	data->mIsSecondary = false;

	if (as->otherPluginsActive () ||
	    // Don't initiate focus anim for current dodgers
	    aw->curAnimation () ||
	    // Don't initiate focus anim for windows being passed thru
	    data->mWinPassingThrough ||
	    // Don't animate with stale restack info
	    !restackInfoStillGood (restackInfo))
	{
	    data->resetRestackInfo (true);
	    continue;
	}

	// Find the first window at a higher stacking order than w
	CompWindow *nw;
	for (nw = w->next; nw; nw = nw->next)
	{
	    if (relevantForRestackAnim (nw))
		break;
	}

	// If w is being lowered, there has to be a window
	// at a higher stacking position than w (like a panel)
	// which this w's copy can be painted before.
	// Otherwise the animation will only show w fading in
	// rather than 2 copies of it cross-fading.
	if (!restackInfo->raised && !nw)
	{
	    // Free unnecessary restackInfo
	    data->resetRestackInfo (true);
	    continue;
	}

	// Check if above window is focus-fading/dodging too.
	// (like a dialog of an app. window)
	// If so, focus-fade/dodge this together with the one above
	// (link to it)
	if (nw)
	{
	    RestackPersistentData *dataNext =
		static_cast<RestackPersistentData *>
		(AnimWindow::get (nw)->persistentData["restack"]);

	    if (dataNext && dataNext->restackInfo () &&
		wontCreateCircularChain (w, nw))
	    {
	    	// Link the two
		dataNext->mMoreToBePaintedPrev = w;
		data->mMoreToBePaintedNext = nw;

		// so far, bottommost on chain
		data->mMoreToBePaintedPrev = 0;
	    }
	}
	else
	{
	    // Reset chain connections as this is not (yet) on a chain
	    data->mMoreToBePaintedNext = 0;
	    data->mMoreToBePaintedPrev = 0;
	}
    }

    // Now initiate focus animations (after the restack chains are formed
    // right above)
    for (CompWindowVector::reverse_iterator rit = mLastClientList.rbegin ();
	 rit != mLastClientList.rend (); ++rit)
    {
	CompWindow *w = (*rit);
	AnimWindow *aw = AnimWindow::get (w);
	RestackPersistentData *data = static_cast<RestackPersistentData *>
	    (aw->persistentData["restack"]);
	if (!data)
	    continue;

	RestackInfo *restackInfo = data->restackInfo ();
	if (restackInfo)
	{
	    if (as->initiateFocusAnim (aw))
		focusAnimInitiated = true;
	    else
		data->resetRestackInfo (true);
	}
    }

    if (!focusAnimInitiated)
	resetStackingInfo ();

    if (!focusAnimInitiated ||
	as->otherPluginsActive () ||
	!as->isAnimEffectPossible (AnimEffectDodge)) // Only dodge stuff below
	return;

    // Calculate dodge amounts
    foreach (CompWindow *w, mLastClientList)
    {
	AnimWindow *aw = AnimWindow::get (w);
	Animation *curAnim = aw->curAnimation ();
	if (!curAnim || curAnim->info () != AnimEffectDodge)
	    continue;

	// Only process subjects with a dodge chain
	DodgePersistentData *dodgeData = static_cast<DodgePersistentData *>
		(aw->persistentData["dodge"]);
	if (!dodgeData || !dodgeData->dodgeChainStart ||
	    !dodgeData->isDodgeSubject)
	    continue;

	dynamic_cast<DodgeAnim *> (curAnim)->calculateDodgeAmounts ();
    }

    // TODO consider removing this loop and skipPostPrepareScreen
    for (CompWindowVector::reverse_iterator rit = mLastClientList.rbegin ();
	 rit != mLastClientList.rend (); ++rit)
    {
	CompWindow *w = (*rit);
	AnimWindow *aw = AnimWindow::get (w);
	PersistentDataMap::iterator itData = aw->persistentData.find ("dodge");
	if (itData == aw->persistentData.end ()) // if not found
	    continue;

	DodgePersistentData *data = static_cast<DodgePersistentData *>
	    (itData->second);
	if (!data->isDodgeSubject)
	    continue;

	bool dodgersAreOnlySubjects = true;
	CompWindow *dw;
	DodgePersistentData *dataDodger;
	for (dw = data->dodgeChainStart; dw;
	     dw = dataDodger->dodgeChainNext)
	{
	    dataDodger = static_cast<DodgePersistentData *>
		(AnimWindow::get (dw)->persistentData["dodge"]);
	    if (!dataDodger)
		break;
	    if (!dataDodger->isDodgeSubject)
		dodgersAreOnlySubjects = false;
	}
	if (dodgersAreOnlySubjects)
	    data->skipPostPrepareScreen = true;
    }
}

void
ExtensionPluginAnimation::handleRestackNotify (AnimWindow *aw)
{
    const CompWindowVector &clients = ::screen->clientList ();

    // Only handle restack notifies when the window is (or was) on the client
    // list (i.e. not for menus, combos, etc.).
    if (find (clients.begin (), clients.end (), aw->mWindow) ==
	    clients.end () &&
    	find (mLastClientList.begin (), mLastClientList.end (), aw->mWindow) ==
    	    mLastClientList.end ())
	return;

    bool winOpenedClosed = false;
    unsigned int n = clients.size ();

    if (n != mLastClientList.size ())
    {
	winOpenedClosed = true;
    }
    // if restacking occurred and not window open/close
    if (!winOpenedClosed)
    {
	RestackPersistentData *data = static_cast<RestackPersistentData *>
	    (aw->persistentData["restack"]);
	data->mConfigureNotified = true;

	// Find which window is restacked
	// e.g. here 8507730 was raised:
	// 54526074 8507730 48234499 14680072 6291497
	// 54526074 48234499 14680072 8507730 6291497
	// compare first changed win. of row 1 with last
	// changed win. of row 2, and vica versa
	// the matching one is the restacked one
	CompWindow *wRestacked = 0;
	CompWindow *wStart = 0;
	CompWindow *wEnd = 0;
	CompWindow *wOldAbove = 0;
	CompWindow *wChangeStart = 0;
	CompWindow *wChangeEnd = 0;

	bool raised = false;
	int changeStart = -1;
	int changeEnd = -1;

	for (unsigned int i = 0; i < n; i++)
	{
	    CompWindow *wi = clients[i];

	    // skip if minimized (prevents flashing problem)
	    if (!wi || wi->destroyed ())
		continue;

	    // TODO find another filter criteria for Group plugin
	    // because some apps like gedit sets its open dialog
	    // to skip taskbar too, which shouldn't be ignored here.
	    /*
	    // skip if (tabbed and) hidden by Group plugin
	    // unless it's a dock/panel
	    if (!(wi->wmType () & CompWindowTypeDockMask) &&
		(wi->state () & (CompWindowStateSkipPagerMask |
				 CompWindowStateSkipTaskbarMask)))
		continue;
	    */
	    if (wi != mLastClientList[i])
	    {
		if (changeStart < 0)
		{
		    changeStart = (int)i;
		    wChangeStart = wi; // make use of already found w
		}
		else
		{
		    changeEnd = (int)i;
		    wChangeEnd = wi;
		}
	    }
	    else if (changeStart >= 0) // found some change earlier
		break;
	}

	// if restacking occurred
	if (changeStart >= 0 && changeEnd >= 0)
	{
	    // if we have only 2 windows changed,
	    // choose the one clicked on
	    bool preferRaised = false;
	    bool onlyTwo = false;

	    if (wChangeEnd &&
		clients[(unsigned)changeEnd] ==
		mLastClientList[(unsigned)changeStart] &&
		clients[(unsigned)changeStart] ==
		mLastClientList[(unsigned)changeEnd])
	    {
		// Check if the window coming on top was
		// mConfigureNotified (clicked on)
		RestackPersistentData *data =
		    static_cast<RestackPersistentData *>
		    (AnimWindow::get (wChangeEnd)->
		     persistentData["restack"]);
		if (data->mConfigureNotified)
		    preferRaised = true;

		onlyTwo = true;
	    }
	    // Clear all mConfigureNotified's
	    foreach (CompWindow *w2, CompositeScreen::get (::screen)->getWindowPaintList ())
	    {
		RestackPersistentData *data =
		    static_cast<RestackPersistentData *>
		    (AnimWindow::get (w2)->persistentData["restack"]);
		data->mConfigureNotified = false;
	    }

	    if (preferRaised ||
		(!onlyTwo &&
		 clients[(unsigned)changeEnd] ==
		 mLastClientList[(unsigned)changeStart]))
	    {
		// raised
		raised = true;
		wRestacked = wChangeEnd;
		wStart = wChangeStart;
		wEnd = wRestacked;
		wOldAbove = wStart;
	    }
	    else if ((unsigned int)changeEnd < n - 1 &&
		     clients[(unsigned)changeStart] ==
		     mLastClientList[(unsigned)changeEnd]) // lowered
		     // We don't animate lowering if there is no
		     // window above this window, since this window needs
		     // to be drawn on such a "host" in animPaintWindow
		     // (at least for now).
	    {
		wRestacked = wChangeStart;
		wStart = wRestacked;
		wEnd = wChangeEnd;
		wOldAbove = mLastClientList[(unsigned)(changeEnd + 1)];
	    }
	    for (; wOldAbove; wOldAbove = wOldAbove->next)
	    {
		if (!wOldAbove->destroyed ())
		    break;
	    }
	}

	if (wRestacked && wStart && wEnd && wOldAbove)
	{
	    AnimWindow *awRestacked = AnimWindow::get (wRestacked);
	    RestackPersistentData *data = static_cast<RestackPersistentData *>
		(awRestacked->persistentData["restack"]);
	    {
		data->setRestackInfo (wRestacked, wStart, wEnd, wOldAbove,
				      raised);
		mAWinWasRestackedJustNow = true;
	    }
	}
    }

    updateLastClientList ();
}

void
ExtensionPluginAnimation::updateLastClientList ()
{
    mLastClientList = ::screen->clientList ();
}

/// Returns true if linking wCur to wNext would not result
/// in a circular chain being formed.
bool
ExtensionPluginAnimation::wontCreateCircularChain (CompWindow *wCur,
						   CompWindow *wNext)
{
    RestackPersistentData *dataNext = 0;

    while (wNext)
    {
	if (wNext == wCur) // would form circular chain
	    return false;

	dataNext = static_cast<RestackPersistentData *>
	    (AnimWindow::get (wNext)->persistentData["restack"]);

	if (!dataNext)
	    return false;

	wNext = dataNext->mMoreToBePaintedNext;
    }
    return true;
}

void
ExtensionPluginAnimation::postUpdateEventEffects (AnimEvent e,
						  bool forRandom)
{
    AnimScreen *as = AnimScreen::get (::screen);

    // If a restacking anim. is (now) possible
    if (e == AnimEventFocus)
    {
	if (as->isRestackAnimPossible ())
	{
	    // Update the stored window list so that we have an up-to-date list,
	    // since that list wasn't updated while a restacking animation
	    // was not possible.
	    updateLastClientList ();

	    foreach (CompWindow *w, CompositeScreen::get (::screen)->getWindowPaintList ())
	    {
		AnimWindow *aw = AnimWindow::get (w);
		// Allocate persistent restack data if it doesn't already exist
		if (aw->persistentData.find ("restack") !=
		    aw->persistentData.end ())
		    continue;
		aw->persistentData["restack"] = new RestackPersistentData ();
	    }
	}
	if (as->isAnimEffectPossible (AnimEffectDodge))
	{
	    foreach (CompWindow *w, CompositeScreen::get (::screen)->getWindowPaintList ())
	    {
		AnimWindow *aw = AnimWindow::get (w);
		// Allocate persistent dodge data if it doesn't already exist
		if (aw->persistentData.find ("dodge") !=
		    aw->persistentData.end ())
		    continue;
		aw->persistentData["dodge"] = new DodgePersistentData ();
	    }
	}
    }
}

void
ExtensionPluginAnimation::initPersistentData (AnimWindow *aw)
{
    AnimScreen *as = AnimScreen::get (::screen);

    // TODO: Optimize (via caching isRestackAnimPossible, isAnimEffectPossible)

    // Only allocate restack data when restack animation is possible
    if (as->isRestackAnimPossible () &&
	// doesn't exist yet
	aw->persistentData.find ("restack") == aw->persistentData.end ())
    {
	aw->persistentData["restack"] = new RestackPersistentData ();
    }
    if (as->isAnimEffectPossible (AnimEffectDodge) &&
	// doesn't exist yet
	aw->persistentData.find ("dodge") == aw->persistentData.end ())
    {
	aw->persistentData["dodge"] = new DodgePersistentData ();
    }
    if (aw->persistentData.find ("multi") == aw->persistentData.end ())
    {
	aw->persistentData["multi"] = new MultiPersistentData ();
    }
}

void
ExtensionPluginAnimation::destroyPersistentData (AnimWindow *aw)
{
    aw->deletePersistentData ("restack");
    aw->deletePersistentData ("dodge");
}

void
ExtensionPluginAnimation::incrementCurRestackAnimCount ()
{
    mRestackAnimCount++;

    // Enable custom paint list when there is now a restack anim happening
    if (mRestackAnimCount == 1)
	AnimScreen::get (::screen)->enableCustomPaintList (true);
}

void
ExtensionPluginAnimation::decrementCurRestackAnimCount ()
{
    mRestackAnimCount--;

    // Disable custom paint list when there is no more a restack anim happening
    if (mRestackAnimCount == 0)
	AnimScreen::get (::screen)->enableCustomPaintList (false);
}

bool
ExtensionPluginAnimation::restackInfoStillGood (RestackInfo *restackInfo)
{
    bool wStartGood = false;
    bool wEndGood = false;
    bool wOldAboveGood = false;
    bool wRestackedGood = false;

    foreach (CompWindow *w, CompositeScreen::get (::screen)->getWindowPaintList ())
    {
	AnimWindow *aw = AnimWindow::get (w);

	if (aw->mWindow->destroyed ())
	    continue;

	if (restackInfo->wStart == w)
	    wStartGood = true;
	if (restackInfo->wEnd == w)
	    wEndGood = true;
	if (restackInfo->wRestacked == w)
	    wRestackedGood = true;
	if (restackInfo->wOldAbove == w)
	    wOldAboveGood = true;
    }
    return (wStartGood && wEndGood && wOldAboveGood && wRestackedGood);
}

/// Resets stacking related info.
void
ExtensionPluginAnimation::resetStackingInfo ()
{
    foreach (CompWindow *w, CompositeScreen::get (::screen)->getWindowPaintList ())
    {
	AnimWindow *aw = AnimWindow::get (w);
	PersistentDataMap::iterator itData =
	    aw->persistentData.find ("restack");
	if (itData != aw->persistentData.end ()) // if found
	{
	    RestackPersistentData *data =
		static_cast<RestackPersistentData *> (itData->second);
	    data->mConfigureNotified = false;
	    if (data->restackInfo ())
		data->resetRestackInfo ();
	}
    }
}

void
ExtensionPluginAnimation::postStartupCountdown ()
{
    updateLastClientList ();
}

void
ExtensionPluginAnimation::preInitiateOpenAnim (AnimWindow *aw)
{
    // Only do when the window is on the client list
    // (i.e. not for menus, combos, etc.).
    if (find (::screen->clientList ().begin (),
	::screen->clientList ().end (), aw->mWindow) !=
	::screen->clientList ().end ())
    {
    	resetStackingInfo ();
    	updateLastClientList ();
    }
}

void
ExtensionPluginAnimation::preInitiateCloseAnim (AnimWindow *aw)
{
    preInitiateOpenAnim (aw);
}

void
ExtensionPluginAnimation::preInitiateMinimizeAnim (AnimWindow *aw)
{
    preInitiateOpenAnim (aw);
}

void
ExtensionPluginAnimation::preInitiateUnminimizeAnim (AnimWindow *aw)
{
    preInitiateOpenAnim (aw);
}

void
ExtensionPluginAnimation::cleanUpAnimation (bool closing,
					    bool destructing)
{
    if (closing || destructing)
	updateLastClientList ();
}

/// Go to the bottommost window in this "focus chain"
/// This chain is used to handle some cases: e.g when Find dialog
/// of an app is open, both windows should be faded when the Find
/// dialog is raised.
CompWindow *
ExtensionPluginAnimation::getBottommostInExtendedFocusChain (CompWindow *wStartPoint)
{
    if (!wStartPoint)
	return 0;

    RestackPersistentData *data = static_cast<RestackPersistentData *>
	(AnimWindow::get (wStartPoint)->persistentData["restack"]);
    CompWindow *wBottommost = data->mWinToBePaintedBeforeThis;

    if (!wBottommost || wBottommost->destroyed ())
	return wStartPoint;

    RestackPersistentData *dataBottommost = static_cast<RestackPersistentData *>
	(AnimWindow::get (wBottommost)->persistentData["restack"]);
    CompWindow *wPrev = dataBottommost->mMoreToBePaintedPrev;
    while (wPrev)
    {
	wBottommost = wPrev;
	RestackPersistentData *dataPrev = static_cast<RestackPersistentData *>
	    (AnimWindow::get (wPrev)->persistentData["restack"]);
	wPrev = dataPrev->mMoreToBePaintedPrev;
    }
    return wBottommost;
}

/// Finds the bottommost subject in restack chain,
/// simpler version of getBottommostInExtendedFocusChain.
CompWindow *
ExtensionPluginAnimation::getBottommostInRestackChain (CompWindow *wStartPoint)
{
    CompWindow *wBottommost = wStartPoint;
    RestackPersistentData *dataCur;
    for (CompWindow *wCur = wStartPoint; wCur;
	 wCur = dataCur->mMoreToBePaintedPrev)
    {
	wBottommost = wCur;
	dataCur = static_cast<RestackPersistentData *>
	    (AnimWindow::get (wCur)->persistentData["restack"]);
	if (!dataCur)
	    break;
    }
    return wBottommost;
}

void
ExtensionPluginAnimation::resetMarks ()
{
    foreach (CompWindow *w, CompositeScreen::get (::screen)->getWindowPaintList ())
    {
	RestackPersistentData *data = static_cast<RestackPersistentData *>
	    (AnimWindow::get (w)->persistentData["restack"]);
	data->mWalkerOverNewCopy = false;
	data->mVisitCount = 0;
    }
}

void
ExtensionPluginAnimation::prePaintWindowsBackToFront ()
{
    resetMarks ();
}

CompWindow *
ExtensionPluginAnimation::walkFirst ()
{
    resetMarks ();

    CompWindow *w =
    	getBottommostInExtendedFocusChain (*CompositeScreen::get (::screen)->getWindowPaintList ().begin ());
    if (w)
    {
	RestackPersistentData *data = static_cast<RestackPersistentData *>
	    (AnimWindow::get (w)->persistentData["restack"]);
	data->mVisitCount++;
    }
    return w;
}

bool
ExtensionPluginAnimation::markNewCopy (CompWindow *w)
{
    RestackPersistentData *data = static_cast<RestackPersistentData *>
	(AnimWindow::get (w)->persistentData["restack"]);

    // if window is in a focus chain
    if (data->mWinThisIsPaintedBefore ||
	data->mMoreToBePaintedPrev)
    {
	data->mWalkerOverNewCopy = true;
	return true;
    }
    return false;
}

CompWindow *
ExtensionPluginAnimation::walkNext (CompWindow *w)
{
    RestackPersistentData *data = static_cast<RestackPersistentData *>
	(AnimWindow::get (w)->persistentData["restack"]);

    CompWindow *wRet = 0;

    if (!data->mWalkerOverNewCopy)
    {
	// Within a chain? (not the 1st or 2nd window)
	if (data->mMoreToBePaintedNext)
	{
	    wRet = data->mMoreToBePaintedNext;
	}
	else if (data->mWinThisIsPaintedBefore) // 2nd one in chain?
	{
	    wRet = data->mWinThisIsPaintedBefore;
	}
    }
    else
	data->mWalkerOverNewCopy = false;

    if (!wRet && w->next && markNewCopy (w->next))
    {
	wRet = w->next;
    }
    else if (!wRet)
    {
	wRet = getBottommostInExtendedFocusChain (w->next);
    }

    if (wRet)
    {
	RestackPersistentData *dataRet = static_cast<RestackPersistentData *>
	    (AnimWindow::get (wRet)->persistentData["restack"]);

	// Prevent cycles, which cause freezes
	if (dataRet->mVisitCount > 1) // each window is visited at most twice
	    return 0;
	dataRet->mVisitCount++;
    }
    return wRet;
}

const CompWindowList &
ExtensionPluginAnimation::getWindowPaintList ()
{
    mWindowList.clear ();
    for (CompWindow *w = walkFirst (); w; w = walkNext (w))
	mWindowList.push_back (w);

    return mWindowList;
}

