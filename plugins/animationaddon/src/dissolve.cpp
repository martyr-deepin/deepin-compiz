/* Compiz Dissolve animation
 * dissolve.cpp
 *
 * Copyright (c) 2010 Jay Catherwood <jay.catherwood@gmail.com>
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
 */

#include "private.h"

DissolveSingleAnim::DissolveSingleAnim (CompWindow *w,
					WindowEvent curWindowEvent,
					float duration,
					const AnimEffect info,
					const CompRect &icon) :
					Animation::Animation (w, curWindowEvent, duration, info, icon),
					TransformAnim::TransformAnim (w, curWindowEvent, duration, info, icon)
					{
					}

void
DissolveSingleAnim::updateAttrib (GLWindowPaintAttrib &attrib)
{  
    int layer = MultiAnim <DissolveSingleAnim, 5>::getCurrAnimNumber (mAWindow);
    float o = 0.2;
    float factor = (4 - layer) * o;
    
    attrib.opacity *= o / (1.0 - factor);
}

void
DissolveSingleAnim::updateTransform (GLMatrix &transform)
{
    int layer = MultiAnim <DissolveSingleAnim, 5>::getCurrAnimNumber (mAWindow);
    
    switch (layer) {
	case 1:
	    transform.translate (3.*getDissolveSingleProgress (), 0.f, 0.f);
	    break;
	case 2:
	    transform.translate (-3.*getDissolveSingleProgress (), 0.f, 0.f);
	    break;
	case 3:
	    transform.translate (0.f, 3.*getDissolveSingleProgress (), 0.f);
	    break;
	case 4:
	    transform.translate (0.f, -3.*getDissolveSingleProgress (), 0.f);
	    break;
	default:
	    break;
    }
}

void
DissolveSingleAnim::updateBB (CompOutput &output)
{
    mAWindow->expandBBWithWindow ();
}
