/**
 *
 * Compiz wall plugin
 *
 * offset-movement.h
 *
 * Copyright (c) 2006 Robert Carr <racarr@beryl-project.org>
 *
 * Authors:
 * Robert Carr <racarr@beryl-project.org>
 * Dennis Kasprzyk <onestone@opencompositing.org>
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

#ifndef _COMPIZ_WALL_OFFSET_MOVEMENT_H
#define _COMPIZ_WALL_OFFSET_MOVEMENT_H

#include <core/region.h>
#include <core/rect.h>
#include <core/point.h>

namespace compiz
{
    namespace wall
    {
	CompPoint movementWindowOnScreen (const CompRect &serverBorderRect,
					  const CompRegion &screenRegion);
    }
}

#endif
