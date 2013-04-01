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

// =====================  Effect: Magic Lamp  =========================

void
MagicLampAnim::initGrid ()
{
    mGridWidth = 2;
    mGridHeight = optValI (AnimationOptions::MagicLampGridRes);
}

void
MagicLampWavyAnim::initGrid ()
{
    mGridWidth = 2;
    mGridHeight = optValI (AnimationOptions::MagicLampWavyGridRes);
}

MagicLampAnim::MagicLampAnim (CompWindow *w,
			      WindowEvent curWindowEvent,
			      float duration,
			      const AnimEffect info,
			      const CompRect &icon) :
    Animation::Animation (w, curWindowEvent, duration, info, icon),
    GridAnim::GridAnim (w, curWindowEvent, duration, info, icon)
{
    CompRect outRect (mAWindow->savedRectsValid () ?
		      mAWindow->savedOutRect () :
		      w->outputRect ());

    mTargetTop = ((outRect.y () + outRect.height () / 2) >
		  (icon.y () + icon.height () / 2));

    mUseQTexCoord = true;
}

MagicLampWavyAnim::MagicLampWavyAnim (CompWindow *w,
				      WindowEvent curWindowEvent,
				      float duration,
				      const AnimEffect info,
				      const CompRect &icon) :
    Animation::Animation (w, curWindowEvent, duration, info, icon),
    MagicLampAnim::MagicLampAnim (w, curWindowEvent, duration, info, icon)
{
    unsigned int maxWaves;
    float waveAmpMin, waveAmpMax;
    float distance;

    maxWaves = (unsigned) optValI (AnimationOptions::MagicLampWavyMaxWaves);
    waveAmpMin = optValF (AnimationOptions::MagicLampWavyAmpMin);
    waveAmpMax = optValF (AnimationOptions::MagicLampWavyAmpMax);

    if (waveAmpMax < waveAmpMin)
	waveAmpMax = waveAmpMin;

    // Initialize waves

    CompRect outRect (mAWindow->savedRectsValid () ?
		      mAWindow->savedOutRect () :
		      w->outputRect ());
    if (mTargetTop)
	distance = outRect.y () + outRect.height () - mIcon.y ();
    else
	distance = mIcon.y () - outRect.y ();

    mNumWaves =
	1 + (float)maxWaves *distance / ::screen->height ();

    mWaves = new WaveParam[mNumWaves];

    // Compute wave parameters

    int ampDirection = (RAND_FLOAT () < 0.5 ? 1 : -1);
    float minHalfWidth = 0.22f;
    float maxHalfWidth = 0.38f;

    for (unsigned int i = 0; i < mNumWaves; i++)
    {
	mWaves[i].amp =
	    ampDirection * (waveAmpMax - waveAmpMin) *
	    rand () / RAND_MAX + ampDirection * waveAmpMin;
	mWaves[i].halfWidth =
	    RAND_FLOAT () * (maxHalfWidth - minHalfWidth) + minHalfWidth;

	// avoid offset at top and bottom part by added waves
	float availPos = 1 - 2 * mWaves[i].halfWidth;
	float posInAvailSegment = 0;

	if (i > 0)
	    posInAvailSegment =
		(availPos / mNumWaves) * RAND_FLOAT ();

	mWaves[i].pos =
	    (posInAvailSegment +
	     i * availPos / mNumWaves +
	     mWaves[i].halfWidth);

	// switch wave direction
	ampDirection *= -1;
    }
}

MagicLampWavyAnim::~MagicLampWavyAnim ()
{
    delete[] mWaves;
}

/// Makes sure the window gets fully damaged with
/// effects that possibly have models that don't cover
/// the whole window (like in MagicLampAnim with menus).
MagicLampAnim::~MagicLampAnim ()
{
    if (mCurWindowEvent == WindowEventOpen ||
    	mCurWindowEvent == WindowEventUnminimize ||
    	mCurWindowEvent == WindowEventUnshade)
    {
	mAWindow->expandBBWithWindow ();
    }
}

bool
MagicLampWavyAnim::hasMovingEnd ()
{
    return optValB (AnimationOptions::MagicLampWavyMovingEnd);
}

bool
MagicLampAnim::hasMovingEnd ()
{
    return optValB (AnimationOptions::MagicLampMovingEnd);
}

/// Applies waves (at each step of the animation).
void
MagicLampWavyAnim::filterTargetX (float &targetX, float x)
{
    for (unsigned int i = 0; i < mNumWaves; i++)
    {
	float cosx = ((x - mWaves[i].pos) /
		       mWaves[i].halfWidth);
	if (cosx < -1 || cosx > 1)
	    continue;
	targetX += (mWaves[i].amp * mModel->scale ().x () *
		    (cos (cosx * M_PI) + 1) / 2);
    }
}

void
MagicLampAnim::step ()
{
    if ((curWindowEvent () == WindowEventOpen ||
	 curWindowEvent () == WindowEventClose) &&
	hasMovingEnd ())
    {
	short x, y;
	// Update icon position
	AnimScreen::get (::screen)->getMousePointerXY (&x, &y);
	mIcon.setX (x);
	mIcon.setY (y);
    }
    float forwardProgress = progressLinear ();

    float iconCloseEndY;
    float iconFarEndY;
    float winFarEndY;
    float winVisibleCloseEndY;

    CompRect inRect (mAWindow->savedRectsValid () ?
		     mAWindow->savedInRect () :
		     mWindow->borderRect ());
    CompRect outRect (mAWindow->savedRectsValid () ?
		      mAWindow->savedOutRect () :
		      mWindow->outputRect ());
    CompWindowExtents outExtents (mAWindow->savedRectsValid () ?
				  mAWindow->savedOutExtents () :
				  mWindow->output ());

    float iconShadowLeft =
	((float)(outRect.x () - inRect.x ())) *
	mIcon.width () / mWindow->width ();
    float iconShadowRight =
	((float)(outRect.x2 () - inRect.x2 ())) *
	mIcon.width () / mWindow->width ();

    float sigmoid0 = sigmoid (0);
    float sigmoid1 = sigmoid (1);

    float winw = outRect.width ();
    float winh = outRect.height ();

    if (mTargetTop)
    {
	iconFarEndY = mIcon.y ();
	iconCloseEndY = mIcon.y () + mIcon.height ();
	winFarEndY = outRect.y () + winh;
	winVisibleCloseEndY = outRect.y ();
	if (winVisibleCloseEndY < iconCloseEndY)
	    winVisibleCloseEndY = iconCloseEndY;
    }
    else
    {
	iconFarEndY = mIcon.y () + mIcon.height ();
	iconCloseEndY = mIcon.y ();
	winFarEndY = outRect.y ();
	winVisibleCloseEndY = outRect.y () + winh;
	if (winVisibleCloseEndY > iconCloseEndY)
	    winVisibleCloseEndY = iconCloseEndY;
    }

    float preShapePhaseEnd = 0.22f;
    float preShapeProgress  = 0;
    float postStretchProgress = 0;
    float stretchProgress = 0;
    float stretchPhaseEnd =
	preShapePhaseEnd + (1 - preShapePhaseEnd) *
	(iconCloseEndY -
	 winVisibleCloseEndY) / ((iconCloseEndY - winFarEndY) +
				 (iconCloseEndY - winVisibleCloseEndY));
    if (stretchPhaseEnd < preShapePhaseEnd + 0.1)
	stretchPhaseEnd = preShapePhaseEnd + 0.1;

    if (forwardProgress < preShapePhaseEnd)
    {
	preShapeProgress = forwardProgress / preShapePhaseEnd;

	// Slow down "shaping" toward the end
	preShapeProgress = 1 - progressDecelerate (1 - preShapeProgress);
    }

    if (forwardProgress < preShapePhaseEnd)
    {
	stretchProgress = forwardProgress / stretchPhaseEnd;
    }
    else
    {
	if (forwardProgress < stretchPhaseEnd)
	{
	    stretchProgress = forwardProgress / stretchPhaseEnd;
	}
	else
	{
	    postStretchProgress =
		(forwardProgress - stretchPhaseEnd) / (1 - stretchPhaseEnd);
	}
    }

    // The other objects are squeezed into a horizontal line behind the icon
    int topmostMovingObjectIdx = -1;
    int bottommostMovingObjectIdx = -1;

    unsigned int n = mModel->numObjects ();
    float fx = 0.0f;
    GridModel::GridObject *object = mModel->objects ();
    for (unsigned int i = 0; i < n; i++, object++)
    {
	Point3d &objPos = object->position ();
	float objGridX = object->gridPosition ().x ();

	if (i % 2 == 0) // object is at the left side
	{
	    float objGridY = object->gridPosition ().y ();

	    float origY = (mWindow->y () +
	    		   (winh * objGridY - outExtents.top) *
			   mModel->scale ().y ());
	    float iconY = (mIcon.y () + mIcon.height () * objGridY);

	    float stretchedPos;
	    if (mTargetTop)
		stretchedPos = objGridY * origY + (1 - objGridY) * iconY;
	    else
		stretchedPos = (1 - objGridY) * origY + objGridY * iconY;

	    // Compute current y position
	    if (forwardProgress < preShapePhaseEnd)
	    {
		objPos.setY ((1 - stretchProgress) * origY +
			     stretchProgress * stretchedPos);
	    }
	    else
	    {
		if (forwardProgress < stretchPhaseEnd)
		{
		    objPos.setY ((1 - stretchProgress) * origY +
		    		 stretchProgress * stretchedPos);
		}
		else
		{
		    objPos.setY ((1 - postStretchProgress) * stretchedPos +
		    		 postStretchProgress *
		    		 (stretchedPos + (iconCloseEndY - winFarEndY)));
		}
	    }

	    if (mTargetTop)
	    {
	    	// pick the first one that is below icon's bottom (close) edge
		if (objPos.y () > iconCloseEndY &&
		    topmostMovingObjectIdx < 0)
		    topmostMovingObjectIdx = (int)i;

		if (objPos.y () < iconFarEndY)
		    objPos.setY (iconFarEndY);
	    }
	    else
	    {
	    	// pick the first one that is below icon's top (close) edge
		if (objPos.y () > iconCloseEndY &&
		    bottommostMovingObjectIdx < 0)
		    bottommostMovingObjectIdx = (int)i;

		if (objPos.y () > iconFarEndY)
		    objPos.setY (iconFarEndY);
	    }

	    fx = ((iconCloseEndY - objPos.y ()) /
	    	  (iconCloseEndY - winFarEndY));
	}
	else // object is at the right side
	{
	    // Set y position to the y position of the object at the left
	    // on the same row (previous object)
	    objPos.setY ((object - 1)->position ().y ());
	}

	float origX = (mWindow->x () +
		       (winw * objGridX - outExtents.left) *
		       mModel->scale ().x ());
	float iconX =
	    (mIcon.x () - iconShadowLeft) +
	    (mIcon.width () + iconShadowLeft + iconShadowRight) * objGridX;

	// Compute "target shape" x position
	float fy = ((sigmoid (fx) - sigmoid0) /
		    (sigmoid1 - sigmoid0));
	float targetX = fy * (origX - iconX) + iconX;

	filterTargetX (targetX, fx);

	// Compute current x position
	if (forwardProgress < preShapePhaseEnd)
	    objPos.setX ((1 - preShapeProgress) * origX +
	    		 preShapeProgress * targetX);
	else
	    objPos.setX (targetX);

	// No need to set objPos.z () to 0, since they won't be used
	// due to modelAnimIs3D being false for magic lamp.
    }

    if (stepRegionUsed ())
    {
    	// Pick objects that will act as the corners of rectangles subtracted
    	// from this step's damaged region

	const float topCornerRowRatio =
	    (mTargetTop ? 0.55 : 0.35);// 0.46 0.42; // rectangle corner row ratio
	const float bottomCornerRowRatio =
	    (mTargetTop ? 0.65 : 0.42);// 0.46 0.42; // rectangle corner row ratio

	if (topmostMovingObjectIdx < 0)
	    topmostMovingObjectIdx = 0;
	if (bottommostMovingObjectIdx < 0)
	    bottommostMovingObjectIdx = (int)n - 2;

	int nRows = (bottommostMovingObjectIdx - topmostMovingObjectIdx) / 2;
	int firstMovingRow = topmostMovingObjectIdx / 2;
	mTopLeftCornerObject = &mModel->objects ()
	    [(int)(firstMovingRow + topCornerRowRatio * nRows) * 2];
	mBottomLeftCornerObject = &mModel->objects ()
	    [(int)(firstMovingRow + bottomCornerRowRatio * nRows) * 2];
    }
}

void
MagicLampAnim::updateBB (CompOutput &output)
{
    // Just consider the corner objects

    GridModel::GridObject *objects = mModel->objects ();
    unsigned int n = mModel->numObjects ();
    for (unsigned int i = 0; i < n; i++)
    {
	Point3d &objPos = objects[i].position ();
	mAWindow->expandBBWithPoint (objPos.x () + 0.5,
				     objPos.y () + 0.5);

	// skip to the last row after considering the first row
	// (each row has 2 objects)
	if (i == 1)
	    i = n - 3;
    }

    // Subtract a rectangle from each bounding box corner left empty by
    // the animation

    mAWindow->resetStepRegionWithBB ();
    BoxPtr BB = mAWindow->BB ();
    CompRegion &region = mAWindow->stepRegion ();

    // Left side
    if (objects[0].position ().x () >
    	objects[n-2].position ().x ())
    {
    	// Top-left corner is empty

    	// Position of grid object to pick as the corner of the subtracted rect.
    	Point3d &objPos = mTopLeftCornerObject->position ();
	region -= CompRect (BB->x1,
			    BB->y1,
			    objPos.x () - BB->x1,
			    objPos.y () - BB->y1);
    }
    else // Bottom-left corner is empty
    {
    	// Position of grid object to pick as the corner of the subtracted rect.
    	Point3d &objPos = mBottomLeftCornerObject->position ();
    	region -= CompRect (BB->x1,
			    objPos.y (),
			    objPos.x () - BB->x1,
			    BB->y2);
    }

    // Right side
    if (objects[1].position ().x () <
    	objects[n-1].position ().x ())
    {
    	// Top-right corner is empty

    	// Position of grid object to pick as the corner of the subtracted rect.
    	Point3d &objPos = (mTopLeftCornerObject + 1)->position ();
    	region -= CompRect (objPos.x (),
			    BB->y1,
			    BB->x2,
			    objPos.y () - BB->y1);
    }
    else // Bottom-right corner is empty
    {
    	// Position of grid object to pick as the corner of the subtracted rect.
    	Point3d &objPos = (mBottomLeftCornerObject + 1)->position ();
    	region -= CompRect (objPos.x (),
			    objPos.y (),
			    BB->x2,
			    BB->y2);
    }
}

void
MagicLampWavyAnim::updateBB (CompOutput &output)
{
    GridAnim::updateBB (output);
}

void
MagicLampAnim::adjustPointerIconSize ()
{
    mIcon.setWidth (MAX (4, optValI
    			 (AnimationOptions::MagicLampOpenStartWidth)));

    // Adjust position so that the icon is centered at the original position.
    mIcon.setX (mIcon.x () - mIcon.width () / 2);
}

void
MagicLampWavyAnim::adjustPointerIconSize ()
{
    mIcon.setWidth (MAX (4, optValI
    			 (AnimationOptions::MagicLampWavyOpenStartWidth)));

    // Adjust position so that the icon is centered at the original position.
    mIcon.setX (mIcon.x () - mIcon.width () / 2);
}

