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
#include <string.h>

#include <gconf/gconf-client.h>

#include "gwd-settings-writable-interface.h"
#include "gwd-settings-storage-interface.h"
#include "gwd-settings-storage-gconf.h"

#define GWD_SETTINGS_STORAGE_GCONF(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GWD_TYPE_SETTINGS_STORAGE_GCONF, GWDSettingsStorageGConf));
#define GWD_SETTINGS_STORAGE_GCONF_CLASS(obj) (G_TYPE_CHECK_CLASS_CAST ((obj), GWD_TYPE_SETTINGS_STORAGE_GCONF, GWDSettingsStorageGConfClass));
#define GWD_IS_MOCK_SETTINGS(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GWD_TYPE_SETTINGS_STORAGE_GCONF));
#define GWD_IS_MOCK_SETTINGS_CLASS(obj) (G_TYPE_CHECK_CLASS_TYPE ((obj), GWD_TYPE_SETTINGS_STORAGE_GCONF));
#define GWD_SETTINGS_STORAGE_GCONF_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GWD_TYPE_SETTINGS_STORAGE_GCONF, GWDSettingsStorageGConfClass));

#define MUTTER_GCONF_DEF "/apps/mutter/general"
#define METACITY_GCONF_DEF "/apps/metacity/general"
#define COMPIZ_GCONF_DEF "/apps/gwd"

const gchar * METACITY_GCONF_DIR = METACITY_GCONF_DEF;
const gchar * MUTTER_GCONF_DIR = MUTTER_GCONF_DEF;
const gchar * COMPIZ_GCONF_DIR = COMPIZ_GCONF_DEF;
const gchar * COMPIZ_USE_SYSTEM_FONT_KEY = METACITY_GCONF_DEF "/titlebar_uses_system_font";
const gchar * COMPIZ_TITLEBAR_FONT_KEY  = METACITY_GCONF_DEF "/titlebar_font";
const gchar * COMPIZ_DOUBLE_CLICK_TITLEBAR_KEY  = METACITY_GCONF_DEF "/action_double_click_titlebar";
const gchar * COMPIZ_MIDDLE_CLICK_TITLEBAR_KEY  = METACITY_GCONF_DEF "/action_middle_click_titlebar";
const gchar * COMPIZ_RIGHT_CLICK_TITLEBAR_KEY  = METACITY_GCONF_DEF "/action_right_click_titlebar";
const gchar * MUTTER_DRAGGABLE_BORDER_WIDTH_KEY = MUTTER_GCONF_DEF "/draggable_border_width";
const gchar * MUTTER_ATTACH_MODAL_DIALOGS_KEY = MUTTER_GCONF_DEF "/attach_modal_dialogs";
const gchar * META_THEME_KEY = METACITY_GCONF_DEF "/theme";
const gchar * META_BUTTON_LAYOUT_KEY = METACITY_GCONF_DEF "/button_layout";
const gchar * COMPIZ_USE_META_THEME_KEY = COMPIZ_GCONF_DEF "/use_metacity_theme";
const gchar * COMPIZ_META_THEME_OPACITY_KEY = COMPIZ_GCONF_DEF "/metacity_theme_opacity";
const gchar * COMPIZ_META_THEME_ACTIVE_OPACITY_KEY = COMPIZ_GCONF_DEF "/metacity_theme_active_opacity";
const gchar * COMPIZ_META_THEME_ACTIVE_OPACITY_SHADE_KEY = COMPIZ_GCONF_DEF "/metacity_theme_active_shade_opacity";
const gchar * COMPIZ_META_THEME_OPACITY_SHADE_KEY = COMPIZ_GCONF_DEF "/metacity_theme_shade_opacity";
const gchar * COMPIZ_BLUR_TYPE_KEY = COMPIZ_GCONF_DEF "/blur_type";
const gchar * COMPIZ_WHEEL_ACTION_KEY = COMPIZ_GCONF_DEF "/mouse_wheel_action";
const gchar * COMPIZ_USE_TOOLTIPS_KEY = COMPIZ_GCONF_DEF "/use_tooltips";

typedef struct _GWDSettingsStorageGConf
{
    GObject parent;
} GWDSettingsStorageGConf;

typedef struct _GWDSettingsStorageGConfClass
{
    GObjectClass parent_class;
} GWDSettingsStorageGConfClass;

static void gwd_settings_storage_gconf_interface_init (GWDSettingsStorageInterface *interface);

G_DEFINE_TYPE_WITH_CODE (GWDSettingsStorageGConf, gwd_settings_storage_gconf, G_TYPE_OBJECT,
			 G_IMPLEMENT_INTERFACE (GWD_TYPE_SETTINGS_STORAGE_INTERFACE,
						gwd_settings_storage_gconf_interface_init))

#define GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), GWD_TYPE_SETTINGS_STORAGE_GCONF, GWDSettingsStorageGConfPrivate))

enum
{
    GWD_SETTINGS_STORAGE_GCONF_PROPERTY_WRITABLE_SETTINGS = 1
};

typedef struct _GWDSettingsStorageGConfPrivate
{
    GConfClient         *client;
    GWDSettingsWritable *writable;
} GWDSettingsStorageGConfPrivate;

static gboolean
gwd_settings_storage_gconf_update_use_tooltips (GWDSettingsStorage *settings)
{
    GWDSettingsStorageGConf	   *storage = GWD_SETTINGS_STORAGE_GCONF (settings);
    GWDSettingsStorageGConfPrivate *priv = GET_PRIVATE (storage);

    return gwd_settings_writable_use_tooltips_changed (priv->writable,
						       gconf_client_get_bool (priv->client,
									      COMPIZ_USE_TOOLTIPS_KEY,
									      NULL));
}

static gboolean
gwd_settings_storage_gconf_update_draggable_border_width (GWDSettingsStorage *settings)
{
    GWDSettingsStorageGConf	   *storage = GWD_SETTINGS_STORAGE_GCONF (settings);
    GWDSettingsStorageGConfPrivate *priv = GET_PRIVATE (storage);

    return gwd_settings_writable_draggable_border_width_changed (priv->writable,
								 gconf_client_get_int (priv->client,
										       MUTTER_DRAGGABLE_BORDER_WIDTH_KEY,
										       NULL));
}

static gboolean
gwd_settings_storage_gconf_update_attach_modal_dialogs (GWDSettingsStorage *settings)
{
    GWDSettingsStorageGConf	   *storage = GWD_SETTINGS_STORAGE_GCONF (settings);
    GWDSettingsStorageGConfPrivate *priv = GET_PRIVATE (storage);

    return gwd_settings_writable_attach_modal_dialogs_changed (priv->writable,
							       gconf_client_get_bool (priv->client,
										      MUTTER_ATTACH_MODAL_DIALOGS_KEY,
										      NULL));
}

static gboolean
gwd_settings_storage_gconf_update_blur (GWDSettingsStorage *settings)
{
    GWDSettingsStorageGConf	   *storage = GWD_SETTINGS_STORAGE_GCONF (settings);
    GWDSettingsStorageGConfPrivate *priv = GET_PRIVATE (storage);

    gchar *value = gconf_client_get_string (priv->client,
					    COMPIZ_BLUR_TYPE_KEY,
					    NULL);

    gboolean ret =  gwd_settings_writable_blur_changed (priv->writable, value);

    if (value)
	g_free (value);

    return ret;
}

static gboolean
gwd_settings_storage_gconf_update_metacity_theme (GWDSettingsStorage *settings)
{
    GWDSettingsStorageGConf	   *storage = GWD_SETTINGS_STORAGE_GCONF (settings);
    GWDSettingsStorageGConfPrivate *priv = GET_PRIVATE (storage);

    gchar *theme = gconf_client_get_string (priv->client,
					    META_THEME_KEY,
					    NULL);

    gboolean ret =  gwd_settings_writable_metacity_theme_changed (priv->writable,
								  gconf_client_get_bool (priv->client,
											 COMPIZ_USE_META_THEME_KEY,
											 NULL),
								  theme);

    if (theme)
	g_free (theme);

    return ret;
}

static gboolean
gwd_settings_storage_gconf_update_opacity (GWDSettingsStorage *settings)
{
    GWDSettingsStorageGConf	   *storage = GWD_SETTINGS_STORAGE_GCONF (settings);
    GWDSettingsStorageGConfPrivate *priv = GET_PRIVATE (storage);

    return gwd_settings_writable_opacity_changed (priv->writable,
						  gconf_client_get_float (priv->client,
									  COMPIZ_META_THEME_ACTIVE_OPACITY_KEY,
									  NULL),
						  gconf_client_get_float (priv->client,
									  COMPIZ_META_THEME_OPACITY_KEY,
									  NULL),
						  gconf_client_get_bool (priv->client,
									 COMPIZ_META_THEME_ACTIVE_OPACITY_SHADE_KEY,
									 NULL),
						  gconf_client_get_bool (priv->client,
									 COMPIZ_META_THEME_OPACITY_SHADE_KEY,
									 NULL));
}

static gboolean
gwd_settings_storage_gconf_update_button_layout (GWDSettingsStorage *settings)
{
    GWDSettingsStorageGConf	   *storage = GWD_SETTINGS_STORAGE_GCONF (settings);
    GWDSettingsStorageGConfPrivate *priv = GET_PRIVATE (storage);

    gchar *button_layout = gconf_client_get_string (priv->client,
						    META_BUTTON_LAYOUT_KEY,
						    NULL);

    gboolean ret =  gwd_settings_writable_button_layout_changed (priv->writable,
							button_layout);

    if (button_layout)
	g_free (button_layout);

    return ret;
}

static gboolean
gwd_settings_storage_gconf_update_font (GWDSettingsStorage *settings)
{
    GWDSettingsStorageGConf	   *storage = GWD_SETTINGS_STORAGE_GCONF (settings);
    GWDSettingsStorageGConfPrivate *priv = GET_PRIVATE (storage);

    gchar *font = gconf_client_get_string (priv->client,
					   COMPIZ_TITLEBAR_FONT_KEY,
					   NULL);

    gboolean ret =  gwd_settings_writable_font_changed (priv->writable,
							gconf_client_get_bool (priv->client,
									       COMPIZ_USE_SYSTEM_FONT_KEY,
									       NULL),
							font);

    if (font)
	g_free (font);

    return ret;
}

static gboolean
gwd_settings_storage_gconf_update_titlebar_actions (GWDSettingsStorage *settings)
{
    GWDSettingsStorageGConf	   *storage = GWD_SETTINGS_STORAGE_GCONF (settings);
    GWDSettingsStorageGConfPrivate *priv = GET_PRIVATE (storage);

    gchar *double_click_action = gconf_client_get_string (priv->client,
							  COMPIZ_DOUBLE_CLICK_TITLEBAR_KEY,
							  NULL);
    gchar *middle_click_action = gconf_client_get_string (priv->client,
							  COMPIZ_MIDDLE_CLICK_TITLEBAR_KEY,
							  NULL);
    gchar *right_click_action = gconf_client_get_string (priv->client,
							 COMPIZ_DOUBLE_CLICK_TITLEBAR_KEY,
							 NULL);
    gchar *mouse_wheel_action = gconf_client_get_string (priv->client,
							 COMPIZ_WHEEL_ACTION_KEY,
							 NULL);

    gboolean ret =  gwd_settings_writable_titlebar_actions_changed (priv->writable,
								    double_click_action,
								    middle_click_action,
								    right_click_action,
								    mouse_wheel_action);

    if (double_click_action)
	g_free (double_click_action);

    if (middle_click_action)
	g_free (middle_click_action);

    if (right_click_action)
	g_free (right_click_action);

    if (mouse_wheel_action)
	g_free (mouse_wheel_action);

    return ret;
}

static void
gwd_settings_storage_gconf_interface_init (GWDSettingsStorageInterface *interface)
{
    interface->update_use_tooltips = gwd_settings_storage_gconf_update_use_tooltips;
    interface->update_draggable_border_width = gwd_settings_storage_gconf_update_draggable_border_width;
    interface->update_attach_modal_dialogs = gwd_settings_storage_gconf_update_attach_modal_dialogs;
    interface->update_blur = gwd_settings_storage_gconf_update_blur;
    interface->update_metacity_theme = gwd_settings_storage_gconf_update_metacity_theme;
    interface->update_opacity = gwd_settings_storage_gconf_update_opacity;
    interface->update_button_layout = gwd_settings_storage_gconf_update_button_layout;
    interface->update_font = gwd_settings_storage_gconf_update_font;
    interface->update_titlebar_actions = gwd_settings_storage_gconf_update_titlebar_actions;
}

static void
gwd_settings_storage_gconf_dispose (GObject *object)
{
    GWDSettingsStorageGConfPrivate *priv = GET_PRIVATE (object);

    G_OBJECT_CLASS (gwd_settings_storage_gconf_parent_class)->dispose (object);

    if (priv->client)
	g_object_unref (priv->client);

    if (priv->writable)
	g_object_unref (priv->writable);
}

static void
gwd_settings_storage_gconf_finalize (GObject *object)
{
    G_OBJECT_CLASS (gwd_settings_storage_gconf_parent_class)->finalize (object);
}

static void
gwd_settings_storage_gconf_set_property (GObject *object,
					 guint   property_id,
					 const GValue *value,
					 GParamSpec *pspec)
{
    GWDSettingsStorageGConfPrivate *priv = GET_PRIVATE (object);

    switch (property_id)
    {
	case GWD_SETTINGS_STORAGE_GCONF_PROPERTY_WRITABLE_SETTINGS:
	    g_return_if_fail (!priv->writable);
	    priv->writable = g_value_get_pointer (value);
	    break;
	default:
	    break;
    }
}

static void
gwd_settings_storage_gconf_class_init (GWDSettingsStorageGConfClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    g_type_class_add_private (klass, sizeof (GWDSettingsStorageGConfPrivate));

    object_class->dispose = gwd_settings_storage_gconf_dispose;
    object_class->finalize = gwd_settings_storage_gconf_finalize;
    object_class->set_property = gwd_settings_storage_gconf_set_property;

    g_object_class_install_property (object_class,
				     GWD_SETTINGS_STORAGE_GCONF_PROPERTY_WRITABLE_SETTINGS,
				     g_param_spec_pointer ("writable-settings",
							   "GWDSettingsWritable",
							   "An object that implements GWDSettingsWritable",
							   G_PARAM_WRITABLE |
							   G_PARAM_CONSTRUCT_ONLY));
}

static void
value_changed (GConfClient *client,
	       const gchar *key,
	       GConfValue  *value,
	       void        *data)
{
    GWDSettingsStorage      *storage = GWD_SETTINGS_STORAGE_INTERFACE (data);

    if (strcmp (key, COMPIZ_USE_SYSTEM_FONT_KEY) == 0 ||
	strcmp (key, COMPIZ_TITLEBAR_FONT_KEY) == 0)
	gwd_settings_storage_update_font (storage);
    else if (strcmp (key, COMPIZ_TITLEBAR_FONT_KEY) == 0)
	gwd_settings_storage_update_font (storage);
    else if (strcmp (key, COMPIZ_DOUBLE_CLICK_TITLEBAR_KEY) == 0 ||
	     strcmp (key, COMPIZ_MIDDLE_CLICK_TITLEBAR_KEY) == 0 ||
	     strcmp (key, COMPIZ_RIGHT_CLICK_TITLEBAR_KEY) == 0 ||
	     strcmp (key, COMPIZ_WHEEL_ACTION_KEY) == 0)
	gwd_settings_storage_update_titlebar_actions (storage);
    else if (strcmp (key, COMPIZ_BLUR_TYPE_KEY) == 0)
	gwd_settings_storage_update_blur (storage);
    else if (strcmp (key, COMPIZ_USE_META_THEME_KEY) == 0 ||
	     strcmp (key, META_THEME_KEY) == 0)
	gwd_settings_storage_update_metacity_theme (storage);
    else if (strcmp (key, META_BUTTON_LAYOUT_KEY) == 0)
	gwd_settings_storage_update_button_layout (storage);
    else if (strcmp (key, COMPIZ_META_THEME_OPACITY_KEY)	       == 0 ||
	     strcmp (key, COMPIZ_META_THEME_OPACITY_SHADE_KEY)	       == 0 ||
	     strcmp (key, COMPIZ_META_THEME_ACTIVE_OPACITY_KEY)        == 0 ||
	     strcmp (key, COMPIZ_META_THEME_ACTIVE_OPACITY_SHADE_KEY)  == 0)
	gwd_settings_storage_update_opacity (storage);
    else if (strcmp (key, MUTTER_DRAGGABLE_BORDER_WIDTH_KEY) == 0)
	gwd_settings_storage_update_draggable_border_width (storage);
    else if (strcmp (key, MUTTER_ATTACH_MODAL_DIALOGS_KEY) == 0)
	gwd_settings_storage_update_attach_modal_dialogs (storage);
    else if (strcmp (key, COMPIZ_USE_TOOLTIPS_KEY) == 0)
	gwd_settings_storage_update_use_tooltips (storage);
}

void
gwd_settings_storage_gconf_init (GWDSettingsStorageGConf *self)
{
    GWDSettingsStorageGConfPrivate *priv = GET_PRIVATE (self);

    priv->client = gconf_client_get_default ();

    gconf_client_add_dir (priv->client,
			  COMPIZ_GCONF_DIR,
			  GCONF_CLIENT_PRELOAD_ONELEVEL,
			  NULL);

    gconf_client_add_dir (priv->client,
			  METACITY_GCONF_DIR,
			  GCONF_CLIENT_PRELOAD_ONELEVEL,
			  NULL);

    gconf_client_add_dir (priv->client,
			  MUTTER_GCONF_DIR,
			  GCONF_CLIENT_PRELOAD_ONELEVEL,
			  NULL);

    g_signal_connect (G_OBJECT (priv->client),
		      "value_changed",
		      G_CALLBACK (value_changed),
		      self);
}

GWDSettingsStorage *
gwd_settings_storage_gconf_new (GWDSettingsWritable *writable)
{
    GValue             writable_value = G_VALUE_INIT;
    static const guint gwd_settings_storage_gconf_n_construction_params = 1;
    GParameter         param[gwd_settings_storage_gconf_n_construction_params];
    GWDSettingsStorage *storage = NULL;

    g_value_init (&writable_value, G_TYPE_POINTER);
    g_value_set_pointer (&writable_value, writable);

    param[0].name = "writable-settings";
    param[0].value = writable_value;

    storage = GWD_SETTINGS_STORAGE_INTERFACE (g_object_newv (GWD_TYPE_SETTINGS_STORAGE_GCONF, 1, param));

    g_value_unset (&writable_value);

    return storage;
}
