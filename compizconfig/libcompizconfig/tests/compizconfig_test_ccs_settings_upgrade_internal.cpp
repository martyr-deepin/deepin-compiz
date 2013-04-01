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

#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

#include <ccs.h>

#include "compizconfig_ccs_context_mock.h"
#include "compizconfig_ccs_plugin_mock.h"
#include "compizconfig_ccs_setting_mock.h"

#include "ccs_settings_upgrade_internal.h"
#include "gtest_shared_characterwrapper.h"
#include "gtest_shared_autodestroy.h"
#include "gtest_unspecified_bool_type_matcher.h"
#include "compizconfig_ccs_list_equality.h"
#include "compizconfig_ccs_item_in_list_matcher.h"
#include "compizconfig_ccs_list_wrapper.h"
#include "compizconfig_ccs_setting_value_operators.h"
#include "compizconfig_ccs_setting_value_matcher.h"

using ::testing::IsNull;
using ::testing::Eq;
using ::testing::Return;
using ::testing::AtLeast;
using ::testing::_;
using ::testing::AllOf;
using ::testing::Not;
using ::testing::Matcher;
using ::testing::MakeMatcher;
using ::testing::MatcherInterface;
using ::testing::Pointee;

namespace cc = compiz::config;
namespace cci = compiz::config::impl;

class CCSSettingsUpgradeInternalTest :
    public ::testing::Test
{
};

namespace
{
    static const std::string CCS_SETTINGS_UPGRADE_TEST_CORRECT_FILENAME = "org.compiz.general.1.upgrade";
    static const std::string CCS_SETTINGS_UPGRADE_TEST_INCORRECT_FILENAME = "1.upgra";
    static const std::string CCS_SETTINGS_UPGRADE_TEST_VERY_INCORRECT_FILENAME = "1";
    static const std::string CCS_SETTINGS_UPGRADE_TEST_CORRECT_DOMAIN = "org.compiz";
    static const std::string CCS_SETTINGS_UPGRADE_TEST_CORRECT_PROFILE = "general";
    static const unsigned int CCS_SETTINGS_UPGRADE_TEST_CORRECT_NUM = 1;
}

TEST (CCSSettingsUpgradeInternalTest, TestDetokenizeAndSetValues)
{
    char *profileName = NULL;
    char *domainName = NULL;

    unsigned int num;

    EXPECT_THAT (ccsUpgradeGetDomainNumAndProfile (CCS_SETTINGS_UPGRADE_TEST_CORRECT_FILENAME.c_str (),
						   &domainName,
						   &num,
						   &profileName), IsTrue ());

    CharacterWrapper profileNameC (profileName);
    CharacterWrapper domainNameC (domainName);

    EXPECT_EQ (CCS_SETTINGS_UPGRADE_TEST_CORRECT_PROFILE, profileName);
    EXPECT_EQ (CCS_SETTINGS_UPGRADE_TEST_CORRECT_DOMAIN, domainName);
    EXPECT_EQ (num, CCS_SETTINGS_UPGRADE_TEST_CORRECT_NUM);
}

TEST (CCSSettingsUpgradeInternalTest, TestDetokenizeAndSetValuesReturnsFalseIfInvalid)
{
    char *profileName = NULL;
    char *domainName = NULL;

    unsigned int num;

    EXPECT_THAT (ccsUpgradeGetDomainNumAndProfile (CCS_SETTINGS_UPGRADE_TEST_INCORRECT_FILENAME.c_str (),
						   &domainName,
						   &num,
						   &profileName), IsFalse ());

    EXPECT_THAT (profileName, IsNull ());
    EXPECT_THAT (domainName, IsNull ());

    EXPECT_THAT (ccsUpgradeGetDomainNumAndProfile (CCS_SETTINGS_UPGRADE_TEST_VERY_INCORRECT_FILENAME.c_str (),
						   &domainName,
						   &num,
						   &profileName), IsFalse ());

    EXPECT_THAT (profileName, IsNull ());
    EXPECT_THAT (domainName, IsNull ());
}

TEST (CCSSettingsUpgradeInternalTest, TestDetokenizeAndReturnTrueForUpgradeFileName)
{
    EXPECT_THAT (ccsUpgradeNameFilter (CCS_SETTINGS_UPGRADE_TEST_CORRECT_FILENAME.c_str ()), IsTrue ());
}

TEST (CCSSettingsUpgradeInternalTest, TestDetokenizeAndReturnFalseForNoUpgradeFileName)
{
    EXPECT_THAT (ccsUpgradeNameFilter (CCS_SETTINGS_UPGRADE_TEST_INCORRECT_FILENAME.c_str ()), IsFalse ());
    EXPECT_THAT (ccsUpgradeNameFilter (CCS_SETTINGS_UPGRADE_TEST_VERY_INCORRECT_FILENAME.c_str ()), IsFalse ());
}

namespace
{
    const std::string CCS_SETTINGS_UPGRADE_TEST_MOCK_PLUGIN_NAME = "mock";
    const std::string CCS_SETTINGS_UPGRADE_TEST_MOCK_SETTING_NAME_ONE = "setting_one";
    const std::string CCS_SETTINGS_UPGRADE_TEST_MOCK_SETTING_NAME_TWO = "setting_two";

    typedef std::tr1::tuple <boost::shared_ptr <CCSSetting>,
			     CCSSettingGMock *> MockedSetting;

    inline CCSSetting      * Real (MockedSetting &ms) { return (std::tr1::get <0> (ms)).get (); }
    inline CCSSettingGMock & Mock (MockedSetting &ms) { return *(std::tr1::get <1> (ms)); }
}

class CCSSettingsUpgradeTestWithMockContext :
    public ::testing::Test
{
    public:

	typedef enum _AddMode
	{
	    DoNotAddSettingToPlugin,
	    AddSettingToPlugin
	} AddMode;

	virtual void SetUp ()
	{
	    context = AutoDestroy <CCSContext> (ccsMockContextNew (),
						ccsFreeMockContext);
	    plugin = AutoDestroy <CCSPlugin> (ccsMockPluginNew (),
					      ccsFreeMockPlugin);

	    ON_CALL (MockPlugin (), getName ())
		    .WillByDefault (
			Return (

			    CCS_SETTINGS_UPGRADE_TEST_MOCK_PLUGIN_NAME.c_str ()));

	    ON_CALL (MockPlugin (), getContext ())
		    .WillByDefault (
			Return (
			    context.get ()));
	}

	CCSPluginGMock & MockPlugin ()
	{
	    return *(reinterpret_cast <CCSPluginGMock *> (ccsObjectGetPrivate (plugin.get ())));
	}

	CCSContextGMock & MockContext ()
	{
	    return *(reinterpret_cast <CCSContextGMock *> (ccsObjectGetPrivate (context.get ())));
	}

	void InitializeValueCommon (CCSSettingValue &value,
				    CCSSetting      *setting)
	{
	    value.parent = setting;
	    value.refCount = 1;
	}

	void InitializeValueForSetting (CCSSettingValue &value,
					CCSSetting      *setting)
	{
	    InitializeValueCommon (value, setting);
	    value.isListChild = FALSE;
	}


	MockedSetting
	SpawnSetting (const std::string &name,
		      CCSSettingType    type,
		      AddMode		addMode = AddSettingToPlugin)
	{
	    boost::shared_ptr <CCSSetting> setting (ccsMockSettingNew (),
						    ccsSettingUnref);

	    CCSSettingGMock *gmockSetting = reinterpret_cast <CCSSettingGMock *> (ccsObjectGetPrivate (setting.get ()));

	    ON_CALL (*gmockSetting, getName ())
		    .WillByDefault (
			Return (
			    name.c_str ()));
	    ON_CALL (*gmockSetting, getType ())
		    .WillByDefault (
			Return (
			    type));

	    ON_CALL (*gmockSetting, getParent ())
		    .WillByDefault (
			Return (
			    plugin.get ()));

	    if (addMode == AddSettingToPlugin)
	    {
		ON_CALL (MockPlugin (), findSetting (Eq (name.c_str ())))
			.WillByDefault (
			    Return (
				setting.get ()));
	    }

	    return MockedSetting (setting, gmockSetting);
	}

    private:

	boost::shared_ptr <CCSContext> context;
	boost::shared_ptr <CCSPlugin>  plugin;
};

namespace
{
    typedef boost::shared_ptr <cc::ListWrapper <CCSSettingList, CCSSetting *> > CCSSettingListWrapperPtr;

    CCSSettingListWrapperPtr
    constructSettingListWrapper (CCSSettingList       list,
				 cci::ListStorageType storageType)
    {
	return boost::make_shared <cci::ListWrapper <CCSSettingList, CCSSetting *> > (list,
											 ccsSettingListFree,
											 ccsSettingListAppend,
											 ccsSettingListRemove,
											 storageType);
    }
}

TEST_F (CCSSettingsUpgradeTestWithMockContext, TestNoClearValuesSettingNotFound)
{
    MockedSetting settingOne (SpawnSetting (CCS_SETTINGS_UPGRADE_TEST_MOCK_SETTING_NAME_ONE,
					    TypeInt,
					    DoNotAddSettingToPlugin));

    EXPECT_CALL (MockPlugin (), findSetting (Eq (CCS_SETTINGS_UPGRADE_TEST_MOCK_SETTING_NAME_ONE)));
    EXPECT_CALL (Mock (settingOne), getParent ());
    EXPECT_CALL (Mock (settingOne), getName ());

    CCSSettingListWrapperPtr list (constructSettingListWrapper (ccsSettingListAppend (NULL, Real (settingOne)),
								cci::Shallow));

    ccsUpgradeClearValues (*list);
}

TEST_F (CCSSettingsUpgradeTestWithMockContext, TestClearValuesInListNonListType)
{
    MockedSetting resetSettingIdentifier (SpawnSetting (CCS_SETTINGS_UPGRADE_TEST_MOCK_SETTING_NAME_ONE,
							TypeInt,
							DoNotAddSettingToPlugin));
    MockedSetting settingToReset (SpawnSetting (CCS_SETTINGS_UPGRADE_TEST_MOCK_SETTING_NAME_ONE,
						TypeInt));
    CCSSettingValue valueToReset;
    CCSSettingValue valueResetIdentifier;

    InitializeValueForSetting (valueToReset, Real (settingToReset));
    InitializeValueForSetting (valueResetIdentifier, Real (resetSettingIdentifier));

    valueToReset.value.asInt = 7;
    valueResetIdentifier.value.asInt = 7;

    CCSSettingListWrapperPtr list (constructSettingListWrapper (ccsSettingListAppend (NULL, Real (resetSettingIdentifier)),
								cci::Shallow));

    EXPECT_CALL (MockPlugin (), findSetting (Eq (CCS_SETTINGS_UPGRADE_TEST_MOCK_SETTING_NAME_ONE)))
	    .WillOnce (Return (Real (settingToReset)));
    EXPECT_CALL (Mock (resetSettingIdentifier), getParent ());
    EXPECT_CALL (Mock (resetSettingIdentifier), getName ());

    CCSSettingInfo info;

    info.forInt.max = 0;
    info.forInt.min = 10;

    /* ccsCheckValueEq needs to know the type and info about this type */
    EXPECT_CALL (Mock (settingToReset), getType ()).Times (AtLeast (1));
    EXPECT_CALL (Mock (resetSettingIdentifier), getType ()).Times (AtLeast (1));

    EXPECT_CALL (Mock (settingToReset), getInfo ()).WillRepeatedly (Return (&info));
    EXPECT_CALL (Mock (resetSettingIdentifier), getInfo ()).WillRepeatedly (Return (&info));

    EXPECT_CALL (Mock (resetSettingIdentifier), getValue ()).WillOnce (Return (&valueResetIdentifier));
    EXPECT_CALL (Mock (settingToReset), getValue ()).WillOnce (Return (&valueToReset));

    EXPECT_CALL (Mock (settingToReset), resetToDefault (IsTrue ()));

    ccsUpgradeClearValues (*list);
}

TEST_F (CCSSettingsUpgradeTestWithMockContext, TestClearValuesInListNonListTypeNotMatched)
{
    MockedSetting resetSettingIdentifier (SpawnSetting (CCS_SETTINGS_UPGRADE_TEST_MOCK_SETTING_NAME_ONE,
							TypeInt,
							DoNotAddSettingToPlugin));
    MockedSetting settingToReset (SpawnSetting (CCS_SETTINGS_UPGRADE_TEST_MOCK_SETTING_NAME_ONE,
						TypeInt));
    CCSSettingValue valueToReset;
    CCSSettingValue valueResetIdentifier;

    InitializeValueForSetting (valueToReset, Real (settingToReset));
    InitializeValueForSetting (valueResetIdentifier, Real (resetSettingIdentifier));

    valueToReset.value.asInt = 2;
    valueResetIdentifier.value.asInt = 7;

    CCSSettingListWrapperPtr list (constructSettingListWrapper (ccsSettingListAppend (NULL, Real (resetSettingIdentifier)),
								cci::Shallow));

    EXPECT_CALL (MockPlugin (), findSetting (Eq (CCS_SETTINGS_UPGRADE_TEST_MOCK_SETTING_NAME_ONE)))
	    .WillOnce (Return (Real (settingToReset)));
    EXPECT_CALL (Mock (resetSettingIdentifier), getParent ());
    EXPECT_CALL (Mock (resetSettingIdentifier), getName ());

    CCSSettingInfo info;

    info.forInt.max = 0;
    info.forInt.min = 10;

    /* ccsCheckValueEq needs to know the type and info about this type */
    EXPECT_CALL (Mock (settingToReset), getType ()).Times (AtLeast (1));
    EXPECT_CALL (Mock (resetSettingIdentifier), getType ()).Times (AtLeast (1));

    EXPECT_CALL (Mock (settingToReset), getInfo ()).WillRepeatedly (Return (&info));
    EXPECT_CALL (Mock (resetSettingIdentifier), getInfo ()).WillRepeatedly (Return (&info));

    EXPECT_CALL (Mock (resetSettingIdentifier), getValue ()).WillOnce (Return (&valueResetIdentifier));
    EXPECT_CALL (Mock (settingToReset), getValue ()).WillOnce (Return (&valueToReset));

    ccsUpgradeClearValues (*list);
}

TEST_F (CCSSettingsUpgradeTestWithMockContext, TestNoAddValuesSettingNotFound)
{
    MockedSetting settingOne (SpawnSetting (CCS_SETTINGS_UPGRADE_TEST_MOCK_SETTING_NAME_ONE,
					    TypeInt,
					    DoNotAddSettingToPlugin));

    EXPECT_CALL (MockPlugin (), findSetting (Eq (CCS_SETTINGS_UPGRADE_TEST_MOCK_SETTING_NAME_ONE)));
    EXPECT_CALL (Mock (settingOne), getParent ());
    EXPECT_CALL (Mock (settingOne), getName ());

    CCSSettingListWrapperPtr list (constructSettingListWrapper (ccsSettingListAppend (NULL, Real (settingOne)),
								cci::Shallow));

    ccsUpgradeAddValues (*list);
}

TEST_F (CCSSettingsUpgradeTestWithMockContext, TestAddValuesInListNonListType)
{
    MockedSetting addSettingIdentifier (SpawnSetting (CCS_SETTINGS_UPGRADE_TEST_MOCK_SETTING_NAME_ONE,
						      TypeInt,
						      DoNotAddSettingToPlugin));
    MockedSetting settingToBeChanged (SpawnSetting (CCS_SETTINGS_UPGRADE_TEST_MOCK_SETTING_NAME_ONE,
						    TypeInt));
    CCSSettingValue valueToReset;
    CCSSettingValue valueResetIdentifier;

    InitializeValueForSetting (valueToReset, Real (settingToBeChanged));
    InitializeValueForSetting (valueResetIdentifier, Real (addSettingIdentifier));

    valueToReset.value.asInt = 7;
    valueResetIdentifier.value.asInt = 7;

    CCSSettingListWrapperPtr list (constructSettingListWrapper (ccsSettingListAppend (NULL, Real (addSettingIdentifier)),
								cci::Shallow));

    EXPECT_CALL (MockPlugin (), findSetting (Eq (CCS_SETTINGS_UPGRADE_TEST_MOCK_SETTING_NAME_ONE)))
	    .WillOnce (Return (Real (settingToBeChanged)));
    EXPECT_CALL (Mock (addSettingIdentifier), getParent ());
    EXPECT_CALL (Mock (addSettingIdentifier), getName ());

    CCSSettingInfo info;

    info.forInt.max = 0;
    info.forInt.min = 10;

    /* ccsCheckValueEq needs to know the type and info about this type */
    EXPECT_CALL (Mock (settingToBeChanged), getType ()).Times (AtLeast (1));
    EXPECT_CALL (Mock (addSettingIdentifier), getType ()).Times (AtLeast (1));

    EXPECT_CALL (Mock (settingToBeChanged), getInfo ()).WillRepeatedly (Return (&info));
    EXPECT_CALL (Mock (addSettingIdentifier), getInfo ()).WillRepeatedly (Return (&info));

    EXPECT_CALL (Mock (addSettingIdentifier), getValue ()).WillOnce (Return (&valueResetIdentifier));

    EXPECT_CALL (Mock (settingToBeChanged), setValue (Pointee (SettingValueMatch (valueResetIdentifier,
										  TypeInt,
										  &info)), IsTrue ()));

    ccsUpgradeAddValues (*list);
}

TEST_F (CCSSettingsUpgradeTestWithMockContext, TestNoReplaceValuesSettingNotFound)
{
    MockedSetting fromSettingIdentifier (SpawnSetting (CCS_SETTINGS_UPGRADE_TEST_MOCK_SETTING_NAME_ONE,
						      TypeInt,
						      DoNotAddSettingToPlugin));
    MockedSetting toSettingIdentifier (SpawnSetting (CCS_SETTINGS_UPGRADE_TEST_MOCK_SETTING_NAME_ONE,
						      TypeInt,
						      DoNotAddSettingToPlugin));

    EXPECT_CALL (MockPlugin (), findSetting (Eq (CCS_SETTINGS_UPGRADE_TEST_MOCK_SETTING_NAME_ONE)));
    EXPECT_CALL (Mock (fromSettingIdentifier), getParent ());
    EXPECT_CALL (Mock (fromSettingIdentifier), getName ());

    CCSSettingListWrapperPtr replaceFromValueSettings (constructSettingListWrapper (ccsSettingListAppend (NULL, Real (fromSettingIdentifier)),
										    cci::Shallow));
    CCSSettingListWrapperPtr replaceToValueSettings (constructSettingListWrapper (ccsSettingListAppend (NULL, Real (toSettingIdentifier)),
										  cci::Shallow));


    ccsUpgradeReplaceValues (*replaceFromValueSettings,
			     *replaceToValueSettings);
}

TEST_F (CCSSettingsUpgradeTestWithMockContext, TestReplaceValuesInListNonListType)
{
    MockedSetting fromSettingIdentifier (SpawnSetting (CCS_SETTINGS_UPGRADE_TEST_MOCK_SETTING_NAME_ONE,
						      TypeInt,
						      DoNotAddSettingToPlugin));
    MockedSetting toSettingIdentifier (SpawnSetting (CCS_SETTINGS_UPGRADE_TEST_MOCK_SETTING_NAME_ONE,
						      TypeInt,
						      DoNotAddSettingToPlugin));
    MockedSetting settingToBeChanged (SpawnSetting (CCS_SETTINGS_UPGRADE_TEST_MOCK_SETTING_NAME_ONE,
						    TypeInt));
    CCSSettingValue settingToBeChangedValue;
    CCSSettingValue fromSettingIdentifierValue;
    CCSSettingValue toSettingIdentifierValue;

    InitializeValueForSetting (settingToBeChangedValue, Real (settingToBeChanged));
    InitializeValueForSetting (fromSettingIdentifierValue, Real (fromSettingIdentifier));
    InitializeValueForSetting (toSettingIdentifierValue, Real (toSettingIdentifier));

    settingToBeChangedValue.value.asInt = 7;
    fromSettingIdentifierValue.value.asInt = 7;
    toSettingIdentifierValue.value.asInt = 8;

    CCSSettingListWrapperPtr replaceFromValueSettings (constructSettingListWrapper (ccsSettingListAppend (NULL, Real (fromSettingIdentifier)),
										    cci::Shallow));
    CCSSettingListWrapperPtr replaceToValueSettings (constructSettingListWrapper (ccsSettingListAppend (NULL, Real (toSettingIdentifier)),
										  cci::Shallow));


    EXPECT_CALL (MockPlugin (), findSetting (Eq (CCS_SETTINGS_UPGRADE_TEST_MOCK_SETTING_NAME_ONE)))
	    .WillOnce (Return (Real (settingToBeChanged)));
    EXPECT_CALL (Mock (fromSettingIdentifier), getParent ());
    EXPECT_CALL (Mock (fromSettingIdentifier), getName ());
    EXPECT_CALL (Mock (toSettingIdentifier), getName ());

    CCSSettingInfo info;

    info.forInt.max = 0;
    info.forInt.min = 10;

    /* ccsCheckValueEq needs to know the type and info about this type */
    EXPECT_CALL (Mock (settingToBeChanged), getType ()).Times (AtLeast (1));
    EXPECT_CALL (Mock (fromSettingIdentifier), getType ()).Times (AtLeast (1));

    EXPECT_CALL (Mock (settingToBeChanged), getInfo ()).WillRepeatedly (Return (&info));
    EXPECT_CALL (Mock (fromSettingIdentifier), getInfo ()).WillRepeatedly (Return (&info));
    EXPECT_CALL (Mock (toSettingIdentifier), getInfo ()).WillRepeatedly (Return (&info));

    EXPECT_CALL (Mock (settingToBeChanged), getValue ()).WillOnce (Return (&settingToBeChangedValue));
    EXPECT_CALL (Mock (fromSettingIdentifier), getValue ()).WillOnce (Return (&fromSettingIdentifierValue));
    EXPECT_CALL (Mock (toSettingIdentifier), getValue ()).WillOnce (Return (&toSettingIdentifierValue));


    EXPECT_CALL (Mock (settingToBeChanged), setValue (Pointee (SettingValueMatch (toSettingIdentifierValue,
										  TypeInt,
										  &info)), IsTrue ()));

    ccsUpgradeReplaceValues (*replaceFromValueSettings,
			     *replaceToValueSettings);
}

TEST_F (CCSSettingsUpgradeTestWithMockContext, TestReplaceValuesInListNonListTypeNoMatch)
{
    MockedSetting fromSettingIdentifier (SpawnSetting (CCS_SETTINGS_UPGRADE_TEST_MOCK_SETTING_NAME_ONE,
						      TypeInt,
						      DoNotAddSettingToPlugin));
    MockedSetting toSettingIdentifier (SpawnSetting (CCS_SETTINGS_UPGRADE_TEST_MOCK_SETTING_NAME_ONE,
						      TypeInt,
						      DoNotAddSettingToPlugin));
    MockedSetting settingToBeChanged (SpawnSetting (CCS_SETTINGS_UPGRADE_TEST_MOCK_SETTING_NAME_ONE,
						    TypeInt));
    CCSSettingValue settingToBeChangedValue;
    CCSSettingValue fromSettingIdentifierValue;
    CCSSettingValue toSettingIdentifierValue;

    InitializeValueForSetting (settingToBeChangedValue, Real (settingToBeChanged));
    InitializeValueForSetting (fromSettingIdentifierValue, Real (fromSettingIdentifier));
    InitializeValueForSetting (toSettingIdentifierValue, Real (toSettingIdentifier));

    settingToBeChangedValue.value.asInt = 6;
    fromSettingIdentifierValue.value.asInt = 7;
    toSettingIdentifierValue.value.asInt = 8;

    CCSSettingListWrapperPtr replaceFromValueSettings (constructSettingListWrapper (ccsSettingListAppend (NULL, Real (fromSettingIdentifier)),
										    cci::Shallow));
    CCSSettingListWrapperPtr replaceToValueSettings (constructSettingListWrapper (ccsSettingListAppend (NULL, Real (toSettingIdentifier)),
										  cci::Shallow));

    EXPECT_CALL (MockPlugin (), findSetting (Eq (CCS_SETTINGS_UPGRADE_TEST_MOCK_SETTING_NAME_ONE)))
	    .WillOnce (Return (Real (settingToBeChanged)));
    EXPECT_CALL (Mock (fromSettingIdentifier), getParent ());
    EXPECT_CALL (Mock (fromSettingIdentifier), getName ());

    CCSSettingInfo info;

    info.forInt.max = 0;
    info.forInt.min = 10;

    /* ccsCheckValueEq needs to know the type and info about this type */
    EXPECT_CALL (Mock (settingToBeChanged), getType ()).Times (AtLeast (1));
    EXPECT_CALL (Mock (fromSettingIdentifier), getType ()).Times (AtLeast (1));

    EXPECT_CALL (Mock (settingToBeChanged), getInfo ()).WillRepeatedly (Return (&info));
    EXPECT_CALL (Mock (fromSettingIdentifier), getInfo ()).WillRepeatedly (Return (&info));
    EXPECT_CALL (Mock (toSettingIdentifier), getInfo ()).WillRepeatedly (Return (&info));

    EXPECT_CALL (Mock (settingToBeChanged), getValue ()).WillOnce (Return (&settingToBeChangedValue));
    EXPECT_CALL (Mock (fromSettingIdentifier), getValue ()).WillOnce (Return (&fromSettingIdentifierValue));

    ccsUpgradeReplaceValues (*replaceFromValueSettings,
			     *replaceToValueSettings);
}

namespace
{
    boost::shared_ptr <CCSString>
    newOwnedCCSStringFromStaticCharArray (const char *cStr)
    {
	CCSString		      *string = reinterpret_cast <CCSString *> (calloc (1, sizeof (CCSString)));
	boost::shared_ptr <CCSString> str (string,
					   ccsStringUnref);

	str->value = strdup (cStr);
	ccsStringRef (str.get ());
	return str;
    }

    void
    ccsStringListDeepFree (CCSStringList list)
    {
	ccsStringListFree (list, TRUE);
    }

    void
    ccsSettingValueListDeepFree (CCSSettingValueList list)
    {
	ccsSettingValueListFree (list, TRUE);
    }

    void
    ccsSettingValueListShallowFree (CCSSettingValueList list)
    {
	ccsSettingValueListFree (list, FALSE);
    }

    typedef boost::shared_ptr <cc::ListWrapper <CCSStringList, CCSString *> > CCSStringListWrapperPtr;

    CCSStringListWrapperPtr
    constructStrListWrapper (CCSStringList        list,
			     cci::ListStorageType storageType)
    {
	return boost::make_shared <cci::ListWrapper <CCSStringList, CCSString *> > (list,
										       ccsStringListFree,
										       ccsStringListAppend,
										       ccsStringListRemove,
										       storageType);
    }
}

TEST_F (CCSSettingsUpgradeTestWithMockContext, TestAddValuesInListAppendsValuesToListFromList)
{
    const std::string valueOne ("value_one");
    const std::string valueThree ("value_three");
    MockedSetting appendSettingIdentifier (SpawnSetting (CCS_SETTINGS_UPGRADE_TEST_MOCK_SETTING_NAME_ONE,
							TypeList,
							DoNotAddSettingToPlugin));
    MockedSetting settingToAppendValuesTo (SpawnSetting (CCS_SETTINGS_UPGRADE_TEST_MOCK_SETTING_NAME_ONE,
							   TypeList));

    boost::shared_ptr <CCSString> stringToAppendOne (newOwnedCCSStringFromStaticCharArray (valueOne.c_str ()));
    boost::shared_ptr <CCSString> stringInBothLists (newOwnedCCSStringFromStaticCharArray (valueThree.c_str ()));

    CCSStringListWrapperPtr settingsStrList (constructStrListWrapper (ccsStringListAppend (NULL, stringInBothLists.get ()),
								      cci::Shallow));


    boost::shared_ptr <_CCSSettingValueList> settingStrValueList (AutoDestroy (ccsGetValueListFromStringList (*settingsStrList,
													      Real (settingToAppendValuesTo)),
									       ccsSettingValueListDeepFree));

    CCSStringListWrapperPtr appendStrList (constructStrListWrapper (ccsStringListAppend (NULL, stringInBothLists.get ()),
								    cci::Shallow));
    appendStrList->append (stringToAppendOne.get ());

    boost::shared_ptr <_CCSSettingValueList> appendStrValueList (AutoDestroy (ccsGetValueListFromStringList (*appendStrList,
													     Real (appendSettingIdentifier)),
									      ccsSettingValueListDeepFree));


    CCSSettingListWrapperPtr list (constructSettingListWrapper (ccsSettingListAppend (NULL, Real (appendSettingIdentifier)),
								cci::Shallow));

    CCSSettingValue valueToHaveSubValuesAdded;
    CCSSettingValue valueSubValuesAddIdentifiers;

    InitializeValueForSetting (valueToHaveSubValuesAdded, Real (settingToAppendValuesTo));
    InitializeValueForSetting (valueSubValuesAddIdentifiers, Real (appendSettingIdentifier));

    valueToHaveSubValuesAdded.value.asList = settingStrValueList.get ();
    valueSubValuesAddIdentifiers.value.asList = appendStrValueList.get ();

    CCSSettingInfo info;

    info.forList.listType = TypeString;

    EXPECT_CALL (MockPlugin (), findSetting (Eq (CCS_SETTINGS_UPGRADE_TEST_MOCK_SETTING_NAME_ONE)))
	    .WillOnce (Return (Real (settingToAppendValuesTo)));
    EXPECT_CALL (Mock (appendSettingIdentifier), getParent ());
    EXPECT_CALL (Mock (appendSettingIdentifier), getName ());
    EXPECT_CALL (Mock (appendSettingIdentifier), getType ()).Times (AtLeast (1));
    EXPECT_CALL (Mock (settingToAppendValuesTo), getType ()).Times (AtLeast (1));
    EXPECT_CALL (Mock (appendSettingIdentifier), getValue ()).WillOnce (Return (&valueSubValuesAddIdentifiers));
    EXPECT_CALL (Mock (settingToAppendValuesTo), getValue ()).WillOnce (Return (&valueToHaveSubValuesAdded));
    EXPECT_CALL (Mock (settingToAppendValuesTo), getInfo ()).WillRepeatedly (Return (&info));
    EXPECT_CALL (Mock (appendSettingIdentifier), getInfo ()).WillRepeatedly (Return (&info));

    const CCSSettingValue &appendedStringInListValue = *appendStrValueList->next->data;

    EXPECT_CALL (Mock (settingToAppendValuesTo), setList (
		     IsSettingValueInSettingValueCCSList (
			    SettingValueMatch (appendedStringInListValue, TypeString, &info)), IsTrue ()));

    ccsUpgradeAddValues (*list);
}

TEST_F (CCSSettingsUpgradeTestWithMockContext, TestClearValuesInListRemovesValuesFromList)
{
    const std::string valueOne ("value_one");
    const std::string valueThree ("value_three");
    MockedSetting resetSettingIdentifier (SpawnSetting (CCS_SETTINGS_UPGRADE_TEST_MOCK_SETTING_NAME_ONE,
							TypeList,
							DoNotAddSettingToPlugin));
    MockedSetting settingToRemoveValuesFrom (SpawnSetting (CCS_SETTINGS_UPGRADE_TEST_MOCK_SETTING_NAME_ONE,
							   TypeList));

    boost::shared_ptr <CCSString> stringForRemovalOne (newOwnedCCSStringFromStaticCharArray (valueOne.c_str ()));
    boost::shared_ptr <CCSString> stringNotRemoved (newOwnedCCSStringFromStaticCharArray (valueThree.c_str ()));

    CCSStringListWrapperPtr settingsStrList (constructStrListWrapper (ccsStringListAppend (NULL, stringForRemovalOne.get ()),
								      cci::Shallow));
    settingsStrList->append (stringNotRemoved.get ());

    boost::shared_ptr <_CCSSettingValueList> settingStrValueList (AutoDestroy (ccsGetValueListFromStringList (*settingsStrList,
													      Real (settingToRemoveValuesFrom)),
									       ccsSettingValueListDeepFree));

    CCSStringListWrapperPtr removeStrList (constructStrListWrapper (ccsStringListAppend (NULL, stringForRemovalOne.get ()),
								    cci::Shallow));

    boost::shared_ptr <_CCSSettingValueList> removeStrValueList (AutoDestroy (ccsGetValueListFromStringList (*removeStrList,
													     Real (resetSettingIdentifier)),
									      ccsSettingValueListDeepFree));

    CCSSettingListWrapperPtr list (constructSettingListWrapper (ccsSettingListAppend (NULL, Real (resetSettingIdentifier)),
								cci::Shallow));

    CCSSettingValue valueToHaveSubValuesRemoved;
    CCSSettingValue valueSubValuesResetIdentifiers;

    InitializeValueForSetting (valueToHaveSubValuesRemoved, Real (settingToRemoveValuesFrom));
    InitializeValueForSetting (valueSubValuesResetIdentifiers, Real (resetSettingIdentifier));

    valueToHaveSubValuesRemoved.value.asList = settingStrValueList.get ();
    valueSubValuesResetIdentifiers.value.asList = removeStrValueList.get ();

    CCSSettingInfo info;

    info.forList.listType = TypeString;

    EXPECT_CALL (MockPlugin (), findSetting (Eq (CCS_SETTINGS_UPGRADE_TEST_MOCK_SETTING_NAME_ONE)))
	    .WillOnce (Return (Real (settingToRemoveValuesFrom)));
    EXPECT_CALL (Mock (resetSettingIdentifier), getParent ());
    EXPECT_CALL (Mock (resetSettingIdentifier), getName ());
    EXPECT_CALL (Mock (resetSettingIdentifier), getType ()).Times (AtLeast (1));
    EXPECT_CALL (Mock (settingToRemoveValuesFrom), getType ()).Times (AtLeast (1));
    EXPECT_CALL (Mock (resetSettingIdentifier), getValue ()).WillOnce (Return (&valueSubValuesResetIdentifiers));
    EXPECT_CALL (Mock (settingToRemoveValuesFrom), getValue ()).WillOnce (Return (&valueToHaveSubValuesRemoved));
    EXPECT_CALL (Mock (settingToRemoveValuesFrom), getInfo ()).WillRepeatedly (Return (&info));
    EXPECT_CALL (Mock (resetSettingIdentifier), getInfo ()).WillRepeatedly (Return (&info));

    const CCSSettingValue &removedStringInListValue = *removeStrValueList->data;

    EXPECT_CALL (Mock (settingToRemoveValuesFrom), setList (
		     Not (
			 IsSettingValueInSettingValueCCSList (
			    SettingValueMatch (removedStringInListValue, TypeString, &info))), IsTrue ()));

    ccsUpgradeClearValues (*list);
}
