/**
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
 * Hexagon tessellator added by : Mike Slegeir
 * E-mail                       : mikeslegeir@mail.utexas.edu>
 *
 * Fold and Skewer added by : Tomasz Kołodziejski
 * E-mail                   : tkolodziejski@gmail.com
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
 **/

#include "private.h"

// =====================  Effect: Skewer  =========================

const float SkewerAnim::kDurationFactor = 1.67;

SkewerAnim::SkewerAnim (CompWindow *w,
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
    mCorrectPerspective = CorrectPerspectiveWindow;
}

static void
getDirection (int *dir, int *c, int direction)
{
    switch (direction)
    {
    case 0:
	// left
	dir[(*c)++] = 0;
	break;
    case 1:
	// right
	dir[(*c)++] = 1;
	break;
    case 2:
	// left-right   
	dir[(*c)++] = 0;
	dir[(*c)++] = 1;
	break;
    case 3:
	// up
	dir[(*c)++] = 2;
	break;
    case 4:
	// downs
	dir[(*c)++] = 3;
	break;
    case 5:
	// up-down
	dir[(*c)++] = 2;
	dir[(*c)++] = 3;
	break;
    case 6:
	// in
	dir[(*c)++] = 4;
	break;
    case 7:
	// out
	dir[(*c)++] = 5;
	break;
    case 8:
	// in-out
	dir[(*c)++] = 4;
	dir[(*c)++] = 5;
	break;
    case 9:
	// random
	getDirection (dir, c, floor (RAND_FLOAT () * 8));
	break;
    }
}

void
SkewerAnim::init ()
{
    float thickness = optValF (AnimationaddonOptions::SkewerThickness);
    int rotation = optValI (AnimationaddonOptions::SkewerRotation);
    int gridSizeX = optValI (AnimationaddonOptions::SkewerGridx);
    int gridSizeY = optValI (AnimationaddonOptions::SkewerGridy);

    int dir[2];			// directions array
    int c = 0;			// number of directions

    getDirection (dir, &c,
		  (int) optValI (AnimationaddonOptions::SkewerDirection));

    if (optValI (AnimationaddonOptions::SkewerTessellation) ==
	AnimationaddonOptions::SkewerTessellationHexagonal)
    {
	if (!tessellateIntoHexagons (gridSizeX, gridSizeY, thickness))
	    return;
    }
    else
    {
	if (!tessellateIntoRectangles (gridSizeX, gridSizeY, thickness))
	    return;
    }

    int numPolygons = mPolygons.size ();
    int times[numPolygons];
    int last_time = numPolygons - 1;

    int maxZ = .8 * DEFAULT_Z_CAMERA * ::screen->width ();

    int i;
    for (i = 0; i < numPolygons; i++)
	times[i] = i;

    foreach (PolygonObject *p, mPolygons)
    {
	if (c > 0)
	{
	    switch (dir[(int)floor (RAND_FLOAT () * c)])
	    {
	    case 0:
		// left
		p->finalRelPos.setX (-::screen->width ());
		p->rotAxis.setX (rotation);
		break;

	    case 1:
		// right
		p->finalRelPos.setX (::screen->width ());
		p->rotAxis.setX (rotation);
		break;

	    case 2:
		// up
		p->finalRelPos.setY (-::screen->height ());
		p->rotAxis.setY (rotation);
		break;

	    case 3:
		// down
		p->finalRelPos.setY (::screen->height ());
		p->rotAxis.setY (rotation);
		break;

	    case 4:
		// in
		p->finalRelPos.setZ (-maxZ);
		p->rotAxis.setX (rotation);
		p->rotAxis.setY (rotation);
		break;

	    case 5:
		// out
		p->finalRelPos.setZ (maxZ);
		p->rotAxis.setX (rotation);
		p->rotAxis.setY (rotation);
		break;
	    }

	    p->finalRotAng = rotation;
	}
	// if no direction is set - just fade

	// choose random start_time
	int rand_time = floor (RAND_FLOAT () * last_time);

	p->moveStartTime = 0.8 / (float)numPolygons * times[rand_time];
	p->moveDuration = 1 - p->moveStartTime;

	p->fadeStartTime = p->moveStartTime + 0.2;
	p->fadeDuration = 1 - p->fadeStartTime;

	times[rand_time] = times[last_time];	// copy last one over times[rand_time]
	last_time--;		//descrease last_time
    }
}

void
SkewerAnim::stepPolygon (PolygonObject *p,
			 float forwardProgress)
{
    float moveProgress = forwardProgress - p->moveStartTime;

    if (p->moveDuration > 0)
	moveProgress /= p->moveDuration;
    if (moveProgress < 0)
	moveProgress = 0;
    else if (moveProgress > 1)
	moveProgress = 1;

    p->centerPos.set (pow (moveProgress, 2) * p->finalRelPos.x () +
		     p->centerPosStart.x (),
		     pow (moveProgress, 2) * p->finalRelPos.y () +
		     p->centerPosStart.y (),
		     1.0f / ::screen->width () *
		     pow (moveProgress, 2) * p->finalRelPos.z () +
		     p->centerPosStart.z ());

    // rotate
    p->rotAngle = pow (moveProgress, 2) * p->finalRotAng + p->rotAngleStart;
}
