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
#include "gwd-cairo-window-decoration-util.h"

void
rounded_rectangle (cairo_t *cr,
		   double  x,
		   double  y,
		   double  w,
		   double  h,
		   double  radius,
		   int	   corner)
{
    if (corner & CORNER_TOPLEFT)
	cairo_move_to (cr, x + radius, y);
    else
	cairo_move_to (cr, x, y);

    if (corner & CORNER_TOPRIGHT)
	cairo_arc (cr, x + w - radius, y + radius, radius,
		   M_PI * 1.5, M_PI * 2.0);
    else
	cairo_line_to (cr, x + w, y);

    if (corner & CORNER_BOTTOMRIGHT)
	cairo_arc (cr, x + w - radius, y + h - radius, radius,
		   0.0, M_PI * 0.5);
    else
	cairo_line_to (cr, x + w, y + h);

    if (corner & CORNER_BOTTOMLEFT)
	cairo_arc (cr, x + radius, y + h - radius, radius,
		   M_PI * 0.5, M_PI);
    else
	cairo_line_to (cr, x, y + h);

    if (corner & CORNER_TOPLEFT)
	cairo_arc (cr, x + radius, y + radius, radius, M_PI, M_PI * 1.5);
    else
	cairo_line_to (cr, x, y);
}

void
fill_rounded_rectangle (cairo_t       *cr,
			double        x,
			double        y,
			double        w,
			double        h,
			double	      radius,
			int	      corner,
			decor_color_t *c0,
			double        alpha0,
			decor_color_t *c1,
			double	      alpha1,
			int	      gravity)
{
    cairo_pattern_t *pattern;

    rounded_rectangle (cr, x, y, w, h, radius, corner);

    if (gravity & SHADE_RIGHT)
    {
	x = x + w;
	w = -w;
    }
    else if (!(gravity & SHADE_LEFT))
    {
	x = w = 0;
    }

    if (gravity & SHADE_BOTTOM)
    {
	y = y + h;
	h = -h;
    }
    else if (!(gravity & SHADE_TOP))
    {
	y = h = 0;
    }

    if (w && h)
    {
	cairo_matrix_t matrix;

	pattern = cairo_pattern_create_radial (0.0, 0.0, 0.0, 0.0, 0.0, w);

	cairo_matrix_init_scale (&matrix, 1.0, w / h);
	cairo_matrix_translate (&matrix, -(x + w), -(y + h));

	cairo_pattern_set_matrix (pattern, &matrix);
    }
    else
    {
	pattern = cairo_pattern_create_linear (x + w, y + h, x, y);
    }

    cairo_pattern_add_color_stop_rgba (pattern, 0.0, c0->r, c0->g, c0->b,
				       alpha0);

    cairo_pattern_add_color_stop_rgba (pattern, 1.0, c1->r, c1->g, c1->b,
				       alpha1);

    cairo_pattern_set_extend (pattern, CAIRO_EXTEND_PAD);

    cairo_set_source (cr, pattern);
    cairo_fill (cr);
    cairo_pattern_destroy (pattern);
}

void
draw_shadow_background (decor_t		*d,
			cairo_t		*cr,
			decor_shadow_t  *s,
			decor_context_t *c)
{
    Display *xdisplay = GDK_DISPLAY_XDISPLAY (gdk_display_get_default ());

    if (!s || !s->picture ||!d->picture)
    {
	cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 0.0);
	cairo_paint (cr);
    }
    else
    {
	decor_fill_picture_extents_with_shadow (xdisplay,
						s, c,
						d->picture,
						&d->border_layout);
    }
}

static void
draw_close_button (decor_t *d,
		   cairo_t *cr,
		   double  s)
{
    cairo_rel_move_to (cr, 0.0, s);

    cairo_rel_line_to (cr, s, -s);
    cairo_rel_line_to (cr, s, s);
    cairo_rel_line_to (cr, s, -s);
    cairo_rel_line_to (cr, s, s);

    cairo_rel_line_to (cr, -s, s);
    cairo_rel_line_to (cr, s, s);
    cairo_rel_line_to (cr, -s, s);
    cairo_rel_line_to (cr, -s, -s);

    cairo_rel_line_to (cr, -s, s);
    cairo_rel_line_to (cr, -s, -s);
    cairo_rel_line_to (cr, s, -s);

    cairo_close_path (cr);
}

static void
draw_max_button (decor_t *d,
		 cairo_t *cr,
		 double  s)
{
    cairo_rel_line_to (cr, 12.0, 0.0);
    cairo_rel_line_to (cr, 0.0, 12.0);
    cairo_rel_line_to (cr, -12.0, 0.0);

    cairo_close_path (cr);

    cairo_rel_move_to (cr, 2.0, s);

    cairo_rel_line_to (cr, 12.0 - 4.0, 0.0);
    cairo_rel_line_to (cr, 0.0, 12.0 - s - 2.0);
    cairo_rel_line_to (cr, -(12.0 - 4.0), 0.0);

    cairo_close_path (cr);
}

static void
draw_unmax_button (decor_t *d,
		   cairo_t *cr,
		   double  s)
{
    cairo_rel_move_to (cr, 1.0, 1.0);

    cairo_rel_line_to (cr, 10.0, 0.0);
    cairo_rel_line_to (cr, 0.0, 10.0);
    cairo_rel_line_to (cr, -10.0, 0.0);

    cairo_close_path (cr);

    cairo_rel_move_to (cr, 2.0, s);

    cairo_rel_line_to (cr, 10.0 - 4.0, 0.0);
    cairo_rel_line_to (cr, 0.0, 10.0 - s - 2.0);
    cairo_rel_line_to (cr, -(10.0 - 4.0), 0.0);

    cairo_close_path (cr);
}

static void
draw_min_button (decor_t *d,
		 cairo_t *cr,
		 double  s)
{
    cairo_rel_move_to (cr, 0.0, 8.0);

    cairo_rel_line_to (cr, 12.0, 0.0);
    cairo_rel_line_to (cr, 0.0, s);
    cairo_rel_line_to (cr, -12.0, 0.0);

    cairo_close_path (cr);
}

typedef void (*draw_proc) (cairo_t *cr);

static void
button_state_offsets (gdouble x,
		      gdouble y,
		      guint   state,
		      gdouble *return_x,
		      gdouble *return_y)
{
    static double off[]	= { 0.0, 0.0, 0.0, 0.5 };

    *return_x  = x + off[state];
    *return_y  = y + off[state];
}

static void
button_state_paint (cairo_t	  *cr,
		    GtkStyle	  *style,
		    decor_color_t *color,
		    guint	  state)
{

#define IN_STATE (PRESSED_EVENT_WINDOW | IN_EVENT_WINDOW)

    if ((state & IN_STATE) == IN_STATE)
    {
	if (state & IN_EVENT_WINDOW)
	    cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
	else
	    cairo_set_source_rgba (cr, color->r, color->g, color->b, 0.95);

	cairo_fill_preserve (cr);

	gdk_cairo_set_source_color_alpha (cr,
					  &style->fg[GTK_STATE_NORMAL],
					  STROKE_ALPHA);

	cairo_set_line_width (cr, 1.0);
	cairo_stroke (cr);
	cairo_set_line_width (cr, 2.0);
    }
    else
    {
	gdk_cairo_set_source_color_alpha (cr,
					  &style->fg[GTK_STATE_NORMAL],
					  STROKE_ALPHA);
	cairo_stroke_preserve (cr);

	if (state & IN_EVENT_WINDOW)
	    cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
	else
	    cairo_set_source_rgba (cr, color->r, color->g, color->b, 0.95);

	cairo_fill (cr);
    }
}

void
draw_window_decoration (decor_t *d)
{
    cairo_t       *cr;
    GtkStyle	  *style;
    GdkDrawable   *drawable;
    decor_color_t color;
    double        alpha;
    double        x1, y1, x2, y2, x, y, h;
    int		  corners = SHADE_LEFT | SHADE_RIGHT | SHADE_TOP | SHADE_BOTTOM;
    int		  top;
    int		  button_x;

    if (!d->pixmap)
	return;


    style = gtk_widget_get_style (d->frame->style_window_rgba);

    if (d->state & (WNCK_WINDOW_STATE_MAXIMIZED_HORIZONTALLY |
		    WNCK_WINDOW_STATE_MAXIMIZED_VERTICALLY))
	corners = 0;

    color.r = style->bg[GTK_STATE_NORMAL].red   / 65535.0;
    color.g = style->bg[GTK_STATE_NORMAL].green / 65535.0;
    color.b = style->bg[GTK_STATE_NORMAL].blue  / 65535.0;

    if (d->frame_window)
    {
	GdkColormap *cmap;

	cmap = get_colormap_for_drawable (GDK_DRAWABLE (d->pixmap));
	gdk_drawable_set_colormap (GDK_DRAWABLE (d->pixmap), cmap);
	gdk_drawable_set_colormap (GDK_DRAWABLE (d->buffer_pixmap), cmap);
	drawable = GDK_DRAWABLE (d->buffer_pixmap);
    }
    else if (d->buffer_pixmap)
	drawable = GDK_DRAWABLE (d->buffer_pixmap);
    else
	drawable = GDK_DRAWABLE (d->pixmap);

    cr = gdk_cairo_create (GDK_DRAWABLE (drawable));
    if (!cr)
	return;

    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);

    top = d->frame->win_extents.top + d->frame->titlebar_height;

    x1 = d->context->left_space - d->frame->win_extents.left;
    y1 = d->context->top_space - d->frame->win_extents.top - d->frame->titlebar_height;
    x2 = d->width - d->context->right_space + d->frame->win_extents.right;
    y2 = d->height - d->context->bottom_space + d->frame->win_extents.bottom;

    h = d->height - d->context->top_space - d->context->bottom_space;

    cairo_set_line_width (cr, 1.0);

    if (!d->frame_window)
	draw_shadow_background (d, cr, d->shadow, d->context);

    if (d->active)
    {
	decor_color_t *title_color = _title_color;

	alpha = decoration_alpha + 0.3;

	fill_rounded_rectangle (cr,
				x1 + 0.5,
				y1 + 0.5,
				d->frame->win_extents.left - 0.5,
				top - 0.5,
				5.0, CORNER_TOPLEFT & corners,
				&title_color[0], 1.0, &title_color[1], alpha,
				SHADE_TOP | SHADE_LEFT);

	fill_rounded_rectangle (cr,
				x1 + d->frame->win_extents.left,
				y1 + 0.5,
				x2 - x1 - d->frame->win_extents.left -
				d->frame->win_extents.right,
				top - 0.5,
				5.0, 0,
				&title_color[0], 1.0, &title_color[1], alpha,
				SHADE_TOP);

	fill_rounded_rectangle (cr,
				x2 - d->frame->win_extents.right,
				y1 + 0.5,
				d->frame->win_extents.right - 0.5,
				top - 0.5,
				5.0, CORNER_TOPRIGHT & corners,
				&title_color[0], 1.0, &title_color[1], alpha,
				SHADE_TOP | SHADE_RIGHT);
    }
    else
    {
	alpha = decoration_alpha;

	fill_rounded_rectangle (cr,
				x1 + 0.5,
				y1 + 0.5,
				d->frame->win_extents.left - 0.5,
				top - 0.5,
				5.0, CORNER_TOPLEFT & corners,
				&color, 1.0, &color, alpha,
				SHADE_TOP | SHADE_LEFT);

	fill_rounded_rectangle (cr,
				x1 + d->frame->win_extents.left,
				y1 + 0.5,
				x2 - x1 - d->frame->win_extents.left -
				d->frame->win_extents.right,
				top - 0.5,
				5.0, 0,
				&color, 1.0, &color, alpha,
				SHADE_TOP);

	fill_rounded_rectangle (cr,
				x2 - d->frame->win_extents.right,
				y1 + 0.5,
				d->frame->win_extents.right - 0.5,
				top - 0.5,
				5.0, CORNER_TOPRIGHT & corners,
				&color, 1.0, &color, alpha,
				SHADE_TOP | SHADE_RIGHT);
    }

    fill_rounded_rectangle (cr,
			    x1 + 0.5,
			    y1 + top,
			    d->frame->win_extents.left - 0.5,
			    h,
			    5.0, 0,
			    &color, 1.0, &color, alpha,
			    SHADE_LEFT);

    fill_rounded_rectangle (cr,
			    x2 - d->frame->win_extents.right,
			    y1 + top,
			    d->frame->win_extents.right - 0.5,
			    h,
			    5.0, 0,
			    &color, 1.0, &color, alpha,
			    SHADE_RIGHT);


    fill_rounded_rectangle (cr,
			    x1 + 0.5,
			    y2 - d->frame->win_extents.bottom,
			    d->frame->win_extents.left - 0.5,
			    d->frame->win_extents.bottom - 0.5,
			    5.0, CORNER_BOTTOMLEFT & corners,
			    &color, 1.0, &color, alpha,
			    SHADE_BOTTOM | SHADE_LEFT);

    fill_rounded_rectangle (cr,
			    x1 + d->frame->win_extents.left,
			    y2 - d->frame->win_extents.bottom,
			    x2 - x1 - d->frame->win_extents.left -
			    d->frame->win_extents.right,
			    d->frame->win_extents.bottom - 0.5,
			    5.0, 0,
			    &color, 1.0, &color, alpha,
			    SHADE_BOTTOM);

    fill_rounded_rectangle (cr,
			    x2 - d->frame->win_extents.right,
			    y2 - d->frame->win_extents.bottom,
			    d->frame->win_extents.right - 0.5,
			    d->frame->win_extents.bottom - 0.5,
			    5.0, CORNER_BOTTOMRIGHT & corners,
			    &color, 1.0, &color, alpha,
			    SHADE_BOTTOM | SHADE_RIGHT);

    cairo_rectangle (cr,
		     d->context->left_space,
		     d->context->top_space,
		     d->width - d->context->left_space -
		     d->context->right_space,
		     h);
    gdk_cairo_set_source_color (cr, &style->bg[GTK_STATE_NORMAL]);
    cairo_fill (cr);

    cairo_set_operator (cr, CAIRO_OPERATOR_OVER);

    if (d->active)
    {
	gdk_cairo_set_source_color_alpha (cr,
					  &style->fg[GTK_STATE_NORMAL],
					  0.7);

	cairo_move_to (cr, x1 + 0.5, y1 + top - 0.5);
	cairo_rel_line_to (cr, x2 - x1 - 1.0, 0.0);

	cairo_stroke (cr);
    }

    rounded_rectangle (cr,
		       x1 + 0.5, y1 + 0.5,
		       x2 - x1 - 1.0, y2 - y1 - 1.0,
		       5.0,
		       (CORNER_TOPLEFT | CORNER_TOPRIGHT | CORNER_BOTTOMLEFT |
			CORNER_BOTTOMRIGHT) & corners);

    cairo_clip (cr);

    cairo_translate (cr, 1.0, 1.0);

    rounded_rectangle (cr,
		       x1 + 0.5, y1 + 0.5,
		       x2 - x1 - 1.0, y2 - y1 - 1.0,
		       5.0,
		       (CORNER_TOPLEFT | CORNER_TOPRIGHT | CORNER_BOTTOMLEFT |
			CORNER_BOTTOMRIGHT) & corners);

    cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 0.4);

    cairo_stroke (cr);

    cairo_translate (cr, -2.0, -2.0);

    rounded_rectangle (cr,
		       x1 + 0.5, y1 + 0.5,
		       x2 - x1 - 1.0, y2 - y1 - 1.0,
		       5.0,
		       (CORNER_TOPLEFT | CORNER_TOPRIGHT | CORNER_BOTTOMLEFT |
			CORNER_BOTTOMRIGHT) & corners);

    cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 0.1);

    cairo_stroke (cr);

    cairo_translate (cr, 1.0, 1.0);

    cairo_reset_clip (cr);

    rounded_rectangle (cr,
		       x1 + 0.5, y1 + 0.5,
		       x2 - x1 - 1.0, y2 - y1 - 1.0,
		       5.0,
		       (CORNER_TOPLEFT | CORNER_TOPRIGHT | CORNER_BOTTOMLEFT |
			CORNER_BOTTOMRIGHT) & corners);

    gdk_cairo_set_source_color_alpha (cr,
				      &style->fg[GTK_STATE_NORMAL],
				      alpha);

    cairo_stroke (cr);

    cairo_set_line_width (cr, 2.0);

    button_x = d->width - d->context->right_space - 13;

    if (d->actions & WNCK_WINDOW_ACTION_CLOSE)
    {
	button_state_offsets (button_x,
			      y1 - 3.0 + d->frame->titlebar_height / 2,
			      d->button_states[BUTTON_CLOSE], &x, &y);

	button_x -= 17;

	if (d->active)
	{
	    cairo_move_to (cr, x, y);
	    draw_close_button (d, cr, 3.0);
	    button_state_paint (cr, style, &color,
				d->button_states[BUTTON_CLOSE]);
	}
	else
	{
	    gdk_cairo_set_source_color_alpha (cr,
					      &style->fg[GTK_STATE_NORMAL],
					      alpha * 0.75);

	    cairo_move_to (cr, x, y);
	    draw_close_button (d, cr, 3.0);
	    cairo_fill (cr);
	}
    }

    if (d->actions & WNCK_WINDOW_ACTION_MAXIMIZE)
    {
	button_state_offsets (button_x,
			      y1 - 3.0 + d->frame->titlebar_height / 2,
			      d->button_states[BUTTON_MAX], &x, &y);

	button_x -= 17;

	cairo_set_fill_rule (cr, CAIRO_FILL_RULE_EVEN_ODD);

	if (d->active)
	{
	    gdk_cairo_set_source_color_alpha (cr,
					      &style->fg[GTK_STATE_NORMAL],
					      STROKE_ALPHA);

	    cairo_move_to (cr, x, y);

	    if (d->state & (WNCK_WINDOW_STATE_MAXIMIZED_HORIZONTALLY |
			    WNCK_WINDOW_STATE_MAXIMIZED_VERTICALLY))
		draw_unmax_button (d, cr, 4.0);
	    else
		draw_max_button (d, cr, 4.0);

	    button_state_paint (cr, style, &color,
				d->button_states[BUTTON_MAX]);
	}
	else
	{
	    gdk_cairo_set_source_color_alpha (cr,
					      &style->fg[GTK_STATE_NORMAL],
					      alpha * 0.75);

	    cairo_move_to (cr, x, y);

	    if (d->state & (WNCK_WINDOW_STATE_MAXIMIZED_HORIZONTALLY |
			    WNCK_WINDOW_STATE_MAXIMIZED_VERTICALLY))
		draw_unmax_button (d, cr, 4.0);
	    else
		draw_max_button (d, cr, 4.0);

	    cairo_fill (cr);
	}
    }

    if (d->actions & WNCK_WINDOW_ACTION_MINIMIZE)
    {
	button_state_offsets (button_x,
			      y1 - 3.0 + d->frame->titlebar_height / 2,
			      d->button_states[BUTTON_MIN], &x, &y);

	button_x -= 17;

	if (d->active)
	{
	    gdk_cairo_set_source_color_alpha (cr,
					      &style->fg[GTK_STATE_NORMAL],
					      STROKE_ALPHA);


	    cairo_move_to (cr, x, y);
	    draw_min_button (d, cr, 4.0);
	    button_state_paint (cr, style, &color,
				d->button_states[BUTTON_MIN]);
	}
	else
	{
	    gdk_cairo_set_source_color_alpha (cr,
					      &style->fg[GTK_STATE_NORMAL],
					      alpha * 0.75);

	    cairo_move_to (cr, x, y);
	    draw_min_button (d, cr, 4.0);
	    cairo_fill (cr);
	}
    }

    if (d->layout)
    {
	if (d->active)
	{
	    cairo_move_to (cr,
			   d->context->left_space + 21.0,
			   y1 + 2.0 + (d->frame->titlebar_height - d->frame->text_height) / 2.0);

	    gdk_cairo_set_source_color_alpha (cr,
					      &style->fg[GTK_STATE_NORMAL],
					      STROKE_ALPHA);

	    pango_cairo_layout_path (cr, d->layout);
	    cairo_stroke (cr);

	    cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
	}
	else
	{
	    gdk_cairo_set_source_color_alpha (cr,
					      &style->fg[GTK_STATE_NORMAL],
					      alpha);
	}

	cairo_move_to (cr,
		       d->context->left_space + 21.0,
		       y1 + 2.0 + (d->frame->titlebar_height - d->frame->text_height) / 2.0);

	pango_cairo_show_layout (cr, d->layout);
    }

    if (d->icon)
    {
	cairo_translate (cr, d->context->left_space + 1,
			 y1 - 5.0 + d->frame->titlebar_height / 2);
	cairo_set_source (cr, d->icon);
	cairo_rectangle (cr, 0.0, 0.0, 16.0, 16.0);
	cairo_clip (cr);

	if (d->active)
	    cairo_paint (cr);
	else
	    cairo_paint_with_alpha (cr, alpha);
    }

    cairo_destroy (cr);

    copy_to_front_buffer (d);

    if (d->frame_window)
    {
	GdkWindow *gdk_frame_window = gtk_widget_get_window (d->decor_window);

	gtk_image_set_from_pixmap (GTK_IMAGE (d->decor_image), d->pixmap, NULL);
	gtk_window_resize (GTK_WINDOW (d->decor_window), d->width, d->height);
	gdk_window_move (gdk_frame_window, 0, 0);
	gdk_window_lower (gdk_frame_window);
    }

    if (d->prop_xid)
    {
	decor_update_window_property (d);
	d->prop_xid = 0;
    }
}

static void
calc_button_size (decor_t *d)
{
    gint button_width = 0;

    if (d->actions & WNCK_WINDOW_ACTION_CLOSE)
	button_width += 17;

    if (d->actions & (WNCK_WINDOW_ACTION_MAXIMIZE_HORIZONTALLY   |
		      WNCK_WINDOW_ACTION_MAXIMIZE_VERTICALLY     |
		      WNCK_WINDOW_ACTION_UNMAXIMIZE_HORIZONTALLY |
		      WNCK_WINDOW_ACTION_UNMAXIMIZE_VERTICALLY))
	button_width += 17;

    if (d->actions & WNCK_WINDOW_ACTION_MINIMIZE)
	button_width += 17;

    if (button_width)
	button_width++;

    d->button_width = button_width;
}

gboolean
calc_decoration_size (decor_t *d,
		      gint    w,
		      gint    h,
		      gint    name_width,
		      gint    *width,
		      gint    *height)
{
    decor_layout_t layout;
    int		   top_width;

    if (!d->decorated)
	return FALSE;

    /* To avoid wasting texture memory, we only calculate the minimal
     * required decoration size then clip and stretch the texture where
     * appropriate
     */

    if (!d->frame_window)
    {
	calc_button_size (d);

	if (w < ICON_SPACE + d->button_width)
	    return FALSE;

	top_width = name_width + d->button_width + ICON_SPACE;
	if (w < top_width)
	    top_width = MAX (ICON_SPACE + d->button_width, w);

	if (d->active)
	    decor_get_default_layout (&d->frame->window_context_active, top_width, 1, &layout);
	else
	    decor_get_default_layout (&d->frame->window_context_inactive, top_width, 1, &layout);

	if (!d->context || memcmp (&layout, &d->border_layout, sizeof (layout)))
	{
	    *width  = layout.width;
	    *height = layout.height;

	    d->border_layout = layout;
	    if (d->active)
	    {
		d->context       = &d->frame->window_context_active;
		d->shadow        = d->frame->border_shadow_active;
	    }
	    else
	    {
		d->context       = &d->frame->window_context_inactive;
		d->shadow        = d->frame->border_shadow_inactive;
	    }

	    return TRUE;
	}
    }
    else
    {
	calc_button_size (d);

	/* _default_win_extents + top height */

	top_width = name_width + d->button_width + ICON_SPACE;
	if (w < top_width)
	    top_width = MAX (ICON_SPACE + d->button_width, w);

	decor_get_default_layout (&d->frame->window_context_no_shadow,
				  d->client_width, d->client_height, &layout);

	*width = layout.width;
	*height = layout.height;

	d->border_layout = layout;
	if (d->active)
	{
	    d->context       = &d->frame->window_context_active;
	    d->shadow        = d->frame->border_shadow_active;
	}
	else
	{
	    d->context       = &d->frame->window_context_inactive;
	    d->shadow        = d->frame->border_shadow_inactive;
	}

	return TRUE;
    }

    return FALSE;
}

gboolean
get_button_position (decor_t *d,
		     gint    i,
		     gint    width,
		     gint    height,
		     gint    *x,
		     gint    *y,
		     gint    *w,
		     gint    *h)
{
    if (i > BUTTON_MENU)
	return FALSE;

    if (d->frame_window)
    {
	*x = bpos[i].x + bpos[i].xw * width + d->frame->win_extents.left + 4;
	*y = bpos[i].y + bpos[i].yh * height + bpos[i].yth *
	    (d->frame->titlebar_height - 17) + d->frame->win_extents.top + 2;
    }
    else
    {
	*x = bpos[i].x + bpos[i].xw * width;
	*y = bpos[i].y + bpos[i].yh * height + bpos[i].yth *
	    (d->frame->titlebar_height - 17);
    }

    *w = bpos[i].w + bpos[i].ww * width;
    *h = bpos[i].h + bpos[i].hh * height + bpos[i].hth +
	(d->frame->titlebar_height - 17);

    /* hack to position multiple buttons on the right */
    if (i != BUTTON_MENU)
	*x -= 10 + 16 * i;

    return TRUE;
}

void
get_event_window_position (decor_t *d,
			   gint    i,
			   gint    j,
			   gint    width,
			   gint    height,
			   gint    *x,
			   gint    *y,
			   gint    *w,
			   gint    *h)
{
    if (d->frame_window)
    {
	*x = pos[i][j].x + pos[i][j].xw * width + d->frame->win_extents.left;
	*y = pos[i][j].y + d->frame->win_extents.top +
	     pos[i][j].yh * height + pos[i][j].yth * (d->frame->titlebar_height - 17);

	if (i == 0 && (j == 0 || j == 2))
	    *y -= d->frame->titlebar_height;
    }
    else
    {
	*x = pos[i][j].x + pos[i][j].xw * width;
	*y = pos[i][j].y +
	     pos[i][j].yh * height + pos[i][j].yth * (d->frame->titlebar_height - 17);
    }

    if ((d->state & WNCK_WINDOW_STATE_MAXIMIZED_HORIZONTALLY) &&
	(j == 0 || j == 2))
    {
	*w = 0;
    }
    else
    {
	*w = pos[i][j].w + pos[i][j].ww * width;
    }

    if ((d->state & WNCK_WINDOW_STATE_MAXIMIZED_VERTICALLY) &&
	(i == 0 || i == 2))
    {
	*h = 0;
    }
    else
    {
	*h = pos[i][j].h +
	     pos[i][j].hh * height + pos[i][j].hth * (d->frame->titlebar_height - 17);
    }
}

gfloat
get_title_scale (decor_frame_t *frame)
{
    return 1.0f;
}

void
update_border_extents (decor_frame_t *frame)
{
    frame = gwd_decor_frame_ref (frame);

    gwd_cairo_window_decoration_get_extents (&frame->win_extents,
					     &frame->max_win_extents);

    frame->titlebar_height = frame->max_titlebar_height =
	    (frame->text_height < 17) ? 17 : frame->text_height;

    gwd_decor_frame_unref (frame);
}
