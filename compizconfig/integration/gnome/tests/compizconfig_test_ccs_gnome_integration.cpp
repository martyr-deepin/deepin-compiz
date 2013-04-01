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
#include <tr1/tuple>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include <gtest_shared_autodestroy.h>
#include <gtest_unspecified_bool_type_matcher.h>

#include <compizconfig_ccs_list_wrapper.h>
#include <compizconfig_ccs_setting_value_operators.h>

#include <ccs.h>
#include <ccs-backend.h>
#include <ccs_gnome_integrated_setting.h>
#include <ccs_gnome_integration.h>
#include <ccs_gnome_integration_constants.h>
#include <compizconfig_ccs_context_mock.h>
#include <compizconfig_ccs_setting_mock.h>
#include <compizconfig_ccs_plugin_mock.h>
#include <compizconfig_ccs_backend_mock.h>
#include <compizconfig_ccs_integrated_setting_factory_mock.h>
#include <compizconfig_ccs_integrated_setting_storage_mock.h>
#include <compizconfig_ccs_integrated_setting_mock.h>
#include "compizconfig_ccs_mock_gnome_integrated_setting_composition.h"
#include "compizconfig_ccs_setting_value_matcher.h"

namespace cc  = compiz::config;
namespace cci = compiz::config::impl;

using ::testing::Pointee;
using ::testing::Eq;
using ::testing::Return;
using ::testing::ReturnNull;
using ::testing::SetArgPointee;
using ::testing::DoAll;
using ::testing::Values;
using ::testing::WithParamInterface;
using ::testing::_;

namespace
{
    typedef std::tr1::tuple <CCSIntegratedSettingGMock                &,
			     boost::shared_ptr <CCSIntegratedSetting>   > IntegratedSettingWithMock;

    typedef boost::shared_ptr <IntegratedSettingWithMock> IntegratedSettingWithMockPtr;

    IntegratedSettingWithMockPtr
    createIntegratedSettingCompositionFromMock (const std::string            &plugin,
						const std::string            &setting,
						CCSSettingType               type,
						SpecialOptionType            gnomeType,
						const std::string            &gnomeName,
						CCSObjectAllocationInterface *allocator)
    {
	CCSIntegratedSettingInfo *integratedSettingInfo =
		ccsSharedIntegratedSettingInfoNew (plugin.c_str (),
						   setting.c_str (),
						   type,
						   allocator);
	/* This takes ownership of integratedSettingInfo */
	CCSGNOMEIntegratedSettingInfo *gnomeIntegratedSettingInfo =
		ccsGNOMEIntegratedSettingInfoNew (integratedSettingInfo,
						  gnomeType,
						  gnomeName.c_str (),
						  allocator);
	CCSIntegratedSetting          *integratedSetting =
		ccsMockIntegratedSettingNew (allocator);

	CCSIntegratedSettingGMock *mockPtr = GET_PRIVATE (CCSIntegratedSettingGMock, integratedSetting)
	CCSIntegratedSettingGMock &mock (*mockPtr);

	boost::shared_ptr <CCSIntegratedSetting> composition =
		AutoDestroy (ccsMockCompositionIntegratedSettingNew (integratedSetting,
								     gnomeIntegratedSettingInfo,
								     integratedSettingInfo,
								     allocator),
			     ccsIntegratedSettingUnref);

	/* We want the composition to take ownership here, so unref the
	 * original members of the composition */
	ccsGNOMEIntegratedSettingInfoUnref (gnomeIntegratedSettingInfo);
	ccsIntegratedSettingUnref (integratedSetting);

	return boost::make_shared <IntegratedSettingWithMock> (mock, composition);
    }

    CCSIntegratedSettingGMock &
    Mock (IntegratedSettingWithMock &integratedSettingWithMocks)
    {
	return std::tr1::get <0> (integratedSettingWithMocks);
    }

    CCSIntegratedSetting *
    Real (IntegratedSettingWithMock &integratedSettingWithMocks)
    {
	return std::tr1::get <1> (integratedSettingWithMocks).get ();
    }

    typedef std::tr1::tuple <CCSContextGMock                   &,
			     CCSBackendGMock                   &,
			     CCSIntegratedSettingsStorageGMock &,
			     CCSIntegratedSettingFactoryGMock  &,
			     boost::shared_ptr <CCSContext>     ,
			     boost::shared_ptr <CCSBackend>     ,
			     CCSIntegratedSettingsStorage      *,
			     CCSIntegratedSettingFactory       *,
			     boost::shared_ptr <CCSIntegration> > CCSGNOMEIntegrationWithMocks;

    CCSGNOMEIntegrationWithMocks
    createIntegrationWithMocks (CCSObjectAllocationInterface *ai)
    {
	boost::shared_ptr <CCSContext>     context (AutoDestroy (ccsMockContextNew (),
								 ccsFreeContext));
	boost::shared_ptr <CCSBackend>     backend (AutoDestroy (ccsMockBackendNew (),
								 ccsFreeMockBackend));
	/* The GNOME Integration backend takes ownership of these two */
	CCSIntegratedSettingsStorage       *storage (ccsMockIntegratedSettingsStorageNew (ai));
	CCSIntegratedSettingFactory        *factory (ccsMockIntegratedSettingFactoryNew (ai));
	boost::shared_ptr <CCSIntegration> integration (AutoDestroy (ccsGNOMEIntegrationBackendNew (backend.get (),
												    context.get (),
												    factory,
												    storage,
												    ai),
							ccsIntegrationUnref));

	CCSContextGMock                   &gmockContext = *(reinterpret_cast <CCSContextGMock *> (ccsObjectGetPrivate (context.get ())));
	CCSBackendGMock                   &gmockBackend = *(reinterpret_cast <CCSBackendGMock *> (ccsObjectGetPrivate (backend.get ())));
	CCSIntegratedSettingsStorageGMock &gmockStorage = *(reinterpret_cast <CCSIntegratedSettingsStorageGMock *> (ccsObjectGetPrivate (storage)));
	CCSIntegratedSettingFactoryGMock  &gmockFactory = *(reinterpret_cast <CCSIntegratedSettingFactoryGMock *> (ccsObjectGetPrivate (factory)));

	return CCSGNOMEIntegrationWithMocks (gmockContext,
					     gmockBackend,
					     gmockStorage,
					     gmockFactory,
					     context,
					     backend,
					     storage,
					     factory,
					     integration);
    }

    CCSIntegration *
    Real (CCSGNOMEIntegrationWithMocks &integrationWithMocks)
    {
	return std::tr1::get <8> (integrationWithMocks).get ();
    }

    CCSContextGMock &
    MockContext (CCSGNOMEIntegrationWithMocks &integrationWithMocks)
    {
	return std::tr1::get <0> (integrationWithMocks);
    }

    CCSBackendGMock &
    MockBackend (CCSGNOMEIntegrationWithMocks &integrationWithMocks)
    {
	return std::tr1::get <1> (integrationWithMocks);
    }

    CCSIntegratedSettingsStorageGMock &
    MockStorage (CCSGNOMEIntegrationWithMocks &integrationWithMocks)
    {
	return std::tr1::get <2> (integrationWithMocks);
    }

    CCSIntegratedSettingFactoryGMock &
    MockFactory (CCSGNOMEIntegrationWithMocks &integrationWithMocks)
    {
	return std::tr1::get <3> (integrationWithMocks);
    }

    void
    IgnoreRegistration (CCSIntegratedSettingsStorageGMock &storage)
    {
	EXPECT_CALL (storage, empty ()).WillOnce (Return (FALSE));
    }

    void
    AllowReadability (CCSSettingGMock &setting)
    {
	EXPECT_CALL (setting, isReadableByBackend ()).WillOnce (Return (TRUE));
    }

    void
    ExpectWriteSettings (CCSContextGMock &context)
    {
	EXPECT_CALL (context, writeChangedSettings ());
    }

    void
    SetNames (CCSSettingGMock   &setting,
	      CCSPluginGMock    &plugin,
	      const std::string &settingName,
	      const std::string &pluginName)
    {
	EXPECT_CALL (setting, getName ()).WillOnce (Return (settingName.c_str ()));
	EXPECT_CALL (setting, getParent ());
	EXPECT_CALL (plugin, getName ()).WillOnce (Return (pluginName.c_str ()));
    }

    void
    SetTypeInfo (CCSSettingGMock &setting,
		 CCSSettingType  type,
		 CCSSettingInfo  *info)
    {
	EXPECT_CALL (setting, getType ()).WillOnce (Return (type));
	EXPECT_CALL (setting, getInfo ()).WillOnce (Return (info));
    }

    CCSSettingValue *
    MakeSettingValue ()
    {
	CCSSettingValue *v = reinterpret_cast <CCSSettingValue *> (calloc (1, sizeof (CCSSettingValue)));
	v->parent = NULL;
	v->isListChild = FALSE;
	v->refCount = 1;

	return v;
    }

    boost::shared_ptr <CCSSettingValue>
    MakeAutoDestroySettingValue (CCSSettingType type)
    {
	CCSSettingValue *v = reinterpret_cast <CCSSettingValue *> (calloc (1, sizeof (CCSSettingValue)));
	v->parent = NULL;
	v->isListChild = FALSE;
	v->refCount = 1;

	return boost::shared_ptr <CCSSettingValue> (v,
						    boost::bind (ccsFreeSettingValueWithType, _1, type));
    }

    const std::string MOCK_PLUGIN ("mock");
    const std::string MOCK_SETTING ("mock");
    const std::string MOCK_GNOME_NAME ("mock");
    const CCSSettingType MOCK_SETTING_TYPE = TypeInt;
    const SpecialOptionType MOCK_GNOME_SETTING_TYPE = OptionInt;
}

TEST (CCSGNOMEIntegrationTest, TestConstructComposition)
{
    IntegratedSettingWithMockPtr settingWithMock (
	createIntegratedSettingCompositionFromMock (MOCK_PLUGIN,
						    MOCK_SETTING,
						    MOCK_SETTING_TYPE,
						    MOCK_GNOME_SETTING_TYPE,
						    MOCK_GNOME_NAME,
						    &ccsDefaultObjectAllocator));
}

class CCSGNOMEIntegrationTestWithMocks :
    public ::testing::Test
{
    public:

	CCSGNOMEIntegrationTestWithMocks () :
	    mIntegration (createIntegrationWithMocks (&ccsDefaultObjectAllocator)),
	    mSetting (AutoDestroy (ccsMockSettingNew (),
				   ccsSettingUnref)),
	    mSettingMock ((*(reinterpret_cast <CCSSettingGMock *> (ccsObjectGetPrivate (mSetting.get ()))))),
	    mIntegratedSetting (),
	    mPlugin (AutoDestroy (ccsMockPluginNew (),
				  ccsPluginUnref)),
	    mPluginMock ((*(reinterpret_cast <CCSPluginGMock *> (ccsObjectGetPrivate (mPlugin.get ())))))
	{
	    ON_CALL (mSettingMock, getParent ()).WillByDefault (Return (mPlugin.get ()));
	}

    protected:

	CCSGNOMEIntegrationWithMocks   mIntegration;
	boost::shared_ptr <CCSSetting> mSetting;
	CCSSettingGMock                &mSettingMock;
	IntegratedSettingWithMockPtr   mIntegratedSetting;
	boost::shared_ptr <CCSPlugin>  mPlugin;
	CCSPluginGMock                 &mPluginMock;
};

class CCSGNOMEIntegrationTestReadIntegrated :
    public CCSGNOMEIntegrationTestWithMocks
{
    public:

	virtual void SetUp ()
	{
	    IgnoreRegistration (MockStorage (mIntegration));
	    AllowReadability (mSettingMock);
	}
};

class CCSGNOMEIntegrationTestWriteIntegrated :
    public CCSGNOMEIntegrationTestWithMocks
{
    public:

	virtual void SetUp ()
	{
	    IgnoreRegistration (MockStorage (mIntegration));
	    ExpectWriteSettings (MockContext (mIntegration));
	}
};

TEST_F (CCSGNOMEIntegrationTestWithMocks, TestConstructGNOMEIntegration)
{
}

TEST_F (CCSGNOMEIntegrationTestReadIntegrated, TestReadInSpecialOptionCurrentViewport)
{
    const std::string settingName ("current_viewport");
    CCSSettingValue   *v = MakeSettingValue ();
    v->value.asBool = FALSE;

    mIntegratedSetting = createIntegratedSettingCompositionFromMock (MOCK_PLUGIN,
								     settingName,
								     TypeBool,
								     OptionSpecial,
								     MOCK_GNOME_NAME,
								     &ccsDefaultObjectAllocator);
    SetNames (mSettingMock, mPluginMock, settingName, MOCK_PLUGIN);
    EXPECT_CALL (Mock (*mIntegratedSetting), readValue (TypeBool)).WillOnce (Return (v));
    EXPECT_CALL (mSettingMock, setBool (IsTrue (), IsTrue ()));

    EXPECT_THAT (ccsIntegrationReadOptionIntoSetting (Real (mIntegration),
						      NULL,
						      mSetting.get (),
						      Real (*mIntegratedSetting)), IsTrue ());
}

TEST_F (CCSGNOMEIntegrationTestReadIntegrated, TestReadInSpecialOptionFullscreenVisualBell)
{
    const std::string settingName ("fullscreen_visual_bell");
    CCSSettingValue   *v = MakeSettingValue ();
    v->value.asString = strdup ("fullscreen");

    mIntegratedSetting = createIntegratedSettingCompositionFromMock (MOCK_PLUGIN,
								     settingName,
								     TypeString,
								     OptionSpecial,
								     MOCK_GNOME_NAME,
								     &ccsDefaultObjectAllocator);
    SetNames (mSettingMock, mPluginMock, settingName, MOCK_PLUGIN);
    EXPECT_CALL (Mock (*mIntegratedSetting), readValue (TypeString)).WillOnce (Return (v));
    EXPECT_CALL (mSettingMock, setBool (IsTrue (), IsTrue ()));

    EXPECT_THAT (ccsIntegrationReadOptionIntoSetting (Real (mIntegration),
						      NULL,
						      mSetting.get (),
						      Real (*mIntegratedSetting)), IsTrue ());
}

TEST_F (CCSGNOMEIntegrationTestReadIntegrated, TestReadInSpecialOptionClickToFocus)
{
    const std::string settingName ("click_to_focus");
    CCSSettingValue   *v = MakeSettingValue ();
    v->value.asString = strdup ("click");

    mIntegratedSetting = createIntegratedSettingCompositionFromMock (MOCK_PLUGIN,
								     settingName,
								     TypeString,
								     OptionSpecial,
								     MOCK_GNOME_NAME,
								     &ccsDefaultObjectAllocator);
    SetNames (mSettingMock, mPluginMock, settingName, MOCK_PLUGIN);
    EXPECT_CALL (Mock (*mIntegratedSetting), readValue (TypeString)).WillOnce (Return (v));
    EXPECT_CALL (mSettingMock, setBool (IsTrue (), IsTrue ()));

    EXPECT_THAT (ccsIntegrationReadOptionIntoSetting (Real (mIntegration),
						      NULL,
						      mSetting.get (),
						      Real (*mIntegratedSetting)), IsTrue ());
}

namespace
{
    struct IntegratedMediaKeysParam
    {
	public:

	    IntegratedMediaKeysParam (const std::string &settingName,
				      const std::string &integratedName,
				      const std::string &keyValueString) :
		settingName (settingName),
		integratedName (integratedName),
		keyValueString (keyValueString)
	    {
		EXPECT_TRUE (ccsStringToKeyBinding (keyValueString.c_str (),
						    &keyValue));
	    }

	    const std::string  settingName;
	    const std::string  integratedName;
	    const std::string  keyValueString;
	    CCSSettingKeyValue keyValue;
    };
}

namespace
{
    std::string RUN_COMMAND_SCREENSHOT_KEY ("run_command_screenshot_key");
    std::string SCREENSHOT ("screenshot");
    std::string SCREENSHOT_BINDING ("<Alt>Print");
    std::string RUN_COMMAND_WINDOW_SCREENSHOT_KEY ("run_command_window_screenshot_key");
    std::string WINDOW_SCREENSHOT ("window_screenshot");
    std::string WINDOW_SCREENSHOT_BINDING ("<Control><Alt>Print");
    std::string RUN_COMMAND_TERMINAL_KEY ("run_command_terminal_key");
    std::string TERMINAL ("terminal");
    std::string TERMINAL_BINDING ("<Control><Alt>t");
}

class CCSGNOMEIntegrationTestReadIntegratedMediaKeys :
    public CCSGNOMEIntegrationTestReadIntegrated,
    public WithParamInterface <IntegratedMediaKeysParam>
{
};

TEST_P (CCSGNOMEIntegrationTestReadIntegratedMediaKeys, TestReadIntegratedMediaKey)
{
    const std::string settingName (GetParam ().settingName);
    CCSSettingValue   *v = MakeSettingValue ();
    v->value.asString = strdup (GetParam ().keyValueString.c_str ());

    mIntegratedSetting = createIntegratedSettingCompositionFromMock (MOCK_PLUGIN,
								     settingName,
								     TypeString,
								     OptionSpecial,
								     MOCK_GNOME_NAME,
								     &ccsDefaultObjectAllocator);
    SetNames (mSettingMock, mPluginMock, settingName, MOCK_PLUGIN);
    EXPECT_CALL (Mock (*mIntegratedSetting), readValue (TypeString)).WillOnce (Return (v));
    EXPECT_CALL (mSettingMock, setKey (Eq (GetParam ().keyValue), IsTrue ()));

    EXPECT_THAT (ccsIntegrationReadOptionIntoSetting (Real (mIntegration),
						      NULL,
						      mSetting.get (),
						      Real (*mIntegratedSetting)), IsTrue ());
}

INSTANTIATE_TEST_CASE_P (CCSGNOMEMediaKeys, CCSGNOMEIntegrationTestReadIntegratedMediaKeys,
			 Values (IntegratedMediaKeysParam (RUN_COMMAND_SCREENSHOT_KEY,
							   SCREENSHOT,
							   SCREENSHOT_BINDING),
				 IntegratedMediaKeysParam (RUN_COMMAND_WINDOW_SCREENSHOT_KEY,
							   WINDOW_SCREENSHOT,
							   WINDOW_SCREENSHOT_BINDING),
				 IntegratedMediaKeysParam (RUN_COMMAND_TERMINAL_KEY,
							   TERMINAL,
							   TERMINAL_BINDING)));

namespace
{
    const std::string DEFAULT_MOUSE_BUTTON_MODIFIERS_STRING ("<Alt>");
    const std::string GNOME_MOUSE_BUTTON_MODIFIERS_STRING ("<Super>");
    const unsigned int DEFAULT_MOUSE_BUTTON_MODIFIERS =
	ccsStringToModifiers (DEFAULT_MOUSE_BUTTON_MODIFIERS_STRING.c_str ());
    const unsigned int GNOME_MOUSE_BUTTON_MODIFIERS =
	ccsStringToModifiers (GNOME_MOUSE_BUTTON_MODIFIERS_STRING.c_str ());

    const unsigned int LEFT_BUTTON = 1;
    const unsigned int MIDDLE_BUTTON = 2;
    const unsigned int RIGHT_BUTTON = 3;

    typedef cci::ListWrapper <CCSIntegratedSettingList, CCSIntegratedSetting *> CCSIntegratedSettingListWrapper;
    typedef boost::shared_ptr <CCSIntegratedSettingListWrapper> CCSIntegratedSettingListWrapperPtr;

    CCSIntegratedSettingListWrapperPtr
    constructCCSIntegratedSettingListWrapper (CCSIntegratedSetting *setting)
    {
	return boost::make_shared <CCSIntegratedSettingListWrapper> (ccsIntegratedSettingListAppend (NULL, setting),
								     ccsIntegratedSettingListFree,
								     ccsIntegratedSettingListAppend,
								     ccsIntegratedSettingListRemove,
								     cci::Shallow);
    }
}

class CCSGNOMEIntegrationTestWithMocksIntegratedMouseButtonModifiers
{
    public:

	CCSGNOMEIntegrationTestWithMocksIntegratedMouseButtonModifiers () :
	    mIntegratedSettingMBM (AutoDestroy (ccsMockIntegratedSettingNew (&ccsDefaultObjectAllocator),
						ccsIntegratedSettingUnref)),
	    mIntegratedSettingMBMMock (*(reinterpret_cast <CCSIntegratedSettingGMock *> (ccsObjectGetPrivate (mIntegratedSettingMBM.get ())))),
	    mIntegratedSettingsMBM (constructCCSIntegratedSettingListWrapper (mIntegratedSettingMBM.get ())),
	    mIntegratedSettingResizeWithRB (AutoDestroy (ccsMockIntegratedSettingNew (&ccsDefaultObjectAllocator),
							 ccsIntegratedSettingUnref)),
	    mIntegratedSettingResizeWithRBMock (*(reinterpret_cast <CCSIntegratedSettingGMock *> (ccsObjectGetPrivate (mIntegratedSettingResizeWithRB.get ())))),
	    mIntegratedSettingsResizeWithRB (constructCCSIntegratedSettingListWrapper (mIntegratedSettingResizeWithRB.get ()))
	{
	    memset (&mButtonValue, 0, sizeof (CCSSettingButtonValue));

	    mButtonValue.button        = MIDDLE_BUTTON;
	    mButtonValue.buttonModMask = DEFAULT_MOUSE_BUTTON_MODIFIERS;
	    mButtonValue.edgeMask      = 0;
	}

	virtual void SetUp (CCSGNOMEIntegrationWithMocks &integration)
	{
	    CCSIntegratedSettingList integratedSettingsMBM =
		    *mIntegratedSettingsMBM;
	    CCSIntegratedSettingList integratedSettingsResizeWithRB =
		    *mIntegratedSettingsResizeWithRB;

	    EXPECT_CALL (MockStorage (integration),
			 findMatchingSettingsByPluginAndSettingName (Eq (std::string (ccsGNOMEIntegratedPluginNames.SPECIAL)),
								     Eq (std::string (ccsGNOMEIntegratedSettingNames.NULL_MOUSE_BUTTON_MODIFIER.compizName))))
		    .WillOnce (Return (integratedSettingsMBM));
	    EXPECT_CALL (MockStorage (integration),
			 findMatchingSettingsByPluginAndSettingName (Eq (std::string (ccsGNOMEIntegratedPluginNames.SPECIAL)),
								     Eq (std::string (ccsGNOMEIntegratedSettingNames.NULL_RESIZE_WITH_RIGHT_BUTTON.compizName))))
		    .WillOnce (Return (integratedSettingsResizeWithRB));
	}


    protected:

	boost::shared_ptr <CCSIntegratedSetting> mIntegratedSettingMBM;
	CCSIntegratedSettingGMock                &mIntegratedSettingMBMMock;
	CCSIntegratedSettingListWrapperPtr       mIntegratedSettingsMBM;
	boost::shared_ptr <CCSIntegratedSetting> mIntegratedSettingResizeWithRB;
	CCSIntegratedSettingGMock                &mIntegratedSettingResizeWithRBMock;
	CCSIntegratedSettingListWrapperPtr       mIntegratedSettingsResizeWithRB;
	CCSSettingButtonValue                    mButtonValue;
};

class CCSGNOMEIntegrationTestWithMocksReadIntegratedMouseButtonModifiers :
    public CCSGNOMEIntegrationTestReadIntegrated,
    public CCSGNOMEIntegrationTestWithMocksIntegratedMouseButtonModifiers
{
    public:

	virtual void SetUp ()
	{
	    CCSSettingValue   *vRB = MakeSettingValue ();
	    CCSSettingValue   *vMBM = MakeSettingValue ();
	    vRB->value.asBool = TRUE;
	    vMBM->value.asString = strdup (GNOME_MOUSE_BUTTON_MODIFIERS_STRING.c_str ());

	    CCSGNOMEIntegrationTestReadIntegrated::SetUp ();
	    CCSGNOMEIntegrationTestWithMocksIntegratedMouseButtonModifiers::SetUp (mIntegration);

	    EXPECT_CALL (mSettingMock, getButton (_))
		    .WillOnce (DoAll (
				SetArgPointee <0> (
				  mButtonValue),
				Return (TRUE)));

	    /* Get the GNOME Mouse button modifier */
	    EXPECT_CALL (mIntegratedSettingMBMMock, readValue (TypeString)).WillOnce (Return (vMBM));
	    EXPECT_CALL (mIntegratedSettingResizeWithRBMock, readValue (TypeBool)).WillOnce (Return (vRB));
	}
};

class CCSGNOMEIntegrationTestWithMocksWriteIntegratedMouseButtonModifiers :
    public CCSGNOMEIntegrationTestWriteIntegrated,
    public CCSGNOMEIntegrationTestWithMocksIntegratedMouseButtonModifiers
{
    public:

	CCSGNOMEIntegrationTestWithMocksWriteIntegratedMouseButtonModifiers () :
	    corePlugin (AutoDestroy (ccsMockPluginNew (),
				     ccsPluginUnref)),
	    corePluginMock (*(reinterpret_cast <CCSPluginGMock *> (ccsObjectGetPrivate (corePlugin.get ())))),
	    resizePlugin (AutoDestroy (ccsMockPluginNew (),
				       ccsPluginUnref)),
	    resizePluginMock (*(reinterpret_cast <CCSPluginGMock *> (ccsObjectGetPrivate (resizePlugin.get ())))),
	    movePlugin (AutoDestroy (ccsMockPluginNew (),
				     ccsPluginUnref)),
	    movePluginMock (*(reinterpret_cast <CCSPluginGMock *> (ccsObjectGetPrivate (movePlugin.get ())))),
	    resizeInitiateButtonSetting (AutoDestroy (ccsMockSettingNew (),
						      ccsSettingUnref)),
	    resizeInitiateButtonSettingMock (*(reinterpret_cast <CCSSettingGMock *> (ccsObjectGetPrivate (resizeInitiateButtonSetting.get ())))),
	    moveInitiateButtonSetting (AutoDestroy (ccsMockSettingNew (),
						    ccsSettingUnref)),
	    moveInitiateButtonSettingMock (*(reinterpret_cast <CCSSettingGMock *> (ccsObjectGetPrivate (moveInitiateButtonSetting.get ())))),
	    coreWindowMenuSetting (AutoDestroy (ccsMockSettingNew (),
						ccsSettingUnref)),
	    coreWindowMenuSettingMock (*(reinterpret_cast <CCSSettingGMock *> (ccsObjectGetPrivate (coreWindowMenuSetting.get ())))),
	    resizeInitiateButton (MakeAutoDestroySettingValue (TypeButton)),
	    moveInitiateButton (MakeAutoDestroySettingValue (TypeButton)),
	    coreWindowMenuButton (MakeAutoDestroySettingValue (TypeButton))
	{
	    resizeInitiateButton->value.asButton.button = LEFT_BUTTON;
	    resizeInitiateButton->value.asButton.buttonModMask = 0;
	    resizeInitiateButton->value.asButton.edgeMask = 0;
	    moveInitiateButton->value.asButton.button = LEFT_BUTTON;
	    moveInitiateButton->value.asButton.buttonModMask = DEFAULT_MOUSE_BUTTON_MODIFIERS;
	    moveInitiateButton->value.asButton.edgeMask = 0;
	    coreWindowMenuButton->value.asButton.button = MIDDLE_BUTTON;
	    coreWindowMenuButton->value.asButton.buttonModMask = 0;
	    coreWindowMenuButton->value.asButton.edgeMask = 0;
	}

	virtual void SetUp ()
	{
	    CCSGNOMEIntegrationTestWriteIntegrated::SetUp ();
	    CCSGNOMEIntegrationTestWithMocksIntegratedMouseButtonModifiers::SetUp (mIntegration);
	}

    protected:

	boost::shared_ptr <CCSPlugin> corePlugin;
	CCSPluginGMock                &corePluginMock;
	boost::shared_ptr <CCSPlugin> resizePlugin;
	CCSPluginGMock                &resizePluginMock;
	boost::shared_ptr <CCSPlugin> movePlugin;
	CCSPluginGMock                &movePluginMock;

	boost::shared_ptr <CCSSetting> resizeInitiateButtonSetting;
	CCSSettingGMock                &resizeInitiateButtonSettingMock;
	boost::shared_ptr <CCSSetting> moveInitiateButtonSetting;
	CCSSettingGMock                &moveInitiateButtonSettingMock;
	boost::shared_ptr <CCSSetting> coreWindowMenuSetting;
	CCSSettingGMock                &coreWindowMenuSettingMock;

	boost::shared_ptr <CCSSettingValue> resizeInitiateButton;
	boost::shared_ptr <CCSSettingValue> moveInitiateButton;
	boost::shared_ptr <CCSSettingValue> coreWindowMenuButton;
};

TEST_F (CCSGNOMEIntegrationTestWithMocksReadIntegratedMouseButtonModifiers, TestReadInSpecialOptionMoveInitiateButton)
{
    const std::string settingName ("initiate_button");
    const std::string pluginName ("move");

    CCSSettingButtonValue newButtonValue = mButtonValue;
    newButtonValue.button = LEFT_BUTTON;
    newButtonValue.buttonModMask = GNOME_MOUSE_BUTTON_MODIFIERS;

    mIntegratedSetting = createIntegratedSettingCompositionFromMock (pluginName,
								     settingName,
								     TypeBool,
								     OptionSpecial,
								     MOCK_GNOME_NAME,
								     &ccsDefaultObjectAllocator);
    SetNames (mSettingMock, mPluginMock, settingName, pluginName);

    /* Set the new mouse button modifier */
    EXPECT_CALL (mSettingMock, setButton (Eq (newButtonValue),
					  IsTrue ()));

    EXPECT_THAT (ccsIntegrationReadOptionIntoSetting (Real (mIntegration),
						      NULL,
						      mSetting.get (),
						      Real (*mIntegratedSetting)), IsTrue ());
}

TEST_F (CCSGNOMEIntegrationTestWithMocksReadIntegratedMouseButtonModifiers, TestReadInSpecialOptionResizeInitiateButton)
{
    const std::string settingName ("initiate_button");
    const std::string pluginName ("resize");

    CCSSettingButtonValue newButtonValue = mButtonValue;
    newButtonValue.button = RIGHT_BUTTON;
    newButtonValue.buttonModMask = GNOME_MOUSE_BUTTON_MODIFIERS;

    mIntegratedSetting = createIntegratedSettingCompositionFromMock (pluginName,
								     settingName,
								     TypeBool,
								     OptionSpecial,
								     MOCK_GNOME_NAME,
								     &ccsDefaultObjectAllocator);
    SetNames (mSettingMock, mPluginMock, settingName, pluginName);

    /* Set the new mouse button modifier */
    EXPECT_CALL (mSettingMock, setButton (Eq (newButtonValue),
					  IsTrue ()));

    EXPECT_THAT (ccsIntegrationReadOptionIntoSetting (Real (mIntegration),
						      NULL,
						      mSetting.get (),
						      Real (*mIntegratedSetting)), IsTrue ());
}

TEST_F (CCSGNOMEIntegrationTestWithMocksReadIntegratedMouseButtonModifiers, TestReadInSpecialOptionWhereIntegratedOptionReturnsNull)
{
    const std::string settingName ("initiate_button");
    const std::string pluginName ("move");

    CCSSettingButtonValue newButtonValue = mButtonValue;
    newButtonValue.button = LEFT_BUTTON;
    /* Reading the gnome mouse button modifiers failed, so default to zero */
    newButtonValue.buttonModMask = 0;

    /* Clear the old expectation */
    ccsFreeSettingValueWithType (ccsIntegratedSettingReadValue (mIntegratedSettingMBM.get (), TypeString),
				 TypeString);
    /* Now return null */
    EXPECT_CALL (mIntegratedSettingMBMMock, readValue (TypeString)).WillOnce (ReturnNull ());

    mIntegratedSetting = createIntegratedSettingCompositionFromMock (pluginName,
								     settingName,
								     TypeBool,
								     OptionSpecial,
								     MOCK_GNOME_NAME,
								     &ccsDefaultObjectAllocator);
    SetNames (mSettingMock, mPluginMock, settingName, pluginName);

    /* Set the new mouse button modifier */
    EXPECT_CALL (mSettingMock, setButton (Eq (newButtonValue),
					  IsTrue ()));

    EXPECT_THAT (ccsIntegrationReadOptionIntoSetting (Real (mIntegration),
						      NULL,
						      mSetting.get (),
						      Real (*mIntegratedSetting)), IsTrue ());
}

TEST_F (CCSGNOMEIntegrationTestWriteIntegrated, TestWriteCurrentViewport)
{
    const std::string                   settingName ("current_viewport");
    boost::shared_ptr <CCSSettingValue> compizValue (MakeAutoDestroySettingValue (TypeBool));
    boost::shared_ptr <CCSSettingValue> gnomeValue (MakeAutoDestroySettingValue (TypeBool));
    CCSSettingInfo                      info;

    compizValue->value.asBool = TRUE;
    gnomeValue->value.asBool = FALSE;

    mIntegratedSetting = createIntegratedSettingCompositionFromMock (MOCK_PLUGIN,
								     settingName,
								     TypeBool,
								     OptionSpecial,
								     MOCK_GNOME_NAME,
								     &ccsDefaultObjectAllocator);
    SetNames (mSettingMock, mPluginMock, settingName, MOCK_PLUGIN);
    SetTypeInfo (mSettingMock, TypeBool, &info);

    EXPECT_CALL (mSettingMock, getValue ()).WillOnce (Return (compizValue.get ()));
    EXPECT_CALL (Mock (*mIntegratedSetting), writeValue (Pointee (SettingValueMatch (*gnomeValue,
										     TypeBool,
										     &info)),
							 TypeBool));
    ccsIntegrationWriteSettingIntoOption (Real (mIntegration),
					  NULL,
					  mSetting.get (),
					  Real (*mIntegratedSetting));
}

TEST_F (CCSGNOMEIntegrationTestWriteIntegrated, TestWriteFullscreenVisualBell)
{
    const std::string                   settingName ("fullscreen_visual_bell");
    boost::shared_ptr <CCSSettingValue> compizValue (MakeAutoDestroySettingValue (TypeBool));
    boost::shared_ptr <CCSSettingValue> gnomeValue (MakeAutoDestroySettingValue (TypeString));
    CCSSettingInfo                      info;

    compizValue->value.asBool = TRUE;
    gnomeValue->value.asString = strdup ("fullscreen");

    mIntegratedSetting = createIntegratedSettingCompositionFromMock (MOCK_PLUGIN,
								     settingName,
								     TypeBool,
								     OptionSpecial,
								     MOCK_GNOME_NAME,
								     &ccsDefaultObjectAllocator);
    SetNames (mSettingMock, mPluginMock, settingName, MOCK_PLUGIN);
    SetTypeInfo (mSettingMock, TypeBool, &info);

    EXPECT_CALL (mSettingMock, getValue ()).WillOnce (Return (compizValue.get ()));
    EXPECT_CALL (Mock (*mIntegratedSetting), writeValue (Pointee (SettingValueMatch (*gnomeValue,
										     TypeString,
										     &info)),
							 TypeString));
    ccsIntegrationWriteSettingIntoOption (Real (mIntegration),
					  NULL,
					  mSetting.get (),
					  Real (*mIntegratedSetting));
}

TEST_F (CCSGNOMEIntegrationTestWriteIntegrated, TestWriteClickToFocus)
{
    const std::string                   settingName ("click_to_focus");
    boost::shared_ptr <CCSSettingValue> compizValue (MakeAutoDestroySettingValue (TypeBool));
    boost::shared_ptr <CCSSettingValue> gnomeValue (MakeAutoDestroySettingValue (TypeString));
    CCSSettingInfo                      info;

    compizValue->value.asBool = TRUE;
    gnomeValue->value.asString = strdup ("click");

    mIntegratedSetting = createIntegratedSettingCompositionFromMock (MOCK_PLUGIN,
								     settingName,
								     TypeBool,
								     OptionSpecial,
								     MOCK_GNOME_NAME,
								     &ccsDefaultObjectAllocator);
    SetNames (mSettingMock, mPluginMock, settingName, MOCK_PLUGIN);
    SetTypeInfo (mSettingMock, TypeBool, &info);

    EXPECT_CALL (mSettingMock, getValue ()).WillOnce (Return (compizValue.get ()));
    EXPECT_CALL (Mock (*mIntegratedSetting), writeValue (Pointee (SettingValueMatch (*gnomeValue,
										     TypeString,
										     &info)),
							 TypeString));
    ccsIntegrationWriteSettingIntoOption (Real (mIntegration),
					  NULL,
					  mSetting.get (),
					  Real (*mIntegratedSetting));
}

class CCSGNOMEIntegrationTestWriteIntegratedMediaKeys :
    public CCSGNOMEIntegrationTestWriteIntegrated,
    public WithParamInterface <IntegratedMediaKeysParam>
{
};

TEST_P (CCSGNOMEIntegrationTestWriteIntegratedMediaKeys, DISABLED_TestWriteIntegratedMediaKey)
{
    const std::string                   settingName (GetParam ().settingName);
    boost::shared_ptr <CCSSettingValue> compizValue (MakeAutoDestroySettingValue (TypeKey));
    boost::shared_ptr <CCSSettingValue> gnomeValue (MakeAutoDestroySettingValue (TypeString));
    CCSSettingInfo                      info;

    compizValue->value.asKey = GetParam ().keyValue;
    gnomeValue->value.asString = strdup (GetParam ().keyValueString.c_str ());

    mIntegratedSetting = createIntegratedSettingCompositionFromMock (MOCK_PLUGIN,
								     settingName,
								     TypeString,
								     OptionSpecial,
								     MOCK_GNOME_NAME,
								     &ccsDefaultObjectAllocator);
    SetNames (mSettingMock, mPluginMock, settingName, MOCK_PLUGIN);
    SetTypeInfo (mSettingMock, TypeKey, &info);

    EXPECT_CALL (mSettingMock, getValue ()).WillOnce (Return (compizValue.get ()));
    EXPECT_CALL (Mock (*mIntegratedSetting), writeValue (Pointee (SettingValueMatch (*gnomeValue,
										     TypeString,
										     &info)),
							 TypeString));
    ccsIntegrationWriteSettingIntoOption (Real (mIntegration),
					  NULL,
					  mSetting.get (),
					  Real (*mIntegratedSetting));
}

INSTANTIATE_TEST_CASE_P (CCSGNOMEMediaKeys, CCSGNOMEIntegrationTestWriteIntegratedMediaKeys,
			 Values (IntegratedMediaKeysParam (RUN_COMMAND_SCREENSHOT_KEY,
							   SCREENSHOT,
							   SCREENSHOT_BINDING),
				 IntegratedMediaKeysParam (RUN_COMMAND_WINDOW_SCREENSHOT_KEY,
							   WINDOW_SCREENSHOT,
							   WINDOW_SCREENSHOT_BINDING),
				 IntegratedMediaKeysParam (RUN_COMMAND_TERMINAL_KEY,
							   TERMINAL,
							   TERMINAL_BINDING)));

/*
 * TODO: Break up the function that this test covers. Its way too complicated
 */
TEST_F (CCSGNOMEIntegrationTestWithMocksWriteIntegratedMouseButtonModifiers, TestWriteMouseButtonModifier)
{
    const std::string moveSettingName ("initiate_button");
    const std::string movePluginName ("move");
    const std::string resizeSettingName ("initiate_button");
    const std::string resizePluginName ("resize");
    const std::string coreSettingName ("window_menu_button");
    const std::string corePluginName ("core");

    const std::string settingName (moveSettingName);
    const std::string pluginName  (movePluginName);

    CCSSettingInfo info;

    mIntegratedSetting = createIntegratedSettingCompositionFromMock (pluginName,
								     settingName,
								     TypeBool,
								     OptionSpecial,
								     MOCK_GNOME_NAME,
								     &ccsDefaultObjectAllocator);
    SetNames (mSettingMock, mPluginMock, settingName, pluginName);
    SetTypeInfo (mSettingMock, TypeButton, &info);

    EXPECT_CALL (MockContext (mIntegration), findPlugin (Eq (movePluginName))).WillRepeatedly (Return (movePlugin.get ()));
    EXPECT_CALL (movePluginMock, findSetting (Eq (moveSettingName))).WillRepeatedly (Return (moveInitiateButtonSetting.get ()));
    EXPECT_CALL (moveInitiateButtonSettingMock, getType ()).WillRepeatedly (Return (TypeButton));
    EXPECT_CALL (moveInitiateButtonSettingMock, getValue ()).WillRepeatedly (Return (moveInitiateButton.get ()));

    EXPECT_CALL (MockContext (mIntegration), findPlugin (Eq (resizePluginName))).WillRepeatedly (Return (resizePlugin.get ()));
    EXPECT_CALL (resizePluginMock, findSetting (Eq (resizeSettingName))).WillRepeatedly (Return (resizeInitiateButtonSetting.get ()));
    EXPECT_CALL (resizeInitiateButtonSettingMock, getType ()).WillRepeatedly (Return (TypeButton));
    EXPECT_CALL (resizeInitiateButtonSettingMock, getValue ()).WillRepeatedly (Return (resizeInitiateButton.get ()));

    EXPECT_CALL (MockContext (mIntegration), findPlugin (Eq (corePluginName))).WillRepeatedly (Return (corePlugin.get ()));
    EXPECT_CALL (corePluginMock, findSetting (Eq (coreSettingName))).WillRepeatedly (Return (coreWindowMenuSetting.get ()));
    EXPECT_CALL (coreWindowMenuSettingMock, getType ()).WillRepeatedly (Return (TypeButton));
    EXPECT_CALL (coreWindowMenuSettingMock, getValue ()).WillRepeatedly (Return (coreWindowMenuButton.get ()));

    boost::shared_ptr <CCSSettingValue> newResizeWithRBValue (MakeAutoDestroySettingValue (TypeBool));
    newResizeWithRBValue->value.asBool = TRUE;

    EXPECT_CALL (mIntegratedSettingResizeWithRBMock, writeValue (Pointee (SettingValueMatch (*newResizeWithRBValue,
											     TypeBool,
											     &info)),
								 TypeBool));

    boost::shared_ptr <CCSSettingValue> newMBMValue (MakeAutoDestroySettingValue (TypeString));
    newMBMValue->value.asString = strdup (DEFAULT_MOUSE_BUTTON_MODIFIERS_STRING.c_str ());

    EXPECT_CALL (mSettingMock, getValue ()).WillOnce (Return (moveInitiateButton.get ()));

    EXPECT_CALL (mIntegratedSettingMBMMock, writeValue (Pointee (SettingValueMatch (*newMBMValue,
										    TypeString,
										    &info)),
							TypeString));

    unsigned int modifiers = moveInitiateButton->value.asButton.buttonModMask;

    CCSSettingButtonValue newResizeInitiateButton;
    CCSSettingButtonValue newMoveInitiateButton;
    CCSSettingButtonValue newWindowMenuButton;

    memset (&newResizeInitiateButton, 0, sizeof (CCSSettingButtonValue));
    memset (&newMoveInitiateButton, 0, sizeof (CCSSettingButtonValue));
    memset (&newWindowMenuButton, 0, sizeof (CCSSettingButtonValue));

    newResizeInitiateButton.button = RIGHT_BUTTON;
    newResizeInitiateButton.buttonModMask = modifiers;
    newMoveInitiateButton.button = LEFT_BUTTON;
    newMoveInitiateButton.buttonModMask = modifiers;
    newWindowMenuButton.button = MIDDLE_BUTTON;
    newWindowMenuButton.buttonModMask = modifiers;

    EXPECT_CALL (resizeInitiateButtonSettingMock, setButton (Eq (newResizeInitiateButton),
							     IsTrue ()));
    /* The move button is exactly the same, so it is not updated */
    EXPECT_CALL (moveInitiateButtonSettingMock, setButton (Eq (newMoveInitiateButton),
							   IsTrue ())).Times (0);
    EXPECT_CALL (coreWindowMenuSettingMock, setButton (Eq (newWindowMenuButton),
						       IsTrue ()));

    ccsIntegrationWriteSettingIntoOption (Real (mIntegration),
					  NULL,
					  mSetting.get (),
					  Real (*mIntegratedSetting));
}
