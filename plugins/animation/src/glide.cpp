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

// =====================  Effect: Glide  =========================

GlideAnim::GlideAnim (CompWindow *w,
		      WindowEvent curWindowEvent,
		      float duration,
		      const AnimEffect info,
		      const CompRect &icon) :
    Animation::Animation (w, curWindowEvent, duration, info, icon),
    TransformAnim::TransformAnim (w, curWindowEvent, duration, info, icon),
    ZoomAnim::ZoomAnim (w, curWindowEvent, duration, info, icon)
{
}

Glide2Anim::Glide2Anim (CompWindow *w,
			WindowEvent curWindowEvent,
			float duration,
			const AnimEffect info,
			const CompRect &icon) :
    Animation::Animation (w, curWindowEvent, duration, info, icon),
    TransformAnim::TransformAnim (w, curWindowEvent, duration, info, icon),
    GlideAnim::GlideAnim (w, curWindowEvent, duration, info, icon)
{
}

void
GlideAnim::getParams (float *finalDistFac,
		      float *finalRotAng,
		      float *thickness)
{
    *finalDistFac = optValF (AnimationOptions::Glide1AwayPosition);
    *finalRotAng = optValF (AnimationOptions::Glide1AwayAngle);
}

void
Glide2Anim::getParams (float *finalDistFac,
		       float *finalRotAng,
		       float *thickness)
{
    *finalDistFac = optValF (AnimationOptions::Glide2AwayPosition);
    *finalRotAng = optValF (AnimationOptions::Glide2AwayAngle);
}

float
GlideAnim::getProgress ()
{
    float forwardProgress = progressLinear ();

    return progressDecelerate (forwardProgress);
}

float
GlideAnim::getFadeProgress ()
{
    if (zoomToIcon ())
	return ZoomAnim::getFadeProgress ();

    return getProgress ();
}

void
GlideAnim::applyTransform ()
{
    if (zoomToIcon ())
	ZoomAnim::applyTransform ();

    float finalDistFac;
    float finalRotAng;
    float thickness;

    getParams (&finalDistFac, &finalRotAng, &thickness);

    float forwardProgress;
    if (zoomToIcon ())
	getZoomProgress (&forwardProgress, 0, true);
    else
	forwardProgress = getProgress ();

    float finalz = finalDistFac * 0.8 * DEFAULT_Z_CAMERA *
	::screen->width ();

    CompRect outRect (mAWindow->savedRectsValid () ?
		      mAWindow->savedOutRect () :
		      mWindow->outputRect ());

    GLVector rotAxis (1, 0, 0, 0);
    GLVector rotAxisOffset (outRect.x () + outRect.width () / 2.0f,
			    outRect.y () + outRect.height () / 2.0f,
			    0, 0);
    GLVector translation (0, 0, finalz * forwardProgress, 0);

    float rotAngle = finalRotAng * forwardProgress;
    glideModRotAngle = fmodf (rotAngle + 720, 360.0f);

    // put back to window position
    mTransform.translate (rotAxisOffset);

    perspectiveDistortAndResetZ (mTransform);

    // animation movement
    mTransform.translate (translation);

    // animation rotation
    mTransform.rotate (rotAngle, rotAxis);

    // intentional scaling of z by 0 to prevent weird opacity results and
    // flashing that happen when z coords are between 0 and 1 (bug in compiz?)
    mTransform.scale (1.0f, 1.0f, 0.0f);

    // place window rotation axis at origin
    mTransform.translate (-rotAxisOffset);
}

void
GlideAnim::adjustDuration ()
{
    if (zoomToIcon ())
    {
	mTotalTime *= kDurationFactor;
	mRemainingTime = mTotalTime;
    }
}

void
GlideAnim::prePaintWindow ()
{
    if (90 < glideModRotAngle &&
	glideModRotAngle < 270)
	glCullFace (GL_FRONT);
}

void
GlideAnim::postPaintWindow ()
{
    if (90 < glideModRotAngle &&
	glideModRotAngle < 270)
	glCullFace (GL_BACK);
}

bool
GlideAnim::zoomToIcon ()
{
    return ((mCurWindowEvent == WindowEventMinimize ||
	     mCurWindowEvent == WindowEventUnminimize) &&
	    optValB (AnimationOptions::Glide1ZoomToTaskbar));
}

bool
Glide2Anim::zoomToIcon ()
{
    return ((mCurWindowEvent == WindowEventMinimize ||
	     mCurWindowEvent == WindowEventUnminimize) &&
	    optValB (AnimationOptions::Glide2ZoomToTaskbar));
}

