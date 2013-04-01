#ifndef _COMPIZ_GSETTINGS_UTIL_H
#define _COMPIZ_GSETTINGS_UTIL_H

#include <ccs.h>
#include <ccs-backend.h>

COMPIZCONFIG_BEGIN_DECLS

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

#include "ccs_gsettings_interface.h"

/* some forward declarations */
void
ccsGSettingsValueChanged (GSettings   *settings,
			  gchar	      *keyname,
			  gpointer    user_data);

void ccsGSettingsWriteIntegratedOption (CCSBackend *backend,
					CCSContext *context,
					CCSSetting *setting,
					int        index);

CCSStringList
ccsGSettingsGetExistingProfiles (CCSBackend *backend, CCSContext *context);

void
ccsGSettingsSetIntegration (CCSBackend *backend, CCSIntegration *integration);

typedef struct _CCSGSettingsBackendPrivate CCSGSettingsBackendPrivate;
typedef struct _CCSGSettingsBackendInterface CCSGSettingsBackendInterface;

extern const CCSBackendInfo gsettingsBackendInfo;

typedef struct _CCSGSettingsWrapper CCSGSettingsWrapper;

gchar *
getSchemaNameForPlugin (const char *plugin);

char *
truncateKeyForGSettings (const char *gsettingName);

char *
translateUnderscoresToDashesForGSettings (const char *truncated);

void
translateToLowercaseForGSettings (char *name);

gchar *
translateKeyForGSettings (const char *gsettingName);

gchar *
translateKeyForCCS (const char *gsettingName);

gboolean
compizconfigTypeHasVariantType (CCSSettingType t);

gboolean
decomposeGSettingsPath (const char *path,
			char **pluginName,
			unsigned int *screenNum);

gboolean
variantIsValidForCCSType (GVariant *gsettingsValue,
			  CCSSettingType settingType);

Bool
appendToPluginsWithSetKeysList (const gchar    *plugin,
				GVariant       *writtenPlugins,
				char	       ***newWrittenPlugins,
				gsize	       *newWrittenPluginsSize);

CCSGSettingsWrapper *
findCCSGSettingsWrapperBySchemaName (const gchar *schemaName,
				     GList	 *iter);

CCSSettingList
filterAllSettingsMatchingType (CCSSettingType type,
			       CCSSettingList settingList);

CCSSettingList
filterAllSettingsMatchingPartOfStringIgnoringDashesUnderscoresAndCase (const gchar *keyName,
								       CCSSettingList sList);

CCSSetting *
attemptToFindCCSSettingFromLossyName (CCSSettingList settingList, const gchar *lossyName, CCSSettingType type);

Bool
findSettingAndPluginToUpdateFromPath (CCSGSettingsWrapper *settings,
				      const char *path,
				      const gchar *keyName,
				      CCSContext *context,
				      CCSPlugin **plugin,
				      CCSSetting **setting,
				      char **uncleanKeyName);

Bool updateSettingWithGSettingsKeyName (CCSBackend *backend,
					CCSGSettingsWrapper *settings,
					const gchar     *keyName,
					CCSBackendUpdateFunc updateSetting);

GList *
variantTypeToPossibleSettingType (const gchar *vt);

gchar *
makeCompizProfilePath (const gchar *profilename);

gchar *
makeCompizPluginPath (const gchar *profileName, const gchar *pluginName);

gchar *
getNameForCCSSetting (CCSSetting *setting);

Bool
checkReadVariantIsValid (GVariant *gsettingsValue, CCSSettingType type, const gchar *pathName);

GVariant *
getVariantAtKey (CCSGSettingsWrapper *settings, const char *key, const char *pathName, CCSSettingType type);

const char * readStringFromVariant (GVariant *gsettingsValue);

int readIntFromVariant (GVariant *gsettingsValue);

Bool readBoolFromVariant (GVariant *gsettingsValue);

float readFloatFromVariant (GVariant *gsettingsValue);

CCSSettingColorValue readColorFromVariant (GVariant *gsettingsValue, Bool *success);

CCSSettingKeyValue readKeyFromVariant (GVariant *gsettingsValue, Bool *success);

CCSSettingButtonValue readButtonFromVariant (GVariant *gsettingsValue, Bool *success);

unsigned int readEdgeFromVariant (GVariant *gsettingsValue);

CCSSettingValueList
readListValue (GVariant *gsettingsValue, CCSSetting *setting, CCSObjectAllocationInterface *allocator);

Bool
writeListValue (CCSSettingValueList list,
		CCSSettingType	    listType,
		GVariant	    **gsettingsValue);

Bool writeStringToVariant (const char *value, GVariant **variant);

Bool writeFloatToVariant (float value, GVariant **variant);

Bool writeIntToVariant (int value, GVariant **variant);

Bool writeBoolToVariant (Bool value, GVariant **variant);

Bool writeColorToVariant (CCSSettingColorValue value, GVariant **variant);

Bool writeKeyToVariant (CCSSettingKeyValue key, GVariant **variant);

Bool writeButtonToVariant (CCSSettingButtonValue button, GVariant **variant);

Bool writeEdgeToVariant (unsigned int edges, GVariant **variant);

void writeVariantToKey (CCSGSettingsWrapper *settings,
			const char *key,
			GVariant   *value);

typedef int (*ComparisonPredicate) (const void *s1, const void *s2);

int voidcmp0 (const void *v1, const void *v2);

gboolean
deleteProfile (CCSBackend *backend,
	       CCSContext *context,
	       const char *profile);

gboolean
appendStringToVariantIfUnique (GVariant	  **variant,
			       const char *string);

gboolean
removeItemFromVariant (GVariant	  **variant,
		       const char *string);

CCSSettingValueList
readBoolListValue (GVariantIter *iter, guint nItems, CCSSetting *setting, CCSObjectAllocationInterface *allocator);

CCSSettingValueList
readIntListValue (GVariantIter *iter, guint nItems, CCSSetting *setting, CCSObjectAllocationInterface *allocator);

CCSSettingValueList
readFloatListValue (GVariantIter *iter, guint nItems, CCSSetting *setting, CCSObjectAllocationInterface *allocator);

CCSSettingValueList
readStringListValue (GVariantIter *iter, guint nItems, CCSSetting *setting, CCSObjectAllocationInterface *allocator);

CCSSettingValueList
readColorListValue (GVariantIter *iter, guint nItems, CCSSetting *setting, CCSObjectAllocationInterface *allocator);

gchar *
makeSettingPath (const char *currentProfile, CCSSetting *setting);

CCSGSettingsWrapper *
getSettingsObjectForCCSSetting (CCSBackend *backend, CCSSetting *setting);

void
resetOptionToDefault (CCSBackend *backend, CCSSetting * setting);

COMPIZCONFIG_END_DECLS

#endif
