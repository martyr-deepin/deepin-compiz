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

const float WaveAnim::kMinDuration = 400;

// =====================  Effect: Wave  =========================

WaveAnim::WaveAnim (CompWindow *w,
		    WindowEvent curWindowEvent,
		    float duration,
		    const AnimEffect info,
		    const CompRect &icon) :
    Animation::Animation (w, curWindowEvent, duration, info, icon),
    TransformAnim::TransformAnim (w, curWindowEvent, duration, info, icon),
    GridTransformAnim::GridTransformAnim (w, curWindowEvent, duration, info,
					  icon)
{
}

void
WaveAnim::adjustDuration ()
{
    if (mTotalTime < kMinDuration)
    {
	mTotalTime = kMinDuration;
	mRemainingTime = mTotalTime;
    }
}

void
WaveAnim::initGrid ()
{
    mGridWidth = 2;
    mGridHeight = optValI (AnimationOptions::MagicLampWavyGridRes); // TODO new option
}

void
WaveAnim::step ()
{
    float forwardProgress = 1 - progressLinear ();
    if (mCurWindowEvent == WindowEventClose)
	forwardProgress = 1 - forwardProgress;

    CompRect winRect (mAWindow->savedRectsValid () ?
		      mAWindow->saveWinRect () :
		      mWindow->geometry ());
    CompRect outRect (mAWindow->savedRectsValid () ?
		      mAWindow->savedOutRect () :
		      mWindow->outputRect ());
    CompWindowExtents outExtents (mAWindow->savedRectsValid () ?
				  mAWindow->savedOutExtents () :
				  mWindow->output ());

    int wx = winRect.x ();
    int wy = winRect.y ();

    int oy = outRect.y ();
    int owidth = outRect.width ();
    int oheight = outRect.height ();

    float waveHalfWidth = (oheight * mModel->scale ().y () *
			   optValF (AnimationOptions::WaveWidth) / 2);

    float waveAmp = (pow ((float)oheight / ::screen->height (), 0.4) *
		     0.04 * optValF (AnimationOptions::WaveAmpMult));

    float wavePosition =
	oy - waveHalfWidth +
	forwardProgress * (oheight * mModel->scale ().y () + 2 * waveHalfWidth);

    GridModel::GridObject *object = mModel->objects ();
    unsigned int n = mModel->numObjects ();
    for (unsigned int i = 0; i < n; i++, object++)
    {
	Point3d &objPos = object->position ();

	if (i % 2 == 0) // object is at the left side
	{
	    float origy = wy + mModel->scale ().y () *
		(oheight * object->gridPosition ().y () -
		 outExtents.top);
	    objPos.setY (origy);

	    float distFromWaveCenter =
		fabs (objPos.y () - wavePosition);

	    if (distFromWaveCenter < waveHalfWidth)
		objPos.
		    setZ (waveAmp * (cos (distFromWaveCenter *
					  M_PI / waveHalfWidth) + 1) / 2);
	    else
		objPos.setZ (0);
	}
	else // object is at the right side
	{
	    // Set y/z position to the y/z position of the object at the left
	    // on the same row (previous object)
	    Point3d &leftObjPos = (object - 1)->position ();
	    objPos.setY (leftObjPos.y ());
	    objPos.setZ (leftObjPos.z ());
	}

	float origx = wx + mModel->scale ().x () *
	    (owidth * object->gridPosition ().x () -
	     outExtents.left);
	objPos.setX (origx);
    }
}

