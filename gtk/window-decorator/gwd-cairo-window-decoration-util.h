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
#ifndef _GWD_CAIRO_WINDOW_DECORATION_UTIL_H
#define _GWD_CAIRO_WINDOW_DECORATION_UTIL_H

#include <glib.h>

G_BEGIN_DECLS

typedef struct _decor_extents decor_extents_t;

const decor_extents_t * gwd_cairo_window_decoration_get_default_max_win_extents ();
const decor_extents_t * gwd_cairo_window_decoration_get_default_win_extents ();

void
gwd_cairo_window_decoration_get_extents (decor_extents_t *win_extents,
					 decor_extents_t *max_win_extents);

G_END_DECLS;

#endif
