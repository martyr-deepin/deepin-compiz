/**
 * Copyright © 2012 Canonical Ltd.
 *
 * Authors:
 * Sam Spilsbury <sam.spilsbury@canonical.com>
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
 **/
#include "wall-offset.h"

namespace compiz
{
    namespace expo
    {
	void
	calculateWallOffset (const CompRect  &output,
			     const CompPoint &offsetInScreenCoords,
			     const CompPoint &vpSize,
			     const CompSize  &screenSize,
			     float           &offsetInWorldX,
			     float           &offsetInWorldY,
			     float	     &worldScaleFactorX,
			     float	     &worldScaleFactorY,
			     float           animationProgress)
	{
	    const float sx = screenSize.width () / static_cast <float> (output.width ());
	    const float sy = screenSize.height () / static_cast <float> (output.height ());
	    offsetInWorldX = 0.0;
	    offsetInWorldY = 0.0;
	    worldScaleFactorX = 1.0f;
	    worldScaleFactorY = 1.0f;

	    if (output.left () == 0)
	    {
		offsetInWorldX = ((vpSize.x () * sx) / ((float) output.width ()) * (offsetInScreenCoords.x ()) * animationProgress);
		worldScaleFactorX = 1.0f - ((float) (offsetInScreenCoords.x ()) / (float) (output.width ())) * animationProgress;
	    }

	    if (output.top () == 0)
	    {
		offsetInWorldY = ((vpSize.y () * sy) / ((float) output.height ()) * (offsetInScreenCoords.y ()) * animationProgress);
		worldScaleFactorY = 1.0f - ((float) (offsetInScreenCoords.y ()) / (float) output.height ()) * animationProgress;
	    }
	}
    }
}
