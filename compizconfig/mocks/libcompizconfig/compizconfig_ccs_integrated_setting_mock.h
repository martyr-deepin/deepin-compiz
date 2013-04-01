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
#ifndef _COMPIZCONFIG_CCS_INTEGRATED_SETTING_MOCK_H
#define _COMPIZCONFIG_CCS_INTEGRATED_SETTING_MOCK_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <ccs.h>
#include <ccs-backend.h>

CCSIntegratedSetting *
ccsMockIntegratedSettingNew (CCSObjectAllocationInterface *ai);

void
ccsMockIntegratedSettingFree (CCSIntegratedSetting *);

class CCSIntegratedSettingGMockInterface
{
    public:

	virtual ~CCSIntegratedSettingGMockInterface () {}

	virtual CCSSettingValue * readValue (CCSSettingType) = 0;
	virtual void writeValue (CCSSettingValue *, CCSSettingType) = 0;
};

class CCSIntegratedSettingGMock :
    public CCSIntegratedSettingGMockInterface
{
    public:

	MOCK_METHOD1 (readValue, CCSSettingValue * (CCSSettingType));
	MOCK_METHOD2 (writeValue, void (CCSSettingValue *, CCSSettingType));

	CCSIntegratedSettingGMock (CCSIntegratedSetting *integratedSetting) :
	    mIntegrationSetting (integratedSetting)
	{
	}

	CCSIntegratedSetting *
	getIntegratedSetting ()
	{
	    return mIntegrationSetting;
	}

    public:

	static CCSSettingValue *
	ccsIntegratedSettingReadValue (CCSIntegratedSetting *integratedSetting,
				       CCSSettingType       type)
	{
	    return reinterpret_cast <CCSIntegratedSettingGMockInterface *> (ccsObjectGetPrivate (integratedSetting))->readValue (type);
	}

	static void
	ccsIntegratedSettingWriteValue (CCSIntegratedSetting *integratedSetting,
					CCSSettingValue      *value,
					CCSSettingType       type)
	{
	    reinterpret_cast <CCSIntegratedSettingGMockInterface *> (ccsObjectGetPrivate (integratedSetting))->writeValue (value, type);
	}

	static void
	ccsIntegratedSettingFree (CCSIntegratedSetting *integratedSetting)
	{
	    ccsMockIntegratedSettingFree (integratedSetting);
	}

    private:

	CCSIntegratedSetting *mIntegrationSetting;
};

extern const CCSIntegratedSettingInterface mockIntegratedSettingInterface;

#endif
