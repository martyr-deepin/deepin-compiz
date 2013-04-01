/*
 * Copyright © 2006 Novell, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Author: David Reveman <davidr@novell.com>
 */

#include "gtk-window-decorator.h"

void
decor_update_blur_property (decor_t *d,
			    int     width,
			    int     height,
			    Region  top_region,
			    int     top_offset,
			    Region  bottom_region,
			    int     bottom_offset,
			    Region  left_region,
			    int     left_offset,
			    Region  right_region,
			    int     right_offset)
{
    Display *xdisplay = GDK_DISPLAY_XDISPLAY (gdk_display_get_default ());
    long    *data = NULL;
    int     size = 0;
    gint    blur_type;

    g_object_get (settings, "blur", &blur_type, NULL);
    
    if (blur_type != BLUR_TYPE_ALL)
    {
	bottom_region = NULL;
	left_region   = NULL;
	right_region  = NULL;
	
	if (blur_type != BLUR_TYPE_TITLEBAR)
	    top_region = NULL;
    }

    if (top_region)
	size += top_region->numRects;
    if (bottom_region)
	size += bottom_region->numRects;
    if (left_region)
	size += left_region->numRects;
    if (right_region)
	size += right_region->numRects;

    if (size)
	data = (long *) malloc (sizeof (long) * (2 + size * 6));

    if (data)
    {
	decor_region_to_blur_property (data, 4, 0, width, height,
				       top_region, top_offset,
				       bottom_region, bottom_offset,
				       left_region, left_offset,
				       right_region, right_offset);

	gdk_error_trap_push ();
	XChangeProperty (xdisplay, d->prop_xid,
			 win_blur_decor_atom,
			 XA_INTEGER,
			 32, PropModeReplace, (guchar *) data,
			 2 + size * 6);
	gdk_display_sync (gdk_display_get_default ());
	gdk_error_trap_pop ();

	free (data);
    }
    else
    {
	gdk_error_trap_push ();
	XDeleteProperty (xdisplay, d->prop_xid, win_blur_decor_atom);
	gdk_display_sync (gdk_display_get_default ());
	gdk_error_trap_pop ();
    }
}
