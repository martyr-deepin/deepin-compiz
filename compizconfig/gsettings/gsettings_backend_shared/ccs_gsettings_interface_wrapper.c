#include <gio/gio.h>
#include "ccs_gsettings_interface_wrapper.h"

typedef struct _CCSGSettingsWrapperPrivate CCSGSettingsWrapperPrivate;

struct _CCSGSettingsWrapperPrivate
{
    GSettings *settings;
    char      *schema;
    char      *path;
};

#define GSETTINGS_WRAPPER_PRIVATE(w) \
    CCSGSettingsWrapperPrivate *gswPrivate = (CCSGSettingsWrapperPrivate *) ccsObjectGetPrivate (w);

static GVariant * ccsGSettingsWrapperGetValueDefault (CCSGSettingsWrapper *wrapper, const char *key)
{
    GSETTINGS_WRAPPER_PRIVATE (wrapper);

    return g_settings_get_value (gswPrivate->settings, key);
}

static void ccsGSettingsWrapperSetValueDefault (CCSGSettingsWrapper *wrapper, const char *key, GVariant *variant)
{
    GSETTINGS_WRAPPER_PRIVATE (wrapper);

    g_settings_set_value (gswPrivate->settings, key, variant);
}

static void ccsGSettingsWrapperResetKeyDefault (CCSGSettingsWrapper *wrapper, const char *key)
{
    GSETTINGS_WRAPPER_PRIVATE (wrapper);

    g_settings_reset (gswPrivate->settings, key);
}

static char ** ccsGSettingsWrapperListKeysDefault (CCSGSettingsWrapper *wrapper)
{
    GSETTINGS_WRAPPER_PRIVATE (wrapper);

    return g_settings_list_keys (gswPrivate->settings);
}

static GSettings * ccsGSettingsWrapperGetGSettingsDefault (CCSGSettingsWrapper *wrapper)
{
    GSETTINGS_WRAPPER_PRIVATE (wrapper);

    return gswPrivate->settings;
}

static const char *
ccsGSettingsWrapperGetSchemaNameDefault (CCSGSettingsWrapper *wrapper)
{
    GSETTINGS_WRAPPER_PRIVATE (wrapper);

    return gswPrivate->schema;
}

static const char *
ccsGSettingsWrapperGetPathDefault (CCSGSettingsWrapper *wrapper)
{
    GSETTINGS_WRAPPER_PRIVATE (wrapper);

    return gswPrivate->path;
}

void
ccsGSettingsWrapperConnectToChangedSignalDefault (CCSGSettingsWrapper *wrapper,
						  GCallback	       callback,
						  gpointer	       data)
{
    GSETTINGS_WRAPPER_PRIVATE (wrapper);

    g_signal_connect (gswPrivate->settings, "changed", callback, data);
}

static void
ccsFreeGSettingsWrapperDefault (CCSGSettingsWrapper *wrapper)
{
    GSETTINGS_WRAPPER_PRIVATE (wrapper);

    if (gswPrivate->settings)
	g_object_unref (gswPrivate->settings);

    if (gswPrivate->path)
	(*wrapper->object.object_allocation->free_) (wrapper->object.object_allocation->allocator,
						     gswPrivate->path);

    if (gswPrivate->schema)
	(*wrapper->object.object_allocation->free_) (wrapper->object.object_allocation->allocator,
						     gswPrivate->schema);

    ccsObjectFinalize (wrapper);

    (*wrapper->object.object_allocation->free_) (wrapper->object.object_allocation->allocator,
						 wrapper);
}

const CCSGSettingsWrapperInterface interface =
{
    ccsGSettingsWrapperSetValueDefault,
    ccsGSettingsWrapperGetValueDefault,
    ccsGSettingsWrapperResetKeyDefault,
    ccsGSettingsWrapperListKeysDefault,
    ccsGSettingsWrapperGetGSettingsDefault,
    ccsGSettingsWrapperGetSchemaNameDefault,
    ccsGSettingsWrapperGetPathDefault,
    ccsGSettingsWrapperConnectToChangedSignalDefault,
    ccsFreeGSettingsWrapperDefault
};

static CCSGSettingsWrapperPrivate *
allocatePrivateWrapper (CCSObjectAllocationInterface *ai, CCSGSettingsWrapper *wrapper)
{
    CCSGSettingsWrapperPrivate *priv = (*ai->calloc_) (ai->allocator, 1, sizeof (CCSGSettingsWrapper));

    if (!priv)
    {
	(*ai->free_) (ai->allocator, wrapper);
	return NULL;
    }

    return priv;
}

static CCSGSettingsWrapper *
allocateWrapper (CCSObjectAllocationInterface *ai)
{
    CCSGSettingsWrapper *wrapper = (*ai->calloc_) (ai->allocator, 1, sizeof (CCSGSettingsWrapper));

    if (!wrapper)
	return NULL;

    return wrapper;
}

static void
freeWrapperAndPriv (CCSGSettingsWrapper *wrapper,
		    CCSGSettingsWrapperPrivate *priv,
		    CCSObjectAllocationInterface *ai)
{
    (*ai->free_) (ai->allocator, priv);
    (*ai->free_) (ai->allocator, wrapper);
}

static GSettings *
newGSettingsWithPath (const char *schema,
		      const char *path,
		      CCSGSettingsWrapper *wrapper,
		      CCSGSettingsWrapperPrivate *priv,
		      CCSObjectAllocationInterface *ai)
{
    GSettings *settings = g_settings_new_with_path (schema, path);

    if (!settings)
    {
	freeWrapperAndPriv (wrapper, priv, ai);
	return NULL;
    }

    return settings;
}

static GSettings *
newGSettings (const char *schema,
	      CCSGSettingsWrapper *wrapper,
	      CCSGSettingsWrapperPrivate *priv,
	      CCSObjectAllocationInterface *ai)
{
    GSettings *settings = g_settings_new (schema);

    if (!settings)
    {
	freeWrapperAndPriv (wrapper, priv, ai);
	return NULL;
    }

    return settings;
}

static Bool
allocateWrapperData (CCSObjectAllocationInterface *ai,
		     CCSGSettingsWrapper	  **wrapper,
		     CCSGSettingsWrapperPrivate   **priv)
{
    *wrapper = allocateWrapper (ai);

    if (!*wrapper)
	return FALSE;

    *priv = allocatePrivateWrapper (ai, *wrapper);

    if (!*priv)
	return FALSE;

    return TRUE;
}

static void
initCCSGSettingsWrapperObject (CCSGSettingsWrapper *wrapper,
			       CCSGSettingsWrapperPrivate *priv,
			       CCSObjectAllocationInterface *ai)
{
    ccsObjectInit (wrapper, ai);
    ccsObjectAddInterface (wrapper, (const CCSInterface *) &interface, GET_INTERFACE_TYPE (CCSGSettingsWrapperInterface));
    ccsObjectSetPrivate (wrapper, (CCSPrivate *) priv);
    ccsGSettingsWrapperRef (wrapper);
}

CCSGSettingsWrapper *
ccsGSettingsWrapperNewForSchemaWithPath (const char *schema,
					 const char *path,
					 CCSObjectAllocationInterface *ai)
{
    CCSGSettingsWrapper *wrapper = NULL;
    CCSGSettingsWrapperPrivate *priv = NULL;

    if (!allocateWrapperData (ai, &wrapper, &priv))
	return NULL;

    priv->schema = g_strdup (schema);
    priv->path = g_strdup (path);
    priv->settings = newGSettingsWithPath (schema, path, wrapper, priv, ai);

    if (!priv->settings)
	return NULL;

    initCCSGSettingsWrapperObject (wrapper, priv, ai);

    return wrapper;
}

CCSGSettingsWrapper *
ccsGSettingsWrapperNewForSchema (const char *schema,
				 CCSObjectAllocationInterface *ai)
{
    CCSGSettingsWrapper *wrapper = NULL;
    CCSGSettingsWrapperPrivate *priv = NULL;

    if (!allocateWrapperData (ai, &wrapper, &priv))
	return NULL;

    priv->schema = g_strdup (schema);
    priv->settings = newGSettings (schema, wrapper, priv, ai);

    if (!priv->settings)
	return NULL;

    GValue pathValue = G_VALUE_INIT;
    g_value_init (&pathValue, G_TYPE_STRING);
    g_object_get_property (G_OBJECT (priv->settings), "path", &pathValue);

    priv->path = g_value_dup_string (&pathValue);

    g_value_unset (&pathValue);

    initCCSGSettingsWrapperObject (wrapper, priv, ai);

    return wrapper;
}
