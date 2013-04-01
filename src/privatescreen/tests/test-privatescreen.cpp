#include "privatescreen.h"


// Get rid of stupid macro from X.h
// Why, oh why, are we including X.h?
#undef None

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <stdlib.h>

using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::_;

namespace {

class MockCompScreen : public CompScreen
{
public:
    MockCompScreen()
    {
	// The PluginManager ctor uses screen->... (indirectly via CoreOptions)
	// We should kill this dependency
	screen = this;
    }

    ~MockCompScreen()
    {
	// Because of another indirect use of screen in PluginManager dtor
	// via option.cpp:finiOptionValue()
	screen = 0;
    }

    // Interface hoisted from CompScreen
    MOCK_METHOD0(updateDefaultIcon, bool ());
    MOCK_METHOD0(dpy, Display * ());
    MOCK_METHOD0(root, Window ());
    MOCK_CONST_METHOD0(vpSize, const CompSize  & () );
    MOCK_METHOD1(forEachWindow, void (CompWindow::ForEach));
    MOCK_METHOD0(windows, CompWindowList & ());
    MOCK_METHOD3(moveViewport, void (int tx, int ty, bool sync));
    MOCK_CONST_METHOD0(vp,const CompPoint & ());
    MOCK_METHOD0(updateWorkarea, void ());
    MOCK_METHOD1(addAction, bool (CompAction *action));
    MOCK_METHOD1(findWindow, CompWindow * (Window id));

    MOCK_METHOD2(findTopLevelWindow, CompWindow * (
	    Window id, bool   override_redirect));
    MOCK_METHOD6(toolkitAction, void (
	    Atom   toolkitAction,
	    Time   eventTime,
	    Window window,
	    long   data0,
	    long   data1,
	    long   data2));
    MOCK_CONST_METHOD0(showingDesktopMask, unsigned int ());
    MOCK_CONST_METHOD0(grabsEmpty, bool ());
    MOCK_METHOD1(sizePluginClasses, void (unsigned int size));

    MOCK_METHOD1(_initPluginForScreen, bool (CompPlugin *));
    MOCK_METHOD1(_finiPluginForScreen, void (CompPlugin *));
    MOCK_METHOD3(_setOptionForPlugin, bool (const char *, const char *, CompOption::Value &));
    MOCK_METHOD1(_handleEvent, void (XEvent *event));
    MOCK_METHOD3(_logMessage, void (const char *, CompLogLevel, const char*));
    MOCK_METHOD0(_enterShowDesktopMode, void ());
    MOCK_METHOD1(_leaveShowDesktopMode, void (CompWindow *));
    MOCK_METHOD1(_addSupportedAtoms, void (std::vector<Atom>& atoms));

    MOCK_METHOD1(_fileWatchAdded, void (CompFileWatch *));
    MOCK_METHOD1(_fileWatchRemoved, void (CompFileWatch *));
    MOCK_METHOD2(_sessionEvent, void (CompSession::Event, CompOption::Vector &));
    MOCK_METHOD3(_handleCompizEvent, void (const char *, const char *, CompOption::Vector &));
    MOCK_METHOD4(_fileToImage, bool (CompString &, CompSize &, int &, void *&));
    MOCK_METHOD5(_imageToFile, bool (CompString &, CompString &, CompSize &, int, void *));
    MOCK_METHOD1(_matchInitExp, CompMatch::Expression * (const CompString&));
    MOCK_METHOD0(_matchExpHandlerChanged, void ());
    MOCK_METHOD1(_matchPropertyChanged, void (CompWindow *));
    MOCK_METHOD0(_outputChangeNotify, void ());

    MOCK_METHOD0(outputDevs, CompOutput::vector & ());
    MOCK_METHOD2(setWindowState, void (unsigned int state, Window id));
    MOCK_METHOD0(XShape, bool ());
    MOCK_METHOD0(screenInfo, std::vector<XineramaScreenInfo> & ());
    MOCK_METHOD0(serverWindows, CompWindowList & ());
    MOCK_METHOD3(setWindowProp, void (Window       id,
			    Atom         property,
			    unsigned int value));
    MOCK_METHOD0(activeWindow, Window ());
    MOCK_METHOD0(currentDesktop, unsigned int ());
    MOCK_METHOD0(currentHistory, CompActiveWindowHistory *());
    MOCK_METHOD0(focusDefaultWindow, void ());
    MOCK_METHOD0(getCurrentTime, Time ());
    MOCK_METHOD3(getWindowProp, unsigned int (Window       id,
				    Atom         property,
				    unsigned int defaultValue));
    MOCK_METHOD2(insertServerWindow, void (CompWindow *w, Window aboveId));
    MOCK_METHOD2(insertWindow, void (CompWindow *w, Window aboveId));
    MOCK_METHOD0(nDesktop, unsigned int ());
    MOCK_METHOD1(outputDeviceForGeometry, int (const CompWindow::Geometry& gm));
    MOCK_METHOD0(screenNum, int ());
    MOCK_METHOD1(unhookServerWindow, void (CompWindow *w));
    MOCK_METHOD1(unhookWindow, void (CompWindow *w));
    MOCK_METHOD2(viewportForGeometry, void (const CompWindow::Geometry &gm,
				  CompPoint                   &viewport));

    MOCK_METHOD1(addToDestroyedWindows, void (CompWindow * cw));

    MOCK_CONST_METHOD0(workArea, CompRect const& ());
    MOCK_METHOD1(removeAction, void (CompAction *action));
    MOCK_METHOD0(getOptions, CompOption::Vector & ());
    MOCK_METHOD2(setOption, bool (const CompString &name, CompOption::Value &value));
    MOCK_METHOD2(storeValue, void (CompString key, CompPrivate value));
    MOCK_METHOD1(hasValue, bool (CompString key));
    MOCK_METHOD1(getValue, CompPrivate (CompString key));
    MOCK_METHOD1(eraseValue, void (CompString key));
    MOCK_METHOD3(addWatchFd, CompWatchFdHandle (int             fd,
				      short int       events,
				      FdWatchCallBack callBack));
    MOCK_METHOD1(removeWatchFd, void (CompWatchFdHandle handle));
    MOCK_METHOD0(eventLoop, void ());
    MOCK_METHOD3(addFileWatch, CompFileWatchHandle (const char        *path,
					  int               mask,
					  FileWatchCallBack callBack));
    MOCK_METHOD1(removeFileWatch, void (CompFileWatchHandle handle));
    MOCK_CONST_METHOD0(getFileWatches, const CompFileWatchList& ());
    MOCK_METHOD0(updateSupportedWmHints, void ());
    MOCK_METHOD0(destroyedWindows, CompWindowList & ());
    MOCK_CONST_METHOD0(region, const CompRegion & ());
    MOCK_METHOD0(hasOverlappingOutputs, bool ());
    MOCK_METHOD0(fullscreenOutput, CompOutput & ());
    MOCK_METHOD3(setWindowProp32, void (Window         id,
			      Atom           property,
			      unsigned short value));
    MOCK_METHOD3(getWindowProp32, unsigned short (Window         id,
    					Atom           property,
    					unsigned short defaultValue));
    MOCK_METHOD4(readImageFromFile, bool (CompString &name,
				CompString &pname,
				CompSize   &size,
				void       *&data));
    MOCK_METHOD0(desktopWindowCount, int ());
    MOCK_METHOD0(attrib, XWindowAttributes ());
    MOCK_CONST_METHOD0(defaultIcon, CompIcon *());
    virtual bool otherGrabExist (const char *, ...) { return false; }  // TODO How to mock?
    MOCK_METHOD2(pushGrab, GrabHandle (Cursor cursor, const char *name));
    MOCK_METHOD2(removeGrab, void (GrabHandle handle, CompPoint *restorePointer));
    MOCK_METHOD4(writeImageToFile, bool (CompString &path,
			       const char *format,
			       CompSize   &size,
			       void       *data));
    MOCK_METHOD1(runCommand, void (CompString command));
    MOCK_METHOD0(shouldSerializePlugins, bool ());
    MOCK_CONST_METHOD1(getWorkareaForOutput, const CompRect & (unsigned int outputNum));
    MOCK_CONST_METHOD0(currentOutputDev, CompOutput & ());
    MOCK_METHOD1(grabExist, bool (const char *));
    MOCK_METHOD0(invisibleCursor, Cursor ());
    MOCK_CONST_METHOD0(activeNum, unsigned int ());
    MOCK_METHOD1(sendWindowActivationRequest, void (Window id));
    MOCK_METHOD1(clientList, const CompWindowVector & (bool stackingOrder));
    MOCK_METHOD1(outputDeviceForPoint, int (const CompPoint &point));
    MOCK_METHOD2(outputDeviceForPoint, int (int x, int y));
    MOCK_METHOD0(xkbEvent, int ());
    MOCK_METHOD2(warpPointer, void (int dx, int dy));
    MOCK_METHOD2(updateGrab, void (GrabHandle handle, Cursor cursor));
    MOCK_METHOD0(shapeEvent, int ());
    MOCK_METHOD0(syncEvent, int ());
    MOCK_METHOD0(autoRaiseWindow, Window  ());
    MOCK_METHOD0(processEvents, void ());
    MOCK_METHOD1(alwaysHandleEvent, void (XEvent *event));
    MOCK_METHOD0(displayString, const char * ());
    MOCK_METHOD0(getCurrentOutputExtents, CompRect ());
    MOCK_METHOD0(normalCursor, Cursor ());
    MOCK_METHOD0(grabbed, bool ());
    MOCK_METHOD0(snDisplay, SnDisplay * ());
    MOCK_CONST_METHOD0(createFailed, bool ());
    MOCK_METHOD0(incrementDesktopWindowCount, void ());
    MOCK_METHOD0(decrementDesktopWindowCount, void ());
    MOCK_METHOD0(nextMapNum, unsigned int ());
    MOCK_CONST_METHOD0(updatePassiveKeyGrabs, void ());
    MOCK_METHOD1(updatePassiveButtonGrabs, void (Window serverFrame));
    MOCK_CONST_METHOD0(lastPing, unsigned int  ());

    MOCK_CONST_METHOD0(displayInitialised, bool ());
    MOCK_METHOD1(applyStartupProperties, void (CompWindow *window));
    MOCK_METHOD0(updateClientList, void ());
    MOCK_CONST_METHOD0(getTopWindow, CompWindow * ());
    MOCK_CONST_METHOD0(getTopServerWindow, CompWindow * ());
    MOCK_METHOD0(getCoreOptions, CoreOptions& ());
    MOCK_CONST_METHOD0(colormap, Colormap ());
    MOCK_METHOD1(setCurrentDesktop, void (unsigned int desktop));
    MOCK_CONST_METHOD0(activeWindow, Window ());
    MOCK_CONST_METHOD1(grabWindowIsNot, bool (Window w));
    MOCK_METHOD0(incrementPendingDestroys, void ());
    MOCK_METHOD1(setNextActiveWindow, void (Window id));
    MOCK_CONST_METHOD0(getNextActiveWindow, Window ());
    MOCK_METHOD0(focusTopMostWindow, CompWindow* ());

    MOCK_METHOD1(getWmState, int (Window id));
    MOCK_CONST_METHOD2(setWmState, void (int state, Window id));
    MOCK_CONST_METHOD3(getMwmHints, void (Window id,
			  unsigned int *func,
			  unsigned int *decor));
    MOCK_METHOD1(getProtocols, unsigned int (Window id));
    MOCK_METHOD1(getWindowType, unsigned int (Window id));
    MOCK_METHOD1(getWindowState, unsigned int (Window id));

    MOCK_METHOD0(grabServer, void ());
    MOCK_METHOD0(ungrabServer, void ());
    MOCK_METHOD0(syncServer, void ());
    MOCK_METHOD0(serverGrabInterface, ServerGrabInterface * ());
};

class MockViewportRetreival :
    public compiz::private_screen::ViewportRetrievalInterface
{
    public:

	MOCK_CONST_METHOD0(getCurrentViewport, const CompPoint & ());
	MOCK_CONST_METHOD0(viewportDimentions, const CompSize & ());
};

class StubActivePluginsOption : public CoreOptions
{
public:
    StubActivePluginsOption() : CoreOptions(false)
    {
	CompOption::Vector& mOptions = getOptions ();
	CompOption::Value::Vector list;
	CompOption::Value value;

	// active_plugins
	mOptions[CoreOptions::ActivePlugins].setName ("active_plugins", CompOption::TypeList);
	list.clear ();
	value.set(CompString ("core"));
	list.push_back (value);
    }

    bool setActivePlugins(const char*, const char* key, CompOption::Value & value)
    {
	return setOption(key, value);
    }
};
} // (anon) namespace

namespace {

class MockVTable: public CompPlugin::VTable {
public:
    MockVTable (CompString const& name) { initVTable (name); }

    MOCK_METHOD0(init, bool ());
    MOCK_METHOD0(fini, void ());

    MOCK_METHOD1(initScreen, bool (CompScreen *s));

    MOCK_METHOD1(finiScreen, void (CompScreen *s));

    MOCK_METHOD1(initWindow, bool (CompWindow *w));

    MOCK_METHOD1(finiWindow, void (CompWindow *w));

    MOCK_METHOD0(getOptions, CompOption::Vector & ());

    MOCK_METHOD2(setOption, bool (const CompString  &name, CompOption::Value &value));
};

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
    MockVTable mockVtableOne;
    MockVTable mockVtableTwo;
    MockVTable mockVtableThree;
    MockVTable mockVtableFour;

    MockPluginFilesystem() :
	mockVtableOne("one"),
    	mockVtableTwo("two"),
    	mockVtableThree("three"),
    	mockVtableFour("four")
    {}

    MOCK_CONST_METHOD3(LoadPlugin, bool (CompPlugin *, const char *, const char *));

    MOCK_CONST_METHOD1(UnloadPlugin, void (CompPlugin *p));

    bool DummyLoader(CompPlugin *p, const char * path, const char * name)
    {
	using namespace testing;
	if (strcmp(name, "one") == 0)
	{
	    p->vTable = &mockVtableOne;
	}
	else if (strcmp(name, "two") == 0)
	{
	    p->vTable = &mockVtableTwo;
	}
	else if (strcmp(name, "three") == 0)
	{
	    p->vTable = &mockVtableThree;
	}
	else
	{
	    p->vTable = &mockVtableFour;
	}
	return true;
    }
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

// tell GLib not to use the slice-allocator implementation
// and avoid spurious valgrind reporting
void glib_nice_for_valgrind() { setenv("G_SLICE", "always-malloc", true); }
int const init = (glib_nice_for_valgrind(), 0);

PluginFilesystem const* PluginFilesystem::instance = 0;

} // (abstract) namespace

namespace cps = compiz::private_screen;

TEST(privatescreen_PluginManagerTest, create_and_destroy)
{
    using namespace testing;

    MockCompScreen comp_screen;

    cps::PluginManager ps;
}

TEST(privatescreen_PluginManagerTest, calling_updatePlugins_does_not_error)
{
    using namespace testing;

    MockCompScreen comp_screen;

    cps::PluginManager ps;

    // Stuff that has to be done before calling updatePlugins()
    CompOption::Value::Vector values;
    values.push_back ("core");
    ps.setPlugins (values);
    ps.setDirtyPluginList ();

    // Now we can call updatePlugins() without a segfault.  Hoorah!
    EXPECT_CALL(comp_screen, _setOptionForPlugin(StrEq("core"), StrEq("active_plugins"), _)).
	WillOnce(Return(false));
    ps.updatePlugins(&comp_screen, StubActivePluginsOption().optionGetActivePlugins());
}

TEST(privatescreen_PluginManagerTest, calling_updatePlugins_after_setting_initialPlugins)
{
    using namespace testing;

    MockCompScreen comp_screen;

    cps::PluginManager ps;

    // Stuff that has to be done before calling updatePlugins()
    CompOption::Value::Vector values;
    values.push_back ("core");
    ps.setPlugins (values);
    ps.setDirtyPluginList ();

    std::list <CompString> plugins;
    plugins.push_back ("one");
    plugins.push_back ("two");
    plugins.push_back ("three");
    initialPlugins = plugins;

    MockPluginFilesystem mockfs;

    EXPECT_CALL(mockfs, LoadPlugin(Ne((void*)0), EndsWith(HOME_PLUGINDIR), StrEq("one"))).
	WillOnce(Invoke(&mockfs, &MockPluginFilesystem::DummyLoader));
    EXPECT_CALL(mockfs.mockVtableOne, init()).WillOnce(Return(true));

    EXPECT_CALL(mockfs, LoadPlugin(Ne((void*)0), EndsWith(HOME_PLUGINDIR), StrEq("two"))).
	WillOnce(Invoke(&mockfs, &MockPluginFilesystem::DummyLoader));
    EXPECT_CALL(mockfs.mockVtableTwo, init()).WillOnce(Return(true));

    EXPECT_CALL(mockfs, LoadPlugin(Ne((void*)0), EndsWith(HOME_PLUGINDIR), StrEq("three"))).
	WillOnce(Invoke(&mockfs, &MockPluginFilesystem::DummyLoader));
    EXPECT_CALL(mockfs.mockVtableThree, init()).WillOnce(Return(false));

    EXPECT_CALL(mockfs, UnloadPlugin(_)).Times(1);  // Once for "three" which doesn't load

    EXPECT_CALL(comp_screen, _setOptionForPlugin(StrEq("core"), StrEq("active_plugins"), _)).
	    WillOnce(Return(false));

    ps.updatePlugins(&comp_screen, StubActivePluginsOption().optionGetActivePlugins());

    Mock::VerifyAndClearExpectations(&mockfs);
    Mock::VerifyAndClearExpectations(&mockfs.mockVtableOne);
    Mock::VerifyAndClearExpectations(&mockfs.mockVtableTwo);
    Mock::VerifyAndClearExpectations(&mockfs.mockVtableThree);
    Mock::VerifyAndClearExpectations(&mockfs.mockVtableFour);

    // TODO Some cleanup that probably ought to be automatic.
    EXPECT_CALL(mockfs, UnloadPlugin(_)).Times(2);
    EXPECT_CALL(comp_screen, _finiPluginForScreen(Ne((void*)0))).Times(2);
    EXPECT_CALL(mockfs.mockVtableOne, finiScreen(Ne((void*)0))).Times(1);
    EXPECT_CALL(mockfs.mockVtableOne, fini()).Times(1);
    EXPECT_CALL(mockfs.mockVtableTwo, finiScreen(Ne((void*)0))).Times(1);
    EXPECT_CALL(mockfs.mockVtableTwo, fini()).Times(1);

    for (CompPlugin* p; (p = CompPlugin::pop ()) != 0; CompPlugin::unload (p));
}

TEST(privatescreen_PluginManagerTest, updating_when_failing_to_load_plugin_in_middle_of_list)
{
    using namespace testing;

    MockCompScreen comp_screen;

    cps::PluginManager ps;
    StubActivePluginsOption sapo;

    CompOption::Value::Vector values;
    values.push_back ("core");
    ps.setPlugins (values);
    ps.setDirtyPluginList ();

    std::list <CompString> plugins;
    plugins.push_back ("one");
    plugins.push_back ("three");
    plugins.push_back ("four");
    initialPlugins = plugins;

    MockPluginFilesystem mockfs;

    EXPECT_CALL(mockfs, LoadPlugin(Ne((void*)0), EndsWith(HOME_PLUGINDIR), StrEq("one"))).
	WillOnce(Invoke(&mockfs, &MockPluginFilesystem::DummyLoader));
    EXPECT_CALL(mockfs.mockVtableOne, init()).WillOnce(Return(true));

    EXPECT_CALL(mockfs, LoadPlugin(Ne((void*)0), EndsWith(HOME_PLUGINDIR), StrEq("three"))).
	WillOnce(Invoke(&mockfs, &MockPluginFilesystem::DummyLoader));
    EXPECT_CALL(mockfs.mockVtableThree, init()).WillOnce(Return(false));

    EXPECT_CALL(mockfs, LoadPlugin(Ne((void*)0), EndsWith(HOME_PLUGINDIR), StrEq("four"))).
	WillOnce(Invoke(&mockfs, &MockPluginFilesystem::DummyLoader));
    EXPECT_CALL(mockfs.mockVtableFour, init()).Times(1).WillRepeatedly(Return(true));

    EXPECT_CALL(mockfs, UnloadPlugin(_)).Times(1);  // Once for "three" which doesn't load

    EXPECT_CALL(comp_screen, _setOptionForPlugin(StrEq("core"), StrEq("active_plugins"), _)).
	    WillOnce(Return(true));

    ps.updatePlugins(&comp_screen, sapo.optionGetActivePlugins());

    Mock::VerifyAndClearExpectations(&mockfs);
    Mock::VerifyAndClearExpectations(&mockfs.mockVtableOne);
    Mock::VerifyAndClearExpectations(&mockfs.mockVtableTwo);
    Mock::VerifyAndClearExpectations(&mockfs.mockVtableThree);
    Mock::VerifyAndClearExpectations(&mockfs.mockVtableFour);

    EXPECT_CALL(comp_screen, _setOptionForPlugin(StrEq("core"), StrEq("active_plugins"), _)).
	    WillOnce(Invoke(&sapo, &StubActivePluginsOption::setActivePlugins));

    ps.updatePlugins(&comp_screen, sapo.optionGetActivePlugins());

    Mock::VerifyAndClearExpectations(&mockfs);
    Mock::VerifyAndClearExpectations(&mockfs.mockVtableOne);
    Mock::VerifyAndClearExpectations(&mockfs.mockVtableTwo);
    Mock::VerifyAndClearExpectations(&mockfs.mockVtableThree);
    Mock::VerifyAndClearExpectations(&mockfs.mockVtableFour);

    // TODO Some cleanup that probably ought to be automatic.
    EXPECT_CALL(mockfs, UnloadPlugin(_)).Times(2);
    EXPECT_CALL(comp_screen, _finiPluginForScreen(Ne((void*)0))).Times(2);
    EXPECT_CALL(mockfs.mockVtableOne, finiScreen(Ne((void*)0))).Times(1);
    EXPECT_CALL(mockfs.mockVtableOne, fini()).Times(1);
    EXPECT_CALL(mockfs.mockVtableFour, finiScreen(Ne((void*)0))).Times(1);
    EXPECT_CALL(mockfs.mockVtableFour, fini()).Times(1);
    for (CompPlugin* p; (p = CompPlugin::pop ()) != 0; CompPlugin::unload (p));
}

TEST(privatescreen_PluginManagerTest, calling_updatePlugins_with_fewer_plugins)
{
    using namespace testing;

    MockCompScreen comp_screen;

    cps::PluginManager ps;

    StubActivePluginsOption sapo;

    // Stuff that has to be done before calling updatePlugins()
    initialPlugins = std::list <CompString>();
    CompOption::Value::Vector values;
    values.push_back ("core");
    ps.setPlugins (values);
    ps.setDirtyPluginList ();

    {
	CompOption::Value::Vector plugins;
	plugins.push_back ("one");
	plugins.push_back ("two");
	plugins.push_back ("three");
	CompOption::Value v(plugins);
	sapo.setActivePlugins("core", "active_plugins", v);
    }

    MockPluginFilesystem mockfs;

    EXPECT_CALL(mockfs, LoadPlugin(Ne((void*)0), EndsWith(HOME_PLUGINDIR), StrEq("one"))).
	WillOnce(Invoke(&mockfs, &MockPluginFilesystem::DummyLoader));
    EXPECT_CALL(mockfs.mockVtableOne, init()).WillOnce(Return(true));

    EXPECT_CALL(mockfs, LoadPlugin(Ne((void*)0), EndsWith(HOME_PLUGINDIR), StrEq("two"))).
	WillOnce(Invoke(&mockfs, &MockPluginFilesystem::DummyLoader));
    EXPECT_CALL(mockfs.mockVtableTwo, init()).WillOnce(Return(true));

    EXPECT_CALL(mockfs, LoadPlugin(Ne((void*)0), EndsWith(HOME_PLUGINDIR), StrEq("three"))).
	WillOnce(Invoke(&mockfs, &MockPluginFilesystem::DummyLoader));
    EXPECT_CALL(mockfs.mockVtableThree, init()).WillOnce(Return(true));

    EXPECT_CALL(comp_screen, _setOptionForPlugin(StrEq("core"), StrEq("active_plugins"), _)).
	    WillOnce(Invoke(&sapo, &StubActivePluginsOption::setActivePlugins));

    ps.updatePlugins(&comp_screen, sapo.optionGetActivePlugins());

    Mock::VerifyAndClearExpectations(&mockfs);
    Mock::VerifyAndClearExpectations(&mockfs.mockVtableOne);
    Mock::VerifyAndClearExpectations(&mockfs.mockVtableTwo);
    Mock::VerifyAndClearExpectations(&mockfs.mockVtableThree);
    Mock::VerifyAndClearExpectations(&mockfs.mockVtableFour);

    EXPECT_CALL(comp_screen, _finiPluginForScreen(Ne((void*)0))).Times(2);
    EXPECT_CALL(mockfs, UnloadPlugin(_)).Times(1);
    EXPECT_CALL(mockfs.mockVtableTwo, finiScreen(Ne((void*)0))).Times(1);
    EXPECT_CALL(mockfs.mockVtableTwo, fini()).Times(1);
    EXPECT_CALL(mockfs.mockVtableThree, fini()).Times(1);
    EXPECT_CALL(mockfs.mockVtableThree, finiScreen(Ne((void*)0))).Times(1);
    EXPECT_CALL(mockfs.mockVtableThree, init()).WillOnce(Return(true));
    EXPECT_CALL(comp_screen, _setOptionForPlugin(StrEq("core"), StrEq("active_plugins"), _)).
	    WillOnce(Invoke(&sapo, &StubActivePluginsOption::setActivePlugins));

    {
	CompOption::Value::Vector plugins;
	plugins.push_back ("one");
	plugins.push_back ("three");
	CompOption::Value v(plugins);
	sapo.setActivePlugins("core", "active_plugins", v);
    }

    ps.updatePlugins(&comp_screen, sapo.optionGetActivePlugins());

    Mock::VerifyAndClearExpectations(&mockfs);
    Mock::VerifyAndClearExpectations(&mockfs.mockVtableOne);
    Mock::VerifyAndClearExpectations(&mockfs.mockVtableTwo);
    Mock::VerifyAndClearExpectations(&mockfs.mockVtableThree);
    Mock::VerifyAndClearExpectations(&mockfs.mockVtableFour);

    // TODO Some cleanup that probably ought to be automatic.
    EXPECT_CALL(mockfs, UnloadPlugin(_)).Times(2);
    EXPECT_CALL(comp_screen, _finiPluginForScreen(Ne((void*)0))).Times(2);
    EXPECT_CALL(mockfs.mockVtableOne, finiScreen(Ne((void*)0))).Times(1);
    EXPECT_CALL(mockfs.mockVtableOne, fini()).Times(1);
    EXPECT_CALL(mockfs.mockVtableThree, finiScreen(Ne((void*)0))).Times(1);
    EXPECT_CALL(mockfs.mockVtableThree, fini()).Times(1);

    for (CompPlugin* p; (p = CompPlugin::pop ()) != 0; CompPlugin::unload (p));
}

// Verify plugin ordering, and verify that plugins not in availablePlugins
// don't get dropped. Because availablePlugins is NOT a definitive list
// of what the dynamic loader might be able to find in its path.
TEST(privatescreen_PluginManagerTest, verify_plugin_ordering)
{
    using namespace testing;

    cps::PluginManager ps;

    initialPlugins.clear();
    initialPlugins.push_back("alice");
    initialPlugins.push_back("bob");
    initialPlugins.push_back("charlie");

    CompOption::Value::Vector extra;
    extra.push_back("charlie");
    extra.push_back("david");
    extra.push_back("alice");
    extra.push_back("eric");
    
    CompOption::Value::Vector merged = ps.mergedPluginList(extra);

    ASSERT_EQ(merged.size(), 6);
    ASSERT_EQ(merged[0].s(), "core");
    ASSERT_EQ(merged[1].s(), "alice");
    ASSERT_EQ(merged[2].s(), "bob");
    ASSERT_EQ(merged[3].s(), "charlie");
    ASSERT_EQ(merged[4].s(), "david");
    ASSERT_EQ(merged[5].s(), "eric");
}

TEST(privatescreen_PluginManagerTest, calling_updatePlugins_with_additional_plugins)
{
    using namespace testing;

    MockCompScreen comp_screen;

    cps::PluginManager ps;

    StubActivePluginsOption sapo;

    // Stuff that has to be done before calling updatePlugins()
    initialPlugins = std::list <CompString>();
    CompOption::Value::Vector values;
    values.push_back ("core");
    ps.setPlugins (values);
    ps.setDirtyPluginList ();

    {
	CompOption::Value::Vector plugins;
	plugins.push_back ("one");
	plugins.push_back ("two");
	plugins.push_back ("four");
	CompOption::Value v(plugins);
	sapo.setActivePlugins("core", "active_plugins", v);
    }

    MockPluginFilesystem mockfs;

    EXPECT_CALL(mockfs, LoadPlugin(Ne((void*)0), EndsWith(HOME_PLUGINDIR), StrEq("one"))).
	WillOnce(Invoke(&mockfs, &MockPluginFilesystem::DummyLoader));
    EXPECT_CALL(mockfs.mockVtableOne, init()).WillOnce(Return(true));

    EXPECT_CALL(mockfs, LoadPlugin(Ne((void*)0), EndsWith(HOME_PLUGINDIR), StrEq("two"))).
	WillOnce(Invoke(&mockfs, &MockPluginFilesystem::DummyLoader));
    EXPECT_CALL(mockfs.mockVtableTwo, init()).WillOnce(Return(true));

    EXPECT_CALL(mockfs, LoadPlugin(Ne((void*)0), EndsWith(HOME_PLUGINDIR), StrEq("four"))).
	WillOnce(Invoke(&mockfs, &MockPluginFilesystem::DummyLoader));
    EXPECT_CALL(mockfs.mockVtableFour, init()).WillOnce(Return(true));

    EXPECT_CALL(comp_screen, _setOptionForPlugin(StrEq("core"), StrEq("active_plugins"), _)).
	    WillOnce(Invoke(&sapo, &StubActivePluginsOption::setActivePlugins));

    ps.updatePlugins(&comp_screen, sapo.optionGetActivePlugins());

    Mock::VerifyAndClearExpectations(&mockfs);
    Mock::VerifyAndClearExpectations(&mockfs.mockVtableOne);
    Mock::VerifyAndClearExpectations(&mockfs.mockVtableTwo);
    Mock::VerifyAndClearExpectations(&mockfs.mockVtableThree);
    Mock::VerifyAndClearExpectations(&mockfs.mockVtableFour);

    EXPECT_CALL(comp_screen, _finiPluginForScreen(Ne((void*)0))).Times(1);
    EXPECT_CALL(mockfs.mockVtableFour, fini()).Times(1);
    EXPECT_CALL(mockfs.mockVtableFour, finiScreen(Ne((void*)0))).Times(1);
    EXPECT_CALL(mockfs, LoadPlugin(Ne((void*)0), EndsWith(HOME_PLUGINDIR), StrEq("three"))).
	WillOnce(Invoke(&mockfs, &MockPluginFilesystem::DummyLoader));
    EXPECT_CALL(mockfs.mockVtableThree, init()).WillOnce(Return(true));
    EXPECT_CALL(mockfs.mockVtableFour, init()).WillOnce(Return(true));
    EXPECT_CALL(comp_screen, _setOptionForPlugin(StrEq("core"), StrEq("active_plugins"), _)).
	    WillOnce(Invoke(&sapo, &StubActivePluginsOption::setActivePlugins));

    {
	CompOption::Value::Vector plugins;
	plugins.push_back ("one");
	plugins.push_back ("two");
	plugins.push_back ("three");
	plugins.push_back ("four");
	CompOption::Value v(plugins);
	sapo.setActivePlugins("core", "active_plugins", v);
    }

    ps.updatePlugins(&comp_screen, sapo.optionGetActivePlugins());

    Mock::VerifyAndClearExpectations(&mockfs);
    Mock::VerifyAndClearExpectations(&mockfs.mockVtableOne);
    Mock::VerifyAndClearExpectations(&mockfs.mockVtableTwo);
    Mock::VerifyAndClearExpectations(&mockfs.mockVtableThree);
    Mock::VerifyAndClearExpectations(&mockfs.mockVtableFour);

    // TODO Some cleanup that probably ought to be automatic.
    EXPECT_CALL(mockfs, UnloadPlugin(_)).Times(4);
    EXPECT_CALL(comp_screen, _finiPluginForScreen(Ne((void*)0))).Times(4);
    EXPECT_CALL(mockfs.mockVtableFour, finiScreen(Ne((void*)0))).Times(1);
    EXPECT_CALL(mockfs.mockVtableFour, fini()).Times(1);
    EXPECT_CALL(mockfs.mockVtableThree, finiScreen(Ne((void*)0))).Times(1);
    EXPECT_CALL(mockfs.mockVtableThree, fini()).Times(1);
    EXPECT_CALL(mockfs.mockVtableTwo, finiScreen(Ne((void*)0))).Times(1);
    EXPECT_CALL(mockfs.mockVtableTwo, fini()).Times(1);
    EXPECT_CALL(mockfs.mockVtableOne, finiScreen(Ne((void*)0))).Times(1);
    EXPECT_CALL(mockfs.mockVtableOne, fini()).Times(1);

    for (CompPlugin* p; (p = CompPlugin::pop ()) != 0; CompPlugin::unload (p));
}

TEST(privatescreen_EventManagerTest, create_and_destroy)
{
    using namespace testing;

    MockCompScreen comp_screen;

    cps::EventManager em;
}

TEST(privatescreen_EventManagerTest, init)
{
    using namespace testing;

    MockCompScreen comp_screen;

    CompOption::Value::Vector values;
    values.push_back ("core");
    initialPlugins = std::list <CompString>();

    EXPECT_CALL(comp_screen, addAction(_)).WillRepeatedly(Return(false));
    EXPECT_CALL(comp_screen, removeAction(_)).WillRepeatedly(Return());
    EXPECT_CALL(comp_screen, _matchInitExp(StrEq("any"))).WillRepeatedly(Return((CompMatch::Expression*)0));

    // The PrivateScreen ctor indirectly calls screen->dpy().
    // We should kill this dependency
    EXPECT_CALL(comp_screen, dpy()).WillRepeatedly(Return((Display*)(0)));

    cps::EventManager em;

    CoreOptions coreOptions(false);
    em.init();
}

TEST(privatescreen_ViewportGeometryTest, PickCurrent)
{
    CompPoint vp;
    compiz::window::Geometry g (250, 250, 500, 500, 0);
    MockViewportRetreival mvp;

    CompPoint current (0, 0);
    CompSize  dimentions (1, 1);

    EXPECT_CALL (mvp, getCurrentViewport ()).WillOnce (ReturnRef (current));
    EXPECT_CALL (mvp, viewportDimentions ()).WillOnce (ReturnRef (dimentions));

    compiz::private_screen::viewports::viewportForGeometry (g, vp, &mvp, CompSize (1000, 1000));

    EXPECT_EQ (vp, CompPoint (0, 0));
}

TEST(privatescreen_ViewportGeometryTest, PickRight)
{
    CompPoint vp;
    compiz::window::Geometry g (1250, 0, 500, 500, 0);
    MockViewportRetreival mvp;

    CompPoint current (0, 0);
    CompSize  dimentions (2, 1);

    EXPECT_CALL (mvp, getCurrentViewport ()).WillOnce (ReturnRef (current));
    EXPECT_CALL (mvp, viewportDimentions ()).WillOnce (ReturnRef (dimentions));

    compiz::private_screen::viewports::viewportForGeometry (g, vp, &mvp, CompSize (1000, 1000));

    EXPECT_EQ (vp, CompPoint (1, 0));
}

TEST(privatescreen_ViewportGeometryTest, PickLeft)
{
    CompPoint vp;
    compiz::window::Geometry g (-750, 0, 500, 500, 0);
    MockViewportRetreival mvp;

    CompPoint current (1, 0);
    CompSize  dimentions (2, 1);

    EXPECT_CALL (mvp, getCurrentViewport ()).WillOnce (ReturnRef (current));
    EXPECT_CALL (mvp, viewportDimentions ()).WillOnce (ReturnRef (dimentions));

    compiz::private_screen::viewports::viewportForGeometry (g, vp, &mvp, CompSize (1000, 1000));

    EXPECT_EQ (vp, CompPoint (0, 0));
}

TEST(privatescreen_ViewportGeometryTest, PickBottom)
{
    CompPoint vp;
    compiz::window::Geometry g (0, 1250, 500, 500, 0);
    MockViewportRetreival mvp;

    CompPoint current (0, 0);
    CompSize  dimentions (1, 2);

    EXPECT_CALL (mvp, getCurrentViewport ()).WillOnce (ReturnRef (current));
    EXPECT_CALL (mvp, viewportDimentions ()).WillOnce (ReturnRef (dimentions));

    compiz::private_screen::viewports::viewportForGeometry (g, vp, &mvp, CompSize (1000, 1000));

    EXPECT_EQ (vp, CompPoint (0, 1));
}

TEST(privatescreen_ViewportGeometryTest, PickTop)
{
    CompPoint vp;
    compiz::window::Geometry g (0, -750, 500, 500, 0);
    MockViewportRetreival mvp;

    CompPoint current (0, 1);
    CompSize  dimentions (1, 2);

    EXPECT_CALL (mvp, getCurrentViewport ()).WillOnce (ReturnRef (current));
    EXPECT_CALL (mvp, viewportDimentions ()).WillOnce (ReturnRef (dimentions));

    compiz::private_screen::viewports::viewportForGeometry (g, vp, &mvp, CompSize (1000, 1000));

    EXPECT_EQ (vp, CompPoint (0, 0));
}

TEST(privatescreen_ViewportGeometryTest, PickTopWhenJustAbove)
{
    CompPoint vp;
    compiz::window::Geometry g (0, -251, 500, 500, 0);
    MockViewportRetreival mvp;

    CompPoint current (0, 1);
    CompSize  dimentions (1, 2);

    EXPECT_CALL (mvp, getCurrentViewport ()).WillOnce (ReturnRef (current));
    EXPECT_CALL (mvp, viewportDimentions ()).WillOnce (ReturnRef (dimentions));

    compiz::private_screen::viewports::viewportForGeometry (g, vp, &mvp, CompSize (1000, 1000));

    EXPECT_EQ (vp, CompPoint (0, 0));
}

TEST(privatescreen_ViewportGeometryTest, PickRightWhenJustRight)
{
    CompPoint vp;
    compiz::window::Geometry g (751, 0, 500, 500, 0);
    MockViewportRetreival mvp;

    CompPoint current (0, 0);
    CompSize  dimentions (2, 1);

    EXPECT_CALL (mvp, getCurrentViewport ()).WillOnce (ReturnRef (current));
    EXPECT_CALL (mvp, viewportDimentions ()).WillOnce (ReturnRef (dimentions));

    compiz::private_screen::viewports::viewportForGeometry (g, vp, &mvp, CompSize (1000, 1000));

    EXPECT_EQ (vp, CompPoint (1, 0));
}

TEST(privatescreen_ViewportGeometryTest, PickLeftWhenJustLeft)
{
    CompPoint vp;
    compiz::window::Geometry g (-251, 0, 500, 500, 0);
    MockViewportRetreival mvp;

    CompPoint current (1, 0);
    CompSize  dimentions (2, 1);

    EXPECT_CALL (mvp, getCurrentViewport ()).WillOnce (ReturnRef (current));
    EXPECT_CALL (mvp, viewportDimentions ()).WillOnce (ReturnRef (dimentions));

    compiz::private_screen::viewports::viewportForGeometry (g, vp, &mvp, CompSize (1000, 1000));

    EXPECT_EQ (vp, CompPoint (0, 0));
}

TEST(privatescreen_ViewportGeometryTest, PickBottomWhenJustBelow)
{
    CompPoint vp;
    compiz::window::Geometry g (0, 751, 500, 500, 0);
    MockViewportRetreival mvp;

    CompPoint current (0, 0);
    CompSize  dimentions (1, 2);

    EXPECT_CALL (mvp, getCurrentViewport ()).WillOnce (ReturnRef (current));
    EXPECT_CALL (mvp, viewportDimentions ()).WillOnce (ReturnRef (dimentions));

    compiz::private_screen::viewports::viewportForGeometry (g, vp, &mvp, CompSize (1000, 1000));

    EXPECT_EQ (vp, CompPoint (0, 1));
}
