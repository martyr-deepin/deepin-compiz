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

#include "gwd-settings-notified-interface.h"
#include "compiz_gwd_mock_settings_notified.h"

#define GWD_MOCK_SETTINGS_NOTIFIED(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GWD_TYPE_MOCK_SETTINGS_NOTIFIED, GWDMockSettingsNotified));
#define GWD_MOCK_SETTINGS_NOTIFIED_CLASS(obj) (G_TYPE_CHECK_CLASS_CAST ((obj), GWD_TYPE_MOCK_SETTINGS_NOTIFIED, GWDMockSettingsNotifiedClass));
#define GWD_IS_MOCK_SETTINGS(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GWD_TYPE_MOCK_SETTINGS_NOTIFIED));
#define GWD_IS_MOCK_SETTINGS_CLASS(obj) (G_TYPE_CHECK_CLASS_TYPE ((obj), GWD_TYPE_MOCK_SETTINGS_NOTIFIED));
#define GWD_MOCK_SETTINGS_NOTIFIED_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GWD_TYPE_MOCK_SETTINGS_NOTIFIED, GWDMockSettingsNotifiedClass));

typedef struct _GWDMockSettingsNotified
{
    GObject parent;
} GWDMockSettingsNotified;

typedef struct _GWDMockSettingsNotifiedClass
{
    GObjectClass parent_class;
} GWDMockSettingsNotifiedClass;

static void gwd_mock_settings_notified_interface_init (GWDSettingsNotifiedInterface *interface);

G_DEFINE_TYPE_WITH_CODE (GWDMockSettingsNotified, gwd_mock_settings_notified, G_TYPE_OBJECT,
			 G_IMPLEMENT_INTERFACE (GWD_TYPE_SETTINGS_NOTIFIED_INTERFACE,
						gwd_mock_settings_notified_interface_init))

#define GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), GWD_TYPE_MOCK_SETTINGS_NOTIFIED, GWDMockSettingsNotifiedPrivate))

enum
{
    GWD_MOCK_SETTINGS_NOTIFIED_PROPERTY_GMOCK_INTERFACE = 1
};

typedef struct _GWDMockSettingsNotifiedPrivate
{
    GWDMockSettingsNotifiedGMockInterface *mock;
} GWDMockSettingsNotifiedPrivate;

gboolean
gwd_mock_settings_notified_update_decorations (GWDSettingsNotified *notified)
{
    GWDMockSettingsNotifiedGMockInterface *notifiedGMock = GET_PRIVATE (notified)->mock;
    return notifiedGMock->updateDecorations ();
}

gboolean
gwd_mock_settings_notified_update_frames (GWDSettingsNotified *notified)
{
    GWDMockSettingsNotifiedGMockInterface *notifiedGMock = GET_PRIVATE (notified)->mock;
    return notifiedGMock->updateFrames ();
}

gboolean
gwd_mock_settings_notified_update_metacity_theme (GWDSettingsNotified *notified)
{
    GWDMockSettingsNotifiedGMockInterface *notifiedGMock = GET_PRIVATE (notified)->mock;
    return notifiedGMock->updateMetacityTheme ();
}

gboolean
gwd_mock_settings_notified_update_metacity_button_layout (GWDSettingsNotified *notified)
{
    GWDMockSettingsNotifiedGMockInterface *notifiedGMock = GET_PRIVATE (notified)->mock;
    return notifiedGMock->updateMetacityButtonLayout ();
}
static void gwd_mock_settings_notified_interface_init (GWDSettingsNotifiedInterface *interface)
{
    interface->update_decorations = gwd_mock_settings_notified_update_decorations;
    interface->update_frames = gwd_mock_settings_notified_update_frames;
    interface->update_metacity_theme = gwd_mock_settings_notified_update_metacity_theme;
    interface->update_metacity_button_layout = gwd_mock_settings_notified_update_metacity_button_layout;
}

static GObject * gwd_mock_settings_notified_constructor (GType	type,
							 guint   n_construction_properties,
							 GObjectConstructParam *construction_properties)
{
    GObject *object = G_OBJECT_CLASS (gwd_mock_settings_notified_parent_class)->constructor (type, n_construction_properties, construction_properties);
    GWDMockSettingsNotifiedPrivate *priv = GET_PRIVATE (object);
    guint   i = 0;

    for (; i < n_construction_properties; ++i)
    {
	if (g_strcmp0 (construction_properties[i].pspec->name, "gmock-interface") == 0)
	{
	    priv->mock = reinterpret_cast <GWDMockSettingsNotifiedGMockInterface *> (g_value_get_pointer (construction_properties[i].value));
	}
	else
	    g_assert_not_reached ();
    }

    return object;
}

static void gwd_mock_settings_notified_set_property (GObject *object,
						     guint   property_id,
						     const GValue  *value,
						     GParamSpec *pspec)
{
    GWDMockSettingsNotifiedPrivate *priv = GET_PRIVATE (object);

    switch (property_id)
    {
	case GWD_MOCK_SETTINGS_NOTIFIED_PROPERTY_GMOCK_INTERFACE:
	    if (!priv->mock)
		priv->mock = reinterpret_cast <GWDMockSettingsNotifiedGMockInterface *> (g_value_get_pointer (value));
	    break;
	default:
	    g_assert_not_reached ();
    }
}

static void gwd_mock_settings_notified_dispose (GObject *object)
{
    GWDMockSettingsNotifiedGMockInterface *settingsGMock = GET_PRIVATE (object)->mock;
    G_OBJECT_CLASS (gwd_mock_settings_notified_parent_class)->dispose (object);
    settingsGMock->dispose ();
}

static void gwd_mock_settings_notified_finalize (GObject *object)
{
    GWDMockSettingsNotifiedGMockInterface *settingsGMock = GET_PRIVATE (object)->mock;
    G_OBJECT_CLASS (gwd_mock_settings_notified_parent_class)->finalize (object);
    settingsGMock->finalize ();
}

static void gwd_mock_settings_notified_class_init (GWDMockSettingsNotifiedClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    g_type_class_add_private (klass, sizeof (GWDMockSettingsNotifiedPrivate));

    object_class->dispose = gwd_mock_settings_notified_dispose;
    object_class->finalize = gwd_mock_settings_notified_finalize;
    object_class->constructor = gwd_mock_settings_notified_constructor;
    object_class->set_property = gwd_mock_settings_notified_set_property;

    g_object_class_install_property (object_class,
				     GWD_MOCK_SETTINGS_NOTIFIED_PROPERTY_GMOCK_INTERFACE,
				     g_param_spec_pointer ("gmock-interface",
							   "Google Mock Interface",
							   "Google Mock Interface",
							   static_cast <GParamFlags> (G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY)));
}

void gwd_mock_settings_notified_init (GWDMockSettingsNotified *self)
{
}

GWDSettingsNotified *
gwd_mock_settings_notified_new (GWDMockSettingsNotifiedGMockInterface *gmock)
{
    GValue gmock_interface_v = G_VALUE_INIT;

    g_value_init (&gmock_interface_v, G_TYPE_POINTER);

    g_value_set_pointer (&gmock_interface_v, reinterpret_cast <gpointer> (gmock));

    GParameter param[1] =
    {
	{ "gmock-interface", gmock_interface_v }
    };

    GWDSettingsNotified *writable = GWD_SETTINGS_NOTIFIED_INTERFACE (g_object_newv (GWD_TYPE_MOCK_SETTINGS_NOTIFIED, 1, param));

    g_value_unset (&gmock_interface_v);

    return writable;
}
