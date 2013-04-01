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
#include <gsettings_util.h>
#include <ccs_gsettings_backend_interface.h>

#include <gtest_shared_autodestroy.h>
#include <gtest_shared_characterwrapper.h>
#include "test_gsettings_tests.h"

#include <glib_gslice_off_env.h>
#include <glib_gsettings_memory_backend_env.h>

#include "compizconfig_ccs_gsettings_settings_env_test.h"

#include "backend-conformance-config.h"
#include "gsettings-mock-schemas-config.h"

namespace cci = compiz::config::impl;
namespace cct = compizconfig::test;

class PrivateCCSGSettingsStorageEnv
{
    public:

	PrivateCCSGSettingsStorageEnv (CCSGSettingsWrapper *settings,
				       const std::string   &profileName);

	CCSGSettingsWrapper                        *mSettings;
	std::string	                           profileName;
};

void
CCSGSettingsStorageEnv::WriteBoolAtKey (const std::string  &plugin,
					const std::string  &key,
					const VariantTypes &value)
{
    GVariant *variant = NULL;
    if (writeBoolToVariant (cct::boolToBool (boost::get <bool> (value)), &variant))
	writeVariantToKey (priv->mSettings, CharacterWrapper (translateKeyForGSettings (key.c_str ())), variant);
    else
	throw std::exception ();
}

void
CCSGSettingsStorageEnv::WriteIntegerAtKey (const std::string  &plugin,
					   const std::string  &key,
					   const VariantTypes &value)
{
    GVariant *variant = NULL;
    if (writeIntToVariant (boost::get <int> (value), &variant))
	writeVariantToKey (priv->mSettings, CharacterWrapper (translateKeyForGSettings (key.c_str ())), variant);
    else
	throw std::exception ();
}

void
CCSGSettingsStorageEnv::WriteFloatAtKey (const std::string  &plugin,
					 const std::string  &key,
					 const VariantTypes &value)
{
    GVariant *variant = NULL;
    if (writeFloatToVariant (boost::get <float> (value), &variant))
	writeVariantToKey (priv->mSettings, CharacterWrapper (translateKeyForGSettings (key.c_str ())), variant);
    else
	throw std::exception ();
}

void
CCSGSettingsStorageEnv::WriteStringAtKey (const std::string  &plugin,
					  const std::string  &key,
					  const VariantTypes &value)
{
    GVariant *variant = NULL;
    if (writeStringToVariant (boost::get <const char *> (value), &variant))
	writeVariantToKey (priv->mSettings, CharacterWrapper (translateKeyForGSettings (key.c_str ())), variant);
    else
	throw std::exception ();
}

void
CCSGSettingsStorageEnv::WriteColorAtKey (const std::string  &plugin,
					 const std::string  &key,
					 const VariantTypes &value)
{
    GVariant *variant = NULL;
    if (writeColorToVariant (boost::get <CCSSettingColorValue> (value), &variant))
	writeVariantToKey (priv->mSettings, CharacterWrapper (translateKeyForGSettings (key.c_str ())), variant);
    else
	throw std::exception ();
}

void
CCSGSettingsStorageEnv::WriteKeyAtKey (const std::string  &plugin,
				       const std::string  &key,
				       const VariantTypes &value)
{
    GVariant *variant = NULL;
    if (writeKeyToVariant (boost::get <CCSSettingKeyValue> (value), &variant))
	writeVariantToKey (priv->mSettings, CharacterWrapper (translateKeyForGSettings (key.c_str ())), variant);
    else
	throw std::exception ();
}

void
CCSGSettingsStorageEnv::WriteButtonAtKey (const std::string  &plugin,
					  const std::string  &key,
					  const VariantTypes &value)
{
    GVariant *variant = NULL;
    if (writeButtonToVariant (boost::get <CCSSettingButtonValue> (value), &variant))
	writeVariantToKey (priv->mSettings, CharacterWrapper (translateKeyForGSettings (key.c_str ())), variant);
    else
	throw std::exception ();
}

void
CCSGSettingsStorageEnv::WriteEdgeAtKey (const std::string  &plugin,
					const std::string  &key,
					const VariantTypes &value)
{
    GVariant *variant = NULL;
    if (writeEdgeToVariant (boost::get <unsigned int> (value), &variant))
	writeVariantToKey (priv->mSettings, CharacterWrapper (translateKeyForGSettings (key.c_str ())), variant);
    else
	throw std::exception ();
}

void
CCSGSettingsStorageEnv::WriteMatchAtKey (const std::string  &plugin,
					const std::string  &key,
					const VariantTypes &value)
{
    GVariant *variant = NULL;
    if (writeStringToVariant (boost::get <const char *> (value), &variant))
	writeVariantToKey (priv->mSettings, CharacterWrapper (translateKeyForGSettings (key.c_str ())), variant);
    else
	throw std::exception ();
}

void
CCSGSettingsStorageEnv::WriteBellAtKey (const std::string  &plugin,
					const std::string  &key,
					const VariantTypes &value)
{
    GVariant *variant = NULL;
    if (writeBoolToVariant (cct::boolToBool (boost::get <bool> (value)), &variant))
	writeVariantToKey (priv->mSettings, CharacterWrapper (translateKeyForGSettings (key.c_str ())), variant);
    else
	throw std::exception ();
}

void
CCSGSettingsStorageEnv::WriteListAtKey (const std::string  &plugin,
					const std::string  &key,
					const VariantTypes &value)
{
    GVariant *variant = NULL;

    const cci::SettingValueListWrapper::Ptr &lw (boost::get <cci::SettingValueListWrapper::Ptr> (value));

    if (writeListValue (*lw, lw->type (), &variant))
	writeVariantToKey (priv->mSettings, CharacterWrapper (translateKeyForGSettings (key.c_str ())), variant);
    else
	throw std::exception ();
}

Bool
CCSGSettingsStorageEnv::ReadBoolAtKey (const std::string &plugin,
				       const std::string &key)
{
    GVariantShared variant (ReadVariantAtKeyToShared (plugin,
						      key,
						      TypeBool));
    return readBoolFromVariant (variant.get ());
}


int
CCSGSettingsStorageEnv::ReadIntegerAtKey (const std::string &plugin,
					  const std::string &key)
{
    GVariantShared variant (ReadVariantAtKeyToShared (plugin,
						      key,
						      TypeInt));
    return readIntFromVariant (variant.get ());
}

float
CCSGSettingsStorageEnv::ReadFloatAtKey (const std::string &plugin,
					const std::string &key)
{
    GVariantShared variant (ReadVariantAtKeyToShared (plugin,
						      key,
						      TypeFloat));
    return readFloatFromVariant (variant.get ());
}

const char *
CCSGSettingsStorageEnv::ReadStringAtKey (const std::string &plugin,
					 const std::string &key)
{
    GVariantShared variant (ReadVariantAtKeyToShared (plugin,
						      key,
						      TypeString));
    return readStringFromVariant (variant.get ());
}

CCSSettingColorValue
CCSGSettingsStorageEnv::ReadColorAtKey (const std::string &plugin,
					const std::string &key)
{
    Bool success = FALSE;
    GVariantShared variant (ReadVariantAtKeyToShared (plugin,
						      key,
						      TypeColor));
    CCSSettingColorValue value = readColorFromVariant (variant.get (), &success);
    EXPECT_TRUE (success);
    return value;
}

CCSSettingKeyValue
CCSGSettingsStorageEnv::ReadKeyAtKey (const std::string &plugin,
				      const std::string &key)
{
    Bool success = FALSE;
    GVariantShared variant (ReadVariantAtKeyToShared (plugin,
						      key,
						      TypeKey));
    CCSSettingKeyValue value = readKeyFromVariant (variant.get (), &success);
    EXPECT_TRUE (success);
    return value;
}

CCSSettingButtonValue
CCSGSettingsStorageEnv::ReadButtonAtKey (const std::string &plugin,
					  const std::string &key)
{
    Bool success = FALSE;
    GVariantShared variant (ReadVariantAtKeyToShared (plugin,
						      key,
						      TypeButton));
    CCSSettingButtonValue value = readButtonFromVariant (variant.get (), &success);
    EXPECT_TRUE (success);
    return value;
}

unsigned int
CCSGSettingsStorageEnv::ReadEdgeAtKey (const std::string &plugin,
					  const std::string &key)
{
    GVariantShared variant (ReadVariantAtKeyToShared (plugin,
						      key,
						      TypeEdge));
    return readEdgeFromVariant (variant.get ());
}

const char *
CCSGSettingsStorageEnv::ReadMatchAtKey (const std::string &plugin,
					const std::string &key)
{
    GVariantShared variant (ReadVariantAtKeyToShared (plugin,
						      key,
						      TypeMatch));
    return readStringFromVariant (variant.get ());
}

Bool
CCSGSettingsStorageEnv::ReadBellAtKey (const std::string &plugin,
				       const std::string &key)
{
    GVariantShared variant (ReadVariantAtKeyToShared (plugin,
						      key,
						      TypeBell));
    return readBoolFromVariant (variant.get ());
}

CCSSettingValueList
CCSGSettingsStorageEnv::ReadListAtKey (const std::string &plugin,
				       const std::string &key,
				       CCSSetting        *setting)
{
    GVariantShared variant (ReadVariantAtKeyToShared (plugin,
						      key,
						      TypeList));
    return readListValue (variant.get (), setting, &ccsDefaultObjectAllocator);
}

GVariantShared
CCSGSettingsStorageEnv::ReadVariantAtKeyToShared (const std::string &plugin,
						  const std::string &key,
						  CCSSettingType    type)
{
    CharacterWrapper translatedKey (translateKeyForGSettings (key.c_str ()));
    CharacterWrapper pluginPath (makeCompizPluginPath (priv->profileName.c_str (),
						       plugin.c_str ()));

    GVariant *rawVariant = getVariantAtKey (priv->mSettings,
					    translatedKey,
					    pluginPath,
					    type);

    GVariantShared shared (AutoDestroy (rawVariant, g_variant_unref));



    return shared;
}

PrivateCCSGSettingsStorageEnv::PrivateCCSGSettingsStorageEnv (CCSGSettingsWrapper *settings,
							      const std::string   &profileName) :
    mSettings (settings),
    profileName (profileName)

{
}

CCSGSettingsStorageEnv::CCSGSettingsStorageEnv (CCSGSettingsWrapper *settings,
						const std::string   &profileName) :
    priv (new PrivateCCSGSettingsStorageEnv (settings,
					     profileName))
{
}
