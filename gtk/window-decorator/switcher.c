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

decor_frame_t *
create_switcher_frame (const gchar *type)
{
    AtkObject	   *switcher_label_obj;
    decor_frame_t *frame = decor_frame_new (type);
    decor_extents_t _switcher_extents    = { 6, 6, 6, 6 + SWITCHER_SPACE };

    decor_context_t _switcher_context = {
	{ 0, 0, 0, 0 },
	6, 6, 6, 6 + SWITCHER_SPACE,
	0, 0, 0, 0
    };

    frame->win_extents = _switcher_extents;
    frame->max_win_extents = _switcher_extents;
    frame->win_extents = _switcher_extents;
    frame->window_context_inactive = _switcher_context;
    frame->window_context_active = _switcher_context;
    frame->window_context_no_shadow = _switcher_context;
    frame->max_window_context_active = _switcher_context;
    frame->max_window_context_inactive = _switcher_context;
    frame->max_window_context_no_shadow = _switcher_context;
    frame->update_shadow = switcher_frame_update_shadow;

    /* keep the switcher frame around since we need to keep its
     * contents */

    gwd_decor_frame_ref (frame);

    switcher_label = gtk_label_new ("");
    switcher_label_obj = gtk_widget_get_accessible (switcher_label);
    atk_object_set_role (switcher_label_obj, ATK_ROLE_STATUSBAR);
    gtk_container_add (GTK_CONTAINER (frame->style_window_rgba), switcher_label);

    return frame;
}

void
destroy_switcher_frame (decor_frame_t *frame)
{
    gtk_widget_destroy (switcher_label);
    decor_frame_destroy (frame);
}

static void
draw_switcher_background (decor_t *d)
{
    Display	  *xdisplay = GDK_DISPLAY_XDISPLAY (gdk_display_get_default ());
    cairo_t	  *cr;
    GtkStyle	  *style;
    decor_color_t color;
    double	  alpha = SWITCHER_ALPHA / 65535.0;
    double	  x1, y1, x2, y2, h;
    int		  top;
    unsigned long pixel;
    ushort	  a = SWITCHER_ALPHA;

    if (!d->buffer_pixmap)
	return;

    style = gtk_widget_get_style (d->frame->style_window_rgba);

    color.r = style->bg[GTK_STATE_NORMAL].red   / 65535.0;
    color.g = style->bg[GTK_STATE_NORMAL].green / 65535.0;
    color.b = style->bg[GTK_STATE_NORMAL].blue  / 65535.0;

    cr = gdk_cairo_create (GDK_DRAWABLE (d->buffer_pixmap));

    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);

    top = d->frame->win_extents.top;

    x1 = d->frame->window_context_active.left_space - d->frame->win_extents.left;
    y1 = d->frame->window_context_active.top_space - d->frame->win_extents.top;
    x2 = d->width - d->frame->window_context_active.right_space + d->frame->win_extents.right;
    y2 = d->height - d->frame->window_context_active.bottom_space + d->frame->win_extents.bottom;

    h = y2 - y1 - d->frame->win_extents.top - d->frame->win_extents.top;

    cairo_set_line_width (cr, 1.0);

    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);

    draw_shadow_background (d, cr, d->frame->border_shadow_active, &d->frame->window_context_active);

    fill_rounded_rectangle (cr,
			    x1 + 0.5,
			    y1 + 0.5,
			    d->frame->win_extents.left - 0.5,
			    top - 0.5,
			    5.0, CORNER_TOPLEFT,
			    &color, alpha, &color, alpha * 0.75,
			    SHADE_TOP | SHADE_LEFT);

    fill_rounded_rectangle (cr,
			    x1 + d->frame->win_extents.left,
			    y1 + 0.5,
			    x2 - x1 - d->frame->win_extents.left -
			    d->frame->win_extents.right,
			    top - 0.5,
			    5.0, 0,
			    &color, alpha, &color, alpha * 0.75,
			    SHADE_TOP);

    fill_rounded_rectangle (cr,
			    x2 - d->frame->win_extents.right,
			    y1 + 0.5,
			    d->frame->win_extents.right - 0.5,
			    top - 0.5,
			    5.0, CORNER_TOPRIGHT,
			    &color, alpha, &color, alpha * 0.75,
			    SHADE_TOP | SHADE_RIGHT);

    fill_rounded_rectangle (cr,
			    x1 + 0.5,
			    y1 + top,
			    d->frame->win_extents.left - 0.5,
			    h,
			    5.0, 0,
			    &color, alpha, &color, alpha * 0.75,
			    SHADE_LEFT);

    fill_rounded_rectangle (cr,
			    x2 - d->frame->win_extents.right,
			    y1 + top,
			    d->frame->win_extents.right - 0.5,
			    h,
			    5.0, 0,
			    &color, alpha, &color, alpha * 0.75,
			    SHADE_RIGHT);

    fill_rounded_rectangle (cr,
			    x1 + 0.5,
			    y2 - d->frame->win_extents.top,
			    d->frame->win_extents.left - 0.5,
			    d->frame->win_extents.top - 0.5,
			    5.0, CORNER_BOTTOMLEFT,
			    &color, alpha, &color, alpha * 0.75,
			    SHADE_BOTTOM | SHADE_LEFT);

    fill_rounded_rectangle (cr,
			    x1 + d->frame->win_extents.left,
			    y2 - d->frame->win_extents.top,
			    x2 - x1 - d->frame->win_extents.left -
			    d->frame->win_extents.right,
			    d->frame->win_extents.top - 0.5,
			    5.0, 0,
			    &color, alpha, &color, alpha * 0.75,
			    SHADE_BOTTOM);

    fill_rounded_rectangle (cr,
			    x2 - d->frame->win_extents.right,
			    y2 - d->frame->win_extents.top,
			    d->frame->win_extents.right - 0.5,
			    d->frame->win_extents.top - 0.5,
			    5.0, CORNER_BOTTOMRIGHT,
			    &color, alpha, &color, alpha * 0.75,
			    SHADE_BOTTOM | SHADE_RIGHT);

    cairo_rectangle (cr, x1 + d->frame->win_extents.left,
		     y1 + top,
		     x2 - x1 - d->frame->win_extents.left - d->frame->win_extents.right,
		     h);
    gdk_cairo_set_source_color_alpha (cr,
				      &style->bg[GTK_STATE_NORMAL],
				      alpha);
    cairo_fill (cr);

    cairo_set_operator (cr, CAIRO_OPERATOR_OVER);

    rounded_rectangle (cr,
		       x1 + 0.5, y1 + 0.5,
		       x2 - x1 - 1.0, y2 - y1 - 1.0,
		       5.0,
		       CORNER_TOPLEFT | CORNER_TOPRIGHT | CORNER_BOTTOMLEFT |
		       CORNER_BOTTOMRIGHT);

    cairo_clip (cr);

    cairo_translate (cr, 1.0, 1.0);

    rounded_rectangle (cr,
		       x1 + 0.5, y1 + 0.5,
		       x2 - x1 - 1.0, y2 - y1 - 1.0,
		       5.0,
		       CORNER_TOPLEFT | CORNER_TOPRIGHT | CORNER_BOTTOMLEFT |
		       CORNER_BOTTOMRIGHT);

    cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 0.4);
    cairo_stroke (cr);

    cairo_translate (cr, -2.0, -2.0);

    rounded_rectangle (cr,
		       x1 + 0.5, y1 + 0.5,
		       x2 - x1 - 1.0, y2 - y1 - 1.0,
		       5.0,
		       CORNER_TOPLEFT | CORNER_TOPRIGHT | CORNER_BOTTOMLEFT |
		       CORNER_BOTTOMRIGHT);

    cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 0.1);
    cairo_stroke (cr);

    cairo_translate (cr, 1.0, 1.0);

    cairo_reset_clip (cr);

    rounded_rectangle (cr,
		       x1 + 0.5, y1 + 0.5,
		       x2 - x1 - 1.0, y2 - y1 - 1.0,
		       5.0,
		       CORNER_TOPLEFT | CORNER_TOPRIGHT | CORNER_BOTTOMLEFT |
		       CORNER_BOTTOMRIGHT);

    gdk_cairo_set_source_color_alpha (cr,
				      &style->fg[GTK_STATE_NORMAL],
				      alpha);

    cairo_stroke (cr);

    cairo_destroy (cr);

    copy_to_front_buffer (d);

    pixel = ((((a * style->bg[GTK_STATE_NORMAL].blue ) >> 24) & 0x0000ff) |
	     (((a * style->bg[GTK_STATE_NORMAL].green) >> 16) & 0x00ff00) |
	     (((a * style->bg[GTK_STATE_NORMAL].red  ) >>  8) & 0xff0000) |
	     (((a & 0xff00) << 16)));

    decor_update_switcher_property (d);

    gdk_error_trap_push ();
    XSetWindowBackground (xdisplay, d->prop_xid, pixel);
    XClearWindow (xdisplay, d->prop_xid);

    gdk_display_sync (gdk_display_get_default ());
    gdk_error_trap_pop ();

    d->prop_xid = 0;
}

static void
draw_switcher_foreground (decor_t *d)
{
    cairo_t	  *cr;
    GtkStyle	  *style;
    double	  alpha = SWITCHER_ALPHA / 65535.0;

    if (!d->pixmap || !d->buffer_pixmap)
	return;

    style = gtk_widget_get_style (d->frame->style_window_rgba);

    cr = gdk_cairo_create (GDK_DRAWABLE (d->buffer_pixmap));

    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);

    cairo_rectangle (cr, d->frame->window_context_active.left_space,
		     d->height - d->frame->window_context_active.bottom_space,
		     d->width - d->frame->window_context_active.left_space -
		     d->frame->window_context_active.right_space,
		     SWITCHER_SPACE);

    gdk_cairo_set_source_color_alpha (cr,
				      &style->bg[GTK_STATE_NORMAL],
				      alpha);
    cairo_fill (cr);

    if (d->layout)
    {
	int w;

	cairo_set_operator (cr, CAIRO_OPERATOR_OVER);

	gdk_cairo_set_source_color_alpha (cr,
					  &style->fg[GTK_STATE_NORMAL],
					  1.0);

	pango_layout_get_pixel_size (d->layout, &w, NULL);

	cairo_move_to (cr, d->width / 2 - w / 2,
		       d->height - d->frame->window_context_active.bottom_space +
		       SWITCHER_SPACE / 2 - d->frame->text_height / 2);

	pango_cairo_show_layout (cr, d->layout);
    }

    cairo_destroy (cr);

    copy_to_front_buffer (d);
}

void
draw_switcher_decoration (decor_t *d)
{
    if (d->prop_xid)
	draw_switcher_background (d);

    draw_switcher_foreground (d);
}

void
switcher_window_closed ()
{
    decor_t *d = switcher_window;
    Display *xdisplay = gdk_x11_get_default_xdisplay ();

    if (d->layout)
	g_object_unref (G_OBJECT (d->layout));

    if (d->name)
	g_free (d->name);

    if (d->pixmap)
	g_object_unref (G_OBJECT (d->pixmap));

    if (d->buffer_pixmap)
	g_object_unref (G_OBJECT (d->buffer_pixmap));

    if (d->cr)
	cairo_destroy (d->cr);

    if (d->picture)
	XRenderFreePicture (xdisplay, d->picture);

    gwd_decor_frame_unref (switcher_window->frame);
    g_free (switcher_window);
    switcher_window = NULL;
}

/* Switcher is override-redirect now, we need to track
 * it separately */
decor_t *
switcher_window_opened (Window popup, Window window)
{
    decor_t      *d;

    d = switcher_window = calloc (1, sizeof (decor_t));
    if (!d)
	return NULL;

    return d;
}


gboolean
update_switcher_window (Window     popup,
			Window     selected)
{
    decor_t           *d = switcher_window;
    GdkPixmap         *pixmap, *buffer_pixmap = NULL;
    unsigned int      height, width = 0, border, depth;
    int		      x, y;
    Window	      root_return;
    WnckWindow        *selected_win;
    Display           *xdisplay;
    XRenderPictFormat *format;

    if (!d)
	d = switcher_window_opened (popup, selected);

    xdisplay = GDK_DISPLAY_XDISPLAY (gdk_display_get_default ());

    /* FIXME: Thats a round-trip */
    XGetGeometry (gdk_x11_get_default_xdisplay (), popup, &root_return,
		  &x, &y, &width, &height, &border, &depth);

    d->decorated = FALSE;
    d->draw	 = draw_switcher_decoration;
    d->frame     = gwd_get_decor_frame ("switcher");

    decor_get_default_layout (&d->frame->window_context_active, width, 1, &d->border_layout);

    width  = d->border_layout.width;
    height = d->border_layout.height;

    selected_win = wnck_window_get (selected);
    if (selected_win)
    {
	glong		name_length;
	PangoLayoutLine *line;
	const gchar	*name;

	if (d->name)
	{
	    g_free (d->name);
	    d->name = NULL;
	}

	name = wnck_window_get_name (selected_win);
	if (name && (name_length = strlen (name)))
	{
	    if (!d->layout)
	    {
		d->layout = pango_layout_new (d->frame->pango_context);
		if (d->layout)
		    pango_layout_set_wrap (d->layout, PANGO_WRAP_CHAR);
	    }

	    if (d->layout)
	    {
		int tw;

		tw = width - d->frame->window_context_active.left_space -
		    d->frame->window_context_active.right_space - 64;
		pango_layout_set_auto_dir (d->layout, FALSE);
		pango_layout_set_width (d->layout, tw * PANGO_SCALE);
		pango_layout_set_text (d->layout, name, name_length);

		line = pango_layout_get_line (d->layout, 0);

		name_length = line->length;
		if (pango_layout_get_line_count (d->layout) > 1)
		{
		    if (name_length < 4)
		    {
			g_object_unref (G_OBJECT (d->layout));
			d->layout = NULL;
		    }
		    else
		    {
			d->name = g_strndup (name, name_length);
			strcpy (d->name + name_length - 3, "...");
		    }
		}
		else
		    d->name = g_strndup (name, name_length);

		if (d->layout)
		    pango_layout_set_text (d->layout, d->name, name_length);
	    }
	}
	else if (d->layout)
	{
	    g_object_unref (G_OBJECT (d->layout));
	    d->layout = NULL;
	}
    }

    if (selected != switcher_selected_window)
    {
	gtk_label_set_text (GTK_LABEL (switcher_label), "");
	if (selected_win && d->name)
	    gtk_label_set_text (GTK_LABEL (switcher_label), d->name);
	switcher_selected_window = selected;
    }

    pixmap = create_pixmap (width, height, d->frame->style_window_rgba);
    if (!pixmap)
	return FALSE;

    buffer_pixmap = create_pixmap (width, height, d->frame->style_window_rgba);
    if (!buffer_pixmap)
    {
	g_object_unref (G_OBJECT (pixmap));
	return FALSE;
    }

    if (d->pixmap)
	g_object_unref (G_OBJECT (d->pixmap));

    if (d->buffer_pixmap)
	g_object_unref (G_OBJECT (d->buffer_pixmap));

    if (d->cr)
	cairo_destroy (d->cr);

    if (d->picture)
	XRenderFreePicture (xdisplay, d->picture);

    d->pixmap	     = pixmap;
    d->buffer_pixmap = buffer_pixmap;
    d->cr	     = gdk_cairo_create (pixmap);

    format = get_format_for_drawable (d, GDK_DRAWABLE (d->buffer_pixmap));
    d->picture = XRenderCreatePicture (xdisplay, GDK_PIXMAP_XID (buffer_pixmap),
				       format, 0, NULL);

    d->width  = width;
    d->height = height;

    d->prop_xid = popup;

    queue_decor_draw (d);

    return TRUE;
}
