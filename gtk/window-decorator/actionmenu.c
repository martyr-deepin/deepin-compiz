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

static void
action_menu_unmap (GObject *object)
{
    action_menu_mapped = FALSE;
}

static void
position_action_menu (GtkMenu  *menu,
		      gint     *x,
		      gint     *y,
		      gboolean *push_in,
		      gpointer user_data)
{
    WnckWindow *win = (WnckWindow *) user_data;
    decor_frame_t  *frame = gwd_get_decor_frame (get_frame_type (win));
    decor_t    *d = g_object_get_data (G_OBJECT (win), "decor");
    gint       bx, by, width, height;

    wnck_window_get_client_window_geometry (win, x, y, &width, &height);

    if ((*theme_get_button_position) (d, BUTTON_MENU, width, height,
				      &bx, &by, &width, &height))
	*x = *x - frame->win_extents.left + bx;

    gwd_decor_frame_unref (frame);

    if (gtk_widget_get_default_direction () == GTK_TEXT_DIR_RTL)
    {
	GtkRequisition req;

	gtk_widget_size_request (GTK_WIDGET (menu), &req);
	*x = MAX (0, *x - req.width + width);
    }

    *push_in = TRUE;
}

void
action_menu_map (WnckWindow *win,
		 long	     button,
		 Time	     time)
{
    GdkDisplay *gdkdisplay;
    GdkScreen  *screen;

    gdkdisplay = gdk_display_get_default ();
    screen     = gdk_display_get_default_screen (gdkdisplay);

    if (action_menu)
    {
	if (action_menu_mapped)
	{
	    gtk_widget_destroy (action_menu);
	    action_menu_mapped = FALSE;
	    action_menu = NULL;
	    return;
	}
	else
	    gtk_widget_destroy (action_menu);
    }

    switch (wnck_window_get_window_type (win)) {
    case WNCK_WINDOW_DESKTOP:
    case WNCK_WINDOW_DOCK:
	/* don't allow window action */
	return;
    case WNCK_WINDOW_NORMAL:
    case WNCK_WINDOW_DIALOG:

#ifndef HAVE_LIBWNCK_2_19_4
    case WNCK_WINDOW_MODAL_DIALOG:
#endif

    case WNCK_WINDOW_TOOLBAR:
    case WNCK_WINDOW_MENU:
    case WNCK_WINDOW_UTILITY:
    case WNCK_WINDOW_SPLASHSCREEN:
	/* allow window action menu */
	break;
    }

    action_menu = wnck_create_window_action_menu (win);

    gtk_menu_set_screen (GTK_MENU (action_menu), screen);

    g_signal_connect_object (G_OBJECT (action_menu), "unmap",
			     G_CALLBACK (action_menu_unmap),
			     0, 0);

    gtk_widget_show (action_menu);

    if (!button || button == 1)
    {
	gtk_menu_popup (GTK_MENU (action_menu),
			NULL, NULL,
			position_action_menu, (gpointer) win,
			button,
			time);
    }
    else
    {
	gtk_menu_popup (GTK_MENU (action_menu),
			NULL, NULL,
			NULL, NULL,
			button,
			time);
    }

    action_menu_mapped = TRUE;
}
