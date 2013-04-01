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

#include "compizconfig_ccs_integration_mock.h"

const CCSIntegrationInterface mockIntegrationBackendInterface =
{
    CCSIntegrationGMock::ccsIntegrationGetIntegratedOptionIndex,
    CCSIntegrationGMock::ccsIntegrationReadOptionIntoSetting,
    CCSIntegrationGMock::ccsIntegrationWriteSettingIntoOption,
    CCSIntegrationGMock::ccsIntegrationUpdateIntegratedSettings,
    CCSIntegrationGMock::ccsIntegrationDisallowIntegratedWrites,
    CCSIntegrationGMock::ccsIntegrationAllowIntegratedWrites,
    CCSIntegrationGMock::ccsFreeIntegration
};

CCSIntegration *
ccsMockIntegrationBackendNew (CCSObjectAllocationInterface *ai)
{
    CCSIntegration *integration = reinterpret_cast <CCSIntegration *> ((*ai->calloc_) (ai->allocator, 1, sizeof (CCSIntegration)));

    if (!integration)
	return NULL;

    CCSIntegrationGMock *gmockBackend = new CCSIntegrationGMock (integration);

    ccsObjectInit (integration, ai);
    ccsObjectSetPrivate (integration, (CCSPrivate *) gmockBackend);
    ccsObjectAddInterface (integration, (const CCSInterface *) &mockIntegrationBackendInterface, GET_INTERFACE_TYPE (CCSIntegrationInterface));

    ccsObjectRef (integration);

    return integration;
}

void
ccsMockIntegrationBackendFree (CCSIntegration *integration)
{
    CCSIntegrationGMock *gmockBackend = reinterpret_cast <CCSIntegrationGMock *> (ccsObjectGetPrivate (integration));

    delete gmockBackend;

    ccsObjectSetPrivate (integration, NULL);
    ccsObjectFinalize (integration);
    (*integration->object.object_allocation->free_) (integration->object.object_allocation->allocator, integration);
}
