#include <stdlib.h>
#include <string.h>

#include <ccs.h>
#include <ccs-backend.h>

#include "ccs_gsettings_backend.h"
#include "ccs_gsettings_backend_interface.h"
#include "ccs_gsettings_interface.h"
#include "ccs_gsettings_interface_wrapper.h"
#include "ccs_gnome_integration.h"
#include "ccs_gnome_integration_gsettings_integrated_setting_factory.h"
#include "gsettings_shared.h"

struct _CCSGSettingsBackendPrivate
{
    GList	   *settingsList;
    CCSGSettingsWrapper *compizconfigSettings;
    CCSGSettingsWrapper *currentProfileSettings;

    char	    *currentProfile;
    CCSContext	    *context;

    CCSIntegration *integration;

    CCSGNOMEValueChangeData valueChangeData;
};

void
ccsGSettingsSetIntegration (CCSBackend *backend, CCSIntegration *integration)
{
    CCSGSettingsBackendPrivate *priv = (CCSGSettingsBackendPrivate *) ccsObjectGetPrivate (backend);

    if (priv->integration)
	ccsIntegrationUnref (priv->integration);

    priv->integration = integration;
    ccsIntegrationRef (integration);
}

CCSStringList
ccsGSettingsGetExistingProfiles (CCSBackend *backend, CCSContext *context)
{
    GVariant      *value;
    char	  *profile;
    GVariantIter  iter;
    CCSStringList ret = NULL;

    CCSGSettingsBackendPrivate *priv = (CCSGSettingsBackendPrivate *) ccsObjectGetPrivate (backend);

    ccsGSettingsBackendUpdateProfile (backend, context);

    value = ccsGSettingsWrapperGetValue (priv->compizconfigSettings,  "existing-profiles");
    g_variant_iter_init (&iter, value);
    while (g_variant_iter_loop (&iter, "s", &profile))
    {
	CCSString *str = calloc (1, sizeof (CCSString));
	str->value = strdup (profile);

	ccsStringRef (str);
	ret = ccsStringListAppend (ret, str);
    }

    g_variant_unref (value);

    return ret;
}

void
ccsGSettingsValueChanged (GSettings   *settings,
			  gchar	      *keyName,
			  gpointer    user_data)
{
    CCSBackend   *backend = (CCSBackend *)user_data;
    GValue       schemaNameValue = G_VALUE_INIT;
    CCSGSettingsBackendPrivate *priv = (CCSGSettingsBackendPrivate *) ccsObjectGetPrivate (backend);
    CCSBackendInterface *backendInterface = (CCSBackendInterface *) GET_INTERFACE (CCSBackendInterface, backend);


    g_value_init (&schemaNameValue, G_TYPE_STRING);
    g_object_get_property (G_OBJECT (settings), "schema-id", &schemaNameValue);

    const char *schemaName = g_value_get_string (&schemaNameValue);
    CCSGSettingsWrapper *wrapper = findCCSGSettingsWrapperBySchemaName (schemaName, priv->settingsList);

    g_value_unset (&schemaNameValue);

    updateSettingWithGSettingsKeyName (backend, wrapper, keyName, backendInterface->updateSetting);
}

static CCSGSettingsWrapper *
ccsGSettingsBackendGetSettingsObjectForPluginWithPathDefault (CCSBackend *backend,
							      const char *plugin,
							      const char *path,
							      CCSContext *context)
{
    CCSGSettingsWrapper *settingsObj = NULL;
    gchar *schemaName = getSchemaNameForPlugin (plugin);
    GVariant        *writtenPlugins;
    gsize            newWrittenPluginsSize;
    gchar           **newWrittenPlugins;

    CCSGSettingsBackendPrivate *priv = (CCSGSettingsBackendPrivate *) ccsObjectGetPrivate (backend);

    settingsObj = findCCSGSettingsWrapperBySchemaName (schemaName, priv->settingsList);

    if (settingsObj)
    {
	g_free (schemaName);
	return settingsObj;
    }

    /* No existing settings object found for this schema, create one */
    
    settingsObj = ccsGSettingsWrapperNewForSchemaWithPath (schemaName, path, &ccsDefaultObjectAllocator);
    ccsGSettingsBackendConnectToChangedSignal (backend, settingsObj);
    priv->settingsList = g_list_append (priv->settingsList, (void *) settingsObj);

    /* Also write the plugin name to the list of modified plugins so
     * that when we delete the profile the keys for that profile are also
     * unset FIXME: This could be a little more efficient, like we could
     * store keys that have changed from their defaults ... though
     * gsettings doesn't seem to give you a way to get all of the schemas */

    writtenPlugins = ccsGSettingsWrapperGetValue (priv->currentProfileSettings, "plugins-with-set-keys");

    appendToPluginsWithSetKeysList (plugin, writtenPlugins, &newWrittenPlugins, &newWrittenPluginsSize);

    GVariant *newWrittenPluginsVariant = g_variant_new_strv ((const gchar * const *) newWrittenPlugins, newWrittenPluginsSize);

    ccsGSettingsWrapperSetValue (priv->currentProfileSettings, "plugins-with-set-keys", newWrittenPluginsVariant);

    g_variant_unref (writtenPlugins);
    g_free (schemaName);
    g_strfreev (newWrittenPlugins);

    return settingsObj;
}

gboolean
ccsGSettingsBackendAddProfileDefault (CCSBackend *backend, const char *profile)
{
    GVariant        *profiles;
    gboolean	    ret = FALSE;

    profiles = ccsGSettingsBackendGetExistingProfiles (backend);
    if (appendStringToVariantIfUnique (&profiles, profile))
    {
	ret = TRUE;
	ccsGSettingsBackendSetExistingProfiles (backend, profiles);
    }
    else
	g_variant_unref (profiles);

    return ret;
}

void
ccsGSettingsBackendUpdateCurrentProfileNameDefault (CCSBackend *backend, const char *profile)
{
    ccsGSettingsBackendAddProfile (backend, profile);
    ccsGSettingsBackendSetCurrentProfile (backend, profile);
}

gboolean
ccsGSettingsBackendUpdateProfileDefault (CCSBackend *backend, CCSContext *context)
{
    const char *currentProfile = ccsGSettingsBackendGetCurrentProfile (backend);
    const char *ccsProfile = ccsGetProfile (context);
    char *profile = NULL;

    if (!ccsProfile)
	profile = strdup (DEFAULTPROF);
    else
	profile = strdup (ccsProfile);

    if (!strlen (profile))
    {
	free (profile);
	profile = strdup (DEFAULTPROF);
    }

    if (g_strcmp0 (profile, currentProfile))
	ccsGSettingsBackendUpdateCurrentProfileName (backend, profile);

    free (profile);

    return TRUE;
}

void
ccsGSettingsBackendUnsetAllChangedPluginKeysInProfileDefault (CCSBackend *backend,
							      CCSContext *context,
							      GVariant *pluginsWithChangedKeys,
							      const char * profile)
{
    GVariantIter    iter;
    char            *plugin;

    g_variant_iter_init (&iter, pluginsWithChangedKeys);
    while (g_variant_iter_loop (&iter, "s", &plugin))
    {
	CCSGSettingsWrapper *settings;
	gchar *pathName = makeCompizPluginPath (profile, plugin);

	settings = ccsGSettingsGetSettingsObjectForPluginWithPath (backend, plugin, pathName, context);
	g_free (pathName);

	/* The GSettings documentation says not to use this API
	 * because we should know our own schema ... though really
	 * we don't because we autogenerate schemas ... */
	if (settings)
	{
	    char **keys = ccsGSettingsWrapperListKeys (settings);
	    char **key_ptr;

	    /* Unset all the keys */
	    for (key_ptr = keys; *key_ptr; key_ptr++)
		ccsGSettingsWrapperResetKey (settings, *key_ptr);

	    g_strfreev (keys);
	}
    }
}

static CCSContext *
ccsGSettingsBackendGetContextDefault (CCSBackend *backend)
{
    CCSGSettingsBackendPrivate *priv = (CCSGSettingsBackendPrivate *) ccsObjectGetPrivate (backend);

    return priv->context;
}

static void
ccsGSettingsBackendConnectToValueChangedSignalDefault (CCSBackend *backend, CCSGSettingsWrapper *wrapper)
{
    ccsGSettingsWrapperConnectToChangedSignal (wrapper, (GCallback) ccsGSettingsValueChanged, (gpointer) backend);
}

static const char *
ccsGSettingsBackendGetCurrentProfileDefault (CCSBackend *backend)
{
    CCSGSettingsBackendPrivate *priv = (CCSGSettingsBackendPrivate *) ccsObjectGetPrivate (backend);

    return priv->currentProfile;
}

static GVariant *
ccsGSettingsBackendGetExistingProfilesDefault (CCSBackend *backend)
{
    CCSGSettingsBackendPrivate *priv = (CCSGSettingsBackendPrivate *) ccsObjectGetPrivate (backend);

    return ccsGSettingsWrapperGetValue (priv->compizconfigSettings, "existing-profiles");
}

static void
ccsGSettingsBackendSetExistingProfilesDefault (CCSBackend *backend, GVariant *value)
{
    CCSGSettingsBackendPrivate *priv = (CCSGSettingsBackendPrivate *) ccsObjectGetPrivate (backend);

    ccsGSettingsWrapperSetValue (priv->compizconfigSettings, "existing-profiles", value);
}

static void
ccsGSettingsBackendSetCurrentProfileDefault (CCSBackend *backend, const gchar *value)
{
    CCSGSettingsBackendPrivate *priv = (CCSGSettingsBackendPrivate *) ccsObjectGetPrivate (backend);
    gchar           *profilePath = makeCompizProfilePath (value);

    /* Change the current profile and current profile settings */
    if (priv->currentProfile)
	free (priv->currentProfile);

    if (priv->currentProfileSettings)
	ccsGSettingsWrapperUnref (priv->currentProfileSettings);

    priv->currentProfile = strdup (value);
    priv->currentProfileSettings = ccsGSettingsWrapperNewForSchemaWithPath (PROFILE_SCHEMA_ID,
									    profilePath,
									    backend->object.object_allocation);

    GVariant *currentProfileVariant = g_variant_new ("s", value, NULL);

    ccsGSettingsWrapperSetValue (priv->compizconfigSettings, "current-profile", currentProfileVariant);

    g_free (profilePath);
}

GVariant *
ccsGSettingsBackendGetPluginsWithSetKeysDefault (CCSBackend *backend)
{
    CCSGSettingsBackendPrivate *priv = (CCSGSettingsBackendPrivate *) ccsObjectGetPrivate (backend);
    return ccsGSettingsWrapperGetValue (priv->currentProfileSettings, "plugins-with-set-keys");
}

void
ccsGSettingsBackendClearPluginsWithSetKeysDefault (CCSBackend *backend)
{
    CCSGSettingsBackendPrivate *priv = (CCSGSettingsBackendPrivate *) ccsObjectGetPrivate (backend);
    ccsGSettingsWrapperResetKey (priv->currentProfileSettings, "plugins-with-set-keys");
}

CCSIntegratedSetting *
ccsGSettingsBackendGetIntegratedOptionIndexDefault (CCSBackend *backend, CCSSetting *setting)
{
    CCSPlugin  *plugin      = ccsSettingGetParent (setting);
    const char *pluginName  = ccsPluginGetName (plugin);
    const char *settingName = ccsSettingGetName (setting);
    CCSGSettingsBackendPrivate *priv = (CCSGSettingsBackendPrivate *) ccsObjectGetPrivate (backend);

    return ccsIntegrationGetIntegratedSetting (priv->integration, pluginName, settingName);
}

Bool
ccsGSettingsBackendReadIntegratedOptionDefault (CCSBackend *backend, CCSSetting *setting, CCSIntegratedSetting *integrated)
{
    CCSGSettingsBackendPrivate *priv = (CCSGSettingsBackendPrivate *) ccsObjectGetPrivate (backend);

    return ccsIntegrationReadOptionIntoSetting (priv->integration,
						priv->context,
						setting,
						integrated);
}

void
ccsGSettingsBackendWriteIntegratedOptionDefault (CCSBackend *backend, CCSSetting *setting, CCSIntegratedSetting *integrated)
{
    CCSGSettingsBackendPrivate *priv = (CCSGSettingsBackendPrivate *) ccsObjectGetPrivate (backend);

    ccsIntegrationWriteSettingIntoOption (priv->integration,
					  priv->context,
					  setting,
					  integrated);
}

static CCSGSettingsBackendInterface gsettingsAdditionalDefaultInterface = {
    ccsGSettingsBackendGetContextDefault,
    ccsGSettingsBackendConnectToValueChangedSignalDefault,
    ccsGSettingsBackendGetSettingsObjectForPluginWithPathDefault,
    ccsGSettingsBackendGetCurrentProfileDefault,
    ccsGSettingsBackendGetExistingProfilesDefault,
    ccsGSettingsBackendSetExistingProfilesDefault,
    ccsGSettingsBackendSetCurrentProfileDefault,
    ccsGSettingsBackendGetPluginsWithSetKeysDefault,
    ccsGSettingsBackendClearPluginsWithSetKeysDefault,
    ccsGSettingsBackendUnsetAllChangedPluginKeysInProfileDefault,
    ccsGSettingsBackendUpdateProfileDefault,
    ccsGSettingsBackendUpdateCurrentProfileNameDefault,
    ccsGSettingsBackendAddProfileDefault,
    ccsGSettingsBackendGetIntegratedOptionIndexDefault,
    ccsGSettingsBackendReadIntegratedOptionDefault,
    ccsGSettingsBackendWriteIntegratedOptionDefault
};

static CCSGSettingsBackendPrivate *
addPrivateToBackend (CCSBackend *backend, CCSObjectAllocationInterface *ai)
{
    CCSGSettingsBackendPrivate *priv = (*ai->calloc_) (ai->allocator, 1, sizeof (CCSGSettingsBackendPrivate));

    if (!priv)
    {
	ccsObjectRemoveInterface (backend, GET_INTERFACE_TYPE (CCSGSettingsBackendInterface));
	return NULL;
    }

    ccsObjectSetPrivate (backend, (CCSPrivate *) priv);
    return priv;
}

static char*
getCurrentProfileName (CCSBackend *backend)
{
    GVariant *value;
    char     *ret = NULL;

    CCSGSettingsBackendPrivate *priv = (CCSGSettingsBackendPrivate *) ccsObjectGetPrivate (backend);

    value = ccsGSettingsWrapperGetValue (priv->compizconfigSettings, "current-profile");

    if (value)
	ret = strdup (g_variant_get_string (value, NULL));
    else
	ret = strdup (DEFAULTPROF);

    g_variant_unref (value);

    return ret;
}

static void
ccsGSettingsWrapperDestroyNotify (gpointer o)
{
    ccsGSettingsWrapperUnref ((CCSGSettingsWrapper *) o);
}

void
ccsGSettingsBackendDetachFromBackend (CCSBackend *backend)
{
    CCSGSettingsBackendPrivate *priv = (CCSGSettingsBackendPrivate *) ccsObjectGetPrivate (backend);

    if (priv->currentProfile)
    {
	free (priv->currentProfile);
	priv->currentProfile = NULL;
    }

    g_list_free_full (priv->settingsList, ccsGSettingsWrapperDestroyNotify);
    priv->settingsList = NULL;

    if (priv->currentProfileSettings)
    {
	ccsGSettingsWrapperUnref (priv->currentProfileSettings);
	priv->currentProfileSettings = NULL;
    }

    ccsGSettingsWrapperUnref (priv->compizconfigSettings);

    priv->compizconfigSettings = NULL;

    ccsIntegrationUnref (priv->integration);

    free (priv);
    ccsObjectSetPrivate (backend, NULL);

}

Bool
ccsGSettingsBackendAttachNewToBackend (CCSBackend *backend, CCSContext *context)
{
    char       *currentProfilePath;

    ccsObjectAddInterface (backend, (CCSInterface *) &gsettingsAdditionalDefaultInterface, GET_INTERFACE_TYPE (CCSGSettingsBackendInterface));

    CCSGSettingsBackendPrivate *priv = addPrivateToBackend (backend, backend->object.object_allocation);

    priv->compizconfigSettings = ccsGSettingsWrapperNewForSchema (COMPIZCONFIG_SCHEMA_ID,
								  backend->object.object_allocation);
    priv->currentProfile = getCurrentProfileName (backend);
    currentProfilePath = makeCompizProfilePath (priv->currentProfile);
    priv->currentProfileSettings = ccsGSettingsWrapperNewForSchemaWithPath (PROFILE_SCHEMA_ID,
									    currentProfilePath,
									    backend->object.object_allocation);
    priv->context = context;

    CCSGNOMEIntegrationGSettingsWrapperFactory *wrapperFactory = ccsGNOMEIntegrationGSettingsWrapperDefaultImplNew (&ccsDefaultObjectAllocator);
    CCSIntegratedSettingsStorage *storage = ccsIntegratedSettingsStorageDefaultImplNew (&ccsDefaultObjectAllocator);

    priv->valueChangeData.storage = storage;
    priv->valueChangeData.context = priv->context;

    CCSIntegratedSettingFactory *factory = ccsGSettingsIntegratedSettingFactoryNew (wrapperFactory,
										    &priv->valueChangeData,
										    &ccsDefaultObjectAllocator);

    priv->valueChangeData.factory = factory;

    priv->integration = ccsGNOMEIntegrationBackendNew (backend, context, factory, storage, backend->object.object_allocation);



    priv->valueChangeData.integration = priv->integration;

    g_free (currentProfilePath);
    return TRUE;
}
