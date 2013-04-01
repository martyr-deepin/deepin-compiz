#include <stdlib.h>
#include <string.h>

#include <gconf/gconf.h>
#include <gconf/gconf-client.h>
#include <gconf/gconf-value.h>

#include <ccs.h>
#include <ccs-backend.h>
#include <ccs-object.h>

#include "ccs_gnome_integration.h"
#include "ccs_gnome_integrated_setting.h"
#include "ccs_gnome_integration_constants.h"
#include "ccs_gnome_integration_gconf_integrated_setting.h"

typedef struct _CCSGConfIntegratedSettingFactoryPrivate CCSGConfIntegratedSettingFactoryPrivate;

struct _CCSGConfIntegratedSettingFactoryPrivate
{
    GConfClient *client;
    guint       gnomeGConfNotifyIds[NUM_WATCHED_DIRS];
    GHashTable  *pluginsToSettingsSectionsHashTable;
    GHashTable  *pluginsToSettingsSpecialTypesHashTable;
    GHashTable  *pluginsToSettingNameGNOMENameHashTable;
    CCSGNOMEValueChangeData *valueChangedData;
};

static void
gnomeGConfValueChanged (GConfClient *client,
			guint       cnxn_id,
			GConfEntry  *entry,
			gpointer    user_data)
{
    CCSGNOMEValueChangeData *data = (CCSGNOMEValueChangeData *) user_data;
    const gchar *keyName = gconf_entry_get_key (entry);
    gchar       *baseName = g_path_get_basename (keyName);

    /* We don't care if integration is not enabled */
    if (!ccsGetIntegrationEnabled (data->context))
	return;

    CCSIntegratedSettingList settingList = ccsIntegratedSettingsStorageFindMatchingSettingsByPredicate (data->storage,
													ccsGNOMEIntegrationFindSettingsMatchingPredicate,
													baseName);

    ccsIntegrationUpdateIntegratedSettings (data->integration,
					    data->context,
					    settingList);

    g_free (baseName);
}

static CCSIntegratedSetting *
createNewGConfIntegratedSetting (GConfClient *client,
				 const char  *sectionName,
				 const char  *gnomeName,
				 const char  *pluginName,
				 const char  *settingName,
				 CCSSettingType type,
				 SpecialOptionType specialOptionType,
				 CCSObjectAllocationInterface *ai)
{
    CCSIntegratedSettingInfo *sharedIntegratedSettingInfo = ccsSharedIntegratedSettingInfoNew (pluginName, settingName, type, ai);

    if (!sharedIntegratedSettingInfo)
	return NULL;

    CCSGNOMEIntegratedSettingInfo *gnomeIntegratedSettingInfo = ccsGNOMEIntegratedSettingInfoNew (sharedIntegratedSettingInfo, specialOptionType, gnomeName, ai);

    if (!gnomeIntegratedSettingInfo)
    {
	ccsIntegratedSettingInfoUnref (sharedIntegratedSettingInfo);
	return NULL;
    }

    CCSIntegratedSetting *gconfIntegratedSetting = ccsGConfIntegratedSettingNew (gnomeIntegratedSettingInfo, client, sectionName, ai);

    if (!gconfIntegratedSetting)
    {
	ccsIntegratedSettingInfoUnref ((CCSIntegratedSettingInfo *) gnomeIntegratedSettingInfo);
	return NULL;
    }

    return gconfIntegratedSetting;
}

static void
finiGConfClient (GConfClient    *client,
		 guint		*gnomeGConfNotifyIds)
{
    int i;

    gconf_client_clear_cache (client);

    for (i = 0; i < NUM_WATCHED_DIRS; i++)
    {
	if (gnomeGConfNotifyIds[i])
	{
	    gconf_client_notify_remove (client, gnomeGConfNotifyIds[0]);
	    gnomeGConfNotifyIds[i] = 0;
	}
	gconf_client_remove_dir (client, watchedGConfGnomeDirectories[i], NULL);
    }
    gconf_client_suggest_sync (client, NULL);

    g_object_unref (client);
}

static void
registerGConfClient (GConfClient    *client,
		     guint	    *gnomeGConfNotifyIds,
		     CCSGNOMEValueChangeData *data,
		     GConfClientNotifyFunc func)
{
    int i;

    for (i = 0; i < NUM_WATCHED_DIRS; i++)
	gnomeGConfNotifyIds[i] = gconf_client_notify_add (client,
							  watchedGConfGnomeDirectories[i],
							  func, (gpointer) data,
							  NULL, NULL);
}

static void
initGConfClient (CCSIntegratedSettingFactory *factory)
{
    int i;
    CCSGConfIntegratedSettingFactoryPrivate *priv = (CCSGConfIntegratedSettingFactoryPrivate *) ccsObjectGetPrivate (factory);
    priv->client = gconf_client_get_default ();

    for (i = 0; i < NUM_WATCHED_DIRS; i++)
	gconf_client_add_dir (priv->client, watchedGConfGnomeDirectories[i],
			      GCONF_CLIENT_PRELOAD_NONE, NULL);
}

CCSIntegratedSetting *
ccsGConfIntegratedSettingFactoryCreateIntegratedSettingForCCSSettingNameAndType (CCSIntegratedSettingFactory *factory,
										 CCSIntegration		     *integration,
										 const char		     *pluginName,
										 const char		     *settingName,
										 CCSSettingType		     type)
{
    CCSGConfIntegratedSettingFactoryPrivate *priv = (CCSGConfIntegratedSettingFactoryPrivate *) ccsObjectGetPrivate (factory);
    GHashTable                              *settingsSectionsHashTable = g_hash_table_lookup (priv->pluginsToSettingsSectionsHashTable, pluginName);
    GHashTable                              *settingsSpecialTypesHashTable = g_hash_table_lookup (priv->pluginsToSettingsSpecialTypesHashTable, pluginName);
    GHashTable				    *settingsSettingNameGNOMENameHashTable = g_hash_table_lookup (priv->pluginsToSettingNameGNOMENameHashTable, pluginName);

    if (!priv->client)
	initGConfClient (factory);

    if (!priv->gnomeGConfNotifyIds[0])
	registerGConfClient (priv->client, priv->gnomeGConfNotifyIds, priv->valueChangedData, gnomeGConfValueChanged);

    if (settingsSectionsHashTable &&
	settingsSpecialTypesHashTable &&
	settingsSettingNameGNOMENameHashTable)
    {
	const gchar *sectionName = g_hash_table_lookup (settingsSectionsHashTable, settingName);
	SpecialOptionType specialType = (SpecialOptionType) GPOINTER_TO_INT (g_hash_table_lookup (settingsSpecialTypesHashTable, settingName));
	const gchar *integratedName = g_hash_table_lookup (settingsSettingNameGNOMENameHashTable, settingName);

	return createNewGConfIntegratedSetting (priv->client,
						sectionName,
						integratedName,
						pluginName,
						settingName,
						type,
						specialType,
						factory->object.object_allocation);
    }


    return NULL;
}

void
ccsGConfIntegratedSettingFactoryFree (CCSIntegratedSettingFactory *factory)
{
    CCSGConfIntegratedSettingFactoryPrivate *priv = (CCSGConfIntegratedSettingFactoryPrivate *) ccsObjectGetPrivate (factory);

    if (priv->client)
	finiGConfClient (priv->client, priv->gnomeGConfNotifyIds);

    if (priv->pluginsToSettingsSectionsHashTable)
	g_hash_table_unref (priv->pluginsToSettingsSectionsHashTable);

    if (priv->pluginsToSettingsSpecialTypesHashTable)
	g_hash_table_unref (priv->pluginsToSettingsSpecialTypesHashTable);

    if (priv->pluginsToSettingNameGNOMENameHashTable)
	g_hash_table_unref (priv->pluginsToSettingNameGNOMENameHashTable);

    ccsObjectFinalize (factory);
    (*factory->object.object_allocation->free_) (factory->object.object_allocation->allocator, factory);
}


const CCSIntegratedSettingFactoryInterface ccsGConfIntegratedSettingFactoryInterface =
{
    ccsGConfIntegratedSettingFactoryCreateIntegratedSettingForCCSSettingNameAndType,
    ccsGConfIntegratedSettingFactoryFree
};

CCSIntegratedSettingFactory *
ccsGConfIntegratedSettingFactoryNew (GConfClient		  *client,
				     CCSGNOMEValueChangeData	  *valueChangedData,
				     CCSObjectAllocationInterface *ai)
{
    CCSIntegratedSettingFactory *factory = (*ai->calloc_) (ai->allocator, 1, sizeof (CCSIntegratedSettingFactory));

    if (!factory)
	return NULL;

    CCSGConfIntegratedSettingFactoryPrivate *priv = (*ai->calloc_) (ai->allocator, 1, sizeof (CCSGConfIntegratedSettingFactoryPrivate));

    if (!priv)
    {
	(*ai->free_) (ai->allocator, factory);
	return NULL;
    }

    if (client)
    {
	int i;
	priv->client = (GConfClient *) g_object_ref (client);
	for (i = 0; i < NUM_WATCHED_DIRS; i++)
	    gconf_client_add_dir (priv->client, watchedGConfGnomeDirectories[i],
				  GCONF_CLIENT_PRELOAD_NONE, NULL);
    }
    else
	priv->client = NULL;

    priv->pluginsToSettingsSectionsHashTable = ccsGNOMEIntegrationPopulateCategoriesHashTables ();
    priv->pluginsToSettingsSpecialTypesHashTable = ccsGNOMEIntegrationPopulateSpecialTypesHashTables ();
    priv->pluginsToSettingNameGNOMENameHashTable = ccsGNOMEIntegrationPopulateSettingNameToGNOMENameHashTables ();
    priv->valueChangedData = valueChangedData;

    ccsObjectInit (factory, ai);
    ccsObjectSetPrivate (factory, (CCSPrivate *) priv);
    ccsObjectAddInterface (factory, (const CCSInterface *) &ccsGConfIntegratedSettingFactoryInterface, GET_INTERFACE_TYPE (CCSIntegratedSettingFactoryInterface));

    ccsIntegratedSettingFactoryRef (factory);

    return factory;
}

