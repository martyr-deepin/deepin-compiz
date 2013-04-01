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
 *
 * 2D Mode: Copyright © 2010 Sam Spilsbury <smspillaz@gmail.com>
 * Frames Management: Copright © 2011 Canonical Ltd.
 *        Authored By: Sam Spilsbury <sam.spilsbury@canonical.com>
 */

#include "gtk-window-decorator.h"

void
decor_update_window_property (decor_t *d)
{
    long	    *data;
    Display	    *xdisplay =
	GDK_DISPLAY_XDISPLAY (gdk_display_get_default ());
    decor_extents_t extents = d->frame->win_extents;
    gint	    nQuad;
    unsigned int    nOffset = 1;
    unsigned int   frame_type = populate_frame_type (d);
    unsigned int   frame_state = populate_frame_state (d);
    unsigned int   frame_actions = populate_frame_actions (d);
    decor_quad_t    quads[N_QUADS_MAX];
    int		    w, h;
    gint	    stretch_offset;
    REGION	    top, bottom, left, right;

    w = d->border_layout.top.x2 - d->border_layout.top.x1 -
	d->context->left_space - d->context->right_space;

    if (d->border_layout.rotation)
	h = d->border_layout.left.x2 - d->border_layout.left.x1;
    else
	h = d->border_layout.left.y2 - d->border_layout.left.y1;

    stretch_offset = w - d->button_width - 1;

    nQuad = decor_set_lSrStXbS_window_quads (quads, d->context,
					     &d->border_layout,
					     stretch_offset);

    extents.top += d->frame->titlebar_height;

    if (d->frame_window)
    {
        data = decor_alloc_property (nOffset, WINDOW_DECORATION_TYPE_WINDOW);
        decor_gen_window_property (data, nOffset - 1, &extents, &extents, 20, 20, frame_type, frame_state, frame_actions);
    }
    else
    {
        data = decor_alloc_property (nOffset, WINDOW_DECORATION_TYPE_PIXMAP);
        decor_quads_to_property (data, nOffset - 1, GDK_PIXMAP_XID (d->pixmap),
			     &extents, &extents,
			     &extents, &extents,
			     ICON_SPACE + d->button_width,
			     0,
			     quads, nQuad, frame_type, frame_state, frame_actions);
    }

    gdk_error_trap_push ();
    XChangeProperty (xdisplay, d->prop_xid,
		     win_decor_atom,
		     XA_INTEGER,
		     32, PropModeReplace, (guchar *) data,
		     PROP_HEADER_SIZE + BASE_PROP_SIZE + QUAD_PROP_SIZE * N_QUADS_MAX);
    gdk_display_sync (gdk_display_get_default ());
    gdk_error_trap_pop ();

    top.rects = &top.extents;
    top.numRects = top.size = 1;

    top.extents.x1 = -extents.left;
    top.extents.y1 = -extents.top;
    top.extents.x2 = w + extents.right;
    top.extents.y2 = 0;

    bottom.rects = &bottom.extents;
    bottom.numRects = bottom.size = 1;

    bottom.extents.x1 = -extents.left;
    bottom.extents.y1 = 0;
    bottom.extents.x2 = w + extents.right;
    bottom.extents.y2 = extents.bottom;

    left.rects = &left.extents;
    left.numRects = left.size = 1;

    left.extents.x1 = -extents.left;
    left.extents.y1 = 0;
    left.extents.x2 = 0;
    left.extents.y2 = h;

    right.rects = &right.extents;
    right.numRects = right.size = 1;

    right.extents.x1 = 0;
    right.extents.y1 = 0;
    right.extents.x2 = extents.right;
    right.extents.y2 = h;

    decor_update_blur_property (d,
				w, h,
				&top, stretch_offset,
				&bottom, w / 2,
				&left, h / 2,
				&right, h / 2);

    free (data);
}

void
decor_update_switcher_property (decor_t *d)
{
    long	 *data;
    Display	 *xdisplay = GDK_DISPLAY_XDISPLAY (gdk_display_get_default ());
    gint	 nQuad;
    decor_quad_t quads[N_QUADS_MAX];
    unsigned int    nOffset = 1;
    unsigned int   frame_type = populate_frame_type (d);
    unsigned int   frame_state = populate_frame_state (d);
    unsigned int   frame_actions = populate_frame_actions (d);
    GtkStyle     *style;
    long         fgColor[4];
    
    nQuad = decor_set_lSrStSbX_window_quads (quads, &d->frame->window_context_active,
					     &d->border_layout,
					     d->border_layout.top.x2 -
					     d->border_layout.top.x1 -
					     d->frame->window_context_active.extents.left -
						 d->frame->window_context_active.extents.right -
						     32);
    
    data = decor_alloc_property (nOffset, WINDOW_DECORATION_TYPE_PIXMAP);
    decor_quads_to_property (data, nOffset - 1, GDK_PIXMAP_XID (d->pixmap),
			     &d->frame->win_extents, &d->frame->win_extents,
			     &d->frame->win_extents, &d->frame->win_extents,
			     0, 0, quads, nQuad, frame_type, frame_state, frame_actions);
    
    style = gtk_widget_get_style (d->frame->style_window_rgba);
    
    fgColor[0] = style->fg[GTK_STATE_NORMAL].red;
    fgColor[1] = style->fg[GTK_STATE_NORMAL].green;
    fgColor[2] = style->fg[GTK_STATE_NORMAL].blue;
    fgColor[3] = SWITCHER_ALPHA;
    
    gdk_error_trap_push ();
    XChangeProperty (xdisplay, d->prop_xid,
		     win_decor_atom,
		     XA_INTEGER,
		     32, PropModeReplace, (guchar *) data,
		     PROP_HEADER_SIZE + BASE_PROP_SIZE + QUAD_PROP_SIZE * N_QUADS_MAX);
    XChangeProperty (xdisplay, d->prop_xid, switcher_fg_atom,
		     XA_INTEGER, 32, PropModeReplace, (guchar *) fgColor, 4);
    gdk_display_sync (gdk_display_get_default ());
    gdk_error_trap_pop ();

    free (data);
}
