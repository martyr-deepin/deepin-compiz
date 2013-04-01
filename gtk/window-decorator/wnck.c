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

const gchar *
get_frame_type (WnckWindow *win)
{
    WnckWindowType wnck_type;

    if (win == NULL)
	return "bare";

    wnck_type  = wnck_window_get_window_type (win);

    switch (wnck_type)
    {
	case WNCK_WINDOW_NORMAL:
	    return "normal";
	case WNCK_WINDOW_DIALOG:
	{
	    Atom	  actual;
	    int		  result, format;
	    unsigned long n, left;
	    unsigned char *data;
	    Window        xid = wnck_window_get_xid (win);

	    if (xid == None)
		return "bare";

	    gdk_error_trap_push ();
	    result = XGetWindowProperty (gdk_x11_get_default_xdisplay (), xid,
					 net_wm_state_atom,
					 0L, 1024L, FALSE, XA_ATOM, &actual, &format,
					 &n, &left, &data);
	    gdk_flush ();
	    gdk_error_trap_pop ();

	    if (result == Success && data)
	    {
		Atom *a = (Atom *) data;

		while (n--)
		    if (*a++ == net_wm_state_modal_atom)
		    {
			XFree ((void *) data);
			return "modal_dialog";
		    }


	    }

	    return "dialog";
	}
	case WNCK_WINDOW_MENU:
	    return "menu";
	case WNCK_WINDOW_UTILITY:
	    return "utility";
	default:
	    return "bare";
    }

    return "normal";
}

static void
window_name_changed (WnckWindow *win)
{
    decor_t *d = g_object_get_data (G_OBJECT (win), "decor");

    if (d->decorated)
    {
	if (!request_update_window_decoration_size (win))
	    queue_decor_draw (d);
    }
}

static void
window_geometry_changed (WnckWindow *win)
{
    decor_t *d = g_object_get_data (G_OBJECT (win), "decor");

    if (d->decorated)
    {
	int width, height;

	wnck_window_get_client_window_geometry (win, NULL, NULL,
						&width, &height);

	if (width != d->client_width || height != d->client_height)
	{
	    d->client_width  = width;
	    d->client_height = height;

	    request_update_window_decoration_size (win);
	    update_event_windows (win);
	}
    }
}

static void
window_icon_changed (WnckWindow *win)
{
    decor_t *d = g_object_get_data (G_OBJECT (win), "decor");

    if (d->decorated)
    {
	update_window_decoration_icon (win);
	queue_decor_draw (d);
    }
}

static void
window_state_changed (WnckWindow *win)
{
    decor_t *d = g_object_get_data (G_OBJECT (win), "decor");

    if (d->decorated)
    {
	update_window_decoration_state (win);
	if (!request_update_window_decoration_size (win))
	    queue_decor_draw (d);

	update_event_windows (win);
    }
}

static void
window_actions_changed (WnckWindow *win)
{
    decor_t *d = g_object_get_data (G_OBJECT (win), "decor");

    if (d->decorated)
    {
	update_window_decoration_actions (win);
	if (!request_update_window_decoration_size (win))
	    queue_decor_draw (d);

	update_event_windows (win);
    }
}

void
update_frames_border_extents (gpointer key,
			      gpointer value,
			      gpointer user_data)
{
    decor_frame_t *frame = (decor_frame_t *) value;

    (*theme_update_border_extents) (frame);
}

void
decorations_changed (WnckScreen *screen)
{
    GdkDisplay *gdkdisplay;
    GdkScreen  *gdkscreen;
    GList      *windows;
    Window     select;

    gdkdisplay = gdk_display_get_default ();
    gdkscreen  = gdk_display_get_default_screen (gdkdisplay);

    const gchar *titlebar_font = NULL;
    g_object_get (settings, "titlebar-font", &titlebar_font, NULL);

    gwd_frames_foreach (set_frames_scales, (gpointer) titlebar_font);

    update_titlebar_font ();
    gwd_process_frames (update_frames_border_extents,
                        window_type_frames,
			WINDOW_TYPE_FRAMES_NUM,
			NULL);
    update_shadow ();

    update_default_decorations (gdkscreen);

    if (minimal)
	return;

    /* Update all normal windows */

    windows = wnck_screen_get_windows (screen);
    while (windows != NULL)
    {
	decor_t *d = g_object_get_data (G_OBJECT (windows->data), "decor");

	if (d->decorated)
	{

#ifdef USE_METACITY
	    if (d->draw == draw_window_decoration ||
		d->draw == meta_draw_window_decoration)
		d->draw = theme_draw_window_decoration;
#endif

	}

	update_window_decoration (WNCK_WINDOW (windows->data));
	windows = windows->next;
    }

    /* Update switcher window */

    if (switcher_window &&
	get_window_prop (switcher_window->prop_xid,
			 select_window_atom, &select))
    {
	decor_t *d = switcher_window;
	/* force size update */
	d->context = NULL;
	d->width = d->height = 0;

	update_switcher_window (d->prop_xid, select);
    }
}

void
restack_window (WnckWindow *win,
		int	   stack_mode)
{
    Display    *xdisplay;
    GdkDisplay *gdkdisplay;
    GdkScreen  *screen;
    Window     xroot;
    XEvent     ev;

    gdkdisplay = gdk_display_get_default ();
    xdisplay   = GDK_DISPLAY_XDISPLAY (gdkdisplay);
    screen     = gdk_display_get_default_screen (gdkdisplay);
    xroot      = RootWindowOfScreen (gdk_x11_screen_get_xscreen (screen));

    if (action_menu_mapped)
    {
	gtk_object_destroy (GTK_OBJECT (action_menu));
	action_menu_mapped = FALSE;
	action_menu = NULL;
	return;
    }

    ev.xclient.type    = ClientMessage;
    ev.xclient.display = xdisplay;

    ev.xclient.serial	  = 0;
    ev.xclient.send_event = TRUE;

    ev.xclient.window	    = wnck_window_get_xid (win);
    ev.xclient.message_type = restack_window_atom;
    ev.xclient.format	    = 32;

    ev.xclient.data.l[0] = 2;
    ev.xclient.data.l[1] = None;
    ev.xclient.data.l[2] = stack_mode;
    ev.xclient.data.l[3] = 0;
    ev.xclient.data.l[4] = 0;

    XSendEvent (xdisplay, xroot, FALSE,
		SubstructureRedirectMask | SubstructureNotifyMask,
		&ev);

    XSync (xdisplay, FALSE);
}


void
add_frame_window (WnckWindow *win,
		  Window     frame,
		  Bool	     mode)
{
    Display		 *xdisplay;
    XSetWindowAttributes attr;
    gulong		 xid = wnck_window_get_xid (win);
    decor_t		 *d = g_object_get_data (G_OBJECT (win), "decor");
    gint		 i, j;

    xdisplay = GDK_DISPLAY_XDISPLAY (gdk_display_get_default ());

    /* If we have already done this, there is no need to do it again, except
     * if the property changed.
     *
     * The reason this check is here is because sometimes the PropertyNotify X
     * event can come a bit after the property on the window is actually set
     * which might result in this function being called twice - once by
     * wnck through window_opened and once through our X event handler
     * event_filter_func
     */

    if (d->created && mode && d->frame_window)
	return;

    d->active = wnck_window_is_active (win);
    d->win = win;
    d->frame = gwd_get_decor_frame (get_frame_type (win));
    d->last_pos_entered = NULL;

    attr.event_mask = ButtonPressMask | EnterWindowMask |
		      LeaveWindowMask | ExposureMask;
    attr.override_redirect = TRUE;

    gdk_error_trap_push ();

    if (mode)
    {
	GdkColormap *colormap;

	d->frame_window = create_gdk_window (frame);
	d->decor_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	colormap = get_colormap_for_drawable (GDK_DRAWABLE (d->frame_window));

	d->decor_image = gtk_image_new ();

	gtk_widget_set_colormap (d->decor_window, colormap);
	gtk_widget_set_colormap (d->decor_image, colormap);

	d->decor_event_box = gtk_event_box_new ();
	gtk_event_box_set_visible_window (GTK_EVENT_BOX (d->decor_event_box),
					  FALSE);
	gtk_widget_set_events (d->decor_event_box, GDK_BUTTON_PRESS_MASK |
						   GDK_BUTTON_RELEASE_MASK |
						   GDK_POINTER_MOTION_MASK);

	g_signal_connect (G_OBJECT (d->decor_event_box), "button-press-event",
			  G_CALLBACK (frame_handle_button_press),
			  (gpointer) (d));

	g_signal_connect (G_OBJECT (d->decor_event_box), "button-release-event",
			  G_CALLBACK (frame_handle_button_release),
			  (gpointer) (d));

	g_signal_connect (G_OBJECT (d->decor_event_box), "motion-notify-event",
			  G_CALLBACK (frame_handle_motion),
			  (gpointer) (d));

	gtk_container_add (GTK_CONTAINER (d->decor_event_box), d->decor_image);
	gtk_event_box_set_above_child (GTK_EVENT_BOX (d->decor_event_box), TRUE);
	gtk_widget_show_all (d->decor_event_box);
	gtk_window_set_decorated (GTK_WINDOW (d->decor_window), FALSE);
	gtk_window_set_default_size (GTK_WINDOW (d->decor_window), 1000, 1000);
	gtk_container_add (GTK_CONTAINER (d->decor_window), d->decor_event_box);

	/* Assumed realization happens here */

	g_signal_connect (G_OBJECT (d->decor_window), "realize",
			  G_CALLBACK (frame_window_realized), (gpointer) d);

	gtk_widget_show_all (d->decor_window);
	gtk_widget_show (d->decor_window);

	g_object_set_data (G_OBJECT (d->frame_window),
			   "client_wnck_window", win);
    }
    else
    {
	d->frame_window = NULL;

	for (i = 0; i < 3; i++)
	{
	    for (j = 0; j < 3; j++)
	    {
		d->event_windows[i][j].window =
		XCreateWindow (xdisplay,
			       frame,
			       0, 0, 1, 1, 0,
			       CopyFromParent, CopyFromParent, CopyFromParent,
			       CWOverrideRedirect | CWEventMask, &attr);

		if (cursor[i][j].cursor)
		    XDefineCursor (xdisplay, d->event_windows[i][j].window,
		    cursor[i][j].cursor);
	    }
	}

	attr.event_mask |= ButtonReleaseMask;

	for (i = 0; i < BUTTON_NUM; i++)
	{
	    d->button_windows[i].window =
	    XCreateWindow (xdisplay,
			   frame,
			   0, 0, 1, 1, 0,
			   CopyFromParent, CopyFromParent, CopyFromParent,
			   CWOverrideRedirect | CWEventMask, &attr);

	    d->button_states[i] = 0;
	}
    }

    gdk_display_sync (gdk_display_get_default ());
    if (!gdk_error_trap_pop ())
    {
	if (get_mwm_prop (xid) & (MWM_DECOR_ALL | MWM_DECOR_TITLE))
	    d->decorated = TRUE;

	for (i = 0; i < 3; i++)
	    for (j = 0; j < 3; j++)
	    {
		Window win = d->event_windows[i][j].window;
		g_hash_table_insert (frame_table,
				     GINT_TO_POINTER (win),
				     GINT_TO_POINTER (xid));
	    }

	for (i = 0; i < BUTTON_NUM; i++)
	    g_hash_table_insert (frame_table,
				 GINT_TO_POINTER (d->button_windows[i].window),
				 GINT_TO_POINTER (xid));

	if (d->frame_window)
	{
	    g_hash_table_insert (frame_table,
				 GINT_TO_POINTER (frame),
				 GINT_TO_POINTER (xid));
	}

	if (d->decorated)
	{
	    update_window_decoration_state (win);
	    update_window_decoration_actions (win);
	    update_window_decoration_icon (win);
	    request_update_window_decoration_size (win);

	    update_event_windows (win);
	}
    }
    else
    {
	for (i = 0; i < 3; i++)
	    for (j = 0; j < 3; j++)
		d->event_windows[i][j].window = None;
    }

    d->created = TRUE;
}

gboolean
clean_destroyed_pixmaps (gpointer key, gpointer value, gpointer d)
{
    return value == d;
}

void
remove_frame_window (WnckWindow *win)
{
    decor_t *d = g_object_get_data (G_OBJECT (win), "decor");
    Display *xdisplay;

    g_hash_table_foreach_remove (destroyed_pixmaps_table,
                                 clean_destroyed_pixmaps,
                                 d);

    xdisplay = GDK_DISPLAY_XDISPLAY (gdk_display_get_default ());

    if (!d->frame_window)
    {
	int i, j;

	for (i = 0; i < 3; i++)
	{
	    for (j = 0; j < 3; j++)
	    {
		XDestroyWindow (xdisplay, d->event_windows[i][j].window);
		d->event_windows[i][j].window = None;
	    }
	}

	for (i = 0; i < BUTTON_NUM; i++)
	{
	    XDestroyWindow (xdisplay, d->button_windows[i].window);
	    d->button_windows[i].window = None;

	    d->button_states[i] = 0;
	}
    }

    if (d->pixmap)
    {
	g_object_unref (G_OBJECT (d->pixmap));
	d->pixmap = NULL;
    }

    if (d->buffer_pixmap)
    {
	g_object_unref (G_OBJECT (d->buffer_pixmap));
	d->buffer_pixmap = NULL;
    }

    if (d->old_pixmaps)
    {
	g_hash_table_destroy (d->old_pixmaps);
	d->old_pixmaps = NULL;
    }

    if (d->cr)
    {
	cairo_destroy (d->cr);
	d->cr = NULL;
    }

    if (d->picture && !d->frame_window)
    {
	XRenderFreePicture (xdisplay, d->picture);
	d->picture = 0;
    }

    if (d->name)
    {
	g_free (d->name);
	d->name = NULL;
    }

    if (d->layout)
    {
	g_object_unref (G_OBJECT (d->layout));
	d->layout = NULL;
    }

    if (d->icon)
    {
	cairo_pattern_destroy (d->icon);
	d->icon = NULL;
    }

    if (d->icon_pixmap)
    {
	g_object_unref (G_OBJECT (d->icon_pixmap));
	d->icon_pixmap = NULL;
    }

    if (d->icon_pixbuf)
    {
	g_object_unref (G_OBJECT (d->icon_pixbuf));
	d->icon_pixbuf = NULL;
    }

    if (d->force_quit_dialog)
    {
	GtkWidget *dialog = d->force_quit_dialog;

	d->force_quit_dialog = NULL;
	gtk_widget_destroy (dialog);
    }

    if (d->frame_window)
    {
	gdk_window_destroy (d->frame_window);
	d->frame_window = NULL;
    }

    if (d->decor_image)
    {
	g_object_unref (d->decor_image);
	d->decor_image = NULL;
    }

    if (d->decor_event_box)
    {
	g_object_unref (d->decor_event_box);
	d->decor_event_box = NULL;
    }

    if (d->decor_window)
    {
	g_object_unref (d->decor_window);
	d->decor_window = NULL;
    }

    if (d->frame)
    {
	gwd_decor_frame_unref (d->frame);
	d->frame = NULL;
    }

    gdk_error_trap_push ();
    XDeleteProperty (xdisplay, wnck_window_get_xid (win), win_decor_atom);
    gdk_display_sync (gdk_display_get_default ());
    gdk_error_trap_pop ();

    d->width  = 0;
    d->height = 0;

    d->decorated = FALSE;

    d->state   = 0;
    d->actions = 0;

    d->context = NULL;
    d->shadow  = NULL;

    draw_list = g_slist_remove (draw_list, d);
}

void
connect_window (WnckWindow *win)
{
    g_signal_connect_object (win, "name_changed",
			     G_CALLBACK (window_name_changed),
			     0, 0);
    g_signal_connect_object (win, "geometry_changed",
			     G_CALLBACK (window_geometry_changed),
			     0, 0);
    g_signal_connect_object (win, "icon_changed",
			     G_CALLBACK (window_icon_changed),
			     0, 0);
    g_signal_connect_object (win, "state_changed",
			     G_CALLBACK (window_state_changed),
			     0, 0);
    g_signal_connect_object (win, "actions_changed",
			     G_CALLBACK (window_actions_changed),
			     0, 0);
}

static void
set_context_if_decorated (decor_t *d, decor_context_t *context)
{
    if (d->decorated)
	d->context = context;
}

static void
active_window_changed (WnckScreen *screen)
{
    WnckWindow *win;
    decor_t    *d;

    win = wnck_screen_get_previously_active_window (screen);
    if (win)
    {
	d = g_object_get_data (G_OBJECT (win), "decor");
	if (d)
	{
	    d->active = wnck_window_is_active (win);

            decor_frame_t *frame = d->decorated ? d->frame : gwd_get_decor_frame (get_frame_type (win));

	    if ((d->state & META_MAXIMIZED) == META_MAXIMIZED)
	    {
		if (!d->frame_window)
		{
		   if (d->active)
		   {
		       set_context_if_decorated (d, &frame->max_window_context_active);
		       d->shadow  = frame->max_border_shadow_active;
		   }
		   else
		   {
		       set_context_if_decorated (d, &frame->max_window_context_inactive);
		       d->shadow  = frame->max_border_shadow_inactive;
		   }
                }
                else
                {
		   d->shadow  = d->frame->max_border_no_shadow;
                }
	    }
	    else
	    {
	       if (!d->frame_window)
	       {
		   if (d->active)
		   {
		       set_context_if_decorated (d, &frame->window_context_active);
		       d->shadow  = frame->border_shadow_active;
		   }
		   else
		   {
		       set_context_if_decorated (d, &frame->window_context_inactive);
		       d->shadow  = frame->border_shadow_inactive;
		   }
	       }
	       else
	       {
		   d->shadow  = d->frame->border_no_shadow;
	       }
	    }

            if (!d->decorated)
                gwd_decor_frame_unref (frame);

	    /* We need to update the decoration size here
	    * since the shadow size might have changed and
	    * in that case the decoration will be redrawn,
	    * however if the shadow size doesn't change
	    * then we need to redraw the decoration anyways
	    * since the image would have changed */
	    if (d->win != NULL &&
		!request_update_window_decoration_size (d->win) &&
		d->decorated &&
		d->pixmap)
		queue_decor_draw (d);

	}
    }

    win = wnck_screen_get_active_window (screen);
    if (win)
    {
	d = g_object_get_data (G_OBJECT (win), "decor");
	if (d)
	{
	    d->active = wnck_window_is_active (win);

            decor_frame_t *frame = d->decorated ? d->frame : gwd_get_decor_frame (get_frame_type (win));

	    if ((d->state & META_MAXIMIZED) == META_MAXIMIZED)
	    {
		if (!d->frame_window)
		{
		   if (d->active)
		   {
		       set_context_if_decorated (d, &frame->max_window_context_active);
		       d->shadow  = frame->max_border_shadow_active;
		   }
		   else
		   {
		       set_context_if_decorated (d, &frame->max_window_context_inactive);
		       d->shadow  = frame->max_border_shadow_inactive;
		   }
		}
		else
		{
		   d->shadow  = frame->max_border_no_shadow;
		}
	    }
	    else
	    {
		if (!d->frame_window)
		{
		   if (d->active)
		   {
		       set_context_if_decorated (d, &frame->window_context_active);
		       d->shadow  = frame->border_shadow_active;
		   }
		   else
		   {
		       set_context_if_decorated (d, &frame->window_context_inactive);
		       d->shadow  = frame->border_shadow_inactive;
		   }
		}
		else
		{
		   d->shadow =  frame->border_no_shadow;
		}
	    }

            if (!d->decorated)
                gwd_decor_frame_unref (frame);

	    /* We need to update the decoration size here
	    * since the shadow size might have changed and
	    * in that case the decoration will be redrawn,
	    * however if the shadow size doesn't change
	    * then we need to redraw the decoration anyways
	    * since the image would have changed */
	    if (d->win != NULL &&
		!request_update_window_decoration_size (d->win) &&
		d->decorated &&
		d->pixmap)
		queue_decor_draw (d);

	}
    }
}

void
window_opened (WnckScreen *screen,
	       WnckWindow *win)
{
    decor_t      *d;
    Window       window;
    gulong       xid;
    unsigned int i, j;

    static event_callback callback[3][3] = {
	{ top_left_event,    top_event,    top_right_event    },
	{ left_event,	     title_event,  right_event	      },
	{ bottom_left_event, bottom_event, bottom_right_event }
    };
    static event_callback button_callback[BUTTON_NUM] = {
	close_button_event,
	max_button_event,
	min_button_event,
	menu_button_event,
	shade_button_event,
	above_button_event,
	stick_button_event,
	unshade_button_event,
	unabove_button_event,
	unstick_button_event
    };

    d = calloc (1, sizeof (decor_t));
    if (!d)
	return;

    for (i = 0; i < 3; i++)
	for (j = 0; j < 3; j++)
	    d->event_windows[i][j].callback = callback[i][j];

    for (i = 0; i < BUTTON_NUM; i++)
	d->button_windows[i].callback = button_callback[i];

    wnck_window_get_client_window_geometry (win, NULL, NULL,
					    &d->client_width,
					    &d->client_height);

    d->draw = theme_draw_window_decoration;

    d->created = FALSE;
    d->pixmap = NULL;
    d->cr = NULL;
    d->buffer_pixmap = NULL;
    d->picture = None;

    connect_window (win);

    g_object_set_data (G_OBJECT (win), "decor", d);

    xid = wnck_window_get_xid (win);

    if (get_window_prop (xid, frame_input_window_atom, &window))
    {
	add_frame_window (win, window, FALSE);
    }
    else if (get_window_prop (xid, frame_output_window_atom, &window))
    {
	add_frame_window (win, window, TRUE);
    }
}

void
window_closed (WnckScreen *screen,
	       WnckWindow *win)
{
    decor_t *d = g_object_get_data (G_OBJECT (win), "decor");

    if (d)
    {
	remove_frame_window (win);
	g_object_set_data (G_OBJECT (win), "decor", NULL);
	g_free (d);
    }
}

void
connect_screen (WnckScreen *screen)
{
    GList *windows;

    g_signal_connect_object (G_OBJECT (screen), "active_window_changed",
                             G_CALLBACK (active_window_changed),
                             0, 0);
    g_signal_connect_object (G_OBJECT (screen), "window_opened",
                             G_CALLBACK (window_opened),
                             0, 0);
    g_signal_connect_object (G_OBJECT (screen), "window_closed",
                             G_CALLBACK (window_closed),
                             0, 0);

    windows = wnck_screen_get_windows (screen);
    while (windows != NULL)
    {
        window_opened (screen, windows->data);
        windows = windows->next;
    }
}
