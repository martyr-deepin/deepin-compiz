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
#ifndef _COMPIZ_GWD_SETTINGS_NOTIFIED_INTERFACE_H
#define _COMPIZ_GWD_SETTINGS_NOTIFIED_INTERFACE_H

#include <glib-object.h>

G_BEGIN_DECLS

#define GWD_SETTINGS_NOTIFIED_INTERFACE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
					      GWD_TYPE_SETTINGS_NOTIFIED_INTERFACE, \
					      GWDSettingsNotified))
#define GWD_SETTINGS_NOTIFIED_GET_INTERFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE(obj, \
										GWD_TYPE_SETTINGS_NOTIFIED_INTERFACE, \
										GWDSettingsNotifiedInterface))
#define GWD_TYPE_SETTINGS_NOTIFIED_INTERFACE (gwd_settings_notified_interface_get_type ())

typedef struct _GWDSettingsNotified GWDSettingsNotified;
typedef struct _GWDSettingsNotifiedInterface GWDSettingsNotifiedInterface;

struct _GWDSettingsNotifiedInterface
{
    GTypeInterface parent;

    gboolean (*update_decorations) (GWDSettingsNotified *notified);
    gboolean (*update_frames) (GWDSettingsNotified *notified);
    gboolean (*update_metacity_theme) (GWDSettingsNotified *notified);
    gboolean (*update_metacity_button_layout) (GWDSettingsNotified *notified);
};

gboolean
gwd_settings_notified_update_decorations (GWDSettingsNotified *notified);

gboolean
gwd_settings_notified_update_frames (GWDSettingsNotified *notified);

gboolean
gwd_settings_notified_update_metacity_theme (GWDSettingsNotified *notified);

gboolean
gwd_settings_notified_metacity_button_layout (GWDSettingsNotified *notified);

GType gwd_settings_notified_interface_get_type (void);

G_END_DECLS

#endif
