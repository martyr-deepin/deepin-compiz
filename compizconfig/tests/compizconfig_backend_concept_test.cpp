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

#include <gtest_unspecified_bool_type_matcher.h>

#include <compizconfig_ccs_setting_mock.h>

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include <compizconfig_backend_concept_test_internal.h>
#include <compizconfig_backend_concept_test_environment_interface.h>

using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnNull;
using ::testing::SetArgPointee;
using ::testing::DoAll;
using ::testing::AtLeast;
using ::testing::Eq;


namespace cct = compizconfig::test;

Bool cct::boolToBool (bool v)
{
    return v ? TRUE : FALSE;
}

cci::SettingValueListWrapper::Ptr
cct::CCSListConstructionExpectationsSetter (const cct::ConstructorFunc &c,
					    CCSSettingType             type,
					    cci::ListStorageType       storageType)
{
    boost::function <void (CCSSetting *)> f (boost::bind (ccsSettingUnref, _1));
    boost::shared_ptr <CCSSetting> mockSetting (ccsNiceMockSettingNew (), f);
    NiceMock <CCSSettingGMock>     *gmockSetting = reinterpret_cast <NiceMock <CCSSettingGMock> *> (ccsObjectGetPrivate (mockSetting.get ()));

    ON_CALL (*gmockSetting, getType ()).WillByDefault (Return (TypeList));

    boost::shared_ptr <CCSSettingInfo> listInfo (new CCSSettingInfo);

    listInfo->forList.listType = type;

    ON_CALL (*gmockSetting, getInfo ()).WillByDefault (Return (listInfo.get ()));
    ON_CALL (*gmockSetting, getDefaultValue ()).WillByDefault (ReturnNull ());
    return boost::make_shared <cci::SettingValueListWrapper> (c (mockSetting.get ()), storageType, type, listInfo, mockSetting);
}

CCSSettingGMock *
cct::getSettingGMockFromSetting (const boost::shared_ptr <CCSSetting> &setting)
{
    return reinterpret_cast <CCSSettingGMock *> (ccsObjectGetPrivate (setting.get ()));
}

void
cct::SetIntWriteExpectation (const std::string                                    &plugin,
			     const std::string                                    &key,
			     const VariantTypes                                   &value,
			     const boost::shared_ptr <CCSSetting>                 &setting,
			     const WriteFunc                                      &write,
			     const CCSBackendConceptTestEnvironmentInterface::Ptr &env)
{
    CCSSettingGMock *gmock (getSettingGMockFromSetting (setting));
    EXPECT_CALL (*gmock, getInt (_)).WillRepeatedly (DoAll (
							 SetArgPointee <0> (
							     boost::get <int> (value)),
							 Return (TRUE)));
    write ();
    EXPECT_EQ (env->ReadIntegerAtKey (plugin, key), boost::get <int> (value));
}

void
cct::SetBoolWriteExpectation (const std::string                                    &plugin,
			      const std::string                                    &key,
			      const VariantTypes                                   &value,
			      const boost::shared_ptr <CCSSetting>                 &setting,
			      const WriteFunc                                      &write,
			      const CCSBackendConceptTestEnvironmentInterface::Ptr &env)
{
    CCSSettingGMock *gmock (getSettingGMockFromSetting (setting));
    EXPECT_CALL (*gmock, getBool (_)).WillRepeatedly (DoAll (
							 SetArgPointee <0> (
							     boolToBool (boost::get <bool> (value))),
							 Return (TRUE)));
    write ();

    bool v (boost::get <bool> (value));

    if (v)
	EXPECT_THAT (env->ReadBoolAtKey (plugin, key), IsTrue ());
    else
	EXPECT_THAT (env->ReadBoolAtKey (plugin, key), IsFalse ());
}

void
cct::SetFloatWriteExpectation (const std::string                                    &plugin,
			       const std::string                                    &key,
			       const VariantTypes                                   &value,
			       const boost::shared_ptr <CCSSetting>                 &setting,
			       const WriteFunc                                      &write,
			       const CCSBackendConceptTestEnvironmentInterface::Ptr &env)
{
    CCSSettingGMock *gmock (getSettingGMockFromSetting (setting));
    EXPECT_CALL (*gmock, getFloat (_)).WillRepeatedly (DoAll (
							 SetArgPointee <0> (
							     boost::get <float> (value)),
							 Return (TRUE)));
    write ();
    EXPECT_EQ (env->ReadFloatAtKey (plugin, key), boost::get <float> (value));
}

void
cct::SetStringWriteExpectation (const std::string                                    &plugin,
				const std::string                                    &key,
				const VariantTypes                                   &value,
				const boost::shared_ptr <CCSSetting>                 &setting,
				const WriteFunc                                      &write,
				const CCSBackendConceptTestEnvironmentInterface::Ptr &env)
{
    CCSSettingGMock *gmock (getSettingGMockFromSetting (setting));
    EXPECT_CALL (*gmock, getString (_)).WillRepeatedly (DoAll (
							 SetArgPointee <0> (
							     const_cast <char *> (boost::get <const char *> (value))),
							 Return (TRUE)));
    write ();
    EXPECT_EQ (std::string (env->ReadStringAtKey (plugin, key)), std::string (boost::get <const char *> (value)));
}

void
cct::SetColorWriteExpectation (const std::string                                    &plugin,
			       const std::string                                    &key,
			       const VariantTypes                                   &value,
			       const boost::shared_ptr <CCSSetting>                 &setting,
			       const WriteFunc                                      &write,
			       const CCSBackendConceptTestEnvironmentInterface::Ptr &env)
{
    CCSSettingGMock *gmock (getSettingGMockFromSetting (setting));
    EXPECT_CALL (*gmock, getColor (_)).WillRepeatedly (DoAll (
							 SetArgPointee <0> (
							     boost::get <CCSSettingColorValue> (value)),
							 Return (TRUE)));
    write ();

    EXPECT_EQ (env->ReadColorAtKey (plugin, key), boost::get <CCSSettingColorValue> (value));
}

void
cct::SetKeyWriteExpectation (const std::string                                    &plugin,
			     const std::string                                    &key,
			     const VariantTypes                                   &value,
			     const boost::shared_ptr <CCSSetting>                 &setting,
			     const WriteFunc                                      &write,
			     const CCSBackendConceptTestEnvironmentInterface::Ptr &env)
{
    CCSSettingGMock *gmock (getSettingGMockFromSetting (setting));
    EXPECT_CALL (*gmock, getKey (_)).WillRepeatedly (DoAll (
							 SetArgPointee <0> (
							     boost::get <CCSSettingKeyValue> (value)),
							 Return (TRUE)));
    write ();
    EXPECT_EQ (env->ReadKeyAtKey (plugin, key), boost::get <CCSSettingKeyValue> (value));
}

void
cct::SetButtonWriteExpectation (const std::string                                    &plugin,
				const std::string                                    &key,
				const VariantTypes                                   &value,
				const boost::shared_ptr <CCSSetting>                 &setting,
				const WriteFunc                                      &write,
				const CCSBackendConceptTestEnvironmentInterface::Ptr &env)
{
    CCSSettingGMock *gmock (getSettingGMockFromSetting (setting));
    EXPECT_CALL (*gmock, getButton (_)).WillRepeatedly (DoAll (
							 SetArgPointee <0> (
							     boost::get <CCSSettingButtonValue> (value)),
							 Return (TRUE)));
    write ();
    EXPECT_EQ (env->ReadButtonAtKey (plugin, key), boost::get <CCSSettingButtonValue> (value));
}

void
cct::SetEdgeWriteExpectation (const std::string                                    &plugin,
			      const std::string                                    &key,
			      const VariantTypes                                   &value,
			      const boost::shared_ptr <CCSSetting>                 &setting,
			      const WriteFunc                                      &write,
			      const CCSBackendConceptTestEnvironmentInterface::Ptr &env)
{
    CCSSettingGMock *gmock (getSettingGMockFromSetting (setting));
    EXPECT_CALL (*gmock, getEdge (_)).WillRepeatedly (DoAll (
							 SetArgPointee <0> (
							     boost::get <unsigned int> (value)),
							 Return (TRUE)));
    write ();
    EXPECT_EQ (env->ReadEdgeAtKey (plugin, key), boost::get <unsigned int> (value));
}

void
cct::SetBellWriteExpectation (const std::string                                    &plugin,
			      const std::string                                    &key,
			      const VariantTypes                                   &value,
			      const boost::shared_ptr <CCSSetting>                 &setting,
			      const WriteFunc                                      &write,
			      const CCSBackendConceptTestEnvironmentInterface::Ptr &env)
{
    CCSSettingGMock *gmock (getSettingGMockFromSetting (setting));
    EXPECT_CALL (*gmock, getBell (_)).WillRepeatedly (DoAll (
							 SetArgPointee <0> (
							     boolToBool (boost::get <bool> (value))),
							 Return (TRUE)));
    write ();
    bool v (boost::get <bool> (value));

    if (v)
	EXPECT_THAT (env->ReadBellAtKey (plugin, key), IsTrue ());
    else
	EXPECT_THAT (env->ReadBellAtKey (plugin, key), IsFalse ());
}

void
cct::SetMatchWriteExpectation (const std::string                                    &plugin,
			       const std::string                                    &key,
			       const VariantTypes                                   &value,
			       const boost::shared_ptr <CCSSetting>                 &setting,
			       const WriteFunc                                      &write,
			       const CCSBackendConceptTestEnvironmentInterface::Ptr &env)
{
    CCSSettingGMock *gmock (getSettingGMockFromSetting (setting));
    EXPECT_CALL (*gmock, getMatch (_)).WillRepeatedly (DoAll (
							 SetArgPointee <0> (
							     const_cast <char *> (
								   boost::get <const char *> (
								       value))),
							 Return (TRUE)));
    write ();
    EXPECT_EQ (std::string (env->ReadMatchAtKey (plugin, key)),
	       std::string (boost::get <const char *> (value)));
}

void
cct::SetListWriteExpectation (const std::string                                    &plugin,
			      const std::string                                    &key,
			      const VariantTypes                                   &value,
			      const boost::shared_ptr <CCSSetting>                 &setting,
			      const WriteFunc                                      &write,
			      const CCSBackendConceptTestEnvironmentInterface::Ptr &env)
{
    CCSSettingGMock *gmock (getSettingGMockFromSetting (setting));
    CCSSettingValueList list = *(boost::get <boost::shared_ptr <cci::SettingValueListWrapper> > (value));

    EXPECT_CALL (*gmock, getInfo ());

    CCSSettingInfo      *info = ccsSettingGetInfo (setting.get ());
    info->forList.listType =
	    (boost::get <boost::shared_ptr <cci::SettingValueListWrapper> > (value))->type ();

    EXPECT_CALL (*gmock, getInfo ()).Times (AtLeast (1));
    EXPECT_CALL (*gmock, getList (_)).WillRepeatedly (DoAll (
							 SetArgPointee <0> (
							     list),
							 Return (TRUE)));
    write ();

    EXPECT_THAT (cci::SettingValueListWrapper (env->ReadListAtKey (plugin, key, setting.get ()),
						  cci::Deep,
						  info->forList.listType,
						  boost::shared_ptr <CCSSettingInfo> (),
						  setting),
		 ListEqual (&info->forList, list));
}

void
cct::SetIntReadExpectation (CCSSettingGMock *gmock, const VariantTypes &value)
{
    EXPECT_CALL (*gmock, setInt (boost::get <int> (value), _));
}

void
cct::SetBoolReadExpectation (CCSSettingGMock *gmock, const VariantTypes &value)
{
    bool v (boost::get <bool> (value));

    if (v)
	EXPECT_CALL (*gmock, setBool (IsTrue (), _));
    else
	EXPECT_CALL (*gmock, setBool (IsFalse (), _));
}

void
cct::SetBellReadExpectation (CCSSettingGMock *gmock, const VariantTypes &value)
{
    bool v (boost::get <bool> (value));

    if (v)
	EXPECT_CALL (*gmock, setBell (IsTrue (), _));
    else
	EXPECT_CALL (*gmock, setBell (IsFalse (), _));
}

void
cct::SetFloatReadExpectation (CCSSettingGMock *gmock, const VariantTypes &value)
{
    EXPECT_CALL (*gmock, setFloat (boost::get <float> (value), _));
}

void
cct::SetStringReadExpectation (CCSSettingGMock *gmock, const VariantTypes &value)
{
    EXPECT_CALL (*gmock, setString (Eq (std::string (boost::get <const char *> (value))), _));
}

void
cct::SetMatchReadExpectation (CCSSettingGMock *gmock, const VariantTypes &value)
{
    EXPECT_CALL (*gmock, setMatch (Eq (std::string (boost::get <const char *> (value))), _));
}

void
cct::SetColorReadExpectation (CCSSettingGMock *gmock, const VariantTypes &value)
{
    EXPECT_CALL (*gmock, setColor (boost::get <CCSSettingColorValue> (value), _));
}

void
cct::SetKeyReadExpectation (CCSSettingGMock *gmock, const VariantTypes &value)
{
    EXPECT_CALL (*gmock, setKey (boost::get <CCSSettingKeyValue> (value), _));
}

void
cct::SetButtonReadExpectation (CCSSettingGMock *gmock, const VariantTypes &value)
{
    EXPECT_CALL (*gmock, setButton (boost::get <CCSSettingButtonValue> (value), _));
}

void
cct::SetEdgeReadExpectation (CCSSettingGMock *gmock, const VariantTypes &value)
{
    EXPECT_CALL (*gmock, setEdge (boost::get <unsigned int> (value), _));
}

void
cct::SetListReadExpectation (CCSSettingGMock *gmock, const VariantTypes &value)
{
    static CCSSettingInfo globalListInfo;

    globalListInfo.forList.listType =
	    (boost::get <boost::shared_ptr <cci::SettingValueListWrapper> > (value))->type ();
    globalListInfo.forList.listInfo = NULL;

    ON_CALL (*gmock, getInfo ()).WillByDefault (Return (&globalListInfo));
    EXPECT_CALL (*gmock, setList (
			    ListEqual (
				&globalListInfo.forList,
				*(boost::get <boost::shared_ptr <cci::SettingValueListWrapper> > (value))), _));
}
