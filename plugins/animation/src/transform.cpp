/*
 * Animation plugin for compiz
 *
 * transform.cpp
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

TransformAnim::TransformAnim (CompWindow *w,
			      WindowEvent curWindowEvent,
			      float duration,
			      const AnimEffect info,
			      const CompRect &icon) :
    Animation::Animation (w, curWindowEvent, duration, info, icon),
    mTransformStartProgress (0.0f),
    mTransformProgress (0.0f)
{
}

void
TransformAnim::init ()
{
    adjustDuration ();
}

void
TransformAnim::updateBB (CompOutput &output)
{
    GLMatrix wTransform;

    prepareTransform (output, wTransform, mTransform);

    CompRect outRect (mAWindow->savedRectsValid () ?
		      mAWindow->savedOutRect () :
		      mWindow->outputRect ());
    float corners[4*3] = {
	static_cast <float> (outRect.x ()), static_cast <float> (outRect.y ()), 0,
	static_cast <float> (outRect.x () + outRect.width ()), static_cast <float> (outRect.y ()), 0,
	static_cast <float> (outRect.x ()), static_cast <float> (outRect.y () + outRect.height ()), 0,
	static_cast <float> (outRect.x () + outRect.width ()), static_cast <float> (outRect.y () + outRect.height ()), 0
    };
    mAWindow->expandBBWithPoints3DTransform (output,
					     wTransform,
					     corners,
					     0,
					     4);
}

void
TransformAnim::step ()
{
    mTransform.reset ();
    applyTransform ();
}

void
TransformAnim::updateTransform (GLMatrix &wTransform)
{
    wTransform *= mTransform;
}

/// Scales z by 0 and does perspective distortion so that it
/// looks the same wherever on the screen.
void
TransformAnim::perspectiveDistortAndResetZ (GLMatrix &transform)
{
    float v = -1.0 / ::screen->width ();
    /*
      This does
      transform = M * transform, where M is
      1, 0, 0, 0,
      0, 1, 0, 0,
      0, 0, 0, v,
      0, 0, 0, 1
    */

    transform[8] = v * transform[12];
    transform[9] = v * transform[13];
    transform[10] = v * transform[14];
    transform[11] = v * transform[15];
}

void
TransformAnim::applyPerspectiveSkew (CompOutput &output,
				     GLMatrix &transform,
				     Point &center)
{
    GLfloat skewx = -(((center.x () - output.region ()->extents.x1) -
		       output.width () / 2) * 1.15);
    GLfloat skewy = -(((center.y () - output.region ()->extents.y1) -
		       output.height () / 2) * 1.15);

    /* transform = M * transform, where M is the skew matrix
	{1,0,0,0,
	 0,1,0,0,
	 skewx,skewy,1,0,
	 0,0,0,1};
    */

    transform[8] = skewx * transform[0] + skewy * transform[4] + transform[8];
    transform[9] = skewx * transform[1] + skewy * transform[5] + transform[9];
    transform[10] = skewx * transform[2] + skewy * transform[6] + transform[10];
    transform[11] = skewx * transform[3] + skewy * transform[7] + transform[11];
}

Point
TransformAnim::getCenter ()
{
    CompRect inRect (mAWindow->savedRectsValid () ?
		     mAWindow->savedInRect () :
		     mWindow->borderRect ());
    Point center (inRect.x () + inRect.width () / 2,
		  inRect.y () + inRect.height () / 2);
    return center;
}

