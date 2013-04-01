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

// =====================  Effect: Domino and Razr  =========================

const float DominoAnim::kDurationFactor = 1.25;

DominoAnim::DominoAnim (CompWindow *w,
				WindowEvent curWindowEvent,
				float duration,
				const AnimEffect info,
				const CompRect &icon) :
    Animation::Animation (w, curWindowEvent, kDurationFactor * duration, info,
			  icon),
    PolygonAnim::PolygonAnim (w, curWindowEvent, kDurationFactor * duration,
			      info, icon)
{
    mDoDepthTest = true;
    mDoLighting = true;
    mCorrectPerspective = CorrectPerspectivePolygon;
}

RazrAnim::RazrAnim (CompWindow *w,
		    WindowEvent curWindowEvent,
		    float duration,
		    const AnimEffect info,
		    const CompRect &icon) :
    Animation::Animation (w, curWindowEvent, kDurationFactor * duration, info,
			  icon),
    DominoAnim::DominoAnim (w, curWindowEvent, duration, info, icon)
{
}

void
DominoAnim::init ()
{
    bool isRazr = (typeid (*this) == typeid (RazrAnim));
    int fallDir;

    if (isRazr)
	fallDir = getActualAnimDirection
	    ((AnimDirection) optValI (AnimationaddonOptions::RazrDirection),
	     true);
    else
	fallDir = getActualAnimDirection
	    ((AnimDirection) optValI (AnimationaddonOptions::DominoDirection),
	     true);

    int defaultGridSize = 20;
    float minCellSize = 30;
    int gridSizeX;
    int gridSizeY;
    int fallDirGridSize;
    float minDistStartEdge;		// half piece size in [0,1] range
    float gridCellW;
    float gridCellH;
    float cellAspectRatio = 1.25;

    if (isRazr)
	cellAspectRatio = 1;

    CompRect outRect (mAWindow->savedRectsValid () ?
		      mAWindow->savedOutRect () :
		      mWindow->outputRect ());

    // Determine sensible domino piece sizes
    if (fallDir == AnimDirectionDown || fallDir == AnimDirectionUp)
    {
	if (minCellSize > outRect.width ())
	    minCellSize = outRect.width ();
	if (outRect.width () / defaultGridSize < minCellSize)
	    gridSizeX = (int)(outRect.width () / minCellSize);
	else
	    gridSizeX = defaultGridSize;
	gridCellW = outRect.width () / gridSizeX;
	gridSizeY = (int)(outRect.height () / (gridCellW * cellAspectRatio));
	if (gridSizeY == 0)
	    gridSizeY = 1;
	gridCellH = outRect.height () / gridSizeY;
	fallDirGridSize = gridSizeY;
    }
    else
    {
	if (minCellSize > outRect.height ())
	    minCellSize = outRect.height ();
	if (outRect.height () / defaultGridSize < minCellSize)
	    gridSizeY = (int)(outRect.height () / minCellSize);
	else
	    gridSizeY = defaultGridSize;
	gridCellH = outRect.height () / gridSizeY;
	gridSizeX = (int)(outRect.width () / (gridCellH * cellAspectRatio));
	if (gridSizeX == 0)
	    gridSizeX = 1;
	gridCellW = outRect.width () / gridSizeX;
	fallDirGridSize = gridSizeX;
    }
    minDistStartEdge = (1.0 / fallDirGridSize) / 2;

    float thickness = MIN (gridCellW, gridCellH) / 3.5;

    if (!tessellateIntoRectangles (gridSizeX, gridSizeY, thickness))
	return;

    float rotAxisX = 0;
    float rotAxisY = 0;
    Point3d rotAxisOff (0, 0, thickness / 2);
    float posX = 0;				// position of standing piece
    float posY = 0;
    float posZ = 0;
    int nFallingColumns = gridSizeX;
    float gridCellHalfW = gridCellW / 2;
    float gridCellHalfH = gridCellH / 2;

    switch (fallDir)
    {
    case AnimDirectionDown:
	rotAxisX = -1;
	if (isRazr)
	    rotAxisOff.setY (-gridCellHalfH);
	else
	{
	    posY = -(gridCellHalfH + thickness);
	    posZ = gridCellHalfH - thickness / 2;
	}
	break;
    case AnimDirectionLeft:
	rotAxisY = -1;
	if (isRazr)
	    rotAxisOff.setX (gridCellHalfW);
	else
	{
	    posX = gridCellHalfW + thickness;
	    posZ = gridCellHalfW - thickness / 2;
	}
	nFallingColumns = gridSizeY;
	break;
    case AnimDirectionUp:
	rotAxisX = 1;
	if (isRazr)
	    rotAxisOff.setY (gridCellHalfH);
	else
	{
	    posY = gridCellHalfH + thickness;
	    posZ = gridCellHalfH - thickness / 2;
	}
	break;
    case AnimDirectionRight:
	rotAxisY = 1;
	if (isRazr)
	    rotAxisOff.setX (-gridCellHalfW);
	else
	{
	    posX = -(gridCellHalfW + thickness);
	    posZ = gridCellHalfW - thickness / 2;
	}
	nFallingColumns = gridSizeY;
	break;
    }

    float fadeDuration;
    float riseDuration;
    float riseTimeRandMax = 0.2;

    if (isRazr)
    {
	riseDuration = (1 - riseTimeRandMax) / fallDirGridSize;
	fadeDuration = riseDuration / 2;
    }
    else
    {
	fadeDuration = 0.18;
	riseDuration = 0.2;
    }

    float riseTimeRandSeed[nFallingColumns];

    for (int i = 0; i < nFallingColumns; i++)
	riseTimeRandSeed[i] = RAND_FLOAT ();

    foreach (PolygonObject *p, mPolygons)
    {
	p->rotAxis.set (rotAxisX, rotAxisY, 0);
	p->finalRelPos.set (posX, posY, posZ);

	// dist. from rise-start / fall-end edge in window ([0,1] range)
	float distStartEdge = 0;

	// dist. from edge perpendicular to movement (for move start time)
	// so that same the blocks in same row/col. appear to knock down
	// the next one
	float distPerpEdge = 0;

	switch (fallDir)
	{
	case AnimDirectionUp:
	    distStartEdge = p->centerRelPos.y ();
	    distPerpEdge = p->centerRelPos.x ();
	    break;
	case AnimDirectionRight:
	    distStartEdge = 1 - p->centerRelPos.x ();
	    distPerpEdge = p->centerRelPos.y ();
	    break;
	case AnimDirectionDown:
	    distStartEdge = 1 - p->centerRelPos.y ();
	    distPerpEdge = p->centerRelPos.x ();
	    break;
	case AnimDirectionLeft:
	    distStartEdge = p->centerRelPos.x ();
	    distPerpEdge = p->centerRelPos.y ();
	    break;
	}

	float riseTimeRand =
	    riseTimeRandSeed[(int)(distPerpEdge * nFallingColumns)] *
	    riseTimeRandMax;

	p->moveDuration = riseDuration;

	float mult = 1;
	if (fallDirGridSize > 1)
	    mult = ((distStartEdge - minDistStartEdge) /
		    (1 - 2 * minDistStartEdge));
	if (isRazr)
	{
	    p->moveStartTime = mult *
		(1 - riseDuration - riseTimeRandMax) + riseTimeRand;
	    p->fadeStartTime = p->moveStartTime + riseDuration / 2;
	    p->finalRotAng = -180;

	    p->rotAxisOffset = rotAxisOff;
	}
	else
	{
	    p->moveStartTime =
		mult *
		(1 - riseDuration - riseTimeRandMax) +
		riseTimeRand;
	    p->fadeStartTime =
		p->moveStartTime + riseDuration - riseTimeRand + 0.03;
	    p->finalRotAng = -90;
	}
	if (p->fadeStartTime > 1 - fadeDuration)
	    p->fadeStartTime = 1 - fadeDuration;
	p->fadeDuration = fadeDuration;
    }
}
