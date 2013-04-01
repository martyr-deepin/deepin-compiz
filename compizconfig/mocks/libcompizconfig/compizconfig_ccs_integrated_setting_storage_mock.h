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
#ifndef _COMPIZCONFIG_CCS_INTEGRATED_SETTING_STORAGE_MOCK_H
#define _COMPIZCONFIG_CCS_INTEGRATED_SETTING_STORAGE_MOCK_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <ccs.h>
#include <ccs-backend.h>

CCSIntegratedSettingsStorage *
ccsMockIntegratedSettingsStorageNew (CCSObjectAllocationInterface *ai);

void
ccsMockIntegratedSettingsStorageFree (CCSIntegratedSettingsStorage *);

class CCSIntegratedSettingsStorageGMockInterface
{
    public:

	virtual ~CCSIntegratedSettingsStorageGMockInterface () {}

	virtual CCSIntegratedSettingList findMatchingSettingsByPluginAndSettingName (const char *pluginName,
										     const char *settingName) = 0;
	virtual void addSetting (CCSIntegratedSetting *setting) = 0;
	virtual CCSIntegratedSettingList findMatchingSettingsByPredicate (CCSIntegratedSettingsStorageFindPredicate pred,
									  void                                      *data) = 0;
	virtual Bool empty () = 0;
};

class CCSIntegratedSettingsStorageGMock :
    public CCSIntegratedSettingsStorageGMockInterface
{
    public:

	MOCK_METHOD2 (findMatchingSettingsByPluginAndSettingName, CCSIntegratedSettingList (const char *,
											    const char *));
	MOCK_METHOD1 (addSetting, void (CCSIntegratedSetting *));
	MOCK_METHOD2 (findMatchingSettingsByPredicate, CCSIntegratedSettingList (CCSIntegratedSettingsStorageFindPredicate ,
										 void                                      *));
	MOCK_METHOD0 (empty, Bool ());

	CCSIntegratedSettingsStorageGMock (CCSIntegratedSettingsStorage *integratedSetting) :
	    mIntegrationSetting (integratedSetting)
	{
	}

	CCSIntegratedSettingsStorage *
	getIntegratedSettingsStorage ()
	{
	    return mIntegrationSetting;
	}

    public:

	static CCSIntegratedSettingList
	ccsIntegratedSettingsStorageFindMatchingSettingsByPredicate (CCSIntegratedSettingsStorage              *storage,
								     CCSIntegratedSettingsStorageFindPredicate pred,
								     void			               *data)
	{
	    return reinterpret_cast <CCSIntegratedSettingsStorageGMockInterface *> (ccsObjectGetPrivate (storage))->findMatchingSettingsByPredicate (pred, data);
	}

	static CCSIntegratedSettingList
	ccsIntegratedSettingsStorageFindMatchingSettingsByPluginAndSettingName (CCSIntegratedSettingsStorage *storage,
										const char                   *pluginName,
										const char                   *settingName)
	{
	    return reinterpret_cast <CCSIntegratedSettingsStorageGMockInterface *> (ccsObjectGetPrivate (storage))->findMatchingSettingsByPluginAndSettingName (pluginName, settingName);
	}

	static void
	ccsIntegratedSettingsStorageAddSetting (CCSIntegratedSettingsStorage *storage,
						CCSIntegratedSetting	     *setting)
	{
	    return reinterpret_cast <CCSIntegratedSettingsStorageGMockInterface *> (ccsObjectGetPrivate (storage))->addSetting (setting);
	}

	static Bool
	ccsIntegratedSettingsStorageEmpty (CCSIntegratedSettingsStorage *storage)
	{
	    return reinterpret_cast <CCSIntegratedSettingsStorageGMockInterface *> (ccsObjectGetPrivate (storage))->empty ();
	}

	static void
	ccsIntegratedSettingsStorageFree (CCSIntegratedSettingsStorage *integratedSetting)
	{
	    ccsMockIntegratedSettingsStorageFree (integratedSetting);
	}

    private:

	CCSIntegratedSettingsStorage *mIntegrationSetting;
};

extern const CCSIntegratedSettingsStorageInterface mockIntegratedSettingsStorageInterface;

#endif
