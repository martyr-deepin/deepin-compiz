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
#ifndef _COMPIZCONFIG_CCS_GSETTINGS_SETTINGS_ENV_TEST_H
#define _COMPIZCONFIG_CCS_GSETTINGS_SETTINGS_ENV_TEST_H

#include <memory>
#include <boost/shared_ptr.hpp>

#include "compizconfig_ccs_settings_test_fixture.h"

typedef boost::shared_ptr <GVariant> GVariantShared;
typedef struct _CCSSetting CCSSetting;
typedef struct _CCSGSettingsWrapper CCSGSettingsWrapper;
typedef enum   _CCSSettingType CCSSettingType;

class PrivateCCSGSettingsStorageEnv;

class CCSGSettingsStorageEnv :
    public CCSSettingsConceptTestEnvironmentInterface
{
    public:

	virtual void SetUp () {}
	virtual void TearDown () {}

	CCSGSettingsStorageEnv (CCSGSettingsWrapper *settings,
				const std::string   &profileName);

	void WriteBoolAtKey (const std::string  &plugin,
			     const std::string  &key,
			     const VariantTypes &value);
	void WriteIntegerAtKey (const std::string  &plugin,
				const std::string  &key,
				const VariantTypes &value);

	void WriteFloatAtKey (const std::string &plugin,
			      const std::string  &key,
			      const VariantTypes  &value);

	void WriteStringAtKey (const std::string &plugin,
			       const std::string  &key,
			       const VariantTypes  &value);

	void WriteColorAtKey (const std::string  &plugin,
			      const std::string  &key,
			      const VariantTypes &value);

	void WriteKeyAtKey (const std::string  &plugin,
			    const std::string  &key,
			    const VariantTypes &value);

	void WriteButtonAtKey (const std::string &plugin,
			       const std::string &key,
			       const VariantTypes &value);

	void WriteEdgeAtKey (const std::string  &plugin,
			     const std::string  &key,
			     const VariantTypes &value);

	void WriteMatchAtKey (const std::string  &plugin,
			      const std::string  &key,
			      const VariantTypes &value);

	void WriteBellAtKey (const std::string  &plugin,
			     const std::string  &key,
			     const VariantTypes &value);

	void WriteListAtKey (const std::string  &plugin,
			     const std::string  &key,
			     const VariantTypes &value);

	Bool ReadBoolAtKey (const std::string &plugin,
			    const std::string &key);

	int ReadIntegerAtKey (const std::string &plugin,
			      const std::string &key);

	float ReadFloatAtKey (const std::string &plugin,
			      const std::string &key);

	const char * ReadStringAtKey (const std::string &plugin,
				      const std::string &key);

	CCSSettingColorValue ReadColorAtKey (const std::string &plugin,
					     const std::string &key);

	CCSSettingKeyValue ReadKeyAtKey (const std::string &plugin,
					 const std::string &key);
	CCSSettingButtonValue ReadButtonAtKey (const std::string &plugin,
					       const std::string &key);

	unsigned int ReadEdgeAtKey (const std::string &plugin,
				    const std::string &key);

	const char * ReadMatchAtKey (const std::string &plugin,
				     const std::string &key);
	Bool ReadBellAtKey (const std::string &plugin,
			    const std::string &key);

	CCSSettingValueList ReadListAtKey (const std::string &plugin,
					   const std::string &key,
					   CCSSetting        *setting);

    private:

	GVariantShared
	ReadVariantAtKeyToShared (const std::string &plugin,
				  const std::string &key,
				  CCSSettingType    type);

	std::auto_ptr <PrivateCCSGSettingsStorageEnv> priv;
};

#endif
