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
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <glib-object.h>

#include "gwd-settings-writable-interface.h"
#include "compiz_gwd_mock_settings_writable.h"

#define GWD_MOCK_SETTINGS_WRITABLE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GWD_TYPE_MOCK_SETTINGS_WRITABLE, GWDMockSettingsWritable));
#define GWD_MOCK_SETTINGS_WRITABLE_CLASS(obj) (G_TYPE_CHECK_CLASS_CAST ((obj), GWD_TYPE_MOCK_SETTINGS_WRITABLE, GWDMockSettingsWritableClass));
#define GWD_IS_MOCK_SETTINGS(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GWD_TYPE_MOCK_SETTINGS_WRITABLE));
#define GWD_IS_MOCK_SETTINGS_CLASS(obj) (G_TYPE_CHECK_CLASS_TYPE ((obj), GWD_TYPE_MOCK_SETTINGS_WRITABLE));
#define GWD_MOCK_SETTINGS_WRITABLE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GWD_TYPE_MOCK_SETTINGS_WRITABLE, GWDMockSettingsWritableClass));

typedef struct _GWDMockSettingsWritable
{
    GObject parent;
} GWDMockSettingsWritable;

typedef struct _GWDMockSettingsWritableClass
{
    GObjectClass parent_class;
} GWDMockSettingsWritableClass;

static void gwd_mock_settings_writable_interface_init (GWDSettingsWritableInterface *interface);

G_DEFINE_TYPE_WITH_CODE (GWDMockSettingsWritable, gwd_mock_settings_writable, G_TYPE_OBJECT,
			 G_IMPLEMENT_INTERFACE (GWD_TYPE_WRITABLE_SETTINGS_INTERFACE,
						gwd_mock_settings_writable_interface_init))

#define GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), GWD_TYPE_MOCK_SETTINGS_WRITABLE, GWDMockSettingsWritablePrivate))

enum
{
    GWD_MOCK_SETTINGS_WRITABLE_PROPERTY_GMOCK_INTERFACE = 1
};

typedef struct _GWDMockSettingsWritablePrivate
{
    GWDMockSettingsWritableGMockInterface *mock;
} GWDMockSettingsWritablePrivate;

void
gwd_mock_settings_writable_freeze_updates (GWDSettingsWritable *settings)
{
    GWDMockSettingsWritableGMockInterface *gmockSettingsWritable = GET_PRIVATE (settings)->mock;
    return gmockSettingsWritable->freezeUpdates ();
}

void
gwd_mock_settings_writable_thaw_updates (GWDSettingsWritable *settings)
{
    GWDMockSettingsWritableGMockInterface *gmockSettingsWritable = GET_PRIVATE (settings)->mock;
    return gmockSettingsWritable->thawUpdates ();
}

gboolean
gwd_mock_settings_writable_shadow_property_changed (GWDSettingsWritable *settings,
						    gdouble     active_shadow_radius,
						    gdouble     active_shadow_opacity,
						    gdouble     active_shadow_offset_x,
						    gdouble     active_shadow_offset_y,
						    const gchar	     *active_shadow_color,
						    gdouble     inactive_shadow_radius,
						    gdouble     inactive_shadow_opacity,
						    gdouble     inactive_shadow_offset_x,
						    gdouble     inactive_shadow_offset_y,
						    const gchar	     *inactive_shadow_color)
{
    GWDMockSettingsWritableGMockInterface *gmockSettingsWritable = GET_PRIVATE (settings)->mock;
    return gmockSettingsWritable->shadowPropertyChanged (active_shadow_radius,
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
gwd_mock_settings_writable_use_tooltips_changed (GWDSettingsWritable *settings,
						 gboolean    use_tooltips)
{
    GWDMockSettingsWritableGMockInterface *gmockSettingsWritable = GET_PRIVATE (settings)->mock;
    return gmockSettingsWritable->useTooltipsChanged (use_tooltips);
}

gboolean
gwd_mock_settings_writable_draggable_border_width_changed (GWDSettingsWritable *settings,
							   gint	 draggable_border_width)
{
    GWDMockSettingsWritableGMockInterface *gmockSettingsWritable = GET_PRIVATE (settings)->mock;
    return gmockSettingsWritable->draggableBorderWidthChanged (draggable_border_width);
}

gboolean
gwd_mock_settings_writable_attach_modal_dialogs_changed (GWDSettingsWritable *settings,
							 gboolean    attach_modal_dialogs)
{
    GWDMockSettingsWritableGMockInterface *gmockSettingsWritable = GET_PRIVATE (settings)->mock;
    return gmockSettingsWritable->attachModalDialogsChanged (attach_modal_dialogs);
}

gboolean
gwd_mock_settings_writable_blur_changed (GWDSettingsWritable *settings,
					 const gchar         *blur_type)
{
    GWDMockSettingsWritableGMockInterface *gmockSettingsWritable = GET_PRIVATE (settings)->mock;
    return gmockSettingsWritable->blurChanged (blur_type);
}

gboolean
gwd_mock_settings_writable_metacity_theme_changed (GWDSettingsWritable *settings,
						   gboolean	 use_metacity_theme,
						   const gchar *metacity_theme)
{
    GWDMockSettingsWritableGMockInterface *gmockSettingsWritable = GET_PRIVATE (settings)->mock;
    return gmockSettingsWritable->metacityThemeChanged (use_metacity_theme, metacity_theme);
}

gboolean
gwd_mock_settings_writable_opacity_changed (GWDSettingsWritable *settings,
					    gdouble inactive_opacity,
					    gdouble active_opacity,
					    gboolean inactive_shade_opacity,
					    gboolean active_shade_opacity)
{
    GWDMockSettingsWritableGMockInterface *gmockSettingsWritable = GET_PRIVATE (settings)->mock;
    return gmockSettingsWritable->opacityChanged (inactive_opacity, active_opacity, inactive_shade_opacity, active_shade_opacity);
}

gboolean
gwd_mock_settings_writable_button_layout_changed (GWDSettingsWritable *settings,
						  const gchar *button_layout)
{
    GWDMockSettingsWritableGMockInterface *gmockSettingsWritable = GET_PRIVATE (settings)->mock;
    return gmockSettingsWritable->buttonLayoutChanged (button_layout);
}

gboolean
gwd_mock_settings_writable_font_changed (GWDSettingsWritable *settings,
					 gboolean		titlebar_uses_system_font,
					 const gchar		*titlebar_font)
{
    GWDMockSettingsWritableGMockInterface *gmockSettingsWritable = GET_PRIVATE (settings)->mock;
    return gmockSettingsWritable->fontChanged (titlebar_uses_system_font, titlebar_font);
}

gboolean
gwd_mock_settings_writable_titlebar_actions_changed (GWDSettingsWritable *settings,
						     const gchar	   *action_double_click_titlebar,
						     const gchar	   *action_middle_click_titlebar,
						     const gchar	   *action_right_click_titlebar,
						     const gchar	   *mouse_wheel_action)
{
    GWDMockSettingsWritableGMockInterface *gmockSettingsWritable = GET_PRIVATE (settings)->mock;
    return gmockSettingsWritable->titlebarActionsChanged (action_double_click_titlebar,
							  action_middle_click_titlebar,
							  action_right_click_titlebar,
							  mouse_wheel_action);
}

static void gwd_mock_settings_writable_interface_init (GWDSettingsWritableInterface *interface)
{
    interface->shadow_property_changed = gwd_mock_settings_writable_shadow_property_changed;
    interface->use_tooltips_changed = gwd_mock_settings_writable_use_tooltips_changed;
    interface->draggable_border_width_changed = gwd_mock_settings_writable_draggable_border_width_changed;
    interface->attach_modal_dialogs_changed = gwd_mock_settings_writable_attach_modal_dialogs_changed;
    interface->blur_changed = gwd_mock_settings_writable_blur_changed;
    interface->metacity_theme_changed = gwd_mock_settings_writable_metacity_theme_changed;
    interface->opacity_changed = gwd_mock_settings_writable_opacity_changed;
    interface->button_layout_changed = gwd_mock_settings_writable_button_layout_changed;
    interface->font_changed = gwd_mock_settings_writable_font_changed;
    interface->titlebar_actions_changed = gwd_mock_settings_writable_titlebar_actions_changed;
    interface->freeze_updates = gwd_mock_settings_writable_freeze_updates;
    interface->thaw_updates = gwd_mock_settings_writable_thaw_updates;
}

static GObject * gwd_mock_settings_writable_constructor (GType	type,
							 guint   n_construction_properties,
							 GObjectConstructParam *construction_properties)
{
    GObject *object = G_OBJECT_CLASS (gwd_mock_settings_writable_parent_class)->constructor (type, n_construction_properties, construction_properties);
    GWDMockSettingsWritablePrivate *priv = GET_PRIVATE (object);
    guint   i = 0;

    for (; i < n_construction_properties; ++i)
    {
	if (g_strcmp0 (construction_properties[i].pspec->name, "gmock-interface") == 0)
	{
	    priv->mock = reinterpret_cast <GWDMockSettingsWritableGMockInterface *> (g_value_get_pointer (construction_properties[i].value));
	}
	else
	    g_assert_not_reached ();
    }

    return object;
}

static void gwd_mock_settings_writable_set_property (GObject *object,
						     guint   property_id,
						     const GValue  *value,
						     GParamSpec *pspec)
{
    GWDMockSettingsWritablePrivate *priv = GET_PRIVATE (object);

    switch (property_id)
    {
	case GWD_MOCK_SETTINGS_WRITABLE_PROPERTY_GMOCK_INTERFACE:
	    if (!priv->mock)
		priv->mock = reinterpret_cast <GWDMockSettingsWritableGMockInterface *> (g_value_get_pointer (value));
	    break;
	default:
	    g_assert_not_reached ();
    }
}

static void gwd_mock_settings_writable_dispose (GObject *object)
{
    GWDMockSettingsWritableGMockInterface *settingsGMock = GET_PRIVATE (object)->mock;
    G_OBJECT_CLASS (gwd_mock_settings_writable_parent_class)->dispose (object);
    settingsGMock->dispose ();
}

static void gwd_mock_settings_writable_finalize (GObject *object)
{
    GWDMockSettingsWritableGMockInterface *settingsGMock = GET_PRIVATE (object)->mock;
    G_OBJECT_CLASS (gwd_mock_settings_writable_parent_class)->finalize (object);
    settingsGMock->finalize ();
}

static void gwd_mock_settings_writable_class_init (GWDMockSettingsWritableClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    g_type_class_add_private (klass, sizeof (GWDMockSettingsWritablePrivate));

    object_class->dispose = gwd_mock_settings_writable_dispose;
    object_class->finalize = gwd_mock_settings_writable_finalize;
    object_class->constructor = gwd_mock_settings_writable_constructor;
    object_class->set_property = gwd_mock_settings_writable_set_property;

    g_object_class_install_property (object_class,
				     GWD_MOCK_SETTINGS_WRITABLE_PROPERTY_GMOCK_INTERFACE,
				     g_param_spec_pointer ("gmock-interface",
							   "Google Mock Interface",
							   "Google Mock Interface",
							   static_cast <GParamFlags> (G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY)));
}

void gwd_mock_settings_writable_init (GWDMockSettingsWritable *self)
{
}

GWDSettingsWritable *
gwd_mock_settings_writable_new (GWDMockSettingsWritableGMockInterface *gmock)
{
    GValue gmock_interface_v = G_VALUE_INIT;

    g_value_init (&gmock_interface_v, G_TYPE_POINTER);

    g_value_set_pointer (&gmock_interface_v, reinterpret_cast <gpointer> (gmock));

    GParameter param[1] =
    {
	{ "gmock-interface", gmock_interface_v }
    };

    GWDSettingsWritable *writable = GWD_SETTINGS_WRITABLE_INTERFACE (g_object_newv (GWD_TYPE_MOCK_SETTINGS_WRITABLE, 1, param));

    g_value_unset (&gmock_interface_v);

    return writable;
}
