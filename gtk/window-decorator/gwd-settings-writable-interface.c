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
#include "gwd-settings-writable-interface.h"

static void gwd_settings_writable_interface_default_init (GWDSettingsWritableInterface *settings_interface);

G_DEFINE_INTERFACE (GWDSettingsWritable, gwd_settings_writable_interface, G_TYPE_OBJECT);

static void gwd_settings_writable_interface_default_init (GWDSettingsWritableInterface *settings_interface)
{
}

void
gwd_settings_writable_freeze_updates (GWDSettingsWritable *settings)
{
    GWDSettingsWritableInterface *iface = GWD_SETTINGS_WRITABLE_GET_INTERFACE (settings);
    (*iface->freeze_updates) (settings);
}

void
gwd_settings_writable_thaw_updates (GWDSettingsWritable *settings)
{
    GWDSettingsWritableInterface *iface = GWD_SETTINGS_WRITABLE_GET_INTERFACE (settings);
    (*iface->thaw_updates) (settings);
}

gboolean
gwd_settings_writable_shadow_property_changed (GWDSettingsWritable *settings,
					       gdouble             active_shadow_radius,
					       gdouble             active_shadow_opacity,
					       gdouble             active_shadow_offset_x,
					       gdouble             active_shadow_offset_y,
					       const gchar         *active_shadow_color,
					       gdouble             inactive_shadow_radius,
					       gdouble             inactive_shadow_opacity,
					       gdouble             inactive_shadow_offset_x,
					       gdouble             inactive_shadow_offset_y,
					       const gchar         *inactive_shadow_color)
{
    GWDSettingsWritableInterface *iface = GWD_SETTINGS_WRITABLE_GET_INTERFACE (settings);
    return (*iface->shadow_property_changed) (settings,
					      active_shadow_radius,
					      active_shadow_opacity,
					      active_shadow_offset_x,
					      active_shadow_offset_y,
					      active_shadow_color,
					      inactive_shadow_radius,
					      inactive_shadow_opacity,
					      inactive_shadow_offset_x,
					      inactive_shadow_offset_y,
					      inactive_shadow_color);
}

gboolean
gwd_settings_writable_use_tooltips_changed (GWDSettingsWritable *settings,
					    gboolean            use_tooltips)
{
    GWDSettingsWritableInterface *iface = GWD_SETTINGS_WRITABLE_GET_INTERFACE (settings);
    return (*iface->use_tooltips_changed) (settings, use_tooltips);
}

gboolean
gwd_settings_writable_draggable_border_width_changed (GWDSettingsWritable *settings,
						      gint	         draggable_border_width)
{
    GWDSettingsWritableInterface *iface = GWD_SETTINGS_WRITABLE_GET_INTERFACE (settings);
    return (*iface->draggable_border_width_changed) (settings, draggable_border_width);
}

gboolean
gwd_settings_writable_attach_modal_dialogs_changed (GWDSettingsWritable *settings,
						    gboolean            attach_modal_dialogs)
{
    GWDSettingsWritableInterface *iface = GWD_SETTINGS_WRITABLE_GET_INTERFACE (settings);
    return (*iface->attach_modal_dialogs_changed) (settings, attach_modal_dialogs);
}

gboolean
gwd_settings_writable_blur_changed (GWDSettingsWritable *settings,
				    const gchar         *blur_type)
{
    GWDSettingsWritableInterface *iface = GWD_SETTINGS_WRITABLE_GET_INTERFACE (settings);
    return (*iface->blur_changed) (settings, blur_type);
}

gboolean
gwd_settings_writable_metacity_theme_changed (GWDSettingsWritable *settings,
					      gboolean	          use_metacity_theme,
					      const gchar         *metacity_theme)
{
    GWDSettingsWritableInterface *iface = GWD_SETTINGS_WRITABLE_GET_INTERFACE (settings);
    return (*iface->metacity_theme_changed) (settings, use_metacity_theme, metacity_theme);
}

gboolean
gwd_settings_writable_opacity_changed (GWDSettingsWritable *settings,
				       gdouble             active_opacity,
				       gdouble             inactive_opacity,
				       gboolean            active_shade_opacity,
				       gboolean            inactive_shade_opacity)
{
    GWDSettingsWritableInterface *iface = GWD_SETTINGS_WRITABLE_GET_INTERFACE (settings);
    return (*iface->opacity_changed) (settings, active_opacity, inactive_opacity, active_shade_opacity, inactive_shade_opacity);
}

gboolean
gwd_settings_writable_button_layout_changed (GWDSettingsWritable *settings,
					     const gchar         *button_layout)
{
    GWDSettingsWritableInterface *iface = GWD_SETTINGS_WRITABLE_GET_INTERFACE (settings);
    return (*iface->button_layout_changed) (settings, button_layout);
}

gboolean
gwd_settings_writable_font_changed (GWDSettingsWritable *settings,
				    gboolean		titlebar_uses_system_font,
				    const gchar		*titlebar_font)
{
    GWDSettingsWritableInterface *iface = GWD_SETTINGS_WRITABLE_GET_INTERFACE (settings);
    return (*iface->font_changed) (settings, titlebar_uses_system_font, titlebar_font);
}

gboolean
gwd_settings_writable_titlebar_actions_changed (GWDSettingsWritable *settings,
						const gchar	    *action_double_click_titlebar,
						const gchar	    *action_middle_click_titlebar,
						const gchar	    *action_right_click_titlebar,
						const gchar	    *mouse_wheel_action)
{
    GWDSettingsWritableInterface *iface = GWD_SETTINGS_WRITABLE_GET_INTERFACE (settings);
    return (*iface->titlebar_actions_changed) (settings,
					       action_double_click_titlebar,
					       action_middle_click_titlebar,
					       action_right_click_titlebar,
					       mouse_wheel_action);
}
