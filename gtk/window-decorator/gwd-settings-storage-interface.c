/*
 * Copyright © 2012 Canonical Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Authored By: Sam Spilsbury <sam.spilsbury@canonical.com>
 */
#include "gwd-settings-storage-interface.h"

static void gwd_settings_storage_interface_default_init (GWDSettingsStorageInterface *settings_interface);

G_DEFINE_INTERFACE (GWDSettingsStorage, gwd_settings_storage_interface, G_TYPE_OBJECT);

static void gwd_settings_storage_interface_default_init (GWDSettingsStorageInterface *settings_interface)
{
}

gboolean gwd_settings_storage_update_use_tooltips (GWDSettingsStorage *settings)
{
    GWDSettingsStorageInterface *interface = GWD_SETTINGS_STORAGE_GET_INTERFACE (settings);
    return (*interface->update_use_tooltips) (settings);
}

gboolean gwd_settings_storage_update_draggable_border_width (GWDSettingsStorage *settings)
{
    GWDSettingsStorageInterface *interface = GWD_SETTINGS_STORAGE_GET_INTERFACE (settings);
    return (*interface->update_draggable_border_width) (settings);
}

gboolean gwd_settings_storage_update_attach_modal_dialogs (GWDSettingsStorage *settings)
{
    GWDSettingsStorageInterface *interface = GWD_SETTINGS_STORAGE_GET_INTERFACE (settings);
    return (*interface->update_attach_modal_dialogs) (settings);
}

gboolean gwd_settings_storage_update_blur (GWDSettingsStorage *settings)
{
    GWDSettingsStorageInterface *interface = GWD_SETTINGS_STORAGE_GET_INTERFACE (settings);
    return (*interface->update_blur) (settings);
}

gboolean gwd_settings_storage_update_metacity_theme (GWDSettingsStorage *settings)
{
    GWDSettingsStorageInterface *interface = GWD_SETTINGS_STORAGE_GET_INTERFACE (settings);
    return (*interface->update_metacity_theme) (settings);
}

gboolean gwd_settings_storage_update_opacity (GWDSettingsStorage *settings)
{
    GWDSettingsStorageInterface *interface = GWD_SETTINGS_STORAGE_GET_INTERFACE (settings);
    return (*interface->update_opacity) (settings);
}

gboolean gwd_settings_storage_update_button_layout (GWDSettingsStorage *settings)
{
    GWDSettingsStorageInterface *interface = GWD_SETTINGS_STORAGE_GET_INTERFACE (settings);
    return (*interface->update_button_layout) (settings);
}

gboolean gwd_settings_storage_update_font (GWDSettingsStorage *settings)
{
    GWDSettingsStorageInterface *interface = GWD_SETTINGS_STORAGE_GET_INTERFACE (settings);
    return (*interface->update_font) (settings);
}

gboolean gwd_settings_storage_update_titlebar_actions (GWDSettingsStorage *settings)
{
    GWDSettingsStorageInterface *interface = GWD_SETTINGS_STORAGE_GET_INTERFACE (settings);
    return (*interface->update_titlebar_actions) (settings);
}
