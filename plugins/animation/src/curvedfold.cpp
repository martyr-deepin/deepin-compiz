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

// =====================  Effect: Curved Fold  =========================

FoldAnim::FoldAnim (CompWindow *w,
		    WindowEvent curWindowEvent,
		    float duration,
		    const AnimEffect info,
		    const CompRect &icon) :
    Animation::Animation (w, curWindowEvent, duration, info, icon),
    TransformAnim::TransformAnim (w, curWindowEvent, duration, info, icon),
    GridZoomAnim::GridZoomAnim (w, curWindowEvent, duration, info, icon)
{
}

float
FoldAnim::getFadeProgress ()
{
    // if shade/unshade, don't do anything
    if (mCurWindowEvent == WindowEventShade ||
	mCurWindowEvent == WindowEventUnshade)
	return 0;

    if (zoomToIcon ())
	return ZoomAnim::getFadeProgress ();

    return progressLinear ();
}

CurvedFoldAnim::CurvedFoldAnim (CompWindow *w,
				WindowEvent curWindowEvent,
				float duration,
				const AnimEffect info,
				const CompRect &icon) :
    Animation::Animation (w, curWindowEvent, duration, info, icon),
    TransformAnim::TransformAnim (w, curWindowEvent, duration, info, icon),
    FoldAnim::FoldAnim (w, curWindowEvent, duration, info, icon)
{
}

void
CurvedFoldAnim::initGrid ()
{
    mGridWidth = 2;
    mGridHeight = optValI (AnimationOptions::MagicLampWavyGridRes); // TODO new option
}

float
CurvedFoldAnim::getObjectZ (GridAnim::GridModel *mModel,
			    float forwardProgress,
			    float sinForProg,
			    float relDistToCenter,
			    float curveMaxAmp)
{
    return -(sinForProg *
	     (1 - pow (pow (2 * relDistToCenter, 1.3), 2)) *
	     curveMaxAmp *
	     mModel->scale ().x ());
}

void
CurvedFoldAnim::step ()
{
    GridZoomAnim::step ();

    float forwardProgress = getActualProgress ();

    CompRect winRect (mAWindow->savedRectsValid () ?
		      mAWindow->saveWinRect () :
		      mWindow->geometry ());
    CompRect inRect (mAWindow->savedRectsValid () ?
		     mAWindow->savedInRect () :
		     mWindow->borderRect ());
    CompRect outRect (mAWindow->savedRectsValid () ?
		      mAWindow->savedOutRect () :
		      mWindow->outputRect ());
    CompWindowExtents outExtents (mAWindow->savedRectsValid () ?
				  mAWindow->savedOutExtents () :
				  mWindow->output ());

    int wx = winRect.x ();
    int wy = winRect.y ();
    int wheight = winRect.height ();

    int oy = outRect.y ();
    int owidth = outRect.width ();
    int oheight = outRect.height ();

    float curveMaxAmp = (0.4 * pow ((float)oheight /
				    ::screen->height (), 0.4) *
			 optValF (AnimationOptions::CurvedFoldAmpMult));

    float sinForProg = sin (forwardProgress * M_PI / 2);

    GridModel::GridObject *object = mModel->objects ();
    unsigned int n = mModel->numObjects ();
    for (unsigned int i = 0; i < n; i++, object++)
    {
	Point3d &objPos = object->position ();

	if (i % 2 == 0) // object is at the left side
	{
	    float objGridY = object->gridPosition ().y ();

	    float origy = (wy +
			   (oheight * objGridY -
			    outExtents.top) * mModel->scale ().y ());

	    if (mCurWindowEvent == WindowEventShade ||
		mCurWindowEvent == WindowEventUnshade)
	    {
		// Execute shade mode

		// find position in window contents
		// (window contents correspond to 0.0-1.0 range)
		float relPosInWinContents =
		    (objGridY * oheight -
		     mDecorTopHeight) / wheight;
		float relDistToCenter = fabs (relPosInWinContents - 0.5);

		if (objGridY == 0)
		{
		    objPos.setY (oy);
		    objPos.setZ (0);
		}
		else if (objGridY == 1)
		{
		    objPos.setY (
			(1 - forwardProgress) * origy +
			forwardProgress *
			(oy + mDecorTopHeight + mDecorBottomHeight));
		    objPos.setZ (0);
		}
		else
		{
		    objPos.setY (
			(1 - forwardProgress) * origy +
			forwardProgress * (oy + mDecorTopHeight));
		    objPos.setZ (
			getObjectZ (mModel, forwardProgress, sinForProg, relDistToCenter,
				    curveMaxAmp));
		}
	    }
	    else
	    {
		// Execute normal mode

		// find position within window borders
		// (border contents correspond to 0.0-1.0 range)
		float relPosInWinBorders =
		    (objGridY * oheight -
		     (inRect.y () - oy)) / inRect.height ();
		float relDistToCenter = fabs (relPosInWinBorders - 0.5);

		// prevent top & bottom shadows from extending too much
		if (relDistToCenter > 0.5)
		    relDistToCenter = 0.5;

		objPos.setY (
		    (1 - forwardProgress) * origy +
		    forwardProgress * (inRect.y () + inRect.height () / 2.0));
		objPos.setZ (
		    getObjectZ (mModel, forwardProgress, sinForProg, relDistToCenter,
				curveMaxAmp));
	    }
	}
	else // object is at the right side
	{
	    // Set y/z position to the y/z position of the object at the left
	    // on the same row (previous object)
	    Point3d &leftObjPos = (object - 1)->position ();
	    objPos.setY (leftObjPos.y ());
	    objPos.setZ (leftObjPos.z ());
	}

	float origx = (wx +
		       (owidth * object->gridPosition ().x () -
			outExtents.left) * mModel->scale ().x ());
	objPos.setX (origx);
    }
}

void
CurvedFoldAnim::updateBB (CompOutput &output)
{
    if (optValF (AnimationOptions::CurvedFoldAmpMult) < 0) // if outward
    {
	GridZoomAnim::updateBB (output); // goes through all objects
	return;
    }

    // Just consider the corner objects

    GridModel::GridObject *objects = mModel->objects ();
    unsigned int n = mModel->numObjects ();
    for (unsigned int i = 0; i < n; i++)
    {
	Point3d &pos = objects[i].position ();
	GLVector coords (pos.x (), pos.y (), 0, 1);
	mAWindow->expandBBWithPoint2DTransform (coords, mTransform);

	if (i == 1)
	{
	    // skip to the last row after considering the first row
	    // (each row has 2 objects)
	    i = n - 3;
	}
    }
}

bool
CurvedFoldAnim::zoomToIcon ()
{
    return ((mCurWindowEvent == WindowEventMinimize ||
	     mCurWindowEvent == WindowEventUnminimize) &&
	    optValB (AnimationOptions::CurvedFoldZoomToTaskbar));
}

