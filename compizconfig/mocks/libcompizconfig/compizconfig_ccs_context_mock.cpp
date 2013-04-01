#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <ccs.h>

#include "compizconfig_ccs_context_mock.h"

CCSContextInterface CCSContextGMockInterface =
{
    CCSContextGMock::ccsContextGetPlugins,
    CCSContextGMock::ccsContextGetCategories,
    CCSContextGMock::ccsContextGetChangedSettings,
    CCSContextGMock::ccsContextGetScreenNum,
    CCSContextGMock::ccsContextAddChangedSetting,
    CCSContextGMock::ccsContextClearChangedSettings,
    CCSContextGMock::ccsContextStealChangedSettings,
    CCSContextGMock::ccsContextGetPrivatePtr,
    CCSContextGMock::ccsContextSetPrivatePtr,
    CCSContextGMock::ccsLoadPlugin,
    CCSContextGMock::ccsFindPlugin,
    CCSContextGMock::ccsPluginIsActive,
    CCSContextGMock::ccsGetActivePluginList,
    CCSContextGMock::ccsGetSortedPluginStringList,
    CCSContextGMock::ccsSetBackend,
    CCSContextGMock::ccsGetBackend,
    CCSContextGMock::ccsSetIntegrationEnabled,
    CCSContextGMock::ccsSetProfile,
    CCSContextGMock::ccsSetPluginListAutoSort,
    CCSContextGMock::ccsGetProfile,
    CCSContextGMock::ccsGetIntegrationEnabled,
    CCSContextGMock::ccsGetPluginListAutoSort,
    CCSContextGMock::ccsProcessEvents,
    CCSContextGMock::ccsReadSettings,
    CCSContextGMock::ccsWriteSettings,
    CCSContextGMock::ccsWriteChangedSettings,
    CCSContextGMock::ccsExportToFile,
    CCSContextGMock::ccsImportFromFile,
    CCSContextGMock::ccsCanEnablePlugin,
    CCSContextGMock::ccsCanDisablePlugin,
    CCSContextGMock::ccsGetExistingProfiles,
    CCSContextGMock::ccsDeleteProfile,
    CCSContextGMock::ccsCheckForSettingsUpgrade,
    CCSContextGMock::ccsLoadPlugins,
    CCSContextGMock::ccsFreeContext
};

CCSContext *
ccsMockContextNew ()
{
    CCSContext *context = (CCSContext *) calloc (1, sizeof (CCSContext));

    if (!context)
	return NULL;

    ccsObjectInit (context, &ccsDefaultObjectAllocator);

    CCSContextGMock *mock = new CCSContextGMock (context);
    ccsObjectSetPrivate (context, (CCSPrivate *) mock);
    ccsObjectAddInterface (context, (CCSInterface *) &CCSContextGMockInterface, GET_INTERFACE_TYPE (CCSContextInterface));

    return context;
}

void
ccsFreeMockContext (CCSContext *context)
{
    /* Need to delete the mock correctly */

    CCSContextGMock *mock = (CCSContextGMock *) ccsObjectGetPrivate (context);

    delete mock;

    ccsObjectSetPrivate (context, NULL);
    ccsObjectFinalize (context);

    free (context);
}
