#ifndef _COMPIZCONFIG_CCS_GSETTINGS_BACKEND_MOCK
#define _COMPIZCONFIG_CCS_GSETTINGS_BACKEND_MOCK

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <ccs-backend.h>
#include "ccs_gsettings_backend_interface.h"

using ::testing::_;
using ::testing::Return;

CCSBackend * ccsGSettingsBackendGMockNew ();
void ccsGSettingsBackendGMockFree (CCSBackend *backend);

class CCSGSettingsBackendGMockInterface
{
    public:

	virtual ~CCSGSettingsBackendGMockInterface () {}

	virtual CCSContext * getContext () = 0;
	virtual void connectToChangedSignal (CCSGSettingsWrapper *) = 0;
	virtual CCSGSettingsWrapper * getSettingsObjectForPluginWithPath (const char * plugin,
									  const char * path,
									  CCSContext * context) = 0;
	virtual const char * getCurrentProfile () = 0;
	virtual GVariant * getExistingProfiles () = 0;
	virtual void setExistingProfiles (GVariant *) = 0;
	virtual void setCurrentProfile (const gchar *) = 0;
	virtual GVariant * getPluginsWithSetKeys () = 0;
	virtual void clearPluginsWithSetKeys () = 0;
	virtual void unsetAllChangedPluginKeysInProfile (CCSContext *, GVariant *, const char *) = 0;
	virtual gboolean updateProfile (CCSContext *) = 0;
	virtual void updateCurrentProfileName (const char *) = 0;
	virtual gboolean addProfile (const char *) = 0;
	virtual CCSIntegratedSetting * getIntegratedSetting (CCSSetting *) = 0;
	virtual Bool readIntegratedOption (CCSSetting *, CCSIntegratedSetting *) = 0;
	virtual void writeIntegratedOption (CCSSetting *, CCSIntegratedSetting *) = 0;

};

class CCSGSettingsBackendGMock :
    public CCSGSettingsBackendGMockInterface
{
    public:

	CCSGSettingsBackendGMock (CCSBackend *backend) :
	    mBackend (backend)
	{
	}

	MOCK_METHOD0 (getContext, CCSContext * ());
	MOCK_METHOD1 (connectToChangedSignal, void (CCSGSettingsWrapper *));
	MOCK_METHOD3 (getSettingsObjectForPluginWithPath, CCSGSettingsWrapper * (const char * plugin,
										 const char * path,
										 CCSContext * context));
	MOCK_METHOD0 (getCurrentProfile, const char * ());
	MOCK_METHOD0 (getExistingProfiles, GVariant * ());
	MOCK_METHOD1 (setExistingProfiles, void (GVariant *));
	MOCK_METHOD1 (setCurrentProfile, void (const gchar *));
	MOCK_METHOD0 (getPluginsWithSetKeys, GVariant * ());
	MOCK_METHOD0 (clearPluginsWithSetKeys, void ());
	MOCK_METHOD3 (unsetAllChangedPluginKeysInProfile, void (CCSContext *, GVariant *, const char *));
	MOCK_METHOD1 (updateProfile, gboolean (CCSContext *));
	MOCK_METHOD1 (updateCurrentProfileName, void (const char *));
	MOCK_METHOD1 (addProfile, gboolean (const char *));
	MOCK_METHOD1 (getIntegratedSetting, CCSIntegratedSetting * (CCSSetting *));
	MOCK_METHOD2 (readIntegratedOption, Bool (CCSSetting *, CCSIntegratedSetting *));
	MOCK_METHOD2 (writeIntegratedOption, void (CCSSetting *, CCSIntegratedSetting *));

	CCSBackend * getBackend () { return mBackend; }

    private:

	CCSBackend *mBackend;

    public:

	static CCSContext *
	ccsGSettingsBackendGetContext (CCSBackend *backend)
	{
	    return (reinterpret_cast <CCSGSettingsBackendGMock *> (ccsObjectGetPrivate (backend)))->getContext ();
	}

	static void
	ccsGSettingsBackendConnectToValueChangedSignal (CCSBackend *backend, CCSGSettingsWrapper *object)
	{
	    (reinterpret_cast <CCSGSettingsBackendGMock *> (ccsObjectGetPrivate (backend)))->connectToChangedSignal(object);
	}

	static CCSGSettingsWrapper *
	ccsGSettingsBackendGetSettingsObjectForPluginWithPath (CCSBackend *backend,
							       const char *plugin,
							       const char *path,
							       CCSContext *context)
	{
	    return (reinterpret_cast <CCSGSettingsBackendGMock *> (ccsObjectGetPrivate (backend)))->getSettingsObjectForPluginWithPath (plugin,
																	path,
																	context);
	}

	static const char *
	ccsGSettingsBackendGetCurrentProfile (CCSBackend *backend)
	{
	    return (reinterpret_cast <CCSGSettingsBackendGMock *> (ccsObjectGetPrivate (backend)))->getCurrentProfile ();
	}

	static GVariant *
	ccsGSettingsBackendGetExistingProfiles (CCSBackend *backend)
	{
	    return (reinterpret_cast <CCSGSettingsBackendGMock *> (ccsObjectGetPrivate (backend)))->getExistingProfiles ();
	}

	static void
	ccsGSettingsBackendSetExistingProfiles (CCSBackend *backend, GVariant *value)
	{
	    (reinterpret_cast <CCSGSettingsBackendGMock *> (ccsObjectGetPrivate (backend)))->setExistingProfiles (value);
	}

	static void
	ccsGSettingsBackendSetCurrentProfile (CCSBackend *backend, const gchar *value)
	{
	    (reinterpret_cast <CCSGSettingsBackendGMock *> (ccsObjectGetPrivate (backend)))->setCurrentProfile (value);
	}

	static GVariant *
	ccsGSettingsBackendGetPluginsWithSetKeys (CCSBackend *backend)
	{
	    return (reinterpret_cast <CCSGSettingsBackendGMock *> (ccsObjectGetPrivate (backend)))->getPluginsWithSetKeys ();
	}

	static void
	ccsGSettingsBackendClearPluginsWithSetKeys (CCSBackend *backend)
	{
	    return (reinterpret_cast <CCSGSettingsBackendGMock *> (ccsObjectGetPrivate (backend)))->clearPluginsWithSetKeys ();
	}

	static void
	ccsGSettingsBackendUnsetAllChangedPluginKeysInProfile (CCSBackend *backend,
							       CCSContext *context,
							       GVariant   *pluginKeys,
							       const char *profile)
	{
	    return (reinterpret_cast <CCSGSettingsBackendGMock *> (ccsObjectGetPrivate (backend)))->unsetAllChangedPluginKeysInProfile (context, pluginKeys, profile);
	}

	static gboolean
	ccsGSettingsBackendUpdateProfile (CCSBackend *backend, CCSContext *context)
	{
	    return (reinterpret_cast <CCSGSettingsBackendGMock *> (ccsObjectGetPrivate (backend)))->updateProfile (context);
	}

	static void
	ccsGSettingsBackendUpdateCurrentProfileName (CCSBackend *backend, const char *profile)
	{
	    return (reinterpret_cast <CCSGSettingsBackendGMock *> (ccsObjectGetPrivate (backend)))->updateCurrentProfileName (profile);
	}

	static gboolean
	ccsGSettingsBackendAddProfile (CCSBackend *backend, const char *profile)
	{
	    return (reinterpret_cast <CCSGSettingsBackendGMock *> (ccsObjectGetPrivate (backend)))->addProfile (profile);
	}

	static CCSIntegratedSetting *
	ccsGSettingsBackendGetIntegratedSetting (CCSBackend *backend, CCSSetting *setting)
	{
	    return (reinterpret_cast <CCSGSettingsBackendGMock *> (ccsObjectGetPrivate (backend)))->getIntegratedSetting (setting);
	}

	static Bool
	ccsGSettingsBackendReadIntegratedOption (CCSBackend *backend, CCSSetting *setting, CCSIntegratedSetting *integrated)
	{
	    return (reinterpret_cast <CCSGSettingsBackendGMock *> (ccsObjectGetPrivate (backend)))->readIntegratedOption (setting, integrated);

	}

	static void
	ccsGSettingsBackendWriteIntegratedOption (CCSBackend *backend, CCSSetting *setting, CCSIntegratedSetting *integrated)
	{
	    (reinterpret_cast <CCSGSettingsBackendGMock *> (ccsObjectGetPrivate (backend)))->writeIntegratedOption (setting, integrated);
	}
};

#endif
