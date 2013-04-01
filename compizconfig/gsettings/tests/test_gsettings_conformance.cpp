/*
 * Compiz configuration system library
 *
 * Copyright (C) 2012 Canonical Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Authored By:
 * Sam Spilsbury <sam.spilsbury@canonical.com>
 */
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <gio/gio.h>

#include "backend-conformance-config.h"
#include "gsettings-mock-schemas-config.h"

#include <ccs.h>
#include <compizconfig_backend_concept_test.h>

#include <gsettings_util.h>
#include <ccs_gsettings_backend_interface.h>

#include "gtest_shared_autodestroy.h"
#include "test_gsettings_tests.h"

#include "compizconfig_ccs_gsettings_settings_env_test.h"
#include "compizconfig_ccs_integration_mock.h"

using ::testing::AtLeast;
using ::testing::Pointee;
using ::testing::ReturnNull;

namespace cci = compiz::config::impl;

class CCSGSettingsBackendEnv :
    public CCSBackendConceptTestEnvironmentInterface,
    public CCSGSettingsMemoryBackendTestingEnv
{
    private:

	virtual void SetUp ()
	{
	}

	virtual void TearDown ()
	{
	}

    public:

	CCSGSettingsBackendEnv () :
	    pluginToMatch ("mock")
	{
	}

	/* A wrapper to prevent signals from being added */
	static void connectToSignalWrapper (CCSBackend *backend, CCSGSettingsWrapper *wrapper)
	{
	};

	const CCSBackendInfo * GetInfo ()
	{
	    return &gsettingsBackendInfo;
	}

	CCSBackend * SetUp (CCSContext *context, CCSContextGMock *gmockContext)
	{
	    CCSGSettingsBackendInterface *overloadedInterface = NULL;

	    SetUpEnv ();
	    g_setenv ("LIBCOMPIZCONFIG_BACKEND_PATH", BACKEND_BINARY_PATH, true);

	    mContext = context;

	    std::string path ("gsettings");

	    mBackend = reinterpret_cast <CCSDynamicBackend *> (ccsOpenBackend (&ccsDefaultInterfaceTable, mContext, path.c_str ()));

	    EXPECT_TRUE (mBackend);

	    mGSettingsBackend = ccsDynamicBackendGetRawBackend (mBackend);


	    CCSBackendInitFunc backendInit = (GET_INTERFACE (CCSBackendInterface, mBackend))->backendInit;

	    if (backendInit)
		(*backendInit) ((CCSBackend *) mBackend, mContext);

	    /* Set the new integration, drop our reference on it */
	    CCSIntegration *integration = ccsMockIntegrationBackendNew (&ccsDefaultObjectAllocator);
	    CCSIntegrationGMock *gmockIntegration = reinterpret_cast <CCSIntegrationGMock *> (ccsObjectGetPrivate (integration));

	    EXPECT_CALL (*gmockIntegration, getIntegratedOptionIndex (_, _)).WillRepeatedly (ReturnNull ());

	    ccsBackendSetIntegration ((CCSBackend *) mBackend, integration);
	    ccsIntegrationUnref (integration);

	    overloadedInterface = GET_INTERFACE (CCSGSettingsBackendInterface, mGSettingsBackend);
	    overloadedInterface->gsettingsBackendConnectToChangedSignal = CCSGSettingsBackendEnv::connectToSignalWrapper;

	    mSettings = ccsGSettingsGetSettingsObjectForPluginWithPath (mGSettingsBackend, "mock",
									CharacterWrapper (makeCompizPluginPath (profileName.c_str (), "mock")),
									mContext);

	    mStorage.reset (new CCSGSettingsStorageEnv (mSettings, profileName));

	    ON_CALL (*gmockContext, getProfile ()).WillByDefault (Return (profileName.c_str ()));

	    return (CCSBackend *) mBackend;
	}

	void TearDown (CCSBackend *)
	{
	    GVariantBuilder noProfilesBuilder;
	    g_variant_builder_init (&noProfilesBuilder, G_VARIANT_TYPE ("as"));
	    g_variant_builder_add (&noProfilesBuilder, "s", "Default");
	    GVariant *noProfiles = g_variant_builder_end (&noProfilesBuilder);

	    GVariantBuilder pluginKeysBuilder;
	    g_variant_builder_init (&pluginKeysBuilder, G_VARIANT_TYPE ("as"));
	    g_variant_builder_add (&pluginKeysBuilder, "s", "mock");
	    GVariant *pluginKeys = g_variant_builder_end (&pluginKeysBuilder);

	    ccsGSettingsBackendUnsetAllChangedPluginKeysInProfile (mGSettingsBackend,
								   mContext,
								   pluginKeys,
								   ccsGSettingsBackendGetCurrentProfile (
								       mGSettingsBackend));
	    ccsGSettingsBackendClearPluginsWithSetKeys (mGSettingsBackend);
	    ccsGSettingsBackendSetExistingProfiles (mGSettingsBackend, noProfiles);
	    ccsGSettingsBackendSetCurrentProfile (mGSettingsBackend, "Default");

	    ccsFreeDynamicBackend (mBackend);

	    mStorage.reset ();

	    g_variant_unref (pluginKeys);

	    TearDownEnv ();
	    g_unsetenv ("LIBCOMPIZCONFIG_BACKEND_PATH");
	}

	void AddProfile (const std::string &profile)
	{
	    ccsGSettingsBackendAddProfile (mGSettingsBackend, profile.c_str ());
	}

	void SetGetExistingProfilesExpectation (CCSContext *context,
						CCSContextGMock *gmockContext)
	{
	    EXPECT_CALL (*gmockContext, getProfile ()).Times (AtLeast (1));
	}

	void SetDeleteProfileExpectation (const std::string &profileToDelete,
					  CCSContext *context,
					  CCSContextGMock *gmockContext)
	{
	    EXPECT_CALL (*gmockContext, getProfile ()).Times (AtLeast (1));
	}

	void SetReadInitExpectation (CCSContext *context,
				     CCSContextGMock *gmockContext)
	{
	    EXPECT_CALL (*gmockContext, getProfile ()).WillOnce (Return (profileName.c_str ()));
	}

	void SetReadDoneExpectation (CCSContext *context,
				     CCSContextGMock *gmockContext)
	{
	}

	void SetWriteInitExpectation (CCSContext *context,
				      CCSContextGMock *gmockContext)
	{
	    EXPECT_CALL (*gmockContext, getProfile ()).WillOnce (Return (profileName.c_str ()));
	}

	void SetWriteDoneExpectation (CCSContext *context,
				      CCSContextGMock *gmockContext)
	{
	}

	void PreWrite (CCSContextGMock *gmockContext,
		       CCSPluginGMock  *gmockPlugin,
		       CCSSettingGMock *gmockSetting,
		       CCSSettingType  type)
	{
	    EXPECT_CALL (*gmockContext, getIntegrationEnabled ()).WillRepeatedly (Return (FALSE));
	    EXPECT_CALL (*gmockPlugin, getContext ()).Times (AtLeast (1));
	    EXPECT_CALL (*gmockPlugin, getName ()).Times (AtLeast (1));
	    EXPECT_CALL (*gmockSetting, getType ()).Times (AtLeast (1));
	    EXPECT_CALL (*gmockSetting, getName ()).Times (AtLeast (1));
	    EXPECT_CALL (*gmockSetting, getParent ()).Times (AtLeast (1));
	    EXPECT_CALL (*gmockSetting, getIsDefault ()).WillRepeatedly (Return (FALSE));

	    if (type == TypeList)
		EXPECT_CALL (*gmockSetting, getDefaultValue ()).WillRepeatedly (ReturnNull ());
	}

	void PostWrite (CCSContextGMock *gmockContext,
			CCSPluginGMock  *gmockPlugin,
			CCSSettingGMock *gmockSetting,
			CCSSettingType  type) {}



	void PreRead (CCSContextGMock *gmockContext,
		      CCSPluginGMock  *gmockPlugin,
		      CCSSettingGMock *gmockSetting,
		      CCSSettingType  type)
	{
	    EXPECT_CALL (*gmockContext, getIntegrationEnabled ()).WillOnce (Return (FALSE));
	    EXPECT_CALL (*gmockPlugin, getContext ()).Times (AtLeast (1));
	    EXPECT_CALL (*gmockPlugin, getName ()).Times (AtLeast (1));
	    EXPECT_CALL (*gmockSetting, getType ()).Times (AtLeast (1));
	    EXPECT_CALL (*gmockSetting, getName ()).Times (AtLeast (1));
	    EXPECT_CALL (*gmockSetting, getParent ()).Times (AtLeast (1));
	    EXPECT_CALL (*gmockSetting, isReadableByBackend ()).WillRepeatedly (Return (TRUE));

	    if (type == TypeList)
	    {
		EXPECT_CALL (*gmockSetting, getInfo ()).Times (AtLeast (1));
		EXPECT_CALL (*gmockSetting, getDefaultValue ()).WillRepeatedly (ReturnNull ());
	    }
	}

	void PostRead (CCSContextGMock *gmockContext,
		       CCSPluginGMock  *gmockPlugin,
		       CCSSettingGMock *gmockSetting,
		       CCSSettingType  type) {}

	void PreUpdate (CCSContextGMock *gmockContext,
		      CCSPluginGMock  *gmockPlugin,
		      CCSSettingGMock *gmockSetting,
		      CCSSettingType  type)
	{
	    EXPECT_CALL (*gmockContext, getIntegrationEnabled ()).WillOnce (Return (FALSE));
	    EXPECT_CALL (*gmockPlugin, getContext ()).Times (AtLeast (1));
	    EXPECT_CALL (*gmockPlugin, getName ()).Times (AtLeast (1));
	    EXPECT_CALL (*gmockSetting, getType ()).Times (AtLeast (1));
	    EXPECT_CALL (*gmockSetting, getName ()).Times (AtLeast (1));
	    EXPECT_CALL (*gmockSetting, getParent ()).Times (AtLeast (1));
	    EXPECT_CALL (*gmockSetting, isReadableByBackend ()).WillRepeatedly (Return (TRUE));

	    if (type == TypeList)
	    {
		EXPECT_CALL (*gmockSetting, getInfo ()).Times (AtLeast (1));
		EXPECT_CALL (*gmockSetting, getDefaultValue ()).WillRepeatedly (ReturnNull ());
	    }

	    EXPECT_CALL (*gmockContext, getProfile ());
	}

	void PostUpdate (CCSContextGMock *gmockContext,
		       CCSPluginGMock  *gmockPlugin,
		       CCSSettingGMock *gmockSetting,
		       CCSSettingType  type) {}

	bool UpdateSettingAtKey (const std::string &plugin,
				 const std::string &setting)
	{
	    CharacterWrapper keyName (translateKeyForGSettings (setting.c_str ()));

	    if (updateSettingWithGSettingsKeyName (reinterpret_cast <CCSBackend *> (mGSettingsBackend), mSettings, keyName, ccsBackendUpdateSetting))
		return true;

	    return false;
	}

	void WriteBoolAtKey (const std::string &plugin,
			     const std::string &key,
			     const VariantTypes &value)
	{
	    mStorage->WriteBoolAtKey (plugin, key, value);
	}

	void WriteIntegerAtKey (const std::string &plugin,
				const std::string &key,
				const VariantTypes &value)
	{
	    mStorage->WriteIntegerAtKey (plugin, key, value);
	}

	void WriteFloatAtKey (const std::string &plugin,
				      const std::string &key,
				      const VariantTypes &value)
	{
	    mStorage->WriteFloatAtKey (plugin, key, value);
	}

	void WriteStringAtKey (const std::string &plugin,
				       const std::string &key,
				       const VariantTypes &value)
	{
	    mStorage->WriteStringAtKey (plugin, key, value);
	}

	void WriteColorAtKey (const std::string &plugin,
				       const std::string &key,
				       const VariantTypes &value)
	{
	    mStorage->WriteColorAtKey (plugin, key, value);
	}

	void WriteKeyAtKey (const std::string &plugin,
				       const std::string &key,
				       const VariantTypes &value)
	{
	    mStorage->WriteKeyAtKey (plugin, key, value);
	}

	void WriteButtonAtKey (const std::string &plugin,
				       const std::string &key,
				       const VariantTypes &value)
	{
	    mStorage->WriteButtonAtKey (plugin, key, value);
	}

	void WriteEdgeAtKey (const std::string &plugin,
				       const std::string &key,
				       const VariantTypes &value)
	{
	    mStorage->WriteEdgeAtKey (plugin, key, value);
	}

	void WriteMatchAtKey (const std::string &plugin,
				      const std::string &key,
				      const VariantTypes &value)
	{
	    mStorage->WriteMatchAtKey (plugin, key, value);
	}

	void WriteBellAtKey (const std::string &plugin,
				       const std::string &key,
				       const VariantTypes &value)
	{
	    mStorage->WriteBellAtKey (plugin, key, value);
	}

	void WriteListAtKey (const std::string &plugin,
				     const std::string &key,
				     const VariantTypes &value)
	{
	    mStorage->WriteListAtKey (plugin, key, value);
	}

	Bool ReadBoolAtKey (const std::string &plugin,
			    const std::string &key)
	{
	    return mStorage->ReadBoolAtKey (plugin, key);
	}

	int ReadIntegerAtKey (const std::string &plugin,
					const std::string &key)
	{
	    return mStorage->ReadIntegerAtKey (plugin, key);
	}

	float ReadFloatAtKey (const std::string &plugin,
				      const std::string &key)
	{
	    return mStorage->ReadFloatAtKey (plugin, key);
	}

	const char * ReadStringAtKey (const std::string &plugin,
				      const std::string &key)
	{
	    return mStorage->ReadStringAtKey (plugin, key);
	}

	CCSSettingColorValue ReadColorAtKey (const std::string &plugin,
				       const std::string &key)
	{
	    return mStorage->ReadColorAtKey (plugin, key);
	}

	CCSSettingKeyValue ReadKeyAtKey (const std::string &plugin,
				       const std::string &key)
	{
	    return mStorage->ReadKeyAtKey (plugin, key);
	}

	CCSSettingButtonValue ReadButtonAtKey (const std::string &plugin,
				       const std::string &key)
	{
	    return mStorage->ReadButtonAtKey (plugin, key);
	}

	unsigned int ReadEdgeAtKey (const std::string &plugin,
				       const std::string &key)
	{
	    return mStorage->ReadEdgeAtKey (plugin, key);
	}

	const char * ReadMatchAtKey (const std::string &plugin,
				     const std::string &key)
	{
	    return mStorage->ReadMatchAtKey (plugin, key);
	}

	Bool ReadBellAtKey (const std::string &plugin,
				       const std::string &key)
	{
	    return mStorage->ReadBellAtKey (plugin, key);
	}

	CCSSettingValueList ReadListAtKey (const std::string &plugin,
					   const std::string &key,
					   CCSSetting        *setting)
	{
	    return mStorage->ReadListAtKey (plugin, key, setting);
	}

    private:

	CCSGSettingsWrapper  *mSettings;
	boost::shared_ptr <CCSGSettingsStorageEnv> mStorage;
	CCSContext *mContext;
	CCSDynamicBackend *mBackend;
	CCSBackend		   *mGSettingsBackend;
	std::string pluginToMatch;

	static const std::string profileName;
};

const std::string CCSGSettingsBackendEnv::profileName = "mock";

INSTANTIATE_TEST_CASE_P (CCSGSettingsBackendConcept, CCSBackendConformanceTestReadWrite,
			 compizconfig::test::GenerateTestingParametersForBackendInterface <CCSGSettingsBackendEnv> ());

INSTANTIATE_TEST_CASE_P (CCSGSettingsBackendConcept, CCSBackendConformanceTestInfo,
			 compizconfig::test::GenerateTestingEnvFactoryBackendInterface <CCSGSettingsBackendEnv> ());

INSTANTIATE_TEST_CASE_P (CCSGSettingsBackendConcept, CCSBackendConformanceTestInitFiniFuncs,
			 compizconfig::test::GenerateTestingEnvFactoryBackendInterface <CCSGSettingsBackendEnv> ());

INSTANTIATE_TEST_CASE_P (CCSGSettingsBackendConcept, CCSBackendConformanceTestProfileHandling,
			 compizconfig::test::GenerateTestingEnvFactoryBackendInterface <CCSGSettingsBackendEnv> ());
