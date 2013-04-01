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
#include "gwd-settings-xproperty-interface.h"

static void gwd_settings_xproperty_storage_interface_default_init (GWDSettingsXPropertyStorageInterface *settings_interface);

G_DEFINE_INTERFACE (GWDSettingsXPropertyStorage, gwd_settings_xproperty_storage_interface, G_TYPE_OBJECT);

static void gwd_settings_xproperty_storage_interface_default_init (GWDSettingsXPropertyStorageInterface *settings_interface)
{
}

gboolean
gwd_settings_xproperty_storage_update_all (GWDSettingsXPropertyStorage *storage)
{
    GWDSettingsXPropertyStorageInterface *iface = GWD_SETTINGS_XPROPERTY_STORAGE_GET_INTERFACE (storage);
    return (*iface->update_all) (storage);
}
