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

/* stolen from gtktooltip.c */

#define DEFAULT_DELAY 500           /* Default delay in ms */
#define STICKY_DELAY 0              /* Delay before popping up next tip
				     * if we're sticky
				     */
#define STICKY_REVERT_DELAY 1000    /* Delay before sticky tooltips revert
				     * to normal
				     */

static void
show_tooltip (const char *text)
{
    GdkDisplay     *gdkdisplay;
    GtkRequisition requisition;
    gint	   x, y, w, h;
    GdkScreen	   *screen;
    gint	   monitor_num;
    GdkRectangle   monitor;

    gdkdisplay = gdk_display_get_default ();

    gtk_label_set_text (GTK_LABEL (tip_label), text);

    gtk_widget_size_request (tip_window, &requisition);

    w = requisition.width;
    h = requisition.height;

    gdk_display_get_pointer (gdkdisplay, &screen, &x, &y, NULL);

    x -= (w / 2 + 4);

    monitor_num = gdk_screen_get_monitor_at_point (screen, x, y);
    gdk_screen_get_monitor_geometry (screen, monitor_num, &monitor);

    if ((x + w) > monitor.x + monitor.width)
	x -= (x + w) - (monitor.x + monitor.width);
    else if (x < monitor.x)
	x = monitor.x;

    if ((y + h + 16) > monitor.y + monitor.height)
	y = y - h - 16;
    else
	y = y + 16;

    gtk_window_move (GTK_WINDOW (tip_window), x, y);
    gtk_widget_show (tip_window);
}

static void
hide_tooltip (void)
{
    if (gtk_widget_get_visible (tip_window))
	g_get_current_time (&tooltip_last_popdown);

    gtk_widget_hide (tip_window);

    if (tooltip_timer_tag)
    {
	g_source_remove (tooltip_timer_tag);
	tooltip_timer_tag = 0;
    }
}

static gboolean
tooltip_recently_shown (void)
{
    GTimeVal now;
    glong    msec;

    g_get_current_time (&now);

    msec = now.tv_sec - tooltip_last_popdown.tv_sec;
    if (msec > STICKY_REVERT_DELAY / 1000)
	return FALSE;

    msec = msec * 1000 + (now.tv_usec - tooltip_last_popdown.tv_usec) / 1000;

    return (msec < STICKY_REVERT_DELAY);
}

static gint
tooltip_timeout (gpointer data)
{
    tooltip_timer_tag = 0;

    show_tooltip ((const char *) data);

    return FALSE;
}

static void
tooltip_start_delay (const char *text)
{
    guint delay = DEFAULT_DELAY;

    if (tooltip_timer_tag)
	return;

    if (tooltip_recently_shown ())
	delay = STICKY_DELAY;

    tooltip_timer_tag = g_timeout_add (delay,
				       tooltip_timeout,
				       (gpointer) text);
}

static gint
tooltip_paint_window (GtkWidget *tooltip)
{
    GtkRequisition req;

    gtk_widget_size_request (tip_window, &req);
    gtk_paint_flat_box (tip_window->style, tip_window->window,
			GTK_STATE_NORMAL, GTK_SHADOW_OUT,
			NULL, GTK_WIDGET (tip_window), "tooltip",
			0, 0, req.width, req.height);

    return FALSE;
}

gboolean
create_tooltip_window (void)
{
    tip_window = gtk_window_new (GTK_WINDOW_POPUP);

    gtk_widget_set_app_paintable (tip_window, TRUE);
    gtk_window_set_resizable (GTK_WINDOW (tip_window), FALSE);
    gtk_widget_set_name (tip_window, "gtk-tooltips");
    gtk_container_set_border_width (GTK_CONTAINER (tip_window), 4);

#if GTK_CHECK_VERSION (2, 10, 0)
    if (!gtk_check_version (2, 10, 0))
	gtk_window_set_type_hint (GTK_WINDOW (tip_window),
				  GDK_WINDOW_TYPE_HINT_TOOLTIP);
#endif

    g_signal_connect_swapped (tip_window,
			      "expose_event",
			      G_CALLBACK (tooltip_paint_window),
			      0);

    tip_label = gtk_label_new (NULL);
    gtk_label_set_line_wrap (GTK_LABEL (tip_label), TRUE);
    gtk_misc_set_alignment (GTK_MISC (tip_label), 0.5, 0.5);
    gtk_widget_show (tip_label);

    gtk_container_add (GTK_CONTAINER (tip_window), tip_label);

    gtk_widget_ensure_style (tip_window);

    return TRUE;
}

void
handle_tooltip_event (WnckWindow *win,
		      decor_event *gtkwd_event,
		      decor_event_type   gtkwd_type,
		      guint	 state,
		      const char *tip)
{
    switch (gtkwd_type) {
    case GButtonPress:
	hide_tooltip ();
	break;
    case GButtonRelease:
	break;
    case GEnterNotify:
	if (!(state & PRESSED_EVENT_WINDOW))
	{
	    if (wnck_window_is_active (win))
		tooltip_start_delay (tip);
	}
	break;
    case GLeaveNotify:
	hide_tooltip ();
	break;
    default:
	break;
    }
}
