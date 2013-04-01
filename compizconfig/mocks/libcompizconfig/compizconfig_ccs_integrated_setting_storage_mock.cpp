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

#include <ccs.h>

#include "compizconfig_ccs_integrated_setting_storage_mock.h"

const CCSIntegratedSettingsStorageInterface mockIntegratedSettingsStorageInterface =
{
    CCSIntegratedSettingsStorageGMock::ccsIntegratedSettingsStorageFindMatchingSettingsByPredicate,
    CCSIntegratedSettingsStorageGMock::ccsIntegratedSettingsStorageFindMatchingSettingsByPluginAndSettingName,
    CCSIntegratedSettingsStorageGMock::ccsIntegratedSettingsStorageAddSetting,
    CCSIntegratedSettingsStorageGMock::ccsIntegratedSettingsStorageEmpty,
    CCSIntegratedSettingsStorageGMock::ccsIntegratedSettingsStorageFree
};

CCSIntegratedSettingsStorage *
ccsMockIntegratedSettingsStorageNew (CCSObjectAllocationInterface *ai)
{
    CCSIntegratedSettingsStorage *storage =
	    reinterpret_cast <CCSIntegratedSettingsStorage *> ((*ai->calloc_) (ai->allocator, 1, sizeof (CCSIntegratedSettingsStorage)));

    if (!storage)
	return NULL;

    CCSIntegratedSettingsStorageGMock *gmockBackend = new CCSIntegratedSettingsStorageGMock (storage);

    ccsObjectInit (storage, ai);
    ccsObjectSetPrivate (storage, (CCSPrivate *) gmockBackend);
    ccsObjectAddInterface (storage,
			   reinterpret_cast <const CCSInterface *> (&mockIntegratedSettingsStorageInterface),
			   GET_INTERFACE_TYPE (CCSIntegratedSettingsStorageInterface));

    ccsObjectRef (storage);

    return storage;
}

void
ccsMockIntegratedSettingsStorageFree (CCSIntegratedSettingsStorage *storage)
{
    CCSIntegratedSettingsStorageGMock *gmockStorage =
	    GET_PRIVATE (CCSIntegratedSettingsStorageGMock, storage);

    delete gmockStorage;

    ccsObjectSetPrivate (storage, NULL);
    ccsObjectFinalize (storage);
    (*storage->object.object_allocation->free_)
	    (storage->object.object_allocation->allocator, storage);
}
