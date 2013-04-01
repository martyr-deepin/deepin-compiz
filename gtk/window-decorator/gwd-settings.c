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
#include <glib-object.h>

#include <stdlib.h>
#include <stdio.h>

#include "gwd-settings.h"
#include "gwd-settings-interface.h"
#include "gwd-settings-writable-interface.h"
#include "gwd-settings-notified-interface.h"
#include "decoration.h"

#define GWD_SETTINGS_IMPL(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GWD_TYPE_SETTINGS_IMPL, GWDSettingsImpl))
#define GWD_SETTINGS_IMPL_CLASS(obj) (G_TYPE_CHECK_CLASS_CAST ((obj), GWD_TYPE_SETTINGS_IMPL, GWDSettingsImplClass))
#define GWD_IS_SETTINGS_IMPL(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GWD_TYPE_SETTINGS_IMPL))
#define GWD_IS_SETTINGS_IMPL_CLASS(obj) (G_TYPE_CHECK_CLASS_TYPE ((obj), GWD_TYPE_SETTINGS_IMPL))
#define GWD_SETTINGS_IMPL_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GWD_TYPE_SETTINGS_IMPL, GWDSettingsImplClass))

typedef struct _GWDSettingsImpl
{
    GObject parent;
} GWDSettingsImpl;

typedef struct _GWDSettingsImplClass
{
    GObjectClass parent_class;
} GWDSettingsImplClass;

static void gwd_settings_interface_init (GWDSettingsInterface *interface);
static void gwd_settings_writable_interface_init (GWDSettingsWritableInterface *interface);

G_DEFINE_TYPE_WITH_CODE (GWDSettingsImpl, gwd_settings_impl, G_TYPE_OBJECT,
			 G_IMPLEMENT_INTERFACE (GWD_TYPE_SETTINGS_INTERFACE,
						gwd_settings_interface_init)
			 G_IMPLEMENT_INTERFACE (GWD_TYPE_WRITABLE_SETTINGS_INTERFACE,
						gwd_settings_writable_interface_init))

#define GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), GWD_TYPE_SETTINGS_IMPL, GWDSettingsImplPrivate))

enum
{
    GWD_SETTINGS_IMPL_PROPERTY_ACTIVE_SHADOW = 1,
    GWD_SETTINGS_IMPL_PROPERTY_INACTIVE_SHADOW = 2,
    GWD_SETTINGS_IMPL_PROPERTY_USE_TOOLTIPS = 3,
    GWD_SETTINGS_IMPL_PROPERTY_DRAGGABLE_BORDER_WIDTH = 4,
    GWD_SETTINGS_IMPL_PROPERTY_ATTACH_MODAL_DIALOGS = 5,
    GWD_SETTINGS_IMPL_PROPERTY_BLUR_CHANGED = 6,
    GWD_SETTINGS_IMPL_PROPERTY_METACITY_THEME = 7,
    GWD_SETTINGS_IMPL_PROPERTY_ACTIVE_OPACITY = 8,
    GWD_SETTINGS_IMPL_PROPERTY_INACTIVE_OPACITY = 9,
    GWD_SETTINGS_IMPL_PROPERTY_ACTIVE_SHADE_OPACITY = 10,
    GWD_SETTINGS_IMPL_PROPERTY_INACTIVE_SHADE_OPACITY = 11,
    GWD_SETTINGS_IMPL_PROPERTY_BUTTON_LAYOUT = 12,
    GWD_SETTINGS_IMPL_PROPERTY_TITLEBAR_ACTION_DOUBLE_CLICK = 13,
    GWD_SETTINGS_IMPL_PROPERTY_TITLEBAR_ACTION_MIDDLE_CLICK = 14,
    GWD_SETTINGS_IMPL_PROPERTY_TITLEBAR_ACTION_RIGHT_CLICK = 15,
    GWD_SETTINGS_IMPL_PROPERTY_MOUSE_WHEEL_ACTION = 16,
    GWD_SETTINGS_IMPL_PROPERTY_TITLEBAR_FONT = 17,
    GWD_SETTINGS_IMPL_PROPERTY_CMDLINE_OPTIONS = 18,
    GWD_SETTINGS_IMPL_PROPERTY_SETTINGS_NOTIFIED = 19
};

enum
{
    CMDLINE_BLUR = (1 << 0),
    CMDLINE_THEME = (1 << 1)
};

typedef gboolean (*NotifyFunc) (GWDSettingsNotified *);

typedef struct _GWDSettingsImplPrivate
{
    decor_shadow_options_t active_shadow;
    decor_shadow_options_t inactive_shadow;
    gboolean		   use_tooltips;
    gint		   draggable_border_width;
    gboolean		   attach_modal_dialogs;
    gint		   blur_type;
    gchar		   *metacity_theme;
    gdouble		   metacity_active_opacity;
    gdouble		   metacity_inactive_opacity;
    gboolean		   metacity_active_shade_opacity;
    gboolean		   metacity_inactive_shade_opacity;
    gchar		   *metacity_button_layout;
    gint		   titlebar_double_click_action;
    gint		   titlebar_middle_click_action;
    gint		   titlebar_right_click_action;
    gint		   mouse_wheel_action;
    gchar		   *titlebar_font;
    guint		   cmdline_opts;
    GWDSettingsNotified    *notified;
    guint		   freeze_count;
    GList		   *notify_funcs;
} GWDSettingsImplPrivate;

static void
append_to_notify_funcs (GWDSettingsImpl *settings,
			NotifyFunc	func)
{
    GWDSettingsImplPrivate *priv = GET_PRIVATE (settings);

    /* Remove if found, the new one will replace the old one */
    GList *link = g_list_find (priv->notify_funcs, func);

    if (link)
	priv->notify_funcs = g_list_remove_link (priv->notify_funcs, link);

    priv->notify_funcs = g_list_append (priv->notify_funcs, (gpointer) func);
}

static void
invoke_notify_func (gpointer data,
		    gpointer user_data)
{
    GWDSettingsNotified *notified = (GWDSettingsNotified *) user_data;
    NotifyFunc	        func = (NotifyFunc) data;

    (*func) (notified);
}

static void
release_notify_funcs (GWDSettingsImpl *settings)
{
    GWDSettingsImplPrivate *priv = GET_PRIVATE (settings);

    if (priv->freeze_count)
	return;

    g_list_foreach (priv->notify_funcs, invoke_notify_func, priv->notified);
    g_list_free (priv->notify_funcs);
    priv->notify_funcs = NULL;
}

gboolean
gwd_settings_shadow_property_changed (GWDSettingsWritable *settings,
				      gdouble     active_shadow_radius,
				      gdouble     active_shadow_opacity,
				      gdouble     active_shadow_offset_x,
				      gdouble     active_shadow_offset_y,
				      const gchar *active_shadow_color,
				      gdouble     inactive_shadow_radius,
				      gdouble     inactive_shadow_opacity,
				      gdouble     inactive_shadow_offset_x,
				      gdouble     inactive_shadow_offset_y,
				      const gchar *inactive_shadow_color)
{
    GWDSettingsImpl        *settings_impl = GWD_SETTINGS_IMPL (settings);
    GWDSettingsImplPrivate *priv = GET_PRIVATE (settings_impl);

    decor_shadow_options_t active_shadow, inactive_shadow;
    unsigned int           c[4];
    gboolean               changed = FALSE;

    active_shadow.shadow_radius = active_shadow_radius;
    active_shadow.shadow_opacity = active_shadow_opacity;
    active_shadow.shadow_offset_x = active_shadow_offset_x;
    active_shadow.shadow_offset_y = active_shadow_offset_y;

    if (sscanf (active_shadow_color,
		"#%2x%2x%2x%2x",
		&c[0], &c[1], &c[2], &c[3]) == 4)
    {
	active_shadow.shadow_color[0] = c[0] << 8 | c[0];
	active_shadow.shadow_color[1] = c[1] << 8 | c[1];
	active_shadow.shadow_color[2] = c[2] << 8 | c[2];
    }
    else
	return FALSE;

    if (sscanf (inactive_shadow_color,
		"#%2x%2x%2x%2x",
		&c[0], &c[1], &c[2], &c[3]) == 4)
    {
	inactive_shadow.shadow_color[0] = c[0] << 8 | c[0];
	inactive_shadow.shadow_color[1] = c[1] << 8 | c[1];
	inactive_shadow.shadow_color[2] = c[2] << 8 | c[2];
    }
    else
	return FALSE;

    inactive_shadow.shadow_radius = inactive_shadow_radius;
    inactive_shadow.shadow_opacity = inactive_shadow_opacity;
    inactive_shadow.shadow_offset_x = inactive_shadow_offset_x;
    inactive_shadow.shadow_offset_y = inactive_shadow_offset_y;

    if (decor_shadow_options_cmp (&priv->inactive_shadow,
				  &inactive_shadow))
    {
	changed |= TRUE;
	priv->inactive_shadow = inactive_shadow;
    }

    if (decor_shadow_options_cmp (&priv->active_shadow,
				  &active_shadow))
    {
	changed |= TRUE;
	priv->active_shadow = active_shadow;
    }

    if (changed)
    {
	append_to_notify_funcs (settings_impl, gwd_settings_notified_update_decorations);
	release_notify_funcs (settings_impl);
    }

    return changed;
}

static gboolean
gwd_settings_use_tooltips_changed (GWDSettingsWritable *settings,
				   gboolean            use_tooltips)
{
    GWDSettingsImpl        *settings_impl = GWD_SETTINGS_IMPL (settings);
    GWDSettingsImplPrivate *priv = GET_PRIVATE (settings_impl);

    if (priv->use_tooltips != use_tooltips)
    {
	priv->use_tooltips = use_tooltips;
	append_to_notify_funcs (settings_impl, gwd_settings_notified_update_decorations);
	release_notify_funcs (settings_impl);
	return TRUE;
    }

    return FALSE;
}

static gboolean
gwd_settings_draggable_border_width_changed (GWDSettingsWritable *settings,
					     gint	         draggable_border_width)
{
    GWDSettingsImpl        *settings_impl = GWD_SETTINGS_IMPL (settings);
    GWDSettingsImplPrivate *priv = GET_PRIVATE (settings_impl);

    if (priv->draggable_border_width != draggable_border_width)
    {
	priv->draggable_border_width = draggable_border_width;
	append_to_notify_funcs (settings_impl, gwd_settings_notified_update_decorations);
	release_notify_funcs (settings_impl);
	return TRUE;
    }
    else
	return FALSE;
}

static gboolean
gwd_settings_attach_modal_dialogs_changed (GWDSettingsWritable *settings,
					   gboolean            attach_modal_dialogs)
{
    GWDSettingsImpl        *settings_impl = GWD_SETTINGS_IMPL (settings);
    GWDSettingsImplPrivate *priv = GET_PRIVATE (settings_impl);

    if (priv->attach_modal_dialogs != attach_modal_dialogs)
    {
	priv->attach_modal_dialogs = attach_modal_dialogs;
	append_to_notify_funcs (settings_impl, gwd_settings_notified_update_decorations);
	release_notify_funcs (settings_impl);
	return TRUE;
    }
    else
	return FALSE;
}

static gboolean
gwd_settings_blur_changed (GWDSettingsWritable *settings,
			   const gchar         *type)

{
    GWDSettingsImpl        *settings_impl = GWD_SETTINGS_IMPL (settings);
    GWDSettingsImplPrivate *priv = GET_PRIVATE (settings_impl);
    gint new_type = -1;

    if (priv->cmdline_opts & CMDLINE_BLUR)
	return FALSE;

    if (strcmp (type, "titlebar") == 0)
	new_type = BLUR_TYPE_TITLEBAR;
    else if (strcmp (type, "all") == 0)
	new_type = BLUR_TYPE_ALL;
    else if (strcmp (type, "none") == 0)
	new_type = BLUR_TYPE_NONE;

    if (new_type == -1)
	return FALSE;

    if (priv->blur_type != new_type)
    {
	priv->blur_type = new_type;
	append_to_notify_funcs (settings_impl, gwd_settings_notified_update_decorations);
	release_notify_funcs (settings_impl);
	return TRUE;
    }
    else
	return FALSE;
}

static void
free_and_set_metacity_theme (GWDSettingsWritable *settings,
			     const gchar	 *metacity_theme)
{
    GWDSettingsImpl        *settings_impl = GWD_SETTINGS_IMPL (settings);
    GWDSettingsImplPrivate *priv = GET_PRIVATE (settings_impl);

    if (priv->metacity_theme)
	g_free (priv->metacity_theme);

    priv->metacity_theme = g_strdup (metacity_theme);
}

static gboolean
gwd_settings_metacity_theme_changed (GWDSettingsWritable *settings,
				     gboolean	         use_metacity_theme,
				     const gchar         *metacity_theme)
{
    GWDSettingsImpl        *settings_impl = GWD_SETTINGS_IMPL (settings);
    GWDSettingsImplPrivate *priv = GET_PRIVATE (settings_impl);

    if (priv->cmdline_opts & CMDLINE_THEME)
	return FALSE;

    if (!metacity_theme)
	return FALSE;

    if (use_metacity_theme)
    {
	if (g_strcmp0 (metacity_theme, priv->metacity_theme) == 0)
	    return FALSE;

	free_and_set_metacity_theme (settings, metacity_theme);
    }
    else
	free_and_set_metacity_theme (settings, "");

    append_to_notify_funcs (settings_impl, gwd_settings_notified_update_metacity_theme);
    append_to_notify_funcs (settings_impl, gwd_settings_notified_update_decorations);
    release_notify_funcs (settings_impl);

    return TRUE;
}

static gboolean
gwd_settings_opacity_changed (GWDSettingsWritable *settings,
			      gdouble             active_opacity,
			      gdouble             inactive_opacity,
			      gboolean            active_shade_opacity,
			      gboolean            inactive_shade_opacity)
{
    GWDSettingsImpl        *settings_impl = GWD_SETTINGS_IMPL (settings);
    GWDSettingsImplPrivate *priv = GET_PRIVATE (settings_impl);

    if (priv->metacity_active_opacity == active_opacity &&
	priv->metacity_inactive_opacity == inactive_opacity &&
	priv->metacity_active_shade_opacity == active_shade_opacity &&
	priv->metacity_inactive_shade_opacity == inactive_shade_opacity)
	return FALSE;

    priv->metacity_active_opacity = active_opacity;
    priv->metacity_inactive_opacity = inactive_opacity;
    priv->metacity_active_shade_opacity = active_shade_opacity;
    priv->metacity_inactive_shade_opacity = inactive_shade_opacity;

    append_to_notify_funcs (settings_impl, gwd_settings_notified_update_decorations);
    release_notify_funcs (settings_impl);

    return TRUE;
}

static gboolean
gwd_settings_button_layout_changed (GWDSettingsWritable *settings,
				    const gchar         *button_layout)
{
    GWDSettingsImpl        *settings_impl = GWD_SETTINGS_IMPL (settings);
    GWDSettingsImplPrivate *priv = GET_PRIVATE (settings_impl);

    if (!button_layout)
	return FALSE;

    if (g_strcmp0 (priv->metacity_button_layout, button_layout) == 0)
	return FALSE;

    if (priv->metacity_button_layout)
	g_free (priv->metacity_button_layout);

    priv->metacity_button_layout = g_strdup (button_layout);

    append_to_notify_funcs (settings_impl, gwd_settings_notified_metacity_button_layout);
    append_to_notify_funcs (settings_impl, gwd_settings_notified_update_decorations);
    release_notify_funcs (settings_impl);

    return TRUE;
}

static gboolean
gwd_settings_font_changed (GWDSettingsWritable *settings,
			   gboolean            titlebar_uses_system_font,
			   const gchar         *titlebar_font)
{
    GWDSettingsImpl        *settings_impl = GWD_SETTINGS_IMPL (settings);
    GWDSettingsImplPrivate *priv = GET_PRIVATE (settings_impl);

    const gchar *no_font = NULL;
    const gchar *use_font = NULL;

    if (!titlebar_font)
	return FALSE;

    if (titlebar_uses_system_font)
	use_font = no_font;
    else
	use_font = titlebar_font;

    if (g_strcmp0 (priv->titlebar_font, use_font) == 0)
	return FALSE;

    if (priv->titlebar_font)
    {
	g_free (priv->titlebar_font);
	priv->titlebar_font = NULL;
    }

    priv->titlebar_font = use_font ? g_strdup (use_font) : NULL;

    append_to_notify_funcs (settings_impl, gwd_settings_notified_update_decorations);
    append_to_notify_funcs (settings_impl, gwd_settings_notified_update_frames);
    release_notify_funcs (settings_impl);

    return TRUE;
}

static gboolean
get_click_action_value (const gchar *action,
			gint	    *action_value,
			gint	    default_value)
{
    if (!action_value)
	return FALSE;

    *action_value = -1;

    if (strcmp (action, "toggle_shade") == 0)
	*action_value = CLICK_ACTION_SHADE;
    else if (strcmp (action, "toggle_maximize") == 0)
	*action_value = CLICK_ACTION_MAXIMIZE;
    else if (strcmp (action, "minimize") == 0)
	*action_value = CLICK_ACTION_MINIMIZE;
    else if (strcmp (action, "raise") == 0)
	*action_value = CLICK_ACTION_RAISE;
    else if (strcmp (action, "lower") == 0)
	*action_value = CLICK_ACTION_LOWER;
    else if (strcmp (action, "menu") == 0)
	*action_value = CLICK_ACTION_MENU;
    else if (strcmp (action, "none") == 0)
	*action_value = CLICK_ACTION_NONE;

    if (*action_value == -1)
    {
	*action_value = default_value;
	return FALSE;
    }

    return TRUE;
}

static gboolean
get_wheel_action_value (const gchar *action,
			gint	    *action_value,
			gint	    default_value)
{
    if (!action_value)
	return FALSE;

    *action_value = -1;

    if (strcmp (action, "shade") == 0)
	*action_value = WHEEL_ACTION_SHADE;
    else if (strcmp (action, "none") == 0)
	*action_value = WHEEL_ACTION_NONE;

    if (*action_value == -1)
    {
	*action_value = default_value;
	return FALSE;
    }

    return TRUE;
}

static gboolean
gwd_settings_actions_changed (GWDSettingsWritable *settings,
			      const gchar	  *action_double_click_titlebar,
			      const gchar	  *action_middle_click_titlebar,
			      const gchar	  *action_right_click_titlebar,
			      const gchar	  *mouse_wheel_action)
{
    GWDSettingsImpl        *settings_impl = GWD_SETTINGS_IMPL (settings);
    GWDSettingsImplPrivate *priv = GET_PRIVATE (settings_impl);

    gboolean ret = FALSE;

    ret |= get_click_action_value (action_double_click_titlebar,
				   &priv->titlebar_double_click_action,
				   DOUBLE_CLICK_ACTION_DEFAULT);
    ret |= get_click_action_value (action_middle_click_titlebar,
				   &priv->titlebar_middle_click_action,
				   MIDDLE_CLICK_ACTION_DEFAULT);
    ret |= get_click_action_value (action_right_click_titlebar,
				   &priv->titlebar_right_click_action,
				   RIGHT_CLICK_ACTION_DEFAULT);
    ret |= get_wheel_action_value (mouse_wheel_action,
				   &priv->mouse_wheel_action,
				   WHEEL_ACTION_DEFAULT);

    return ret;
}

static void
gwd_settings_freeze_updates (GWDSettingsWritable *writable)
{
    GWDSettingsImplPrivate *priv = GET_PRIVATE (writable);
    priv->freeze_count++;
}

static void
gwd_settings_thaw_updates (GWDSettingsWritable *writable)
{
    GWDSettingsImplPrivate *priv = GET_PRIVATE (writable);

    if (priv->freeze_count)
	priv->freeze_count--;

    release_notify_funcs (GWD_SETTINGS_IMPL (writable));
}

static void
gwd_settings_writable_interface_init (GWDSettingsWritableInterface *interface)
{
    interface->shadow_property_changed = gwd_settings_shadow_property_changed;
    interface->use_tooltips_changed = gwd_settings_use_tooltips_changed;
    interface->draggable_border_width_changed = gwd_settings_draggable_border_width_changed;
    interface->attach_modal_dialogs_changed = gwd_settings_attach_modal_dialogs_changed;
    interface->blur_changed = gwd_settings_blur_changed;
    interface->metacity_theme_changed = gwd_settings_metacity_theme_changed;
    interface->opacity_changed = gwd_settings_opacity_changed;
    interface->button_layout_changed = gwd_settings_button_layout_changed;
    interface->font_changed = gwd_settings_font_changed;
    interface->titlebar_actions_changed = gwd_settings_actions_changed;
    interface->freeze_updates = gwd_settings_freeze_updates;
    interface->thaw_updates = gwd_settings_thaw_updates;
}

static void
gwd_settings_interface_init (GWDSettingsInterface *interface)
{
}

static void
gwd_settings_dispose (GObject *object)
{
    G_OBJECT_CLASS (gwd_settings_impl_parent_class)->dispose (object);
}

static void
gwd_settings_finalize (GObject *object)
{
    GWDSettingsImplPrivate *priv = GET_PRIVATE (object);
    G_OBJECT_CLASS (gwd_settings_impl_parent_class)->finalize (object);

    if (priv->metacity_theme)
    {
	g_free (priv->metacity_theme);
	priv->metacity_theme = NULL;
    }

    if (priv->metacity_button_layout)
    {
	g_free (priv->metacity_button_layout);
	priv->metacity_button_layout = NULL;
    }

    if (priv->titlebar_font)
    {
	g_free (priv->titlebar_font);
	priv->titlebar_font = NULL;
    }

    if (priv->notified)
    {
	g_object_unref (priv->notified);
	priv->notified = NULL;
    }
}

static void
gwd_settings_set_property (GObject      *object,
			   guint        property_id,
			   const GValue *value,
			   GParamSpec   *pspec)
{
    GWDSettingsImplPrivate *priv = GET_PRIVATE (object);

    switch (property_id)
    {
	case GWD_SETTINGS_IMPL_PROPERTY_CMDLINE_OPTIONS:
	    priv->cmdline_opts = g_value_get_int (value);
	    break;
	case GWD_SETTINGS_IMPL_PROPERTY_BLUR_CHANGED:
	    priv->blur_type = g_value_get_int (value);
	    break;
	case GWD_SETTINGS_IMPL_PROPERTY_METACITY_THEME:
	    if (priv->metacity_theme)
		g_free (priv->metacity_theme);

	    priv->metacity_theme = g_value_dup_string (value);
	    break;
	case GWD_SETTINGS_IMPL_PROPERTY_SETTINGS_NOTIFIED:
	    g_return_if_fail (!priv->notified);
	    priv->notified = (GWDSettingsNotified *) g_value_get_pointer (value);
	default:
	    break;
    }
}

static void
gwd_settings_get_property (GObject    *object,
			   guint      property_id,
			   GValue     *value,
			   GParamSpec *pspec)
{
    GWDSettingsImplPrivate *priv = GET_PRIVATE (object);

    switch (property_id)
    {
	case GWD_SETTINGS_IMPL_PROPERTY_ACTIVE_SHADOW:
	    g_value_set_pointer (value, &priv->active_shadow);
	    break;
	case GWD_SETTINGS_IMPL_PROPERTY_INACTIVE_SHADOW:
	    g_value_set_pointer (value, &priv->inactive_shadow);
	    break;
	case GWD_SETTINGS_IMPL_PROPERTY_USE_TOOLTIPS:
	    g_value_set_boolean (value, priv->use_tooltips);
	    break;
	case GWD_SETTINGS_IMPL_PROPERTY_DRAGGABLE_BORDER_WIDTH:
	    g_value_set_int (value, priv->draggable_border_width);
	    break;
	case GWD_SETTINGS_IMPL_PROPERTY_ATTACH_MODAL_DIALOGS:
	    g_value_set_boolean (value, priv->attach_modal_dialogs);
	    break;
	case GWD_SETTINGS_IMPL_PROPERTY_BLUR_CHANGED:
	    g_value_set_int (value, priv->blur_type);
	    break;
	case GWD_SETTINGS_IMPL_PROPERTY_METACITY_THEME:
	    g_value_set_string (value, priv->metacity_theme);
	    break;
	case GWD_SETTINGS_IMPL_PROPERTY_ACTIVE_OPACITY:
	    g_value_set_double (value, priv->metacity_active_opacity);
	    break;
	case GWD_SETTINGS_IMPL_PROPERTY_INACTIVE_OPACITY:
	    g_value_set_double (value, priv->metacity_inactive_opacity);
	    break;
	case GWD_SETTINGS_IMPL_PROPERTY_ACTIVE_SHADE_OPACITY:
	    g_value_set_boolean (value, priv->metacity_active_shade_opacity);
	    break;
	case GWD_SETTINGS_IMPL_PROPERTY_INACTIVE_SHADE_OPACITY:
	    g_value_set_boolean (value, priv->metacity_inactive_shade_opacity);
	    break;
	case GWD_SETTINGS_IMPL_PROPERTY_BUTTON_LAYOUT:
	    g_value_set_string (value, priv->metacity_button_layout);
	    break;
	case GWD_SETTINGS_IMPL_PROPERTY_TITLEBAR_ACTION_DOUBLE_CLICK:
	    g_value_set_int (value, priv->titlebar_double_click_action);
	    break;
	case GWD_SETTINGS_IMPL_PROPERTY_TITLEBAR_ACTION_MIDDLE_CLICK:
	    g_value_set_int (value, priv->titlebar_middle_click_action);
	    break;
	case GWD_SETTINGS_IMPL_PROPERTY_TITLEBAR_ACTION_RIGHT_CLICK:
	    g_value_set_int (value, priv->titlebar_right_click_action);
	    break;
	case GWD_SETTINGS_IMPL_PROPERTY_MOUSE_WHEEL_ACTION:
	    g_value_set_int (value, priv->mouse_wheel_action);
	    break;
	case GWD_SETTINGS_IMPL_PROPERTY_TITLEBAR_FONT:
	    g_value_set_string (value, priv->titlebar_font);
	    break;
	default:
	    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	    break;
    }
}

static void
gwd_settings_impl_class_init (GWDSettingsImplClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    g_type_class_add_private (klass, sizeof (GWDSettingsImplPrivate));

    object_class->dispose = gwd_settings_dispose;
    object_class->finalize = gwd_settings_finalize;
    object_class->get_property = gwd_settings_get_property;
    object_class->set_property = gwd_settings_set_property;

    g_object_class_override_property (object_class,
				      GWD_SETTINGS_IMPL_PROPERTY_ACTIVE_SHADOW,
				      "active-shadow");
    g_object_class_override_property (object_class,
				      GWD_SETTINGS_IMPL_PROPERTY_INACTIVE_SHADOW,
				      "inactive-shadow");
    g_object_class_override_property (object_class,
				      GWD_SETTINGS_IMPL_PROPERTY_USE_TOOLTIPS,
				      "use-tooltips");
    g_object_class_override_property (object_class,
				      GWD_SETTINGS_IMPL_PROPERTY_DRAGGABLE_BORDER_WIDTH,
				      "draggable-border-width");
    g_object_class_override_property (object_class,
				      GWD_SETTINGS_IMPL_PROPERTY_ATTACH_MODAL_DIALOGS,
				      "attach-modal-dialogs");
    g_object_class_override_property (object_class,
				      GWD_SETTINGS_IMPL_PROPERTY_BLUR_CHANGED,
				      "blur");
    g_object_class_override_property (object_class,
				      GWD_SETTINGS_IMPL_PROPERTY_METACITY_THEME,
				      "metacity-theme");
    g_object_class_override_property (object_class,
				      GWD_SETTINGS_IMPL_PROPERTY_ACTIVE_OPACITY,
				      "metacity-active-opacity");
    g_object_class_override_property (object_class,
				      GWD_SETTINGS_IMPL_PROPERTY_INACTIVE_OPACITY,
				      "metacity-inactive-opacity");
    g_object_class_override_property (object_class,
				      GWD_SETTINGS_IMPL_PROPERTY_ACTIVE_SHADE_OPACITY,
				      "metacity-active-shade-opacity");
    g_object_class_override_property (object_class,
				      GWD_SETTINGS_IMPL_PROPERTY_INACTIVE_SHADE_OPACITY,
				      "metacity-inactive-shade-opacity");
    g_object_class_override_property (object_class,
				      GWD_SETTINGS_IMPL_PROPERTY_BUTTON_LAYOUT,
				      "metacity-button-layout");
    g_object_class_override_property (object_class,
				      GWD_SETTINGS_IMPL_PROPERTY_TITLEBAR_ACTION_DOUBLE_CLICK,
				      "titlebar-double-click-action");
    g_object_class_override_property (object_class,
				      GWD_SETTINGS_IMPL_PROPERTY_TITLEBAR_ACTION_MIDDLE_CLICK,
				      "titlebar-middle-click-action");
    g_object_class_override_property (object_class,
				      GWD_SETTINGS_IMPL_PROPERTY_TITLEBAR_ACTION_RIGHT_CLICK,
				      "titlebar-right-click-action");
    g_object_class_override_property (object_class,
				      GWD_SETTINGS_IMPL_PROPERTY_MOUSE_WHEEL_ACTION,
				      "mouse-wheel-action");
    g_object_class_override_property (object_class,
				      GWD_SETTINGS_IMPL_PROPERTY_TITLEBAR_FONT,
				      "titlebar-font");
    g_object_class_install_property (object_class,
				     GWD_SETTINGS_IMPL_PROPERTY_CMDLINE_OPTIONS,
				     g_param_spec_int ("cmdline-options",
						       "Command line options",
						       "Which options were specified on the command line",
						       0,
						       G_MAXINT32,
						       0,
						       G_PARAM_READABLE |
						       G_PARAM_WRITABLE |
						       G_PARAM_CONSTRUCT_ONLY));
    g_object_class_install_property (object_class,
				     GWD_SETTINGS_IMPL_PROPERTY_SETTINGS_NOTIFIED,
				     g_param_spec_pointer ("settings-notified",
							   "GWDSettingsNotified",
							   "A GWDSettingsNotified which will be updated",
							   G_PARAM_WRITABLE |
							   G_PARAM_CONSTRUCT_ONLY));
}

static void gwd_settings_impl_init (GWDSettingsImpl *self)
{
    GWDSettingsImplPrivate *priv = GET_PRIVATE (self);

    priv->use_tooltips = USE_TOOLTIPS_DEFAULT;
    priv->active_shadow.shadow_radius = ACTIVE_SHADOW_RADIUS_DEFAULT;
    priv->active_shadow.shadow_opacity = ACTIVE_SHADOW_OPACITY_DEFAULT;
    priv->active_shadow.shadow_offset_x = ACTIVE_SHADOW_OFFSET_X_DEFAULT;
    priv->active_shadow.shadow_offset_y = ACTIVE_SHADOW_OFFSET_Y_DEFAULT;
    priv->active_shadow.shadow_color[0] = 0;
    priv->active_shadow.shadow_color[1] = 0;
    priv->active_shadow.shadow_color[2] = 0;
    priv->inactive_shadow.shadow_radius = INACTIVE_SHADOW_RADIUS_DEFAULT;
    priv->inactive_shadow.shadow_opacity = INACTIVE_SHADOW_OPACITY_DEFAULT;
    priv->inactive_shadow.shadow_offset_x = INACTIVE_SHADOW_OFFSET_X_DEFAULT;
    priv->inactive_shadow.shadow_offset_y = INACTIVE_SHADOW_OFFSET_Y_DEFAULT;
    priv->inactive_shadow.shadow_color[0] = 0;
    priv->inactive_shadow.shadow_color[1] = 0;
    priv->inactive_shadow.shadow_color[2] = 0;
    priv->draggable_border_width  = DRAGGABLE_BORDER_WIDTH_DEFAULT;
    priv->attach_modal_dialogs = ATTACH_MODAL_DIALOGS_DEFAULT;
    priv->blur_type = BLUR_TYPE_DEFAULT;
    priv->metacity_theme = g_strdup (METACITY_THEME_DEFAULT);
    priv->metacity_active_opacity = METACITY_ACTIVE_OPACITY_DEFAULT;
    priv->metacity_inactive_opacity = METACITY_INACTIVE_OPACITY_DEFAULT;
    priv->metacity_active_shade_opacity = METACITY_ACTIVE_SHADE_OPACITY_DEFAULT;
    priv->metacity_inactive_shade_opacity = METACITY_INACTIVE_SHADE_OPACITY_DEFAULT;
    priv->metacity_button_layout = g_strdup (METACITY_BUTTON_LAYOUT_DEFAULT);
    priv->titlebar_double_click_action = DOUBLE_CLICK_ACTION_DEFAULT;
    priv->titlebar_middle_click_action = MIDDLE_CLICK_ACTION_DEFAULT;
    priv->titlebar_right_click_action = RIGHT_CLICK_ACTION_DEFAULT;
    priv->mouse_wheel_action = WHEEL_ACTION_DEFAULT;
    priv->titlebar_font = g_strdup (TITLEBAR_FONT_DEFAULT);
    priv->cmdline_opts = 0;
    priv->notified = NULL;
    priv->freeze_count = 0;

    /* Append all notify funcs so that external state can be updated in case
     * the settings backend can't do it itself */
    append_to_notify_funcs (self, gwd_settings_notified_update_metacity_theme);
    append_to_notify_funcs (self, gwd_settings_notified_metacity_button_layout);
    append_to_notify_funcs (self, gwd_settings_notified_update_frames);
    append_to_notify_funcs (self, gwd_settings_notified_update_decorations);
}

static gboolean
set_blur_construction_value (gint	*blur,
			     GParameter *params,
			     GValue	*blur_value)
{
    if (blur)
    {
	g_value_set_int (blur_value, *blur);

	params->name = "blur";
	params->value = *blur_value;

	return TRUE;
    }

    return FALSE;
}

static gboolean
set_metacity_theme_construction_value (const gchar **metacity_theme,
				       GParameter  *params,
				       GValue	   *metacity_theme_value)
{
    if (metacity_theme)
    {
	g_value_set_string (metacity_theme_value, *metacity_theme);

	params->name = "metacity-theme";
	params->value = *metacity_theme_value;

	return TRUE;
    }

    return FALSE;
}

static guint
set_flag_and_increment (guint n_param,
			guint *flags,
			guint flag)
{
    if (!flags)
	return n_param;

    *flags |= flag;
    return n_param + 1;
}

GWDSettings *
gwd_settings_impl_new (gint                *blur,
		       const gchar         **metacity_theme,
		       GWDSettingsNotified *notified)
{
    /* Always N command line parameters + 2 for command line
     * options enum & notified */
    const guint     gwd_settings_impl_n_construction_params = 4;
    GParameter      param[gwd_settings_impl_n_construction_params];
    GWDSettings     *settings = NULL;

    int      n_param = 0;
    guint    cmdline_opts = 0;

    GValue blur_value = G_VALUE_INIT;
    GValue metacity_theme_value = G_VALUE_INIT;
    GValue cmdline_opts_value = G_VALUE_INIT;
    GValue settings_notified_value = G_VALUE_INIT;

    g_value_init (&blur_value, G_TYPE_INT);
    g_value_init (&metacity_theme_value, G_TYPE_STRING);
    g_value_init (&cmdline_opts_value, G_TYPE_INT);
    g_value_init (&settings_notified_value, G_TYPE_POINTER);

    if (set_blur_construction_value (blur, &param[n_param], &blur_value))
	n_param = set_flag_and_increment (n_param, &cmdline_opts, CMDLINE_BLUR);

    if (set_metacity_theme_construction_value (metacity_theme, &param[n_param], &metacity_theme_value))
	n_param = set_flag_and_increment (n_param, &cmdline_opts, CMDLINE_THEME);

    g_value_set_int (&cmdline_opts_value, cmdline_opts);

    param[n_param].name = "cmdline-options";
    param[n_param].value = cmdline_opts_value;

    n_param++;

    g_value_set_pointer (&settings_notified_value, notified);

    param[n_param].name = "settings-notified";
    param[n_param].value = settings_notified_value;

    n_param++;

    settings = GWD_SETTINGS_INTERFACE (g_object_newv (GWD_TYPE_SETTINGS_IMPL, n_param, param));

    g_value_unset (&blur_value);
    g_value_unset (&metacity_theme_value);
    g_value_unset (&cmdline_opts_value);

    return settings;
}
