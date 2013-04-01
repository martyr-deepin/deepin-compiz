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

// =====================  Effect: Zoom and Sidekick  =========================

const float ZoomAnim::kDurationFactor = 1.33;
const float ZoomAnim::kSpringyDurationFactor = 1.82;
const float ZoomAnim::kNonspringyDurationFactor = 1.67;

ZoomAnim::ZoomAnim (CompWindow *w,
		    WindowEvent curWindowEvent,
		    float duration,
		    const AnimEffect info,
		    const CompRect &icon) :
    Animation::Animation (w, curWindowEvent, duration, info, icon),
    TransformAnim::TransformAnim (w, curWindowEvent, duration, info, icon),
    FadeAnim::FadeAnim (w, curWindowEvent, duration, info, icon)
{
    CompRect outRect (mAWindow->savedRectsValid () ?
		      mAWindow->savedOutRect () :
		      mWindow->outputRect ());

    if (isZoomFromCenter ())
    {
	mIcon.setX (outRect.x () + outRect.width () / 2 - mIcon.width () / 2);
	mIcon.setY (outRect.y () + outRect.height () / 2 - mIcon.height () / 2);
    }
}

SidekickAnim::SidekickAnim (CompWindow *w,
			    WindowEvent curWindowEvent,
			    float duration,
			    const AnimEffect info,
			    const CompRect &icon) :
    Animation::Animation (w, curWindowEvent, duration, info, icon),
    TransformAnim::TransformAnim (w, curWindowEvent, duration, info, icon),
    ZoomAnim::ZoomAnim (w, curWindowEvent, duration, info, icon)
{
    // determine number of rotations randomly in [0.9, 1.1] range
    mNumRotations =
	optValF (AnimationOptions::SidekickNumRotations) *
	(1.0f + 0.2f * rand () / RAND_MAX - 0.1f);

    CompRect outRect (mAWindow->savedRectsValid () ?
		      mAWindow->savedOutRect () :
		      mWindow->outputRect ());

    float winCenterX = outRect.x () + outRect.width () / 2.0;
    float iconCenterX = mIcon.x () + mIcon.width () / 2.0;

    // if window is to the right of icon, rotate clockwise instead
    // to make rotation look more pleasant
    if (winCenterX > iconCenterX)
	mNumRotations *= -1;
}

float
ZoomAnim::getSpringiness ()
{
    return 2 * optValF (AnimationOptions::ZoomSpringiness);
}

float
SidekickAnim::getSpringiness ()
{
    return 1.6 * optValF (AnimationOptions::SidekickSpringiness);
}

bool
ZoomAnim::isZoomFromCenter ()
{
    return (optValI (AnimationOptions::ZoomFromCenter) ==
	    AnimationOptions::ZoomFromCenterOn ||
	    ((mCurWindowEvent == WindowEventMinimize ||
	      mCurWindowEvent == WindowEventUnminimize) &&
	     optValI (AnimationOptions::ZoomFromCenter) ==
	     AnimationOptions::ZoomFromCenterMinimizeUnminimizeOnly) ||
	    ((mCurWindowEvent == WindowEventOpen ||
	      mCurWindowEvent == WindowEventClose) &&
	     optValI (AnimationOptions::ZoomFromCenter) ==
	     AnimationOptions::ZoomFromCenterOpenCloseOnly));
}

bool
SidekickAnim::isZoomFromCenter ()
{
    return (optValI (AnimationOptions::SidekickZoomFromCenter) ==
	    AnimationOptions::ZoomFromCenterOn ||
	    ((mCurWindowEvent == WindowEventMinimize ||
	      mCurWindowEvent == WindowEventUnminimize) &&
	     optValI (AnimationOptions::SidekickZoomFromCenter) ==
	     AnimationOptions::SidekickZoomFromCenterMinimizeUnminimizeOnly) ||
	    ((mCurWindowEvent == WindowEventOpen ||
	      mCurWindowEvent == WindowEventClose) &&
	     optValI (AnimationOptions::SidekickZoomFromCenter) ==
	     AnimationOptions::SidekickZoomFromCenterOpenCloseOnly));
}

void
ZoomAnim::adjustDuration ()
{
    // allow extra time for spring damping / deceleration
    if ((mCurWindowEvent == WindowEventUnminimize ||
	 mCurWindowEvent == WindowEventOpen) &&
	getSpringiness () > 1e-4)
    {
	mTotalTime *= kSpringyDurationFactor;
    }
    else if (mCurWindowEvent == WindowEventOpen ||
	     mCurWindowEvent == WindowEventClose)
    {
	mTotalTime *= kNonspringyDurationFactor;
    }
    else
    {
	mTotalTime *= kDurationFactor;
    }
    mRemainingTime = mTotalTime;
}

void
ZoomAnim::getZoomProgress (float *pMoveProgress,
			   float *pScaleProgress,
			   bool neverSpringy)
{
    float forwardProgress =
	1 - mRemainingTime /
	(mTotalTime - mTimestep);
    forwardProgress = MIN (forwardProgress, 1);
    forwardProgress = MAX (forwardProgress, 0);

    float x = forwardProgress;
    bool backwards = false;
    int animProgressDir = 1;

    if (mCurWindowEvent == WindowEventUnminimize ||
	mCurWindowEvent == WindowEventOpen)
	animProgressDir = 2;
    if (mOverrideProgressDir != 0)
	animProgressDir = mOverrideProgressDir;
    if ((animProgressDir == 1 &&
	 (mCurWindowEvent == WindowEventUnminimize ||
	  mCurWindowEvent == WindowEventOpen)) ||
	(animProgressDir == 2 &&
	 (mCurWindowEvent == WindowEventMinimize ||
	  mCurWindowEvent == WindowEventClose)))
	backwards = true;
    if (backwards)
	x = 1 - x;

    float dampBase = (pow (1-pow (x,1.2)*0.5,10)-pow (0.5,10))/(1-pow (0.5,10));
    float nonSpringyProgress =
	1 - pow (progressDecelerateCustom (1 - x, .5f, .8f), 1.7f);

    float damping =
	pow (dampBase, 0.5);

    float damping2 =
	((pow (1-(pow (x,0.7)*0.5),10)-pow (0.5,10))/(1-pow (0.5,10))) *
	0.7 + 0.3;
    float springiness = 0;

    // springy only when appearing
    if ((mCurWindowEvent == WindowEventUnminimize ||
	 mCurWindowEvent == WindowEventOpen) &&
	!neverSpringy)
    {
	springiness = getSpringiness ();
    }

    float springyMoveProgress =
	cos (2*M_PI*pow (x,1)*1.25) * damping * damping2;

    float scaleProgress;
    float moveProgress;

    if (springiness > 1e-4f)
    {
	if (x > 0.2)
	{
	    springyMoveProgress *= springiness;
	}
	else
	{
	    // interpolate between (springyMoveProgress * springiness)
	    // and springyMoveProgress for smooth transition at 0.2
	    // (where it crosses y=0)
	    float progressUpto02 = x / 0.2f;
	    springyMoveProgress =
		(1 - progressUpto02) * springyMoveProgress +
		progressUpto02 * springyMoveProgress * springiness;
	}
	moveProgress = 1 - springyMoveProgress;
    }
    else
    {
	moveProgress = nonSpringyProgress;
    }
    if (mCurWindowEvent == WindowEventUnminimize ||
	mCurWindowEvent == WindowEventOpen)
	moveProgress = 1 - moveProgress;
    if (backwards)
	moveProgress = 1 - moveProgress;

    float scProgress = nonSpringyProgress;
    if (mCurWindowEvent == WindowEventUnminimize ||
	mCurWindowEvent == WindowEventOpen)
	scProgress = 1 - scProgress;
    if (backwards)
	scProgress = 1 - scProgress;

    scaleProgress =
	pow (scProgress, 1.25);

    if (pMoveProgress)
	*pMoveProgress = moveProgress;
    if (pScaleProgress)
	*pScaleProgress = scaleProgress;
}

float
ZoomAnim::getFadeProgress ()
{
    float fadeProgress;
    getZoomProgress (0, &fadeProgress, false);
    return fadeProgress;
}

void
ZoomAnim::getCenterScaleFull (Point *pCurCenter, Point *pCurScale,
			      Point *pWinCenter, Point *pIconCenter,
			      float *pMoveProgress)
{
    CompRect outRect (mAWindow->savedRectsValid () ?
		      mAWindow->savedOutRect () :
		      mWindow->outputRect ());

    Point winCenter ((outRect.x () + outRect.width () / 2.0),
		     (outRect.y () + outRect.height () / 2.0));
    Point iconCenter (mIcon.x () + mIcon.width () / 2.0,
		      mIcon.y () + mIcon.height () / 2.0);
    Point winSize (outRect.width (), outRect.height ());

    winSize.setX (winSize.x () == 0 ? 1 : winSize.x ());
    winSize.setY (winSize.y () == 0 ? 1 : winSize.y ());

    float scaleProgress;
    float moveProgress;

    getZoomProgress (&moveProgress, &scaleProgress, neverSpringy ());

    Point curCenter
	((1 - moveProgress) * winCenter.x () + moveProgress * iconCenter.x (),
	 (1 - moveProgress) * winCenter.y () + moveProgress * iconCenter.y ());
    Point curScale
	(((1 - scaleProgress) * winSize.x () +
	  scaleProgress * mIcon.width ()) / winSize.x (),
	 ((1 - scaleProgress) * winSize.y () +
	  scaleProgress * mIcon.height ()) / winSize.y ());

    // Copy calculated variables
    if (pCurCenter)
	*pCurCenter = curCenter;
    if (pCurScale)
	*pCurScale = curScale;
    if (pWinCenter)
	*pWinCenter = winCenter;
    if (pIconCenter)
	*pIconCenter = iconCenter;
    if (pMoveProgress)
	*pMoveProgress = moveProgress;
}

void
ZoomAnim::applyTransform ()
{
    if (!zoomToIcon ())
	return;
    Point curCenter;
    Point curScale;
    Point winCenter;
    Point iconCenter;
    float moveProgress;

    getCenterScaleFull (&curCenter, &curScale,
			&winCenter, &iconCenter, &moveProgress);

    if (scaleAroundIcon ())
    {
	mTransform.translate (iconCenter.x (), iconCenter.y (), 0);
	mTransform.scale (curScale.x (), curScale.y (), curScale.y ());
	mTransform.translate (-iconCenter.x (), -iconCenter.y (), 0);

	if (hasExtraTransform ())
	{
	    mTransform.translate (winCenter.x (), winCenter.y (), 0);
	    applyExtraTransform (moveProgress);
	    mTransform.translate (-winCenter.x (), -winCenter.y (), 0);
	}
    }
    else
    {
	mTransform.translate (winCenter.x (), winCenter.y (), 0);
	float tx, ty;
	if (shouldAvoidParallelogramLook ())
	{
	    // avoid parallelogram look
	    float maxScale = MAX (curScale.x (), curScale.y ());
	    mTransform.scale (maxScale, maxScale, maxScale);
	    tx = (curCenter.x () - winCenter.x ()) / maxScale;
	    ty = (curCenter.y () - winCenter.y ()) / maxScale;
	}
	else
	{
	    mTransform.scale (curScale.x (), curScale.y (), curScale.y ());
	    tx = (curCenter.x () - winCenter.x ()) / curScale.x ();
	    ty = (curCenter.y () - winCenter.y ()) / curScale.y ();
	}
	mTransform.translate (tx, ty, 0);
	applyExtraTransform (moveProgress);
	mTransform.translate (-winCenter.x (), -winCenter.y (), 0);
    }
}

void
SidekickAnim::applyExtraTransform (float progress)
{
    mTransform.rotate (progress * 360 * mNumRotations, 0.0f, 0.0f, 1.0f);
}

bool
ZoomAnim::scaleAroundIcon ()
{
    return (getSpringiness () == 0.0f &&
	    (mCurWindowEvent == WindowEventOpen ||
	     mCurWindowEvent == WindowEventClose));
}

void
ZoomAnim::getCenterScale (Point *pCurCenter, Point *pCurScale)
{
    getCenterScaleFull (pCurCenter, pCurScale, NULL, NULL, NULL);
}

float
ZoomAnim::getActualProgress ()
{
    float forwardProgress = 0;

    if (zoomToIcon ())
	getZoomProgress (&forwardProgress, 0, true);
    else
	forwardProgress = progressLinear ();

    return forwardProgress;
}

Point
ZoomAnim::getCenter ()
{
    Point center;

    if (zoomToIcon ())
    {
	getCenterScale (&center, 0);
    }
    else
    {
	float forwardProgress = progressLinear ();

	CompRect inRect (mAWindow->savedRectsValid () ?
			 mAWindow->savedInRect () :
			 mWindow->borderRect ());

	center.setX (inRect.x () + inRect.width () / 2.0);

	if (mCurWindowEvent == WindowEventShade ||
	    mCurWindowEvent == WindowEventUnshade)
	{
	    float origCenterY = (inRect.y () +
				 inRect.height () / 2.0);
	    center.setY ((1 - forwardProgress) * origCenterY +
			 forwardProgress * (inRect.y () +
					    mDecorTopHeight));
	}
	else // i.e. (un)minimizing without zooming
	{
	    center.setY (inRect.y () + inRect.height () / 2.0);
	}
    }
    return center;
}

GridZoomAnim::GridZoomAnim (CompWindow *w,
			    WindowEvent curWindowEvent,
			    float duration,
			    const AnimEffect info,
			    const CompRect &icon) :
    Animation::Animation (w, curWindowEvent, duration, info, icon),
    TransformAnim::TransformAnim (w, curWindowEvent, duration, info, icon),
    GridTransformAnim::GridTransformAnim (w, curWindowEvent, duration, info,
					  icon),
    ZoomAnim::ZoomAnim (w, curWindowEvent, duration, info, icon)
{
}

void
GridZoomAnim::adjustDuration ()
{
    if (zoomToIcon ())
    {
	mTotalTime *= ZoomAnim::kDurationFactor;
	mRemainingTime = mTotalTime;
    }
}

