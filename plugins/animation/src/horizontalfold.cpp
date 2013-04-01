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

// =====================  Effect: Horizontal Folds  =========================

HorizontalFoldsAnim::HorizontalFoldsAnim (CompWindow *w,
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
HorizontalFoldsAnim::initGrid ()
{
    mGridWidth = 2;
    if (mCurWindowEvent == WindowEventShade ||
	mCurWindowEvent == WindowEventUnshade)
	mGridHeight = 3 + 2 *
	    optValI (AnimationOptions::HorizontalFoldsNumFolds);
    else
	mGridHeight = 1 + 2 *
	    optValI (AnimationOptions::HorizontalFoldsNumFolds);
}

float
HorizontalFoldsAnim::getObjectZ (GridAnim::GridModel *mModel,
				 float forwardProgress,
				 float sinForProg,
				 float relDistToFoldCenter,
				 float foldMaxAmp)
{
    return -(sinForProg *
	     foldMaxAmp *
	     mModel->scale ().x () *
	     2 * (0.5 - relDistToFoldCenter));
}

void
HorizontalFoldsAnim::step ()
{
    GridZoomAnim::step ();

    CompRect winRect (mAWindow->savedRectsValid () ?
		      mAWindow->saveWinRect () :
		      mWindow->geometry ());
    CompRect inRect (mAWindow->savedRectsValid () ?
		     mAWindow->savedInRect () :
		     mWindow->inputRect ());
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

    float winHeight = 0;
    if (mCurWindowEvent == WindowEventShade ||
	mCurWindowEvent == WindowEventUnshade)
    {
	winHeight = winRect.height ();
    }
    else
    {
	winHeight = inRect.height ();
    }
    int nHalfFolds =
	2.0 * optValI (AnimationOptions::HorizontalFoldsNumFolds);
    float foldMaxAmp =
	0.3 * pow ((winHeight / nHalfFolds) / ::screen->height (), 0.3) *
	optValF (AnimationOptions::HorizontalFoldsAmpMult);

    float forwardProgress = getActualProgress ();

    float sinForProg = sin (forwardProgress * M_PI / 2);

    GridModel::GridObject *object = mModel->objects ();
    unsigned int n = mModel->numObjects ();
    for (unsigned int i = 0; i < n; i++, object++)
    {
	Point3d &objPos = object->position ();

	if (i % 2 == 0) // object is at the left side
	{
	    float objGridY = object->gridPosition ().y ();

	    int rowNo = (int)i / mGridWidth;
	    float origy = (wy +
			   (oheight * objGridY -
			    outExtents.top) * mModel->scale ().y ());
	    if (mCurWindowEvent == WindowEventShade ||
		mCurWindowEvent == WindowEventUnshade)
	    {
		// Execute shade mode

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
		    float relDistToFoldCenter = (rowNo % 2 == 1 ? 0.5 : 0);

		    objPos.setY (
			(1 - forwardProgress) * origy +
			forwardProgress * (oy + mDecorTopHeight));
		    objPos.setZ (
			getObjectZ (mModel, forwardProgress, sinForProg,
				    relDistToFoldCenter, foldMaxAmp));
		}
	    }
	    else
	    {
		// Execute normal mode

		float relDistToFoldCenter = (rowNo % 2 == 0 ? 0.5 : 0);

		objPos.setY (
		    (1 - forwardProgress) * origy +
		    forwardProgress * (inRect.y () + inRect.height () / 2.0));
		objPos.setZ (
			getObjectZ (mModel, forwardProgress, sinForProg,
				    relDistToFoldCenter, foldMaxAmp));
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

bool
HorizontalFoldsAnim::zoomToIcon ()
{
    return ((mCurWindowEvent == WindowEventMinimize ||
	     mCurWindowEvent == WindowEventUnminimize) &&
	    optValB (AnimationOptions::HorizontalFoldsZoomToTaskbar));
}

