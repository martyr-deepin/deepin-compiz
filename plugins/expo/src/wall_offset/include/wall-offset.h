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

#ifndef _COMPIZ_EXPO_WALL_OFFSET_H
#define _COMPIZ_EXPO_WALL_OFFSET_H

#include <core/point.h>
#include <core/size.h>
#include <core/rect.h>

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
			     float           animationProgress);
    }
}

#endif
