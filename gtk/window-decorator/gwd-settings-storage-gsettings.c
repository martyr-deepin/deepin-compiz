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

#include <gio/gio.h>

#include <string.h>

#include "gwd-settings-writable-interface.h"
#include "gwd-settings-storage-interface.h"
#include "gwd-settings-storage-gsettings.h"

const gchar * ORG_COMPIZ_GWD = "org.compiz.gwd";
const gchar * ORG_GNOME_MUTTER = "org.gnome.mutter";
const gchar * ORG_GNOME_DESKTOP_WM_PREFERENCES = "org.gnome.desktop.wm.preferences";

const gchar * ORG_COMPIZ_GWD_KEY_USE_TOOLTIPS = "use-tooltips";
const gchar * ORG_COMPIZ_GWD_KEY_BLUR_TYPE = "blur-type";
const gchar * ORG_COMPIZ_GWD_KEY_METACITY_THEME_ACTIVE_OPACITY = "metacity-theme-active-opacity";
const gchar * ORG_COMPIZ_GWD_KEY_METACITY_THEME_INACTIVE_OPACITY = "metacity-theme-inactive-opacity";
const gchar * ORG_COMPIZ_GWD_KEY_METACITY_THEME_ACTIVE_SHADE_OPACITY = "metacity-theme-active-shade-opacity";
const gchar * ORG_COMPIZ_GWD_KEY_METACITY_THEME_INACTIVE_SHADE_OPACITY = "metacity-theme-inactive-shade-opacity";
const gchar * ORG_COMPIZ_GWD_KEY_USE_METACITY_THEME = "use-metacity-theme";
const gchar * ORG_COMPIZ_GWD_KEY_MOUSE_WHEEL_ACTION = "mouse-wheel-action";
const gchar * ORG_GNOME_DESKTOP_WM_PREFERENCES_ACTION_DOUBLE_CLICK_TITLEBAR = "action-double-click-titlebar";
const gchar * ORG_GNOME_DESKTOP_WM_PREFERENCES_ACTION_MIDDLE_CLICK_TITLEBAR = "action-middle-click-titlebar";
const gchar * ORG_GNOME_DESKTOP_WM_PREFERENCES_ACTION_RIGHT_CLICK_TITLEBAR = "action-right-click-titlebar";
const gchar * ORG_GNOME_DESKTOP_WM_PREFERENCES_THEME = "theme";
const gchar * ORG_GNOME_DESKTOP_WM_PREFERENCES_TITLEBAR_USES_SYSTEM_FONT = "titlebar-uses-system-font";
const gchar * ORG_GNOME_DESKTOP_WM_PREFERENCES_TITLEBAR_FONT = "titlebar-font";
const gchar * ORG_GNOME_DESKTOP_WM_PREFERENCES_BUTTON_LAYOUT = "button-layout";
const gchar * ORG_GNOME_MUTTER_ATTACH_MODAL_DIALOGS = "attach-modal-dialogs";
const gchar * ORG_GNOME_MUTTER_DRAGGABLE_BORDER_WIDTH = "draggable-border-width";

#define GWD_SETTINGS_STORAGE_GSETTINGS(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GWD_TYPE_SETTINGS_STORAGE_GSETTINGS, GWDSettingsStorageGSettings));
#define GWD_SETTINGS_STORAGE_GSETTINGS_CLASS(obj) (G_TYPE_CHECK_CLASS_CAST ((obj), GWD_TYPE_SETTINGS_STORAGE_GSETTINGS, GWDSettingsStorageGSettingsClass));
#define GWD_IS_MOCK_SETTINGS(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GWD_TYPE_SETTINGS_STORAGE_GSETTINGS));
#define GWD_IS_MOCK_SETTINGS_CLASS(obj) (G_TYPE_CHECK_CLASS_TYPE ((obj), GWD_TYPE_SETTINGS_STORAGE_GSETTINGS));
#define GWD_SETTINGS_STORAGE_GSETTINGS_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GWD_TYPE_SETTINGS_STORAGE_GSETTINGS, GWDSettingsStorageGSettingsClass));

typedef struct _GWDSettingsStorageGSettings
{
    GObject parent;
} GWDSettingsStorageGSettings;

typedef struct _GWDSettingsStorageGSettingsClass
{
    GObjectClass parent_class;
} GWDSettingsStorageGSettingsClass;

static void gwd_settings_storage_gsettings_interface_init (GWDSettingsStorageInterface *interface);

G_DEFINE_TYPE_WITH_CODE (GWDSettingsStorageGSettings, gwd_settings_storage_gsettings, G_TYPE_OBJECT,
			 G_IMPLEMENT_INTERFACE (GWD_TYPE_SETTINGS_STORAGE_INTERFACE,
						gwd_settings_storage_gsettings_interface_init))

#define GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), GWD_TYPE_SETTINGS_STORAGE_GSETTINGS, GWDSettingsStorageGSettingsPrivate))

enum
{
    GWD_SETTINGS_STORAGE_GSETTINGS_PROPERTY_DESKTOP_GSETTINGS = 1,
    GWD_SETTINGS_STORAGE_GSETTINGS_PROPERTY_MUTTER_GSETTINGS  = 2,
    GWD_SETTINGS_STORAGE_GSETTINGS_PROPERTY_GWD_GSETTINGS     = 3,
    GWD_SETTINGS_STORAGE_GSETTINGS_PROPERTY_WRITABLE_SETTINGS = 4
};

const guint GWD_SETTINGS_STORAGE_GSETTINGS_N_CONSTRUCTION_PARAMS = 4;

typedef struct _GWDSettingsStorageGSettingsPrivate
{
    GSettings *desktop;
    GSettings *mutter;
    GSettings *gwd;
    GWDSettingsWritable *writable;
} GWDSettingsStorageGSettingsPrivate;

static gboolean
gwd_settings_storage_gsettings_update_use_tooltips (GWDSettingsStorage *settings)
{
    GWDSettingsStorageGSettings	       *storage = GWD_SETTINGS_STORAGE_GSETTINGS (settings);
    GWDSettingsStorageGSettingsPrivate *priv = GET_PRIVATE (storage);

    if (!priv->gwd)
	return FALSE;

    return gwd_settings_writable_use_tooltips_changed (priv->writable,
						       g_settings_get_boolean (priv->gwd,
									       ORG_COMPIZ_GWD_KEY_USE_TOOLTIPS));
}

static gboolean
gwd_settings_storage_gsettings_update_draggable_border_width (GWDSettingsStorage *settings)
{
    GWDSettingsStorageGSettings	       *storage = GWD_SETTINGS_STORAGE_GSETTINGS (settings);
    GWDSettingsStorageGSettingsPrivate *priv = GET_PRIVATE (storage);

    if (!priv->mutter)
	return FALSE;

    return gwd_settings_writable_draggable_border_width_changed (priv->writable,
								 g_settings_get_int (priv->mutter,
										     ORG_GNOME_MUTTER_DRAGGABLE_BORDER_WIDTH));
}

static gboolean
gwd_settings_storage_gsettings_update_attach_modal_dialogs (GWDSettingsStorage *settings)
{
    GWDSettingsStorageGSettings	       *storage = GWD_SETTINGS_STORAGE_GSETTINGS (settings);
    GWDSettingsStorageGSettingsPrivate *priv = GET_PRIVATE (storage);

    if (!priv->mutter)
	return FALSE;

    return gwd_settings_writable_attach_modal_dialogs_changed (priv->writable,
							       g_settings_get_boolean (priv->mutter,
										       ORG_GNOME_MUTTER_ATTACH_MODAL_DIALOGS));
}

static gboolean
gwd_settings_storage_gsettings_update_blur (GWDSettingsStorage *settings)
{
    GWDSettingsStorageGSettings	       *storage = GWD_SETTINGS_STORAGE_GSETTINGS (settings);
    GWDSettingsStorageGSettingsPrivate *priv = GET_PRIVATE (storage);

    if (!priv->gwd)
	return FALSE;

    return gwd_settings_writable_blur_changed (priv->writable,
					       g_settings_get_string (priv->gwd,
								      ORG_COMPIZ_GWD_KEY_BLUR_TYPE));
}

static gboolean
gwd_settings_storage_gsettings_update_metacity_theme (GWDSettingsStorage *settings)
{
    GWDSettingsStorageGSettings	       *storage = GWD_SETTINGS_STORAGE_GSETTINGS (settings);
    GWDSettingsStorageGSettingsPrivate *priv = GET_PRIVATE (storage);

    if (!priv->gwd)
	return FALSE;

    if (!priv->desktop)
	return FALSE;

    return gwd_settings_writable_metacity_theme_changed (priv->writable,
							 g_settings_get_boolean (priv->gwd,
										 ORG_COMPIZ_GWD_KEY_USE_METACITY_THEME),
							 g_settings_get_string (priv->desktop,
										ORG_GNOME_DESKTOP_WM_PREFERENCES_THEME));
}

static gboolean
gwd_settings_storage_gsettings_update_opacity (GWDSettingsStorage *settings)
{
    GWDSettingsStorageGSettings	       *storage = GWD_SETTINGS_STORAGE_GSETTINGS (settings);
    GWDSettingsStorageGSettingsPrivate *priv = GET_PRIVATE (storage);

    if (!priv->gwd)
	return FALSE;

    return gwd_settings_writable_opacity_changed (priv->writable,
						  g_settings_get_double (priv->gwd,
									 ORG_COMPIZ_GWD_KEY_METACITY_THEME_ACTIVE_OPACITY),
						  g_settings_get_double (priv->gwd,
									 ORG_COMPIZ_GWD_KEY_METACITY_THEME_INACTIVE_OPACITY),
						  g_settings_get_boolean (priv->gwd,
									  ORG_COMPIZ_GWD_KEY_METACITY_THEME_ACTIVE_SHADE_OPACITY),
						  g_settings_get_boolean (priv->gwd,
									  ORG_COMPIZ_GWD_KEY_METACITY_THEME_INACTIVE_SHADE_OPACITY));
}

static gboolean
gwd_settings_storage_gsettings_update_button_layout (GWDSettingsStorage *settings)
{
    GWDSettingsStorageGSettings	       *storage = GWD_SETTINGS_STORAGE_GSETTINGS (settings);
    GWDSettingsStorageGSettingsPrivate *priv = GET_PRIVATE (storage);

    if (!priv->desktop)
	return FALSE;

    return gwd_settings_writable_button_layout_changed (priv->writable,
							g_settings_get_string (priv->desktop,
									       ORG_GNOME_DESKTOP_WM_PREFERENCES_BUTTON_LAYOUT));
}

static gboolean
gwd_settings_storage_gsettings_update_font (GWDSettingsStorage *settings)
{
    GWDSettingsStorageGSettings	       *storage = GWD_SETTINGS_STORAGE_GSETTINGS (settings);
    GWDSettingsStorageGSettingsPrivate *priv = GET_PRIVATE (storage);

    if (!priv->desktop)
	return FALSE;

    return gwd_settings_writable_font_changed (priv->writable,
					       g_settings_get_boolean (priv->desktop,
								       ORG_GNOME_DESKTOP_WM_PREFERENCES_TITLEBAR_USES_SYSTEM_FONT),
					       g_settings_get_string (priv->desktop,
								      ORG_GNOME_DESKTOP_WM_PREFERENCES_TITLEBAR_FONT));
}

static inline gchar *
translate_dashes_to_underscores (const gchar *original)
{
    gint i = 0;
    gchar *copy = g_strdup (original);

    if (!copy)
	return NULL;

    for (; i < strlen (copy); ++i)
    {
	if (copy[i] == '-')
	    copy[i] = '_';
    }

    return copy;
}

static gboolean
gwd_settings_storage_gsettings_update_titlebar_actions (GWDSettingsStorage *settings)
{
    GWDSettingsStorageGSettings	       *storage = GWD_SETTINGS_STORAGE_GSETTINGS (settings);
    GWDSettingsStorageGSettingsPrivate *priv = GET_PRIVATE (storage);

    if (!priv->desktop)
	return FALSE;

    if (!priv->gwd)
	return FALSE;

    gchar *double_click_action = translate_dashes_to_underscores (g_settings_get_string (priv->desktop,
											 ORG_GNOME_DESKTOP_WM_PREFERENCES_ACTION_DOUBLE_CLICK_TITLEBAR));
    gchar *middle_click_action = translate_dashes_to_underscores (g_settings_get_string (priv->desktop,
											 ORG_GNOME_DESKTOP_WM_PREFERENCES_ACTION_MIDDLE_CLICK_TITLEBAR));
    gchar *right_click_action = translate_dashes_to_underscores (g_settings_get_string (priv->desktop,
											 ORG_GNOME_DESKTOP_WM_PREFERENCES_ACTION_RIGHT_CLICK_TITLEBAR));

    return gwd_settings_writable_titlebar_actions_changed (priv->writable,
							   double_click_action,
							   middle_click_action,
							   right_click_action,
							   g_settings_get_string (priv->gwd,
										  ORG_COMPIZ_GWD_KEY_MOUSE_WHEEL_ACTION));

    if (double_click_action)
	g_free (double_click_action);

    if (middle_click_action)
	g_free (middle_click_action);

    if (right_click_action)
	g_free (right_click_action);
}

static void
gwd_settings_storage_gsettings_interface_init (GWDSettingsStorageInterface *interface)
{
    interface->update_use_tooltips = gwd_settings_storage_gsettings_update_use_tooltips;
    interface->update_draggable_border_width = gwd_settings_storage_gsettings_update_draggable_border_width;
    interface->update_attach_modal_dialogs = gwd_settings_storage_gsettings_update_attach_modal_dialogs;
    interface->update_blur = gwd_settings_storage_gsettings_update_blur;
    interface->update_metacity_theme = gwd_settings_storage_gsettings_update_metacity_theme;
    interface->update_opacity = gwd_settings_storage_gsettings_update_opacity;
    interface->update_button_layout = gwd_settings_storage_gsettings_update_button_layout;
    interface->update_font = gwd_settings_storage_gsettings_update_font;
    interface->update_titlebar_actions = gwd_settings_storage_gsettings_update_titlebar_actions;
}

static void
gwd_settings_storage_gsettings_set_property (GObject *object,
					     guint   property_id,
					     const GValue  *value,
					     GParamSpec *pspec)
{
    GWDSettingsStorageGSettingsPrivate *priv = GET_PRIVATE (object);

    switch (property_id)
    {
	case GWD_SETTINGS_STORAGE_GSETTINGS_PROPERTY_DESKTOP_GSETTINGS:
	    if (priv->desktop)
		g_object_unref (priv->desktop);

	    priv->desktop = g_value_dup_object (value);
	    break;
	case GWD_SETTINGS_STORAGE_GSETTINGS_PROPERTY_MUTTER_GSETTINGS:
	    if (priv->mutter)
		g_object_unref (priv->mutter);

	    priv->mutter = g_value_dup_object (value);
	    break;
	case GWD_SETTINGS_STORAGE_GSETTINGS_PROPERTY_GWD_GSETTINGS:
	    if (priv->gwd)
		g_object_unref (priv->gwd);

	    priv->gwd = g_value_dup_object (value);
	    break;
	case GWD_SETTINGS_STORAGE_GSETTINGS_PROPERTY_WRITABLE_SETTINGS:
	    priv->writable = g_value_get_pointer (value);
	    break;
	default:
	    g_assert_not_reached ();
    }
}

static void
gwd_settings_storage_gsettings_dispose (GObject *object)
{
    GWDSettingsStorageGSettingsPrivate *priv = GET_PRIVATE (object);

    G_OBJECT_CLASS (gwd_settings_storage_gsettings_parent_class)->dispose (object);

    if (priv->desktop)
	g_object_unref (priv->desktop);

    if (priv->mutter)
	g_object_unref (priv->mutter);

    if (priv->gwd)
	g_object_unref (priv->gwd);
}

static void
gwd_settings_storage_gsettings_finalize (GObject *object)
{
    G_OBJECT_CLASS (gwd_settings_storage_gsettings_parent_class)->finalize (object);
}

static void
gwd_settings_storage_gsettings_class_init (GWDSettingsStorageGSettingsClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    GParamSpec   *properties[] =
    {
	NULL,
	g_param_spec_object ("desktop-gsettings",
			     ORG_GNOME_DESKTOP_WM_PREFERENCES,
			     "GSettings Object for org.gnome.desktop.wm.preferences",
			     G_TYPE_SETTINGS,
			     G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY),
	g_param_spec_object ("mutter-gsettings",
			     ORG_GNOME_MUTTER,
			     "GSettings Object for org.gnome.mutter",
			     G_TYPE_SETTINGS,
			     G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY),
	g_param_spec_object ("gwd-gsettings",
			     ORG_COMPIZ_GWD,
			     "GSettings Object for org.compiz.gwd",
			     G_TYPE_SETTINGS,
			     G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY),
	g_param_spec_pointer ("writable-settings",
			      "GWDWritableSettings",
			      "A GWDWritableSettings object",
			      G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY)
    };

    g_type_class_add_private (klass, sizeof (GWDSettingsStorageGSettingsPrivate));

    object_class->dispose = gwd_settings_storage_gsettings_dispose;
    object_class->finalize = gwd_settings_storage_gsettings_finalize;
    object_class->set_property = gwd_settings_storage_gsettings_set_property;

    g_object_class_install_properties (object_class,
				       sizeof (properties) / sizeof (properties[0]),
				       properties);
}

void gwd_settings_storage_gsettings_init (GWDSettingsStorageGSettings *self)
{
}

GWDSettingsStorage *
gwd_settings_storage_gsettings_new (GSettings *desktop,
				    GSettings *mutter,
				    GSettings *gwd,
				    GWDSettingsWritable *writable)
{
    static const guint gwd_settings_storage_gsettings_n_construction_params = 4;
    GParameter         param[gwd_settings_storage_gsettings_n_construction_params];

    GValue desktop_value = G_VALUE_INIT;
    GValue mutter_value = G_VALUE_INIT;
    GValue gwd_value = G_VALUE_INIT;
    GValue writable_value = G_VALUE_INIT;

    GWDSettingsStorage *storage = NULL;

    g_return_val_if_fail (writable != NULL, NULL);

    g_value_init (&desktop_value, G_TYPE_OBJECT);
    g_value_init (&mutter_value, G_TYPE_OBJECT);
    g_value_init (&gwd_value, G_TYPE_OBJECT);
    g_value_init (&writable_value, G_TYPE_POINTER);

    g_value_take_object (&desktop_value, desktop);
    g_value_take_object (&mutter_value, mutter);
    g_value_take_object (&gwd_value, gwd);
    g_value_set_pointer (&writable_value, writable);

    param[0].name = "desktop-gsettings";
    param[0].value = desktop_value;
    param[1].name = "mutter-gsettings";
    param[1].value = mutter_value;
    param[2].name = "gwd-gsettings";
    param[2].value = gwd_value;
    param[3].name = "writable-settings";
    param[3].value = writable_value;

    storage = GWD_SETTINGS_STORAGE_INTERFACE (g_object_newv (GWD_TYPE_SETTINGS_STORAGE_GSETTINGS,
							     gwd_settings_storage_gsettings_n_construction_params,
							     param));

    g_value_unset (&desktop_value);
    g_value_unset (&mutter_value);
    g_value_unset (&gwd_value);
    g_value_unset (&writable_value);

    return storage;
}

/* Factory methods */

static gpointer
list_all_schemas (gpointer data)
{
    return (gpointer) g_settings_list_schemas ();
}

static inline GSettings *
get_settings_no_abort (const gchar *schema)
{
    static GOnce	   get_settings_once = G_ONCE_INIT;
    const  gchar * const * schemas;
    guint                  i = 0;

    g_once (&get_settings_once, list_all_schemas, NULL);
    schemas = (const gchar * const *) get_settings_once.retval;

    for (; schemas[i]; i++)
	if (g_strcmp0 (schema, schemas[i]) == 0)
	    return g_settings_new (schema);

    return NULL;
}

static void
org_compiz_gwd_settings_changed (GSettings   *settings,
				 const gchar *key,
				 gpointer    user_data)
{
    GWDSettingsStorage *storage = GWD_SETTINGS_STORAGE_INTERFACE (user_data);

    if (strcmp (key, ORG_COMPIZ_GWD_KEY_MOUSE_WHEEL_ACTION) == 0)
	gwd_settings_storage_update_titlebar_actions (storage);
    else if (strcmp (key, ORG_COMPIZ_GWD_KEY_BLUR_TYPE) == 0)
	gwd_settings_storage_update_blur (storage);
    else if (strcmp (key, ORG_COMPIZ_GWD_KEY_USE_METACITY_THEME) == 0)
	gwd_settings_storage_update_metacity_theme (storage);
    else if (strcmp (key, ORG_COMPIZ_GWD_KEY_METACITY_THEME_INACTIVE_OPACITY)	     == 0 ||
	     strcmp (key, ORG_COMPIZ_GWD_KEY_METACITY_THEME_INACTIVE_SHADE_OPACITY)  == 0 ||
	     strcmp (key, ORG_COMPIZ_GWD_KEY_METACITY_THEME_ACTIVE_OPACITY)          == 0 ||
	     strcmp (key, ORG_COMPIZ_GWD_KEY_METACITY_THEME_ACTIVE_SHADE_OPACITY)    == 0)
	gwd_settings_storage_update_opacity (storage);
    else if (strcmp (key, ORG_COMPIZ_GWD_KEY_USE_TOOLTIPS) == 0)
	gwd_settings_storage_update_use_tooltips (storage);
}

void
gwd_connect_org_compiz_gwd_settings (GSettings		*settings,
				     GWDSettingsStorage *storage)
{
    if (!settings)
	return;

    g_signal_connect (settings, "changed", (GCallback) org_compiz_gwd_settings_changed, storage);
}

GSettings *
gwd_get_org_compiz_gwd_settings ()
{
    return get_settings_no_abort (ORG_COMPIZ_GWD);
}

static void
org_gnome_mutter_settings_changed (GSettings   *settings,
				   const gchar *key,
				   gpointer    user_data)
{
    GWDSettingsStorage *storage = GWD_SETTINGS_STORAGE_INTERFACE (user_data);

    if (strcmp (key, ORG_GNOME_MUTTER_DRAGGABLE_BORDER_WIDTH) == 0)
	gwd_settings_storage_update_draggable_border_width (storage);
    else if (strcmp (key, ORG_GNOME_MUTTER_ATTACH_MODAL_DIALOGS) == 0)
	gwd_settings_storage_update_attach_modal_dialogs (storage);
}

void
gwd_connect_org_gnome_mutter_settings (GSettings	  *settings,
				       GWDSettingsStorage *storage)
{
    if (!settings)
	return;

    g_signal_connect (settings, "changed", (GCallback) org_gnome_mutter_settings_changed, storage);
}

GSettings *
gwd_get_org_gnome_mutter_settings ()
{
    return get_settings_no_abort (ORG_GNOME_MUTTER);
}

static void
org_gnome_desktop_wm_keybindings_settings_changed (GSettings   *settings,
						   const gchar *key,
						   gpointer    user_data)
{
    GWDSettingsStorage *storage = GWD_SETTINGS_STORAGE_INTERFACE (user_data);

    if (strcmp (key, ORG_GNOME_DESKTOP_WM_PREFERENCES_TITLEBAR_USES_SYSTEM_FONT) == 0 ||
	strcmp (key, ORG_GNOME_DESKTOP_WM_PREFERENCES_TITLEBAR_FONT) == 0)
	gwd_settings_storage_update_font (storage);
    else if (strcmp (key, ORG_GNOME_DESKTOP_WM_PREFERENCES_TITLEBAR_FONT) == 0)
	gwd_settings_storage_update_font (storage);
    else if (strcmp (key, ORG_GNOME_DESKTOP_WM_PREFERENCES_ACTION_DOUBLE_CLICK_TITLEBAR) == 0 ||
	     strcmp (key, ORG_GNOME_DESKTOP_WM_PREFERENCES_ACTION_MIDDLE_CLICK_TITLEBAR) == 0 ||
	     strcmp (key, ORG_GNOME_DESKTOP_WM_PREFERENCES_ACTION_RIGHT_CLICK_TITLEBAR) == 0)
	gwd_settings_storage_update_titlebar_actions (storage);
    else if (strcmp (key, ORG_GNOME_DESKTOP_WM_PREFERENCES_THEME) == 0)
	gwd_settings_storage_update_metacity_theme (storage);
    else if (strcmp (key, ORG_GNOME_DESKTOP_WM_PREFERENCES_BUTTON_LAYOUT) == 0)
	gwd_settings_storage_update_button_layout (storage);
}

void
gwd_connect_org_gnome_desktop_wm_preferences_settings (GSettings	  *settings,
						       GWDSettingsStorage *storage)
{
    if (!settings)
	return;

    g_signal_connect (settings, "changed",
		      (GCallback) org_gnome_desktop_wm_keybindings_settings_changed, storage);
}

GSettings *
gwd_get_org_gnome_desktop_wm_preferences_settings ()
{
    return get_settings_no_abort (ORG_GNOME_DESKTOP_WM_PREFERENCES);
}
