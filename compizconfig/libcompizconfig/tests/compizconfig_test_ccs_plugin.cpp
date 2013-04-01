#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <ccs.h>

#include "compizconfig_ccs_plugin_mock.h"

using ::testing::_;
using ::testing::Return;

class CCSPluginTest :
    public ::testing::Test
{
};

TEST(CCSPluginTest, TestMock)
{
    CCSPlugin *plugin = ccsMockPluginNew ();
    CCSPluginGMock *mock = (CCSPluginGMock *) ccsObjectGetPrivate (plugin);

    EXPECT_CALL (*mock, getName ());
    EXPECT_CALL (*mock, getShortDesc ());
    EXPECT_CALL (*mock, getLongDesc ());
    EXPECT_CALL (*mock, getHints ());
    EXPECT_CALL (*mock, getCategory ());
    EXPECT_CALL (*mock, getLoadAfter ());
    EXPECT_CALL (*mock, getLoadBefore ());
    EXPECT_CALL (*mock, getRequiresPlugins ());
    EXPECT_CALL (*mock, getConflictPlugins ());
    EXPECT_CALL (*mock, getProvidesFeatures ());
    EXPECT_CALL (*mock, getRequiresFeatures ());
    EXPECT_CALL (*mock, getPrivatePtr ());
    EXPECT_CALL (*mock, setPrivatePtr (_));
    EXPECT_CALL (*mock, getContext ());
    EXPECT_CALL (*mock, findSetting (_));
    EXPECT_CALL (*mock, getPluginGroups ());
    EXPECT_CALL (*mock, readPluginSettings ());
    EXPECT_CALL (*mock, getPluginStrExtensions ());

    ccsPluginGetName (plugin);
    ccsPluginGetShortDesc  (plugin);
    ccsPluginGetLongDesc  (plugin);
    ccsPluginGetHints (plugin);
    ccsPluginGetCategory (plugin);
    ccsPluginGetLoadAfter (plugin);
    ccsPluginGetLoadBefore (plugin);
    ccsPluginGetRequiresPlugins (plugin);
    ccsPluginGetConflictPlugins (plugin);
    ccsPluginGetProvidesFeatures (plugin);
    ccsPluginGetRequiresFeatures (plugin);
    ccsPluginGetPrivatePtr (plugin);
    ccsPluginSetPrivatePtr (plugin, NULL);
    ccsPluginGetContext (plugin);
    ccsFindSetting (plugin, "foo");
    ccsGetPluginGroups (plugin);
    ccsReadPluginSettings (plugin);
    ccsGetPluginStrExtensions (plugin);

    ccsPluginUnref (plugin);
}
