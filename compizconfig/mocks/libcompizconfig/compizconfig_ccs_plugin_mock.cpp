#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <ccs.h>

#include "compizconfig_ccs_plugin_mock.h"

CCSPluginInterface CCSPluginGMockInterface =
{
    CCSPluginGMock::ccsPluginGetName,
    CCSPluginGMock::ccsPluginGetShortDesc,
    CCSPluginGMock::ccsPluginGetLongDesc,
    CCSPluginGMock::ccsPluginGetHints,
    CCSPluginGMock::ccsPluginGetCategory,
    CCSPluginGMock::ccsPluginGetLoadAfter,
    CCSPluginGMock::ccsPluginGetLoadBefore,
    CCSPluginGMock::ccsPluginGetRequiresPlugins,
    CCSPluginGMock::ccsPluginGetConflictPlugins,
    CCSPluginGMock::ccsPluginGetProvidesFeatures,
    CCSPluginGMock::ccsPluginGetRequiresFeatures,
    CCSPluginGMock::ccsPluginGetPrivatePtr,
    CCSPluginGMock::ccsPluginSetPrivatePtr,
    CCSPluginGMock::ccsPluginGetContext,
    CCSPluginGMock::ccsFindSetting,
    CCSPluginGMock::ccsGetPluginSettings,
    CCSPluginGMock::ccsGetPluginGroups,
    CCSPluginGMock::ccsReadPluginSettings,
    CCSPluginGMock::ccsGetPluginStrExtensions,
    CCSPluginGMock::ccsFreePlugin
};

CCSPlugin *
ccsMockPluginNew ()
{
    CCSPlugin *plugin = (CCSPlugin *) calloc (1, sizeof (CCSPlugin));

    if (!plugin)
	return NULL;

    ccsObjectInit (plugin, &ccsDefaultObjectAllocator);

    CCSPluginGMock *mock = new CCSPluginGMock (plugin);
    ccsObjectSetPrivate (plugin, (CCSPrivate *) mock);
    ccsObjectAddInterface (plugin, (CCSInterface *) &CCSPluginGMockInterface, GET_INTERFACE_TYPE (CCSPluginInterface));

    ccsPluginRef (plugin);

    return plugin;
}

void
ccsFreeMockPlugin (CCSPlugin *plugin)
{
    /* Need to delete the mock correctly */

    CCSPluginGMock *mock = (CCSPluginGMock *) ccsObjectGetPrivate (plugin);

    delete mock;

    ccsObjectSetPrivate (plugin, NULL);
    ccsObjectFinalize (plugin);

    free (plugin);
}
