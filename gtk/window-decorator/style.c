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
update_style (GtkWidget *widget)
{
    GtkStyle      *style;
    decor_color_t spot_color;
    
    style = gtk_widget_get_style (widget);
    g_object_ref (G_OBJECT (style));
    
    style = gtk_style_attach (style, widget->window);
    
    spot_color.r = style->bg[GTK_STATE_SELECTED].red   / 65535.0;
    spot_color.g = style->bg[GTK_STATE_SELECTED].green / 65535.0;
    spot_color.b = style->bg[GTK_STATE_SELECTED].blue  / 65535.0;
    
    g_object_unref (G_OBJECT (style));
    
    shade (&spot_color, &_title_color[0], 1.05);
    shade (&_title_color[0], &_title_color[1], 0.85);
    
}

void
style_changed (GtkWidget *widget,
	       void      *user_data)
{
    GdkDisplay *gdkdisplay;
    GdkScreen  *gdkscreen;
    WnckScreen *screen;

    PangoContext *context = (PangoContext *) user_data;

    gdkdisplay = gdk_display_get_default ();
    gdkscreen  = gdk_display_get_default_screen (gdkdisplay);
    screen     = wnck_screen_get_default ();

    update_style (widget);

    pango_cairo_context_set_resolution (context,
					gdk_screen_get_resolution (gdkscreen));

    decorations_changed (screen);
}
