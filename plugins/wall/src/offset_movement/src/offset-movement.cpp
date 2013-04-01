/**
 *
 * Compiz wall plugin
 *
 * offset-movement.c
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

#include "offset-movement.h"

CompPoint
compiz::wall::movementWindowOnScreen (const CompRect &serverBorderRect,
				      const CompRegion &screenRegion)
{
    CompRegion sbrRegion (serverBorderRect);

    /* If the window would be partially offscreen
     * after it was moved then we should move it back
     * so that it is completely onscreen, since we moved
     * from mostly offscreen on B to mostly onscreen on A,
     * the user should be able to see their selected window */
    CompRegion inter = sbrRegion.intersected (screenRegion);
    CompRegion rem = sbrRegion - screenRegion;

    int dx = 0;
    int dy = 0;

    const CompRect::vector &rects (rem.rects ());

    for (std::vector <CompRect>::const_iterator it = rects.begin ();
	 it != rects.end ();
	 ++it)
    {
	const CompRect &r = *it;

	if (r.x1 () >= inter.boundingRect ().x2 ())
	    dx -= r.width ();
	else if (r.x2 () <= inter.boundingRect ().x1 ())
	    dx += r.width ();

	if (r.y1 () >= inter.boundingRect ().y2 ())
	    dy -= r.height ();
	else if (r.y2 () <= inter.boundingRect ().y1 ())
	    dy += r.height ();
    }

    return CompPoint (dx, dy);
}
