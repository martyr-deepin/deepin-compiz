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

#include "compizconfig_ccs_integrated_setting_mock.h"

const CCSIntegratedSettingInterface mockIntegratedSettingInterface =
{
    CCSIntegratedSettingGMock::ccsIntegratedSettingReadValue,
    CCSIntegratedSettingGMock::ccsIntegratedSettingWriteValue,
    CCSIntegratedSettingGMock::ccsIntegratedSettingFree
};

CCSIntegratedSetting *
ccsMockIntegratedSettingNew (CCSObjectAllocationInterface *ai)
{
    CCSIntegratedSetting *integratedSetting =
	    reinterpret_cast <CCSIntegratedSetting *> ((*ai->calloc_) (ai->allocator, 1, sizeof (CCSIntegratedSetting)));

    if (!integratedSetting)
	return NULL;

    CCSIntegratedSettingGMock *gmockBackend = new CCSIntegratedSettingGMock (integratedSetting);

    ccsObjectInit (integratedSetting, ai);
    ccsObjectSetPrivate (integratedSetting, (CCSPrivate *) gmockBackend);
    ccsObjectAddInterface (integratedSetting,
			   reinterpret_cast <const CCSInterface *> (&mockIntegratedSettingInterface),
			   GET_INTERFACE_TYPE (CCSIntegratedSettingInterface));

    ccsObjectRef (integratedSetting);

    return integratedSetting;
}

void
ccsMockIntegratedSettingFree (CCSIntegratedSetting *integration)
{
    CCSIntegratedSettingGMock *gmockIntegration =
	    GET_PRIVATE (CCSIntegratedSettingGMock, integration);

    delete gmockIntegration;

    ccsObjectSetPrivate (integration, NULL);
    ccsObjectFinalize (integration);
    (*integration->object.object_allocation->free_)
	    (integration->object.object_allocation->allocator, integration);
}
