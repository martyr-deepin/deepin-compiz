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

// =====================  Effect: Focus Fade  =========================

FocusFadeAnim::FocusFadeAnim (CompWindow *w,
			      WindowEvent curWindowEvent,
			      float duration,
			      const AnimEffect info,
			      const CompRect &icon) :
    Animation::Animation (w, curWindowEvent, duration, info, icon),
    RestackAnim::RestackAnim (w, curWindowEvent, duration, info, icon),
    FadeAnim::FadeAnim (w, curWindowEvent, duration, info, icon)
{
}

/// Compute the cross-fade opacity to make the effect look good with every
/// window opacity value.
GLushort
FocusFadeAnim::computeOpacity (GLushort opacityInt)
{
    float progress = 1 - progressLinear ();
    float opacity = opacityInt / (float)OPAQUE;
    float multiplier;

    bool newCopy = overNewCopy ();

    // flip opacity behavior for the other side of the cross-fade
    if (newCopy)
        progress = 1 - progress;

    if (mWindow->alpha () || (newCopy && opacity >= 0.91f))
	multiplier = progressDecelerate (progress);
    else if (opacity > 0.94f)
	multiplier = progressDecelerateCustom (progress, 0.55, 1.32);
    else if (opacity >= 0.91f && opacity < 0.94f)
	multiplier = progressDecelerateCustom (progress, 0.62, 0.92);
    else if (opacity >= 0.89f && opacity < 0.91f)
	multiplier = progressDecelerate (progress);
    else if (opacity >= 0.84f && opacity < 0.89f)
	multiplier = progressDecelerateCustom (progress, 0.64, 0.80);
    else if (opacity >= 0.79f && opacity < 0.84f)
	multiplier = progressDecelerateCustom (progress, 0.67, 0.77);
    else if (opacity >= 0.54f && opacity < 0.79f)
	multiplier = progressDecelerateCustom (progress, 0.61, 0.69);
    else
	multiplier = progress;

    multiplier = 1 - multiplier;
    float finalOpacity = opacity * multiplier;
    finalOpacity = MIN (finalOpacity, 1);
    finalOpacity = MAX (finalOpacity, 0);

    return (GLushort)(finalOpacity * OPAQUE);
}

void
FocusFadeAnim::updateAttrib (GLWindowPaintAttrib &attrib)
{
    attrib.opacity = computeOpacity (attrib.opacity);
}

void
FocusFadeAnim::processCandidate (CompWindow *candidateWin,
				 CompWindow *subjectWin,
				 CompRegion &candidateAndSubjectIntersection,
				 int &numSelectedCandidates)
{
    AnimWindow *aCandidateWin = AnimWindow::get (candidateWin);
    RestackPersistentData *data = static_cast<RestackPersistentData *>
	(aCandidateWin->persistentData["restack"]);
    data->mWinPassingThrough = subjectWin;
}

void
FocusFadeAnim::cleanUp (bool closing, bool destructing)
{
    // Clear winPassingThrough of each window
    // that this one was passing through
    // during focus effect.
    foreach (CompWindow *w, ::screen->windows ())
    {
	AnimWindow *aw = AnimWindow::get (w);
	PersistentDataMap::iterator itData = aw->persistentData.find ("restack");
	if (itData != aw->persistentData.end ()) // if found
	{
	    RestackPersistentData *data =
		static_cast<RestackPersistentData *> (itData->second);
	    if (data->mWinPassingThrough == mWindow)
		data->mWinPassingThrough = 0;
	}
    }

    RestackAnim::cleanUp (closing, destructing);
}

