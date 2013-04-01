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
#include "gwd-settings-writable-interface.h"
#include "gwd-settings-storage-interface.h"

#ifdef USE_GSETTINGS
#include "gwd-settings-storage-gsettings.h"
#else
#ifdef USE_GCONF
#include "gwd-settings-storage-gconf.h"
#endif
#endif

#include "gwd-settings-xproperty-interface.h"
#include "gwd-settings-xproperty-storage.h"

GWDSettingsStorage *storage = NULL;
GWDSettingsXPropertyStorage *xprop_storage = NULL;

#ifdef USE_GSETTINGS


#endif

gboolean
init_settings (GWDSettingsWritable *writable,
	       WnckScreen	    *screen)
{
#ifdef USE_GSETTINGS
#define STORAGE_USED
    GSettings *compiz = gwd_get_org_compiz_gwd_settings ();
    GSettings *mutter = gwd_get_org_gnome_mutter_settings ();
    GSettings *gnome  = gwd_get_org_gnome_desktop_wm_preferences_settings ();
    storage = gwd_settings_storage_gsettings_new (gnome, mutter, compiz, writable);

    gwd_connect_org_compiz_gwd_settings (compiz, storage);
    gwd_connect_org_gnome_mutter_settings (mutter, storage);
    gwd_connect_org_gnome_desktop_wm_preferences_settings (gnome, storage);
#else
#ifdef USE_GSETTINGS
#define STORAGE_USED
    storage = gwd_settings_storage_gconf_new (writable);
#endif
#endif

    GdkDisplay *display = gdk_display_get_default ();
    Display    *xdisplay = gdk_x11_display_get_xdisplay (display);
    Window     root = gdk_x11_get_default_root_xwindow ();

    xprop_storage = gwd_settings_storage_xprop_new (xdisplay,
						    root,
						    writable);

#ifdef STORAGE_USED
    gwd_settings_storage_update_metacity_theme (storage);
    gwd_settings_storage_update_opacity (storage);
    gwd_settings_storage_update_button_layout (storage);
    gwd_settings_storage_update_font (storage);
    gwd_settings_storage_update_titlebar_actions (storage);
    gwd_settings_storage_update_blur (storage);
    gwd_settings_storage_update_draggable_border_width (storage);
    gwd_settings_storage_update_attach_modal_dialogs (storage);
    gwd_settings_storage_update_use_tooltips (storage);
    gwd_process_decor_shadow_property_update ();
#else
    storage = NULL;
#endif

#undef STORAGE_USED

    return TRUE;
}

void
fini_settings ()
{
    if (storage)
	g_object_unref (storage);

    if (xprop_storage)
	g_object_unref (xprop_storage);
}

gboolean
gwd_process_decor_shadow_property_update ()
{
    return gwd_settings_xproperty_storage_update_all (xprop_storage);
}
