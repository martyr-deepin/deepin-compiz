#ifndef _COMPIZCONFIG_GSETTINGS_BACKEND_INTERFACE_H
#define _COMPIZCONFIG_GSETTINGS_BACKEND_INTERFACE_H

#include <ccs-defs.h>
#include <ccs-object.h>
#include <glib.h>

COMPIZCONFIG_BEGIN_DECLS

typedef struct _CCSBackend		     CCSBackend;
typedef struct _CCSGSettingsBackend          CCSGSettingsBackend;
typedef struct _CCSGSettingsWrapper	     CCSGSettingsWrapper;
typedef struct _CCSGSettingsBackendInterface CCSGSettingsBackendInterface;
typedef struct _CCSSetting		     CCSSetting;
typedef struct _CCSIntegratedSetting	     CCSIntegratedSetting;
typedef struct _CCSContext		     CCSContext;

typedef CCSContext * (*CCSGSettingsBackendGetContext) (CCSBackend *);
typedef void (*CCSGSettingsBackendConnectToChangedSignal) (CCSBackend *, CCSGSettingsWrapper *);
typedef CCSGSettingsWrapper * (*CCSGSettingsBackendGetSettingsObjectForPluginWithPath) (CCSBackend *backend,
											const char *plugin,
											const char *path,
											CCSContext *context);

typedef const char * (*CCSGSettingsBackendGetCurrentProfile) (CCSBackend *backend);

typedef GVariant * (*CCSGSettingsBackendGetExistingProfiles) (CCSBackend *backend);
typedef void (*CCSGSettingsBackendSetExistingProfiles) (CCSBackend *backend, GVariant *value);
typedef void (*CCSGSettingsBackendSetCurrentProfile) (CCSBackend *backend, const gchar *value);

typedef GVariant * (*CCSGSettingsBackendGetPluginsWithSetKeys) (CCSBackend *backend);
typedef void (*CCSGSettingsBackendClearPluginsWithSetKeys) (CCSBackend *backend);

typedef void (*CCSGSettingsBackendUnsetAllChangedPluginKeysInProfile) (CCSBackend *backend, CCSContext *, GVariant *, const char *);

typedef gboolean (*CCSGSettingsBackendUpdateProfile) (CCSBackend *, CCSContext *);
typedef void (*CCSGSettingsBackendUpdateCurrentProfileName) (CCSBackend *backend, const char *profile);

typedef gboolean (*CCSGSettingsBackendAddProfile) (CCSBackend *backend, const char *profile);

typedef CCSIntegratedSetting * (*CCSGSettingsBackendGetIntegratedSetting) (CCSBackend *backend, CCSSetting *setting);
typedef Bool (*CCSGSettingsBackendReadIntegratedOption) (CCSBackend *backend, CCSSetting *setting, CCSIntegratedSetting *);
typedef void (*CCSGSettingsBackendWriteIntegratedOption) (CCSBackend *backend, CCSSetting *setting, CCSIntegratedSetting *);

/**
 * @brief The _CCSGSettingsBackendInterface struct
 *
 * This interface represents a loaded CCSGSettingsBackend and some of the
 * special operations that go with it. It is mainly an interface that
 * exists for testing purposes - there are some operations which we
 * want to mock out or replace since they can't be done in testing mode.
 *
 * CCSGSettingsBackendInterface isn't an interface that is implemented
 * by any objects itself - it is attached to an existing CCSBackend
 * at runtime, and the CCSBackend is passed to any utility functions in
 * the GSettings backend.
 */
struct _CCSGSettingsBackendInterface
{
    CCSGSettingsBackendGetContext gsettingsBackendGetContext;
    CCSGSettingsBackendConnectToChangedSignal gsettingsBackendConnectToChangedSignal;
    CCSGSettingsBackendGetSettingsObjectForPluginWithPath gsettingsBackendGetSettingsObjectForPluginWithPath;
    CCSGSettingsBackendGetCurrentProfile   gsettingsBackendGetCurrentProfile;
    CCSGSettingsBackendGetExistingProfiles gsettingsBackendGetExistingProfiles;
    CCSGSettingsBackendSetExistingProfiles gsettingsBackendSetExistingProfiles;
    CCSGSettingsBackendSetCurrentProfile gsettingsBackendSetCurrentProfile;
    CCSGSettingsBackendGetPluginsWithSetKeys gsettingsBackendGetPluginsWithSetKeys;
    CCSGSettingsBackendClearPluginsWithSetKeys gsettingsBackendClearPluginsWithSetKeys;
    CCSGSettingsBackendUnsetAllChangedPluginKeysInProfile gsettingsBackendUnsetAllChangedPluginKeysInProfile;
    CCSGSettingsBackendUpdateProfile gsettingsBackendUpdateProfile;
    CCSGSettingsBackendUpdateCurrentProfileName gsettingsBackendUpdateCurrentProfileName;
    CCSGSettingsBackendAddProfile gsettingsBackendAddProfile;
    CCSGSettingsBackendGetIntegratedSetting gsettingsBackendGetIntegratedSetting;
    CCSGSettingsBackendReadIntegratedOption gsettingsBackendReadIntegratedOption;
    CCSGSettingsBackendWriteIntegratedOption gsettingsBackendWriteIntegratedOption;
};

unsigned int ccsCCSGSettingsBackendInterfaceGetType ();

gboolean
ccsGSettingsBackendUpdateProfile (CCSBackend *backend, CCSContext *context);

void
ccsGSettingsBackendUpdateCurrentProfileName (CCSBackend *backend, const char *profile);

CCSContext *
ccsGSettingsBackendGetContext (CCSBackend *backend);

void
ccsGSettingsBackendConnectToChangedSignal (CCSBackend *backend, CCSGSettingsWrapper *object);

CCSGSettingsWrapper *
ccsGSettingsGetSettingsObjectForPluginWithPath (CCSBackend *backend,
						const char *plugin,
						const char *path,
						CCSContext *context);

const char *
ccsGSettingsBackendGetCurrentProfile (CCSBackend *backend);

GVariant *
ccsGSettingsBackendGetExistingProfiles (CCSBackend *backend);

void
ccsGSettingsBackendSetExistingProfiles (CCSBackend *backend, GVariant *value);

void
ccsGSettingsBackendSetCurrentProfile (CCSBackend *backend, const gchar *value);

GVariant *
ccsGSettingsBackendGetPluginsWithSetKeys (CCSBackend *backend);

void
ccsGSettingsBackendClearPluginsWithSetKeys (CCSBackend *backend);

void
ccsGSettingsBackendUnsetAllChangedPluginKeysInProfile (CCSBackend *backend,
						       CCSContext *context,
						       GVariant   *pluginKeys,
						       const char *profile);

void
ccsGSettingsBackendAddProfile (CCSBackend *backend,
			       const char *profile);


CCSIntegratedSetting *
ccsGSettingsBackendGetIntegratedSetting (CCSBackend *backend,
					 CCSSetting *setting);

Bool
ccsGSettingsBackendReadIntegratedOption (CCSBackend *backend,
					 CCSSetting *setting,
					 CCSIntegratedSetting *integrated);
void
ccsGSettingsBackendWriteIntegratedOption (CCSBackend *backend,
					  CCSSetting *setting,
					  CCSIntegratedSetting *integrated);

COMPIZCONFIG_END_DECLS

#endif
