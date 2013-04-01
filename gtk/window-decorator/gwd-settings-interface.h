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
#ifndef _COMPIZ_GWD_SETTINGS_INTERFACE_H
#define _COMPIZ_GWD_SETTINGS_INTERFACE_H

#include <glib-object.h>

G_BEGIN_DECLS

#define GWD_SETTINGS_INTERFACE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
								 GWD_TYPE_SETTINGS_INTERFACE, \
								 GWDSettings))
#define GWD_SETTINGS_GET_INTERFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE(obj, \
								       GWD_TYPE_SETTINGS_INTERFACE, \
								       GWDSettingsInterface))
#define GWD_TYPE_SETTINGS_INTERFACE (gwd_settings_interface_get_type ())

typedef struct _GWDSettings GWDSettings;
typedef struct _GWDSettingsInterface GWDSettingsInterface;

struct _GWDSettingsInterface
{
    GTypeInterface parent;
};

enum
{
    BLUR_TYPE_NONE = 0,
    BLUR_TYPE_TITLEBAR = 1,
    BLUR_TYPE_ALL = 2
};

enum
{
    CLICK_ACTION_NONE,
    CLICK_ACTION_SHADE,
    CLICK_ACTION_MAXIMIZE,
    CLICK_ACTION_MINIMIZE,
    CLICK_ACTION_RAISE,
    CLICK_ACTION_LOWER,
    CLICK_ACTION_MENU
};

enum {
    WHEEL_ACTION_NONE,
    WHEEL_ACTION_SHADE
};

extern const gboolean USE_TOOLTIPS_DEFAULT;

extern const gdouble ACTIVE_SHADOW_RADIUS_DEFAULT;
extern const gdouble ACTIVE_SHADOW_OPACITY_DEFAULT;
extern const gint    ACTIVE_SHADOW_OFFSET_X_DEFAULT;
extern const gint    ACTIVE_SHADOW_OFFSET_Y_DEFAULT;
extern const gchar   *ACTIVE_SHADOW_COLOR_DEFAULT;

extern const gdouble INACTIVE_SHADOW_RADIUS_DEFAULT;
extern const gdouble INACTIVE_SHADOW_OPACITY_DEFAULT;
extern const gint    INACTIVE_SHADOW_OFFSET_X_DEFAULT;
extern const gint    INACTIVE_SHADOW_OFFSET_Y_DEFAULT;
extern const gchar   *INACTIVE_SHADOW_COLOR_DEFAULT;

extern const guint   DRAGGABLE_BORDER_WIDTH_DEFAULT;
extern const gboolean ATTACH_MODAL_DIALOGS_DEFAULT;
extern const gint    BLUR_TYPE_DEFAULT;

extern const gchar   *METACITY_THEME_DEFAULT;
extern const gdouble METACITY_ACTIVE_OPACITY_DEFAULT;
extern const gdouble METACITY_INACTIVE_OPACITY_DEFAULT;
extern const gboolean METACITY_ACTIVE_SHADE_OPACITY_DEFAULT;
extern const gboolean METACITY_INACTIVE_SHADE_OPACITY_DEFAULT;

extern const gchar *  METACITY_BUTTON_LAYOUT_DEFAULT;

extern const guint DOUBLE_CLICK_ACTION_DEFAULT;
extern const guint MIDDLE_CLICK_ACTION_DEFAULT;
extern const guint RIGHT_CLICK_ACTION_DEFAULT;
extern const guint WHEEL_ACTION_DEFAULT;

extern const gchar * TITLEBAR_FONT_DEFAULT;

GType gwd_settings_interface_get_type (void);

G_END_DECLS

#endif
