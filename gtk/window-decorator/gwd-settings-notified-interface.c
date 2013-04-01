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
#include "gwd-settings-notified-interface.h"

static void gwd_settings_notified_interface_default_init (GWDSettingsNotifiedInterface *settings_interface);

G_DEFINE_INTERFACE (GWDSettingsNotified, gwd_settings_notified_interface, G_TYPE_OBJECT);

static void gwd_settings_notified_interface_default_init (GWDSettingsNotifiedInterface *settings_interface)
{
}

gboolean
gwd_settings_notified_update_decorations (GWDSettingsNotified *notified)
{
    GWDSettingsNotifiedInterface *iface = GWD_SETTINGS_NOTIFIED_GET_INTERFACE (notified);
    return (*iface->update_decorations) (notified);
}

gboolean
gwd_settings_notified_update_frames (GWDSettingsNotified *notified)
{
    GWDSettingsNotifiedInterface *iface = GWD_SETTINGS_NOTIFIED_GET_INTERFACE (notified);
    return (*iface->update_frames) (notified);
}

gboolean
gwd_settings_notified_update_metacity_theme (GWDSettingsNotified *notified)
{
    GWDSettingsNotifiedInterface *iface = GWD_SETTINGS_NOTIFIED_GET_INTERFACE (notified);
    return (*iface->update_metacity_theme) (notified);
}

gboolean
gwd_settings_notified_metacity_button_layout (GWDSettingsNotified *notified)
{
    GWDSettingsNotifiedInterface *iface = GWD_SETTINGS_NOTIFIED_GET_INTERFACE (notified);
    return (*iface->update_metacity_button_layout) (notified);
}
