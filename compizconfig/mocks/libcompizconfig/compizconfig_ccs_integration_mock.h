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
#ifndef _COMPIZCONFIG_CCS_INTEGRATION_MOCK_H
#define _COMPIZCONFIG_CCS_INTEGRATION_MOCK_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <ccs.h>

CCSIntegration *
ccsMockIntegrationBackendNew (CCSObjectAllocationInterface *ai);

void
ccsMockIntegrationBackendFree (CCSIntegration *integration);

class CCSIntegrationGMockInterface
{
    public:

	virtual ~CCSIntegrationGMockInterface () {}

	virtual CCSIntegratedSetting * getIntegratedOptionIndex (const char *pluginName, const char *settingName) = 0;
	virtual Bool readOptionIntoSetting (CCSContext *context, CCSSetting *setting, CCSIntegratedSetting *) = 0;
	virtual void writeOptionFromSetting (CCSContext *context, CCSSetting *setting, CCSIntegratedSetting *) = 0;
	virtual void updateIntegratedSettings (CCSContext *context, CCSIntegratedSettingList settingList) = 0;
	virtual void disallowIntegratedWrites () = 0;
	virtual void allowIntegratedWrites () = 0;
};

class CCSIntegrationGMock :
    public CCSIntegrationGMockInterface
{
    public:

	MOCK_METHOD2 (getIntegratedOptionIndex, CCSIntegratedSetting * (const char *, const char *));
	MOCK_METHOD3 (readOptionIntoSetting, Bool (CCSContext *, CCSSetting *, CCSIntegratedSetting *));
	MOCK_METHOD3 (writeOptionFromSetting, void (CCSContext *, CCSSetting *, CCSIntegratedSetting *));
	MOCK_METHOD2 (updateIntegratedSettings, void (CCSContext *, CCSIntegratedSettingList));
	MOCK_METHOD0 (disallowIntegratedWrites, void ());
	MOCK_METHOD0 (allowIntegratedWrites, void ());


	CCSIntegrationGMock (CCSIntegration *integration) :
	    mIntegration (integration)
	{
	}

	CCSIntegration *
	getIntegrationBackend ()
	{
	    return mIntegration;
	}

    public:

	static CCSIntegratedSetting * ccsIntegrationGetIntegratedOptionIndex (CCSIntegration *integration,
									      const char     *pluginName,
									      const char     *settingName)
	{
	    return reinterpret_cast <CCSIntegrationGMockInterface *> (ccsObjectGetPrivate (integration))->getIntegratedOptionIndex (pluginName, settingName);
	}

	static Bool ccsIntegrationReadOptionIntoSetting (CCSIntegration       *integration,
							 CCSContext           *context,
							 CCSSetting           *setting,
							 CCSIntegratedSetting *integrated)
	{
	    return reinterpret_cast <CCSIntegrationGMockInterface *> (ccsObjectGetPrivate (integration))->readOptionIntoSetting (context, setting, integrated);
	}

	static void ccsIntegrationWriteSettingIntoOption  (CCSIntegration      *integration,
							   CCSContext		*context,
							   CCSSetting		*setting,
							   CCSIntegratedSetting *integrated)
	{
	    return reinterpret_cast <CCSIntegrationGMockInterface *> (ccsObjectGetPrivate (integration))->writeOptionFromSetting (context, setting, integrated);
	}

	static void ccsIntegrationUpdateIntegratedSettings (CCSIntegration           *integration,
							    CCSContext	             *context,
							    CCSIntegratedSettingList settingList)
	{
	    return reinterpret_cast <CCSIntegrationGMockInterface *> (ccsObjectGetPrivate (integration))->updateIntegratedSettings (context, settingList);
	}

	static void ccsFreeIntegration (CCSIntegration *integration)
	{
	    ccsMockIntegrationBackendFree (integration);
	}

	static void ccsIntegrationDisallowIntegratedWrites (CCSIntegration *integration)
	{
	    reinterpret_cast <CCSIntegrationGMockInterface *> (ccsObjectGetPrivate (integration))->disallowIntegratedWrites ();
	}

	static void ccsIntegrationAllowIntegratedWrites (CCSIntegration *integration)
	{
	    reinterpret_cast <CCSIntegrationGMockInterface *> (ccsObjectGetPrivate (integration))->allowIntegratedWrites ();
	}

    private:

	CCSIntegration *mIntegration;
};

extern const CCSIntegrationInterface mockIntegrationBackendInterface;

#endif
