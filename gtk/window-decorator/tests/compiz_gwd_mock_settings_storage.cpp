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

#include "gwd-settings-storage-interface.h"
#include "compiz_gwd_mock_settings_storage.h"

#define GWD_MOCK_SETTINGS_STORAGE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GWD_TYPE_MOCK_SETTINGS_STORAGE, GWDMockSettingsStorage));
#define GWD_MOCK_SETTINGS_STORAGE_CLASS(obj) (G_TYPE_CHECK_CLASS_CAST ((obj), GWD_TYPE_MOCK_SETTINGS_STORAGE, GWDMockSettingsStorageClass));
#define GWD_IS_MOCK_SETTINGS(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GWD_TYPE_MOCK_SETTINGS_STORAGE));
#define GWD_IS_MOCK_SETTINGS_CLASS(obj) (G_TYPE_CHECK_CLASS_TYPE ((obj), GWD_TYPE_MOCK_SETTINGS_STORAGE));
#define GWD_MOCK_SETTINGS_STORAGE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GWD_TYPE_MOCK_SETTINGS_STORAGE, GWDMockSettingsStorageClass));

typedef struct _GWDMockSettingsStorage
{
    GObject parent;
} GWDMockSettingsStorage;

typedef struct _GWDMockSettingsStorageClass
{
    GObjectClass parent_class;
} GWDMockSettingsStorageClass;

static void gwd_mock_settings_storage_interface_init (GWDSettingsStorageInterface *interface);

G_DEFINE_TYPE_WITH_CODE (GWDMockSettingsStorage, gwd_mock_settings_storage, G_TYPE_OBJECT,
			 G_IMPLEMENT_INTERFACE (GWD_TYPE_SETTINGS_STORAGE_INTERFACE,
						gwd_mock_settings_storage_interface_init))

#define GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), GWD_TYPE_MOCK_SETTINGS_STORAGE, GWDMockSettingsStoragePrivate))

enum
{
    GWD_MOCK_SETTINGS_STORAGE_PROPERTY_GMOCK_INTERFACE = 1
};

typedef struct _GWDMockSettingsStoragePrivate
{
    GWDMockSettingsStorageGMockInterface *mock;
} GWDMockSettingsStoragePrivate;

gboolean gwd_mock_settings_storage_update_use_tooltips (GWDSettingsStorage *settings)
{
    GWDMockSettingsStorage *settingsStorageMock = GWD_MOCK_SETTINGS_STORAGE (settings);
    GWDMockSettingsStorageGMockInterface *settingsGMock = GET_PRIVATE (settingsStorageMock)->mock;
    return settingsGMock->updateUseTooltips ();
}

gboolean gwd_mock_settings_storage_update_draggable_border_width (GWDSettingsStorage *settings)
{
    GWDMockSettingsStorage *settingsStorageMock = GWD_MOCK_SETTINGS_STORAGE (settings);
    GWDMockSettingsStorageGMockInterface *settingsGMock = GET_PRIVATE (settingsStorageMock)->mock;
    return settingsGMock->updateDraggableBorderWidth ();
}

gboolean gwd_mock_settings_storage_update_attach_modal_dialogs (GWDSettingsStorage *settings)
{
    GWDMockSettingsStorage *settingsStorageMock = GWD_MOCK_SETTINGS_STORAGE (settings)
    GWDMockSettingsStorageGMockInterface *settingsGMock = GET_PRIVATE (settingsStorageMock)->mock;
    return settingsGMock->updateAttachModalDialogs ();
}

gboolean gwd_mock_settings_storage_update_blur (GWDSettingsStorage *settings)
{
    GWDMockSettingsStorage *settingsStorageMock = GWD_MOCK_SETTINGS_STORAGE (settings);
    GWDMockSettingsStorageGMockInterface *settingsGMock = GET_PRIVATE (settingsStorageMock)->mock;
    return settingsGMock->updateBlur ();
}

gboolean gwd_mock_settings_storage_update_metacity_theme (GWDSettingsStorage *settings)
{
    GWDMockSettingsStorage *settingsStorageMock = GWD_MOCK_SETTINGS_STORAGE (settings);
    GWDMockSettingsStorageGMockInterface *settingsGMock = GET_PRIVATE (settingsStorageMock)->mock;
    return settingsGMock->updateMetacityTheme ();
}

gboolean gwd_mock_settings_storage_update_opacity (GWDSettingsStorage *settings)
{
    GWDMockSettingsStorage *settingsStorageMock = GWD_MOCK_SETTINGS_STORAGE (settings);
    GWDMockSettingsStorageGMockInterface *settingsGMock = GET_PRIVATE (settingsStorageMock)->mock;
    return settingsGMock->updateOpacity ();
}

gboolean gwd_mock_settings_storage_update_button_layout (GWDSettingsStorage *settings)
{
    GWDMockSettingsStorage *settingsStorageMock = GWD_MOCK_SETTINGS_STORAGE (settings);
    GWDMockSettingsStorageGMockInterface *settingsGMock = GET_PRIVATE (settingsStorageMock)->mock;
    return settingsGMock->updateButtonLayout ();
}

gboolean gwd_mock_settings_storage_update_font (GWDSettingsStorage *settings)
{
    GWDMockSettingsStorage *settingsStorageMock = GWD_MOCK_SETTINGS_STORAGE (settings);
    GWDMockSettingsStorageGMockInterface *settingsGMock = GET_PRIVATE (settingsStorageMock)->mock;
    return settingsGMock->updateFont ();
}

gboolean gwd_mock_settings_storage_update_titlebar_actions (GWDSettingsStorage *settings)
{
    GWDMockSettingsStorage *settingsStorageMock = GWD_MOCK_SETTINGS_STORAGE (settings);
    GWDMockSettingsStorageGMockInterface *settingsGMock = GET_PRIVATE (settingsStorageMock)->mock;
    return settingsGMock->updateTitlebarActions ();
}

static void gwd_mock_settings_storage_interface_init (GWDSettingsStorageInterface *interface)
{
    interface->update_use_tooltips = gwd_mock_settings_storage_update_use_tooltips;
    interface->update_draggable_border_width = gwd_mock_settings_storage_update_draggable_border_width;
    interface->update_attach_modal_dialogs = gwd_mock_settings_storage_update_attach_modal_dialogs;
    interface->update_blur = gwd_mock_settings_storage_update_blur;
    interface->update_metacity_theme = gwd_mock_settings_storage_update_metacity_theme;
    interface->update_opacity = gwd_mock_settings_storage_update_opacity;
    interface->update_button_layout = gwd_mock_settings_storage_update_button_layout;
    interface->update_font = gwd_mock_settings_storage_update_font;
    interface->update_titlebar_actions = gwd_mock_settings_storage_update_titlebar_actions;
}

static void gwd_mock_settings_storage_set_property (GObject *object,
						     guint   property_id,
						     const GValue  *value,
						     GParamSpec *pspec)
{
    GWDMockSettingsStoragePrivate *priv = GET_PRIVATE (object);

    switch (property_id)
    {
	case GWD_MOCK_SETTINGS_STORAGE_PROPERTY_GMOCK_INTERFACE:
	    if (!priv->mock)
		priv->mock = reinterpret_cast <GWDMockSettingsStorageGMockInterface *> (g_value_get_pointer (value));
	    break;
	default:
	    g_assert_not_reached ();
    }
}

static void gwd_mock_settings_storage_dispose (GObject *object)
{
    GWDMockSettingsStorageGMockInterface *settingsGMock = GET_PRIVATE (object)->mock;
    G_OBJECT_CLASS (gwd_mock_settings_storage_parent_class)->dispose (object);
    settingsGMock->dispose ();
}

static void gwd_mock_settings_storage_finalize (GObject *object)
{
    GWDMockSettingsStorageGMockInterface *settingsGMock = GET_PRIVATE (object)->mock;
    G_OBJECT_CLASS (gwd_mock_settings_storage_parent_class)->finalize (object);
    settingsGMock->finalize ();
}

static void gwd_mock_settings_storage_class_init (GWDMockSettingsStorageClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    g_type_class_add_private (klass, sizeof (GWDMockSettingsStoragePrivate));

    object_class->dispose = gwd_mock_settings_storage_dispose;
    object_class->finalize = gwd_mock_settings_storage_finalize;
    object_class->set_property = gwd_mock_settings_storage_set_property;

    g_object_class_install_property (object_class,
				     GWD_MOCK_SETTINGS_STORAGE_PROPERTY_GMOCK_INTERFACE,
				     g_param_spec_pointer ("gmock-interface",
							   "Google Mock Interface",
							   "Google Mock Interface",
							   static_cast <GParamFlags> (G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY)));
}

void gwd_mock_settings_storage_init (GWDMockSettingsStorage *self)
{
}

GWDSettingsStorage *
gwd_mock_settings_storage_new (GWDMockSettingsStorageGMockInterface *gmock)
{
    GValue gmock_interface_v = G_VALUE_INIT;

    g_value_init (&gmock_interface_v, G_TYPE_POINTER);

    g_value_set_pointer (&gmock_interface_v, reinterpret_cast <gpointer> (gmock));

    GParameter param[1] =
    {
	{ "gmock-interface", gmock_interface_v }
    };

    GWDSettingsStorage *storage = GWD_SETTINGS_STORAGE_INTERFACE (g_object_newv (GWD_TYPE_MOCK_SETTINGS_STORAGE, 1, param));

    g_value_unset (&gmock_interface_v);

    return storage;
}
