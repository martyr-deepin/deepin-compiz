#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <ccs.h>

CCSContext * ccsMockContextNew ();
void ccsFreeMockContext (CCSContext *);

class CCSContextGMockInterface
{
    public:

	virtual ~CCSContextGMockInterface () {};

	virtual CCSPluginList getPlugins () = 0;
	virtual CCSPluginCategory * getCategories () = 0;
	virtual CCSSettingList getChangedSettings () = 0;
	virtual unsigned int getScreenNum () = 0;
	virtual Bool addChangedSetting (CCSSetting *) = 0;
	virtual Bool clearChangedSettings () = 0;
	virtual CCSSettingList stealChangedSettings () = 0;
	virtual void * getPrivatePtr () = 0;
	virtual void setPrivatePtr (void *) = 0;
	virtual Bool loadPlugin (char *name) = 0;
	virtual CCSPlugin * findPlugin (const char *name) = 0;
	virtual Bool pluginIsActive (const char *name) = 0;
	virtual CCSPluginList getActivePluginList () = 0;
	virtual CCSStringList getSortedPluginStringList () = 0;
	virtual const char * getBackend () = 0;
	virtual Bool setBackend (char *name) = 0;
	virtual void setIntegrationEnabled (Bool value) = 0;
	virtual void setProfile (char *name) = 0;
	virtual void setPluginListAutoSort (Bool value) = 0;
	virtual const char * getProfile () = 0;
	virtual Bool getIntegrationEnabled () = 0;
	virtual Bool getPluginListAutoSort () = 0;
	virtual void processEvents (unsigned int flags) = 0;
	virtual void readSettings () = 0;
	virtual void writeSettings () = 0;
	virtual void writeChangedSettings () = 0;
	virtual Bool exportToFile (const char *fileName, Bool skipDefaults) = 0;
	virtual Bool importFromFile (const char *fileName, Bool overwriteNonDefaults) = 0;
	virtual CCSPluginConflictList canEnablePlugin (CCSPlugin *) = 0;
	virtual CCSPluginConflictList canDisablePlugin (CCSPlugin *) = 0;
	virtual void deleteProfile (char *name) = 0;
	virtual CCSStringList getExistingProfiles () = 0;
	virtual Bool checkForSettingsUpgrade () = 0;
	virtual void loadPlugins () = 0;
};

class CCSContextGMock :
    public CCSContextGMockInterface
{
    public:

	CCSContextGMock (CCSContext *c) :
	    mContext (c)
	{
	}

	CCSContext * context () { return mContext; }

	MOCK_METHOD0 (getPlugins, CCSPluginList ());
	MOCK_METHOD0 (getCategories, CCSPluginCategory * ());
	MOCK_METHOD0 (getChangedSettings, CCSSettingList ());
	MOCK_METHOD0 (getScreenNum, unsigned int ());
	MOCK_METHOD1 (addChangedSetting, Bool (CCSSetting *));
	MOCK_METHOD0 (clearChangedSettings, Bool ());
	MOCK_METHOD0 (stealChangedSettings, CCSSettingList ());
	MOCK_METHOD0 (getPrivatePtr, void * ());
	MOCK_METHOD1 (setPrivatePtr, void (void *));
	MOCK_METHOD1 (loadPlugin, Bool (char *));
	MOCK_METHOD1 (findPlugin, CCSPlugin * (const char *));
	MOCK_METHOD1 (pluginIsActive, Bool (const char *));
	MOCK_METHOD0 (getActivePluginList, CCSPluginList ());
	MOCK_METHOD0 (getSortedPluginStringList, CCSStringList ());
	MOCK_METHOD0 (getBackend, const char * ());
	MOCK_METHOD1 (setBackend, Bool (char *));
	MOCK_METHOD1 (setIntegrationEnabled, void (Bool));
	MOCK_METHOD1 (setProfile, void (char *));
	MOCK_METHOD1 (setPluginListAutoSort, void (Bool));
	MOCK_METHOD0 (getProfile, const char * ());
	MOCK_METHOD0 (getIntegrationEnabled, Bool ());
	MOCK_METHOD0 (getPluginListAutoSort, Bool ());
	MOCK_METHOD1 (processEvents, void (unsigned int));
	MOCK_METHOD0 (readSettings, void ());
	MOCK_METHOD0 (writeSettings, void ());
	MOCK_METHOD0 (writeChangedSettings, void ());
	MOCK_METHOD2 (exportToFile, Bool (const char *, Bool));
	MOCK_METHOD2 (importFromFile, Bool (const char *, Bool));
	MOCK_METHOD1 (canEnablePlugin, CCSPluginConflictList (CCSPlugin *));
	MOCK_METHOD1 (canDisablePlugin, CCSPluginConflictList (CCSPlugin *));
	MOCK_METHOD1 (deleteProfile, void (char *));
	MOCK_METHOD0 (getExistingProfiles, CCSStringList ());
	MOCK_METHOD0 (checkForSettingsUpgrade, Bool ());
	MOCK_METHOD0 (loadPlugins, void ());

    private:

	CCSContext *mContext;

    public:

	/* Thunking from C interface callbacks to the virtual functions ... */
	static CCSPluginList
	ccsContextGetPlugins (CCSContext *context)
	{
	    return ((CCSContextGMock *) ccsObjectGetPrivate (context))->getPlugins ();
	}

	static CCSPluginCategory *
	ccsContextGetCategories (CCSContext *context)
	{
	    return ((CCSContextGMock *) ccsObjectGetPrivate (context))->getCategories ();
	}

	static CCSSettingList
	ccsContextGetChangedSettings (CCSContext *context)
	{
	    return ((CCSContextGMock *) ccsObjectGetPrivate (context))->getChangedSettings ();
	}

	static unsigned int
	ccsContextGetScreenNum (CCSContext *context)
	{
	    return ((CCSContextGMock *) ccsObjectGetPrivate (context))->getScreenNum ();
	}

	static Bool
	ccsContextAddChangedSetting (CCSContext *context, CCSSetting *setting)
	{
	    return ((CCSContextGMock *) ccsObjectGetPrivate (context))->addChangedSetting (setting);
	}

	static Bool
	ccsContextClearChangedSettings (CCSContext *context)
	{
	    return ((CCSContextGMock *) ccsObjectGetPrivate (context))->clearChangedSettings ();
	}

	static CCSSettingList
	ccsContextStealChangedSettings (CCSContext *context)
	{
	    return ((CCSContextGMock *) ccsObjectGetPrivate (context))->stealChangedSettings ();
	}

	static void *
	ccsContextGetPrivatePtr (CCSContext *context)
	{
	    return ((CCSContextGMock *) ccsObjectGetPrivate (context))->getPrivatePtr ();
	}

	static void
	ccsContextSetPrivatePtr (CCSContext *context, void *ptr)
	{
	    ((CCSContextGMock *) ccsObjectGetPrivate (context))->setPrivatePtr (ptr);
	}

	static CCSPlugin *
	ccsFindPlugin (CCSContext *context, const char *name)
	{
	    return ((CCSContextGMock *) ccsObjectGetPrivate (context))->findPlugin ((char *) name);
	}

	static Bool
	ccsPluginIsActive (CCSContext *context, const char *name)
	{
	    return ((CCSContextGMock *) ccsObjectGetPrivate (context))->pluginIsActive (name);
	}

	static Bool
	ccsSetBackend (CCSContext *context, char *name)
	{
	    return ((CCSContextGMock *) ccsObjectGetPrivate (context))->setBackend (name);
	}

	static CCSPluginList
	ccsGetActivePluginList (CCSContext *context)
	{
	    return ((CCSContextGMock *) ccsObjectGetPrivate (context))->getActivePluginList ();
	}

	static CCSStringList
	ccsGetSortedPluginStringList (CCSContext *context)
	{
	    return ((CCSContextGMock *) ccsObjectGetPrivate (context))->getSortedPluginStringList ();
	}

	static const char *
	ccsGetBackend (CCSContext *context)
	{
	    return ((CCSContextGMock *) ccsObjectGetPrivate (context))->getBackend ();
	}

	static Bool
	ccsGetIntegrationEnabled (CCSContext *context)
	{
	    if (!context)
		return FALSE;

	    return ((CCSContextGMock *) ccsObjectGetPrivate (context))->getIntegrationEnabled ();
	}

	static const char *
	ccsGetProfile (CCSContext *context)
	{
	    if (!context)
		return NULL;

	    return ((CCSContextGMock *) ccsObjectGetPrivate (context))->getProfile ();
	}

	static Bool
	ccsGetPluginListAutoSort (CCSContext *context)
	{
	    if (!context)
		return FALSE;

	    return ((CCSContextGMock *) ccsObjectGetPrivate (context))->getPluginListAutoSort ();
	}

	static void
	ccsSetIntegrationEnabled (CCSContext *context, Bool value)
	{
	    ((CCSContextGMock *) ccsObjectGetPrivate (context))->setIntegrationEnabled (value);
	}

	static void
	ccsSetPluginListAutoSort (CCSContext *context, Bool value)
	{
	    ((CCSContextGMock *) ccsObjectGetPrivate (context))->setPluginListAutoSort (value);
	}

	static void
	ccsSetProfile (CCSContext *context, char *name)
	{
	    ((CCSContextGMock *) ccsObjectGetPrivate (context))->setProfile (name);
	}

	static void
	ccsProcessEvents (CCSContext *context, unsigned int flags)
	{
	    if (!context)
		return;

	    ((CCSContextGMock *) ccsObjectGetPrivate (context))->processEvents (flags);
	}

	static void
	ccsReadSettings (CCSContext *context)
	{
	    if (!context)
		return;

	    ((CCSContextGMock *) ccsObjectGetPrivate (context))->readSettings ();
	}

	static void
	ccsWriteSettings (CCSContext *context)
	{
	    if (!context)
		return;

	    ((CCSContextGMock *) ccsObjectGetPrivate (context))->writeSettings ();
	}

	static void
	ccsWriteChangedSettings (CCSContext *context)
	{
	    if (!context)
		return;

	    ((CCSContextGMock *) ccsObjectGetPrivate (context))->writeChangedSettings ();
	}

	static CCSPluginConflictList
	ccsCanEnablePlugin (CCSContext *context, CCSPlugin *plugin)
	{
	    return ((CCSContextGMock *) ccsObjectGetPrivate (context))->canEnablePlugin (plugin);
	}

	static CCSPluginConflictList
	ccsCanDisablePlugin (CCSContext *context, CCSPlugin *plugin)
	{
	    return ((CCSContextGMock *) ccsObjectGetPrivate (context))->canDisablePlugin (plugin);
	}

	static CCSStringList
	ccsGetExistingProfiles (CCSContext *context)
	{
	    if (!context)
		return NULL;

	    return ((CCSContextGMock *) ccsObjectGetPrivate (context))->getExistingProfiles ();
	}

	static void
	ccsDeleteProfile (CCSContext *context, char *name)
	{
	    if (!context)
		return;

	    ((CCSContextGMock *) ccsObjectGetPrivate (context))->deleteProfile (name);
	}

	static Bool
	ccsCheckForSettingsUpgrade (CCSContext *context)
	{
	    return ((CCSContextGMock *) ccsObjectGetPrivate (context))->checkForSettingsUpgrade ();
	}

	static Bool
	ccsImportFromFile (CCSContext *context, const char *fileName, Bool overwriteNonDefault)
	{
	    if (!context)
		return FALSE;

	    return ((CCSContextGMock *) ccsObjectGetPrivate (context))->importFromFile (fileName, overwriteNonDefault);
	}

	static Bool
	ccsLoadPlugin (CCSContext *context, char *name)
	{
	    if (!context)
		return FALSE;

	    return ((CCSContextGMock *) ccsObjectGetPrivate (context))->loadPlugin (name);
	}

	static Bool
	ccsExportToFile (CCSContext *context, const char *fileName, Bool skipDefaults)
	{
	    if (!context)
		return FALSE;

	    return ((CCSContextGMock *) ccsObjectGetPrivate (context))->exportToFile (fileName, skipDefaults);
	}

	static void
	ccsLoadPlugins (CCSContext *context)
	{
	    if (!context)
		return;

	    return ((CCSContextGMock *) ccsObjectGetPrivate (context))->loadPlugins ();
	}

	static void
	ccsFreeContext (CCSContext *context)
	{
	    if (!context)
		return;

	    ccsFreeMockContext (context);
	}
};

extern CCSContextInterface CCSContextGMockInterface;
