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

// =====================  Effect: Leaf Spread  =========================

const float LeafSpreadAnim::kDurationFactor = 1.67;

LeafSpreadAnim::LeafSpreadAnim (CompWindow *w,
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

void
LeafSpreadAnim::init ()
{
    if (!tessellateIntoRectangles (20, 14, 15.0f))
	return;

    CompRect outRect (mAWindow->savedRectsValid () ?
		      mAWindow->savedOutRect () :
		      mWindow->outputRect ());

    float fadeDuration = 0.26;
    float life = 0.4;
    float spreadFac = 3.5;
    float randYMax = 0.07;
    float winFacX = outRect.width () / 800.0;
    float winFacY = outRect.height () / 800.0;
    float winFacZ = (outRect.height () + outRect.width ()) / 2.0 / 800.0;

    float screenSizeFactor = (0.8 * DEFAULT_Z_CAMERA * ::screen->width ());

    foreach (PolygonObject *p, mPolygons)
    {
	p->rotAxis.set (RAND_FLOAT (), RAND_FLOAT (), RAND_FLOAT ());

	float speed = screenSizeFactor / 10 * (0.2 + RAND_FLOAT ());

	float xx = 2 * (p->centerRelPos.x () - 0.5);
	float yy = 2 * (p->centerRelPos.y () - 0.5);

	float x = speed * winFacX * spreadFac * (xx +
						 0.5 * (RAND_FLOAT () - 0.5));
	float y = speed * winFacY * spreadFac * (yy +
						 0.5 * (RAND_FLOAT () - 0.5));
	float z = speed * winFacZ * 7 * ((RAND_FLOAT () - 0.5) / 0.5);

	p->finalRelPos.set (x, y, z);

	p->moveStartTime =
	    p->centerRelPos.y () * (1 - fadeDuration - randYMax) +
	    randYMax * RAND_FLOAT ();
	p->moveDuration = 1;

	p->fadeStartTime = p->moveStartTime + life;
	if (p->fadeStartTime > 1 - fadeDuration)
	    p->fadeStartTime = 1 - fadeDuration;
	p->fadeDuration = fadeDuration;

	p->finalRotAng = 150;
    }
}
