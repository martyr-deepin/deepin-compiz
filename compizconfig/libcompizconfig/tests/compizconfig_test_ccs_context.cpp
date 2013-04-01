#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <ccs.h>

#include "compizconfig_ccs_context_mock.h"

using ::testing::_;
using ::testing::Return;

class CCSContextTest :
    public ::testing::Test
{
};

TEST(CCSContextTest, TestMock)
{
    CCSContext *context = ccsMockContextNew ();
    CCSContextGMock *mock = (CCSContextGMock *) ccsObjectGetPrivate (context);

    EXPECT_CALL (*mock, getPlugins ());
    EXPECT_CALL (*mock, getCategories ());
    EXPECT_CALL (*mock, getChangedSettings ());
    EXPECT_CALL (*mock, getScreenNum ());
    EXPECT_CALL (*mock, addChangedSetting (_));
    EXPECT_CALL (*mock, clearChangedSettings ());
    EXPECT_CALL (*mock, stealChangedSettings ());
    EXPECT_CALL (*mock, getPrivatePtr ());
    EXPECT_CALL (*mock, setPrivatePtr (_));
    EXPECT_CALL (*mock, loadPlugin (_));
    EXPECT_CALL (*mock, findPlugin (_));
    EXPECT_CALL (*mock, pluginIsActive (_));
    EXPECT_CALL (*mock, getActivePluginList ());
    EXPECT_CALL (*mock, getSortedPluginStringList ());
    EXPECT_CALL (*mock, setBackend (_));
    EXPECT_CALL (*mock, getBackend ());
    EXPECT_CALL (*mock, setIntegrationEnabled (_));
    EXPECT_CALL (*mock, setProfile (_));
    EXPECT_CALL (*mock, setPluginListAutoSort (_));
    EXPECT_CALL (*mock, getIntegrationEnabled ());
    EXPECT_CALL (*mock, getProfile ());
    EXPECT_CALL (*mock, getPluginListAutoSort ());
    EXPECT_CALL (*mock, processEvents (_));
    EXPECT_CALL (*mock, readSettings ());
    EXPECT_CALL (*mock, writeSettings ());
    EXPECT_CALL (*mock, writeChangedSettings ());
    EXPECT_CALL (*mock, exportToFile (_, _));
    EXPECT_CALL (*mock, importFromFile (_, _));
    EXPECT_CALL (*mock, canEnablePlugin (_));
    EXPECT_CALL (*mock, canDisablePlugin (_));
    EXPECT_CALL (*mock, getExistingProfiles ());
    EXPECT_CALL (*mock, deleteProfile (_));

    char *foo = strdup ("foo");
    char *bar = strdup ("bar");

    ccsContextGetPlugins (context);
    ccsContextGetCategories (context);
    ccsContextGetChangedSettings (context);
    ccsContextGetScreenNum (context);
    ccsContextAddChangedSetting (context, NULL);
    ccsContextClearChangedSettings (context);
    ccsContextStealChangedSettings (context);
    ccsContextGetPrivatePtr (context);
    ccsContextSetPrivatePtr (context, NULL);
    ccsLoadPlugin (context, foo);
    ccsFindPlugin (context, foo);
    ccsPluginIsActive (context, foo);
    ccsGetActivePluginList (context);
    ccsGetSortedPluginStringList (context);
    ccsSetBackend (context, bar);
    ccsGetBackend (context);
    ccsSetIntegrationEnabled (context, TRUE);
    ccsSetProfile (context, foo);
    ccsSetPluginListAutoSort (context, TRUE);
    ccsGetIntegrationEnabled (context);
    ccsGetProfile (context);
    ccsGetPluginListAutoSort (context);
    ccsProcessEvents (context, 0);
    ccsReadSettings (context);
    ccsWriteSettings (context);
    ccsWriteChangedSettings (context);
    ccsExportToFile (context, bar, TRUE);
    ccsImportFromFile (context, foo, FALSE);
    ccsCanEnablePlugin (context, NULL);
    ccsCanDisablePlugin (context, NULL);
    ccsGetExistingProfiles (context);
    ccsDeleteProfile (context, foo);

    free (foo);
    free (bar);

    ccsFreeContext (context);
}
