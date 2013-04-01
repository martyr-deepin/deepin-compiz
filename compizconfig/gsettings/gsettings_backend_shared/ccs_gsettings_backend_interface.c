#include <ccs-backend.h>
#include "ccs_gsettings_backend_interface.h"

INTERFACE_TYPE (CCSGSettingsBackendInterface);

CCSContext *
ccsGSettingsBackendGetContext (CCSBackend *backend)
{
    return (*(GET_INTERFACE (CCSGSettingsBackendInterface, backend))->gsettingsBackendGetContext) (backend);
}


void
ccsGSettingsBackendConnectToChangedSignal (CCSBackend *backend,
					   CCSGSettingsWrapper *object)
{
     (*(GET_INTERFACE (CCSGSettingsBackendInterface, backend))->gsettingsBackendConnectToChangedSignal) (backend, object);
}

CCSGSettingsWrapper *
ccsGSettingsGetSettingsObjectForPluginWithPath (CCSBackend *backend,
						const char *plugin,
						const char *path,
						CCSContext *context)
{
    return (*(GET_INTERFACE (CCSGSettingsBackendInterface, backend))->gsettingsBackendGetSettingsObjectForPluginWithPath) (backend, plugin, path, context);
}

const char *
ccsGSettingsBackendGetCurrentProfile (CCSBackend *backend)
{
    return (*(GET_INTERFACE (CCSGSettingsBackendInterface, backend))->gsettingsBackendGetCurrentProfile) (backend);
}

GVariant *
ccsGSettingsBackendGetExistingProfiles (CCSBackend *backend)
{
    return (*(GET_INTERFACE (CCSGSettingsBackendInterface, backend))->gsettingsBackendGetExistingProfiles) (backend);
}

void
ccsGSettingsBackendSetExistingProfiles (CCSBackend *backend, GVariant *value)
{
    (*(GET_INTERFACE (CCSGSettingsBackendInterface, backend))->gsettingsBackendSetExistingProfiles) (backend, value);
}

void
ccsGSettingsBackendSetCurrentProfile (CCSBackend *backend, const gchar *value)
{
    (*(GET_INTERFACE (CCSGSettingsBackendInterface, backend))->gsettingsBackendSetCurrentProfile) (backend, value);
}

GVariant *
ccsGSettingsBackendGetPluginsWithSetKeys (CCSBackend *backend)
{
    return (*(GET_INTERFACE (CCSGSettingsBackendInterface, backend))->gsettingsBackendGetPluginsWithSetKeys) (backend);
}

void
ccsGSettingsBackendClearPluginsWithSetKeys (CCSBackend *backend)
{
    (*(GET_INTERFACE (CCSGSettingsBackendInterface, backend))->gsettingsBackendClearPluginsWithSetKeys) (backend);
}

void
ccsGSettingsBackendUnsetAllChangedPluginKeysInProfile (CCSBackend *backend,
						       CCSContext *context,
						       GVariant   *pluginKeys,
						       const char *profile)
{
    (*(GET_INTERFACE (CCSGSettingsBackendInterface, backend))->gsettingsBackendUnsetAllChangedPluginKeysInProfile) (backend, context, pluginKeys, profile);
}

gboolean
ccsGSettingsBackendUpdateProfile (CCSBackend *backend, CCSContext *context)
{
    return (*(GET_INTERFACE (CCSGSettingsBackendInterface, backend))->gsettingsBackendUpdateProfile) (backend, context);
}

void
ccsGSettingsBackendUpdateCurrentProfileName (CCSBackend *backend, const char *profile)
{
    (*(GET_INTERFACE (CCSGSettingsBackendInterface, backend))->gsettingsBackendUpdateCurrentProfileName) (backend, profile);
}

void
ccsGSettingsBackendAddProfile (CCSBackend *backend, const char *profile)
{
    (*(GET_INTERFACE (CCSGSettingsBackendInterface, backend))->gsettingsBackendAddProfile) (backend, profile);
}

CCSIntegratedSetting * ccsGSettingsBackendGetIntegratedSetting (CCSBackend *backend,
								CCSSetting *setting)
{
    return (*(GET_INTERFACE (CCSGSettingsBackendInterface, backend))->gsettingsBackendGetIntegratedSetting) (backend, setting);
}

Bool
ccsGSettingsBackendReadIntegratedOption (CCSBackend	      *backend,
					 CCSSetting	      *setting,
					 CCSIntegratedSetting *integrated)
{
    return (*(GET_INTERFACE (CCSGSettingsBackendInterface, backend))->gsettingsBackendReadIntegratedOption) (backend, setting, integrated);
}

void
ccsGSettingsBackendWriteIntegratedOption (CCSBackend	       *backend,
					  CCSSetting	       *setting,
					  CCSIntegratedSetting *integrated)
{
    (*(GET_INTERFACE (CCSGSettingsBackendInterface, backend))->gsettingsBackendWriteIntegratedOption) (backend, setting, integrated);
}
