#include "core/plugin.h"

// This prevents an instantiation error - not sure why ATM
#include "core/screen.h"

// Get rid of stupid macro from X.h
// Why, oh why, are we including X.h?
#undef None

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace {

class PluginFilesystem
{
public:
    virtual bool
    LoadPlugin(CompPlugin *p, const char *path, const char *name) const = 0;

    virtual void
    UnloadPlugin(CompPlugin *p) const = 0;

    static PluginFilesystem const* instance;

protected:
    PluginFilesystem();
    virtual ~PluginFilesystem() {}
};

class MockPluginFilesystem : public PluginFilesystem
{
public:
    MOCK_CONST_METHOD3(LoadPlugin, bool (CompPlugin *, const char *, const char *));

    MOCK_CONST_METHOD1(UnloadPlugin, void (CompPlugin *p));
};

class MockVTable : public CompPlugin::VTable
{
public:
    MOCK_METHOD0(init, bool ());
};


bool
ThunkLoadPluginProc(CompPlugin *p, const char *path_, const char *name)
{
    return PluginFilesystem::instance->LoadPlugin(p, path_, name);
}

void
ThunkUnloadPluginProc(CompPlugin *p)
{
    PluginFilesystem::instance->UnloadPlugin(p);
}


PluginFilesystem::PluginFilesystem()
{
	::loaderLoadPlugin = ::ThunkLoadPluginProc;
	::loaderUnloadPlugin = ::ThunkUnloadPluginProc;

	instance = this;
}

PluginFilesystem const* PluginFilesystem::instance = 0;

} // (abstract) namespace



TEST(PluginTest, load_non_existant_plugin_must_fail)
{
    MockPluginFilesystem mockfs;

    using namespace testing;

    EXPECT_CALL(mockfs, LoadPlugin(Ne((void*)0), EndsWith(HOME_PLUGINDIR), StrEq("dummy"))).
	WillOnce(Return(false));

    EXPECT_CALL(mockfs, LoadPlugin(Ne((void*)0), EndsWith(PLUGINDIR), StrEq("dummy"))).
	WillOnce(Return(false));

    EXPECT_CALL(mockfs, LoadPlugin(Ne((void*)0), Eq((void*)0), StrEq("dummy"))).
	WillOnce(Return(false));

    EXPECT_CALL(mockfs, UnloadPlugin(_)).Times(AtMost(0));

    ASSERT_EQ(0, CompPlugin::load("dummy"));
}

TEST(PluginTest, load_plugin_from_HOME_PLUGINDIR_succeeds)
{
    MockPluginFilesystem mockfs;

    using namespace testing;

    EXPECT_CALL(mockfs, LoadPlugin(Ne((void*)0), EndsWith(HOME_PLUGINDIR), StrEq("dummy"))).
	WillOnce(Return(true));

    EXPECT_CALL(mockfs, LoadPlugin(Ne((void*)0), EndsWith(PLUGINDIR), StrEq("dummy"))).
	Times(AtMost(0));

    EXPECT_CALL(mockfs, LoadPlugin(Ne((void*)0), Eq((void*)0), StrEq("dummy"))).
	Times(AtMost(0));

    EXPECT_CALL(mockfs, UnloadPlugin(_)).Times(1);

    CompPlugin* cp = CompPlugin::load("dummy");
    ASSERT_NE((void*)0, cp);

    CompPlugin::unload(cp);
}

TEST(PluginTest, load_plugin_from_PLUGINDIR_succeeds)
{
    MockPluginFilesystem mockfs;

    using namespace testing;

    EXPECT_CALL(mockfs, LoadPlugin(Ne((void*)0), EndsWith(HOME_PLUGINDIR), StrEq("dummy"))).
	WillOnce(Return(false));

    EXPECT_CALL(mockfs, LoadPlugin(Ne((void*)0), EndsWith(PLUGINDIR), StrEq("dummy"))).
	WillOnce(Return(true));

    EXPECT_CALL(mockfs, LoadPlugin(Ne((void*)0), Eq((void*)0), StrEq("dummy"))).
	    Times(AtMost(0));;

    EXPECT_CALL(mockfs, UnloadPlugin(_)).Times(1);

    CompPlugin* cp = CompPlugin::load("dummy");
    ASSERT_NE((void*)0, cp);

    CompPlugin::unload(cp);
}

TEST(PluginTest, load_plugin_from_void_succeeds)
{
    MockPluginFilesystem mockfs;

    using namespace testing;

    EXPECT_CALL(mockfs, LoadPlugin(Ne((void*)0), EndsWith(HOME_PLUGINDIR), StrEq("dummy"))).
	WillOnce(Return(false));

    EXPECT_CALL(mockfs, LoadPlugin(Ne((void*)0), EndsWith(PLUGINDIR), StrEq("dummy"))).
	WillOnce(Return(false));

    EXPECT_CALL(mockfs, LoadPlugin(Ne((void*)0), Eq((void*)0), StrEq("dummy"))).
	WillOnce(Return(true));

    EXPECT_CALL(mockfs, UnloadPlugin(_)).Times(1);

    CompPlugin* cp = CompPlugin::load("dummy");
    ASSERT_NE((void*)0, cp);

    CompPlugin::unload(cp);
}

TEST(PluginTest, when_we_push_plugin_init_is_called)
{
    MockPluginFilesystem mockfs;

    using namespace testing;

    EXPECT_CALL(mockfs, LoadPlugin(Ne((void*)0), EndsWith(HOME_PLUGINDIR), StrEq("dummy"))).
	WillOnce(Return(true));

    EXPECT_CALL(mockfs, LoadPlugin(Ne((void*)0), EndsWith(PLUGINDIR), StrEq("dummy"))).
	Times(AtMost(0));

    EXPECT_CALL(mockfs, LoadPlugin(Ne((void*)0), Eq((void*)0), StrEq("dummy"))).
	Times(AtMost(0));

    EXPECT_CALL(mockfs, UnloadPlugin(_)).Times(1);

    MockVTable mockVtable;
    EXPECT_CALL(mockVtable, init()).WillOnce(Return(true));

    CompPlugin* cp = CompPlugin::load("dummy");

    cp->vTable = &mockVtable;

    CompPlugin::push(cp);
    ASSERT_EQ(cp, CompPlugin::pop());
    CompPlugin::unload(cp);
}
