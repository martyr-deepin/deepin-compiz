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

static char *
get_client_machine (Window xwindow)
{
    Atom   atom, type;
    gulong nitems, bytes_after;
    guchar *str = NULL;
    int    format, result;
    char   *retval;

    atom = XInternAtom (GDK_DISPLAY_XDISPLAY (gdk_display_get_default ()), "WM_CLIENT_MACHINE", FALSE);

    gdk_error_trap_push ();

    result = XGetWindowProperty (GDK_DISPLAY_XDISPLAY (gdk_display_get_default ()),
				 xwindow, atom,
				 0, G_MAXLONG,
				 FALSE, XA_STRING, &type, &format, &nitems,
				 &bytes_after, &str);

    gdk_error_trap_pop ();

    if (result != Success)
	return NULL;

    if (type != XA_STRING)
    {
	XFree (str);
	return NULL;
    }

    retval = g_strdup ((gchar *) str);

    XFree (str);

    return retval;
}

static void
kill_window (WnckWindow *win)
{
    WnckApplication *app;

    app = wnck_window_get_application (win);
    if (app)
    {
	gchar buf[257], *client_machine;
	int   pid;

	pid = wnck_application_get_pid (app);
	client_machine = get_client_machine (wnck_application_get_xid (app));

	if (client_machine && pid > 0)
	{
	    if (gethostname (buf, sizeof (buf) - 1) == 0)
	    {
		if (strcmp (buf, client_machine) == 0)
		    kill (pid, 9);
	    }
	}

	if (client_machine)
	    g_free (client_machine);
    }

    gdk_error_trap_push ();
    XKillClient (GDK_DISPLAY_XDISPLAY (gdk_display_get_default ()), wnck_window_get_xid (win));
    gdk_display_sync (gdk_display_get_default ());
    gdk_error_trap_pop ();
}

static void
force_quit_dialog_realize (GtkWidget *dialog,
			   void      *data)
{
    WnckWindow *win = data;

    gdk_error_trap_push ();
    XSetTransientForHint (GDK_DISPLAY_XDISPLAY (gdk_display_get_default ()),
			  GDK_WINDOW_XID (dialog->window),
			  wnck_window_get_xid (win));
    gdk_display_sync (gdk_display_get_default ());
    gdk_error_trap_pop ();
}

static void
force_quit_dialog_response (GtkWidget *dialog,
			    gint      response,
			    void      *data)
{
    WnckWindow *win = data;
    decor_t    *d = g_object_get_data (G_OBJECT (win), "decor");

    if (response == GTK_RESPONSE_ACCEPT)
	kill_window (win);

    if (d->force_quit_dialog)
    {
	d->force_quit_dialog = NULL;
	gtk_widget_destroy (dialog);
    }
}

void
show_force_quit_dialog (WnckWindow *win,
			Time        timestamp)
{
    decor_t   *d = g_object_get_data (G_OBJECT (win), "decor");
    GtkWidget *dialog;
    gchar     *str, *tmp;

    if (d->force_quit_dialog)
	return;

    tmp = g_markup_escape_text (wnck_window_get_name (win), -1);
    str = g_strdup_printf (_("The window \"%s\" is not responding."), tmp);

    g_free (tmp);

    dialog = gtk_message_dialog_new (NULL, 0,
				     GTK_MESSAGE_WARNING,
				     GTK_BUTTONS_NONE,
				     "<b>%s</b>\n\n%s",
				     str,
				     _("Forcing this application to "
				     "quit will cause you to lose any "
				     "unsaved changes."));
    g_free (str);

    gtk_window_set_icon_name (GTK_WINDOW (dialog), "force-quit");

    gtk_label_set_use_markup (GTK_LABEL (GTK_MESSAGE_DIALOG (dialog)->label),
			      TRUE);
    gtk_label_set_line_wrap (GTK_LABEL (GTK_MESSAGE_DIALOG (dialog)->label),
			     TRUE);

    gtk_dialog_add_buttons (GTK_DIALOG (dialog),
			    GTK_STOCK_CANCEL,
			    GTK_RESPONSE_REJECT,
			    _("_Force Quit"),
			    GTK_RESPONSE_ACCEPT,
			    NULL);

    gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_REJECT);

    g_signal_connect (G_OBJECT (dialog), "realize",
		      G_CALLBACK (force_quit_dialog_realize),
		      win);

    g_signal_connect (G_OBJECT (dialog), "response",
		      G_CALLBACK (force_quit_dialog_response),
		      win);

    gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

    gtk_widget_realize (dialog);

    gdk_x11_window_set_user_time (dialog->window, timestamp);

    gtk_widget_show (dialog);

    d->force_quit_dialog = dialog;
}

void
hide_force_quit_dialog (WnckWindow *win)
{
    decor_t *d = g_object_get_data (G_OBJECT (win), "decor");

    if (d->force_quit_dialog)
    {
	gtk_widget_destroy (d->force_quit_dialog);
	d->force_quit_dialog = NULL;
    }
}
