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
#ifndef _COMPIZCONFIG_CCS_INTEGRATED_SETTING_FACTORY_MOCK_H
#define _COMPIZCONFIG_CCS_INTEGRATED_SETTING_FACTORY_MOCK_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <ccs.h>
#include <ccs-backend.h>

CCSIntegratedSettingFactory *
ccsMockIntegratedSettingFactoryNew (CCSObjectAllocationInterface *ai);

void
ccsMockIntegratedSettingFactoryFree (CCSIntegratedSettingFactory *);

class CCSIntegratedSettingFactoryGMockInterface
{
    public:

	virtual ~CCSIntegratedSettingFactoryGMockInterface () {}

	virtual CCSIntegratedSetting * createIntegratedSettingForCCSNameAndType (CCSIntegration *integration,
										 const char     *pluginName,
										 const char     *settingName,
										 CCSSettingType type) = 0;
};

class CCSIntegratedSettingFactoryGMock :
    public CCSIntegratedSettingFactoryGMockInterface
{
    public:

	MOCK_METHOD4 (createIntegratedSettingForCCSNameAndType, CCSIntegratedSetting * (CCSIntegration *,
											const char     *,
											const char     *,
											CCSSettingType  ));

	CCSIntegratedSettingFactoryGMock (CCSIntegratedSettingFactory *integratedSettingFactory) :
	    mIntegratedSettingFactory (integratedSettingFactory)
	{
	}

	CCSIntegratedSettingFactory *
	getIntegratedSettingFactory ()
	{
	    return mIntegratedSettingFactory;
	}

    public:

	static CCSIntegratedSetting *
	ccsIntegratedSettingFactoryCreateIntegratedSettingForCCSSettingNameAndType (CCSIntegratedSettingFactory *factory,
										    CCSIntegration              *integration,
										    const char                  *pluginName,
										    const char                  *settingName,
										    CCSSettingType              type)
	{
	    return reinterpret_cast <CCSIntegratedSettingFactoryGMockInterface *> (factory)->createIntegratedSettingForCCSNameAndType (integration,
																       pluginName,
																       settingName,
																       type);
	}

	static void
	ccsIntegratedSettingFactoryFree (CCSIntegratedSettingFactory *integratedSettingFactory)
	{
	    ccsMockIntegratedSettingFactoryFree (integratedSettingFactory);
	}

    private:

	CCSIntegratedSettingFactory *mIntegratedSettingFactory;
};

extern const CCSIntegratedSettingFactoryInterface mockIntegratedSettingFactoryInterface;

#endif
