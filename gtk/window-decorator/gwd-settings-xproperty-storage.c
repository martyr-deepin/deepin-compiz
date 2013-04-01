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

#include "gtk-window-decorator.h"

#include "gwd-settings-writable-interface.h"
#include "gwd-settings-xproperty-interface.h"
#include "gwd-settings-xproperty-storage.h"

#define GWD_SETTINGS_STORAGE_XPROP(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GWD_TYPE_SETTINGS_STORAGE_XPROP, GWDSettingsStorageXProp));
#define GWD_SETTINGS_STORAGE_XPROP_CLASS(obj) (G_TYPE_CHECK_CLASS_CAST ((obj), GWD_TYPE_SETTINGS_STORAGE_XPROP, GWDSettingsStorageXPropClass));
#define GWD_IS_MOCK_SETTINGS(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GWD_TYPE_SETTINGS_STORAGE_XPROP));
#define GWD_IS_MOCK_SETTINGS_CLASS(obj) (G_TYPE_CHECK_CLASS_TYPE ((obj), GWD_TYPE_SETTINGS_STORAGE_XPROP));
#define GWD_SETTINGS_STORAGE_XPROP_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GWD_TYPE_SETTINGS_STORAGE_XPROP, GWDSettingsStorageXPropClass));

typedef struct _GWDSettingsStorageXProp
{
    GObject parent;
} GWDSettingsStorageXProp;

typedef struct _GWDSettingsStorageXPropClass
{
    GObjectClass parent_class;
} GWDSettingsStorageXPropClass;

static void gwd_settings_storage_xprop_interface_init (GWDSettingsXPropertyStorageInterface *interface);

G_DEFINE_TYPE_WITH_CODE (GWDSettingsStorageXProp, gwd_settings_storage_xprop, G_TYPE_OBJECT,
			 G_IMPLEMENT_INTERFACE (GWD_TYPE_XPROPERTY_SETTINGS_STORAGE_INTERFACE,
						gwd_settings_storage_xprop_interface_init))

#define GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), GWD_TYPE_SETTINGS_STORAGE_XPROP, GWDSettingsStorageXPropPrivate))

enum
{
    GWD_SETTINGS_STORAGE_XPROP_PROPERTY_WRITABLE_SETTINGS = 1,
    GWD_SETTINGS_STORAGE_XPROP_PROPERTY_DISPLAY = 2,
    GWD_SETTINGS_STORAGE_XPROP_PROPERTY_ROOT = 3
};

typedef struct _GWDSettingsStorageXPropPrivate
{
    Display             *xdpy;
    Window              root;
    GWDSettingsWritable *writable;

} GWDSettingsStorageXPropPrivate;

static gboolean
gwd_settings_storage_xprop_update_all (GWDSettingsXPropertyStorage *storage)
{
    GWDSettingsStorageXProp        *xprop = GWD_SETTINGS_STORAGE_XPROP (storage);
    GWDSettingsStorageXPropPrivate *priv = GET_PRIVATE (xprop);

    Atom          actual;
    int           result, format;
    unsigned long n, left;
    unsigned char *prop_data;
    XTextProperty shadow_color_xtp;

    gdouble aradius;
    gdouble aopacity;
    gint    ax_off;
    gint    ay_off;
    char    *active_shadow_color = NULL;

    gdouble iradius;
    gdouble iopacity;
    gint    ix_off;
    gint    iy_off;
    char    *inactive_shadow_color = NULL;

    result = XGetWindowProperty (priv->xdpy, priv->root, compiz_shadow_info_atom,
				 0, 32768, 0, XA_INTEGER, &actual,
				 &format, &n, &left, &prop_data);

    if (result != Success)
	return FALSE;

    if (n == 8)
    {
	long *data      = (long *) prop_data;
	aradius  = data[0];
	aopacity = data[1];
	ax_off      = data[2];
	ay_off      = data[3];

	iradius  = data[4];
	iopacity = data[5];
	ix_off      = data[6];
	iy_off      = data[7];
	/* Radius and Opacity are multiplied by 1000 to keep precision,
	 * divide by that much to get our real radius and opacity
	 */
	aradius /= 1000;
	aopacity /= 1000;
	iradius /= 1000;
	iopacity /= 1000;

	XFree (prop_data);
    }
    else
    {
	XFree (prop_data);
	return FALSE;
    }

    result = XGetTextProperty (priv->xdpy, priv->root, &shadow_color_xtp,
			       compiz_shadow_color_atom);

    if (shadow_color_xtp.value)
    {
	int  ret_count = 0;
	char **t_data = NULL;

	XTextPropertyToStringList (&shadow_color_xtp, &t_data, &ret_count);

	if (ret_count == 2)
	{
	    active_shadow_color = strdup (t_data[0]);
	    inactive_shadow_color = strdup (t_data[1]);

	    XFree (shadow_color_xtp.value);
	    if (t_data)
		XFreeStringList (t_data);
	}
	else
	{
	    XFree (shadow_color_xtp.value);
	    return FALSE;
	}
    }

    return gwd_settings_writable_shadow_property_changed (priv->writable,
							  (gdouble) MAX (0.0, MIN (aradius, 48.0)),
							  (gdouble) MAX (0.0, MIN (aopacity, 6.0)),
							  (gdouble) MAX (-16, MIN (ax_off, 16)),
							  (gdouble) MAX (-16, MIN (ay_off, 16)),
							  active_shadow_color,
							  (gdouble) MAX (0.0, MIN (iradius, 48.0)),
							  (gdouble) MAX (0.0, MIN (iopacity, 6.0)),
							  (gdouble) MAX (-16, MIN (ix_off, 16)),
							  (gdouble) MAX (-16, MIN (iy_off, 16)),
							  inactive_shadow_color);
}

static void
gwd_settings_storage_xprop_interface_init (GWDSettingsXPropertyStorageInterface *interface)
{
    interface->update_all = gwd_settings_storage_xprop_update_all;
}

static void
gwd_settings_storage_xprop_dispose (GObject *object)
{
    GWDSettingsStorageXPropPrivate *priv = GET_PRIVATE (object);

    G_OBJECT_CLASS (gwd_settings_storage_xprop_parent_class)->dispose (object);

    if (priv->writable)
	g_object_unref (priv->writable);
}

static void
gwd_settings_storage_xprop_finalize (GObject *object)
{
    G_OBJECT_CLASS (gwd_settings_storage_xprop_parent_class)->finalize (object);
}

static void
gwd_settings_storage_xprop_set_property (GObject *object,
					 guint   property_id,
					 const GValue *value,
					 GParamSpec *pspec)
{
    GWDSettingsStorageXPropPrivate *priv = GET_PRIVATE (object);

    switch (property_id)
    {
	case GWD_SETTINGS_STORAGE_XPROP_PROPERTY_WRITABLE_SETTINGS:
	    g_return_if_fail (!priv->writable);
	    priv->writable = g_value_get_pointer (value);
	    break;
	case GWD_SETTINGS_STORAGE_XPROP_PROPERTY_DISPLAY:
	    g_return_if_fail (!priv->xdpy);
	    priv->xdpy = (Display *) g_value_get_pointer (value);
	    break;
	case GWD_SETTINGS_STORAGE_XPROP_PROPERTY_ROOT:
	    g_return_if_fail (!priv->root);
	    priv->root = (Window) g_value_get_int (value);
	    break;
	default:
	    break;
    }
}

static void
gwd_settings_storage_xprop_class_init (GWDSettingsStorageXPropClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    g_type_class_add_private (klass, sizeof (GWDSettingsStorageXPropPrivate));

    object_class->dispose = gwd_settings_storage_xprop_dispose;
    object_class->finalize = gwd_settings_storage_xprop_finalize;
    object_class->set_property = gwd_settings_storage_xprop_set_property;

    g_object_class_install_property (object_class,
				     GWD_SETTINGS_STORAGE_XPROP_PROPERTY_WRITABLE_SETTINGS,
				     g_param_spec_pointer ("writable-settings",
							   "GWDSettingsWritable",
							   "An object that implements GWDSettingsWritable",
							   G_PARAM_WRITABLE |
							   G_PARAM_CONSTRUCT_ONLY));
    g_object_class_install_property (object_class,
				     GWD_SETTINGS_STORAGE_XPROP_PROPERTY_DISPLAY,
				     g_param_spec_pointer ("display",
							   "A Display",
							   "An Xlib connection",
							   G_PARAM_WRITABLE |
							   G_PARAM_CONSTRUCT_ONLY));
    g_object_class_install_property (object_class,
				     GWD_SETTINGS_STORAGE_XPROP_PROPERTY_ROOT,
				     g_param_spec_int ("root-window",
						       "Root Window",
						       "A Window which is the root window to store properties on",
						       0,
						       G_MAXINT32,
						       0,
						       G_PARAM_WRITABLE |
						       G_PARAM_CONSTRUCT_ONLY));
}

static void
gwd_settings_storage_xprop_init (GWDSettingsStorageXProp *self)
{
}

GWDSettingsXPropertyStorage *
gwd_settings_storage_xprop_new (Display *dpy,
				Window  root,
				GWDSettingsWritable *writable)
{
    static const guint          gwd_settings_xprop_storage_n_construction_params = 3;
    GParameter                  param[gwd_settings_xprop_storage_n_construction_params];
    GWDSettingsXPropertyStorage *storage = NULL;

    GValue display_value = G_VALUE_INIT;
    GValue root_window_value = G_VALUE_INIT;
    GValue writable_value = G_VALUE_INIT;


    g_value_init (&display_value, G_TYPE_POINTER);
    g_value_init (&root_window_value, G_TYPE_INT);
    g_value_init (&writable_value, G_TYPE_POINTER);

    g_value_set_pointer (&display_value, dpy);
    g_value_set_int (&root_window_value, root);
    g_value_set_pointer (&writable_value, writable);

    param[0].name = "writable-settings";
    param[0].value = writable_value;
    param[1].name = "display";
    param[1].value = display_value;
    param[2].name = "root-window";
    param[2].value = root_window_value;

    storage = GWD_SETTINGS_XPROPERTY_STORAGE_INTERFACE (g_object_newv (GWD_TYPE_SETTINGS_STORAGE_XPROP,
								       gwd_settings_xprop_storage_n_construction_params,
								       param));

    g_value_unset (&display_value);
    g_value_unset (&root_window_value);
    g_value_unset (&writable_value);

    return storage;
}
