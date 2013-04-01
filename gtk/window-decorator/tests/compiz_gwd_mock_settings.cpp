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

#include "gwd-settings-interface.h"
#include "compiz_gwd_mock_settings.h"

#define GWD_MOCK_SETTINGS(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GWD_TYPE_MOCK_SETTINGS, GWDMockSettings));
#define GWD_MOCK_SETTINGS_CLASS(obj) (G_TYPE_CHECK_CLASS_CAST ((obj), GWD_TYPE_MOCK_SETTINGS, GWDMockSettingsClass));
#define GWD_IS_MOCK_SETTINGS(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GWD_TYPE_MOCK_SETTINGS));
#define GWD_IS_MOCK_SETTINGS_CLASS(obj) (G_TYPE_CHECK_CLASS_TYPE ((obj), GWD_TYPE_MOCK_SETTINGS));
#define GWD_MOCK_SETTINGS_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GWD_TYPE_MOCK_SETTINGS, GWDMockSettingsClass));

typedef struct _GWDMockSettings
{
    GObject parent;
} GWDMockSettings;

typedef struct _GWDMockSettingsClass
{
    GObjectClass parent_class;
} GWDMockSettingsClass;

static void gwd_mock_settings_interface_init (GWDSettingsInterface *interface);

G_DEFINE_TYPE_WITH_CODE (GWDMockSettings, gwd_mock_settings, G_TYPE_OBJECT,
			 G_IMPLEMENT_INTERFACE (GWD_TYPE_SETTINGS_INTERFACE,
						gwd_mock_settings_interface_init))

#define GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), GWD_TYPE_MOCK_SETTINGS, GWDMockSettingsPrivate))

typedef struct _GWDMockSettingsPrivate
{
    GWDMockSettingsGMockInterface *mock;
} GWDMockSettingsPrivate;

static void gwd_mock_settings_interface_init (GWDSettingsInterface *interface)
{
}

static void gwd_mock_settings_dispose (GObject *object)
{
    GWDMockSettingsGMockInterface *settingsGMock = GET_PRIVATE (object)->mock;
    G_OBJECT_CLASS (gwd_mock_settings_parent_class)->dispose (object);
    settingsGMock->dispose ();
}

static void gwd_mock_settings_finalize (GObject *object)
{
    GWDMockSettingsGMockInterface *settingsGMock = GET_PRIVATE (object)->mock;
    G_OBJECT_CLASS (gwd_mock_settings_parent_class)->finalize (object);
    settingsGMock->finalize ();
}

static void gwd_mock_settings_set_property (GObject *object,
					    guint   property_id,
					    const GValue  *value,
					    GParamSpec *pspec)
{
    GWDMockSettingsPrivate *priv = GET_PRIVATE (object);

    switch (property_id)
    {
	case GWD_MOCK_SETTINGS_PROPERTY_GMOCK_INTERFACE:
	    if (!priv->mock)
		priv->mock = reinterpret_cast <GWDMockSettingsGMockInterface *> (g_value_get_pointer (value));
	    break;
	case GWD_MOCK_SETTINGS_PROPERTY_BLUR_CHANGED:
	case GWD_MOCK_SETTINGS_PROPERTY_ACTIVE_OPACITY:
	case GWD_MOCK_SETTINGS_PROPERTY_ACTIVE_SHADE_OPACITY:
	case GWD_MOCK_SETTINGS_PROPERTY_INACTIVE_OPACITY:
	case GWD_MOCK_SETTINGS_PROPERTY_INACTIVE_SHADE_OPACITY:
	case GWD_MOCK_SETTINGS_PROPERTY_METACITY_THEME:
	    break;
	default:
	    g_assert_not_reached ();
	    break;
    }
}

static void gwd_mock_settings_get_property (GObject *object,
					    guint   property_id,
					    GValue  *value,
					    GParamSpec *pspec)
{
    GWDMockSettingsGMockInterface *settingsGMock = GET_PRIVATE (object)->mock;
    settingsGMock->getProperty (property_id, value, pspec);
}

static void gwd_mock_settings_class_init (GWDMockSettingsClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    g_type_class_add_private (klass, sizeof (GWDMockSettingsPrivate));

    object_class->dispose = gwd_mock_settings_dispose;
    object_class->finalize = gwd_mock_settings_finalize;
    object_class->get_property = gwd_mock_settings_get_property;
    object_class->set_property = gwd_mock_settings_set_property;

    g_object_class_override_property (object_class,
				      GWD_MOCK_SETTINGS_PROPERTY_ACTIVE_SHADOW,
				      "active-shadow");
    g_object_class_override_property (object_class,
				      GWD_MOCK_SETTINGS_PROPERTY_INACTIVE_SHADOW,
				      "inactive-shadow");
    g_object_class_override_property (object_class,
				      GWD_MOCK_SETTINGS_PROPERTY_USE_TOOLTIPS,
				      "use-tooltips");
    g_object_class_override_property (object_class,
				      GWD_MOCK_SETTINGS_PROPERTY_DRAGGABLE_BORDER_WIDTH,
				      "draggable-border-width");
    g_object_class_override_property (object_class,
				      GWD_MOCK_SETTINGS_PROPERTY_ATTACH_MODAL_DIALOGS,
				      "attach-modal-dialogs");
    g_object_class_override_property (object_class,
				      GWD_MOCK_SETTINGS_PROPERTY_BLUR_CHANGED,
				      "blur");
    g_object_class_override_property (object_class,
				      GWD_MOCK_SETTINGS_PROPERTY_METACITY_THEME,
				      "metacity-theme");
    g_object_class_override_property (object_class,
				      GWD_MOCK_SETTINGS_PROPERTY_ACTIVE_OPACITY,
				      "metacity-active-opacity");
    g_object_class_override_property (object_class,
				      GWD_MOCK_SETTINGS_PROPERTY_INACTIVE_OPACITY,
				      "metacity-inactive-opacity");
    g_object_class_override_property (object_class,
				      GWD_MOCK_SETTINGS_PROPERTY_ACTIVE_SHADE_OPACITY,
				      "metacity-active-shade-opacity");
    g_object_class_override_property (object_class,
				      GWD_MOCK_SETTINGS_PROPERTY_INACTIVE_SHADE_OPACITY,
				      "metacity-inactive-shade-opacity");
    g_object_class_override_property (object_class,
				      GWD_MOCK_SETTINGS_PROPERTY_BUTTON_LAYOUT,
				      "metacity-button-layout");
    g_object_class_override_property (object_class,
				      GWD_MOCK_SETTINGS_PROPERTY_TITLEBAR_ACTION_DOUBLE_CLICK,
				      "titlebar-double-click-action");
    g_object_class_override_property (object_class,
				      GWD_MOCK_SETTINGS_PROPERTY_TITLEBAR_ACTION_MIDDLE_CLICK,
				      "titlebar-middle-click-action");
    g_object_class_override_property (object_class,
				      GWD_MOCK_SETTINGS_PROPERTY_TITLEBAR_ACTION_RIGHT_CLICK,
				      "titlebar-right-click-action");
    g_object_class_override_property (object_class,
				      GWD_MOCK_SETTINGS_PROPERTY_MOUSE_WHEEL_ACTION,
				      "mouse-wheel-action");
    g_object_class_override_property (object_class,
				      GWD_MOCK_SETTINGS_PROPERTY_TITLEBAR_FONT,
				      "titlebar-font");
    g_object_class_install_property (object_class,
				     GWD_MOCK_SETTINGS_PROPERTY_GMOCK_INTERFACE,
				     g_param_spec_pointer ("gmock-interface",
							   "Google Mock Interface",
							   "Google Mock Interface",
							   static_cast <GParamFlags> (G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY)));
}

static void gwd_mock_settings_init (GWDMockSettings *self)
{
}

GWDSettings *
gwd_mock_settings_new (GWDMockSettingsGMockInterface *gmock)
{
    GValue gmock_interface_v = G_VALUE_INIT;

    g_value_init (&gmock_interface_v, G_TYPE_POINTER);

    g_value_set_pointer (&gmock_interface_v, reinterpret_cast <gpointer> (gmock));

    GParameter param[1] =
    {
	{ "gmock-interface", gmock_interface_v }
    };

    GWDSettings *settings = GWD_SETTINGS_INTERFACE (g_object_newv (GWD_TYPE_MOCK_SETTINGS, 1, param));

    g_value_unset (&gmock_interface_v);

    return settings;
}


