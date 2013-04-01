/*
 * Copyright © 2012 Canonical Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Authored By: Sam Spilsbury <sam.spilsbury@canonical.com>
 */
#include <decoration.h>
#include "gwd-cairo-window-decoration-util.h"

static const decor_extents_t gwd_cairo_window_decoration_default_win_extents =
{
    6, 6, 10, 6
};

static const decor_extents_t gwd_cairo_window_decoration_default_max_win_extents =
{
    6, 6, 4, 6
};

const decor_extents_t *
gwd_cairo_window_decoration_get_default_max_win_extents ()
{
    return &gwd_cairo_window_decoration_default_max_win_extents;
}


const decor_extents_t *
gwd_cairo_window_decoration_get_default_win_extents ()
{
    return &gwd_cairo_window_decoration_default_win_extents;
}

void
gwd_cairo_window_decoration_get_extents (decor_extents_t *win_extents,
					 decor_extents_t *max_win_extents)
{
    memcpy (win_extents,
	    gwd_cairo_window_decoration_get_default_max_win_extents (),
	    sizeof (decor_extents_t));
    memcpy (max_win_extents,
	    gwd_cairo_window_decoration_get_default_win_extents (),
	    sizeof (decor_extents_t));
}
