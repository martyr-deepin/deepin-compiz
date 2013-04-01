#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <ccs.h>

#include "compizconfig_ccs_backend_mock.h"

CCSBackendInterface CCSBackendGMockInterface =
{
    CCSBackendGMock::ccsBackendGetInfo,
    CCSBackendGMock::ccsBackendExecuteEvents,
    CCSBackendGMock::ccsBackendInit,
    CCSBackendGMock::ccsBackendFini,
    CCSBackendGMock::ccsBackendReadInit,
    CCSBackendGMock::ccsBackendReadSetting,
    CCSBackendGMock::ccsBackendReadDone,
    CCSBackendGMock::ccsBackendWriteInit,
    CCSBackendGMock::ccsBackendWriteSetting,
    CCSBackendGMock::ccsBackendWriteDone,
    CCSBackendGMock::ccsBackendUpdateSetting,
    CCSBackendGMock::ccsBackendGetSettingIsIntegrated,
    CCSBackendGMock::ccsBackendGetSettingIsReadOnly,
    CCSBackendGMock::ccsBackendGetExistingProfiles,
    CCSBackendGMock::ccsBackendDeleteProfile
};

CCSBackend *
ccsMockBackendNew ()
{
    CCSBackend *backend = (CCSBackend *) calloc (1, sizeof (CCSBackend));

    if (!backend)
	return NULL;

    ccsObjectInit (backend, &ccsDefaultObjectAllocator);

    CCSBackendGMock *mock = new CCSBackendGMock (backend);
    ccsObjectSetPrivate (backend, (CCSPrivate *) mock);
    ccsObjectAddInterface (backend, (CCSInterface *) &CCSBackendGMockInterface, GET_INTERFACE_TYPE (CCSBackendInterface));

    ccsBackendRef (backend);

    return backend;
}

void
ccsFreeMockBackend (CCSBackend *backend)
{
    /* Need to delete the mock correctly */

    CCSBackendGMock *mock = (CCSBackendGMock *) ccsObjectGetPrivate (backend);

    delete mock;

    ccsObjectSetPrivate (backend, NULL);
    ccsObjectFinalize (backend);

    free (backend);
}
