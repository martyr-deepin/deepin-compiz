#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <ccs.h>

#include "ccs_gsettings_backend_mock.h"

CCSGSettingsBackendInterface ccsGSettingsBackendGMockInterface =
{
    CCSGSettingsBackendGMock::ccsGSettingsBackendGetContext,
    CCSGSettingsBackendGMock::ccsGSettingsBackendConnectToValueChangedSignal,
    CCSGSettingsBackendGMock::ccsGSettingsBackendGetSettingsObjectForPluginWithPath,
    CCSGSettingsBackendGMock::ccsGSettingsBackendGetCurrentProfile,
    CCSGSettingsBackendGMock::ccsGSettingsBackendGetExistingProfiles,
    CCSGSettingsBackendGMock::ccsGSettingsBackendSetExistingProfiles,
    CCSGSettingsBackendGMock::ccsGSettingsBackendSetCurrentProfile,
    CCSGSettingsBackendGMock::ccsGSettingsBackendGetPluginsWithSetKeys,
    CCSGSettingsBackendGMock::ccsGSettingsBackendClearPluginsWithSetKeys,
    CCSGSettingsBackendGMock::ccsGSettingsBackendUnsetAllChangedPluginKeysInProfile,
    CCSGSettingsBackendGMock::ccsGSettingsBackendUpdateProfile,
    CCSGSettingsBackendGMock::ccsGSettingsBackendUpdateCurrentProfileName,
    CCSGSettingsBackendGMock::ccsGSettingsBackendAddProfile,
    CCSGSettingsBackendGMock::ccsGSettingsBackendGetIntegratedSetting,
    CCSGSettingsBackendGMock::ccsGSettingsBackendReadIntegratedOption,
    CCSGSettingsBackendGMock::ccsGSettingsBackendWriteIntegratedOption
};

CCSBackend *
ccsGSettingsBackendGMockNew ()
{
    CCSBackend *backend = (CCSBackend *) calloc (1, sizeof (CCSBackend));

    if (!backend)
	return NULL;

    CCSGSettingsBackendGMock *gmock = new CCSGSettingsBackendGMock (backend);

    if (!gmock)
    {
	free (backend);
	return NULL;
    }

    ccsObjectInit (backend, &ccsDefaultObjectAllocator);
    ccsObjectAddInterface (backend, (const CCSInterface *) &ccsGSettingsBackendGMockInterface, GET_INTERFACE_TYPE (CCSGSettingsBackendInterface));
    ccsObjectSetPrivate (backend, reinterpret_cast <CCSPrivate *> (gmock));

    return backend;
}

void
ccsGSettingsBackendGMockFree (CCSBackend *backend)
{
    CCSGSettingsBackendGMock *mock = reinterpret_cast <CCSGSettingsBackendGMock *> (ccsObjectGetPrivate (backend));

    delete mock;

    ccsObjectSetPrivate (backend, NULL);
    ccsObjectFinalize (backend);

    free (backend);
}
