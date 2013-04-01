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
#ifndef _COMPIZCONFIG_CCS_SETTINGS_TEST_FIXTURE_H
#define _COMPIZCONFIG_CCS_SETTINGS_TEST_FIXTURE_H
#include <string>
#include <ccs-defs.h>
#include "compizconfig_ccs_variant_types.h"

typedef enum _CCSSettingType CCSSettingType;
typedef struct _CCSSetting   CCSSetting;

namespace compizconfig
{
    namespace test
    {
	Bool boolToBool (bool v);
    }
}

class CCSSettingsConceptTestEnvironmentInterface
{
    public:

	virtual void SetUp () = 0;
	virtual void TearDown () = 0;

	virtual void WriteBoolAtKey (const std::string  &plugin,
				     const std::string  &key,
				     const VariantTypes   &value) = 0;
	virtual void WriteIntegerAtKey (const std::string &plugin,
					const std::string &key,
					const VariantTypes &value) = 0;
	virtual void WriteFloatAtKey (const std::string  &plugin,
				      const std::string  &key,
				      const VariantTypes &value) = 0;
	virtual void WriteStringAtKey (const std::string  &plugin,
				       const std::string  &key,
				       const VariantTypes &value) = 0;
	virtual void WriteColorAtKey (const std::string  &plugin,
				      const std::string  &key,
				      const VariantTypes &value) = 0;
	virtual void WriteKeyAtKey (const std::string  &plugin,
				    const std::string  &key,
				    const VariantTypes &value) = 0;
	virtual void WriteButtonAtKey (const std::string  &plugin,
				       const std::string  &key,
				       const VariantTypes &value) = 0;
	virtual void WriteEdgeAtKey (const std::string  &plugin,
				     const std::string  &key,
				     const VariantTypes &value) = 0;
	virtual void WriteMatchAtKey (const std::string  &plugin,
				      const std::string  &key,
				      const VariantTypes &value) = 0;
	virtual void WriteBellAtKey (const std::string  &plugin,
				     const std::string  &key,
				     const VariantTypes &value) = 0;
	virtual void WriteListAtKey (const std::string  &plugin,
				     const std::string  &key,
				     const VariantTypes &value) = 0;

	virtual Bool ReadBoolAtKey (const std::string  &plugin,
				    const std::string  &key) = 0;
	virtual int ReadIntegerAtKey (const std::string &plugin,
				      const std::string &key) = 0;
	virtual float ReadFloatAtKey (const std::string &plugin,
				      const std::string &key) = 0;
	virtual const char * ReadStringAtKey (const std::string &plugin,
					      const std::string &key) = 0;
	virtual CCSSettingColorValue ReadColorAtKey (const std::string &plugin,
						     const std::string &key) = 0;
	virtual CCSSettingKeyValue ReadKeyAtKey (const std::string &plugin,
						 const std::string &key) = 0;
	virtual CCSSettingButtonValue ReadButtonAtKey (const std::string &plugin,
						       const std::string &key) = 0;
	virtual unsigned int ReadEdgeAtKey (const std::string &plugin,
					    const std::string &key) = 0;
	virtual const char * ReadMatchAtKey (const std::string &plugin,
					     const std::string &key) = 0;
	virtual Bool ReadBellAtKey (const std::string &plugin,
				    const std::string &key) = 0;
	virtual CCSSettingValueList ReadListAtKey (const std::string &plugin,
						   const std::string &key,
						   CCSSetting        *setting) = 0;
};

#endif
