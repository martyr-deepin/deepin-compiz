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

// =====================  Effect: Dodge  =========================

void
DodgeAnim::applyDodgeTransform ()
{
    if (mDodgeData->isDodgeSubject && mDodgeDirection == DodgeDirectionNone)
	return;

    float amountX = 0.0f;
    float amountY = 0.0f;

    if (mDodgeMaxAmountX != 0)
	amountX = sin (M_PI * mTransformProgress) * mDodgeMaxAmountX;

    if (mDodgeMaxAmountY != 0)
	amountY = sin (M_PI * mTransformProgress) * mDodgeMaxAmountY;

    mTransform.translate (amountX, amountY, 0.0f);
}

bool
DodgeAnim::moveUpdate (int dx, int dy)
{
    if (mDodgeData->isDodgeSubject &&
    	mDodgeDirection == DodgeDirectionXY)
    {
    	mDodgeDirection = DodgeDirectionNone;
    	mDodgeMaxAmountX = 0;
    	mDodgeMaxAmountY = 0;
    }

    CompWindow *wBottommost =
    	ExtensionPluginAnimation::getBottommostInRestackChain (mWindow);

    // Update dodge amount for the dodgers of all subjects
    // in the restack chain
    RestackPersistentData *dataCur;
    for (CompWindow *wCur = wBottommost; wCur;
	 wCur = dataCur->mMoreToBePaintedNext)
    {
	AnimWindow *awCur = AnimWindow::get (wCur);
	dataCur = static_cast<RestackPersistentData *>
	    (awCur->persistentData["restack"]);
	if (!dataCur)
	    break;

	Animation *curAnim = awCur->curAnimation ();
	if (!curAnim || curAnim->info () != AnimEffectDodge)
	    continue;

	DodgePersistentData *dodgeDataDodger;

	// Update dodge amount for each dodger
	for (CompWindow *dw = mDodgeData->dodgeChainStart; dw;
	     dw = dodgeDataDodger->dodgeChainNext)
	{
	    AnimWindow *adw = AnimWindow::get (dw);
	    dodgeDataDodger =
		static_cast<DodgePersistentData *>
		(adw->persistentData["dodge"]);

	    DodgeAnim *animDodger =
		dynamic_cast<DodgeAnim *> (adw->curAnimation ());
	    if (!animDodger)
		continue;

	    if (animDodger->mDodgeSubjectWin &&
		animDodger->mTransformProgress <= 0.5f)
	    {
		animDodger->updateDodgerDodgeAmount ();
	    }
	}
    }

    return false;
}

/// Should only be called for non-subjects.
void
DodgeAnim::updateDodgerDodgeAmount ()
{
    // Find the box to be dodged, it can contain multiple windows
    // when there are dialog/utility windows of subject windows
    // (stacked in the mMoreToBePaintedNext chain).
    // Then this would be a bounding box of the subject windows
    // intersecting with dodger.
    CompRect subjectRect (unionRestackChain (mDodgeSubjectWin).boundingRect ());

    // Update dodge amount if subject window(s) moved during dodge
    float newDodgeAmount =
	getDodgeAmount (subjectRect, mWindow, mDodgeDirection);

    // Only update if amount got larger
    if (((mDodgeDirection == DodgeDirectionDown && newDodgeAmount > 0) ||
	 (mDodgeDirection == DodgeDirectionUp && newDodgeAmount < 0)) &&
	abs (newDodgeAmount) > abs (mDodgeMaxAmountY))
    {
	mDodgeMaxAmountY = newDodgeAmount;
    }
    else if (((mDodgeDirection == DodgeDirectionRight && newDodgeAmount > 0) ||
	      (mDodgeDirection == DodgeDirectionLeft && newDodgeAmount < 0)) &&
	     abs (newDodgeAmount) > abs (mDodgeMaxAmountX))
    {
	mDodgeMaxAmountX = newDodgeAmount;
    }
}

float
DodgeAnim::dodgeProgress ()
{
    float forwardProgress = progressLinear ();

    forwardProgress = 1 - forwardProgress;
    return forwardProgress;
}

void
DodgeAnim::step ()
{
    TransformAnim::step ();

    mTransformProgress = 0;

    float forwardProgress = dodgeProgress ();
    if (forwardProgress > mTransformStartProgress)
    {
    	// Compute transform progress and normalize
	mTransformProgress =
	    (forwardProgress - mTransformStartProgress) /
	    (1 - mTransformStartProgress);
    }

    mTransform.reset ();
    applyDodgeTransform ();
}

void
DodgeAnim::updateTransform (GLMatrix &wTransform)
{
    TransformAnim::updateTransform (wTransform);
}

void
DodgeAnim::postPreparePaint ()
{
    // Only dodge subjects (with dodger chains) should be processed here
    if (!mDodgeData || !mDodgeData->isDodgeSubject ||
    	!mDodgeData->dodgeChainStart)
	return;

    if (!mRestackData || !mRestackData->restackInfo ())
	return;

    if (mDodgeData->skipPostPrepareScreen)
	return;

    // Find the bottommost subject in restack chain
    CompWindow *wBottommost = mWindow;
    RestackPersistentData *dataCur;
    for (CompWindow *wCur = mRestackData->mMoreToBePaintedPrev; wCur;
    	 wCur = dataCur->mMoreToBePaintedPrev)
    {
    	wBottommost = wCur;
    	dataCur = static_cast<RestackPersistentData *>
    	    (AnimWindow::get (wCur)->persistentData["restack"]);
    	if (!dataCur)
    	    break;
    }
    AnimWindow *awBottommost = AnimWindow::get (wBottommost);
    RestackPersistentData *restackDataBottommost =
    	static_cast<RestackPersistentData *>
	(awBottommost->persistentData["restack"]);

    // Find the first dodging window that hasn't yet
    // reached 50% progress yet. The subject window should be
    // painted right behind that one (or right in front of it
    // if subject is being lowered).
    RestackPersistentData *restackDataDodger = NULL;
    DodgePersistentData *dodgeDataDodger = NULL;
    CompWindow *dw;
    for (dw = mDodgeData->dodgeChainStart; dw;
	 dw = dodgeDataDodger->dodgeChainNext)
    {
	AnimWindow *adw = AnimWindow::get (dw);
	restackDataDodger = static_cast<RestackPersistentData *>
	    (adw->persistentData["restack"]);
	dodgeDataDodger = static_cast<DodgePersistentData *>
	    (adw->persistentData["dodge"]);

	DodgeAnim *animDodger =
	    dynamic_cast<DodgeAnim *> (adw->curAnimation ());

	if (!(animDodger->mTransformProgress > 0.5f))
	    break;
    }

    RestackInfo *bottommostRestackInfo = restackDataBottommost->restackInfo ();
    if (!bottommostRestackInfo)
    	return;

    if (bottommostRestackInfo->raised &&
	// if mWindow's host should change
    	dw != restackDataBottommost->mWinThisIsPaintedBefore)
    {
	if (restackDataBottommost->mWinThisIsPaintedBefore)
	{
	    // Clear old host
	    RestackPersistentData *dataOldHost =
		static_cast<RestackPersistentData *>
		(AnimWindow::get (restackDataBottommost->
				  mWinThisIsPaintedBefore)->
		 persistentData["restack"]);
	    dataOldHost->mWinToBePaintedBeforeThis = 0;
	}
	// if a dodger win. is still at <0.5 progress
	if (dw && restackDataDodger)
	{
	    // Put subject right behind new host
	    restackDataDodger->mWinToBePaintedBeforeThis = wBottommost;
	}
	// otherwise all dodger win.s have passed 0.5 progress

	CompWindow *wCur = wBottommost;
	while (wCur)
	{
	    RestackPersistentData *dataCur =
		static_cast<RestackPersistentData *>
		(AnimWindow::get (wCur)->persistentData["restack"]);
	    // dw can be null, which is ok
	    dataCur->mWinThisIsPaintedBefore = dw;
	    wCur = dataCur->mMoreToBePaintedNext;
	}
    }
    else if (!bottommostRestackInfo->raised)
    {
	// Put the subject right in front of dw.
	// But we need to find the (dodger) window above dw
	// (since we need to put the subject *behind* a window).

	CompWindow *wDodgeChainAbove = 0;

	// if a dodger win. is still at <0.5 progress
	if (dw && dodgeDataDodger)
	{
	    if (dodgeDataDodger->dodgeChainPrev)
	    {
	    	wDodgeChainAbove = dodgeDataDodger->dodgeChainPrev;
	    }
	    else
	    {
	    	// Use the wOldAbove of topmost subject
		wDodgeChainAbove = mRestackData->restackInfo ()->wOldAbove;
	    }
	    if (!wDodgeChainAbove)
		compLogMessage ("animation", CompLogLevelError,
				"%s: error at line %d", __FILE__, __LINE__);
	    else if (restackDataBottommost->mWinThisIsPaintedBefore !=
		     wDodgeChainAbove) // w's host is changing
	    {
		RestackPersistentData *dataNewHost =
		    static_cast<RestackPersistentData *>
		    (AnimWindow::get (wDodgeChainAbove)->
		     persistentData["restack"]);

		// Put subject right behind new host
		dataNewHost->mWinToBePaintedBeforeThis = wBottommost;
	    }
	}
	if (restackDataBottommost->mWinThisIsPaintedBefore &&
	    restackDataBottommost->mWinThisIsPaintedBefore != wDodgeChainAbove)
	{
	    // Clear old host
	    RestackPersistentData *dataOldHost =
		static_cast<RestackPersistentData *>
		(AnimWindow::get (restackDataBottommost->
				  mWinThisIsPaintedBefore)->
		 persistentData["restack"]);
	    dataOldHost->mWinToBePaintedBeforeThis = 0;
	}
	// otherwise all dodger win.s have passed 0.5 progress

	CompWindow *wCur = wBottommost;
	while (wCur)
	{
	    RestackPersistentData *dataCur =
		static_cast<RestackPersistentData *>
		(AnimWindow::get (wCur)->persistentData["restack"]);
	    // wDodgeChainAbove can be null, which is ok
	    dataCur->mWinThisIsPaintedBefore = wDodgeChainAbove;
	    wCur = dataCur->mMoreToBePaintedNext;
	}
    }
}

bool
DodgeAnim::shouldDamageWindowOnStart ()
{
    // for dodging windows only, when subject is fixed
    return !(mDodgeMode == AnimationOptions::DodgeModeFixedClickedWindow &&
    	     mDodgeData->isDodgeSubject);
}

void
DodgeAnim::updateBB (CompOutput &output)
{
    TransformAnim::updateBB (output);
}

DodgeAnim::DodgeAnim (CompWindow *w,
		      WindowEvent curWindowEvent,
		      float duration,
		      const AnimEffect info,
		      const CompRect &icon) :
    Animation::Animation (w, curWindowEvent, duration, info, icon),
    RestackAnim::RestackAnim (w, curWindowEvent, duration, info, icon),
    TransformAnim::TransformAnim (w, curWindowEvent, duration, info, icon),
    mDodgeData (static_cast<DodgePersistentData *>
		(AnimWindow::get (w)->persistentData["dodge"])),
    mDodgeSubjectWin (0),
    mDodgeMaxAmountX (0),
    mDodgeMaxAmountY (0),
    mDodgeDirection (DodgeDirectionNone),
    mDodgeMode (optValI (AnimationOptions::DodgeMode))
{
}

void
DodgeAnim::cleanUp (bool closing,
		    bool destructing)
{
    // Remove this window from its subject's dodger chain
    if (mDodgeSubjectWin)
    {
    	CompWindow *w = mDodgeSubjectWin;
	AnimWindow *aw = AnimWindow::get (w);
	Animation *curAnim = aw->curAnimation ();
	DodgePersistentData *dodgeData = static_cast<DodgePersistentData *>
		(aw->persistentData["dodge"]);

	if (curAnim && curAnim->info () == AnimEffectDodge &&
	    // Only process subjects with a dodge chain
	    dodgeData && dodgeData->dodgeChainStart &&
	    dodgeData->isDodgeSubject)
	{
	    // Go through each dodger, checking if this is that one
	    // dw: Dodger window
	    DodgePersistentData *dodgeDataDodger;
	    for (CompWindow *dw = dodgeData->dodgeChainStart; dw;
		 dw = dodgeDataDodger->dodgeChainNext)
	    {
		AnimWindow *adw = AnimWindow::get (dw);
		dodgeDataDodger = static_cast<DodgePersistentData *>
		    (adw->persistentData["dodge"]);
		if (dw == mWindow)
		{
		    // Remove mWindow from the chain
		    CompWindow *dwNext = dodgeDataDodger->dodgeChainNext;
		    if (dwNext)
		    {
			AnimWindow *adwNext = AnimWindow::get (dwNext);
			DodgePersistentData *dodgeDataDodgerNext =
			    static_cast<DodgePersistentData *>
			    (adwNext->persistentData["dodge"]);
			dodgeDataDodgerNext->dodgeChainPrev =
			    dodgeDataDodger->dodgeChainPrev;
		    }
		    CompWindow *dwPrev = dodgeDataDodger->dodgeChainPrev;
		    if (dwPrev)
		    {
			AnimWindow *adwPrev = AnimWindow::get (dwPrev);
			DodgePersistentData *dodgeDataDodgerPrev =
			    static_cast<DodgePersistentData *>
			    (adwPrev->persistentData["dodge"]);
			dodgeDataDodgerPrev->dodgeChainNext =
			    dodgeDataDodger->dodgeChainNext;
		    }
		    if (dodgeData->dodgeChainStart == mWindow)
			dodgeData->dodgeChainStart =
			    dodgeDataDodger->dodgeChainNext;
		    dodgeDataDodger->dodgeChainPrev = 0;
		    dodgeDataDodger->dodgeChainNext = 0;
		}
	    }
	}
    }
    else
    {
	DodgePersistentData *dodgeData = static_cast<DodgePersistentData *>
		(mAWindow->persistentData["dodge"]);

	if (dodgeData && dodgeData->isDodgeSubject)
	{
	    // Update this window's dodgers so that they no longer point
	    // to this window as their subject
	    DodgePersistentData *dodgeDataDodger;
	    for (CompWindow *dw = dodgeData->dodgeChainStart; dw;
		 dw = dodgeDataDodger->dodgeChainNext)
	    {
		AnimWindow *adw = AnimWindow::get (dw);
		if (!adw)
		    break;
		dodgeDataDodger = static_cast<DodgePersistentData *>
		    (adw->persistentData["dodge"]);

		Animation *curAnim = adw->curAnimation ();

		if (curAnim && curAnim->info () == AnimEffectDodge)
		{
		    DodgeAnim *animDodger = dynamic_cast<DodgeAnim *> (curAnim);
		    if (animDodger->mDodgeSubjectWin == mWindow)
			animDodger->mDodgeSubjectWin = NULL;
		}
	    }
	}
    }

    // Reset dodge parameters
    //if (!(restackData->mMoreToBePaintedPrev ||
	//  restackData->mMoreToBePaintedNext))
    //{
	mDodgeData->isDodgeSubject = false;
	mDodgeData->skipPostPrepareScreen = false;
    //}
    RestackAnim::cleanUp (closing, destructing);
}

int
DodgeAnim::getDodgeAmount (CompRect &rect,
			   CompWindow *dw,
			   DodgeDirection dir)
{
    CompRect dRect (dw->borderRect ().x () +
		    (dw->outputRect ().x () - dw->borderRect ().x ()) / 2,
		    dw->borderRect ().y () +
		    (dw->outputRect ().y () - dw->borderRect ().y ()) / 2,
		    (dw->borderRect ().width () +
		     dw->outputRect ().width ()) / 2,
		    (dw->borderRect ().height () +
		     dw->outputRect ().height ()) / 2);

    int amount = 0;
    switch (dir)
    {
	case DodgeDirectionUp:
	    amount = (rect.y () - (dRect.y () + dRect.height ()));
	    break;
	case DodgeDirectionDown:
	    amount = (rect.y () + rect.height () - dRect.y ());
	    break;
	case DodgeDirectionLeft:
	    amount = (rect.x () - (dRect.x () + dRect.width ()));
	    break;
	case DodgeDirectionRight:
	    amount = (rect.x () + rect.width () - dRect.x ());
	    break;
	default:
	    break;
    }
    return amount;
}

void
DodgeAnim::processCandidate (CompWindow *candidateWin,
			     CompWindow *subjectWin,
			     CompRegion &candidateAndSubjectIntersection,
			     int &numSelectedCandidates)
{
    AnimWindow *aCandidateWin = AnimWindow::get (candidateWin);
    AnimScreen *as = AnimScreen::get (::screen);

    if ((!aCandidateWin->curAnimation () ||
	 aCandidateWin->curAnimation ()->info () == AnimEffectDodge) &&
	candidateWin != subjectWin) // don't let the subject dodge itself
    {
	// Mark this window for dodge

	bool nonMatching = false;
	if (as->getMatchingAnimSelection (candidateWin, AnimEventFocus, 0) !=
	    AnimEffectDodge)
	    nonMatching = true;

	numSelectedCandidates++;
	DodgePersistentData *data = static_cast<DodgePersistentData *>
	    (aCandidateWin->persistentData["dodge"]);
	data->dodgeOrder = numSelectedCandidates;
	if (nonMatching) // Use neg. values for non-matching windows
	    data->dodgeOrder *= -1;
    }
}

void
DodgeAnim::postInitiateRestackAnim (int numSelectedCandidates,
				    int duration,
				    CompWindow *wStart,
				    CompWindow *wEnd,
				    bool raised)
{
    DodgePersistentData *dataSubject = mDodgeData;
    if (!dataSubject)
    	return;

    dataSubject->isDodgeSubject = true;
    dataSubject->dodgeChainStart = 0;

    if (mRestackData && mRestackData->mIsSecondary)
	return; // We're done here

    float maxTransformTotalProgress = 0;
    float dodgeMaxStartProgress =
	numSelectedCandidates * optValF (AnimationOptions::DodgeGapRatio) *
	duration / 1000.0f;

    CompWindow *wDodgeChainLastVisited = 0;

    // dw: Dodger window(s)
    for (CompWindow *dw = wStart; dw && dw != wEnd->next; dw = dw->next)
    {
	AnimWindow *adw = AnimWindow::get (dw);
	DodgePersistentData *dataDodger = static_cast<DodgePersistentData *>
	    (adw->persistentData["dodge"]);

	// Skip non-dodgers
	if (dataDodger->dodgeOrder == 0)
	    continue;

	// Initiate dodge for this window

	bool stationaryDodger = false;
	if (dataDodger->dodgeOrder < 0)
	{
	    dataDodger->dodgeOrder *= -1; // Make it positive again
	    stationaryDodger = true;
	}
	if (!adw->curAnimation ())
	{
	    // Create dodge animation for dodger
	    adw->createFocusAnimation (AnimEffectDodge);
	    ExtensionPluginAnimation *extPlugin =
		static_cast<ExtensionPluginAnimation *>
		(getExtensionPluginInfo ());
	    extPlugin->incrementCurRestackAnimCount ();
	}

	DodgeAnim *animDodger =
	    dynamic_cast<DodgeAnim *> (adw->curAnimation ());

	animDodger->mDodgeSubjectWin = mWindow;

	if (mDodgeMode == AnimationOptions::DodgeModeFixedClickedWindow)
	{
	    // Slight change in dodge movement start
	    // to reflect stacking order of dodger windows
	    if (raised)
		animDodger->mTransformStartProgress =
		    dodgeMaxStartProgress *
		    (dataDodger->dodgeOrder - 1) / numSelectedCandidates;
	    else
		animDodger->mTransformStartProgress =
		    dodgeMaxStartProgress *
		    (1 - (float)dataDodger->dodgeOrder / numSelectedCandidates);
	}

	float transformTotalProgress =
	    1 + animDodger->mTransformStartProgress;

	if (maxTransformTotalProgress < transformTotalProgress)
	    maxTransformTotalProgress = transformTotalProgress;

	// normalize
	animDodger->mTransformStartProgress /= transformTotalProgress;

	if (stationaryDodger)
	{
	    animDodger->mTransformStartProgress = 0;
	    transformTotalProgress = 0;
	}

	animDodger->mTotalTime = transformTotalProgress * duration;
	animDodger->mRemainingTime = animDodger->mTotalTime;

	// Put window on dodge chain

	// if dodge chain was started before
	if (wDodgeChainLastVisited)
	{
	    DodgePersistentData *dataDodgeChainLastVisited =
		static_cast<DodgePersistentData *>
		(AnimWindow::get (wDodgeChainLastVisited)->
		 persistentData["dodge"]);
	    if (raised)
	    {
		dataDodgeChainLastVisited->dodgeChainNext = dw;
	    }
	    else
		dataDodgeChainLastVisited->dodgeChainPrev = dw;
	}
	else if (raised) // mark chain start
	{
	    dataSubject->dodgeChainStart = dw;
	}
	if (raised)
	{
	    dataDodger->dodgeChainPrev = wDodgeChainLastVisited;
	    dataDodger->dodgeChainNext = 0;
	}
	else
	{
	    dataDodger->dodgeChainPrev = 0;
	    dataDodger->dodgeChainNext = wDodgeChainLastVisited;
	}

	wDodgeChainLastVisited = dw;

	// Reset back to 0 for the next dodge calculation
	dataDodger->dodgeOrder = 0;
    }

    // if subject is being lowered,
    // point chain-start to the topmost doding window
    if (!raised)
	dataSubject->dodgeChainStart = wDodgeChainLastVisited;

    mTotalTime = maxTransformTotalProgress * duration;
    mRemainingTime = mTotalTime;
}

void
DodgeAnim::calculateDodgeAmounts ()
{
    // holds whether each side of the subject is covered by dodgers or not
    bool coveredSides[4] = {false, false, false, false};

    // maximum distance between a dodger window and the subject in X and Y axes
    int maxDistX = 0;
    int maxDistXActual = 0;
    int maxDistY = 0;
    int maxDistYActual = 0;

    CompRect subjectRect (unionRestackChain (mWindow).boundingRect ());

    // Go through each dodger, calculating its dodge amount.
    // dw: Dodger window
    DodgePersistentData *dodgeDataDodger;
    for (CompWindow *dw = mDodgeData->dodgeChainStart; dw;
	 dw = dodgeDataDodger->dodgeChainNext)
    {
	AnimWindow *adw = AnimWindow::get (dw);

	dodgeDataDodger = static_cast<DodgePersistentData *>
	    (adw->persistentData["dodge"]);

	DodgeAnim *animDodger =
	    dynamic_cast<DodgeAnim *> (adw->curAnimation ());
	if (!animDodger)
	    continue;

	// Find direction (left, right, up, down) that minimizes dodge amount

	int dodgeAmount[4];

	for (int i = 0; i < 4; i++)
	    dodgeAmount[i] =
		DodgeAnim::getDodgeAmount (subjectRect, dw, (DodgeDirection)i);

	int amountMinActual = dodgeAmount[0];
	int amountMinAbs = abs (amountMinActual);
	int iMin = 0;
	for (int i=1; i<4; i++)
	{
	    int absAmount = abs (dodgeAmount[i]);
	    if (amountMinAbs > absAmount)
	    {
		amountMinAbs = absAmount;
		amountMinActual = dodgeAmount[i];
		iMin = i;
	    }
	}
	if (iMin == DodgeDirectionUp ||
	    iMin == DodgeDirectionDown)
	{
	    animDodger->mDodgeMaxAmountX = 0;
	    animDodger->mDodgeMaxAmountY = dodgeAmount[iMin];
	    if (mDodgeMode == AnimationOptions::DodgeModeAllMoving &&
	    	maxDistY < amountMinAbs)
	    {
		maxDistY = amountMinAbs;
		maxDistYActual = amountMinActual;
	    }
	}
	else
	{
	    animDodger->mDodgeMaxAmountX = dodgeAmount[iMin];
	    animDodger->mDodgeMaxAmountY = 0;
	    if (mDodgeMode == AnimationOptions::DodgeModeAllMoving && maxDistX < amountMinAbs)
	    {
		maxDistX = amountMinAbs;
		maxDistXActual = amountMinActual;
	    }
	}
	animDodger->mDodgeDirection = (DodgeDirection)iMin;

	coveredSides[iMin] = true;
    }

    if (mDodgeMode == AnimationOptions::DodgeModeFixedClickedWindow)
    {
	// Subject doesn't move
	mDodgeMaxAmountX = 0;
	mDodgeMaxAmountY = 0;
	mDodgeDirection = DodgeDirectionNone;
    }
    else
    {
	// Subject should dodge in an axis if only one side is
	// covered by a dodger.
	bool subjectDodgesInX = (coveredSides[DodgeDirectionLeft] ^
				 coveredSides[DodgeDirectionRight]);
	bool subjectDodgesInY = (coveredSides[DodgeDirectionUp] ^
				 coveredSides[DodgeDirectionDown]);

	float dodgeAmountX = subjectDodgesInX ? -maxDistXActual / 2 : 0;
	float dodgeAmountY = subjectDodgesInY ? -maxDistYActual / 2 : 0;
	DodgeDirection dodgeDirection;

	if (!subjectDodgesInX && !subjectDodgesInY)
	    dodgeDirection = DodgeDirectionNone;
	else
	    dodgeDirection = DodgeDirectionXY;

	CompWindow *wBottommost =
	    ExtensionPluginAnimation::getBottommostInRestackChain (mWindow);

	float offsetX = 0;
	float offsetY = 0;
	float offsetIncrementX = (dodgeAmountX == 0 ? 0 :
				  100 * dodgeAmountX / fabs (dodgeAmountX));
	float offsetIncrementY = (dodgeAmountY == 0 ? 0 :
				  100 * dodgeAmountY / fabs (dodgeAmountY));

	// Set dodge amount and direction for all subjects
	// in the restack chain
	RestackPersistentData *dataCur;
	for (CompWindow *wCur = wBottommost; wCur;
	     wCur = dataCur->mMoreToBePaintedNext,
	     offsetX += offsetIncrementX,
	     offsetY += offsetIncrementY)
	{
	    AnimWindow *awCur = AnimWindow::get (wCur);

	    dataCur = static_cast<RestackPersistentData *>
	    	(awCur->persistentData["restack"]);
	    if (!dataCur)
	    	break;

	    Animation *curAnim = awCur->curAnimation ();
	    if (!curAnim || curAnim->info () != AnimEffectDodge)
		continue;
	    DodgeAnim *dodgeAnim = dynamic_cast<DodgeAnim *> (curAnim);

	    dodgeAnim->mDodgeMaxAmountX = dodgeAmountX + offsetX;
	    dodgeAnim->mDodgeMaxAmountY = dodgeAmountY + offsetY;
	    dodgeAnim->mDodgeDirection = dodgeDirection;

	    dodgeAnim->mTransformStartProgress = 0;
	}

	if (dodgeDirection == DodgeDirectionXY)
	{
	    // Go through each dodger, adjusting its dodge amount if the
	    // subject(s) is dodging in that axis (X or Y).
	    // dw: Dodger window
	    DodgePersistentData *dodgeDataDodger;
	    for (CompWindow *dw = mDodgeData->dodgeChainStart; dw;
		 dw = dodgeDataDodger->dodgeChainNext)
	    {
		AnimWindow *adw = AnimWindow::get (dw);

		dodgeDataDodger = static_cast<DodgePersistentData *>
		    (adw->persistentData["dodge"]);

		DodgeAnim *animDodger =
		    dynamic_cast<DodgeAnim *> (adw->curAnimation ());
		if (!animDodger)
		    continue;

		// if both dodge in X axis
		if (subjectDodgesInX && animDodger->mDodgeMaxAmountX != 0)
		{
		    if (animDodger->mDodgeMaxAmountX *
		    	(animDodger->mDodgeMaxAmountX + dodgeAmountX) < 0)
		    {
		    	// If the sign is going to change, just reset instead
			animDodger->mDodgeMaxAmountX = 0;
		    }
		    else
		    	animDodger->mDodgeMaxAmountX += dodgeAmountX;
		}

		// if both dodge in Y axis
		if (subjectDodgesInY && animDodger->mDodgeMaxAmountY != 0)
		{
		    if (animDodger->mDodgeMaxAmountY *
		    	(animDodger->mDodgeMaxAmountY + dodgeAmountY) < 0)
		    {
		    	// If the sign is going to change, just reset instead
			animDodger->mDodgeMaxAmountY = 0;
		    }
		    else
		    	animDodger->mDodgeMaxAmountY += dodgeAmountY;
		}
	    }
	}
    }
}

bool
DodgeAnim::paintedElsewhere ()
{
    bool elsewhere =
	mRestackData &&
	mRestackData->mWinThisIsPaintedBefore && // has to be currently hosted
	mDodgeData &&
	mDodgeData->isDodgeSubject &&
	overNewCopy ();

    return elsewhere;
}

DodgePersistentData::DodgePersistentData () :
    dodgeOrder (0),
    isDodgeSubject (false),
    skipPostPrepareScreen (false),
    dodgeChainStart (0),
    dodgeChainPrev (0),
    dodgeChainNext (0)
{
}

