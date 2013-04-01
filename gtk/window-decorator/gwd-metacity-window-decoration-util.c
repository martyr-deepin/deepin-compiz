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
#include <string.h>
#include <glib.h>
#include "gwd-metacity-window-decoration-util.h"

gboolean
gwd_metacity_window_decoration_update_meta_theme (const gchar		     *theme,
						  GWDMetaThemeGetCurrentProc get_current,
						  GWDMetaThemeSetProc	     set_current)
{
    if (!theme)
	return FALSE;

    /* meta_theme_get_current seems to return the last
     * good theme now, so if one was already set this function
     * will be ineffectual, so we need to check if the theme
     * is obviously bad as the user intended to disable metacity
     * themes */
    if (!strlen (theme))
	return FALSE;

    (*set_current) (theme, TRUE);
    if (!(*get_current) ())
	return FALSE;

    return TRUE;
}
