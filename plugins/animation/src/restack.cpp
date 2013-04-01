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

// =====================  Restack  =========================

RestackAnim::RestackAnim (CompWindow *w,
			  WindowEvent curWindowEvent,
			  float duration,
			  const AnimEffect info,
			  const CompRect &icon) :
    Animation::Animation (w, curWindowEvent, duration, info, icon)
{
    mRestackData = static_cast<RestackPersistentData *>
    	(AnimWindow::get (w)->persistentData["restack"]);
}

void
RestackAnim::cleanUp (bool closing, bool destructing)
{
    if (mRestackData->restackInfo ())
	mRestackData->resetRestackInfo ();

    bool thereIsUnfinishedChainElem = false;

    // Look for still playing windows in parent-child chain
    CompWindow *wCur = mRestackData->mMoreToBePaintedNext;
    while (wCur)
    {
	AnimWindow *awCur = AnimWindow::get (wCur);

	if (awCur->curAnimation () &&
	    awCur->curAnimation ()->remainingTime () > 0)
	{
	    thereIsUnfinishedChainElem = true;
	    break;
	}
	RestackPersistentData *dataCur = static_cast<RestackPersistentData *>
	    (awCur->persistentData["restack"]);
	wCur = dataCur->mMoreToBePaintedNext;
    }
    if (!thereIsUnfinishedChainElem)
    {
	wCur = mRestackData->mMoreToBePaintedPrev;
	while (wCur)
	{
	    AnimWindow *awCur = AnimWindow::get (wCur);

	    if (awCur->curAnimation () &&
		awCur->curAnimation ()->remainingTime () > 0)
	    {
		thereIsUnfinishedChainElem = true;
		break;
	    }
	    RestackPersistentData *dataCur =
		static_cast<RestackPersistentData *>
		(awCur->persistentData["restack"]);
	    wCur = dataCur->mMoreToBePaintedPrev;
	}
    }

    if (closing || destructing || !thereIsUnfinishedChainElem)
    {
	// Finish off all windows in parent-child chain
	CompWindow *wCur = mRestackData->mMoreToBePaintedNext;
	while (wCur)
	{
	    AnimWindow *awCur = AnimWindow::get (wCur);
	    RestackPersistentData *dataCur =
		static_cast<RestackPersistentData *>
		(awCur->persistentData["restack"]);
	    wCur = dataCur->mMoreToBePaintedNext;
	    static_cast<ExtensionPluginAnimation *>
		(getExtensionPluginInfo ())->cleanUpParentChildChainItem (awCur);
	}
	wCur = mWindow;
	while (wCur)
	{
	    AnimWindow *awCur = AnimWindow::get (wCur);
	    RestackPersistentData *dataCur =
		static_cast<RestackPersistentData *>
		(awCur->persistentData["restack"]);
	    wCur = dataCur->mMoreToBePaintedPrev;
	    static_cast<ExtensionPluginAnimation *>
		(getExtensionPluginInfo ())->cleanUpParentChildChainItem (awCur);
	}
    }

    ExtensionPluginAnimation *extPlugin =
	static_cast<ExtensionPluginAnimation *> (getExtensionPluginInfo ());
    extPlugin->decrementCurRestackAnimCount ();
}

bool
RestackAnim::initiateRestackAnim (int duration)
{
    CompWindow *wStart = 0;
    CompWindow *wEnd = 0;
    CompWindow *wOldAbove = 0;

    if (!mRestackData)
    	return false;

    ExtensionPluginAnimation *extPlugin =
	static_cast<ExtensionPluginAnimation *> (getExtensionPluginInfo ());
    extPlugin->incrementCurRestackAnimCount ();

    // If a focus chain (application with open dialog, etc.) is the subject,
    // in compiz++, their order changes during restack (which wasn't the case
    // in compiz 0.8. e.g: (subject chain: a b, dodger: x) (a: secondary)
    // a b x
    // b x a
    // x a b
    if (mRestackData->mIsSecondary)
    {
    	if (!mRestackData->mMoreToBePaintedNext)
    	    return false;

    	AnimWindow *awAbove =
    	    AnimWindow::get (mRestackData->mMoreToBePaintedNext);
    	RestackPersistentData *dataAbove = static_cast<RestackPersistentData *>
    	    (awAbove->persistentData["restack"]);

	mTotalTime = awAbove->curAnimation ()->totalTime ();
	mRemainingTime = mTotalTime;

	if (dataAbove && dataAbove->mWinThisIsPaintedBefore)
	{
	    // Host this subject instead, on the above subject's host
	    mRestackData->getHostedOnWin (mWindow,
	    				  dataAbove->mWinThisIsPaintedBefore);
	}
	// do basic secondary subject initialization
	postInitiateRestackAnim (0, 0, 0, 0, false);

	return true; // We're done here
    }

    RestackInfo *restackInfo = mRestackData->restackInfo ();
    bool raised = true;

    if (restackInfo)
    {
	wStart = restackInfo->wStart;
	wEnd = restackInfo->wEnd;
	wOldAbove = restackInfo->wOldAbove;
	raised = restackInfo->raised;
    }

    // Find union region of all windows that will be
    // faded through by w. If the region is empty, don't
    // run focus fade effect.

    CompRegion fadeRegion;

    int numSelectedCandidates = 0;

    CompRegion subjectsRegion (unionRestackChain (mWindow));

    // Compute subject win. region

    // wCand: Dodge or Focus fade candidate window
    for (CompWindow *wCand = wStart; wCand && wCand != wEnd->next;
	 wCand = wCand->next)
    {
	RestackPersistentData *dataCand = static_cast<RestackPersistentData *>
	    (AnimWindow::get (wCand)->persistentData["restack"]);
	if (!extPlugin->relevantForRestackAnim (wCand))
	    continue;

	// Skip windows that have been restacked
	if (wCand != wEnd && dataCand->restackInfo ())
	    continue;

	if (wCand->minimized ())
	    continue;

	if (!CompositeWindow::get (wCand)->pixmap ())
	    continue;

	if (onSameRestackChain (mWindow, wCand))
	    continue;

	// Compute intersection of this (wCand) with subject
	CompRegion candidateWinRegion (wCand->borderRect ());
	CompRegion candidateAndSubjectIntersection
	    (candidateWinRegion.intersected (subjectsRegion));
	fadeRegion += candidateAndSubjectIntersection;

	if (!candidateAndSubjectIntersection.isEmpty ())
	    processCandidate (wCand, mWindow, candidateAndSubjectIntersection,
			      numSelectedCandidates);
    }

    if (fadeRegion.isEmpty ())
    {
	// empty intersection -> won't be drawn
	return false;
    }
    if (wOldAbove)
    {
	// Store this window in the next window
	// so that this is drawn before that, i.e. in its old place
	mRestackData->getHostedOnWin (mWindow, wOldAbove);
    }

    postInitiateRestackAnim (numSelectedCandidates, duration,
			     wStart, wEnd, raised);

    // Handle other subjects down the chain if there are any
    if (mRestackData->mMoreToBePaintedPrev)
    {
	RestackPersistentData *dataCur;
	for (CompWindow *wCur = mRestackData->mMoreToBePaintedPrev; wCur;
	     wCur = dataCur->mMoreToBePaintedPrev)
	{
	    dataCur = static_cast<RestackPersistentData *>
		(AnimWindow::get (wCur)->persistentData["restack"]);
	    if (!dataCur)
		break;
	    dataCur->mIsSecondary = true;
	}
    }
    return true;
}

bool
RestackAnim::onSameRestackChain (CompWindow *wSubject, CompWindow *wOther)
{
    RestackPersistentData *dataCur;
    for (CompWindow *wCur = wSubject; wCur;
	 wCur = dataCur->mMoreToBePaintedNext)
    {
	if (wOther == wCur)
	    return true;
	dataCur = static_cast<RestackPersistentData *>
	    (AnimWindow::get (wCur)->persistentData["restack"]);
	if (!dataCur)
	    break;
    }

    RestackPersistentData *dataSubj = static_cast<RestackPersistentData *>
	    (AnimWindow::get (wSubject)->
	     persistentData["restack"]);
    for (CompWindow *wCur = dataSubj->mMoreToBePaintedPrev; wCur;
	 wCur = dataCur->mMoreToBePaintedPrev)
    {
	if (wOther == wCur)
	    return true;
	dataCur = static_cast<RestackPersistentData *>
	    (AnimWindow::get (wCur)->persistentData["restack"]);
	if (!dataCur)
	    break;
    }
    return false;
}

bool
RestackAnim::overNewCopy ()
{
    bool lowering = (mRestackData->restackInfo () &&
		     !mRestackData->restackInfo ()->raised);

    // Reverse behavior if lowering (i.e. not raising)
    return ((!lowering && mRestackData->mVisitCount == 2) ||
	    (lowering && mRestackData->mVisitCount == 1));
}

CompRegion
RestackAnim::unionRestackChain (CompWindow *w)
{
    CompRegion unionRegion;

    RestackPersistentData *dataCur;
    for (CompWindow *wCur = w; wCur;
	 wCur = dataCur->mMoreToBePaintedNext)
    {
	unionRegion += wCur->borderRect ();
	dataCur = static_cast<RestackPersistentData *>
	    (AnimWindow::get (wCur)->persistentData["restack"]);
	if (!dataCur)
	    break;
    }

    RestackPersistentData *dataSubj = static_cast<RestackPersistentData *>
	    (AnimWindow::get (w)->
	     persistentData["restack"]);
    for (CompWindow *wCur = dataSubj->mMoreToBePaintedPrev; wCur;
	 wCur = dataCur->mMoreToBePaintedPrev)
    {
	unionRegion += wCur->borderRect ();
	dataCur = static_cast<RestackPersistentData *>
	    (AnimWindow::get (wCur)->persistentData["restack"]);
	if (!dataCur)
	    break;
    }

    return unionRegion;
}

RestackInfo::RestackInfo (CompWindow *wRestacked,
			  CompWindow *wStart,
			  CompWindow *wEnd,
			  CompWindow *wOldAbove,
			  bool raised) :
    wRestacked (wRestacked),
    wStart (wStart),
    wEnd (wEnd),
    wOldAbove (wOldAbove),
    raised (raised)
{
}

RestackPersistentData::RestackPersistentData () :
    PersistentData (),
    mRestackInfo (0),
    mWinToBePaintedBeforeThis (0),
    mWinThisIsPaintedBefore (0),
    mMoreToBePaintedPrev (0),
    mMoreToBePaintedNext (0),
    mConfigureNotified (false),
    mWinPassingThrough (0),
    mWalkerOverNewCopy (false),
    mVisitCount (0),
    mIsSecondary (false)
{
}

RestackPersistentData::~RestackPersistentData ()
{
    if (mRestackInfo)
	delete mRestackInfo;
}

void
RestackPersistentData::resetRestackInfo (bool alsoResetChain)
{
    delete mRestackInfo;
    mRestackInfo = 0;

    if (alsoResetChain)
    {
    	// Reset chain connections as this is not on a chain
    	mMoreToBePaintedNext = 0;
    	mMoreToBePaintedPrev = 0;
    }
}

void
RestackPersistentData::setRestackInfo (CompWindow *wRestacked,
				       CompWindow *wStart,
				       CompWindow *wEnd,
				       CompWindow *wOldAbove,
				       bool raised)
{
    if (mRestackInfo)
	delete mRestackInfo;
    mRestackInfo =
	new RestackInfo (wRestacked, wStart, wEnd, wOldAbove, raised);
}

/// Make this window be hosted on (i.e. drawn before) the given window.
void
RestackPersistentData::getHostedOnWin (CompWindow *wGuest, CompWindow *wHost)
{
    RestackPersistentData *dataHost = static_cast<RestackPersistentData *>
	(AnimWindow::get (wHost)->persistentData["restack"]);
    dataHost->mWinToBePaintedBeforeThis = wGuest;
    mWinThisIsPaintedBefore = wHost;
}

