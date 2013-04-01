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

// =====================  Effect: Roll Up  =========================

const float RollUpAnim::kDurationFactor = 1.67;

RollUpAnim::RollUpAnim (CompWindow *w,
			WindowEvent curWindowEvent,
			float duration,
			const AnimEffect info,
			const CompRect &icon) :
    Animation::Animation (w, curWindowEvent, kDurationFactor * duration, info,
			  icon),
    GridAnim::GridAnim (w, curWindowEvent, kDurationFactor * duration, info,
			icon)
{
}

void
RollUpAnim::initGrid ()
{
    mGridWidth = 2;
    if (mCurWindowEvent == WindowEventShade ||
	mCurWindowEvent == WindowEventUnshade)
	mGridHeight = 4;
    else
	mGridHeight = 2;
}

void
RollUpAnim::step ()
{
    float forwardProgress = progressEaseInEaseOut ();
    bool fixedInterior = optValB (AnimationOptions::RollupFixedInterior);

    CompRect outRect (mAWindow->savedRectsValid () ?
		      mAWindow->savedOutRect () :
		      mWindow->outputRect ());

    int ox = outRect.x ();
    int oy = outRect.y ();
    int owidth = outRect.width ();
    int oheight = outRect.height ();

    GridModel::GridObject *object = mModel->objects ();
    unsigned int n = mModel->numObjects ();
    for (unsigned int i = 0; i < n; i++, object++)
    {
	// Executing shade mode

	Point3d &objPos = object->position ();

	if (i % 2 == 0) // object is at the left side
	{
	    float objGridY = object->gridPosition ().y ();

	    if (objGridY == 0)
	    {
		objPos.setY (oy);
	    }
	    else if (objGridY == 1)
	    {
		objPos.setY (
		    (1 - forwardProgress) * (oy + oheight * objGridY) +
		    forwardProgress * (oy +
				       mDecorTopHeight + mDecorBottomHeight));
	    }
	    else
	    {
		// find position in window contents
		// (window contents correspond to 0.0-1.0 range)
		float relPosInWinContents =
		    (objGridY * oheight -
		     mDecorTopHeight) / mWindow->height ();

		if (relPosInWinContents > forwardProgress)
		{
		    objPos.setY (
			(1 - forwardProgress) * (oy + oheight * objGridY) +
			forwardProgress * (oy + mDecorTopHeight));

		    if (fixedInterior)
			object->offsetTexCoordForQuadBefore ().
			    setY (-forwardProgress * mWindow->height ());
		}
		else
		{
		    objPos.setY (oy + mDecorTopHeight);
		    if (!fixedInterior)
			object->offsetTexCoordForQuadAfter ().
			    setY ((forwardProgress - relPosInWinContents) *
				  mWindow->height ());
		}
	    }
	}
	else // object is at the right side
	{
	    // Set y position to the y position of the object at the left
	    // on the same row (previous object)
	    objPos.setY ((object - 1)->position ().y ());

	    // Also copy offset texture y coordinates
	    object->offsetTexCoordForQuadBefore ().
		setY ((object - 1)->offsetTexCoordForQuadBefore ().y ());
	    object->offsetTexCoordForQuadAfter ().
		setY ((object - 1)->offsetTexCoordForQuadAfter ().y ());
	}

	float origx = ox + owidth * object->gridPosition ().x ();

	objPos.setX (origx);
    }
}

