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

#include <map>
#include <memory>
#include <tr1/tuple>
#include <boost/shared_ptr.hpp>

#include <glib_gslice_off_env.h>
#include <gtest_shared_autodestroy.h>

#include <ccs.h>
#include <ccs-backend.h>
#include <ccs_gnome_integrated_setting.h>
#include <ccs_gnome_integration_gsettings_integrated_setting.h>
#include <ccs_gsettings_wrapper_mock.h>

using ::testing::Combine;
using ::testing::Return;
using ::testing::IsNull;
using ::testing::ValuesIn;
using ::testing::Values;
using ::testing::Eq;
using ::testing::WithArgs;
using ::testing::_;

namespace compiz
{
    namespace config
    {
	namespace integration
	{
	    namespace test
	    {
		const std::string KEYBINDING_ONE = "keybinding_one";
		const std::string KEYBINDING_TWO = "keybinding_two";
		const std::string STRING = "string";
		const Bool        BOOLEAN = TRUE;
		const int         INTEGER = 2;

		const std::string STRING_ALT = "string_alt";
		const Bool        BOOLEAN_ALT = FALSE;
		const int         INTEGER_ALT = 1;

		namespace variant_generators
		{
		    GVariant * i ();
		    GVariant * s ();
		    GVariant * b ();
		    GVariant * as ();
		    GVariant * fromValue (CCSSettingValue *v, CCSSettingType type);
		}

		namespace value_generators
		{
		    CCSSettingValue * integer ();
		    CCSSettingValue * string ();
		    CCSSettingValue * key ();
		    CCSSettingValue * boolean ();
		}

		namespace expectations
		{
		    void integer (CCSSettingValue *);
		    void string  (CCSSettingValue *);
		    void boolean (CCSSettingValue *);
		    void key     (CCSSettingValue *);

		    void integerVariant (GVariant *, int);
		    void stringVariant (GVariant *, const std::string &);
		    void booleanVariant (GVariant *, bool);
		    void keyVariant (GVariant *, const std::string &);
		}

		typedef GVariant * (*VariantGenerator) ();
		typedef CCSSettingValue * (*ValueGenerator) ();
		typedef void (*Expectation) (CCSSettingValue *);

		struct GSettingsIntegratedSettingInfo
		{
		    VariantGenerator variantGenerator;
		    ValueGenerator   valueGenerator;
		    Expectation      expectation;
		    CCSSettingType   settingType;
		    CCSSettingType   returnType;
		};

		namespace impl
		{
		    namespace ccit = compiz::config::integration::test;
		    namespace vg = compiz::config::integration::test::variant_generators;
		    namespace cvg = compiz::config::integration::test::value_generators;
		    namespace ex = compiz::config::integration::test::expectations;

		    ccit::GSettingsIntegratedSettingInfo settingsInfo[] =
		    {
			{ vg::i, cvg::integer, ex::integer, TypeInt, TypeInt },
			{ vg::b, cvg::boolean, ex::boolean, TypeBool, TypeBool },
			{ vg::s, cvg::string, ex::string, TypeString, TypeString },
			{ vg::as, cvg::key, ex::key, TypeKey, TypeString }
		    };
		}
	    }
	}
    }
}

MATCHER_P (VariantEqual, lhs, "Variants Equal")
{
    return g_variant_equal (lhs, arg);
}

namespace
{
    std::map <CCSSettingType, SpecialOptionType> &
    ccsTypeToSpecialType ()
    {
	static std::map <CCSSettingType, SpecialOptionType> types;
	static bool initialized = false;

	if (!initialized)
	{
	    types[TypeInt] = OptionInt;
	    types[TypeBool] = OptionBool;
	    types[TypeString] = OptionString;
	    types[TypeKey] = OptionKey;
	}

	return types;
    }
}

namespace ccit = compiz::config::integration::test;
namespace cciti = compiz::config::integration::test::impl;
namespace ccvg = compiz::config::integration::test::variant_generators;
namespace ccvalg = compiz::config::integration::test::value_generators;
namespace ccex = compiz::config::integration::test::expectations;

typedef std::tr1::tuple <CCSSettingType,
			 ccit::GSettingsIntegratedSettingInfo> CCSGSettingsIntegratedSettingTestInfo;

class CCSGSettingsIntegratedSettingTest :
    public ::testing::TestWithParam <CCSGSettingsIntegratedSettingTestInfo>
{
    public:

	virtual void SetUp ();
	virtual void TearDown ();

    protected:

	CompizGLibGSliceOffEnv                  env;
	boost::shared_ptr <CCSGSettingsWrapper> mWrapper;
	CCSGSettingsWrapperGMock                *mWrapperMock;
};

GVariant *
ccvg::fromValue (CCSSettingValue *v,
		 CCSSettingType  type)
{
    switch (type)
    {
	case TypeInt:
	    return g_variant_new ("i", v->value.asInt);
	    break;
	case TypeBool:
	    return g_variant_new ("b", v->value.asBool);
	    break;
	case TypeString:
	    return g_variant_new ("s", v->value.asString);
	    break;
	case TypeKey:
	{
	    GVariantBuilder builder;
	    g_variant_builder_init (&builder, G_VARIANT_TYPE ("as"));

	    /* Represented internally as strings */
	    std::string kb (v->value.asString);
	    if (kb == "Disabled")
		kb[0] = 'd';

	    g_variant_builder_add (&builder, "s", kb.c_str ());
	    return g_variant_builder_end (&builder);
	}
	default:
	    break;
    }

    return NULL;
}

GVariant *
ccvg::as ()
{
    GVariantBuilder builder;
    g_variant_builder_init (&builder, G_VARIANT_TYPE ("as"));
    g_variant_builder_add (&builder, "s", ccit::KEYBINDING_ONE.c_str ());
    g_variant_builder_add (&builder, "s", ccit::KEYBINDING_TWO.c_str ());

    return g_variant_builder_end (&builder);
}

GVariant *
ccvg::i ()
{
    return g_variant_new ("i", ccit::INTEGER);
}

GVariant *
ccvg::b ()
{
    return g_variant_new ("b", ccit::BOOLEAN);
}

GVariant *
ccvg::s ()
{
    return g_variant_new ("s", ccit::STRING.c_str ());
}

namespace
{
    CCSSettingValue * createSettingValue ()
    {
	CCSSettingValue *v = reinterpret_cast <CCSSettingValue *> (calloc (1, sizeof (CCSSettingValue)));

	v->isListChild = FALSE;
	v->parent = NULL;
	v->refCount = 1;

	return v;
    }
}

CCSSettingValue *
ccvalg::integer ()
{
    CCSSettingValue *v = createSettingValue ();
    v->value.asInt = ccit::INTEGER_ALT;
    return v;
}

CCSSettingValue *
ccvalg::string ()
{
    CCSSettingValue *v = createSettingValue ();
    v->value.asString = strdup (ccit::STRING_ALT.c_str ());
    return v;
}

CCSSettingValue *
ccvalg::key ()
{
    CCSSettingValue *v = createSettingValue ();
    v->value.asString = strdup (ccit::KEYBINDING_TWO.c_str ());
    return v;
}

CCSSettingValue *
ccvalg::boolean ()
{
    CCSSettingValue *v = createSettingValue ();
    v->value.asBool = ccit::BOOLEAN_ALT;
    return v;
}

void
ccex::boolean (CCSSettingValue *v)
{
    EXPECT_EQ (v->value.asBool, ccit::BOOLEAN);
}

void
ccex::integer (CCSSettingValue *v)
{
    EXPECT_EQ (v->value.asInt, ccit::INTEGER);
}

void
ccex::string (CCSSettingValue *v)
{
    EXPECT_EQ (v->value.asString, ccit::STRING);
}

void
ccex::key (CCSSettingValue *v)
{
    EXPECT_EQ (v->value.asString, ccit::KEYBINDING_ONE);
}

void
ccex::integerVariant (GVariant *v , int i)
{
    EXPECT_EQ (g_variant_get_int32 (v), i);
}

void
ccex::stringVariant (GVariant *v, const std::string &s)
{
    gsize len;
    EXPECT_EQ (g_variant_get_string (v, &len), s);
}

void
ccex::booleanVariant (GVariant *v, bool b)
{
    EXPECT_EQ (g_variant_get_boolean (v), b);
}

void
ccex::keyVariant (GVariant *v, const std::string &s)
{
    gsize len;
    const gchar * const *strv = g_variant_get_strv (v, &len);
    EXPECT_EQ (strv[0], s);
}

void
CCSGSettingsIntegratedSettingTest::SetUp ()
{
    env.SetUpEnv ();
    mWrapper.reset (ccsMockGSettingsWrapperNew (),
		    boost::bind (ccsMockGSettingsWrapperFree, _1));
    mWrapperMock = reinterpret_cast <CCSGSettingsWrapperGMock *> (
		       ccsObjectGetPrivate (mWrapper.get ()));
}

void
CCSGSettingsIntegratedSettingTest::TearDown ()
{
    mWrapper.reset ();
    mWrapperMock = NULL;
    env.TearDownEnv ();
}

TEST_P (CCSGSettingsIntegratedSettingTest, MatchedTypesReturnValueMismatchedTypesReturnNull)
{
    const std::string keyName ("mock");
    const ccit::GSettingsIntegratedSettingInfo &integratedSettingInfo =
	    std::tr1::get <1> (GetParam ());
    const CCSSettingType                       createSettingType =
	    std::tr1::get <0> (GetParam ());

    /* The GSettings Integrated setting takes ownership of these */
    CCSIntegratedSettingInfo *integratedSetting = ccsSharedIntegratedSettingInfoNew (keyName.c_str (),
										     keyName.c_str (),
										     integratedSettingInfo.settingType,
										     &ccsDefaultObjectAllocator);
    SpecialOptionType             specialType = ccsTypeToSpecialType ()[integratedSettingInfo.settingType];
    CCSGNOMEIntegratedSettingInfo *gnomeIntegratedSetting = ccsGNOMEIntegratedSettingInfoNew (integratedSetting,
											      specialType,
											      keyName.c_str (),
											      &ccsDefaultObjectAllocator);
    boost::shared_ptr <CCSIntegratedSetting> gsettingsIntegrated (AutoDestroy (ccsGSettingsIntegratedSettingNew (gnomeIntegratedSetting,
														 mWrapper.get (),
														 &ccsDefaultObjectAllocator),
									       ccsIntegratedSettingUnref));

    GVariant *variant = (*integratedSettingInfo.variantGenerator) ();
    EXPECT_CALL (*mWrapperMock, getValue (Eq (keyName))).WillOnce (Return (variant));

    CCSSettingValue *value = ccsIntegratedSettingReadValue (gsettingsIntegrated.get (), createSettingType);

    if (createSettingType == integratedSettingInfo.settingType)
	(*integratedSettingInfo.expectation) (value);
    else
	EXPECT_THAT (value, IsNull ());

    if (value)
	ccsFreeSettingValueWithType (value, integratedSettingInfo.returnType);
}

ACTION (FreeVariant)
{
    g_variant_unref (arg0);
}

TEST_P (CCSGSettingsIntegratedSettingTest, MatchedTypesReturnValueMismatchedTypesResetOrWrite)
{
    const std::string keyName ("mock");
    const ccit::GSettingsIntegratedSettingInfo &integratedSettingInfo =
	    std::tr1::get <1> (GetParam ());
    const CCSSettingType                       createSettingType =
	    std::tr1::get <0> (GetParam ());

    CCSIntegratedSettingInfo *integratedSetting = ccsSharedIntegratedSettingInfoNew (keyName.c_str (),
										     keyName.c_str (),
										     integratedSettingInfo.settingType,
										     &ccsDefaultObjectAllocator);
    SpecialOptionType             specialType = ccsTypeToSpecialType ()[integratedSettingInfo.settingType];
    CCSGNOMEIntegratedSettingInfo *gnomeIntegratedSetting = ccsGNOMEIntegratedSettingInfoNew (integratedSetting,
											      specialType,
											      keyName.c_str (),
											      &ccsDefaultObjectAllocator);
    boost::shared_ptr <CCSIntegratedSetting> gsettingsIntegrated (AutoDestroy (ccsGSettingsIntegratedSettingNew (gnomeIntegratedSetting,
														 mWrapper.get (),
														 &ccsDefaultObjectAllocator),
									       ccsIntegratedSettingUnref));

    boost::shared_ptr <CCSSettingValue> value ((*integratedSettingInfo.valueGenerator) (),
					       boost::bind (ccsFreeSettingValueWithType,
							    _1,
							    integratedSettingInfo.returnType));
    boost::shared_ptr <GVariant>        variant = AutoDestroy (g_variant_ref ((*integratedSettingInfo.variantGenerator) ()),
							       g_variant_unref);
    boost::shared_ptr <GVariant>        newVariant = AutoDestroy (ccvg::fromValue (value.get (),
										   integratedSettingInfo.settingType),
								  g_variant_unref);
    EXPECT_CALL (*mWrapperMock, getValue (Eq (keyName))).WillOnce (Return (variant.get ()));

    if (createSettingType == integratedSettingInfo.settingType)
	EXPECT_CALL (*mWrapperMock, setValue (Eq (keyName), VariantEqual (newVariant.get ())))
		.WillOnce (WithArgs <1> (FreeVariant ()));
    else
	EXPECT_CALL (*mWrapperMock, resetKey (Eq (keyName)));

    ccsIntegratedSettingWriteValue (gsettingsIntegrated.get (), value.get (), createSettingType);
}

INSTANTIATE_TEST_CASE_P (CCSGSettingsIntegratedSettingTestMismatchedValues, CCSGSettingsIntegratedSettingTest,
			 Combine (Values (TypeInt, TypeString, TypeBool, TypeKey),
				  ValuesIn (cciti::settingsInfo)));
