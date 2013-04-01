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

#ifdef USE_METACITY

static void
decor_update_meta_window_property (decor_t	  *d,
				   MetaTheme	  *theme,
				   MetaFrameFlags flags,
				   Region	  top,
				   Region	  bottom,
				   Region	  left,
				   Region	  right)
{
    long	    *data;
    Display	    *xdisplay =
	GDK_DISPLAY_XDISPLAY (gdk_display_get_default ());
    gint	    nQuad;
    decor_extents_t win_extents, frame_win_extents;
    decor_extents_t max_win_extents, frame_max_win_extents;
    decor_quad_t    quads[N_QUADS_MAX];
    unsigned int    nOffset = 1;
    unsigned int    frame_type = populate_frame_type (d);
    unsigned int    frame_state = populate_frame_state (d);
    unsigned int    frame_actions = populate_frame_actions (d);
    gint            w, lh, rh;
    gint	    top_stretch_offset;
    gint	    bottom_stretch_offset;
    gint	    left_stretch_offset;
    gint	    right_stretch_offset;
    gint	    mutter_draggable_border_width = 0;

    win_extents = frame_win_extents = d->frame->win_extents;
    max_win_extents = frame_max_win_extents = d->frame->max_win_extents;

    g_object_get (settings, "draggable-border-width", &mutter_draggable_border_width, NULL);

    /* Add the invisible grab area padding, but only for
     * pixmap type decorations */
    if (!d->frame_window)
    {
	if (flags & META_FRAME_ALLOWS_HORIZONTAL_RESIZE)
	{
	    frame_win_extents.left += mutter_draggable_border_width;
	    frame_win_extents.right += mutter_draggable_border_width;
	    frame_max_win_extents.left += mutter_draggable_border_width;
	    frame_max_win_extents.right += mutter_draggable_border_width;
	}

	if (flags & META_FRAME_ALLOWS_VERTICAL_RESIZE)
	{
	    frame_win_extents.bottom += mutter_draggable_border_width;
	    frame_max_win_extents.bottom += mutter_draggable_border_width;
	}
    }

    w = d->border_layout.top.x2 - d->border_layout.top.x1 -
	d->context->left_space - d->context->right_space;

    if (d->border_layout.rotation)
	lh = d->border_layout.left.x2 - d->border_layout.left.x1;
    else
	lh = d->border_layout.left.y2 - d->border_layout.left.y1;

    if (d->border_layout.rotation)
	rh = d->border_layout.right.x2 - d->border_layout.right.x1;
    else
	rh = d->border_layout.right.y2 - d->border_layout.right.y1;

    left_stretch_offset   = lh / 2;
    right_stretch_offset  = rh / 2;
    top_stretch_offset    = w - d->button_width - 1;
    bottom_stretch_offset = (d->border_layout.bottom.x2 -
			     d->border_layout.bottom.x1 -
			     d->context->left_space -
			     d->context->right_space) / 2;

    nQuad = decor_set_lXrXtXbX_window_quads (quads, d->context,
					     &d->border_layout,
					     left_stretch_offset,
					     right_stretch_offset,
					     top_stretch_offset,
					     bottom_stretch_offset);

    win_extents.top += d->frame->titlebar_height;
    frame_win_extents.top += d->frame->titlebar_height;
    max_win_extents.top += d->frame->max_titlebar_height;
    frame_max_win_extents.top += d->frame->max_titlebar_height;

    if (d->frame_window)
    {
	data = decor_alloc_property (nOffset, WINDOW_DECORATION_TYPE_WINDOW);
	decor_gen_window_property (data, nOffset - 1, &win_extents, &max_win_extents, 20, 20,
				   frame_type, frame_state, frame_actions);
    }
    else
    {
	data = decor_alloc_property (nOffset, WINDOW_DECORATION_TYPE_PIXMAP);
	decor_quads_to_property (data, nOffset - 1, GDK_PIXMAP_XID (d->pixmap),
				 &frame_win_extents, &win_extents,
				 &frame_max_win_extents, &max_win_extents,
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

    free (data);

    decor_update_blur_property (d,
				w, lh,
				top, top_stretch_offset,
				bottom, bottom_stretch_offset,
				left, left_stretch_offset,
				right, right_stretch_offset);
}

static void
meta_get_corner_radius (const MetaFrameGeometry *fgeom,
			int			*top_left_radius,
			int			*top_right_radius,
			int			*bottom_left_radius,
			int			*bottom_right_radius)
{

#ifdef HAVE_METACITY_2_17_0
    *top_left_radius     = fgeom->top_left_corner_rounded_radius;
    *top_right_radius    = fgeom->top_right_corner_rounded_radius;
    *bottom_left_radius  = fgeom->bottom_left_corner_rounded_radius;
    *bottom_right_radius = fgeom->bottom_right_corner_rounded_radius;
#else
    *top_left_radius     = fgeom->top_left_corner_rounded ? 5 : 0;
    *top_right_radius    = fgeom->top_right_corner_rounded ? 5 : 0;
    *bottom_left_radius  = fgeom->bottom_left_corner_rounded ? 5 : 0;
    *bottom_right_radius = fgeom->bottom_right_corner_rounded ? 5 : 0;
#endif

}

static int
radius_to_width (int radius,
		 int i)
{
    float r1 = sqrt (radius) + radius;
    float r2 = r1 * r1 - (r1 - (i + 0.5)) * (r1 - (i + 0.5));

    return floor (0.5f + r1 - sqrt (r2));
}

static Region
meta_get_top_border_region (const MetaFrameGeometry *fgeom,
			    int			    width)
{
    Region     corners_xregion, border_xregion;
    XRectangle xrect;
    int	       top_left_radius;
    int	       top_right_radius;
    int	       bottom_left_radius;
    int	       bottom_right_radius;
    int	       w, i;

    corners_xregion = XCreateRegion ();

    meta_get_corner_radius (fgeom,
			    &top_left_radius,
			    &top_right_radius,
			    &bottom_left_radius,
			    &bottom_right_radius);

    if (top_left_radius)
    {
	for (i = 0; i < top_left_radius; i++)
	{
	    w = radius_to_width (top_left_radius, i);

	    xrect.x	 = 0;
	    xrect.y	 = i;
	    xrect.width  = w;
	    xrect.height = 1;

	    XUnionRectWithRegion (&xrect, corners_xregion, corners_xregion);
	}
    }

    if (top_right_radius)
    {
	for (i = 0; i < top_right_radius; i++)
	{
	    w = radius_to_width (top_right_radius, i);

	    xrect.x	 = width - w;
	    xrect.y	 = i;
	    xrect.width  = w;
	    xrect.height = 1;

	    XUnionRectWithRegion (&xrect, corners_xregion, corners_xregion);
	}
    }

    border_xregion = XCreateRegion ();

    xrect.x = 0;
    xrect.y = 0;
    xrect.width = width;
    xrect.height = fgeom->top_height;

    XUnionRectWithRegion (&xrect, border_xregion, border_xregion);

    XSubtractRegion (border_xregion, corners_xregion, border_xregion);

    XDestroyRegion (corners_xregion);

    return border_xregion;
}

static Region
meta_get_bottom_border_region (const MetaFrameGeometry *fgeom,
			       int		        width)
{
    Region     corners_xregion, border_xregion;
    XRectangle xrect;
    int	       top_left_radius;
    int	       top_right_radius;
    int	       bottom_left_radius;
    int	       bottom_right_radius;
    int	       w, i;

    corners_xregion = XCreateRegion ();

    meta_get_corner_radius (fgeom,
			    &top_left_radius,
			    &top_right_radius,
			    &bottom_left_radius,
			    &bottom_right_radius);

    if (bottom_left_radius)
    {
	for (i = 0; i < bottom_left_radius; i++)
	{
	    w = radius_to_width (bottom_left_radius, i);

	    xrect.x	 = 0;
	    xrect.y	 = fgeom->bottom_height - i - 1;
	    xrect.width  = w;
	    xrect.height = 1;

	    XUnionRectWithRegion (&xrect, corners_xregion, corners_xregion);
	}
    }

    if (bottom_right_radius)
    {
	for (i = 0; i < bottom_right_radius; i++)
	{
	    w = radius_to_width (bottom_right_radius, i);

	    xrect.x	 = width - w;
	    xrect.y	 = fgeom->bottom_height - i - 1;
	    xrect.width  = w;
	    xrect.height = 1;

	    XUnionRectWithRegion (&xrect, corners_xregion, corners_xregion);
	}
    }

    border_xregion = XCreateRegion ();

    xrect.x = 0;
    xrect.y = 0;
    xrect.width = width;
    xrect.height = fgeom->bottom_height;

    XUnionRectWithRegion (&xrect, border_xregion, border_xregion);

    XSubtractRegion (border_xregion, corners_xregion, border_xregion);

    XDestroyRegion (corners_xregion);

    return border_xregion;
}

static Region
meta_get_left_border_region (const MetaFrameGeometry *fgeom,
			     int		     height)
{
    Region     border_xregion;
    XRectangle xrect;

    border_xregion = XCreateRegion ();

    xrect.x	 = 0;
    xrect.y	 = 0;
    xrect.width  = fgeom->left_width;
    xrect.height = height - fgeom->top_height - fgeom->bottom_height;

    XUnionRectWithRegion (&xrect, border_xregion, border_xregion);

    return border_xregion;
}

static Region
meta_get_right_border_region (const MetaFrameGeometry *fgeom,
			      int		      height)
{
    Region     border_xregion;
    XRectangle xrect;

    border_xregion = XCreateRegion ();

    xrect.x	 = 0;
    xrect.y	 = 0;
    xrect.width  = fgeom->right_width;
    xrect.height = height - fgeom->top_height - fgeom->bottom_height;

    XUnionRectWithRegion (&xrect, border_xregion, border_xregion);

    return border_xregion;
}

static MetaButtonState
meta_button_state (int state)
{
    if (state & IN_EVENT_WINDOW)
    {
	if (state & PRESSED_EVENT_WINDOW)
	    return META_BUTTON_STATE_PRESSED;

	return META_BUTTON_STATE_PRELIGHT;
    }

    return META_BUTTON_STATE_NORMAL;
}

static MetaButtonType
meta_function_to_type (MetaButtonFunction function)
{
    switch (function) {
    case META_BUTTON_FUNCTION_MENU:
	return META_BUTTON_TYPE_MENU;
    case META_BUTTON_FUNCTION_MINIMIZE:
	return META_BUTTON_TYPE_MINIMIZE;
    case META_BUTTON_FUNCTION_MAXIMIZE:
	return META_BUTTON_TYPE_MAXIMIZE;
    case META_BUTTON_FUNCTION_CLOSE:
	return META_BUTTON_TYPE_CLOSE;

#ifdef HAVE_METACITY_2_17_0
    case META_BUTTON_FUNCTION_SHADE:
	return META_BUTTON_TYPE_SHADE;
    case META_BUTTON_FUNCTION_ABOVE:
	return META_BUTTON_TYPE_ABOVE;
    case META_BUTTON_FUNCTION_STICK:
	return META_BUTTON_TYPE_STICK;
    case META_BUTTON_FUNCTION_UNSHADE:
	return META_BUTTON_TYPE_UNSHADE;
    case META_BUTTON_FUNCTION_UNABOVE:
	return META_BUTTON_TYPE_UNABOVE;
    case META_BUTTON_FUNCTION_UNSTICK:
	return META_BUTTON_TYPE_UNSTICK;
#endif

    default:
	break;
    }

    return META_BUTTON_TYPE_LAST;
}

static MetaButtonState
meta_button_state_for_button_type (decor_t	  *d,
				   MetaButtonType type)
{
    switch (type) {
    case META_BUTTON_TYPE_LEFT_LEFT_BACKGROUND:
	type = meta_function_to_type (meta_button_layout.left_buttons[0]);
	break;
    case META_BUTTON_TYPE_LEFT_MIDDLE_BACKGROUND:
	type = meta_function_to_type (meta_button_layout.left_buttons[1]);
	break;
    case META_BUTTON_TYPE_LEFT_RIGHT_BACKGROUND:
	type = meta_function_to_type (meta_button_layout.left_buttons[2]);
	break;
    case META_BUTTON_TYPE_RIGHT_LEFT_BACKGROUND:
	type = meta_function_to_type (meta_button_layout.right_buttons[0]);
	break;
    case META_BUTTON_TYPE_RIGHT_MIDDLE_BACKGROUND:
	type = meta_function_to_type (meta_button_layout.right_buttons[1]);
	break;
    case META_BUTTON_TYPE_RIGHT_RIGHT_BACKGROUND:
	type = meta_function_to_type (meta_button_layout.right_buttons[2]);
    default:
	break;
    }

    switch (type) {
    case META_BUTTON_TYPE_CLOSE:
	return meta_button_state (d->button_states[BUTTON_CLOSE]);
    case META_BUTTON_TYPE_MAXIMIZE:
	return meta_button_state (d->button_states[BUTTON_MAX]);
    case META_BUTTON_TYPE_MINIMIZE:
	return meta_button_state (d->button_states[BUTTON_MIN]);
    case META_BUTTON_TYPE_MENU:
	return meta_button_state (d->button_states[BUTTON_MENU]);

#ifdef HAVE_METACITY_2_17_0
    case META_BUTTON_TYPE_SHADE:
	return meta_button_state (d->button_states[BUTTON_SHADE]);
    case META_BUTTON_TYPE_ABOVE:
	return meta_button_state (d->button_states[BUTTON_ABOVE]);
    case META_BUTTON_TYPE_STICK:
	return meta_button_state (d->button_states[BUTTON_STICK]);
    case META_BUTTON_TYPE_UNSHADE:
	return meta_button_state (d->button_states[BUTTON_UNSHADE]);
    case META_BUTTON_TYPE_UNABOVE:
	return meta_button_state (d->button_states[BUTTON_UNABOVE]);
    case META_BUTTON_TYPE_UNSTICK:
	return meta_button_state (d->button_states[BUTTON_UNSTICK]);
#endif

    default:
	break;
    }

    return META_BUTTON_STATE_NORMAL;
}

void
meta_get_decoration_geometry (decor_t		*d,
			      MetaTheme	        *theme,
			      MetaFrameFlags    *flags,
			      MetaFrameGeometry *fgeom,
			      MetaButtonLayout  *button_layout,
			      MetaFrameType	frame_type,
			      GdkRectangle      *clip)
{
    gint left_width, right_width, top_height, bottom_height;

    if (!(frame_type < META_FRAME_TYPE_LAST))
	frame_type = META_FRAME_TYPE_NORMAL;

    if (meta_button_layout_set)
    {
	*button_layout = meta_button_layout;
    }
    else
    {
	gint i;

	button_layout->left_buttons[0] = META_BUTTON_FUNCTION_MENU;

	for (i = 1; i < MAX_BUTTONS_PER_CORNER; i++)
	    button_layout->left_buttons[i] = META_BUTTON_FUNCTION_LAST;

	button_layout->right_buttons[0] = META_BUTTON_FUNCTION_MINIMIZE;
	button_layout->right_buttons[1] = META_BUTTON_FUNCTION_MAXIMIZE;
	button_layout->right_buttons[2] = META_BUTTON_FUNCTION_CLOSE;

	for (i = 3; i < MAX_BUTTONS_PER_CORNER; i++)
	    button_layout->right_buttons[i] = META_BUTTON_FUNCTION_LAST;
    }

    *flags = 0;

    if (d->actions & WNCK_WINDOW_ACTION_CLOSE)
	*flags |= (MetaFrameFlags ) META_FRAME_ALLOWS_DELETE;

    if (d->actions & WNCK_WINDOW_ACTION_MINIMIZE)
	*flags |= (MetaFrameFlags ) META_FRAME_ALLOWS_MINIMIZE;

    if (d->actions & WNCK_WINDOW_ACTION_MAXIMIZE)
	*flags |= (MetaFrameFlags ) META_FRAME_ALLOWS_MAXIMIZE;

    *flags |= (MetaFrameFlags ) META_FRAME_ALLOWS_MENU;

    if (d->actions & WNCK_WINDOW_ACTION_RESIZE)
    {
	if (!(d->state & WNCK_WINDOW_STATE_MAXIMIZED_VERTICALLY))
	    *flags |= (MetaFrameFlags ) META_FRAME_ALLOWS_VERTICAL_RESIZE;
	if (!(d->state & WNCK_WINDOW_STATE_MAXIMIZED_HORIZONTALLY))
	    *flags |= (MetaFrameFlags ) META_FRAME_ALLOWS_HORIZONTAL_RESIZE;
    }

    if (d->actions & WNCK_WINDOW_ACTION_MOVE)
	*flags |= (MetaFrameFlags ) META_FRAME_ALLOWS_MOVE;

    if (d->actions & WNCK_WINDOW_ACTION_MAXIMIZE)
	*flags |= (MetaFrameFlags ) META_FRAME_ALLOWS_MAXIMIZE;

    if (d->actions & WNCK_WINDOW_ACTION_SHADE)
	*flags |= (MetaFrameFlags ) META_FRAME_ALLOWS_SHADE;

    if (d->active)
	*flags |= (MetaFrameFlags ) META_FRAME_HAS_FOCUS;

    if ((d->state & META_MAXIMIZED) == META_MAXIMIZED)
	*flags |= (MetaFrameFlags ) META_FRAME_MAXIMIZED;

    if (d->state & WNCK_WINDOW_STATE_STICKY)
	*flags |= (MetaFrameFlags ) META_FRAME_STUCK;

    if (d->state & WNCK_WINDOW_STATE_FULLSCREEN)
	*flags |= (MetaFrameFlags ) META_FRAME_FULLSCREEN;

    if (d->state & WNCK_WINDOW_STATE_SHADED)
	*flags |= (MetaFrameFlags ) META_FRAME_SHADED;

#ifdef HAVE_METACITY_2_17_0
    if (d->state & WNCK_WINDOW_STATE_ABOVE)
	*flags |= (MetaFrameFlags ) META_FRAME_ABOVE;
#endif

    meta_theme_get_frame_borders (theme,
				  frame_type,
				  d->frame->text_height,
				  *flags,
				  &top_height,
				  &bottom_height,
				  &left_width,
				  &right_width);

    clip->x = d->context->left_space - left_width;
    clip->y = d->context->top_space - top_height;

    clip->width = d->border_layout.top.x2 - d->border_layout.top.x1;
    clip->width -= d->context->right_space + d->context->left_space;

    if (d->border_layout.rotation)
	clip->height = d->border_layout.left.x2 - d->border_layout.left.x1;
    else
	clip->height = d->border_layout.left.y2 - d->border_layout.left.y1;

    meta_theme_calc_geometry (theme,
			      frame_type,
			      d->frame->text_height,
			      *flags,
			      clip->width,
			      clip->height,
			      button_layout,
			      fgeom);

    clip->width  += left_width + right_width;
    clip->height += top_height + bottom_height;
}

void
meta_draw_window_decoration (decor_t *d)
{
    Display	      *xdisplay =
	GDK_DISPLAY_XDISPLAY (gdk_display_get_default ());
    GdkPixmap	      *pixmap;
    Picture	      src;
    MetaButtonState   button_states [META_BUTTON_TYPE_LAST];
    MetaButtonLayout  button_layout;
    MetaFrameGeometry fgeom;
    MetaFrameFlags    flags;
    MetaFrameType     frame_type;
    MetaTheme	      *theme;
    GtkStyle	      *style;
    cairo_t	      *cr;
    gint	      size, i;
    GdkRectangle      clip, rect;
    GdkDrawable       *drawable;
    Region	      top_region = NULL;
    Region	      bottom_region = NULL;
    Region	      left_region = NULL;
    Region	      right_region = NULL;
    gdouble	      meta_active_opacity, meta_inactive_opacity;
    gboolean	      meta_active_shade_opacity, meta_inactive_shade_opacity;

    g_object_get (settings, "metacity-active-opacity", &meta_active_opacity, NULL);
    g_object_get (settings, "metacity-inactive-opacity", &meta_inactive_opacity, NULL);
    g_object_get (settings, "metacity-active-shade-opacity", &meta_active_shade_opacity, NULL);
    g_object_get (settings, "metacity-inactive-shade-opacity", &meta_inactive_shade_opacity, NULL);

    double	      alpha = (d->active) ? meta_active_opacity : meta_inactive_opacity;
    gboolean	      shade_alpha = (d->active) ? meta_active_shade_opacity :
						  meta_inactive_shade_opacity;
    MetaFrameStyle    *frame_style;
    GtkWidget	      *style_window;
    GdkColor	      bg_color;
    double	      bg_alpha;

    if (!d->pixmap || !d->picture)
	return;

    if (d->frame_window)
    {
	GdkColormap *cmap;
	
	cmap = get_colormap_for_drawable (GDK_DRAWABLE (d->pixmap));
	gdk_drawable_set_colormap (GDK_DRAWABLE (d->pixmap), cmap);
	gdk_drawable_set_colormap (GDK_DRAWABLE (d->buffer_pixmap), cmap);
    }

    if (decoration_alpha == 1.0)
	alpha = 1.0;

    if (gdk_drawable_get_depth (GDK_DRAWABLE (d->pixmap)) == 32)
    {
	style = gtk_widget_get_style (d->frame->style_window_rgba);
	style_window = d->frame->style_window_rgba;
    }
    else
    {
	style = gtk_widget_get_style (d->frame->style_window_rgb);
	style_window = d->frame->style_window_rgb;
    }

    drawable = d->buffer_pixmap ? d->buffer_pixmap : d->pixmap;

    cr = gdk_cairo_create (GDK_DRAWABLE (drawable));

    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);

    theme = meta_theme_get_current ();

    frame_type = meta_frame_type_from_string (d->frame->type);

    if (frame_type == META_FRAME_TYPE_LAST)
	frame_type = META_FRAME_TYPE_NORMAL;

    meta_get_decoration_geometry (d, theme, &flags, &fgeom, &button_layout,
				  frame_type, &clip);

    if ((d->prop_xid || !d->buffer_pixmap) && !d->frame_window)
	draw_shadow_background (d, cr, d->shadow, d->context);

    for (i = 0; i < META_BUTTON_TYPE_LAST; i++)
	button_states[i] = meta_button_state_for_button_type (d, i);

    frame_style = meta_theme_get_frame_style (theme,
					      frame_type,
					      flags);

    bg_color = style->bg[GTK_STATE_NORMAL];
    bg_alpha = 1.0;

#ifdef HAVE_METACITY_2_17_0
    if (frame_style->window_background_color)
    {
	meta_color_spec_render (frame_style->window_background_color,
				GTK_WIDGET (style_window),
				&bg_color);

	bg_alpha = frame_style->window_background_alpha / 255.0;
    }
#endif

    cairo_destroy (cr);

    rect.x     = 0;
    rect.y     = 0;
    rect.width = clip.width;

    size = MAX (fgeom.top_height, fgeom.bottom_height);

    if (rect.width && size)
    {
	XRenderPictFormat *format;

	if (d->frame_window)
	{
	    GdkColormap *cmap;

	    cmap   = get_colormap_for_drawable (GDK_DRAWABLE (d->pixmap));
	    pixmap = create_pixmap (rect.width, size, d->frame->style_window_rgb);
	    gdk_drawable_set_colormap (GDK_DRAWABLE (pixmap), cmap);
	}
	else
	    pixmap = create_pixmap (rect.width, size, d->frame->style_window_rgba);

	cr = gdk_cairo_create (GDK_DRAWABLE (pixmap));
	gdk_cairo_set_source_color_alpha (cr, &bg_color, bg_alpha);
	cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);

	format = get_format_for_drawable (d, GDK_DRAWABLE (pixmap));
	src = XRenderCreatePicture (xdisplay, GDK_PIXMAP_XID (pixmap),
				    format, 0, NULL);

	if (fgeom.top_height)
	{
	    rect.height = fgeom.top_height;

	    cairo_paint (cr);

	    meta_theme_draw_frame (theme,
				   style_window,
				   pixmap,
				   &rect,
				   0, 0,
				   frame_type,
				   flags,
				   clip.width - fgeom.left_width -
				   fgeom.right_width,
				   clip.height - fgeom.top_height -
				   fgeom.bottom_height,
				   d->layout,
				   d->frame->text_height,
				   &button_layout,
				   button_states,
				   d->icon_pixbuf,
				   NULL);

	    top_region = meta_get_top_border_region (&fgeom, clip.width);

	    decor_blend_border_picture (xdisplay,
					d->context,
					src,
					0, 0,
					d->picture,
					&d->border_layout,
					BORDER_TOP,
					top_region,
					alpha * 0xffff,
					shade_alpha,
				        0);
	}

	if (fgeom.bottom_height)
	{
	    rect.height = fgeom.bottom_height;

	    cairo_paint (cr);

	    meta_theme_draw_frame (theme,
				   style_window,
				   pixmap,
				   &rect,
				   0,
				   -(clip.height - fgeom.bottom_height),
				   frame_type,
				   flags,
				   clip.width - fgeom.left_width -
				   fgeom.right_width,
				   clip.height - fgeom.top_height -
				   fgeom.bottom_height,
				   d->layout,
				   d->frame->text_height,
				   &button_layout,
				   button_states,
				   d->icon_pixbuf,
				   NULL);

	    bottom_region = meta_get_bottom_border_region (&fgeom, clip.width);

	    decor_blend_border_picture (xdisplay,
					d->context,
					src,
					0, 0,
					d->picture,
					&d->border_layout,
					BORDER_BOTTOM,
					bottom_region,
					alpha * 0xffff,
					shade_alpha,
					0);

	}

	cairo_destroy (cr);

	g_object_unref (G_OBJECT (pixmap));

	XRenderFreePicture (xdisplay, src);
    }

    rect.height = clip.height - fgeom.top_height - fgeom.bottom_height;

    size = MAX (fgeom.left_width, fgeom.right_width);

    if (size && rect.height)
    {
	XRenderPictFormat *format;

	if (d->frame_window)
	{
	    GdkColormap *cmap;

	    cmap   = get_colormap_for_drawable (GDK_DRAWABLE (d->pixmap));
	    pixmap = create_pixmap (size, rect.height, d->frame->style_window_rgb);
	    gdk_drawable_set_colormap (GDK_DRAWABLE (pixmap), cmap);
	}
	else
	    pixmap = create_pixmap (size, rect.height, d->frame->style_window_rgba);

	cr = gdk_cairo_create (GDK_DRAWABLE (pixmap));
	gdk_cairo_set_source_color_alpha (cr, &bg_color, bg_alpha);
	cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);

	format = get_format_for_drawable (d, GDK_DRAWABLE (pixmap));
	src = XRenderCreatePicture (xdisplay, GDK_PIXMAP_XID (pixmap),
				    format, 0, NULL);

	if (fgeom.left_width)
	{
	    rect.width = fgeom.left_width;

	    cairo_paint (cr);

	    meta_theme_draw_frame (theme,
				   style_window,
				   pixmap,
				   &rect,
				   0,
				   -fgeom.top_height,
				   frame_type,
				   flags,
				   clip.width - fgeom.left_width -
				   fgeom.right_width,
				   clip.height - fgeom.top_height -
				   fgeom.bottom_height,
				   d->layout,
				   d->frame->text_height,
				   &button_layout,
				   button_states,
				   d->icon_pixbuf,
				   NULL);

	    left_region = meta_get_left_border_region (&fgeom, clip.height);

	    decor_blend_border_picture (xdisplay,
					d->context,
					src,
					0, 0,
					d->picture,
					&d->border_layout,
					BORDER_LEFT,
					left_region,
					alpha * 0xffff,
					shade_alpha,
				        0);
	}

	if (fgeom.right_width)
	{
	    rect.width = fgeom.right_width;

	    cairo_paint (cr);

	    meta_theme_draw_frame (theme,
				   style_window,
				   pixmap,
				   &rect,
				   -(clip.width - fgeom.right_width),
				   -fgeom.top_height,
				   frame_type,
				   flags,
				   clip.width - fgeom.left_width -
				   fgeom.right_width,
				   clip.height - fgeom.top_height -
				   fgeom.bottom_height,
				   d->layout,
				   d->frame->text_height,
				   &button_layout,
				   button_states,
				   d->icon_pixbuf,
				   NULL);

	    right_region = meta_get_right_border_region (&fgeom, clip.height);

	    decor_blend_border_picture (xdisplay,
					d->context,
					src,
					0, 0,
					d->picture,
					&d->border_layout,
					BORDER_RIGHT,
					right_region,
					alpha * 0xffff,
					shade_alpha,
				        0);
	}

	cairo_destroy (cr);

	g_object_unref (G_OBJECT (pixmap));

	XRenderFreePicture (xdisplay, src);
    }

    copy_to_front_buffer (d);

    if (d->frame_window)
    {
	GdkWindow *gdk_frame_window = gtk_widget_get_window (d->decor_window);

	/*
	 * FIXME: What is '4' supposed to be for here...
	 */

	gtk_image_set_from_pixmap (GTK_IMAGE (d->decor_image), d->pixmap, NULL);
	gtk_window_resize (GTK_WINDOW (d->decor_window), d->width, d->height);
	gdk_window_move (gdk_frame_window,
			 d->context->left_corner_space - 1,
			 d->context->top_corner_space - 1);
	gdk_window_lower (gdk_frame_window);
    }

    if (d->prop_xid)
    {
	/* translate from frame to client window space */
	if (top_region)
	    XOffsetRegion (top_region, -fgeom.left_width, -fgeom.top_height);
	if (bottom_region)
	    XOffsetRegion (bottom_region, -fgeom.left_width, 0);
	if (left_region)
	    XOffsetRegion (left_region, -fgeom.left_width, 0);

	decor_update_meta_window_property (d, theme, flags,
					   top_region,
					   bottom_region,
					   left_region,
					   right_region);
	d->prop_xid = 0;
    }

    if (top_region)
	XDestroyRegion (top_region);
    if (bottom_region)
	XDestroyRegion (bottom_region);
    if (left_region)
	XDestroyRegion (left_region);
    if (right_region)
	XDestroyRegion (right_region);
}

void
meta_calc_button_size (decor_t *d)
{
    gint i, min_x, x, y, w, h, width;

    width = d->border_layout.top.x2 - d->border_layout.top.x1 -
	    d->context->left_space - d->context->right_space;
    min_x = width;

    for (i = 0; i < 3; i++)
    {
	static guint button_actions[3] = {
	    WNCK_WINDOW_ACTION_CLOSE,
	    WNCK_WINDOW_ACTION_MAXIMIZE,
	    WNCK_WINDOW_ACTION_MINIMIZE
	};

	if (d->actions & button_actions[i])
	{
	    if (meta_get_button_position (d, i, width, 256,
					  &x, &y, &w, &h))
	    {
		if (x > width / 2 && x < min_x)
		    min_x = x;
	    }
	}
    }

    d->button_width = width - min_x + 6;
}

gboolean
meta_get_button_position (decor_t	 *d,
			  gint		i,
			  gint		width,
			  gint		height,
			  gint		*x,
			  gint		*y,
			  gint		*w,
			  gint		*h)
{
    MetaButtonLayout  button_layout;
    MetaFrameGeometry fgeom;
    MetaFrameFlags    flags;
    MetaTheme	      *theme;

    GdkRectangle      clip;

#ifdef HAVE_METACITY_2_15_21
    MetaButtonSpace   *space;
#else
    GdkRectangle      *space;
#endif

    gint mutter_draggable_border_width = 0;

    g_object_get (settings, "draggable-border-width", &mutter_draggable_border_width, NULL);

    if (!d->context)
    {
	/* undecorated windows implicitly have no buttons */
	return FALSE;
    }

    theme = meta_theme_get_current ();

    meta_get_decoration_geometry (d, theme, &flags, &fgeom, &button_layout,
				  meta_frame_type_from_string (d->frame->type),
				  &clip);

    switch (i) {
    case BUTTON_MENU:
	if (!meta_button_present (&button_layout, META_BUTTON_FUNCTION_MENU))
	    return FALSE;

	space = &fgeom.menu_rect;
	break;
    case BUTTON_MIN:
	if (!meta_button_present (&button_layout,
				  META_BUTTON_FUNCTION_MINIMIZE))
	    return FALSE;

	space = &fgeom.min_rect;
	break;
    case BUTTON_MAX:
	if (!meta_button_present (&button_layout,
				  META_BUTTON_FUNCTION_MAXIMIZE))
	    return FALSE;

	space = &fgeom.max_rect;
	break;
    case BUTTON_CLOSE:
	if (!meta_button_present (&button_layout, META_BUTTON_FUNCTION_CLOSE))
	    return FALSE;

	space = &fgeom.close_rect;
	break;

#if defined (HAVE_METACITY_2_17_0) && defined (HAVE_LIBWNCK_2_18_1)
    case BUTTON_SHADE:
	if (!meta_button_present (&button_layout, META_BUTTON_FUNCTION_SHADE))
	    return FALSE;

	space = &fgeom.shade_rect;
	break;
    case BUTTON_ABOVE:
	if (!meta_button_present (&button_layout, META_BUTTON_FUNCTION_ABOVE))
	    return FALSE;

	space = &fgeom.above_rect;
	break;
    case BUTTON_STICK:
	if (!meta_button_present (&button_layout, META_BUTTON_FUNCTION_STICK))
	    return FALSE;

	space = &fgeom.stick_rect;
	break;
    case BUTTON_UNSHADE:
	if (!meta_button_present (&button_layout, META_BUTTON_FUNCTION_UNSHADE))
	    return FALSE;

	space = &fgeom.unshade_rect;
	break;
    case BUTTON_UNABOVE:
	if (!meta_button_present (&button_layout, META_BUTTON_FUNCTION_UNABOVE))
	    return FALSE;

	space = &fgeom.unabove_rect;
	break;
    case BUTTON_UNSTICK:
	if (!meta_button_present (&button_layout, META_BUTTON_FUNCTION_UNSTICK))
	    return FALSE;

	space = &fgeom.unstick_rect;
	break;
#endif

    default:
	return FALSE;
    }

#ifdef HAVE_METACITY_2_15_21
    if (!space->clickable.width && !space->clickable.height)
	return FALSE;

    *x = space->clickable.x;
    *y = space->clickable.y;
    *w = space->clickable.width;
    *h = space->clickable.height;
#else
    if (!space->width && !space->height)
	return FALSE;

    *x = space->x;
    *y = space->y;
    *w = space->width;
    *h = space->height;
#endif

    if (d->frame_window)
    {
	*x += d->frame->win_extents.left + 4;
	*y += d->frame->win_extents.top + 2;
    }
    else if (flags & META_FRAME_ALLOWS_HORIZONTAL_RESIZE)
    {
	*x += mutter_draggable_border_width;
    }

    return TRUE;
}

gfloat
meta_get_title_scale (decor_frame_t *frame)
{
    MetaTheme	   *theme = meta_theme_get_current ();
    MetaFrameType  type;
    MetaFrameFlags flags = 0xc33; /* fixme */

    type = meta_frame_type_from_string (frame->type);

    if (type == META_FRAME_TYPE_LAST)
	return 1.0f;

    gfloat scale = meta_theme_get_title_scale (theme, type, flags);

    return scale;
}

gboolean
meta_calc_decoration_size (decor_t *d,
			   gint    w,
			   gint    h,
			   gint    name_width,
			   gint    *width,
			   gint    *height)
{
    decor_layout_t  layout;
    decor_context_t *context;
    decor_shadow_t  *shadow;

    if (!d->decorated)
	return FALSE;

    if ((d->state & META_MAXIMIZED) == META_MAXIMIZED)
    {
	if (!d->frame_window)
	{
	    if (d->active)
	    {
		context = &d->frame->max_window_context_active;
		shadow  = d->frame->max_border_shadow_active;
	    }
	    else
	    {
		context = &d->frame->max_window_context_inactive;
		shadow  = d->frame->max_border_shadow_inactive;
	    }
	}
	else
	{
	    context = &d->frame->max_window_context_no_shadow;
	    shadow  = d->frame->max_border_no_shadow;
	}
    }
    else
    {
	if (!d->frame_window)
	{
	    if (d->active)
	    {
		context = &d->frame->window_context_active;
		shadow  = d->frame->border_shadow_active;
	    }
	    else
	    {
		context = &d->frame->window_context_inactive;
		shadow  = d->frame->border_shadow_inactive;
	    }
	}
	else
	{
	    context = &d->frame->window_context_no_shadow;
	    shadow  = d->frame->border_no_shadow;
	}
    }

    if (!d->frame_window)
    {
	decor_get_best_layout (context, w, h, &layout);

	if (context != d->context ||
	    memcmp (&layout, &d->border_layout, sizeof (layout)))
	{
	    *width  = layout.width;
	    *height = layout.height;

	    d->border_layout = layout;
	    d->context       = context;
	    d->shadow        = shadow;

	    meta_calc_button_size (d);

	    return TRUE;
	}
    }
    else
    {
	if ((d->state & META_MAXIMIZED) == META_MAXIMIZED)
	    decor_get_default_layout (context, d->client_width,
				      d->client_height - d->frame->titlebar_height,
				      &layout);
	else
	    decor_get_default_layout (context, d->client_width,
				      d->client_height, &layout);

	*width  = layout.width;
	*height = layout.height;

	d->border_layout = layout;
	d->shadow	 = shadow;
	d->context       = context;

	meta_calc_button_size (d);

	return TRUE;
    }

    return FALSE;
}

gboolean
meta_button_present (MetaButtonLayout   *button_layout,
		     MetaButtonFunction function)
{
    int i;

    for (i = 0; i < MAX_BUTTONS_PER_CORNER; i++)
	if (button_layout->left_buttons[i] == function)
	    return TRUE;

    for (i = 0; i < MAX_BUTTONS_PER_CORNER; i++)
	if (button_layout->right_buttons[i] == function)
	    return TRUE;

    return FALSE;
}

#define TOP_RESIZE_HEIGHT 2
#define RESIZE_EXTENDS 15

void
meta_get_event_window_position (decor_t *d,
				gint    i,
				gint    j,
				gint	width,
				gint	height,
				gint    *x,
				gint    *y,
				gint    *w,
				gint    *h)
{
    MetaButtonLayout  button_layout;
    MetaFrameGeometry fgeom;
    MetaFrameFlags    flags;
    MetaTheme	      *theme;
    GdkRectangle      clip;

    gint mutter_draggable_border_width = 0;

    g_object_get (settings, "draggable-border-width", &mutter_draggable_border_width, NULL);

    theme = meta_theme_get_current ();

    meta_get_decoration_geometry (d, theme, &flags, &fgeom, &button_layout,
				  meta_frame_type_from_string (d->frame->type),
				  &clip);

    width  += fgeom.right_width + fgeom.left_width;
    height += fgeom.top_height  + fgeom.bottom_height;

    switch (i) {
    case 2: /* bottom */
	switch (j) {
	case 2: /* bottom right */
	    if (d->frame_window)
	    {
		*x = width - fgeom.right_width - RESIZE_EXTENDS +
		     d->frame->win_extents.left + 2;
		*y = height - fgeom.bottom_height - RESIZE_EXTENDS +
		     d->frame->win_extents.top + 2;
	    }
	    else
	    {
		*x = width - fgeom.right_width - RESIZE_EXTENDS;
		*y = height - fgeom.bottom_height - RESIZE_EXTENDS;
	    }
	    *w = fgeom.right_width + RESIZE_EXTENDS;
	    *h = fgeom.bottom_height + RESIZE_EXTENDS;

	    if (!d->frame_window)
	    {
		*x += mutter_draggable_border_width;
		*w += mutter_draggable_border_width;
		*h += mutter_draggable_border_width;
	    }

	    break;
	case 1: /* bottom */
	    *x = fgeom.left_width + RESIZE_EXTENDS;
	    *y = height - fgeom.bottom_height;
	    if (d->frame_window)
		*y += d->frame->win_extents.top + 2;
	    *w = width - fgeom.left_width - fgeom.right_width -
		 (2 * RESIZE_EXTENDS);
	    *h = fgeom.bottom_height;

	    if (!d->frame_window)
	    {
		*x -= mutter_draggable_border_width;
		*h += mutter_draggable_border_width;
		*w += mutter_draggable_border_width * 2;
	    }

	    break;
	case 0: /* bottom left */
	default:
	    *x = 0;
	    *y = height - fgeom.bottom_height - RESIZE_EXTENDS;
	    if (d->frame_window)
	    {
		*x += d->frame->win_extents.left + 4;
		*y += d->frame->win_extents.bottom + 2;
	    }
	    *w = fgeom.left_width + RESIZE_EXTENDS;
	    *h = fgeom.bottom_height + RESIZE_EXTENDS;

	    if (!d->frame_window)
	    {
		*w += mutter_draggable_border_width;
		*h += mutter_draggable_border_width;
	    }

	    break;
	}
	break;
    case 1: /* middle */
	switch (j) {
	case 2: /* right */
	    *x = width - fgeom.right_width;
	    *y = fgeom.top_height + RESIZE_EXTENDS;
	    if (d->frame_window)
		*x += d->frame->win_extents.left + 2;
	    *w = fgeom.right_width;
	    *h = height - fgeom.top_height - fgeom.bottom_height -
		 (2 * RESIZE_EXTENDS);

	   if (!d->frame_window)
	   {
	       *x += mutter_draggable_border_width;
	       *w += mutter_draggable_border_width;
	       *h += mutter_draggable_border_width;
	   }


	    break;
	case 1: /* middle */
	    *x = fgeom.left_width;
	    *y = fgeom.title_rect.y + TOP_RESIZE_HEIGHT;
	    *w = width - fgeom.left_width - fgeom.right_width;
	    *h = height - fgeom.top_titlebar_edge - fgeom.bottom_height;

	    if (!d->frame_window)
	    {
		*x += mutter_draggable_border_width;
	    }

	    break;
	case 0: /* left */
	default:
	    *x = 0;
	    if (d->frame_window)
		*x += d->frame->win_extents.left + 4;
	    *y = fgeom.top_height + RESIZE_EXTENDS;
	    *w = fgeom.left_width;
	    *h = height - fgeom.top_height - fgeom.bottom_height -
		 (2 * RESIZE_EXTENDS);

	    if (!d->frame_window)
	    {
		*h += mutter_draggable_border_width;
		*w += mutter_draggable_border_width;
	    }


	    break;
	}
	break;
    case 0: /* top */
    default:
	switch (j) {
	case 2: /* top right */
	    *x = width - fgeom.right_width - RESIZE_EXTENDS;
	    *y = 0;
	    if (d->frame_window)
	    {
		*x += d->frame->win_extents.left + 2;
		*y += d->frame->win_extents.top + 2 - fgeom.title_rect.height;
	    }
	    *w = fgeom.right_width + RESIZE_EXTENDS;
	    *h = fgeom.top_height + RESIZE_EXTENDS;

	    if (!d->frame_window)
	    {
		*x += mutter_draggable_border_width;
		*w += mutter_draggable_border_width;
	    }
	    break;
	case 1: /* top */
	    *x = fgeom.left_width + RESIZE_EXTENDS;
	    *y = 0;
	    if (d->frame_window)
		*y += d->frame->win_extents.top + 2;
	    *w = width - fgeom.left_width - fgeom.right_width -
		 (2 * RESIZE_EXTENDS);
	    *h = fgeom.title_rect.y + TOP_RESIZE_HEIGHT;

	    if (!d->frame_window)
	    {
		*x -= mutter_draggable_border_width;
		*w += mutter_draggable_border_width * 2;
	    }

	    break;
	case 0: /* top left */
	default:
	    *x = 0;
	    *y = 0;
	    if (d->frame_window)
	    {
		*x += d->frame->win_extents.left + 4;
		*y += d->frame->win_extents.top + 2 - fgeom.title_rect.height;
	    }
	    *w = fgeom.left_width + RESIZE_EXTENDS;
	    *h = fgeom.top_height + RESIZE_EXTENDS;

	    if (!d->frame_window)
		*w += mutter_draggable_border_width;

	    break;
	}
    }

    if (!(flags & META_FRAME_ALLOWS_VERTICAL_RESIZE))
    {
	/* turn off top and bottom event windows */
	if (i == 0 || i == 2)
	    *w = *h = 0;
    }

    if (!(flags & META_FRAME_ALLOWS_HORIZONTAL_RESIZE))
    {
	/* turn off left and right event windows */
	if (j == 0 || j == 2)
	    *w = *h = 0;
    }
}


static MetaButtonFunction
meta_button_function_from_string (const char *str)
{
    if (strcmp (str, "menu") == 0)
	return META_BUTTON_FUNCTION_MENU;
    else if (strcmp (str, "minimize") == 0)
	return META_BUTTON_FUNCTION_MINIMIZE;
    else if (strcmp (str, "maximize") == 0)
	return META_BUTTON_FUNCTION_MAXIMIZE;
    else if (strcmp (str, "close") == 0)
	return META_BUTTON_FUNCTION_CLOSE;

#ifdef HAVE_METACITY_2_17_0
    else if (strcmp (str, "shade") == 0)
	return META_BUTTON_FUNCTION_SHADE;
    else if (strcmp (str, "above") == 0)
	return META_BUTTON_FUNCTION_ABOVE;
    else if (strcmp (str, "stick") == 0)
	return META_BUTTON_FUNCTION_STICK;
    else if (strcmp (str, "unshade") == 0)
	return META_BUTTON_FUNCTION_UNSHADE;
    else if (strcmp (str, "unabove") == 0)
	return META_BUTTON_FUNCTION_UNABOVE;
    else if (strcmp (str, "unstick") == 0)
	return META_BUTTON_FUNCTION_UNSTICK;
#endif

    else
	return META_BUTTON_FUNCTION_LAST;
}

static MetaButtonFunction
meta_button_opposite_function (MetaButtonFunction ofwhat)
{
    switch (ofwhat)
    {
#ifdef HAVE_METACITY_2_17_0
    case META_BUTTON_FUNCTION_SHADE:
	return META_BUTTON_FUNCTION_UNSHADE;
    case META_BUTTON_FUNCTION_UNSHADE:
	return META_BUTTON_FUNCTION_SHADE;

    case META_BUTTON_FUNCTION_ABOVE:
	return META_BUTTON_FUNCTION_UNABOVE;
    case META_BUTTON_FUNCTION_UNABOVE:
	return META_BUTTON_FUNCTION_ABOVE;

    case META_BUTTON_FUNCTION_STICK:
	return META_BUTTON_FUNCTION_UNSTICK;
    case META_BUTTON_FUNCTION_UNSTICK:
	return META_BUTTON_FUNCTION_STICK;
#endif

    default:
	return META_BUTTON_FUNCTION_LAST;
    }
}

static void
meta_initialize_button_layout (MetaButtonLayout *layout)
{
    int	i;

    for (i = 0; i < MAX_BUTTONS_PER_CORNER; i++)
    {
	layout->left_buttons[i] = META_BUTTON_FUNCTION_LAST;
	layout->right_buttons[i] = META_BUTTON_FUNCTION_LAST;
#ifdef HAVE_METACITY_2_23_2
	layout->left_buttons_has_spacer[i] = FALSE;
	layout->right_buttons_has_spacer[i] = FALSE;
#endif
    }
}

void
meta_update_button_layout (const char *value)
{
    MetaButtonLayout   new_layout;
    MetaButtonFunction f;
    char	       **sides;
    int		       i;

    meta_initialize_button_layout (&new_layout);

    sides = g_strsplit (value, ":", 2);

    if (sides[0] != NULL)
    {
	char	 **buttons;
	int	 b;
	gboolean used[META_BUTTON_FUNCTION_LAST];

	for (i = 0; i < META_BUTTON_FUNCTION_LAST; i++)
	   used[i] = FALSE;

	buttons = g_strsplit (sides[0], ",", -1);

	i = b = 0;
	while (buttons[b] != NULL)
	{
	    f = meta_button_function_from_string (buttons[b]);
#ifdef HAVE_METACITY_2_23_2
	    if (i > 0 && strcmp ("spacer", buttons[b]) == 0)
            {
	       new_layout.left_buttons_has_spacer[i - 1] = TRUE;
	       f = meta_button_opposite_function (f);

	       if (f != META_BUTTON_FUNCTION_LAST)
                  new_layout.left_buttons_has_spacer[i - 2] = TRUE;
            }
	    else
#endif
	    {
	       if (f != META_BUTTON_FUNCTION_LAST && !used[f])
	       {
                  used[f] = TRUE;
                  new_layout.left_buttons[i++] = f;

		  f = meta_button_opposite_function (f);

                  if (f != META_BUTTON_FUNCTION_LAST)
                      new_layout.left_buttons[i++] = f;

	       }
	       else
	       {
		  fprintf (stderr, "%s: Ignoring unknown or already-used "
			   "button name \"%s\"\n", program_name, buttons[b]);
	       }
	    }
	    b++;
	}

	new_layout.left_buttons[i] = META_BUTTON_FUNCTION_LAST;

	g_strfreev (buttons);

	if (sides[1] != NULL)
	{
	    for (i = 0; i < META_BUTTON_FUNCTION_LAST; i++)
		used[i] = FALSE;

	    buttons = g_strsplit (sides[1], ",", -1);

	    i = b = 0;
	    while (buttons[b] != NULL)
	    {
	       f = meta_button_function_from_string (buttons[b]);
#ifdef HAVE_METACITY_2_23_2
	       if (i > 0 && strcmp ("spacer", buttons[b]) == 0)
	       {
		  new_layout.right_buttons_has_spacer[i - 1] = TRUE;
		  f = meta_button_opposite_function (f);
		  if (f != META_BUTTON_FUNCTION_LAST)
		     new_layout.right_buttons_has_spacer[i - 2] = TRUE;
	       }
	       else
#endif
	       {
		   if (f != META_BUTTON_FUNCTION_LAST && !used[f])
		   {
		       used[f] = TRUE;
		       new_layout.right_buttons[i++] = f;

		       f = meta_button_opposite_function (f);

		       if (f != META_BUTTON_FUNCTION_LAST)
			   new_layout.right_buttons[i++] = f;
		   }
		   else
		   {
		       fprintf (stderr, "%s: Ignoring unknown or "
				"already-used button name \"%s\"\n",
				program_name, buttons[b]);
		   }
	       }
	       b++;
	    }
	    new_layout.right_buttons[i] = META_BUTTON_FUNCTION_LAST;

	    g_strfreev (buttons);
	}
    }

    g_strfreev (sides);

    /* Invert the button layout for RTL languages */
    if (gtk_widget_get_default_direction () == GTK_TEXT_DIR_RTL)
    {
	MetaButtonLayout rtl_layout;
	int j;

	meta_initialize_button_layout (&rtl_layout);

	i = 0;
	while (new_layout.left_buttons[i] != META_BUTTON_FUNCTION_LAST)
	    i++;

	for (j = 0; j < i; j++)
	{
	    rtl_layout.right_buttons[j] = new_layout.left_buttons[i - j - 1];
#ifdef HAVE_METACITY_2_23_2
	    if (j == 0)
		rtl_layout.right_buttons_has_spacer[i - 1] =
		    new_layout.left_buttons_has_spacer[i - j - 1];
	    else
		rtl_layout.right_buttons_has_spacer[j - 1] =
		    new_layout.left_buttons_has_spacer[i - j - 1];
#endif
	}

	i = 0;
	while (new_layout.right_buttons[i] != META_BUTTON_FUNCTION_LAST)
	    i++;

	for (j = 0; j < i; j++)
	{
	    rtl_layout.left_buttons[j] = new_layout.right_buttons[i - j - 1];
#ifdef HAVE_METACITY_2_23_2
	    if (j == 0)
		rtl_layout.left_buttons_has_spacer[i - 1] =
		    new_layout.right_buttons_has_spacer[i - j - 1];
	    else
		rtl_layout.left_buttons_has_spacer[j - 1] =
		    new_layout.right_buttons_has_spacer[i - j - 1];
#endif
	}

	new_layout = rtl_layout;
    }

    meta_button_layout = new_layout;
}

void
meta_update_border_extents (decor_frame_t *frame)
{
    MetaTheme *theme = meta_theme_get_current ();

    gwd_decor_frame_ref (frame);
    MetaFrameType frame_type = meta_frame_type_from_string (frame->type);
    gint          top_height, bottom_height, left_width, right_width;

    if (!(frame_type < META_FRAME_TYPE_LAST))
	frame_type = META_FRAME_TYPE_NORMAL;

    meta_theme_get_frame_borders (theme,
				  frame_type,
				  frame->text_height,
				  0,
				  &top_height,
				  &bottom_height,
				  &left_width,
				  &right_width);

    frame->win_extents.top    = frame->win_extents.top;
    frame->win_extents.bottom = bottom_height;
    frame->win_extents.left   = left_width;
    frame->win_extents.right  = right_width;

    frame->titlebar_height = top_height - frame->win_extents.top;

    meta_theme_get_frame_borders (theme,
				  frame_type,
				  frame->text_height,
				  META_FRAME_MAXIMIZED,
				  &top_height,
				  &bottom_height,
				  &left_width,
				  &right_width);

    frame->max_win_extents.top    = frame->win_extents.top;
    frame->max_win_extents.bottom = bottom_height;
    frame->max_win_extents.left   = left_width;
    frame->max_win_extents.right  = right_width;

    frame->max_titlebar_height = top_height - frame->max_win_extents.top;

    gwd_decor_frame_unref (frame);
}

#endif
